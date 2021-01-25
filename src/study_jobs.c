/*
 ** Copyright 2014-2018 The Earlham Institute
 **
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 **
 **     http://www.apache.org/licenses/LICENSE-2.0
 **
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 */
/*
 * study_jobs.c
 *
 *  Created on: 1 Oct 2018
 *      Author: billy
 */

#define ALLOCATE_STUDY_JOB_CONSTANTS (1)
#include "study_jobs.h"

#include "study.h"
#include "location.h"
#include "string_utils.h"
#include "crop_jobs.h"
#include "location_jobs.h"
#include "field_trial_jobs.h"
#include "treatment_jobs.h"
#include "treatment_factor_jobs.h"
#include "dfw_util.h"
#include "key_value_pair.h"
#include "time_util.h"

#include "plot.h"
#include "row.h"
#include "observation.h"
#include "treatment_factor.h"

#include "string_parameter.h"
#include "string_array_parameter.h"
#include "double_parameter.h"
#include "time_parameter.h"
#include "boolean_parameter.h"
#include "signed_int_parameter.h"
#include "unsigned_int_parameter.h"
#include "json_parameter.h"
#include "mongodb_tool.h"


/*
 * Study parameters
 */

static NamedParameterType S_SEARCH_TRIAL_ID_S = { "Get all studies for this Field Trial", PT_STRING };





#define S_NUM_DIRECTIONS (9)

static const uint32 S_UNKNOWN_DIRECTION_INDEX = S_NUM_DIRECTIONS - 1;

static const KeyValuePair S_DIRECTIONS_P [] =
{
	{ "North", "http://purl.obolibrary.org/obo/NCIT_C45849" },
	{ "North-East", "http://purl.obolibrary.org/obo/NCIT_C45853" },
	{ "East", "http://purl.obolibrary.org/obo/NCIT_C45851" },
	{ "South-East", "http://purl.obolibrary.org/obo/NCIT_C45855" },
	{ "South", "http://purl.obolibrary.org/obo/NCIT_C45850" },
	{ "South-West", "http://purl.obolibrary.org/obo/NCIT_C45856" },
	{ "West", "http://purl.obolibrary.org/obo/NCIT_C45852" },
	{ "North-West", "http://purl.obolibrary.org/obo/NCIT_C45854" },
	{ ST_UNKNOWN_DIRECTION_S, ST_UNKNOWN_DIRECTION_S},
};


static const char * const S_EMPTY_LIST_OPTION_S = "<empty>";

static const char * const S_UNKNOWN_CROP_OPTION_S = "Unknown";

/*
 * STATIC DECLARATIONS
 */

static bool AddStudy (ServiceJob *job_p, ParameterSet *param_set_p, FieldTrialServiceData *data_p);


static bool GetStudyForGivenId (FieldTrialServiceData *data_p, ParameterSet *param_set_p, ServiceJob *job_p, ViewFormat format, JSONProcessor *processor_p);


static bool AddStudyLocationCriteria (bson_t *query_p, ParameterSet *param_set_p);


static bool AddStudyDateCriteria (bson_t *query_p, ParameterSet *param_set_p);


static bool GetMatchingStudies (bson_t *query_p, FieldTrialServiceData *data_p, ServiceJob *job_p, ViewFormat format);


static Parameter *GetAndAddAspectParameter (const char *aspect_s, FieldTrialServiceData *data_p, ParameterSet *param_set_p, ParameterGroup *group_p);


static Parameter *AddPhParameter (const ServiceData *service_data_p, ParameterSet *params_p, ParameterGroup *group_p, const NamedParameterType *param_type_p, const char * const display_name_s, const char * const description_s);


static bool GetValidCrop (const char *crop_s, Crop **crop_pp, const FieldTrialServiceData *data_p);

static Study *GetStudyFromJSONResource (const json_t *resource_data_p, ServiceData *data_p);


static const char *GetStudyDefaultValueFromJSON (const char *study_id_param_s, const json_t *params_json_p);


static bool SetUpDefaultsFromExistingStudy (const Study * const study_p, char **id_ss, char **this_crop_ss, char **previous_crop_ss, char **trial_ss, char **location_ss);


static bool AddDefaultPlotsParameters (ServiceData *data_p, ParameterSet *params_p, const Study *study_p);


static bool AddLayoutParams (ParameterSet *params_p, const Study *study_p, FieldTrialServiceData *dfw_data_p);


static bool AddTermToJSON (const SchemaTerm *term_p, json_t *phenotypes_p);


static bool AddAccession (const char *oid_s, json_t *values_p, const FieldTrialServiceData *data_p);


static bool AddPhenotype (const char *oid_s, json_t *values_p, const FieldTrialServiceData *data_p);


static json_t *GetDistinctValuesAsJSON (bson_oid_t *study_id_p, const char *key_s, bool (*add_value_fn) (const char *oid_s, json_t *values_p, const FieldTrialServiceData *data_p), const FieldTrialServiceData *data_p);


static bool AddTreatmentFactorParameters (ParameterSet *params_p, const Study *study_p, FieldTrialServiceData *data_p);

static bool AddTreatmentFactorsToStudy (Study *study_p, Parameter *treatment_names_p, Parameter *treatment_levels_p, const size_t num_treatments, const FieldTrialServiceData *data_p);

/*
 * API DEFINITIONS
 */

bool AddSubmissionStudyParams (ServiceData *data_p, ParameterSet *params_p, Resource *resource_p)
{
	bool success_flag = false;
	Parameter *param_p = NULL;
	ParameterGroup *group_p = CreateAndAddParameterGroupToParameterSet ("Study", false, data_p, params_p);
	FieldTrialServiceData *dfw_data_p = (FieldTrialServiceData *) data_p;

	Study *active_study_p = GetStudyFromResource (resource_p, STUDY_ID, dfw_data_p);
	bool defaults_flag = false;

	char *id_s = NULL;
	char *this_crop_s= NULL;
	char *previous_crop_s = NULL;
	char *trial_s = NULL;
	char *location_s = NULL;

	if (active_study_p)
		{
			if (SetUpDefaultsFromExistingStudy (active_study_p, &id_s, &this_crop_s, &previous_crop_s, &trial_s, &location_s))
				{
					defaults_flag = true;
				}
		}
	else
		{
			id_s = (char *) S_EMPTY_LIST_OPTION_S;
			defaults_flag = true;
		}

	if (defaults_flag)
		{
			if ((param_p = EasyCreateAndAddStringParameterToParameterSet (data_p, params_p, group_p, STUDY_ID.npt_type, STUDY_ID.npt_name_s, "Load Study", "Edit an existing study", id_s, PL_ALL)) != NULL)
				{
					if (SetUpStudiesListParameter (dfw_data_p, (StringParameter *) param_p, NULL, true))
						{
							/*
							 * We want to update all of the values in the form
							 * when a user selects a study from the list so
							 * we need to make the parameter automatically
							 * refresh the values. So we set the
							 * pa_refresh_service_flag to true.
							 */
							param_p -> pa_refresh_service_flag = true;

							if ((param_p = EasyCreateAndAddStringParameterToParameterSet (data_p, params_p, group_p, STUDY_NAME.npt_type, STUDY_NAME.npt_name_s, "Name", "The name of the Study", active_study_p ? active_study_p -> st_name_s : NULL, PL_ALL)) != NULL)
								{
									param_p -> pa_required_flag = true;

									param_p = EasyCreateAndAddStringParameterToParameterSet (data_p, params_p, group_p, STUDY_FIELD_TRIALS_LIST.npt_type, STUDY_FIELD_TRIALS_LIST.npt_name_s, "Field trials", "The available field trials", trial_s, PL_ALL);

									if (param_p)
										{
											if (SetUpFieldTrialsListParameter (dfw_data_p, (StringParameter *) param_p, active_study_p ? active_study_p -> st_parent_p : NULL, false))
												{
													if ((param_p = EasyCreateAndAddStringParameterToParameterSet (data_p, params_p, group_p, STUDY_LOCATIONS_LIST.npt_type, STUDY_LOCATIONS_LIST.npt_name_s, "Locations", "The available locations", location_s, PL_ALL)) != NULL)
														{
															if (SetUpLocationsListParameter (dfw_data_p, (StringParameter *) param_p, active_study_p ? active_study_p -> st_location_p : NULL, NULL))
																{
																	if ((param_p = EasyCreateAndAddTimeParameterToParameterSet (data_p, params_p, group_p, STUDY_SOWING_YEAR.npt_name_s, "Sowing date", "The sowing year for the Study", active_study_p ? active_study_p -> st_sowing_date_p : NULL, PL_ALL)) != NULL)
																		{
																			if ((param_p = EasyCreateAndAddTimeParameterToParameterSet (data_p, params_p, group_p, STUDY_HARVEST_YEAR.npt_name_s, "Harvest date", "The harvest date for the Study", active_study_p ? active_study_p -> st_harvest_date_p : NULL, PL_ALL)) != NULL)
																				{
																					if ((param_p = EasyCreateAndAddStringParameterToParameterSet (data_p, params_p, group_p, STUDY_DESCRIPTION.npt_type, STUDY_DESCRIPTION.npt_name_s, "Description", "A description of the Study", active_study_p ? active_study_p -> st_description_s : NULL, PL_ALL)) != NULL)
																						{
																							if ((param_p = EasyCreateAndAddStringParameterToParameterSet (data_p, params_p, group_p, STUDY_DESIGN.npt_type, STUDY_DESIGN.npt_name_s, "Design", "Information about the Study design", active_study_p ? active_study_p -> st_design_s : NULL, PL_ALL)) != NULL)
																								{
																									if ((param_p = EasyCreateAndAddStringParameterToParameterSet (data_p, params_p, group_p, STUDY_GROWING_CONDITIONS.npt_type, STUDY_GROWING_CONDITIONS.npt_name_s, "Growing conditions", "Information about the Growing conditions", active_study_p ? active_study_p -> st_growing_conditions_s : NULL, PL_ALL)) != NULL)
																										{
																											if ((param_p = EasyCreateAndAddStringParameterToParameterSet (data_p, params_p, group_p, STUDY_PHENOTYPE_GATHERING_NOTES.npt_type, STUDY_PHENOTYPE_GATHERING_NOTES.npt_name_s, "Phenotype gathering notes", "Notes on how the Phenotype information was gathered", active_study_p ? active_study_p -> st_phenotype_gathering_notes_s : NULL, PL_ALL)) != NULL)
																												{
																													if ((param_p = EasyCreateAndAddStringParameterToParameterSet (data_p, params_p, group_p, STUDY_WEATHER_LINK.npt_type, STUDY_WEATHER_LINK.npt_name_s, "Weather", "Link out to the weather data for this study", active_study_p ? active_study_p -> st_weather_link_s : NULL, PL_ALL)) != NULL)
																														{
																															if (AddLayoutParams (params_p, active_study_p, dfw_data_p))
																																{
																																	if ((param_p = EasyCreateAndAddStringParameterToParameterSet (data_p, params_p, group_p, STUDY_THIS_CROP.npt_type, STUDY_THIS_CROP.npt_name_s, "Crop", "The crop variety for this study", this_crop_s, PL_ALL)) != NULL)
																																		{
																																			if (SetUpCropsListParameter (dfw_data_p, (StringParameter *) param_p, active_study_p ? active_study_p -> st_current_crop_p : NULL, S_UNKNOWN_CROP_OPTION_S))
																																				{
																																					if ((param_p = EasyCreateAndAddStringParameterToParameterSet (data_p, params_p, group_p, STUDY_PREVIOUS_CROP.npt_type, STUDY_PREVIOUS_CROP.npt_name_s, "Previous Crop", "The previous crop variety planted in this field", previous_crop_s, PL_ALL)) != NULL)
																																						{
																																							if (SetUpCropsListParameter (dfw_data_p, (StringParameter *) param_p, active_study_p ? active_study_p -> st_previous_crop_p : NULL, S_UNKNOWN_CROP_OPTION_S))
																																								{
																																									if (AddPhParameter (data_p, params_p, group_p, &STUDY_MIN_PH, "pH Minimum", "The lower bound of the soil's pH range"))
																																										{
																																											if (AddPhParameter (data_p, params_p, group_p, &STUDY_MAX_PH, "pH Maximum", "The upper bound of the soil's pH range"))
																																												{
																																													if ((param_p = EasyCreateAndAddStringParameterToParameterSet (data_p, params_p, group_p, STUDY_SOIL.npt_type, STUDY_SOIL.npt_name_s, "Soil", "The soil of the Study", active_study_p ? active_study_p -> st_soil_type_s : NULL, PL_ALL)) != NULL)
																																														{
																																															if ((param_p = EasyCreateAndAddStringParameterToParameterSet (data_p, params_p, group_p, STUDY_LINK.npt_type, STUDY_LINK.npt_name_s, "Link", "The url for any downloads relating to this Study", active_study_p ? active_study_p -> st_data_url_s : NULL, PL_ALL)) != NULL)
																																																{
																																																	if (AddDefaultPlotsParameters (data_p, params_p, active_study_p))
																																																		{
																																																			if ((param_p = EasyCreateAndAddJSONParameterToParameterSet (data_p, params_p, group_p, STUDY_SHAPE_DATA.npt_type, STUDY_SHAPE_DATA.npt_name_s, "Plots GPS", "The GeoJSON for the vertices of the plots layout", active_study_p ? active_study_p -> st_shape_p : NULL, PL_ALL)) != NULL)
																																																				{
																																																					success_flag = true;
																																																				}


																																																			if (AddTreatmentFactorParameters (params_p, active_study_p, dfw_data_p))
																																																				{

																																																				}
																																																			else
																																																				{
																																																					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "AddTreatmentFactorParameters failed");
																																																				}


																																																		}
																																																	else
																																																		{
																																																			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "AddDefaultPlotsParameters failed");
																																																		}


																																																}		/* if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, STUDY_LINK.npt_type, STUDY_LINK.npt_name_s, "Link", "The url for any downloads relating to this Study", def, PL_ALL)) != NULL) */
																																															else
																																																{
																																																	PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", STUDY_LINK.npt_name_s);
																																																}
																																														}
																																													else
																																														{
																																															PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", STUDY_SOIL.npt_name_s);
																																														}

																																												}		/* if (AddPhParameter (data_p, param_set_p, group_p, &PH_MAX, "pH Maximum", "The upper bound of the soil's pH range")) */
																																											else
																																												{
																																													PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", STUDY_MAX_PH.npt_name_s);
																																												}
																																										}		/* if (AddPhParameter (data_p, param_set_p, group_p, &PH_MIN, "pH Minimum", "The lower bound of the soil's pH range")) */
																																									else
																																										{
																																											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", STUDY_MIN_PH.npt_name_s);
																																										}

																																								}		/* if (SetUpCropsListParameter (dfw_data_p, param_p)) */
																																							else
																																								{
																																									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "SetUpCropsListParameter failed for \"%s\"", STUDY_PREVIOUS_CROP.npt_name_s);;
																																								}

																																						}		/* if (param_p) */
																																					else
																																						{
																																							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", STUDY_PREVIOUS_CROP.npt_name_s);
																																						}
																																				}		/* if (SetUpCropsListParameter (dfw_data_p, param_p)) */
																																			else
																																				{
																																					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "SetUpCropsListParameter failed for \"%s\"", STUDY_THIS_CROP.npt_name_s);
																																				}

																																		}		/* if (param_p) */
																																	else
																																		{
																																			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", STUDY_THIS_CROP.npt_name_s);
																																		}

																																}
																															else
																																{
																																	PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "AddLayoutParams failed");
																																}

																														}		/* if ((param_p = EasyCreateAndAddStringParameterToParameterSet (data_p, param_set_p, group_p, STUDY_WEATHER_LINK.npt_type, STUDY_WEATHER_LINK.npt_name_s, "Weather", "Link out to the weather data for this study", weather_s, PL_ALL)) != NULL) */
																													else
																														{
																															PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", STUDY_WEATHER_LINK.npt_name_s);
																														}


																												}		/* if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, STUDY_PHENOTYPE_GATHERING_NOTES.npt_type, STUDY_PHENOTYPE_GATHERING_NOTES.npt_name_s, "Phenotype Gathering", "NOtes on hoe the Phenotype information was gathered", def, PL_ALL)) != NULL) */
																											else
																												{
																													PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", STUDY_GROWING_CONDITIONS.npt_name_s);
																												}

																										}		/* if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, STUDY_GROWING_CONDITIONS.npt_type, STUDY_GROWING_CONDITIONS.npt_name_s, "Growing Conditions", "Information about the Growing conditions", def, PL_ALL)) != NULL) */
																									else
																										{
																											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", STUDY_GROWING_CONDITIONS.npt_name_s);
																										}

																								}		/* if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, STUDY_DESIGN.npt_type, STUDY_DESIGN.npt_name_s, "Design", "Information about the Study design", def, PL_ALL)) != NULL) */
																							else
																								{
																									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", STUDY_DESIGN.npt_name_s);
																								}

																						}		/* if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, STUDY_NOTES.npt_type, STUDY_NOTES.npt_name_s, "Notes", "Any additional information about the study", def, PL_ALL)) != NULL) */
																					else
																						{
																							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", STUDY_DESCRIPTION.npt_name_s);
																						}

																				}
																			else
																				{
																					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", STUDY_HARVEST_YEAR.npt_name_s);
																				}
																		}
																	else
																		{
																			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", STUDY_SOWING_YEAR.npt_name_s);
																		}



																}
															else
																{
																	PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "SetUpLocationsListParameter failed");
																}
														}
													else
														{
															PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", STUDY_FIELD_TRIALS_LIST.npt_name_s);
														}
												}
											else
												{
													PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "SetUpFieldTrialsListParameter failed");
												}
										}
									else
										{
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", STUDY_FIELD_TRIALS_LIST.npt_name_s);
										}


								}
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", STUDY_NAME.npt_name_s);
								}
						}		/* if (SetUpStudiesListParameter (data_p, param_p)) */
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "SetUpStudiesListParameter failed");
						}

				}		/* if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, STUDY_ID.npt_type, STUDY_ID.npt_name_s, "Load Study", "Edit an existing study", def, PL_ADVANCED)) != NULL) */
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", STUDY_ID.npt_name_s);
				}

		}		/* if (defaults_flag) */


	if (active_study_p)
		{
			FreeStudy (active_study_p);
		}

	if (this_crop_s)
		{
			FreeCopiedString (this_crop_s);
		}

	if (previous_crop_s)
		{
			FreeCopiedString (previous_crop_s);
		}

	if (trial_s)
		{
			FreeCopiedString (trial_s);
		}

	if (location_s)
		{
			FreeCopiedString (location_s);
		}

	return success_flag;
}


json_t *GetStudyIndexingData (Service *service_p)
{
	FieldTrialServiceData *data_p = (FieldTrialServiceData *) (service_p -> se_data_p);
	json_t *src_studies_p = GetAllStudiesAsJSON (data_p);

	if (src_studies_p)
		{
			if (json_is_array (src_studies_p))
				{
					FieldTrialServiceData *dfw_data_p = (FieldTrialServiceData *) (service_p -> se_data_p);
					size_t i;
					json_t *src_study_p;
					size_t num_added = 0;

					json_array_foreach (src_studies_p, i, src_study_p)
						{
							bson_oid_t id;

							if (AddDatatype (src_study_p, DFTD_STUDY))
								{
									if (GetMongoIdFromJSON (src_study_p, &id))
										{
											Crop *crop_p = NULL;
											json_t *values_p = GetStudyDistinctPhenotypesAsJSON (&id, dfw_data_p);

											if (values_p)
												{
													if (json_object_set_new (src_study_p, "phenotypes", values_p) != 0)
														{
															PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, src_study_p, "Failed to add phenotypes");
															json_decref (values_p);
														}
												}

											values_p = GetStudyDistinctAccessionsAsJSON (&id, dfw_data_p);

											if (values_p)
												{
													if (json_object_set_new (src_study_p, "accessions", values_p) != 0)
														{
															PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, src_study_p, "Failed to add accessions");
															json_decref (values_p);
														}
												}


											if (GetNamedIdFromJSON (src_study_p, ST_PARENT_FIELD_TRIAL_S, &id))
												{
													FieldTrial *trial_p = GetFieldTrialById (&id, VF_STORAGE, data_p);

													json_object_del (src_study_p, ST_PARENT_FIELD_TRIAL_S);

													if (trial_p)
														{
															json_t *trial_json_p = GetFieldTrialAsJSON (trial_p, VF_CLIENT_MINIMAL, data_p);

															if (trial_json_p)
																{
																	json_object_del (trial_json_p, MONGO_ID_S);

																	if (json_object_set_new (src_study_p, ST_PARENT_FIELD_TRIAL_S, trial_json_p) == 0)
																		{

																		}
																	else
																		{
																			json_decref (trial_json_p);
																		}
																}

															FreeFieldTrial (trial_p);
														}		/* if (trial_p) */

												}

											if (GetNamedIdFromJSON (src_study_p, ST_LOCATION_ID_S, &id))
												{
													Location *location_p = GetLocationById (&id, VF_STORAGE, data_p);

													json_object_del (src_study_p, ST_LOCATION_ID_S);

													if (location_p)
														{
															json_t *location_json_p = GetLocationAsJSON (location_p);

															if (location_json_p)
																{
																	json_object_del (location_json_p, MONGO_ID_S);

																	if (json_object_set_new (src_study_p, ST_LOCATION_S, location_json_p) == 0)
																		{

																		}
																	else
																		{
																			json_decref (location_json_p);
																		}

																}

															FreeLocation (location_p);
														}		/* if (location_p) */

												}		/* */


											crop_p = GetStoredCropValue (src_study_p, ST_CURRENT_CROP_S, data_p);
											if (crop_p)
												{
													SetJSONString (src_study_p, ST_CURRENT_CROP_S, crop_p -> cr_name_s);
													FreeCrop (crop_p);
												}

											crop_p = GetStoredCropValue (src_study_p, ST_PREVIOUS_CROP_S, data_p);
											if (crop_p)
												{
													SetJSONString (src_study_p, ST_PREVIOUS_CROP_S, crop_p -> cr_name_s);
													FreeCrop (crop_p);
												}


										}		/* if (GetMongoIdFromJSON (entry_p, &id)) */

								}


						}		/* json_array_foreach (src_studies_p, i, src_study_p) */

				}		/* if (json_is_array (src_studies_p)) */

			return src_studies_p;
		}		/* if (src_studies_p) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "No studies for \"%s\"", GetServiceName (service_p));
		}

	return NULL;
}






static bool AddLayoutParams (ParameterSet *params_p, const Study *study_p, FieldTrialServiceData *dfw_data_p)
{
	ServiceData *data_p = & (dfw_data_p -> dftsd_base_data);
	ParameterGroup *group_p = CreateAndAddParameterGroupToParameterSet ("Layout", false, data_p, params_p);
	Parameter *param_p;

	if ((param_p = GetAndAddAspectParameter (study_p ? study_p -> st_aspect_s : NULL, dfw_data_p, params_p, group_p)) != NULL)
		{
			if ((param_p = EasyCreateAndAddStringParameterToParameterSet (data_p, params_p, group_p, STUDY_SLOPE.npt_type, STUDY_SLOPE.npt_name_s, "Slope", "The slope of the Study", study_p ? study_p -> st_slope_s : NULL, PL_ALL)) != NULL)
				{
					if (EasyCreateAndAddDoubleParameterToParameterSet (data_p, params_p, group_p, STUDY_PLOT_HGAP.npt_type, STUDY_PLOT_HGAP.npt_name_s, "Horizontal plot gap", "The distance (in metres) between plots in a row.", study_p ? study_p -> st_plot_horizontal_gap_p : NULL, PL_ALL))
						{
							if (EasyCreateAndAddDoubleParameterToParameterSet (data_p, params_p, group_p, STUDY_PLOT_VGAP.npt_type, STUDY_PLOT_VGAP.npt_name_s, "Vertical plot gap", "The distance (in metres) between plots in a column.", study_p ? study_p -> st_plot_vertical_gap_p : NULL, PL_ALL))
								{
									if (EasyCreateAndAddUnsignedIntParameterToParameterSet (data_p, params_p, group_p, STUDY_PLOT_ROWS_PER_BLOCK.npt_name_s, "Plot rows per block", "The number of rows of plots in a block", study_p ? study_p -> st_plots_rows_per_block_p : NULL, PL_ALL))
										{
											if (EasyCreateAndAddUnsignedIntParameterToParameterSet (data_p, params_p, group_p, STUDY_PLOT_COLS_PER_BLOCK.npt_name_s, "Plot columns per block", "The number of columns of plots in a block", study_p ? study_p -> st_plots_columns_per_block_p : NULL, PL_ALL))
												{
													if (EasyCreateAndAddDoubleParameterToParameterSet (data_p, params_p, group_p, STUDY_PLOT_BLOCK_HGAP.npt_type, STUDY_PLOT_BLOCK_HGAP.npt_name_s, "Horizontal plot block gap", "The distance (in metres) between blocks of plots in a row.", study_p ? study_p -> st_plot_block_horizontal_gap_p : NULL, PL_ALL))
														{
															if (EasyCreateAndAddDoubleParameterToParameterSet (data_p, params_p, group_p, STUDY_PLOT_BLOCK_VGAP.npt_type, STUDY_PLOT_BLOCK_VGAP.npt_name_s, "Vertical plot block gap", "The distance (in metres) between blocks of plots in a column.", study_p ? study_p -> st_plot_block_vertical_gap_p : NULL, PL_ALL))
																{
																	return true;
																}
															else
																{
																	PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", STUDY_PLOT_BLOCK_VGAP.npt_name_s);
																}
														}
													else
														{
															PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", STUDY_PLOT_BLOCK_HGAP.npt_name_s);
														}
												}
											else
												{
													PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", STUDY_PLOT_VGAP.npt_name_s);
												}
										}
									else
										{
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", STUDY_PLOT_ROWS_PER_BLOCK.npt_name_s);
										}
								}
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", STUDY_PLOT_ROWS_PER_BLOCK.npt_name_s);
								}

						}
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", STUDY_PLOT_ROWS_PER_BLOCK.npt_name_s);
						}

				}
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", STUDY_SLOPE.npt_name_s);
				}
		}
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "GetAndAddAspectParameter failed with aspect \"%s\"", study_p ? study_p -> st_aspect_s : "NULL");
		}

	return false;
}


bool RunForSubmissionStudyParams (FieldTrialServiceData *data_p, ParameterSet *param_set_p, ServiceJob *job_p)
{
	bool success_flag = AddStudy (job_p, param_set_p, data_p);

	return success_flag;
}



bool GetSubmissionStudyParameterTypeForDefaultPlotNamedParameter (const char *param_name_s, ParameterType *pt_p)
{
	bool success_flag = true;

	if (strcmp (param_name_s, STUDY_NUM_PLOT_ROWS.npt_name_s) == 0)
		{
			*pt_p = STUDY_NUM_PLOT_ROWS.npt_type;
		}
	else if (strcmp (param_name_s, STUDY_NUM_PLOT_COLS.npt_name_s) == 0)
		{
			*pt_p = STUDY_NUM_PLOT_COLS.npt_type;
		}
	else if (strcmp (param_name_s, STUDY_NUM_REPLICATES.npt_name_s) == 0)
		{
			*pt_p = STUDY_NUM_REPLICATES.npt_type;
		}
	else if (strcmp (param_name_s, STUDY_PLOT_WIDTH.npt_name_s) == 0)
		{
			*pt_p = STUDY_PLOT_WIDTH.npt_type;
		}
	else if (strcmp (param_name_s, STUDY_PLOT_LENGTH.npt_name_s) == 0)
		{
			*pt_p = STUDY_PLOT_LENGTH.npt_type;
		}
	else if (strcmp (param_name_s, STUDY_SOWING_YEAR.npt_name_s) == 0)
		{
			*pt_p = STUDY_SOWING_YEAR.npt_type;
		}
	else if (strcmp (param_name_s, STUDY_HARVEST_YEAR.npt_name_s) == 0)
		{
			*pt_p = STUDY_HARVEST_YEAR.npt_type;
		}
	else
		{
			success_flag = false;
		}

	return success_flag;
}

bool GetSubmissionStudyParameterTypeForNamedParameter (const char *param_name_s, ParameterType *pt_p)
{
	bool success_flag = true;

	if (strcmp (param_name_s, STUDY_ID.npt_name_s) == 0)
		{
			*pt_p = STUDY_ID.npt_type;
		}
	else if (strcmp (param_name_s, STUDY_NAME.npt_name_s) == 0)
		{
			*pt_p = STUDY_NAME.npt_type;
		}
	else if (strcmp (param_name_s, STUDY_SOIL.npt_name_s) == 0)
		{
			*pt_p = STUDY_SOIL.npt_type;
		}
	else if (strcmp (param_name_s, STUDY_LINK.npt_name_s) == 0)
		{
			*pt_p = STUDY_LINK.npt_type;
		}
	else if (strcmp (param_name_s, STUDY_FIELD_TRIALS_LIST.npt_name_s) == 0)
		{
			*pt_p = STUDY_FIELD_TRIALS_LIST.npt_type;
		}
	else if (strcmp (param_name_s, STUDY_LOCATIONS_LIST.npt_name_s) == 0)
		{
			*pt_p = STUDY_LOCATIONS_LIST.npt_type;
		}
	else if (strcmp (param_name_s, STUDY_ASPECT.npt_name_s) == 0)
		{
			*pt_p = STUDY_ASPECT.npt_type;
		}
	else if (strcmp (param_name_s, STUDY_SLOPE.npt_name_s) == 0)
		{
			*pt_p = STUDY_SLOPE.npt_type;
		}
	else if (strcmp (param_name_s, STUDY_THIS_CROP.npt_name_s) == 0)
		{
			*pt_p = STUDY_THIS_CROP.npt_type;
		}
	else if (strcmp (param_name_s, STUDY_PREVIOUS_CROP.npt_name_s) == 0)
		{
			*pt_p = STUDY_PREVIOUS_CROP.npt_type;
		}
	else if (strcmp (param_name_s, STUDY_MIN_PH.npt_name_s) == 0)
		{
			*pt_p = STUDY_MIN_PH.npt_type;
		}
	else if (strcmp (param_name_s, STUDY_MAX_PH.npt_name_s) == 0)
		{
			*pt_p = STUDY_MAX_PH.npt_type;
		}
	else if (strcmp (param_name_s, STUDY_DESCRIPTION.npt_name_s) == 0)
		{
			*pt_p = STUDY_DESCRIPTION.npt_type;
		}
	else if (strcmp (param_name_s, STUDY_DESIGN.npt_name_s) == 0)
		{
			*pt_p = STUDY_DESIGN.npt_type;
		}
	else if (strcmp (param_name_s, STUDY_GROWING_CONDITIONS.npt_name_s) == 0)
		{
			*pt_p = STUDY_GROWING_CONDITIONS.npt_type;
		}
	else if (strcmp (param_name_s, STUDY_PHENOTYPE_GATHERING_NOTES.npt_name_s) == 0)
		{
			*pt_p = STUDY_PHENOTYPE_GATHERING_NOTES.npt_type;
		}
	else if (strcmp (param_name_s, STUDY_WEATHER_LINK.npt_name_s) == 0)
		{
			*pt_p = STUDY_WEATHER_LINK.npt_type;
		}
	else if (strcmp (param_name_s, STUDY_SHAPE_DATA.npt_name_s) == 0)
		{
			*pt_p = STUDY_SHAPE_DATA.npt_type;
		}
	else if (strcmp (param_name_s, STUDY_PLOT_ROWS_PER_BLOCK.npt_name_s) == 0)
		{
			*pt_p = STUDY_PLOT_ROWS_PER_BLOCK.npt_type;
		}
	else if (strcmp (param_name_s, STUDY_PLOT_COLS_PER_BLOCK.npt_name_s) == 0)
		{
			*pt_p = STUDY_PLOT_COLS_PER_BLOCK.npt_type;
		}
	else if (strcmp (param_name_s, STUDY_PLOT_HGAP.npt_name_s) == 0)
		{
			*pt_p = STUDY_PLOT_HGAP.npt_type;
		}
	else if (strcmp (param_name_s, STUDY_PLOT_VGAP.npt_name_s) == 0)
		{
			*pt_p = STUDY_PLOT_VGAP.npt_type;
		}
	else if (strcmp (param_name_s, STUDY_PLOT_BLOCK_HGAP.npt_name_s) == 0)
		{
			*pt_p = STUDY_PLOT_BLOCK_HGAP.npt_type;
		}
	else if (strcmp (param_name_s, STUDY_PLOT_BLOCK_VGAP.npt_name_s) == 0)
		{
			*pt_p = STUDY_PLOT_BLOCK_VGAP.npt_type;
		}
	else
		{
			success_flag = GetSubmissionStudyParameterTypeForDefaultPlotNamedParameter (param_name_s, pt_p);
		}

	return success_flag;
}


/*
Parameters

    commonCropName
    Common name for the crop associated with this study
    String

    programDbId
    Program filter to only return studies associated with given program id.
    String

    locationDbId
    Filter by location
    String

    seasonDbId
    Filter by season or year
    String

    trialDbId
    Filter by trial
    String

    studyDbId
    Filter by study DbId
    String

		active
    Filter active status true/false.
    String

    sortBy
    Name of the field to sort by.
    String

    sortOrder
    Sort order direction. Ascending/Descending.
    String

    page
    Which result page is requested. The page indexing starts at 0 (the first page is 'page'= 0). Default is 0.
    String

    pageSize
    The size of the pages to be returned. Default is 1000.
    String

 */



bool GetSearchStudyParameterTypeForNamedParameter (const char *param_name_s, ParameterType *pt_p)
{
	bool success_flag = true;

	if (strcmp (param_name_s, STUDY_SEARCH_STUDIES.npt_name_s) == 0)
		{
			*pt_p = STUDY_SEARCH_STUDIES.npt_type;
		}
	else if (strcmp (param_name_s, STUDY_ID.npt_name_s) == 0)
		{
			*pt_p = STUDY_ID.npt_type;
		}
	else if (strcmp (param_name_s, STUDY_GET_ALL_PLOTS.npt_name_s) == 0)
		{
			*pt_p = STUDY_GET_ALL_PLOTS.npt_type;
		}
	else if (strcmp (param_name_s, STUDY_LOCATIONS_LIST.npt_name_s) == 0)
		{
			*pt_p = STUDY_LOCATIONS_LIST.npt_type;
		}
	else if (strcmp (param_name_s, STUDY_SEARCH_ACTIVE_DATE.npt_name_s) == 0)
		{
			*pt_p = STUDY_SEARCH_ACTIVE_DATE.npt_type;
		}
	else if (strcmp (param_name_s, S_SEARCH_TRIAL_ID_S.npt_name_s) == 0)
		{
			*pt_p = S_SEARCH_TRIAL_ID_S.npt_type;
		}
	else
		{
			success_flag = false;
		}

	return success_flag;
}



bool AddSearchStudyParams (ServiceData *data_p, ParameterSet *param_set_p)
{
	bool success_flag = false;
	Parameter *param_p = NULL;
	const char * const group_name_s = "Studies";
	ParameterGroup *group_p = CreateAndAddParameterGroupToParameterSet (group_name_s, false, data_p, param_set_p);

	if (group_p)
		{
			bool search_flag = false;

			if ((param_p = EasyCreateAndAddBooleanParameterToParameterSet (data_p, param_set_p, group_p, STUDY_SEARCH_STUDIES.npt_name_s, "Search Studies", "Get the matching Studies", &search_flag, PL_ADVANCED)) != NULL)
				{
					if ((param_p = EasyCreateAndAddStringParameterToParameterSet (data_p, param_set_p, group_p, STUDY_ID.npt_type, STUDY_ID.npt_name_s, "Id", "The id of the Study", NULL, PL_ADVANCED)) != NULL)
						{
							if ((param_p = EasyCreateAndAddBooleanParameterToParameterSet (data_p, param_set_p, group_p, STUDY_GET_ALL_PLOTS.npt_name_s, "Plots", "Get all of the plots", &search_flag, PL_ADVANCED)) != NULL)
								{
									if ((param_p = EasyCreateAndAddStringParameterToParameterSet (data_p, param_set_p, group_p, S_SEARCH_TRIAL_ID_S.npt_type, S_SEARCH_TRIAL_ID_S.npt_name_s, "Parent Field Trial", "Get all Studies for a given Field Trial", NULL, PL_ADVANCED)) != NULL)
										{
											if ((param_p = EasyCreateAndAddStringParameterToParameterSet (data_p, param_set_p, group_p, STUDY_LOCATIONS_LIST.npt_type, STUDY_LOCATIONS_LIST.npt_name_s, "Locations", "The available locations", NULL, PL_ADVANCED)) != NULL)
												{
													if (SetUpLocationsListParameter ((FieldTrialServiceData *) data_p, (StringParameter *) param_p, NULL, GetUnsetLocationValue ()))
														{
															struct tm t;

															ClearTime (&t);
															SetDateValuesForTime (&t, 2017, 1, 1);

															if ((param_p = EasyCreateAndAddTimeParameterToParameterSet (data_p, param_set_p, group_p, STUDY_SEARCH_ACTIVE_DATE.npt_name_s, "Active date", "Date during which the study was active", &t, PL_ADVANCED)) != NULL)
																{
																	success_flag = true;
																}
															else
																{
																	PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", STUDY_SEARCH_ACTIVE_DATE.npt_name_s);
																}

														}
													else
														{
															PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "SetUpLocationsListParameter failed");
														}

												}
											else
												{
													PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", STUDY_LOCATIONS_LIST.npt_name_s);
												}

										}		/* if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_SEARCH_TRIAL_ID_S.npt_type, S_SEARCH_TRIAL_ID_S.npt_name_s, "Parent Field Trial", "Get all Studies for a given Field Trial", def, PL_ADVANCED)) != NULL) */
									else
										{
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", S_SEARCH_TRIAL_ID_S.npt_name_s);
										}

								}
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", STUDY_GET_ALL_PLOTS.npt_name_s);
								}
						}
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", STUDY_ID.npt_name_s);
						}

				}
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", STUDY_SEARCH_STUDIES.npt_name_s);
				}
		}
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "CreateAndAddParameterGroupToParameterSet failed for %s", group_name_s);
		}

	return success_flag;
}


bool RunForSearchStudyParams (FieldTrialServiceData *data_p, ParameterSet *param_set_p, ServiceJob *job_p)
{
	bool job_done_flag = false;
	const bool *search_flag_p = NULL;
	ViewFormat format = VF_CLIENT_MINIMAL;

	if (GetCurrentBooleanParameterValueFromParameterSet (param_set_p, STUDY_SEARCH_STUDIES.npt_name_s, &search_flag_p))
		{
			if ((search_flag_p != NULL) && (*search_flag_p == true))
				{
					const char *id_s = NULL;

					if (GetCurrentBooleanParameterValueFromParameterSet (param_set_p, STUDY_GET_ALL_PLOTS.npt_name_s, &search_flag_p))
						{
							if ((search_flag_p != NULL) && (*search_flag_p == true))
								{
									format = VF_CLIENT_FULL;
								}		/* if (value.st_boolean_value) */

						}		/* if (GetParameterValueFromParameterSet (param_set_p, S_GET_ALL_PLOTS.npt_name_s, &value, true)) */

					/*
					 * Are we searching for all studies within a trial?
					 */
					if (GetCurrentStringParameterValueFromParameterSet (param_set_p, S_SEARCH_TRIAL_ID_S.npt_name_s, &id_s))
						{
							if (!IsStringEmpty (id_s))
								{

									/*
									 * We're building up a query for the given parameters
									 */
									bson_t *query_p = bson_new ();

									if (query_p)
										{
											bool built_query_success_flag = true;

											bson_oid_t *id_p = GetBSONOidFromString (id_s);

											if (id_p)
												{
													if (!BSON_APPEND_OID (query_p, ST_PARENT_FIELD_TRIAL_S, id_p))
														{
															FreeBSONOid (id_p);
															built_query_success_flag = false;
														}
												}
											else
												{
												}

											if (!built_query_success_flag)
												{
													const char *prefix_s = "Failed to build Field Trial query";
													char *error_s = ConcatenateVarargsStrings (prefix_s, " for ", id_s, NULL);

													if (error_s)
														{
															AddParameterErrorMessageToServiceJob  (job_p, S_SEARCH_TRIAL_ID_S.npt_name_s, S_SEARCH_TRIAL_ID_S.npt_type, error_s);
															FreeCopiedString (error_s);
														}
													else
														{
															AddParameterErrorMessageToServiceJob  (job_p, S_SEARCH_TRIAL_ID_S.npt_name_s, S_SEARCH_TRIAL_ID_S.npt_type, prefix_s);
														}
												}
										}		/* if (query_p) */

									/*
									 * Search with our given criteria
									 */
									if (GetMatchingStudies (query_p, data_p, job_p, format))
										{

										}

									job_done_flag = true;
								}
						}

					if (!job_done_flag)
						{
							if (GetStudyForGivenId (data_p, param_set_p, job_p, format, NULL))
								{
									job_done_flag = true;
								}		/* if (GetStudyForGivenId (data_p, param_set_p, job_p)) */
							else
								{
									/*
									 * We're building up a query for the given parameters
									 */
									bson_t *query_p = bson_new ();

									if (query_p)
										{
											if (AddStudyLocationCriteria (query_p, param_set_p))
												{
													if (AddStudyDateCriteria (query_p, param_set_p))
														{
															/*
															 * Search with our given criteria
															 */
															if (GetMatchingStudies (query_p, data_p, job_p, format))
																{
																	job_done_flag = true;
																}

														}		/* if (AddStudyLocationCriteria (query_p, param_set_p)) */

												}		/* if (AddStudyLocationCriteria (query_p, param_set_p)) */

											bson_destroy (query_p);
										}		/* if (query_p) */


								}		/* if (GetStudyForGivenId (data_p, param_set_p, job_p)) else ... */

						}
				}
		}


	return job_done_flag;
}


/*
 * STATIC DEFINITIONS
 */


static const char *GetStudyDefaultValueFromJSON (const char *study_id_param_s, const json_t *params_json_p)
{
	const char *study_id_s;

	if (params_json_p)
		{
			const size_t num_entries = json_array_size (params_json_p);
			size_t i;

			for (i = 0; i < num_entries; ++ i)
				{
					const json_t *param_json_p = json_array_get (params_json_p, i);
					const char *name_s = GetJSONString (param_json_p, PARAM_NAME_S);

					if (name_s)
						{
							if (strcmp (name_s, study_id_param_s) == 0)
								{
									study_id_s = GetJSONString (param_json_p, PARAM_CURRENT_VALUE_S);

									if (!study_id_s)
										{
											PrintJSONToErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, param_json_p, "Failed to get \"%s\" from \"%s\"", PARAM_CURRENT_VALUE_S, study_id_param_s);
										}

									/* force exit from loop */
									i = num_entries;
								}
						}		/* if (name_s) */

				}		/* for (i = 0; i < num_entries; ++ i) */

		}		/* if (params_json_p) */

	return study_id_s;
}



static bool SetUpDefaultsFromExistingStudy (const Study * const study_p, char **id_ss, char **this_crop_ss, char **previous_crop_ss, char **trial_ss, char **location_ss)
{
	bool success_flag = false;
	char *study_id_s = GetBSONOidAsString (study_p -> st_id_p);

	if (study_id_s)
		{
			if (study_p -> st_location_p)
				{
					char *location_id_s = GetBSONOidAsString (study_p -> st_location_p -> lo_id_p);

					if (location_id_s)
						{
							if (study_p -> st_parent_p)
								{
									char *field_trial_id_s = GetBSONOidAsString (study_p -> st_parent_p -> ft_id_p);

									if (field_trial_id_s)
										{
											bool got_crops_flag = true;
											char *current_crop_id_s = NULL;
											char *previous_crop_id_s = NULL;

											if (study_p -> st_current_crop_p)
												{
													current_crop_id_s = GetBSONOidAsString (study_p -> st_current_crop_p -> cr_id_p);

													if (!current_crop_id_s)
														{
															got_crops_flag = false;
															PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get current crop id as string for study \"%s\"", study_p -> st_name_s);
														}
												}

											if (got_crops_flag)
												{
													if (study_p -> st_previous_crop_p)
														{
															previous_crop_id_s = GetBSONOidAsString (study_p -> st_previous_crop_p -> cr_id_p);

															if (!previous_crop_id_s)
																{
																	got_crops_flag = false;
																	PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get previous crop id as string for study \"%s\"", study_p -> st_name_s);
																}
														}
												}


											if (got_crops_flag)
												{
													*id_ss = study_id_s;
	/*
													*name_ss = study_p -> st_name_s;
													*soil_ss = study_p -> st_soil_type_s;
													*link_ss = study_p -> st_data_url_s;
													*slope_ss = study_p -> st_slope_s;
													*aspect_ss = study_p -> st_aspect_s;
*/
													*this_crop_ss = current_crop_id_s;
													*previous_crop_ss = previous_crop_id_s;
													*trial_ss = field_trial_id_s;
													*location_ss = location_id_s;
	/*
													*description_ss = study_p -> st_description_s;
													*design_ss = study_p -> st_design_s;
													*growing_conditons_ss = study_p -> st_growing_conditions_s;
													*phenotype_gathering_notes_ss = study_p -> st_phenotype_gathering_notes_s;
													*sowing_time_pp = study_p -> st_sowing_date_p;
													*harvest_time_pp = study_p -> st_harvest_date_p;
													*ph_min_pp = study_p -> st_min_ph_p;
													*ph_max_pp = study_p -> st_max_ph_p;
													*weather_ss = study_p -> st_weather_link_s;
													*shape_pp = study_p -> st_shape_p;
													*plot_horizontal_gap_pp = study_p -> st_plot_horizontal_gap_p;
													*plot_vertical_gap_pp = study_p -> st_plot_vertical_gap_p;
													*plots_rows_per_block_pp = study_p -> st_plots_rows_per_block_p;
													*plots_cols_per_block_pp = study_p -> st_plots_columns_per_block_p;
													*plot_block_horizontal_gap_pp = study_p -> st_plot_block_horizontal_gap_p;
													*plot_block_vertical_gap_pp = study_p -> st_plot_block_vertical_gap_p;
*/
													return true;

												}		/* if (got_crops_flag) */

											FreeCopiedString (field_trial_id_s);
										}		/* if (field_trial_id_s) */
									else
										{
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get field trial id as string for study \"%s\"", study_p -> st_name_s);
										}
								}
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "No parent field trial for study \"%s\"", study_p -> st_name_s);
								}

							FreeCopiedString (location_id_s);
						}		/* if (location_id_s) */
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get location id as string for study \"%s\"", study_p -> st_name_s);
						}
				}
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "No location for study \"%s\"", study_p -> st_name_s);
				}

			FreeCopiedString (study_id_s);
		}		/* if (study_id_s) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to copy study id");
		}

	return success_flag;
}



static bool AddStudy (ServiceJob *job_p, ParameterSet *param_set_p, FieldTrialServiceData *data_p)
{
	OperationStatus status = OS_FAILED;
	const char *id_s = NULL;
	const char *name_s = NULL;
	bson_oid_t *study_id_p = NULL;
	size_t num_treatment_names = 0;
	size_t num_treatment_levels = 0;
	Parameter *treatment_names_p = GetParameterFromParameterSetByName (param_set_p, TFJ_TREATMENT_NAME.npt_name_s);
	Parameter *treatment_levels_p = GetParameterFromParameterSetByName (param_set_p, TFJ_VALUES.npt_name_s);

	if (treatment_names_p)
		{
			if (IsStringArrayParameter (treatment_names_p))
				{
					StringArrayParameter *tf_names_p = (StringArrayParameter *) treatment_names_p;
					num_treatment_names = GetNumberOfStringArrayCurrentParameterValues (tf_names_p);
				}
			else if (IsStringParameter (treatment_names_p))
				{
					StringParameter *tf_names_p = (StringParameter *) treatment_names_p;

					const char *value_s = GetStringParameterCurrentValue (tf_names_p);

					if (value_s)
						{
							num_treatment_names = 1;
						}
				}

		}

	treatment_levels_p = GetParameterFromParameterSetByName (param_set_p, TFJ_VALUES.npt_name_s);

	if (treatment_levels_p)
		{
			JSONParameter *tf_levels_p = (JSONParameter *) treatment_levels_p;
			const json_t *levels_json_p = GetJSONParameterCurrentValue (tf_levels_p);

			if (levels_json_p)
				{
					if (json_is_array (levels_json_p))
						{
							num_treatment_levels = json_array_size (levels_json_p);
						}
				}
		}


	if (num_treatment_levels == num_treatment_names)
		{
			/*
			 * Get the existing study id if specified
			 */
			GetCurrentStringParameterValueFromParameterSet (param_set_p, STUDY_ID.npt_name_s, &id_s);

			if (id_s)
				{
					if (strcmp (S_EMPTY_LIST_OPTION_S, id_s) != 0)
						{
							study_id_p = GetBSONOidFromString (id_s);

							if (!study_id_p)
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to load study \"%s\" for editing", id_s);
									return false;
								}
						}
				}		/* if (id_value.st_string_value_s) */


			if (GetCurrentStringParameterValueFromParameterSet (param_set_p, STUDY_NAME.npt_name_s, &name_s))
				{
					const char *parent_field_trial_id_s = NULL;

					if (GetCurrentStringParameterValueFromParameterSet (param_set_p, STUDY_FIELD_TRIALS_LIST.npt_name_s, &parent_field_trial_id_s))
						{
							if (parent_field_trial_id_s)
								{
									bool study_freed_flag = false;
									FieldTrial *trial_p = GetUniqueFieldTrialBySearchString (parent_field_trial_id_s, VF_STORAGE, data_p);

									if (trial_p)
										{
											const char *location_s = NULL;

											if (GetCurrentStringParameterValueFromParameterSet (param_set_p, STUDY_LOCATIONS_LIST.npt_name_s, &location_s))
												{
													if (location_s)
														{
															Location *location_p = GetUniqueLocationBySearchString (location_s, VF_STORAGE, data_p);

															if (location_p)
																{
																	const char *notes_s = NULL;

																	if (GetCurrentStringParameterValueFromParameterSet (param_set_p, STUDY_DESCRIPTION.npt_name_s, &notes_s))
																		{
																			Crop *current_crop_p = NULL;
																			const char *crop_s = NULL;

																			GetCurrentStringParameterValueFromParameterSet (param_set_p, STUDY_THIS_CROP.npt_name_s, &crop_s);

																			if ((strcmp (crop_s, S_UNKNOWN_CROP_OPTION_S) == 0) || (GetValidCrop (crop_s, &current_crop_p, data_p)))
																				{
																					Crop *previous_crop_p = NULL;

																					GetCurrentStringParameterValueFromParameterSet (param_set_p, STUDY_PREVIOUS_CROP.npt_name_s, &crop_s);

																					if ((strcmp (crop_s, S_UNKNOWN_CROP_OPTION_S) == 0) || (GetValidCrop (crop_s, &previous_crop_p, data_p)))
																						{
																							Study *study_p = NULL;
																							const char *soil_s = NULL;
																							const char *aspect_s = NULL;
																							const char *slope_s = NULL;
																							const char *data_link_s = NULL;
																							const char *design_s = NULL;
																							const char *growing_conditions_s = NULL;
																							const char *phenotype_notes_s = NULL;
																							const struct tm *sowing_date_p = NULL;
																							const struct tm *harvest_date_p = NULL;
																							const double64 *min_ph_p = NULL;
																							const double64 *max_ph_p = NULL;
																							const uint32 *num_rows_p = NULL;
																							const uint32 *num_cols_p = NULL;
																							const uint32 *num_replicates_p = NULL;
																							const double64 *plot_width_p = NULL;
																							const double64 *plot_length_p = NULL;
																							const char *weather_s = NULL;
																							const json_t *shape_p = NULL;

																							const double64 *plot_horizontal_gap_p = NULL;
																							const double64 *plot_vertical_gap_p = NULL;
																							const uint32 *plots_rows_per_block_p = NULL;
																							const uint32 *plots_columns_per_block_p = NULL;
																							const double64 *plot_block_horizontal_gap_p = NULL;
																							const double64 *plot_block_vertical_gap_p = NULL;


																							GetCurrentStringParameterValueFromParameterSet (param_set_p, STUDY_SOIL.npt_name_s, &soil_s);
																							GetCurrentStringParameterValueFromParameterSet (param_set_p, STUDY_ASPECT.npt_name_s, &aspect_s);
																							GetCurrentStringParameterValueFromParameterSet (param_set_p, STUDY_SLOPE.npt_name_s, &slope_s);
																							GetCurrentStringParameterValueFromParameterSet (param_set_p, STUDY_LINK.npt_name_s, &data_link_s);

																							GetCurrentStringParameterValueFromParameterSet (param_set_p, STUDY_DESIGN.npt_name_s, &design_s);
																							GetCurrentStringParameterValueFromParameterSet (param_set_p, STUDY_GROWING_CONDITIONS.npt_name_s, &growing_conditions_s);
																							GetCurrentStringParameterValueFromParameterSet (param_set_p, STUDY_PHENOTYPE_GATHERING_NOTES.npt_name_s, &phenotype_notes_s);

																							GetCurrentTimeParameterValueFromParameterSet (param_set_p, STUDY_SOWING_YEAR.npt_name_s, &sowing_date_p);
																							GetCurrentTimeParameterValueFromParameterSet (param_set_p, STUDY_HARVEST_YEAR.npt_name_s, &harvest_date_p);


																							GetCurrentDoubleParameterValueFromParameterSet (param_set_p, STUDY_MIN_PH.npt_name_s, &min_ph_p);
																							GetCurrentDoubleParameterValueFromParameterSet (param_set_p, STUDY_MAX_PH.npt_name_s, &max_ph_p);

																							GetCurrentUnsignedIntParameterValueFromParameterSet (param_set_p, STUDY_NUM_PLOT_ROWS.npt_name_s, &num_rows_p);
																							GetCurrentUnsignedIntParameterValueFromParameterSet (param_set_p, STUDY_NUM_PLOT_COLS.npt_name_s, &num_cols_p);
																							GetCurrentUnsignedIntParameterValueFromParameterSet (param_set_p, STUDY_NUM_REPLICATES.npt_name_s, &num_replicates_p);
																							GetCurrentDoubleParameterValueFromParameterSet (param_set_p, STUDY_PLOT_WIDTH.npt_name_s, &plot_width_p);
																							GetCurrentDoubleParameterValueFromParameterSet (param_set_p, STUDY_PLOT_LENGTH.npt_name_s, &plot_length_p);


																							GetCurrentStringParameterValueFromParameterSet (param_set_p, STUDY_WEATHER_LINK.npt_name_s, &weather_s);

																							GetCurrentJSONParameterValueFromParameterSet (param_set_p, STUDY_SHAPE_DATA.npt_name_s, &shape_p);

																							GetCurrentDoubleParameterValueFromParameterSet (param_set_p, STUDY_PLOT_HGAP.npt_name_s, &plot_horizontal_gap_p);
																							GetCurrentDoubleParameterValueFromParameterSet (param_set_p, STUDY_PLOT_VGAP.npt_name_s, &plot_vertical_gap_p);
																							GetCurrentUnsignedIntParameterValueFromParameterSet (param_set_p, STUDY_PLOT_ROWS_PER_BLOCK.npt_name_s, &plots_rows_per_block_p);
																							GetCurrentUnsignedIntParameterValueFromParameterSet (param_set_p, STUDY_PLOT_COLS_PER_BLOCK.npt_name_s, &plots_columns_per_block_p);
																							GetCurrentDoubleParameterValueFromParameterSet (param_set_p, STUDY_PLOT_BLOCK_HGAP.npt_name_s, &plot_block_horizontal_gap_p);
																							GetCurrentDoubleParameterValueFromParameterSet (param_set_p, STUDY_PLOT_BLOCK_VGAP.npt_name_s, &plot_block_vertical_gap_p);

																							study_p = AllocateStudy (study_id_p, name_s, soil_s, data_link_s, aspect_s,
																																			 slope_s, sowing_date_p, harvest_date_p, location_p, trial_p, MF_SHALLOW_COPY, current_crop_p, previous_crop_p,
																																			 min_ph_p, max_ph_p, notes_s, design_s,
																																			 growing_conditions_s, phenotype_notes_s,
																																			 num_rows_p, num_cols_p, num_replicates_p, plot_width_p, plot_length_p,
																																			 weather_s, shape_p,
																																			 plot_horizontal_gap_p, plot_vertical_gap_p, plots_rows_per_block_p, plots_columns_per_block_p,
																																			 plot_block_horizontal_gap_p, plot_block_vertical_gap_p,
																																			 data_p);

																							if (study_p)
																								{

																									if (AddTreatmentFactorsToStudy (study_p, treatment_names_p, treatment_levels_p, num_treatment_levels, data_p))
																										{
																											status = SaveStudy (study_p, job_p, data_p);

																											if (status == OS_FAILED)
																												{
																													PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to save Study named \"%s\"", name_s);
																												}

																										}
																									else
																										{
																											status = OS_FAILED_TO_START;
																										}



																									FreeStudy (study_p);
																									study_freed_flag = true;
																								}


																							if (!study_freed_flag)
																								{
																									if (previous_crop_p)
																										{
																											FreeCrop (previous_crop_p);
																										}
																								}
																						}		/* if (GetValidCrop (previous_crop_value.st_string_value_s, &previous_crop_p)) */


																					if (!study_freed_flag)
																						{
																							if (current_crop_p)
																								{
																									FreeCrop (current_crop_p);
																								}
																						}
																				}		/* if (GetValidCrop (current_crop_value.st_string_value_s, &current_crop_p)) */

																		}		/* if (GetCurrentParameterValueFromParameterSet (param_set_p, STUDY_NOTES.npt_name_s, &notes_value)) */
																	else
																		{

																		}



																	if (!study_freed_flag)
																		{
																			FreeLocation (location_p);
																		}
																}		/* if (location_p) */
															else
																{
																	PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to find Location named \"%s\"", location_s);
																}

														}		/* if (location_id_s) */



												}		/* if (GetCurrentParameterValueFromParameterSet (param_set_p, STUDY_LOCATIONS_LIST.npt_name_s, &name_value)) */
											else
												{
													PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get study parameter %s", STUDY_LOCATIONS_LIST.npt_name_s);
												}

											if (!study_freed_flag)
												{
													FreeFieldTrial (trial_p);
												}
										}
									else
										{
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to find Field Trial named \"%s\"", parent_field_trial_id_s);
										}

								}		/* if (parent_field_trial_id_s) */


						}
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get study parameter %s", STUDY_FIELD_TRIALS_LIST.npt_name_s);
						}

				}
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get study parameter %s", STUDY_NAME.npt_name_s);
				}


		}		/* if (num_treatment_levels == num_treatment_names) */





	SetServiceJobStatus (job_p, status);

	return ((status == OS_SUCCEEDED) || (status == OS_PARTIALLY_SUCCEEDED));
}



static Parameter *AddPhParameter (const ServiceData *service_data_p, ParameterSet *params_p, ParameterGroup *group_p, const NamedParameterType *param_type_p, const char * const display_name_s, const char * const description_s)
{
	Parameter *param_p = EasyCreateAndAddDoubleParameterToParameterSet (service_data_p, params_p, group_p, param_type_p -> npt_type, param_type_p -> npt_name_s, display_name_s, description_s, NULL, PL_ALL);

	if (param_p)
		{
			const double min_ph = 0.0f;
			const double max_ph = 14.0f;

			if (SetDoubleParameterBounds ((DoubleParameter *) param_p, &min_ph, &max_ph))
				{
					return param_p;
				}
		}
	else
		{
		}

	return NULL;
}


bool AddStudyToServiceJob (ServiceJob *job_p, Study *study_p, const ViewFormat format, JSONProcessor *processor_p, FieldTrialServiceData *data_p)
{
	bool success_flag = false;
	json_t *study_json_p = GetStudyAsJSON (study_p, format, processor_p, data_p);

	if (study_json_p)
		{
			char *title_s = GetStudyAsString (study_p);

			if (title_s)
				{
					if (AddContext (study_json_p))
						{
							json_t *dest_record_p = GetResourceAsJSONByParts (PROTOCOL_INLINE_S, NULL, title_s, study_json_p);

							if (dest_record_p)
								{
									AddImage (dest_record_p, DFTD_STUDY, data_p);

									if (AddResultToServiceJob (job_p, dest_record_p))
										{
											success_flag = true;
										}
									else
										{
											json_decref (dest_record_p);
										}

								}		/* if (dest_record_p) */

						}		/* if (AddContext (trial_json_p)) */

					FreeCopiedString (title_s);
				}		/* if (title_s) */

			json_decref (study_json_p);
		}		/* if (study_json_p) */

	return success_flag;
}


json_t *GetAllStudiesAsJSON (const FieldTrialServiceData *data_p)
{
	json_t *results_p = NULL;

	if (SetMongoToolCollection (data_p -> dftsd_mongo_p, data_p -> dftsd_collection_ss [DFTD_STUDY]))
		{
			bson_t *query_p = NULL;
			bson_t *opts_p =  BCON_NEW ( "sort", "{", ST_NAME_S, BCON_INT32 (1), "}");

			results_p = GetAllMongoResultsAsJSON (data_p -> dftsd_mongo_p, query_p, opts_p);

			if (opts_p)
				{
					bson_destroy (opts_p);
				}
		}

	return results_p;
}


json_t *GetStudyDistinctAccessionsAsJSON (bson_oid_t *study_id_p, const FieldTrialServiceData *data_p)
{
	json_t *accessions_p = NULL;
	char *key_s = ConcatenateVarargsStrings (PL_ROWS_S, ".", RO_MATERIAL_ID_S, NULL);

	if (key_s)
		{
			accessions_p = GetDistinctValuesAsJSON (study_id_p, key_s, AddAccession, data_p);
			FreeCopiedString (key_s);
		}

	return accessions_p;
}


json_t *GetStudyDistinctPhenotypesAsJSON (bson_oid_t *study_id_p, const FieldTrialServiceData *data_p)
{
	json_t *phenotypes_p = NULL;
	char *key_s = ConcatenateVarargsStrings (PL_ROWS_S, ".", RO_OBSERVATIONS_S, ".", OB_PHENOTYPE_ID_S, NULL);

	if (key_s)
		{
			phenotypes_p = GetDistinctValuesAsJSON (study_id_p, key_s, AddPhenotype, data_p);

			FreeCopiedString (key_s);
		}		/* if (key_s) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "ConcatenateVarargsStrings () failed for \"%s\", \"%s\", \"%s\"", PL_ROWS_S, RO_OBSERVATIONS_S, OB_PHENOTYPE_ID_S);
		}



	return phenotypes_p;
}


static json_t *GetDistinctValuesAsJSON (bson_oid_t *study_id_p, const char *key_s, bool (*add_value_fn) (const char *oid_s, json_t *values_p, const FieldTrialServiceData *data_p), const FieldTrialServiceData *data_p)
{
	json_t *values_p = NULL;

	if (SetMongoToolCollection (data_p -> dftsd_mongo_p, data_p -> dftsd_collection_ss [DFTD_PLOT]))
		{
			bson_t *command_p = BCON_NEW ("distinct",
														 BCON_UTF8 (data_p -> dftsd_collection_ss [DFTD_PLOT]),
														 "key",
														 BCON_UTF8 (key_s),
														 "query",
														 "{",
														 PL_PARENT_STUDY_S,
														 BCON_OID (study_id_p),
														 "}");

			if (command_p)
				{
					bson_t *reply_p = NULL;

					if (RunMongoCommand (data_p -> dftsd_mongo_p, command_p, &reply_p))
						{
							if (reply_p)
								{
									json_t *results_p = ConvertBSONToJSON (reply_p);

									if (results_p)
										{
											json_t *oid_values_p = json_object_get (results_p, "values");

											if (oid_values_p)
												{
													if (json_is_array (oid_values_p))
														{
															values_p = json_array ();

															if (values_p)
																{
																	json_t *oid_value_p;
																	size_t i;

																	json_array_foreach (oid_values_p, i, oid_value_p)
																		{
																			const char *oid_s = GetJSONString (oid_value_p, "$oid");

																			if (oid_s)
																				{
																					if (!add_value_fn (oid_s, values_p, data_p))
																						{
																							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add data for\"%s\"", oid_s);
																						}
																				}		/* if (oid_s) */

																		}		/* json_array_foreach (oids_p, i, oid_p) */

																}		/* if (phenotypes_p) */


														}		/* if (json_is_array (oids_p)) */

												}		/* if (oids_p) */


											json_decref (results_p);
										}		/* if (results_p) */


									bson_destroy (reply_p);
								}		/* if (reply_p) */
							else
								{
									size_t length;
									char *json_s = bson_as_json (command_p, &length);
									if (json_s)
										{
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "RunMongoCommand had empty reply for \"%s\"", json_s);
											bson_free (json_s);
										}
									else
										{
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "RunMongoCommand had empty reply");
										}

								}
						}		/* if (RunMongoCommand (data_p -> dftsd_mongo_p, command_p, &reply_p)) */
					else
						{
							size_t length;
							char *json_s = bson_as_json (command_p, &length);

							if (json_s)
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "RunMongoCommand failed for \"%s\"", json_s);
									bson_free (json_s);
								}
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "RunMongoCommand failed");
								}
						}

					bson_destroy (command_p);
				}		/* if (command_p) */
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to create command");
				}

		}

	return values_p;
}



static bool AddAccession (const char *oid_s, json_t *values_p, const FieldTrialServiceData *data_p)
{
	bool success_flag = false;
	bson_oid_t *material_id_p = GetBSONOidFromString (oid_s);

	if (material_id_p)
		{
			Material *material_p = GetMaterialById (material_id_p, data_p);

			if (material_p)
				{
					json_t *accession_p = json_string (material_p -> ma_accession_s);

					if (accession_p)
						{
							if (json_array_append_new (values_p, accession_p) == 0)
								{
									success_flag = true;
								}
							else
								{
									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, accession_p, "Failed to add accession to array");
									json_decref (accession_p);
								}
						}

					FreeMaterial (material_p);
				}		/* if (material_p) */

			FreeBSONOid (material_id_p);
		}		/* if (material_id_p) */

	return success_flag;
}



static bool AddPhenotype (const char *oid_s, json_t *values_p, const FieldTrialServiceData *data_p)
{
	bool success_flag = false;
	bson_oid_t *phenotype_id_p = GetBSONOidFromString (oid_s);

	if (phenotype_id_p)
		{
			MeasuredVariable *variable_p = GetMeasuredVariableById (phenotype_id_p, data_p);

			if (variable_p)
				{
					if (AddTermToJSON (variable_p -> mv_trait_term_p, values_p))
						{
							if (AddTermToJSON (variable_p -> mv_measurement_term_p, values_p))
								{
									success_flag = true;
								}
						}

					FreeMeasuredVariable (variable_p);
				}		/* if (variable_p) */

			FreeBSONOid (phenotype_id_p);
		}		/* if (phenotype_id_p) */

	return success_flag;
}		/* if (oid_s) */




static bool AddTermToJSON (const SchemaTerm *term_p, json_t *phenotypes_p)
{
	json_t *value_p = json_object ();

	if (value_p)
		{
			if (SetJSONString (value_p, INDEXING_NAME_S, term_p -> st_name_s))
				{
					if (SetJSONString (value_p, INDEXING_DESCRIPTION_S, term_p -> st_description_s))
						{
							if (json_array_append_new (phenotypes_p, value_p) == 0)
								{
									return true;
								}
						}

				}

			json_decref (value_p);
		}		/* if (value_p) */

	return false;
}


bool SetUpStudiesListParameter (const FieldTrialServiceData *data_p, StringParameter *param_p, const Study *active_study_p, const bool empty_option_flag)
{
	bool success_flag = false;
	json_t *results_p = GetAllStudiesAsJSON (data_p);
	bool value_set_flag = false;

	if (results_p)
		{
			if (json_is_array (results_p))
				{
					const size_t num_results = json_array_size (results_p);

					success_flag = true;

					/*
					 * If there's an empty option, add it
					 */
					if (empty_option_flag)
						{
							success_flag = CreateAndAddStringParameterOption (param_p, S_EMPTY_LIST_OPTION_S, S_EMPTY_LIST_OPTION_S);
						}

					if (success_flag)
						{
							if (num_results > 0)
								{
									size_t i = 0;
									const char *param_value_s = GetStringParameterCurrentValue (param_p);

									while ((i < num_results) && success_flag)
										{
											json_t *entry_p = json_array_get (results_p, i);
											Study *study_p = GetStudyFromJSON (entry_p, VF_CLIENT_MINIMAL, data_p);

											if (study_p)
												{
													char *id_s = GetBSONOidAsString (study_p -> st_id_p);

													if (id_s)
														{
															if (param_value_s && (strcmp (param_value_s, id_s) == 0))
																{
																	value_set_flag = true;
																}

															if (!CreateAndAddStringParameterOption (param_p, id_s, study_p -> st_name_s))
																{
																	success_flag = false;
																	PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add param option \"%s\": \"%s\"", id_s,  study_p -> st_name_s);
																}

															FreeCopiedString (id_s);
														}
													else
														{
															success_flag = false;
															PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, entry_p, "Failed to get Study BSON oid");
														}

													FreeStudy (study_p);
												}		/* if (study_p) */


											if (success_flag)
												{
													++ i;
												}

										}		/* while ((i < num_results) && success_flag) */

									/*
									 * If the parameter's value isn't on the list, reset it
									 */
									if (!value_set_flag)
										{
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "param value \"%s\" not on list of existing studies", param_value_s);
										}

								}		/* if (num_results > 0) */
							else
								{
									/* nothing to add */
									success_flag = true;
								}

						}		/* if (success_flag) */


				}		/* if (json_is_array (results_p)) */

			json_decref (results_p);
		}		/* if (results_p) */

	if (success_flag)
		{
			if (active_study_p)
				{
					char *id_s = GetBSONOidAsString (active_study_p -> st_id_p);

					if (id_s)
						{
							success_flag = SetStringParameterDefaultValue (param_p, id_s);
							success_flag = SetStringParameterCurrentValue (param_p, id_s);
							FreeCopiedString (id_s);
						}
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get id string for active study \"%s\"", active_study_p -> st_name_s);
							success_flag = false;
						}
				}
		}

	return success_flag;
}


json_t *GetAllStudiesAsJSONInViewFormat (const FieldTrialServiceData *data_p, const ViewFormat format)
{
	json_t *raw_results_p = GetAllStudiesAsJSON (data_p);
	json_t *formatted_results_p = NULL;

	if (raw_results_p)
		{
			const size_t num_results = json_array_size (raw_results_p);

			if (num_results > 0)
				{
					if ((format == VF_CLIENT_FULL) || (format == VF_CLIENT_MINIMAL))
						{
							formatted_results_p = json_array ();

							if (formatted_results_p)
								{
									size_t i = 0;
									bool success_flag = true;

									while ((i < num_results) && success_flag)
										{
											json_t *entry_p = json_array_get (raw_results_p, i);
											Study *study_p = GetStudyFromJSON (entry_p, format, data_p);

											if (study_p)
												{
													json_t *formatted_study_json_p = GetStudyAsJSON (study_p, format, NULL, data_p);

													if (formatted_study_json_p)
														{
															if (json_array_append_new (formatted_results_p, formatted_study_json_p) != 0)
																{
																	json_decref (formatted_study_json_p);
																	PrintJSONToErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, formatted_results_p, "Failed to append formatted study for \"%s\"", study_p -> st_name_s);
																}
														}		/* if (formatted_study_json_p) */
													else
														{
															PrintJSONToErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, formatted_results_p, "GetStudyAsJSON failed in format %d for study \"%s\"", format, study_p -> st_name_s);
														}

													FreeStudy (study_p);
												}		/* if (study_p) */
											else
												{
													PrintJSONToErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, entry_p, "GetStudyFromJSON failed in format %d", format);
												}

											++ i;
										}		/* while ((i < num_results) && success_flag) */

								}		/* if (formatted_results_p) */
							else
								{
									PrintJSONToErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, raw_results_p, "Failed to create formatted results array");
								}





						}		/* if ((format == VF_CLIENT_FULL) || (format == VF_CLIENT_MINIMAL)) */
					else if (format == VF_STORAGE)
						{
							formatted_results_p = raw_results_p;
						}		/* else if (format == VF_STORAGE) */

				}		/* if (num_results > 0) */
			else
				{
					/* return the empty array */
					formatted_results_p = raw_results_p;
				}

			if (raw_results_p != formatted_results_p)
				{
					json_decref (raw_results_p);
				}

		}		/* if (raw_results_p) */

	return formatted_results_p;
}


const KeyValuePair *GetAspect (const char *aspect_value_s)
{
	const KeyValuePair *aspect_p = S_DIRECTIONS_P;
	uint32 i;

	for (i = S_NUM_DIRECTIONS; i > 0; -- i, ++ aspect_p)
		{
			if ((Stricmp (aspect_p -> kvp_value_s, aspect_value_s) == 0) || (strcmp (aspect_p -> kvp_key_s, aspect_value_s) == 0))
				{
					return aspect_p;
				}
		}

	return NULL;
}


static bool GetStudyForGivenId (FieldTrialServiceData *data_p, ParameterSet *param_set_p, ServiceJob *job_p, ViewFormat format, JSONProcessor *processor_p)
{
	bool job_done_flag = false;
	const char *id_s = NULL;

	if (GetCurrentStringParameterValueFromParameterSet (param_set_p, STUDY_ID.npt_name_s, &id_s))
		{
			if (id_s)
				{
					FindAndAddStudyToServiceJob (id_s, format, job_p, processor_p, data_p);
					job_done_flag = true;
				}		/* if (id_s*/

		}		/* if (GetParameterValueFromParameterSet (param_set_p, S_ARST_ID.npt_name_s, &value, true)) */

	return job_done_flag;
}


json_t *GetStudyJSONForId (const char *id_s, const ViewFormat format, JSONProcessor *processor_p, char **study_name_ss, const FieldTrialServiceData *data_p)
{
	json_t *study_json_p = NULL;

	if (format == VF_CLIENT_FULL)
		{
			study_json_p = GetCachedStudy (id_s, data_p);

			if (study_json_p)
				{
					const char *name_s = GetJSONString (study_json_p, ST_NAME_S);

					if (name_s)
						{
							*study_name_ss = EasyCopyToNewString (name_s);
						}
				}
		}

	if (!study_json_p)
		{
			bson_oid_t *id_p = GetBSONOidFromString (id_s);

			if (id_p)
				{
					Study *study_p = GetStudyById (id_p, format, data_p);

					if (study_p)
						{
							study_json_p = GetStudyAsJSON (study_p, format, processor_p, data_p);

							if (study_json_p)
								{
									if (AddContext (study_json_p))
										{
											if (format == VF_CLIENT_FULL)
												{
													CacheStudy (id_s, study_json_p, data_p);
												}

											*study_name_ss = EasyCopyToNewString (study_p -> st_name_s);
										}		/* if (AddContext (trial_json_p)) */

								}		/* if (study_json_p) */

							FreeStudy (study_p);
						}		/* if (study_p) */


					FreeBSONOid (id_p);
				}		/* if (id_p) */

		}		/* if (!study_json_p) */

	return study_json_p;
}


void FindAndAddStudyToServiceJob (const char *id_s, const ViewFormat format, ServiceJob *job_p, JSONProcessor *processor_p, const FieldTrialServiceData *data_p)
{
	OperationStatus status = OS_FAILED;
	char *study_name_s = NULL;
	json_t *study_json_p = GetStudyJSONForId (id_s, format, processor_p, &study_name_s, data_p);

	bson_oid_t *study_id_p = GetBSONOidFromString (id_s);

	if (study_id_p)
		{
			GetStudyDistinctPhenotypesAsJSON (study_id_p, data_p);
		}


	if (study_json_p)
		{
			json_t *dest_record_p = GetResourceAsJSONByParts (PROTOCOL_INLINE_S, NULL, study_name_s, study_json_p);

			if (dest_record_p)
				{
					if (AddResultToServiceJob (job_p, dest_record_p))
						{
							status = OS_SUCCEEDED;
						}
					else
						{
							json_decref (dest_record_p);
						}

				}		/* if (dest_record_p) */

			json_decref (study_json_p);
		}		/* if (study_json_p) */

	if (study_name_s)
		{
			FreeCopiedString (study_name_s);
		}

	SetServiceJobStatus (job_p, status);
}


static bool GetMatchingStudies (bson_t *query_p, FieldTrialServiceData *data_p, ServiceJob *job_p, ViewFormat format)
{
	bool job_done_flag = false;

	if (SetMongoToolCollection (data_p -> dftsd_mongo_p, data_p -> dftsd_collection_ss [DFTD_STUDY]))
		{
			bson_t *opts_p =  BCON_NEW ( "sort", "{", ST_NAME_S, BCON_INT32 (1), "}");
			json_t *results_p = GetAllMongoResultsAsJSON (data_p -> dftsd_mongo_p, query_p, opts_p);

			if (results_p)
				{
					if (json_is_array (results_p))
						{
							OperationStatus status = OS_FAILED;
							size_t num_added = 0;
							size_t i = 0;
							const size_t num_results = json_array_size (results_p);

							job_done_flag = true;

							for (i = 0; i < num_results; ++ i)
								{
									Study *study_p = NULL;
									json_t *entry_p = json_array_get (results_p, i);

									if (format == VF_CLIENT_FULL)
										{
											bson_oid_t id;

											if (GetMongoIdFromJSON (entry_p, &id))
												{
													char *id_s = GetBSONOidAsString (&id);

													if (id_s)
														{

															FreeCopiedString (id_s);
														}		/* if (id_s) */

												}		/* if (GetMongoIdFromJSON (entry_p, &id)) */

										}		/* if (format == VF_CLIENT_FULL) */

									study_p = GetStudyFromJSON (entry_p, format, data_p);

									if (study_p)
										{
											json_t *study_json_p = GetStudyAsJSON (study_p, format, NULL, data_p);

											if (study_json_p)
												{
													bool added_flag = false;

													if (AddContext (study_json_p))
														{
															json_t *dest_record_p = GetResourceAsJSONByParts (PROTOCOL_INLINE_S, NULL, study_p -> st_name_s, study_json_p);

															if (dest_record_p)
																{
																	if (AddResultToServiceJob (job_p, dest_record_p))
																		{
																			++ num_added;
																			added_flag = true;
																		}
																	else
																		{
																			json_decref (dest_record_p);
																		}

																}		/* if (dest_record_p) */

														}		/* if (AddContext (trial_json_p)) */

													if (!added_flag)
														{
															json_decref (study_json_p);
														}

												}		/* if (study_json_p) */

										}		/* if (study_p) */

								}		/* if (num_results > 0) */

							if (num_added == num_results)
								{
									status = OS_SUCCEEDED;
								}
							else if (status > 0)
								{
									status = OS_PARTIALLY_SUCCEEDED;
								}

							SetServiceJobStatus (job_p, status);
						}		/* if (json_is_array (results_p)) */

				}		/* if (results_p) */

			if (opts_p)
				{
					bson_destroy (opts_p);
				}

		}		/* if (SetMongoToolCollection (data_p -> dftsd_mongo_p, data_p -> dftsd_collection_ss [DFTD_EXPERIMENTAL_AREA]) */

	return job_done_flag;
}


static Parameter *GetAndAddAspectParameter (const char *aspect_s, FieldTrialServiceData *data_p, ParameterSet *param_set_p, ParameterGroup *group_p)
{
	const char *def_s = (S_DIRECTIONS_P + S_UNKNOWN_DIRECTION_INDEX) -> kvp_value_s;
	Parameter *param_p = NULL;

	/*
	 * Is the given aspect on our list?
	 */
	if (aspect_s)
		{
			uint32 i = S_NUM_DIRECTIONS;
			const KeyValuePair *direction_p = S_DIRECTIONS_P;
			bool success_flag = false;

			while (i > 0)
				{
					if (strcmp (direction_p -> kvp_value_s, aspect_s) == 0)
						{
							def_s = aspect_s;
							/* force exit from loop */
							i = 0;
						}
					else
						{
							-- i;
							++ direction_p;
						}
				}
		}

	if (def_s != aspect_s)
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Unknown aspect \"%s\"", aspect_s);
		}

	param_p = EasyCreateAndAddStringParameterToParameterSet (& (data_p -> dftsd_base_data), param_set_p, group_p, STUDY_ASPECT.npt_type, STUDY_ASPECT.npt_name_s, "Aspect", "The direction that the study area was oriented to", def_s, PL_ALL);

	if (param_p)
		{
			uint32 i = S_NUM_DIRECTIONS;
			const KeyValuePair *direction_p = S_DIRECTIONS_P;
			bool success_flag = true;

			/*
			 * Set up the direction options
			 */
			while (success_flag & (i > 0))
				{
					if (CreateAndAddStringParameterOption ((StringParameter *) param_p, direction_p -> kvp_value_s, direction_p -> kvp_key_s))
						{
							-- i;
							++ direction_p;
						}
					else
						{
							success_flag = false;
						}
				}

			if (success_flag)
				{
					return param_p;
				}

			FreeParameter (param_p);
		}		/* if (param_p) */

	return NULL;
}



static bool AddStudyLocationCriteria (bson_t *query_p, ParameterSet *param_set_p)
{
	bool success_flag = true;
	const char *location_id_s = NULL;

	/*
	 * Are we looking for a specific location?
	 */
	if (GetCurrentStringParameterValueFromParameterSet (param_set_p, STUDY_LOCATIONS_LIST.npt_name_s, &location_id_s))
		{
			if (location_id_s)
				{
					const char *unset_value_s = GetUnsetLocationValue ();

					if (strcmp (location_id_s, unset_value_s) != 0)
						{
							bson_oid_t *id_p = GetBSONOidFromString (location_id_s);

							success_flag  = false;

							if (id_p)
								{
									if (BSON_APPEND_OID (query_p, ST_LOCATION_ID_S, id_p))
										{
											success_flag = true;
										}

									FreeBSONOid (id_p);
								}		/* if (id_p) */

						}		/* if (strcmp (value.st_string_value_s, unset_value_s) != 0) */

				}		/* if (value.st_string_value_s) */

		}		/* if (GetParameterValueFromParameterSet (param_set_p, S_LOCATIONS_LIST.npt_name_s, &value, true)) */

	return success_flag;
}




static bool AddStudyDateCriteria (bson_t *query_p, ParameterSet *param_set_p)
{
	bool success_flag = false;
	const struct tm *active_time_p = NULL;


	/*
	 * Are we looking for a specific location?
	 */
	if (GetCurrentTimeParameterValueFromParameterSet (param_set_p, STUDY_SEARCH_ACTIVE_DATE.npt_name_s, &active_time_p))
		{
			if (active_time_p)
				{
					time_t t = mktime (active_time_p);

					if (t != -1)
						{
							bson_t *sowing_date_doc_p = bson_new ();

							if (sowing_date_doc_p)
								{
									if (BSON_APPEND_INT32 (sowing_date_doc_p, "$lte", t))
										{
											if (BSON_APPEND_DOCUMENT (query_p, ST_SOWING_DATE_S, sowing_date_doc_p))
												{
													bson_t *harvest_date_doc_p = bson_new ();

													if (harvest_date_doc_p)
														{
															if (BSON_APPEND_INT32 (harvest_date_doc_p, "$gte", t))
																{
																	if (BSON_APPEND_DOCUMENT (query_p, ST_HARVEST_DATE_S, harvest_date_doc_p))
																		{
																			success_flag = true;
																		}
																}

															bson_destroy (harvest_date_doc_p);
														}
												}
										}

									bson_destroy (sowing_date_doc_p);
								}

						}		/* if (t != -1) */

				}		/* if (value.st_time_p) */

		}		/* if (GetParameterValueFromParameterSet (param_set_p, S_LOCATIONS_LIST.npt_name_s, &value, true)) */
	else
		{
			success_flag = true;
		}

	return success_flag;
}


char *GetStudyAsString (const Study *study_p)
{
	char *study_s = NULL;

	if (study_p -> st_parent_p)
		{
			char *trial_s = GetFieldTrialAsString (study_p -> st_parent_p);

			if (trial_s)
				{
					study_s = ConcatenateVarargsStrings (trial_s, ": ", study_p -> st_name_s, NULL);

					FreeCopiedString (trial_s);
				}
		}
	else
		{
			study_s = EasyCopyToNewString (study_p -> st_name_s);
		}

	return study_s;
}


static bool GetValidCrop (const char *crop_s, Crop **crop_pp, const FieldTrialServiceData *data_p)
{
	bool success_flag = true;

	if (crop_s)
		{
			*crop_pp = GetCropByIdString (crop_s, data_p);

			if (! (*crop_pp))
				{
					success_flag = false;
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to create crop from \"%s\"", crop_s);
				}
		}

	return success_flag;
}



static Study *GetStudyFromJSONResource (const json_t *resource_data_p, ServiceData *data_p)
{
	const json_t *param_set_json_p = json_object_get (resource_data_p, PARAM_SET_KEY_S);

	if (param_set_json_p)
		{
			const json_t *params_json_p = json_object_get (resource_data_p, PARAM_SET_PARAMS_S);

			if (params_json_p)
				{
					if (json_is_array (params_json_p))
						{
							const size_t num_params = json_array_size (params_json_p);
							size_t i = 0;

							for (i = 0; i < num_params; ++ i)
								{
									const json_t *param_json_p = json_array_get (params_json_p, i);
									const char *param_name_s = GetJSONString (param_json_p, PARAM_NAME_S);

									if (param_name_s)
										{
											/*
											 * Is this the Study ID parameter?
											 */
											if (strcmp (param_name_s, STUDY_ID.npt_name_s) == 0)
												{
													const char *id_s = GetJSONString (param_json_p, PARAM_CURRENT_VALUE_S);

													if (id_s)
														{
															Study *study_p = GetStudyByIdString (id_s, VF_STORAGE, (FieldTrialServiceData *) data_p);

															if (study_p)
																{
																	return study_p;
																}		/* if (study_p) */
															else
																{

																}

														}		/* if (id_s) */

												}

										}		/* if (param_name_s) */

								}		/* for (i = 0; i < num_params; ++ i) */

						}		/* if (json_is_array (params_json_p)) */

				}		/* if (params_json_p) */

		}		/* if (param_set_json_p) */

	return NULL;
}


Study *GetStudyFromResource (Resource *resource_p, const NamedParameterType study_param_type, FieldTrialServiceData *dfw_data_p)
{
	Study *study_p = NULL;

	/*
	 * Have we been set some parameter values to refresh from?
	 */
	if (resource_p && (resource_p -> re_data_p))
		{
			const json_t *param_set_json_p = json_object_get (resource_p -> re_data_p, PARAM_SET_KEY_S);

			if (param_set_json_p)
				{
					json_t *params_json_p = json_object_get (param_set_json_p, PARAM_SET_PARAMS_S);

					if (params_json_p)
						{
							const char *study_id_s = GetStudyDefaultValueFromJSON (study_param_type.npt_name_s, params_json_p);

							/*
							 * Do we have an existing study id?
							 */
							if (study_id_s)
								{
									study_p = GetStudyByIdString (study_id_s, VF_CLIENT_FULL , dfw_data_p);

									if (!study_p)
										{
											PrintJSONToErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, params_json_p, "Failed to load Study with id \"%s\"", study_id_s);
										}

								}		/* if (study_id_s) */

						}
					else
						{
							PrintJSONToErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, param_set_json_p, "Failed to get params with key \"%s\"", PARAM_SET_PARAMS_S);
						}
				}
			else
				{
					PrintJSONToErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, resource_p -> re_data_p, "Failed to get param set with key \"%s\"", PARAM_SET_KEY_S);
				}

		}		/* if (resource_p && (resource_p -> re_data_p)) */

	return study_p;
}


OperationStatus RemovePlotsForStudyById (const char *id_s, FieldTrialServiceData *data_p)
{
	OperationStatus status = OS_FAILED;
	MongoTool *tool_p = data_p -> dftsd_mongo_p;

	if (SetMongoToolCollection (tool_p, data_p -> dftsd_collection_ss [DFTD_PLOT]))
		{
			bson_oid_t *id_p = GetBSONOidFromString (id_s);

			if (id_p)
				{
					bson_t *query_p = bson_new ();

					if (query_p)
						{
							if (BSON_APPEND_OID (query_p, PL_PARENT_STUDY_S, id_p))
								{
									if (RemoveMongoDocumentsByBSON (tool_p, query_p, false))
										{
											status = OS_SUCCEEDED;
										}

								}

							bson_free (query_p);
						}

					FreeBSONOid (id_p);
				}		/* if (id_p) */


		}		/* if (SetMongoToolCollection (tool_p, data_p -> dftsd_collection_ss [DFTD_PLOT])) */

	return status;
}


TreatmentFactor *GetOrCreateTreatmentFactorForStudy (Study *study_p, const bson_oid_t *treatment_id_p, const FieldTrialServiceData *data_p)
{
	TreatmentFactor *tf_p = NULL;
	TreatmentFactorNode *node_p = (TreatmentFactorNode *) (study_p -> st_treatments_p -> ll_head_p);

	while (node_p)
		{
			TreatmentFactor *temp_p = node_p -> tfn_p;

			if (bson_oid_equal (treatment_id_p, temp_p -> tf_treatment_p -> tr_id_p))
				{
					tf_p = temp_p;

					/* force exit from loop */
					node_p = NULL;
				}
			else
				{
					node_p = (TreatmentFactorNode *) (node_p -> tfn_node.ln_next_p);
				}

		}		/* while (node_p) */


	if (!tf_p)
		{
			Treatment *treatment_p = GetTreatmentById (treatment_id_p, VF_STORAGE, data_p);

			if (treatment_p)
				{
					tf_p = AllocateTreatmentFactor (treatment_p, study_p);

					if (tf_p)
						{
							node_p = AllocateTreatmentFactorNode (tf_p);

							if (node_p)
								{
									LinkedListAddTail (study_p -> st_treatments_p, & (node_p -> tfn_node));
								}
							else
								{
									FreeTreatmentFactor (tf_p);
									tf_p = NULL;
								}
						}
					else
						{
							FreeTreatment (treatment_p);
						}		/* if (!tf_p) */

				}		/* if (treatment_p) */

		}		/* if (!tf_p) */

	return tf_p;
}



static bool AddDefaultPlotsParameters (ServiceData *data_p, ParameterSet *params_p, const Study *study_p)
{
	bool success_flag = false;
	ParameterGroup *group_p = CreateAndAddParameterGroupToParameterSet ("Default Plots data", false, data_p, params_p);

	if (EasyCreateAndAddUnsignedIntParameterToParameterSet (data_p, params_p, group_p, STUDY_NUM_PLOT_ROWS.npt_name_s, "Number of plot rows", "The number of plot rows", study_p ? study_p -> st_num_rows_p : NULL, PL_ALL))
		{
			if (EasyCreateAndAddUnsignedIntParameterToParameterSet (data_p, params_p, group_p, STUDY_NUM_PLOT_COLS.npt_name_s, "Number of plot columns", "The number of plot columns", study_p ? study_p -> st_num_columns_p : NULL, PL_ALL))
				{
					if (EasyCreateAndAddUnsignedIntParameterToParameterSet (data_p, params_p, group_p, STUDY_NUM_REPLICATES.npt_name_s, "Number of replicates", "The number of replicates", study_p ? study_p -> st_num_replicates_p : NULL, PL_ALL))
						{
							if (EasyCreateAndAddDoubleParameterToParameterSet (data_p, params_p, group_p, STUDY_PLOT_WIDTH.npt_type, STUDY_PLOT_WIDTH.npt_name_s, "Plot width", "The default width, in metres, of each plot", study_p ? study_p -> st_default_plot_width_p : NULL, PL_ALL))
								{
									if (EasyCreateAndAddDoubleParameterToParameterSet (data_p, params_p, group_p, STUDY_PLOT_LENGTH.npt_type, STUDY_PLOT_LENGTH.npt_name_s, "Plot length", "The default length, in metres, of each plot", study_p ? study_p -> st_default_plot_length_p : NULL, PL_ALL))
										{
											return true;
										}		/* if (EasyCreateAndAddUnsignedIntParameterToParameterSet (service_data_p, params_p, group_p, STUDY_NUM_PLOT_ROWS.npt_name_s, "Plot width", "The default width, in metres, of each plot", NULL, PL_ALL)) */

								}		/* if (EasyCreateAndAddUnsignedIntParameterToParameterSet (service_data_p, params_p, group_p, STUDY_NUM_PLOT_ROWS.npt_name_s, "Plot width", "The default width, in metres, of each plot", NULL, PL_ALL)) */

						}		/* if (EasyCreateAndAddUnsignedIntParameterToParameterSet (service_data_p, params_p, group_p, STUDY_NUM_REPLICATES.npt_name_s, "Number of replicates", "The number of replicates", NULL, PL_ALL)) */

				}		/* if (EasyCreateAndAddUnsignedIntParameterToParameterSet (service_data_p, params_p, group_p, STUDY_NUM_PLOT_ROWS.npt_name_s, "Plot width", "The default width, in metres, of each plot", NULL, PL_ALL)) */

		}		/* if (EasyCreateAndAddUnsignedIntParameterToParameterSet (service_data_p, params_p, group_p, STUDY_NUM_PLOT_ROWS.npt_name_s, "Plot width", "The default width, in metres, of each plot", NULL, PL_ALL)) */


	return success_flag;
}



static bool AddTreatmentFactorParameters (ParameterSet *params_p, const Study *study_p, FieldTrialServiceData *data_p)
{
	ParameterGroup *group_p = CreateAndAddParameterGroupToParameterSet ("Treatment Factors", true, & (data_p -> dftsd_base_data), params_p);

	if (group_p)
		{
			const char *active_tf_name_s = NULL;
			Parameter *param_p = EasyCreateAndAddStringParameterToParameterSet (& (data_p -> dftsd_base_data), params_p, group_p, TFJ_TREATMENT_NAME.npt_type, TFJ_TREATMENT_NAME.npt_name_s, "Treatment name", "The name of the treatment", active_tf_name_s, PL_ALL);

			if (param_p)
				{
					TreatmentFactor *active_tf_p = NULL;

					group_p -> pg_repeatable_param_p = param_p;

					if ((param_p = GetTreatmentFactorTableParameter (params_p, group_p, active_tf_p, data_p)) != NULL)
						{
							return true;
						}
				}
		}

	return false;
}


static bool AddTreatmentFactorsToStudy (Study *study_p, Parameter *treatment_names_p, Parameter *treatment_levels_p, const size_t num_treatments, const FieldTrialServiceData *data_p)
{
	bool success_flag = true;

	const json_t *levels_json_p = GetJSONParameterCurrentValue ((JSONParameter *) treatment_levels_p);

	if (IsStringParameter (treatment_names_p))
		{
			const char *name_s = GetStringParameterCurrentValue ((StringParameter *) treatment_names_p);
			const json_t *level_p = json_array_get (levels_json_p, 0);

			if (!AddTreatmentFactorToStudy (name_s, level_p, study_p, data_p))
				{
					PrintJSONToErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, level_p, "Failed to add treatment factor \"%s\" to study \"%s\"", name_s, study_p -> st_name_s);

					success_flag = false;
				}
		}
	else if (IsStringArrayParameter (treatment_names_p))
		{
			const char **values_ss = GetStringArrayParameterCurrentValues ((StringArrayParameter *) treatment_names_p);

			if (values_ss)
				{
					const char **value_ss = values_ss;
					size_t i = 0;

					while (success_flag && (*value_ss))
						{
							const json_t *level_p = json_array_get (levels_json_p, i);

							if (AddTreatmentFactorToStudy (*value_ss, level_p, study_p, data_p))
								{
									++ i;
									++ value_ss;
								}
							else
								{
									PrintJSONToErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, level_p, "Failed to add treatment factor \"%s\" to study \"%s\"", *value_ss, study_p -> st_name_s);
									success_flag = false;
								}
						}

				}
		}
	else
		{
			PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Treatment Parameter \"%s\" is not a string or string array, its type is %lu, to study \"%s\"", treatment_names_p -> pa_type, treatment_names_p -> pa_name_s, study_p -> st_name_s);
			success_flag = false;
		}

	return success_flag;
}



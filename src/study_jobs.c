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
 * experimental_arst_jobs.c
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
#include "time_util.h"
#include "dfw_util.h"
#include "key_value_pair.h"


/*
 * Study parameters
 */
static NamedParameterType S_GET_ALL_STUDIES = { "Get all Studies", PT_BOOLEAN };

static NamedParameterType S_ACTIVE_DATE = { "Active on date", PT_TIME };

static NamedParameterType S_SEARCH_STUDIES = { "Search Studies", PT_BOOLEAN };




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


/*
 * STATIC DECLARATIONS
 */

static bool AddStudy (ServiceJob *job_p, ParameterSet *param_set_p, DFWFieldTrialServiceData *data_p);


static bool GetStudyForGivenId (DFWFieldTrialServiceData *data_p, ParameterSet *param_set_p, ServiceJob *job_p, ViewFormat format);


static bool AddStudyLocationCriteria (bson_t *query_p, ParameterSet *param_set_p);


static bool AddStudyDateCriteria (bson_t *query_p, ParameterSet *param_set_p);


static bool GetMatchingStudies (bson_t *query_p, DFWFieldTrialServiceData *data_p, ServiceJob *job_p, ViewFormat format);


static Parameter *GetAndAddAspectParameter (DFWFieldTrialServiceData *data_p, ParameterSet *param_set_p, ParameterGroup *group_p);


static Parameter *AddPhParameter (const ServiceData *service_data_p, ParameterSet *params_p, ParameterGroup *group_p, const NamedParameterType *param_type_p, const char * const display_name_s, const char * const description_s);


static bool GetValidCrop (const char *crop_s, Crop **crop_pp, const DFWFieldTrialServiceData *data_p);


/*
 * API DEFINITIONS
 */

bool AddSubmissionStudyParams (ServiceData *data_p, ParameterSet *param_set_p)
{
	bool success_flag = false;
	Parameter *param_p = NULL;
	SharedType def;
	ParameterGroup *group_p = CreateAndAddParameterGroupToParameterSet ("Study", false, data_p, param_set_p);

	def.st_string_value_s = NULL;

	if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, STUDY_NAME.npt_type, STUDY_NAME.npt_name_s, "Name", "The name of the Study", def, PL_ALL)) != NULL)
		{
			if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, STUDY_SOIL.npt_type, STUDY_SOIL.npt_name_s, "Soil", "The soil of the Study", def, PL_ALL)) != NULL)
				{
					if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, STUDY_LINK.npt_type, STUDY_LINK.npt_name_s, "Link", "The url for any downloads relating to this Study", def, PL_ALL)) != NULL)
						{
							struct tm t;

							ClearTime (&t);
							SetDateValuesForTime (&t, 2017, 1, 1);

							def.st_time_p = &t;

							if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, STUDY_SOWING_YEAR.npt_type, STUDY_SOWING_YEAR.npt_name_s, "Sowing date", "The sowing year for the Study", def, PL_ALL)) != NULL)
								{
									ClearTime (&t);

									if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, STUDY_HARVEST_YEAR.npt_type, STUDY_HARVEST_YEAR.npt_name_s, "Harvest date", "The harvest date for the Study", def, PL_ALL)) != NULL)
										{
											def.st_string_value_s = NULL;

											param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, STUDY_FIELD_TRIALS_LIST.npt_type, STUDY_FIELD_TRIALS_LIST.npt_name_s, "Field trials", "The available field trials", def, PL_ALL);

											if (param_p)
												{
													DFWFieldTrialServiceData *dfw_data_p = (DFWFieldTrialServiceData *) data_p;

													if (SetUpFieldTrialsListParameter (dfw_data_p, param_p))
														{
															if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, STUDY_LOCATIONS_LIST.npt_type, STUDY_LOCATIONS_LIST.npt_name_s, "Locations", "The available locations", def, PL_ALL)) != NULL)
																{
																	if (SetUpLocationsListParameter (dfw_data_p, param_p, false))
																		{
																			if ((param_p = GetAndAddAspectParameter (dfw_data_p, param_set_p, group_p)) != NULL)
																				{
																					if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, STUDY_SLOPE.npt_type, STUDY_SLOPE.npt_name_s, "Slope", "The slope of the Study", def, PL_ALL)) != NULL)
																						{
																							if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, STUDY_THIS_CROP.npt_type, STUDY_THIS_CROP.npt_name_s, "Crop", "The crop variety for this study", def, PL_ALL)) != NULL)
																								{
																									if (SetUpCropsListParameter (dfw_data_p, param_p))
																										{
																											if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, STUDY_PREVIOUS_CROP.npt_type, STUDY_PREVIOUS_CROP.npt_name_s, "Previous Crop", "The previous crop variety planted in this field", def, PL_ALL)) != NULL)
																												{
																													if (SetUpCropsListParameter (dfw_data_p, param_p))
																														{
																															if (AddPhParameter (data_p, param_set_p, group_p, &STUDY_MIN_PH, "pH Minimum", "The lower bound of the soil's pH range"))
																																{
																																	if (AddPhParameter (data_p, param_set_p, group_p, &STUDY_MAX_PH, "pH Maximum", "The upper bound of the soil's pH range"))
																																		{
																																			success_flag = true;
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


																						}		/* if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, SLOPE.npt_type, SLOPE.npt_name_s, "Slope", "The slope of the Study", def, PL_ALL)) != NULL) */
																					else
																						{
																							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", STUDY_SLOPE.npt_name_s);
																						}

																				}		/* if ((param_p = GetAndAddAspectParameter (data_p, param_set_p)) != NULL) */
																			else
																				{
																					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "GetAndAddAspectParameter failed");
																				}
																		}
																	else
																		{
																			FreeParameter (param_p);
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
															FreeParameter (param_p);
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
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", STUDY_HARVEST_YEAR.npt_name_s);
										}
								}
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", STUDY_SOWING_YEAR.npt_name_s);
								}

						}		/* if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, STUDY_LINK.npt_type, STUDY_LINK.npt_name_s, "Link", "The url for any downloads relating to this Study", def, PL_ALL)) != NULL) */
					else
						{

						}

				}
			else
				{
					FreeParameter (param_p);
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", STUDY_SOIL.npt_name_s);
				}
		}
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", STUDY_NAME.npt_name_s);
		}


	return success_flag;
}


bool RunForSubmissionStudyParams (DFWFieldTrialServiceData *data_p, ParameterSet *param_set_p, ServiceJob *job_p)
{
	bool success_flag = AddStudy (job_p, param_set_p, data_p);

	return success_flag;
}


bool GetSubmissionStudyParameterTypeForNamedParameter (const char *param_name_s, ParameterType *pt_p)
{
	bool success_flag = true;

	if (strcmp (param_name_s, STUDY_NAME.npt_name_s) == 0)
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
	else if (strcmp (param_name_s, STUDY_SOWING_YEAR.npt_name_s) == 0)
		{
			*pt_p = STUDY_SOWING_YEAR.npt_type;
		}
	else if (strcmp (param_name_s, STUDY_HARVEST_YEAR.npt_name_s) == 0)
		{
			*pt_p = STUDY_HARVEST_YEAR.npt_type;
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
	else
		{
			success_flag = false;
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

	if (strcmp (param_name_s, S_SEARCH_STUDIES.npt_name_s) == 0)
		{
			*pt_p = S_SEARCH_STUDIES.npt_type;
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
	else if (strcmp (param_name_s, S_ACTIVE_DATE.npt_name_s) == 0)
		{
			*pt_p = S_ACTIVE_DATE.npt_type;
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
	SharedType def;
	const char * const group_name_s = "Studies";
	ParameterGroup *group_p = CreateAndAddParameterGroupToParameterSet (group_name_s, false, data_p, param_set_p);

	if (group_p)
		{
			def.st_boolean_value = false;

			if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_SEARCH_STUDIES.npt_type, S_SEARCH_STUDIES.npt_name_s, "Search Studies", "Get the matching Studies", def, PL_ADVANCED)) != NULL)
				{
					def.st_string_value_s = NULL;

					if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, STUDY_ID.npt_type, STUDY_ID.npt_name_s, "id", "The id of the Study", def, PL_ADVANCED)) != NULL)
						{
							if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, STUDY_GET_ALL_PLOTS.npt_type, STUDY_GET_ALL_PLOTS.npt_name_s, "Plots", "Get all of the plots", def, PL_ADVANCED)) != NULL)
								{
									if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, STUDY_LOCATIONS_LIST.npt_type, STUDY_LOCATIONS_LIST.npt_name_s, "Locations", "The available locations", def, PL_ADVANCED)) != NULL)
										{
											if (SetUpLocationsListParameter ((DFWFieldTrialServiceData *) data_p, param_p, true))
												{
													struct tm t;

													ClearTime (&t);
													SetDateValuesForTime (&t, 2017, 1, 1);

													def.st_time_p = &t;

													if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_ACTIVE_DATE.npt_type, S_ACTIVE_DATE.npt_name_s, "Active date", "Date during which the study was active", def, PL_ADVANCED)) != NULL)
														{
															success_flag = true;
														}
													else
														{
															PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", S_ACTIVE_DATE.npt_name_s);
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
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", S_SEARCH_STUDIES.npt_name_s);
				}
		}
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "CreateAndAddParameterGroupToParameterSet failed for %s", group_name_s);
		}

	return success_flag;
}


bool RunForSearchStudyParams (DFWFieldTrialServiceData *data_p, ParameterSet *param_set_p, ServiceJob *job_p)
{
	bool job_done_flag = false;
	SharedType value;
	InitSharedType (&value);
	ViewFormat format = VF_CLIENT_MINIMAL;

	if (GetParameterValueFromParameterSet (param_set_p, S_SEARCH_STUDIES.npt_name_s, &value, true))
		{
			if (value.st_boolean_value)
				{
					if (GetParameterValueFromParameterSet (param_set_p, STUDY_GET_ALL_PLOTS.npt_name_s, &value, true))
						{
							if (value.st_boolean_value)
								{
									format = VF_CLIENT_FULL;
								}		/* if (value.st_boolean_value) */

						}		/* if (GetParameterValueFromParameterSet (param_set_p, S_GET_ALL_PLOTS.npt_name_s, &value, true)) */


					if (GetStudyForGivenId (data_p, param_set_p, job_p, format))
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


	return job_done_flag;
}


/*
 * STATIC DEFINITIONS
 */




static bool AddStudy (ServiceJob *job_p, ParameterSet *param_set_p, DFWFieldTrialServiceData *data_p)
{
	OperationStatus status = OS_FAILED;
	SharedType name_value;


	if (GetCurrentParameterValueFromParameterSet (param_set_p, STUDY_NAME.npt_name_s, &name_value))
		{
			SharedType parent_field_trial_value;

			if (GetCurrentParameterValueFromParameterSet (param_set_p, STUDY_FIELD_TRIALS_LIST.npt_name_s, &parent_field_trial_value))
				{
					FieldTrial *trial_p = GetUniqueFieldTrialBySearchString (parent_field_trial_value.st_string_value_s, data_p);

					if (trial_p)
						{
							SharedType location_value;

							if (GetCurrentParameterValueFromParameterSet (param_set_p, STUDY_LOCATIONS_LIST.npt_name_s, &location_value))
								{
									bool study_freed_flag = false;
									Location *location_p = GetUniqueLocationBySearchString (location_value.st_string_value_s, VF_STORAGE, data_p);

									if (location_p)
										{
											Crop *current_crop_p = NULL;
											SharedType current_crop_value;

											GetCurrentParameterValueFromParameterSet (param_set_p, STUDY_THIS_CROP.npt_name_s, &current_crop_value);

											if (GetValidCrop (current_crop_value.st_string_value_s, &current_crop_p, data_p))
												{
													Crop *previous_crop_p = NULL;
													SharedType previous_crop_value;

													GetCurrentParameterValueFromParameterSet (param_set_p, STUDY_PREVIOUS_CROP.npt_name_s, &previous_crop_value);

													if (GetValidCrop (previous_crop_value.st_string_value_s, &previous_crop_p, data_p))
														{
															SharedType soil_value;
															SharedType aspect_value;
															SharedType slope_value;
															SharedType data_link_value;
															SharedType sowing_year_value;
															SharedType harvest_year_value;
															SharedType min_ph_value;
															SharedType max_ph_value;
															Study *study_p = NULL;
															struct tm *sowing_date_p = NULL;
															struct tm *harvest_date_p = NULL;

															min_ph_value.st_long_value = ST_UNSET_PH;
															max_ph_value.st_long_value = ST_UNSET_PH;

															GetCurrentParameterValueFromParameterSet (param_set_p, STUDY_SOIL.npt_name_s, &soil_value);
															GetCurrentParameterValueFromParameterSet (param_set_p, STUDY_ASPECT.npt_name_s, &aspect_value);
															GetCurrentParameterValueFromParameterSet (param_set_p, STUDY_SLOPE.npt_name_s, &slope_value);
															GetCurrentParameterValueFromParameterSet (param_set_p, STUDY_LINK.npt_name_s, &data_link_value);
															GetCurrentParameterValueFromParameterSet (param_set_p, STUDY_SOWING_YEAR.npt_name_s, &sowing_year_value);
															GetCurrentParameterValueFromParameterSet (param_set_p, STUDY_HARVEST_YEAR.npt_name_s, &harvest_year_value);
															GetParameterValueFromParameterSet (param_set_p, STUDY_MIN_PH.npt_name_s, &min_ph_value, true);
															GetParameterValueFromParameterSet (param_set_p, STUDY_MAX_PH.npt_name_s, &max_ph_value, true);


															if (IsValidDate (sowing_year_value.st_time_p))
																{
																	sowing_date_p = sowing_year_value.st_time_p;
																}

															if (IsValidDate (harvest_year_value.st_time_p))
																{
																	harvest_date_p = harvest_year_value.st_time_p;
																}

															study_p = AllocateStudy (NULL, name_value.st_string_value_s, soil_value.st_string_value_s, data_link_value.st_string_value_s, aspect_value.st_string_value_s, slope_value.st_string_value_s, sowing_date_p, harvest_date_p, location_p, trial_p, current_crop_p, previous_crop_p, min_ph_value.st_long_value, max_ph_value.st_long_value, data_p);

															if (study_p)
																{
																	status = SaveStudy (study_p, job_p, data_p);

																	if (status == OS_FAILED)
																		{
																			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to save Study named \"%s\"", name_value.st_string_value_s);
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


											if (!study_freed_flag)
												{
													FreeLocation (location_p);
												}
										}		/* if (location_p) */
									else
										{
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to find Location named \"%s\"", location_value.st_string_value_s);
										}

								}		/* if (GetCurrentParameterValueFromParameterSet (param_set_p, STUDY_LOCATIONS_LIST.npt_name_s, &name_value)) */
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get study parameter %s", STUDY_LOCATIONS_LIST.npt_name_s);
								}

							FreeFieldTrial (trial_p);
						}
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to find Field Trial named \"%s\"", parent_field_trial_value.st_string_value_s);
						}

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


	SetServiceJobStatus (job_p, status);

	return ((status == OS_SUCCEEDED) || (status == OS_PARTIALLY_SUCCEEDED));
}



static Parameter *AddPhParameter (const ServiceData *service_data_p, ParameterSet *params_p, ParameterGroup *group_p, const NamedParameterType *param_type_p, const char * const display_name_s, const char * const description_s)
{
	ParameterBounds *bounds_p = AllocateParameterBounds ();

	if (bounds_p)
		{
			Parameter *param_p = NULL;
			SharedType def;

			bounds_p -> pb_lower.st_data_value  = 0.0;
			bounds_p -> pb_upper.st_data_value  = 14.0;

			def.st_data_value = 7.0;

			param_p = CreateAndAddParameterToParameterSet (service_data_p, params_p, group_p, param_type_p -> npt_type, false, param_type_p -> npt_name_s, display_name_s, description_s, NULL, def, NULL, bounds_p, PL_ALL, NULL);

			if (param_p)
				{
					return param_p;
				}

			FreeParameterBounds (bounds_p, param_type_p -> npt_type);
		}		/* if (bounds_p) */

	return NULL;
}


bool AddStudyToServiceJob (ServiceJob *job_p, Study *study_p, const ViewFormat format, DFWFieldTrialServiceData *data_p)
{
	bool success_flag = false;
	json_t *study_json_p = GetStudyAsJSON (study_p, format, data_p);

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

		}		/* if (trial_p) */

	return success_flag;
}


json_t *GetAllStudiesAsJSON (const DFWFieldTrialServiceData *data_p)
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


bool SetUpStudiesListParameter (const DFWFieldTrialServiceData *data_p, Parameter *param_p)
{
	bool success_flag = false;
	json_t *results_p = GetAllStudiesAsJSON (data_p);

	if (results_p)
		{
			if (json_is_array (results_p))
				{
					const size_t num_results = json_array_size (results_p);

					if (num_results > 0)
						{
							size_t i = 0;
							json_t *entry_p = json_array_get (results_p, i);
							Study *study_p = GetStudyFromJSON (entry_p, VF_CLIENT_MINIMAL, data_p);

							if (study_p)
								{
									SharedType def;
									char *id_s = GetBSONOidAsString (study_p -> st_id_p);

									if (id_s)
										{
											def.st_string_value_s = id_s;

											if (SetParameterValueFromSharedType (param_p, &def, false))
												{
													if (SetParameterValueFromSharedType (param_p, &def, true))
														{
															success_flag = CreateAndAddParameterOptionToParameter (param_p, def, study_p -> st_name_s);
														}
												}

											FreeCopiedString (id_s);
										}

									FreeStudy (study_p);
								}		/* if (study_p) */

							if (success_flag)
								{
									for (++ i; i < num_results; ++ i)
										{
											entry_p = json_array_get (results_p, i);
											study_p = GetStudyFromJSON (entry_p, false, data_p);

											if (study_p)
												{
													SharedType def;
													char *id_s = GetBSONOidAsString (study_p -> st_id_p);

													if (id_s)
														{
															def.st_string_value_s = id_s;

															if (param_p)
																{
																	success_flag = CreateAndAddParameterOptionToParameter (param_p, def, study_p -> st_name_s);
																}

															FreeCopiedString (id_s);
														}

													FreeStudy (study_p);
												}		/* if (study_p) */

										}		/* for (++ i; i < num_results; ++ i) */

									if (!success_flag)
										{
											FreeParameter (param_p);
											param_p = NULL;
										}

								}		/* if (param_p) */

						}		/* if (num_results > 0) */
					else
						{
							/* nothing to add */
							success_flag = true;
						}

				}		/* if (json_is_array (results_p)) */

			json_decref (results_p);
		}		/* if (results_p) */

	return success_flag;
}


const KeyValuePair *GetAspect (const char *aspect_value_s)
{
	const KeyValuePair *aspect_p = S_DIRECTIONS_P;
	uint32 i;

	for (i = S_NUM_DIRECTIONS; i > 0; -- i, ++ aspect_p)
		{
			if ((Stricmp (aspect_p -> kvp_key_s, aspect_value_s) == 0) || (strcmp (aspect_p -> kvp_key_s, aspect_value_s) == 0))
				{
					return aspect_p;
				}
		}

	return NULL;
}


static bool GetStudyForGivenId (DFWFieldTrialServiceData *data_p, ParameterSet *param_set_p, ServiceJob *job_p, ViewFormat format)
{
	bool job_done_flag = false;
	SharedType value;
	InitSharedType (&value);

	if (GetParameterValueFromParameterSet (param_set_p, STUDY_ID.npt_name_s, &value, true))
		{
			if (value.st_string_value_s)
				{
					bson_oid_t *id_p = GetBSONOidFromString (value.st_string_value_s);

					if (id_p)
						{
							OperationStatus status = OS_FAILED;
							Study *study_p = GetStudyById (id_p, format, data_p);

							if (study_p)
								{
									if (GetStudyPlots (study_p, data_p))
										{
											json_t *arst_json_p = GetStudyAsJSON (study_p, format, data_p);

											if (arst_json_p)
												{
													bool added_flag = false;

													if (AddContext (arst_json_p))
														{
															json_t *dest_record_p = GetResourceAsJSONByParts (PROTOCOL_INLINE_S, NULL, study_p -> st_name_s, arst_json_p);

															if (dest_record_p)
																{
																	if (AddResultToServiceJob (job_p, dest_record_p))
																		{
																			added_flag = true;
																			job_done_flag = true;
																			status = OS_SUCCEEDED;
																		}
																	else
																		{
																			json_decref (dest_record_p);
																		}

																}		/* if (dest_record_p) */

														}		/* if (AddContext (trial_json_p)) */

													if (!added_flag)
														{
															json_decref (arst_json_p);
														}

												}		/* if (arst_json_p) */

										}		/* if (GetStudyPlots (study_p, data_p)) */


									FreeStudy (study_p);
								}		/* if (study_p) */

							SetServiceJobStatus (job_p, status);

							FreeBSONOid (id_p);
						}		/* if (id_p) */

				}		/* if (value.st_string_value_s)*/

		}		/* if (GetParameterValueFromParameterSet (param_set_p, S_ARST_ID.npt_name_s, &value, true)) */

	return job_done_flag;
}


static bool GetMatchingStudies (bson_t *query_p, DFWFieldTrialServiceData *data_p, ServiceJob *job_p, ViewFormat format)
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
									json_t *entry_p = json_array_get (results_p, i);
									Study *study_p = GetStudyFromJSON (entry_p, format, data_p);

									if (study_p)
										{
											json_t *study_json_p = GetStudyAsJSON (study_p, format, data_p);

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


static Parameter *GetAndAddAspectParameter (DFWFieldTrialServiceData *data_p, ParameterSet *param_set_p, ParameterGroup *group_p)
{
	LinkedList *options_p = CreateParameterOptionsList ();

	if (options_p)
		{
			uint32 i = S_NUM_DIRECTIONS;
			const KeyValuePair *direction_p = S_DIRECTIONS_P;
			bool success_flag = true;
			SharedType def;

			InitSharedType (&def);

			/*
			 * Set up the direction options
			 */
			while (success_flag & (i > 0))
				{
					def.st_string_value_s = direction_p -> kvp_value_s;

					if (CreateAndAddParameterOption (options_p, def, direction_p -> kvp_key_s, PT_STRING))
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
					Parameter *param_p = NULL;

					/* default to grassroots */
					def.st_string_value_s = CopyToNewString ((S_DIRECTIONS_P + S_UNKNOWN_DIRECTION_INDEX) -> kvp_value_s, 0, false);

					param_p = CreateAndAddParameterToParameterSet (& (data_p -> dftsd_base_data), param_set_p, group_p, STUDY_ASPECT.npt_type, false, STUDY_ASPECT.npt_name_s, "Aspect", "The direction that the study area was oriented to", options_p, def, NULL, NULL, PL_ALL, NULL);

					if (def.st_string_value_s)
						{
							FreeCopiedString (def.st_string_value_s);
						}


					if (param_p)
						{
							return param_p;
						}
				}

			FreeLinkedList (options_p);
		}		/* if (options_p) */

	return NULL;
}



static bool AddStudyLocationCriteria (bson_t *query_p, ParameterSet *param_set_p)
{
	bool success_flag = true;
	SharedType value;
	InitSharedType (&value);

	/*
	 * Are we looking for a specific location?
	 */
	if (GetParameterValueFromParameterSet (param_set_p, STUDY_LOCATIONS_LIST.npt_name_s, &value, true))
		{
			if (value.st_string_value_s)
				{
					const char *unset_value_s = GetUnsetLocationValue ();

					if (strcmp (value.st_string_value_s, unset_value_s) != 0)
						{
							bson_oid_t *id_p = GetBSONOidFromString (value.st_string_value_s);

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
	SharedType value;
	InitSharedType (&value);

	/*
	 * Are we looking for a specific location?
	 */
	if (GetParameterValueFromParameterSet (param_set_p, S_ACTIVE_DATE.npt_name_s, &value, true))
		{
			if (value.st_time_p)
				{
					time_t t = mktime (value.st_time_p);

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


static bool GetValidCrop (const char *crop_s, Crop **crop_pp, const DFWFieldTrialServiceData *data_p)
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



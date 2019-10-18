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

static Study *GetStudyFromJSONResource (const json_t *resource_data_p, ServiceData *data_p);


static bool GetStudyDefaultValueFromJSON (SharedType *value_p, const json_t *params_json_p, const NamedParameterType param_type, void *default_p);


static bool SetUpDefaultsFromExistingStudy (const Study * const study_p, char **id_ss, const char **name_ss, const char **soil_ss, const char **link_ss, const char **slope_ss, const char **aspect_ss,
																						char **this_crop_ss, char **previous_crop_ss, char **trial_ss, char **location_ss, const char **notes_ss, struct tm **sowing_time_pp,
																						struct tm **harvest_time_pp, double64 **ph_min_pp, double64 **ph_max_pp);

static bool SetUpDefaults (char **id_ss, const char **name_ss, const char **soil_ss, const char **link_ss, const char **slope_ss, const char **aspect_ss, char **this_crop_ss,
													 char **previous_crop_ss, char **trial_ss, char **location_ss, const char **notes_ss, struct tm **sowing_time_pp, struct tm **harvest_time_pp,
													 double64 **ph_min_pp, double64 **ph_max_pp);

static Study *GetStudyFromResource (Resource *resource_p, DFWFieldTrialServiceData *data_p);



/*
 * API DEFINITIONS
 */

bool AddSubmissionStudyParams (ServiceData *data_p, ParameterSet *param_set_p, Resource *resource_p)
{
	bool success_flag = false;
	Parameter *param_p = NULL;
	ParameterGroup *group_p = CreateAndAddParameterGroupToParameterSet ("Study", false, data_p, param_set_p);
	DFWFieldTrialServiceData *dfw_data_p = (DFWFieldTrialServiceData *) data_p;

	char *id_s = NULL;
	const char *name_s = NULL;
	const char *soil_s = NULL;
	const char *link_s = NULL;
	const char *slope_s = NULL;
	const char *aspect_s = NULL;
	char *this_crop_s= NULL;
	char *previous_crop_s = NULL;
	char *trial_s = NULL;
	char *location_s = NULL;
	const char *notes_s = NULL;
	struct tm *sowing_time_p = NULL;
	struct tm *harvest_time_p = NULL;
	Study *active_study_p = GetStudyFromResource (resource_p, dfw_data_p);
	bool defaults_flag = false;
	double ph_min = -1.0;
	double ph_max = -1.0;
	double64 *ph_min_p = &ph_min;
	double64 *ph_max_p = &ph_max;


	if (active_study_p)
		{
			if (SetUpDefaultsFromExistingStudy (active_study_p, &id_s, &name_s, &soil_s, &link_s, &slope_s, &aspect_s, &this_crop_s, &previous_crop_s, &trial_s, &location_s, &notes_s, &sowing_time_p, &harvest_time_p, &ph_min_p, &ph_max_p))
				{
					defaults_flag = true;
				}
		}
	else
		{
			if (SetUpDefaults (&id_s, &name_s, &soil_s, &link_s, &slope_s, &aspect_s, &this_crop_s, &previous_crop_s, &trial_s, &location_s, &notes_s, &sowing_time_p, &harvest_time_p, &ph_min_p, &ph_max_p))
				{
					defaults_flag = true;
				}
		}

	if (defaults_flag)
		{
			SharedType def;

			InitSharedType (&def);

			if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, STUDY_ID.npt_type, STUDY_ID.npt_name_s, "Load Study", "Edit an existing study", def, PL_ADVANCED)) != NULL)
				{
					if (SetUpStudiesListParameter (dfw_data_p, param_p, S_EMPTY_LIST_OPTION_S))
						{
							param_p -> pa_refresh_service_flag = true;

							ClearSharedType (&def, STUDY_ID.npt_type);

							def.st_string_value_s = (char *) name_s;

							if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, STUDY_NAME.npt_type, STUDY_NAME.npt_name_s, "Name", "The name of the Study", def, PL_ALL)) != NULL)
								{
									def.st_string_value_s = (char *) soil_s;

									if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, STUDY_SOIL.npt_type, STUDY_SOIL.npt_name_s, "Soil", "The soil of the Study", def, PL_ALL)) != NULL)
										{
											def.st_string_value_s = (char *) link_s;

											if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, STUDY_LINK.npt_type, STUDY_LINK.npt_name_s, "Link", "The url for any downloads relating to this Study", def, PL_ALL)) != NULL)
												{
													struct tm t;

													if (sowing_time_p)
														{
															def.st_time_p = sowing_time_p;
														}
													else
														{
															ClearTime (&t);
															SetDateValuesForTime (&t, 2017, 1, 1);

															def.st_time_p = &t;
														}

													if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, STUDY_SOWING_YEAR.npt_type, STUDY_SOWING_YEAR.npt_name_s, "Sowing date", "The sowing year for the Study", def, PL_ALL)) != NULL)
														{
															ClearTime (&t);

															if (harvest_time_p)
																{
																	def.st_time_p = harvest_time_p;
																}
															else
																{
																	ClearTime (&t);
																	SetDateValuesForTime (&t, 2018, 1, 1);

																	def.st_time_p = &t;
																}

															if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, STUDY_HARVEST_YEAR.npt_type, STUDY_HARVEST_YEAR.npt_name_s, "Harvest date", "The harvest date for the Study", def, PL_ALL)) != NULL)
																{
																	def.st_string_value_s = trial_s;

																	param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, STUDY_FIELD_TRIALS_LIST.npt_type, STUDY_FIELD_TRIALS_LIST.npt_name_s, "Field trials", "The available field trials", def, PL_ALL);

																	if (param_p)
																		{
																			if (SetUpFieldTrialsListParameter (dfw_data_p, param_p))
																				{
																					def.st_string_value_s = location_s;

																					if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, STUDY_LOCATIONS_LIST.npt_type, STUDY_LOCATIONS_LIST.npt_name_s, "Locations", "The available locations", def, PL_ALL)) != NULL)
																						{
																							if (SetUpLocationsListParameter (dfw_data_p, param_p, false))
																								{
																									def.st_string_value_s = (char *) notes_s;

																									if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, STUDY_NOTES.npt_type, STUDY_NOTES.npt_name_s, "Notes", "Any additional information about the study", def, PL_ALL)) != NULL)
																										{
																											if ((param_p = GetAndAddAspectParameter (dfw_data_p, param_set_p, group_p)) != NULL)
																												{
																													def.st_string_value_s = (char *) slope_s;

																													if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, STUDY_SLOPE.npt_type, STUDY_SLOPE.npt_name_s, "Slope", "The slope of the Study", def, PL_ALL)) != NULL)
																														{
																															def.st_string_value_s = this_crop_s;

																															if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, STUDY_THIS_CROP.npt_type, STUDY_THIS_CROP.npt_name_s, "Crop", "The crop variety for this study", def, PL_ALL)) != NULL)
																																{
																																	if (SetUpCropsListParameter (dfw_data_p, param_p))
																																		{
																																			def.st_string_value_s = previous_crop_s;

																																			if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, STUDY_PREVIOUS_CROP.npt_type, STUDY_PREVIOUS_CROP.npt_name_s, "Previous Crop", "The previous crop variety planted in this field", def, PL_ALL)) != NULL)
																																				{
																																					if (SetUpCropsListParameter (dfw_data_p, param_p))
																																						{
																																							def.st_data_value = ph_min_p ? *ph_min_p : 0.0;

																																							if (AddPhParameter (data_p, param_set_p, group_p, &STUDY_MIN_PH, "pH Minimum", "The lower bound of the soil's pH range or -1 if the value is unknown"))
																																								{
																																									def.st_data_value = ph_max_p ? *ph_max_p : 0.0;

																																									if (AddPhParameter (data_p, param_set_p, group_p, &STUDY_MAX_PH, "pH Maximum", "The upper bound of the soil's pH range or -1 if the value is unknown"))
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

																										}		/* if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, STUDY_NOTES.npt_type, STUDY_NOTES.npt_name_s, "Notes", "Any additional information about the study", def, PL_ALL)) != NULL) */
																									else
																										{
																											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", STUDY_NOTES.npt_name_s);
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
													PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", STUDY_LINK.npt_name_s);
												}
										}
									else
										{
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", STUDY_SOIL.npt_name_s);
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


bool RunForSubmissionStudyParams (DFWFieldTrialServiceData *data_p, ParameterSet *param_set_p, ServiceJob *job_p)
{
	bool success_flag = AddStudy (job_p, param_set_p, data_p);

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
	else if (strcmp (param_name_s, STUDY_NOTES.npt_name_s) == 0)
		{
			*pt_p = STUDY_NOTES.npt_type;
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
	SharedType def;
	const char * const group_name_s = "Studies";
	ParameterGroup *group_p = CreateAndAddParameterGroupToParameterSet (group_name_s, false, data_p, param_set_p);

	if (group_p)
		{
			def.st_boolean_value = false;

			if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_SEARCH_STUDIES.npt_type, S_SEARCH_STUDIES.npt_name_s, "Search Studies", "Get the matching Studies", def, PL_ADVANCED)) != NULL)
				{
					def.st_string_value_s = NULL;

					if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, STUDY_ID.npt_type, STUDY_ID.npt_name_s, "Id", "The id of the Study", def, PL_ADVANCED)) != NULL)
						{
							if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, STUDY_GET_ALL_PLOTS.npt_type, STUDY_GET_ALL_PLOTS.npt_name_s, "Plots", "Get all of the plots", def, PL_ADVANCED)) != NULL)
								{
									if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_SEARCH_TRIAL_ID_S.npt_type, S_SEARCH_TRIAL_ID_S.npt_name_s, "Parent Field Trial", "Get all Studies for a given Field Trial", def, PL_ADVANCED)) != NULL)
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
	ViewFormat format = VF_CLIENT_MINIMAL;

	if (GetCurrentParameterValueFromParameterSet (param_set_p, S_SEARCH_STUDIES.npt_name_s, &value))
		{
			if (value.st_boolean_value)
				{
					if (GetCurrentParameterValueFromParameterSet (param_set_p, STUDY_GET_ALL_PLOTS.npt_name_s, &value))
						{
							if (value.st_boolean_value)
								{
									format = VF_CLIENT_FULL;
								}		/* if (value.st_boolean_value) */

						}		/* if (GetParameterValueFromParameterSet (param_set_p, S_GET_ALL_PLOTS.npt_name_s, &value, true)) */

					/*
					 * Are we searching for all studies within a trial?
					 */
					if (GetCurrentParameterValueFromParameterSet (param_set_p, S_SEARCH_TRIAL_ID_S.npt_name_s, &value))
						{
							const char *id_s = value.st_string_value_s;

							if (!IsStringEmpty (id_s))
								{

									/*
									 * We're building up a query for the given parameters
									 */
									bson_t *query_p = bson_new ();

									if (query_p)
										{
											bson_oid_t *id_p = GetBSONOidFromString (id_s);

											if (id_p)
												{
													if (BSON_APPEND_OID (query_p, ST_PARENT_FIELD_TRIAL_S, id_p))
														{
															/*
															 * Search with our given criteria
															 */
															if (GetMatchingStudies (query_p, data_p, job_p, format))
																{
																}

														}

													FreeBSONOid (id_p);
												}
											else
												{
													AddErrorToServiceJob (job_p, "Parent field trial id error", id_s);
												}

											bson_destroy (query_p);
										}		/* if (query_p) */

									job_done_flag = true;
								}
						}

					if (!job_done_flag)
						{
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
		}


	return job_done_flag;
}


/*
 * STATIC DEFINITIONS
 */


static bool GetStudyDefaultValueFromJSON (SharedType *value_p, const json_t *params_json_p, const NamedParameterType param_type, void *default_p)
{
	bool success_flag = false;

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
							if (strcmp (name_s, param_type.npt_name_s) == 0)
								{
									if (GetValueFromJSON (param_json_p, PARAM_CURRENT_VALUE_S, param_type.npt_type, value_p))
										{
											success_flag = true;
										}
									else
										{
											PrintJSONToErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, param_json_p, "Failed to set \"%s\" from \"%s\"", param_type.npt_name_s, PARAM_CURRENT_VALUE_S);
										}

									/* force exit from loop */
									i = num_entries;
								}
						}		/* if (name_s) */

				}		/* for (i = 0; i < num_entries; ++ i) */

		}		/* if (params_json_p) */
	else
		{
			if (SetSharedTypeValue (value_p, param_type.npt_type, default_p, NULL))
				{
					success_flag = true;
				}
			else
				{
					PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to set default value for \"%s\"", param_type.npt_name_s);
				}
		}

	return success_flag;
}



static bool SetUpDefaultsFromExistingStudy (const Study * const study_p, char **id_ss, const char **name_ss, const char **soil_ss, const char **link_ss, const char **slope_ss, const char **aspect_ss,
																						char **this_crop_ss, char **previous_crop_ss, char **trial_ss, char **location_ss, const char **notes_ss, struct tm **sowing_time_pp,
																						struct tm **harvest_time_pp, double64 **ph_min_pp, double64 **ph_max_pp)
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
													*name_ss = study_p -> st_name_s;
													*soil_ss = study_p -> st_soil_type_s;
													*link_ss = study_p -> st_data_url_s;
													*slope_ss = study_p -> st_slope_s;
													*aspect_ss = study_p -> st_aspect_s;
													*this_crop_ss = current_crop_id_s;
													*previous_crop_ss = previous_crop_id_s;
													*trial_ss = field_trial_id_s;
													*location_ss = location_id_s;
													*notes_ss = study_p -> st_description_s;
													*sowing_time_pp = study_p -> st_sowing_date_p;
													*harvest_time_pp = study_p -> st_harvest_date_p;
													**ph_min_pp = study_p -> st_min_ph;
													**ph_max_pp = study_p -> st_max_ph;

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



static bool SetUpDefaults (char **id_ss, const char **name_ss, const char **soil_ss, const char **link_ss, const char **slope_ss, const char **aspect_ss, char **this_crop_ss,
													 char **previous_crop_ss, char **trial_ss, char **location_ss, const char **notes_ss, struct tm **sowing_time_pp, struct tm **harvest_time_pp,
													 double64 **ph_min_pp, double64 **ph_max_pp)
{
	bool success_flag = true;

	*id_ss = NULL;
	*name_ss = NULL;
	*soil_ss = NULL;
	*link_ss = NULL;
	*slope_ss = NULL;
	*aspect_ss = NULL;
	*this_crop_ss = NULL;
	*previous_crop_ss = NULL;
	*trial_ss = NULL;
	*location_ss = NULL;
	*notes_ss = NULL;
	*sowing_time_pp = NULL;
	*harvest_time_pp = NULL;
	*ph_min_pp = NULL;
	*ph_max_pp = NULL;

	return success_flag;
}



static bool AddStudy (ServiceJob *job_p, ParameterSet *param_set_p, DFWFieldTrialServiceData *data_p)
{
	OperationStatus status = OS_FAILED;
	SharedType name_value;

	if (GetCurrentParameterValueFromParameterSet (param_set_p, STUDY_NAME.npt_name_s, &name_value))
		{
			SharedType parent_field_trial_value;

			if (GetCurrentParameterValueFromParameterSet (param_set_p, STUDY_FIELD_TRIALS_LIST.npt_name_s, &parent_field_trial_value))
				{
					FieldTrial *trial_p = GetUniqueFieldTrialBySearchString (parent_field_trial_value.st_string_value_s, VF_STORAGE, data_p);

					if (trial_p)
						{
							SharedType location_value;

							if (GetCurrentParameterValueFromParameterSet (param_set_p, STUDY_LOCATIONS_LIST.npt_name_s, &location_value))
								{
									bool study_freed_flag = false;
									Location *location_p = GetUniqueLocationBySearchString (location_value.st_string_value_s, VF_STORAGE, data_p);

									if (location_p)
										{
											SharedType notes_value;

											if (GetCurrentParameterValueFromParameterSet (param_set_p, STUDY_NOTES.npt_name_s, &notes_value))
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
																	GetCurrentParameterValueFromParameterSet (param_set_p, STUDY_MIN_PH.npt_name_s, &min_ph_value);
																	GetCurrentParameterValueFromParameterSet (param_set_p, STUDY_MAX_PH.npt_name_s, &max_ph_value);


																	if (IsValidDate (sowing_year_value.st_time_p))
																		{
																			sowing_date_p = sowing_year_value.st_time_p;
																		}

																	if (IsValidDate (harvest_year_value.st_time_p))
																		{
																			harvest_date_p = harvest_year_value.st_time_p;
																		}

																	study_p = AllocateStudy (NULL, name_value.st_string_value_s, soil_value.st_string_value_s, data_link_value.st_string_value_s, aspect_value.st_string_value_s, slope_value.st_string_value_s, sowing_date_p, harvest_date_p, location_p, trial_p, current_crop_p, previous_crop_p, min_ph_value.st_long_value, max_ph_value.st_long_value, notes_value.st_string_value_s, data_p);

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
			else
				{
					FreeParameterBounds (bounds_p, param_type_p -> npt_type);
				}
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


bool SetUpStudiesListParameter (const DFWFieldTrialServiceData *data_p, Parameter *param_p, const char *empty_option_s)
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

					SharedType def;

					InitSharedType (&def);

					/*
					 * If there's an empty option, add it
					 */
					if (empty_option_s)
						{
							def.st_string_value_s = (char *) S_EMPTY_LIST_OPTION_S;

							success_flag = CreateAndAddParameterOptionToParameter (param_p, def, S_EMPTY_LIST_OPTION_S);
						}

					if (success_flag)
						{
							if (num_results > 0)
								{
									size_t i = 0;
									const char *param_value_s = param_p -> pa_current_value.st_string_value_s;

									while ((i < num_results) && success_flag)
										{
											json_t *entry_p = json_array_get (results_p, i);
											Study *study_p = GetStudyFromJSON (entry_p, VF_CLIENT_MINIMAL, data_p);

											if (study_p)
												{
													char *id_s = GetBSONOidAsString (study_p -> st_id_p);

													if (id_s)
														{
															InitSharedType (&def);
															def.st_string_value_s = id_s;

															if (param_value_s && (strcmp (param_value_s, id_s) == 0))
																{
																	value_set_flag = true;
																}

															if (!CreateAndAddParameterOptionToParameter (param_p, def, study_p -> st_name_s))
																{
																	success_flag = false;
																	PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add param option \"%s\": \"%s\"", def.st_string_value_s,  study_p -> st_name_s);
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
															Study *study_p = GetStudyByIdString (id_s, VF_STORAGE, (DFWFieldTrialServiceData *) data_p);

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


static Study *GetStudyFromResource (Resource *resource_p, DFWFieldTrialServiceData *dfw_data_p)
{
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
							SharedType def;

							InitSharedType (&def);

							/*
							 * Do we have an existing study id?
							 */
							if (GetStudyDefaultValueFromJSON (&def, params_json_p, STUDY_ID, NULL))
								{
									Study *study_p = GetStudyByIdString (def.st_string_value_s, VF_CLIENT_FULL , dfw_data_p);

									if (study_p)
										{
											return study_p;
										}		/* if (study_p) */
									else
										{
											PrintJSONToErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, param_set_json_p, "Failed to load Study with id \"%s\"", def.st_string_value_s);
										}

								}		/* if (GetStudyDefaultValue (&def, params_json_p, STUDY_ID.npt_type, NULL)) */


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

	return NULL;
}



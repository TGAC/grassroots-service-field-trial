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
 * experimental_area_jobs.c
 *
 *  Created on: 1 Oct 2018
 *      Author: billy
 */

#include "experimental_area_jobs.h"

#include "experimental_area.h"
#include "location.h"
#include "string_utils.h"
#include "location_jobs.h"
#include "field_trial_jobs.h"
#include "time_util.h"

/*
 * Experimental Area parameters
 */
static NamedParameterType S_EXPERIMENTAL_AREA_NAME = { "EA Name", PT_STRING };
static NamedParameterType S_EXPERIMENTAL_AREA_SOIL = { "EA Soil", PT_STRING };
static NamedParameterType S_EXPERIMENTAL_AREA_SOWING_YEAR = { "EA Sowing Year", PT_TIME };
static NamedParameterType S_EXPERIMENTAL_AREA_HARVEST_YEAR = { "EA Harvest Year", PT_TIME };
static NamedParameterType S_ADD_EXPERIMENTAL_AREA = { "Add Experimental Area", PT_BOOLEAN };
static NamedParameterType S_GET_ALL_EXPERIMENTAL_AREAS = { "Get all Experimental Areas", PT_BOOLEAN };



static NamedParameterType S_FIELD_TRIALS_LIST = { "Field Trials", PT_STRING };
static NamedParameterType S_LOCATIONS_LIST = { "Locations", PT_STRING };




static bool AddExperimentalArea (ServiceJob *job_p, ParameterSet *param_set_p, DFWFieldTrialServiceData *data_p);





bool AddExperimentalAreaParams (ServiceData *data_p, ParameterSet *param_set_p)
{
	bool success_flag = false;
	Parameter *param_p = NULL;
	SharedType def;
	ParameterGroup *group_p = CreateAndAddParameterGroupToParameterSet ("Experimental Area", NULL, false, data_p, param_set_p);

	def.st_string_value_s = NULL;

	if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_EXPERIMENTAL_AREA_NAME.npt_type, S_EXPERIMENTAL_AREA_NAME.npt_name_s, "Name", "The name of the Experimental Area", def, PL_ALL)) != NULL)
		{
			if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_EXPERIMENTAL_AREA_SOIL.npt_type, S_EXPERIMENTAL_AREA_SOIL.npt_name_s, "Soil", "The soil of the Experimental Area", def, PL_ALL)) != NULL)
				{
					struct tm t;

					ClearTime (&t);
					SetDateValuesForTime (&t, 2017, 1, 1);

					def.st_time_p = &t;

					if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_EXPERIMENTAL_AREA_SOWING_YEAR.npt_type, S_EXPERIMENTAL_AREA_SOWING_YEAR.npt_name_s, "Sowing date", "The sowing year for the Experimental Area", def, PL_ALL)) != NULL)
						{
							ClearTime (&t);

							if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_EXPERIMENTAL_AREA_HARVEST_YEAR.npt_type, S_EXPERIMENTAL_AREA_HARVEST_YEAR.npt_name_s, "Harvest date", "The harvest date for the Experimental Area", def, PL_ALL)) != NULL)
								{
									def.st_string_value_s = NULL;

									param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_FIELD_TRIALS_LIST.npt_type, S_FIELD_TRIALS_LIST.npt_name_s, "Field Trials", "The available field trials", def, PL_ALL);

									if (param_p)
										{
											if (SetUpFieldTrialsListParameter ((DFWFieldTrialServiceData *) data_p, param_p))
												{
													if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_LOCATIONS_LIST.npt_type, S_LOCATIONS_LIST.npt_name_s, "Locations", "The available locations", def, PL_ALL)) != NULL)
														{
															if (SetUpLocationsListParameter ((DFWFieldTrialServiceData *) data_p, param_p))
																{
																	if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_ADD_EXPERIMENTAL_AREA.npt_type, S_ADD_EXPERIMENTAL_AREA.npt_name_s, "Add", "Add a new Experimental Area", def, PL_ALL)) != NULL)
																		{
																			if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_GET_ALL_EXPERIMENTAL_AREAS.npt_type, S_GET_ALL_EXPERIMENTAL_AREAS.npt_name_s, "List", "Get all of the existing Experimental Areas", def, PL_ALL)) != NULL)
																				{
																					success_flag = true;
																				}
																			else
																				{
																					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", S_GET_ALL_EXPERIMENTAL_AREAS.npt_name_s);
																				}
																		}
																	else
																		{
																			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", S_ADD_EXPERIMENTAL_AREA.npt_name_s);
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
															PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", S_FIELD_TRIALS_LIST.npt_name_s);
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
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", S_FIELD_TRIALS_LIST.npt_name_s);
										}
								}
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", S_EXPERIMENTAL_AREA_HARVEST_YEAR.npt_name_s);
								}
						}
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", S_EXPERIMENTAL_AREA_SOWING_YEAR.npt_name_s);
						}
				}
			else
				{													FreeParameter (param_p);

					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", S_EXPERIMENTAL_AREA_SOIL.npt_name_s);
				}
		}
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", S_EXPERIMENTAL_AREA_NAME.npt_name_s);
		}


	return success_flag;
}


bool RunForExperimentalAreaParams (DFWFieldTrialServiceData *data_p, ParameterSet *param_set_p, ServiceJob *job_p)
{
	bool job_done_flag = false;
	SharedType value;
	InitSharedType (&value);

	if (GetParameterValueFromParameterSet (param_set_p, S_ADD_EXPERIMENTAL_AREA.npt_name_s, &value, true))
		{
			if (value.st_boolean_value)
				{
					bool success_flag = AddExperimentalArea (job_p, param_set_p, data_p);

					job_done_flag = true;
				}		/* if (value.st_boolean_value) */

		}		/* if (GetParameterValueFromParameterSet (param_set_p, S_ADD_EXPERIMENTAL_AREA.npt_name_s, &value, true)) */


	if (!job_done_flag)
		{
			if (GetParameterValueFromParameterSet (param_set_p, S_GET_ALL_EXPERIMENTAL_AREAS.npt_name_s, &value, true))
				{
					if (value.st_boolean_value)
						{
							//bool success_flag = AddExperimentalArea (job_p, param_set_p, data_p);

							job_done_flag = true;
						}		/* if (value.st_boolean_value) */

				}		/* if (GetParameterValueFromParameterSet (param_set_p, S_ADD_EXPERIMENTAL_AREA.npt_name_s, &value, true)) */

		}

	return job_done_flag;
}


static bool AddExperimentalArea (ServiceJob *job_p, ParameterSet *param_set_p, DFWFieldTrialServiceData *data_p)
{
	bool success_flag = false;
	SharedType value;
	InitSharedType (&value);

	if (GetParameterValueFromParameterSet (param_set_p, S_EXPERIMENTAL_AREA_NAME.npt_name_s, &value, true))
		{
			SharedType soil_value;
			InitSharedType (&soil_value);

			if (GetParameterValueFromParameterSet (param_set_p, S_EXPERIMENTAL_AREA_SOIL.npt_name_s, &soil_value, true))
				{
					SharedType sowing_year_value;
					InitSharedType (&sowing_year_value);

					if (GetParameterValueFromParameterSet (param_set_p, S_EXPERIMENTAL_AREA_SOWING_YEAR.npt_name_s, &sowing_year_value, true))
						{
							SharedType harvest_year_value;
							InitSharedType (&harvest_year_value);

							if (GetParameterValueFromParameterSet (param_set_p, S_EXPERIMENTAL_AREA_SOWING_YEAR.npt_name_s, &harvest_year_value, true))
								{
									SharedType parent_field_trial_value;
									InitSharedType (&parent_field_trial_value);

									if (GetParameterValueFromParameterSet (param_set_p, S_FIELD_TRIALS_LIST.npt_name_s, &parent_field_trial_value, true))
										{
											FieldTrial *trial_p = GetFieldTrialByIdString (parent_field_trial_value.st_string_value_s, data_p);

											if (trial_p)
												{
													SharedType location_value;
													InitSharedType (&location_value);

													if (GetParameterValueFromParameterSet (param_set_p, S_LOCATIONS_LIST.npt_name_s, &location_value, true))
														{
															Location *location_p = GetLocationByIdString (location_value.st_string_value_s, data_p);

															if (location_p)
																{
																	ExperimentalArea *area_p = NULL;
																	struct tm *sowing_date_p = NULL;
																	struct tm *harvest_date_p = NULL;

																	if (IsValidDate (sowing_year_value.st_time_p))
																		{
																			sowing_date_p = sowing_year_value.st_time_p;
																		}

																	if (IsValidDate (harvest_year_value.st_time_p))
																		{
																			harvest_date_p = harvest_year_value.st_time_p;
																		}

																	area_p = AllocateExperimentalArea (NULL, value.st_string_value_s, soil_value.st_string_value_s, sowing_date_p, harvest_date_p, location_p, trial_p, data_p);

																	if (area_p)
																		{
																			success_flag = SaveExperimentalArea (area_p, data_p);

																			FreeExperimentalArea (area_p);
																		}
																	else
																		{
																			FreeLocation (location_p);
																		}
																}		/* if (location_p) */
														}
												}		/* if (GetParameterValueFromParameterSet (param_set_p, S_FIELD_TRIALS_LIST.npt_name_s, &parent_field_trial_value, true)) */
										}

								}
						}
				}

		}

	SetServiceJobStatus (job_p, success_flag ? OS_SUCCEEDED : OS_FAILED);

	return success_flag;
}


bool SetUpExperimentalAreasListParameter (const DFWFieldTrialServiceData *data_p, Parameter *param_p)
{
	bool success_flag = false;

	if (SetMongoToolCollection (data_p -> dftsd_mongo_p, data_p -> dftsd_collection_ss [DFTD_EXPERIMENTAL_AREA]))
		{
			bson_t *query_p = NULL;
			bson_t *opts_p =  BCON_NEW ( "sort", "{", EA_NAME_S, BCON_INT32 (1), "}");
			json_t *results_p = GetAllMongoResultsAsJSON (data_p -> dftsd_mongo_p, query_p, opts_p);

			if (results_p)
				{
					if (json_is_array (results_p))
						{
							const size_t num_results = json_array_size (results_p);

							if (num_results > 0)
								{
									size_t i = 0;
									json_t *entry_p = json_array_get (results_p, i);
									ExperimentalArea *area_p = GetExperimentalAreaFromJSON (entry_p, false, data_p);

									if (area_p)
										{
											SharedType def;
											char *id_s = GetBSONOidAsString (area_p -> ea_id_p);

											if (id_s)
												{
													def.st_string_value_s = id_s;

													if (SetParameterValueFromSharedType (param_p, &def, false))
														{
															if (SetParameterValueFromSharedType (param_p, &def, true))
																{
																	success_flag = CreateAndAddParameterOptionToParameter (param_p, def, area_p -> ea_name_s);
																}
														}

													FreeCopiedString (id_s);
												}

											FreeExperimentalArea (area_p);
										}		/* if (area_p) */

									if (success_flag)
										{
											for (++ i; i < num_results; ++ i)
												{
													entry_p = json_array_get (results_p, i);
													area_p = GetExperimentalAreaFromJSON (entry_p, false, data_p);

													if (area_p)
														{
															SharedType def;
															char *id_s = GetBSONOidAsString (area_p -> ea_id_p);

															if (id_s)
																{
																	def.st_string_value_s = id_s;

																	if (param_p)
																		{
																			success_flag = CreateAndAddParameterOptionToParameter (param_p, def, area_p -> ea_name_s);
																		}

																	FreeCopiedString (id_s);
																}

															FreeExperimentalArea (area_p);
														}		/* if (area_p) */

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

		}		/* if (SetMongoToolCollection (data_p -> dftsd_mongo_p, data_p -> dftsd_collection_ss [DFTD_FIELD_TRIAL])) */

	return success_flag;

}




static bool SearchExperimentalAreas (ServiceJob *job_p, const char * const name_s, const Location * const location_p, const DFWFieldTrialServiceData *data_p)
{
	bool success_flag = false;
	OperationStatus status = OS_FAILED_TO_START;

	if (SetMongoToolCollection (data_p -> dftsd_mongo_p, data_p -> dftsd_collection_ss [DFTD_FIELD_TRIAL]))
		{
			bson_t *query_p = bson_new ();

			/*
			 * Make the query to get the matching field trials
			 */
			if (query_p)
				{
					bool ok_flag = AddQueryTerm (query_p, FT_NAME_S, name_s, false);

					if (ok_flag)
						{
							if (location_p)
								{
									//ok_flag = AddQueryTerm (query_p, FT_TEAM_S, team_s, false);
								}
						}

					if (ok_flag)
						{
							bson_t *opts_p =  BCON_NEW ( "sort", "{", EA_SOWING_DATE_S, BCON_INT32 (1), "}");

							if (opts_p)
								{
									json_t *results_p = GetAllMongoResultsAsJSON (data_p -> dftsd_mongo_p, query_p, opts_p);

									if (results_p)
										{
											if (json_is_array (results_p))
												{
													size_t i;
													const size_t num_results = json_array_size (results_p);

													success_flag = true;

													if (num_results > 0)
														{
															json_t *trial_json_p;

															json_array_foreach (results_p, i, trial_json_p)
																{
																	FieldTrial *trial_p = GetFieldTrialFromJSON (trial_json_p, data_p);

																	if (trial_p)
																		{
																			if (GetAllFieldTrialExperimentalAreas (trial_p, data_p))
																				{
																					if (AddExperimentalAreasToFieldTrialJSON (trial_p, trial_json_p))
																						{
																							char *title_s = GetFieldTrialAsString (trial_p);

																							if (title_s)
																								{
																									json_t *dest_record_p = GetResourceAsJSONByParts (PROTOCOL_INLINE_S, NULL, title_s, trial_json_p);

																									if (dest_record_p)
																										{
																											if (!AddResultToServiceJob (job_p, dest_record_p))
																												{
																													json_decref (dest_record_p);
																												}
																										}

																									FreeCopiedString (title_s);
																								}

																						}
																				}

																			FreeFieldTrial (trial_p);
																		}

																}		/* json_array_foreach (results_p, i, entry_p) */

															i = GetNumberOfServiceJobResults (job_p);


														}		/* if (num_results > 0) */

													if (i == num_results)
														{
															status = OS_SUCCEEDED;
														}
													else if (i == 0)
														{
															status = OS_FAILED;
														}
													else
														{
															status = OS_PARTIALLY_SUCCEEDED;
														}


												}		/* if (json_is_array (results_p)) */

											json_decref (results_p);
										}		/* if (results_p) */

									bson_destroy (opts_p);
								}		/* if (opts_p) */
						}

					bson_destroy (query_p);
				}		/* if (query_p) */

		}

	if (!success_flag)
		{
			status = OS_FAILED;
		}

	SetServiceJobStatus (job_p, status);

	return success_flag;

}


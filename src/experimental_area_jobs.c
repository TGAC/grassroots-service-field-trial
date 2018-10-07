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


/*
 * Experimental Area parameters
 */
static NamedParameterType S_EXPERIMENTAL_AREA_NAME = { "EA Name", PT_STRING };
static NamedParameterType S_EXPERIMENTAL_AREA_LOCATION = { "EA Location", PT_STRING };
static NamedParameterType S_EXPERIMENTAL_AREA_SOIL = { "EA Soil", PT_STRING };
static NamedParameterType S_EXPERIMENTAL_AREA_SOWING_YEAR = { "EA Sowing Year", PT_UNSIGNED_INT };
static NamedParameterType S_EXPERIMENTAL_AREA_HARVEST_YEAR = { "EA Harvest Year", PT_UNSIGNED_INT };
static NamedParameterType S_ADD_EXPERIMENTAL_AREA = { "Add Experimental Area", PT_BOOLEAN };
static NamedParameterType S_GET_ALL_EXPERIMENTAL_AREAS = { "Get all Experimental Areas", PT_BOOLEAN };



static NamedParameterType S_FIELD_TRIALS_LIST = { "Field Trials", PT_STRING };
static NamedParameterType S_LOCATIONS_LIST = { "Locations", PT_STRING };


static bool SetUpFieldTrialsListParameter (const DFWFieldTrialServiceData *data_p, ParameterSet *param_set_p, ParameterGroup *group_p);

static bool AddExperimentalArea (ServiceJob *job_p, ParameterSet *param_set_p, DFWFieldTrialServiceData *data_p);




bool AddExperimentalAreaParams (ServiceData *data_p, ParameterSet *param_set_p)
{
	bool success_flag = false;
	Parameter *param_p = NULL;
	SharedType def;
	ParameterGroup *group_p = CreateAndAddParameterGroupToParameterSet ("Experimental Area", NULL, false, data_p, param_set_p);

	def.st_string_value_s = NULL;

	if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_EXPERIMENTAL_AREA_NAME.npt_type, S_EXPERIMENTAL_AREA_NAME.npt_name_s, "Name", "The name of the Experimental Area", def, PL_BASIC)) != NULL)
		{
			if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_EXPERIMENTAL_AREA_LOCATION.npt_type, S_EXPERIMENTAL_AREA_LOCATION.npt_name_s, "Location", "The location of the  Experimental Area", def, PL_BASIC)) != NULL)
				{
					if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_EXPERIMENTAL_AREA_SOIL.npt_type, S_EXPERIMENTAL_AREA_SOIL.npt_name_s, "Soil", "The soil of the Experimental Area", def, PL_BASIC)) != NULL)
						{
							def.st_ulong_value = 2017;

							if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_EXPERIMENTAL_AREA_SOWING_YEAR.npt_type, S_EXPERIMENTAL_AREA_SOWING_YEAR.npt_name_s, "Sowing Year", "The sowing year of the Experimental Area", def, PL_BASIC)) != NULL)
								{
									if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_EXPERIMENTAL_AREA_HARVEST_YEAR.npt_type, S_EXPERIMENTAL_AREA_HARVEST_YEAR.npt_name_s, "Harvest Year", "The harvest year of the Experimental Area", def, PL_BASIC)) != NULL)
										{
											if (SetUpFieldTrialsListParameter ((DFWFieldTrialServiceData *) data_p, param_set_p, group_p))
												{
													if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_ADD_EXPERIMENTAL_AREA.npt_type, S_ADD_EXPERIMENTAL_AREA.npt_name_s, "Add", "Add a new Experimental Area", def, PL_BASIC)) != NULL)
														{
															if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_GET_ALL_EXPERIMENTAL_AREAS.npt_type, S_GET_ALL_EXPERIMENTAL_AREAS.npt_name_s, "List", "Get all of the existing Experimental Areas", def, PL_BASIC)) != NULL)
																{
																	success_flag = true;
																}
														}
												}
										}
								}
						}
				}
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

		}		/* if (GetParameterValueFromParameterSet (param_set_p, S_ADD_FIELD_TRIAL.npt_name_s, &value, true)) */

	return job_done_flag;
}




static bool AddExperimentalArea (ServiceJob *job_p, ParameterSet *param_set_p, DFWFieldTrialServiceData *data_p)
{
	bool success_flag = false;
	SharedType value;
	InitSharedType (&value);

	if (GetParameterValueFromParameterSet (param_set_p, S_EXPERIMENTAL_AREA_NAME.npt_name_s, &value, true))
		{
			SharedType location_value;
			InitSharedType (&location_value);

			if (GetParameterValueFromParameterSet (param_set_p, S_EXPERIMENTAL_AREA_LOCATION.npt_name_s, &location_value, true))
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
															SharedType use_gps_value;
															Address *address_p = NULL;
															ExperimentalArea *area_p = NULL;

															area_p = AllocateExperimentalArea (NULL, value.st_string_value_s, soil_value.st_string_value_s, sowing_year_value.st_ulong_value, harvest_year_value.st_ulong_value, address_p, trial_p, data_p);

															if (area_p)
																{
																	success_flag = SaveExperimentalArea (area_p, data_p);

																	FreeExperimentalArea (area_p);
																}
														}		/* if (GetParameterValueFromParameterSet (param_set_p, S_FIELD_TRIALS_LIST.npt_name_s, &parent_field_trial_value, true)) */

												}

										}
								}
						}

				}

		}

	SetServiceJobStatus (job_p, success_flag ? OS_SUCCEEDED : OS_FAILED);

	return success_flag;
}



static bool GetExperimentalAreaGeocoordinates (const DFWFieldTrialServiceData *data_p, ParameterSet *param_set_p)
{
	return false;
}


static bool SetUpFieldTrialsListParameter (const DFWFieldTrialServiceData *data_p, ParameterSet *param_set_p, ParameterGroup *group_p)
{
	bool success_flag = false;
	Parameter *param_p = NULL;

	if (SetMongoToolCollection (data_p -> dftsd_mongo_p, data_p -> dftsd_collection_ss [DFTD_FIELD_TRIAL]))
		{
			bson_t *query_p = NULL;
			bson_t *opts_p =  BCON_NEW ( "sort", "{", "team", BCON_INT32 (1), "}");
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
									FieldTrial *trial_p = GetFieldTrialFromJSON (entry_p, data_p);

									if (trial_p)
										{
											char *name_s = GetFieldTrialAsString (trial_p);

											if (name_s)
												{
													SharedType def;
													char *id_s = GetBSONOidAsString (trial_p -> ft_id_p);

													if (id_s)
														{
															def.st_string_value_s = id_s;

															param_p = EasyCreateAndAddParameterToParameterSet (& (data_p -> dftsd_base_data), param_set_p, group_p, S_FIELD_TRIALS_LIST.npt_type, S_FIELD_TRIALS_LIST.npt_name_s, "Field Trials", "The available field trials", def, PL_ALL);

															if (param_p)
																{
																	success_flag = CreateAndAddParameterOptionToParameter (param_p, def, name_s);
																}

															FreeCopiedString (id_s);
														}

													FreeCopiedString (name_s);
												}

											FreeFieldTrial (trial_p);
										}		/* if (trial_p) */

									if (success_flag)
										{
											for (++ i; i < num_results; ++ i)
												{
													entry_p = json_array_get (results_p, i);
													trial_p = GetFieldTrialFromJSON (entry_p, data_p);

													if (trial_p)
														{
															char *name_s = GetFieldTrialAsString (trial_p);

															if (name_s)
																{
																	SharedType def;
																	char *id_s = GetBSONOidAsString (trial_p -> ft_id_p);

																	if (id_s)
																		{
																			def.st_string_value_s = id_s;

																			if (param_p)
																				{
																					success_flag = CreateAndAddParameterOptionToParameter (param_p, def, name_s);
																				}

																			FreeCopiedString (id_s);
																		}

																	FreeCopiedString (name_s);
																}		/* if (name_s) */

															FreeFieldTrial (trial_p);
														}		/* if (trial_p) */

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

				}		/* if (results_p) */

		}		/* if (SetMongoToolCollection (data_p -> dftsd_mongo_p, data_p -> dftsd_collection_ss [DFTD_FIELD_TRIAL])) */

	return success_flag;
}



static bool SetUpLocationsListParameter (const DFWFieldTrialServiceData *data_p, ParameterSet *param_set_p, ParameterGroup *group_p)
{
	bool success_flag = false;
	Parameter *param_p = NULL;

	if (SetMongoToolCollection (data_p -> dftsd_mongo_p, data_p -> dftsd_collection_ss [DFTD_LOCATION]))
		{
			bson_t *query_p = NULL;
			bson_t *opts_p =  BCON_NEW ( "sort", "{", "name", BCON_INT32 (1), "}");
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
									Location *location_p = GetLocationFromJSON (entry_p, data_p);

									if (location_p)
										{
											char *name_s = GetLocationAsString (location_p);

											if (name_s)
												{
													SharedType def;
													char *id_s = GetBSONOidAsString (location_p -> lo_id_p);

													if (id_s)
														{
															def.st_string_value_s = id_s;

															param_p = EasyCreateAndAddParameterToParameterSet (& (data_p -> dftsd_base_data), param_set_p, group_p, S_LOCATIONS_LIST.npt_type, S_LOCATIONS_LIST.npt_name_s, "Locations", "The available locations", def, PL_ALL);

															if (param_p)
																{
																	success_flag = CreateAndAddParameterOptionToParameter (param_p, def, name_s);
																}

															FreeCopiedString (id_s);
														}

													FreeCopiedString (name_s);
												}

											FreeLocation (location_p);
										}		/* if (location_p) */

									if (success_flag)
										{
											for (++ i; i < num_results; ++ i)
												{
													entry_p = json_array_get (results_p, i);
													location_p = GetLocationFromJSON (entry_p, data_p);

													if (location_p)
														{
															char *name_s = GetLocationAsString (location_p);

															if (name_s)
																{
																	SharedType def;
																	char *id_s = GetBSONOidAsString (location_p -> lo_id_p);

																	if (id_s)
																		{
																			def.st_string_value_s = id_s;

																			if (param_p)
																				{
																					success_flag = CreateAndAddParameterOptionToParameter (param_p, def, name_s);
																				}

																			FreeCopiedString (id_s);
																		}

																	FreeCopiedString (name_s);
																}		/* if (name_s) */

															FreeLocation (location_p);
														}		/* if (location_p) */

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

				}		/* if (results_p) */

		}		/* if (SetMongoToolCollection (data_p -> dftsd_mongo_p, data_p -> dftsd_collection_ss [DFTD_LOCATION])) */

	return success_flag;

}


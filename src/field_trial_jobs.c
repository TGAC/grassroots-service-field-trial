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
 * field_trial_jobs.c
 *
 *  Created on: 1 Oct 2018
 *      Author: billy
 */

#include "field_trial_jobs.h"

#include "field_trial.h"
#include "string_utils.h"
#include "dfw_util.h"


/*
 * Field Trial parameters
 */
static NamedParameterType S_FIELD_TRIAL_NAME = { "FT Name", PT_STRING };
static NamedParameterType S_FIELD_TRIAL_TEAM = { "FT Team", PT_STRING };
static NamedParameterType S_ADD_FIELD_TRIAL = { "Add Field Trial", PT_BOOLEAN };
static NamedParameterType S_SEARCH_FIELD_TRIALS = { "Search Field Trials", PT_BOOLEAN };
static NamedParameterType S_FUZZY_SEARCH_FIELD_TRIALS = { "Fuzzy Search", PT_BOOLEAN };
static NamedParameterType S_FULL_DATA = { "Get full data from search", PT_BOOLEAN };





static bool AddFieldTrial (ServiceJob *job_p, const char *name_s, const char *team_s, DFWFieldTrialServiceData *data_p);


static bool SearchFieldTrials (ServiceJob *job_p, const char *name_s, const char *team_s, const bool regex_flag, const ViewFormat format, DFWFieldTrialServiceData *data_p);



bool AddSubmissionFieldTrialParams (ServiceData *data_p, ParameterSet *param_set_p)
{
	bool success_flag = false;
	Parameter *param_p = NULL;
	SharedType def;
	ParameterGroup *group_p = CreateAndAddParameterGroupToParameterSet ("Field Trials", NULL, false, data_p, param_set_p);

	def.st_string_value_s = NULL;

	if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_FIELD_TRIAL_NAME.npt_type, S_FIELD_TRIAL_NAME.npt_name_s, "Name", "The name of the Field Trial", def, PL_SIMPLE)) != NULL)
		{
			if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_FIELD_TRIAL_TEAM.npt_type, S_FIELD_TRIAL_TEAM.npt_name_s, "Team", "The team name of the Field Trial", def, PL_SIMPLE)) != NULL)
				{
					def.st_boolean_value = false;

					if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_ADD_FIELD_TRIAL.npt_type, S_ADD_FIELD_TRIAL.npt_name_s, "Add", "Add a new Field Trial", def, PL_SIMPLE)) != NULL)
						{
							success_flag = true;
						}
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", S_ADD_FIELD_TRIAL.npt_name_s);
						}
				}
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", S_FIELD_TRIAL_TEAM.npt_name_s);
				}
		}
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", S_FIELD_TRIAL_NAME.npt_name_s);
		}

	return success_flag;
}


bool RunForSubmissionFieldTrialParams (DFWFieldTrialServiceData *data_p, ParameterSet *param_set_p, ServiceJob *job_p)
{
	bool job_done_flag = false;
	SharedType value;
	SharedType name_value;
	InitSharedType (&name_value);
	const char *name_s = NULL;
	const char *team_s = NULL;


	if (GetParameterValueFromParameterSet (param_set_p, S_FIELD_TRIAL_NAME.npt_name_s, &name_value, true))
		{
			SharedType team_value;
			InitSharedType (&team_value);

			name_s = name_value.st_string_value_s;

			if (GetParameterValueFromParameterSet (param_set_p, S_FIELD_TRIAL_TEAM.npt_name_s, &team_value, true))
				{
					team_s = team_value.st_string_value_s;
				}

		}		/* if (GetParameterValueFromParameterSet (param_set_p, S_FIELD_TRIAL_NAME.npt_name_s, &value, true)) */



	if (GetParameterValueFromParameterSet (param_set_p, S_ADD_FIELD_TRIAL.npt_name_s, &value, true))
		{
			if (value.st_boolean_value)
				{
					/* It's a job for FieldTrials */
					bool success_flag = AddFieldTrial (job_p, name_s, team_s, data_p);

					job_done_flag = true;
				}		/* if (value.st_boolean_value) */

		}		/* if (GetParameterValueFromParameterSet (param_set_p, S_ADD_FIELD_TRIAL.npt_name_s, &value, true)) */

	return job_done_flag;
}



bool AddSearchFieldTrialParams (ServiceData *data_p, ParameterSet *param_set_p)
{
	bool success_flag = false;
	Parameter *param_p = NULL;
	SharedType def;
	ParameterGroup *group_p = CreateAndAddParameterGroupToParameterSet ("Field Trials", NULL, false, data_p, param_set_p);

	def.st_string_value_s = NULL;

	if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_FIELD_TRIAL_NAME.npt_type, S_FIELD_TRIAL_NAME.npt_name_s, "Name", "The name of the Field Trial", def, PL_ADVANCED)) != NULL)
		{
			if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_FIELD_TRIAL_TEAM.npt_type, S_FIELD_TRIAL_TEAM.npt_name_s, "Team", "The team name of the Field Trial", def, PL_ADVANCED)) != NULL)
				{
					if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_SEARCH_FIELD_TRIALS.npt_type, S_SEARCH_FIELD_TRIALS.npt_name_s, "Search", "Search for matching Field Trials", def, PL_ADVANCED)) != NULL)
						{
							if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_FUZZY_SEARCH_FIELD_TRIALS.npt_type, S_FUZZY_SEARCH_FIELD_TRIALS.npt_name_s, "Fuzzy search", "When doing a search, do a fuzzy search", def, PL_ADVANCED)) != NULL)
								{
									def.st_boolean_value = false;

									if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_FULL_DATA.npt_type, S_FULL_DATA.npt_name_s, "Full data", "When doing a search, get the full data", def, PL_ADVANCED)) != NULL)
										{
											success_flag = true;
										}
									else
										{
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", S_FULL_DATA.npt_name_s);
										}
								}
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", S_FUZZY_SEARCH_FIELD_TRIALS.npt_name_s);
								}
						}
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", S_SEARCH_FIELD_TRIALS.npt_name_s);
						}
				}
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", S_FIELD_TRIAL_TEAM.npt_name_s);
				}
		}
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", S_FIELD_TRIAL_NAME.npt_name_s);
		}

	return success_flag;
}


bool RunForSearchFieldTrialParams (DFWFieldTrialServiceData *data_p, ParameterSet *param_set_p, ServiceJob *job_p)
{
	bool job_done_flag = false;
	SharedType name_value;
	SharedType value;
	const char *name_s = NULL;
	const char *team_s = NULL;

	InitSharedType (&name_value);


	if (GetParameterValueFromParameterSet (param_set_p, S_FIELD_TRIAL_NAME.npt_name_s, &name_value, true))
		{
			SharedType team_value;
			InitSharedType (&team_value);

			name_s = name_value.st_string_value_s;

			if (GetParameterValueFromParameterSet (param_set_p, S_FIELD_TRIAL_TEAM.npt_name_s, &team_value, true))
				{
					team_s = team_value.st_string_value_s;
				}

		}		/* if (GetParameterValueFromParameterSet (param_set_p, S_FIELD_TRIAL_NAME.npt_name_s, &value, true)) */

	InitSharedType (&value);


	if (GetParameterValueFromParameterSet (param_set_p, S_SEARCH_FIELD_TRIALS.npt_name_s, &value, true))
		{
			bool search_flag = value.st_boolean_value;

			if (search_flag)
				{
					bool success_flag;
					bool full_data_flag = false;
					bool fuzzy_search_flag = false;

					GetParameterValueFromParameterSet (param_set_p, S_FUZZY_SEARCH_FIELD_TRIALS.npt_name_s, &value, true);
					fuzzy_search_flag = value.st_boolean_value;

					GetParameterValueFromParameterSet (param_set_p, S_FULL_DATA.npt_name_s, &value, true);
					full_data_flag = value.st_boolean_value;

					success_flag = SearchFieldTrials (job_p, name_s, team_s, fuzzy_search_flag, full_data_flag ? VF_CLIENT_FULL : VF_CLIENT_MINIMAL, data_p);

					job_done_flag = true;
				}		/* if (value.st_boolean_value) */
		}

	return job_done_flag;
}



bool SetUpFieldTrialsListParameter (const DFWFieldTrialServiceData *data_p, Parameter *param_p)
{
	bool success_flag = false;

	if (SetMongoToolCollection (data_p -> dftsd_mongo_p, data_p -> dftsd_collection_ss [DFTD_FIELD_TRIAL]))
		{
			bson_t *query_p = NULL;
			bson_t *opts_p =  BCON_NEW ( "sort", "{", FT_TEAM_S, BCON_INT32 (1), "}");
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

															if (SetParameterValueFromSharedType (param_p, &def, false))
																{
																	if (SetParameterValueFromSharedType (param_p, &def, true))
																		{
																			success_flag = CreateAndAddParameterOptionToParameter (param_p, def, name_s);
																		}
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

					json_decref (results_p);
				}		/* if (results_p) */

			if (opts_p)
				{
					bson_destroy (opts_p);
				}

		}		/* if (SetMongoToolCollection (data_p -> dftsd_mongo_p, data_p -> dftsd_collection_ss [DFTD_FIELD_TRIAL])) */

	return success_flag;
}


static bool AddFieldTrial (ServiceJob *job_p, const char *name_s, const char *team_s, DFWFieldTrialServiceData *data_p)
{
	bool success_flag = false;
	FieldTrial *trial_p = AllocateFieldTrial (name_s, team_s, NULL);

	if (trial_p)
		{
			success_flag = SaveFieldTrial (trial_p, data_p);
			FreeFieldTrial (trial_p);
		}

	SetServiceJobStatus (job_p, success_flag ? OS_SUCCEEDED : OS_FAILED);

	return success_flag;
}


static bool SearchFieldTrials (ServiceJob *job_p, const char *name_s, const char *team_s, const bool regex_flag, const ViewFormat format, DFWFieldTrialServiceData *data_p)
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
					bool ok_flag = AddQueryTerm (query_p, FT_NAME_S, name_s, regex_flag);

					if (ok_flag)
						{
							ok_flag = AddQueryTerm (query_p, FT_TEAM_S, team_s, regex_flag);
						}

					if (ok_flag)
						{
							bson_t *opts_p =  BCON_NEW ( "sort", "{", FT_TEAM_S, BCON_INT32 (1), "}");

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
																			if (GetAllFieldTrialExperimentalAreas (trial_p, format, data_p))
																				{
																					if (AddExperimentalAreasToFieldTrialJSON (trial_p, trial_json_p, format, data_p))
																						{
																							char *title_s = GetFieldTrialAsString (trial_p);

																							if (title_s)
																								{
																									if (AddContext (trial_json_p))
																										{
																											json_t *dest_record_p = GetResourceAsJSONByParts (PROTOCOL_INLINE_S, NULL, title_s, trial_json_p);

																											if (dest_record_p)
																												{
																													if (!AddResultToServiceJob (job_p, dest_record_p))
																														{
																															json_decref (dest_record_p);
																														}

																												}		/* if (dest_record_p) */

																										}		/* if (AddContext (trial_json_p)) */

																									FreeCopiedString (title_s);
																								}		/* if (title_s) */

																						}		/* if (AddExperimentalAreasToFieldTrialJSON (trial_p, trial_json_p, data_p)) */

																				}

																			FreeFieldTrial (trial_p);
																		}		/* if (trial_p) */

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

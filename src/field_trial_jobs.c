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



/*
 * Field Trial parameters
 */
static NamedParameterType S_FIELD_TRIAL_NAME = { "FT Name", PT_STRING };
static NamedParameterType S_FIELD_TRIAL_TEAM = { "FT Team", PT_STRING };
static NamedParameterType S_ADD_FIELD_TRIAL = { "Add Field Trial", PT_BOOLEAN };
static NamedParameterType S_SEARCH_FIELD_TRIALS = { "Search Field Trials", PT_BOOLEAN };
static NamedParameterType S_GET_ALL_FIELD_TRIALS = { "Get all Field Trials", PT_BOOLEAN };




static bool AddFieldTrial (ServiceJob *job_p, const char *name_s, const char *team_s, DFWFieldTrialServiceData *data_p);


static bool SearchFieldTrials (ServiceJob *job_p, const char *name_s, const char *team_s, DFWFieldTrialServiceData *data);




bool AddFieldTrialParams (ServiceData *data_p, ParameterSet *param_set_p)
{
	bool success_flag = false;
	Parameter *param_p = NULL;
	SharedType def;
	ParameterGroup *group_p = CreateAndAddParameterGroupToParameterSet ("Field Trials", NULL, false, data_p, param_set_p);

	def.st_string_value_s = NULL;

	if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_FIELD_TRIAL_NAME.npt_type, S_FIELD_TRIAL_NAME.npt_name_s, "Name", "The name of the Field Trial", def, PL_BASIC)) != NULL)
		{
			if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_FIELD_TRIAL_TEAM.npt_type, S_FIELD_TRIAL_TEAM.npt_name_s, "Team", "The team name of the Field Trial", def, PL_BASIC)) != NULL)
				{
					def.st_boolean_value = false;

					if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_ADD_FIELD_TRIAL.npt_type, S_ADD_FIELD_TRIAL.npt_name_s, "Add", "Add a new Field Trial", def, PL_BASIC)) != NULL)
						{
							if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_SEARCH_FIELD_TRIALS.npt_type, S_SEARCH_FIELD_TRIALS.npt_name_s, "Search", "Search for matching Field Trials", def, PL_BASIC)) != NULL)
								{
									if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_GET_ALL_FIELD_TRIALS.npt_type, S_GET_ALL_FIELD_TRIALS.npt_name_s, "List", "Get all of the existing Field Trials", def, PL_BASIC)) != NULL)
										{
											success_flag = true;
										}
								}
						}
				}
		}

	return success_flag;
}




bool RunForFieldTrialParams (DFWFieldTrialServiceData *data_p, ParameterSet *param_set_p, ServiceJob *job_p)
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


	if (!job_done_flag)
		{
			if (GetParameterValueFromParameterSet (param_set_p, S_SEARCH_FIELD_TRIALS.npt_name_s, &value, true))
				{
					if (value.st_boolean_value)
						{
							bool success_flag = SearchFieldTrials (job_p, name_s, team_s, data_p);

							job_done_flag = true;
						}		/* if (value.st_boolean_value) */
				}
		}


	return job_done_flag;
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


static bool SearchFieldTrials (ServiceJob *job_p, const char *name_s, const char *team_s, DFWFieldTrialServiceData *data_p)
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
							ok_flag = AddQueryTerm (query_p, FT_TEAM_S, team_s, false);
						}

					if (ok_flag)
						{
							bson_t *opts_p =  BCON_NEW ( "sort", "{", "team", BCON_INT32 (1), "}");

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

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

#define ALLOCATE_FIELD_TRIAL_CONSTANTS (1)
#include "field_trial_jobs.h"

#include "field_trial.h"
#include "string_utils.h"
#include "dfw_util.h"

#include "boolean_parameter.h"

/*
 * Field Trial parameters
 */

static NamedParameterType S_FUZZY_SEARCH_FIELD_TRIALS = { "Fuzzy Search", PT_BOOLEAN };
static NamedParameterType S_FULL_DATA = { "Get full data from search", PT_BOOLEAN };





static bool AddFieldTrial (ServiceJob *job_p, const char *name_s, const char *team_s, DFWFieldTrialServiceData *data_p);


static bool SearchFieldTrials (ServiceJob *job_p, const char *name_s, const char *team_s, const bool regex_flag, const ViewFormat format, DFWFieldTrialServiceData *data_p);

static bool AddFieldTrialToServiceJobResult (ServiceJob *job_p, FieldTrial *trial_p, json_t *trial_json_p, const ViewFormat format, DFWFieldTrialServiceData *data_p);




bool AddSubmissionFieldTrialParams (ServiceData *data_p, ParameterSet *param_set_p)
{
	bool success_flag = false;
	Parameter *param_p = NULL;

	if ((param_p = EasyCreateAndAddStringParameterToParameterSet (data_p, param_set_p, NULL, FIELD_TRIAL_NAME.npt_type, FIELD_TRIAL_NAME.npt_name_s, "Name", "The name of the Field Trial", NULL, PL_SIMPLE)) != NULL)
		{
			param_p -> pa_required_flag = true;

			if ((param_p = EasyCreateAndAddStringParameterToParameterSet (data_p, param_set_p, NULL, FIELD_TRIAL_TEAM.npt_type, FIELD_TRIAL_TEAM.npt_name_s, "Team", "The team name of the Field Trial", NULL, PL_SIMPLE)) != NULL)
				{
					if ((param_p = EasyCreateAndAddBooleanParameterToParameterSet (data_p, param_set_p, NULL, FIELD_TRIAL_ADD.npt_name_s, "Add", "Add a new Field Trial", false, PL_SIMPLE)) != NULL)
						{
							success_flag = true;
						}
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", FIELD_TRIAL_ADD.npt_name_s);
						}
				}
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", FIELD_TRIAL_TEAM.npt_name_s);
				}
		}
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", FIELD_TRIAL_NAME.npt_name_s);
		}

	return success_flag;
}


bool RunForSubmissionFieldTrialParams (DFWFieldTrialServiceData *data_p, ParameterSet *param_set_p, ServiceJob *job_p)
{
	bool job_done_flag = false;
	const char *name_s = NULL;
	const char *team_s = NULL;

	if (GetCurrentStringParameterValueFromParameterSet (param_set_p, FIELD_TRIAL_NAME.npt_name_s, &name_s))
		{
			const bool *add_flag_p = NULL;

			GetCurrentStringParameterValueFromParameterSet (param_set_p, FIELD_TRIAL_TEAM.npt_name_s, &team_s);

			if (GetCurrentBooleanParameterValueFromParameterSet (param_set_p, FIELD_TRIAL_ADD.npt_name_s, &add_flag_p))
				{
					if ((add_flag_p != NULL) && (*add_flag_p == true))
						{
							/* It's a job for FieldTrials */
							if (!AddFieldTrial (job_p, name_s, team_s, data_p))
								{

								}

							job_done_flag = true;
						}		/* if (value.st_boolean_value) */

				}		/* if (GetParameterValueFromParameterSet (param_set_p, S_ADD_FIELD_TRIAL.npt_name_s, &value, true)) */

		}		/* if (GetParameterValueFromParameterSet (param_set_p, S_FIELD_TRIAL_NAME.npt_name_s, &value, true)) */

	return job_done_flag;
}


bool GetSubmissionFieldTrialParameterTypeForNamedParameter (const char *param_name_s, ParameterType *pt_p)
{
	bool success_flag = true;

	if (strcmp (param_name_s, FIELD_TRIAL_NAME.npt_name_s) == 0)
		{
			*pt_p = FIELD_TRIAL_NAME.npt_type;
		}
	else if (strcmp (param_name_s, FIELD_TRIAL_TEAM.npt_name_s) == 0)
		{
			*pt_p = FIELD_TRIAL_TEAM.npt_type;
		}
	else if (strcmp (param_name_s, FIELD_TRIAL_ADD.npt_name_s) == 0)
		{
			*pt_p = FIELD_TRIAL_ADD.npt_type;
		}
	else
		{
			success_flag = false;
		}

	return success_flag;
}


bool AddSearchFieldTrialParams (ServiceData *data_p, ParameterSet *param_set_p)
{
	bool success_flag = false;
	Parameter *param_p = NULL;
	ParameterGroup *group_p = CreateAndAddParameterGroupToParameterSet ("Field Trials", false, data_p, param_set_p);

	if ((param_p = EasyCreateAndAddStringParameterToParameterSet (data_p, param_set_p, group_p, FIELD_TRIAL_ID.npt_type, FIELD_TRIAL_ID.npt_name_s, "Id", "The Id of the Field Trial", NULL, PL_ADVANCED)) != NULL)
		{
			if ((param_p = EasyCreateAndAddStringParameterToParameterSet (data_p, param_set_p, group_p, FIELD_TRIAL_NAME.npt_type, FIELD_TRIAL_NAME.npt_name_s, "Name", "The name of the Field Trial", NULL, PL_ADVANCED)) != NULL)
				{
					if ((param_p = EasyCreateAndAddStringParameterToParameterSet (data_p, param_set_p, group_p, FIELD_TRIAL_TEAM.npt_type, FIELD_TRIAL_TEAM.npt_name_s, "Team", "The team name of the Field Trial", NULL, PL_ADVANCED)) != NULL)
						{
							if ((param_p = EasyCreateAndAddStringParameterToParameterSet (data_p, param_set_p, group_p, FIELD_TRIAL_SEARCH.npt_type, FIELD_TRIAL_SEARCH.npt_name_s, "Search", "Search for matching Field Trials", NULL, PL_ADVANCED)) != NULL)
								{
									if ((param_p = EasyCreateAndAddStringParameterToParameterSet (data_p, param_set_p, group_p, S_FUZZY_SEARCH_FIELD_TRIALS.npt_type, S_FUZZY_SEARCH_FIELD_TRIALS.npt_name_s, "Fuzzy search", "When doing a search, do a fuzzy search", NULL, PL_ADVANCED)) != NULL)
										{
											bool full_data_flag = false;

											if ((param_p = EasyCreateAndAddBooleanParameterToParameterSet (data_p, param_set_p, group_p, S_FULL_DATA.npt_name_s, "Full data", "When doing a search, get the full data", &full_data_flag, PL_ADVANCED)) != NULL)
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
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", FIELD_TRIAL_SEARCH.npt_name_s);
								}
						}
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", FIELD_TRIAL_TEAM.npt_name_s);
						}
				}
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", FIELD_TRIAL_NAME.npt_name_s);
				}
		}
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", FIELD_TRIAL_ID.npt_name_s);
		}

	return success_flag;
}



bool GetSearchFieldTrialParameterTypeForNamedParameter (const char *param_name_s, ParameterType *pt_p)
{
	bool success_flag = true;

	if (strcmp (param_name_s, FIELD_TRIAL_ID.npt_name_s) == 0)
		{
			*pt_p = FIELD_TRIAL_ID.npt_type;
		}
	else if (strcmp (param_name_s, FIELD_TRIAL_NAME.npt_name_s) == 0)
		{
			*pt_p = FIELD_TRIAL_NAME.npt_type;
		}
	else if (strcmp (param_name_s, FIELD_TRIAL_TEAM.npt_name_s) == 0)
		{
			*pt_p = FIELD_TRIAL_TEAM.npt_type;
		}
	else if (strcmp (param_name_s, FIELD_TRIAL_SEARCH.npt_name_s) == 0)
		{
			*pt_p = FIELD_TRIAL_SEARCH.npt_type;
		}
	else if (strcmp (param_name_s, S_FUZZY_SEARCH_FIELD_TRIALS.npt_name_s) == 0)
		{
			*pt_p = S_FUZZY_SEARCH_FIELD_TRIALS.npt_type;
		}
	else if (strcmp (param_name_s, S_FULL_DATA.npt_name_s) == 0)
		{
			*pt_p = S_FULL_DATA.npt_type;
		}
	else
		{
			success_flag = false;
		}

	return success_flag;
}


bool RunForSearchFieldTrialParams (DFWFieldTrialServiceData *data_p, ParameterSet *param_set_p, ServiceJob *job_p)
{
	const bool *full_data_flag_p = NULL;
	bool job_done_flag = false;
	const char *id_s = NULL;

	GetCurrentBooleanParameterValueFromParameterSet (param_set_p, S_FULL_DATA.npt_name_s, &full_data_flag_p);

	if (GetCurrentStringParameterValueFromParameterSet (param_set_p, FIELD_TRIAL_ID.npt_name_s, &id_s))
		{
			if (!IsStringEmpty (id_s))
				{
					const size_t l = strlen (id_s);
					if (bson_oid_is_valid (id_s, l))
						{
							FieldTrial *trial_p = NULL;
							ViewFormat format = VF_CLIENT_MINIMAL;

							if ((full_data_flag_p != NULL) && (*full_data_flag_p == true))
								{
									format = VF_CLIENT_FULL;
								}

							trial_p = GetFieldTrialByIdString (id_s, format, data_p);

							if (trial_p)
								{
									if (AddFieldTrialToServiceJob (job_p, trial_p, format, data_p))
										{
											job_done_flag = true;
											SetServiceJobStatus (job_p, OS_SUCCEEDED);
										}

									FreeFieldTrial (trial_p);
								}

						}

					job_done_flag = true;
				}

		}		/* if (GetCurrentParameterValueFromParameterSet (param_set_p, FIELD_TRIAL_ID.npt_name_s, &value)) */


	if (!job_done_flag)
		{
			const char *name_s = NULL;
			const char *team_s = NULL;
			const bool *search_flag_p = NULL;

			if (GetCurrentStringParameterValueFromParameterSet (param_set_p, FIELD_TRIAL_NAME.npt_name_s, &name_s))
				{
					GetCurrentStringParameterValueFromParameterSet (param_set_p, FIELD_TRIAL_NAME.npt_name_s, &team_s);
				}		/* if (GetParameterValueFromParameterSet (param_set_p, S_FIELD_TRIAL_NAME.npt_name_s, &value, true)) */


			if (GetCurrentBooleanParameterValueFromParameterSet (param_set_p, FIELD_TRIAL_SEARCH.npt_name_s, &search_flag_p))
				{
					if ((search_flag_p != NULL) && (*search_flag_p == true))
						{
							bool success_flag;
							const bool *fuzzy_search_flag_p = NULL;
							bool fuzzy_search_flag = false;
							ViewFormat vf = VF_CLIENT_MINIMAL;

							GetCurrentBooleanParameterValueFromParameterSet (param_set_p, S_FUZZY_SEARCH_FIELD_TRIALS.npt_name_s, &fuzzy_search_flag_p);

							if ((fuzzy_search_flag_p != NULL) && (*fuzzy_search_flag_p == true))
								{
									fuzzy_search_flag = true;
								}

							if ((full_data_flag_p != NULL) && (*full_data_flag_p == true))
								{
									vf = VF_CLIENT_FULL;
								}

							success_flag = SearchFieldTrials (job_p, name_s, team_s, fuzzy_search_flag, vf, data_p);

							job_done_flag = true;
						}		/* if (value.st_boolean_value) */
				}

		}		/* if (!job_done_flag) */

	return job_done_flag;
}


json_t *GetAllFieldTrialsAsJSON (const DFWFieldTrialServiceData *data_p, bson_t *opts_p)
{
	json_t *results_p = NULL;

	if (SetMongoToolCollection (data_p -> dftsd_mongo_p, data_p -> dftsd_collection_ss [DFTD_FIELD_TRIAL]))
		{
			bson_t *query_p = NULL;

			results_p = GetAllMongoResultsAsJSON (data_p -> dftsd_mongo_p, query_p, opts_p);
		}		/* if (SetMongoToolCollection (data_p -> dftsd_mongo_p, data_p -> dftsd_collection_ss [DFTD_LOCATION])) */

	return results_p;
}


bool SetUpFieldTrialsListParameter (const DFWFieldTrialServiceData *data_p, StringParameter *param_p)
{
	bool success_flag = false;
	json_t *results_p = GetAllFieldTrialsAsJSON (data_p, NULL);
	bool value_set_flag = false;

	if (results_p)
		{
			if (json_is_array (results_p))
				{
					const size_t num_results = json_array_size (results_p);

					success_flag = true;

					if (num_results > 0)
						{
							size_t i = 0;
							const char *param_value_s = GetStringParameterDefaultValue (param_p);

							while ((i < num_results) && success_flag)
								{
									json_t *entry_p = json_array_get (results_p, i);
									FieldTrial *trial_p = GetFieldTrialFromJSON (entry_p, data_p);

									if (trial_p)
										{
											char *name_s = GetFieldTrialAsString (trial_p);

											if (name_s)
												{
													char *id_s = GetBSONOidAsString (trial_p -> ft_id_p);

													if (id_s)
														{
															if (param_value_s && (strcmp (param_value_s, id_s) == 0))
																{
																	value_set_flag = true;
																}

															if (!CreateAndAddStringParameterOption (param_p, id_s, name_s))
																{
																	success_flag = false;
																	PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add param option \"%s\": \"%s\"", id_s, name_s);
																}

															FreeCopiedString (id_s);
														}
													else
														{
															success_flag = false;
															PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, entry_p, "Failed to get FieldTrial BSON oid");
														}

													FreeCopiedString (name_s);
												}		/* if (name_s) */
											else
												{
													success_flag = false;
													PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, entry_p, "Failed to get FieldTrial as string");
												}

											FreeFieldTrial (trial_p);
										}		/* if (trial_p) */
									else
										{
											success_flag = false;
											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, entry_p, "Failed to get FieldTrial");
										}

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
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "param value \"%s\" not on list of existing trials", param_value_s);
								}

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


static bool AddFieldTrial (ServiceJob *job_p, const char *name_s, const char *team_s, DFWFieldTrialServiceData *data_p)
{
	bool success_flag = false;
	FieldTrial *trial_p = AllocateFieldTrial (name_s, team_s, NULL);

	if (trial_p)
		{
			success_flag = SaveFieldTrial (trial_p, job_p, data_p);
			FreeFieldTrial (trial_p);
		}

	SetServiceJobStatus (job_p, success_flag ? OS_SUCCEEDED : OS_FAILED);

	return success_flag;
}



static bool AddFieldTrialToServiceJobResult (ServiceJob *job_p, FieldTrial *trial_p, json_t *trial_json_p, const ViewFormat format, DFWFieldTrialServiceData *data_p)
{
	bool success_flag = false;

	if (GetAllFieldTrialStudies (trial_p, format, data_p))
		{
			if (AddStudiesToFieldTrialJSON (trial_p, trial_json_p, format, data_p))
				{
					char *title_s = GetFieldTrialAsString (trial_p);

					if (title_s)
						{
							if (AddContext (trial_json_p))
								{
									json_t *dest_record_p = GetResourceAsJSONByParts (PROTOCOL_INLINE_S, NULL, title_s, trial_json_p);

									if (dest_record_p)
										{
											AddImage (dest_record_p, DFTD_FIELD_TRIAL, data_p);

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

				}		/* if (AddStudiesToFieldTrialJSON (trial_p, trial_json_p, data_p)) */

		}

	return success_flag;
}


json_t *GetFieldTrialJSONForId (const char *id_s, const ViewFormat format, JSONProcessor *processor_p, char **name_ss, const DFWFieldTrialServiceData *data_p)
{
	json_t *trial_json_p = NULL;
	FieldTrial *trial_p = GetFieldTrialByIdString (id_s, format, data_p);

	if (trial_p)
		{
			if (format == VF_CLIENT_FULL)
				{

				}
			else
				{
					trial_json_p = GetFieldTrialAsJSON (trial_p, format, data_p);

				}

			*name_ss = EasyCopyToNewString (trial_p -> ft_name_s);

			FreeFieldTrial (trial_p);
		}		/* if (trial_p) */

	return trial_json_p;
}


bool AddFieldTrialToServiceJob (ServiceJob *job_p, FieldTrial *trial_p, const ViewFormat format, DFWFieldTrialServiceData *data_p)
{
	bool success_flag = false;
	json_t *trial_json_p = GetFieldTrialAsJSON (trial_p, format, data_p);

	if (trial_json_p)
		{
			if (AddFieldTrialToServiceJobResult (job_p, trial_p, trial_json_p, format, data_p))
				{
					success_flag = true;
				}

			json_decref (trial_json_p);
		}		/* if (trial_json_p) */

	return success_flag;
}


bool AddFieldTrialToServiceJobFromJSON (ServiceJob *job_p, json_t *trial_json_p, const ViewFormat format, DFWFieldTrialServiceData *data_p)
{
	bool success_flag = false;
	FieldTrial *trial_p = GetFieldTrialFromJSON (trial_json_p, data_p);

	if (trial_p)
		{
			if (AddFieldTrialToServiceJobResult (job_p, trial_p, trial_json_p, format, data_p))
				{
					success_flag = true;
				}

			FreeFieldTrial (trial_p);
		}		/* if (trial_p) */

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
																	if (!AddFieldTrialToServiceJobFromJSON (job_p, trial_json_p, format, data_p))
																		{
																			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, trial_json_p, "Failed to add FieldTrial to ServiceJob");
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



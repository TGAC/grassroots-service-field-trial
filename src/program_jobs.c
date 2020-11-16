/*
** Copyright 2014-2020 The Earlham Institute
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
 * program_jobs.c
 *
 *  Created on: 13 Nov 2020
 *      Author: billy
 */

#define ALLOCATE_PROGRAM_JOB_CONSTANTS (1)
#include "program_jobs.h"
#include "crop_jobs.h"


static const char * const S_EMPTY_LIST_OPTION_S = "<empty>";


static bool AddProgram (ServiceJob *job_p, ParameterSet *param_set_p, FieldTrialServiceData *data_p);

static bool AddProgramToServiceJobResult (ServiceJob *job_p, Program *program_p, json_t *program_json_p, const ViewFormat format, FieldTrialServiceData *data_p);



bool AddSubmissionProgramParams (ServiceData *data_p, ParameterSet *param_set_p, Resource *resource_p)
{
	FieldTrialServiceData *dfw_data_p = (FieldTrialServiceData *) data_p;
	bool success_flag = false;
	Parameter *param_p = NULL;
	char *id_s = NULL;
	char *abbreviation_s = NULL;
	char *crop_s = NULL;
	char *documentation_url_s = NULL;
	char *name_s = NULL;
	char *objective_s = NULL;
	char *pi_name_s = NULL;
	Program *active_program_p = NULL; //GetFieldTrialFromResource (resource_p, FIELD_TRIAL_ID, dfw_data_p);
	bool defaults_flag = false;

	if (active_program_p)
		{
			/*
			if (SetUpDefaultsFromExistingFieldTrial (active_trial_p, &id_s, &program_id_s, &name_s, &team_s))
				{
					defaults_flag = true;
				}
				*/
		}
	else
		{
			id_s = (char *) S_EMPTY_LIST_OPTION_S;
			defaults_flag = true;
		}


	if (defaults_flag)
		{
			if ((param_p = EasyCreateAndAddStringParameterToParameterSet (data_p, param_set_p, NULL, PROGRAM_ID.npt_type, PROGRAM_ID.npt_name_s, "Load Program", "Edit an existing Program", id_s, PL_ALL)) != NULL)
				{
					if (SetUpProgramsListParameter (dfw_data_p, (StringParameter *) param_p, active_program_p, true))
						{
							/*
							 * We want to update all of the values in the form
							 * when a user selects a study from the list so
							 * we need to make the parameter automatically
							 * refresh the values. So we set the
							 * pa_refresh_service_flag to true.
							 */
							param_p -> pa_refresh_service_flag = true;

							if ((param_p = EasyCreateAndAddStringParameterToParameterSet (data_p, param_set_p, NULL, PROGRAM_NAME.npt_type, PROGRAM_NAME.npt_name_s, "Name", "The name of the Prgram", name_s, PL_ALL)) != NULL)
								{
									param_p -> pa_required_flag = true;

									if ((param_p = EasyCreateAndAddStringParameterToParameterSet (data_p, param_set_p, NULL, PROGRAM_ABBREVIATION.npt_type, PROGRAM_ABBREVIATION.npt_name_s, "Abbreviation", "The abbreviation for the Program", abbreviation_s, PL_ALL)) != NULL)
										{
											if ((param_p = EasyCreateAndAddStringParameterToParameterSet (data_p, param_set_p, NULL, PROGRAM_CROP.npt_type, PROGRAM_CROP.npt_name_s, "Crop", "The crop for the Program", crop_s, PL_ALL)) != NULL)
												{
													if (SetUpCropsListParameter (data_p, param_p, S_EMPTY_LIST_OPTION_S))
														{
															if ((param_p = EasyCreateAndAddStringParameterToParameterSet (data_p, param_set_p, NULL, PROGRAM_OBJECTIVE.npt_type, PROGRAM_OBJECTIVE.npt_name_s, "Objective", "The Program's objective", objective_s, PL_ALL)) != NULL)
																{
																	if ((param_p = EasyCreateAndAddStringParameterToParameterSet (data_p, param_set_p, NULL, PROGRAM_URL.npt_type, PROGRAM_URL.npt_name_s, "Url", "The web page documenting this Program", documentation_url_s, PL_ALL)) != NULL)
																		{
																			if ((param_p = EasyCreateAndAddStringParameterToParameterSet (data_p, param_set_p, NULL, PROGRAM_PI_NAME.npt_type, PROGRAM_PI_NAME.npt_name_s, "Principal Investigator", "The Program's lead", pi_name_s, PL_ALL)) != NULL)
																				{
																					success_flag = true;
																				}
																			else
																				{
																					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", PROGRAM_PI_NAME.npt_name_s);
																				}
																		}
																	else
																		{
																			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", PROGRAM_URL.npt_name_s);
																		}
																}

														}

												}
										}
								}

						}		/* if (SetUpFieldTrialsListParameter ((FieldTrialServiceData *) data_p, (StringParameter *) param_p, NULL, true)) */

				}		/* if ((param_p = EasyCreateAndAddStringParameterToParameterSet (data_p, param_set_p, NULL, FIELD_TRIAL_ID.npt_type, FIELD_TRIAL_ID.npt_name_s, "Load Field Trial", "Edit an existing Field Trial", id_s, PL_ADVANCED)) != NULL) */

		}		/* if (defaults_flag) */


	return success_flag;
}



json_t *GetAllProgramsAsJSON (const FieldTrialServiceData *data_p, bson_t *opts_p)
{
	json_t *results_p = NULL;

	if (SetMongoToolCollection (data_p -> dftsd_mongo_p, data_p -> dftsd_collection_ss [DFTD_PROGRAM]))
		{
			bson_t *query_p = NULL;

			results_p = GetAllMongoResultsAsJSON (data_p -> dftsd_mongo_p, query_p, opts_p);
		}		/* if (SetMongoToolCollection (data_p -> dftsd_mongo_p, data_p -> dftsd_collection_ss [DFTD_LOCATION])) */

	return results_p;
}


bool SetUpProgramsListParameter (const FieldTrialServiceData *data_p, StringParameter *param_p, const Program *active_program_p, const bool empty_option_flag)
{
	bool success_flag = false;
	json_t *results_p = GetAllProgramsAsJSON (data_p, NULL);
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
											Program *program_p = GetProgramFromJSON (entry_p, VF_CLIENT_MINIMAL, data_p);

											if (program_p)
												{
													char *id_s = GetBSONOidAsString (program_p -> pr_id_p);

													if (id_s)
														{
															if (param_value_s && (strcmp (param_value_s, id_s) == 0))
																{
																	value_set_flag = true;
																}

															if (!CreateAndAddStringParameterOption (param_p, id_s, program_p -> pr_name_s))
																{
																	success_flag = false;
																	PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add param option \"%s\": \"%s\"", id_s, program_p -> pr_name_s);
																}

															FreeCopiedString (id_s);
														}
													else
														{
															success_flag = false;
															PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, entry_p, "Failed to get Program BSON oid");
														}

													FreeProgram (program_p);
												}		/* if (program_p) */


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
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "param value \"%s\" not on list of existing programs", param_value_s);
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
			if (active_program_p)
				{
					char *id_s = GetBSONOidAsString (active_program_p -> pr_id_p);

					if (id_s)
						{
							success_flag = SetStringParameterDefaultValue (param_p, id_s);
							FreeCopiedString (id_s);
						}
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get id string for active program \"%s\"", active_program_p -> pr_name_s);
							success_flag = false;
						}
				}
		}

	return success_flag;
}


bool RunForSubmissionProgramParams (FieldTrialServiceData *data_p, ParameterSet *param_set_p, ServiceJob *job_p)
{
	bool success_flag = AddProgram (job_p, param_set_p, data_p);

	return success_flag;
}



bool GetSubmissionProgramParameterTypeForNamedParameter (const char *param_name_s, ParameterType *pt_p)
{
	bool success_flag = true;

	if (strcmp (param_name_s, PROGRAM_ID.npt_name_s) == 0)
		{
			*pt_p = PROGRAM_ID.npt_type;
		}
	else if (strcmp (param_name_s, PROGRAM_ABBREVIATION.npt_name_s) == 0)
		{
			*pt_p = PROGRAM_ABBREVIATION.npt_type;
		}
	else if (strcmp (param_name_s, PROGRAM_CROP.npt_name_s) == 0)
		{
			*pt_p = PROGRAM_CROP.npt_type;
		}
	else if (strcmp (param_name_s, PROGRAM_NAME.npt_name_s) == 0)
		{
			*pt_p = PROGRAM_NAME.npt_type;
		}
	else if (strcmp (param_name_s, PROGRAM_OBJECTIVE.npt_name_s) == 0)
		{
			*pt_p = PROGRAM_OBJECTIVE.npt_type;
		}
	else if (strcmp (param_name_s, PROGRAM_PI_NAME.npt_name_s) == 0)
		{
			*pt_p = PROGRAM_PI_NAME.npt_type;
		}
	else if (strcmp (param_name_s, PROGRAM_URL.npt_name_s) == 0)
		{
			*pt_p = PROGRAM_URL.npt_type;
		}
	else
		{
			success_flag = false;
		}

	return success_flag;
}




static bool AddProgram (ServiceJob *job_p, ParameterSet *param_set_p, FieldTrialServiceData *data_p)
{
	OperationStatus status = OS_FAILED;
	const char *id_s = NULL;
	const char *name_s = NULL;
	bson_oid_t *program_id_p = NULL;

	/*
	 * Get the existing program id if specified
	 */
	GetCurrentStringParameterValueFromParameterSet (param_set_p, PROGRAM_ID.npt_name_s, &id_s);

	if (id_s)
		{
			if (strcmp (S_EMPTY_LIST_OPTION_S, id_s) != 0)
				{
					program_id_p = GetBSONOidFromString (id_s);

					if (!program_id_p)
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to load Program \"%s\" for editing", id_s);
							return false;
						}
				}
		}		/* if (id_value.st_string_value_s) */


	if (GetCurrentStringParameterValueFromParameterSet (param_set_p, PROGRAM_NAME.npt_name_s, &name_s))
		{
			const char *pi_s = NULL;

			if (GetCurrentStringParameterValueFromParameterSet (param_set_p, PROGRAM_PI_NAME.npt_name_s, &pi_s))
				{
					Program *program_p = NULL;
					const char *abbreviation_s = NULL;
					const char *crop_s = NULL;
					const char *url_s = NULL;
					const char *objective_s = NULL;

					GetCurrentStringParameterValueFromParameterSet (param_set_p, PROGRAM_ABBREVIATION.npt_name_s, &abbreviation_s);
					GetCurrentStringParameterValueFromParameterSet (param_set_p, PROGRAM_CROP.npt_name_s, &crop_s);
					GetCurrentStringParameterValueFromParameterSet (param_set_p, PROGRAM_URL.npt_name_s, &url_s);
					GetCurrentStringParameterValueFromParameterSet (param_set_p, PROGRAM_OBJECTIVE.npt_name_s, &objective_s);

					program_p = AllocateProgram (program_id_p, abbreviation_s, crop_s, url_s, name_s, objective_s, pi_s);

					if (program_p)
						{
							status = SaveProgram (program_p, job_p, data_p);

							if (status == OS_FAILED)
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to save Program named \"%s\"", name_s);
								}

							FreeProgram (program_p);
						}

				}
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get Program parameter %s", PROGRAM_PI_NAME.npt_name_s);
				}

		}
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get Program parameter %s", PROGRAM_NAME.npt_name_s);
		}


	SetServiceJobStatus (job_p, status);

	return ((status == OS_SUCCEEDED) || (status == OS_PARTIALLY_SUCCEEDED));
}


bool AddProgramToServiceJob (ServiceJob *job_p, Program *program_p, const ViewFormat format, FieldTrialServiceData *data_p)
{
	bool success_flag = false;
	json_t *program_json_p = GetProgramAsJSON (program_p, format, data_p);

	if (program_json_p)
		{
			if (AddProgramToServiceJobResult (job_p, program_p, program_json_p, format, data_p))
				{
					success_flag = true;
				}

			json_decref (program_json_p);
		}		/* if (program_json_p) */

	return success_flag;
}


static bool AddProgramToServiceJobResult (ServiceJob *job_p, Program *program_p, json_t *program_json_p, const ViewFormat format, FieldTrialServiceData *data_p)
{
	bool success_flag = false;

	json_t *dest_record_p = GetResourceAsJSONByParts (PROTOCOL_INLINE_S, NULL, program_p -> pr_name_s, program_json_p);

	if (dest_record_p)
		{
			AddImage (dest_record_p, DFTD_PROGRAM, data_p);

			if (AddResultToServiceJob (job_p, dest_record_p))
				{
					success_flag = true;
				}
			else
				{
					json_decref (dest_record_p);
				}

		}		/* if (dest_record_p) */

	return success_flag;
}

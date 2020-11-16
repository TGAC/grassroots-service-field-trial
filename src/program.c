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
 * program.c
 *
 *  Created on: 7 Jan 2019
 *      Author: billy
 */


#define ALLOCATE_PROGRAM_TAGS (1)
#include "program.h"

#include "memory_allocations.h"
#include "dfw_util.h"
#include "indexing.h"


static LinkedList *GetMatchingPrograms (const FieldTrialServiceData *data_p, const char **keys_ss, const char **values_ss);

static void *GetProgramObjectFromJSON (const json_t *json_p, const ViewFormat format, const FieldTrialServiceData *data_p);

static bool AddFullDetailsToProgramJSON (const Program *program_p, json_t *program_json_p);


Program *AllocateProgram (bson_oid_t *id_p, const char *abbreviation_s, const char *common_crop_name_s, const char *documentation_url_s, const char *name_s, const char *objective_s, const char *pi_name_s)
{
	char *copied_abbreviation_s = NULL;

	if ((abbreviation_s == NULL) || (copied_abbreviation_s = EasyCopyToNewString (abbreviation_s)))
		{
			char *copied_crop_s = NULL;

			if ((common_crop_name_s == NULL) || (copied_crop_s = EasyCopyToNewString (common_crop_name_s)))
				{
					char *copied_documentation_url_s = NULL;

					if ((documentation_url_s == NULL) || (copied_documentation_url_s = EasyCopyToNewString (documentation_url_s)))
						{
							char *copied_name_s = EasyCopyToNewString (name_s);

							if (copied_name_s)
								{
									char *copied_objective_s = NULL;

									if ((objective_s == NULL) || (copied_objective_s = EasyCopyToNewString (objective_s)))
										{
											char *copied_pi_s = EasyCopyToNewString (pi_name_s);

											if (copied_pi_s)
												{
													LinkedList *trials_p = AllocateLinkedList (FreeFieldTrialNode);

													if (trials_p)
														{
															Program *program_p = (Program *) AllocMemory (sizeof (Program));

															if (program_p)
																{
																	program_p -> pr_abbreviation_s = copied_abbreviation_s;
																	program_p -> pr_common_crop_name_s = copied_crop_s;
																	program_p -> pr_documentation_url_s = copied_documentation_url_s;
																	program_p -> pr_id_p = id_p;
																	program_p -> pr_name_s = copied_name_s;
																	program_p -> pr_objective_s = copied_objective_s;
																	program_p -> pr_pi_name_s = copied_pi_s;
																	program_p -> pr_trials_p = trials_p;

																	return program_p;
																}		/* if (program_p) */

															FreeLinkedList (trials_p);
														}		/* if (trials_p) */

													FreeCopiedString (copied_pi_s);
												}		/* if (copied_pi_s) */


											if (copied_objective_s)
												{
													FreeCopiedString (copied_objective_s);
												}

										}		/* if ((objective_s == NULL) || (copied_objective_s = EasyCopyToNewString (objective_s))) */

									FreeCopiedString (copied_name_s);
								}		/* if (copied_name_s) */


							if (copied_documentation_url_s)
								{
									FreeCopiedString (copied_documentation_url_s);
								}

						}		/* if ((documentation_url_s == NULL) || (copied_documentation_url_s = EasyCopyToNewString (documentation_url_s))) */

					if (copied_crop_s)
						{
							FreeCopiedString (copied_crop_s);
						}

				}		/* if ((common_crop_name_s == NULL) || (copied_crop_s = EasyCopyToNewString (common_crop_name_s))) */

			if (copied_abbreviation_s)
				{
					FreeCopiedString (copied_abbreviation_s);
				}

		}		/* if ((abbreviation_s == NULL) || (copied_abbreviation_s = EasyCopyToNewString (abbreviation_s))) */

	return NULL;
}


void FreeProgram (Program *program_p)
{
	if (program_p -> pr_abbreviation_s)
		{
			FreeCopiedString (program_p -> pr_abbreviation_s);
		}

	if (program_p -> pr_common_crop_name_s)
		{
			FreeCopiedString (program_p -> pr_common_crop_name_s);
		}

	if (program_p -> pr_documentation_url_s)
		{
			FreeCopiedString (program_p -> pr_documentation_url_s);
		}

	if (program_p -> pr_objective_s)
		{
			FreeCopiedString (program_p -> pr_objective_s);
		}

	FreeCopiedString (program_p -> pr_name_s);

	FreeCopiedString (program_p -> pr_pi_name_s);

	FreeLinkedList (program_p -> pr_trials_p);

	FreeMemory (program_p);
}


json_t *GetProgramAsJSON (Program *program_p, const ViewFormat format, const FieldTrialServiceData *data_p)
{
	json_t *program_json_p = json_object ();

	if (program_json_p)
		{
			if (SetNonTrivialString (program_json_p, PR_NAME_S, program_p -> pr_name_s))
				{
					if (SetNonTrivialString (program_json_p, PR_PI_NAME_S, program_p -> pr_pi_name_s))
						{
							if (AddDatatype (program_json_p, DFTD_PROGRAM))
								{
									bool success_flag = false;

									switch (format)
										{
											case VF_CLIENT_FULL:
												{
													if (AddFullDetailsToProgramJSON (program_p, program_json_p))
														{
															if (AddFieldTrialsToProgramJSON (program_p, program_json_p, format, data_p))
																{
																	success_flag = true;
																}
														}
												}
												break;

											case VF_STORAGE:
												{
													if (AddFullDetailsToProgramJSON (program_p, program_json_p))
														{
															success_flag = true;
														}
												}
												break;

											case VF_CLIENT_MINIMAL:
											default:
												success_flag = true;
												break;
										}

									if (success_flag)
										{
											return program_json_p;
										}
								}

						}		/* if (json_object_set_new (trial_json_p, team_key_s, json_string (trial_p -> ft_team_s)) == 0) */

				}		/* if (json_object_set_new (trial_json_p, name_key_s, json_string (trial_p -> ft_name_s)) == 0) */

			json_decref (program_json_p);
		}		/* if (program_json_p) */

	return NULL;

}


Program *GetProgramFromJSON (const json_t *json_p, const ViewFormat format, const FieldTrialServiceData *data_p)
{
	const char *name_s = GetJSONString (json_p, PR_NAME_S);

	if (name_s)
		{
			const char *pi_s = GetJSONString (json_p, PR_PI_NAME_S);

			if (pi_s)
				{
					bson_oid_t *id_p = GetNewUnitialisedBSONOid ();

					if (id_p)
						{
							if (GetMongoIdFromJSON (json_p, id_p))
								{
									const char *objective_s = GetJSONString (json_p, PR_OBJECTIVE_S);
									const char *documentation_url_s = GetJSONString (json_p, PR_DOCUMENTATION_URL_S);
									const char *crop_s = GetJSONString (json_p, PR_CROP_S);
									const char *abbreviation_s = GetJSONString (json_p, PR_ABBREVIATION_S);

									Program *program_p = AllocateProgram (id_p, abbreviation_s, crop_s, documentation_url_s, name_s, objective_s, pi_s);

									if (program_p)
										{
											return program_p;
										}
								}

							FreeBSONOid (id_p);
						}
				}
		}

	return NULL;
}


bool AddProgramFieldTrial (Program *program_p, FieldTrial *trial_p, MEM_FLAG mf)
{
	bool success_flag = false;
	FieldTrialNode *node_p = AllocateFieldTrialNode (trial_p);

	if (node_p)
		{
			trial_p -> ft_parent_p = program_p;
			trial_p -> ft_parent_program_mem = mf;

			LinkedListAddTail (program_p -> pr_trials_p, & (node_p -> ftn_node));
			success_flag  = true;
		}

	return success_flag;
}



uint32 GetNumberOfProgramFieldTrials (const Program *program_p)
{
	return (program_p -> pr_trials_p ? program_p -> pr_trials_p -> ll_size : 0);
}



bool AddFieldTrialsToProgramJSON (Program *program_p, json_t *program_json_p, const ViewFormat format, const FieldTrialServiceData *data_p)
{
	bool success_flag = true;

	if (program_p -> pr_trials_p -> ll_size > 0)
		{
			json_t *trials_p = json_array ();

			if (trials_p)
				{
					FieldTrialNode *node_p = (FieldTrialNode *) (program_p -> pr_trials_p -> ll_head_p);
					bool ok_flag = true;

					while (node_p && ok_flag)
						{
							json_t *trial_p = GetFieldTrialAsJSON (node_p -> ftn_field_trial_p, format, data_p);

							if (trial_p)
								{
									if (json_array_append_new (trials_p, trial_p) == 0)
										{
											node_p = (FieldTrialNode *) (node_p -> ftn_node.ln_next_p);
										}
									else
										{
											ok_flag = false;
											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, trial_p, "Failed to add Field Trial json to array for program \"%s\" - \"%s\"", program_p -> pr_name_s, program_p -> pr_pi_name_s);
										}
								}
							else
								{
									char buffer_s [MONGO_OID_STRING_BUFFER_SIZE];

									ok_flag = false;
									bson_oid_to_string (node_p -> ftn_field_trial_p -> ft_id_p, buffer_s);

									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, trial_p, "Failed to get Trial json for area \"%s\"", buffer_s);
								}
						}		/* while (node_p && success_flag) */

					if (ok_flag)
						{
							if (json_object_set_new (program_json_p, PR_TRIALS_S, trials_p) == 0)
								{
									success_flag = true;
								}
							else
								{
									char buffer_s [MONGO_OID_STRING_BUFFER_SIZE];

									ok_flag = false;

									bson_oid_to_string (program_p -> pr_id_p, buffer_s);

									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, trials_p, "Failed to add Trial json to Prpgram \"%s\"", buffer_s);

									json_decref (trials_p);
								}
						}

				}		/* if (exp_areas_p) */
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate Trials json object for Program \"%s\" - \"%s\"", program_p -> pr_name_s, program_p -> pr_pi_name_s);
				}

		}		/* if (trial_p -> ft_studies_p -> ll_size > 0) */
	else
		{
			/* nothing to add */
			success_flag = true;
		}

	return success_flag;
}


/*
 * The Program could be the bson_oid or a name so check
 */

Program *GetUniqueProgramBySearchString (const char *program_s, const ViewFormat format, const FieldTrialServiceData *data_p)
{
	Program *program_p = NULL;

	if (bson_oid_is_valid (program_s, strlen (program_s)))
		{
			program_p = GetProgramByIdString (program_s, format, data_p);
		}

	if (!program_p)
		{
			LinkedList *programs_p = GetProgramsByName (program_s, data_p);

			if (programs_p)
				{
					if (programs_p -> ll_size == 1)
						{
							ProgramNode *node_p = (ProgramNode *) (programs_p -> ll_head_p);

							/* Remove the program from the node */
							program_p = node_p -> pn_program_p;
							node_p -> pn_program_p = NULL;
						}

					FreeLinkedList (programs_p);
				}
		}

	return program_p;
}


LinkedList *GetProgramsByName (const char * const program_s, const FieldTrialServiceData *data_p)
{
	const char *keys_ss [] = { PR_NAME_S, NULL };
	const char *values_ss [] = { program_s, NULL };

	return GetMatchingPrograms (data_p, keys_ss, values_ss);
}


static LinkedList *GetMatchingPrograms (const FieldTrialServiceData *data_p, const char **keys_ss, const char **values_ss)
{
	LinkedList *field_trials_list_p = AllocateLinkedList (FreeFieldTrialNode);

	if (field_trials_list_p)
		{
			bson_t *query_p = bson_new ();

			if (query_p)
				{
					const char **key_ss = keys_ss;
					const char **value_ss = values_ss;
					bool success_flag = true;

					while (success_flag && (*key_ss != NULL) && (*value_ss != NULL))
						{
							if (BSON_APPEND_UTF8 (query_p, *key_ss, *value_ss))
								{
									++ key_ss;
									++ value_ss;
								}
							else
								{
									success_flag = false;
								}
						}

					if (success_flag)
						{
							if (SetMongoToolCollection (data_p -> dftsd_mongo_p, data_p -> dftsd_collection_ss [DFTD_FIELD_TRIAL]))
								{
									json_t *results_p = GetAllMongoResultsAsJSON (data_p -> dftsd_mongo_p, query_p, NULL);

									if (results_p)
										{
											const size_t size = json_array_size (results_p);
											size_t i = 0;

											for (i = 0; i < size; ++ i)
												{
													json_t *result_p = json_array_get (results_p, i);
													FieldTrial *trial_p = GetFieldTrialFromJSON (result_p, VF_STORAGE, data_p);

													if (trial_p)
														{
															FieldTrialNode *node_p = AllocateFieldTrialNode (trial_p);

															if (node_p)
																{
																	LinkedListAddTail (field_trials_list_p, & (node_p -> ftn_node));
																}
															else
																{
																	PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, result_p, "Failed to create FieldTrialNode");
																}

														}		/* if (trial_p) */
													else
														{
															PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, result_p, "Failed to get FieldTrial from JSON");
														}

												}		/* for (i = 0; i < size; ++ i) */


											json_decref (results_p);
										}		/* if (results_p) */

								}		/* if (SetMongoToolCollection (data_p -> dftsd_mongo_p, data_p -> dftsd_collection_ss [DFTD_FIELD_TRIAL])) */
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to set mongo tool collection to \"%s\"", data_p -> dftsd_collection_ss [DFTD_FIELD_TRIAL]);
								}

						}		/* if (success_flag) */
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add query");
						}

					bson_destroy (query_p);
				}		/* if (query_p) */


		}		/* if (field_trials_list_p) */

	return field_trials_list_p;
}





Program *GetProgramById (const bson_oid_t *id_p, const ViewFormat format, const FieldTrialServiceData *data_p)
{
	Program *program_p = NULL;
	char *id_s = GetBSONOidAsString (id_p);

	if (id_s)
		{
			program_p = GetProgramByIdString (id_s, format, data_p);

			FreeCopiedString (id_s);
		}

	return program_p;
}

static void *GetProgramObjectFromJSON (const json_t *json_p, const ViewFormat format, const FieldTrialServiceData *data_p)
{
	return (void *) GetProgramFromJSON (json_p, format, data_p);
}


Program *GetProgramByIdString (const char *program_id_s, const ViewFormat format, const FieldTrialServiceData *data_p)
{
	Program *program_p = NULL;

	if (bson_oid_is_valid (program_id_s, strlen (program_id_s)))
		{
			bson_oid_t *id_p = GetBSONOidFromString (program_id_s);

			if (id_p)
				{
					void *obj_p = GetDFWObjectById (id_p, DFTD_PROGRAM, GetProgramObjectFromJSON, format, data_p);
					program_p = (Program *) obj_p;

					FreeBSONOid (id_p);
				}
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to create BSON OID for \"%s\"", program_id_s);
				}

		}		/* if (bson_oid_is_valid (field_trial_id_s, strlen (field_trial_id_s))) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "\"%s\" is not a valid oid", program_id_s);
		}

	return program_p;
}



OperationStatus SaveProgram (Program *program_p, ServiceJob *job_p, FieldTrialServiceData *data_p)
{
	OperationStatus status = OS_FAILED;
	bson_t *selector_p = NULL;
	bool success_flag = PrepareSaveData (& (program_p -> pr_id_p), &selector_p);

	if (success_flag)
		{
			json_t *program_json_p = GetProgramAsJSON (program_p, VF_STORAGE, data_p);

			if (program_json_p)
				{
					if (SaveMongoData (data_p -> dftsd_mongo_p, program_json_p, data_p -> dftsd_collection_ss [DFTD_PROGRAM], selector_p))
						{
							status = IndexData (job_p, program_json_p);

							if (status != OS_SUCCEEDED)
								{
									status = OS_PARTIALLY_SUCCEEDED;
									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, program_json_p, "Failed to index Program \"%s\" as JSON to Lucene", program_p -> pr_name_s);
									AddGeneralErrorMessageToServiceJob (job_p, "Program saved but failed to index for searching");
								}
						}

					json_decref (program_json_p);
				}		/* if (program_json_p) */

		}		/* if (success_flag) */

	SetServiceJobStatus (job_p, status);

	return status;
}



bool RemoveProgramFieldTrial (Program *program_p, FieldTrial *trial_p)
{
	bool removed_flag = false;

	if (program_p -> pr_trials_p)
		{
			FieldTrialNode *node_p = (FieldTrialNode *) (program_p -> pr_trials_p -> ll_head_p);

			while (node_p && !removed_flag)
				{
					if (node_p -> ftn_field_trial_p == trial_p)
						{
							LinkedListRemove (program_p -> pr_trials_p, & (node_p -> ftn_node));

							node_p -> ftn_field_trial_p = NULL;
							FreeStudyNode (& (node_p -> ftn_node));
							removed_flag = true;
						}


					if (!removed_flag)
						{
							node_p = (FieldTrialNode *) (node_p -> ftn_node.ln_next_p);
						}
				}
		}


	return removed_flag;
}





static bool AddFullDetailsToProgramJSON (const Program *program_p, json_t *program_json_p)
{
	if (SetNonTrivialString (program_json_p, PR_DOCUMENTATION_URL_S, program_p -> pr_documentation_url_s))
		{
			if (SetNonTrivialString (program_json_p, PR_OBJECTIVE_S, program_p -> pr_objective_s))
				{
					if (SetNonTrivialString (program_json_p, PR_ABBREVIATION_S, program_p -> pr_abbreviation_s))
						{
							if (SetNonTrivialString (program_json_p, PR_CROP_S, program_p -> pr_common_crop_name_s))
								{
									if (AddCompoundIdToJSON (program_json_p, program_p -> pr_id_p))
										{
											return true;

										}
								}
						}
				}
		}

	return false;
}

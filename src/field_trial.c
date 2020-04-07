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
 * field_trial.c
 *
 *  Created on: 11 Sep 2018
 *      Author: billy
 */


#define ALLOCATE_FIELD_TRIAL_TAGS (1)
#include "field_trial.h"
#include "field_trial_mongodb.h"
#include "dfw_field_trial_service_data.h"
#include "string_utils.h"
#include "study.h"
#include "memory_allocations.h"
#include "streams.h"
#include "dfw_util.h"
#include "indexing.h"
#include "json_processor.h"


FieldTrial *AllocateFieldTrial (const char *name_s, const char *team_s, bson_oid_t *id_p)
{
	char *copied_name_s = EasyCopyToNewString (name_s);

	if (copied_name_s)
		{
			char *copied_team_s = NULL;

			if ((team_s == NULL) || (copied_team_s = EasyCopyToNewString (team_s)))
				{
					LinkedList *areas_p = AllocateLinkedList (FreeStudyNode);

					if (areas_p)
						{
							FieldTrial *trial_p = (FieldTrial *) AllocMemory (sizeof (FieldTrial));

							if (trial_p)
								{
									trial_p -> ft_name_s = copied_name_s;
									trial_p -> ft_team_s = copied_team_s;
									trial_p -> ft_id_p = id_p;
									trial_p -> ft_studies_p = areas_p;

									return trial_p;
								}

							FreeLinkedList (areas_p);
						}		/* if (areas_p) */

					FreeCopiedString (copied_team_s);
				}		/* if (copied_team_s) */

			FreeCopiedString (copied_name_s);
		}		/* if (copied_name_s) */

	return NULL;
}


FieldTrial *AllocateFieldTrialWithIdAsString (const char *name_s, const char *team_s, const char *id_s)
{
	if (id_s)
		{
			bson_oid_t *id_p = GetBSONOidFromString (id_s);

			if (id_p)
				{
					FieldTrial *trial_p = AllocateFieldTrial (name_s, team_s, id_p);

					if (trial_p)
						{
							return trial_p;
						}

					FreeMemory (id_p);
				}

		}

	return NULL;
}




bool GenerateIdFromString (bson_oid_t **id_pp, const char *id_s)
{
	bool success_flag = false;

	if (bson_oid_is_valid (id_s, strlen (id_s)))
		{
			bson_oid_t *id_p = GetNewUnitialisedBSONOid ();

			if (id_p)
				{
					bson_oid_init_from_string (id_p, id_s);
					*id_pp = id_p;
					success_flag = true;
				}
		}

	return success_flag;
}


char *GetFieldTrialIdAsString (const FieldTrial *trial_p)
{
	char *id_s = NULL;

	if (trial_p -> ft_id_p)
		{
			id_s = GetBSONOidAsString (trial_p -> ft_id_p);
		}

	return id_s;
}


bool SaveFieldTrial (FieldTrial *trial_p, ServiceJob *job_p, FieldTrialServiceData *data_p)
{
	bson_t *selector_p = NULL;
	bool success_flag = PrepareSaveData (& (trial_p -> ft_id_p), &selector_p);
	OperationStatus status = OS_FAILED;

	if (success_flag)
		{
			json_t *field_trial_json_p = GetFieldTrialAsJSON (trial_p, VF_STORAGE, data_p);

			if (field_trial_json_p)
				{
					if (SaveMongoData (data_p -> dftsd_mongo_p, field_trial_json_p, data_p -> dftsd_collection_ss [DFTD_FIELD_TRIAL], selector_p))
						{
							if (IndexData (job_p, field_trial_json_p))
								{
									status = OS_SUCCEEDED;
								}
							else
								{
									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, field_trial_json_p, "Failed to index FieldTrial \"%s\" as JSON to Lucene", trial_p -> ft_name_s);
									AddGeneralErrorMessageToServiceJob (job_p, "Trial saved but failed to index for searching");

									status = OS_PARTIALLY_SUCCEEDED;
								}

						}		/* if (SaveMongoData (data_p -> dftsd_mongo_p, field_trial_json_p, data_p -> dftsd_collection_ss [DFTD_FIELD_TRIAL], selector_p)) */

					json_decref (field_trial_json_p);
				}		/* if (field_trial_json_p) */

		}		/* if (success_flag) */

	SetServiceJobStatus (job_p, status);

	return success_flag;
}



void FreeFieldTrial (FieldTrial *trial_p)
{
	FreeCopiedString (trial_p -> ft_name_s);
	FreeCopiedString (trial_p -> ft_team_s);

	if (trial_p -> ft_id_p)
		{
			FreeBSONOid (trial_p -> ft_id_p);
		}

	if (trial_p -> ft_studies_p)
		{
			FreeLinkedList (trial_p -> ft_studies_p);
		}

	FreeMemory (trial_p);
}


uint32 GetNumberOfFieldTrialStudies (const FieldTrial *trial_p)
{
	return (trial_p -> ft_studies_p ? trial_p -> ft_studies_p -> ll_size : 0);
}


bool RemoveFieldTrialStudy (FieldTrial *trial_p, Study *study_p)
{
	bool removed_flag = false;

	if (trial_p -> ft_studies_p)
		{
			StudyNode *node_p = (StudyNode *) (trial_p -> ft_studies_p -> ll_head_p);

			while (node_p && !removed_flag)
				{
					if (node_p -> stn_study_p == study_p)
						{
							LinkedListRemove (trial_p -> ft_studies_p, & (node_p -> stn_node));

							node_p -> stn_study_p = NULL;
							FreeStudyNode (& (node_p -> stn_node));
							removed_flag = true;
						}


					if (!removed_flag)
						{
							node_p = (StudyNode *) (node_p -> stn_node.ln_next_p);
						}
				}
		}


	return removed_flag;
}


LinkedList *GetFieldTrialsByName (const char * const trial_s, const FieldTrialServiceData *data_p)
{
	LinkedList *trials_p = GetFieldTrialsByNameFromMongoDB (data_p, trial_s);

	return trials_p;
}



json_t *GetFieldTrialAsJSON (FieldTrial *trial_p, const ViewFormat format, const FieldTrialServiceData *data_p)
{
	json_t *trial_json_p = json_object ();

	if (trial_json_p)
		{
			if (SetNonTrivialString (trial_json_p, FT_NAME_S, trial_p -> ft_name_s))
				{
					if (SetNonTrivialString (trial_json_p, FT_TEAM_S, trial_p -> ft_team_s))
						{
							if (AddCompoundIdToJSON (trial_json_p, trial_p -> ft_id_p))
								{
									if (AddStudiesToFieldTrialJSON (trial_p, trial_json_p, format, data_p))
										{
											if (AddDatatype (trial_json_p, DFTD_FIELD_TRIAL))
												{
													return trial_json_p;
												}
										}
								}

						}		/* if (json_object_set_new (trial_json_p, team_key_s, json_string (trial_p -> ft_team_s)) == 0) */

				}		/* if (json_object_set_new (trial_json_p, name_key_s, json_string (trial_p -> ft_name_s)) == 0) */

			json_decref (trial_json_p);
		}		/* if (trial_json_p) */

	return NULL;
}



bool AddStudiesToFieldTrialJSON (FieldTrial *trial_p, json_t *trial_json_p, const ViewFormat format, const FieldTrialServiceData *data_p)
{
	bool success_flag = true;

	if (trial_p -> ft_studies_p -> ll_size > 0)
		{
			json_t *studies_p = json_array ();

			if (studies_p)
				{
					StudyNode *node_p = (StudyNode *) (trial_p -> ft_studies_p -> ll_head_p);
					bool ok_flag = true;
					JSONProcessor *processor_p = NULL;

					while (node_p && ok_flag)
						{
							json_t *study_p = GetStudyAsJSON (node_p -> stn_study_p, format, processor_p, data_p);

							if (study_p)
								{
									if (json_array_append_new (studies_p, study_p) == 0)
										{
											node_p = (StudyNode *) (node_p -> stn_node.ln_next_p);
										}
									else
										{
											ok_flag = false;
											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, study_p, "Failed to add Study json to array for field trial \"%s\" - \"%s\"", trial_p -> ft_team_s, trial_p -> ft_name_s);
										}
								}
							else
								{
									char buffer_s [MONGO_OID_STRING_BUFFER_SIZE];

									ok_flag = false;
									bson_oid_to_string (node_p -> stn_study_p -> st_id_p, buffer_s);

									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, study_p, "Failed to get Study json for area \"%s\"", buffer_s);
								}
						}		/* while (node_p && success_flag) */

					if (ok_flag)
						{
							if (json_object_set_new (trial_json_p, FT_STUDIES_S, studies_p) == 0)
								{
									success_flag = true;
								}
							else
								{
									char buffer_s [MONGO_OID_STRING_BUFFER_SIZE];

									ok_flag = false;

									json_decref (studies_p);

									bson_oid_to_string (trial_p -> ft_id_p, buffer_s);

									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, studies_p, "Failed to add Study json to trial \"%s\"", buffer_s);
								}
						}

				}		/* if (exp_areas_p) */
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate Studies json object for field trial \"%s\" - \"%s\"", trial_p -> ft_team_s, trial_p -> ft_name_s);
				}

		}		/* if (trial_p -> ft_studies_p -> ll_size > 0) */
	else
		{
			/* nothing to add */
			success_flag = true;
		}

	return success_flag;
}


FieldTrial *GetFieldTrialFromJSON (const json_t *json_p, const FieldTrialServiceData * UNUSED_PARAM (data_p))
{
	const char *name_s = GetJSONString (json_p, FT_NAME_S);

	if (name_s)
		{
			const char *team_s = GetJSONString (json_p, FT_TEAM_S);
			bson_oid_t *id_p = GetNewUnitialisedBSONOid ();

			if (id_p)
				{
					if (GetMongoIdFromJSON (json_p, id_p))
						{
							FieldTrial *trial_p = AllocateFieldTrial (name_s, team_s, id_p);

							return trial_p;
						}

					FreeBSONOid (id_p);
				}
		}

	return NULL;
}


FieldTrialNode *AllocateFieldTrialNode (FieldTrial *trial_p)
{
	FieldTrialNode *node_p = (FieldTrialNode *) AllocMemory (sizeof (FieldTrialNode));

	if (node_p)
		{
			InitListItem (& (node_p -> ftn_node));
			node_p -> ftn_field_trial_p = trial_p;
		}

	return node_p;
}


void FreeFieldTrialNode (ListItem *node_p)
{
	FieldTrialNode *field_trial_node_p = (FieldTrialNode *) node_p;

	if (field_trial_node_p -> ftn_field_trial_p)
		{
			FreeFieldTrial (field_trial_node_p -> ftn_field_trial_p);
		}

	FreeMemory (field_trial_node_p);
}


/*
 * The trial could be the bson_oid or a name so check
 */

FieldTrial *GetUniqueFieldTrialBySearchString (const char *trial_s, const ViewFormat format, const FieldTrialServiceData *data_p)
{
	FieldTrial *trial_p = NULL;

	if (bson_oid_is_valid (trial_s, strlen (trial_s)))
		{
			trial_p = GetFieldTrialByIdString (trial_s, format, data_p);
		}

	if (!trial_p)
		{
			LinkedList *trials_p = GetFieldTrialsByName (trial_s, data_p);

			if (trials_p)
				{
					if (trials_p -> ll_size == 1)
						{
							FieldTrialNode *node_p = (FieldTrialNode *) (trials_p -> ll_head_p);

							/* Remove the trial from the node */
							trial_p = node_p -> ftn_field_trial_p;
							node_p -> ftn_field_trial_p = NULL;
						}

					FreeLinkedList (trials_p);
				}
		}


	return trial_p;
}


FieldTrial *GetFieldTrialById (const bson_oid_t *id_p, const ViewFormat format, const FieldTrialServiceData *data_p)
{
	FieldTrial *trial_p = NULL;
	char *id_s = GetBSONOidAsString (id_p);

	if (id_s)
		{
			trial_p = GetFieldTrialByIdString (id_s, format, data_p);

			FreeCopiedString (id_s);
		}

	return trial_p;
}



FieldTrial *GetFieldTrialByIdString (const char *field_trial_id_s, const ViewFormat format, const FieldTrialServiceData *data_p)
{
	FieldTrial *trial_p = NULL;
	MongoTool *tool_p = data_p -> dftsd_mongo_p;

	if (bson_oid_is_valid (field_trial_id_s, strlen (field_trial_id_s)))
		{
			if (SetMongoToolCollection (tool_p, data_p -> dftsd_collection_ss [DFTD_FIELD_TRIAL]))
				{
					bson_t *query_p = bson_new ();

					if (query_p)
						{
							bson_oid_t oid;

							bson_oid_init_from_string (&oid, field_trial_id_s);

							if (BSON_APPEND_OID (query_p, MONGO_ID_S, &oid))
								{
									json_t *results_p = GetAllMongoResultsAsJSON (tool_p, query_p, NULL);

									if (results_p)
										{
											if (json_is_array (results_p))
												{
													size_t num_results = json_array_size (results_p);

													if (num_results == 1)
														{
															json_t *res_p = json_array_get (results_p, 0);

															trial_p = GetFieldTrialFromJSON (res_p, data_p);

															if (!trial_p)
																{
																	PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, res_p, "failed to create field trial for id \"%s\"", field_trial_id_s);
																}

														}		/* if (num_results == 1) */
													else
														{
															PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, results_p, "" SIZET_FMT " results when searching for field trial with id \"%s\"", num_results, field_trial_id_s);
														}

												}		/* if (json_is_array (results_p) */
											else
												{
													PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, results_p, "Results are not an array");
												}

											json_decref (results_p);
										}		/* if (results_p) */
									else
										{
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get results searching for field trial with id \"%s\"", field_trial_id_s);
										}

								}		/* if (BSON_APPEND_OID (query_p, MONGO_ID_S, &oid)) */
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to create query for field trial with id \"%s\"", field_trial_id_s);
								}

							bson_destroy (query_p);
						}		/* if (query_p) */
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to create query for field trial with id \"%s\"", field_trial_id_s);
						}

				}		/* if (SetMongoToolCollection (tool_p, data_p -> dftsd_collection_ss [DFTD_FIELD_TRIAL])) */
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to set collection to \"%s\"", data_p -> dftsd_collection_ss [DFTD_FIELD_TRIAL]);
				}

		}		/* if (bson_oid_is_valid (field_trial_id_s, strlen (field_trial_id_s))) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "\"%s\" is not a valid oid", field_trial_id_s);
		}

	return trial_p;
}




bool AddFieldTrialStudy (FieldTrial *trial_p, Study *study_p, MEM_FLAG mf)
{
	bool success_flag = false;
	StudyNode *node_p = AllocateStudyNode (study_p);

	if (node_p)
		{
			study_p -> st_parent_p = trial_p;
			study_p -> st_parent_field_trial_mem = mf;

			LinkedListAddTail (trial_p -> ft_studies_p, & (node_p -> stn_node));
			success_flag  = true;
		}

	return success_flag;
}



bool GetAllFieldTrialStudies (FieldTrial *trial_p, const ViewFormat format, const FieldTrialServiceData *data_p)
{
	bool success_flag = false;

	bson_t *query_p = bson_new ();

	if (query_p)
		{
			if (BSON_APPEND_OID (query_p, ST_PARENT_FIELD_TRIAL_S, trial_p -> ft_id_p))
				{
					if (SetMongoToolCollection (data_p -> dftsd_mongo_p, data_p -> dftsd_collection_ss [DFTD_STUDY]))
						{
							bson_t *opts_p =  BCON_NEW ( "sort", "{", ST_SOWING_DATE_S, BCON_INT32 (1), "}");

							if (opts_p)
								{
									json_t *results_p = GetAllMongoResultsAsJSON (data_p -> dftsd_mongo_p, query_p, opts_p);

									if (results_p)
										{
											if (json_is_array (results_p))
												{
													const size_t num_results = json_array_size (results_p);

													success_flag = true;

													if (num_results > 0)
														{
															size_t i;
															json_t *study_json_p;

															json_array_foreach (results_p, i, study_json_p)
																{
																	Study *study_p = GetStudyFromJSON (study_json_p, format, data_p);

																	if (study_p)
																		{
																			if (!AddFieldTrialStudy (trial_p, study_p, MF_SHADOW_USE))
																				{
																					FreeStudy (study_p);
																					success_flag = false;
																				}
																		}
																	else
																		{
																			success_flag = false;
																		}
																}		/* json_array_foreach (results_p, i, study_json_p) */

														}
												}

											json_decref (results_p);
										}		/* if (results_p) */

									bson_destroy (opts_p);
								}		/* if (opts_p) */

						}		/* if (SetMongoToolCollection (data_p -> dftsd_mongo_p, data_p -> dftsd_collection_ss [DFTD_EXPERIMENTAL_AREA])) */

				}		/* if (BSON_APPEND_OID (query_p, ST_PARENT_FIELD_TRIAL_S, trial_p -> ft_id_p)) */

			bson_destroy (query_p);
		}		/* if (query_p) */


	return success_flag;
}



char *GetFieldTrialAsString (const FieldTrial *trial_p)
{
	char *trial_s = ConcatenateVarargsStrings (trial_p -> ft_team_s, " - ", trial_p -> ft_name_s, NULL);

	return trial_s;
}


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
 * field_trial_mongodb.c
 *
 *  Created on: 11 Sep 2018
 *      Author: billy
 */

#include "field_trial_mongodb.h"


static LinkedList *GetMatchingFieldTrialsFromMongoDB (DFWFieldTrialServiceData *data_p, const char **keys_ss, const char **values_ss);




LinkedList *GetFieldTrialsByNameFromMongoDB (DFWFieldTrialServiceData *data_p, const char *name_s)
{
	const char *keys_ss [] = { FT_NAME_S, NULL };
	const char *values_ss [] = { name_s, NULL };

	return GetMatchingFieldTrialsFromMongoDB (data_p, keys_ss, values_ss);
}


LinkedList *GetFieldTrialsByTeamFromMongoDB (DFWFieldTrialServiceData *data_p, const char *team_s)
{
	const char *keys_ss [] = { FT_TEAM_S, NULL };
	const char *values_ss [] = { team_s, NULL };

	return GetMatchingFieldTrialsFromMongoDB (data_p, keys_ss, values_ss);
}


FieldTrial *GetFieldTrialFromMongoDB (DFWFieldTrialServiceData *data_p, const char *name_s, const char *team_s)
{
	FieldTrial *trial_p = NULL;
	const char *keys_ss [] = { FT_NAME_S, FT_TEAM_S, NULL };
	const char *values_ss [] = { name_s, team_s, NULL };
	LinkedList *results_p = GetMatchingFieldTrialsFromMongoDB (data_p, keys_ss, values_ss);

	if (results_p)
		{
			if (results_p -> ll_size == 1)
				{
					FieldTrialNode *node_p = (FieldTrialNode *) LinkedListRemHead (results_p);

					trial_p = node_p -> ftn_field_trial_p;
					node_p -> ftn_field_trial_p = NULL;

					FreeFieldTrialNode (& (node_p -> ftn_node));
				}

			FreeLinkedList (results_p);
		}

	return trial_p;
}



bool AddFieldTrialByNameToMongoDB (DFWFieldTrialServiceData *data_p, FieldTrial *trial_p)
{
	bool success_flag = false;

	switch (data_p -> dftsd_backend)
		{
			case DB_MONGO_DB:
				{
					json_t *values_p = GetFieldTrialAsJSON (trial_p);

					if (values_p)
						{
							if (SetMongoToolCollection (data_p -> dftsd_mongo_p, data_p -> dftsd_database_s, data_p -> dftsd_collection_ss [DFTD_FIELD_TRIAL]))
								{
									const char *keys_ss [2] = { FT_NAME_S, FT_TEAM_S };
									const size_t num_keys = 2;
									const char *error_s = NULL;

									if (IsIdSet (& (trial_p -> ft_id), data_p))
										{
											json_t *query_p = json_object ();

											if (query_p)
												{
													if (json_object_set_new (query_p, "_id", json_string (trial_p -> ft_id.di_id_s)) == 0)
														{

															if (UpdateMongoDocumentByJSON (data_p -> dftsd_mongo_p, query_p, values_p))
																{
																	success_flag = true;
																}
															else
																{
																	error_s = "Failed to update doc";
																}

														}
													else
														{
															error_s = "Failed to set _id";
														}

													json_decref (query_p);
												}
											else
												{
													error_s = "failed to create query";
												}
										}
									else
										{
											bson_oid_t *id_p = InsertJSONIntoMongoCollection (data_p -> dftsd_mongo_p, values_p);

											if (id_p)
												{
													char buffer_s [25];

													bson_oid_to_string (id_p, buffer_s);

													if (!SetIdString (& (trial_p -> ft_id), buffer_s))
														{
															error_s = "Failed to set id";
														}

													FreeMemory (id_p);
												}
											else
												{
													error_s = "Failed to insert into mongodb";
												}
										}


									if (error_s)
										{
											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, values_p, "Failed to write data, error \"%s\"", error_s);
										}		/* if (error_s) */
									else
										{
											success_flag = true;
										}

								}		/* if (SetMongoToolCollection (data_p -> dftsd_mongo_p, data_p -> dftsd_database_s, data_p -> dftsd_collection_ss [DFTD_FIELD])) */


							json_decref (values_p);
						}		/* if (values_p) */

				}		/* case DB_MONGO_DB: */
				break;

			default:
				break;
		}

	return success_flag;
}



static LinkedList *GetMatchingFieldTrialsFromMongoDB (DFWFieldTrialServiceData *data_p, const char **keys_ss, const char **values_ss)
{
	LinkedList *field_trials_list_p = AllocateLinkedList (FreeFieldTrialNode);

	if (field_trials_list_p)
		{
			switch (data_p -> dftsd_backend)
				{
					case DB_MONGO_DB:
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
											json_t *results_p = GetAllMongoResultsAsJSON (data_p -> dftsd_mongo_p, query_p);

											if (results_p)
												{
													const size_t size = json_array_size (results_p);
													size_t i = 0;

													for (i = 0; i < size; ++ i)
														{
															json_t *result_p = json_array_get (results_p, i);
															FieldTrial *trial_p = GetFieldTrialFromJSON (result_p, data_p);

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

										}		/* if (success_flag) */
									else
										{
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add query");
										}

									bson_destroy (query_p);
								}		/* if (query_p) */

						}		/* case DB_MONGO_DB: */
						break;

					default:
						break;
				}

		}		/* if (field_trials_list_p) */

	return field_trials_list_p;
}


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
#include "experimental_area.h"
#include "memory_allocations.h"
#include "streams.h"


FieldTrial *AllocateFieldTrial (const char *name_s, const char *team_s, bson_oid_t *id_p)
{
	char *copied_name_s = EasyCopyToNewString (name_s);

	if (copied_name_s)
		{
			char *copied_team_s = EasyCopyToNewString (team_s);

			if (copied_team_s)
				{
					LinkedList *areas_p = AllocateLinkedList (FreeExperimentalAreaNode);

					if (areas_p)
						{
							FieldTrial *trial_p = (FieldTrial *) AllocMemory (sizeof (FieldTrial));

							if (trial_p)
								{
									trial_p -> ft_name_s = copied_name_s;
									trial_p -> ft_team_s = copied_team_s;
									trial_p -> ft_id_p = id_p;
									trial_p -> ft_experimental_areas_p = areas_p;

									return trial_p;
								}

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
			bson_oid_t *id_p = AllocMemory (sizeof (bson_oid_t));

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


bool SaveFieldTrial (FieldTrial *trial_p, DFWFieldTrialServiceData *data_p)
{
	bool success_flag = false;
	bool insert_flag = false;
	json_t *field_trial_json_p = NULL;

	if (! (trial_p -> ft_id_p))
		{
			trial_p -> ft_id_p  = GetNewId ();

			if (trial_p -> ft_id_p)
				{
					insert_flag = true;
				}
		}

	if (trial_p -> ft_id_p)
		{
			field_trial_json_p = GetFieldTrialAsJSON (trial_p, false, data_p);

			if (field_trial_json_p)
				{
					success_flag = SaveMongoData (data_p -> dftsd_mongo_p, field_trial_json_p, data_p -> dftsd_collection_ss [DFTD_FIELD_TRIAL], insert_flag);

					json_decref (field_trial_json_p);
				}		/* if (field_trial_json_p) */

		}		/* if (trial_p -> ft_id_p) */

	return success_flag;
}



void FreeFieldTrial (FieldTrial *trial_p)
{
	FreeCopiedString (trial_p -> ft_name_s);
	FreeCopiedString (trial_p -> ft_team_s);

	if (trial_p -> ft_id_p)
		{
			FreeMemory (trial_p -> ft_id_p);
		}

	if (trial_p -> ft_experimental_areas_p)
		{
			FreeLinkedList (trial_p -> ft_experimental_areas_p);
		}

	FreeMemory (trial_p);
}


LinkedList *GetFieldTrialsByName (DFWFieldTrialServiceData *data_p, const char *name_s)
{
	LinkedList *trials_p = NULL;

	switch (data_p -> dftsd_backend)
		{
			case DB_MONGO_DB:
				trials_p = GetFieldTrialsByNameFromMongoDB (data_p, name_s);
				break;

			default:
				break;
		}

	return trials_p;
}



json_t *GetFieldTrialAsJSON (FieldTrial *trial_p, const bool get_experimental_areas_flag, DFWFieldTrialServiceData *data_p)
{
	json_t *trial_json_p = json_object ();

	if (trial_json_p)
		{
			if (json_object_set_new (trial_json_p, FT_NAME_S, json_string (trial_p -> ft_name_s)) == 0)
				{
					if (json_object_set_new (trial_json_p, FT_TEAM_S, json_string (trial_p -> ft_team_s)) == 0)
						{
							if (AddCompoundIdToJSON (trial_json_p, trial_p -> ft_id_p))
								{
									bool success_flag = true;

									if (get_experimental_areas_flag)
										{

											if (!GetAllFieldTrialExperimentalAreas (trial_p, data_p))
												{
													success_flag = false;
												}
										}

									if (success_flag)
										{
											return trial_json_p;
										}
								}

						}		/* if (json_object_set_new (trial_json_p, team_key_s, json_string (trial_p -> ft_team_s)) == 0) */

				}		/* if (json_object_set_new (trial_json_p, name_key_s, json_string (trial_p -> ft_name_s)) == 0) */

			json_decref (trial_json_p);
		}		/* if (trial_json_p) */

	return NULL;
}


FieldTrial *GetFieldTrialFromJSON (const json_t *json_p, const DFWFieldTrialServiceData *data_p)
{
	const char *name_s = GetJSONString (json_p, FT_NAME_S);

	if (name_s)
		{
			const char *team_s = GetJSONString (json_p, FT_TEAM_S);

			if (team_s)
				{
					bson_oid_t *id_p = AllocMemory (sizeof (bson_oid_t));

					if (id_p)
						{
							if (GetCompoundIdFromJSON (json_p, id_p))
								{
									FieldTrial *trial_p = AllocateFieldTrial (name_s, team_s, id_p);

									return trial_p;
								}

							FreeMemory (id_p);
						}
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


FieldTrial *GetFieldTrialByIdString (const char *field_trial_id_s, DFWFieldTrialServiceData *data_p)
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
															PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, results_p, "" SIZET_FMT " results when searching for field trial with id \"%s\"", field_trial_id_s);
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




bool AddFieldTrialExperimentalArea (FieldTrial *trial_p, ExperimentalArea *area_p, DFWFieldTrialServiceData *data_p)
{
	bool success_flag = false;
	bson_oid_t *area_id_p = area_p -> ea_id_p;



	return success_flag;
}


bool GetAllFieldTrialExperimentalAreaIds (FieldTrial *trial_p, DFWFieldTrialServiceData *data_p)
{
	bool success_flag = false;

	return success_flag;
}


bool GetAllFieldTrialExperimentalAreas (FieldTrial *trial_p, DFWFieldTrialServiceData *data_p)
{
	bool success_flag = false;
	char *id_s = GetBSONOidAsString (trial_p -> ft_id_p);

	if (id_s)
		{
			if (SetMongoToolCollection (data_p -> dftsd_mongo_p, data_p -> dftsd_collection_ss [DFTD_EXPERIMENTAL_AREA]))
				{
					bson_t *query_p = BCON_NEW (EA_PARENT_FIELD_TRIAL_S, id_s);
					bson_t *opts_p =  BCON_NEW ( "sort", "{", EA_SOWING_YEAR_S, BCON_INT32 (1), "}");
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
											json_t *area_json_p;

											json_array_foreach (results_p, i, area_json_p)
												{
													ExperimentalArea *area_p = GetExperimentalAreaFromJSON (area_json_p, data_p);

													if (area_p)
														{
															ExperimentalAreaNode *node_p = AllocateExperimentalAreaNode (area_p);

															if (node_p)
																{
																	LinkedListAddTail (trial_p -> ft_experimental_areas_p, & (node_p -> ean_node));
																}
															else
																{
																	success_flag = false;
																}
														}
													else
														{
															success_flag = false;
														}
												}

										}
								}
						}
				}

			FreeMemory (id_s);
		}		/* if (id_s) */


	return success_flag;
}



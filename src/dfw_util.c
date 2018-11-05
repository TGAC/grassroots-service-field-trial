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
 * dfw_util.c
 *
 *  Created on: 8 Oct 2018
 *      Author: billy
 */

#include "dfw_util.h"
#include "streams.h"
#include "time_util.h"
#include "string_utils.h"
#include "schema_keys.h"



#ifdef _DEBUG
	#define DFW_UTIL_DEBUG	(STM_LEVEL_INFO)
#else
	#define DFW_UTIL_DEBUG	(STM_LEVEL_NONE)
#endif




void *GetDFWObjectById (const bson_oid_t *id_p, DFWFieldTrialData collection_type, void *(*get_obj_from_json_fn) (const json_t *json_p, const ViewFormat format, const DFWFieldTrialServiceData *data_p), const ViewFormat format, const DFWFieldTrialServiceData *data_p)
{
	void *result_p = NULL;
	MongoTool *tool_p = data_p -> dftsd_mongo_p;

	if (SetMongoToolCollection (tool_p, data_p -> dftsd_collection_ss [collection_type]))
		{
			bson_t *query_p = bson_new ();
			char id_s [MONGO_OID_STRING_BUFFER_SIZE];

			bson_oid_to_string (id_p, id_s);

			if (query_p)
				{
					if (BSON_APPEND_OID (query_p, MONGO_ID_S, id_p))
						{
							json_t *results_p = NULL;

							#if DFW_UTIL_DEBUG >= STM_LEVEL_FINER
								{
									PrintBSONToLog (STM_LEVEL_FINER, __FILE__, __LINE__, query_p, "GetDFWObjectById query ");
								}
							#endif

							results_p = GetAllMongoResultsAsJSON (tool_p, query_p, NULL);

							if (results_p)
								{
									if (json_is_array (results_p))
										{
											size_t num_results = json_array_size (results_p);

											if (num_results == 1)
												{
													json_t *res_p = json_array_get (results_p, 0);

													result_p = get_obj_from_json_fn (res_p, format, data_p);

													if (!result_p)
														{
															PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, res_p, "failed to create object for id \"%s\"", id_s);
														}

												}		/* if (num_results == 1) */
											else
												{
													PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, results_p, "" SIZET_FMT " results when searching for object_id_s with id \"%s\"", num_results, id_s);
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
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get results searching for object_id_s with id \"%s\"", id_s);
								}

						}		/* if (BSON_APPEND_OID (query_p, MONGO_ID_S, id_p)) */
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to create query for object_id_s with id \"%s\"", id_s);
						}

					bson_destroy (query_p);
				}		/* if (query_p) */
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to create query for object_id_s with id \"%s\"", id_s);
				}

		}		/* if (SetMongoToolCollection (tool_p, data_p -> dftsd_collection_ss [collection_type])) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to set collection to \"%s\"", data_p -> dftsd_collection_ss [collection_type]);
		}

	return result_p;

}


void *GetDFWObjectByIdString (const char *object_id_s, DFWFieldTrialData collection_type, void *(*get_obj_from_json_fn) (const json_t *json_p, const ViewFormat format, const DFWFieldTrialServiceData *data_p), const ViewFormat format, const DFWFieldTrialServiceData *data_p)
{
	void *result_p = NULL;

	if (bson_oid_is_valid (object_id_s, strlen (object_id_s)))
		{
			bson_oid_t oid;

			bson_oid_init_from_string (&oid, object_id_s);

			result_p = GetDFWObjectById (&oid, collection_type, get_obj_from_json_fn, format, data_p);
		}		/* if (bson_oid_is_valid (field_trial_id_s, strlen (object_id_s))) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "\"%s\" is not a valid oid", object_id_s);
		}

	return result_p;
}


bool CopyValidDate (const struct tm *src_p, struct tm **dest_pp)
{
	bool success_flag = true;

	if (src_p)
		{
			struct tm *dest_p = DuplicateTime (src_p);

			if (dest_p)
				{
					*dest_pp = dest_p;
					success_flag = true;
				}
			else
				{
					success_flag = false;
				}
		}

	return success_flag;
}



bool AddValidDateToJSON (struct tm *time_p, json_t *json_p, const char *key_s)
{
	bool success_flag = false;

	if (time_p)
		{
			char *time_s = GetTimeAsString (time_p, false);

			if (time_s)
				{
					if (SetJSONString (json_p, key_s, time_s))
						{
							success_flag = true;
						}

					FreeCopiedString (time_s);
				}
		}
	else
		{
			success_flag = true;
		}

	return success_flag;
}


bool CreateValidDateFromJSON (const json_t *json_p, const char *key_s, struct tm **time_pp)
{
	bool success_flag = false;
	const char *time_s = GetJSONString (json_p, key_s);

	if (time_s)
		{
			struct tm *time_p = GetTimeFromString (time_s);

			if (time_p)
				{
					*time_pp = time_p;
					success_flag = true;
				}
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to convert \"%s\" to a time", time_s);
				}

		}		/* if (time_s) */
	else
		{
			success_flag = true;
		}

	return success_flag;
}



bool PrepareSaveData (bson_oid_t **id_pp, bson_t **selector_pp)
{
	bool success_flag = false;

	if (*id_pp)
		{
			*selector_pp = BCON_NEW (MONGO_ID_S, BCON_OID (*id_pp));

			if (*selector_pp)
				{
					success_flag = true;
				}
		}
	else
		{
			if ((*id_pp = GetNewBSONOid ()) != NULL)
				{
					success_flag = true;
				}
		}

	return success_flag;
}


bool AddContext (json_t *data_p)
{
	bool success_flag = false;
	json_t *context_p = json_object ();

	if (context_p)
		{
			if (json_object_set_new (data_p, "@context", context_p) == 0)
				{
					if (SetJSONString (context_p, CONTEXT_PREFIX_SCHEMA_ORG_S, CONTEXT_URL_SCHEMA_ORG_S))
						{
							if (SetJSONString (context_p, CONTEXT_PREFIX_CROP_ONTOLOGY_S, CONTEXT_URL_CROP_ONTOLOGY_S))
								{
									success_flag = true;
								}
						}

				}		/* if (json_object_set_new (data_p, "@context", context_p) == 0) */
			else
				{
					json_decref (context_p);
				}

		}		/* if (context_p) */

	return success_flag;
}


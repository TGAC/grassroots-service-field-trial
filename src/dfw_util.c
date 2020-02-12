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
	#define DFW_UTIL_DEBUG	(STM_LEVEL_FINE)
#else
	#define DFW_UTIL_DEBUG	(STM_LEVEL_NONE)
#endif


static char *GetCacheFilename (const char *id_s, const DFWFieldTrialServiceData *data_p);



bool FindAndAddResultToServiceJob (const char *id_s, const ViewFormat format, ServiceJob *job_p, JSONProcessor *processor_p,
																	 json_t *(get_json_fn) (const char *id_s, const ViewFormat format, JSONProcessor *processor_p, char **name_ss, const DFWFieldTrialServiceData *data_p),
																	 const DFWFieldTrialServiceData *data_p)
{
	OperationStatus status = OS_FAILED;
	char *name_s = NULL;
	json_t *json_p = get_json_fn (id_s, format, processor_p, &name_s, data_p);

	if (json_p)
		{
			json_t *dest_record_p = GetResourceAsJSONByParts (PROTOCOL_INLINE_S, NULL, name_s, json_p);

			if (dest_record_p)
				{
					if (AddResultToServiceJob (job_p, dest_record_p))
						{
							status = OS_SUCCEEDED;
						}
					else
						{
							json_decref (dest_record_p);
						}

				}		/* if (dest_record_p) */

			json_decref (json_p);
		}		/* if (json_p) */

	if (name_s)
		{
			FreeCopiedString (name_s);
		}

	SetServiceJobStatus (job_p, status);
	return (status == OS_SUCCEEDED);
}



bool CacheStudy (const char *id_s, const json_t *study_json_p, const DFWFieldTrialServiceData *data_p)
{
	bool success_flag = false;

	/*
	 * Is the Study cached?
	 */
	if (data_p -> dftsd_study_cache_path_s)
		{
			char *filename_s = GetCacheFilename (id_s, data_p);

			if (filename_s)
				{
					int res = json_dump_file (study_json_p, filename_s, 0);

					if (res != 0)
						{
							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, study_json_p, "Failed to save cached study to \"%s\", error \"%s\" at [%d, %d]", filename_s);
						}

					FreeCopiedString (filename_s);
				}		/* if (filename_s) */

		}		/* if (data_p -> dftsd_study_cache_path_s) */
	else
		{
			/* No cache path configured*/
			success_flag = true;
		}

	return success_flag;
}


json_t *GetCachedStudy (const char *id_s, const DFWFieldTrialServiceData *data_p)
{
	json_t *study_json_p = NULL;

	/*
	 * Is the Study cached?
	 */
	if (data_p -> dftsd_study_cache_path_s)
		{
			char *filename_s = NULL;

			#if DFW_UTIL_DEBUG >= STM_LEVEL_FINE
			PrintLog (STM_LEVEL_FINE, __FILE__, __LINE__, "Checking for cached study \"%s\" in \"%s\"", id_s, data_p -> dftsd_study_cache_path_s);
			#endif

			filename_s = GetCacheFilename (id_s, data_p);

			if (filename_s)
				{
					if (IsPathValid (filename_s))
						{
							json_error_t err;

							#if DFW_UTIL_DEBUG >= STM_LEVEL_FINE
							PrintLog (STM_LEVEL_FINE, __FILE__, __LINE__, "Loading cached study \"%s\" in \"%s\"", id_s, data_p -> dftsd_study_cache_path_s);
							#endif

							study_json_p = json_load_file (filename_s, 0, &err);

							if (!study_json_p)
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to load cached study from \"%s\", error \"%s\" at [%d, %d]", filename_s, err.text, err.line, err.column);
								}
						}
					else
						{
							#if DFW_UTIL_DEBUG >= STM_LEVEL_FINE
							PrintLog (STM_LEVEL_FINE, __FILE__, __LINE__, "No cached study \"%s\" in \"%s\"", id_s, data_p -> dftsd_study_cache_path_s);
							#endif
						}

					FreeCopiedString (filename_s);
				}		/* if (filename_s) */
			else
				{
					PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "GetCacheFilename failed for \"%s\"", id_s);
				}

		}		/* if (data_p -> dftsd_study_cache_path_s) */
	else
		{
			#if DFW_UTIL_DEBUG >= STM_LEVEL_FINE
			PrintLog (STM_LEVEL_FINE, __FILE__, __LINE__, "No cache path configured");
			#endif
		}

	return study_json_p;
}


bool ClearCachedStudy (const char *id_s, const DFWFieldTrialServiceData *data_p)
{
	bool success_flag = true;
	char *filename_s = GetCacheFilename (id_s, data_p);

	if (filename_s)
		{
			if (IsPathValid (filename_s))
				{
					if (!RemoveFile (filename_s))
						{
							success_flag = false;
						}
				}
		}		/* if (filename_s) */

	return success_flag;
}


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
				}
			else
				{
					success_flag = false;
				}
		}
	else
		{
			*dest_pp = NULL;
		}

	return success_flag;
}


bool CopyValidReal (const double64 *src_p, double64 **dest_pp)
{
	bool success_flag = true;

	if (src_p)
		{
			double64 *dest_p = (double64 *) AllocMemory (sizeof (double64));

			if (dest_p)
				{
					*dest_pp = dest_p;
				}
			else
				{
					success_flag = false;
				}
		}
	else
		{
			*dest_pp = NULL;
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


bool AddValidDateAsEpochToJSON (struct tm *time_p, json_t *json_p, const char *key_s)
{
	bool success_flag = false;

	if (time_p)
		{
			time_t t = mktime (time_p);

			if (t != -1)
				{
					if (SetJSONInteger (json_p, key_s, t))
						{
							success_flag = true;
						}
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
	struct tm *time_p = NULL;

	/*
	 * The date could either be stored in ISO-8601 format or as an epoch value
	 */
	const char *time_s = GetJSONString (json_p, key_s);

	if (time_s)
		{
			time_p = GetTimeFromString (time_s);

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
			int i;

			if (GetJSONInteger (json_p, key_s, &i))
				{
					time_t t = (time_t) i;
					struct tm *src_p = gmtime (&t);

					if (src_p)
						{
							time_p = DuplicateTime (src_p);

							if (time_p)
								{
									*time_pp = time_p;
									success_flag = true;
								}
						}
				}
			else
				{
					success_flag = true;
				}
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


bool AddDatatype (json_t *doc_p, const DFWFieldTrialData data_type)
{
	bool success_flag = false;
	const char *type_s = GetDatatypeAsString (data_type);

	if (type_s)
		{
			if (SetJSONString (doc_p, "@type", type_s))
				{
					success_flag = true;
				}
		}

	return success_flag;
}



bool AddImage (json_t *doc_p, const DFWFieldTrialData data_type, const DFWFieldTrialServiceData *data_p)
{
	bool success_flag = false;
	const char *type_s = GetDatatypeAsString (data_type);

	if (type_s)
		{
			const char *image_s = GetImageForDatatype (data_p, type_s);

			if (image_s)
				{
					if (SetJSONString (doc_p, CONTEXT_PREFIX_SCHEMA_ORG_S "image", image_s))
						{
							success_flag = true;
						}
				}
		}

	return success_flag;
}


LinkedList *SearchObjects (const DFWFieldTrialServiceData *data_p, const DFWFieldTrialData collection_type, const char **keys_ss, const char **values_ss, void (*free_list_item_fn) (ListItem * const item_p), bool (*add_result_to_list_fn) (const json_t *result_p, LinkedList *list_p, const DFWFieldTrialServiceData *service_data_p))
{
	LinkedList *results_list_p = AllocateLinkedList (free_list_item_fn);

	if (results_list_p)
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
							if (SetMongoToolCollection (data_p -> dftsd_mongo_p, data_p -> dftsd_collection_ss [collection_type]))
								{
									json_t *results_p = GetAllMongoResultsAsJSON (data_p -> dftsd_mongo_p, query_p, NULL);

									if (results_p)
										{
											const size_t size = json_array_size (results_p);
											size_t i = 0;

											for (i = 0; i < size; ++ i)
												{
													json_t *result_p = json_array_get (results_p, i);

													if (!add_result_to_list_fn (result_p, results_list_p, data_p))
														{
															PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, result_p, "Failed to add result to list");
														}

												}		/* for (i = 0; i < size; ++ i) */

											json_decref (results_p);
										}		/* if (results_p) */


								}		/* if (SetMongoToolCollection (data_p -> dftsd_mongo_p, data_p -> dftsd_collection_ss [collection_type])) */
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to set MongoTool collection to \"%s\"", data_p -> dftsd_collection_ss [collection_type]);
								}

						}		/* if (success_flag) */
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add query");
						}

					bson_destroy (query_p);
				}		/* if (query_p) */


		}		/* if (results_list_p) */

	return results_list_p;
}


static char *GetCacheFilename (const char *id_s, const DFWFieldTrialServiceData *data_p)
{
	char *filename_s = NULL;
	char *local_filename_s = ConcatenateStrings (id_s, ".json");

	if (local_filename_s)
		{
			filename_s = MakeFilename (data_p -> dftsd_study_cache_path_s, local_filename_s);

			if (!filename_s)
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get main cache filename for \"%s\" and \"%s\"", data_p -> dftsd_study_cache_path_s, local_filename_s);
				}		/* if (filename_s) */

			FreeCopiedString (local_filename_s);
		}		/* if (local_filename_s) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get local cache filename for \"%s\"", id_s);
		}

	return filename_s;
}

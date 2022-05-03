/*
 * mongo_scale_class_processor.c
 *
 *  Created on: 31 Mar 2022
 *      Author: billy
 */

#include <stdio.h>

#include "bson/bson.h"
#include "mongoc/mongoc.h"

#include "streams.h"

#include "mongodb_tool.h"

#include "parameter_type.h"
#include "json_util.h"
#include "time_util.h"


typedef enum
{
	VS_FAIL,
	VS_OK,
	VS_NEEDS_UPDATE
} ValueStatus;

static json_t *GetNextDocAsJSON (mongoc_cursor_t *cursor_p);


static ValueStatus SetInteger (json_t *observation_json_p, const char * const key_s);

static ValueStatus SetReal (json_t *observation_json_p, const char * const key_s);

static ValueStatus CheckString (json_t *observation_json_p, const char * const key_s);

static ValueStatus CheckTime (json_t *observation_json_p, const char * const key_s);

static bool WriteObservation (bson_oid_t *observation_id_p, mongoc_collection_t *plots_collection_p, json_t *observation_p);

static bool GetPhenotypeDatatype (const json_t *phenotype_json_p, ParameterType *param_type_p);

static bool WritePlotRows (bson_oid_t *plot_id_p, mongoc_collection_t *plots_collection_p, json_t *rows_p);




int main (void)
{
	mongoc_client_t *client_p;

	mongoc_init ();

	client_p = mongoc_client_new ("mongodb://localhost:27017/?appname=set_datatypes");

	if (client_p)
		{
			mongoc_collection_t *plots_collection_p = mongoc_client_get_collection (client_p, "dfw_field_trial", "Plots");

			if (plots_collection_p)
				{
					mongoc_collection_t *phenotypes_collection_p = mongoc_client_get_collection (client_p, "dfw_field_trial", "Phenotypes");

					if (phenotypes_collection_p)
						{
							bson_t *query_p;

							/*
							 * Iterate over each plot and get all of the rows and
							 * subsequently their observations.
							 *
							 * For each observation, get its raw and corrected values
							 * and check if they can be converted into the correct
							 * datatype as specified by the phenotype's scale class
							 */

							query_p = bson_new ();

							if (query_p)
								{
									mongoc_cursor_t *plots_cursor_p = mongoc_collection_find_with_opts (plots_collection_p, query_p, NULL, NULL);

									if (plots_cursor_p)
										{
											size_t num_succeeded = 0;
											size_t num_failed = 0;
											size_t num_ignored = 0;

											json_t *plot_json_p = NULL;

											while ((plot_json_p = GetNextDocAsJSON (plots_cursor_p)) != NULL)
												{
													json_t *rows_p = json_object_get (plot_json_p, "rows");

													if (rows_p)
														{
															if (json_is_array (rows_p))
																{
																	size_t i;
																	json_t *row_p;
																	bool plot_updated_flag = false;

																	json_array_foreach (rows_p, i, row_p)
																		{
																			json_t *observations_p = json_object_get (row_p, "observations");

																			if (observations_p)
																				{
																					if (json_is_array (observations_p))
																						{
																							size_t j;
																							json_t *observation_p;
																							bson_t phenotype_query;
																							bool first_flag = true;
																							bool error_flag = false;

																							json_array_foreach (observations_p, j, observation_p)
																								{
																									bson_oid_t phenotype_id;

																									if (GetNamedIdFromJSON (observation_p, "phenotype_id", &phenotype_id))
																										{
																											if (first_flag)
																												{
																													bson_init (&phenotype_query);
																													first_flag = false;
																												}
																											else
																												{
																													bson_reinit (&phenotype_query);
																												}

																											if (BSON_APPEND_OID (&phenotype_query, MONGO_ID_S, &phenotype_id))
																												{
																													mongoc_cursor_t *phenotypes_cursor_p = mongoc_collection_find_with_opts (phenotypes_collection_p, &phenotype_query, NULL, NULL);


																													/*
																													size_t query_length = 0;
																													char *query_s = bson_as_json (&phenotype_query, &query_length);

																													if (query_s)
																														{
																															PrintLog (STM_LEVEL_INFO, __FILE__, __LINE__, "phenotype_query: \"%s\"", query_s);

																															bson_free (query_s);
																														}
																													*/

																													if (phenotypes_cursor_p)
																														{
																															json_t *phenotype_json_p = GetNextDocAsJSON (phenotypes_cursor_p);

																															if (phenotype_json_p)
																																{
																																	ParameterType pt;
																																	const char * const RAW_KEY_S = "raw_value";
																																	const char * const CORRECTED_KEY_S = "corrected_value";

																																	if (GetPhenotypeDatatype (phenotype_json_p, &pt))
																																		{
																																			ValueStatus raw_ret = VS_OK;
																																			ValueStatus corrected_ret = VS_OK;

																																			switch (pt)
																																				{
																																					case PT_SIGNED_INT:
																																						{
																																							raw_ret = SetInteger (observation_p, RAW_KEY_S);
																																							corrected_ret = SetInteger (observation_p, CORRECTED_KEY_S);
																																						}
																																						break;

																																					case PT_SIGNED_REAL:
																																						{
																																							raw_ret = SetReal (observation_p, RAW_KEY_S);
																																							corrected_ret = SetReal (observation_p, CORRECTED_KEY_S);
																																						}
																																						break;

																																					case PT_TIME:
																																						{
																																							raw_ret = CheckTime (observation_p, RAW_KEY_S);
																																							corrected_ret = CheckTime (observation_p, CORRECTED_KEY_S);
																																						}
																																						break;

																																					case PT_STRING:
																																						{
																																							raw_ret = CheckString (observation_p, RAW_KEY_S);
																																							corrected_ret = CheckString (observation_p, CORRECTED_KEY_S);
																																						}
																																						break;

																																					default:
																																						break;

																																				}		/* switch (pt) */

																																			if ((raw_ret == VS_FAIL) || (corrected_ret == VS_FAIL))
																																				{
																																					/* An error occurred */
																																					error_flag = true;
																																				}
																																			else if ((raw_ret == VS_NEEDS_UPDATE) || (corrected_ret == VS_NEEDS_UPDATE))
																																				{
																																					/*
																																					 * The observation's values have been updated so we'll need to
																																					 * save it back to the database
																																					 */
																																					plot_updated_flag = true;
																																				}
																																			else if ((raw_ret == VS_OK) || (corrected_ret == VS_OK))
																																				{
																																					++ num_ignored;
																																				}

																																		}		/* if (GetPhenotypeDatatype (phenotype_json_p, &pt)) */
																																	else
																																		{
																																			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, phenotype_json_p, "GetPhenotypeDatatype () failed");
																																			error_flag = true;
																																		}

																																	json_decref (phenotype_json_p);
																																}		/* if (phenotype_json_p) */

																															mongoc_cursor_destroy (phenotypes_cursor_p);
																														}		/* if (phenotypes_cursor_p) */
																													else
																														{
																															char *phenotype_id_s = GetBSONOidAsString (&phenotype_id);

																															if (phenotype_id_s)
																																{
																																	PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to find phenotype with id \"%s\"", phenotype_id_s);
																																	FreeBSONOidString (phenotype_id_s);
																																}
																															else
																																{
																																	PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to find phenotype");
																																}

																															error_flag = true;
																														}

																												}		/* if (BSON_APPEND_OID (&phenotype_query, MONGO_ID_S, &phenotype_id)) */
																											else
																												{
																													char *phenotype_id_s = GetBSONOidAsString (&phenotype_id);

																													if (phenotype_id_s)
																														{
																															PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to append \"%s\": \"%s\" to phenotype query", MONGO_ID_S, phenotype_id_s);
																															FreeBSONOidString (phenotype_id_s);
																														}
																													else
																														{
																															PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to append id to phenotype query");
																														}

																													error_flag = true;
																												}

																										}		/* if (GetNamedIdFromJSON (observation_p, "phenotype_id", &phenotype_id)) */
																									else
																										{
																											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, observation_p, "Failed to get phenotype_id");
																										}

																								}		/* json_array_foreach (observations_p, j, observation_p) */

																							if (error_flag)
																								{
																									++ num_failed;
																								}

																						}		/* if (json_is_array (observations_p)) */

																				}		/* if (observations_p) */

																		}		/* json_array_foreach (rows_p, i, row_p) */

																	/*
																	 * Do we need to update the database?
																	 */
																	if (plot_updated_flag)
																		{
																			bson_oid_t plot_id;

																			if (GetMongoIdFromJSON (plot_json_p, &plot_id))
																				{
																					char *id_s = GetBSONOidAsString (&plot_id);

																					if (id_s)
																						{
																							PrintJSONToLog (STM_LEVEL_INFO, __FILE__, __LINE__, plot_json_p, "About to update plot \"%s\"", id_s);
																							FreeBSONOidString (id_s);
																						}
																					else
																						{
																							PrintJSONToLog (STM_LEVEL_INFO, __FILE__, __LINE__, plot_json_p, "About to update plot");
																						}

																					if (WritePlotRows (&plot_id, plots_collection_p, rows_p))
																						{
																							++ num_succeeded;
																						}
																					else
																						{
																							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, plot_json_p, "WritePlot () failed");
																						}
																				}
																			else
																				{
																					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, plot_json_p, "Failed to get plot_id");
																				}

																		}		/* if (plot_updated_flag) */

																}		/* if (json_is_array (rows_p)) */
														}

													json_decref (plot_json_p);
												}		/* while ((plot_json_p = GetNextDocAsJSON (plots_cursor_p)) != NULL) */

											mongoc_cursor_destroy (plots_cursor_p);
										}		/* if (plots_cursor_p) */
									else
										{
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get plots cursor");
										}

									bson_destroy (query_p);
								}		/* if (query_p) */
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to create query");
								}

							mongoc_collection_destroy (phenotypes_collection_p);
						}		/* if (phenotypes_collection_p) */
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get phenotypes collection");
						}

					mongoc_collection_destroy (plots_collection_p);
				}		/* if (plots_collection_p) */
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get plots collection");
				}

			mongoc_client_destroy (client_p);
		}		/* if (client_p) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get mongo client");
		}

	mongoc_cleanup ();

	return 0;
}



static json_t *GetNextDocAsJSON (mongoc_cursor_t *cursor_p)
{
	json_t *json_p = NULL;
	const bson_t *doc_p;

	if (mongoc_cursor_next (cursor_p, &doc_p))
		{
			char *doc_s = bson_as_canonical_extended_json (doc_p, NULL);

			if (doc_s)
				{
					json_error_t err;

					json_p = json_loads (doc_s, 0, &err);

					if (!json_p)
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to load json from \"%s\" error at %d\n", doc_s, err.position);
						}

					bson_free (doc_s);
				}
			else
				{
					puts ("Failed to convert bson to json");
				}

			//bson_destroy (doc_p);
		}


	return json_p;
}


static ValueStatus SetInteger (json_t *observation_json_p, const char * const key_s)
{
	ValueStatus ret = VS_FAIL;
	const char *value_s = GetJSONString (observation_json_p, key_s);
	int i;

	if (value_s)
		{
			int res = sscanf (value_s, "%d", &i);

			if (res == 1)
				{
					if (SetJSONInteger (observation_json_p, key_s, i))
						{
							ret = VS_NEEDS_UPDATE;
						}
					else
						{
							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, observation_json_p, "Failed to set \"%s\": %d", key_s, i);
						}
				}
			else
				{
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, observation_json_p, "Failed to convert \"%s\" to an integer, err: %d", value_s, res);
				}
		}
	else
		{
			/* no existing value set */
			ret = VS_OK;
		}

	return ret;
}


static ValueStatus CheckString (json_t *observation_json_p, const char * const key_s)
{
	ValueStatus ret = VS_FAIL;
	json_t *value_p = json_object_get (observation_json_p, key_s);


	if (value_p)
		{
			if (json_is_string (value_p))
				{
					ret = VS_OK;
				}
			else
				{
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, observation_json_p, "Stored value for \"%s\" is not a string: %d", key_s, json_typeof (value_p));
				}
		}
	else
		{
			/* no existing value set */
			ret = VS_OK;
		}

	return ret;
}


static ValueStatus CheckTime (json_t *observation_json_p, const char * const key_s)
{
	ValueStatus ret = VS_FAIL;
	const char *time_s = GetJSONString (observation_json_p, key_s);

	if (time_s)
		{
			struct tm time_val;

			if (SetTimeFromString (&time_val, time_s))
				{
					ret = VS_OK;
				}
			else
				{
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, observation_json_p, "Stored value for \"%s\", \"%s\" is not a time", key_s, time_s);
				}
		}
	else
		{
			/* no existing value set */
			ret = VS_OK;
		}

	return ret;
}



static ValueStatus SetReal (json_t *observation_json_p, const char * const key_s)
{
	ValueStatus ret = VS_FAIL;
	const char *value_s = GetJSONString (observation_json_p, key_s);
	double d;

	if (value_s)
		{
			int res = sscanf (value_s, "%lf", &d);

			if (res == 1)
				{
					if (SetJSONReal (observation_json_p, key_s, d))
						{
							ret = VS_NEEDS_UPDATE;
						}
					else
						{
							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, observation_json_p, "Failed to set \"%s\": %lf", key_s, d);
						}
				}
			else
				{
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, observation_json_p, "Failed to convert \"%s\" to an integer, err: %d", value_s, res);
				}
		}
	else
		{
			/* no existing value set */
			ret = VS_OK;
		}

	return ret;
}


static bool GetPhenotypeDatatype (const json_t *phenotype_json_p, ParameterType *param_type_p)
{
	bool success_flag = false;
	const json_t *scale_json_p = json_object_get (phenotype_json_p, "scale");

	if (scale_json_p)
		{
			const char *class_s = GetJSONString (scale_json_p, "so:name");

			if (class_s)
				{
					if (strcmp (class_s, "Date") == 0)
						{
							*param_type_p = PT_TIME;
							success_flag = true;
						}
					else if (strcmp (class_s, "Duration") == 0)
						{
							*param_type_p = PT_SIGNED_REAL;
							success_flag = true;
						}
					else if (strcmp (class_s, "Nominal") == 0)
						{
							*param_type_p = PT_STRING;
							success_flag = true;
						}
					else if (strcmp (class_s, "Numerical") == 0)
						{
							*param_type_p = PT_SIGNED_REAL;
							success_flag = true;
						}
					else if (strcmp (class_s, "Ordinal") == 0)
						{
							*param_type_p = PT_SIGNED_INT;
							success_flag = true;
						}
					else if (strcmp (class_s, "Text") == 0)
						{
							*param_type_p = PT_STRING;
							success_flag = true;
						}
					else if (strcmp (class_s, "Code") == 0)
						{
							*param_type_p = PT_STRING;
							success_flag = true;
						}
					else
						{
							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, phenotype_json_p, "Unknown scale class \"%s\"", class_s);
						}
				}
		}

	return success_flag;
}


static bool WriteObservation (bson_oid_t *observation_id_p, mongoc_collection_t *plots_collection_p, json_t *observation_p)
{
	bool success_flag = false;

  char *observations_s = json_dumps (observation_p, 0);

  if (observations_s)
  	{
  	  bson_t *query_p = BCON_NEW ("rows.observations._id", BCON_OID (observation_id_p));

  	  if (query_p)
  	  	{
  	  		bson_t *observations_bson_p = NULL;
  				bson_error_t error;

  				observations_bson_p = bson_new_from_json ((const uint8 *) observations_s, -1, &error);

  				if (observations_bson_p)
  					{
  	  	  		bson_t *update_p = BCON_NEW ("$set",
  	  												 "{",
  	  												 "observations.$",
															 observations_bson_p,
  	  												 "}");

  	  				if (update_p)
  	  					{
									if (mongoc_collection_update_one (plots_collection_p, query_p, update_p, NULL, NULL, &error))
										{
											success_flag = true;
										}
									else
										{

										}

  	  						bson_destroy (update_p);
  	  					}

  						bson_destroy (observations_bson_p);
  					}

  	  		bson_destroy (query_p);
  	  	}

  		free (observations_s);
  	}

  return success_flag;
}




static bool WritePlotRows (bson_oid_t *plot_id_p, mongoc_collection_t *plots_collection_p, json_t *rows_p)
{
	bool success_flag = false;
  char *rows_s = json_dumps (rows_p, 0);

  if (rows_s)
  	{
  	  bson_t *query_p = BCON_NEW ("plot._id", BCON_OID (plot_id_p));

  	  if (query_p)
  	  	{
  	  		bson_t *rows_bson_p = NULL;
  				bson_error_t error;

  				rows_bson_p = bson_new_from_json ((const uint8 *) rows_s, -1, &error);

  				if (rows_bson_p)
  					{
  	  	  		bson_t *update_p = BCON_NEW ("$set",
  	  												 "{",
															 "rows",
															 rows_bson_p,
  	  												 "}");

  	  				if (update_p)
  	  					{
									if (mongoc_collection_update_one (plots_collection_p, query_p, update_p, NULL, NULL, &error))
										{
											success_flag = true;
										}
									else
										{
		  	  	  	  		PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, rows_p, "mongoc_collection_update_one () failed for \"%s\" with error code: " UINT32_FMT " domain: " UINT32_FMT " message: \"%s\"", rows_s, error.code, error.domain, error.message);
										}

  	  						bson_destroy (update_p);
  	  					}
  	  	  	  else
  	  	  	  	{
  	  	  	  		PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, rows_p, "Failed to create update statement for \"%s\"", rows_s);
  	  	  	  	}

  						bson_destroy (rows_bson_p);
  					}
  	  	  else
  	  	  	{
  	  	  		PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, rows_p, "Failed to create bson from \"%s\"", rows_s);
  	  	  	}

  	  		bson_destroy (query_p);
  	  	}
  	  else
  	  	{
  	  		PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, rows_p, "Failed to create query");
  	  	}

  		free (rows_s);
  	}
  else
  	{
  		PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get plot json as string");
  	}

  return success_flag;
}


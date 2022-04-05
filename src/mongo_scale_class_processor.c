/*
 * mongo_scale_class_processor.c
 *
 *  Created on: 31 Mar 2022
 *      Author: billy
 */

#include <stdio.h>

#include "bson/bson.h"
#include "mongoc/mongoc.h"
#include "jansson.h"

#include "streams.h"

#include "mongodb_tool.h"


static json_t *GetNextDocAsJSON (mongoc_cursor_t *cursor_p);


static int SetInteger (json_t *observation_json_p, const char * const key_s);

static int SetReal (json_t *observation_json_p, const char * const key_s);

static bool WriteObservation (bson_oid_t *observation_id_p, mongoc_collection_t *plots_collection_p, json_t *observation_p);



int main (int argc, char *argv [])
{
	mongoc_client_t *client_p;
	const FieldTrialServiceData *data_p = NULL;

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
											json_t *plot_json_p = GetNextDocAsJSON (plots_cursor_p);

											if (plot_json_p)
												{
													json_t *rows_p = json_object_get (plot_json_p, "rows");

													if (rows_p)
														{
															if (json_is_array (rows_p))
																{
																	size_t i;
																	json_t *row_p;

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
																							bool updated_flag = false;
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
																													mongoc_cursor_t *phenotypes_cursor_p = mongoc_collection_find_with_opts (phenotypes_collection_p, query_p, NULL, NULL);

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
																																			int raw_ret;
																																			int corrected_ret;

																																			switch (pt)
																																				{
																																					case PT_SIGNED_INT:
																																						{
																																							raw_ret = SetInteger (observation_p, RAW_KEY_S);
																																							corrected_ret = SetInteger (observation_p, RAW_KEY_S);

																																						}
																																						break;

																																					case PT_SIGNED_REAL:
																																						{
																																							raw_ret = SetReal (observation_p, RAW_KEY_S);
																																							corrected_ret = SetReal (observation_p, RAW_KEY_S);
																																						}
																																				}		/* switch (pt) */

																																			if ((raw_ret == -1) || (corrected_ret == -1))
																																				{
																																					/* An error occurred */
																																					error_flag = true;
																																				}
																																			else if ((raw_ret == 1) || (corrected_ret == 1))
																																				{
																																					/*
																																					 * The observation's values have been updated so we'll need to
																																					 * save it back to the database
																																					 */
																																					updated_flag = true;
																																					bson_oid_t observation_id;

																																					if (GetMongoIdFromJSON (observation_p, &observation_id))
																																						{
																																							if (WriteObservation (&observation_id, plots_collection_p, observation_p))
																																								{

																																								}

																																						}

																																				}

																																		}
																																	else
																																		{

																																		}

																																	json_decref (phenotype_json_p);
																																}

																															mongoc_cursor_destroy (phenotypes_cursor_p);
																														}
																												}

																										}		/* if (GetNamedIdFromJSON (observation_p, "phenotype_id", &phenotype_id)) */
																									else
																										{

																										}
																								}		/* json_array_foreach (observations_p, j, observation_p) */


																							if (error_flag)
																								{

																								}
																							else
																								{
																									if (updated_flag)
																										{
																											/* Save the updated observations */

																										}
																								}
																						}
																				}
																		}
																}
														}

													json_decref (plot_json_p);
												}

											mongoc_cursor_destroy (plots_cursor_p);
										}

									bson_destroy (query_p);
								}

							mongoc_collection_destroy (phenotypes_collection_p);
						}

					mongoc_collection_destroy (plots_collection_p);
				}

			mongoc_client_destroy (client_p);
		}

	mongoc_cleanup ();

	return 0;
}



static json_t *GetNextDocAsJSON (mongoc_cursor_t *cursor_p)
{
	json_t *json_p = NULL;
	bson_t *doc_p;

	if (mongoc_cursor_next (cursor_p, &doc_p))
		{
			char *doc_s = bson_as_canonical_extended_json (doc_p, NULL);

			if (doc_s)
				{
					json_error_t err;
					json_t *json_p = json_loads (doc_s, 0, &err);

					if (!json_p)
						{
							printf ("Failed to load json from \"%s\" error at %d\n", doc_s, err.position);
						}

					bson_free (doc_s);
				}
			else
				{
					puts ("Failed to convert bson to json");
				}

			bson_destroy (doc_p);
		}


	return json_p;
}


static int SetInteger (json_t *observation_json_p, const char * const key_s)
{
	int ret = -1;
	const char *value_s = GetJSONString (observation_json_p, key_s);
	int i;

	if (value_s)
		{
			int res = sscanf (value_s, "%d", &i);

			if (res == 1)
				{
					if (SetJSONInteger (observation_json_p, key_s, i))
						{
							ret = 1;
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
			ret = 0;
		}

	return ret;
}


static int SetReal (json_t *observation_json_p, const char * const key_s)
{
	int ret = -1;
	const char *value_s = GetJSONString (observation_json_p, key_s);
	double d;

	if (value_s)
		{
			int res = sscanf (value_s, "%lf", &d);

			if (res == 1)
				{
					if (SetJSONReal (observation_json_p, key_s, d))
						{
							ret = 1;
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
			ret = 0;
		}

	return ret;
}

bool GetPhenotypeDatatype (const json_t *phenotype_json_p, ParameterType *param_type_p)
{
	bool success_flag = false;
	const ScaleClass *scale_class_p = NULL;
	const json_t *scale_json_p = json_object_get (phenotype_json_p, "scale");

	if (scale_json_p)
		{
			const char *class_s = GetJSONString (scale_class_p, "so:name");

			if (class_s)
				{
					if (strcmp (class_s, "Date") == 0)
						{

						}
					else if (strcmp (class_s, "Duration") == 0)
						{

						}
					else if (strcmp (class_s, "Nominal") == 0)
						{

						}
					else if (strcmp (class_s, "Numerical") == 0)
						{
							*param_type_p = PT_SIGNED_REAL;
							success_flag = true;
						}
					else if (strcmp (class_s, "Ordinal") == 0)
						{

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
  	  bson_t *query_p = BCON_NEW ("_id", BCON_OID (observation_id_p));


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

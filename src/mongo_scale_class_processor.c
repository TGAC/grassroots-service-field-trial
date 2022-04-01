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


#include "mongodb_tool.h"


static json_t *GetNextDocAsJSON (mongoc_cursor_t *cursor_p);



int main (int argc, char *argv [])
{
	mongoc_client_t *client_p;
	const FieldTrialServiceData *data_p = NULL;

	mongoc_init ();

	client_p = mongoc_client_new ("mongodb://localhost:27017/?appname=insert-example");

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



																																	json_decref (phenotype_json_p);
																																}

																															mongoc_cursor_destroy (phenotypes_cursor_p);
																														}
																												}
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

						}
					else if (strcmp (class_s, "Ordinal") == 0)
						{

						}
					else if (strcmp (class_s, "Text") == 0)
						{

						}
					else if (strcmp (class_s, "Code") == 0)
						{

						}
					else
						{

						}
				}
		}

	return success_flag;
}


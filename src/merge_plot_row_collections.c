/*
 ** Copyright 2014-2016 The Earlham Institute
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
 *
 * merge_plot_row_collections.c
 *
 *  Created on: 16 Dec 2019
 *      Author: billy
 */

#include <stdio.h>

#include "jansson.h"

#include "/dfw_field_trial_service_data.h"
#include "mongodb_util.h"
#include "mongo_client_manager.h"
#include "mongodb_tool.h"

#include "/row.h"
#include "/plot.h"
#include "bson/bson.h"

/**
 * A program to move the data from within the Rows collection into the
 * relevant documents in the Plots collection.
 */
int main (void)
{
	int ret = 0;
	const char *uri_s = "mongodb://localhost:27017";

	if (InitMongoDB ())
		{
			struct MongoClientManager *mongo_clients_p = AllocateMongoClientManager (uri_s);

			if (mongo_clients_p)
				{
					MongoTool *mongo_p = AllocateMongoTool (NULL, mongo_clients_p);

					if (mongo_p)
						{
							/*
							 * Get each document in the plot collection and add each of their rows to
							 * a "rows" array for these plots.
							 */
							if (SetMongoToolDatabaseAndCollection (mongo_p, "dfw_field_trial", DFT_PLOT_S))
								{
									json_t *plots_p = GetAllMongoResultsAsJSON (mongo_p, NULL, NULL);

									if (plots_p)
										{
											if (json_is_array (plots_p))
												{
													bson_oid_t *plot_id_p = GetNewUnitialisedBSONOid ();

													if (plot_id_p)
														{
															size_t i;
															const size_t num_plots = json_array_size (plots_p);

															if (SetMongoToolCollection (mongo_p, DFT_ROW_S))
																{
																	json_t *update_p = json_object ();

																	if (update_p)
																		{
																			size_t num_successes = 0;
																			size_t num_failures = 0;
																			bool success_flag;

																			for (i = 0; i < num_plots; ++ i)
																				{
																					json_t *plot_json_p = json_array_get (plots_p, i);
																					success_flag = false;

																					if (GetNamedIdFromJSON (plot_json_p, MONGO_ID_S, plot_id_p))
																						{
																							char *id_s = GetBSONOidAsString (plot_id_p);
																							/*
																							 * Get all of the rows for this plot
																							 */
																							bson_t *row_query_p = BCON_NEW (RO_PLOT_ID_S, BCON_OID (plot_id_p));

																							if (row_query_p)
																								{
																									if (SetMongoToolCollection (mongo_p, DFT_ROW_S))
																										{
																											json_t *rows_json_p = GetAllMongoResultsAsJSON (mongo_p, row_query_p, NULL);

																											if (rows_json_p)
																												{
																													if (json_object_set_new (update_p, PL_ROWS_S, rows_json_p) == 0)
																														{

																															if (SetMongoToolCollection (mongo_p, DFT_PLOT_S))
																																{
																																	if (UpdateMongoDocument (mongo_p, plot_id_p, update_p))
																																		{
																																			success_flag = true;
																																		}
																																	else
																																		{
																																			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, update_p, "Failed to update document");
																																		}

																																	json_object_del (update_p, PL_ROWS_S);
																																}
																															else
																																{
																																	PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, rows_json_p, "Failed to set %s collection", DFT_PLOT_S);
																																}
																														}		/* if (json_objeect_set_new (update_p, PL_ROWS_S, rows_json_p) == 0) */
																													else
																														{
																															PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, rows_json_p, "Failed to add rows to plot");
																														}

																													json_decref (rows_json_p);
																												}		/* if (rows_json_p) */
																											else
																												{
																													PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, plot_json_p, "Failed to get row results for plot");
																												}

																											if (success_flag)
																												{
																													++ num_successes;
																												}
																											else
																												{
																													++ num_failures;
																												}


																											bson_destroy (row_query_p);
																										}		/* if (row_query_p) */
																									else
																										{
																											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, plot_json_p, "Failed to set %s collection", DFT_ROW_S);
																										}

																								}

																							if (id_s)
																								{
																									FreeCopiedString (id_s);
																								}

																						}		/* if (GetNamedIdFromJSON (plot_p, MONGO_ID_S, plot_id_p)) */

																				}		/* for (i = 0; i < num_plots; ++ i) */

																			printf ("updated %lu out of %lu plots successfully\n", num_successes, num_plots);

																			json_decref (update_p);
																		}

																}		/* if (SetMongoToolCollection (mongo_p, DFT_ROW_S)) */

														}		/* if (plot_id_p) */

												}		/* if (json_is_array (plots_p)) */

											json_decref (plots_p);
										}		/* if (plots_p) */

								}		/* if (SetMongoToolCollection (mongo_p,  */

							FreeMongoTool (mongo_p);
						}		/* if (mongo_p) */

					FreeMongoClientManager (mongo_clients_p);
				}		/* if (mongo_clients_p) */

			ExitMongoDB ();
		}




	return ret;
}

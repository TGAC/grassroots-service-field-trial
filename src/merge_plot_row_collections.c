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

#include "dfw_field_trial_service_data.h"
#include "mongo_client_manager.h"
#include "mongodb_tool.h"

#include "row.h"

/**
 * A program to move the data from within the Rows collection into the
 * relevant documents in the Plots collection.
 */
int main (void)
{
	int ret = 0;
	const char *uri_s = "mongodb://localhost:27017";
	struct MongoClientManager *mongo_clients_p = AllocateMongoClientManager (uri_s);

	if (mongo_clients_p)
		{
			MongoTool *mongo_p = AllocateMongoTool (NULL, mongo_clients_p);

			if (mongo_p)
				{
					/*
					 * Get each document in the rows collection and add it to the "rows"
					 * array for their parent plots.
					 */
					if (SetMongoToolCollection (mongo_p, DFT_ROW_S))
						{
							bson_t *opts_p = BCON_NEW ( "sort", "{", RO_PLOT_ID_S, BCON_INT32 (1), "}");

							if (opts_p)
								{
									json_t *rows_p = GetAllMongoResultsAsJSON (mongo_p, NULL, opts_p);

									if (rows_p)
										{
											if (json_is_array (rows_p))
												{
													bson_oid_t *plot_id_p = GetNewUnitialisedBSONOid ();

													if (plot_id_p)
														{
															size_t i;
															const size_t num_rows = json_array_size (rows_p);

															if (SetMongoToolCollection (mongo_p, DFT_PLOT_S))
																{
																	for (i = 0; i < num_rows; ++ i)
																		{
																			json_t *row_p = json_array_get (rows_p, i);

																			if (GetNamedIdFromJSON (row_p, RO_PLOT_ID_S, plot_id_p))
																				{
																					bson_t *plot_query_p = BCON_NEW (RO_PLOT_ID_S, BCON_OID (plot_id_p));

																					if (plot_query_p)
																						{

																							bson_destroy (plot_query_p);
																						}		/* if (plot_query_p) */

																				}		/* if (GetNamedIdFromJSON (row_p, RO_PLOT_ID_S, plot_id_p)) */

																		}		/* for (i = 0; i < num_rows; ++ i) */

																}		/* if (SetMongoToolCollection (mongo_p, DFT_PLOT_S)) */

														}		/* if (plot_id_p) */

												}		/* if (json_is_array (rows_p)) */

										}		/* if (rows_p) */

									bson_destroy (opts_p);
								}		/* if (opts_p) */

						}		/* if (SetMongoToolCollection (mongo_p,  */

					FreeMongoTool (mongo_p);
				}		/* if (mongo_p) */

			FreeMongoClientManager (mongo_clients_p);
		}		/* if (mongo_clients_p) */

	return ret;
}

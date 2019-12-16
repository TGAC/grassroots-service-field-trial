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


/**
 * A program to move the data from within the Rows collection into the
 * relevant documents in the Plots collection.
 */
int main (void)
{
	int ret = 0;
	const char *uri_s = "mongodb://localhost:27017";
	MongoClientManager *mongo_clients_p = AllocateMongoClientManager (uri_s);

	if (mongo_clients_p)
		{
			MongoTool *mongo_p = AllocateMongoTool (NULL, mongo_clients_p);

			if (mongo_p)
				{
					/*
					 * Get each document in the rows collection and add it to the "rows"
					 * array for their parent plots.
					 */
					if (SetMongoToolCollection (mongo_p, data_p -> dftsd_collection_ss [DFTD_ROW]))
						{
							json_t *results_p = GetAllMongoResultsAsJSON (mongo_p, NULL, NULL);

							if (results_p)
								{
									if (json_is_array (results_p))
										{

										}		/* if (json_is_array (results_p)) */

								}		/* if (results_p) */

						}

					FreeMongoTool (mongo_p);
				}		/* if (mongo_p) */

			FreeMongoClientManager (mongo_clients_p);
		}		/* if (mongo_clients_p) */



	return ret;
}

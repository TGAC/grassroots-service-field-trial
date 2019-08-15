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


/**
 * @file
 * @brief
 */
#include <stdio.h>

#include "jansson.h"

#include "typedefs.h"
#include "streams.h"
#include "json_util.h"
#include "byte_buffer.h"


#include "location_jobs.h"
#include "field_trial.h"

typedef enum
{
	IM_LOCATIONS,
	IM_TRIALS,
	IM_NUM_MODES
} ImportMode;



/*
 * STATIC DECLARATIONS
 */

static bool ImportTrials (const json_t *trial_p, const char *grassroots_url_s);

static bool ImportLocations (const json_t *locations_p, const char *grassroots_url_s);

static bool ImportLocation (const json_t *location_p, const char *grassroots_url_s);



/*
 * DEFINITIONS
 */

int main (int argc, char **argv)
{
	int res = 0;


	if (argc != 1)
		{
			const char *grassroots_url_s = NULL;
			const char *filename_s = NULL;
			ImportMode im = IM_NUM_MODES;
			int i = 1;

			while (i < argc)
				{
					const char *arg_s = * (argv + i);

					if (strcmp (arg_s, "--mode") == 0)
						{
							if ((i + 1) < argc)
								{
									arg_s = argv [++ i];

									if (strcmp (arg_s, "trials") == 0)
										{
											im = IM_TRIALS;
										}
									else if (strcmp (arg_s, "locations") == 0)
										{
											im = IM_LOCATIONS;
										}
									else
										{
											printf ("unknown mode \"%s\"", arg_s);
										}
								}
							else
								{
									printf ("mode argument missing");
								}

						}		/* if (strcmp (arg_s "--mode") == 0) */
					else if (strcmp (arg_s, "--in") == 0)
						{
							if ((i + 1) < argc)
								{
									filename_s = argv [++ i];
								}
							else
								{
									printf ("input filename argument missing");
								}
						}		/* else if (strcmp (arg_s, "--in ") == 0) */
					else if (strcmp (arg_s, "--url") == 0)
						{
							if ((i + 1) < argc)
								{
									grassroots_url_s = argv [++ i];
								}
							else
								{
									printf ("grassroots url argument missing");
								}
						}		/* else if (strcmp (arg_s, "--url") == 0) */

					++ i;
				}		/* while (i < argc) */



			if ((im != IM_NUM_MODES) && (grassroots_url_s != NULL) && (filename_s != NULL))
				{
					json_error_t err;
					json_t *data_p = json_load_file (filename_s, 0, &err);

					if (data_p)
						{
							switch (im)
								{
									case IM_LOCATIONS:
										ImportLocations (data_p, grassroots_url_s);
										break;

									case IM_TRIALS:
										ImportTrials (data_p, grassroots_url_s);
										break;

									default:
										break;
								}

							json_decref (data_p);
						}		/* if (data_p) */

				}		/* if ((im != IM_NUM_MODES) && (grassroots_url_s != NULL) && (filename_s != NULL)) */
			else
				{
					printf ("incomplete args: mode %d, input file \"%s\", grassroots url \"%s\"", im, filename_s, grassroots_url_s);
				}
		}
	else
		{
			printf ("USAGE: importer --mode (trials|locations) --in <filename> --url <grassroots url>\n");
		}




	return res;
}


/*
 * STATIC DEFINITIONS
 */


static bool ImportTrials (const json_t *trials_p, const char *grassroots_url_s)
{
	bool success_flag = false;

	if (json_is_array (trials_p))
		{
			json_t *locations_cache_p = json_object ();

			if (locations_cache_p)
				{

				}		/* if (locations_cache_p) */

		}
	else
		{

		}

	return success_flag;
}



static bool ImportLocations (const json_t *locations_p, const char *grassroots_url_s)
{
	bool success_flag = false;

	if (json_is_array (locations_p))
		{
			json_t *location_p;
			size_t i;

			json_array_foreach (locations_p, i, location_p)
				{
					if (!ImportLocation (location_p, grassroots_url_s))
						{
						}
				}
		}
	else
		{

		}

	return success_flag;
}


static bool ImportLocation (const json_t *location_p, const char *grassroots_url_s)
{
	bool success_flag = false;
	const char *field_s = GetJSONString (location_p, "Field");

	if (field_s)
		{
			const char *lat_s = GetJSONString (location_p, "Lat");

			if (lat_s)
				{
					const char *long_s = GetJSONString (location_p, "Long");

					if (long_s)
						{
							/*
							 * Build the request
							 */
							ByteBuffer *buffer_p = AllocateByteBuffer (1024);

							if (buffer_p)
								{
									/*
									 * https://grassroots.tools/grassroots-test/5/controller/service/DFWFieldTrial%20search%20service?FT%20Keyword%20Search=simon
									 */

									if (AppendStringsToByteBuffer (buffer_p, grassroots_url_s, "service/Submit%20Field%20Trial%20Location", NULL))
										{
											if (AppendStringsToByteBuffer (buffer_p, "?", LOCATION_NAME.npt_name_s, "=", field_s, NULL))
												{
													if (AppendStringsToByteBuffer (buffer_p, "&", LOCATION_LATITUDE.npt_name_s, "=", lat_s, NULL))
														{
															if (AppendStringsToByteBuffer (buffer_p, "&", LOCATION_LONGITUDE.npt_name_s, "=", long_s, NULL))
																{
																	if (AppendStringsToByteBuffer (buffer_p, "&", LOCATION_USE_GPS.npt_name_s, "=true", NULL))
																		{
																			CurlTool *curl_p = AllocateCurlTool (CM_MEMORY);

																			if (curl_p)
																				{
																					const char *url_s = GetByteBufferData (buffer_p);

																					if (SetUriForCurlTool (curl_p, url_s))
																						{
																							CURLcode c = RunCurlTool (curl_p);

																							if (c == CURLE_OK)
																								{
																									const char *response_s = GetCurlToolData (curl_p);

																									if (response_s)
																										{
																											json_error_t err;
																											json_t *res_p = json_loads (response_s, JSON_DECODE_ANY, &err);

																											if (res_p)
																												{
																													/*
																													{
																														"header": {
																															"schema": {
																																"so:softwareVersion": "0.10"
																															}
																														},
																														"@context": {
																															"so:": "http://schema.org/",
																															"eo:": "http://edamontology.org/",
																															"efo:": "http://www.ebi.ac.uk/efo/",
																															"swo:": "http://www.ebi.ac.uk/swo/"
																														},
																														"results": [
																															{
																																"service_name": "Submit Field Trial Location",
																																"job_type": "default_service_job",
																																"status": 5,
																																"status_text": "Succeeded",
																																"job_uuid": "2beb4af8-c565-4aff-a185-f04e602a5c53",
																																"so:description": "Submit Location"
																															}
																														]
																													}
																													*/

																													json_t *results_p = json_object_get (res_p, "results");

																													if (results_p)
																														{
																															if (json_is_array (results_p))
																																{
																																	size_t num_results = json_array_size (results_p);
																																	json_t *result_p;
																																	size_t j;
																																	size_t num_succeeded = 0;

																																	json_array_foreach (results_p, j , result_p)
																																		{
																																			OperationStatus status = OS_ERROR;
																																			const char *value_s = GetJSONString (result_p, SERVICE_STATUS_S);

																																			if (value_s)
																																				{
																																					status = GetOperationStatusFromString (value_s);
																																				}
																																			else
																																				{
																																					int i;
																																					/* Get the job status */

																																					if (GetJSONInteger (result_p, SERVICE_STATUS_VALUE_S, &i))
																																						{
																																							if ((i > OS_LOWER_LIMIT) && (i < OS_UPPER_LIMIT))
																																								{
																																									status = (OperationStatus) i;
																																								}
																																						}
																																				}

																																			if (status == OS_SUCCEEDED)
																																				{
																																					++ num_succeeded;
																																				}
																																			else
																																				{
																																					printf ("an import failed");
																																					success_flag = false;
																																				}

																																		}

																																	printf ("imported %lu out of %lu records successfully\n", num_succeeded, num_results);
																																}
																														}		/* if (results_p) */

																													json_decref (res_p);
																												}
																										}
																								}
																						}
																				}

																			FreeCurlTool (curl_p);
																		}

																}
														}
												}
										}

									FreeByteBuffer (buffer_p);
								}
						}
					else
						{
							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, location_p, "Failed to get Long");
						}

				}
			else
				{
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, location_p, "Failed to get Lat");
				}

		}
	else
		{
			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, location_p, "Failed to get Field");
		}

	return success_flag;
}

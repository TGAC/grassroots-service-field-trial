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
#include "curl_tools.h"

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


static bool ImportLocation (const json_t *location_p, const char *grassroots_url_s, size_t *num_successes_p, size_t *num_failures_p);

static bool AddVariableToBuffer (ByteBuffer *buffer_p, const char *prefix_s, const char *key_s, const char *value_s, CurlTool *curl_p);


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
			json_t *trial_p;
			size_t i;
			size_t num_successes = 0;
			size_t num_failures = 0;

			json_array_foreach (trials_p, i, trial_p)
				{
					ImportTrial (trial_p, grassroots_url_s, &num_successes, &num_failures);
				}

			printf ("Imported %lu out of %lu trials successfully\n", num_successes, num_failures + num_successes);
		}
	else
		{

		}

	return success_flag;
}



/*
{
	"id": "2793",
	"propNo": "2018/R/WW/1812",
	"propTitle": "DFW Mapping Populations 7th year",
	"projectName": "Awaiting project code allocation",
	"experimentType": "Annual Experiments",
	"fieldname": "Summerdells 2",
	"confidentialTreatments": "0",
	"studyDirectorSupplySeed": "1",
	"treatmentFactors": "(9 Population/Individuals thereof) * N (2 levels of N).",
	"NoOfTreatments": "(9 populations/Individuals thereof) * 2 N levels",
	"designLayout": "Split plot randomised design in 3 blocks, split by N treatment and populations, individuals of the populations randomised within the split plots.",
	"plotLength": "1",
	"plotWidth": "1",
	"NoOfReplicates": "3",
	"TotalNoOfPlots": "4800",
	"measurementsToBeTakenAndDivisionOfLabour": "In season measurements by sponsor",
	"pAndKFertiliser": "1",
	"specificSowingDate": "0",
	"seedTreatment": "1",
	"autumnSpringFungicides": "1",
	"autumnSpringHerbicides": "1",
	"autumnSpringInsecticides": "1",
	"plantGrowthRegulators": "1",
	"sFertiliser": "1",
	"nFertiliser": "0",
	"nFertiliserNotes": "2 rates, 50 and 200 kg/ha",
	"irrigation": "0",
	"yieldsTakenByFarm": "1",
	"yieldsTakenBySponsor": "0",
	"postHarvest": "0",
	"postHarvestSampling": "0",
	"licences": "0",
	"gmoInvolved": "0",
	"cropDestruct": "0",
	"reportDeadlines": "Dec 2018"
}
 */
static bool ImportTrial (const json_t *location_p, const char *grassroots_url_s, size_t *num_successes_p, size_t *num_failures_p)
{
	bool success_flag = false;

	return success_flag;
}


static bool ImportLocations (const json_t *locations_p, const char *grassroots_url_s)
{
	bool success_flag = false;

	if (json_is_array (locations_p))
		{
			json_t *location_p;
			size_t i;
			size_t num_successes = 0;
			size_t num_failures = 0;

			json_array_foreach (locations_p, i, location_p)
				{
					ImportLocation (location_p, grassroots_url_s, &num_successes, &num_failures);
				}

			printf ("imported %lu out of %lu locations successfully\n", num_successes, num_failures + num_successes);

		}
	else
		{

		}

	return success_flag;
}


static bool AddVariableToBuffer (ByteBuffer *buffer_p, const char *prefix_s, const char *key_s, const char *value_s, CurlTool *curl_p)
{
	bool success_flag = false;
	char *escaped_key_s = GetURLEscapedString (curl_p, key_s);

	if (escaped_key_s)
		{
			char *escaped_value_s = GetURLEscapedString (curl_p, value_s);

			if (escaped_value_s)
				{
					if (AppendStringsToByteBuffer (buffer_p, prefix_s, escaped_key_s, "=", escaped_value_s, NULL))
						{
							success_flag = true;
						}

					FreeURLEscapedString (escaped_value_s);
				}

			FreeURLEscapedString (escaped_key_s);
		}

	return success_flag;
}


static bool ImportLocation (const json_t *location_p, const char *grassroots_url_s, size_t *num_successes_p, size_t *num_failures_p)
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
									CurlTool *curl_p = AllocateCurlTool (CM_MEMORY);

									if (curl_p)
										{
											if (AppendStringsToByteBuffer (buffer_p, grassroots_url_s, "service/Submit%20Field%20Trial%20Location", NULL))
												{
													if (AddVariableToBuffer (buffer_p, "?", LOCATION_NAME.npt_name_s, field_s, curl_p))
														{
															if (AddVariableToBuffer (buffer_p, "&", LOCATION_LATITUDE.npt_name_s, lat_s, curl_p))
																{
																	if (AddVariableToBuffer (buffer_p, "&", LOCATION_LONGITUDE.npt_name_s, long_s, curl_p))
																		{
																			if (AddVariableToBuffer (buffer_p, "&", LOCATION_USE_GPS.npt_name_s, "true", curl_p))
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
																																					++ *num_successes_p;
																																				}
																																			else
																																				{
																																					printf ("an import failed");
																																					++ *num_failures_p;
																																				}

																																		}
																																}
																														}		/* if (results_p) */

																													json_decref (res_p);
																												}
																										}
																								}
																						}
																				}
																		}
																}
														}
												}

											FreeCurlTool (curl_p);
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

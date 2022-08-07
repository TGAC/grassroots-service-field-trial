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
#include "field_trial_jobs.h"
#include "study_jobs.h"
#include "field_trial.h"
#include "schema_keys.h"
#include "submit_study.h"


typedef enum
{
	IM_LOCATIONS,
	IM_TRIALS,
	IM_STUDIES,
	IM_NUM_MODES
} ImportMode;



/*
 * STATIC DECLARATIONS
 */

static void ImportData (const json_t *data_p, const char *grassroots_url_s, bool (*import_callback_fn) (const json_t *data_p, const char *grassroots_url_s, size_t *num_successes_p, size_t *num_failures_p));

static bool ImportTrial (const json_t *study_p, const char *grassroots_url_s, size_t *num_successes_p, size_t *num_failures_p);

static bool ImportStudy (const json_t *study_p, const char *grassroots_url_s, size_t *num_successes_p, size_t *num_failures_p);

static bool ImportLocation (const json_t *location_p, const char *grassroots_url_s, size_t *num_successes_p, size_t *num_failures_p);

static bool AddVariableToBuffer (ByteBuffer *buffer_p, const char *prefix_s, const char *key_s, const char *value_s, CurlTool *curl_p);

static bool CallFieldTrialWebservice (const json_t *req_p, CurlTool *curl_p, size_t *num_successes_p, size_t *num_failures_p);

static json_t *PreprocessTrials (const json_t *src_p);

static bool AddUniqueTrial (json_t *trials_p, json_t *cache_p, const char *name_s, const char *team_s);

static json_t *GetInitialRequest (const char *service_name_s, json_t **params_array_pp);


static bool AddParamFromImportData (json_t *params_p, const json_t *study_p, const char *study_key_s, const char *db_key_s);

static bool AddParam (json_t *params_p, const char *key_s, const char *value_s);

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

			bool (*import_callback_fn) (const json_t *data_p, const char *grassroots_url_s, size_t *num_successes_p, size_t *num_failures_p) = NULL;
			json_t *(*preprocess_data_fn) (const json_t *src_p) = NULL;
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
											import_callback_fn = ImportTrial;
											preprocess_data_fn = PreprocessTrials;
										}
									else if (strcmp (arg_s, "locations") == 0)
										{
											import_callback_fn = ImportLocation;
										}
									else if (strcmp (arg_s, "studies") == 0)
										{
											import_callback_fn = ImportStudy;
										}
									else
										{
											printf ("unknown mode \"%s\"", arg_s);
										}
								}
							else
								{
									puts ("mode argument missing");
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
									puts ("input filename argument missing");
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
									puts ("grassroots url argument missing");
								}
						}		/* else if (strcmp (arg_s, "--url") == 0) */

					++ i;
				}		/* while (i < argc) */



			if ((import_callback_fn != NULL) && (grassroots_url_s != NULL) && (filename_s != NULL))
				{
					json_error_t err;
					json_t *original_data_p = json_load_file (filename_s, 0, &err);

					if (original_data_p)
						{
							bool run_flag = true;
							json_t *data_p = original_data_p;

							if (preprocess_data_fn)
								{
									data_p = preprocess_data_fn (original_data_p);

									if (!data_p)
										{
											puts ("Failed to preprocess data");
											run_flag = false;
										}
								}

							if (run_flag)
								{
									ImportData (data_p, grassroots_url_s, import_callback_fn);
								}

							if (data_p != original_data_p)
								{
									json_decref (data_p);
								}

							json_decref (original_data_p);
						}		/* if (data_p) */

				}		/* if ((im != IM_NUM_MODES) && (grassroots_url_s != NULL) && (filename_s != NULL)) */
			else
				{
					printf ("incomplete args: input file \"%s\", grassroots url \"%s\"\n", filename_s, grassroots_url_s);
				}
		}
	else
		{
			printf ("USAGE: importer --mode (trials|locations|studies) --in <filename> --url <grassroots url>\n");
		}

	return res;
}


/*
 * STATIC DEFINITIONS
 */


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


static void ImportData (const json_t *data_p, const char *grassroots_url_s, bool (*import_callback_fn) (const json_t *data_p, const char *grassroots_url_s, size_t *num_successes_p, size_t *num_failures_p))
{
	size_t num_successes = 0;
	size_t num_failures = 0;

	if (json_is_array (data_p))
		{
			json_t *item_p;
			size_t i;
			const size_t size = json_array_size (data_p);
			json_array_foreach (data_p, i, item_p)
			{
				printf ("importing record " SIZET_FMT " of " SIZET_FMT "\n", i, size);
				import_callback_fn (item_p, grassroots_url_s, &num_successes, &num_failures);
			}

		}
	else
		{
			import_callback_fn (data_p, grassroots_url_s, &num_successes, &num_failures);
		}

	printf ("imported " SIZET_FMT " out of " SIZET_FMT " items successfully\n", num_successes, num_failures + num_successes);
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
							 * https://grassroots.tools/grassroots-test/5/controller/service/DFWFieldTrial%20search%20service?FT%20Keyword%20Search=simon
							 */
							CurlTool *curl_p = AllocateMemoryCurlTool (0);

							if (curl_p)
								{
									/*
									 * Build the request
									 */
									ByteBuffer *buffer_p = AllocateByteBuffer (1024);

									if (buffer_p)
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

																					CallFieldTrialWebservice (NULL, curl_p, num_successes_p, num_failures_p);
																				}
																		}
																}
														}
												}

											FreeByteBuffer (buffer_p);
										}

									FreeCurlTool (curl_p);
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
static bool ImportTrial (const json_t *trial_p, const char *grassroots_url_s, size_t *num_successes_p, size_t *num_failures_p)
{
	bool success_flag = false;
	const char *name_s = GetJSONString (trial_p, FIELD_TRIAL_NAME.npt_name_s);

	if (name_s)
		{
			const char *team_s = GetJSONString (trial_p, FIELD_TRIAL_TEAM.npt_name_s);

			if (team_s)
				{
					CurlTool *curl_p = AllocateMemoryCurlTool (0);

					if (curl_p)
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
									if (AppendStringsToByteBuffer (buffer_p, grassroots_url_s, "service/Submit%20Field%20Trials", NULL))
										{
											if (AddVariableToBuffer (buffer_p, "?", FIELD_TRIAL_NAME.npt_name_s, name_s, curl_p))
												{
													if (AddVariableToBuffer (buffer_p, "&", FIELD_TRIAL_TEAM.npt_name_s, team_s, curl_p))
														{
															if (AddVariableToBuffer (buffer_p, "&", FIELD_TRIAL_ADD.npt_name_s, "true", curl_p))
																{
																	const char *url_s = GetByteBufferData (buffer_p);

																	PrintJSONToLog (STM_LEVEL_INFO, __FILE__, __LINE__, trial_p, "Importing");

																	CallFieldTrialWebservice (NULL, curl_p, num_successes_p, num_failures_p);
																}
														}
												}
										}
									FreeByteBuffer (buffer_p);
								}

							FreeCurlTool (curl_p);
						}

				}
			else
				{
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, trial_p, "Failed to get %s", FIELD_TRIAL_TEAM.npt_name_s);
				}

		}
	else
		{
			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, trial_p, "Failed to get %s", FIELD_TRIAL_NAME.npt_name_s);
		}

	return success_flag;

}


static bool AddUniqueTrial (json_t *trials_p, json_t *cache_p, const char *name_s, const char *team_s)
{
	bool success_flag = false;

	if (!json_object_get (cache_p, name_s))
		{
			json_t *trial_p = json_object ();

			if (trial_p)
				{
					if (SetJSONString (trial_p, FIELD_TRIAL_NAME.npt_name_s, name_s))
						{
							if (SetJSONString (trial_p, FIELD_TRIAL_TEAM.npt_name_s, team_s))
								{
									if (SetJSONString (cache_p, name_s, name_s))
										{
											if (json_array_append_new (trials_p, trial_p) == 0)
												{
													success_flag = true;
												}
											else
												{
													if (json_object_del (cache_p, name_s) != 0)
														{
															printf ("Failed to delete \"%s\" from cache\n", name_s);
														}
												}
										}
									else
										{
											printf ("Failed to add \"%s\" to cache\n", name_s);
										}


								}		/* if (SetJSONString (trial_p, FIELD_TRIAL_TEAM, team_s)) */
							else
								{
									printf ("failed to set \"%s\": \"%s\" for trial\n", FIELD_TRIAL_TEAM.npt_name_s, team_s);
								}

						}		/* if (SetJSONString (trial_p, FIELD_TRIAL_NAME, name_s)) */
					else
						{
							printf ("failed to set \"%s\": \"%s\" for trial\n", FIELD_TRIAL_NAME.npt_name_s, name_s);
						}


				}		/* if (trial_p) */
			else
				{
					puts ("failed to allocate trial");
				}

		}
	else
		{
			success_flag = true;
		}

	return success_flag;
}


static json_t *PreprocessTrials (const json_t *src_p)
{
	bool success_flag = false;
	json_t *trials_p = json_array ();

	if (trials_p)
		{
			if (json_is_array (src_p))
				{
					json_t *cache_p = json_object ();

					if (cache_p)
						{
							size_t i = 0;
							const size_t num_trials = json_array_size (src_p);
							const char * const NAME_KEY_S = "projectName";
							success_flag = true;

							while ((i < num_trials) && success_flag)
								{
									const json_t *src_trial_p = json_array_get (src_p, i);
									const char *name_s = GetJSONString (src_trial_p, NAME_KEY_S);

									printf ("Preprocessing trial " SIZET_FMT "\n", i);

									if (name_s)
										{
											if (!AddUniqueTrial (trials_p, cache_p, name_s, "Rothamsted Research"))
												{
													printf ("Failed to add trial \"%s\"\n", name_s);
													success_flag = false;
												}
										}
									else
										{
											printf ("Failed to get name \"%s\"\n", NAME_KEY_S);
										}

									if (success_flag)
										{
											++ i;
										}
								}		/* while ((i < num_trials) && success_flag) */


							json_decref (cache_p);
						}

				}

			if (success_flag)
				{
					return trials_p;
				}

			json_decref (trials_p);
		}		/* if (trials_p) */

	return NULL;
}


/*
{
  "services": [
    {
      "so:name": "Submit Field Trial Study",
      "start_service": true,
      "parameter_set": {
        "level": "simple",
        "parameters": [
				]
      }
    }
  ]
}


 */
static json_t *GetInitialRequest (const char *service_name_s, json_t **params_array_pp)
{
	json_t *root_p = json_object ();

	if (root_p)
		{
			json_t *services_p = json_array ();

			if (services_p)
				{
					if (json_object_set_new (root_p, SERVICES_NAME_S, services_p) == 0)
						{
							json_t *service_p = json_object ();

							if (service_p)
								{
									if (json_array_append_new (services_p, service_p) == 0)
										{
											if (SetJSONString (service_p, SERVICE_NAME_S, service_name_s))
												{
													if (SetJSONBoolean (service_p, SERVICE_RUN_S, true))
														{
															json_t *param_set_p = json_object ();

															if (param_set_p)
																{
																	if (json_object_set_new (service_p, PARAM_SET_KEY_S, param_set_p) == 0)
																		{
																			if (SetJSONString (service_p, PARAM_LEVEL_S, PARAM_LEVEL_TEXT_SIMPLE_S))
																				{
																					json_t *params_array_p = json_array ();

																					if (params_array_p)
																						{
																							if (json_object_set_new (param_set_p, PARAM_SET_PARAMS_S, params_array_p) == 0)
																								{
																									*params_array_pp = params_array_p;
																									return root_p;
																								}
																							else
																								{
																									json_decref (params_array_p);
																								}
																						}
																				}

																		}
																	else
																		{
																			json_decref (param_set_p);
																		}
																}

														}

												}

										}
									else
										{
											json_decref (service_p);
										}
								}

						}
					else
						{
							json_decref (services_p);
						}
				}

			json_decref (root_p);
		}

	return NULL;
}


static bool AddParamFromImportData (json_t *params_p, const json_t *study_p, const char *study_key_s, const char *db_key_s)
{
	bool success_flag = false;
	const char *value_s = GetJSONString (study_p, study_key_s);

	if (value_s)
		{
			success_flag = AddParam (params_p, db_key_s, value_s);
		}

	return success_flag;
}


static bool AddParam (json_t *params_p, const char *key_s, const char *value_s)
{
	json_t *param_p = json_object ();

	if (param_p)
		{
			if (SetJSONString (param_p, PARAM_NAME_S, key_s))
				{
					if (SetJSONString (param_p, PARAM_CURRENT_VALUE_S, value_s))
						{
							if (json_array_append_new (params_p, param_p) == 0)
								{
									return true;
								}
							else
								{
									json_decref (param_p);
								}
						}
				}
		}

	return false;
}


static bool ImportStudy (const json_t *study_p, const char *grassroots_url_s, size_t *num_successes_p, size_t *num_failures_p)
{
	bool success_flag = false;
	CurlTool *curl_p = AllocateMemoryCurlTool (0);

	if (curl_p)
		{
			json_t *params_p = NULL;
			json_t *submission_p = GetInitialRequest (GetStudySubmissionServiceName (NULL), &params_p);


			if (submission_p)
				{
					if (AddParam (params_p, STUDY_DESCRIPTION.npt_name_s, ""))
						{
							if (AddParam (params_p, STUDY_THIS_CROP.npt_name_s, "Unknown"))
								{
									if (AddParam (params_p, STUDY_PREVIOUS_CROP.npt_name_s, "Unknown"))
										{
											if (AddParamFromImportData (params_p, study_p, "propTitle", STUDY_NAME.npt_name_s))
												{
													if (AddParamFromImportData (params_p, study_p, "projectName", STUDY_FIELD_TRIALS_LIST.npt_name_s))
														{
															if (AddParamFromImportData (params_p, study_p, "fieldname", STUDY_LOCATIONS_LIST.npt_name_s))
																{
																	if (AddParamFromImportData (params_p, study_p, "designLayout", STUDY_DESIGN.npt_name_s))
																		{
																			if (AddParamFromImportData (params_p, study_p, "measurementsToBeTakenAndDivisionOfLabour", STUDY_PHENOTYPE_GATHERING_NOTES.npt_name_s))
																				{
																					if (SetUriForCurlTool (curl_p, grassroots_url_s))
																						{
																							CallFieldTrialWebservice (submission_p, curl_p, num_successes_p, num_failures_p);
																							success_flag = true;
																						}

																				}
																			else
																				{
																					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, submission_p, "Failed to add \"measurementsToBeTakenAndDivisionOfLabour\" -> \"%s\" to params", STUDY_PHENOTYPE_GATHERING_NOTES.npt_name_s);
																				}


																		}
																	else
																		{
																			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, submission_p, "Failed to add \"designLayout\" -> \"%s\" to params", STUDY_DESIGN.npt_name_s);
																		}
																}
															else
																{
																	PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, submission_p, "Failed to add \"fieldname\" -> \"%s\" to params", STUDY_LOCATIONS_LIST.npt_name_s);
																}

														}		/* if (AddParamFromImportData (params_p, STUDY_FIELD_TRIALS_LIST.npt_name_s, trial_s)) */
													else
														{
															PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, submission_p, "Failed to add \"projectName\" -> \"%s\" to params", STUDY_FIELD_TRIALS_LIST.npt_name_s);
														}

												}		/* if (AddParam (params_p, STUDY_NAME.npt_name_s, study_s)) */
											else
												{
													PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, submission_p, "Failed to add \"propTitle\" -> \"%s\" to params", STUDY_NAME.npt_name_s);
												}

										}

								}

						}


					json_decref (submission_p);
				}		/* if (submission_p) */

			FreeCurlTool (curl_p);
		}		/* if (curl_p) */


	if (!success_flag)
		{
			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, study_p, "Failed to import study");

			++ (*num_failures_p);
		}

	return success_flag;
}



static bool CallFieldTrialWebservice (const json_t *req_p, CurlTool *curl_p, size_t *num_successes_p, size_t *num_failures_p)
{
	bool success_flag = false;


	PrintJSONToLog (STM_LEVEL_INFO, __FILE__, __LINE__, req_p, "REQUEST: ");

	if (MakeRemoteJSONCallFromCurlTool (curl_p, req_p))
		{
			const char *response_s = GetCurlToolData (curl_p);

			if (response_s)
				{
					json_error_t err;
					json_t *res_p = json_loads (response_s, JSON_DECODE_ANY, &err);


					printf ("response:\n%s\n", response_s);

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
											json_t *result_p;
											size_t j;

											success_flag = true;

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
														json_int_t i;
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
														puts ("an import failed");
														++ *num_failures_p;
													}

											}
										}
								}		/* if (results_p) */

							json_decref (res_p);
						}
				}

		}

	return success_flag;
}

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
#include <string.h>

#include "jansson.h"

#define ALLOCATE_DFW_FIELD_TRIAL_TAGS
#include "dfw_field_trial_service.h"
#include "dfw_field_trial_service_data.h"
#include "memory_allocations.h"
#include "parameter.h"
#include "service_job.h"
#include "mongodb_tool.h"
#include "string_utils.h"
#include "json_tools.h"
#include "grassroots_config.h"
#include "string_linked_list.h"
#include "math_utils.h"
#include "search_options.h"
#include "time_util.h"
#include "io_utils.h"
#include "audit.h"


#ifdef _DEBUG
#define DFW_FIELD_TRIAL_SERVICE_DEBUG	(STM_LEVEL_FINER)
#else
#define DFW_FIELD_TRIAL_SERVICE_DEBUG	(STM_LEVEL_NONE)
#endif


static NamedParameterType PGS_UPDATE = { "Update", PT_JSON };
static NamedParameterType PGS_QUERY = { "Search", PT_JSON };
static NamedParameterType PGS_REMOVE = { "Delete", PT_JSON };
static NamedParameterType PGS_DUMP = { "Dump data", PT_BOOLEAN };
static NamedParameterType PGS_COLLECTION = { "Collection", PT_STRING };
static NamedParameterType PGS_DELIMITER = { "Data delimiter", PT_CHAR };
static NamedParameterType PGS_FILE = { "Upload", PT_TABLE};


static const char *s_data_names_pp [DFTD_NUM_TYPES];


static const char S_DEFAULT_COLUMN_DELIMITER =  '|';


/*
 * STATIC PROTOTYPES
 */

static Service *GetDFWFieldTrialService (void);

static const char *GetDFWFieldTrialServiceName (Service *service_p);

static const char *GetDFWFieldTrialServiceDesciption (Service *service_p);

static const char *GetDFWFieldTrialServiceInformationUri (Service *service_p);

static ParameterSet *GetDFWFieldTrialServiceParameters (Service *service_p, Resource *resource_p, UserDetails *user_p);

static void ReleaseDFWFieldTrialServiceParameters (Service *service_p, ParameterSet *params_p);

static ServiceJobSet *RunDFWFieldTrialService (Service *service_p, ParameterSet *param_set_p, UserDetails *user_p, ProvidersStateTable *providers_p);

static  ParameterSet *IsResourceForDFWFieldTrialService (Service *service_p, Resource *resource_p, Handler *handler_p);


static DFWFieldTrialServiceData *AllocateDFWFieldTrialServiceData (void);

static json_t *GetDFWFieldTrialServiceResults (Service *service_p, const uuid_t job_id);

static bool ConfigureDFWFieldTrialService (DFWFieldTrialServiceData *data_p);


static void FreeDFWFieldTrialServiceData (DFWFieldTrialServiceData *data_p);

static bool CloseDFWFieldTrialService (Service *service_p);


static uint32 InsertData (MongoTool *tool_p, ServiceJob *job_p, json_t *values_p, const DFWFieldTrialData collection_type, DFWFieldTrialServiceData *service_data_p);


static OperationStatus SearchData (MongoTool *tool_p, ServiceJob *job_p, json_t *data_p, const DFWFieldTrialData collection_type, DFWFieldTrialServiceData *service_data_p, const bool preview_flag);


static uint32 DeleteData (MongoTool *tool_p, ServiceJob *job_p, json_t *data_p, const DFWFieldTrialData collection_type, DFWFieldTrialServiceData *service_data_p);

static bool AddUploadParams (ServiceData *data_p, ParameterSet *param_set_p);

static bool GetCollectionName (ParameterSet *param_set_p, DFWFieldTrialServiceData *data_p, const char **collection_name_ss, DFWFieldTrialData *collection_type_p);


static json_t *ConvertToResource (const size_t i, json_t *src_record_p);


static ServiceMetadata *GetDFWFieldTrialServiceMetadata (Service *service_p);

/*
static const char *InsertFieldData (MongoTool *tool_p, json_t *values_p, DFWFieldTrialServiceData *data_p);
static const char *InsertPlotData (MongoTool *tool_p, json_t *values_p, DFWFieldTrialServiceData *data_p);
static const char *InsertDrillingData (MongoTool *tool_p, json_t *values_p, DFWFieldTrialServiceData *data_p);
static const char *InsertRawPhenotypeData (MongoTool *tool_p, json_t *values_p, DFWFieldTrialServiceData *data_p);
static const char *InsertCorrectedPhenotypeData (MongoTool *tool_p, json_t *values_p, DFWFieldTrialServiceData *data_p);
*/

/*
 * API FUNCTIONS
 */


ServicesArray *GetServices (UserDetails *user_p)
{
	ServicesArray *services_p = AllocateServicesArray (1);

	if (services_p)
		{
			Service *service_p = GetDFWFieldTrialService ();

			if (service_p)
				{
					* (services_p -> sa_services_pp) = service_p;
					return services_p;
				}

			FreeServicesArray (services_p);
		}

	return NULL;
}


void ReleaseServices (ServicesArray *services_p)
{
	FreeServicesArray (services_p);
}


static json_t *GetDFWFieldTrialServiceResults (Service *service_p, const uuid_t job_id)
{
	DFWFieldTrialServiceData *data_p = (DFWFieldTrialServiceData *) (service_p -> se_data_p);
	ServiceJob *job_p = GetServiceJobFromServiceJobSetById (service_p -> se_jobs_p, job_id);
	json_t *res_p = NULL;

	if (job_p)
		{
			if (job_p -> sj_status == OS_SUCCEEDED)
				{
					json_error_t error;
					//const char *buffer_data_p = GetCurlToolData (data_p -> wsd_curl_data_p);
					//res_p = json_loads (buffer_data_p, 0, &error);
				}
		}		/* if (job_p) */

	return res_p;
}



static Service *GetDFWFieldTrialService (void)
{
	Service *service_p = (Service *) AllocMemory (sizeof (Service));

	if (service_p)
		{
			DFWFieldTrialServiceData *data_p = AllocateDFWFieldTrialServiceData ();

			if (data_p)
				{
					if (InitialiseService (service_p,
														 GetDFWFieldTrialServiceName,
														 GetDFWFieldTrialServiceDesciption,
														 GetDFWFieldTrialServiceInformationUri,
														 RunDFWFieldTrialService,
														 IsResourceForDFWFieldTrialService,
														 GetDFWFieldTrialServiceParameters,
														 ReleaseDFWFieldTrialServiceParameters,
														 CloseDFWFieldTrialService,
														 NULL,
														 false,
														 SY_SYNCHRONOUS,
														 (ServiceData *) data_p,
														 GetDFWFieldTrialServiceMetadata))
						{

							if (ConfigureDFWFieldTrialService (data_p))
								{
									* (s_data_names_pp + DFTD_FIELD) = DFT_FIELD_S;
									* (s_data_names_pp + DFTD_PLOT) = DFT_PLOT_S;
									* (s_data_names_pp + DFTD_DRILLING) = DFT_DRILLING_S;
									* (s_data_names_pp + DFTD_RAW_PHENOTYPE) = DFT_RAW_PHENOTYPE_S;
									* (s_data_names_pp + DFTD_CORRECTED_PHENOTYPE) = DFT_CORRECTED_PHENOTYPE_S;

									return service_p;
								}

						}		/* if (InitialiseService (.... */

					FreeDFWFieldTrialServiceData (data_p);
				}

			FreeMemory (service_p);
		}		/* if (service_p) */

	return NULL;
}


static bool ConfigureDFWFieldTrialService (DFWFieldTrialServiceData *data_p)
{
	bool success_flag = false;
	const json_t *service_config_p = data_p -> dftsd_base_data.sd_config_p;

	data_p -> dftsd_database_s = GetJSONString (service_config_p, "database");

	if (data_p -> dftsd_database_s)
		{
			if ((* (data_p -> dftsd_collection_ss + DFTD_FIELD) = GetJSONString (service_config_p, "field_collection")) != NULL)
				{
					if ((* (data_p -> dftsd_collection_ss + DFTD_PLOT) = GetJSONString (service_config_p, "plot_collection")) != NULL)
						{
							if ((* (data_p -> dftsd_collection_ss + DFTD_DRILLING) = GetJSONString (service_config_p, "drilling_collection")) != NULL)
								{
									if ((* (data_p -> dftsd_collection_ss + DFTD_RAW_PHENOTYPE) = GetJSONString (service_config_p, "raw_phenotype_collection")) != NULL)
										{
											if ((* (data_p -> dftsd_collection_ss + DFTD_CORRECTED_PHENOTYPE) = GetJSONString (service_config_p, "corrected_phenotype_collection")) != NULL)
												{
													success_flag = true;
												}
										}
								}
						}
				}

		} /* if (data_p -> psd_database_s) */

	return success_flag;
}


static DFWFieldTrialServiceData *AllocateDFWFieldTrialServiceData (void)
{
	MongoTool *mongo_p = AllocateMongoTool ();

	if (mongo_p)
		{
			DFWFieldTrialServiceData *data_p = (DFWFieldTrialServiceData *) AllocMemory (sizeof (DFWFieldTrialServiceData));

			if (data_p)
				{
					data_p -> dftsd_mongo_p = mongo_p;
					data_p -> dftsd_database_s = NULL;

					memset (data_p -> dftsd_collection_ss, 0, DFTD_NUM_TYPES * sizeof (const char *));

					return data_p;
				}

			FreeMongoTool (mongo_p);
		}

	return NULL;
}


static void FreeDFWFieldTrialServiceData (DFWFieldTrialServiceData *data_p)
{
	FreeMongoTool (data_p -> dftsd_mongo_p);

	FreeMemory (data_p);
}


static const char *GetDFWFieldTrialServiceName (Service * UNUSED_PARAM (service_p))
{
	return "DFWFieldTrial Geoservice";
}


static const char *GetDFWFieldTrialServiceDesciption (Service * UNUSED_PARAM (service_p))
{
	return "A service to analyse the spread of Wheat-related diseases both geographically and temporally";
}


static const char *GetDFWFieldTrialServiceInformationUri (Service * UNUSED_PARAM (service_p))
{
	return NULL;
}


static ParameterSet *GetDFWFieldTrialServiceParameters (Service *service_p, Resource * UNUSED_PARAM (resource_p), UserDetails * UNUSED_PARAM (user_p))
{
	ParameterSet *params_p  = AllocateParameterSet ("DFWFieldTrial service parameters", "The parameters used for the DFWFieldTrial service");

	if (params_p)
		{
			ServiceData *service_data_p = service_p -> se_data_p;
			Parameter *param_p = NULL;
			SharedType def;

			def.st_json_p = NULL;

			if ((param_p = EasyCreateAndAddParameterToParameterSet (service_data_p, params_p, NULL, PGS_UPDATE.npt_type, PGS_UPDATE.npt_name_s, "Update", "Add data to the system", def, PL_ADVANCED)) != NULL)
				{
					if ((param_p = EasyCreateAndAddParameterToParameterSet (service_data_p, params_p, NULL, PGS_QUERY.npt_type, PGS_QUERY.npt_name_s, "Search", "Find data to the system", def, PL_ALL)) != NULL)
						{
							if ((param_p = EasyCreateAndAddParameterToParameterSet (service_data_p, params_p, NULL, PGS_REMOVE.npt_type, PGS_REMOVE.npt_name_s, "Delete", "Delete data to the system", def, PL_ADVANCED)) != NULL)
								{
									def.st_boolean_value = false;

									if ((param_p = EasyCreateAndAddParameterToParameterSet (service_data_p, params_p, NULL, PGS_DUMP.npt_type, PGS_DUMP.npt_name_s, "Dump", "Get all of the data in the system", def, PL_INTERMEDIATE | PL_ADVANCED)) != NULL)
										{
											if ((param_p = EasyCreateAndAddParameterToParameterSet (service_data_p, params_p, NULL, PGS_COLLECTION.npt_type, PGS_COLLECTION.npt_name_s, "Collection", "The collection to act upon", def, PL_ALL)) != NULL)
												{
													bool success_flag = true;
													uint32 i;

													for (i = 0; i < DFTD_NUM_TYPES; ++ i)
														{
															def.st_string_value_s = (char *) s_data_names_pp [i];

															if (!CreateAndAddParameterOptionToParameter (param_p, def, NULL))
																{
																	i = DFTD_NUM_TYPES;
																	success_flag = false;
																}
														}

													if (success_flag)
														{
															if (AddUploadParams (service_p -> se_data_p, params_p))
																{
																	return params_p;
																}

														}
												}
										}
								}
						}
				}

			FreeParameterSet (params_p);
		}

	return NULL;
}


static bool AddUploadParams (ServiceData *data_p, ParameterSet *param_set_p)
{
	bool success_flag = false;
	Parameter *param_p = NULL;
	SharedType def;
	ParameterGroup *group_p = CreateAndAddParameterGroupToParameterSet ("Spreadsheet Import Parameters", NULL, false, data_p, param_set_p);

	def.st_char_value = S_DEFAULT_COLUMN_DELIMITER;

	if ((param_p = CreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, PGS_DELIMITER.npt_type, false, PGS_DELIMITER.npt_name_s, "Delimiter", "The character delimiting columns", NULL, def, NULL, NULL, PL_ALL, NULL)) != NULL)
		{
			def.st_string_value_s = NULL;

			if ((param_p = CreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, PGS_FILE.npt_type, false, PGS_FILE.npt_name_s, "Data to upload", "The data to upload", NULL, def, NULL, NULL, PL_ALL, NULL)) != NULL)
				{
					success_flag = true;
				}
		}

	return success_flag;
}


static void ReleaseDFWFieldTrialServiceParameters (Service * UNUSED_PARAM (service_p), ParameterSet *params_p)
{
	FreeParameterSet (params_p);
}




static bool CloseDFWFieldTrialService (Service *service_p)
{
	bool success_flag = true;

	FreeDFWFieldTrialServiceData ((DFWFieldTrialServiceData *) (service_p -> se_data_p));;

	return success_flag;
}


static bool GetCollectionName (ParameterSet *param_set_p, DFWFieldTrialServiceData *data_p, const char **collection_name_ss, DFWFieldTrialData *collection_type_p)
{
	SharedType value;

	if (GetParameterValueFromParameterSet (param_set_p, PGS_COLLECTION.npt_name_s, &value, true))
		{
			const char *collection_s = value.st_string_value_s;

			if (collection_s)
				{
					uint32 i;

					for (i = 0; i < DFTD_NUM_TYPES; ++ i)
						{
							if (strcmp (collection_s, * (s_data_names_pp + i)) == 0)
								{
									*collection_name_ss = * ((data_p -> dftsd_collection_ss) + i);
									*collection_type_p = (DFWFieldTrialData) i;

									return true;
								}
						}		/* for (i = 0; i < PD_NUM_TYPES; ++ i) */
				}
		}

	return false;
}


static ServiceJobSet *RunDFWFieldTrialService (Service *service_p, ParameterSet *param_set_p, UserDetails * UNUSED_PARAM (user_p), ProvidersStateTable * UNUSED_PARAM (providers_p))
{
	DFWFieldTrialServiceData *data_p = (DFWFieldTrialServiceData *) (service_p -> se_data_p);

	service_p -> se_jobs_p = AllocateSimpleServiceJobSet (service_p, NULL, "DFWFieldTrial");

	if (service_p -> se_jobs_p)
		{
			ServiceJob *job_p = GetServiceJobFromServiceJobSet (service_p -> se_jobs_p, 0);

			LogParameterSet (param_set_p, job_p);

			SetServiceJobStatus (job_p, OS_FAILED_TO_START);

			if (param_set_p)
				{

				}		/* if (param_set_p) */

#if DFW_FIELD_TRIAL_SERVICE_DEBUG >= STM_LEVEL_FINE
			PrintJSONToLog (STM_LEVEL_FINE, __FILE__, __LINE__, job_p -> sj_metadata_p, "metadata 3: ");
#endif

			LogServiceJob (job_p);
		}		/* if (service_p -> se_jobs_p) */

	return service_p -> se_jobs_p;
}




static json_t *ConvertToResource (const size_t i, json_t *src_record_p)
{
	json_t *resource_p = NULL;
	char *title_s = ConvertUnsignedIntegerToString (i);

	if (title_s)
		{
			resource_p = GetResourceAsJSONByParts (PROTOCOL_INLINE_S, NULL, title_s, src_record_p);

			FreeCopiedString (title_s);
		}		/* if (raw_result_p) */

	return resource_p;
}


static OperationStatus SearchData (MongoTool *tool_p, ServiceJob *job_p, json_t *data_p, const DFWFieldTrialData collection_type, DFWFieldTrialServiceData * UNUSED_PARAM (service_data_p), const bool preview_flag)
{
	OperationStatus status = OS_FAILED;
	json_t *values_p = json_object_get (data_p, MONGO_OPERATION_DATA_S);

	if (values_p)
		{
			const char **fields_ss = NULL;
			json_t *fields_p = json_object_get (data_p, MONGO_OPERATION_FIELDS_S);

			if (fields_p)
				{
					if (json_is_array (fields_p))
						{
							size_t size = json_array_size (fields_p);

							fields_ss = (const char **) AllocMemoryArray (size + 1, sizeof (const char *));

							if (fields_ss)
								{
									const char **field_ss = fields_ss;
									size_t i;
									json_t *field_p;

									json_array_foreach (fields_p, i, field_p)
										{
											if (json_is_string (field_p))
												{
													*field_ss = json_string_value (field_p);
													++ field_ss;
												}
											else
												{
													char *dump_s = json_dumps (field_p, JSON_INDENT (2));

													PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to get field from %s", dump_s);
													free (dump_s);
												}
										}

								}		/* if (fields_ss) */

						}		/* if (json_is_array (fields_p)) */

				}		/* if (fields_p) */

			if (FindMatchingMongoDocumentsByJSON (tool_p, values_p, fields_ss))
				{
					json_t *raw_results_p = GetAllExistingMongoResultsAsJSON (tool_p);

					if (raw_results_p)
						{
							if (json_is_array (raw_results_p))
								{
									const size_t size = json_array_size (raw_results_p);
									size_t i = 0;
									json_t *raw_result_p = NULL;
									char *date_s = NULL;
									bool success_flag = true;

									/*
									 * If we are on the public view, we need to filter
									 * out entries that don't meet the live dates.
									 */
									if (!preview_flag)
										{
											struct tm current_time;

											if (GetCurrentTime (&current_time))
												{
													date_s = GetTimeAsString (&current_time);

													if (!date_s)
														{
															PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to convert time to a string");
															success_flag = false;
														}
												}
											else
												{
													PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get current time");
													success_flag = false;
												}

										}		/* if (!private_view_flag) */

									if (success_flag)
										{
											for (i = 0; i < size; ++ i)
												{
													raw_result_p = json_array_get (raw_results_p, i);
													char *title_s = ConvertUnsignedIntegerToString (i + 1);

													if (!title_s)
														{
															PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to convert " SIZET_FMT " to string for title", i + 1);
														}

													/* We don't need to return the internal mongo id so remove it */
													json_object_del (raw_result_p, MONGO_ID_S);

													if (raw_result_p)
														{
															json_t *resource_p = GetResourceAsJSONByParts (PROTOCOL_INLINE_S, NULL, title_s, raw_result_p);

															if (resource_p)
																{
																	if (!AddResultToServiceJob (job_p, resource_p))
																		{
																			AddErrorToServiceJob (job_p, title_s, "Failed to add result");

																			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add json resource for " SIZET_FMT " to results array", i);
																			json_decref (resource_p);
																		}
																}		/* if (resource_p) */
															else
																{
																	PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to create json resource for " SIZET_FMT, i);
																}

														}		/* if (raw_result_p) */

													if (title_s)
														{
															FreeCopiedString (title_s);
														}
												}		/* for (i = 0; i < size; ++ i) */


											i = GetNumberOfServiceJobResults (job_p);
											status = (i == json_array_size (raw_results_p)) ? OS_SUCCEEDED : OS_PARTIALLY_SUCCEEDED;
										}		/* if (success_flag) */

									if (date_s)
										{
											FreeCopiedString (date_s);
										}

								}		/* if (json_is_array (raw_results_p)) */
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Search results is not an array");
								}

							json_decref (raw_results_p);
						}		/* if (raw_results_p) */
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Couldn't get raw results from search");
						}
				}		/* if (FindMatchingMongoDocumentsByJSON (tool_p, values_p, fields_ss)) */
			else
				{
#if DFW_FIELD_TRIAL_SERVICE_DEBUG >= STM_LEVEL_FINE
					PrintJSONToLog (STM_LEVEL_SEVERE, __FILE__, __LINE__, values_p, "No results found for ");
#endif
				}

			if (fields_ss)
				{
					FreeMemory (fields_ss);
				}
		}		/* if (values_p) */


#if DFW_FIELD_TRIAL_SERVICE_DEBUG >= STM_LEVEL_FINE
	PrintJSONToLog (STM_LEVEL_SEVERE, __FILE__, __LINE__, job_p -> sj_result_p, "results_p: ");
#endif

	SetServiceJobStatus (job_p, status);

	return status;
}


static char *CheckDataIsValid (const json_t *row_p, DFWFieldTrialServiceData *data_p)
{
	char *errors_s = NULL;

	return errors_s;
}


/*
bool AddErrorMessage (json_t *errors_p, const json_t *values_p, const size_t row, const char * const error_s)
{
	bool success_flag = false;
	const char *pathogenomics_id_s = GetJSONString (values_p, PG_ID_S);

	if (pathogenomics_id_s)
		{
			json_error_t error;
			json_t *error_p = json_pack_ex (&error, 0, "{s:s,s:i,s:s}", "ID", pathogenomics_id_s, "row", row, "error", error_s);

			if (error_p)
				{
					if (json_array_append_new (errors_p, error_p) == 0)
						{
							success_flag = true;
						}
				}
		}

#if DFW_FIELD_TRIAL_SERVICE_DEBUG >= STM_LEVEL_FINE
	PrintJSONToLog (STM_LEVEL_FINE, __FILE__, __LINE__, errors_p, "errors data: ");
#endif

	return success_flag;
}
*/


static uint32 InsertData (MongoTool *tool_p, ServiceJob *job_p, json_t *values_p, const DFWFieldTrialData collection_type, DFWFieldTrialServiceData *data_p)
{
	uint32 num_imports = 0;
	const char *(*insert_fn) (MongoTool *tool_p, json_t *values_p, DFWFieldTrialServiceData *data_p) = NULL;

	#if DFW_FIELD_TRIAL_SERVICE_DEBUG >= STM_LEVEL_FINE
	PrintJSONToLog (STM_LEVEL_FINE, __FILE__, __LINE__, values_p, "values_p: ");
	#endif

	/*
	switch (collection_type)
		{
			case DFTD_FIELD:
				insert_fn = InsertFieldData;
				break;

			case DFTD_PLOT:
				insert_fn = InsertPlotData;
				break;

			case DFTD_DRILLING:
				insert_fn = InsertDrillingData;
				break;

			case DFTD_RAW_PHENOTYPE:
				insert_fn = InsertRawPhenotypeData;
				break;

			case DFTD_CORRECTED_PHENOTYPE:
				insert_fn = InsertCorrectedPhenotypeData;
				break;

			default:
				break;
		}
*/

	if (insert_fn)
		{
			if (json_is_array (values_p))
				{
					json_t *value_p;
					size_t i;

					json_array_foreach (values_p, i, value_p)
						{
							const char *error_s = insert_fn (tool_p, value_p, data_p);

							if (error_s)
								{
									AddErrorMessage (job_p, value_p, error_s, i);
								}
							else
								{
									++ num_imports;
								}
						}
				}
			else
				{
					const char *error_s = insert_fn (tool_p, values_p, data_p);

					if (error_s)
						{
							AddErrorMessage (job_p, values_p, error_s, 0);

							PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "%s", error_s);
						}
					else
						{
							++ num_imports;
						}
				}
		}


	return num_imports;
}


bool AddErrorMessage (ServiceJob *job_p, const json_t *value_p, const char *error_s, const int index)
{
	char *dump_s = json_dumps (value_p, JSON_INDENT (2) | JSON_PRESERVE_ORDER);
	const char *id_s = GetJSONString (value_p, "id");
	bool added_error_flag = false;


	if (id_s)
		{
			added_error_flag = AddErrorToServiceJob (job_p, id_s, error_s);
		}
	else
		{
			char *index_s = GetIntAsString (index);

			if (index_s)
				{
					char *row_s = ConcatenateStrings ("row ", index_s);

					if (row_s)
						{
							added_error_flag = AddErrorToServiceJob (job_p, row_s, error_s);

							FreeCopiedString (row_s);
						}

					FreeCopiedString (index_s);
				}

		}

	if (!added_error_flag)
		{
			PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "failed to add %s to client feedback messsage", error_s);
		}


	if (dump_s)
		{
			PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to import \"%s\": error=%s", dump_s, error_s);
			free (dump_s);
		}
	else
		{
			PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to import error=%s", dump_s, error_s);
		}

	return added_error_flag;
}


static uint32 DeleteData (MongoTool *tool_p, ServiceJob * UNUSED_PARAM (job_p), json_t *data_p, const DFWFieldTrialData UNUSED_PARAM (collection_type), DFWFieldTrialServiceData * UNUSED_PARAM (service_data_p))
{
	bool success_flag = false;
	json_t *selector_p = json_object_get (data_p, MONGO_OPERATION_DATA_S);

	if (selector_p)
		{
			success_flag = RemoveMongoDocuments (tool_p, selector_p, false);
		}		/* if (values_p) */

	return success_flag ? 1 : 0;
}


static ParameterSet *IsResourceForDFWFieldTrialService (Service * UNUSED_PARAM (service_p), Resource * UNUSED_PARAM (resource_p), Handler * UNUSED_PARAM (handler_p))
{
	return NULL;
}


static ServiceMetadata *GetDFWFieldTrialServiceMetadata (Service *service_p)
{
	const char *term_url_s = CONTEXT_PREFIX_EDAM_ONTOLOGY_S "topic_0625";
	SchemaTerm *category_p = AllocateSchemaTerm (term_url_s, "Genotype and phenotype",
		"The study of genetic constitution of a living entity, such as an individual, and organism, a cell and so on, "
		"typically with respect to a particular observable phenotypic traits, or resources concerning such traits, which "
		"might be an aspect of biochemistry, physiology, morphology, anatomy, development and so on.");

	if (category_p)
		{
			SchemaTerm *subcategory_p;

			term_url_s = CONTEXT_PREFIX_EDAM_ONTOLOGY_S "operation_0304";
			subcategory_p = AllocateSchemaTerm (term_url_s, "Query and retrieval", "Search or query a data resource and retrieve entries and / or annotation.");

			if (subcategory_p)
				{
					ServiceMetadata *metadata_p = AllocateServiceMetadata (category_p, subcategory_p);

					if (metadata_p)
						{
							SchemaTerm *input_p;

							term_url_s = CONTEXT_PREFIX_EDAM_ONTOLOGY_S "data_0968";
							input_p = AllocateSchemaTerm (term_url_s, "Keyword",
								"Boolean operators (AND, OR and NOT) and wildcard characters may be allowed. Keyword(s) or phrase(s) used (typically) for text-searching purposes.");

							if (input_p)
								{
									if (AddSchemaTermToServiceMetadataInput (metadata_p, input_p))
										{
											SchemaTerm *output_p;

											/* Place */
											term_url_s = CONTEXT_PREFIX_SCHEMA_ORG_S "Place";
											output_p = AllocateSchemaTerm (term_url_s, "Place", "Entities that have a somewhat fixed, physical extension.");

											if (output_p)
												{
													if (AddSchemaTermToServiceMetadataOutput (metadata_p, output_p))
														{
															/* Date */
															term_url_s = CONTEXT_PREFIX_SCHEMA_ORG_S "Date";
															output_p = AllocateSchemaTerm (term_url_s, "Date", "A date value in ISO 8601 date format.");

															if (output_p)
																{
																	if (AddSchemaTermToServiceMetadataOutput (metadata_p, output_p))
																		{
																			/* Pathogen */
																			term_url_s = CONTEXT_PREFIX_EXPERIMENTAL_FACTOR_ONTOLOGY_S "EFO_0000643";
																			output_p = AllocateSchemaTerm (term_url_s, "pathogen", "A biological agent that causes disease or illness to its host.");

																			if (output_p)
																				{
																					if (AddSchemaTermToServiceMetadataOutput (metadata_p, output_p))
																						{
																							/* Phenotype */
																							term_url_s = CONTEXT_PREFIX_EXPERIMENTAL_FACTOR_ONTOLOGY_S "EFO_0000651";
																							output_p = AllocateSchemaTerm (term_url_s, "phenotype", "The observable form taken by some character (or group of characters) "
																								"in an individual or an organism, excluding pathology and disease. The detectable outward manifestations of a specific genotype.");

																							if (output_p)
																								{
																									if (AddSchemaTermToServiceMetadataOutput (metadata_p, output_p))
																										{
																											/* Genotype */
																											term_url_s = CONTEXT_PREFIX_EXPERIMENTAL_FACTOR_ONTOLOGY_S "EFO_0000513";
																											output_p = AllocateSchemaTerm (term_url_s, "genotype", "Information, making the distinction between the actual physical material "
																												"(e.g. a cell) and the information about the genetic content (genotype).");

																											if (output_p)
																												{
																													if (AddSchemaTermToServiceMetadataOutput (metadata_p, output_p))
																														{
																															return metadata_p;
																														}		/* if (AddSchemaTermToServiceMetadataOutput (metadata_p, output_p)) */
																													else
																														{
																															PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add output term %s to service metadata", term_url_s);
																															FreeSchemaTerm (output_p);
																														}

																												}		/* if (output_p) */
																											else
																												{
																													PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate output term %s for service metadata", term_url_s);
																												}
																										}		/* if (AddSchemaTermToServiceMetadataOutput (metadata_p, output_p)) */
																									else
																										{
																											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add output term %s to service metadata", term_url_s);
																											FreeSchemaTerm (output_p);
																										}

																								}		/* if (output_p) */
																							else
																								{
																									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate output term %s for service metadata", term_url_s);
																								}

																						}		/* if (AddSchemaTermToServiceMetadataOutput (metadata_p, output_p)) */
																					else
																						{
																							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add output term %s to service metadata", term_url_s);
																							FreeSchemaTerm (output_p);
																						}

																				}		/* if (output_p) */
																			else
																				{
																					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate output term %s for service metadata", term_url_s);
																				}

																		}		/* if (AddSchemaTermToServiceMetadataOutput (metadata_p, output_p)) */
																	else
																		{
																			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add output term %s to service metadata", term_url_s);
																			FreeSchemaTerm (output_p);
																		}

																}		/* if (output_p) */
															else
																{
																	PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate output term %s for service metadata", term_url_s);
																}


														}		/* if (AddSchemaTermToServiceMetadataOutput (metadata_p, output_p)) */
													else
														{
															PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add output term %s to service metadata", term_url_s);
															FreeSchemaTerm (output_p);
														}

												}		/* if (output_p) */
											else
												{
													PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate output term %s for service metadata", term_url_s);
												}

										}		/* if (AddSchemaTermToServiceMetadataInput (metadata_p, input_p)) */
									else
										{
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add input term %s to service metadata", term_url_s);
											FreeSchemaTerm (input_p);
										}

								}		/* if (input_p) */
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate input term %s for service metadata", term_url_s);
								}

						}		/* if (metadata_p) */
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate service metadata");
						}

				}		/* if (subcategory_p) */
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate sub-category term %s for service metadata", term_url_s);
				}

		}		/* if (category_p) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate category term %s for service metadata", term_url_s);
		}

	return NULL;
}



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

#include "field_trial_jobs.h"
#include "experimental_area_jobs.h"
#include "location_jobs.h"
#include "plot_jobs.h"


#ifdef _DEBUG
#define DFW_FIELD_TRIAL_SERVICE_DEBUG	(STM_LEVEL_FINER)
#else
#define DFW_FIELD_TRIAL_SERVICE_DEBUG	(STM_LEVEL_NONE)
#endif



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


static json_t *GetDFWFieldTrialServiceResults (Service *service_p, const uuid_t job_id);


static bool CloseDFWFieldTrialService (Service *service_p);


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
									return service_p;
								}

						}		/* if (InitialiseService (.... */

					FreeDFWFieldTrialServiceData (data_p);
				}

			FreeMemory (service_p);
		}		/* if (service_p) */

	return NULL;
}


static const char *GetDFWFieldTrialServiceName (Service * UNUSED_PARAM (service_p))
{
	return "DFWFieldTrial service";
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
			ServiceData *data_p = service_p -> se_data_p;

			if (AddFieldTrialParams (data_p, params_p))
				{
					if (AddExperimentalAreaParams (data_p, params_p))
						{
							if (AddLocationParams (data_p, params_p))
								{
									if (AddPlotParams (service_p -> se_data_p, params_p))
										{
											return params_p;
										}
								}
						}
				}

			FreeParameterSet (params_p);
		}

	return NULL;
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
					if (!RunForFieldTrialParams (data_p, param_set_p, job_p))
						{
							if (!RunForExperimentalAreaParams (data_p, param_set_p, job_p))
								{
									if (!RunForLocationParams (data_p, param_set_p, job_p))
										{
											if (!RunForPlotParams (data_p, param_set_p, job_p))
												{

												}		/* if (!RunForPlotParams (data_p, param_set_p, job_p)) */

										}		/* if (!RunForLocationParams (data_p, param_set_p, job_p)) */

								}		/* if (!RunForExperimentalAreaParams (data_p, param_set_p, job_p)) */

						}		/* if (!RunForFieldTrialParams (data_p, param_set_p, job_p)) */

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



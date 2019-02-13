/*
** Copyright 2014-2018 The Earlham Institute
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
 * search_service.c
 *
 *  Created on: 24 Oct 2018
 *      Author: billy
 */

#include "submission_service.h"
#include "plot_jobs.h"
#include "field_trial_jobs.h"
#include "study_jobs.h"
#include "material_jobs.h"
#include "location_jobs.h"
#include "gene_bank_jobs.h"

#include "audit.h"
#include "streams.h"
#include "math_utils.h"
#include "string_utils.h"

#include "lucene_tool.h"
#include "key_value_pair.h"

/*
 * Static declarations
 */


static NamedParameterType S_KEYWORD = { "FT Keyword Search", PT_KEYWORD };


static const char *GetDFWFieldTrialSearchServiceName (Service *service_p);

static const char *GetDFWFieldTrialSearchServiceDesciption (Service *service_p);

static const char *GetDFWFieldTrialSearchServiceInformationUri (Service *service_p);

static ParameterSet *GetDFWFieldTrialSearchServiceParameters (Service *service_p, Resource *resource_p, UserDetails *user_p);

static bool GetDFWFieldTrialSearchServiceParameterTypesForNamedParameters (struct Service *service_p, const char *param_name_s, ParameterType *pt_p);


static void ReleaseDFWFieldTrialSearchServiceParameters (Service *service_p, ParameterSet *params_p);

static ServiceJobSet *RunDFWFieldTrialSearchService (Service *service_p, ParameterSet *param_set_p, UserDetails *user_p, ProvidersStateTable *providers_p);

static ParameterSet *IsResourceForDFWFieldTrialSearchService (Service *service_p, Resource *resource_p, Handler *handler_p);

static bool CloseDFWFieldTrialSearchService (Service *service_p);

static ServiceMetadata *GetDFWFieldTrialSearchServiceMetadata (Service *service_p);

static void SearchFieldTrialsForKeyword (const char *keyword_s, ServiceJob *job_p, DFWFieldTrialServiceData *data_p);

static bool GetIdsFromLuceneResults (LuceneDocument *document_p, const uint32 index, void *data_p);


/*
 * API definitions
 */


Service *GetDFWFieldTrialSearchService (void)
{
	Service *service_p = (Service *) AllocMemory (sizeof (Service));

	if (service_p)
		{
			DFWFieldTrialServiceData *data_p = AllocateDFWFieldTrialServiceData ();

			if (data_p)
				{
					if (InitialiseService (service_p,
														 GetDFWFieldTrialSearchServiceName,
														 GetDFWFieldTrialSearchServiceDesciption,
														 GetDFWFieldTrialSearchServiceInformationUri,
														 RunDFWFieldTrialSearchService,
														 IsResourceForDFWFieldTrialSearchService,
														 GetDFWFieldTrialSearchServiceParameters,
														 GetDFWFieldTrialSearchServiceParameterTypesForNamedParameters,
														 ReleaseDFWFieldTrialSearchServiceParameters,
														 CloseDFWFieldTrialSearchService,
														 NULL,
														 false,
														 SY_SYNCHRONOUS,
														 (ServiceData *) data_p,
														 GetDFWFieldTrialSearchServiceMetadata))
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



static const char *GetDFWFieldTrialSearchServiceName (Service * UNUSED_PARAM (service_p))
{
	return "DFWFieldTrial search service";
}


static const char *GetDFWFieldTrialSearchServiceDesciption (Service * UNUSED_PARAM (service_p))
{
	return "A service to search field trial data";
}


static const char *GetDFWFieldTrialSearchServiceInformationUri (Service * UNUSED_PARAM (service_p))
{
	return NULL;
}


static ParameterSet *GetDFWFieldTrialSearchServiceParameters (Service *service_p, Resource * UNUSED_PARAM (resource_p), UserDetails * UNUSED_PARAM (user_p))
{
	ParameterSet *params_p = AllocateParameterSet ("DFWFieldTrial search service parameters", "The parameters used for the DFWFieldTrial search service");

	if (params_p)
		{
			ServiceData *data_p = service_p -> se_data_p;
			ParameterGroup *group_p = NULL;

			Parameter *param_p = NULL;
			SharedType def;

			def.st_string_value_s = NULL;

			if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, params_p, group_p, S_KEYWORD.npt_type, S_KEYWORD.npt_name_s, "Search", "Search the field trial data", def, PL_SIMPLE)) != NULL)
				{
					if (AddSearchFieldTrialParams (data_p, params_p))
						{
							if (AddSearchStudyParams (data_p, params_p))
								{
									if (AddSearchLocationParams (data_p, params_p))
										{
											return params_p;
										}
									else
										{
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "AddSearchLocationParams failed");
										}
								}
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "AddSearchStudyParams failed");
								}
						}
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "AddSearchFieldTrialParams failed");
						}

				}		/* if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, params_p, NULL, S_KEYWORD.npt_type, S_KEYWORD.npt_name_s, "Search", "Search the field trial data", def, PL_SIMPLE)) != NULL) */
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", S_KEYWORD.npt_name_s);
				}


			FreeParameterSet (params_p);
		}
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate %s ParameterSet", GetDFWFieldTrialSearchServiceName (service_p));
		}

	return NULL;
}


static bool GetDFWFieldTrialSearchServiceParameterTypesForNamedParameters (struct Service *service_p, const char *param_name_s, ParameterType *pt_p)
{
	bool success_flag = true;

	if (strcmp (param_name_s, S_KEYWORD.npt_name_s) == 0)
		{
			*pt_p = S_KEYWORD.npt_type;
		}
	else
		{
			if (!GetSearchFieldTrialParameterTypeForNamedParameter (param_name_s, pt_p))
				{
					if (!GetSearchStudyParameterTypeForNamedParameter (param_name_s, pt_p))
						{
							if (!GetSearchLocationParameterTypeForNamedParameter (param_name_s, pt_p))
								{
									success_flag = false;
								}		/* if (!GetSearchLocationParameterTypeForNamedParameter (param_name_s, pt_p)) */

						}		/* if (!GetSearchStudyParameterTypeForNamedParameter (param_name_s, pt_p)) */

				}		/* if (!GetSearchFieldTrialParameterTypeForNamedParameter (param_name_s, pt_p)) */
		}

	return success_flag;
}



static void ReleaseDFWFieldTrialSearchServiceParameters (Service * UNUSED_PARAM (service_p), ParameterSet *params_p)
{
	FreeParameterSet (params_p);
}


static bool CloseDFWFieldTrialSearchService (Service *service_p)
{
	bool success_flag = true;

	FreeDFWFieldTrialServiceData ((DFWFieldTrialServiceData *) (service_p -> se_data_p));;

	return success_flag;
}


static ServiceJobSet *RunDFWFieldTrialSearchService (Service *service_p, ParameterSet *param_set_p, UserDetails * UNUSED_PARAM (user_p), ProvidersStateTable * UNUSED_PARAM (providers_p))
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
					/*
					 * check for simple search first
					 */
					bool ran_flag = false;
					SharedType value;

					InitSharedType (&value);

					if (GetParameterValueFromParameterSet (param_set_p, S_KEYWORD.npt_name_s, &value, true))
						{
							if (!IsStringEmpty (value.st_string_value_s))
								{
									SearchFieldTrialsForKeyword (value.st_string_value_s, job_p, data_p);
									ran_flag = true;
								}		/* if (!IsStringEmpty (value.st_string_value_s)) */

						}		/* if (GetParameterValueFromParameterSet (param_set_p, S_KEYWORD.npt_name_s, &value, true)) */


					if (!ran_flag)
						{
							/*
							 * check for the advanced search
							 */
							if (!RunForSearchFieldTrialParams (data_p, param_set_p, job_p))
								{
									if (!RunForSearchStudyParams (data_p, param_set_p, job_p))
										{
											if (!RunForSearchLocationParams (data_p, param_set_p, job_p))
												{

												}		/* if (!RunForLocationParams (data_p, param_set_p, job_p)) */

										}		/* if (!RunForStudyParams (data_p, param_set_p, job_p)) */

								}		/* if (!RunForFieldTrialParams (data_p, param_set_p, job_p)) */

						}		/* if (!ran_flag) */


				}		/* if (param_set_p) */

#if DFW_FIELD_TRIAL_SERVICE_DEBUG >= STM_LEVEL_FINE
			PrintJSONToLog (STM_LEVEL_FINE, __FILE__, __LINE__, job_p -> sj_metadata_p, "metadata 3: ");
#endif

			LogServiceJob (job_p);
		}		/* if (service_p -> se_jobs_p) */

	return service_p -> se_jobs_p;
}


static ServiceMetadata *GetDFWFieldTrialSearchServiceMetadata (Service * UNUSED_PARAM (service_p))
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


static ParameterSet *IsResourceForDFWFieldTrialSearchService (Service * UNUSED_PARAM (service_p), Resource * UNUSED_PARAM (resource_p), Handler * UNUSED_PARAM (handler_p))
{
	return NULL;
}


static void SearchFieldTrialsForKeyword (const char *keyword_s, ServiceJob *job_p, const ViewFormat fmt, DFWFieldTrialServiceData *data_p)
{
	OperationStatus status = OS_FAILED_TO_START;
	LuceneTool *lucene_p = AllocateLuceneTool (job_p -> sj_id);

	if (lucene_p)
		{
			LinkedList *facets_p = AllocateLinkedList (FreeKeyValuePairNode);

			if (facets_p)
				{
					KeyValuePairNode *facet_p = AllocateKeyValuePairNode ("type", "Field Trial");

					if (facet_p)
						{
							LinkedListAddTail (facets_p, & (facet_p -> kvpn_node));

							if (RunLuceneTool (lucene_p, keyword_s, facets_p))
								{
									ByteBuffer *buffer_p = AllocateByteBuffer (1024);

									if (buffer_p)
										{
											if (AppendStringsToByteBuffer (buffer_p, "{ ", MONGO_ID_S, " : {\"$in\" : [", NULL))
												{
													if (ParseLuceneResults (lucene_p, GetIdsFromLuceneResults, buffer_p))
														{
															if (AppendStringToByteBuffer (buffer_p, "]}}"))
																{
																	bson_t *query_p = NULL;
																	const char *data_s = GetByteBufferData (buffer_p);
																	size_t l = GetByteBufferSize (buffer_p);
																	bson_error_t err;

																	if (bson_init_from_json (query_p, data_s, l, &err))
																		{
																			json_t *docs_p = GetAllMongoResultsAsJSON (data_p -> dftsd_mongo_p, query_p, NULL);

																			if (docs_p)
																				{
																					if (json_is_array (docs_p))
																						{
																							size_t i = 0;
																							const size_t num_docs = json_array_size (docs_p);

																							for ( ; i < num_docs; ++ i)
																								{
																									json_t *doc_p = json_array_get (docs_p, i);

																									const char *type_s = GetJSONString (doc_p, "type");

																									if (type_s)
																										{
																											const char *id_s = GetJSONString (doc_p, MONGO_ID_S);

																											if (id_s)
																												{
																													DFWFieldTrialData datatype = GetDatatypeFromString (type_s);

																													switch (datatype)
																														{
																															case DFTD_FIELD_TRIAL:
																																{
																																	FieldTrial *trial_p = GetFieldTrialByIdString (id_s, fmt);

																																	if (trial_p)
																																		{
																																			//AddFieldTrialToServiceJobFromJSON()
																																		}
																																}
																																break;

																														case DFTD_STUDY:
																																{
																																	Study *study_p = GetStudyByIdString (id_s, fmt);

																																	if (study_p)
																																		{

																																		}
																																}
																																break;

																														}		/* switch (datatype) */

																												}		/* if (id_s) */

																										}		/* if (type_s) */

																								}

																						}		/* if (json_is_array (docs_p)) */

																				}		/* if (docs_p) */

																			bson_destroy (query_p);
																		}		/* if (bson_init_from_json (query_p, data_s, l, &err)) */

																}		/* if (AppendStringToByteBuffer (buffer_p, "]}}")) */

														}		/* if (ParseLuceneResults (lucene_p, GetIdsFromLuceneResults, ids_p)) */

												}

											FreeByteBuffer (buffer_p);
										}		/* if (buffer_p) */

								}		/* if (RunLuceneTool (lucene_p, keyword_s, facets_p)) */

						}		/* if (facet_p) */

					FreeLinkedList (facets_p);
				}		/* if (facets_p) */

			FreeLuceneTool (lucene_p);
		}		/* if (lucene_p) */


	SetServiceJobStatus (job_p, status);
}


static bool GetIdsFromLuceneResults (LuceneDocument *document_p, const uint32 index, void *data_p)
{
	bool success_flag = false;
	ByteBuffer *buffer_p = (ByteBuffer *) data_p;
	const char *id_s = GetDocumentFieldValue (document_p, "_id");

	if (id_s)
		{
			success_flag = true;

			if (index != 0)
				{
					success_flag = AppendStringToByteBuffer (buffer_p, ", ");
				}

			if (success_flag)
				{
					success_flag = AppendStringsToByteBuffer (buffer_p, "ObjectId(\"", id_s, "\")", NULL);
				}
		}		/* if (id_s) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get \"_id\" from document");
		}

	return success_flag;
}


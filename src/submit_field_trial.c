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
 * field_trial_service.c
 *
 *  Created on: 5 Apr 2019
 *      Author: billy
 */


#include "submit_field_trial.h"

#include "audit.h"

#include "field_trial_jobs.h"
#include "person_jobs.h"
#include "string_parameter.h"
#include "string_array_parameter.h"


/*
 * Static declarations
 */



static const char *GetFieldTrialSubmissionServiceName (const Service *service_p);

static const char *GetFieldTrialSubmissionServiceDescription (const Service *service_p);

static const char *GetFieldTrialSubmissionServiceAlias (const Service *service_p);

static const char *GetFieldTrialSubmissionServiceInformationUri (const Service *service_p);

static ParameterSet *GetFieldTrialSubmissionServiceParameters (Service *service_p, DataResource *resource_p, UserDetails *user_p);

static bool GetFieldTrialSubmissionServiceParameterTypesForNamedParameters (const Service *service_p, const char *param_name_s, ParameterType *pt_p);

static void ReleaseFieldTrialSubmissionServiceParameters (Service *service_p, ParameterSet *params_p);

static ServiceJobSet *RunFieldTrialSubmissionService (Service *service_p, ParameterSet *param_set_p, UserDetails *user_p, ProvidersStateTable *providers_p);

static ParameterSet *IsResourceForFieldTrialSubmissionService (Service *service_p, DataResource *resource_p, Handler *handler_p);

static bool CloseFieldTrialSubmissionService (Service *service_p);

static ServiceMetadata *GetFieldTrialSubmissionServiceMetadata (Service *service_p);


static Parameter *CreateSubmitTrialParameterFromJSON (struct Service *service_p, json_t *param_json_p, const bool concise_flag);


/*
 * API definitions
 */


Service *GetFieldTrialSubmissionService (GrassrootsServer *grassroots_p)
{
	Service *service_p = (Service *) AllocMemory (sizeof (Service));

	if (service_p)
		{
			FieldTrialServiceData *data_p = AllocateFieldTrialServiceData ();

			if (data_p)
				{
					if (InitialiseService (service_p,
														 GetFieldTrialSubmissionServiceName,
														 GetFieldTrialSubmissionServiceDescription,
														 GetFieldTrialSubmissionServiceAlias,
														 GetFieldTrialSubmissionServiceInformationUri,
														 RunFieldTrialSubmissionService,
														 NULL,
														 GetFieldTrialSubmissionServiceParameters,
														 GetFieldTrialSubmissionServiceParameterTypesForNamedParameters,
														 ReleaseFieldTrialSubmissionServiceParameters,
														 CloseFieldTrialSubmissionService,
														 NULL,
														 false,
														 SY_SYNCHRONOUS,
														 (ServiceData *) data_p,
														 GetFieldTrialSubmissionServiceMetadata,
														 GetFieldTrialIndexingData,
														 grassroots_p))
						{

							if (ConfigureFieldTrialService (data_p, grassroots_p))
								{
									service_p -> se_custom_parameter_decoder_fn = CreateSubmitTrialParameterFromJSON;
									
									return service_p;
								}

						}		/* if (InitialiseService (.... */

					FreeFieldTrialServiceData (data_p);
				}

			FreeMemory (service_p);
		}		/* if (service_p) */

	return NULL;
}


static const char *GetFieldTrialSubmissionServiceName (const Service * UNUSED_PARAM (service_p))
{
	return "Submit Field Trials";
}


static const char *GetFieldTrialSubmissionServiceDescription (const Service * UNUSED_PARAM (service_p))
{
	return "Add a Field Trial to the system. Following the same nomenclature as <a href='https://brapi.docs.apiary.io/'>BrAPI</a>,"
			" a Field Trial contains multiple Studies. This is equivalent to an Investigation in <a href='https://www.miappe.org/'>MIAPPE</a>.";
}


static const char *GetFieldTrialSubmissionServiceAlias (const Service * UNUSED_PARAM (service_p))
{
	return DFT_GROUP_ALIAS_PREFIX_S SERVICE_GROUP_ALIAS_SEPARATOR "submit_trial";
}


static const char *GetFieldTrialSubmissionServiceInformationUri (const Service *service_p)
{
	const char *url_s = GetServiceInformationPage (service_p);

	if (!url_s)
		{
			url_s = "https://grassroots.tools/docs/user/services/field_trial/submit_trial.md";
		}

	return url_s;
}


static bool GetFieldTrialSubmissionServiceParameterTypesForNamedParameters (const Service *service_p, const char *param_name_s, ParameterType *pt_p)
{
	return GetSubmissionFieldTrialParameterTypeForNamedParameter (param_name_s, pt_p);
}



static ParameterSet *GetFieldTrialSubmissionServiceParameters (Service *service_p, DataResource *resource_p, UserDetails * UNUSED_PARAM (user_p))
{
	ParameterSet *params_p = AllocateParameterSet ("FieldTrial submission service parameters", "The parameters used for the FieldTrial submission service");

	if (params_p)
		{
			ServiceData *data_p = service_p -> se_data_p;

			if (AddSubmissionFieldTrialParams (data_p, params_p, resource_p))
				{
					return params_p;
				}
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "AddSubmissionFieldTrialParams failed");
				}

			FreeParameterSet (params_p);
		}
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate %s ParameterSet", GetFieldTrialSubmissionServiceName (service_p));
		}

	return NULL;
}






static void ReleaseFieldTrialSubmissionServiceParameters (Service * UNUSED_PARAM (service_p), ParameterSet *params_p)
{
	FreeParameterSet (params_p);
}




static bool CloseFieldTrialSubmissionService (Service *service_p)
{
	bool success_flag = true;

	FreeFieldTrialServiceData ((FieldTrialServiceData *) (service_p -> se_data_p));

	return success_flag;
}



static ServiceJobSet *RunFieldTrialSubmissionService (Service *service_p, ParameterSet *param_set_p, UserDetails * UNUSED_PARAM (user_p), ProvidersStateTable * UNUSED_PARAM (providers_p))
{
	FieldTrialServiceData *data_p = (FieldTrialServiceData *) (service_p -> se_data_p);

	service_p -> se_jobs_p = AllocateSimpleServiceJobSet (service_p, NULL, "Submit Field Trial");

	if (service_p -> se_jobs_p)
		{
			ServiceJob *job_p = GetServiceJobFromServiceJobSet (service_p -> se_jobs_p, 0);

			LogParameterSet (param_set_p, job_p);

			SetServiceJobStatus (job_p, OS_FAILED_TO_START);

			if (!RunForSubmissionFieldTrialParams (data_p, param_set_p, job_p))
				{

				}		/* if (!RunForFieldTrialParams (data_p, param_set_p, job_p)) */


			LogServiceJob (job_p);
		}		/* if (service_p -> se_jobs_p) */

	return service_p -> se_jobs_p;
}


static ServiceMetadata *GetFieldTrialSubmissionServiceMetadata (Service *service_p)
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




static ParameterSet *IsResourceForFieldTrialSubmissionService (Service * UNUSED_PARAM (service_p), DataResource * UNUSED_PARAM (resource_p), Handler * UNUSED_PARAM (handler_p))
{
	return NULL;
}


static Parameter *CreateSubmitTrialParameterFromJSON (struct Service *service_p, json_t *param_json_p, const bool concise_flag)
{
	Parameter *param_p = NULL;
	const char *name_s = GetJSONString (param_json_p, PARAM_NAME_S);
	bool done_flag = false;
	ParameterType pt = PT_NUM_TYPES;

	if (GetPersonParameterTypeForNamedParameter (name_s, &pt))
		{
			if (pt != PT_NUM_TYPES)
				{
					json_t *current_value_p = json_object_get (param_json_p, PARAM_CURRENT_VALUE_S);

					if (current_value_p)
						{
							if (json_is_array (current_value_p))
								{
									switch (pt)
										{
											case PT_STRING:
												{
													StringArrayParameter *string_array_param_p = AllocateStringArrayParameterFromJSON (param_json_p, service_p, concise_flag, NULL);

													if (string_array_param_p)
														{
															param_p = & (string_array_param_p -> sap_base_param);
														}
												}
											break;
																						
											default:
												PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, param_json_p, "Unknown ParameterType %u", pt);
												break;
										}		/* switch (pt) */
																		
								}		/* if (json_is_array (current_value_p)) */
							else
								{
									switch (pt)
										{
											case PT_STRING:
												{
													StringParameter *string_param_p  = AllocateStringParameterFromJSON (param_json_p, service_p, concise_flag, &pt);

													if (string_param_p)
														{
															param_p = & (string_param_p -> sp_base_param);
														}
												}
											break;
																						
											default:
												PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, param_json_p, "Unknown ParameterType %u", pt);
												break;
										}		/* switch (pt) */									
								
								}		/* /* if (json_is_array (current_value_p)) */ 							
							
						}		/* if (current_value_p) */

				}		/* if (pt != PT_NUM_TYPES) */
			else
				{
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, param_json_p, "Unknown ParameterType %u", pt);
				}

		}		/* if (GetPersonParameterTypeForNamedParameter (name_s, &pt)) */
	else
		{
		}

	if (!param_p)
		{
			param_p = CreateParameterFromJSON (param_json_p, service_p, concise_flag);
		}

	return param_p;
}

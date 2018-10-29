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
 * submission_service.c
 *
 *  Created on: 22 Oct 2018
 *      Author: billy
 */


#include "submission_service.h"
#include "plot_jobs.h"
#include "field_trial_jobs.h"
#include "experimental_area_jobs.h"
#include "material_jobs.h"
#include "location_jobs.h"
#include "gene_bank_jobs.h"
#include "phenotype_jobs.h"


#include "audit.h"
#include "streams.h"
#include "math_utils.h"
#include "string_utils.h"

/*
 * Static declarations
 */



static const char *GetDFWFieldTrialSubmissionServiceName (Service *service_p);

static const char *GetDFWFieldTrialSubmissionServiceDesciption (Service *service_p);

static const char *GetDFWFieldTrialSubmissionServiceInformationUri (Service *service_p);

static ParameterSet *GetDFWFieldTrialSubmissionServiceParameters (Service *service_p, Resource *resource_p, UserDetails *user_p);

static void ReleaseDFWFieldTrialSubmissionServiceParameters (Service *service_p, ParameterSet *params_p);

static ServiceJobSet *RunDFWFieldTrialSubmissionService (Service *service_p, ParameterSet *param_set_p, UserDetails *user_p, ProvidersStateTable *providers_p);

static ParameterSet *IsResourceForDFWFieldTrialSubmissionService (Service *service_p, Resource *resource_p, Handler *handler_p);

static bool CloseDFWFieldTrialSubmissionService (Service *service_p);

static ServiceMetadata *GetDFWFieldTrialSubmissionServiceMetadata (Service *service_p);


/*
 * API definitions
 */


Service *GetDFWFieldTrialSubmissionService (void)
{
	Service *service_p = (Service *) AllocMemory (sizeof (Service));

	if (service_p)
		{
			DFWFieldTrialServiceData *data_p = AllocateDFWFieldTrialServiceData ();

			if (data_p)
				{
					if (InitialiseService (service_p,
														 GetDFWFieldTrialSubmissionServiceName,
														 GetDFWFieldTrialSubmissionServiceDesciption,
														 GetDFWFieldTrialSubmissionServiceInformationUri,
														 RunDFWFieldTrialSubmissionService,
														 IsResourceForDFWFieldTrialSubmissionService,
														 GetDFWFieldTrialSubmissionServiceParameters,
														 ReleaseDFWFieldTrialSubmissionServiceParameters,
														 CloseDFWFieldTrialSubmissionService,
														 NULL,
														 false,
														 SY_SYNCHRONOUS,
														 (ServiceData *) data_p,
														 GetDFWFieldTrialSubmissionServiceMetadata))
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



static const char *GetDFWFieldTrialSubmissionServiceName (Service * UNUSED_PARAM (service_p))
{
	return "DFWFieldTrial submission service";
}


static const char *GetDFWFieldTrialSubmissionServiceDesciption (Service * UNUSED_PARAM (service_p))
{
	return "A service to submit field trial data";
}


static const char *GetDFWFieldTrialSubmissionServiceInformationUri (Service * UNUSED_PARAM (service_p))
{
	return NULL;
}


static ParameterSet *GetDFWFieldTrialSubmissionServiceParameters (Service *service_p, Resource * UNUSED_PARAM (resource_p), UserDetails * UNUSED_PARAM (user_p))
{
	ParameterSet *params_p = AllocateParameterSet ("DFWFieldTrial submission service parameters", "The parameters used for the DFWFieldTrial submission service");

	if (params_p)
		{
			ServiceData *data_p = service_p -> se_data_p;

			if (AddSubmissionFieldTrialParams (data_p, params_p))
				{
					if (AddSubmissionExperimentalAreaParams (data_p, params_p))
						{
							if (AddSubmissionLocationParams (data_p, params_p))
								{
									if (AddSubmissionPlotParams (data_p, params_p))
										{
											if (AddSubmissionGeneBankParams (data_p, params_p))
												{
													if (AddSubmissionMaterialParams (data_p, params_p))
														{
															if (AddSubmissionPhenotypeParams (data_p, params_p))
																{
																	if (AddSubmissionRowPhenotypeParams (data_p, params_p))
																		{
																			return params_p;
																		}
																	else
																		{
																			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "AddSubmissionRowPhenotypeParams failed");
																		}
																}
															else
																{
																	PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "AddSubmissionPhenotypeParams failed");
																}
														}
													else
														{
															PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "AddSubmissionMaterialParams failed");
														}
												}
											else
												{
													PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "AddSubmissionGeneBankParams failed");
												}

										}
									else
										{
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "AddSubmissionPlotParams failed");
										}
								}
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "AddSubmissionLocationParams failed");
								}
						}
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "AddSubmissionExperimentalAreaParams failed");
						}
				}
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "AddSubmissionFieldTrialParams failed");
				}

			FreeParameterSet (params_p);
		}
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate %s ParameterSet", GetDFWFieldTrialSubmissionServiceName (service_p));
		}

	return NULL;
}






static void ReleaseDFWFieldTrialSubmissionServiceParameters (Service * UNUSED_PARAM (service_p), ParameterSet *params_p)
{
	FreeParameterSet (params_p);
}




static bool CloseDFWFieldTrialSubmissionService (Service *service_p)
{
	bool success_flag = true;

	FreeDFWFieldTrialServiceData ((DFWFieldTrialServiceData *) (service_p -> se_data_p));;

	return success_flag;
}



static ServiceJobSet *RunDFWFieldTrialSubmissionService (Service *service_p, ParameterSet *param_set_p, UserDetails * UNUSED_PARAM (user_p), ProvidersStateTable * UNUSED_PARAM (providers_p))
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
					if (!RunForSubmissionFieldTrialParams (data_p, param_set_p, job_p))
						{
							if (!RunForSubmissionExperimentalAreaParams (data_p, param_set_p, job_p))
								{
									if (!RunForSubmissionLocationParams (data_p, param_set_p, job_p))
										{
											if (!RunForSubmissionPlotParams (data_p, param_set_p, job_p))
												{
													if (!RunForSubmissionGeneBankParams (data_p, param_set_p, job_p))
														{
															if (!RunForSubmissionMaterialParams (data_p, param_set_p, job_p))
																{
																	if (!RunForSubmissionPhenotypeParams (data_p, param_set_p, job_p))
																		{
																			if (!RunForSubmissionRowPhenotypeParams (data_p, param_set_p, job_p))
																				{

																				}		/* if (!RunForSubmissionRowPhenotypeParams (data_p, param_set_p, job_p)) */

																		}		/* if (!RunForSubmissionPhenotypeParams (data_p, param_set_p, job_p)) */

																}		/* if (!RunForMaterialParams (data_p, param_set_p, job_p)) */

														}		/* if (!RunForGeneBankParams (data_p, param_set_p, job_p)) */

												}		/* if (!RunForPlotParams (data_p, param_set_p, job_p)) */

										}		/* if (!RunForLocationParams (data_p, param_set_p, job_p)) */

								}		/* if (!RunForExperimentalAreaParams (data_p, param_set_p, job_p)) */

						}		/* if (!RunForFieldTrialParams (data_p, param_set_p, job_p)) */

				}		/* if (param_set_p) */


			LogServiceJob (job_p);
		}		/* if (service_p -> se_jobs_p) */

	return service_p -> se_jobs_p;
}


static ServiceMetadata *GetDFWFieldTrialSubmissionServiceMetadata (Service *service_p)
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




static ParameterSet *IsResourceForDFWFieldTrialSubmissionService (Service * UNUSED_PARAM (service_p), Resource * UNUSED_PARAM (resource_p), Handler * UNUSED_PARAM (handler_p))
{
	return NULL;
}


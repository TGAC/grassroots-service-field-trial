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


#include "measured_variable_jobs.h"
#include "submission_service.h"
#include "plot_jobs.h"
#include "field_trial_jobs.h"
#include "study_jobs.h"
#include "material_jobs.h"
#include "location_jobs.h"
#include "gene_bank_jobs.h"
#include "row_jobs.h"
#include "indexing.h"

#include "audit.h"
#include "streams.h"
#include "math_utils.h"
#include "string_utils.h"

#include "boolean_parameter.h"

/*
 * Static declarations
 */


static NamedParameterType S_REINDEX_ALL_DATA = { "SS Reindex all data", PT_BOOLEAN };
static NamedParameterType S_REINDEX_TRIALS = { "SS Reindex trials", PT_BOOLEAN };
static NamedParameterType S_REINDEX_STUDIES = { "SS Reindex studies", PT_BOOLEAN };
static NamedParameterType S_REINDEX_LOCATIONS = { "SS Reindex locations", PT_BOOLEAN };
static NamedParameterType S_REINDEX_TREATMENTS = { "SS Reindex treatments", PT_BOOLEAN };


static const char *GetDFWFieldTrialSubmissionServiceName (const Service *service_p);

static const char *GetDFWFieldTrialSubmissionServiceDescription (const Service *service_p);

static const char *GetDFWFieldTrialSubmissionServiceAlias (const Service *service_p);

static const char *GetDFWFieldTrialSubmissionServiceInformationUri (const Service *service_p);

static ParameterSet *GetDFWFieldTrialSubmissionServiceParameters (Service *service_p, Resource *resource_p, UserDetails *user_p);

static bool GetDFWFieldTrialSubmissionServiceParameterTypesForNamedParameters (const Service *service_p, const char *param_name_s, ParameterType *pt_p);

static void ReleaseDFWFieldTrialSubmissionServiceParameters (Service *service_p, ParameterSet *params_p);

static ServiceJobSet *RunDFWFieldTrialSubmissionService (Service *service_p, ParameterSet *param_set_p, UserDetails *user_p, ProvidersStateTable *providers_p);

static ParameterSet *IsResourceForDFWFieldTrialSubmissionService (Service *service_p, Resource *resource_p, Handler *handler_p);

static bool CloseDFWFieldTrialSubmissionService (Service *service_p);

static ServiceMetadata *GetDFWFieldTrialSubmissionServiceMetadata (Service *service_p);

static bool AddIndexingParameters (ServiceData *data_p, ParameterSet *params_p);

static bool GetSubmissionIndexingParameterTypeForNamedParameter (const char *param_name_s, ParameterType *pt_p);

static bool RunReindexing (ParameterSet *param_set_p, ServiceJob *job_p, DFWFieldTrialServiceData *data_p);

/*
 * API definitions
 */


Service *GetDFWFieldTrialSubmissionService (GrassrootsServer *grassroots_p)
{
	Service *service_p = (Service *) AllocMemory (sizeof (Service));

	if (service_p)
		{
			DFWFieldTrialServiceData *data_p = AllocateDFWFieldTrialServiceData ();

			if (data_p)
				{
					if (InitialiseService (service_p,
														 GetDFWFieldTrialSubmissionServiceName,
														 GetDFWFieldTrialSubmissionServiceDescription,
														 GetDFWFieldTrialSubmissionServiceAlias,
														 GetDFWFieldTrialSubmissionServiceInformationUri,
														 RunDFWFieldTrialSubmissionService,
														 IsResourceForDFWFieldTrialSubmissionService,
														 GetDFWFieldTrialSubmissionServiceParameters,
														 GetDFWFieldTrialSubmissionServiceParameterTypesForNamedParameters,
														 ReleaseDFWFieldTrialSubmissionServiceParameters,
														 CloseDFWFieldTrialSubmissionService,
														 NULL,
														 false,
														 SY_SYNCHRONOUS,
														 (ServiceData *) data_p,
														 GetDFWFieldTrialSubmissionServiceMetadata,
														 NULL,
														 grassroots_p))
						{

							if (ConfigureDFWFieldTrialService (data_p, grassroots_p))
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



static const char *GetDFWFieldTrialSubmissionServiceName (const Service * UNUSED_PARAM (service_p))
{
	return "Submit any Field Trial data";
}


static const char *GetDFWFieldTrialSubmissionServiceDescription (const Service * UNUSED_PARAM (service_p))
{
	return "A service to submit field trial data";
}


static const char *GetDFWFieldTrialSubmissionServiceAlias (const Service * UNUSED_PARAM (service_p))
{
	return DFT_GROUP_ALIAS_PREFIX_S "submit";
}

static const char *GetDFWFieldTrialSubmissionServiceInformationUri (const Service * UNUSED_PARAM (service_p))
{
	return NULL;
}


static bool GetDFWFieldTrialSubmissionServiceParameterTypesForNamedParameters (const Service *service_p, const char *param_name_s, ParameterType *pt_p)
{
	bool success_flag = true;

	if (!GetSubmissionFieldTrialParameterTypeForNamedParameter (param_name_s, pt_p))
		{
			if (!GetSubmissionStudyParameterTypeForNamedParameter (param_name_s, pt_p))
				{
					if (!GetSubmissionLocationParameterTypeForNamedParameter (param_name_s, pt_p))
						{
							if (!GetSubmissionPlotParameterTypeForNamedParameter (param_name_s, pt_p))
								{
									if (!GetSubmissionGeneBankParameterTypeForNamedParameter (param_name_s, pt_p))
										{
											if (!GetSubmissionMaterialParameterTypeForNamedParameter (param_name_s, pt_p))
												{
													if (!GetSubmissionMeasuredVariableParameterTypeForNamedParameter (param_name_s, pt_p))
														{
															if (!GetSubmissionRowPhenotypeParameterTypeForNamedParameter (param_name_s, pt_p))
																{
																	if (!GetSubmissionIndexingParameterTypeForNamedParameter (param_name_s, pt_p))
																		{
																			success_flag = false;
																		}
																}		/* if (!GetSearchRowPhenotypeParameterTypeForNamedParameter (param_name_s, pt_p)) */

														}		/* if (!GetSubmissionMeasuredVariableParameterTypeForNamedParameter (param_name_s, pt_p)) */

												}		/* if (!GetSearchMaterialParameterTypeForNamedParameter (param_name_s, pt_p)) */

										}		/* if (!GetSearchGeneBankParameterTypeForNamedParameter (param_name_s, pt_p)) */

								}		/* if (!GetSearchPlotParameterTypeForNamedParameter (param_name_s, pt_p)) */

						}		/* if (!GetSearchLocationParameterTypeForNamedParameter (param_name_s, pt_p)) */

				}		/* if (!GetSearchStudyParameterTypeForNamedParameter (param_name_s, pt_p)) */

		}		/* if (!GetSearchFieldTrialParameterTypeForNamedParameter (param_name_s, pt_p)) */

	return success_flag;
}



static bool AddIndexingParameters (ServiceData *data_p, ParameterSet *params_p)
{
	Parameter *param_p = NULL;
	ParameterGroup *group_p = CreateAndAddParameterGroupToParameterSet ("Indexing", false, data_p, params_p);

	bool b = false;

	if ((param_p = EasyCreateAndAddBooleanParameterToParameterSet (data_p, params_p, group_p, S_REINDEX_ALL_DATA.npt_name_s, "Reindex all data", "Reindex all data into Lucene", &b, PL_ADVANCED)) != NULL)
		{
			if ((param_p = EasyCreateAndAddBooleanParameterToParameterSet (data_p, params_p, group_p,S_REINDEX_TRIALS.npt_name_s, "Reindex all Field Trials", "Reindex all Field Trials into Lucene", &b, PL_ADVANCED)) != NULL)
				{
					if ((param_p = EasyCreateAndAddBooleanParameterToParameterSet (data_p, params_p, group_p, S_REINDEX_STUDIES.npt_name_s, "Reindex all Studies", "Reindex all Studies into Lucene", &b, PL_ADVANCED)) != NULL)
						{
							if ((param_p = EasyCreateAndAddBooleanParameterToParameterSet (data_p, params_p, group_p, S_REINDEX_LOCATIONS.npt_name_s, "Reindex all Locations", "Reindex all Locations into Lucene", &b, PL_ADVANCED)) != NULL)
								{
										{
											return true;
										}
								}
						}
				}
		}


	return false;
}


static bool GetSubmissionIndexingParameterTypeForNamedParameter (const char *param_name_s, ParameterType *pt_p)
{
	bool success_flag = true;

	if (strcmp (param_name_s, S_REINDEX_ALL_DATA.npt_name_s) == 0)
		{
			*pt_p = S_REINDEX_ALL_DATA.npt_type;
		}
	else if (strcmp (param_name_s, S_REINDEX_TRIALS.npt_name_s) == 0)
		{
			*pt_p = S_REINDEX_TRIALS.npt_type;
		}
	else if (strcmp (param_name_s, S_REINDEX_STUDIES.npt_name_s) == 0)
		{
			*pt_p = S_REINDEX_STUDIES.npt_type;
		}
	else if (strcmp (param_name_s, S_REINDEX_LOCATIONS.npt_name_s) == 0)
		{
			*pt_p = S_REINDEX_LOCATIONS.npt_type;
		}
	else if (strcmp (param_name_s, S_REINDEX_TREATMENTS.npt_name_s) == 0)
		{
			*pt_p = S_REINDEX_TREATMENTS.npt_type;
		}
	else
		{
			success_flag = false;
		}

	return success_flag;
}


static ParameterSet *GetDFWFieldTrialSubmissionServiceParameters (Service *service_p, Resource *resource_p, UserDetails * UNUSED_PARAM (user_p))
{
	ParameterSet *params_p = AllocateParameterSet ("DFWFieldTrial submission service parameters", "The parameters used for the DFWFieldTrial submission service");

	if (params_p)
		{
			ServiceData *data_p = service_p -> se_data_p;

			if (AddSubmissionFieldTrialParams (data_p, params_p))
				{
					if (AddSubmissionStudyParams (data_p, params_p, resource_p))
						{
							if (AddSubmissionLocationParams (data_p, params_p))
								{
									if (AddSubmissionPlotParams (data_p, params_p, resource_p))
										{
											if (AddSubmissionGeneBankParams (data_p, params_p))
												{
													if (AddSubmissionMaterialParams (data_p, params_p))
														{
															if (AddSubmissionMeasuredVariableParams (data_p, params_p))
																{
																	if (AddSubmissionRowPhenotypeParams (data_p, params_p, resource_p))
																		{
																			if (AddIndexingParameters (data_p, params_p))
																				{
																					return params_p;
																				}
																			else
																				{
																					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "AddIndexingParameters failed");
																				}
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
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "AddSubmissionStudyParams failed");
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


static bool RunReindexing (ParameterSet *param_set_p, ServiceJob *job_p, DFWFieldTrialServiceData *data_p)
{
	bool done_flag = false;
	OperationStatus status = GetServiceJobStatus (job_p);
	const bool *index_flag_p = NULL;

	if (GetCurrentBooleanParameterValueFromParameterSet (param_set_p, S_REINDEX_ALL_DATA.npt_name_s, &index_flag_p))
		{
			if ((index_flag_p != NULL) && (*index_flag_p == true))
				{
					ReindexAllData (job_p, data_p);

					done_flag = true;
				}
		}

	if (!done_flag)
		{
			GrassrootsServer *grassroots_p = GetGrassrootsServerFromService (job_p -> sj_service_p);
			LuceneTool *lucene_p = AllocateLuceneTool (grassroots_p, job_p -> sj_id);
			uint32 num_attempted = 0;
			uint32 num_succeeded = 0;

			if (lucene_p)
				{
					bool update_flag = false;

					if (GetCurrentBooleanParameterValueFromParameterSet (param_set_p, S_REINDEX_TRIALS.npt_name_s, &index_flag_p))
						{
							if ((index_flag_p != NULL) && (*index_flag_p == true))
								{
									if (ReindexTrials (job_p, lucene_p, update_flag, data_p))
										{
											++ num_succeeded;
										}

									++ num_attempted;

									update_flag = true;
									done_flag = true;
								}
						}

					if (GetCurrentBooleanParameterValueFromParameterSet (param_set_p, S_REINDEX_STUDIES.npt_name_s, &index_flag_p))
						{
							if ((index_flag_p != NULL) && (*index_flag_p == true))
								{
									if (ReindexStudies (job_p, lucene_p, update_flag, data_p))
										{
											++ num_succeeded;
										}

									++ num_attempted;


									update_flag = true;
									done_flag = true;
								}
						}

					if (GetCurrentBooleanParameterValueFromParameterSet (param_set_p, S_REINDEX_LOCATIONS.npt_name_s, &index_flag_p))
						{
							if ((index_flag_p != NULL) && (*index_flag_p == true))
								{
									if (ReindexLocations (job_p, lucene_p, update_flag, data_p))
										{
											++ num_succeeded;
										}

									++ num_attempted;

									update_flag = true;
									done_flag = true;
								}
						}

					if (GetCurrentBooleanParameterValueFromParameterSet (param_set_p, S_REINDEX_TREATMENTS.npt_name_s, &index_flag_p))
						{
							if ((index_flag_p != NULL) && (*index_flag_p == true))
								{
									if (ReindexMeasuredVariables (job_p, lucene_p, update_flag, data_p))
										{
											++ num_succeeded;
										}

									++ num_attempted;

									update_flag = true;
									done_flag = true;
								}
						}

					FreeLuceneTool (lucene_p);
				}		/* if (lucene_p) */

			if (num_succeeded == num_attempted)
				{
					status = OS_SUCCEEDED;
				}
			else if (num_succeeded > 0)
				{
					status = OS_PARTIALLY_SUCCEEDED;
				}
			else
				{
					status = OS_FAILED;
				}

			SetServiceJobStatus (job_p, status);

		}		/* if (!done_flag) */


	return done_flag;
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
					if (!RunReindexing (param_set_p, job_p, data_p))
						{
							if (!RunForSubmissionFieldTrialParams (data_p, param_set_p, job_p))
								{
									if (!RunForSubmissionStudyParams (data_p, param_set_p, job_p))
										{
											if (!RunForSubmissionLocationParams (data_p, param_set_p, job_p))
												{
													if (!RunForSubmissionPlotParams (data_p, param_set_p, job_p))
														{
															if (!RunForSubmissionGeneBankParams (data_p, param_set_p, job_p))
																{
																	if (!RunForSubmissionMaterialParams (data_p, param_set_p, job_p))
																		{
																			if (!RunForSubmissionMeasuredVariableParams (data_p, param_set_p, job_p))
																				{
																					if (!RunForSubmissionRowPhenotypeParams (data_p, param_set_p, job_p))
																						{
																						}		/* if (!RunForSubmissionRowPhenotypeParams (data_p, param_set_p, job_p)) */

																				}		/* if (!RunForSubmissionMeasuredVariableParams (data_p, param_set_p, job_p)) */

																		}		/* if (!RunForMaterialParams (data_p, param_set_p, job_p)) */

																}		/* if (!RunForGeneBankParams (data_p, param_set_p, job_p)) */

														}		/* if (!RunForPlotParams (data_p, param_set_p, job_p)) */

												}		/* if (!RunForLocationParams (data_p, param_set_p, job_p)) */

										}		/* if (!RunForStudyParams (data_p, param_set_p, job_p)) */

								}		/* if (!RunForFieldTrialParams (data_p, param_set_p, job_p)) */

						}		/* if (!RunReindexing (param_set_p, job_p, data_p)) */

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




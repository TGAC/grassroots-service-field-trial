/*
 * study_manager.c
 *
 *  Created on: 5 Aug 2022
 *      Author: billy
 */


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
 * submit_study.c
 *
 *  Created on: 5 Apr 2019
 *      Author: billy
 */


#include "study_manager.h"

#include "audit.h"

#include "study_jobs.h"


#include "boolean_parameter.h"

#include "handbook_generator.h"

#include "dfw_util.h"

/*
 * Static declarations
 */


/*
 * Static declarations
 */


static NamedParameterType S_READONLY_UUID = { "SM uuid", PT_STRING };


/*
 * caching parameters
 */
static NamedParameterType S_CACHE_CLEAR = { "SM clear cached study", PT_BOOLEAN };

/*
 * study management parameters
 */
static NamedParameterType S_REMOVE_STUDY = { "SM Delete study", PT_BOOLEAN };
static NamedParameterType S_GENERATE_FD_PACKAGE = { "SM Generate FD Packages", PT_BOOLEAN};
static NamedParameterType S_REMOVE_STUDY_PLOTS = { "SM Remove Study Plots", PT_BOOLEAN };
static NamedParameterType S_GENERATE_HANDBOOK = { "SM Generate Handbook", PT_BOOLEAN };


static NamedParameterType S_GENERATE_STUDY_STATISTICS = { "SM Generate Phenotypes", PT_LARGE_STRING };

static NamedParameterType S_INDEXER = { "SM indexer", PT_STRING };

static const char * const  S_INDEXER_NONE_S = "<NONE>";
static const char * const  S_INDEXER_DELETE_S = "Delete";
static const char * const  S_INDEXER_INDEX_S = "Reindex";


static const char *GetStudyManagerServiceDescription (const Service *service_p);

static const char *GetStudyManagerServiceName (const Service *service_p);


static const char *GetStudyManagerServiceAlias (const Service *service_p);

static const char *GetStudyManagerServiceInformationUri (const Service *service_p);

static ParameterSet *GetStudyManagerServiceParameters (Service *service_p, DataResource *resource_p, UserDetails *user_p);

static bool GetStudyManagerServiceParameterTypesForNamedParameters (const Service *service_p, const char *param_name_s, ParameterType *pt_p);

static void ReleaseStudyManagerServiceParameters (Service *service_p, ParameterSet *params_p);

static ServiceJobSet *RunStudyManagerService (Service *service_p, ParameterSet *param_set_p, UserDetails *user_p, ProvidersStateTable *providers_p);


static bool CloseStudyManagerService (Service *service_p);

static ServiceMetadata *GetStudyManagerServiceMetadata (Service *service_p);

static bool SetUpIndexingParameter (ParameterSet *params_p, ParameterGroup *group_p, const ServiceData *data_p);


/*
 * API definitions
 */


Service *GetStudyManagerService (GrassrootsServer *grassroots_p)
{
	Service *service_p = (Service *) AllocMemory (sizeof (Service));

	if (service_p)
		{
			FieldTrialServiceData *data_p = AllocateFieldTrialServiceData ();

			if (data_p)
				{
					if (InitialiseService (service_p,
														 GetStudyManagerServiceName,
														 GetStudyManagerServiceDescription,
														 GetStudyManagerServiceAlias,
														 GetStudyManagerServiceInformationUri,
														 RunStudyManagerService,
														 NULL,
														 GetStudyManagerServiceParameters,
														 GetStudyManagerServiceParameterTypesForNamedParameters,
														 ReleaseStudyManagerServiceParameters,
														 CloseStudyManagerService,
														 NULL,
														 false,
														 SY_SYNCHRONOUS,
														 (ServiceData *) data_p,
														 GetStudyManagerServiceMetadata,
														 NULL,
														 grassroots_p))
						{

							if (ConfigureFieldTrialService (data_p, grassroots_p))
								{
									return service_p;
								}

						}		/* if (InitialiseService (.... */

					FreeFieldTrialServiceData (data_p);
				}

			FreeMemory (service_p);
		}		/* if (service_p) */

	return NULL;
}





static const char *GetStudyManagerServiceName (const Service * UNUSED_PARAM (service_p))
{
	return "Manage Study";
}


static const char *GetStudyManagerServiceDescription (const Service * UNUSED_PARAM (service_p))
{
	return "Following the same nomenclature as <a href='https://brapi.docs.apiary.io/'>BrAPI</a>, a Study is a phenotyping experiment "
			"taking place at a single location. One or more Studies can take place within a single Trial.";
}


static const char *GetStudyManagerServiceAlias (const Service * UNUSED_PARAM (service_p))
{
	return DFT_GROUP_ALIAS_PREFIX_S SERVICE_GROUP_ALIAS_SEPARATOR "manage_study";
}


static const char *GetStudyManagerServiceInformationUri (const Service *service_p)
{
	const char *url_s = GetServiceInformationPage (service_p);

	if (!url_s)
		{
			url_s = "https://grassroots.tools/docs/user/services/field_trial/study_manager.md";
		}

	return url_s;
}



static bool GetStudyManagerServiceParameterTypesForNamedParameters (const struct Service *service_p, const char *param_name_s, ParameterType *pt_p)
{
	const NamedParameterType params [] =
		{
			STUDY_ID,
			S_READONLY_UUID,
			S_CACHE_CLEAR,
			S_REMOVE_STUDY_PLOTS,
			S_GENERATE_FD_PACKAGE,
			S_REMOVE_STUDY,
			S_GENERATE_HANDBOOK,
			S_GENERATE_STUDY_STATISTICS,
			S_INDEXER,
			NULL
		};

	return DefaultGetParameterTypeForNamedParameter (param_name_s, pt_p, params);
}


static ParameterSet *GetStudyManagerServiceParameters (Service *service_p, DataResource *resource_p, UserDetails * UNUSED_PARAM (user_p))
{
	ParameterSet *params_p = AllocateParameterSet ("Study manager service parameters", "The parameters used for the FieldTrial submission service");

	if (params_p)
		{
			ServiceData *data_p = service_p -> se_data_p;
			ParameterGroup *group_p = NULL;
			FieldTrialServiceData *dfw_data_p = (FieldTrialServiceData *) (service_p -> se_data_p);
			Study *active_study_p = GetStudyFromResource (resource_p, STUDY_ID, dfw_data_p);
			char *study_id_s = NULL;
			Parameter *param_p = NULL;

			if (active_study_p)
				{
					study_id_s = GetBSONOidAsString (active_study_p -> st_id_p);
				}

			param_p = EasyCreateAndAddStringParameterToParameterSet (data_p, params_p, group_p, STUDY_ID.npt_type, STUDY_ID.npt_name_s, "Load Study", "Manage an existing study", study_id_s, PL_ALL);

			if (param_p)
				{
					if (SetUpStudiesListParameter (dfw_data_p, (StringParameter *) param_p, NULL, false))
						{
							/*
							 * We want to update all of the values in the form
							 * when a user selects a study from the list so
							 * we need to make the parameter automatically
							 * refresh the values. So we set the
							 * pa_refresh_service_flag to true.
							 */
							param_p -> pa_refresh_service_flag = true;
							bool b = false;

							if ((param_p = EasyCreateAndAddStringParameterToParameterSet (data_p, params_p, group_p, S_READONLY_UUID.npt_type, S_READONLY_UUID.npt_name_s, "UUID", "The UUID for the given Study", study_id_s, PL_ALL)) != NULL)
								{
									param_p -> pa_read_only_flag = true;

									if ((param_p = EasyCreateAndAddBooleanParameterToParameterSet (data_p, params_p, group_p, S_CACHE_CLEAR.npt_name_s, "Clear cached Study", "If the Study is cached, clear it", &b, PL_ALL)) != NULL)
										{
											if ((param_p = EasyCreateAndAddBooleanParameterToParameterSet (data_p, params_p, group_p, S_REMOVE_STUDY.npt_name_s, "Remove Study", "Remove a Study and all of its Plots", &b, PL_ALL)) != NULL)
												{
													if ((param_p = EasyCreateAndAddBooleanParameterToParameterSet (data_p, params_p, group_p, S_REMOVE_STUDY_PLOTS.npt_name_s, "Remove Plots", "Remove all of the Plots from a Study", &b, PL_ALL)) != NULL)
														{
															if ((param_p = EasyCreateAndAddBooleanParameterToParameterSet (data_p, params_p, group_p,S_GENERATE_FD_PACKAGE.npt_name_s, "Frictionless Data", "Generate the Frictionless Data Package for a Study", &b, PL_ALL)) != NULL)
																{
																	if ((param_p = EasyCreateAndAddBooleanParameterToParameterSet (data_p, params_p, group_p, S_GENERATE_HANDBOOK.npt_name_s, "Generate Handbook", "Generate a handbook for a Study ", &b, PL_ALL)) != NULL)
																		{
																			if ((param_p = EasyCreateAndAddStringParameterToParameterSet (data_p, params_p, group_p, S_GENERATE_STUDY_STATISTICS.npt_type, S_GENERATE_STUDY_STATISTICS.npt_name_s, "Collate Phenotypes", "Create and store a list of all of the Phenotypes in a Study and generate statistics where appropriate", &b, PL_ALL)) != NULL)
																				{
																					if (SetUpIndexingParameter (params_p, group_p, data_p))
																						{
																							return params_p;
																						}
																				}
																			else
																				{
																					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate %s Parameter", S_GENERATE_STUDY_STATISTICS.npt_name_s);
																				}
																		}
																	else
																		{
																			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate %s Parameter", S_GENERATE_HANDBOOK.npt_name_s);
																		}
																}
															else
																{
																	PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate %s Parameter", S_GENERATE_FD_PACKAGE.npt_name_s);
																}
														}
													else
														{
															PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate %s Parameter", S_REMOVE_STUDY_PLOTS.npt_name_s);
														}
												}
											else
												{
													PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate %s Parameter", S_REMOVE_STUDY.npt_name_s);
												}
										}
									else
										{
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate %s Parameter", S_CACHE_CLEAR.npt_name_s);
										}

								}		/* if ((param_p = EasyCreateAndAddStringParameterToParameterSet (data_p, params_p, group_p, S_READONLY_UUID.npt_type, S_READONLY_UUID.npt_name_s, "UUID", "The UUID for the given Study", study_id_s, PL_ALL)) != NULL) */
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate %s Parameter", S_READONLY_UUID.npt_name_s);
								}

						}
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "SetUpStudiesListParameter () failed");
						}

				}
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate %s Parameter", STUDY_ID.npt_name_s);
				}


			FreeParameterSet (params_p);
		}
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate %s ParameterSet", GetStudyManagerServiceName (service_p));
		}

	return NULL;
}



static bool SetUpIndexingParameter (ParameterSet *params_p, ParameterGroup *group_p, const ServiceData *data_p)
{
	bool success_flag = false;

	StringParameter *param_p = (StringParameter *) EasyCreateAndAddStringParameterToParameterSet (data_p, params_p, group_p, S_INDEXER.npt_type, S_INDEXER.npt_name_s, "Index Status", "Manage the existing study in the search engine", S_INDEXER_NONE_S, PL_ALL);

	if (param_p)
		{
			if (CreateAndAddStringParameterOption (param_p, S_INDEXER_NONE_S, S_INDEXER_NONE_S))
				{
					if (CreateAndAddStringParameterOption (param_p, S_INDEXER_INDEX_S, S_INDEXER_INDEX_S))
						{
							if (CreateAndAddStringParameterOption (param_p, S_INDEXER_DELETE_S, S_INDEXER_DELETE_S))
								{
									success_flag = true;
								}

						}

				}

		}

	return success_flag;
}


static void ReleaseStudyManagerServiceParameters (Service * UNUSED_PARAM (service_p), ParameterSet *params_p)
{
	FreeParameterSet (params_p);
}




static bool CloseStudyManagerService (Service *service_p)
{
	bool success_flag = true;

	FreeFieldTrialServiceData ((FieldTrialServiceData *) (service_p -> se_data_p));

	return success_flag;
}



static ServiceJobSet *RunStudyManagerService (Service *service_p, ParameterSet *param_set_p, UserDetails * UNUSED_PARAM (user_p), ProvidersStateTable * UNUSED_PARAM (providers_p))
{
	FieldTrialServiceData *data_p = (FieldTrialServiceData *) (service_p -> se_data_p);

	service_p -> se_jobs_p = AllocateSimpleServiceJobSet (service_p, NULL, GetServiceName (service_p));

	if (service_p -> se_jobs_p)
		{
			ServiceJob *job_p = GetServiceJobFromServiceJobSet (service_p -> se_jobs_p, 0);

			SetServiceJobStatus (job_p, OS_IDLE);


			if (param_set_p)
				{
					const char *id_s = NULL;
					const char *phenotypes_s = NULL;

					LogParameterSet (param_set_p, job_p);


					/*
					 * Get the existing study id if specified
					 */
					GetCurrentStringParameterValueFromParameterSet (param_set_p, STUDY_ID.npt_name_s, &id_s);

					if (id_s)
						{
							Study *study_p = GetStudyByIdString (id_s, VF_CLIENT_FULL, data_p);

							if (study_p)
								{
									bool run_flag = false;
									const bool *run_flag_p = &run_flag;
									bool backed_up_flag = false;

									if (GetCurrentBooleanParameterValueFromParameterSet (param_set_p, S_CACHE_CLEAR.npt_name_s, &run_flag_p))
										{
											if ((run_flag_p != NULL) && (*run_flag_p == true))
												{
													OperationStatus s = OS_FAILED;
													char *filename_s = GetFullCacheFilename (id_s, data_p);

													if (filename_s)
														{
															if (RemoveFile (filename_s))
																{
																	s = OS_SUCCEEDED;
																}
															else
																{
																	PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to remove file \"%s\"", filename_s);
																}

															FreeCopiedString (filename_s);
														}
													else
														{
															PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "GetFullCacheFilename () failed for \"%s\"", id_s);
														}

													MergeServiceJobStatus (job_p, s);
												}
										}

									if (GetCurrentBooleanParameterValueFromParameterSet (param_set_p, S_REMOVE_STUDY_PLOTS.npt_name_s, &run_flag_p))
										{
											if ((run_flag_p != NULL) && (*run_flag_p == true))
												{
													if (BackupStudyByIdString (id_s, data_p))
														{
															OperationStatus s = RemovePlotsForStudyById (id_s, data_p);

															if ((s == OS_SUCCEEDED) || (s == OS_PARTIALLY_SUCCEEDED))
																{
																	if (!ClearCachedStudy (id_s, data_p))
																		{
																			AddGeneralErrorMessageToServiceJob (job_p, "Failed to remove cached Study");

																			if (s == OS_SUCCEEDED)
																				{
																					s = OS_PARTIALLY_SUCCEEDED;
																				}
																		}



																}

															backed_up_flag = true;

															MergeServiceJobStatus (job_p, s);
														}
												}
										}

									if (GetCurrentBooleanParameterValueFromParameterSet (param_set_p, S_REMOVE_STUDY.npt_name_s, &run_flag_p))
										{
											if ((run_flag_p != NULL) && (*run_flag_p == true))
												{
													if (backed_up_flag || (BackupStudyByIdString (id_s, data_p)))
														{
															OperationStatus s = DeleteStudyById (id_s, job_p, data_p, false);

															if ((s == OS_SUCCEEDED) || (s == OS_PARTIALLY_SUCCEEDED))
																{
																	if (!ClearCachedStudy (id_s, data_p))
																		{
																			AddGeneralErrorMessageToServiceJob (job_p, "Failed to remove cached Study");

																			if (s == OS_SUCCEEDED)
																				{
																					s = OS_PARTIALLY_SUCCEEDED;
																				}
																		}
																}

															MergeServiceJobStatus (job_p, s);
														}
												}
										}


									if (GetCurrentBooleanParameterValueFromParameterSet (param_set_p, S_GENERATE_FD_PACKAGE.npt_name_s, &run_flag_p))
										{
											if ((run_flag_p != NULL) && (*run_flag_p == true))
												{
													OperationStatus s = OS_FAILED;

													if (SaveStudyAsFrictionlessData (study_p, data_p))
														{
															s = OS_SUCCEEDED;
														}
													else
														{
															PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "GetFullCacheFilename () failed for \"%s\"", id_s);
														}

													MergeServiceJobStatus (job_p, s);
												}
										}

									if (GetCurrentStringParameterValueFromParameterSet (param_set_p, S_GENERATE_STUDY_STATISTICS.npt_name_s, &phenotypes_s))
										{
											if (!IsStringEmpty (phenotypes_s))
												{
													/* do all phenotypes? */
													if (strcmp (phenotypes_s, "*") == 0)
														{
															OperationStatus s = OS_FAILED;

															if (GenerateStatisticsForStudy (study_p, job_p, data_p))
																{
																	s = OS_SUCCEEDED;
																}
															else
																{
																	PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "GenerateStatisticsForStudy () failed for \"%s\"", study_p -> st_name_s);
																}


															MergeServiceJobStatus (job_p, s);
														}
													else
														{
															/* Get the list of phenotypes tp regenerate */
														}


												}		/* if (!IsStringEmpty (phenotypes_s)) */

										}


									run_flag = false;
									if (GetCurrentBooleanParameterValueFromParameterSet (param_set_p, S_GENERATE_HANDBOOK.npt_name_s, &run_flag_p))
										{
											if ((run_flag_p != NULL) && (*run_flag_p == true))
												{
													OperationStatus s = GenerateStudyAsPDF (study_p, data_p);

													MergeServiceJobStatus (job_p, s);
												}
										}

									const char *value_s = NULL;

									if (GetCurrentStringParameterValueFromParameterSet (param_set_p, S_INDEXER.npt_name_s, &value_s))
										{
											if (value_s)
												{
													if (strcmp (value_s, S_INDEXER_INDEX_S) == 0)
														{
															OperationStatus s = IndexStudy (study_p, job_p, id_s, data_p);

															MergeServiceJobStatus (job_p, s);
														}
													else if (strcmp (value_s, S_INDEXER_DELETE_S) == 0)
														{
															OperationStatus s = DeleteStudyFromLuceneIndexById (id_s,  job_p -> sj_id, data_p);

															MergeServiceJobStatus (job_p, s);
														}
													else
														{

														}

												}

										}		/* if (GetCurrentStringParameterValueFromParameterSet (param_set_p, S_INDEXER.npt_name_s, &value_s)) */

								}		/*  if (study_p) */





						}		/* if (id_s) */


				}

		}

	return service_p -> se_jobs_p;

}


static ServiceMetadata *GetStudyManagerServiceMetadata (Service *service_p)
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


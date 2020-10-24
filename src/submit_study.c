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


#include "submit_study.h"

#include "audit.h"

#include "study_jobs.h"


/*
 * Static declarations
 */



static const char *GetStudySubmissionServiceDescription (const Service *service_p);

static const char *GetStudySubmissionServiceAlias (const Service *service_p);

static const char *GetStudySubmissionServiceInformationUri (const Service *service_p);

static ParameterSet *GetStudySubmissionServiceParameters (Service *service_p, Resource *resource_p, UserDetails *user_p);

static bool GetStudySubmissionServiceParameterTypesForNamedParameters (const Service *service_p, const char *param_name_s, ParameterType *pt_p);

static void ReleaseStudySubmissionServiceParameters (Service *service_p, ParameterSet *params_p);

static ServiceJobSet *RunStudySubmissionService (Service *service_p, ParameterSet *param_set_p, UserDetails *user_p, ProvidersStateTable *providers_p);


static bool CloseStudySubmissionService (Service *service_p);

static ServiceMetadata *GetStudySubmissionServiceMetadata (Service *service_p);

static json_t *GetStudyIndexingData (Service *service_p);


/*
 * API definitions
 */


Service *GetStudySubmissionService (GrassrootsServer *grassroots_p)
{
	Service *service_p = (Service *) AllocMemory (sizeof (Service));

	if (service_p)
		{
			FieldTrialServiceData *data_p = AllocateFieldTrialServiceData ();

			if (data_p)
				{
					if (InitialiseService (service_p,
														 GetStudySubmissionServiceName,
														 GetStudySubmissionServiceDescription,
														 GetStudySubmissionServiceAlias,
														 GetStudySubmissionServiceInformationUri,
														 RunStudySubmissionService,
														 NULL,
														 GetStudySubmissionServiceParameters,
														 GetStudySubmissionServiceParameterTypesForNamedParameters,
														 ReleaseStudySubmissionServiceParameters,
														 CloseStudySubmissionService,
														 NULL,
														 false,
														 SY_SYNCHRONOUS,
														 (ServiceData *) data_p,
														 GetStudySubmissionServiceMetadata,
														 GetStudyIndexingData,
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





const char *GetStudySubmissionServiceName (const Service * UNUSED_PARAM (service_p))
{
	return "Submit Field Trial Study";
}


static const char *GetStudySubmissionServiceDescription (const Service * UNUSED_PARAM (service_p))
{
	return "Following the same nomenclature as <a href='https://brapi.docs.apiary.io/'>BrAPI</a>, a Study is a phenotyping experiment "
			"taking place at a single location. One or more Studies can take place within a single Trial.";
}


static const char *GetStudySubmissionServiceAlias (const Service * UNUSED_PARAM (service_p))
{
	return DFT_GROUP_ALIAS_PREFIX_S SERVICE_GROUP_ALIAS_SEPARATOR "submit_study";
}


static const char *GetStudySubmissionServiceInformationUri (const Service *service_p)
{
	const char *url_s = GetServiceInformationPage (service_p);

	if (!url_s)
		{
			url_s = "https://grassroots.tools/docs/user/services/field_trial/submit_study.md";
		}

	return url_s;
}



static bool GetStudySubmissionServiceParameterTypesForNamedParameters (const struct Service *service_p, const char *param_name_s, ParameterType *pt_p)
{
	return GetSubmissionStudyParameterTypeForNamedParameter (param_name_s, pt_p);
}



static ParameterSet *GetStudySubmissionServiceParameters (Service *service_p, Resource *resource_p, UserDetails * UNUSED_PARAM (user_p))
{
	ParameterSet *params_p = AllocateParameterSet ("FieldTrial submission service parameters", "The parameters used for the FieldTrial submission service");

	if (params_p)
		{
			ServiceData *data_p = service_p -> se_data_p;

			if (AddSubmissionStudyParams (data_p, params_p, resource_p))
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
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate %s ParameterSet", GetStudySubmissionServiceName (service_p));
		}

	return NULL;
}






static void ReleaseStudySubmissionServiceParameters (Service * UNUSED_PARAM (service_p), ParameterSet *params_p)
{
	FreeParameterSet (params_p);
}




static bool CloseStudySubmissionService (Service *service_p)
{
	bool success_flag = true;

	FreeFieldTrialServiceData ((FieldTrialServiceData *) (service_p -> se_data_p));

	return success_flag;
}



static ServiceJobSet *RunStudySubmissionService (Service *service_p, ParameterSet *param_set_p, UserDetails * UNUSED_PARAM (user_p), ProvidersStateTable * UNUSED_PARAM (providers_p))
{
	FieldTrialServiceData *data_p = (FieldTrialServiceData *) (service_p -> se_data_p);

	service_p -> se_jobs_p = AllocateSimpleServiceJobSet (service_p, NULL, "Submit Study");

	if (service_p -> se_jobs_p)
		{
			ServiceJob *job_p = GetServiceJobFromServiceJobSet (service_p -> se_jobs_p, 0);

			LogParameterSet (param_set_p, job_p);

			SetServiceJobStatus (job_p, OS_FAILED_TO_START);

			if (!RunForSubmissionStudyParams (data_p, param_set_p, job_p))
				{

				}		/* if (!RunForSubmissionStudyParams (data_p, param_set_p, job_p)) */


			LogServiceJob (job_p);
		}		/* if (service_p -> se_jobs_p) */

	return service_p -> se_jobs_p;
}


static ServiceMetadata *GetStudySubmissionServiceMetadata (Service *service_p)
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


static json_t *GetStudyIndexingData (Service *service_p)
{
	FieldTrialServiceData *data_p = (FieldTrialServiceData *) (service_p -> se_data_p);
	json_t *src_studies_p = GetAllStudiesAsJSON (data_p);

	if (src_studies_p)
		{
			if (json_is_array (src_studies_p))
				{
					FieldTrialServiceData *dfw_data_p = (FieldTrialServiceData *) (service_p -> se_data_p);
					size_t i;
					json_t *src_study_p;
					size_t num_added = 0;

					json_array_foreach (src_studies_p, i, src_study_p)
						{
							bson_oid_t id;

							if (GetMongoIdFromJSON (src_study_p, &id))
								{
									json_t *phenotypes_p = GetStudyDistinctPhenotypesAsJSON (&id, dfw_data_p);

									if (phenotypes_p)
										{
											if (json_object_set_new (src_study_p, "phenotypes", phenotypes_p) == 0)
												{
													++ num_added;
												}
											else
												{
													PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, src_study_p, "Failed to add phenotypes");
													json_decref (phenotypes_p);
												}
										}

									if (GetNamedIdFromJSON (src_study_p, ST_PARENT_FIELD_TRIAL_S, &id))
										{
											FieldTrial *trial_p = GetFieldTrialById (&id, VF_STORAGE, data_p);

											json_object_del (src_study_p, ST_PARENT_FIELD_TRIAL_S);

											if (trial_p)
												{
													json_t *trial_json_p = GetFieldTrialAsJSON (trial_p, VF_CLIENT_MINIMAL, data_p);

													if (trial_json_p)
														{
															json_object_del (trial_json_p, MONGO_ID_S);

															if (json_object_set_new (src_study_p, ST_PARENT_FIELD_TRIAL_S, trial_json_p) == 0)
																{

																}
															else
																{
																	json_decref (trial_json_p);
																}
														}

													FreeFieldTrial (trial_p);
												}		/* if (trial_p) */

										}

									if (GetNamedIdFromJSON (src_study_p, ST_LOCATION_ID_S, &id))
										{
											Location *location_p = GetLocationById (&id, VF_STORAGE, data_p);

											json_object_del (src_study_p, ST_LOCATION_ID_S);

											if (location_p)
												{
													json_t *location_json_p = GetLocationAsJSON (location_p);

													if (location_json_p)
														{
															json_object_del (location_json_p, MONGO_ID_S);

															if (json_object_set_new (src_study_p, ST_LOCATION_S, location_json_p) == 0)
																{

																}
															else
																{
																	json_decref (location_json_p);
																}

														}

													FreeLocation (location_p);
												}		/* if (location_p) */

										}


								}		/* if (GetMongoIdFromJSON (entry_p, &id)) */

						}		/* json_array_foreach (src_studies_p, i, src_study_p) */

				}		/* if (json_is_array (src_studies_p)) */

			return src_studies_p;
		}		/* if (src_studies_p) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "No studies for \"%s\"", GetServiceName (service_p));
		}

	return NULL;
}



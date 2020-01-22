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
 * phenotype.c
 *
 *  Created on: 17 Oct 2018
 *      Author: billy
 */


#define ALLOCATE_PHENOTYPE_TAGS (1)
#include <treatment.h>
#include "memory_allocations.h"
#include "string_utils.h"
#include "dfw_util.h"
#include "time_util.h"
#include "indexing.h"

/*
 * static declarations
 */

static bool CreateInstrumentFromTreatmentJSON (const json_t *phenotype_json_p, Instrument **instrument_pp, const DFWFieldTrialServiceData *data_p);

static bool AddSchemaTermToJSON (json_t *doc_p, const char * const key_s, const SchemaTerm *term_p);

static SchemaTerm *GetChildSchemTermFromJSON (const json_t *doc_p, const char * const key_s);

static bool AppendSchemaTermQuery (bson_t *query_p, const char *parent_key_s, const char *child_key_s, const char *child_value_s);


/*
 * API definitions
 */
Treatment *AllocateTreatment (bson_oid_t *id_p, SchemaTerm *trait_p, SchemaTerm *measurement_p, SchemaTerm *unit_p, SchemaTerm *variable_p, SchemaTerm *form_p, const char *internal_name_s)
{
	char *copied_internal_name_s = NULL;

	if ((IsStringEmpty (internal_name_s)) || ((copied_internal_name_s = EasyCopyToNewString (internal_name_s)) != NULL))
		{
			Treatment *treatment_p = (Treatment *) AllocMemory (sizeof (Treatment));

			if (treatment_p)
				{
					treatment_p -> tr_id_p = id_p;
					treatment_p -> tr_trait_term_p = trait_p;
					treatment_p -> tr_measurement_term_p = measurement_p;
					treatment_p -> tr_unit_term_p = unit_p;
					treatment_p -> tr_variable_term_p = variable_p;
					treatment_p -> tr_form_term_p = form_p;
					treatment_p -> tr_internal_name_s = copied_internal_name_s;

					return treatment_p;
				}		/* if (treatment_p) */

			if (copied_internal_name_s)
				{
					FreeCopiedString (copied_internal_name_s);
				}

		}		/* if ((IsStringEmpty (internal_name_s)) || ((copied_internal_name_s = EasyCopyToNewString (internal_name_s)) != NULL)) */

	return NULL;
}



void FreeTreatment (Treatment *treatment_p)
{
	if (treatment_p -> tr_id_p)
		{
			FreeBSONOid (treatment_p -> tr_id_p);
		}

	if (treatment_p -> tr_trait_term_p)
		{
			FreeSchemaTerm (treatment_p -> tr_trait_term_p);
		}

	if (treatment_p -> tr_measurement_term_p)
		{
			FreeSchemaTerm (treatment_p -> tr_measurement_term_p);
		}

	if (treatment_p -> tr_unit_term_p)
		{
			FreeSchemaTerm (treatment_p -> tr_unit_term_p);
		}

	if (treatment_p -> tr_variable_term_p)
		{
			FreeSchemaTerm (treatment_p -> tr_variable_term_p);
		}


	if (treatment_p -> tr_internal_name_s)
		{
			FreeCopiedString (treatment_p -> tr_internal_name_s);
		}


	/*
	 * Still need to deallocate the instrument
	 *
	 * treatment_p -> tr_instrument_p
	 */

	FreeMemory (treatment_p);
}



json_t *GetTreatmentAsJSON (const Treatment *treatment_p, const ViewFormat format)
{
	json_t *phenotype_json_p = json_object ();

	if (phenotype_json_p)
		{
			if (AddSchemaTermToJSON (phenotype_json_p, TR_TRAIT_S, treatment_p -> tr_trait_term_p))
				{
					if (AddSchemaTermToJSON (phenotype_json_p, TR_MEASUREMENT_S, treatment_p -> tr_measurement_term_p))
						{
							if (AddSchemaTermToJSON (phenotype_json_p, TR_UNIT_S, treatment_p -> tr_unit_term_p))
								{
									if ((! (treatment_p -> tr_variable_term_p)) || (AddSchemaTermToJSON (phenotype_json_p, TR_VARIABLE_S, treatment_p -> tr_variable_term_p)))
										{
											/*
											 * The form term is optional
											 */
											if ((! (treatment_p -> tr_form_term_p)) || (AddSchemaTermToJSON (phenotype_json_p, TR_FORM_S, treatment_p -> tr_form_term_p)))
												{
													bool success_flag = false;

													if (format == VF_STORAGE)
														{
															if ((IsStringEmpty (treatment_p -> tr_internal_name_s)) || (SetJSONString (phenotype_json_p, TR_INTERNAL_NAME_S, treatment_p -> tr_internal_name_s)))
																{
																	success_flag = true;
																}		/* if ((IsStringEmpty (treatment_p -> tr_internal_name_s)) || SetJSONString (phenotype_json_p, TR_INTERNAL_NAME_S, treatment_p -> tr_internal_name_s)) */
														}
													else
														{
															success_flag = true;
														}

													if (success_flag)
														{
															if (AddCompoundIdToJSON (phenotype_json_p, treatment_p -> tr_id_p))
																{
																	if (AddDatatype (phenotype_json_p, DFTD_TREATMENT))
																		{
																			return phenotype_json_p;
																		}

																	return phenotype_json_p;
																}		/* if (AddCompoundIdToJSON (phenotype_json_p, treatment_p -> tr_id_p)) */

														}		/* if (success_flag) */

												}		/* if ((! (treatment_p -> tr_form_term_p)) || (AddSchemTermToJSON (phenotype_json_p, TR_FORM_S, treatment_p -> tr_form_term_p))) */
											else
												{
													PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, phenotype_json_p, "Failed to add SchemaTerm for \"%s\" to JSON", treatment_p -> tr_form_term_p -> st_url_s);
												}

										}
									else
										{
											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, phenotype_json_p, "Failed to add SchemaTerm for \"%s\" to JSON", treatment_p -> tr_variable_term_p -> st_url_s);
										}

								}		/* if (AddSchemTermToJSON (phenotype_json_p, TR_UNIT_S, treatment_p -> tr_unit_term_p)) */
							else
								{
									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, phenotype_json_p, "Failed to add SchemaTerm for \"%s\" to JSON", treatment_p -> tr_unit_term_p -> st_url_s);
								}

						}		/* if (AddSchemTermToJSON (phenotype_json_p, TR_MEASUREMENT_S, treatment_p -> tr_measurement_term_p)) */
					else
						{
							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, phenotype_json_p, "Failed to add SchemaTerm for \"%s\" to JSON", treatment_p -> tr_measurement_term_p -> st_url_s);
						}

				}		/* if (AddSchemTermToJSON (phenotype_json_p, TR_TRAIT_S, treatment_p -> tr_trait_term_p)) */
			else
				{
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, phenotype_json_p, "Failed to add SchemaTerm for \"%s\" to JSON", treatment_p -> tr_trait_term_p -> st_url_s);
				}

			json_decref (phenotype_json_p);
		}		/* if (phenotype_json_p) */

	return NULL;
}


Treatment *GetTreatmentFromJSON (const json_t *phenotype_json_p, const DFWFieldTrialServiceData *data_p)
{
	SchemaTerm *trait_p = GetChildSchemTermFromJSON (phenotype_json_p, TR_TRAIT_S);

	if (trait_p)
		{
			SchemaTerm *unit_p = GetChildSchemTermFromJSON (phenotype_json_p, TR_UNIT_S);

			if (unit_p)
				{
					SchemaTerm *measurement_p = GetChildSchemTermFromJSON (phenotype_json_p, TR_MEASUREMENT_S);

					if (measurement_p)
						{
							SchemaTerm *variable_p = GetChildSchemTermFromJSON (phenotype_json_p, TR_VARIABLE_S);

							if (variable_p)
								{


								}

							/*
							 * The form is optional
							 */
							bool success_flag = true;
							SchemaTerm *form_p = NULL;
							const json_t *form_json_p = json_object_get (phenotype_json_p, TR_FORM_S);

							if (form_json_p)
								{
									form_p = GetSchemaTermFromJSON (form_json_p);

									if (!form_p)
										{
											success_flag = false;
											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, form_json_p, "Failed to get SchemaTerm for form from JSON");
										}

								}		/* if (form_json_p) */

							if (success_flag)
								{
									bson_oid_t *id_p = GetNewUnitialisedBSONOid ();

									if (id_p)
										{
											if (GetMongoIdFromJSON (phenotype_json_p, id_p))
												{
													const char *internal_name_s = GetJSONString (phenotype_json_p, TR_INTERNAL_NAME_S);
													Treatment *treatment_p = AllocateTreatment (id_p, trait_p, measurement_p, unit_p, variable_p, form_p, internal_name_s);

													if (treatment_p)
														{
															return treatment_p;
														}
													else
														{
															PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, phenotype_json_p, "Failed to allocate Treatment");
														}


												}		/* if (GetMongoIdFromJSON (phenotype_json_p, id_p)) */
											else
												{
													PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, phenotype_json_p, "Failed to get id \"%s\"", MONGO_ID_S);
												}

											FreeBSONOid (id_p);
										}		/* if (id_p) */
									else
										{
											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, phenotype_json_p, "Failed to allocate id");
										}

								}		/* if (success_flag) */


							if (form_p)
								{
									FreeSchemaTerm (form_p);
								}

							if (variable_p)
								{
									FreeSchemaTerm (variable_p);
								}


							FreeSchemaTerm (measurement_p);
						}		/* if (measurement_p) */
					else
						{
							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, phenotype_json_p, "Failed to get SchemaTerm for \"%s\" from JSON", TR_MEASUREMENT_S);
						}

					FreeSchemaTerm (unit_p);
				}		/* if (unit_p) */
			else
				{
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, phenotype_json_p, "Failed to get SchemaTerm for \"%s\" from JSON", TR_UNIT_S);
				}

			FreeSchemaTerm (trait_p);
		}		/* if (trait_p) */
	else
		{
			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, phenotype_json_p, "Failed to get SchemaTerm for \"%s\" from JSON", TR_TRAIT_S);
		}

	return NULL;
}


OperationStatus SaveTreatment (Treatment *treatment_p, ServiceJob *job_p, const DFWFieldTrialServiceData *data_p)
{
	OperationStatus status = OS_FAILED;
	bson_t *selector_p = NULL;

	if (PrepareSaveData (& (treatment_p -> tr_id_p), &selector_p))
		{
			json_t *phenotype_json_p = GetTreatmentAsJSON (treatment_p, VF_STORAGE);

			if (phenotype_json_p)
				{
					if (SaveMongoData (data_p -> dftsd_mongo_p, phenotype_json_p, data_p -> dftsd_collection_ss [DFTD_TREATMENT], selector_p))
						{
							if (IndexData (job_p, phenotype_json_p))
								{
									status = OS_SUCCEEDED;
								}
							else
								{
									status = OS_PARTIALLY_SUCCEEDED;
									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, phenotype_json_p, "Failed to index Treatment \"%s\" as JSON to Lucene", treatment_p -> tr_internal_name_s);
								}

						}

					json_decref (phenotype_json_p);
				}		/* if (phenotype_json_p) */

		}		/* if (treatment_p -> tr_id_p) */


	return status;
}


static bool AppendSchemaTermQuery (bson_t *query_p, const char *parent_key_s, const char *child_key_s, const char *child_value_s)
{
	bool success_flag = false;
	char *compound_key_s = ConcatenateVarargsStrings (parent_key_s, ".", child_key_s, NULL);

	if (compound_key_s)
		{
			if (BSON_APPEND_UTF8 (query_p, compound_key_s, child_value_s))
				{
					success_flag = true;
				}

			FreeCopiedString (compound_key_s);
		}		/* if (compound_key_s) */

	return success_flag;
}


Treatment *GetTreatmentBySchemaURLs (const char *trait_url_s, const char *method_url_s, const char *unit_url_s, const DFWFieldTrialServiceData *data_p)
{
	Treatment *treatment_p = NULL;
	MongoTool *tool_p = data_p -> dftsd_mongo_p;

	if (SetMongoToolCollection (data_p -> dftsd_mongo_p, data_p -> dftsd_collection_ss [DFTD_TREATMENT]))
		{
			bson_t *query_p = AllocateBSON ();

			if (query_p)
				{
					if (AppendSchemaTermQuery (query_p, TR_TRAIT_S, SCHEMA_TERM_URL_S, trait_url_s))
						{
							if (AppendSchemaTermQuery (query_p, TR_MEASUREMENT_S, SCHEMA_TERM_URL_S, method_url_s))
								{
									if (AppendSchemaTermQuery (query_p, TR_UNIT_S, SCHEMA_TERM_URL_S, unit_url_s))
										{
											json_t *results_p = GetAllMongoResultsAsJSON (data_p -> dftsd_mongo_p, query_p, NULL, NULL, 1);

											if (results_p)
												{
													if (json_is_array (results_p))
														{
															const size_t num_results = json_array_size (results_p);

															if (num_results == 1)
																{
																	json_t *entry_p = json_array_get (results_p, 0);

																	treatment_p = GetTreatmentFromJSON (entry_p, data_p);

																	if (!treatment_p)
																		{

																		}		/* if (!instrument_p) */

																}		/* if (num_results == 1) */

														}		/* if (json_is_array (results_p)) */

													json_decref (results_p);
												}		/* if (results_p) */
										}
								}
						}

					FreeBSON (query_p);
				}		/* if (query_p) */

		}

	return treatment_p;
}



Treatment *GetTreatmentByIdString (const char *id_s, const DFWFieldTrialServiceData *data_p)
{
	Treatment *treatment_p = NULL;
	MongoTool *tool_p = data_p -> dftsd_mongo_p;

	if (bson_oid_is_valid (id_s, strlen (id_s)))
		{
			bson_oid_t oid;

			bson_oid_init_from_string (&oid, id_s);

			treatment_p = GetTreatmentById (&oid, data_p);
		}

	return treatment_p;
}



Treatment *GetTreatmentById (const bson_oid_t *phenotype_id_p, const DFWFieldTrialServiceData *data_p)
{
	Treatment *treatment_p = NULL;

	if (SetMongoToolCollection (data_p -> dftsd_mongo_p, data_p -> dftsd_collection_ss [DFTD_TREATMENT]))
		{
			bson_t *query_p = BCON_NEW (MONGO_ID_S, BCON_OID (phenotype_id_p));

			if (query_p)
				{
					json_t *results_p = GetAllMongoResultsAsJSON (data_p -> dftsd_mongo_p, query_p, NULL, NULL, 1);

					if (results_p)
						{
							if (json_is_array (results_p))
								{
									const size_t num_results = json_array_size (results_p);

									if (num_results == 1)
										{
											json_t *entry_p = json_array_get (results_p, 0);

											treatment_p = GetTreatmentFromJSON (entry_p, data_p);

											if (!treatment_p)
												{

												}		/* if (!treatment_p) */

										}		/* if (num_results == 1) */

								}		/* if (json_is_array (results_p)) */

							json_decref (results_p);
						}		/* if (results_p) */

					bson_destroy (query_p);
				}		/* if (query_p) */

		}		/* if (SetMongoToolCollection (data_p -> dftsd_mongo_p, data_p -> dftsd_collection_ss [DFTD_PHENOTYPE])) */

	return treatment_p;
}



/*
 * static definitions
 */


static bool AddSchemaTermToJSON (json_t *doc_p, const char * const key_s, const SchemaTerm *term_p)
{
	bool success_flag = false;
	json_t *term_json_p = GetSchemaTermAsJSON (term_p);

	if (term_json_p)
		{
			if (json_object_set_new (doc_p, key_s, term_json_p) == 0)
				{
					success_flag = true;
				}		/* if (json_object_set_new (doc_p, key_s, term_json_p) == 0) */
			else
				{
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, doc_p, "Failed to add SchemaTerm to JSON document");
					json_decref (term_json_p);
				}

		}		/* if (term_json_p) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get SchemaTerm as JSON for \"%s\"", term_p -> st_url_s);
		}

	return success_flag;
}


static SchemaTerm *GetChildSchemTermFromJSON (const json_t *doc_p, const char * const key_s)
{
	const json_t *term_json_p = json_object_get (doc_p, key_s);

	if (term_json_p)
		{
			SchemaTerm *term_p = GetSchemaTermFromJSON (term_json_p);

			if (term_p)
				{
					return term_p;
				}
			else
				{
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, term_json_p, "Failed to get SchemaTerm from JSON");
				}
		}		/* if (term_json_p) */
	else
		{
			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, term_json_p, "Failed to get \"%s\" for SchemaTerm child", key_s);
		}

	return NULL;
}


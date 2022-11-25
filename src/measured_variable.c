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


#define ALLOCATE_MEASURED_VARIABLE_TAGS (1)
#include "measured_variable.h"
#include "memory_allocations.h"
#include "string_utils.h"
#include "dfw_util.h"
#include "time_util.h"
#include "indexing.h"

/*
 * static declarations
 */

static bool CreateInstrumentFromMeasuredVariableJSON (const json_t *phenotype_json_p, Instrument **instrument_pp, const FieldTrialServiceData *data_p);

static bool AppendSchemaTermToJSON (json_t *doc_p, const char * const key_s, const SchemaTerm *term_p);

static SchemaTerm *GetChildSchemTermFromJSON (const json_t *doc_p, const char * const key_s);

static bool AppendSchemaTermQuery (bson_t *query_p, const char *parent_key_s, const char *child_key_s, const char *child_value_s);


static bool AddCommonTermsToJSON (const MeasuredVariable *mv_p, json_t *phenotype_json_p);


/*
static UnitTerm *GetUnitTermFromJSON (const json_t *phenotype_json_p, const FieldTrialServiceData *data_p)
{
	UnitTerm *unit_p = (UnitTerm *) AllocMemory (sizeof (UnitTerm));

	if (unit_p)
		{
			if (SetSchemaTermValues (& (unit_p -> ut_base_term), const char *url_s, const char *name_s, const char *description_s))
				{

				}

		}

}
*/


/*
 * API definitions
 */
MeasuredVariable *AllocateMeasuredVariable (bson_oid_t *id_p, SchemaTerm *trait_p, SchemaTerm *measurement_p, SchemaTerm *unit_p, SchemaTerm *variable_p, const ScaleClass *class_p)
{
	MeasuredVariable *treatment_p = (MeasuredVariable *) AllocMemory (sizeof (MeasuredVariable));

	if (treatment_p)
		{
			treatment_p -> mv_id_p = id_p;
			treatment_p -> mv_trait_term_p = trait_p;
			treatment_p -> mv_measurement_term_p = measurement_p;
			treatment_p -> mv_unit_term_p = unit_p;
			treatment_p -> mv_variable_term_p = variable_p;
			treatment_p -> mv_scale_class_p = class_p;

			return treatment_p;
		}		/* if (treatment_p) */

	return NULL;
}



void FreeMeasuredVariable (MeasuredVariable *treatment_p)
{
	if (treatment_p -> mv_id_p)
		{
			FreeBSONOid (treatment_p -> mv_id_p);
		}

	if (treatment_p -> mv_trait_term_p)
		{
			FreeSchemaTerm (treatment_p -> mv_trait_term_p);
		}

	if (treatment_p -> mv_measurement_term_p)
		{
			FreeSchemaTerm (treatment_p -> mv_measurement_term_p);
		}

	if (treatment_p -> mv_unit_term_p)
		{
			FreeSchemaTerm (treatment_p -> mv_unit_term_p);
		}

	if (treatment_p -> mv_variable_term_p)
		{
			FreeSchemaTerm (treatment_p -> mv_variable_term_p);
		}



	/*
	 * Still need to deallocate the instrument
	 *
	 * treatment_p -> mv_instrument_p
	 */

	FreeMemory (treatment_p);
}



json_t *GetMeasuredVariableAsJSON (const MeasuredVariable *mv_p, const ViewFormat format)
{
	json_t *phenotype_json_p = json_object ();

	if (phenotype_json_p)
		{
			if (AddDatatype (phenotype_json_p, DFTD_MEASURED_VARIABLE))
				{
					bool success_flag = false;

					switch (format)
						{
							case VF_STORAGE:
								{
									if (AddCommonTermsToJSON (mv_p, phenotype_json_p))
										{
											if (AddCompoundIdToJSON (phenotype_json_p, mv_p -> mv_id_p))
												{
													if (mv_p -> mv_scale_class_p)
														{
															json_t *scale_json_p = GetScaleClassAsJSON (mv_p -> mv_scale_class_p);

															if (scale_json_p)
																{
																	if (json_object_set_new (phenotype_json_p, MV_SCALE_S, scale_json_p) == 0)
																		{
																			success_flag = true;
																		}
																	else
																		{
																			json_decref (scale_json_p);
																		}
																}
														}

												}		/* if (AddCompoundIdToJSON (phenotype_json_p, mv_p -> mv_id_p)) */
											else
												{
													PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, phenotype_json_p, "AddCompoundIdToJSON () failed for \"%s\" to JSON", mv_p -> mv_variable_term_p -> st_url_s);
												}

										}		/* if (AddCommonTermsToJSON (mv_p, phenotype_json_p)) */

								}		/* case VF_STORAGE: */
								break;

							case VF_CLIENT_FULL:
							case VF_INDEXING:
								{
									success_flag = AddCommonTermsToJSON (mv_p, phenotype_json_p);
								}		/* case VF_CLIENT_FULL: */
								break;

							case VF_CLIENT_MINIMAL:
								{
									const char *variable_s = GetMeasuredVariableName (mv_p);

									if (SetJSONString (phenotype_json_p, MV_VARIABLE_S, variable_s))
										{
											success_flag = true;
										}
									else
										{
											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, phenotype_json_p, MV_VARIABLE_S, variable_s);
										}

								}		/* case VF_CLIENT_FULL: */
								break;

						}		/* switch (format) */


					if (success_flag)
						{
							return phenotype_json_p;
						}		/* if (success_flag) */


				}		/* if (AddDatatype (phenotype_json_p, DFTD_MEASURED_VARIABLE)) */

			json_decref (phenotype_json_p);
		}		/* if (phenotype_json_p) */

	return NULL;
}


MeasuredVariable *GetMeasuredVariableFromJSON (const json_t *phenotype_json_p, const FieldTrialServiceData *data_p)
{
	SchemaTerm *trait_p = GetChildSchemTermFromJSON (phenotype_json_p, MV_TRAIT_S);

	if (trait_p)
		{
			SchemaTerm *unit_p = GetChildSchemTermFromJSON (phenotype_json_p, MV_UNIT_S);

			if (unit_p)
				{
					SchemaTerm *measurement_p = GetChildSchemTermFromJSON (phenotype_json_p, MV_MEASUREMENT_S);

					if (measurement_p)
						{
							SchemaTerm *variable_p = GetChildSchemTermFromJSON (phenotype_json_p, MV_VARIABLE_S);

							if (variable_p)
								{


								}

							/*
							 * The form is optional
							 */
							bool success_flag = true;
							SchemaTerm *form_p = NULL;
							const json_t *form_json_p = json_object_get (phenotype_json_p, MV_FORM_S);

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
													const ScaleClass *class_p = NULL;
													MeasuredVariable *treatment_p = NULL;

													const json_t *scale_json_p = json_object_get (phenotype_json_p, MV_SCALE_S);

													if (scale_json_p)
														{
															class_p = GetScaleClassFromJSON (scale_json_p);

															if (!class_p)
																{
																	PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, scale_json_p, "Failed to get scale class");
																}
														}

													treatment_p = AllocateMeasuredVariable (id_p, trait_p, measurement_p, unit_p, variable_p, class_p);

													if (treatment_p)
														{
															return treatment_p;
														}
													else
														{
															PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, phenotype_json_p, "Failed to allocate MeasuredVariable");
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
							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, phenotype_json_p, "Failed to get SchemaTerm for \"%s\" from JSON", MV_MEASUREMENT_S);
						}

					FreeSchemaTerm (unit_p);
				}		/* if (unit_p) */
			else
				{
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, phenotype_json_p, "Failed to get SchemaTerm for \"%s\" from JSON", MV_UNIT_S);
				}

			FreeSchemaTerm (trait_p);
		}		/* if (trait_p) */
	else
		{
			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, phenotype_json_p, "Failed to get SchemaTerm for \"%s\" from JSON", MV_TRAIT_S);
		}

	return NULL;
}


OperationStatus SaveMeasuredVariable (MeasuredVariable *treatment_p, ServiceJob *job_p, const FieldTrialServiceData *data_p)
{
	OperationStatus status = OS_FAILED;
	bson_t *selector_p = NULL;

	if (PrepareSaveData (& (treatment_p -> mv_id_p), &selector_p))
		{
			json_t *phenotype_json_p = GetMeasuredVariableAsJSON (treatment_p, VF_STORAGE);

			if (phenotype_json_p)
				{
					if (SaveMongoDataWithTimestamp (data_p -> dftsd_mongo_p, phenotype_json_p, data_p -> dftsd_collection_ss [DFTD_MEASURED_VARIABLE], selector_p, DFT_TIMESTAMP_S))
						{
							status = IndexData (job_p, phenotype_json_p);

							if (status != OS_SUCCEEDED)
								{
									status = OS_PARTIALLY_SUCCEEDED;
									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, phenotype_json_p, "Failed to index Measured Variable \"%s\" as JSON to Lucene", treatment_p -> mv_variable_term_p -> st_name_s);
									AddGeneralErrorMessageToServiceJob (job_p, "Measured Variable saved but failed to index for searching");
								}

						}

					json_decref (phenotype_json_p);
				}		/* if (phenotype_json_p) */

		}		/* if (treatment_p -> mv_id_p) */

	SetServiceJobStatus (job_p, status);

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


MeasuredVariable *GetMeasuredVariableBySchemaURLs (const char *trait_url_s, const char *method_url_s, const char *unit_url_s, const FieldTrialServiceData *data_p)
{
	MeasuredVariable *treatment_p = NULL;

	if (SetMongoToolCollection (data_p -> dftsd_mongo_p, data_p -> dftsd_collection_ss [DFTD_MEASURED_VARIABLE]))
		{
			bson_t *query_p = AllocateBSON ();

			if (query_p)
				{
					if (AppendSchemaTermQuery (query_p, MV_TRAIT_S, SCHEMA_TERM_URL_S, trait_url_s))
						{
							if (AppendSchemaTermQuery (query_p, MV_MEASUREMENT_S, SCHEMA_TERM_URL_S, method_url_s))
								{
									if (AppendSchemaTermQuery (query_p, MV_UNIT_S, SCHEMA_TERM_URL_S, unit_url_s))
										{
											json_t *results_p = GetAllMongoResultsAsJSON (data_p -> dftsd_mongo_p, query_p, NULL);

											if (results_p)
												{
													if (json_is_array (results_p))
														{
															const size_t num_results = json_array_size (results_p);

															if (num_results == 1)
																{
																	json_t *entry_p = json_array_get (results_p, 0);

																	treatment_p = GetMeasuredVariableFromJSON (entry_p, data_p);

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



MeasuredVariable *GetMeasuredVariableByIdString (const char *id_s, const FieldTrialServiceData *data_p)
{
	MeasuredVariable *treatment_p = NULL;

	if (bson_oid_is_valid (id_s, strlen (id_s)))
		{
			bson_oid_t oid;

			bson_oid_init_from_string (&oid, id_s);

			treatment_p = GetMeasuredVariableById (&oid, data_p);
		}

	return treatment_p;
}


MeasuredVariable *GetMeasuredVariableById (const bson_oid_t *phenotype_id_p, const FieldTrialServiceData *data_p)
{
	MeasuredVariable *treatment_p = NULL;

	if (SetMongoToolCollection (data_p -> dftsd_mongo_p, data_p -> dftsd_collection_ss [DFTD_MEASURED_VARIABLE]))
		{
			bson_t *query_p = BCON_NEW (MONGO_ID_S, BCON_OID (phenotype_id_p));

			if (query_p)
				{
					json_t *results_p = GetAllMongoResultsAsJSON (data_p -> dftsd_mongo_p, query_p, NULL);

					if (results_p)
						{
							if (json_is_array (results_p))
								{
									const size_t num_results = json_array_size (results_p);

									if (num_results == 1)
										{
											json_t *entry_p = json_array_get (results_p, 0);

											treatment_p = GetMeasuredVariableFromJSON (entry_p, data_p);

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



const char *GetMeasuredVariableName (const MeasuredVariable *mv_p)
{
	const char *name_s = NULL;

	if (mv_p -> mv_variable_term_p)
		{
			name_s = mv_p -> mv_variable_term_p -> st_name_s;
		}

	return name_s;
}


const char *GetMeasuredVariableURL (const MeasuredVariable *mv_p)
{
	const char *name_s = NULL;

	if (mv_p -> mv_variable_term_p)
		{
			name_s = mv_p -> mv_variable_term_p -> st_url_s;
		}

	return name_s;
}






MeasuredVariableNode *AllocateMeasuredVariableNode (MeasuredVariable *variable_p, MEM_FLAG mv_mem)
{
	char *id_s = GetBSONOidAsString (variable_p -> mv_id_p);

	if (id_s)
		{
			MeasuredVariableNode *mv_node_p = (MeasuredVariableNode *) AllocMemory (sizeof (MeasuredVariableNode));

			if (mv_node_p)
				{
					InitListItem (& (mv_node_p -> mvn_node));

					mv_node_p -> mvn_measured_variable_p = variable_p;
					mv_node_p -> mvn_measured_variable_mem = mv_mem;
					mv_node_p -> mvn_id_s = id_s;

					return mv_node_p;
				}

			FreeBSONOidString (id_s);
		}
	else
		{
			const char *name_s = GetMeasuredVariableName (variable_p);
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "GetBSONOidAsString () failed for MeasuredVariable \"%s\"", name_s ? name_s : "");
		}

	return NULL;
}

void FreeMeasuredVariableNode (ListItem *node_p)
{
	MeasuredVariableNode *mv_node_p = (MeasuredVariableNode *) node_p;

	if (mv_node_p -> mvn_id_s)
		{
			FreeBSONOidString (mv_node_p -> mvn_id_s);
		}

	if (mv_node_p -> mvn_measured_variable_p)
		{
			if ((mv_node_p -> mvn_measured_variable_mem == MF_DEEP_COPY) || (mv_node_p -> mvn_measured_variable_mem == MF_SHALLOW_COPY))
				{
					FreeMeasuredVariable (mv_node_p -> mvn_measured_variable_p);
				}
		}

	FreeMemory (mv_node_p);
}


const ScaleClass *GetMeasuredVariableScaleClass (const MeasuredVariable * const variable_p)
{
	return variable_p -> mv_scale_class_p;
}


/*
 * static definitions
 */


static bool AppendSchemaTermToJSON (json_t *doc_p, const char * const key_s, const SchemaTerm *term_p)
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



static bool AddCommonTermsToJSON (const MeasuredVariable *mv_p, json_t *phenotype_json_p)
{
	bool success_flag = false;

	if (AppendSchemaTermToJSON (phenotype_json_p, MV_VARIABLE_S, mv_p -> mv_variable_term_p))
		{
			if (AppendSchemaTermToJSON (phenotype_json_p, MV_TRAIT_S, mv_p -> mv_trait_term_p))
				{
					if (AppendSchemaTermToJSON (phenotype_json_p, MV_MEASUREMENT_S, mv_p -> mv_measurement_term_p))
						{
							if (AppendSchemaTermToJSON (phenotype_json_p, MV_UNIT_S, mv_p -> mv_unit_term_p))
								{
									success_flag = true;

								}		/* if (AddSchemTermToJSON (phenotype_json_p, MV_UNIT_S, mv_p -> mv_unit_term_p)) */
							else
								{
									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, phenotype_json_p, "Failed to add SchemaTerm for \"%s\" to JSON", mv_p -> mv_unit_term_p -> st_url_s);
								}

						}		/* if (AddSchemTermToJSON (phenotype_json_p, MV_MEASUREMENT_S, mv_p -> mv_measurement_term_p)) */
					else
						{
							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, phenotype_json_p, "Failed to add SchemaTerm for \"%s\" to JSON", mv_p -> mv_measurement_term_p -> st_url_s);
						}

				}		/* if (AddSchemTermToJSON (phenotype_json_p, MV_TRAIT_S, mv_p -> mv_trait_term_p)) */
			else
				{
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, phenotype_json_p, "Failed to add SchemaTerm for \"%s\" to JSON", mv_p -> mv_trait_term_p -> st_url_s);
				}

		}		/* if (AppendSchemaTermToJSON (phenotype_json_p, MV_VARIABLE_S, mv_p -> mv_variable_term_p)) */
	else
		{
			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, phenotype_json_p, "Failed to add SchemaTerm for \"%s\" to JSON", mv_p -> mv_variable_term_p -> st_url_s);
		}

	return success_flag;
}


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
#include "phenotype.h"
#include "memory_allocations.h"
#include "string_utils.h"
#include "dfw_util.h"
#include "time_util.h"

/*
 * static declarations
 */

static bool CreateInstrumentFromPhenotypeJSON (const json_t *phenotype_json_p, Instrument **instrument_pp, const DFWFieldTrialServiceData *data_p);

static bool AddSchemTermToJSON (json_t *doc_p, const char * const key_s, const SchemaTerm *term_p);

static SchemaTerm *GetChildSchemTermFromJSON (const json_t *doc_p, const char * const key_s);


/*
 * API definitions
 */
Phenotype *AllocatePhenotype (bson_oid_t *id_p, SchemaTerm *trait_p, SchemaTerm *measurement_p, SchemaTerm *unit_p, SchemaTerm *form_p, const char *internal_name_s)
{
	char *copied_internal_name_s = NULL;

	if ((IsStringEmpty (internal_name_s)) || ((copied_internal_name_s = EasyCopyToNewString (internal_name_s)) != NULL))
		{
			Phenotype *phenotype_p = (Phenotype *) AllocMemory (sizeof (Phenotype));

			if (phenotype_p)
				{
					phenotype_p -> ph_id_p = id_p;
					phenotype_p -> ph_trait_term_p = trait_p;
					phenotype_p -> ph_measurement_term_p = measurement_p;
					phenotype_p -> ph_unit_term_p = unit_p;
					phenotype_p -> ph_form_term_p = form_p;
					phenotype_p -> ph_internal_name_s = copied_internal_name_s;

					return phenotype_p;
				}		/* if (phenotype_p) */

			if (copied_internal_name_s)
				{
					FreeCopiedString (copied_internal_name_s);
				}

		}		/* if ((IsStringEmpty (internal_name_s)) || ((copied_internal_name_s = EasyCopyToNewString (internal_name_s)) != NULL)) */

	return NULL;
}



void FreePhenotype (Phenotype *phenotype_p)
{
	if (phenotype_p -> ph_id_p)
		{
			FreeBSONOid (phenotype_p -> ph_id_p);
		}

	if (phenotype_p -> ph_trait_term_p)
		{
			FreeSchemaTerm (phenotype_p -> ph_trait_term_p);
		}

	if (phenotype_p -> ph_measurement_term_p)
		{
			FreeSchemaTerm (phenotype_p -> ph_measurement_term_p);
		}

	if (phenotype_p -> ph_unit_term_p)
		{
			FreeSchemaTerm (phenotype_p -> ph_unit_term_p);
		}

	if (phenotype_p -> ph_internal_name_s)
		{
			FreeCopiedString (phenotype_p -> ph_internal_name_s);
		}


	/*
	 * Still need to deallocate the instrument
	 *
	 * phenotype_p -> ph_instrument_p
	 */

	FreeMemory (phenotype_p);
}



json_t *GetPhenotypeAsJSON (const Phenotype *phenotype_p, const ViewFormat format)
{
	json_t *phenotype_json_p = json_object ();

	if (phenotype_json_p)
		{
			if (AddSchemTermToJSON (phenotype_json_p, PH_TRAIT_S, phenotype_p -> ph_trait_term_p))
				{
					if (AddSchemTermToJSON (phenotype_json_p, PH_MEASUREMENT_S, phenotype_p -> ph_measurement_term_p))
						{
							if (AddSchemTermToJSON (phenotype_json_p, PH_UNIT_S, phenotype_p -> ph_unit_term_p))
								{
									/*
									 * The form term is optional
									 */
									if ((! (phenotype_p -> ph_form_term_p)) || (AddSchemTermToJSON (phenotype_json_p, PH_FORM_S, phenotype_p -> ph_form_term_p)))
										{
											bool success_flag = false;

											if (format == VF_STORAGE)
												{
													if ((IsStringEmpty (phenotype_p -> ph_internal_name_s)) || (SetJSONString (phenotype_json_p, PH_INTERNAL_NAME_S, phenotype_p -> ph_internal_name_s)))
														{
															success_flag = true;
														}		/* if ((IsStringEmpty (phenotype_p -> ph_internal_name_s)) || SetJSONString (phenotype_json_p, PH_INTERNAL_NAME_S, phenotype_p -> ph_internal_name_s)) */
												}
											else
												{
													success_flag = true;
												}

											if (success_flag)
												{
													if (AddCompoundIdToJSON (phenotype_json_p, phenotype_p -> ph_id_p))
														{
															if (AddDatatype (phenotype_json_p, DFTD_PHENOTYPE))
																{
																	return phenotype_json_p;
																}

															return phenotype_json_p;
														}		/* if (AddCompoundIdToJSON (phenotype_json_p, phenotype_p -> ph_id_p)) */

												}		/* if (success_flag) */

										}		/* if ((! (phenotype_p -> ph_form_term_p)) || (AddSchemTermToJSON (phenotype_json_p, PH_FORM_S, phenotype_p -> ph_form_term_p))) */
									else
										{
											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, phenotype_json_p, "Failed to add SchemaTerm for \"%s\" to JSON", phenotype_p -> ph_form_term_p -> st_url_s);
										}

								}		/* if (AddSchemTermToJSON (phenotype_json_p, PH_UNIT_S, phenotype_p -> ph_unit_term_p)) */
							else
								{
									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, phenotype_json_p, "Failed to add SchemaTerm for \"%s\" to JSON", phenotype_p -> ph_unit_term_p -> st_url_s);
								}

						}		/* if (AddSchemTermToJSON (phenotype_json_p, PH_MEASUREMENT_S, phenotype_p -> ph_measurement_term_p)) */
					else
						{
							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, phenotype_json_p, "Failed to add SchemaTerm for \"%s\" to JSON", phenotype_p -> ph_measurement_term_p -> st_url_s);
						}

				}		/* if (AddSchemTermToJSON (phenotype_json_p, PH_TRAIT_S, phenotype_p -> ph_trait_term_p)) */
			else
				{
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, phenotype_json_p, "Failed to add SchemaTerm for \"%s\" to JSON", phenotype_p -> ph_trait_term_p -> st_url_s);
				}

			json_decref (phenotype_json_p);
		}		/* if (phenotype_json_p) */

	return NULL;
}


Phenotype *GetPhenotypeFromJSON (const json_t *phenotype_json_p, const DFWFieldTrialServiceData *data_p)
{
	SchemaTerm *trait_p = GetChildSchemTermFromJSON (phenotype_json_p, PH_TRAIT_S);

	if (trait_p)
		{
			SchemaTerm *unit_p = GetChildSchemTermFromJSON (phenotype_json_p, PH_UNIT_S);

			if (unit_p)
				{
					SchemaTerm *measurement_p = GetChildSchemTermFromJSON (phenotype_json_p, PH_MEASUREMENT_S);

					if (measurement_p)
						{
							/*
							 * The form is optional
							 */
							bool success_flag = true;
							SchemaTerm *form_p = NULL;
							const json_t *form_json_p = json_object_get (phenotype_json_p, PH_FORM_S);

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
													const char *internal_name_s = GetJSONString (phenotype_json_p, PH_INTERNAL_NAME_S);

													if (internal_name_s)
														{
															Phenotype *phenotype_p = AllocatePhenotype (id_p, trait_p, measurement_p, unit_p, form_p, internal_name_s);

															if (phenotype_p)
																{
																	return phenotype_p;
																}
															else
																{
																	PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, phenotype_json_p, "Failed to allocate Phenotype");
																}

														}		/* if (internal_name_s) */
													else
														{
															PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, phenotype_json_p, "Failed to get internal name \"%s\"", PH_INTERNAL_NAME_S);
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

							FreeSchemaTerm (measurement_p);
						}		/* if (measurement_p) */
					else
						{
							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, phenotype_json_p, "Failed to get SchemaTerm for \"%s\" from JSON", PH_MEASUREMENT_S);
						}

					FreeSchemaTerm (unit_p);
				}		/* if (unit_p) */
			else
				{
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, phenotype_json_p, "Failed to get SchemaTerm for \"%s\" from JSON", PH_UNIT_S);
				}

			FreeSchemaTerm (trait_p);
		}		/* if (trait_p) */
	else
		{
			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, phenotype_json_p, "Failed to get SchemaTerm for \"%s\" from JSON", PH_TRAIT_S);
		}

	return NULL;
}


bool SavePhenotype (Phenotype *phenotype_p, const DFWFieldTrialServiceData *data_p)
{
	bson_t *selector_p = NULL;
	bool success_flag = PrepareSaveData (& (phenotype_p -> ph_id_p), &selector_p);

	if (success_flag)
		{
			json_t *phenotype_json_p = GetPhenotypeAsJSON (phenotype_p, VF_STORAGE);

			if (phenotype_json_p)
				{
					success_flag = SaveMongoData (data_p -> dftsd_mongo_p, phenotype_json_p, data_p -> dftsd_collection_ss [DFTD_PHENOTYPE], selector_p);

					json_decref (phenotype_json_p);
				}		/* if (phenotype_json_p) */

		}		/* if (phenotype_p -> ph_id_p) */

	return success_flag;
}



Phenotype *GetPhenotypeById (const bson_oid_t *phenotype_id_p, const DFWFieldTrialServiceData *data_p)
{
	Phenotype *phenotype_p = NULL;

	if (SetMongoToolCollection (data_p -> dftsd_mongo_p, data_p -> dftsd_collection_ss [DFTD_PHENOTYPE]))
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

											phenotype_p = GetPhenotypeFromJSON (entry_p, data_p);

											if (!DFTD_PHENOTYPE)
												{

												}		/* if (!instrument_p) */

										}		/* if (num_results == 1) */

								}		/* if (json_is_array (results_p)) */

							json_decref (results_p);
						}		/* if (results_p) */

					bson_destroy (query_p);
				}		/* if (query_p) */

		}		/* if (SetMongoToolCollection (data_p -> dftsd_mongo_p, data_p -> dftsd_collection_ss [DFTD_PHENOTYPE])) */

	return phenotype_p;
}



/*
 * static definitions
 */


static bool AddSchemTermToJSON (json_t *doc_p, const char * const key_s, const SchemaTerm *term_p)
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


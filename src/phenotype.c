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

Phenotype *AllocatePhenotype (bson_oid_t *id_p, const struct tm *date_p, SchemaTerm *trait_p, SchemaTerm *measurement_p, SchemaTerm *unit_p, const char *value_s,
															const char *growth_stage_s, const bool corrected_value_flag, const char *method_s, const char *internal_name_s, Instrument *instrument_p)
{
	if (date_p)
		{
			struct tm *copied_date_p = DuplicateTime (date_p);

			if (copied_date_p)
				{
					char *copied_value_s = EasyCopyToNewString (value_s);

					if (copied_value_s)
						{
							char *copied_growth_stage_s = NULL;

							if ((IsStringEmpty (growth_stage_s)) || ((copied_growth_stage_s = EasyCopyToNewString (growth_stage_s)) != NULL))
								{
									char *copied_method_s = NULL;

									if ((IsStringEmpty (method_s)) || ((copied_method_s = EasyCopyToNewString (method_s)) != NULL))
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
															phenotype_p -> ph_measured_value_s = copied_value_s;
															phenotype_p -> ph_date_p = copied_date_p;
															phenotype_p -> ph_instrument_p = instrument_p;
															phenotype_p -> ph_growth_stage_s = copied_growth_stage_s;
															phenotype_p -> ph_corrected_flag = corrected_value_flag;
															phenotype_p -> ph_method_s = copied_method_s;
															phenotype_p -> ph_internal_name_s = copied_internal_name_s;

															return phenotype_p;
														}		/* if (phenotype_p) */

													if (copied_internal_name_s)
														{
															FreeCopiedString (copied_internal_name_s);
														}

												}		/* if ((IsStringEmpty (internal_name_s)) || ((copied_internal_name_s = EasyCopyToNewString (internal_name_s)) != NULL)) */

											if (copied_method_s)
												{
													FreeCopiedString (copied_method_s);
												}

										}		/* if ((IsStringEmpty (method_s)) || ((copied_method_s = EasyCopyToNewString (method_s)) != NULL)) */

									if (copied_growth_stage_s)
										{
											FreeCopiedString (copied_growth_stage_s);
										}

								}		/* if ((IsStringEmpty (growth_stage_s)) || ((copied_growth_stage_s = EasyCopyToNewString (growth_stage_s)) != NULL)) */

							FreeCopiedString (copied_value_s);
						}		/* if (copied_value_s) */

					FreeTime (copied_date_p);
				}		/* if (copied_date_p) */

		}		/* if (date_p) */

	return NULL;
}


Phenotype *AllocatePhenotypeFromDefinition (bson_oid_t *id_p, SchemaTerm *trait_p, SchemaTerm *measurement_p, SchemaTerm *unit_p, const char *internal_name_s)
{
	char *copied_internal_name_s = EasyCopyToNewString (internal_name_s);

	if (copied_internal_name_s)
		{
			Phenotype *phenotype_p = (Phenotype *) AllocMemory (sizeof (Phenotype));

			if (phenotype_p)
				{
					phenotype_p -> ph_id_p = id_p;
					phenotype_p -> ph_trait_term_p = trait_p;
					phenotype_p -> ph_measurement_term_p = measurement_p;
					phenotype_p -> ph_unit_term_p = unit_p;
					phenotype_p -> ph_measured_value_s = NULL;
					phenotype_p -> ph_date_p = NULL;
					phenotype_p -> ph_instrument_p = NULL;
					phenotype_p -> ph_growth_stage_s = NULL;
					phenotype_p -> ph_corrected_flag = false;
					phenotype_p -> ph_method_s = NULL;
					phenotype_p -> ph_internal_name_s = copied_internal_name_s;

					return phenotype_p;
				}		/* if (phenotype_p) */

			if (copied_internal_name_s)
				{
					FreeCopiedString (copied_internal_name_s);
				}
		}

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

	if (phenotype_p -> ph_growth_stage_s)
		{
			FreeCopiedString (phenotype_p -> ph_growth_stage_s);
		}

	if (phenotype_p -> ph_measured_value_s)
		{
			FreeCopiedString (phenotype_p -> ph_measured_value_s);
		}

	if (phenotype_p -> ph_date_p)
		{
			FreeTime (phenotype_p -> ph_date_p);
		}

	if (phenotype_p -> ph_method_s)
		{
			FreeCopiedString (phenotype_p -> ph_method_s);
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


json_t *GetPhenotypeAsJSON (const Phenotype *phenotype_p, const bool expand_fields_flag)
{
	json_t *phenotype_json_p = json_object ();

	if (phenotype_json_p)
		{
			if (AddValidDateToJSON (phenotype_p -> ph_date_p, phenotype_json_p, PH_DATE_S))
				{
					if (AddSchemTermToJSON (phenotype_json_p, PH_TRAIT_S, phenotype_p -> ph_trait_term_p))
						{
							if (AddSchemTermToJSON (phenotype_json_p, PH_MEASUREMENT_S, phenotype_p -> ph_measurement_term_p))
								{
									if (AddSchemTermToJSON (phenotype_json_p, PH_UNIT_S, phenotype_p -> ph_unit_term_p))
										{
											if ((IsStringEmpty (phenotype_p -> ph_measured_value_s)) || (SetJSONString (phenotype_json_p, PH_VALUE_S, phenotype_p -> ph_measured_value_s)))
												{
													if ((IsStringEmpty (phenotype_p -> ph_growth_stage_s)) || (SetJSONString (phenotype_json_p, PH_GROWTH_STAGE_S, phenotype_p -> ph_growth_stage_s)))
														{
															if ((IsStringEmpty (phenotype_p -> ph_method_s)) || (SetJSONString (phenotype_json_p, PH_METHOD_S, phenotype_p -> ph_method_s)))
																{
																	if ((IsStringEmpty (phenotype_p -> ph_internal_name_s)) || (SetJSONString (phenotype_json_p, PH_INTERNAL_NAME_S, phenotype_p -> ph_internal_name_s)))
																		{
																			if (AddCompoundIdToJSON (phenotype_json_p, phenotype_p -> ph_id_p))
																				{
																					if (SetJSONBoolean (phenotype_json_p, PH_CORRECTED_S, phenotype_p -> ph_corrected_flag))
																						{
																							bool done_instrument_flag = false;

																							if (phenotype_p -> ph_instrument_p)
																								{
																									if (expand_fields_flag)
																										{
																											json_t *instrument_json_p = GetInstrumentAsJSON (phenotype_p -> ph_instrument_p);

																											if (instrument_json_p)
																												{
																													if (json_object_set_new (phenotype_json_p, PH_INSTRUMENT_S, instrument_json_p) == 0)
																														{
																															done_instrument_flag = true;
																														}		/* if (json_object_set_new (phenotype_json_p, PH_INSTRUMENT_S, instrument_json_p) == 0) */
																													else
																														{
																															json_decref (phenotype_json_p);
																														}
																												}
																										}		/* if (expand_fields_flag) */
																									else
																										{
																											if (AddNamedCompoundIdToJSON (phenotype_json_p, phenotype_p -> ph_instrument_p -> in_id_p, PH_INSTRUMENT_ID_S))
																												{
																													done_instrument_flag = true;
																												}
																										}

																								}		/* if (phenotype_p -> ph_instrument_p) */
																							else
																								{
																									done_instrument_flag = true;
																								}

																							if (done_instrument_flag)
																								{
																									return phenotype_json_p;
																								}

																						}		/* if (SetJSONBoolean (phenotype_json_p, PH_CORRECTED_S, phenotype_p -> ph_corrected_flag)) */


																				}		/* if (AddCompoundIdToJSON (phenotype_json_p, phenotype_p -> ph_id_p)) */

																		}		/* if ((IsStringEmpty (phenotype_p -> ph_internal_name_s)) || SetJSONString (phenotype_json_p, PH_INTERNAL_NAME_S, phenotype_p -> ph_internal_name_s)) */


																}		/* if ((IsStringEmpty (phenotype_p -> ph_method_s)) || SetJSONString (phenotype_json_p, PH_METHOD_S, phenotype_p -> ph_method_s)) */

														}		/* if ((IsStringEmpty (phenotype_p -> ph_growth_stage_s)) || SetJSONString (phenotype_json_p, PH_GROWTH_STAGE_S, phenotype_p -> ph_growth_stage_s)) */

												}		/* if (SetJSONString (phenotype_json_p, PH_VALUE_S, phenotype_p -> ph_measured_value_s)) */
											else
												{
													PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, phenotype_json_p, "Failed to add \"%s\": \"%s\" to JSON", PH_VALUE_S, phenotype_p -> ph_measured_value_s);
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

				}		/* if (AddValidDateToJSON (phenotype_p -> ph_date_p, phenotype_json_p, PH_DATE_S)) */


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
							const char *value_s = GetJSONString (phenotype_json_p, PH_VALUE_S);

							if (value_s)
								{
									struct tm *date_p = NULL;

									if (CreateValidDateFromJSON (phenotype_json_p, PH_DATE_S, &date_p))
										{
											bson_oid_t *id_p = GetNewUnitialisedBSONOid ();

											if (id_p)
												{
													if (GetMongoIdFromJSON (phenotype_json_p, id_p))
														{
															Instrument *instrument_p = NULL;

															if (CreateInstrumentFromPhenotypeJSON (phenotype_json_p, &instrument_p, data_p))
																{
																	Phenotype *phenotype_p = NULL;
																	bool corrected_flag;
																	const char *growth_stage_s = GetJSONString (phenotype_json_p, PH_GROWTH_STAGE_S);
																	const char *method_s = GetJSONString (phenotype_json_p, PH_METHOD_S);
																	const char *internal_name_s = GetJSONString (phenotype_json_p, PH_INTERNAL_NAME_S);

																	GetJSONBoolean (phenotype_json_p, PH_CORRECTED_S, &corrected_flag);

																	phenotype_p = AllocatePhenotype (id_p, date_p, trait_p, measurement_p, unit_p, value_s, growth_stage_s, corrected_flag, method_s, internal_name_s, instrument_p);

																	if (phenotype_p)
																		{
																			return phenotype_p;
																		}
																	else
																		{
																			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, phenotype_json_p, "Failed to allocate Phenotype");
																		}

																}		/* if (CreateInstrumentFromPhenotypeJSON (phenotype_json_p, &instrument_p, data_p)) */
															else
																{
																	PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, phenotype_json_p, "Failed to create instrument from \"%s\"", PH_DATE_S);
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

										}		/* if (CreateValidDateFromJSON (phenotype_json_p, PH_DATE_S, &date_p)) */
									else
										{
											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, phenotype_json_p, "Failed to create date from \"%s\"", PH_DATE_S);
										}

								}		/* if (value_s) */
							else
								{
									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, phenotype_json_p, "Failed to get \"%s\" from JSON", PH_VALUE_S);
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
	bool success_flag = false;
	bool insert_flag = false;

	if (! (phenotype_p -> ph_id_p))
		{
			phenotype_p -> ph_id_p  = GetNewBSONOid ();

			if (phenotype_p -> ph_id_p)
				{
					insert_flag = true;
				}
		}

	if (phenotype_p -> ph_id_p)
		{
			json_t *phenotype_json_p = GetPhenotypeAsJSON (phenotype_p, false);

			if (phenotype_json_p)
				{
					const char *collection_s = phenotype_p -> ph_corrected_flag ? data_p -> dftsd_collection_ss [DFTD_CORRECTED_PHENOTYPE] : data_p -> dftsd_collection_ss [DFTD_RAW_PHENOTYPE];
					success_flag = SaveMongoData (data_p -> dftsd_mongo_p, phenotype_json_p, collection_s, insert_flag);

					json_decref (phenotype_json_p);
				}		/* if (phenotype_json_p) */

		}		/* if (phenotype_p -> ph_id_p) */

	return success_flag;
}


/*
 * static definitions
 */

static bool CreateInstrumentFromPhenotypeJSON (const json_t *phenotype_json_p, Instrument **instrument_pp, const DFWFieldTrialServiceData *data_p)
{
	bool success_flag = false;
	Instrument *instrument_p = NULL;
	const json_t *val_p = json_object_get (phenotype_json_p, PH_INSTRUMENT_S);

	if (val_p)
		{
			instrument_p = GetInstrumentFromJSON (val_p);

			if (instrument_p)
				{
					*instrument_pp = instrument_p;
					success_flag = true;
				}
		}
	else
		{
			bson_oid_t *instrument_id_p = GetNewUnitialisedBSONOid ();

			if (instrument_id_p)
				{
					if (GetNamedIdFromJSON (phenotype_json_p, PH_INSTRUMENT_ID_S, instrument_id_p))
						{
							instrument_p = GetInstrumentById (instrument_id_p, data_p);

							if (instrument_p)
								{
									*instrument_pp = instrument_p;
									success_flag = true;
								}

						}
					else
						{
							/* no instrument in json */
							success_flag = true;
						}

					FreeBSONOid (instrument_id_p);
				}
		}


	return success_flag;
}


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



static bool AddPhenotypeNatureToJSON (const Phenotype *phenotype_p, json_t *doc_p)
{
	bool success_flag = false;

	return success_flag;
}


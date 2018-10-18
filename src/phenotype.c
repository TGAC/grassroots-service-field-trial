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



Phenotype *AllocatePhenotype (bson_oid_t *id_p, const struct tm *date_p, const char *trait_s, const char *trait_abbreviation_s, const char *measurement_s,
															const char *unit_s, const char *growth_stage_s, const bool corrected_value_flag, const char *method_s, Instrument *instrument_p)
{
	if (date_p)
		{
			struct tm *copied_date_p = DuplicateTime (date_p);

			if (copied_date_p)
				{
					char *copied_trait_s = EasyCopyToNewString (trait_s);

					if (copied_trait_s)
						{
							char *copied_measurement_s = EasyCopyToNewString (measurement_s);

							if (copied_measurement_s)
								{
									char *copied_unit_s = EasyCopyToNewString (unit_s);

									if (copied_unit_s)
										{
											char *copied_trait_abbreviation_s = NULL;

											if ((IsStringEmpty (trait_abbreviation_s)) || ((copied_trait_abbreviation_s = EasyCopyToNewString (trait_abbreviation_s)) != NULL))
												{
													char *copied_growth_stage_s = NULL;

													if ((IsStringEmpty (growth_stage_s)) || ((copied_growth_stage_s = EasyCopyToNewString (growth_stage_s)) != NULL))
														{
															char *copied_method_s = NULL;

															if ((IsStringEmpty (method_s)) || ((copied_method_s = EasyCopyToNewString (method_s)) != NULL))
																{
																	Phenotype *phenotype_p = (Phenotype *) AllocMemory (sizeof (Phenotype));

																	if (phenotype_p)
																		{
																			phenotype_p -> ph_corrected_flag = corrected_value_flag;
																			phenotype_p -> ph_date_p = copied_date_p;
																			phenotype_p -> ph_growth_stage_s = copied_growth_stage_s;
																			phenotype_p -> ph_id_p = id_p;
																			phenotype_p -> ph_instrument_p = instrument_p;
																			phenotype_p -> ph_measurement_s = copied_measurement_s;
																			phenotype_p -> ph_method_s = copied_method_s;
																			phenotype_p -> ph_trait_abbreviation_s = copied_trait_abbreviation_s;
																			phenotype_p -> ph_trait_s = copied_trait_s;
																			phenotype_p -> ph_unit_s = copied_unit_s;

																			return phenotype_p;
																		}		/* if (phenotype_p) */

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

													if (copied_trait_abbreviation_s)
														{
															FreeCopiedString (copied_trait_abbreviation_s);
														}

												}		/* if ((IsStringEmpty (trait_abbreviation_s)) || ((copied_trait_abbreviation_s = EasyCopyToNewString (trait_abbreviation_s)) != NULL)) */

											FreeCopiedString (copied_unit_s);
										}		/* if (copied_unit_s) */

									FreeCopiedString (copied_measurement_s);
								}		/* if (copied_measurement_s) */

							FreeCopiedString (copied_trait_s);
						}		/* if (copied_trait_s) */

					FreeTime (copied_date_p);
				}		/* if (copied_date_p) */

		}		/* if (date_p) */

	return NULL;
}


void FreePhenotype (Phenotype *phenotype_p)
{
	if (phenotype_p -> ph_id_p)
		{
			FreeBSONOid (phenotype_p -> ph_id_p);
		}

	if (phenotype_p -> ph_growth_stage_s)
		{
			FreeCopiedString (phenotype_p -> ph_growth_stage_s);
		}

	if (phenotype_p -> ph_measurement_s)
		{
			FreeCopiedString (phenotype_p -> ph_measurement_s);
		}

	if (phenotype_p -> ph_trait_abbreviation_s)
		{
			FreeCopiedString (phenotype_p -> ph_trait_abbreviation_s);
		}

	if (phenotype_p -> ph_trait_s)
		{
			FreeCopiedString (phenotype_p -> ph_trait_s);
		}

	if (phenotype_p -> ph_unit_s)
		{
			FreeCopiedString (phenotype_p -> ph_unit_s);
		}

	if (phenotype_p -> ph_growth_stage_s)
		{
			FreeCopiedString (phenotype_p -> ph_growth_stage_s);
		}

	if (phenotype_p -> ph_method_s)
		{
			FreeCopiedString (phenotype_p -> ph_method_s);
		}

	FreeMemory (phenotype_p);
}



json_t *GetPhenotypeAsJSON (const Phenotype *phenotype_p, const bool expand_fields_flag)
{
	json_t *phenotype_json_p = json_object ();

	if (phenotype_json_p)
		{
			if (AddValidDateToJSON (phenotype_p -> ph_date_p, phenotype_json_p, PH_DATE_S))
				{
					if (SetJSONString (phenotype_json_p, PH_TRAIT_S, phenotype_p -> ph_trait_s))
						{
							if (SetJSONString (phenotype_json_p, PH_MEASUREMENT_S, phenotype_p -> ph_measurement_s))
								{
									if (SetJSONString (phenotype_json_p, PH_UNIT_S, phenotype_p -> ph_unit_s))
										{
											if ((IsStringEmpty (phenotype_p -> ph_growth_stage_s)) || SetJSONString (phenotype_json_p, PH_GROWTH_STAGE_S, phenotype_p -> ph_growth_stage_s))
												{
													if ((IsStringEmpty (phenotype_p -> ph_trait_abbreviation_s)) || SetJSONString (phenotype_json_p, PH_TRAIT_ABBREVIATION_S, phenotype_p -> ph_trait_abbreviation_s))
														{
															if ((IsStringEmpty (phenotype_p -> ph_method_s)) || SetJSONString (phenotype_json_p, PH_METHOD_S, phenotype_p -> ph_method_s))
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

																}		/* if ((IsStringEmpty (phenotype_p -> ph_method_s)) || SetJSONString (phenotype_json_p, PH_METHOD_S, phenotype_p -> ph_method_s)) */

														}		/* if ((IsStringEmpty (phenotype_p -> ph_trait_abbreviation_s)) || SetJSONString (phenotype_json_p, PH_TRAIT_ABBREVIATION_S, phenotype_p -> ph_trait_abbreviation_s)) */

												}		/* if ((IsStringEmpty (phenotype_p -> ph_growth_stage_s)) || SetJSONString (phenotype_json_p, PH_GROWTH_STAGE_S, phenotype_p -> ph_growth_stage_s)) */

										}		/* if (SetJSONString (phenotype_json_p, PH_UNIT_S, phenotype_p -> ph_unit_s)) */

								}		/* if (SetJSONString (phenotype_json_p, PH_MEASUREMENT_S, phenotype_p -> ph_measurement_s)) */

						}		/* if (SetJSONString (phenotype_json_p, PH_TRAIT_S, phenotype_p -> ph_trait_s)) */


				}		/* if (AddValidDateToJSON (phenotype_p -> ph_date_p, phenotype_json_p, PH_DATE_S)) */


			json_decref (phenotype_json_p);
		}		/* if (phenotype_json_p) */

	return NULL;
}


Phenotype *GetPhenotypeFromJSON (const json_t *phenotype_json_p)
{
	const char *trait_s = GetJSONString (phenotype_json_p, PH_TRAIT_S);

	if (trait_s)
		{
			const char *unit_s = GetJSONString (phenotype_json_p, PH_UNIT_S);

			if (unit_s)
				{
					const char *measurement_s = GetJSONString (phenotype_json_p, PH_MEASUREMENT_S);

					if (measurement_s)
						{
							struct tm *date_p = NULL;

							if (CreateValidDateFromJSON (phenotype_json_p, PH_DATE_S, &date_p))
								{
									bool corrected_flag;

									if (GetJSONBoolean (phenotype_json_p, PH_CORRECTED_S, &corrected_flag))
										{
											bson_oid_t *id_p = GetNewUnitialisedBSONOid ();

											if (id_p)
												{
													if (GetMongoIdFromJSON (phenotype_json_p, id_p))
														{


														}		/* if (GetMongoIdFromJSON (phenotype_json_p, id_p)) */

												}		/* if (id_p) */

										}		/* if (GetJSONBoolean (phenotype_json_p, PH_CORRECTED_S, &corrected_flag)) */

								}		/* if (CreateValidDateFromJSON (phenotype_json_p, PH_DATE_S, &date_p)) */

						}		/* if (measurement_s) */

				}		/* if (unit_s) */

		}		/* if (trait_s) */

	return NULL;
}




bool SavePhenotype (Phenotype *phenotype_p, const DFWFieldTrialServiceData *data_p, bool corrected_value_flag)
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
					const char *collection_s = corrected_value_flag ? data_p -> dftsd_collection_ss [DFTD_CORRECTED_PHENOTYPE] : data_p -> dftsd_collection_ss [DFTD_RAW_PHENOTYPE];
					success_flag = SaveMongoData (data_p -> dftsd_mongo_p, phenotype_json_p, collection_s, insert_flag);

					json_decref (phenotype_json_p);
				}		/* if (phenotype_json_p) */

		}		/* if (phenotype_p -> ph_id_p) */

	return success_flag;
}


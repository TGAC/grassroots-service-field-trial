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


/*
DFW_FIELD_TRIAL_SERVICE_LOCAL Phenotype *AllocatePhenotype (bson_oid_t *id_p, const struct tm *date_p, const char *trait_s, const char *trait_abbreviation_s, const char *measurement_s,
																														const char *unit_s, const char *growth_stage_s, Instrument *instrument_p);
*/

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
															if (AddCompoundIdToJSON (phenotype_json_p, phenotype_p -> ph_id_p))
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

																}		/* if (AddCompoundIdToJSON (phenotype_json_p, phenotype_p -> ph_id_p)) */

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


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


/*
DFW_FIELD_TRIAL_SERVICE_LOCAL json_t *GetPhenotypeAsJSON (const Phenotype *phenotype_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL Phenotype *GetPhenotypeFromJSON (const json_t *phenotype_json_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL LinkedList *GetPhenotypeRows (Phenotype *phenotype_p);
*/

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
			json_t *phenotype_json_p = GetPhenotypeAsJSON (phenotype_p);

			if (phenotype_json_p)
				{
					const char *collection_s = corrected_value_flag ? data_p -> dftsd_collection_ss [DFTD_CORRECTED_PHENOTYPE] : data_p -> dftsd_collection_ss [DFTD_RAW_PHENOTYPE];
					success_flag = SaveMongoData (data_p -> dftsd_mongo_p, phenotype_json_p, collection_s, insert_flag);

					json_decref (phenotype_json_p);
				}		/* if (phenotype_json_p) */

		}		/* if (phenotype_p -> ph_id_p) */

	return success_flag;
}


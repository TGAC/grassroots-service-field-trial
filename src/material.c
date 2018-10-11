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
 * material.c
 *
 *  Created on: 10 Oct 2018
 *      Author: billy
 */

#define ALLOCATE_MATERIAL_TAGS (1)
#include "material.h"
#include "memory_allocations.h"
#include "string_utils.h"


/*
 * API FUNCTIONS
 */


Material *AllocateMaterial (bson_oid_t *id_p, const char *source_s, const char *accession_s, const char *pedigree_s, const char *barcode_s, const bool in_gru_flag, const DFWFieldTrialServiceData *data_p)
{
	char *copied_source_s = EasyCopyToNewString (source_s);

	if (copied_source_s)
		{
			char *copied_accession_s = EasyCopyToNewString (accession_s);

			if (copied_accession_s)
				{
					char *copied_pedigree_s = EasyCopyToNewString (pedigree_s);

					if (copied_pedigree_s)
						{
							char *copied_barcode_s = EasyCopyToNewString (barcode_s);

							if (copied_barcode_s)
								{
									Material *material_p = (Material *) AllocMemory (sizeof (Material));

									if (material_p)
										{
											material_p -> ma_id_p = id_p;
											material_p -> ma_source_s = copied_source_s;
											material_p -> ma_accession_s = copied_accession_s;
											material_p -> ma_pedigree_s = copied_pedigree_s;
											material_p -> ma_barcode_s = copied_barcode_s;
											material_p -> ma_in_gru_flag = in_gru_flag;
											material_p -> ma_germplasm_id_p = NULL;

											return material_p;
										}		/* if (material_p) */

									FreeCopiedString (copied_barcode_s);
								}		/* if (copied_barcode_s) */
							else
								{

								}

							FreeCopiedString (copied_pedigree_s);
						}		/* if (copied_pedigree_s) */
					else
						{

						}


					FreeCopiedString (copied_accession_s);
				}		/* if (copied_accession_s) */
			else
				{

				}

			FreeCopiedString (copied_source_s);
		}		/* if (copied_source_s) */
	else
		{

		}

	return NULL;
}



void FreeMaterial (Material *material_p)
{
	if (material_p -> ma_id_p)
		{
			FreeBSONOid (material_p -> ma_id_p);
		}

	if (material_p -> ma_accession_s)
		{
			FreeCopiedString (material_p -> ma_accession_s);
		}

	if (material_p -> ma_barcode_s)
		{
			FreeCopiedString (material_p -> ma_barcode_s);
		}

	if (material_p -> ma_source_s)
		{
			FreeCopiedString (material_p -> ma_source_s);
		}

	if (material_p -> ma_pedigree_s)
		{
			FreeCopiedString (material_p -> ma_pedigree_s);
		}

	FreeMemory (material_p);
}


json_t *GetMaterialAsJSON (const Material *material_p)
{
	json_t *material_json_p = json_object ();

	if (material_json_p)
		{
			if (AddCompoundIdToJSON (material_json_p, material_p -> ma_id_p))
				{
					if (SetJSONString (material_json_p, MA_SOURCE_S, material_p -> ma_source_s))
						{
							if (SetJSONString (material_json_p, MA_ACCESSION_S, material_p -> ma_accession_s))
								{
									if (SetJSONString (material_json_p, MA_BARCODE_S, material_p -> ma_barcode_s))
										{
											if (SetJSONString (material_json_p, MA_PEDIGREE_S, material_p -> ma_pedigree_s))
												{
													if (SetJSONBoolean (material_json_p, MA_IN_GRU_S, material_p -> ma_in_gru_flag))
														{
															return material_json_p;
														}

												}

										}

								}

						}

				}		/* if (AddCompoundIdToJSON (material_json_p, material_p -> ma_id_p)) */

			json_decref (material_json_p);
		}		/* if (material_json_p) */

	return NULL;
}


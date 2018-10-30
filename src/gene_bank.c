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
 * germplasm.c
 *
 *  Created on: 23 Jul 2018
 *      Author: billy
 */

#define ALLOCATE_GENE_BANK_TAGS (1)
#include "gene_bank.h"
#include "dfw_util.h"

#include "memory_allocations.h"
#include "string_utils.h"


/*
 * static declarations
 */

static void *GetGeneBankCallback (const json_t *json_p, const DFWFieldTrialServiceData *data_p);



/*
 * API definitions
 */

GeneBank *AllocateGeneBank (bson_oid_t *id_p, const char *name_s, const char *url_s, const char *api_url_s)
{
	char *copied_name_s = EasyCopyToNewString (name_s);

	if (copied_name_s)
		{
			char *copied_url_s = EasyCopyToNewString (url_s);

			if (copied_url_s)
				{
					char *copied_api_url_s = EasyCopyToNewString (api_url_s);

					if (copied_api_url_s)
						{
							GeneBank *germplasm_p = (GeneBank *) AllocMemory (sizeof (GeneBank));

							if (germplasm_p)
								{
									germplasm_p -> gb_id_p = id_p;
									germplasm_p -> gb_name_s = copied_name_s;
									germplasm_p -> gb_url_s = copied_url_s;
									germplasm_p -> gb_api_url_s = copied_api_url_s;

									return germplasm_p;
								}		/* if (germplasm_p) */

							FreeCopiedString (copied_api_url_s);
						}		/* if (copied_api_url_s) */

					FreeCopiedString (copied_url_s);
				}		/* if (copied_url_s) */

			FreeCopiedString (copied_name_s);
		}		/* if (copied_name_s) */

	return NULL;
}


void FreeGeneBank (GeneBank *germplasm_p)
{
	if (germplasm_p -> gb_id_p)
		{
			FreeBSONOid (germplasm_p -> gb_id_p);
		}

	if (germplasm_p -> gb_name_s)
		{
			FreeCopiedString (germplasm_p -> gb_name_s);
		}

	if (germplasm_p -> gb_url_s)
		{
			FreeCopiedString (germplasm_p -> gb_url_s);
		}

	if (germplasm_p -> gb_api_url_s)
		{
			FreeCopiedString (germplasm_p -> gb_api_url_s);
		}


	FreeMemory (germplasm_p);
}


json_t *GetGeneBankAsJSON (const GeneBank *gene_bank_p, const char * const api_query_s)
{
	json_t *res_p = json_object ();

	if (res_p)
		{
			if (SetJSONString (res_p, GB_NAME_S, gene_bank_p -> gb_name_s))
				{
					if (SetJSONString (res_p, GB_URL_S, gene_bank_p -> gb_url_s))
						{
							bool success_flag = false;

							if (api_query_s)
								{
									char *value_s = ConcatenateStrings (gene_bank_p -> gb_api_url_s, api_query_s);

									if (value_s)
										{
											success_flag = SetJSONString (res_p, GB_API_URL_S, value_s);
											FreeCopiedString (value_s);
										}
									else
										{
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to join \"%s\" and \"%s\"", gene_bank_p -> gb_api_url_s, api_query_s);
										}

								}
							else
								{
									success_flag = SetJSONString (res_p, GB_API_URL_S, gene_bank_p -> gb_api_url_s);
								}

							if (success_flag)
								{
									if (AddCompoundIdToJSON (res_p, gene_bank_p -> gb_id_p))
										{
											return res_p;
										}		/* if (AddCompoundIdToJSON (res_p, gene_bank_p -> gb_id_p)) */
									else
										{
											char id_s [MONGO_OID_STRING_BUFFER_SIZE];

											bson_oid_to_string (gene_bank_p -> gb_id_p, id_s);
											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, res_p, "Failed to add compound id for \"%s\": \"%s\"", MONGO_ID_S, id_s);
										}

								}		/* if (SetJSONString (res_p, GB_API_URL_S, gene_bank_p -> gb_api_url_s)) */
							else
								{
									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, res_p, "Failed to add \"%s\": \"%s\"", GB_API_URL_S, gene_bank_p -> gb_api_url_s);
								}

						}		/* if (SetJSONString (res_p, GB_URL_S, gene_bank_p -> gb_url_s)) */
					else
						{
							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, res_p, "Failed to add \"%s\": \"%s\"", GB_URL_S, gene_bank_p -> gb_url_s);
						}

				}		/* if (SetJSONString (res_p, GB_NAME_S, gene_bank_p -> gb_name_s)) */
			else
				{
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, res_p, "Failed to add \"%s\": \"%s\"", GB_NAME_S, gene_bank_p -> gb_name_s);
				}

			json_decref (res_p);
		}

	return NULL;
}


GeneBank *GetGeneBankFromJSON (const json_t *gene_bank_json_p)
{
	const char *name_s = GetJSONString (gene_bank_json_p, GB_NAME_S);

	if (name_s)
		{
			const char *url_s = GetJSONString (gene_bank_json_p, GB_URL_S);

			if (url_s)
				{
					const char *api_url_s = GetJSONString (gene_bank_json_p, GB_API_URL_S);

					if (api_url_s)
						{
							bson_oid_t *id_p = GetNewUnitialisedBSONOid ();

							if (id_p)
								{
									if (GetMongoIdFromJSON (gene_bank_json_p, id_p))
										{
											GeneBank *gene_bank_p = AllocateGeneBank (id_p, name_s, url_s, api_url_s);

											if (gene_bank_p)
												{
													return gene_bank_p;
												}		/* if (gene_bank_p) */
											else
												{
													PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, gene_bank_json_p, "Failed to allocate GeneBank");
												}

										}		/* if (GetMongoIdFromJSON (gene_bank_json_p, id_p)) */
									else
										{
											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, gene_bank_json_p, "Failed to get \"%s\"", MONGO_ID_S);
										}

									FreeBSONOid (id_p);
								}

						}		/* if (api_url_s) */
					else
						{
							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, gene_bank_json_p, "Failed to get \"%s\"", GB_API_URL_S);
						}

				}		/* if (url_s) */
			else
				{
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, gene_bank_json_p, "Failed to get \"%s\"", GB_URL_S);
				}

		}		/* if (name_s) */
	else
		{
			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, gene_bank_json_p, "Failed to get \"%s\"", GB_NAME_S);
		}

	return NULL;
}


bool SaveGeneBank (GeneBank *gene_bank_p, DFWFieldTrialServiceData *data_p)
{
	bson_t *selector_p = NULL;
	bool success_flag = PrepareSaveData (& (gene_bank_p -> gb_id_p), &selector_p);

	if (gene_bank_p -> gb_id_p)
		{
			json_t *gene_bank_json_p = GetGeneBankAsJSON (gene_bank_p, NULL);

			if (gene_bank_json_p)
				{
					success_flag = SaveMongoData (data_p -> dftsd_mongo_p, gene_bank_json_p, data_p -> dftsd_collection_ss [DFTD_GENE_BANK], selector_p);

					json_decref (gene_bank_json_p);
				}		/* if (area_json_p) */

		}		/* if (gene_bank_p -> gb_id_p) */

	return success_flag;
}



GeneBank *GetGeneBankById (const bson_oid_t *id_p, const DFWFieldTrialServiceData *data_p)
{
	GeneBank *gene_bank_p = GetDFWObjectById (id_p, DFTD_GENE_BANK, GetGeneBankCallback, data_p);

	return gene_bank_p;

}


GeneBank *GetGeneBankByIdString (const char *gene_bank_id_s, const DFWFieldTrialServiceData *data_p)
{
	GeneBank *gene_bank_p = GetDFWObjectByIdString (gene_bank_id_s, DFTD_GENE_BANK, GetGeneBankCallback, data_p);

	return gene_bank_p;
}


static void *GetGeneBankCallback (const json_t *json_p, const DFWFieldTrialServiceData *data_p)
{
	return GetGeneBankFromJSON (json_p);
}

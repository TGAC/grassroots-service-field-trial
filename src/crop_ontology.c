


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
 * crop_ontology.c
 *
 *  Created on: 15 Feb 2025
 *      Author: billy
 */

#define ALLOCATE_CROP_ONTOLOGY_TAGS (1)
#include "crop_ontology.h"
#include "dfw_util.h"

#include "memory_allocations.h"
#include "string_utils.h"
#include "mongodb_util.h"


static void *GetCropOntologyCallback (const json_t *json_p, const ViewFormat format, const FieldTrialServiceData *data_p);


CropOntology *AllocateCropOntology (bson_oid_t *id_p, const char *name_s, const char *url_s, const char *crop_s, const char *image_s)
{
	char *copied_name_s = EasyCopyToNewString (name_s);

	if (copied_name_s)
		{
			char *copied_crop_s = EasyCopyToNewString (crop_s);

			if (copied_crop_s)
				{
					char *copied_url_s = EasyCopyToNewString (url_s);

					if (copied_url_s)
						{
							char *copied_image_s = NULL;

							if ((image_s == NULL) || ((copied_image_s = EasyCopyToNewString (url_s)) != NULL))
								{
									CropOntology *co_p = (CropOntology *) AllocMemory (sizeof (CropOntology));

									if (co_p)
										{
											co_p -> co_id_p = id_p;
											co_p -> co_name_s = copied_name_s;
											co_p -> co_url_s = copied_url_s;
											co_p -> co_crop_s = copied_crop_s;
											co_p -> co_image_s = copied_image_s;

											return co_p;
										}		/* if (co_p) */

									if (copied_image_s)
										{
											FreeCopiedString (copied_image_s);
										}
								}

							FreeCopiedString (copied_url_s);
						}		/* if (copied_agrovoc_uri_s) */

					FreeCopiedString (copied_crop_s);
				}		/* if (copied_argovoc_preferred_term_s) */

			FreeCopiedString (copied_name_s);
		}		/* if (copied_name_s) */

	return NULL;

}



void FreeCropOntology (CropOntology *co_p)
{
	if (co_p -> co_id_p)
		{
			FreeBSONOid (co_p -> co_id_p);
		}

	if (co_p -> co_name_s)
		{
			FreeCopiedString (co_p -> co_name_s);
		}

	if (co_p -> co_url_s)
		{
			FreeCopiedString (co_p -> co_url_s);
		}

	if (co_p -> co_image_s)
		{
			FreeCopiedString (co_p -> co_image_s);
		}



	FreeCopiedString (co_p -> co_name_s);

	FreeMemory (co_p);
}


json_t *GetCropOntologyAsJSON (CropOntology *co_p, const ViewFormat format, const FieldTrialServiceData *data_p)
{
	json_t *res_p = json_object ();

	if (res_p)
		{
			if (SetJSONString (res_p, CO_NAME_S, co_p -> co_name_s))
				{
					if (SetJSONString (res_p, CO_URL_S, co_p -> co_url_s))
						{
							if (SetJSONString (res_p, CO_CROP_S, co_p -> co_crop_s))
								{
									if ((co_p -> co_image_s == NULL) || (SetJSONString (res_p, CO_IMAGE_S, co_p -> co_image_s)))
										{
											if (AddCompoundIdToJSON (res_p, co_p -> co_id_p))
												{
													if (AddDatatype (res_p, DFTD_ONTOLOGY))
														{
															return res_p;
														}

												}		/* if (AddCompoundIdToJSON (res_p, gene_bank_p -> gb_id_p)) */
											else
												{
													char id_s [MONGO_OID_STRING_BUFFER_SIZE];

													bson_oid_to_string (co_p -> co_id_p, id_s);
													PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, res_p, "Failed to add compound id for \"%s\": \"%s\"", MONGO_ID_S, id_s);
												}

										}		/* if ((co_p -> co_image_s == NULL) || (SetJSONString (res_p, CO_IMAGE_S, co_p -> co_image_s))) */
									else
										{
											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, res_p, "Failed to add \"%s\": \"%s\"",  CO_IMAGE_S, co_p -> co_image_s);
										}


								}		/* if (SetJSONString (res_p, CO_AGROVOC_PREFERRED_TERM_S, co_p -> CO_argovoc_preferred_term_s)) */
							else
								{
									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, res_p, "Failed to add \"%s\": \"%s\"",  CO_CROP_S, co_p -> co_crop_s);
								}

						}		/* if (SetJSONString (res_p, CO_AGROVOC_URL_S, co_p -> CO_agrovoc_uri_s)) */
					else
						{
							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, res_p, "Failed to add \"%s\": \"%s\"", CO_URL_S, co_p -> co_url_s);
						}

				}		/* if (SetJSONString (res_p, CO_NAME_S, co_p -> CO_name_s)) */
			else
				{
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, res_p, "Failed to add \"%s\": \"%s\"", CO_NAME_S,  co_p -> co_name_s);
				}

			json_decref (res_p);
		}

	return NULL;
}



CropOntology *GetCropOntologyFromJSON (const json_t *crop_json_p, const FieldTrialServiceData *data_p)
{
	const char *name_s = GetJSONString (crop_json_p, CO_NAME_S);

	if (name_s)
		{
			const char *url_s = GetJSONString (crop_json_p, CO_URL_S);

			if (url_s)
				{
					const char *term_s = GetJSONString (crop_json_p, CO_CROP_S);

					if (term_s)
						{
							bson_oid_t *id_p = GetNewUnitialisedBSONOid ();

							if (id_p)
								{
									if (GetMongoIdFromJSON (crop_json_p, id_p))
										{
											const char *logo_s = GetJSONString (crop_json_p, CO_IMAGE_S);
											CropOntology *co_p = AllocateCropOntology (id_p, name_s, term_s, url_s, logo_s);

											if (co_p)
												{
													return co_p;
												}		/* if (co_p) */
											else
												{
													PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, crop_json_p, "Failed to allocate CropOntology");
												}

										}		/* if (GetMongoIdFromJSON (gene_bank_json_p, id_p)) */
									else
										{
											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, crop_json_p, "Failed to get \"%s\"", MONGO_ID_S);
										}

									FreeBSONOid (id_p);
								}		/* if (id_p) */
							else
								{
									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, crop_json_p, "Failed to allocate uninitialised id");
								}


						}		/* if (term_s) */
					else
						{
							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, crop_json_p, "Failed to get \"%s\"", CO_CROP_S);
						}

				}		/* if (url_s) */
			else
				{
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, crop_json_p, "Failed to get \"%s\"", CO_URL_S);
				}

		}		/* if (name_s) */
	else
		{
			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, crop_json_p, "Failed to get \"%s\"", CO_NAME_S);
		}

	return NULL;

}


bool SaveCropOntology (CropOntology *co_p, const FieldTrialServiceData *data_p)
{
	bson_t *selector_p = NULL;
	bool success_flag = PrepareSaveData (& (co_p -> co_id_p), &selector_p);

	if (co_p -> co_id_p)
		{
			json_t *crop_json_p = GetCropOntologyAsJSON (co_p, VF_STORAGE, data_p);

			if (crop_json_p)
				{
					success_flag = SaveAndBackupMongoDataWithTimestamp (data_p -> dftsd_mongo_p, crop_json_p, data_p -> dftsd_collection_ss [DFTD_CROP], data_p -> dftsd_backup_collection_ss [DFTD_CROP], DFT_BACKUPS_ID_KEY_S, selector_p, MONGO_TIMESTAMP_S);

					json_decref (crop_json_p);
				}		/* if (crop_json_p) */

		}		/* if (co_p -> CO_id_p) */

	return success_flag;
}


CropOntology *GetCropOntologyById (const bson_oid_t *id_p, const FieldTrialServiceData *data_p)
{
	CropOntology *co_p = NULL;
	char *id_s = GetBSONOidAsString (id_p);

	if (id_s)
		{
			co_p = GetCropOntologyByIdString (id_s, data_p);
			FreeBSONOidString (id_s);
		}
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "GetBSONOidAsString failed for crop");
		}

	return co_p;
}



CropOntology *GetCropOntologyByIdString (const char *id_s, const FieldTrialServiceData *data_p)
{
	CropOntology *co_p = (CropOntology *) GetDFWObjectByIdString (id_s, DFTD_ONTOLOGY, GetCropOntologyCallback, VF_STORAGE, data_p);

	return co_p;
}


static void *GetCropOntologyCallback (const json_t *json_p, const ViewFormat format, const FieldTrialServiceData *data_p)
{
	return GetCropOntologyFromJSON (json_p, data_p);
}

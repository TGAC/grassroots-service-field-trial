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
#include "gene_bank.h"


static bool ReplaceMaterialField (const char *new_value_s, char **value_ss);

static Material *SearchForMaterial (bson_t *query_p, const DFWFieldTrialServiceData *data_p);

/*
 * API FUNCTIONS
 */

Material *AllocateMaterial (bson_oid_t *id_p, const char *accession_s, const char *pedigree_s, const char *barcode_s, const char *internal_name_s, const ExperimentalArea *area_p, const bson_oid_t *gene_bank_id_p, const DFWFieldTrialServiceData *data_p)
{
	char *copied_accession_s = EasyCopyToNewString (accession_s);

	if (copied_accession_s)
		{
			bool success_flag = true;
			char *copied_pedigree_s = NULL;

			if (!IsStringEmpty (pedigree_s))
				{
					copied_pedigree_s = EasyCopyToNewString (pedigree_s);
					success_flag = (copied_pedigree_s != NULL);
				}

			if (success_flag)
				{
					char *copied_barcode_s = NULL;


					if (!IsStringEmpty (barcode_s))
						{
							copied_barcode_s = EasyCopyToNewString (barcode_s);
							success_flag = (copied_barcode_s != NULL);
						}

					if (success_flag)
						{
							char *copied_internal_name_s = EasyCopyToNewString (internal_name_s);

							if (copied_internal_name_s)
								{
									bson_oid_t *copied_gene_bank_id_p = CopyBSONOid (gene_bank_id_p);

									if (copied_gene_bank_id_p)
										{
											Material *material_p = (Material *) AllocMemory (sizeof (Material));

											if (material_p)
												{
													material_p -> ma_id_p = id_p;
													material_p -> ma_accession_s = copied_accession_s;
													material_p -> ma_pedigree_s = copied_pedigree_s;
													material_p -> ma_barcode_s = copied_barcode_s;
													material_p -> ma_gene_bank_id_p = copied_gene_bank_id_p;
													material_p -> ma_parent_area_p = area_p;
													material_p -> ma_internal_name_s = copied_internal_name_s;

													return material_p;
												}		/* if (material_p) */

											FreeBSONOid (copied_gene_bank_id_p);
										}

									FreeCopiedString (copied_internal_name_s);
								}		/* if (copied_internal_name_s) */

							if (copied_barcode_s)
								{
									FreeCopiedString (copied_barcode_s);
								}

						}		/* if (copied_barcode_s) */
					else
						{

						}

					if (copied_pedigree_s)
						{
							FreeCopiedString (copied_pedigree_s);
						}
				}		/* if (copied_pedigree_s) */
			else
				{

				}


			FreeCopiedString (copied_accession_s);
		}		/* if (copied_accession_s) */
	else
		{

		}


	return NULL;
}


Material *AllocateMaterialByInternalName (bson_oid_t *id_p, const char *internal_name_s, const ExperimentalArea *area_p, const DFWFieldTrialServiceData *data_p)
{
	char *copied_internal_name_s = EasyCopyToNewString (internal_name_s);

	if (copied_internal_name_s)
		{
			Material *material_p = (Material *) AllocMemory (sizeof (Material));

			if (material_p)
				{
					material_p -> ma_id_p = id_p;
					material_p -> ma_accession_s = NULL;
					material_p -> ma_pedigree_s = NULL;
					material_p -> ma_barcode_s = NULL;
					material_p -> ma_gene_bank_id_p = NULL;
					material_p -> ma_parent_area_p = area_p;
					material_p -> ma_internal_name_s = copied_internal_name_s;

					return material_p;
				}		/* if (material_p) */

			FreeCopiedString (copied_internal_name_s);
		}

	return NULL;
}



void FreeMaterial (Material *material_p)
{
	if (material_p -> ma_id_p)
		{
			FreeBSONOid (material_p -> ma_id_p);
		}

	if (material_p -> ma_gene_bank_id_p)
		{
			FreeBSONOid (material_p -> ma_gene_bank_id_p);
		}

	if (material_p -> ma_accession_s)
		{
			FreeCopiedString (material_p -> ma_accession_s);
		}

	if (material_p -> ma_barcode_s)
		{
			FreeCopiedString (material_p -> ma_barcode_s);
		}

	if (material_p -> ma_pedigree_s)
		{
			FreeCopiedString (material_p -> ma_pedigree_s);
		}

	FreeMemory (material_p);
}


json_t *GetMaterialAsJSON (const Material *material_p, const bool expand_gene_bank_flag, const DFWFieldTrialServiceData *data_p)
{
	json_t *material_json_p = json_object ();

	if (material_json_p)
		{
			if (AddCompoundIdToJSON (material_json_p, material_p -> ma_id_p))
				{
					bool success_flag = false;

					if (material_p -> ma_gene_bank_id_p)
						{
							if (expand_gene_bank_flag)
								{
									GeneBank *gene_bank_p = GetGeneBankById (material_p -> ma_gene_bank_id_p, data_p);

									if (gene_bank_p)
										{
											json_t *gene_bank_json_p = GetGeneBankAsJSON (gene_bank_p, material_p -> ma_accession_s);

											if (gene_bank_json_p)
												{
													if (json_object_set_new (material_json_p, MA_GENE_BANK_S, gene_bank_json_p) == 0)
														{
															success_flag = true;
														}
													else
														{
															PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, material_json_p, "Failed to add gene bank json to material");
															json_decref (gene_bank_json_p);
														}
												}
											else
												{
													PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get gene bank \"%s\"", gene_bank_p -> gb_name_s);
												}

											FreeGeneBank (gene_bank_p);
										}
									else
										{
											char id_s [MONGO_OID_STRING_BUFFER_SIZE];

											bson_oid_to_string (material_p -> ma_gene_bank_id_p, id_s);
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to find gene bank with id \"%s\"", id_s);
										}
								}
							else
								{
									success_flag = AddNamedCompoundIdToJSON (material_json_p, material_p -> ma_gene_bank_id_p, MA_GENE_BANK_ID_S);
								}
						}

					if (success_flag)
						{
							if (AddNamedCompoundIdToJSON (material_json_p, material_p -> ma_parent_area_p -> ea_id_p, MA_EXPERIMENTAL_AREA_ID_S))
								{
									if (SetJSONString (material_json_p, MA_INTERNAL_NAME_S, material_p -> ma_internal_name_s))
										{
											if (SetJSONString (material_json_p, MA_ACCESSION_S, material_p -> ma_accession_s))
												{
													if ((IsStringEmpty (material_p -> ma_barcode_s)) || (SetJSONString (material_json_p, MA_BARCODE_S, material_p -> ma_barcode_s)))
														{
															if ((IsStringEmpty (material_p -> ma_pedigree_s)) || (SetJSONString (material_json_p, MA_PEDIGREE_S, material_p -> ma_pedigree_s)))
																{
																	return material_json_p;
																}		/* if ((IsStringEmpty (material_p -> ma_pedigree_s)) || (SetJSONString (material_json_p, MA_PEDIGREE_S, material_p -> ma_pedigree_s))) */
															else
																{
																	PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, material_json_p, "Failed to add \"%s\": \"%s\"", MA_PEDIGREE_S, material_p -> ma_pedigree_s);
																}

														}		/* if ((IsStringEmpty (material_p -> ma_barcode_s)) || (SetJSONString (material_json_p, MA_BARCODE_S, material_p -> ma_barcode_s))) */
													else
														{
															PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, material_json_p, "Failed to add \"%s\": \"%s\"", MA_BARCODE_S, material_p -> ma_barcode_s);
														}

												}		/* if (SetJSONString (material_json_p, MA_ACCESSION_S, material_p -> ma_accession_s) */
											else
												{
													PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, material_json_p, "Failed to add \"%s\": \"%s\"", MA_ACCESSION_S, material_p -> ma_accession_s);
												}

										}		/* if (SetJSONString (material_json_p, MA_INTERNAL_NAME_S, material_p -> ma_internal_name_s) */
									else
										{
											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, material_json_p, "Failed to add \"%s\": \"%s\"", MA_INTERNAL_NAME_S, material_p -> ma_internal_name_s);
										}

								}		/* if (AddNamedCompoundIdToJSON (material_json_p, material_p -> ma_parent_area_p -> ea_id_p, MA_EXPERIMENTAL_AREA_ID_S) */
							else
								{
									char id_s [MONGO_OID_STRING_BUFFER_SIZE];

									bson_oid_to_string (material_p -> ma_parent_area_p -> ea_id_p, id_s);
									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, material_json_p, "Failed to add compound id for \"%s\": \"%s\"", MA_EXPERIMENTAL_AREA_ID_S, id_s);
								}
						}		/* if (success_flag)) */
					else
						{
							char id_s [MONGO_OID_STRING_BUFFER_SIZE];

							bson_oid_to_string (material_p -> ma_gene_bank_id_p, id_s);
							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, material_json_p, "Failed to add gene with id for \"%s\": \"%s\", expanded = %c", MA_GENE_BANK_ID_S, id_s, expand_gene_bank_flag);
						}
				}		/* if (AddCompoundIdToJSON (material_json_p, material_p -> ma_id_p)) */
			else
				{
					char id_s [MONGO_OID_STRING_BUFFER_SIZE];

					bson_oid_to_string (material_p -> ma_id_p, id_s);
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, material_json_p, "Failed to add compound id for \"%s\": \"%s\"", MONGO_ID_S, id_s);
				}

			json_decref (material_json_p);
		}		/* if (material_json_p) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate JSON object for Material \"%s\"", material_p -> ma_accession_s);
		}

	return NULL;
}



Material *GetMaterialFromJSON (const json_t *json_p, const bool expand_experimental_area_flag, const DFWFieldTrialServiceData *data_p)
{
	const char *accession_s = GetJSONString (json_p, MA_ACCESSION_S);

	if (accession_s)
		{
			const char *internal_name_s = GetJSONString (json_p, MA_INTERNAL_NAME_S);

			if (internal_name_s)
				{
					bson_oid_t *id_p = GetNewUnitialisedBSONOid ();

					if (id_p)
						{
							if (GetMongoIdFromJSON (json_p, id_p))
								{
									bson_oid_t *germplasm_id_p = GetNewUnitialisedBSONOid ();

									if (germplasm_id_p)
										{
											if (GetNamedIdFromJSON (json_p, MA_GENE_BANK_ID_S, germplasm_id_p))
												{
													ExperimentalArea *area_p = NULL;
													bool success_flag = !expand_experimental_area_flag;

													if (expand_experimental_area_flag)
														{
															bson_oid_t *exp_area_id_p = GetNewUnitialisedBSONOid ();

															if (exp_area_id_p)
																{
																	if (GetNamedIdFromJSON (json_p, MA_EXPERIMENTAL_AREA_ID_S, exp_area_id_p))
																		{
																			area_p = GetExperimentalAreaById (exp_area_id_p, data_p);

																			if (!area_p)
																				{
																					char id_s [MONGO_OID_STRING_BUFFER_SIZE];

																					bson_oid_to_string (exp_area_id_p, id_s);
																					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, json_p, "Failed to find ExperimentalArea with id \"%s\"", id_s);
																				}
																		}
																	else
																		{
																			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, json_p, "Failed to get \"%s\"", MA_EXPERIMENTAL_AREA_ID_S);
																		}

																	FreeBSONOid (exp_area_id_p);
																}		/* if (exp_area_id_p) */
															else
																{
																	PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate id for \"%s\"", MA_EXPERIMENTAL_AREA_ID_S);
																}
														}

													if (success_flag)
														{
															const char *barcode_s = GetJSONString (json_p, MA_BARCODE_S);
															const char *pedigree_s = GetJSONString (json_p, MA_PEDIGREE_S);
															Material *material_p = AllocateMaterial (id_p, accession_s, pedigree_s, barcode_s, internal_name_s, area_p, germplasm_id_p, data_p);

															if (material_p)
																{
																	return material_p;
																}
															else
																{
																	PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, json_p, "Failed to allocate Material");
																}
														}

													if (area_p)
														{
															FreeExperimentalArea (area_p);
														}

												}		/* if (GetNamedIdFromJSON (json_p, MA_GENE_BANK_ID_S, germplasm_id_p)) */
											else
												{
													PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, json_p, "Failed to get \"%s\"", MA_GENE_BANK_ID_S);
												}

											FreeBSONOid (germplasm_id_p);
										}		/* if (germplasm_id_p) */
									else
										{
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate id for \"%s\"", MA_GENE_BANK_ID_S);
										}

								}		/* if (GetMongoIdFromJSON (json_p, id_p)) */
							else
								{
									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, json_p, "Failed to get \"%s\"", MONGO_ID_S);
								}

							FreeBSONOid (id_p);
						}		/* if (id_p) */
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed tp allocate id for \"%s\"", MONGO_ID_S);
						}

				}		/* if (internal_name_s) */
			else
				{
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, json_p, "Failed to get \"%s\"", MA_INTERNAL_NAME_S);
				}

		}		/* if (accession_s) */
	else
		{
			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, json_p, "Failed to get \"%s\"", MA_ACCESSION_S);
		}

	return NULL;
}


bool SaveMaterial (Material *material_p, const DFWFieldTrialServiceData *data_p)
{
	bool success_flag = false;
	bool insert_flag = false;

	if (! (material_p -> ma_id_p))
		{
			material_p -> ma_id_p = GetNewBSONOid ();

			if (material_p -> ma_id_p)
				{
					insert_flag = true;
				}
		}

	if (material_p -> ma_id_p)
		{
			json_t *material_json_p = GetMaterialAsJSON (material_p, false, data_p);

			if (material_json_p)
				{
					success_flag = SaveMongoData (data_p -> dftsd_mongo_p, material_json_p, data_p -> dftsd_collection_ss [DFTD_MATERIAL], insert_flag);

					json_decref (material_json_p);
				}		/* if (material_json_p) */

		}		/* if (material_p -> ma_id_p) */

	return success_flag;
}


Material *GetOrCreateMaterialByInternalName (const char *material_s, ExperimentalArea *area_p, const DFWFieldTrialServiceData *data_p)
{
	Material *material_p = GetMaterialByInternalName (material_s, area_p, data_p);

	if (!material_p)
		{
			material_p  = AllocateMaterialByInternalName (NULL, material_s, area_p, data_p);

			if (!material_p)
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate Material with internal name of \"%s\" for area \"%s\"", material_s, area_p -> ea_name_s);
				}

		}		/* if (!material_p) */

	return material_p;
}


Material *GetMaterialByInternalName (const char *material_s, ExperimentalArea *area_p, const DFWFieldTrialServiceData *data_p)
{
	Material *material_p = NULL;
	bson_t *query_p = BCON_NEW (MA_INTERNAL_NAME_S, BCON_UTF8 (material_s), MA_EXPERIMENTAL_AREA_ID_S, BCON_OID (area_p -> ea_id_p));

	if (query_p)
		{
			material_p = SearchForMaterial (query_p, data_p);

			if (material_p)
				{
					material_p -> ma_parent_area_p = area_p;
				}

			bson_destroy (query_p);
		}		/* if (query_p) */

	return material_p;
}


Material *GetMaterialById (const bson_oid_t *material_id_p, ExperimentalArea *area_p, const DFWFieldTrialServiceData *data_p)
{
	Material *material_p = NULL;
	bson_t *query_p = BCON_NEW (MONGO_ID_S, BCON_OID (material_id_p), MA_EXPERIMENTAL_AREA_ID_S, BCON_OID (area_p -> ea_id_p));

	if (query_p)
		{
			material_p = SearchForMaterial (query_p, data_p);

			if (material_p)
				{
					material_p -> ma_parent_area_p = area_p;
				}

			bson_destroy (query_p);
		}		/* if (query_p) */

	return material_p;
}


static Material *SearchForMaterial (bson_t *query_p, const DFWFieldTrialServiceData *data_p)
{
	Material *material_p = NULL;

	if (SetMongoToolCollection (data_p -> dftsd_mongo_p, data_p -> dftsd_collection_ss [DFTD_MATERIAL]))
		{
			json_t *results_p = GetAllMongoResultsAsJSON (data_p -> dftsd_mongo_p, query_p, NULL);

			if (results_p)
				{
					if (json_is_array (results_p))
						{
							if (json_array_size (results_p) == 1)
								{
									json_t *result_p = json_array_get (results_p, 0);

									material_p = GetMaterialFromJSON (result_p, false, data_p);

									if (!material_p)
										{
											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, result_p, "Failed to get Material from JSON");
										}

								}		/* if (json_array_size (results_p) == 1) */
							else
								{
									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, results_p, "Materials array does not contain just a single item");
								}

						}		/* if (json_is_array (results_p)) */
					else
						{
							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, results_p, "results are not an array");
						}

					json_decref (results_p);
				}		/* if (results_p) */
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "No results returned");
				}
		}		/* if (SetMongoToolCollection (data_p -> dftsd_mongo_p, data_p -> dftsd_collection_ss [DFTD_MATERIAL])) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to set mongo collection to \"%s\"", data_p -> dftsd_collection_ss [DFTD_MATERIAL]);
		}

	return material_p;
}


bool IsMaterialComplete (const Material * const material_p)
{
	return ((material_p -> ma_pedigree_s) && (material_p -> ma_accession_s));
}


bool SetMaterialAccession (Material *material_p, const char * const accession_s)
{
	return ReplaceMaterialField (accession_s, & (material_p -> ma_accession_s));
}


bool SetMaterialPedigree (Material *material_p, const char * const pedigree_s)
{
	return ReplaceMaterialField (pedigree_s, & (material_p -> ma_pedigree_s));
}


bool SetMaterialBarcode (Material *material_p, const char * const barcode_s)
{
	return ReplaceMaterialField (barcode_s, & (material_p -> ma_barcode_s));
}


bool SetMaterialInternalName (Material *material_p, const char * const internal_name_s)
{
	return ReplaceMaterialField (internal_name_s, & (material_p -> ma_internal_name_s));
}


static bool ReplaceMaterialField (const char *new_value_s, char **value_ss)
{
	bool success_flag = false;

	if (new_value_s)
		{
			char *copied_value_s = EasyCopyToNewString (new_value_s);

			if (copied_value_s)
				{
					if (*value_ss)
						{
							FreeCopiedString (*value_ss);
							*value_ss = copied_value_s;
							success_flag = true;
						}
				}
		}
	else
		{
			if (*value_ss)
				{
					FreeCopiedString (*value_ss);
					*value_ss = NULL;
					success_flag = true;
				}
		}

	return success_flag;
}

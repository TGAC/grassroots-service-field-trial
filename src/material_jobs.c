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
 * material_jobs.c
 *
 *  Created on: 23 Oct 2018
 *      Author: billy
 */


#include "material.h"
#include "material_jobs.h"
#include "string_utils.h"
#include "study_jobs.h"
#include "gene_bank.h"
#include "gene_bank_jobs.h"


/*
 * static declarations
 */

static const char S_DEFAULT_COLUMN_DELIMITER =  '|';

/*
static const char * const S_INTERNAL_NAME_TITLE_S = "Accession";
static const char * const S_PEDIGREE_TITLE_S = "Pedigree";
static const char * const S_BARCODE_TITLE_S = "Barcode";
static const char * const S_ACCESSION_TITLE_S = "Store code";
*/

static const char * const S_SPECIES_NAME_TITLE_S = "Species Name";
static const char * const S_GERMPLASM_ID_TITLE_S = "Germplasm ID";
static const char * const S_TYPE_TITLE_S = "Material Type";
static const char * const S_SELECTION_REASON_TITLE_S = "Selection Reason";
static const char * const S_GENERATION_TITLE_S = "Generation";
static const char * const S_SEED_SUPPLIER_TITLE_S = "Seed Supplier";
static const char * const S_SEED_SOURCE_TITLE_S = "Seed Source";
static const char * const S_GERMPLASM_ORIGIN_TITLE_S = "Germplasm Origin";
static const char * const S_IN_GRU_TITLE_S = "In GRU?";
static const char * const S_GRU_ACCESSION_TITLE_S = "GRU Accession";
static const char * const S_TGW_TITLE_S = "TGW";
static const char * const S_SEED_TREATMENT_TITLE_S = "Seed Treatment";
static const char * const S_CLEANED_TITLE_S = "Cleaned?";


static NamedParameterType S_MATERIAL_TABLE_COLUMN_DELIMITER = { "MA Data delimiter", PT_CHAR };
static NamedParameterType S_MATERIAL_TABLE = { "MA Upload", PT_TABLE};
static NamedParameterType S_STUDIES_LIST = { "MA Study", PT_STRING };
static NamedParameterType S_GENE_BANKS_LIST = { "MA Gene Bank", PT_STRING };


static json_t *GetTableParameterHints (void);

static Parameter *GetTableParameter (ParameterSet *param_set_p, ParameterGroup *group_p, const DFWFieldTrialServiceData *data_p);

static bool AddMaterialsFromJSON (ServiceJob *job_p, const json_t *materials_json_p, Study *area_p, GeneBank *gene_bank_p, const DFWFieldTrialServiceData *data_p);



/*
 * API definitions
 */

bool AddSubmissionMaterialParams (ServiceData *data_p, ParameterSet *param_set_p)
{
	bool success_flag = false;

	ParameterGroup *group_p = CreateAndAddParameterGroupToParameterSet ("Materials", false, data_p, param_set_p);

	if (group_p)
		{
			Parameter *param_p = NULL;
			SharedType def;

			InitSharedType (&def);

			if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_STUDIES_LIST.npt_type, S_STUDIES_LIST.npt_name_s, "Experimental Area", "The available experimental areas", def, PL_ALL)) != NULL)
				{
					const DFWFieldTrialServiceData *dfw_service_data_p = (DFWFieldTrialServiceData *) data_p;

					if (SetUpStudiesListParameter (dfw_service_data_p, param_p, NULL))
						{
							if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_GENE_BANKS_LIST.npt_type, S_GENE_BANKS_LIST.npt_name_s, "Gene Bank", "The available gene banks", def, PL_ALL)) != NULL)
								{
									if (SetUpGenBanksListParameter ((DFWFieldTrialServiceData *) data_p, param_p))
										{
											def.st_char_value = S_DEFAULT_COLUMN_DELIMITER;

											if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_MATERIAL_TABLE_COLUMN_DELIMITER.npt_type, S_MATERIAL_TABLE_COLUMN_DELIMITER.npt_name_s, "Delimiter", "The character delimiting columns", def, PL_ADVANCED)) != NULL)
												{
													def.st_string_value_s = NULL;

													if ((param_p = GetTableParameter (param_set_p, group_p, dfw_service_data_p)) != NULL)
														{
															success_flag = true;
														}
												}

										}
								}
						}
				}
		}		/* if (group_p) */


	return success_flag;
}


bool RunForSubmissionMaterialParams (DFWFieldTrialServiceData *data_p, ParameterSet *param_set_p, ServiceJob *job_p)
{
	bool job_done_flag = false;

	SharedType value;
	InitSharedType (&value);

	if (GetParameterValueFromParameterSet (param_set_p, S_MATERIAL_TABLE.npt_name_s, &value, true))
		{
			/*
			 * Has a spreadsheet been uploaded?
			 */
			if (! (IsStringEmpty (value.st_string_value_s)))
				{
					bool success_flag = false;
					json_error_t e;
					json_t *materials_json_p = NULL;

					job_done_flag = true;

					/*
					 * The data could be either an array of json objects
					 * or a tabular string. so try it as json array first
					 */
					materials_json_p = json_loads (value.st_string_value_s, 0, &e);

					if (materials_json_p)
						{
							InitSharedType (&value);

							if (GetParameterValueFromParameterSet (param_set_p, S_STUDIES_LIST.npt_name_s, &value, true))
								{
									Study *area_p = GetStudyByIdString (value.st_string_value_s, VF_STORAGE, data_p);

									if (area_p)
										{
											InitSharedType (&value);

											if (GetParameterValueFromParameterSet (param_set_p, S_GENE_BANKS_LIST.npt_name_s, &value, true))
												{
													GeneBank *gene_bank_p = GetGeneBankByIdString (value.st_string_value_s, VF_STORAGE, data_p);

													if (gene_bank_p)
														{
															if (AddMaterialsFromJSON (job_p, materials_json_p, area_p, gene_bank_p, data_p))
																{
																	success_flag = true;
																}
															else
																{
																	char area_id_s [MONGO_OID_STRING_BUFFER_SIZE];

																	bson_oid_to_string (area_p -> st_id_p, area_id_s);
																	PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, materials_json_p, "AddMaterialsFromJSON for GeneBank with id \"%s\" and Study with id \"%s\"", value.st_string_value_s,materials_json_p);
																}

															FreeGeneBank (gene_bank_p);
														}		/* if (gene_bank_p) */
													else
														{
															PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get GeneBank with id \"%s\"", value.st_string_value_s);
														}

												}		/* if (GetParameterValueFromParameterSet (param_set_p, S_GENE_BANKS_LIST.npt_name_s, &value, true)) */
											else
												{
													PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get \"%s\" parameter value", S_GENE_BANKS_LIST.npt_name_s);
												}

											FreeStudy (area_p);
										}		/* if (area_p) */
									else
										{
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get Study with id \"%s\"", value.st_string_value_s);
										}

								}		/* if (GetParameterValueFromParameterSet (param_set_p, S_STUDIES_LIST.npt_name_s, &value, true)) */
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get \"%s\" parameter value", S_STUDIES_LIST.npt_name_s);
								}

							json_decref (materials_json_p);
						}		/* if (materials_json_p) */
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to load \"%s\" as JSON", value.st_string_value_s);
						}

					job_done_flag = true;
				}		/* if (! (IsStringEmpty (value.st_string_value_s))) */

		}		/* if (GetParameterValueFromParameterSet (param_set_p, S_ADD_EXPERIMENTAL_AREA.npt_name_s, &value, true)) */


	return job_done_flag;
}


bool GetSubmissionMaterialParameterTypeForNamedParameter (const char *param_name_s, ParameterType *pt_p)
{
	bool success_flag = true;

	if (strcmp (param_name_s, S_STUDIES_LIST.npt_name_s) == 0)
		{
			*pt_p = S_STUDIES_LIST.npt_type;
		}
	else if (strcmp (param_name_s, S_GENE_BANKS_LIST.npt_name_s) == 0)
		{
			*pt_p = S_GENE_BANKS_LIST.npt_type;
		}
	else if (strcmp (param_name_s, S_MATERIAL_TABLE.npt_name_s) == 0)
		{
			*pt_p = S_MATERIAL_TABLE.npt_type;
		}
	else if (strcmp (param_name_s, S_MATERIAL_TABLE_COLUMN_DELIMITER.npt_name_s) == 0)
		{
			*pt_p = S_MATERIAL_TABLE_COLUMN_DELIMITER.npt_type;
		}
	else
		{
			success_flag = false;
		}

	return success_flag;
}

/*
 * static definitions
 */


static json_t *GetTableParameterHints (void)
{
	json_t *hints_p = json_array ();

	if (hints_p)
		{
			if (AddColumnParameterHint (S_SPECIES_NAME_TITLE_S, PT_STRING, hints_p))
				{
					if (AddColumnParameterHint (S_GERMPLASM_ID_TITLE_S, PT_STRING, hints_p))
						{
							if (AddColumnParameterHint (S_TYPE_TITLE_S, PT_STRING, hints_p))
								{
									if (AddColumnParameterHint (S_SELECTION_REASON_TITLE_S, PT_STRING, hints_p))
										{
											if (AddColumnParameterHint (S_GENERATION_TITLE_S, PT_STRING, hints_p))
												{
													if (AddColumnParameterHint (S_SEED_SUPPLIER_TITLE_S, PT_STRING, hints_p))
														{
															if (AddColumnParameterHint (S_SEED_SOURCE_TITLE_S, PT_STRING, hints_p))
																{
																	if (AddColumnParameterHint (S_GERMPLASM_ORIGIN_TITLE_S, PT_STRING, hints_p))
																		{
																			if (AddColumnParameterHint (S_IN_GRU_TITLE_S, PT_BOOLEAN, hints_p))
																				{
																					if (AddColumnParameterHint (S_GRU_ACCESSION_TITLE_S, PT_STRING, hints_p))
																						{
																							if (AddColumnParameterHint (S_TGW_TITLE_S, PT_UNSIGNED_INT, hints_p))
																								{
																									if (AddColumnParameterHint (S_SEED_TREATMENT_TITLE_S, PT_STRING, hints_p))
																										{
																											if (AddColumnParameterHint (S_CLEANED_TITLE_S, PT_BOOLEAN, hints_p))
																												{
																													return hints_p;
																												}
																										}
																								}
																						}
																				}
																		}
																}
														}
												}
										}
								}
						}
				}

			json_decref (hints_p);
		}

	return NULL;
}


static Parameter *GetTableParameter (ParameterSet *param_set_p, ParameterGroup *group_p, const DFWFieldTrialServiceData *data_p)
{
	Parameter *param_p = NULL;
	const char delim_s [2] = { S_DEFAULT_COLUMN_DELIMITER, '\0' };
	SharedType def;

	InitSharedType (&def);

	param_p = EasyCreateAndAddParameterToParameterSet (& (data_p -> dftsd_base_data), param_set_p, group_p, S_MATERIAL_TABLE.npt_type, S_MATERIAL_TABLE.npt_name_s, "Material data to upload", "The data to upload", def, PL_ALL);

	if (param_p)
		{
			bool success_flag = false;
			json_t *hints_p = GetTableParameterHints ();

			if (hints_p)
				{
					if (AddParameterKeyJSONValuePair (param_p, PA_TABLE_COLUMN_HEADINGS_S, hints_p))
						{
							if (AddParameterKeyStringValuePair (param_p, PA_TABLE_COLUMN_DELIMITER_S, delim_s))
								{
									success_flag = true;
								}
						}

					json_decref (hints_p);
				}

			if (!success_flag)
				{
					FreeParameter (param_p);
					param_p = NULL;
				}

		}		/* if (param_p) */

	return param_p;
}




static bool AddMaterialsFromJSON (ServiceJob *job_p, const json_t *materials_json_p, Study *area_p, GeneBank *gene_bank_p, const DFWFieldTrialServiceData *data_p)
{
	bool success_flag	= true;
	OperationStatus status = OS_FAILED;

	if (json_is_array (materials_json_p))
		{
			const size_t num_rows = json_array_size (materials_json_p);
			size_t i;
			size_t num_imported = 0;

			for (i = 0; i < num_rows; ++ i)
				{
					json_t *table_row_json_p = json_array_get (materials_json_p, i);


					const char *species_name_s = GetJSONString (table_row_json_p, S_SPECIES_NAME_TITLE_S);

					if (!IsStringEmpty (species_name_s))
						{
							const char *germplasm_id_s = GetJSONString (table_row_json_p, S_GERMPLASM_ID_TITLE_S);

							if (!IsStringEmpty (germplasm_id_s))
								{
									const char *type_s = GetJSONString (table_row_json_p, S_TYPE_TITLE_S);

									if (!IsStringEmpty (type_s))
										{
											const char *reason_s = GetJSONString (table_row_json_p, S_SELECTION_REASON_TITLE_S);

											if (!IsStringEmpty (reason_s))
												{
													const char *generation_s = GetJSONString (table_row_json_p, S_GENERATION_TITLE_S);

													if (!IsStringEmpty (generation_s))
														{
															const char *supplier_s = GetJSONString (table_row_json_p, S_SEED_SUPPLIER_TITLE_S);

															if (!IsStringEmpty (supplier_s))
																{
																	const char *source_s = GetJSONString (table_row_json_p, S_SEED_SOURCE_TITLE_S);

																	if (!IsStringEmpty (source_s))
																		{
																			const char *germplasm_origin_s = GetJSONString (table_row_json_p, S_GERMPLASM_ORIGIN_TITLE_S);

																			if (!IsStringEmpty (germplasm_origin_s))
																				{
																					bool in_gru_flag;

																					if (GetJSONBoolean (table_row_json_p, S_IN_GRU_TITLE_S, &in_gru_flag))
																						{
																							const char *accession_s = GetJSONString (table_row_json_p, S_GRU_ACCESSION_TITLE_S);

																							if (!IsStringEmpty (accession_s))
																								{
																									uint32 tgw;

																									if (GetJSONInteger (table_row_json_p, S_TGW_TITLE_S, (int *) &tgw))
																										{
																											const char *treatment_s = GetJSONString (table_row_json_p, S_SEED_TREATMENT_TITLE_S);

																											if (!IsStringEmpty (treatment_s))
																												{
																													bool cleaned_flag;

																													if (GetJSONBoolean (table_row_json_p, S_CLEANED_TITLE_S, &cleaned_flag))
																														{
																															Material *material_p = AllocateMaterial (NULL, accession_s, species_name_s, type_s, reason_s, generation_s, supplier_s, source_s, germplasm_origin_s, treatment_s, in_gru_flag, cleaned_flag, tgw, area_p, gene_bank_p -> gb_id_p, data_p);

																															if (material_p)
																																{
																																	if (SaveMaterial (material_p, data_p))
																																		{
																																			++ num_imported;
																																		}
																																	else
																																		{
																																			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, table_row_json_p, "Failed to save Material");
																																			success_flag = false;
																																		}

																																	FreeMaterial (material_p);
																																}		/* if (material_p) */

																														}		/* if (GetJSONBoolean (table_row_json_p, S_CLEANED_TITLE_S, &cleaned_flag)) */
																													else
																														{
																															PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, table_row_json_p, "Failed to get \"%s\"", S_CLEANED_TITLE_S);
																														}

																												}		/* if (!IsStringEmpty (treatment_s)) */
																											else
																												{
																													PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, table_row_json_p, "Failed to get \"%s\"", S_CLEANED_TITLE_S);
																												}
																										}		/* if (GetJSONInteger (table_row_json_p, S_TGW_TITLE_S, (int *) &tgw)) */
																									else
																										{
																											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, table_row_json_p, "Failed to get \"%s\"", S_TGW_TITLE_S);
																										}

																								}		/* if (!IsStringEmpty (accession_s)) */
																							else
																								{
																									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, table_row_json_p, "Failed to get \"%s\"", S_GRU_ACCESSION_TITLE_S);
																								}

																						}
																					else
																						{
																							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, table_row_json_p, "Failed to get \"%s\"", S_CLEANED_TITLE_S);
																						}

																				}		/* if (!IsStringEmpty (germplasm_origin_s)) */
																			else
																				{
																					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, table_row_json_p, "Failed to get \"%s\"", S_GERMPLASM_ORIGIN_TITLE_S);
																				}

																		}		/* if (!IsStringEmpty (source_s)) */
																	else
																		{
																			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, table_row_json_p, "Failed to get \"%s\"", S_SEED_SOURCE_TITLE_S);
																		}

																}		/* if (!IsStringEmpty (supplier_s)) */
															else
																{
																	PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, table_row_json_p, "Failed to get \"%s\"", S_SEED_SUPPLIER_TITLE_S);
																}

														}		/* if (!IsStringEmpty (generation_s)) */
													else
														{
															PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, table_row_json_p, "Failed to get \"%s\"", S_GENERATION_TITLE_S);
														}

												}		/* if (!IsStringEmpty (reason_s)) */
											else
												{
													PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, table_row_json_p, "Failed to get \"%s\"", S_SELECTION_REASON_TITLE_S);
												}

										}		/* if (!IsStringEmpty (type_s)) */
									else
										{
											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, table_row_json_p, "Failed to get \"%s\"", S_TYPE_TITLE_S);
										}

								}		/* if (!IsStringEmpty (germplasm_id_s)) */
							else
								{
									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, table_row_json_p, "Failed to get \"%s\"", S_GERMPLASM_ID_TITLE_S);
								}

						}		/* if (!IsStringEmpty (species_name_s)) */
					else
						{
							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, table_row_json_p, "Failed to get \"%s\"", S_SPECIES_NAME_TITLE_S);
						}
				}		/* for (i = 0; i < num_rows; ++ i) */


			if (num_imported == num_rows)
				{
					status = OS_SUCCEEDED;
				}
			else if (num_imported > 0)
				{
					status = OS_PARTIALLY_SUCCEEDED;
				}

		}		/* if (json_is_array (plots_json_p)) */

	SetServiceJobStatus (job_p, status);

	return success_flag;
}


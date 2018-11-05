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
#include "experimental_area_jobs.h"
#include "gene_bank.h"
#include "gene_bank_jobs.h"


/*
 * static declarations
 */

static const char S_DEFAULT_COLUMN_DELIMITER =  '|';

static const char * const S_INTERNAL_NAME_TITLE_S = "Accession";
static const char * const S_PEDIGREE_TITLE_S = "Pedigree";
static const char * const S_BARCODE_TITLE_S = "Barcode";
static const char * const S_ACCESSION_TITLE_S = "Store code";

static NamedParameterType S_MATERIAL_TABLE_COLUMN_DELIMITER = { "MA Data delimiter", PT_CHAR };
static NamedParameterType S_MATERIAL_TABLE = { "MA Upload", PT_TABLE};
static NamedParameterType S_EXPERIMENTAL_AREAS_LIST = { "MA Experimental Area", PT_STRING };
static NamedParameterType S_GENE_BANKS_LIST = { "MA Gene Bank", PT_STRING };

static Parameter *GetTableParameter (ParameterSet *param_set_p, ParameterGroup *group_p, const DFWFieldTrialServiceData *data_p);

static bool AddMaterialsFromJSON (ServiceJob *job_p, const json_t *materials_json_p, ExperimentalArea *area_p, GeneBank *gene_bank_p, const DFWFieldTrialServiceData *data_p);



/*
 * API definitions
 */

bool AddSubmissionMaterialParams (ServiceData *data_p, ParameterSet *param_set_p)
{
	bool success_flag = false;

	ParameterGroup *group_p = CreateAndAddParameterGroupToParameterSet ("Materials", NULL, false, data_p, param_set_p);

	if (group_p)
		{
			Parameter *param_p = NULL;
			SharedType def;

			InitSharedType (&def);

			if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_EXPERIMENTAL_AREAS_LIST.npt_type, S_EXPERIMENTAL_AREAS_LIST.npt_name_s, "Experimental Area", "The available experimental areas", def, PL_ALL)) != NULL)
				{
					const DFWFieldTrialServiceData *dfw_service_data_p = (DFWFieldTrialServiceData *) data_p;

					if (SetUpExperimentalAreasListParameter (dfw_service_data_p, param_p))
						{
							if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_GENE_BANKS_LIST.npt_type, S_GENE_BANKS_LIST.npt_name_s, "Gene Bank", "The available gene banks", def, PL_ALL)) != NULL)
								{
									if (SetUpGenBanksListParameter ((DFWFieldTrialServiceData *) data_p, param_p))
										{
											def.st_char_value = S_DEFAULT_COLUMN_DELIMITER;

											if ((param_p = CreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_MATERIAL_TABLE_COLUMN_DELIMITER.npt_type, false, S_MATERIAL_TABLE_COLUMN_DELIMITER.npt_name_s, "Delimiter", "The character delimiting columns", NULL, def, NULL, NULL, PL_ADVANCED, NULL)) != NULL)
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

							if (GetParameterValueFromParameterSet (param_set_p, S_EXPERIMENTAL_AREAS_LIST.npt_name_s, &value, true))
								{
									ExperimentalArea *area_p = GetExperimentalAreaByIdString (value.st_string_value_s, VF_STORAGE, data_p);

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

																	bson_oid_to_string (area_p -> ea_id_p, area_id_s);
																	PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, materials_json_p, "AddMaterialsFromJSON for GeneBank with id \"%s\" and ExperimentalArea with id \"%s\"", value.st_string_value_s,materials_json_p);
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

											FreeExperimentalArea (area_p);
										}		/* if (area_p) */
									else
										{
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get ExperimentalArea with id \"%s\"", value.st_string_value_s);
										}

								}		/* if (GetParameterValueFromParameterSet (param_set_p, S_EXPERIMENTAL_AREAS_LIST.npt_name_s, &value, true)) */
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get \"%s\" parameter value", S_EXPERIMENTAL_AREAS_LIST.npt_name_s);
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


/*
 * static definitions
 */

static Parameter *GetTableParameter (ParameterSet *param_set_p, ParameterGroup *group_p, const DFWFieldTrialServiceData *data_p)
{
	Parameter *param_p = NULL;
	const char delim_s [2] = { S_DEFAULT_COLUMN_DELIMITER, '\0' };
	char *headers_s = NULL;
	SharedType def;

	InitSharedType (&def);

	headers_s = ConcatenateVarargsStrings (S_INTERNAL_NAME_TITLE_S, delim_s, S_PEDIGREE_TITLE_S, delim_s, S_BARCODE_TITLE_S, delim_s, S_ACCESSION_TITLE_S, delim_s, NULL);

	if (headers_s)
		{
			param_p = CreateAndAddParameterToParameterSet (& (data_p -> dftsd_base_data), param_set_p, group_p, S_MATERIAL_TABLE.npt_type, false, S_MATERIAL_TABLE.npt_name_s, "Material data to upload", "The data to upload", NULL, def, NULL, NULL, PL_ALL, NULL);

			if (param_p)
				{
					bool success_flag = false;

					if (AddParameterKeyValuePair (param_p, PA_TABLE_COLUMN_HEADINGS_S, headers_s))
						{
							if (AddParameterKeyValuePair (param_p, PA_TABLE_COLUMN_DELIMITER_S, delim_s))
								{
									success_flag = true;
								}
						}

					if (!success_flag)
						{
							FreeParameter (param_p);
							param_p = NULL;
						}

				}		/* if (param_p) */

			FreeCopiedString (headers_s);
		}		/* if (headers_s) */

	return param_p;
}


static bool AddMaterialsFromJSON (ServiceJob *job_p, const json_t *materials_json_p, ExperimentalArea *area_p, GeneBank *gene_bank_p, const DFWFieldTrialServiceData *data_p)
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
					const char *internal_name_s = GetJSONString (table_row_json_p, S_INTERNAL_NAME_TITLE_S);

					if (!IsStringEmpty (internal_name_s))
						{
							const char *accession_s = GetJSONString (table_row_json_p, S_ACCESSION_TITLE_S);

							if (!IsStringEmpty (accession_s))
								{
									const char *pedigree_s = GetJSONString (table_row_json_p, S_PEDIGREE_TITLE_S);
									const char *barcode_s = GetJSONString (table_row_json_p, S_BARCODE_TITLE_S);

									Material *material_p = AllocateMaterial (NULL, accession_s, pedigree_s, barcode_s, internal_name_s, area_p, gene_bank_p -> gb_id_p, data_p);

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
									else
										{
											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, table_row_json_p, "Failed to allocate Material");
										}

								}		/* if (!IsStringEmpty (accession_s)) */
							else
								{
									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, table_row_json_p, "Failed to get \"%s\"", S_ACCESSION_TITLE_S);
								}

						}		/* if (!IsStringEmpty (internal_name_s)) */
					else
						{
							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, table_row_json_p, "Failed to get \"%s\"", S_INTERNAL_NAME_TITLE_S);
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


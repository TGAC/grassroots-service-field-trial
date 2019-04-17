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
 * crop_jobs.c
 *
 *  Created on: 17 Apr 2019
 *      Author: billy
 */



#include "crop.h"
#include "crop_jobs.h"
#include "string_utils.h"



/*
 * static declarations
 */


typedef struct Crop
{
	bson_oid_t *cr_id_p;

	char *cr_name_s;

	char *cr_argovoc_preferred_term_s;

	char *cr_agrovoc_uri_s;

	/*
	 * A NULL-terminated array of synonyms for this crop variety.
	 * This can be NULL meaning there are no synonyms.
	 */
	char **cr_synonyms_ss;

} Crop;

static NamedParameterType S_NAME = { "CR Name", PT_KEYWORD };
static NamedParameterType S_PREFERRED_TERM = { "CR Preferred Term", PT_KEYWORD};
static NamedParameterType S_ONTOLOGY_URL = { "CR Ontology URL", PT_STRING };
static NamedParameterType S_SYNONYMS = { "CR Synonyms", PT_LARGE_STRING };


static bool AddCropsFromJSON (ServiceJob *job_p, const json_t *materials_json_p, Study *area_p, GeneBank *gene_bank_p, const DFWFieldTrialServiceData *data_p);

/*
 * API definitions
 */

bool AddSubmissionCropParams (ServiceData *data_p, ParameterSet *param_set_p)
{
	bool success_flag = false;

	ParameterGroup *group_p = CreateAndAddParameterGroupToParameterSet ("Crops", false, data_p, param_set_p);

	if (group_p)
		{
			Parameter *param_p = NULL;
			SharedType def;

			InitSharedType (&def);

			def.st_string_value_s = NULL;

			if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_NAME.npt_type, S_NAME.npt_name_s, "Name", "The crop name", def, PL_ALL)) != NULL)
				{
					if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_PREFERRED_TERM.npt_type, S_PREFERRED_TERM.npt_name_s, "Preferred name", "The AgroVOC preferred term for the crop", def, PL_ALL)) != NULL)
						{
							if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_ONTOLOGY_URL.npt_type, S_ONTOLOGY_URL.npt_name_s, "Definition", "The URL of the ontology definition for this crop", def, PL_ALL)) != NULL)
								{
									if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_SYNONYMS.npt_type, S_SYNONYMS.npt_name_s, "Synonyms", "The comma-separated list of synonyms for this crop", def, PL_ALL)) != NULL)
										{
											success_flag = true;
										}
								}
						}
				}
		}		/* if (group_p) */


	return success_flag;
}


bool RunForSubmissionCropParams (DFWFieldTrialServiceData *data_p, ParameterSet *param_set_p, ServiceJob *job_p)
{
	bool job_done_flag = false;

	SharedType name_value;
	InitSharedType (&name_value);

	if (GetParameterValueFromParameterSet (param_set_p, S_NAME.npt_name_s, &name_value, true))
		{
			/*
			 * The name is required
			 */
			if (! (IsStringEmpty (name_value.st_string_value_s)))
				{
					SharedType term_value;
					InitSharedType (&term_value);

					if (GetParameterValueFromParameterSet (param_set_p, S_PREFERRED_TERM.npt_name_s, &term_value, true))
						{
							SharedType ontology_value;
							InitSharedType (&ontology_value);

							if (GetParameterValueFromParameterSet (param_set_p, S_ONTOLOGY_URL.npt_name_s, &ontology_value, true))
								{
									SharedType synonyms_value;
									InitSharedType (&synonyms_value);

									if (GetParameterValueFromParameterSet (param_set_p, S_SYNONYMS.npt_name_s, &synonyms_value, true))
										{
											char **synonyms_ss = NULL;


											if (!IsStringEmpty (synonyms_value.st_string_value_s))
												{

												}		/* if (!IsStringEmpty (synonyms_value.st_string_value_s)) */

										}		/* if (GetParameterValueFromParameterSet (param_set_p, S_SYNONYMS.npt_name_s, &synonyms_value, true)) */
									else
										{
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get \"%s\" parameter value", S_SYNONYMS.npt_name_s);
										}

								}		/* if (GetParameterValueFromParameterSet (param_set_p, S_ONTOLOGY_URL.npt_name_s, &ontology_value, true)) */
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get \"%s\" parameter value", S_ONTOLOGY_URL.npt_name_s);
								}

						}		/* if (GetParameterValueFromParameterSet (param_set_p, S_PREFERRED_TERM.npt_name_s, &term_value, true)) */
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get \"%s\" parameter value", S_PREFERRED_TERM.npt_name_s);
						}

					job_done_flag = true;
				}		/* if (! (IsStringEmpty (value.st_string_value_s))) */

		}		/* if (GetParameterValueFromParameterSet (param_set_p, S_NAME.npt_name_s, &name_value, true)) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get \"%s\" parameter value", S_NAME.npt_name_s);
		}

	return job_done_flag;
}


bool GetSubmissionCropParameterTypeForNamedParameter (const char *param_name_s, ParameterType *pt_p)
{
	bool success_flag = true;

	if (strcmp (param_name_s, S_NAME.npt_name_s) == 0)
		{
			*pt_p = S_NAME.npt_type;
		}
	else if (strcmp (param_name_s, S_PREFERRED_TERM.npt_name_s) == 0)
		{
			*pt_p = S_PREFERRED_TERM.npt_type;
		}
	else if (strcmp (param_name_s, S_ONTOLOGY_URL.npt_name_s) == 0)
		{
			*pt_p = S_ONTOLOGY_URL.npt_type;
		}
	else if (strcmp (param_name_s, S_SYNONYMS.npt_name_s) == 0)
		{
			*pt_p = S_SYNONYMS.npt_type;
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

static bool AddCropsFromJSON (ServiceJob *job_p, const json_t *materials_json_p, Study *area_p, GeneBank *gene_bank_p, const DFWFieldTrialServiceData *data_p)
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

									Crop *material_p = AllocateCrop (NULL, accession_s, pedigree_s, barcode_s, internal_name_s, area_p, gene_bank_p -> gb_id_p, data_p);

									if (material_p)
										{
											if (SaveCrop (material_p, data_p))
												{
													++ num_imported;
												}
											else
												{
													PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, table_row_json_p, "Failed to save Crop");
													success_flag = false;
												}

											FreeCrop (material_p);
										}		/* if (material_p) */
									else
										{
											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, table_row_json_p, "Failed to allocate Crop");
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


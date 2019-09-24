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
 * drilling_jobs.c
 *
 *  Created on: 24 Sep 2019
 *      Author: billy
 */


#include "plot.h"


typedef enum
{
	DP_INDEX,
	DP_ROW,
	DP_COLUMN,
	DP_GENOTYPE_REPLICATE,
	DP_ACCESSION,
	DP_SOWING_DATE,
	DP_SOWING_RATE,
	DP_NUM_PARAMS
} DrillingParam;


static const char * const S_INDEX_TITLE_S = "Plot ID";
static const char * const S_ROW_TITLE_S = "Row";
static const char * const S_COLUMN_TITLE_S = "Column";
static const char * const S_REPLICATE_TITLE_S = "Genotype Replicate";
static const char * const S_ACCESSION_TITLE_S = "Accession";
static const char * const S_SOWING_DATE_TITLE_S = "Sowing Date";
static const char * const S_SOWING_RATE_TITLE_S = "Sowing Rate";


static NamedParameterType S_DRILLING_TABLE_COLUMN_DELIMITER = { "DR Data delimiter", PT_CHAR };
static NamedParameterType S_DRILLING_TABLE = { "DR Upload", PT_TABLE};


static NamedParameterType S_STUDIES_LIST = { "DR Study", PT_STRING };


static const char S_DEFAULT_COLUMN_DELIMITER =  '|';


/*
 * Static declarations
 */


static Parameter *GetTableParameter (ParameterSet *param_set_p, ParameterGroup *group_p, const DFWFieldTrialServiceData *data_p);

static json_t *GetTableParameterHints (void);


/*
 * API definitions
 */


bool AddSubmissionDrillingParams (ServiceData *data_p, ParameterSet *param_set_p)
{
	bool success_flag = false;
	Parameter *param_p = NULL;
	ParameterGroup *group_p = CreateAndAddParameterGroupToParameterSet ("Plots", false, data_p, param_set_p);
	SharedType def;

	InitSharedType (&def);

	if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_STUDIES_LIST.npt_type, S_STUDIES_LIST.npt_name_s, "Experimental Areas", "The available experimental areas", def, PL_ALL)) != NULL)
		{
			const DFWFieldTrialServiceData *dfw_service_data_p = (DFWFieldTrialServiceData *) data_p;

			if (SetUpStudiesListParameter (dfw_service_data_p, param_p, NULL))
				{
					def.st_char_value = S_DEFAULT_COLUMN_DELIMITER;

					if ((param_p = CreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_DRILLING_TABLE_COLUMN_DELIMITER.npt_type, false, S_DRILLING_TABLE_COLUMN_DELIMITER.npt_name_s, "Delimiter", "The character delimiting columns", NULL, def, NULL, NULL, PL_ADVANCED, NULL)) != NULL)
						{
							def.st_string_value_s = NULL;

							if ((param_p = GetTableParameter (param_set_p, group_p, dfw_service_data_p)) != NULL)
								{
									success_flag = true;
								}
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "GetTableParameter failed");
								}
						}
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", S_STUDIES_LIST.npt_name_s);
						}
				}
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "SetUpStudysListParameter failed");
				}
		}
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", S_STUDIES_LIST.npt_name_s);
		}

	return success_flag;
}


bool RunForSubmissionDrillingParams (DFWFieldTrialServiceData *data_p, ParameterSet *param_set_p, ServiceJob *job_p)
{
	bool job_done_flag = false;
	SharedType value;

	if (GetCurrentParameterValueFromParameterSet (param_set_p, S_DRILLING_TABLE.npt_name_s, &value))
		{
			/*
			 * Has a spreadsheet been uploaded?
			 */
			if (! (IsStringEmpty (value.st_string_value_s)))
				{
					bool success_flag = false;
					json_error_t e;
					json_t *plots_json_p = NULL;

					/*
					 * The data could be either an array of json objects
					 * or a tabular string. so try it as json array first
					 */
					plots_json_p = json_loads (value.st_string_value_s, 0, &e);

					if (plots_json_p)
						{
							SharedType parent_study_value;

							if (GetCurrentParameterValueFromParameterSet (param_set_p, S_STUDIES_LIST.npt_name_s, &parent_study_value))
								{
									Study *area_p = GetStudyByIdString (parent_study_value.st_string_value_s, VF_STORAGE, data_p);

									if (area_p)
										{
											success_flag = AddDrillingsFromJSON (job_p, plots_json_p, area_p, data_p);

											FreeStudy (area_p);
										}
								}

							json_decref (plots_json_p);
						}		/* if (plots_json_p) */
					else
						{
							SharedType delimiter;
							InitSharedType (&delimiter);

							delimiter.st_char_value = S_DEFAULT_COLUMN_DELIMITER;
							GetParameterValueFromParameterSet (param_set_p, S_DRILLING_TABLE_COLUMN_DELIMITER.npt_name_s, &delimiter, true);

							success_flag = AddDrillingsTableFromTabularString (job_p, value.st_string_value_s, delimiter.st_char_value, data_p);
						}

					job_done_flag = true;
				}		/* if (value.st_boolean_value) */

		}		/* if (GetParameterValueFromParameterSet (param_set_p, S_ADD_EXPERIMENTAL_AREA.npt_name_s, &value, true)) */


	return job_done_flag;
}


bool GetSubmissionDrillingParameterTypeForNamedParameter (const char *param_name_s, ParameterType *pt_p)
{
	bool success_flag = true;

	if (strcmp (param_name_s, S_STUDIES_LIST.npt_name_s) == 0)
		{
			*pt_p = S_STUDIES_LIST.npt_type;
		}
	else if (strcmp (param_name_s, S_DRILLING_TABLE_COLUMN_DELIMITER.npt_name_s) == 0)
		{
			*pt_p = S_DRILLING_TABLE_COLUMN_DELIMITER.npt_type;
		}
	else if (strcmp (param_name_s, S_DRILLING_TABLE.npt_name_s) == 0)
		{
			*pt_p = S_DRILLING_TABLE.npt_type;
		}
	else
		{
			success_flag = false;
		}

	return success_flag;
}



/*
 * STATIC DEFINITIONS
 */


static Parameter *GetTableParameter (ParameterSet *param_set_p, ParameterGroup *group_p, const DFWFieldTrialServiceData *data_p)
{
	Parameter *param_p = NULL;
	const char delim_s [2] = { S_DEFAULT_COLUMN_DELIMITER, '\0' };
	SharedType def;

	InitSharedType (&def);

	param_p = CreateAndAddParameterToParameterSet (& (data_p -> dftsd_base_data), param_set_p, group_p, S_DRILLING_TABLE.npt_type, false, S_DRILLING_TABLE.npt_name_s, "Drilling data to upload", "The data to upload", NULL, def, NULL, NULL, PL_ALL, NULL);

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
				}		/* if (hints_p) */


			if (!success_flag)
				{
					FreeParameter (param_p);
					param_p = NULL;
				}

		}		/* if (param_p) */

	return param_p;
}



static json_t *GetTableParameterHints (void)
{
	json_t *hints_p = json_array ();

	if (hints_p)
		{
			if (AddColumnParameterHint (S_INDEX_TITLE_S, PT_UNSIGNED_INT, hints_p))
				{
					if (AddColumnParameterHint (S_ROW_TITLE_S, PT_UNSIGNED_INT, hints_p))
						{
							if (AddColumnParameterHint (S_COLUMN_TITLE_S, PT_UNSIGNED_INT, hints_p))
								{
									if (AddColumnParameterHint (S_REPLICATE_TITLE_S, PT_UNSIGNED_INT, hints_p))
										{
											if (AddColumnParameterHint (S_ACCESSION_TITLE_S, PT_STRING, hints_p))
												{
													if (AddColumnParameterHint (S_SOWING_DATE_TITLE_S, PT_STRING, hints_p))
														{
															if (AddColumnParameterHint (S_SOWING_RATE_TITLE_S, PT_STRING, hints_p))
																{
																	return hints_p;
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


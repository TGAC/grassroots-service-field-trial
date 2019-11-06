/*
 * phenotype_jobs.c
 *
 *  Created on: 6 Nov 2019
 *      Author: billy
 */

#include <string.h>

#include "observation.h"
#include "phenotype_jobs.h"
#include "study.h"
#include "streams.h"
#include "study_jobs.h"


/*
 * Static variables
 */

static NamedParameterType S_PHENOTYPES_TABLE_COLUMN_DELIMITER = { "PH Data delimiter", PT_CHAR };
static NamedParameterType S_PHENOTYPES_TABLE = { "PH Upload", PT_TABLE};


static NamedParameterType S_STUDIES_LIST = { "PH Study", PT_STRING };


static const char S_DEFAULT_COLUMN_DELIMITER =  '|';

static const char * const S_PLOT_INDEX_TITLE_S = "Plot ID";


/*
 * Static declarations
 */

static Parameter *GetTableParameter (ParameterSet *param_set_p, ParameterGroup *group_p, const DFWFieldTrialServiceData *data_p);

static json_t *GetTableParameterHints (void);

static bool AddPhenotypesFromJSON (ServiceJob *job_p, const json_t *phenotypes_json_p, Study *area_p, const DFWFieldTrialServiceData *data_p);


/*
 * API definitions
 */

bool AddSubmissionPhenotypeParams (ServiceData *data_p, ParameterSet *param_set_p, Resource *resource_p)
{
	bool success_flag = false;
	Parameter *param_p = NULL;
	ParameterGroup *group_p = CreateAndAddParameterGroupToParameterSet ("Phenotypes", false, data_p, param_set_p);
	SharedType def;

	InitSharedType (&def);

	if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_STUDIES_LIST.npt_type, S_STUDIES_LIST.npt_name_s, "Study", "The Study that these phenotype are from", def, PL_ALL)) != NULL)
		{
			const DFWFieldTrialServiceData *dfw_service_data_p = (DFWFieldTrialServiceData *) data_p;

			if (SetUpStudiesListParameter (dfw_service_data_p, param_p, NULL))
				{
					def.st_char_value = S_DEFAULT_COLUMN_DELIMITER;

					if ((param_p = CreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_PHENOTYPES_TABLE_COLUMN_DELIMITER.npt_type, false, S_PHENOTYPES_TABLE_COLUMN_DELIMITER.npt_name_s, "Delimiter", "The character delimiting columns", NULL, def, NULL, NULL, PL_ADVANCED, NULL)) != NULL)
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
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "SetUpStudiesListParameter failed");
				}
		}
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", S_STUDIES_LIST.npt_name_s);
		}

	return success_flag;
}


bool GetSubmissionPhenotypesParameterTypeForNamedParameter (const char *param_name_s, ParameterType *pt_p)
{
	bool success_flag = true;

	if (strcmp (param_name_s, S_STUDIES_LIST.npt_name_s) == 0)
		{
			*pt_p = S_STUDIES_LIST.npt_type;
		}
	else if (strcmp (param_name_s, S_PHENOTYPES_TABLE_COLUMN_DELIMITER.npt_name_s) == 0)
		{
			*pt_p = S_PHENOTYPES_TABLE_COLUMN_DELIMITER.npt_type;
		}
	else if (strcmp (param_name_s, S_PHENOTYPES_TABLE.npt_name_s) == 0)
		{
			*pt_p = S_PHENOTYPES_TABLE.npt_type;
		}
	else
		{
			success_flag = false;
		}

	return success_flag;
}


bool RunForSubmissionPhenotypesParams (DFWFieldTrialServiceData *data_p, ParameterSet *param_set_p, ServiceJob *job_p)
{
	bool job_done_flag = false;

	SharedType value;
	InitSharedType (&value);

	if (GetParameterValueFromParameterSet (param_set_p, S_PHENOTYPES_TABLE.npt_name_s, &value, true))
		{
			/*
			 * Has a spreadsheet been uploaded?
			 */
			if (! (IsStringEmpty (value.st_string_value_s)))
				{
					bool success_flag = false;
					json_error_t e;
					json_t *phenotypes_json_p = NULL;

					/*
					 * The data could be either an array of json objects
					 * or a tabular string. so try it as json array first
					 */
					phenotypes_json_p = json_loads (value.st_string_value_s, 0, &e);

					if (phenotypes_json_p)
						{
							SharedType parent_study_value;
							InitSharedType (&parent_study_value);

							if (GetParameterValueFromParameterSet (param_set_p, S_STUDIES_LIST.npt_name_s, &parent_study_value, true))
								{
									Study *study_p = GetStudyByIdString (parent_study_value.st_string_value_s, VF_STORAGE, data_p);

									if (study_p)
										{
											success_flag = AddPhenotypesFromJSON (job_p, phenotypes_json_p, study_p, data_p);

											FreeStudy (study_p);
										}
								}

							json_decref (phenotypes_json_p);
						}		/* if (phenotypes_json_p) */

					job_done_flag = true;
				}		/* if (value.st_boolean_value) */

		}		/* if (GetParameterValueFromParameterSet (param_set_p, S_ADD_EXPERIMENTAL_AREA.npt_name_s, &value, true)) */


	return job_done_flag;
}



/*
 * Static definitions
 */

static Parameter *GetTableParameter (ParameterSet *param_set_p, ParameterGroup *group_p, const DFWFieldTrialServiceData *data_p)
{
	Parameter *param_p = NULL;
	const char delim_s [2] = { S_DEFAULT_COLUMN_DELIMITER, '\0' };
	SharedType def;

	InitSharedType (&def);

	param_p = CreateAndAddParameterToParameterSet (& (data_p -> dftsd_base_data), param_set_p, group_p, S_PHENOTYPES_TABLE.npt_type, false, S_PHENOTYPES_TABLE.npt_name_s, "Plot data to upload", "The data to upload", NULL, def, NULL, NULL, PL_ALL, NULL);

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
			if (AddColumnParameterHint (S_PLOT_INDEX_TITLE_S, PT_TIME, hints_p))
				{
					return hints_p;
				}
			json_decref (hints_p);
		}

	return NULL;
}



static bool AddPhenotypesFromJSON (ServiceJob *job_p, const json_t *phenotypes_json_p, Study *area_p, const DFWFieldTrialServiceData *data_p)
{
	OperationStatus status = OS_FAILED;
	bool success_flag	= true;

	if (json_is_array (phenotypes_json_p))
		{
			const size_t num_rows = json_array_size (phenotypes_json_p);
			size_t i;
			size_t num_imported = 0;
			bool imported_row_flag;

			for (i = 0; i < num_rows; ++ i)
				{
					const json_t *table_row_json_p = json_array_get (phenotypes_json_p, i);

					imported_row_flag = false;



					if (!imported_row_flag)
						{
							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, table_row_json_p, "Failed to import plot data");
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


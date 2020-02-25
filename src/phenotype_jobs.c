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
#include "string_utils.h"

#include "char_parameter.h"
#include "string_parameter.h"
#include "json_parameter.h"



/*
 * Static variables
 */

static NamedParameterType S_PHENOTYPES_TABLE_COLUMN_DELIMITER = { "PH Data delimiter", PT_CHAR };
static NamedParameterType S_PHENOTYPES_TABLE = { "PH Upload", PT_JSON_TABLE};


static NamedParameterType S_STUDIES_LIST = { "PH Study", PT_STRING };


static const char S_DEFAULT_COLUMN_DELIMITER =  '|';

static const char * const S_ROW_INDEX_TITLE_S = "Plot ID";


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

	if ((param_p = EasyCreateAndAddStringParameterToParameterSet (data_p, param_set_p, group_p, S_STUDIES_LIST.npt_type, S_STUDIES_LIST.npt_name_s, "Study", "The Study that these phenotype are from", NULL, PL_ALL)) != NULL)
		{
			const DFWFieldTrialServiceData *dfw_service_data_p = (DFWFieldTrialServiceData *) data_p;

			if (SetUpStudiesListParameter (dfw_service_data_p, (StringParameter *) param_p, NULL, false))
				{
					const char delim = S_DEFAULT_COLUMN_DELIMITER;

					if ((param_p = EasyCreateAndAddCharParameterToParameterSet (data_p, param_set_p, group_p, S_PHENOTYPES_TABLE_COLUMN_DELIMITER.npt_name_s, "Delimiter", "The character delimiting columns", &delim, PL_ADVANCED)) != NULL)
						{
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
	const json_t *phenotypes_json_p = NULL;

	if (GetCurrentJSONParameterValueFromParameterSet (param_set_p, S_PHENOTYPES_TABLE.npt_name_s, &phenotypes_json_p))
		{
			/*
			 * Has a spreadsheet been uploaded?
			 */
			if ((phenotypes_json_p != NULL) && (json_array_size (phenotypes_json_p) > 0))
				{
					OperationStatus status = OS_FAILED;

					/*
					 * The data could be either an array of json objects
					 * or a tabular string. so try it as json array first
					 */
					const char *study_id_s = NULL;

					if (GetCurrentStringParameterValueFromParameterSet (param_set_p, S_STUDIES_LIST.npt_name_s, &study_id_s))
						{
							Study *study_p = GetStudyByIdString (study_id_s, VF_STORAGE, data_p);

							if (study_p)
								{
									if (AddPhenotypesFromJSON (job_p, phenotypes_json_p, study_p, data_p))
										{
											status = OS_SUCCEEDED;
										}

									FreeStudy (study_p);
								}
						}

					SetServiceJobStatus (job_p, status);

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
	Parameter *param_p = EasyCreateAndAddJSONParameterToParameterSet (& (data_p -> dftsd_base_data), param_set_p, group_p, S_PHENOTYPES_TABLE.npt_type, S_PHENOTYPES_TABLE.npt_name_s, "Plot data to upload", "The data to upload", NULL,  PL_ALL);

	if (param_p)
		{
			bool success_flag = false;
			json_t *hints_p = GetTableParameterHints ();

			if (hints_p)
				{
					if (AddParameterKeyJSONValuePair (param_p, PA_TABLE_COLUMN_HEADINGS_S, hints_p))
						{
							const char delim_s [2] = { S_DEFAULT_COLUMN_DELIMITER, '\0' };

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
			if (AddColumnParameterHint (S_ROW_INDEX_TITLE_S, PT_TIME, hints_p))
				{
					return hints_p;
				}
			json_decref (hints_p);
		}

	return NULL;
}



static bool AddPhenotypesFromJSON (ServiceJob *job_p, const json_t *phenotypes_json_p, Study *study_p, const DFWFieldTrialServiceData *data_p)
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
					const json_t *phenotype_json_p = json_array_get (phenotypes_json_p, i);
					int32 row_index = 0;

					imported_row_flag = false;


					if (GetJSONInteger (phenotype_json_p, S_ROW_INDEX_TITLE_S, &row_index))
						{
							const char *key_s;
							json_t *value_p;
							const char *date_suffix_s = " date";

							json_object_foreach (phenotype_json_p, key_s, value_p)
								{
									/*
									 * Is it a measurement date or the treatment?
									 */
									if (!DoesStringEndWith (key_s, date_suffix_s))
										{
											char *date_key_s = ConcatenateStrings (key_s, date_suffix_s);

											if (date_key_s)
												{
													const char *date_s = GetJSONString (phenotype_json_p, date_key_s);

													if (date_s)
														{
															if (json_is_string (value_p))
																{
																	const char *treatment_s = json_string_value (value_p);

																	if (treatment_s)
																		{
																			Observation *obs_p;

																			//AddObservationToRow ();
																		}
																}
														}

													FreeCopiedString (date_key_s);
												}
										}
								}

						}		/* if (GetJSONInteger (phenotype_json_p, S_PLOT_INDEX_TITLE_S, &plot_index)) */
					else
						{
							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, phenotype_json_p, "No \"%s\" key", S_ROW_INDEX_TITLE_S);
						}


					if (!imported_row_flag)
						{
							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, phenotype_json_p, "Failed to import plot data");
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


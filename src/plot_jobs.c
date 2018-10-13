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
 * plot_jobs.c
 *
 *  Created on: 1 Oct 2018
 *      Author: billy
 */


#include "plot_jobs.h"

#include "plot.h"
#include "time_util.h"
#include "string_utils.h"
#include "experimental_area_jobs.h"


typedef enum
{
	PP_SOWING_DATE,
	PP_HARVEST_DATE,
	PP_WIDTH,
	PP_LENGTH,
	PP_ROW,
	PP_COLUMN,
	PP_TRIAL_DESIGN,
	PP_GROWING_CONDITION,
	PP_TREATMENT,
	PP_NUM_PARAMS
} PlotParam;


static const char * const S_SOWING_TITLE_S = "Sowing date";
static const char * const S_HARVEST_TITLE_S = "Harvest date";
static const char * const S_WIDTH_TITLE_S = "Width";
static const char * const S_LENGTH_TITLE_S = "Length";
static const char * const S_ROW_TITLE_S = "Row";
static const char * const S_COLUMN_TITLE_S = "Column";
static const char * const S_TRIAL_DESIGN_TITLE_S = "Trial design";
static const char * const S_GROWING_CONDITION_TITLE_S = "Growing condition";
static const char * const S_TREATMENT_TITLE_S = "Treatment";

static NamedParameterType S_PLOT_SOWING_DATE = { "PL Sowing Year", PT_TIME };
static NamedParameterType S_PLOT_HARVEST_DATE = { "PL Harvest Year", PT_TIME };
static NamedParameterType S_PLOT_WIDTH = { "PL Width", PT_UNSIGNED_REAL };
static NamedParameterType S_PLOT_LENGTH = { "PL Length", PT_UNSIGNED_REAL };


static NamedParameterType S_PLOT_ROW = { "PL Row", PT_UNSIGNED_INT };
static NamedParameterType S_PLOT_COLUMN = { "PL Column", PT_UNSIGNED_INT };

static NamedParameterType S_PLOT_TRIAL_DESIGN = { "PL Trial Design", PT_STRING };
static NamedParameterType S_PLOT_GROWING_CONDITION = { "PL Growing Condition", PT_STRING };
static NamedParameterType S_PLOT_TREATMENT = { "PL Treatment", PT_STRING };

static NamedParameterType S_PLOT_TABLE_COLUMN_DELIMITER = { "PL Data delimiter", PT_CHAR };
static NamedParameterType S_PLOT_TABLE = { "PL Upload", PT_TABLE};


static NamedParameterType S_EXPERIMENTAL_AREAS_LIST = { "PL Experimental Area", PT_STRING };


static const char S_DEFAULT_COLUMN_DELIMITER =  '|';


/*
 * static declarations
 */

static bool AddPlotsTableFromTabularString (ServiceJob *job_p, const char *table_data_s, const char delimiter, const DFWFieldTrialServiceData *data_p);

static bool AddPlotsFromJSON (ServiceJob *job_p, const json_t *plots_json_p, ExperimentalArea *area_p,  const DFWFieldTrialServiceData *data_p);

static Plot *GetPlotFromTableRow (const char *current_row_s, const char column_delimiter);


/*
 * API definitions
 */

bool AddPlotParams (ServiceData *data_p, ParameterSet *param_set_p)
{
	bool success_flag = false;
	Parameter *param_p = NULL;
	ParameterGroup *group_p = CreateAndAddParameterGroupToParameterSet ("Plots", NULL, false, data_p, param_set_p);
	SharedType def;

	InitSharedType (&def);

	if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_EXPERIMENTAL_AREAS_LIST.npt_type, S_EXPERIMENTAL_AREAS_LIST.npt_name_s, "Experimental Areas", "The available experimental areas", def, PL_ALL)) != NULL)
		{
			if (SetUpExperimentalAreasListParameter ((DFWFieldTrialServiceData *) data_p, param_p))
				{
					def.st_char_value = S_DEFAULT_COLUMN_DELIMITER;

					if ((param_p = CreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_PLOT_TABLE_COLUMN_DELIMITER.npt_type, false, S_PLOT_TABLE_COLUMN_DELIMITER.npt_name_s, "Delimiter", "The character delimiting columns", NULL, def, NULL, NULL, PL_ALL, NULL)) != NULL)
						{
							def.st_string_value_s = NULL;

							if ((param_p = CreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_PLOT_TABLE.npt_type, false, S_PLOT_TABLE.npt_name_s, "Plot data to upload", "The data to upload", NULL, def, NULL, NULL, PL_ALL, NULL)) != NULL)
								{
									const char delim_s [2] = { S_DEFAULT_COLUMN_DELIMITER, '\0' };
									char *headers_s = ConcatenateVarargsStrings (S_SOWING_TITLE_S, delim_s, S_HARVEST_TITLE_S, delim_s, S_WIDTH_TITLE_S, delim_s, S_LENGTH_TITLE_S, delim_s, S_ROW_TITLE_S, delim_s, S_COLUMN_TITLE_S, delim_s,
																															 S_TRIAL_DESIGN_TITLE_S, delim_s, S_GROWING_CONDITION_TITLE_S, delim_s, S_TREATMENT_TITLE_S, delim_s, NULL);

									if (headers_s)
										{
											if (AddParameterKeyValuePair (param_p, PA_TABLE_COLUMN_HEADINGS_S, headers_s))
												{
													if (AddParameterKeyValuePair (param_p, PA_TABLE_COLUMN_DELIMITER_S, delim_s))
														{
															SharedType date_def;

															InitSharedType (&date_def);

															if ((date_def.st_time_p = AllocateTime ()) != NULL)
																{
																	SetDateValuesForTime (date_def.st_time_p, 2017, 1, 1);

																	if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_PLOT_SOWING_DATE.npt_type, S_PLOT_SOWING_DATE.npt_name_s, S_SOWING_TITLE_S, "The date when the seeds were sown", date_def, PL_BASIC)) != NULL)
																		{
																			if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_PLOT_HARVEST_DATE.npt_type, S_PLOT_HARVEST_DATE.npt_name_s, S_HARVEST_TITLE_S, "The date when the seeds were harvested", date_def, PL_BASIC)) != NULL)
																				{
																					InitSharedType (&def);

																					def.st_ulong_value = 0;

																					if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_PLOT_WIDTH.npt_type, S_PLOT_WIDTH.npt_name_s, S_WIDTH_TITLE_S, "The width of the plot", def, PL_BASIC)) != NULL)
																						{
																							if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_PLOT_LENGTH.npt_type, S_PLOT_LENGTH.npt_name_s, S_LENGTH_TITLE_S, "The length of the plot", def, PL_BASIC)) != NULL)
																								{
																									if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_PLOT_TRIAL_DESIGN.npt_type, S_PLOT_TRIAL_DESIGN.npt_name_s, S_TRIAL_DESIGN_TITLE_S, "The trial desgin of the plot", def, PL_BASIC)) != NULL)
																										{
																											if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_PLOT_TREATMENT.npt_type, S_PLOT_TREATMENT.npt_name_s, S_TREATMENT_TITLE_S, "The treatments of the plot", def, PL_BASIC)) != NULL)
																												{
																													if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_PLOT_GROWING_CONDITION.npt_type, S_PLOT_GROWING_CONDITION.npt_name_s, S_GROWING_CONDITION_TITLE_S, "The growing condtions of the plot", def, PL_BASIC)) != NULL)
																														{
																															success_flag = true;
																														}
																												}
																										}
																								}
																						}
																				}
																		}

																	ClearSharedType (&date_def, PT_TIME);
																}		/* if ((sowing_date.st_time_p = AllocateTime ()) != NULL) */

														}		/* if (AddParameterKeyValuePair (param_p, PA_TABLE_COLUMN_DELIMITER_S, delim_s)) */

												}		/* if (AddParameterKeyValuePair (param_p, PA_TABLE_COLUMN_HEADINGS_S, headers_s)) */

											FreeCopiedString (headers_s);
										}		/* if (headers_s) */

								}

						}

				}
		}

	return success_flag;
}


bool RunForPlotParams (DFWFieldTrialServiceData *data_p, ParameterSet *param_set_p, ServiceJob *job_p)
{
	bool job_done_flag = false;

	SharedType value;
	InitSharedType (&value);

	if (GetParameterValueFromParameterSet (param_set_p, S_PLOT_TABLE.npt_name_s, &value, true))
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
							SharedType parent_experimental_area_value;
							InitSharedType (&parent_experimental_area_value);

							if (GetParameterValueFromParameterSet (param_set_p, S_EXPERIMENTAL_AREAS_LIST.npt_name_s, &parent_experimental_area_value, true))
								{
									ExperimentalArea *area_p = GetExperimentalAreaByIdString (parent_experimental_area_value.st_string_value_s, data_p);

									if (area_p)
										{
											success_flag = AddPlotsFromJSON (job_p, plots_json_p, area_p, data_p);

											FreeExperimentalArea (area_p);
										}
								}

						}		/* if (value_as_json_p) */
					else
						{
							SharedType delimiter;
							InitSharedType (&delimiter);

							delimiter.st_char_value = S_DEFAULT_COLUMN_DELIMITER;
							GetParameterValueFromParameterSet (param_set_p, S_PLOT_TABLE_COLUMN_DELIMITER.npt_name_s, &delimiter, true);

							success_flag = AddPlotsTableFromTabularString (job_p, value.st_string_value_s, delimiter.st_char_value, data_p);
						}

					job_done_flag = true;
				}		/* if (value.st_boolean_value) */

		}		/* if (GetParameterValueFromParameterSet (param_set_p, S_ADD_EXPERIMENTAL_AREA.npt_name_s, &value, true)) */


	return job_done_flag;
}


/*
 * static definitions
 */


static bool AddPlotsFromJSON (ServiceJob *job_p, const json_t *plots_json_p, ExperimentalArea *area_p,  const DFWFieldTrialServiceData *data_p)
{
	bool success_flag	= true;

	if (json_is_array (plots_json_p))
		{
			const size_t num_rows = json_array_size (plots_json_p);
			size_t i;

			for (i = 0; i < num_rows; ++ i)
				{
					json_t *row_p = json_array_get (plots_json_p, i);
					Plot *plot_p = NULL;

					const char *sowing_date_s = GetJSONString (plots_json_p, S_PLOT_SOWING_DATE.npt_name_s);
					const char *harvest_date_s = GetJSONString (plots_json_p, S_PLOT_HARVEST_DATE.npt_name_s);
					const char *growing_condition_s = GetJSONString (plots_json_p, S_PLOT_GROWING_CONDITION.npt_name_s);
					const char *treatment_s = GetJSONString (plots_json_p, S_PLOT_TREATMENT.npt_name_s);
					const char *trial_design_s = GetJSONString (plots_json_p, S_PLOT_TRIAL_DESIGN.npt_name_s);
					const char *width_s = GetJSONString (plots_json_p, S_PLOT_WIDTH.npt_name_s);
					const char *length_s = GetJSONString (plots_json_p, S_PLOT_LENGTH.npt_name_s);
					const char *row_s = GetJSONString (plots_json_p, S_PLOT_ROW.npt_name_s);
					const char *column_s = GetJSONString (plots_json_p, S_PLOT_COLUMN.npt_name_s);




					/*
					Plot *AllocatePlot (bson_oid_t *id_p, const uint32 sowing_date, const uint32 harvest_date, const double64 width, const double64 height, const uint32 row_index,
					const uint32 column_index, const char *trial_design_s, const char *growing_conditions_s, const char *treatments_s, ExperimentalArea *parent_p);
					 */




				}		/* for (i = 0; i < num_rows; ++ i) */

		}		/* if (json_is_array (plots_json_p)) */


	return success_flag;
}



static bool AddPlotsTableFromTabularString (ServiceJob *job_p, const char *table_data_s, const char column_delimiter, const DFWFieldTrialServiceData *data_p)
{
	bool success_flag	= true;
	const char *current_row_s = table_data_s;
	const char *next_row_s = strchr (current_row_s, '\n');


	/*
	 * column headings are S_SOWING_TITLE_S, delim_s, S_HARVEST_TITLE_S, delim_s, S_WIDTH_TITLE_S, delim_s, S_LENGTH_TITLE_S, delim_s, S_ROW_TITLE_S, delim_s, S_COLUMN_TITLE_S, delim_s,
	 * S_TRIAL_DESIGN_TITLE_S, delim_s, S_GROWING_CONDITION_TITLE_S, delim_s, S_TREATMENT_TITLE_S, delim_s
	 *
	 */
	while (success_flag && (*current_row_s != '\0'))
		{
			Plot *plot_p = GetPlotFromTableRow (current_row_s, column_delimiter);

			/*
			 * move onto the next row
			 */
			current_row_s = next_row_s + 1;

			if (*current_row_s != '\0')
				{
					next_row_s = strchr (current_row_s, '\n');
				}
		}		/* while (success_flag && (*current_row_s != '\0')) */


	return success_flag;
}


static Plot *GetPlotFromTableRow (const char *current_row_s, const char column_delimiter)
{
	bool success_flag	= true;
	const char *current_column_s = current_row_s;
	const char *next_column_s = strchr (current_column_s, column_delimiter);
	PlotParam i;

	char *column_values_s [PP_NUM_PARAMS];
///	char *

	/*
	 * column headings are S_SOWING_TITLE_S, delim_s, S_HARVEST_TITLE_S, delim_s, S_WIDTH_TITLE_S, delim_s, S_LENGTH_TITLE_S, delim_s, S_ROW_TITLE_S, delim_s, S_COLUMN_TITLE_S, delim_s,
	 * S_TRIAL_DESIGN_TITLE_S, delim_s, S_GROWING_CONDITION_TITLE_S, delim_s, S_TREATMENT_TITLE_S, delim_s
	 *
	 */
	while (success_flag && (*current_column_s != '\0'))
		{

			const char *sowing_s = NULL;


			if (next_column_s)
				{
					/*
					 * move onto the next column
					 */
					current_column_s = next_column_s + 1;

					if (*current_column_s != '\0')
						{
							next_column_s = strchr (current_column_s, column_delimiter);
						}

				}		/* if (next_column_s) */
			else
				{
					//*current_column_s = '\0';
				}

		}		/* while (success_flag && (*current_column_s != '\0')) */

	return NULL;
}

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
#include "math_utils.h"
#include "material.h"
#include "row.h"


typedef enum
{
	PP_SOWING_DATE,
	PP_HARVEST_DATE,
	PP_WIDTH,
	PP_LENGTH,
	PP_ROW,
	PP_COLUMN,
	PP_RACK_INDEX,
	PP_MATERIAL,
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
static const char * const S_RACK_TITLE_S = "Rack";
static const char * const S_MATERIAL_TITLE_S = "Material";
static const char * const S_TRIAL_DESIGN_TITLE_S = "Trial design";
static const char * const S_GROWING_CONDITION_TITLE_S = "Growing condition";
static const char * const S_TREATMENT_TITLE_S = "Treatment";
static const char * const S_REPLICATE_TITLE_S = "Replicate";

/*
static NamedParameterType S_PLOT_SOWING_DATE = { "PL Sowing Year", PT_TIME };
static NamedParameterType S_PLOT_HARVEST_DATE = { "PL Harvest Year", PT_TIME };
static NamedParameterType S_PLOT_WIDTH = { "PL Width", PT_UNSIGNED_REAL };
static NamedParameterType S_PLOT_LENGTH = { "PL Length", PT_UNSIGNED_REAL };


static NamedParameterType S_PLOT_ROW = { "PL Row", PT_UNSIGNED_INT };
static NamedParameterType S_PLOT_COLUMN = { "PL Column", PT_UNSIGNED_INT };

static NamedParameterType S_PLOT_TRIAL_DESIGN = { "PL Trial Design", PT_STRING };
static NamedParameterType S_PLOT_GROWING_CONDITION = { "PL Growing Condition", PT_STRING };
static NamedParameterType S_PLOT_TREATMENT = { "PL Treatment", PT_STRING };
*/

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

static Parameter *GetTableParameter (ParameterSet *param_set_p, ParameterGroup *group_p, const DFWFieldTrialServiceData *data_p);


static bool GetJSONStringAsInteger (const json_t *json_p, const char * const key_s, int *answer_p);

static bool GetJSONStringAsDouble (const json_t *json_p, const char * const key_s, double *answer_p);




/*
 * API definitions
 */

bool AddSubmissionPlotParams (ServiceData *data_p, ParameterSet *param_set_p)
{
	bool success_flag = false;
	Parameter *param_p = NULL;
	ParameterGroup *group_p = CreateAndAddParameterGroupToParameterSet ("Plots", NULL, false, data_p, param_set_p);
	SharedType def;

	InitSharedType (&def);

	if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_EXPERIMENTAL_AREAS_LIST.npt_type, S_EXPERIMENTAL_AREAS_LIST.npt_name_s, "Experimental Areas", "The available experimental areas", def, PL_ALL)) != NULL)
		{
			const DFWFieldTrialServiceData *dfw_service_data_p = (DFWFieldTrialServiceData *) data_p;

			if (SetUpExperimentalAreasListParameter (dfw_service_data_p, param_p))
				{
					def.st_char_value = S_DEFAULT_COLUMN_DELIMITER;

					if ((param_p = CreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_PLOT_TABLE_COLUMN_DELIMITER.npt_type, false, S_PLOT_TABLE_COLUMN_DELIMITER.npt_name_s, "Delimiter", "The character delimiting columns", NULL, def, NULL, NULL, PL_ADVANCED, NULL)) != NULL)
						{
							def.st_string_value_s = NULL;

							if ((param_p = GetTableParameter (param_set_p, group_p, dfw_service_data_p)) != NULL)
								{
									success_flag = true;
//									SharedType date_def;
//
//									InitSharedType (&date_def);
//
//									if ((date_def.st_time_p = AllocateTime ()) != NULL)
//										{
//											SetDateValuesForTime (date_def.st_time_p, 2017, 1, 1);
//
//											if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_PLOT_SOWING_DATE.npt_type, S_PLOT_SOWING_DATE.npt_name_s, S_SOWING_TITLE_S, "The date when the seeds were sown", date_def, PL_ADVANCED)) != NULL)
//												{
//													if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_PLOT_HARVEST_DATE.npt_type, S_PLOT_HARVEST_DATE.npt_name_s, S_HARVEST_TITLE_S, "The date when the seeds were harvested", date_def, PL_ADVANCED)) != NULL)
//														{
//															InitSharedType (&def);
//
//															def.st_ulong_value = 0;
//
//															if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_PLOT_WIDTH.npt_type, S_PLOT_WIDTH.npt_name_s, S_WIDTH_TITLE_S, "The width of the plot", def, PL_ADVANCED)) != NULL)
//																{
//																	if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_PLOT_LENGTH.npt_type, S_PLOT_LENGTH.npt_name_s, S_LENGTH_TITLE_S, "The length of the plot", def, PL_ADVANCED)) != NULL)
//																		{
//																			if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_PLOT_TRIAL_DESIGN.npt_type, S_PLOT_TRIAL_DESIGN.npt_name_s, S_TRIAL_DESIGN_TITLE_S, "The trial desgin of the plot", def, PL_ADVANCED)) != NULL)
//																				{
//																					if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_PLOT_TREATMENT.npt_type, S_PLOT_TREATMENT.npt_name_s, S_TREATMENT_TITLE_S, "The treatments of the plot", def, PL_ADVANCED)) != NULL)
//																						{
//																							if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_PLOT_GROWING_CONDITION.npt_type, S_PLOT_GROWING_CONDITION.npt_name_s, S_GROWING_CONDITION_TITLE_S, "The growing condtions of the plot", def, PL_ADVANCED)) != NULL)
//																								{
//																									success_flag = true;
//																								}
//																							else
//																								{
//																									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", S_PLOT_GROWING_CONDITION.npt_name_s);
//																								}
//																						}
//																					else
//																						{
//																							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", S_PLOT_TREATMENT.npt_name_s);
//																						}
//																				}
//																			else
//																				{
//																					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", S_PLOT_TRIAL_DESIGN.npt_name_s);
//																				}
//																		}
//																	else
//																		{
//																			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", S_PLOT_LENGTH.npt_name_s);
//																		}
//																}
//															else
//																{
//																	PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", S_PLOT_WIDTH.npt_name_s);
//																}
//														}
//													else
//														{
//															PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", S_PLOT_HARVEST_DATE.npt_name_s);
//														}
//												}
//											else
//												{
//													PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", S_PLOT_SOWING_DATE.npt_name_s);
//												}
//
//											ClearSharedType (&date_def, PT_TIME);
//										}		/* if ((date_def.st_time_p = AllocateTime ()) != NULL) */
//									else
//										{
//											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "AllocateTime failed");
//										}
								}
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "GetTableParameter failed");
								}
						}
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", S_EXPERIMENTAL_AREAS_LIST.npt_name_s);
						}
				}
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "SetUpExperimentalAreasListParameter failed");
				}
		}
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", S_EXPERIMENTAL_AREAS_LIST.npt_name_s);
		}

	return success_flag;
}


bool RunForSubmissionPlotParams (DFWFieldTrialServiceData *data_p, ParameterSet *param_set_p, ServiceJob *job_p)
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

							json_decref (plots_json_p);
						}		/* if (plots_json_p) */
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


static Parameter *GetTableParameter (ParameterSet *param_set_p, ParameterGroup *group_p, const DFWFieldTrialServiceData *data_p)
{
	Parameter *param_p = NULL;
	const char delim_s [2] = { S_DEFAULT_COLUMN_DELIMITER, '\0' };
	char *headers_s = NULL;
	SharedType def;

	InitSharedType (&def);

	headers_s = ConcatenateVarargsStrings (S_SOWING_TITLE_S, delim_s, S_HARVEST_TITLE_S, delim_s, S_WIDTH_TITLE_S, delim_s, S_LENGTH_TITLE_S, delim_s, S_ROW_TITLE_S, delim_s, S_COLUMN_TITLE_S, delim_s,
																				 S_REPLICATE_TITLE_S, delim_s, S_RACK_TITLE_S, delim_s, S_MATERIAL_TITLE_S, delim_s, S_TRIAL_DESIGN_TITLE_S, delim_s, S_GROWING_CONDITION_TITLE_S, delim_s, S_TREATMENT_TITLE_S, delim_s, NULL);


	if (headers_s)
		{
			param_p = CreateAndAddParameterToParameterSet (& (data_p -> dftsd_base_data), param_set_p, group_p, S_PLOT_TABLE.npt_type, false, S_PLOT_TABLE.npt_name_s, "Plot data to upload", "The data to upload", NULL, def, NULL, NULL, PL_ALL, NULL);

			if (param_p)
				{
					bool success_flag = false;

					if (AddParameterKeyValuePair (param_p, PA_TABLE_COLUMN_HEADINGS_S, headers_s))
						{
							if (AddParameterKeyValuePair (param_p, PA_TABLE_COLUMN_DELIMITER_S, delim_s))
								{
									if (AddParameterKeyValuePair (param_p, S_WIDTH_TITLE_S, PA_TYPE_NUMBER_S))
										{
											if (AddParameterKeyValuePair (param_p, S_LENGTH_TITLE_S, PA_TYPE_NUMBER_S))
												{
													if (AddParameterKeyValuePair (param_p, S_ROW_TITLE_S, PA_TYPE_INTEGER_S))
														{
															if (AddParameterKeyValuePair (param_p, S_COLUMN_TITLE_S, PA_TYPE_INTEGER_S))
																{
																	success_flag = true;
																}
														}
												}
										}
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


static bool AddPlotsFromJSON (ServiceJob *job_p, const json_t *plots_json_p, ExperimentalArea *area_p, const DFWFieldTrialServiceData *data_p)
{
	OperationStatus status = OS_FAILED;
	bool success_flag	= true;

	if (json_is_array (plots_json_p))
		{
			const size_t num_rows = json_array_size (plots_json_p);
			size_t i;
			size_t num_imported = 0;

			for (i = 0; i < num_rows; ++ i)
				{
					json_t *table_row_json_p = json_array_get (plots_json_p, i);
					Material *material_p = NULL;

					const char *material_s = GetJSONString (table_row_json_p, S_MATERIAL_TITLE_S);

					if (material_s)
						{
							material_p = GetOrCreateMaterialByInternalName (material_s, area_p, data_p);

							if (material_p)
								{
									Plot *plot_p = NULL;
									int32 row = -1;

									if (GetJSONStringAsInteger (table_row_json_p, S_ROW_TITLE_S, &row))
										{
											int32 column = -1;

											if (GetJSONStringAsInteger (table_row_json_p, S_COLUMN_TITLE_S, &column))
												{
													/*
													 * does the plot already exist?
													 */
													plot_p = GetPlotByRowAndColumn (row, column, area_p, data_p);

													if (!plot_p)
														{
															double width = 0.0;

															if (GetJSONStringAsDouble (table_row_json_p, S_WIDTH_TITLE_S, &width))
																{
																	double length = 0.0;

																	if (GetJSONStringAsDouble (table_row_json_p, S_LENGTH_TITLE_S, &length))
																		{
																			const char *growing_condition_s = GetJSONString (table_row_json_p, S_GROWING_CONDITION_TITLE_S);
																			const char *treatment_s = GetJSONString (table_row_json_p, S_TREATMENT_TITLE_S);
																			const char *trial_design_s = GetJSONString (table_row_json_p, S_TRIAL_DESIGN_TITLE_S);
																			struct tm *sowing_date_p = NULL;
																			struct tm *harvest_date_p = NULL;
																			const char *date_s = GetJSONString (table_row_json_p, S_SOWING_TITLE_S);
																			int32 replicate = 1;

																			GetJSONStringAsInteger (table_row_json_p, S_REPLICATE_TITLE_S, &replicate);

																			if (date_s)
																				{
																					sowing_date_p = GetTimeFromString (date_s);

																					if (!sowing_date_p)
																						{
																							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get sowing date from \"%s\"", date_s);
																						}
																				}

																			date_s = GetJSONString (table_row_json_p, S_HARVEST_TITLE_S);
																			if (date_s)
																				{
																					harvest_date_p = GetTimeFromString (date_s);

																					if (!harvest_date_p)
																						{
																							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get harvest date from \"%s\"", date_s);
																						}
																				}

																			plot_p = AllocatePlot (NULL, sowing_date_p, harvest_date_p, width, length, row, column, replicate, trial_design_s, growing_condition_s, treatment_s, area_p);

																			if (plot_p)
																				{
																					if (!SavePlot (plot_p, data_p))
																						{
																							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, table_row_json_p, "Failed to save plot");
																							plot_p = NULL;
																						}		/* if (!SavePlot (plot_p, data_p)) */

																				}		/* if (plot_p) */
																			else
																				{
																					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, table_row_json_p, "Failed to allocate Plot");
																				}

																		}		/* if (GetJSONStringAsDouble (table_row_json_p, S_LENGTH_TITLE_S, &length)) */
																	else
																		{
																			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, table_row_json_p, "Failed to get \"%s\"", S_LENGTH_TITLE_S);
																		}

																}		/* if (GetJSONStringAsDouble (table_row_json_p, S_WIDTH_TITLE_S, &width) */
															else
																{
																	PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, table_row_json_p, "Failed to get \"%s\"", S_WIDTH_TITLE_S);
																}

														}		/* if (!plot_p) */


													if (plot_p)
														{
															/*
															 * plot_p now has an id, so we can add the row/rack.
															 */
															int32 rack = -1;

															if (GetJSONStringAsInteger (table_row_json_p, S_RACK_TITLE_S, &rack))
																{
																	Row *row_p = AllocateRow (NULL, rack, material_p, plot_p);

																	if (row_p)
																		{
																			if (SaveRow (row_p, data_p, true))
																				{
																					++ num_imported;
																				}
																			else
																				{
																					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, table_row_json_p, "Failed to save row");
																				}

																			FreeRow (row_p);
																		}
																	else
																		{
																			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, table_row_json_p, "Failed to allocate row");
																		}

																}		/* if (GetJSONStringAsInteger (table_row_json_p, S_RACK_TITLE_S, &rack)) */
															else
																{
																	PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, table_row_json_p, "Failed to get \"%s\"", S_RACK_TITLE_S);
																}

															FreePlot (plot_p);
														}		/* if (plot_p) */

												}		/* if (GetJSONStringAsInteger (row_p, S_COLUMN_TITLE_S, &column)) */
											else
												{
													PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, table_row_json_p, "Failed to get \"%s\"", S_COLUMN_TITLE_S);
												}

										}		/* if (GetJSONStringAsInteger (row_p, S_ROW_TITLE_S, &row)) */
									else
										{
											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, table_row_json_p, "Failed to get \"%s\"", S_ROW_TITLE_S);
										}

								}		/* if (material_p) */
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get Material with internal name \"%s\" for area \"%s\"", material_s, area_p -> ea_name_s);
								}

						}		/* if (material_s) */
					else
						{
							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, table_row_json_p, "Failed to get \"%s\"", S_MATERIAL_TITLE_S);
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


static bool GetJSONStringAsInteger (const json_t *json_p, const char * const key_s, int *answer_p)
{
	bool success_flag = false;
	const char *value_s = GetJSONString (json_p, key_s);

	if (value_s)
		{
			success_flag = GetValidInteger (&value_s, answer_p);
		}		/* if (value_s) */

	return success_flag;
}



static bool GetJSONStringAsDouble (const json_t *json_p, const char * const key_s, double *answer_p)
{
	bool success_flag = false;
	const char *value_s = GetJSONString (json_p, key_s);

	if (value_s)
		{
			success_flag = GetValidRealNumber (&value_s, answer_p, NULL);
		}		/* if (value_s) */

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



Plot *GetPlotByRowAndColumn (const uint32 row, const uint32 column, ExperimentalArea *area_p, const DFWFieldTrialServiceData *data_p)
{
	Plot *plot_p = NULL;

	if (SetMongoToolCollection (data_p -> dftsd_mongo_p, data_p -> dftsd_collection_ss [DFTD_PLOT]))
		{
			bson_t *query_p = BCON_NEW (PL_ROW_INDEX_S, BCON_INT32 (row), PL_COLUMN_INDEX_S, BCON_INT32 (column), PL_PARENT_EXPERIMENTAL_AREA_S, BCON_OID (area_p -> ea_id_p));

			if (query_p)
				{
					json_t *results_p = GetAllMongoResultsAsJSON (data_p -> dftsd_mongo_p, query_p, NULL);

					if (results_p)
						{
							if (json_is_array (results_p))
								{
									const size_t num_results = json_array_size (results_p);

									if (num_results == 1)
										{
											size_t i = 0;
											json_t *entry_p = json_array_get (results_p, i);

											plot_p = GetPlotFromJSON (entry_p, area_p, data_p);

											if (!plot_p)
												{

												}

										}		/* if (num_results == 1) */

								}		/* if (json_is_array (results_p)) */

							json_decref (results_p);
						}		/* if (results_p) */

					bson_destroy (query_p);
				}		/* if (query_p) */

		}		/* if (SetMongoToolCollection (data_p -> dftsd_mongo_p, data_p -> dftsd_collection_ss [DFTD_PLOT])) */


	return plot_p;
}

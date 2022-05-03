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

#define ALLOCATE_PLOT_JOB_CONSTANTS (1)
#include "plot_jobs.h"

#include "plot.h"
#include "time_util.h"
#include "string_utils.h"
#include "row_jobs.h"
#include "study_jobs.h"
#include "math_utils.h"
#include "material.h"
#include "row.h"
#include "gene_bank.h"
#include "dfw_util.h"

#include "boolean_parameter.h"
#include "char_parameter.h"
#include "double_parameter.h"
#include "json_parameter.h"
#include "string_parameter.h"
#include "time_parameter.h"
#include "unsigned_int_parameter.h"

#include "frictionless_data_util.h"

#include "treatment_factor.h"

typedef enum
{
	PP_SOWING_DATE,
	PP_HARVEST_DATE,
	PP_WIDTH,
	PP_LENGTH,
	PP_INDEX,
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
#define S_SOWING_DESCRIPTION_S "Sowing date of the plot"


static const char * const S_HARVEST_TITLE_S = "Harvest date";
#define S_HARVEST_DESCRIPTION_S "Harvest date of the plot"

static const char * const S_WIDTH_TITLE_S = "Width";
#define S_WIDTH_DESCRIPTION_S "This is the width, in metres, of each plot"

static const char * const S_LENGTH_TITLE_S = "Length";
#define S_LENGTH_DESCRIPTION_S "This is the length, in metres, of each plot"

static const char * const S_INDEX_DESCRIPTION_S = "The ID of the rack. This is a number given to uniquely identify each rack in the Study similar to a primary key in a database."
		" If GeoJSON and/or images are available, this will be used to identify which plot this information refers to.";


static const char * const S_ROW_TITLE_S = "Row";
static const char * const S_ROW_DESCRIPTION_S = "Row number of the plot. The numbering starts at 1 at the left-hand edge of the plots.";

static const char * const S_COLUMN_TITLE_S = "Column";
static const char * const S_COLUMN_DESCRIPTION_S = "Column number of the plot. The numbering starts at 1 at the bottom-edge of the plots.";



static const char * const S_RACK_TITLE_S = "Rack";
static const char * const S_RACK_DESCRIPTION_S = "Within the plot, this is the number of the cassette that is filled for drilling.";

static const char * const S_ACCESSION_DESCRIPTION_S = "This is the unique identifier from a particular seed/gene bank to identify the material.";

static const char * const S_GENE_BANK_S = "Gene Bank";
static const char * const S_GENE_BANK_DESCRIPTION_S = "";

static const char * const S_TREATMENT_TITLE_S = "Treatment";
static const char * const S_TREATMENT_DESCRIPTION_S = "";

static const char * const S_REPLICATE_TITLE_S = "Replicate";
static const char * const S_REPLICATE_DESCRIPTION_S = "";

static const char * const S_COMMENT_TITLE_S = "Comment";
static const char * const S_COMMENT_DESCRIPTION_S = "Any comments for the plot.";

static const char * const S_IMAGE_TITLE_S = "Image";
static const char * const S_IMAGE_DESCRIPTION_S = "The link for an full-sized image of this plot.";

static const char * const S_THUMBNAIL_TITLE_S = "Thumbnail";
static const char * const S_THUMBNAIL_DESCRIPTION_S = "The link for a thumbnail image of this plot.";

static const char * const S_SOWING_ORDER_TITLE_S = "Sowing order";
static const char * const S_SOWING_ORDER_DESCRIPTION_S = "The order that the plots were sown.";

static const char * const S_WALKING_ORDER_TITLE_S = "Walking order";
static const char * const S_WALKING_ORDER_DESCRIPTION_S = "The order that the plots were walked";


static NamedParameterType S_PLOT_TABLE_COLUMN_DELIMITER = { "PL Data delimiter", PT_CHAR };

static NamedParameterType S_AMEND = { "PL Amend", PT_BOOLEAN};

static NamedParameterType S_STUDIES_LIST = { "PL Study", PT_STRING };


static const char S_DEFAULT_COLUMN_DELIMITER =  '|';


/*
 * static declarations
 */



static bool AddPlotsFromJSON (ServiceJob *job_p, json_t *plots_json_p, Study *study_p,  FieldTrialServiceData *data_p);

static Parameter *GetTableParameter (ParameterSet *param_set_p, ParameterGroup *group_p, Study *active_study_p, FieldTrialServiceData *data_p);

static json_t *GetTableParameterHints (void);

static Plot *GetUniquePlot (bson_t *query_p, Study *study_p, FieldTrialServiceData *data_p);

static json_t *GetPlotTableRow (const Row *row_p, const FieldTrialServiceData *service_data_p);

static json_t *GetStudyPlotsForSubmissionTable (Study *study_p, FieldTrialServiceData *service_data_p);

static bool AddPlotRowsToTable (const Plot *plot_p, json_t *plots_table_p, const FieldTrialServiceData *service_data_p);

static Plot *CreatePlotFromTabularJSON (const json_t *table_row_json_p, const int32 row, const int32 column, Study *study_p, const FieldTrialServiceData *data_p);

static json_t *GetPlotRowTemplate (const uint32 row, const uint32 column, const double64 *width_p, const double64 *height_p);

static json_t *GeneratePlotsTemplate (const Study *study_p);

static bool AddPlotDefaultsFromStudy (Study *study_p, ServiceData *data_p, ParameterSet *param_set_p);

static bool RemoveExistingPlotsForStudy (Study *study_p, const FieldTrialServiceData *data_p);

static json_t *GetPlotsAsFrictionlessData (const Study *study_p, const FieldTrialServiceData *service_data_p, const char * const null_sequence_s);

static OperationStatus AddPlotFromJSON (ServiceJob *job_p, json_t *table_row_json_p, Study *study_p, GeneBank *gru_gene_bank_p, json_t *unknown_cols_p, const uint32 row_index, FieldTrialServiceData *data_p);


/*
 * API definitions
 */

bool AddSubmissionPlotParams (ServiceData *data_p, ParameterSet *param_set_p, Resource *resource_p)
{
	FieldTrialServiceData *dfw_data_p = (FieldTrialServiceData *) data_p;
	bool success_flag = false;
	Parameter *param_p = NULL;
	ParameterGroup *group_p = CreateAndAddParameterGroupToParameterSet ("Plots", false, data_p, param_set_p);
	Study *active_study_p = GetStudyFromResource (resource_p, S_STUDIES_LIST, dfw_data_p);


	if ((param_p = EasyCreateAndAddStringParameterToParameterSet (data_p, param_set_p, group_p, S_STUDIES_LIST.npt_type, S_STUDIES_LIST.npt_name_s, "Study", "The Study that these plots are from", NULL, PL_ALL)) != NULL)
		{
			if (SetUpStudiesListParameter (dfw_data_p, (StringParameter *) param_p, active_study_p, false))
				{
					char c = S_DEFAULT_COLUMN_DELIMITER;

					/*
					 * We want to update all of the values in the form
					 * when a user selects a study from the list so
					 * we need to make the parameter automatically
					 * refresh the values. So we set the
					 * pa_refresh_service_flag to true.
					 */
					param_p -> pa_refresh_service_flag = true;

					if (AddPlotDefaultsFromStudy (active_study_p, data_p, param_set_p))
						{

						}		/* if (AddPlotDefaultsFromStudy (active_study_p, data_p, param_set_p)) */


					if ((param_p = EasyCreateAndAddCharParameterToParameterSet (data_p, param_set_p, group_p, S_PLOT_TABLE_COLUMN_DELIMITER.npt_name_s, "Delimiter", "The character delimiting columns", &c, PL_ADVANCED)) != NULL)
						{

							if ((param_p = GetTableParameter (param_set_p, group_p, active_study_p, dfw_data_p)) != NULL)
								{
									bool append_flag = false;

									if ((param_p = EasyCreateAndAddBooleanParameterToParameterSet (data_p, param_set_p, group_p, S_AMEND.npt_name_s, "Append to existing plot data", "Append these plots to the already existing ones rather than removing the existing entries upon submission", &append_flag, PL_ALL)) != NULL)
										{
											success_flag = true;
										}
									else
										{
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", S_AMEND.npt_name_s);
										}
								}
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "GetTableParameter failed");
								}
						}
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", S_PLOT_TABLE_COLUMN_DELIMITER.npt_name_s);
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

	if (active_study_p)
		{
			FreeStudy (active_study_p);
		}

	return success_flag;
}


bool RunForSubmissionPlotParams (FieldTrialServiceData *data_p, ParameterSet *param_set_p, ServiceJob *job_p)
{
	bool job_done_flag = false;
	const char *study_id_s = NULL;

	if (GetCurrentStringParameterValueFromParameterSet (param_set_p, S_STUDIES_LIST.npt_name_s, &study_id_s))
		{
			Study *study_p = GetStudyByIdString (study_id_s, VF_STORAGE, data_p);

			if (study_p)
				{
					json_t *plots_table_p = NULL;

					if (GetCurrentJSONParameterValueFromParameterSet (param_set_p, PL_PLOT_TABLE.npt_name_s, (const json_t **) &plots_table_p))
						{
							job_done_flag = true;

							/*
							 * Has a spreadsheet been uploaded?
							 */
							if (plots_table_p)
								{
									const size_t num_rows = json_array_size (plots_table_p);

									if (num_rows > 0)
										{
											const bool *append_flag_p = NULL;
											bool success_flag = true;

											GetCurrentBooleanParameterValueFromParameterSet (param_set_p, S_AMEND.npt_name_s, &append_flag_p);

											if (!append_flag_p || (! (*append_flag_p)))
												{
													if (!RemoveExistingPlotsForStudy (study_p, data_p))
														{
															success_flag = false;
															PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to remove existing plots for study \"%s\"", study_id_s);
														}
												}

											if (success_flag)
												{
													if (!AddPlotsFromJSON (job_p, plots_table_p, study_p, data_p))
														{
															PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, plots_table_p, "AddPlotsFromJSON failed for study \"%s\"", study_p -> st_name_s);
														}
												}

										}		/* if (num_rows > 0) */
									else
										{
											PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Empty JSON for uploaded plots for study \"%s\"", study_id_s);
										}

								}		/* if (table_value.st_json_p) */
							else
								{
									PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "NULL JSON for uploaded plots for study \"%s\"", study_id_s);
								}

						}		/* if (GetCurrentParameterValueFromParameterSet (param_set_p, S_PLOT_TABLE.npt_name_s, &table_value)) */
					else
						{
							PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to get param \%s\"", PL_PLOT_TABLE.npt_name_s);
						}

					FreeStudy (study_p);
				}		/* if (study_p) */
			else
				{
					PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to get parent study for plots with id \%s\"", study_id_s);
				}

		}		/* if (GetCurrentParameterValueFromParameterSet (param_set_p, S_STUDIES_LIST.npt_name_s, &parent_study_value)) */
	else
		{
			PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to get param \%s\"", S_STUDIES_LIST.npt_name_s);
		}

	return job_done_flag;
}



bool GetSubmissionPlotParameterTypeForNamedParameter (const char *param_name_s, ParameterType *pt_p)
{
	bool success_flag = true;

	if (strcmp (param_name_s, S_STUDIES_LIST.npt_name_s) == 0)
		{
			*pt_p = S_STUDIES_LIST.npt_type;
		}
	else if (strcmp (param_name_s, S_PLOT_TABLE_COLUMN_DELIMITER.npt_name_s) == 0)
		{
			*pt_p = S_PLOT_TABLE_COLUMN_DELIMITER.npt_type;
		}
	else if (strcmp (param_name_s, PL_PLOT_TABLE.npt_name_s) == 0)
		{
			*pt_p = PL_PLOT_TABLE.npt_type;
		}
	else if (strcmp (param_name_s, S_AMEND.npt_name_s) == 0)
		{
			*pt_p = S_AMEND.npt_type;
		}
	else
		{
			success_flag = GetSubmissionStudyParameterTypeForDefaultPlotNamedParameter (param_name_s, pt_p);
		}

	return success_flag;
}


json_t *GetPlotsAsFDTabularPackage (const Study *study_p, const FieldTrialServiceData *service_data_p)
{
	json_t *plots_p = json_object ();

	if (plots_p)
		{
			if (SetJSONString (plots_p, FD_PROFILE_S, FD_PROFILE_TABULAR_RESOURCE_S))
				{
					json_t *schema_p = GetPlotsFrictionlessDataTableSchema (study_p, service_data_p);

					if (schema_p)
						{
							if (json_object_set_new (plots_p, FD_SCHEMA_S, schema_p) == 0)
								{
									const char *null_sequence_s = "-";
									json_t *data_p = GetPlotsAsFrictionlessData (study_p, service_data_p, null_sequence_s);

									if (data_p)
										{
											if (json_object_set_new (plots_p, FD_DATA_S, data_p) == 0)
												{
													json_t *dialect_p = GetPlotsCSVDialect (null_sequence_s);

													if (dialect_p)
														{
															if (json_object_set_new (plots_p, FD_CSV_DIALECT, dialect_p) == 0)
																{
																	return plots_p;
																}		/* if (json_object_set_new (plots_p, "dialect", dialect_p) == 0) */
															else
																{
																	json_decref (dialect_p);
																}

														}		/* if (dialect_p) */
												}
											else
												{
													json_decref (data_p);
												}
										}

								}		/* if (json_object_set_new (plots_p, "schema", schema_p) == 0) */
							else
								{
									json_decref (schema_p);
								}

						}		/* if (schema_p) */

				}		/* if (SetJSONString (plots_p, FD_PROFILE_S, FD_PROFILE_TABULAR_RESOURCE_S)) */


			json_decref (plots_p);
		}		/* if (plots_p) */

	return NULL;
}


json_t *GetPlotsCSVDialect (const char *null_sequence_s)
{
	json_t *dialect_p = GetCSVDialect (NULL, NULL, NULL, NULL, null_sequence_s, false, false, true, true, NULL, 0, 1);


	return dialect_p;
}


static json_t *GetPlotsAsFrictionlessData (const Study *study_p, const FieldTrialServiceData *service_data_p, const char * const null_sequence_s)
{
	if (study_p -> st_plots_p)
		{
			json_t *plots_p = json_array ();

			if (plots_p)
				{
					bool success_flag = true;
					PlotNode *node_p = (PlotNode *) (study_p -> st_plots_p -> ll_head_p);

					while (node_p && success_flag)
						{
							if (AddPlotAsFrictionlessData (node_p -> pn_plot_p, plots_p, study_p, service_data_p, null_sequence_s))
								{
									node_p = (PlotNode *) (node_p -> pn_node.ln_next_p);
								}
							else
								{
									success_flag = false;
								}
						}

					if (success_flag)
						{
							return plots_p;
						}

					json_decref (plots_p);
				}



		}		/* if (study_p -> st_plots_p) */

	return NULL;
}



bool AddPlotAsFrictionlessData (const Plot *plot_p, json_t *plots_array_p, const Study * const study_p, const FieldTrialServiceData *service_data_p, const char * const null_sequence_s)
{
	uint32 num_added = 0;
	uint32 num_rows = 0;

	if (plot_p -> pl_rows_p)
		{
			bool success_flag = true;
			RowNode *row_node_p = (RowNode *) (plot_p -> pl_rows_p -> ll_head_p);
			num_rows = plot_p -> pl_rows_p -> ll_size;

			while (row_node_p && success_flag)
				{
					json_t *row_fd_p = json_object ();

					if (row_fd_p)
						{
							success_flag = false;

							if (SetJSONInteger (row_fd_p, S_ROW_TITLE_S, plot_p -> pl_row_index))
								{
									if (SetJSONInteger (row_fd_p, S_COLUMN_TITLE_S, plot_p -> pl_column_index))
										{
											if (SetFDTableReal (row_fd_p, S_LENGTH_TITLE_S, plot_p -> pl_length_p, null_sequence_s))
												{
													if (SetFDTableReal (row_fd_p, S_WIDTH_TITLE_S, plot_p -> pl_width_p, null_sequence_s))
														{
															if (AddValidDateToJSON (plot_p -> pl_sowing_date_p, row_fd_p, S_SOWING_TITLE_S, false))
																{
																	if (AddValidDateToJSON (plot_p -> pl_harvest_date_p, row_fd_p, S_HARVEST_TITLE_S, false))
																		{
																			if (AddRowFrictionlessDataDetails (row_node_p -> rn_row_p, row_fd_p, service_data_p, null_sequence_s))
																				{
																					if (json_array_append_new (plots_array_p, row_fd_p) == 0)
																						{
																							++ num_added;

																							row_node_p = (RowNode *) (row_node_p -> rn_node.ln_next_p);
																							success_flag = true;
																						}

																				}

																		}

																}
														}
												}
										}
								}

							if (!success_flag)
								{
									json_decref (row_fd_p);
								}
						}


				}		/* while (row_node_p) */


		}		/* if (plot_p -> pl_rows_p) */


	return (num_added == num_rows);
}



json_t *GetStudyPlotHeaderAsFrictionlessData (const Study *study_p, const FieldTrialServiceData *service_data_p)
{
	json_t *fields_p = json_array ();

	if (fields_p)
		{
			int min_int = 1;

			json_t *field_p = AddIntegerField (fields_p, PL_INDEX_TABLE_TITLE_S, PL_INDEX_TABLE_TITLE_S, NULL, S_INDEX_DESCRIPTION_S, NULL, &min_int);

			if (field_p)
				{
					/*
					 * The Plot Id starts from 1, is unique and is required
					 */
					if ((SetTableFieldUnique (field_p)) && (SetTableFieldRequired (field_p)))
						{
							if (AddTableField (fields_p, S_SOWING_TITLE_S, S_SOWING_TITLE_S, FD_TYPE_DATE, NULL, S_SOWING_DESCRIPTION_S, NULL))
								{
									if (AddTableField (fields_p, S_HARVEST_TITLE_S, S_HARVEST_TITLE_S, FD_TYPE_DATE, NULL, S_HARVEST_DESCRIPTION_S, NULL))
										{
											double min_num = 0.0f;

											if (AddNumberField (fields_p, S_WIDTH_TITLE_S, S_WIDTH_TITLE_S, NULL, S_WIDTH_DESCRIPTION_S, NULL, &min_num))
												{
													if (AddNumberField (fields_p, S_LENGTH_TITLE_S, S_LENGTH_TITLE_S, NULL, S_LENGTH_DESCRIPTION_S, NULL, &min_num))
														{
															if (AddIntegerField (fields_p, S_ROW_TITLE_S, S_ROW_TITLE_S, NULL, S_ROW_DESCRIPTION_S, NULL, &min_int))
																{
																	if (AddIntegerField (fields_p, S_COLUMN_TITLE_S, S_COLUMN_TITLE_S, NULL, S_COLUMN_DESCRIPTION_S, NULL, &min_int))
																		{
																			if (AddIntegerField (fields_p, S_RACK_TITLE_S, S_RACK_TITLE_S, NULL, S_RACK_DESCRIPTION_S, NULL, &min_int))
																				{
																					if (AddTableField (fields_p, PL_ACCESSION_TABLE_TITLE_S, PL_ACCESSION_TABLE_TITLE_S, FD_TYPE_STRING, NULL, S_ACCESSION_DESCRIPTION_S, NULL))
																						{
																							if (AddTableField (fields_p, PL_REPLICATE_TITLE_S, PL_REPLICATE_TITLE_S, FD_TYPE_INTEGER, NULL, S_REPLICATE_DESCRIPTION_S, NULL))
																								{
																									if (AddTableField (fields_p, S_COMMENT_TITLE_S, S_COMMENT_DESCRIPTION_S, FD_TYPE_STRING, NULL, S_COMMENT_DESCRIPTION_S, NULL))
																										{
																											if (AddTableField (fields_p, S_IMAGE_TITLE_S, S_IMAGE_DESCRIPTION_S, FD_TYPE_STRING, NULL, S_IMAGE_DESCRIPTION_S, FD_TYPE_STRING_FORMAT_URI))
																												{
																													if (AddTableField (fields_p, S_THUMBNAIL_TITLE_S, S_THUMBNAIL_DESCRIPTION_S, FD_TYPE_STRING, NULL, S_THUMBNAIL_DESCRIPTION_S, FD_TYPE_STRING_FORMAT_URI))
																														{
																															if (AddIntegerField (fields_p, S_SOWING_ORDER_TITLE_S, S_SOWING_ORDER_DESCRIPTION_S, NULL, S_SOWING_ORDER_DESCRIPTION_S, NULL, &min_int))
																																{
																																	if (AddIntegerField (fields_p, S_WALKING_ORDER_TITLE_S, S_WALKING_ORDER_DESCRIPTION_S, NULL, S_WALKING_ORDER_DESCRIPTION_S, NULL, &min_int))
																																		{
																																			bool b = true;

																																			if (study_p -> st_treatments_p)
																																				{
																																					TreatmentFactorNode *node_p = (TreatmentFactorNode *) (study_p -> st_treatments_p -> ll_head_p);

																																					while (node_p && b)
																																						{
																																							TreatmentFactor *tf_p = node_p -> tfn_p;
																																							const char *name_s = GetTreatmentFactorName (tf_p);

																																							if (name_s)
																																								{
																																									const char *url_s = GetTreatmentFactorUrl (tf_p);

																																									if (url_s)
																																										{
																																											const char *description_s = GetTreatmentFactorDescription (tf_p);

																																											if (AddTableField (fields_p, url_s ? url_s : name_s, name_s, FD_TYPE_STRING, NULL, description_s, NULL))
																																												{
																																													node_p = (TreatmentFactorNode *) (node_p -> tfn_node.ln_next_p);
																																												}
																																											else
																																												{
																																													b = false;
																																												}
																																										}
																																								}
																																							else
																																								{
																																									b = false;
																																								}

																																						}
																																				}

																																			if (b)
																																				{
																																					bool added_flag = true;
																																					json_t *study_phenotypes_p = GetStudyDistinctPhenotypesAsFrictionlessDataJSON (study_p -> st_id_p, service_data_p);

																																					if (study_phenotypes_p)
																																						{
																																							size_t i;
																																							const size_t num_phenotypes = json_array_size (study_phenotypes_p);

																																							for (i = 0; i < num_phenotypes; ++ i)
																																								{
																																									json_t *phenotype_p = json_array_get (study_phenotypes_p, i);

																																									if (json_array_append (fields_p, phenotype_p) != 0)
																																										{
																																											added_flag = false;

																																											/* force exit from loop */
																																											i = num_phenotypes;
																																										}
																																								}

																																							json_array_clear (study_phenotypes_p);
																																							json_decref (study_phenotypes_p);
																																						}		/* if (study_phenotypes_p) */

																																					if (added_flag)
																																						{
																																							return fields_p;
																																						}
																																					else
																																						{
																																							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add Phenotypes");
																																						}

																																				}
																																			else
																																				{
																																					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add TreatmentFactors");
																																				}

																																		}
																																	else
																																		{
																																			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, fields_p, "Failed to add %s field", S_WALKING_ORDER_TITLE_S);
																																		}
																																}
																															else
																																{
																																	PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, fields_p, "Failed to add %s field", S_SOWING_ORDER_TITLE_S);
																																}
																														}
																													else
																														{
																															PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, fields_p, "Failed to add %s field", S_THUMBNAIL_TITLE_S);
																														}
																												}
																											else
																												{
																													PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, fields_p, "Failed to add %s field", S_IMAGE_TITLE_S);
																												}
																										}
																									else
																										{
																											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, fields_p, "Failed to add %s field", S_COMMENT_TITLE_S);
																										}
																								}
																							else
																								{
																									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, fields_p, "Failed to add %s field", PL_REPLICATE_TITLE_S);
																								}
																						}
																					else
																						{
																							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, fields_p, "Failed to add %s field", PL_ACCESSION_TABLE_TITLE_S);
																						}
																				}
																			else
																				{
																					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, fields_p, "Failed to add %s field", S_RACK_TITLE_S);
																				}


																		}		/* if (field_p) */
																	else
																		{
																			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, fields_p, "Failed to add %s field", S_COLUMN_TITLE_S);
																		}

																}
															else
																{
																	PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, fields_p, "Failed to add %s field", S_ROW_TITLE_S);
																}


														}
													else
														{
															PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, fields_p, "Failed to add %s field", S_LENGTH_TITLE_S);
														}



												}
											else
												{
													PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, fields_p, "Failed to add %s field", S_WIDTH_TITLE_S);
												}
										}
									else
										{
											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, fields_p, "Failed to add %s field", S_HARVEST_TITLE_S);
										}
								}
							else
								{
									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, fields_p, "Failed to add %s field", S_SOWING_TITLE_S);
								}

						}		/* if ((SetTableFieldMinimumInteger (field_p, 1)) && (SetTableFieldUnique (field_p)) && (SetTableFieldRequired (field_p))) */


				}		/* if (AddTableField (fields_p, S_INDEX_TITLE_S, S_INDEX_TITLE_S, FD_TYPE_INTEGER, NULL, S_INDEX_DESCRIPTION_S, NULL)) */
			else
				{
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, fields_p, "Failed to add %s field", PL_INDEX_TABLE_TITLE_S);
				}

			json_decref (fields_p);
		}		/* if (fields_p) */

	return NULL;
}



json_t *GetPlotsFrictionlessDataTableSchema (const Study *study_p, const FieldTrialServiceData *service_data_p)
{
	json_t *schema_p = json_object ();

	if (schema_p)
		{
			json_t *fields_p = GetStudyPlotHeaderAsFrictionlessData (study_p, service_data_p);

			if (fields_p)
				{
					if (json_object_set_new (schema_p, FD_TABLE_FIELDS_S, fields_p) == 0)
						{
							bool added_flag = false;
							char *title_s = ConcatenateVarargsStrings (study_p -> st_name_s, " - Plots", NULL);

							if (title_s)
								{
									added_flag = SetJSONString (schema_p, "title", title_s);

									FreeCopiedString (title_s);
								}
							else
								{
									added_flag = SetJSONString (schema_p, "title", "Plots");
								}

							if (added_flag)
								{
									return schema_p;
								}
						}
					else
						{
							json_decref (fields_p);
						}

				}

			json_decref (schema_p);
		}

	return NULL;
}



/*
 * static definitions
 */


static json_t *GetTableParameterHints (void)
{
	/*
	headers_s = ConcatenateVarargsStrings (S_SOWING_TITLE_S, delim_s, S_HARVEST_TITLE_S, delim_s, S_WIDTH_TITLE_S, delim_s, S_LENGTH_TITLE_S, delim_s, S_ROW_TITLE_S, delim_s, S_COLUMN_TITLE_S, delim_s,
																				 PL_REPLICATE_TITLE_S, delim_s, S_RACK_TITLE_S, delim_s, S_MATERIAL_TITLE_S, delim_s, S_TRIAL_DESIGN_TITLE_S, delim_s, S_GROWING_CONDITION_TITLE_S, delim_s, S_TREATMENT_TITLE_S, delim_s, NULL);
	 */
	json_t *hints_p = json_array ();

	if (hints_p)
		{
			if (AddColumnParameterHint (S_SOWING_TITLE_S, S_SOWING_DESCRIPTION_S ". If left blank, then the *Sowing date* specified for the Study will be used.", PT_TIME, false, hints_p))
				{
					if (AddColumnParameterHint (S_HARVEST_TITLE_S, "Harvest date of the plot. If this is blank, then the *Harvest date* specified for the Study will be used.", PT_TIME, false, hints_p))
						{
							if (AddColumnParameterHint (S_WIDTH_TITLE_S, S_WIDTH_DESCRIPTION_S ". If this is blank, then the *Plot width* specified for the Study will be used.", PT_UNSIGNED_REAL, false, hints_p))
								{
									if (AddColumnParameterHint (S_LENGTH_TITLE_S, S_LENGTH_DESCRIPTION_S ". If this is blank, then the *Plot height* specified for the Study will be used.", PT_UNSIGNED_REAL, false, hints_p))
										{
											if (AddColumnParameterHint (PL_INDEX_TABLE_TITLE_S, S_INDEX_DESCRIPTION_S, PT_UNSIGNED_INT, true, hints_p))
												{
													if (AddColumnParameterHint (S_ROW_TITLE_S, S_ROW_DESCRIPTION_S, PT_UNSIGNED_INT, true, hints_p))
														{
															if (AddColumnParameterHint (S_COLUMN_TITLE_S, S_COLUMN_DESCRIPTION_S, PT_UNSIGNED_INT, true, hints_p))
																{
																	if (AddColumnParameterHint (PL_REPLICATE_TITLE_S, S_REPLICATE_DESCRIPTION_S, PT_UNSIGNED_INT, false, hints_p))
																		{
																			if (AddColumnParameterHint (S_RACK_TITLE_S, S_RACK_DESCRIPTION_S, PT_UNSIGNED_INT, true, hints_p))
																				{
																					if (AddColumnParameterHint (PL_ACCESSION_TABLE_TITLE_S, S_ACCESSION_DESCRIPTION_S, PT_STRING, true, hints_p))
																						{
																							if (AddColumnParameterHint (S_COMMENT_TITLE_S, S_COMMENT_DESCRIPTION_S, PT_STRING, false, hints_p))
																								{
																									if (AddColumnParameterHint (S_IMAGE_TITLE_S, S_IMAGE_DESCRIPTION_S, PT_STRING, false, hints_p))
																										{
																											if (AddColumnParameterHint (S_THUMBNAIL_TITLE_S, S_THUMBNAIL_DESCRIPTION_S, PT_STRING, false, hints_p))
																												{
																													if (AddColumnParameterHint (S_SOWING_ORDER_TITLE_S, S_SOWING_ORDER_DESCRIPTION_S, PT_UNSIGNED_INT, false, hints_p))
																														{
																															if (AddColumnParameterHint (S_WALKING_ORDER_TITLE_S, S_WALKING_ORDER_DESCRIPTION_S, PT_UNSIGNED_INT, false, hints_p))
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
						}
				}

			json_decref (hints_p);
		}

	return NULL;
}



static Parameter *GetTableParameter (ParameterSet *param_set_p, ParameterGroup *group_p, Study *active_study_p, FieldTrialServiceData *data_p)
{
	Parameter *param_p = NULL;
	const char delim_s [2] = { S_DEFAULT_COLUMN_DELIMITER, '\0' };
	bool success_flag = false;
	json_t *hints_p = GetTableParameterHints ();

	if (hints_p)
		{
			json_t *plots_json_p = NULL;

			if (active_study_p)
				{
					plots_json_p = GetStudyPlotsForSubmissionTable (active_study_p, data_p);

					if (plots_json_p)
						{
							success_flag = true;
						}
					else
						{
							/*
							 * Are there default values for the study
							 */
						}
				}
			else
				{
					success_flag = true;
				}

			if (success_flag)
				{
					param_p = EasyCreateAndAddJSONParameterToParameterSet (& (data_p -> dftsd_base_data), param_set_p, group_p, PL_PLOT_TABLE.npt_type, PL_PLOT_TABLE.npt_name_s, "Plot data to upload", "The data to upload", plots_json_p, PL_ALL);

					if (param_p)
						{
							success_flag = false;

							if (AddParameterKeyJSONValuePair (param_p, PA_TABLE_COLUMN_HEADINGS_S, hints_p))
								{
									if (AddParameterKeyStringValuePair (param_p, PA_TABLE_COLUMN_DELIMITER_S, delim_s))
										{
											if (AddParameterKeyStringValuePair (param_p, PA_TABLE_ADD_COLUMNS_FLAG_S, "true"))
												{
													success_flag = true;
												}
										}
								}

							if (!success_flag)
								{
									FreeParameter (param_p);
									param_p = NULL;
								}

						}		/* if (param_p) */

				}		/* if (success_flag) */

			if (plots_json_p)
				{
					json_decref (plots_json_p);
				}

			json_decref (hints_p);
		}		/* if (hints_p) */


	return param_p;
}


static OperationStatus AddPlotFromJSON (ServiceJob *job_p, json_t *table_row_json_p, Study *study_p, GeneBank *gru_gene_bank_p, json_t *unknown_cols_p, const uint32 row_index, FieldTrialServiceData *data_p)
{
	const char *gene_bank_s = GetJSONString (table_row_json_p, S_GENE_BANK_S);
	GeneBank *gene_bank_p = NULL;
	OperationStatus add_status = OS_FAILED;

	if (!IsStringEmpty (gene_bank_s))
		{
			/* default to using the GRU */
			gene_bank_p = GetGeneBankByName (gene_bank_s, data_p);
		}
	else
		{
			gene_bank_p = gru_gene_bank_p;
		}

	if (gene_bank_p)
		{
			const char *accession_s = GetJSONString (table_row_json_p, PL_ACCESSION_TABLE_TITLE_S);

			if (!IsStringEmpty (accession_s))
				{
					Material *material_p = GetOrCreateMaterialByAccession (accession_s, gene_bank_p, data_p);

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
											plot_p = GetPlotByRowAndColumn (row, column, study_p, data_p);

											if (!plot_p)
												{
													plot_p = CreatePlotFromTabularJSON (table_row_json_p, row, column, study_p, data_p);

													if (!plot_p)
														{

														}		/* if (!plot_p) */

												}		/* if (!plot_p) */


											if (plot_p)
												{
													/*
													 * plot_p now has an id, so we can add the row/rack.
													 */
													int32 rack_plotwise_index = -1;

													if (GetJSONStringAsInteger (table_row_json_p, S_RACK_TITLE_S, &rack_plotwise_index))
														{
															int32 rack_studywise_index = -1;

															if (GetJSONStringAsInteger (table_row_json_p, PL_INDEX_TABLE_TITLE_S, &rack_studywise_index))
																{
																	int32 replicate = 1;
																	const char *rep_s = GetJSONString (table_row_json_p, PL_REPLICATE_TITLE_S);
																	bool control_rep_flag = false;
																	bool success_flag = true;

																	if (!IsStringEmpty (rep_s))
																		{
																			if (Stricmp (rep_s, RO_REPLICATE_CONTROL_S) == 0)
																				{
																					control_rep_flag = true;
																				}
																			else
																				{
																					success_flag = GetValidInteger (&rep_s, &replicate);
																				}		/* if (value_s) */
																		}

																	if (success_flag)
																		{
																			Row *row_p = GetRowFromPlotByStudyIndex (plot_p, rack_studywise_index);
																			bool is_existing_row_flag = true;
																			const MEM_FLAG material_mem = MF_SHALLOW_COPY;

																			if (row_p)
																				{
																					/*
																					 * update existing row
																					 */
																					UpdateRow (row_p, rack_plotwise_index, material_p, material_mem, control_rep_flag, replicate);
																				}
																			else
																				{
																					row_p = AllocateRow (NULL, rack_plotwise_index, rack_studywise_index, replicate, material_p, material_mem, plot_p);
																					is_existing_row_flag = false;
																				}

																			if (row_p)
																				{
																					const char *key_s;
																					json_t *value_p;
																					size_t num_columns;
																					size_t imported_columns = 0;

																					/*
																					 * Remove any of the normal plot keys
																					 */
																					json_object_del (table_row_json_p, S_SOWING_TITLE_S);
																					json_object_del (table_row_json_p, S_HARVEST_TITLE_S);
																					json_object_del (table_row_json_p, S_WIDTH_TITLE_S);
																					json_object_del (table_row_json_p, S_LENGTH_TITLE_S);
																					json_object_del (table_row_json_p, PL_INDEX_TABLE_TITLE_S);
																					json_object_del (table_row_json_p, S_ROW_TITLE_S);
																					json_object_del (table_row_json_p, S_COLUMN_TITLE_S);
																					json_object_del (table_row_json_p, S_RACK_TITLE_S);
																					json_object_del (table_row_json_p, PL_ACCESSION_TABLE_TITLE_S);
																					json_object_del (table_row_json_p, S_GENE_BANK_S);
																					json_object_del (table_row_json_p, S_TREATMENT_TITLE_S);
																					json_object_del (table_row_json_p, PL_REPLICATE_TITLE_S);
																					json_object_del (table_row_json_p, S_COMMENT_TITLE_S);
																					json_object_del (table_row_json_p, S_IMAGE_TITLE_S);
																					json_object_del (table_row_json_p, S_THUMBNAIL_TITLE_S);
																					json_object_del (table_row_json_p, S_SOWING_ORDER_TITLE_S);
																					json_object_del (table_row_json_p, S_WALKING_ORDER_TITLE_S);

																					/*
																					 * Remove any columns that we have previously seen that we don't recognise
																					 */
																					json_object_foreach (unknown_cols_p, key_s, value_p)
																						{
																							json_object_del (table_row_json_p, key_s);
																						}

																					/*
																					 * If there are any columns left, try to add them as observations
																					 */
																					if ((num_columns = json_object_size (table_row_json_p)) > 0)
																						{
																							OperationStatus status = OS_FAILED;

																							bool loop_success_flag = true;
																							void *iterator_p = json_object_iter (table_row_json_p);

																							while (iterator_p && loop_success_flag)
																								{
																									bool observation_key_flag = false;

																									key_s = json_object_iter_key (iterator_p);

																									/*
																									 * ignore our column names
																									 */
																									if ((strcmp (key_s, RO_IMPORT_RACK_S) != 0) && (strcmp (key_s, RO_PLOT_INDEX_S) != 0))
																										{
																											const char *value_s = NULL;

																											value_p = json_object_iter_value (iterator_p);

																											/*
																											 * Is it an observation?
																											 */
																											status = AddObservationValueToRow (row_p, key_s, value_p, study_p, job_p, row_index, data_p);

																											if (status == OS_SUCCEEDED)
																												{
																													++ imported_columns;
																												}
																											else if (status == OS_IDLE)
																												{
																													value_s = json_string_value (value_p);

																													if (!IsStringEmpty (value_s))
																														{

																														}

																													status = AddSingleTreatmentFactorValueToRow (row_p, key_s, value_s, study_p, job_p, row_index, data_p);

																													if (status == OS_SUCCEEDED)
																														{
																															++ imported_columns;
																														}
																													else if (status == OS_IDLE)
																														{
																															char *error_s = ConcatenateVarargsStrings ("Unknown column name \"", key_s, "\"", NULL);

																															/*
																															 * unknown column
																															 */
																															if (!SetJSONNull (unknown_cols_p, key_s))
																																{
																																	PrintJSONToErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, unknown_cols_p, "Failed to add unknown column \"%s\"", key_s);
																																}

																															if (error_s)
																																{
																																	AddParameterErrorMessageToServiceJob (job_p,PL_PLOT_TABLE.npt_name_s, PL_PLOT_TABLE.npt_type, error_s);
																																	FreeCopiedString (error_s);
																																}
																															else
																																{
																																	AddParameterErrorMessageToServiceJob (job_p,PL_PLOT_TABLE.npt_name_s, PL_PLOT_TABLE.npt_type, "Unknown column name");
																																}

																														}

																												}		/* if (status == OS_IDLE) */

																										}		/* if ((strcmp (key_s, RO_IMPORT_RACK_S) != 0) && (strcmp (key_s, RO_PLOT_INDEX_S) != 0)) */

																									iterator_p = json_object_iter_next (table_row_json_p, iterator_p);

																								}		/* while (iterator_p && loop_success_flag) */
//
//																							OperationStatus obs_status = AddObservationValuesToRow (row_p, table_row_json_p, study_p, unknown_cols_p, job_p, i, data_p);
//																							OperationStatus tr_status = AddTreatmentFactorValuesToRow (row_p, table_row_json_p, study_p, unknown_cols_p, data_p);/


																							if (status != OS_SUCCEEDED)
																								{

																								}

																						}		/* if (json_object_size (table_row_json_p) > 0) */



																					if (control_rep_flag)
																						{
																							SetRowGenotypeControl (row_p, true);
																						}

																					if (is_existing_row_flag || (AddRowToPlot (plot_p, row_p)))
																						{
																							if (SavePlot (plot_p, data_p))
																								{
																									if (num_columns == imported_columns)
																										{
																											add_status = OS_SUCCEEDED;
																										}
																									else if (imported_columns > 0)
																										{
																											add_status = OS_PARTIALLY_SUCCEEDED;
																										}

																								}
																							else
																								{
																									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, table_row_json_p, "Failed to save row");
																								}

																						}
																					else
																						{
																							FreeRow (row_p);
																						}



																				}
																			else
																				{
																					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, table_row_json_p, "Failed to allocate row");
																				}

																		}		/* if (success_flag) */

																}		/* if (GetJSONStringAsInteger (table_row_json_p, S_INDEX_TITLE_S, &study_index)) */
															else
																{
																	AddTabularParameterErrorMessageToServiceJob (job_p, PL_PLOT_TABLE.npt_name_s, PL_PLOT_TABLE.npt_type, "Value not set", row_index, PL_INDEX_TABLE_TITLE_S);
																	PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, table_row_json_p, "Failed to get \"%s\"", PL_INDEX_TABLE_TITLE_S);
																}

														}		/* if (GetJSONStringAsInteger (table_row_json_p, S_RACK_TITLE_S, &rack)) */
													else
														{
															AddTabularParameterErrorMessageToServiceJob (job_p, PL_PLOT_TABLE.npt_name_s, PL_PLOT_TABLE.npt_type, "Value not set", row_index, S_RACK_TITLE_S);
															PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, table_row_json_p, "Failed to get \"%s\"", S_RACK_TITLE_S);
														}

													FreePlot (plot_p);
												}		/* if (plot_p) */

										}		/* if (GetJSONStringAsInteger (row_p, S_COLUMN_TITLE_S, &column)) */
									else
										{
											AddTabularParameterErrorMessageToServiceJob (job_p, PL_PLOT_TABLE.npt_name_s, PL_PLOT_TABLE.npt_type, "Value not set", row_index, S_COLUMN_TITLE_S);
											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, table_row_json_p, "Failed to get \"%s\"", S_COLUMN_TITLE_S);
										}

								}		/* if (GetJSONStringAsInteger (row_p, S_ROW_TITLE_S, &row)) */
							else
								{
									AddTabularParameterErrorMessageToServiceJob (job_p, PL_PLOT_TABLE.npt_name_s, PL_PLOT_TABLE.npt_type, "Value not set", row_index, S_ROW_TITLE_S);
									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, table_row_json_p, "Failed to get \"%s\"", S_ROW_TITLE_S);
								}


						}		/* if (material_p) */
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get Material with internal name \"%s\" for area \"%s\"", accession_s, study_p -> st_name_s);
						}

				}		/* if (accession_s) */
			else
				{
					AddTabularParameterErrorMessageToServiceJob (job_p, PL_PLOT_TABLE.npt_name_s, PL_PLOT_TABLE.npt_type, "Value not set", row_index, PL_ACCESSION_TABLE_TITLE_S);
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, table_row_json_p, "Failed to get \"%s\"", PL_ACCESSION_TABLE_TITLE_S);
				}

			if (gene_bank_p != gru_gene_bank_p)
				{
					FreeGeneBank (gene_bank_p);
				}

		}		/* if (gene_bank_p) */
	else
		{
			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, table_row_json_p, "Failed to get gene bank \%s\"", gene_bank_s);
		}

	if (add_status == OS_FAILED)
		{
			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, table_row_json_p, "Failed to import plot data");
		}

	return add_status;
}


static bool AddPlotsFromJSON (ServiceJob *job_p, json_t *plots_json_p, Study *study_p, FieldTrialServiceData *data_p)
{
	OperationStatus status = OS_FAILED;
	bool success_flag	= true;

	if (json_is_array (plots_json_p))
		{
			const size_t num_rows = json_array_size (plots_json_p);
			size_t i;
			size_t num_empty_rows = 0;
			size_t num_fully_imported = 0;
			size_t num_partially_imported = 0;
			bool imported_row_flag;
			char *study_id_s = NULL;
			GeneBank *gru_gene_bank_p = GetGeneBankByName ("Germplasm Resources Unit", data_p);

			if (gru_gene_bank_p)
				{
					json_t *unknown_cols_p = json_object ();

					if (unknown_cols_p)
						{
							for (i = 0; i < num_rows; ++ i)
								{
									json_t *table_row_json_p = json_array_get (plots_json_p, i);

									/*
									 * Is the row non-empty?
									 */
									if (json_object_size (table_row_json_p) > 0)
										{
											OperationStatus add_status = AddPlotFromJSON (job_p, table_row_json_p, study_p, gru_gene_bank_p, unknown_cols_p, i, data_p);

											switch (add_status)
												{
													case OS_SUCCEEDED:
														++ num_fully_imported;
														break;

													case OS_PARTIALLY_SUCCEEDED:
														++ num_partially_imported;
														break;

													default:
														PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, table_row_json_p, "Failed to import plots row");
														break;
												}
										}
									else
										{
											++ num_empty_rows;
										}
								}

							json_decref (unknown_cols_p);
						}		/* if (unknown_cols_p) */


					FreeGeneBank (gru_gene_bank_p);
				}		/* if (gru_gene_bank_p) */


			if (num_fully_imported + num_empty_rows == num_rows)
				{
					status = OS_SUCCEEDED;
				}
			else if (num_fully_imported + num_partially_imported > 0)
				{
					status = OS_PARTIALLY_SUCCEEDED;
				}

			/*
			 * As the plots have been updated, clear any cached study
			 */
			study_id_s = GetBSONOidAsString (study_p -> st_id_p);

			if (study_id_s)
				{
					if (!ClearCachedStudy (study_id_s, data_p))
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to clear cached Study with id \"%s\"", study_id_s);
						}

					FreeBSONOidString (study_id_s);
				}
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get id for Study \"%s\"", study_p -> st_name_s);
				}
		}		/* if (json_is_array (plots_json_p)) */



	SetServiceJobStatus (job_p, status);

	return success_flag;
}


static Plot *CreatePlotFromTabularJSON (const json_t *table_row_json_p, const int32 row, const int32 column, Study *study_p, const FieldTrialServiceData *data_p)
{
	double *width_p = NULL;
	double *length_p = NULL;
	Plot *plot_p = NULL;
	const char *treatment_s = GetJSONString (table_row_json_p, S_TREATMENT_TITLE_S);
	const char *comment_s = GetJSONString (table_row_json_p, S_COMMENT_TITLE_S);
	const char *image_s = GetJSONString (table_row_json_p, S_IMAGE_TITLE_S);
	const char *thumbnail_s = GetJSONString (table_row_json_p, S_THUMBNAIL_TITLE_S);
	struct tm *sowing_date_p = NULL;
	struct tm *harvest_date_p = NULL;
	const char *date_s = GetJSONString (table_row_json_p, S_SOWING_TITLE_S);
	uint32 *sowing_order_p = NULL;
	uint32 *walking_order_p = NULL;

	GetValidRealFromJSON (table_row_json_p, S_WIDTH_TITLE_S, &width_p);
	GetValidRealFromJSON (table_row_json_p, S_LENGTH_TITLE_S, &length_p);

	GetValidUnsignedIntFromJSON (table_row_json_p, S_SOWING_ORDER_TITLE_S, &sowing_order_p);
	GetValidUnsignedIntFromJSON (table_row_json_p, S_WALKING_ORDER_TITLE_S, &walking_order_p);


	if (!IsStringEmpty (date_s))
		{
			sowing_date_p = GetTimeFromString (date_s);

			if (!sowing_date_p)
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get sowing date from \"%s\"", date_s);
				}
		}

	date_s = GetJSONString (table_row_json_p, S_HARVEST_TITLE_S);
	if (!IsStringEmpty (date_s))
		{
			harvest_date_p = GetTimeFromString (date_s);

			if (!harvest_date_p)
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get harvest date from \"%s\"", date_s);
				}
		}

	plot_p = AllocatePlot (NULL, sowing_date_p, harvest_date_p, width_p, length_p, row, column, treatment_s, comment_s,
												 image_s, thumbnail_s, sowing_order_p, walking_order_p, study_p);

	if (plot_p)
		{
			if (!SavePlot (plot_p, data_p))
				{
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, table_row_json_p, "Failed to save plot");

					FreePlot (plot_p);
					plot_p = NULL;
				}		/* if (!SavePlot (plot_p, data_p)) */
		}		/* if (plot_p) */
	else
		{
			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, table_row_json_p, "Failed to allocate Plot");
		}

	if (width_p)
		{
			FreeMemory (width_p);
		}

	if (length_p)
		{
			FreeMemory (length_p);
		}


	if (harvest_date_p)
		{
			FreeTime (harvest_date_p);
		}


	if (sowing_date_p)
		{
			FreeTime (sowing_date_p);
		}


	return plot_p;
}


Plot *GetPlotById (bson_oid_t *id_p, Study *study_p, FieldTrialServiceData *data_p)
{
	Plot *plot_p = NULL;

	bson_t *query_p = BCON_NEW (MONGO_ID_S, BCON_OID (id_p));

	if (query_p)
		{
			plot_p = GetUniquePlot (query_p, study_p, data_p);

			bson_destroy (query_p);
		}		/* if (query_p) */

	return plot_p;
}


Plot *GetPlotByRowAndColumn (const uint32 row, const uint32 column, Study *study_p, FieldTrialServiceData *data_p)
{
	Plot *plot_p = NULL;
	bson_t *query_p = BCON_NEW (PL_ROW_INDEX_S, BCON_INT32 (row), PL_COLUMN_INDEX_S, BCON_INT32 (column), PL_PARENT_STUDY_S, BCON_OID (study_p -> st_id_p));

	if (query_p)
		{
			plot_p = GetUniquePlot (query_p, study_p, data_p);

			bson_destroy (query_p);
		}		/* if (query_p) */

	return plot_p;
}


/*
 * https://frictionlessdata.io/data-package/#the-data-package-suite-of-specifications
 * https://specs.frictionlessdata.io/table-schema/
 * https://specs.frictionlessdata.io/csv-dialect/#specification
 * */
/*
 * https://specs.frictionlessdata.io/table-schema/
 */
bool AddPlotsTableSchema (json_t *table_p)
{
	bool success_flag = false;
	json_t *fields_p = json_array ();

	if (fields_p)
		{

		}		/* if (fields_p) */

	return success_flag;
}


/*
 * STATIC DEFINITIONS
 */

static bool RemoveExistingPlotsForStudy (Study *study_p, const FieldTrialServiceData *data_p)
{
	bool success_flag = false;

	if (SetMongoToolCollection (data_p -> dftsd_mongo_p, data_p -> dftsd_collection_ss [DFTD_PLOT]))
		{
			bson_t *query_p = BCON_NEW (PL_PARENT_STUDY_S, BCON_OID (study_p -> st_id_p));

			if (query_p)
				{
					success_flag = RemoveMongoDocumentsByBSON (data_p -> dftsd_mongo_p, query_p, false);

					bson_destroy (query_p);
				}		/* if (query_p) */
		}

	return success_flag;
}


static Plot *GetUniquePlot (bson_t *query_p, Study *study_p, FieldTrialServiceData *data_p)
{
	Plot *plot_p = NULL;

	if (SetMongoToolCollection (data_p -> dftsd_mongo_p, data_p -> dftsd_collection_ss [DFTD_PLOT]))
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

									plot_p = GetPlotFromJSON (entry_p, study_p, data_p);

									if (!plot_p)
										{

										}

								}		/* if (num_results == 1) */

						}		/* if (json_is_array (results_p)) */

					json_decref (results_p);
				}		/* if (results_p) */

		}		/* if (SetMongoToolCollection (data_p -> dftsd_mongo_p, data_p -> dftsd_collection_ss [DFTD_PLOT])) */


	return plot_p;
}


static json_t *GetStudyPlotsForSubmissionTable (Study *study_p, FieldTrialServiceData *service_data_p)
{
	json_t *plots_table_p = NULL;

	/*
	 * Has the study got any plots?
	 */
	if (study_p -> st_plots_p -> ll_size == 0)
		{
			if (!GetStudyPlots (study_p, service_data_p))
				{

				}

		}		/* if ((study_p -> st_plots_p == NULL) || (study_p -> st_plots_p -> ll_size == 0)) */


	if (study_p -> st_plots_p -> ll_size > 0)
		{
			bool success_flag = true;

			plots_table_p = json_array ();

			if (plots_table_p)
				{
					PlotNode *plot_node_p = (PlotNode *) (study_p -> st_plots_p -> ll_head_p);

					while (success_flag && plot_node_p)
						{
							if (AddPlotRowsToTable (plot_node_p -> pn_plot_p, plots_table_p, service_data_p))
								{
									plot_node_p = (PlotNode *) (plot_node_p -> pn_node.ln_next_p);
								}
							else
								{
									success_flag = false;
								}

						}		/* while (success_flag && plot_node_p) */

					if (!success_flag)
						{
							json_decref (plots_table_p);
						}

				}		/* if (plots_table_p) */

		}		/* if (study_p -> st_plots_p -> ll_size >= 0) */
	else
		{
			/*
			 * Does the study have default values to load?
			 */
			if (HasStudyGotPlotLayoutDetails (study_p))
				{
					plots_table_p = GeneratePlotsTemplate (study_p);
				}
			else
				{
					/* it's an empty table */
					plots_table_p = json_array ();
				}
		}

	return plots_table_p;
}


static bool AddPlotRowsToTable (const Plot *plot_p, json_t *plots_table_p, const FieldTrialServiceData *service_data_p)
{
	bool success_flag = true;
	LinkedList *rows_p = plot_p -> pl_rows_p;

	if (rows_p)
		{
			RowNode *row_node_p = (RowNode *) (rows_p -> ll_head_p);

			while (success_flag && row_node_p)
				{
					json_t *row_json_p = GetPlotTableRow (row_node_p -> rn_row_p, service_data_p);

					if (row_json_p)
						{
							if (json_array_append_new (plots_table_p, row_json_p) == 0)
								{
									row_node_p = (RowNode *) (row_node_p -> rn_node.ln_next_p);
								}		/* if (json_array_append_new (plots_json_p, plot_row_p) == 0) */
							else
								{
									json_decref (row_json_p);
									success_flag = false;
								}

						}		/* if (plot_row_p) */
					else
						{
							success_flag = false;
						}

				}		/* while (success_flag && row_node_p) */


		}		/* if (rows_p) */

	return success_flag;
}


static json_t *GeneratePlotsTemplate (const Study *study_p)
{
	json_t *plots_p = json_array ();

	if (plots_p)
		{
			bool success_flag = true;
			const uint32 num_rows = * (study_p -> st_num_rows_p);
			const uint32 num_cols = * (study_p -> st_num_columns_p);
			uint32 i = 1;

			while ((i <= num_rows) && (success_flag == true))
				{
					uint32 j = 1;

					while ((j <= num_cols) && (success_flag == true))
						{
							json_t *row_json_p = GetPlotRowTemplate (i, j, study_p -> st_default_plot_width_p, study_p -> st_default_plot_length_p);

							if (row_json_p)
								{
									if (!json_array_append_new (plots_p, row_json_p) == 0)
										{
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add plot template for [ " UINT32_FMT ", " UINT32_FMT " ]", i, j);
											json_decref (row_json_p);
											success_flag = false;
										}

								}		/* if (plot_row_p) */
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to generate plot template for [ " UINT32_FMT ", " UINT32_FMT " ]", i, j);
									success_flag = false;
								}

							++ j;
						}		/* while ((j <= num_cols) && (success_flag == true)) */

					++ i;
				}		/* while ((i <= num_rows) && (success_flag == true)) */

			if (success_flag)
				{
					return plots_p;
				}

			json_decref (plots_p);
		}		/* if (plots_p) */

	return NULL;
}



static json_t *GetPlotRowTemplate (const uint32 row, const uint32 column, const double64 *width_p, const double64 *height_p)
{
	json_t *table_row_p = json_object ();

	if (table_row_p)
		{
			if (SetJSONInteger (table_row_p, S_ROW_TITLE_S, row))
				{
					if (SetJSONInteger (table_row_p, S_COLUMN_TITLE_S, column))
						{
							if ((width_p == NULL) || (SetJSONReal (table_row_p, S_WIDTH_TITLE_S, *width_p)))
								{
									if ((height_p == NULL) || (SetJSONReal (table_row_p, S_LENGTH_TITLE_S, *height_p)))
										{
											return table_row_p;
										}
								}
						}
				}

			json_decref (table_row_p);
		}

	return NULL;
}


static json_t *GetPlotTableRow (const Row *row_p, const FieldTrialServiceData *service_data_p)
{
	json_t *table_row_p = json_object ();

	if (table_row_p)
		{
			/*
				if (AddColumnParameterHint (S_ACCESSION_TITLE_S, PT_STRING, hints_p))
			*/
			Plot *plot_p = row_p -> ro_plot_p;

			if ((plot_p -> pl_row_index == 0) || (SetJSONInteger (table_row_p, S_ROW_TITLE_S, plot_p -> pl_row_index)))
				{
					if ((plot_p -> pl_column_index == 0) || (SetJSONInteger (table_row_p, S_COLUMN_TITLE_S, plot_p -> pl_column_index)))
						{
							if ((plot_p -> pl_length_p == NULL) || (SetJSONReal (table_row_p, S_LENGTH_TITLE_S, * (plot_p -> pl_length_p))))
								{
									if ((plot_p -> pl_width_p == NULL) || (SetJSONReal (table_row_p, S_WIDTH_TITLE_S, * (plot_p -> pl_width_p))))
										{
											if ((plot_p -> pl_comment_s == NULL) || (SetJSONString (table_row_p, S_COMMENT_TITLE_S, plot_p -> pl_comment_s)))
												{
													if ((plot_p -> pl_treatments_s == NULL) || (SetJSONString (table_row_p, S_TREATMENT_TITLE_S, plot_p -> pl_treatments_s)))
														{
															if (AddValidDateToJSON (plot_p -> pl_sowing_date_p, table_row_p, S_SOWING_TITLE_S, false))
																{
																	if (AddValidDateToJSON (plot_p -> pl_harvest_date_p, table_row_p, S_HARVEST_TITLE_S, false))
																		{
																			if ((row_p -> ro_rack_index == 0) || (SetJSONInteger (table_row_p, S_RACK_TITLE_S, row_p -> ro_rack_index)))
																				{
																					if ((row_p -> ro_replicate_index == 0) || (SetJSONInteger (table_row_p, PL_REPLICATE_TITLE_S, row_p -> ro_replicate_index)))
																						{
																							if ((row_p -> ro_by_study_index == 0) || (SetJSONInteger (table_row_p, PL_INDEX_TABLE_TITLE_S, row_p -> ro_by_study_index)))
																								{
																									bool success_flag = false;

																									if (row_p -> ro_material_p)
																										{
																											if ((row_p -> ro_material_p -> ma_accession_s == NULL) || (SetJSONString (table_row_p, PL_ACCESSION_TABLE_TITLE_S, row_p -> ro_material_p -> ma_accession_s)))
																												{

																												}

																											GeneBank *gene_bank_p = GetGeneBankById (row_p -> ro_material_p -> ma_gene_bank_id_p, VF_CLIENT_MINIMAL, service_data_p);

																											if (gene_bank_p)
																												{
																													if (SetJSONString (table_row_p, S_GENE_BANK_S, gene_bank_p -> gb_name_s))
																														{
																															success_flag = true;
																														}

																													FreeGeneBank (gene_bank_p);
																												}
																										}
																									else
																										{
																											success_flag = true;
																										}

																									if (success_flag)
																										{
																											return table_row_p;
																										}


																								}		/* if ((row_p -> ro_by_study_index == 0) || (SetJSONInteger (table_row_p, S_INDEX_TITLE_S, row_p -> ro_by_study_index))) */

																						}		/* if ((row_p -> ro_replicate_index == 0) || (SetJSONInteger (table_row_p, PL_REPLICATE_TITLE_S, row_p -> ro_rephttps://www.theguardian.com/ukhttps://www.theguardian.com/uklicate_index))) */

																				}		/* if ((row_p -> ro_rack_index == 0) || (SetJSONInteger (table_row_p, S_RACK_TITLE_S, row_p -> ro_rack_index))) */


																		}		/* if ((plot_p -> pl_harvest_date_p == NULL) || (AddValidDateToJSON (plot_p -> pl_harvest_date_p, table_row_p, S_HARVEST_TITLE_S))) */

																}		/* if ((plot_p -> pl_sowing_date_p == NULL) || (AddValidDateToJSON (plot_p -> pl_sowing_date_p, table_row_p, S_SOWING_TITLE_S))) */

														}		/* if ((plot_p -> pl_treatments_s == NULL) || (SetJSONString (table_row_p, S_TREATMENT_TITLE_S, plot_p -> pl_treatments_s))) */

												}		/* if ((plot_p -> pl_comment_s == NULL) || (SetJSONString (table_row_p, S_COMMENT_TITLE_S, plot_p -> pl_comment_s))) */

										}		/* if ((CompareDoubles (plot_p -> pl_width, 0.0) <= 0) || (SetJSONReal (table_row_p, S_WIDTH_TITLE_S, plot_p -> pl_width))) */

								}		/* if ((CompareDoubles (plot_p -> pl_length, 0.0) <= 0) || (SetJSONReal (table_row_p, S_LENGTH_TITLE_S, plot_p -> pl_length))) */


						}		/* if ((plot_p -> pl_column_index == 0) || (SetJSONInteger (table_row_p, S_COLUMN_TITLE_S, plot_p -> pl_column_index))) */

				}		/* if ((plot_p -> pl_row_index == 0) || (SetJSONInteger (table_row_p, S_ROW_TITLE_S, plot_p -> pl_row_index))) */

			json_decref (table_row_p);
		}		/* if (table_row_p) */

	return NULL;
}



static bool AddPlotDefaultsFromStudy (Study *study_p, ServiceData *data_p, ParameterSet *params_p)
{
	bool success_flag = false;
	Parameter *param_p = NULL;
	ParameterGroup *group_p = CreateAndAddParameterGroupToParameterSet ("Default Plots data for Study", false, data_p, params_p);
	uint32 *num_replicates_p = NULL;
	double64 *width_p = NULL;
	double64 *length_p = NULL;

	if (study_p)
		{
			num_replicates_p = study_p -> st_num_replicates_p;
			width_p = study_p -> st_default_plot_width_p;
			length_p = study_p -> st_default_plot_length_p;
		}

		if ((param_p = EasyCreateAndAddUnsignedIntParameterToParameterSet (data_p, params_p, group_p, STUDY_NUM_REPLICATES.npt_name_s, "Default number of replicates", "The number of replicates", num_replicates_p, PL_ALL)) != NULL)
			{
				param_p -> pa_read_only_flag = true;

				if ((param_p = EasyCreateAndAddDoubleParameterToParameterSet (data_p, params_p, group_p, STUDY_PLOT_WIDTH.npt_type, STUDY_PLOT_WIDTH.npt_name_s, "Default plot width", "The default width, in metres, of each plot", width_p, PL_ALL)) != NULL)
					{
						param_p -> pa_read_only_flag = true;

						if ((param_p = EasyCreateAndAddDoubleParameterToParameterSet (data_p, params_p, group_p, STUDY_PLOT_LENGTH.npt_type, STUDY_PLOT_LENGTH.npt_name_s, "Default plot length", "The default length, in metres, of each plot", length_p, PL_ALL)) != NULL)
							{
								param_p -> pa_read_only_flag = true;

								success_flag = true;
							}		/* if (EasyCreateAndAddUnsignedIntParameterToParameterSet (service_data_p, params_p, group_p, STUDY_NUM_PLOT_ROWS.npt_name_s, "Plot width", "The default width, in metres, of each plot", NULL, PL_ALL)) */

					}		/* if (EasyCreateAndAddUnsignedIntParameterToParameterSet (service_data_p, params_p, group_p, STUDY_NUM_PLOT_ROWS.npt_name_s, "Plot width", "The default width, in metres, of each plot", NULL, PL_ALL)) */

			}		/* if (EasyCreateAndAddUnsignedIntParameterToParameterSet (service_data_p, params_p, group_p, STUDY_NUM_REPLICATES.npt_name_s, "Number of replicates", "The number of replicates", NULL, PL_ALL)) */


	return success_flag;
}


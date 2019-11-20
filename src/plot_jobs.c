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
#include "study_jobs.h"
#include "math_utils.h"
#include "material.h"
#include "row.h"
#include "gene_bank.h"


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
static const char * const S_HARVEST_TITLE_S = "Harvest date";
static const char * const S_WIDTH_TITLE_S = "Width";
static const char * const S_LENGTH_TITLE_S = "Length";
static const char * const S_INDEX_TITLE_S = "Plot ID";
static const char * const S_ROW_TITLE_S = "Row";
static const char * const S_COLUMN_TITLE_S = "Column";
static const char * const S_RACK_TITLE_S = "Rack";
static const char * const S_ACCESSION_TITLE_S = "Accession";
static const char * const S_GENE_BANK_S = "Gene Bank";
static const char * const S_TREATMENT_TITLE_S = "Treatment";
static const char * const S_REPLICATE_TITLE_S = "Replicate";
static const char * const S_COMMENT_TITLE_S = "Comment";


static NamedParameterType S_PLOT_TABLE_COLUMN_DELIMITER = { "PL Data delimiter", PT_CHAR };
static NamedParameterType S_PLOT_TABLE = { "PL Upload", PT_TABLE};


static NamedParameterType S_STUDIES_LIST = { "PL Study", PT_STRING };


static const char S_DEFAULT_COLUMN_DELIMITER =  '|';


/*
 * static declarations
 */



static bool AddPlotsFromJSON (ServiceJob *job_p, const json_t *plots_json_p, Study *study_p,  const DFWFieldTrialServiceData *data_p);

static Parameter *GetTableParameter (ParameterSet *param_set_p, ParameterGroup *group_p, const DFWFieldTrialServiceData *data_p);

static json_t *GetTableParameterHints (void);

static Plot *GetUniquePlot (bson_t *query_p, Study *study_p, const DFWFieldTrialServiceData *data_p);


/*
 * API definitions
 */

bool AddSubmissionPlotParams (ServiceData *data_p, ParameterSet *param_set_p, Resource *UNUSED_PARAM (resource_p))
{
	bool success_flag = false;
	Parameter *param_p = NULL;
	ParameterGroup *group_p = CreateAndAddParameterGroupToParameterSet ("Plots", false, data_p, param_set_p);
	SharedType def;

	InitSharedType (&def);

	if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_STUDIES_LIST.npt_type, S_STUDIES_LIST.npt_name_s, "Study", "The Study that these plots are from", def, PL_ALL)) != NULL)
		{
			const DFWFieldTrialServiceData *dfw_service_data_p = (DFWFieldTrialServiceData *) data_p;

			if (SetUpStudiesListParameter (dfw_service_data_p, param_p, NULL))
				{
					def.st_char_value = S_DEFAULT_COLUMN_DELIMITER;

					if ((param_p = CreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_PLOT_TABLE_COLUMN_DELIMITER.npt_type, false, S_PLOT_TABLE_COLUMN_DELIMITER.npt_name_s, "Delimiter", "The character delimiting columns", NULL, def, NULL, NULL, PL_ADVANCED, NULL)) != NULL)
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
					OperationStatus status = OS_FAILED;
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

							if (GetParameterValueFromParameterSet (param_set_p, S_STUDIES_LIST.npt_name_s, &parent_experimental_area_value, true))
								{
									Study *study_p = GetStudyByIdString (parent_experimental_area_value.st_string_value_s, VF_STORAGE, data_p);

									if (study_p)
										{
											if (!AddPlotsFromJSON (job_p, plots_json_p, study_p, data_p))
												{
													PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, plots_json_p, "AddPlotsFromJSON failed for study \"%s\"", study_p -> st_name_s);
												}

											FreeStudy (study_p);
										}
								}

							json_decref (plots_json_p);
						}		/* if (plots_json_p) */

					job_done_flag = true;


				}		/* if (value.st_boolean_value) */

		}		/* if (GetParameterValueFromParameterSet (param_set_p, S_ADD_EXPERIMENTAL_AREA.npt_name_s, &value, true)) */


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
	else if (strcmp (param_name_s, S_PLOT_TABLE.npt_name_s) == 0)
		{
			*pt_p = S_PLOT_TABLE.npt_type;
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
	/*
	headers_s = ConcatenateVarargsStrings (S_SOWING_TITLE_S, delim_s, S_HARVEST_TITLE_S, delim_s, S_WIDTH_TITLE_S, delim_s, S_LENGTH_TITLE_S, delim_s, S_ROW_TITLE_S, delim_s, S_COLUMN_TITLE_S, delim_s,
																				 S_REPLICATE_TITLE_S, delim_s, S_RACK_TITLE_S, delim_s, S_MATERIAL_TITLE_S, delim_s, S_TRIAL_DESIGN_TITLE_S, delim_s, S_GROWING_CONDITION_TITLE_S, delim_s, S_TREATMENT_TITLE_S, delim_s, NULL);
	 */
	json_t *hints_p = json_array ();

	if (hints_p)
		{
			if (AddColumnParameterHint (S_SOWING_TITLE_S, PT_TIME, hints_p))
				{
					if (AddColumnParameterHint (S_HARVEST_TITLE_S, PT_TIME, hints_p))
						{
							if (AddColumnParameterHint (S_WIDTH_TITLE_S, PT_UNSIGNED_REAL, hints_p))
								{
									if (AddColumnParameterHint (S_LENGTH_TITLE_S, PT_UNSIGNED_REAL, hints_p))
										{
											if (AddColumnParameterHint (S_INDEX_TITLE_S, PT_UNSIGNED_INT, hints_p))
												{
													if (AddColumnParameterHint (S_ROW_TITLE_S, PT_UNSIGNED_INT, hints_p))
														{
															if (AddColumnParameterHint (S_COLUMN_TITLE_S, PT_UNSIGNED_INT, hints_p))
																{
																	if (AddColumnParameterHint (S_REPLICATE_TITLE_S, PT_UNSIGNED_INT, hints_p))
																		{
																			if (AddColumnParameterHint (S_RACK_TITLE_S, PT_UNSIGNED_INT, hints_p))
																				{
																					if (AddColumnParameterHint (S_ACCESSION_TITLE_S, PT_STRING, hints_p))
																						{
																							if (AddColumnParameterHint (S_COMMENT_TITLE_S, PT_STRING, hints_p))
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

	param_p = CreateAndAddParameterToParameterSet (& (data_p -> dftsd_base_data), param_set_p, group_p, S_PLOT_TABLE.npt_type, false, S_PLOT_TABLE.npt_name_s, "Plot data to upload", "The data to upload", NULL, def, NULL, NULL, PL_ALL, NULL);

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


static bool AddPlotsFromJSON (ServiceJob *job_p, const json_t *plots_json_p, Study *study_p, const DFWFieldTrialServiceData *data_p)
{
	OperationStatus status = OS_FAILED;
	bool success_flag	= true;

	if (json_is_array (plots_json_p))
		{
			const size_t num_rows = json_array_size (plots_json_p);
			size_t i;
			size_t num_imported = 0;
			bool imported_row_flag;

			for (i = 0; i < num_rows; ++ i)
				{
					const json_t *table_row_json_p = json_array_get (plots_json_p, i);
					const char *gene_bank_s = GetJSONString (table_row_json_p, S_GENE_BANK_S);
					GeneBank *gene_bank_p = NULL;

					imported_row_flag = false;

					if (!gene_bank_s)
						{
							/* default to using the GRU */
							gene_bank_s = "Germplasm Resources Unit";
						}

					gene_bank_p = GetGeneBankByName (gene_bank_s, data_p);

					if (gene_bank_p)
						{
							const char *accession_s = GetJSONString (table_row_json_p, S_ACCESSION_TITLE_S);

							if (accession_s)
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

															if (plot_p)
																{
																	/* We're good to go */
																}		/* if (plot_p) */
															else
																{
																	double width = 0.0;

																	if (GetJSONStringAsDouble (table_row_json_p, S_WIDTH_TITLE_S, &width))
																		{
																			double length = 0.0;

																			if (GetJSONStringAsDouble (table_row_json_p, S_LENGTH_TITLE_S, &length))
																				{
																					const char *treatment_s = GetJSONString (table_row_json_p, S_TREATMENT_TITLE_S);
																					const char *comment_s = GetJSONString (table_row_json_p, S_COMMENT_TITLE_S);
																					struct tm *sowing_date_p = NULL;
																					struct tm *harvest_date_p = NULL;
																					const char *date_s = GetJSONString (table_row_json_p, S_SOWING_TITLE_S);


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

																					plot_p = AllocatePlot (NULL, sowing_date_p, harvest_date_p, width, length, row, column, treatment_s, comment_s, study_p);

																					if (plot_p)
																						{
																							if (SavePlot (plot_p, data_p))
																								{
																									success_flag = false;
																								}
																							else
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
																	int32 rack_plotwise_index = -1;

																	if (GetJSONStringAsInteger (table_row_json_p, S_RACK_TITLE_S, &rack_plotwise_index))
																		{
																			int32 rack_studywise_index = -1;

																			if (GetJSONStringAsInteger (table_row_json_p, S_INDEX_TITLE_S, &rack_studywise_index))
																				{
																					int32 replicate = 1;
																					const char *rep_s = GetJSONString (table_row_json_p, S_REPLICATE_TITLE_S);
																					bool control_rep_flag = false;
																					Row *row_p = NULL;

																					if (rep_s)
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
																							row_p = AllocateRow (NULL, rack_plotwise_index, rack_studywise_index, replicate, material_p, plot_p);

																							if (row_p)
																								{
																									if (control_rep_flag)
																										{
																											SetRowGenotypeControl (row_p, true);
																										}

																									if (SaveRow (row_p, data_p, true))
																										{
																											++ num_imported;
																											imported_row_flag = true;
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

																						}		/* if (success_flag) */

																				}		/* if (GetJSONStringAsInteger (table_row_json_p, S_INDEX_TITLE_S, &study_index)) */
																			else
																				{
																					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, table_row_json_p, "Failed to get \"%s\"", S_INDEX_TITLE_S);
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
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get Material with internal name \"%s\" for area \"%s\"", accession_s, study_p -> st_name_s);
										}

								}		/* if (accession_s) */
							else
								{
									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, table_row_json_p, "Failed to get \"%s\"", S_ACCESSION_TITLE_S);
								}
						}		/* if (gene_bank_p) */
					else
						{
							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, table_row_json_p, "Failed to get gene bank \%s\"", gene_bank_s);
						}

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



Plot *GetPlotById (bson_oid_t *id_p, Study *study_p, const DFWFieldTrialServiceData *data_p)
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


Plot *GetPlotByRowAndColumn (const uint32 row, const uint32 column, Study *study_p, const DFWFieldTrialServiceData *data_p)
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



static Plot *GetUniquePlot (bson_t *query_p, Study *study_p, const DFWFieldTrialServiceData *data_p)
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

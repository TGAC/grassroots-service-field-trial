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
 * row_jobs.c
 *
 *  Created on: 28 Oct 2018
 *      Author: billy
 */


#include <treatment_jobs.h>
#include "row_jobs.h"
#include "plot_jobs.h"
#include "string_utils.h"
#include "study_jobs.h"
#include "math_utils.h"
#include "time_util.h"
#include "observation.h"


#include "char_parameter.h"
#include "json_parameter.h"
#include "string_parameter.h"

/*
 * static declarations
 */

static const char S_DEFAULT_COLUMN_DELIMITER =  '|';

static const char * const S_ROW_S = "Row";
static const char * const S_COLUMN_S = "Column";
static const char * const S_RACK_S = "Rack";
static const char * const S_PLOT_INDEX_S = "Plot index";


static NamedParameterType S_ROW_PHENOTYPE_DATA_TABLE_COLUMN_DELIMITER = { "RO phenotype data delimiter", PT_CHAR };
static NamedParameterType S_ROW_PHENOTYPE_DATA_TABLE = { "RO phenotype data upload", PT_JSON_TABLE};
static NamedParameterType S_STUDIES_LIST = { "RO Study", PT_STRING };


static Parameter *GetPhenotypesDataTableParameter (ParameterSet *param_set_p, ParameterGroup *group_p, const DFWFieldTrialServiceData *data_p);

static bool AddObservationValuesFromJSON (ServiceJob *job_p, const json_t *observations_json_p, Study *study_p, const DFWFieldTrialServiceData *data_p);


static json_t *GetTableParameterHints (void);

static bool GetRackStudyIndex (const json_t *observation_json_p, int32 *plot_index_p);

/*
 * API Definitions
 */


bool AddSubmissionRowPhenotypeParams (ServiceData *data_p, ParameterSet *param_set_p, Resource *resource_p)
{
	bool success_flag = false;

	ParameterGroup *group_p = CreateAndAddParameterGroupToParameterSet ("Row Phenotypes", false, data_p, param_set_p);

	if (group_p)
		{
			Parameter *param_p = NULL;
			const DFWFieldTrialServiceData *dfw_service_data_p = (DFWFieldTrialServiceData *) data_p;

			if ((param_p = EasyCreateAndAddStringParameterToParameterSet (data_p, param_set_p, group_p, S_STUDIES_LIST.npt_type, S_STUDIES_LIST.npt_name_s, "Study", "The Study to update the phenotypes for", NULL, PL_ALL)) != NULL)
				{
					if (SetUpStudiesListParameter (dfw_service_data_p, (StringParameter *) param_p, NULL))
						{
							char delim = S_DEFAULT_COLUMN_DELIMITER;

							if ((param_p = EasyCreateAndAddCharParameterToParameterSet (data_p, param_set_p, group_p, S_ROW_PHENOTYPE_DATA_TABLE_COLUMN_DELIMITER.npt_name_s, "Delimiter", "The character delimiting columns", &delim, PL_ADVANCED)) != NULL)
								{
									if ((param_p = GetPhenotypesDataTableParameter (param_set_p, group_p, dfw_service_data_p)) != NULL)
										{
											success_flag = true;
										}
								}

						}

				}

		}		/* if (group_p) */


	return success_flag;
}


bool RunForSubmissionRowPhenotypeParams (DFWFieldTrialServiceData *data_p, ParameterSet *param_set_p, ServiceJob *job_p)
{
	bool job_done_flag = false;
	const char *study_id_s = NULL;

	if (GetCurrentStringParameterValueFromParameterSet (param_set_p, S_STUDIES_LIST.npt_name_s, &study_id_s))
		{
			Study *study_p = GetStudyByIdString (study_id_s, VF_STORAGE, data_p);

			if (study_p)
				{
					const json_t *rows_json_p = NULL;

					if (GetCurrentJSONParameterValueFromParameterSet (param_set_p, S_ROW_PHENOTYPE_DATA_TABLE.npt_name_s, &rows_json_p))
						{
							/*
							 * Has a spreadsheet been uploaded?
							 */
							job_done_flag = true;

							/*
							 * Has a spreadsheet been uploaded?
							 */
							if (rows_json_p)
								{
									const size_t num_rows = json_array_size (rows_json_p);

									if (num_rows > 0)
										{
											if (!AddObservationValuesFromJSON (job_p, rows_json_p, study_p, data_p))
												{
													PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, rows_json_p, "AddObservationValuesFromJSON for study \"%s\" failed", study_id_s);
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

						}		/* if (GetParameterValueFromParameterSet (param_set_p, S_PHENOTYPE_TABLE.npt_name_s, &value, true)) */

					FreeStudy (study_p);
				}		/* if (study_p) */

		}		/* if (GetParameterValueFromParameterSet (param_set_p, S_STUDIES_LIST.npt_name_s, &parent_experimental_area_value, true)) */

	return job_done_flag;
}


bool GetSubmissionRowPhenotypeParameterTypeForNamedParameter (const char *param_name_s, ParameterType *pt_p)
{
	bool success_flag = true;

	if (strcmp (param_name_s, S_STUDIES_LIST.npt_name_s) == 0)
		{
			*pt_p = S_STUDIES_LIST.npt_type;
		}
	else if (strcmp (param_name_s, S_ROW_PHENOTYPE_DATA_TABLE_COLUMN_DELIMITER.npt_name_s) == 0)
		{
			*pt_p = S_ROW_PHENOTYPE_DATA_TABLE_COLUMN_DELIMITER.npt_type;
		}
	else if (strcmp (param_name_s, S_ROW_PHENOTYPE_DATA_TABLE.npt_name_s) == 0)
		{
			*pt_p = S_ROW_PHENOTYPE_DATA_TABLE.npt_type;
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
			if (AddColumnParameterHint (S_ROW_S, PT_UNSIGNED_INT, hints_p))
				{
					if (AddColumnParameterHint (S_COLUMN_S, PT_UNSIGNED_INT, hints_p))
						{
							if (AddColumnParameterHint (S_RACK_S, PT_UNSIGNED_INT, hints_p))
								{
									return hints_p;
								}
						}
				}

			json_decref (hints_p);
		}

	return NULL;
}


static Parameter *GetPhenotypesDataTableParameter (ParameterSet *param_set_p, ParameterGroup *group_p, const DFWFieldTrialServiceData *data_p)
{
	Parameter *param_p = EasyCreateAndAddJSONParameterToParameterSet (& (data_p -> dftsd_base_data), param_set_p, group_p, S_ROW_PHENOTYPE_DATA_TABLE.npt_type, S_ROW_PHENOTYPE_DATA_TABLE.npt_name_s, "Phenotype data values to upload", "The data to upload", NULL, PL_ALL);

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
									if (AddParameterKeyStringValuePair (param_p, PA_TABLE_COLUMN_HEADERS_PLACEMENT_S, PA_TABLE_COLUMN_HEADERS_PLACEMENT_FIRST_ROW_S))
										{
											success_flag = true;
										}
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


static bool GetRackStudyIndex (const json_t *observation_json_p, int32 *rack_study_index_p)
{
	bool success_flag = false;
	const json_t *index_p = json_object_get (observation_json_p, S_PLOT_INDEX_S);

	if (index_p)
		{
			if (json_is_integer (index_p))
				{
					*rack_study_index_p = json_integer_value (index_p);
					success_flag = true;
				}
			else if (json_is_string (index_p))
				{
					const char *index_s = json_string_value (index_p);

					if (GetValidInteger (&index_s, rack_study_index_p))
						{
							success_flag = true;
						}
					else
						{
							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, observation_json_p, "Failed to get plot index \"%s\" as integer", index_s);
						}
				}
			else
				{
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, observation_json_p, "\"%s\" is not a string or a number", S_PLOT_INDEX_S);
				}
		}		/* if (index_p) */
	else
		{
			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, observation_json_p, "Failed to get \"%s\"", S_PLOT_INDEX_S);
		}


	return success_flag;
}


static bool AddObservationValuesFromJSON (ServiceJob *job_p, const json_t *observations_json_p, Study *study_p, const DFWFieldTrialServiceData *data_p)
{
	bool success_flag	= true;
	OperationStatus status = OS_FAILED;

	if (json_is_array (observations_json_p))
		{
			const size_t num_rows = json_array_size (observations_json_p);
			size_t i;
			size_t num_imported = 0;
			size_t num_empty_rows = 0;
			bool imported_obeservation_flag = false;

			for (i = 0; i < num_rows; ++ i)
				{
					json_t *observation_json_p = json_array_get (observations_json_p, i);
					const size_t row_size = json_object_size (observation_json_p);

					if (row_size > 0)
						{
							int32 rack_index = -1;

							imported_obeservation_flag = false;

							if (GetRackStudyIndex (observation_json_p, &rack_index))
								{
									const bool expand_fields_flag = true;
									Row *row_p = GetRowByStudyIndex (rack_index, study_p, data_p);

									if (row_p)
										{
											bool loop_success_flag = true;
											void *iterator_p = json_object_iter (observation_json_p);

											while (iterator_p && loop_success_flag)
												{
													const char *key_s = json_object_iter_key (iterator_p);
													json_t *value_p = json_object_iter_value (iterator_p);

													/*
													 * ignore our column names
													 */
													if ((strcmp (key_s, S_PLOT_INDEX_S) != 0) && (strcmp (key_s, S_RACK_S) != 0))
														{
															/*
															 * make sure it isn't a date column
															 */
															const char * const DATE_ENDING_S = " date";
															const char * const CORRECTED_ENDING_S = " corrected";

															if ((!DoesStringEndWith (key_s, DATE_ENDING_S)) && (!DoesStringEndWith (key_s, CORRECTED_ENDING_S)))
																{
																	Treatment *treatment_p = GetTreatmentByVariableName (key_s, data_p);

																	if (treatment_p)
																		{
																			Observation *observation_p = NULL;
																			bool added_phenotype_flag = false;
																			const char *raw_value_s = json_string_value (value_p);
																			const char *corrected_value_s = NULL;
																			char *column_header_s = NULL;

																			/* corrected value */
																			column_header_s = ConcatenateStrings (key_s, CORRECTED_ENDING_S);
																			if (column_header_s)
																				{
																					corrected_value_s = GetJSONString (observation_json_p, column_header_s);
																					FreeCopiedString (column_header_s);
																				}		/* if (column_header_s) */


																			if ((!IsStringEmpty (raw_value_s)) || (!IsStringEmpty (corrected_value_s)))
																				{
																					const char *growth_stage_s = NULL;
																					const char *method_s = NULL;
																					ObservationNature nature = ON_ROW;
																					Instrument *instrument_p = NULL;
																					bson_oid_t *observation_id_p = GetNewBSONOid ();
																					struct tm *observation_date_p = NULL;

																					/*
																					 * assume failure to import
																					 */
																					loop_success_flag = false;

																					/* date */
																					column_header_s = ConcatenateStrings (key_s, DATE_ENDING_S);
																					if (column_header_s)
																						{
																							const char *date_s = GetJSONString (observation_json_p, column_header_s);

																							if (date_s)
																								{
																									observation_date_p = GetTimeFromString (date_s);
																								}

																							FreeCopiedString (column_header_s);
																						}		/* if (column_header_s) */

																					if (observation_id_p)
																						{
																							observation_p = AllocateObservation (observation_id_p, observation_date_p, treatment_p, raw_value_s, corrected_value_s, growth_stage_s, method_s, instrument_p, nature);

																							if (observation_p)
																								{
																									if (AddObservationToRow (row_p, observation_p))
																										{
																											added_phenotype_flag = true;
																											loop_success_flag = true;
																										}
																									else
																										{
																											char id_s [MONGO_OID_STRING_BUFFER_SIZE];

																											bson_oid_to_string (row_p -> ro_id_p, id_s);

																											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, observation_json_p, "AddObservationToRow failed for row \"%s\" and key \"%s\"", id_s, key_s);
																											FreeObservation (observation_p);
																										}

																								}		/* if (observation_p) */
																							else
																								{
																									char id_s [MONGO_OID_STRING_BUFFER_SIZE];

																									bson_oid_to_string (row_p -> ro_id_p, id_s);

																									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, observation_json_p, "Failed to allocate Observation for row \"%s\" and key \"%s\"", id_s, key_s);

																									FreeBSONOid (observation_id_p);
																								}

																						}		/* if (observation_id_p) */
																					else
																						{
																							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, observation_json_p, "Failed to allocate observation id");
																						}

																					if (observation_date_p)
																						{
																							FreeTime (observation_date_p);
																						}
																				}		/* if ((!IsStringEmpty (raw_value_s)) || (!IsStringEmpty (corrected_value_s))) */
																			else
																				{
																					PrintJSONToLog (STM_LEVEL_INFO, __FILE__, __LINE__, observation_json_p, "No measured value for \"%s\", skipping", key_s);
																				}

																			if (!added_phenotype_flag)
																				{
																					FreeTreatment (treatment_p);
																				}

																		}		/* if (phenotype_p) */
																	else
																		{
																			PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to get phenotype with variable name \"%s\"", key_s);
																		}

																}		/* if (! (DoesStringEndWith (mapped_key_s, "date"))) */

														}		/* if ((strcmp (key_s, S_PLOT_INDEX_S) != 0) && (strcmp (key_s, S_RACK_S) != 0)) */

													iterator_p = json_object_iter_next (observation_json_p, iterator_p);
												}		/* while (iterator_p && loop_success_flag) */

											if (loop_success_flag)
												{
													if (SavePlot (row_p -> ro_plot_p, data_p))
														{
															imported_obeservation_flag = true;
															++ num_imported;
														}
													else
														{
															char id_s [MONGO_OID_STRING_BUFFER_SIZE];

															bson_oid_to_string (study_p -> st_id_p, id_s);
															PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to save row " INT32_FMT " for Study \"%s\"", rack_index, id_s);
														}
												}		/* if (loop_success_flag) */

											FreeRow (row_p);

										}		/*  if (row_p) */
									else
										{
											char id_s [MONGO_OID_STRING_BUFFER_SIZE];

											bson_oid_to_string (study_p -> st_id_p, id_s);
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get row " INT32_FMT " for Study \"%s\"", rack_index, id_s);
										}

								}		/* if (GetIntegerFromJSON (observation_json_p, S_PLOT_INDEX_S, &plot_index)) */
							else
								{
									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, observation_json_p, "Failed to get %s", S_PLOT_INDEX_S);
								}

							if (!imported_obeservation_flag)
								{
									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, observation_json_p, "Failed to import observations");
								}

						}		/* if (row_size > 0) */
					else
						{
							++ num_empty_rows;
						}


				}		/* for (i = 0; i < num_rows; ++ i) */


			if (num_imported + num_empty_rows == num_rows)
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


Row *GetRowByStudyIndex (const int32 by_study_index, Study *study_p, const DFWFieldTrialServiceData *data_p)
{
	Row *row_p = NULL;
	char *index_key_s = ConcatenateVarargsStrings (PL_ROWS_S, ".", RO_STUDY_INDEX_S, NULL);

	if (index_key_s)
		{
			char *study_key_s = ConcatenateVarargsStrings (PL_ROWS_S, ".", RO_STUDY_ID_S, NULL);

			if (study_key_s)
				{
					bson_t *query_p = BCON_NEW (index_key_s, BCON_INT32 (by_study_index), study_key_s, BCON_OID (study_p -> st_id_p));

					if (query_p)
						{
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
															json_t *plot_json_p = json_array_get (results_p, i);
															Plot *plot_p = GetPlotFromJSON (plot_json_p, study_p, data_p);

															if (plot_p)
																{
																	row_p = GetRowFromPlotByStudyIndex (plot_p, by_study_index);

																	if (!row_p)
																		{
																			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, plot_json_p, "Failed to get row with study index " UINT32_FMT,  by_study_index);
																			FreePlot (plot_p);
																		}
																}
															else
																{
																	PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, plot_json_p, "Failed to create plot");
																}

														}		/* if (num_results == 1) */
													else
														{
															char *query_json_s = bson_as_json (query_p, NULL);

															if (query_json_s)
																{
																	PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, results_p, UINT32_FMT " results for \"%s\"", query_json_s);
																	free (query_json_s);
																}
															else
																{
																	PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, results_p, UINT32_FMT " for row " UINT32_FMT " in study \"%s\"", by_study_index, study_p -> st_name_s);
																}
														}

												}		/* if (json_is_array (results_p)) */

											json_decref (results_p);
										}		/* if (results_p) */
									else
										{
											PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "No results for row " UINT32_FMT " in study \"%s\"", by_study_index, study_p -> st_name_s);
										}

								}		/* if (SetMongoToolCollection (data_p -> dftsd_mongo_p, data_p -> dftsd_collection_ss [DFTD_ROW]) */

							bson_destroy (query_p);
						}		/* if (query_p) */
					else
						{
							char *study_id_s = GetBSONOidAsString (study_p -> st_id_p);

							if (study_id_s)
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to create query for row " UINT32_FMT " in study \"%s\"", by_study_index, study_id_s);
									FreeCopiedString (study_id_s);
								}
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to create query for row " UINT32_FMT " in study \"%s\"", study_p -> st_name_s);
								}
						}

					FreeCopiedString (study_key_s);
				}		/* if (study_key_s) */
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to concatenate \"%s\", \".\" and \"%s\"", PL_ROWS_S, RO_STUDY_ID_S);
				}

			FreeCopiedString (index_key_s);
		}		/* if (index_key_s) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to concatenate \"%s\", \".\" and \"%s\"", PL_ROWS_S, RO_STUDY_INDEX_S);
		}

	return row_p;
}



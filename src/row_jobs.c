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


#include "row_jobs.h"
#include "plot_jobs.h"
#include "phenotype_jobs.h"
#include "string_utils.h"
#include "experimental_area_jobs.h"
#include "math_utils.h"
#include "time_util.h"
#include "observation.h"


/*
 * static declarations
 */

static const char S_DEFAULT_COLUMN_DELIMITER =  '|';

static const char * const S_ROW_S = "Row";
static const char * const S_COLUMN_S = "Column";
static const char * const S_RACK_S = "Rack";


static NamedParameterType S_ROW_PHENOTYPE_DATA_TABLE_COLUMN_DELIMITER = { "RO phenotype data delimiter", PT_CHAR };
static NamedParameterType S_ROW_PHENOTYPE_DATA_TABLE = { "RO phenotype data upload", PT_TABLE};
static NamedParameterType S_EXPERIMENTAL_AREAS_LIST = { "RO Experimental Area", PT_STRING };


static Parameter *GetPhenotypesDataTableParameter (ParameterSet *param_set_p, ParameterGroup *group_p, const DFWFieldTrialServiceData *data_p);

static bool AddObservationValuesFromJSON (ServiceJob *job_p, const json_t *observations_json_p, ExperimentalArea *area_p, const DFWFieldTrialServiceData *data_p);

static bool GetIntegerFromJSON (const json_t * const json_p, const char * const key_s, int *value_p);


/*
 * API Definitions
 */


bool AddSubmissionRowPhenotypeParams (ServiceData *data_p, ParameterSet *param_set_p)
{
	bool success_flag = false;

	ParameterGroup *group_p = CreateAndAddParameterGroupToParameterSet ("Row Phenotypes", NULL, false, data_p, param_set_p);

	if (group_p)
		{
			Parameter *param_p = NULL;
			SharedType def;
			const DFWFieldTrialServiceData *dfw_service_data_p = (DFWFieldTrialServiceData *) data_p;

			InitSharedType (&def);

			if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_EXPERIMENTAL_AREAS_LIST.npt_type, S_EXPERIMENTAL_AREAS_LIST.npt_name_s, "Experimental Areas", "The available experimental areas", def, PL_ALL)) != NULL)
				{
					if (SetUpExperimentalAreasListParameter (dfw_service_data_p, param_p))
						{
							def.st_char_value = S_DEFAULT_COLUMN_DELIMITER;

							if ((param_p = CreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_ROW_PHENOTYPE_DATA_TABLE_COLUMN_DELIMITER.npt_type, false, S_ROW_PHENOTYPE_DATA_TABLE_COLUMN_DELIMITER.npt_name_s, "Delimiter", "The character delimiting columns", NULL, def, NULL, NULL, PL_ADVANCED, NULL)) != NULL)
								{
									def.st_string_value_s = NULL;

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

	SharedType value;
	InitSharedType (&value);

	if (GetParameterValueFromParameterSet (param_set_p, S_ROW_PHENOTYPE_DATA_TABLE.npt_name_s, &value, true))
		{
			/*
			 * Has a spreadsheet been uploaded?
			 */
			if (! (IsStringEmpty (value.st_string_value_s)))
				{
					bool success_flag = false;
					json_error_t e;
					json_t *observations_json_p = NULL;

					job_done_flag = true;

					/*
					 * The data could be either an array of json objects
					 * or a tabular string. so try it as json array first
					 */
					observations_json_p = json_loads (value.st_string_value_s, 0, &e);

					if (observations_json_p)
						{
							SharedType parent_experimental_area_value;
							InitSharedType (&parent_experimental_area_value);

							if (GetParameterValueFromParameterSet (param_set_p, S_EXPERIMENTAL_AREAS_LIST.npt_name_s, &parent_experimental_area_value, true))
								{
									ExperimentalArea *area_p = GetExperimentalAreaByIdString (parent_experimental_area_value.st_string_value_s, data_p);

									if (area_p)
										{
											if (AddObservationValuesFromJSON (job_p, observations_json_p, area_p, data_p))
												{
													success_flag = true;
												}
											else
												{
													char area_id_s [MONGO_OID_STRING_BUFFER_SIZE];

													PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, observations_json_p, "AddObservationValuesFromJSON for failed");
												}

											FreeExperimentalArea (area_p);
										}		/* if (area_p) */

								}		/* if (GetParameterValueFromParameterSet (param_set_p, S_EXPERIMENTAL_AREAS_LIST.npt_name_s, &parent_experimental_area_value, true)) */

							json_decref (observations_json_p);
						}		/* if (observations_json_p) */
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to load \"%s\" as JSON", value.st_string_value_s);
						}

					job_done_flag = true;
				}		/* if (! (IsStringEmpty (value.st_string_value_s))) */

		}		/* if (GetParameterValueFromParameterSet (param_set_p, S_PHENOTYPE_TABLE.npt_name_s, &value, true)) */


	return job_done_flag;
}




/*
 * static definitions
 */


static Parameter *GetPhenotypesDataTableParameter (ParameterSet *param_set_p, ParameterGroup *group_p, const DFWFieldTrialServiceData *data_p)
{
	Parameter *param_p = NULL;
	const char delim_s [2] = { S_DEFAULT_COLUMN_DELIMITER, '\0' };
	char *headers_s = NULL;

	headers_s = ConcatenateVarargsStrings (S_ROW_S, delim_s, S_COLUMN_S, delim_s, S_RACK_S, delim_s, NULL);

	if (headers_s)
		{
			SharedType def;

			InitSharedType (&def);
			def.st_string_value_s = NULL;

			param_p = CreateAndAddParameterToParameterSet (& (data_p -> dftsd_base_data), param_set_p, group_p, S_ROW_PHENOTYPE_DATA_TABLE.npt_type, false, S_ROW_PHENOTYPE_DATA_TABLE.npt_name_s, "Phenotype data values to upload", "The data to upload", NULL, def, NULL, NULL, PL_ALL, NULL);

			if (param_p)
				{
					bool success_flag = false;

					if (AddParameterKeyValuePair (param_p, PA_TABLE_COLUMN_HEADINGS_S, headers_s))
						{
							if (AddParameterKeyValuePair (param_p, PA_TABLE_COLUMN_DELIMITER_S, delim_s))
								{
									if (AddParameterKeyValuePair (param_p, PA_TABLE_COLUMN_HEADERS_PLACEMENT_S, PA_TABLE_COLUMN_HEADERS_PLACEMENT_FIRST_ROW_S))
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

			FreeCopiedString (headers_s);
		}		/* if (headers_s) */

	return param_p;
}




static bool AddObservationValuesFromJSON (ServiceJob *job_p, const json_t *observations_json_p, ExperimentalArea *area_p, const DFWFieldTrialServiceData *data_p)
{
	bool success_flag	= true;
	OperationStatus status = OS_FAILED;

	if (json_is_array (observations_json_p))
		{
			const size_t num_rows = json_array_size (observations_json_p);
			size_t i;
			size_t num_imported = 0;
			size_t num_empty_rows = 0;

			for (i = 0; i < num_rows; ++ i)
				{
					json_t *table_row_json_p = json_array_get (observations_json_p, i);

					const size_t row_size = json_object_size (table_row_json_p);

					if (row_size > 0)
						{
							int32 row = -1;

							if (GetIntegerFromJSON (table_row_json_p, S_ROW_S, &row))
								{
									int32 column = -1;

									if (GetIntegerFromJSON (table_row_json_p, S_COLUMN_S, &column))
										{
											int32 rack = -1;

											if (GetIntegerFromJSON (table_row_json_p, S_RACK_S, &rack))
												{
													Plot *plot_p = GetPlotByRowAndColumn (row, column, area_p, data_p);

													if (plot_p)
														{
															const bool expand_fields_flag = true;
															Row *row_p = GetRowByIndex (rack, plot_p, expand_fields_flag, data_p);

															if (row_p)
																{
																	bool loop_success_flag = true;
																	void *iterator_p = json_object_iter (table_row_json_p);

																	while (iterator_p && loop_success_flag)
																		{
																			const char *key_s = json_object_iter_key (iterator_p);
																			json_t *value_p = json_object_iter_value (iterator_p);

																			/*
																			 * ignore our column names
																			 */
																			if ((strcmp (key_s, S_ROW_S) != 0) && (strcmp (key_s, S_COLUMN_S) != 0) && (strcmp (key_s, S_RACK_S) != 0))
																				{
																					/*
																					 * make sure it isn't a date column
																					 */
																					const char * const DATE_ENDING_S = " date";

																					if (! (DoesStringEndWith (key_s, DATE_ENDING_S)))
																						{
																							Phenotype *phenotype_p = GetPhenotypeByInternalName (key_s, data_p);

																							if (phenotype_p)
																								{
																									struct tm *observation_date_p = NULL;
																									Observation *observation_p = NULL;
																									bool added_phenotype_flag = false;
																									char *date_column_header_s = ConcatenateStrings (key_s, DATE_ENDING_S);

																									if (date_column_header_s)
																										{
																											const char *date_s = GetJSONString (table_row_json_p, date_column_header_s);

																											if (date_s)
																												{
																													observation_date_p = GetTimeFromString (date_s);
																												}

																											FreeCopiedString (date_column_header_s);
																										}		/* if (date_column_header_s) */

																									if (json_is_string (value_p))
																										{
																											const char *value_s = json_string_value (value_p);
																											const char *growth_stage_s = NULL;
																											bool corrected_flag = false;
																											const char *method_s = NULL;
																											ObservationNature nature = ON_ROW;
																											Instrument *instrument_p = NULL;
																											bson_oid_t *observation_id_p = GetNewBSONOid ();

																											if (observation_id_p)
																												{
																													observation_p = AllocateObservation (observation_id_p, observation_date_p, phenotype_p, value_s, growth_stage_s, corrected_flag, method_s, instrument_p, nature);

																													if (observation_p)
																														{
																															if (AddObservationToRow (row_p, observation_p))
																																{
																																	added_phenotype_flag = true;
																																}
																															else
																																{
																																	char id_s [MONGO_OID_STRING_BUFFER_SIZE];

																																	bson_oid_to_string (row_p -> ro_id_p, id_s);

																																	PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to set  value for row \"%s\" to \"%s\"", id_s, value_s);
																																	FreeObservation (observation_p);
																																}

																														}		/* if (observation_p) */
																													else
																														{
																															char id_s [MONGO_OID_STRING_BUFFER_SIZE];

																															bson_oid_to_string (row_p -> ro_id_p, id_s);

																															PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate Observation for row \"%s\" to \"%s\"", id_s, value_s);

																															FreeBSONOid (observation_id_p);
																														}

																												}		/* if (observation_id_p) */
																											else
																												{
																													PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, table_row_json_p, "Failed to allocate observation id");
																												}


																										}		/* if (json_is_string (value_p)) */
																									else
																										{
																											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, table_row_json_p, "Value for \"%s\" is not a string", key_s);
																										}

																									if (!added_phenotype_flag)
																										{
																											loop_success_flag = false;
																											FreePhenotype (phenotype_p);
																										}


																									if (observation_date_p)
																										{
																											FreeTime (observation_date_p);
																										}

																								}		/* if (phenotype_p) */
																							else
																								{
																									PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to get phenotype with internal name \"%s\"", key_s);
																								}

																						}		/* if (! (DoesStringEndWith (mapped_key_s, "date"))) */

																				}		/* if ((strcmp (key_s, S_ROW_S) != 0) && (strcmp (key_s, S_COLUMN_S) != 0) && (strcmp (key_s, S_RACK_S) != 0)) */


																			iterator_p = json_object_iter_next (table_row_json_p, iterator_p);
																		}

																	if (loop_success_flag)
																		{
																			if (SaveRow (row_p, data_p, false))
																				{
																					++ num_imported;
																				}
																			else
																				{
																					char id_s [MONGO_OID_STRING_BUFFER_SIZE];

																					bson_oid_to_string (plot_p -> pl_id_p, id_s);
																					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to save row " INT32_FMT " for plot_p \"%s\"", rack, id_s);
																				}
																		}

																	FreeRow (row_p);
																}		/* if (row_p) */
															else
																{
																	char id_s [MONGO_OID_STRING_BUFFER_SIZE];

																	bson_oid_to_string (plot_p -> pl_id_p, id_s);
																	PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get row " INT32_FMT " for plot_p \"%s\"", rack, id_s);
																}

															FreePlot (plot_p);
														}		/* if (plot_p) */
													else
														{
															char id_s [MONGO_OID_STRING_BUFFER_SIZE];

															bson_oid_to_string (area_p -> ea_id_p, id_s);
															PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get plot at [" INT32_FMT ", " INT32_FMT "] for area \"%s\"", row, column, id_s);
														}


												}		/* if (GetJSONInteger (table_row_json_p, S_RACK_S, &rack)) */
											else
												{
													PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, table_row_json_p, "Failed to get %s", S_RACK_S);
												}

										}		/* if (GetJSONInteger (table_row_json_p, S_COLUMN_S, &column)) */
									else
										{
											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, table_row_json_p, "Failed to get %s", S_COLUMN_S);
										}

								}		/* if (GetJSONInteger (table_row_json_p, S_ROW_S, &row)) */
							else
								{
									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, table_row_json_p, "Failed to get %s", S_ROW_S);
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



Row *GetRowByIndex (const int32 row, Plot *plot_p, const bool expand_fields_flag, const DFWFieldTrialServiceData *data_p)
{
	Row *row_p = NULL;

	if (SetMongoToolCollection (data_p -> dftsd_mongo_p, data_p -> dftsd_collection_ss [DFTD_ROW]))
		{
			bson_t *query_p = BCON_NEW (RO_INDEX_S, BCON_INT32 (row), RO_PLOT_ID_S, BCON_OID (plot_p -> pl_id_p));

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

											row_p = GetRowFromJSON (entry_p, plot_p, NULL, expand_fields_flag, data_p);

											if (!row_p)
												{

												}

										}		/* if (num_results == 1) */

								}		/* if (json_is_array (results_p)) */

							json_decref (results_p);
						}		/* if (results_p) */

					bson_destroy (query_p);
				}		/* if (query_p) */

		}		/* if (SetMongoToolCollection (data_p -> dftsd_mongo_p, data_p -> dftsd_collection_ss [DFTD_PLOT])) */

	return row_p;
}


static bool GetIntegerFromJSON (const json_t * const json_p, const char * const key_s, int *value_p)
{
	bool success_flag = false;

	if (GetJSONInteger (json_p, key_s, value_p))
		{
			success_flag = true;
		}
	else
		{
			const char *value_s = GetJSONString (json_p, key_s);

			if (value_s)
				{
					if (GetValidInteger (&value_s, value_p))
						{
							success_flag = true;
						}
				}
		}

	return success_flag;
}

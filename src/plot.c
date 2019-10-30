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
 * plot.c
 *
 *  Created on: 28 Sep 2018
 *      Author: billy
 */


#define ALLOCATE_PLOT_TAGS (1)
#include "plot.h"
#include "memory_allocations.h"
#include "string_utils.h"
#include "row.h"
#include "dfw_util.h"
#include "time_util.h"


static bool AddRowsToJSON (const Plot *plot_p, json_t *plot_json_p, const DFWFieldTrialServiceData *data_p);





Plot *AllocatePlot (bson_oid_t *id_p, const struct tm *sowing_date_p, const struct tm *harvest_date_p, const double64 width, const double64 length, const uint32 plot_index, const uint32 row_index,
										const uint32 column_index, const uint32 replicate, const char *trial_design_s, const char *growing_conditions_s, const char *treatments_s, const char *comment_s, Study *parent_p)
{
	char *copied_trial_design_s = NULL;

	if ((IsStringEmpty (trial_design_s)) || ((copied_trial_design_s = EasyCopyToNewString (trial_design_s)) != NULL))
		{
			char *copied_growing_conditions_s = NULL;

			if ((IsStringEmpty (growing_conditions_s)) || ((copied_growing_conditions_s = EasyCopyToNewString (growing_conditions_s)) != NULL))
				{
					char *copied_treatments_s = NULL;

					if ((IsStringEmpty (treatments_s)) || ((copied_trial_design_s = EasyCopyToNewString (treatments_s)) != NULL))
						{
							char *copied_comment_s = NULL;

							if ((IsStringEmpty (comment_s)) || ((copied_comment_s = EasyCopyToNewString (comment_s)) != NULL))
								{
									struct tm *copied_sowing_date_p = NULL;

									if (CopyValidDate (sowing_date_p, &copied_sowing_date_p))
										{
											struct tm *copied_harvest_date_p = NULL;

											if (CopyValidDate (harvest_date_p, &copied_harvest_date_p))
												{
													LinkedList *rows_p = AllocateLinkedList (FreeRowNode);

													if (rows_p)
														{
															Plot *plot_p = (Plot *) AllocMemory (sizeof (Plot));

															if (plot_p)
																{
																	plot_p -> pl_id_p = id_p;
																	plot_p -> pl_sowing_date_p = copied_sowing_date_p;
																	plot_p -> pl_harvest_date_p = copied_harvest_date_p;
																	plot_p -> pl_width = width;
																	plot_p -> pl_length = length;
																	plot_p -> pl_index = plot_index;
																	plot_p -> pl_row_index = row_index;
																	plot_p -> pl_column_index = column_index;

																	plot_p -> pl_replicate_index = replicate;
																	plot_p -> pl_replicate_control_flag = false;
																	plot_p -> pl_parent_p = parent_p;
																	plot_p -> pl_rows_p = rows_p;
																	plot_p -> pl_growing_conditions_s = copied_growing_conditions_s;

																	plot_p -> pl_comment_s = copied_comment_s;

																	//plot_p -> pl_treatments_s = copied_treatments_s;
																	plot_p -> pl_trial_design_s = copied_trial_design_s;

																	plot_p -> pl_accession_s = NULL;
																	plot_p -> pl_soil_type_s = NULL;
																	plot_p -> pl_sowing_rate = 0.0f;
																	return plot_p;
																}		/* if (plot_p) */

															FreeLinkedList (rows_p);
														}		/* if (rows_p) */

													if (copied_harvest_date_p)
														{
															FreeTime (copied_harvest_date_p);
														}

												}		/* if (CopyValidDate (harvest_date_p, &copied_harvest_date_p)) */

											if (copied_sowing_date_p)
												{
													FreeTime (copied_sowing_date_p);
												}

										}		/* if (CopyValidDate (sowing_date_p, &copied_sowing_date_p)) */

								}		/* if ((IsStringEmpty (comment_s)) || ((copied_comment_s = EasyCopyToNewString (comment_s)) != NULL)) */


							if (copied_treatments_s)
								{
									FreeCopiedString (copied_treatments_s);
								}

						}		/* if ((IsStringEmpty (treatments_s)) || ((copied_treatments_s = EasyCopyToNewString (treatments_s)) != NULL)) */

					if (copied_growing_conditions_s)
						{
							FreeCopiedString (copied_growing_conditions_s);
						}

				}		/* if ((IsStringEmpty (growing_conditions_)) || ((copied_growing_conditions_s = EasyCopyToNewString (growing_conditions_)) != NULL)) */

			if (copied_trial_design_s)
				{
					FreeCopiedString (copied_trial_design_s);
				}

		}		/* if ((IsStringEmpty (trial_design_s)) || ((copied_trial_design_s = EasyCopyToNewString (trial_design_s)) != NULL)) */

	return NULL;
}




void FreePlot (Plot *plot_p)
{
	if (plot_p -> pl_id_p)
		{
			FreeBSONOid (plot_p -> pl_id_p);
		}

/*
	if (plot_p -> pl_trial_design_s)
		{
			FreeCopiedString (plot_p -> pl_trial_design_s);
		}
*/

	if (plot_p -> pl_growing_conditions_s)
		{
			FreeCopiedString (plot_p -> pl_growing_conditions_s);
		}


	if (plot_p -> pl_comment_s)
		{
			FreeCopiedString (plot_p -> pl_comment_s);
		}

/*
	if (plot_p -> pl_treatments_s)
		{
			FreeCopiedString (plot_p -> pl_treatments_s);
		}
*/

	FreeLinkedList (plot_p -> pl_rows_p);

	if (plot_p -> pl_sowing_date_p)
		{
			FreeTime (plot_p -> pl_sowing_date_p);
		}


	if (plot_p -> pl_harvest_date_p)
		{
			FreeTime (plot_p -> pl_harvest_date_p);
		}

	FreeMemory (plot_p);
}


PlotNode *AllocatePlotNode (Plot *plot_p)
{
	PlotNode *pl_node_p = (PlotNode *) AllocMemory (sizeof (PlotNode));

	if (pl_node_p)
		{
			InitListItem (& (pl_node_p -> pn_node));

			pl_node_p -> pn_plot_p = plot_p;
		}

	return pl_node_p;
}



void FreePlotNode (ListItem *node_p)
{
	PlotNode *pl_node_p = (PlotNode *) node_p;

	if (pl_node_p -> pn_plot_p)
		{
			FreePlot (pl_node_p -> pn_plot_p);
		}

	FreeMemory (pl_node_p);
}


bool SavePlot (Plot *plot_p, const DFWFieldTrialServiceData *data_p)
{
	bson_t *selector_p = NULL;
	bool success_flag = PrepareSaveData (& (plot_p -> pl_id_p), &selector_p);

	if (success_flag)
		{
			json_t *plot_json_p = GetPlotAsJSON (plot_p, false, data_p);

			if (plot_json_p)
				{
					success_flag = SaveMongoData (data_p -> dftsd_mongo_p, plot_json_p, data_p -> dftsd_collection_ss [DFTD_PLOT], selector_p);

					json_decref (plot_json_p);
				}		/* if (plot_json_p) */

		}		/* if (plot_p -> pl_id_p) */

	return success_flag;
}


json_t *GetPlotAsJSON (Plot *plot_p, const ViewFormat format, const DFWFieldTrialServiceData *data_p)
{
	json_t *plot_json_p = json_object ();

	if (plot_json_p)
		{
			if ((plot_p -> pl_index == PL_UNSET_ID) || (SetJSONInteger (plot_json_p, PL_INDEX_S, plot_p -> pl_index)))
				{
					if (SetJSONInteger (plot_json_p, PL_ROW_INDEX_S, plot_p -> pl_row_index))
						{
							if (SetJSONInteger (plot_json_p, PL_COLUMN_INDEX_S, plot_p -> pl_column_index))
								{
									bool success_flag = false;

									if (plot_p -> pl_replicate_control_flag)
										{
											success_flag = SetJSONString (plot_json_p, PL_REPLICATE_S, PL_REPLICATE_CONTROL_S);
										}
									else
										{
											success_flag = SetJSONInteger (plot_json_p, PL_REPLICATE_S, plot_p -> pl_replicate_index);
										}

									if (success_flag)
										{
											if (SetJSONReal (plot_json_p, PL_WIDTH_S, plot_p -> pl_width))
												{
													if (SetJSONReal (plot_json_p, PL_LENGTH_S, plot_p -> pl_length))
														{
															if ((IsStringEmpty (plot_p -> pl_growing_conditions_s)) || (SetJSONString (plot_json_p, PL_GROWING_CONDITION_S, plot_p -> pl_growing_conditions_s)))
																{
																	if ((IsStringEmpty (plot_p -> pl_comment_s)) || (SetJSONString (plot_json_p, PL_COMMENT_S, plot_p -> pl_comment_s)))
																		{
																			if ((IsStringEmpty (plot_p -> pl_trial_design_s)) || (SetJSONString (plot_json_p, PL_TRIAL_DESIGN_S, plot_p -> pl_trial_design_s)))
																				{
																					if (AddValidDateToJSON (plot_p -> pl_sowing_date_p, plot_json_p, PL_SOWING_DATE_S))
																						{
																							if (AddValidDateToJSON (plot_p -> pl_harvest_date_p, plot_json_p, PL_HARVEST_DATE_S))
																								{
																									if (AddCompoundIdToJSON (plot_json_p, plot_p -> pl_id_p))
																										{
																											success_flag = false;

																											switch (format)
																												{
																													case VF_CLIENT_FULL:
																														{
																															if (GetPlotRows (plot_p, data_p))
																																{
																																	if (AddRowsToJSON (plot_p, plot_json_p, data_p))
																																		{
																																			success_flag = true;
																																		}
																																}
																														}		/* case VF_CLIENT_FULL: */
																														break;

																													case VF_STORAGE:
																														{
																															if (AddNamedCompoundIdToJSON (plot_json_p, plot_p -> pl_parent_p -> st_id_p, PL_PARENT_STUDY_S))
																																{
																																	success_flag = true;
																																}		/* if (AddNamedCompoundIdToJSON (plot_json_p, plot_p -> pl_parent_p -> st_id_p, PL_PARENT_FIELD_TRIAL_S)) */
																															else
																																{
																																	PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, plot_json_p, "Failed to add id for \"%s\"", PL_PARENT_STUDY_S);
																																}
																														}		/* case VF_STORAGE */
																														break;

																													default:
																														break;

																												}		/* switch (format) */

																											if (success_flag)
																												{
																													if (AddDatatype (plot_json_p, DFTD_PLOT))
																														{
																															return plot_json_p;
																														}
																												}

																										}		/* if (AddCompoundIdToJSON (plot_json_p, plot_p -> pl_id_p)) */
																									else
																										{
																											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, plot_json_p, "Failed to add id");
																										}

																								}		/* if (AddValidDateToJSON (plot_p -> pl_harvest_date_p, plot_json_p, PL_HARVEST_DATE_S)) */
																							else
																								{
																									char *time_s = NULL;

																									if (plot_p -> pl_harvest_date_p)
																										{
																											time_s = GetTimeAsString (plot_p -> pl_harvest_date_p, false);
																										}

																									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, plot_json_p, "Failed to add \"%s\": \"%s\"", PL_HARVEST_DATE_S, time_s ? time_s : "");

																									if (time_s)
																										{
																											FreeCopiedString (time_s);
																										}
																								}

																						}		/* if (AddValidDateToJSON (plot_p -> pl_sowing_date_p, plot_json_p, PL_SOWING_DATE_S)) */
																					else
																						{
																							char *time_s = NULL;

																							if (plot_p -> pl_sowing_date_p)
																								{
																									time_s = GetTimeAsString (plot_p -> pl_sowing_date_p, false);
																								}

																							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, plot_json_p, "Failed to add \"%s\": \"%s\"", PL_SOWING_DATE_S, time_s ? time_s : "");

																							if (time_s)
																								{
																									FreeCopiedString (time_s);
																								}
																						}

																				}		/* if ((IsStringEmpty (plot_p -> pl_trial_design_s)) || (SetJSONString (plot_json_p, PL_TRIAL_DESIGN_S, plot_p -> pl_trial_design_s))) */
																			else
																				{
																					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, plot_json_p, "Failed to add \"%s\": \"%s\"", PL_TRIAL_DESIGN_S, plot_p -> pl_trial_design_s);
																				}

																		}		/* if ((IsStringEmpty (plot_p -> pl_comment_s)) || (SetJSONString (plot_json_p, PL_COMMENT_S, plot_p -> pl_comment_s))) */
																	else
																		{
																			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, plot_json_p, "Failed to add \"%s\": \"%s\"", PL_COMMENT_S, plot_p -> pl_comment_s);
																		}

																}		/* if ((IsStringEmpty (plot_p -> pl_growing_conditions_s)) || (SetJSONString (plot_json_p, PL_GROWING_CONDITION_S, plot_p -> pl_growing_conditions_s))) */
															else
																{
																	PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, plot_json_p, "Failed to add \"%s\": \"%s\"", PL_GROWING_CONDITION_S, plot_p -> pl_growing_conditions_s);
																}

														}		/* if (SetJSONReal (plot_json_p, PL_LENGTH_S, plot_p -> pl_length)) */
													else
														{
															PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, plot_json_p, "Failed to add \"%s\": " DOUBLE64_FMT, PL_LENGTH_S, plot_p -> pl_length);
														}

												}		/* if (SetJSONReal (plot_json_p, PL_WIDTH_S, plot_p -> pl_width)) */
											else
												{
													PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, plot_json_p, "Failed to add \"%s\": " DOUBLE64_FMT, PL_WIDTH_S, plot_p -> pl_width);
												}

										}		/* if ((! (plot_p -> pl_replicate_index_p)) || (SetJSONInteger (plot_json_p, PL_REPLICATE_INDEX_S, * (plot_p -> pl_replicate_index_p)))) */
									else
										{
											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, plot_json_p, "Failed to add \"%s\": " UINT32_FMT " control: \%s", PL_REPLICATE_S, plot_p -> pl_replicate_index, plot_p -> pl_replicate_control_flag ? "true": "false");
										}

								}		/* if (SetJSONInteger (plot_json_p, PL_COLUMN_INDEX_S, plot_p -> pl_column_index)) */
							else
								{
									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, plot_json_p, "Failed to add \"%s\": " UINT32_FMT, PL_COLUMN_INDEX_S, plot_p -> pl_column_index);
								}

						}		/* if (SetJSONInteger (plot_json_p, PL_ROW_INDEX_S, plot_p -> pl_row_index)) */
					else
						{
							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, plot_json_p, "Failed to add \"%s\": " UINT32_FMT, PL_ROW_INDEX_S, plot_p -> pl_row_index);
						}

				}		/* if (SetJSONInteger (plot_json_p, PL_INDEX_S, plot_p -> pl_index) */
			else
				{
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, plot_json_p, "Failed to add \"%s\": " UINT32_FMT, PL_INDEX_S, plot_p -> pl_index);
				}

			json_decref (plot_json_p);
		}		/* if (plot_json_p) */

	return NULL;
}



Plot *GetPlotFromJSON (const json_t *plot_json_p, Study *parent_area_p, const DFWFieldTrialServiceData *data_p)
{
	Plot *plot_p = NULL;
	uint32 index = PL_UNSET_ID;
	int32 row;

	GetJSONInteger (plot_json_p, PL_INDEX_S, &index);


	if (GetJSONInteger (plot_json_p, PL_ROW_INDEX_S, &row))
		{
			int32 column;

			if (GetJSONInteger (plot_json_p, PL_COLUMN_INDEX_S, &column))
				{
					double64 width;

					if (GetJSONReal (plot_json_p, PL_WIDTH_S, &width))
						{
							double64 length;

							if (GetJSONReal (plot_json_p, PL_LENGTH_S, &length))
								{
									const char *growing_conditions_s = GetJSONString (plot_json_p, PL_GROWING_CONDITION_S);
									const char *treatments_s = GetJSONString (plot_json_p, PL_TREATMENT_S);
									const char *trial_design_s = GetJSONString (plot_json_p, PL_TRIAL_DESIGN_S);
									const char *comment_s = GetJSONString (plot_json_p, PL_COMMENT_S);
									struct tm *sowing_date_p = NULL;
									uint32 replicate = 1;
									bool rep_control_flag = false;
									const json_t *rep_json_p = json_object_get (plot_json_p, PL_REPLICATE_S);

									if (rep_json_p)
										{
											if (json_is_string (rep_json_p))
												{
													const char *rep_s = json_string_value (rep_json_p);

													if (rep_s)
														{
															if (Stricmp (rep_s, PL_REPLICATE_CONTROL_S) == 0)
																{
																	rep_control_flag = true;
																}
															else
																{
																	PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, plot_json_p, "Invalid replicate value \"%s\"", rep_s);
																}
														}
													else
														{
															PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, plot_json_p, "Missing replicate value");
														}
												}
											else if (json_is_integer (rep_json_p))
												{
													replicate = json_integer_value (rep_json_p);
												}
										}


									if (CreateValidDateFromJSON (plot_json_p, PL_SOWING_DATE_S, &sowing_date_p))
										{
											struct tm *harvest_date_p = NULL;

											if (CreateValidDateFromJSON (plot_json_p, PL_HARVEST_DATE_S, &harvest_date_p))
												{
													bson_oid_t *id_p = GetNewUnitialisedBSONOid ();

													if (id_p)
														{
															if (GetMongoIdFromJSON (plot_json_p, id_p))
																{
																	if (!parent_area_p)
																		{
																			bson_oid_t *parent_area_id_p = GetNewUnitialisedBSONOid ();

																			if (parent_area_id_p)
																				{
																					if (GetNamedIdFromJSON (plot_json_p, PL_PARENT_STUDY_S, parent_area_id_p))
																						{


																						}		/* if (GetNamedIdFromJSON (plot_json_p, PL_PARENT_FIELD_TRIAL_S, field_trial_id_p)) */
																					else
																						{
																							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, plot_json_p, "Failed to get id for \"%s\"", PL_PARENT_STUDY_S);
																						}

																				}		/* if (parent_area_id_p) */
																			else
																				{
																					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate id for \"%s\"", PL_PARENT_STUDY_S);
																				}

																		}		/* if (!parent_area_p) */

																	plot_p = AllocatePlot (id_p, sowing_date_p, harvest_date_p, width, length, index, row, column, replicate, trial_design_s, growing_conditions_s, treatments_s, comment_s, parent_area_p);

																	if (plot_p)
																		{
																			if (rep_control_flag)
																				{
																					SetPlotGenotypeControl (plot_p, true);
																				}
																		}
																	else
																		{
																			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, plot_json_p, "Failed to get create Plot");
																		}

																}		/* if (GetMongoIdFromJSON (plot_json_p, id_p)) */
															else
																{
																	PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, plot_json_p, "Failed to get id for \"%s\"", MONGO_ID_S);
																}

														}		/* if (id_p) */
													else
														{
															PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate id for \"%s\"", MONGO_ID_S);
														}

												}		/* if (CreateValidDateFromJSON (plot_json_p, PL_HARVEST_DATE_S, &harvest_date_p)) */
											else
												{
													PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, plot_json_p, "Failed to get time from \"%s\"", PL_HARVEST_DATE_S);
												}

										}		/* if (CreateValidDateFromJSON (plot_json_p, PL_SOWING_DATE_S, &sowing_date_p)) */
									else
										{
											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, plot_json_p, "Failed to get time from \"%s\"", PL_HARVEST_DATE_S);
										}

								}		/* if (GetJSONReal (plot_json_p, PL_LENGTH_S, &length)) */
							else
								{
									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, plot_json_p, "Failed to get \"%s\": ", PL_LENGTH_S);
								}

						}		/* if (GetJSONReal (plot_json_p, PL_WIDTH_S, &width)) */
					else
						{
							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, plot_json_p, "Failed to get \"%s\": ", PL_WIDTH_S);
						}

				}		/* if (GetJSONInteger (plot_json_p, PL_COLUMN_INDEX_S, plot_p -> pl_column_index)) */
			else
				{
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, plot_json_p, "Failed to get \"%s\": ", PL_COLUMN_INDEX_S);
				}

		}		/* if (GetJSONInteger (plot_json_p, PL_ROW_INDEX_S, &row)) */
	else
		{
			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, plot_json_p, "Failed to get \"%s\": ", PL_ROW_INDEX_S);
		}



	return plot_p;
}


bool GetPlotRows (Plot *plot_p, const DFWFieldTrialServiceData *data_p)
{
	bool success_flag = false;

	ClearLinkedList (plot_p -> pl_rows_p);

	if (SetMongoToolCollection (data_p -> dftsd_mongo_p, data_p -> dftsd_collection_ss [DFTD_ROW]))
		{
			bson_t *query_p = BCON_NEW (RO_PLOT_ID_S, BCON_OID (plot_p -> pl_id_p));

			/*
			 * Make the query to get the matching plots
			 */
			if (query_p)
				{
					bson_t *opts_p = BCON_NEW ( "sort", "{", RO_INDEX_S, BCON_INT32 (1), "}");

					if (opts_p)
						{
							json_t *results_p = GetAllMongoResultsAsJSON (data_p -> dftsd_mongo_p, query_p, opts_p);

							if (results_p)
								{
									if (json_is_array (results_p))
										{
											size_t i;
											const size_t num_results = json_array_size (results_p);

											success_flag = true;

											if (num_results > 0)
												{
													json_t *row_json_p;

													json_array_foreach (results_p, i, row_json_p)
														{
															Row *row_p = GetRowFromJSON (row_json_p, plot_p, NULL, true, data_p);

															if (row_p)
																{
																	RowNode *node_p = AllocateRowNode (row_p);

																	if (node_p)
																		{
																			LinkedListAddTail (plot_p -> pl_rows_p, & (node_p -> rn_node));
																		}
																	else
																		{
																			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, row_json_p, "Failed to add row to plot's list");
																			FreePlot (plot_p);
																		}
																}

														}		/* json_array_foreach (results_p, i, entry_p) */

												}		/* if (num_results > 0) */
											else
												{
													char id_s [MONGO_OID_STRING_BUFFER_SIZE];

													bson_oid_to_string (plot_p -> pl_id_p, id_s);
													PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "No rows found for plot with id \"%s\"");
												}

										}		/* if (json_is_array (results_p)) */

									json_decref (results_p);
								}		/* if (results_p) */

							bson_destroy (opts_p);
						}		/* if (opts_p) */

					bson_destroy (query_p);
				}		/* if (query_p) */

		}

	return success_flag;
}


void SetPlotGenotypeControl (Plot *plot_p, bool control_flag)
{
	plot_p -> pl_replicate_control_flag = control_flag;
}


bool IsPlotGenotypeControl (const Plot *plot_p)
{
	return plot_p -> pl_replicate_control_flag;
}


static bool AddRowsToJSON (const Plot *plot_p, json_t *plot_json_p, const DFWFieldTrialServiceData *data_p)
{
	bool success_flag = false;
	json_t *rows_json_p = json_array ();

	if (rows_json_p)
		{
			if (json_object_set_new (plot_json_p, PL_ROWS_S, rows_json_p) == 0)
				{
					RowNode *node_p = (RowNode *) (plot_p -> pl_rows_p -> ll_head_p);

					success_flag = true;

					while (node_p && success_flag)
						{
							json_t *row_json_p = GetRowAsJSON (node_p -> rn_row_p, true, data_p);

							if (row_json_p)
								{
									if (json_array_append_new (rows_json_p, row_json_p) == 0)
										{
											node_p = (RowNode *) (node_p -> rn_node.ln_next_p);
										}
									else
										{
											success_flag = false;
											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, plot_json_p, "Failed to add row json to results");
										}
								}
							else
								{
									char id_s [MONGO_OID_STRING_BUFFER_SIZE];

									success_flag = false;
									bson_oid_to_string (node_p -> rn_row_p -> ro_id_p, id_s);

									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, plot_json_p, "Failed to create row json for \"%s\"", id_s);
								}

						}		/* while (node_p && &success_flag) */

				}
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add rows array");
					json_decref (rows_json_p);
				}
		}
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to create plots array");
		}

	return success_flag;
}



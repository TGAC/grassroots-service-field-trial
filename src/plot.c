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


Plot *AllocatePlot (bson_oid_t *id_p, const struct tm *sowing_date_p, const struct tm *harvest_date_p, const double64 width, const double64 length, const uint32 row_index,
										const uint32 column_index, const char *trial_design_s, const char *growing_conditions_s, const char *treatments_s, ExperimentalArea *parent_p)
{
	char *copied_trial_design_s = EasyCopyToNewString (trial_design_s);

	if (copied_trial_design_s)
		{
			char *copied_growing_conditions_s = EasyCopyToNewString (growing_conditions_s);

			if (copied_growing_conditions_s)
				{
					char *copied_treatments_s = EasyCopyToNewString (treatments_s);

					if (copied_treatments_s)
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
															plot_p -> pl_row_index = row_index;
															plot_p -> pl_column_index = column_index;
															plot_p -> pl_parent_p = parent_p;
															plot_p -> pl_rows_p = rows_p;
															plot_p -> pl_growing_conditions_s = copied_growing_conditions_s;
															plot_p -> pl_treatments_s = copied_treatments_s;
															plot_p -> pl_trial_design_s = copied_trial_design_s;

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


							FreeCopiedString (copied_treatments_s);
						}		/* if (copied_treatments_s) */
					else
						{

						}

					FreeCopiedString (copied_growing_conditions_s);
				}		/* if (copied_growing_conditions_s) */
			else
				{

				}

			FreeCopiedString (copied_trial_design_s);
		}		/* if (copied_trial_design_s) */
	else
		{

		}

	return NULL;
}




void FreePlot (Plot *plot_p)
{
	if (plot_p -> pl_id_p)
		{
			FreeBSONOid (plot_p -> pl_id_p);
		}

	if (plot_p -> pl_trial_design_s)
		{
			FreeCopiedString (plot_p -> pl_trial_design_s);
		}

	if (plot_p -> pl_growing_conditions_s)
		{
			FreeCopiedString (plot_p -> pl_growing_conditions_s);
		}


	if (plot_p -> pl_treatments_s)
		{
			FreeCopiedString (plot_p -> pl_treatments_s);
		}

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
	bool success_flag = false;
	bool insert_flag = false;

	if (! (plot_p -> pl_id_p))
		{
			plot_p -> pl_id_p  = GetNewBSONOid ();

			if (plot_p -> pl_id_p)
				{
					insert_flag = true;
				}
		}

	if (plot_p -> pl_id_p)
		{
			json_t *plot_json_p = GetPlotAsJSON (plot_p);

			if (plot_json_p)
				{
					success_flag = SaveMongoData (data_p -> dftsd_mongo_p, plot_json_p, data_p -> dftsd_collection_ss [DFTD_PLOT], insert_flag);

					json_decref (plot_json_p);
				}		/* if (plot_json_p) */

		}		/* if (plot_p -> pl_id_p) */

	return success_flag;
}


json_t *GetPlotAsJSON (const Plot *plot_p)
{
	json_t *plot_json_p = json_object ();

	if (plot_json_p)
		{
			if (SetJSONInteger (plot_json_p, PL_ROW_INDEX_S, plot_p -> pl_row_index))
				{
					if (SetJSONInteger (plot_json_p, PL_COLUMN_INDEX_S, plot_p -> pl_column_index))
						{
							if (SetJSONReal (plot_json_p, PL_WIDTH_S, plot_p -> pl_width))
								{
									if (SetJSONReal (plot_json_p, PL_LENGTH_S, plot_p -> pl_length))
										{
											if ((IsStringEmpty (plot_p -> pl_growing_conditions_s)) || (SetJSONString (plot_json_p, PL_GROWING_CONDITION_S, plot_p -> pl_growing_conditions_s)))
												{
													if ((IsStringEmpty (plot_p -> pl_treatments_s)) || (SetJSONString (plot_json_p, PL_TREATMENT_S, plot_p -> pl_treatments_s)))
														{
															if ((IsStringEmpty (plot_p -> pl_trial_design_s)) || (SetJSONString (plot_json_p, PL_TRIAL_DESIGN_S, plot_p -> pl_trial_design_s)))
																{
																	if (AddValidDateToJSON (plot_p -> pl_sowing_date_p, plot_json_p, PL_SOWING_DATE_S))
																		{
																			if (AddValidDateToJSON (plot_p -> pl_harvest_date_p, plot_json_p, PL_HARVEST_DATE_S))
																				{
																					if (AddCompoundIdToJSON (plot_json_p, plot_p -> pl_id_p))
																						{
																							if (AddNamedCompoundIdToJSON (plot_json_p, plot_p -> pl_parent_p -> ea_id_p, PL_PARENT_EXPERIMENTAL_AREA_S))
																								{
																									return plot_json_p;
																								}		/* if (AddNamedCompoundIdToJSON (plot_json_p, plot_p -> pl_parent_p -> ea_id_p, PL_PARENT_FIELD_TRIAL_S)) */
																							else
																								{
																									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, plot_json_p, "Failed to add id for \"%s\"", PL_PARENT_EXPERIMENTAL_AREA_S);
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

														}		/* if ((IsStringEmpty (plot_p -> pl_growing_conditions_s)) || (SetJSONString (plot_json_p, PL_TREATMENT_S, plot_p -> pl_treatments_s))) */
													else
														{
															PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, plot_json_p, "Failed to add \"%s\": \"%s\"", PL_TREATMENT_S, plot_p -> pl_treatments_s);
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

			json_decref (plot_json_p);
		}		/* if (plot_json_p) */

	return NULL;
}


Plot *GetPlotFromJSON (const json_t *plot_json_p)
{
	Plot *plot_p = NULL;
	int32 row;

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
									struct tm *sowing_date_p = NULL;

									if (CreateValidDateFromJSON (plot_json_p, PL_SOWING_DATE_S, &sowing_date_p))
										{
											struct tm *harvest_date_p = NULL;

											if (CreateValidDateFromJSON (plot_json_p, PL_HARVEST_DATE_S, &harvest_date_p))
												{
													bson_oid_t *field_trial_id_p = GetNewUnitialisedBSONOid ();

													if (field_trial_id_p)
														{
															if (GetNamedIdFromJSON (plot_json_p, PL_PARENT_EXPERIMENTAL_AREA_S, field_trial_id_p))
																{
																	ExperimentalArea *area_p = NULL;
																	bson_oid_t *id_p = GetNewUnitialisedBSONOid ();

																	if (id_p)
																		{
																			if (GetMongoIdFromJSON (plot_json_p, id_p))
																				{
																					plot_p = AllocatePlot (id_p, sowing_date_p, harvest_date_p, width, length, row, column, trial_design_s, growing_conditions_s, treatments_s, area_p);

																					if (!plot_p)
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
																			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate id for \"%s\"", PL_PARENT_EXPERIMENTAL_AREA_S);
																		}

																}		/* if (GetNamedIdFromJSON (plot_json_p, PL_PARENT_FIELD_TRIAL_S, field_trial_id_p)) */
															else
																{
																	PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, plot_json_p, "Failed to get id for \"%s\"", PL_PARENT_EXPERIMENTAL_AREA_S);
																}

														}		/* if (field_trial_id_p) */
													else
														{
															PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate id for \"%s\"", PL_PARENT_EXPERIMENTAL_AREA_S);
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



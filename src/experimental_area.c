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
 * experimental_area.c
 *
 *  Created on: 18 Sep 2018
 *      Author: billy
 */

#define ALLOCATE_EXPERIMENTAL_AREA_TAGS (1)
#include "experimental_area.h"
#include "memory_allocations.h"
#include "string_utils.h"
#include "plot.h"
#include "location.h"
#include "dfw_util.h"
#include "time_util.h"

/*
 * DB COLUMN NAMES
 */

/*
 * STATIC PROTOTYPES
 */
static void *GetExperimentalAreaCallback (const json_t *json_p, const ViewFormat format, const DFWFieldTrialServiceData *data_p);

static bool AddPlotsToJSON (ExperimentalArea *area_p, json_t *area_json_p, const ViewFormat format, const DFWFieldTrialServiceData *data_p);


/*
 * API FUNCTIONS
 */

ExperimentalArea *AllocateExperimentalArea (bson_oid_t *id_p, const char *name_s, const char *soil_s, const char *data_url_s, const struct tm *sowing_date_p, const struct tm *harvest_date_p, Location *location_p, FieldTrial *parent_field_trial_p, const DFWFieldTrialServiceData *data_p)
{
	char *copied_name_s = EasyCopyToNewString (name_s);

	if (copied_name_s)
		{
			char *copied_soil_s = NULL;
			bool empty_flag = IsStringEmpty (soil_s);

			if (!empty_flag)
				{
					copied_soil_s = EasyCopyToNewString (soil_s);
				}

			if (empty_flag || copied_soil_s)
				{
					char *copied_url_s = NULL;

					empty_flag = IsStringEmpty (data_url_s);

					if (!empty_flag)
						{
							copied_url_s = EasyCopyToNewString (data_url_s);
						}

					if (empty_flag || copied_url_s)
						{
							struct tm *copied_sowing_date_p = NULL;

							if (CopyValidDate (sowing_date_p, &copied_sowing_date_p))
								{
									struct tm *copied_harvest_date_p = NULL;

									if (CopyValidDate (harvest_date_p, &copied_harvest_date_p))
										{
											LinkedList *plots_p = AllocateLinkedList (FreePlotNode);

											if (plots_p)
												{
													ExperimentalArea *area_p = (ExperimentalArea *) AllocMemory (sizeof (ExperimentalArea));

													if (area_p)
														{
															area_p -> ea_id_p = id_p;
															area_p -> ea_name_s = copied_name_s;
															area_p -> ea_data_url_s = copied_url_s;
															area_p -> ea_soil_type_s = copied_soil_s;
															area_p -> ea_sowing_date_p = copied_sowing_date_p;
															area_p -> ea_harvest_date_p = copied_harvest_date_p;
															area_p -> ea_parent_p = parent_field_trial_p;
															area_p -> ea_location_p = location_p;
															area_p -> ea_plots_p = plots_p;

															return area_p;
														}

													FreeLinkedList (plots_p);
												}		/* if (plots_p) */

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

							if (copied_url_s)
								{
									FreeCopiedString (copied_url_s);
								}

						}		/* if (empty_flag || copied_url_s) */


					if (copied_soil_s)
						{
							FreeCopiedString (copied_soil_s);
						}

				}		/* if (copied_soil_s) */
			else
				{

				}

			FreeCopiedString (copied_name_s);
		}		/* if (copied_name_s) */
	else
		{

		}

	return NULL;
}


void FreeExperimentalArea (ExperimentalArea *area_p)
{
	if (area_p -> ea_id_p)
		{
			FreeBSONOid (area_p -> ea_id_p);
		}

	if (area_p -> ea_name_s)
		{
			FreeCopiedString (area_p -> ea_name_s);
		}

	if (area_p -> ea_data_url_s)
		{
			FreeCopiedString (area_p -> ea_data_url_s);
		}

	if (area_p -> ea_soil_type_s)
		{
			FreeCopiedString (area_p -> ea_soil_type_s);
		}

	if (area_p -> ea_location_p)
		{
			FreeLocation (area_p -> ea_location_p);
		}

	if (area_p -> ea_plots_p)
		{
			FreeLinkedList (area_p -> ea_plots_p);
		}


	if (area_p -> ea_sowing_date_p)
		{
			FreeTime (area_p -> ea_sowing_date_p);
		}

	if (area_p -> ea_harvest_date_p)
		{
			FreeTime (area_p -> ea_harvest_date_p);
		}

	FreeMemory (area_p);
}


ExperimentalAreaNode *AllocateExperimentalAreaNode (ExperimentalArea *area_p)
{
	ExperimentalAreaNode *ea_node_p = (ExperimentalAreaNode *) AllocMemory (sizeof (ExperimentalAreaNode));

	if (ea_node_p)
		{
			InitListItem (& (ea_node_p -> ean_node));

			ea_node_p -> ean_experimental_area_p = area_p;
		}

	return ea_node_p;
}

void FreeExperimentalAreaNode (ListItem *node_p)
{
	ExperimentalAreaNode *ea_node_p = (ExperimentalAreaNode *) node_p;

	if (ea_node_p -> ean_experimental_area_p)
		{
			FreeExperimentalArea (ea_node_p -> ean_experimental_area_p);
		}

	FreeMemory (ea_node_p);
}


bool GetExperimentalAreaPlots (ExperimentalArea *area_p, const DFWFieldTrialServiceData *data_p)
{
	bool success_flag = false;

	ClearLinkedList (area_p -> ea_plots_p);

	if (SetMongoToolCollection (data_p -> dftsd_mongo_p, data_p -> dftsd_collection_ss [DFTD_PLOT]))
		{
			bson_t *query_p = BCON_NEW (PL_PARENT_EXPERIMENTAL_AREA_S, BCON_OID (area_p -> ea_id_p));

			/*
			 * Make the query to get the matching plots
			 */
			if (query_p)
				{
					bson_t *opts_p =  BCON_NEW ( "sort", "{", PL_ROW_INDEX_S, BCON_INT32 (1), PL_COLUMN_INDEX_S, BCON_INT32 (1), "}");

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
													json_t *plot_json_p;

													json_array_foreach (results_p, i, plot_json_p)
														{
															Plot *plot_p = GetPlotFromJSON (plot_json_p, area_p, data_p);

															if (plot_p)
																{
																	PlotNode *node_p = AllocatePlotNode (plot_p);

																	if (node_p)
																		{
																			LinkedListAddTail (area_p -> ea_plots_p, & (node_p -> pn_node));
																		}
																	else
																		{
																			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, plot_json_p, "Failed to add plot to experimental area's list");
																			FreePlot (plot_p);
																		}
																}

														}		/* json_array_foreach (results_p, i, entry_p) */

												}		/* if (num_results > 0) */


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


bool SaveExperimentalArea (ExperimentalArea *area_p, DFWFieldTrialServiceData *data_p)
{
	bson_t *selector_p = NULL;
	bool success_flag = PrepareSaveData (& (area_p -> ea_id_p), &selector_p);

	if (success_flag)
		{
			json_t *area_json_p = GetExperimentalAreaAsJSON (area_p, VF_STORAGE, data_p);

			if (area_json_p)
				{
					success_flag = SaveMongoData (data_p -> dftsd_mongo_p, area_json_p, data_p -> dftsd_collection_ss [DFTD_EXPERIMENTAL_AREA], selector_p);

					json_decref (area_json_p);
				}		/* if (area_json_p) */

		}		/* if (success_flag) */

	return success_flag;
}


json_t *GetExperimentalAreaAsJSON (ExperimentalArea *area_p, const ViewFormat format, const DFWFieldTrialServiceData *data_p)
{
	json_t *area_json_p = json_object ();

	if (area_json_p)
		{
			if (SetJSONString (area_json_p, EA_NAME_S, area_p -> ea_name_s))
				{
					if ((IsStringEmpty (area_p -> ea_data_url_s)) || (SetJSONString (area_json_p, EA_DATA_LINK_S, area_p -> ea_data_url_s)))
						{
							if ((IsStringEmpty (area_p -> ea_soil_type_s)) || (SetJSONString (area_json_p, EA_SOIL_S, area_p -> ea_soil_type_s)))
								{
									bool add_item_flag = false;

									/*
									 * Add the location
									 */
									if ((format == VF_CLIENT_FULL) || (format == VF_CLIENT_MINIMAL))
										{
											json_t *location_json_p = GetLocationAsJSON (area_p -> ea_location_p);

											if (location_json_p)
												{
													if (json_object_set_new (area_json_p, EA_LOCATION_S, location_json_p) == 0)
														{
															add_item_flag = true;
														}		/* if (json_object_set_new (area_json_p, EA_LOCATION_S, location_json_p) == 0) */
													else
														{
															json_decref (location_json_p);
														}
												}
										}
									else
										{
											if (AddNamedCompoundIdToJSON (area_json_p, area_p -> ea_location_p -> lo_id_p, EA_LOCATION_ID_S))
												{
													add_item_flag = true;
												}
										}

									if (add_item_flag)
										{
											add_item_flag = false;

											/*
											 * Add the dates
											 */
											if (format == VF_STORAGE)
												{
													if (AddValidDateAsEpochToJSON (area_p -> ea_sowing_date_p, area_json_p, EA_SOWING_DATE_S))
														{
															if (AddValidDateAsEpochToJSON (area_p -> ea_harvest_date_p, area_json_p, EA_HARVEST_DATE_S))
																{
																	add_item_flag = true;
																}
														}
												}
											else
												{
													if (AddValidDateToJSON (area_p -> ea_sowing_date_p, area_json_p, EA_SOWING_DATE_S))
														{
															if (AddValidDateToJSON (area_p -> ea_harvest_date_p, area_json_p, EA_HARVEST_DATE_S))
																{
																	add_item_flag = true;
																}
														}
												}

											if (add_item_flag)
												{

													if (AddCompoundIdToJSON (area_json_p, area_p -> ea_id_p))
														{
															bool success_flag = false;

															if (format == VF_STORAGE)
																{
																	success_flag = AddNamedCompoundIdToJSON (area_json_p, area_p -> ea_parent_p -> ft_id_p, EA_PARENT_FIELD_TRIAL_S);
																}
															else if (format == VF_CLIENT_FULL)
																{
																	if (GetExperimentalAreaPlots (area_p, data_p))
																		{
																			if (AddPlotsToJSON (area_p, area_json_p, format, data_p))
																				{
																					success_flag = true;
																				}
																		}
																}
															else
																{
																	success_flag = true;
																}

															if (success_flag)
																{
																	return area_json_p;
																}
														}

												}
										}

								}		/* if ((IsStringEmpty (area_p -> ea_soil_type_s)) || (SetJSONString (area_json_p, EA_SOIL_S, area_p -> ea_soil_type_s) == 0)) */

						}		/* if ((IsStringEmpty (area_p -> ea_data_url_s)) || (SetJSONString (area_json_p, EA_DATA_LINK_S, area_p -> ea_data_url_s) == 0)) */

				}		/* if (SetJSONString (area_json_p, EA_NAME_S, area_p -> ea_name_s) == 0) */

			json_decref (area_json_p);
		}		/* if (area_json_p) */

	return NULL;
}



ExperimentalArea *GetExperimentalAreaFromJSON (const json_t *json_p, const ViewFormat format, const DFWFieldTrialServiceData *data_p)
{
	const char *name_s = GetJSONString (json_p, EA_NAME_S);
	ExperimentalArea *area_p = NULL;

	if (name_s)
		{
			bson_oid_t *location_id_p = GetNewUnitialisedBSONOid ();

			if (location_id_p)
				{
					if (GetNamedIdFromJSON (json_p, EA_LOCATION_ID_S, location_id_p))
						{
							bson_oid_t *id_p = GetNewUnitialisedBSONOid ();

							if (id_p)
								{
									if (GetMongoIdFromJSON (json_p, id_p))
										{
											FieldTrial *trial_p = NULL;
											const char *parent_field_trial_id_s = NULL;
											Location *location_p = NULL;
											bool success_flag = true;

											if ((format == VF_CLIENT_FULL) || (format == VF_CLIENT_MINIMAL))
												{
													if (! (location_p = GetLocationById (location_id_p, format, data_p)))
														{
															success_flag = false;
														}
												}

											if (success_flag)
												{
													struct tm *sowing_date_p = NULL;

													if (CreateValidDateFromJSON (json_p, EA_SOWING_DATE_S, &sowing_date_p))
														{
															struct tm *harvest_date_p = NULL;

															if (CreateValidDateFromJSON (json_p, EA_HARVEST_DATE_S, &harvest_date_p))
																{
																	const char *soil_s = GetJSONString (json_p, EA_SOIL_S);
																	const char *data_url_s = GetJSONString (json_p, EA_DATA_LINK_S);

																	area_p = AllocateExperimentalArea (id_p, name_s, soil_s, data_url_s, sowing_date_p, harvest_date_p, location_p, trial_p, data_p);

																	/*
																	 * The dates are copied by AllocateExperimentalArea so we can free our values.
																	 */
																	if (harvest_date_p)
																		{
																			FreeTime (harvest_date_p);
																		}

																}		/* if (CreateValidDateFromJSON (json_p, EA_HARVEST_DATE_S, &harvest_date_p)) */

															/*
															 * The dates are copied by AllocateExperimentalArea so we can free our values.
															 */
															if (sowing_date_p)
																{
																	FreeTime (sowing_date_p);
																}

														}		/* if (CreateValidDateFromJSON (json_p, EA_SOWING_DATE_S, &sowing_date_p)) */

												}		/* if (success_flag) */

										}

									if (!area_p)
										{
											FreeBSONOid (id_p);
										}

								}

						}		/* if (GetNamedIdFromJSON (json_p, EA_LOCATION_S, address_id_p)) */

					FreeBSONOid (location_id_p);
				}		/* if (location_id_p) */

		}		/* if (name_s) */

	return area_p;
}


ExperimentalArea *GetExperimentalAreaByIdString (const char *area_id_s, const ViewFormat format, const DFWFieldTrialServiceData *data_p)
{
	ExperimentalArea *area_p = GetDFWObjectByIdString (area_id_s, DFTD_EXPERIMENTAL_AREA, GetExperimentalAreaCallback, format, data_p);

	return area_p;
}


ExperimentalArea *GetExperimentalAreaById (bson_oid_t *area_id_p, const ViewFormat format, const DFWFieldTrialServiceData *data_p)
{
	ExperimentalArea *area_p = GetDFWObjectById (area_id_p, DFTD_EXPERIMENTAL_AREA, GetExperimentalAreaCallback, format, data_p);

	return area_p;
}


static void *GetExperimentalAreaCallback (const json_t *json_p, const ViewFormat format, const DFWFieldTrialServiceData *data_p)
{
	return GetExperimentalAreaFromJSON (json_p, format, data_p);
}


static bool AddPlotsToJSON (ExperimentalArea *area_p, json_t *area_json_p, const ViewFormat format, const DFWFieldTrialServiceData *data_p)
{
	bool success_flag = false;
	json_t *plots_json_p = json_array ();

	if (plots_json_p)
		{
			if (json_object_set_new (area_json_p, EA_PLOTS_S, plots_json_p) == 0)
				{
					PlotNode *node_p = (PlotNode *) (area_p -> ea_plots_p -> ll_head_p);

					success_flag = true;

					while (node_p && success_flag)
						{
							json_t *plot_json_p = GetPlotAsJSON (node_p -> pn_plot_p, format, data_p);

							if (plot_json_p)
								{
									if (json_array_append_new (plots_json_p, plot_json_p) == 0)
										{
											node_p = (PlotNode *) (node_p -> pn_node.ln_next_p);
										}
									else
										{
											success_flag = false;
											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, plot_json_p, "Failed to add plot json to results");
										}
								}
							else
								{
									char id_s [MONGO_OID_STRING_BUFFER_SIZE];

									success_flag = false;
									bson_oid_to_string (node_p -> pn_plot_p -> pl_id_p, id_s);

									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, plot_json_p, "Failed to create plot json for \"%s\"", id_s);
								}

						}		/* while (node_p && &success_flag) */

				}
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add plots array");
					json_decref (plots_json_p);
				}
		}
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to create plots array");
		}

	return success_flag;
}


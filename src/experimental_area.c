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
static void *GetExperimentalAreaCallback (const json_t *json_p, const DFWFieldTrialServiceData *data_p);


/*
 * API FUNCTIONS
 */

ExperimentalArea *AllocateExperimentalArea (bson_oid_t *id_p, const char *name_s, const char *soil_s, const struct tm *sowing_date_p, const struct tm *harvest_date_p, Location *location_p, FieldTrial *parent_field_trial_p, const DFWFieldTrialServiceData *data_p)
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

					FreeCopiedString (copied_soil_s);
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


ExperimentalArea *AllocateExperimentalAreaByIDString (bson_oid_t *id_p, const char *name_s, const char *location_s, const char *soil_s, const uint32 sowing_year, const uint32 harvest_year, const char *parent_field_trial_id_s, const DFWFieldTrialServiceData *data_p)
{
	FieldTrial *parent_field_trial_p = GetFieldTrialByIdString (parent_field_trial_id_s, data_p);

	if (parent_field_trial_p)
		{
			Address *address_p = NULL;
			ExperimentalArea *area_p = NULL; // AllocateExperimentalArea (id_p, name_s, location_s, soil_s, sowing_year, harvest_year,address_p,  parent_field_trial_p, data_p);

			if (area_p)
				{
					return area_p;
				}

			FreeFieldTrial (parent_field_trial_p);
		}		/* if (parent_field_trial_p) */

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


bool GetExperimentalAreaPlots (ExperimentalArea *area_p, DFWFieldTrialServiceData *data_p)
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

											if (num_results > 0)
												{
													json_t *plot_json_p;

													json_array_foreach (results_p, i, plot_json_p)
														{
															Plot *plot_p = GetPlotFromJSON (plot_json_p, data_p);

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
	bool success_flag = false;
	bool insert_flag = false;

	if (! (area_p -> ea_id_p))
		{
			area_p -> ea_id_p  = GetNewBSONOid ();

			if (area_p -> ea_id_p)
				{
					insert_flag = true;
				}
		}

	if (area_p -> ea_id_p)
		{
			json_t *area_json_p = GetExperimentalAreaAsJSON (area_p, false, data_p);

			if (area_json_p)

				{
					success_flag = SaveMongoData (data_p -> dftsd_mongo_p, area_json_p, data_p -> dftsd_collection_ss [DFTD_EXPERIMENTAL_AREA], insert_flag);

					json_decref (area_json_p);
				}		/* if (area_json_p) */

		}		/* if (area_p -> ea_id_p) */

	return success_flag;
}


ExperimentalArea *LoadExperimentalArea (const int32 area_id, DFWFieldTrialServiceData *data_p)
{
	ExperimentalArea *area_p = NULL;


	return area_p;
}


json_t *GetExperimentalAreaAsJSON (ExperimentalArea *area_p, const bool expand_fields_flag, DFWFieldTrialServiceData *data_p)
{
	json_t *area_json_p = json_object ();

	if (area_json_p)
		{
			if (json_object_set_new (area_json_p, EA_NAME_S, json_string (area_p -> ea_name_s)) == 0)
				{
					if ((IsStringEmpty (area_p -> ea_soil_type_s)) || (json_object_set_new (area_json_p, EA_SOIL_S, json_string (area_p -> ea_soil_type_s)) == 0))
						{
							bool add_location_flag = false;

							if (expand_fields_flag)
								{
									json_t *location_json_p = GetLocationAsJSON (area_p -> ea_location_p);

									if (location_json_p)
										{
											if (json_object_set_new (area_json_p, EA_LOCATION_S, location_json_p) == 0)
												{
													add_location_flag = true;
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
											add_location_flag = true;
										}
								}

							if (add_location_flag)
								{
									if (AddValidDateToJSON (area_p -> ea_sowing_date_p, area_json_p, EA_SOWING_DATE_S))
										{
											if (AddValidDateToJSON (area_p -> ea_harvest_date_p, area_json_p, EA_HARVEST_DATE_S))
												{
													if (AddCompoundIdToJSON (area_json_p, area_p -> ea_id_p))
														{
															if (AddNamedCompoundIdToJSON (area_json_p, area_p -> ea_parent_p -> ft_id_p, EA_PARENT_FIELD_TRIAL_S))
																{
																	if (expand_fields_flag)
																		{
																			GetExperimentalAreaPlots (area_p, data_p);
																		}

																	return area_json_p;
																}
														}
												}

										}
								}

						}
				}

			json_decref (area_json_p);
		}		/* if (area_json_p) */

	return NULL;
}



ExperimentalArea *GetExperimentalAreaFromJSON (const json_t *json_p, const bool full_location_flag, const DFWFieldTrialServiceData *data_p)
{
	const char *name_s = GetJSONString (json_p, EA_NAME_S);

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
											ExperimentalArea *area_p = NULL;
											FieldTrial *trial_p = NULL;
											const char *parent_field_trial_id_s = NULL;
											Location *location_p = NULL;
											bool success_flag = true;

											if (full_location_flag)
												{
													if (! (location_p = GetLocationById (location_id_p, data_p)))
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

																	area_p = AllocateExperimentalArea (id_p, name_s, soil_s, sowing_date_p, harvest_date_p, location_p, trial_p, data_p);

																	if (area_p)
																		{
																			return area_p;
																		}

																	if (harvest_date_p)
																		{
																			FreeTime (harvest_date_p);
																		}

																}		/* if (CreateValidDateFromJSON (json_p, EA_HARVEST_DATE_S, &harvest_date_p)) */

															if (sowing_date_p)
																{
																	FreeTime (sowing_date_p);
																}

														}		/* if (CreateValidDateFromJSON (json_p, EA_SOWING_DATE_S, &sowing_date_p)) */

												}		/* if (success_flag) */

										}

									FreeBSONOid (id_p);
								}
						}		/* if (GetNamedIdFromJSON (json_p, EA_LOCATION_S, address_id_p)) */

				}		/* if (address_id_p) */

		}		/* if (name_s) */

	return NULL;
}


ExperimentalArea *GetExperimentalAreaByIdString (const char *area_id_s, const DFWFieldTrialServiceData *data_p)
{
	ExperimentalArea *area_p = GetDFWObjectByIdString (area_id_s, DFTD_EXPERIMENTAL_AREA, GetExperimentalAreaCallback, data_p);

	return area_p;
}


ExperimentalArea *GetExperimentalAreaById (bson_oid_t *area_id_p, const DFWFieldTrialServiceData *data_p)
{
	ExperimentalArea *area_p = GetDFWObjectById (area_id_p, DFTD_EXPERIMENTAL_AREA, GetExperimentalAreaCallback, data_p);

	return area_p;
}


static void *GetExperimentalAreaCallback (const json_t *json_p, const DFWFieldTrialServiceData *data_p)
{
	return GetExperimentalAreaFromJSON (json_p, false, data_p);
}


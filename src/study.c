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

#define ALLOCATE_STUDY_TAGS (1)
#include "study.h"
#include "memory_allocations.h"
#include "string_utils.h"
#include "plot.h"
#include "location.h"
#include "dfw_util.h"
#include "time_util.h"

#include "study_jobs.h"
#include "indexing.h"

/*
 * DB COLUMN NAMES
 */

/*
 * STATIC PROTOTYPES
 */
static void *GetStudyCallback (const json_t *json_p, const ViewFormat format, const DFWFieldTrialServiceData *data_p);

static bool AddPlotsToJSON (Study *study_p, json_t *study_json_p, const ViewFormat format, const DFWFieldTrialServiceData *data_p);

static bool AddValidAspectToJSON (const Study *study_p, json_t *study_json_p);


/*
 * API FUNCTIONS
 */

Study *AllocateStudy (bson_oid_t *id_p, const char *name_s, const char *soil_s, const char *data_url_s, const char *aspect_s, const char *slope_s, const struct tm *sowing_date_p, const struct tm *harvest_date_p, struct Location *location_p, FieldTrial *parent_field_trial_p, Crop *current_crop_p, Crop *previous_crop_p, const int32 min_ph, const int32 max_ph, const char *notes_s, const DFWFieldTrialServiceData *data_p)
{
	char *copied_name_s = EasyCopyToNewString (name_s);

	if (copied_name_s)
		{
			char *copied_soil_s = NULL;

			if (CloneValidString (soil_s, &copied_soil_s))
				{
					char *copied_url_s = NULL;

					if (CloneValidString (data_url_s, &copied_url_s))
						{
							char *copied_aspect_s = NULL;

							if (CloneValidString (aspect_s, &copied_aspect_s))
								{
									char *copied_slope_s = NULL;

									if (CloneValidString (slope_s, &copied_slope_s))
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
																	Study *study_p = (Study *) AllocMemory (sizeof (Study));

																	if (study_p)
																		{
																			study_p -> st_id_p = id_p;
																			study_p -> st_name_s = copied_name_s;
																			study_p -> st_data_url_s = copied_url_s;
																			study_p -> st_soil_type_s = copied_soil_s;
																			study_p -> st_aspect_s = copied_aspect_s;
																			study_p -> st_slope_s = copied_slope_s;
																			study_p -> st_sowing_date_p = copied_sowing_date_p;
																			study_p -> st_harvest_date_p = copied_harvest_date_p;
																			study_p -> st_parent_p = parent_field_trial_p;
																			study_p -> st_location_p = location_p;
																			study_p -> st_plots_p = plots_p;
																			study_p -> st_min_ph = min_ph;
																			study_p -> st_max_ph = max_ph;
																			study_p -> st_current_crop_p = current_crop_p;
																			study_p -> st_previous_crop_p = previous_crop_p;

																			return study_p;
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

											if (copied_slope_s)
												{
													FreeCopiedString (copied_slope_s);
												}

										}		/* if (CloneValidString (slope_s, &copied_slope_s)) */

									if (copied_aspect_s)
										{
											FreeCopiedString (copied_aspect_s);
										}

								}		/* if (CloneValidString (aspect_s, &copied_aspect_s)) */

							if (copied_url_s)
								{
									FreeCopiedString (copied_url_s);
								}

						}		/* if (CloneValidString (data_url_s, &copied_url_s)) */


					if (copied_soil_s)
						{
							FreeCopiedString (copied_soil_s);
						}

				}		/* if (CloneValidString (soil_s, &copied_soil_s)) */
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


void FreeStudy (Study *study_p)
{
	if (study_p -> st_id_p)
		{
			FreeBSONOid (study_p -> st_id_p);
		}

	if (study_p -> st_name_s)
		{
			FreeCopiedString (study_p -> st_name_s);
		}

	if (study_p -> st_data_url_s)
		{
			FreeCopiedString (study_p -> st_data_url_s);
		}

	if (study_p -> st_soil_type_s)
		{
			FreeCopiedString (study_p -> st_soil_type_s);
		}

	if (study_p -> st_aspect_s)
		{
			FreeCopiedString (study_p -> st_aspect_s);
		}

	if (study_p -> st_slope_s)
		{
			FreeCopiedString (study_p -> st_slope_s);
		}

	if (study_p -> st_location_p)
		{
			FreeLocation (study_p -> st_location_p);
		}

	if (study_p -> st_plots_p)
		{
			FreeLinkedList (study_p -> st_plots_p);
		}


	if (study_p -> st_sowing_date_p)
		{
			FreeTime (study_p -> st_sowing_date_p);
		}

	if (study_p -> st_harvest_date_p)
		{
			FreeTime (study_p -> st_harvest_date_p);
		}


	if (study_p -> st_current_crop_p)
		{
			FreeCrop (study_p -> st_current_crop_p);
		}

	if (study_p -> st_previous_crop_p)
		{
			FreeCrop (study_p -> st_previous_crop_p);
		}


	FreeMemory (study_p);
}


StudyNode *AllocateStudyNode (Study *study_p)
{
	StudyNode *st_node_p = (StudyNode *) AllocMemory (sizeof (StudyNode));

	if (st_node_p)
		{
			InitListItem (& (st_node_p -> stn_node));

			st_node_p -> stn_study_p = study_p;
		}

	return st_node_p;
}

void FreeStudyNode (ListItem *node_p)
{
	StudyNode *st_node_p = (StudyNode *) node_p;

	if (st_node_p -> stn_study_p)
		{
			FreeStudy (st_node_p -> stn_study_p);
		}

	FreeMemory (st_node_p);
}


bool GetStudyPlots (Study *study_p, const DFWFieldTrialServiceData *data_p)
{
	bool success_flag = false;

	ClearLinkedList (study_p -> st_plots_p);

	if (SetMongoToolCollection (data_p -> dftsd_mongo_p, data_p -> dftsd_collection_ss [DFTD_PLOT]))
		{
			bson_t *query_p = BCON_NEW (PL_PARENT_STUDY_S, BCON_OID (study_p -> st_id_p));

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
														Plot *plot_p = GetPlotFromJSON (plot_json_p, study_p, data_p);

														if (plot_p)
															{
																PlotNode *node_p = AllocatePlotNode (plot_p);

																if (node_p)
																	{
																		LinkedListAddTail (study_p -> st_plots_p, & (node_p -> pn_node));
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


OperationStatus SaveStudy (Study *study_p, ServiceJob *job_p, DFWFieldTrialServiceData *data_p)
{
	OperationStatus status = OS_FAILED;
	bson_t *selector_p = NULL;
	bool success_flag = PrepareSaveData (& (study_p -> st_id_p), &selector_p);

	if (success_flag)
		{
			json_t *study_json_p = GetStudyAsJSON (study_p, VF_STORAGE, data_p);

			if (study_json_p)
				{
					if (SaveMongoData (data_p -> dftsd_mongo_p, study_json_p, data_p -> dftsd_collection_ss [DFTD_STUDY], selector_p))
						{
							if (IndexData (job_p, study_json_p))
								{
									status = OS_SUCCEEDED;
								}
							else
								{
									status = OS_PARTIALLY_SUCCEEDED;
									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, study_json_p, "Failed to index Study \"%s\" as JSON to Lucene", study_p -> st_name_s);
								}
						}

					json_decref (study_json_p);
				}		/* if (study_json_p) */

		}		/* if (success_flag) */

	return status;
}


json_t *GetStudyAsJSON (Study *study_p, const ViewFormat format, const DFWFieldTrialServiceData *data_p)
{
	json_t *study_json_p = json_object ();

	if (study_json_p)
		{
			if (SetJSONString (study_json_p, ST_NAME_S, study_p -> st_name_s))
				{
					if ((study_p -> st_min_ph == ST_UNSET_PH) || (SetJSONInteger (study_json_p, ST_MIN_PH_S, study_p -> st_min_ph)))
						{
							if ((study_p -> st_max_ph == ST_UNSET_PH) || (SetJSONInteger (study_json_p, ST_MAX_PH_S, study_p -> st_min_ph)))
								{
									if ((IsStringEmpty (study_p -> st_notes_s)) || (SetJSONString (study_json_p, ST_NOTES_S, study_p -> st_notes_s)))
										{
											if ((IsStringEmpty (study_p -> st_data_url_s)) || (SetJSONString (study_json_p, ST_DATA_LINK_S, study_p -> st_data_url_s)))
												{
													if ((IsStringEmpty (study_p -> st_soil_type_s)) || (SetJSONString (study_json_p, ST_SOIL_S, study_p -> st_soil_type_s)))
														{
															if ((IsStringEmpty (study_p -> st_slope_s)) || (SetJSONString (study_json_p, ST_SLOPE_S, study_p -> st_slope_s)))
																{
																	if (AddValidAspectToJSON (study_p, study_json_p))
																		{
																			bool add_item_flag = false;

																			/*
																			 * Add the location
																			 */
																			if ((format == VF_CLIENT_FULL) || (format == VF_CLIENT_MINIMAL))
																				{
																					json_t *location_json_p = GetLocationAsJSON (study_p -> st_location_p);

																					if (location_json_p)
																						{
																							if (json_object_set_new (study_json_p, ST_LOCATION_S, location_json_p) == 0)
																								{
																									add_item_flag = true;
																								}		/* if (json_object_set_new (study_json_p, ST_LOCATION_S, location_json_p) == 0) */
																							else
																								{
																									json_decref (location_json_p);
																								}
																						}
																				}
																			else
																				{
																					if (AddNamedCompoundIdToJSON (study_json_p, study_p -> st_location_p -> lo_id_p, ST_LOCATION_ID_S))
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
																							if (AddValidDateAsEpochToJSON (study_p -> st_sowing_date_p, study_json_p, ST_SOWING_DATE_S))
																								{
																									if (AddValidDateAsEpochToJSON (study_p -> st_harvest_date_p, study_json_p, ST_HARVEST_DATE_S))
																										{
																											add_item_flag = true;
																										}
																								}
																						}		/* if (format == VF_STORAGE) */
																					else
																						{
																							if (AddValidDateToJSON (study_p -> st_sowing_date_p, study_json_p, ST_SOWING_DATE_S))
																								{
																									if (AddValidDateToJSON (study_p -> st_harvest_date_p, study_json_p, ST_HARVEST_DATE_S))
																										{
																											add_item_flag = true;
																										}
																								}
																						}

																					if (add_item_flag)
																						{

																							if (AddCompoundIdToJSON (study_json_p, study_p -> st_id_p))
																								{
																									bool success_flag = false;

																									if (format == VF_STORAGE)
																										{

																											success_flag = AddNamedCompoundIdToJSON (study_json_p, study_p -> st_parent_p -> ft_id_p, ST_PARENT_FIELD_TRIAL_S);
																										}
																									else if (format == VF_CLIENT_FULL)
																										{
																											if (GetStudyPlots (study_p, data_p))
																												{
																													if (AddPlotsToJSON (study_p, study_json_p, format, data_p))
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
																											if (AddDatatype (study_json_p, DFTD_STUDY))
																												{
																													return study_json_p;
																												}
																										}
																								}

																						}		/* if (add_item_flag) */

																				}		/* if (add_item_flag) */

																		}		/* if (AddValidAspectToJSON (study_p, study_json_p)) */

																}		/* if ((IsStringEmpty (study_p -> st_slope_s)) || (SetJSONString (study_json_p, ST_SLOPE_S, study_p -> st_slope_s))) */

														}		/* if ((IsStringEmpty (study_p -> st_soil_type_s)) || (SetJSONString (study_json_p, ST_SOIL_S, study_p -> st_soil_type_s) == 0)) */

												}		/* if ((IsStringEmpty (study_p -> st_data_url_s)) || (SetJSONString (study_json_p, ST_DATA_LINK_S, study_p -> st_data_url_s) == 0)) */

										}


								}		/* if ((study_p -> st_min_ph == ST_UNSET_PH) || (SetJSONInteger (study_json_p, ST_MIN_PH_S, study_p -> st_min_ph))) */

						}		/* if ((study_p -> st_min_ph == ST_UNSET_PH) || (SetJSONInteger (study_json_p, ST_MIN_PH_S, study_p -> st_min_ph))) */

				}		/* if (SetJSONString (study_json_p, ST_NAME_S, study_p -> st_name_s) == 0) */

			json_decref (study_json_p);
		}		/* if (study_json_p) */

	return NULL;
}


Study *GetStudyFromJSON (const json_t *json_p, const ViewFormat format, const DFWFieldTrialServiceData *data_p)
{
	const char *name_s = GetJSONString (json_p, ST_NAME_S);
	Study *study_p = NULL;

	if (name_s)
		{
			bson_oid_t *parent_field_trial_id_p = GetNewUnitialisedBSONOid ();

			if (parent_field_trial_id_p)
				{
					if (GetNamedIdFromJSON (json_p, ST_PARENT_FIELD_TRIAL_S, parent_field_trial_id_p))
						{
							bson_oid_t *location_id_p = GetNewUnitialisedBSONOid ();

							if (location_id_p)
								{
									if (GetNamedIdFromJSON (json_p, ST_LOCATION_ID_S, location_id_p))
										{
											bson_oid_t *id_p = GetNewUnitialisedBSONOid ();

											if (id_p)
												{
													if (GetMongoIdFromJSON (json_p, id_p))
														{
															FieldTrial *trial_p = NULL;
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

																	if (CreateValidDateFromJSON (json_p, ST_SOWING_DATE_S, &sowing_date_p))
																		{
																			struct tm *harvest_date_p = NULL;

																			if (CreateValidDateFromJSON (json_p, ST_HARVEST_DATE_S, &harvest_date_p))
																				{
																					const char *soil_s = GetJSONString (json_p, ST_SOIL_S);
																					const char *data_url_s = GetJSONString (json_p, ST_DATA_LINK_S);
																					const char *slope_s = GetJSONString (json_p, ST_SLOPE_S);
																					const char *aspect_s = GetJSONString (json_p, ST_ASPECT_S);
																					const char *notes_s = GetJSONString (json_p, ST_NOTES_S);
																					Crop *current_crop_p = NULL;
																					Crop *previous_crop_p = NULL;
																					const KeyValuePair *aspect_p = NULL;
																					int32 min_ph = ST_UNSET_PH;
																					int32 max_ph = ST_UNSET_PH;

																					if (aspect_s)
																						{
																							aspect_p = GetAspect (aspect_s);

																							if (aspect_p)
																								{
																									aspect_s = aspect_p -> kvp_value_s;
																								}
																							else
																								{
																									aspect_s = NULL;
																								}
																						}


																					GetJSONInteger (json_p, ST_MIN_PH_S, &min_ph);
																					GetJSONInteger (json_p, ST_MAX_PH_S, &max_ph);

																					study_p = AllocateStudy (id_p, name_s, soil_s, data_url_s, aspect_s, slope_s, sowing_date_p, harvest_date_p, location_p, trial_p, current_crop_p, previous_crop_p, min_ph, max_ph, notes_s, data_p);

																					/*
																					 * The dates are copied by AllocateStudy so we can free our values.
																					 */
																					if (harvest_date_p)
																						{
																							FreeTime (harvest_date_p);
																						}

																				}		/* if (CreateValidDateFromJSON (json_p, ST_HARVEST_DATE_S, &harvest_date_p)) */

																			/*
																			 * The dates are copied by AllocateStudy so we can free our values.
																			 */
																			if (sowing_date_p)
																				{
																					FreeTime (sowing_date_p);
																				}

																		}		/* if (CreateValidDateFromJSON (json_p, ST_SOWING_DATE_S, &sowing_date_p)) */

																}		/* if (success_flag) */

														}

													if (!study_p)
														{
															FreeBSONOid (id_p);
														}

												}

										}		/* if (GetNamedIdFromJSON (json_p, ST_LOCATION_S, address_id_p)) */

									FreeBSONOid (location_id_p);
								}		/* if (location_id_p) */

						}		/* if (GetNamedIdFromJSON (json_p, ST_PARENT_FIELD_TRIAL_S, parent_field_trial_id_p)) */

					FreeBSONOid (parent_field_trial_id_p);
				}		/* if (parent_field_trial_id_p) */


		}		/* if (name_s) */

	return study_p;
}


Study *GetStudyByIdString (const char *study_id_s, const ViewFormat format, const DFWFieldTrialServiceData *data_p)
{
	Study *study_p = GetDFWObjectByIdString (study_id_s, DFTD_STUDY, GetStudyCallback, format, data_p);

	return study_p;
}


Study *GetStudyById (bson_oid_t *study_id_p, const ViewFormat format, const DFWFieldTrialServiceData *data_p)
{
	Study *study_p = GetDFWObjectById (study_id_p, DFTD_STUDY, GetStudyCallback, format, data_p);

	return study_p;
}


static bool AddValidAspectToJSON (const Study *study_p, json_t *study_json_p)
{
	bool success_flag = true;

	/*
	 * Is the aspect a valid set value?
	 */
	if (! ((IsStringEmpty (study_p -> st_aspect_s)) || (strcmp (study_p -> st_aspect_s, ST_UNKNOWN_DIRECTION_S) == 0)))
		{
			if (!SetJSONString (study_json_p, ST_ASPECT_S, study_p -> st_aspect_s))
				{
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, study_json_p, "Failed to add \"%s\": \"%s\"", ST_ASPECT_S, study_p -> st_aspect_s);
					success_flag = false;
				}
		}

	return success_flag;
}


static void *GetStudyCallback (const json_t *json_p, const ViewFormat format, const DFWFieldTrialServiceData *data_p)
{
	return GetStudyFromJSON (json_p, format, data_p);
}


static bool AddPlotsToJSON (Study *study_p, json_t *study_json_p, const ViewFormat format, const DFWFieldTrialServiceData *data_p)
{
	bool success_flag = false;
	json_t *plots_json_p = json_array ();

	if (plots_json_p)
		{
			if (json_object_set_new (study_json_p, ST_PLOTS_S, plots_json_p) == 0)
				{
					PlotNode *node_p = (PlotNode *) (study_p -> st_plots_p -> ll_head_p);

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


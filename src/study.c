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


static bool AddPlotsToJSON (Study *study_p, json_t *study_json_p, const ViewFormat format, JSONProcessor *processor_p, const DFWFieldTrialServiceData *data_p);


static bool AddValidAspectToJSON (const Study *study_p, json_t *study_json_p);

static bool AddValidCropToJSON (Crop *crop_p, json_t *study_json_p, const ViewFormat format, const DFWFieldTrialServiceData *data_p);

static Crop *GetStoredCropValue (const json_t *json_p, const char *key_s, const DFWFieldTrialServiceData *data_p);

static bool SetNonTrivialString (json_t *value_p, const char *key_s, const char *value_s);

static bool SetNonTrivialDouble (json_t *json_p, const char *key_s, const double64 *value_p);

static int32 GetNumberOfPlotsInStudy (const Study *study_p, const DFWFieldTrialServiceData *data_p);

static bool AddParentFieldTrialToJSON (Study *study_p, json_t *study_json_p, const DFWFieldTrialServiceData *data_p);

static bool AddDefaultPlotValuesToJSON (const Study *study_p, json_t *study_json_p, const DFWFieldTrialServiceData *data_p);

static bool GetValidRealFromJSON (const json_t *study_json_p, const char *key_s, double64 **ph_pp);

static bool SetNonTrivialUnsignedInt (json_t *json_p, const char *key_s, const uint32 *value_p);

static bool GetValidUnsignedIntFromJSON (const json_t *study_json_p, const char *key_s, uint32 **value_pp);


/*
 * API FUNCTIONS
 */

Study *AllocateStudy (bson_oid_t *id_p, const char *name_s, const char *soil_s, const char *data_url_s, const char *aspect_s, const char *slope_s,
											const struct tm *sowing_date_p, const struct tm *harvest_date_p, struct Location *location_p, FieldTrial *parent_field_trial_p,
											MEM_FLAG parent_field_trial_mem, Crop *current_crop_p, Crop *previous_crop_p, const double64 *min_ph_p, const double64 *max_ph_p, const char *description_s,
											const char *design_s, const char *growing_conditions_s, const char *phenotype_gathering_notes_s,
											const uint32 *num_rows_p, const uint32 *num_cols_p, const double64 *plot_width_p, const double64 *plot_length_p,
											const char *weather_s,
											const DFWFieldTrialServiceData *data_p)
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
																	char *copied_description_s = NULL;

																	if (CloneValidString (description_s, &copied_description_s))
																		{
																			char *copied_design_s = NULL;

																			if (CloneValidString (design_s, &copied_design_s))
																				{
																					char *copied_growing_conditions_s = NULL;

																					if (CloneValidString (growing_conditions_s, &copied_growing_conditions_s))
																						{
																							char *copied_phenotype_notes_s = NULL;

																							if (CloneValidString (phenotype_gathering_notes_s, &copied_phenotype_notes_s))
																								{
																									char *copied_weather_s = NULL;

																									if (CloneValidString (weather_s, &copied_weather_s))
																										{
																											double64 *copied_min_ph_p = NULL;

																											if (CopyValidReal (min_ph_p, &copied_min_ph_p))
																												{
																													double64 *copied_max_ph_p = NULL;

																													if (CopyValidReal (max_ph_p, &copied_max_ph_p))
																														{
																															double64 *copied_plot_width_p = NULL;

																															if (CopyValidReal (plot_width_p, &copied_plot_width_p))
																																{
																																	double64 *copied_plot_length_p = NULL;

																																	if (CopyValidReal (plot_length_p, &copied_plot_length_p))
																																		{
																																			uint32 *copied_num_rows_p = NULL;

																																			if (CopyValidUnsignedInteger (num_rows_p, &copied_num_rows_p))
																																				{
																																					uint32 *copied_num_cols_p = NULL;

																																					if (CopyValidUnsignedInteger (num_cols_p, &copied_num_cols_p))
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
																																									study_p -> st_parent_field_trial_mem = parent_field_trial_mem;
																																									study_p -> st_location_p = location_p;
																																									study_p -> st_plots_p = plots_p;
																																									study_p -> st_min_ph_p = copied_min_ph_p;
																																									study_p -> st_max_ph_p = copied_max_ph_p;
																																									study_p -> st_current_crop_p = current_crop_p;
																																									study_p -> st_previous_crop_p = previous_crop_p;
																																									study_p -> st_description_s = copied_description_s;
																																									study_p -> st_growing_conditions_s = copied_growing_conditions_s;
																																									study_p -> st_phenotype_gathering_notes_s = copied_phenotype_notes_s;
																																									study_p -> st_design_s = copied_design_s;

																																									study_p -> st_default_plot_width_p = copied_plot_width_p;
																																									study_p -> st_default_plot_length_p = copied_plot_length_p;
																																									study_p -> st_num_rows_p = copied_num_rows_p;
																																									study_p -> st_num_columns_p = copied_num_cols_p;

																																									study_p -> st_weather_link_s = copied_weather_s;

																																									return study_p;
																																								}

																																							if (copied_num_cols_p)
																																								{
																																									FreeMemory (copied_num_cols_p);
																																								}
																																						}

																																					if (copied_num_rows_p)
																																						{
																																							FreeMemory (copied_num_rows_p);
																																						}

																																				}

																																			if (copied_plot_length_p)
																																				{
																																					FreeMemory (copied_plot_length_p);
																																				}

																																		}


																																	if (copied_plot_width_p)
																																		{
																																			FreeMemory (copied_plot_width_p);
																																		}
																																}

																															if (copied_max_ph_p)
																																{
																																	FreeMemory (copied_max_ph_p);
																																}

																														}		/* if (CopyValidReal (max_ph_p, &copied_max_ph_p)) */


																													if (copied_min_ph_p)
																														{
																															FreeMemory (copied_min_ph_p);
																														}

																												}		/* if (CopyValidReal (min_ph_p, &copied_min_ph_p)) */

																											if (copied_weather_s)
																												{
																													FreeCopiedString (copied_weather_s);
																												}

																										}		/* if (CloneValidString (weather_s, &copied_weather_s)) */



																									if (copied_phenotype_notes_s)
																										{
																											FreeCopiedString (copied_phenotype_notes_s);
																										}

																								}		/* if (CloneValidString (phenotype_gathering_notes_s, &copied_phenotype_notes_s)) */

																							if (copied_growing_conditions_s)
																								{
																									FreeCopiedString (copied_growing_conditions_s);
																								}

																						}		/* if (CloneValidString (growing_conditions_s, &copied_growing_conditions_s)) */

																					if (copied_design_s)
																						{
																							FreeCopiedString (copied_design_s);
																						}

																				}		/* if (CloneValidString (design_s, &copied_design_s)) */


																			if (copied_description_s)
																				{
																					FreeCopiedString (copied_description_s);
																				}

																		}		/* if (CloneValidString (notes_s, &copied_notes_s)) */


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


	if (study_p -> st_phenotype_gathering_notes_s)
		{
			FreeCopiedString (study_p -> st_phenotype_gathering_notes_s);
		}

	if (study_p -> st_design_s)
		{
			FreeCopiedString (study_p -> st_design_s);
		}

	if (study_p -> st_growing_conditions_s)
		{
			FreeCopiedString (study_p -> st_growing_conditions_s);
		}

	if (study_p -> st_min_ph_p)
		{
			FreeMemory (study_p -> st_min_ph_p);
		}

	if (study_p -> st_max_ph_p)
		{
			FreeMemory (study_p -> st_max_ph_p);
		}

	if (study_p -> st_default_plot_width_p)
		{
			FreeMemory (study_p -> st_default_plot_width_p);
		}

	if (study_p -> st_default_plot_length_p)
		{
			FreeMemory (study_p -> st_default_plot_length_p);
		}

	if (study_p -> st_num_rows_p)
		{
			FreeMemory (study_p -> st_num_rows_p);
		}

	if (study_p -> st_num_columns_p)
		{
			FreeMemory (study_p -> st_num_columns_p);
		}

	if (study_p -> st_parent_p)
		{
			if ((study_p -> st_parent_field_trial_mem == MF_DEEP_COPY) || (study_p -> st_parent_field_trial_mem == MF_SHALLOW_COPY))
				{
					//FreeFieldTrial (study_p -> st_parent_p);
				}
		}

	if (study_p -> st_weather_link_s)
		{
			FreeCopiedString (study_p -> st_weather_link_s);
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


bool AddStudyPlotsJSONDirectly (Study *study_p, json_t *study_json_p,  const DFWFieldTrialServiceData *data_p)
{
	bool success_flag = false;

	return success_flag;
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
			json_t *study_json_p = GetStudyAsJSON (study_p, VF_STORAGE, NULL, data_p);

			if (study_json_p)
				{
					if (SaveMongoData (data_p -> dftsd_mongo_p, study_json_p, data_p -> dftsd_collection_ss [DFTD_STUDY], selector_p))
						{
							char *id_s = GetBSONOidAsString (study_p -> st_id_p);

							if (id_s)
								{
									if (data_p -> dftsd_study_cache_path_s)
										{
											ClearCachedStudy (id_s, data_p);
										}

									FreeCopiedString (id_s);
								}
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to clear potential cached Study \"%s\"", study_p -> st_name_s);
								}

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


json_t *GetStudyAsJSON (Study *study_p, const ViewFormat format, JSONProcessor *processor_p, const DFWFieldTrialServiceData *data_p)
{
	json_t *study_json_p = json_object ();

	if (study_json_p)
		{
			if (SetJSONString (study_json_p, ST_NAME_S, study_p -> st_name_s))
				{
					if (SetNonTrivialDouble (study_json_p, ST_MIN_PH_S, study_p -> st_min_ph_p))
						{
							if (SetNonTrivialDouble (study_json_p, ST_MAX_PH_S, study_p -> st_max_ph_p))
								{
									if (SetNonTrivialString (study_json_p, ST_DESCRIPTION_S, study_p -> st_description_s))
										{
											if (SetNonTrivialString (study_json_p, ST_GROWING_CONDITIONS_S, study_p -> st_growing_conditions_s))
												{
													if (SetNonTrivialString (study_json_p, ST_DESCRIPTION_S, study_p -> st_description_s))
														{
															if (SetNonTrivialString (study_json_p, ST_DESIGN_S, study_p -> st_design_s))
																{
																	if (SetNonTrivialString (study_json_p, ST_GROWING_CONDITIONS_S, study_p -> st_growing_conditions_s))
																		{
																			if (SetNonTrivialString (study_json_p, ST_PHENOTYPE_GATHERING_NOTES_S, study_p -> st_phenotype_gathering_notes_s))
																				{
																					if (SetNonTrivialString (study_json_p, ST_DATA_LINK_S, study_p -> st_data_url_s))
																						{
																							if (SetNonTrivialString (study_json_p, ST_SOIL_S, study_p -> st_soil_type_s))
																								{
																									if (SetNonTrivialString (study_json_p, ST_SLOPE_S, study_p -> st_slope_s))
																										{
																											if (SetNonTrivialString (study_json_p, ST_WEATHER_S, study_p -> st_weather_link_s))
																												{
																													if (AddValidAspectToJSON (study_p, study_json_p))
																														{
																															if (AddDefaultPlotValuesToJSON (study_p, study_json_p, data_p))
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
																																							if (AddValidCropToJSON (study_p -> st_current_crop_p, study_json_p, format, data_p))
																																								{
																																									if (AddValidCropToJSON (study_p -> st_previous_crop_p, study_json_p, format, data_p))
																																										{
																																											if (AddParentFieldTrialToJSON (study_p, study_json_p, data_p))
																																												{
																																													add_item_flag = true;
																																												}
																																											else
																																												{
																																													PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, study_json_p, "Failed to add parent field trial to study \"%s\"", study_p -> st_parent_p -> ft_name_s);
																																												}
																																										}
																																									else
																																										{
																																											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, study_json_p, "Failed to add previous crop to study \"%s\"", study_p -> st_previous_crop_p -> cr_name_s);
																																										}
																																								}
																																							else
																																								{
																																									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, study_json_p, "Failed to add current crop to study \"%s\"", study_p -> st_current_crop_p -> cr_name_s);
																																								}

																																						}		/* if (json_object_set_new (study_json_p, ST_LOCATION_S, location_json_p) == 0) */
																																					else
																																						{
																																							json_decref (location_json_p);
																																							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, study_json_p, "Failed to add location to study \"%s\"", study_p -> st_location_p -> lo_address_p -> ad_name_s);
																																						}
																																				}		/* if (location_json_p) */
																																			else
																																				{
																																					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get location \"%s\" as JSON", study_p -> st_location_p -> lo_address_p -> ad_name_s);
																																				}
																																		}
																																	else
																																		{
																																			if (AddNamedCompoundIdToJSON (study_json_p, study_p -> st_parent_p -> ft_id_p, ST_PARENT_FIELD_TRIAL_S))
																																				{

																																					if ((! (study_p -> st_location_p)) || (AddNamedCompoundIdToJSON (study_json_p, study_p -> st_location_p -> lo_id_p, ST_LOCATION_ID_S)))
																																						{
																																							if ((! (study_p -> st_current_crop_p)) || (AddNamedCompoundIdToJSON (study_json_p, study_p -> st_current_crop_p -> cr_id_p, ST_CURRENT_CROP_S)))
																																								{
																																									if ((! (study_p -> st_previous_crop_p)) || (AddNamedCompoundIdToJSON (study_json_p, study_p -> st_previous_crop_p -> cr_id_p, ST_PREVIOUS_CROP_S)))
																																										{
																																											add_item_flag = true;
																																										}		/* if (AddNamedCompoundIdToJSON (study_json_p, study_p -> st_previous_crop_p -> cr_id_p, ST_PREVIOUS_CROP_S)) */
																																									else
																																										{
																																											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, study_json_p, "Failed to add previous crop \"%s\"", study_p -> st_previous_crop_p -> cr_name_s);
																																										}
																																								}		/* if (AddNamedCompoundIdToJSON (study_json_p, study_p -> st_current_crop_p -> cr_id_p, ST_CURRENT_CROP_S)) */
																																							else
																																								{
																																									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, study_json_p, "Failed to add current crop \"%s\"", study_p -> st_current_crop_p -> cr_name_s);
																																								}
																																						}		/* if (AddNamedCompoundIdToJSON (study_json_p, study_p -> st_location_p -> lo_id_p, ST_LOCATION_ID_S)) */
																																					else
																																						{
																																							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, study_json_p, "Failed to add location \"%s\"", study_p -> st_location_p -> lo_address_p -> ad_name_s);
																																						}
																																				}		/* if (AddNamedCompoundIdToJSON (study_json_p, study_p -> st_parent_p -> ft_id_p, ST_PARENT_FIELD_TRIAL_S)) */
																																			else
																																				{
																																					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, study_json_p, "Failed to add \"%s\" \"%s\"", ST_PARENT_FIELD_TRIAL_S, study_p -> st_parent_p -> ft_name_s);
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

																																							if (format == VF_CLIENT_FULL)
																																								{
																																									if (GetStudyPlots (study_p, data_p))
																																										{
																																											if (AddPlotsToJSON (study_p, study_json_p, format, processor_p, data_p))
																																												{
																																													success_flag = true;
																																												}
																																										}
																																								}
																																							else if (format == VF_CLIENT_MINIMAL)
																																								{
																																									int32 num_plots = GetNumberOfPlotsInStudy (study_p, data_p);

																																									if (num_plots >= 0)
																																										{
																																											if (SetJSONInteger (study_json_p, ST_NUMBER_OF_PLOTS_S, num_plots))
																																												{
																																													success_flag = true;
																																												}
																																										}
																																									else
																																										{

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


																																}		/* if (AddDefaultPlotValuesToJSON (study_p, study_json_p, data_p)) */
																															else
																																{
																																	PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, study_json_p, "AddDefaultPlotValuesToJSON failed for study \"%s\"", study_p -> st_parent_p -> ft_name_s);
																																}

																														}		/* if (AddValidAspectToJSON (study_p, study_json_p)) */

																												}		/* if (SetNonTrivialString (study_json_p, ST_WEATHER_S, study_p -> st_weather_link_s)) */

																										}		/* if ((IsStringEmpty (study_p -> st_slope_s)) || (SetJSONString (study_json_p, ST_SLOPE_S, study_p -> st_slope_s))) */

																								}		/* if ((IsStringEmpty (study_p -> st_soil_type_s)) || (SetJSONString (study_json_p, ST_SOIL_S, study_p -> st_soil_type_s) == 0)) */

																						}		/* if ((IsStringEmpty (study_p -> st_data_url_s)) || (SetJSONString (study_json_p, ST_DATA_LINK_S, study_p -> st_data_url_s) == 0)) */

																				}
																		}
																}

														}		/* if ((IsStringEmpty (study_p -> st_description_s)) || (SetJSONString (study_json_p, ST_DESCRIPTION_S, study_p -> st_description_s))) */

												}		/* if ((study_p -> st_min_ph == ST_UNSET_PH) || (SetJSONInteger (study_json_p, ST_MIN_PH_S, study_p -> st_min_ph))) */
										}
								}
						}

					/* if ((study_p -> st_min_ph == ST_UNSET_PH) || (SetJSONInteger (study_json_p, ST_MIN_PH_S, study_p -> st_min_ph))) */

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
																	else if (! (trial_p = GetFieldTrialById (parent_field_trial_id_p, format, data_p)))
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
																					const char *description_s = GetJSONString (json_p, ST_DESCRIPTION_S);
																					const char *design_s = GetJSONString (json_p, ST_DESIGN_S);
																					const char *growing_conditions_s = GetJSONString (json_p, ST_GROWING_CONDITIONS_S);
																					const char *phenotype_gathering_notes_s = GetJSONString (json_p, ST_PHENOTYPE_GATHERING_NOTES_S);
																					Crop *current_crop_p = GetStoredCropValue (json_p, ST_CURRENT_CROP_S, data_p);
																					Crop *previous_crop_p = GetStoredCropValue (json_p, ST_PREVIOUS_CROP_S, data_p);
																					const KeyValuePair *aspect_p = NULL;
																					double64 *min_ph_p = NULL;
																					double64 *max_ph_p = NULL;
																					double64 *plot_width_p = NULL;
																					double64 *plot_length_p = NULL;
																					uint32 *num_plot_rows_p = NULL;
																					uint32 *num_plot_columns_p = NULL;
																					const char *weather_s = GetJSONString (json_p, ST_WEATHER_S);

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

																					GetValidRealFromJSON (json_p, ST_MIN_PH_S, &min_ph_p);
																					GetValidRealFromJSON (json_p, ST_MAX_PH_S, &max_ph_p);

																					GetValidUnsignedIntFromJSON (json_p, ST_NUMBER_OF_PLOT_ROWS_S, &num_plot_rows_p);
																					GetValidUnsignedIntFromJSON (json_p, ST_NUMBER_OF_PLOT_COLUMN_S, &num_plot_columns_p);

																					GetValidRealFromJSON (json_p, ST_PLOT_WIDTH_S, &plot_width_p);
																					GetValidRealFromJSON (json_p, ST_PLOT_LENGTH_S, &plot_length_p);



																					study_p = AllocateStudy (id_p, name_s, soil_s, data_url_s, aspect_s, slope_s, sowing_date_p, harvest_date_p, location_p, trial_p, MF_SHALLOW_COPY, current_crop_p, previous_crop_p,
																																	 min_ph_p, max_ph_p, description_s, design_s, growing_conditions_s, phenotype_gathering_notes_s,
																																	 num_plot_rows_p, num_plot_columns_p, plot_width_p, plot_length_p,
																																	 weather_s,
																																	 data_p);

																					/*
																					 * The dates are copied by AllocateStudy so we can free our values.
																					 */
																					if (harvest_date_p)
																						{
																							FreeTime (harvest_date_p);
																						}


																					if (min_ph_p)
																						{
																							FreeMemory (min_ph_p);
																						}


																					if (max_ph_p)
																						{
																							FreeMemory (max_ph_p);
																						}


																					if (num_plot_rows_p)
																						{
																							FreeMemory (num_plot_rows_p);
																						}


																					if (num_plot_columns_p)
																						{
																							FreeMemory (num_plot_columns_p);
																						}


																					if (plot_width_p)
																						{
																							FreeMemory (plot_width_p);
																						}


																					if (plot_length_p)
																						{
																							FreeMemory (plot_length_p);
																						}



																					/*
																					 * If the Study wasn't allocated, free any allocated
																					 * resources.
																					 */
																					if (!study_p)
																						{
																							if (current_crop_p)
																								{
																									FreeCrop (current_crop_p);
																								}

																							if (previous_crop_p)
																								{
																									FreeCrop (previous_crop_p);
																								}

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


bool HasStudyGotPlotLayoutDetails (const Study *study_p)
{
	return ((study_p -> st_num_rows_p) && (*study_p -> st_num_rows_p > 0) && (study_p -> st_num_columns_p) && (*study_p -> st_num_columns_p > 0));
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


static bool GetValidRealFromJSON (const json_t *study_json_p, const char *key_s, double64 **ph_pp)
{
	bool success_flag = false;
	double64 d;

	if (GetJSONReal (study_json_p, key_s, &d))
		{
			if (CopyValidReal (&d, ph_pp))
				{
					success_flag = true;
				}
			else
				{
					PrintJSONToErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, study_json_p, "Failed to copy double value for \"%s\"", key_s);
				}
		}
	else
		{
			success_flag = true;
		}

	return success_flag;
}


static bool GetValidUnsignedIntFromJSON (const json_t *study_json_p, const char *key_s, uint32 **value_pp)
{
	bool success_flag = false;
	uint32 u;

	if (GetJSONUnsignedInteger (study_json_p, key_s, &u))
		{
			if (CopyValidUnsignedInteger (&u, value_pp))
				{
					success_flag = true;
				}
			else
				{
					PrintJSONToErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, study_json_p, "Failed to copy uint32 value for \"%s\"", key_s);
				}
		}
	else
		{
			success_flag = true;
		}

	return success_flag;
}



static bool AddValidCropToJSON (Crop *crop_p, json_t *study_json_p, const ViewFormat format, const DFWFieldTrialServiceData *data_p)
{
	bool success_flag = false;

	if (crop_p)
		{
			json_t *crop_json_p = GetCropAsJSON (crop_p, format, data_p);

			if (crop_json_p)
				{
					if (json_object_set_new (study_json_p, ST_CURRENT_CROP_S, crop_json_p) == 0)
						{
							success_flag = true;
						}
					else
						{
							json_decref (crop_json_p);
							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, crop_json_p, "Failed to add crop to study");
						}
				}
		}
	else
		{
			success_flag = true;
		}

	return success_flag;
}


static Crop *GetStoredCropValue (const json_t *json_p, const char *key_s, const DFWFieldTrialServiceData *data_p)
{
	Crop *crop_p = NULL;
	bson_oid_t *crop_id_p = GetNewUnitialisedBSONOid ();

	if (crop_id_p)
		{
			if (GetNamedIdFromJSON (json_p, key_s, crop_id_p))
				{
					char *id_s = GetBSONOidAsString (crop_id_p);

					if (id_s)
						{
							crop_p = GetCropByIdString (id_s, data_p);

							if (!crop_p)
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "GetCropByIdString failed for \"%s\"", id_s);
								}

							FreeCopiedString (id_s);
						}		/* if (id_s) */
					else
						{
							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, json_p, "GetBSONOidAsString for \"%s\"", key_s);
						}

				}		/* if (GetNamedIdFromJSON (json_p, key_s, crop_id_p)) */
			else
				{
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, json_p, "GetNamedIdFromJSON failed for \"%s\"", key_s);
				}

			FreeBSONOid (crop_id_p);
		}		/* if (crop_id_p) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "GetNewUnitialisedBSONOid failed");
		}

	return crop_p;
}


static void *GetStudyCallback (const json_t *json_p, const ViewFormat format, const DFWFieldTrialServiceData *data_p)
{
	return GetStudyFromJSON (json_p, format, data_p);
}


static bool AddPlotsToJSON (Study *study_p, json_t *study_json_p, const ViewFormat format, JSONProcessor *processor_p, const DFWFieldTrialServiceData *data_p)
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
							json_t *plot_json_p = ProcessPlotJSON (processor_p, node_p -> pn_plot_p, format, data_p);

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


static bool SetNonTrivialString (json_t *value_p, const char *key_s, const char *value_s)
{
	return ((IsStringEmpty (value_s)) || (SetJSONString (value_p, key_s, value_s)));
}


static bool SetNonTrivialDouble (json_t *json_p, const char *key_s, const double64 *value_p)
{
	return ((value_p == NULL) || (SetJSONReal (json_p, key_s, *value_p)));
}


static bool SetNonTrivialUnsignedInt (json_t *json_p, const char *key_s, const uint32 *value_p)
{
	return ((value_p == NULL) || (SetJSONInteger (json_p, key_s, *value_p)));
}



static int32 GetNumberOfPlotsInStudy (const Study *study_p, const DFWFieldTrialServiceData *data_p)
{
	int32 res = -1;

	if (SetMongoToolCollection (data_p -> dftsd_mongo_p, data_p -> dftsd_collection_ss [DFTD_PLOT]))
		{
			bson_t *query_p = BCON_NEW (PL_PARENT_STUDY_S, BCON_OID (study_p -> st_id_p));

			/*
			 * Make the query to get the matching plots
			 */
			if (query_p)
				{
					json_t *results_p = GetAllMongoResultsAsJSON (data_p -> dftsd_mongo_p, query_p, NULL);

					if (results_p)
						{
							if (json_is_array (results_p))
								{
									res = json_array_size (results_p);
								}		/* if (json_is_array (results_p)) */

							json_decref (results_p);
						}		/* if (results_p) */

					bson_destroy (query_p);
				}		/* if (query_p) */

		}

	return res;
}



/*
 * For Client formats
 */
static bool AddParentFieldTrialToJSON (Study *study_p, json_t *study_json_p, const DFWFieldTrialServiceData *data_p)
{
	json_t *field_trial_json_p = json_object ();

	if (field_trial_json_p)
		{
			if (AddCompoundIdToJSON (field_trial_json_p, study_p -> st_parent_p -> ft_id_p))
				{
					if (SetJSONString (field_trial_json_p, FT_NAME_S, study_p -> st_parent_p -> ft_name_s))
						{
							if (json_object_set_new (study_json_p, ST_PARENT_FIELD_TRIAL_S, field_trial_json_p) == 0)
								{
									return true;
								}
						}
				}

			json_decref (field_trial_json_p);
		}

	return false;
}


static bool AddDefaultPlotValuesToJSON (const Study *study_p, json_t *study_json_p, const DFWFieldTrialServiceData *data_p)
{
	if (SetNonTrivialDouble (study_json_p, ST_PLOT_WIDTH_S, study_p -> st_default_plot_width_p))
		{
			if (SetNonTrivialDouble (study_json_p, ST_PLOT_LENGTH_S, study_p -> st_default_plot_length_p))
				{
					if (SetNonTrivialUnsignedInt (study_json_p, ST_NUMBER_OF_PLOT_ROWS_S, study_p -> st_num_rows_p))
						{
							if (SetNonTrivialUnsignedInt (study_json_p, ST_NUMBER_OF_PLOT_COLUMN_S, study_p -> st_num_columns_p))
								{
									return true;
								}
						}
				}
		}

	return false;
}


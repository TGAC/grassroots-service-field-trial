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
#include "treatment_factor.h"
#include "location.h"
#include "dfw_util.h"
#include "time_util.h"
#include "crop_jobs.h"

#include "study_jobs.h"
#include "indexing.h"
#include "programme.h"
#include "person.h"

/*
 * DB COLUMN NAMES
 */

/*
 * STATIC PROTOTYPES
 */
static void *GetStudyCallback (const json_t *json_p, const ViewFormat format, const FieldTrialServiceData *data_p);


static bool AddPlotsToJSON (Study *study_p, json_t *study_json_p, const ViewFormat format, JSONProcessor *processor_p, const FieldTrialServiceData *data_p);


static bool AddValidAspectToJSON (const Study *study_p, json_t *study_json_p);

static bool AddValidCropToJSON (Crop *crop_p, json_t *study_json_p, const char * const key_s, const ViewFormat format, const FieldTrialServiceData *data_p);

static int32 GetNumberOfPlotsInStudy (const Study *study_p, const FieldTrialServiceData *data_p);

static bool AddParentFieldTrialToJSON (Study *study_p, json_t *study_json_p, const FieldTrialServiceData *data_p);

static bool AddDefaultPlotValuesToJSON (const Study *study_p, json_t *study_json_p, const FieldTrialServiceData *data_p);


static bool AddGrandParentProgramToJSON (Study *study_p, json_t *study_json_p, const ViewFormat format, const FieldTrialServiceData *data_p);

static bool AddTreatmentsToJSON (const Study *study_p, json_t *study_json_p, const ViewFormat format);

static bool AddTreatmentsFromJSON (Study *study_p, const json_t *study_json_p, const FieldTrialServiceData *data_p);

static bool AddCommonStudyJSONValues (Study *study_p, json_t *study_json_p, const ViewFormat format, const FieldTrialServiceData *data_p);

static bool AddFrictionlessDataLink (const Study * const study_p, json_t *study_json_p, const FieldTrialServiceData *data_p);


/*
 * API FUNCTIONS
 */



Study *AllocateStudy (bson_oid_t *id_p, const char *name_s, const char *soil_s, const char *data_url_s, const char *aspect_s, const char *slope_s,
											const struct tm *sowing_date_p, const struct tm *harvest_date_p, struct Location *location_p, FieldTrial *parent_field_trial_p,
											MEM_FLAG parent_field_trial_mem, Crop *current_crop_p, Crop *previous_crop_p, const double64 *min_ph_p, const double64 *max_ph_p, const char *description_s,
											const char *design_s, const char *growing_conditions_s, const char *phenotype_gathering_notes_s,
											const uint32 *num_rows_p, const uint32 *num_cols_p, const uint32 *num_replicates_p, const double64 *plot_width_p, const double64 *plot_length_p,
											const char *weather_s, const json_t *shape_p, const double64 *plot_horizontal_gap_p, const double64 *plot_vertical_gap_p,
											const uint32 *plot_rows_per_block_p, const uint32 *plot_columns_per_block_p, const double64 *plot_block_horizontal_gap_p,
											const double64 *plot_block_vertical_gap_p,
											Person *curator_p, Person *contact_p,
											const FieldTrialServiceData *data_p)
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
																																							uint32 *copied_num_replicates_p = NULL;

																																							if (CopyValidUnsignedInteger (num_replicates_p, &copied_num_replicates_p))
																																								{
																																									json_t *copied_shape_p = NULL;

																																									if ((shape_p == NULL) || (copied_shape_p = json_deep_copy (shape_p)))
																																										{
																																											double64 *copied_plot_hgap_p = NULL;

																																											if (CopyValidReal (plot_horizontal_gap_p, &copied_plot_hgap_p))
																																												{
																																													double64 *copied_plot_vgap_p = NULL;

																																													if (CopyValidReal (plot_vertical_gap_p, &copied_plot_vgap_p))
																																														{
																																															uint32 *copied_plot_rows_per_block_p = NULL;

																																															if (CopyValidUnsignedInteger (plot_rows_per_block_p, &copied_plot_rows_per_block_p))
																																																{
																																																	uint32 *copied_plot_columns_per_block_p = NULL;

																																																	if (CopyValidUnsignedInteger (plot_columns_per_block_p, &copied_plot_columns_per_block_p))
																																																		{
																																																			double64 *copied_plot_block_horizontal_gap_p = NULL;

																																																			if (CopyValidReal (plot_block_horizontal_gap_p, &copied_plot_block_horizontal_gap_p))
																																																				{
																																																					double64 *copied_plot_block_vertical_gap_p = NULL;

																																																					if (CopyValidReal (plot_block_vertical_gap_p, &copied_plot_block_vertical_gap_p))
																																																						{
																																																							LinkedList *treatments_p = AllocateLinkedList (FreeTreatmentFactorNode);

																																																							if (treatments_p)
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
																																																											study_p -> st_num_replicates_p = copied_num_replicates_p;

																																																											study_p -> st_weather_link_s = copied_weather_s;
																																																											study_p -> st_shape_p = copied_shape_p;

																																																											study_p -> st_plot_horizontal_gap_p = copied_plot_hgap_p;
																																																											study_p -> st_plot_vertical_gap_p = copied_plot_vgap_p;
																																																											study_p -> st_plots_rows_per_block_p = copied_plot_rows_per_block_p;
																																																											study_p -> st_plots_columns_per_block_p = copied_plot_columns_per_block_p;
																																																											study_p -> st_plot_block_horizontal_gap_p = copied_plot_block_horizontal_gap_p;
																																																											study_p -> st_plot_block_vertical_gap_p = copied_plot_block_vertical_gap_p;

																																																											study_p -> st_treatments_p = treatments_p;


																																																											study_p -> st_curator_p = curator_p;
																																																											study_p -> st_contact_p = contact_p;

																																																											return study_p;
																																																										}


																																																									FreeLinkedList (treatments_p);
																																																								}		/* if (treatments_p) */


																																																							if (copied_plot_block_vertical_gap_p)
																																																								{
																																																									FreeMemory (copied_plot_block_vertical_gap_p);
																																																								}

																																																						}		/* if (CopyValidReal (plot_block_vertical_gap_p, &copied_plot_block_vertical_gap_p)) */

																																																					if (copied_plot_block_horizontal_gap_p)
																																																						{
																																																							FreeMemory (copied_plot_block_horizontal_gap_p);
																																																						}

																																																				}		/* if (CopyValidReal (plot_block_horizontal_gap_p, &plot_block_horizontal_gap_p)) */

																																																			if (copied_plot_columns_per_block_p)
																																																				{
																																																					FreeMemory (copied_plot_columns_per_block_p);
																																																				}

																																																		}		/* if (CopyValidUnsignedInteger (plots_per_vertical_block_p, &copied_plot_columns_per_block_p)) */

																																																	if (copied_plot_rows_per_block_p)
																																																		{
																																																			FreeMemory (copied_plot_rows_per_block_p);
																																																		}

																																																}		/* if (CopyValidUnsignedInteger (plots_per_horizontal_block_p, &copied_plot_rows_per_block_p)) */


																																															if (copied_plot_vgap_p)
																																																{
																																																	FreeMemory (copied_plot_vgap_p);
																																																}

																																														}		/* if (CopyValidReal (plot_vertical_gap_p, &copied_plot_vgap_p)) */

																																													if (copied_plot_hgap_p)
																																														{
																																															FreeMemory (copied_plot_hgap_p);
																																														}

																																												}		/* if (CopyValidReal (plot_horizontal_gap_p, &copied_plot_hgap_p)) */


																																											if (copied_shape_p)
																																												{
																																													json_decref (copied_shape_p);
																																												}
																																										}		/* if ((shape_p == NULL) || (copied_shape_p = json_deep_copy (shape_p))) */

																																									if (copied_num_replicates_p)
																																										{
																																											FreeMemory (copied_num_replicates_p);
																																										}

																																								}		/* if (CopyValidUnsignedInteger (num_cols_p, &copied_num_replicates_p)) */


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


	if (study_p -> st_num_replicates_p)
		{
			FreeMemory (study_p -> st_num_replicates_p);
		}


	if (study_p -> st_parent_p)
		{
			if ((study_p -> st_parent_field_trial_mem == MF_DEEP_COPY) || (study_p -> st_parent_field_trial_mem == MF_SHALLOW_COPY))
				{
					RemoveFieldTrialStudy (study_p -> st_parent_p, study_p);

					if (GetNumberOfFieldTrialStudies (study_p -> st_parent_p) == 0)
						{
							FreeFieldTrial (study_p -> st_parent_p);
						}
				}
		}

	if (study_p -> st_weather_link_s)
		{
			FreeCopiedString (study_p -> st_weather_link_s);
		}

	if (study_p -> st_shape_p)
		{
			json_decref (study_p -> st_shape_p);
		}

	if (study_p -> st_plot_horizontal_gap_p)
		{
			FreeMemory (study_p -> st_plot_horizontal_gap_p);
		}

	if (study_p -> st_plot_vertical_gap_p)
		{
			FreeMemory (study_p -> st_plot_vertical_gap_p);
		}

	if (study_p -> st_plots_rows_per_block_p)
		{
			FreeMemory (study_p -> st_plots_rows_per_block_p);
		}

	if (study_p -> st_plots_columns_per_block_p)
		{
			FreeMemory (study_p -> st_plots_columns_per_block_p);
		}

	if (study_p -> st_plot_block_horizontal_gap_p)
		{
			FreeMemory (study_p -> st_plot_block_horizontal_gap_p);
		}

	if (study_p -> st_plot_block_vertical_gap_p)
		{
			FreeMemory (study_p -> st_plot_block_vertical_gap_p);
		}

	if (study_p -> st_treatments_p)
		{
			FreeLinkedList (study_p -> st_treatments_p);
		}


	if (study_p -> st_curator_p)
		{
			FreePerson (study_p -> st_curator_p);
		}

	if (study_p -> st_contact_p)
		{
			FreePerson (study_p -> st_contact_p);
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


bool AddStudyPlotsJSONDirectly (Study *study_p, json_t *study_json_p,  const FieldTrialServiceData *data_p)
{
	bool success_flag = false;

	return success_flag;
}


bool GetStudyPlots (Study *study_p, const FieldTrialServiceData *data_p)
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


OperationStatus SaveStudy (Study *study_p, ServiceJob *job_p, FieldTrialServiceData *data_p)
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
									status = OS_SUCCEEDED;

									if (data_p -> dftsd_study_cache_path_s)
										{
											ClearCachedStudy (id_s, data_p);
										}

									if (data_p -> dftsd_fd_path_s)
										{
											if (!SaveStudyAsFrictionlessData (study_p, data_p))
												{

												}
										}


									FreeCopiedString (id_s);
								}
							else
								{
									status = OS_PARTIALLY_SUCCEEDED;

									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to clear potential cached Study \"%s\"", study_p -> st_name_s);
								}

						}

					json_decref (study_json_p);
				}		/* if (study_json_p) */

			if ((status == OS_SUCCEEDED) || (status == OS_PARTIALLY_SUCCEEDED))
				{
					study_json_p = GetStudyAsJSON (study_p, VF_CLIENT_MINIMAL, NULL, data_p);

					if (study_json_p)
						{
							status = IndexData (job_p, study_json_p);

							if (status != OS_SUCCEEDED)
								{
									status = OS_PARTIALLY_SUCCEEDED;
									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, study_json_p, "Failed to index Study \"%s\" as JSON to Lucene", study_p -> st_name_s);
									AddGeneralErrorMessageToServiceJob (job_p, "Study saved but failed to index for searching");
								}

							json_decref (study_json_p);
						}
					else
						{
							status = OS_PARTIALLY_SUCCEEDED;

							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "GetStudyAsJSON for \"%s\" failed", study_p -> st_name_s);
							AddGeneralErrorMessageToServiceJob (job_p, "Study saved but failed to index for searching");
						}
				}

		}		/* if (success_flag) */

	SetServiceJobStatus (job_p, status);

	return status;
}



json_t *GetStudyAsJSON (Study *study_p, const ViewFormat format, JSONProcessor *processor_p, const FieldTrialServiceData *data_p)
{
	json_t *study_json_p = json_object ();

	if (study_json_p)
		{
			if (AddCommonStudyJSONValues (study_p, study_json_p, format, data_p))
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
											if (AddValidCropToJSON (study_p -> st_current_crop_p, study_json_p, ST_CURRENT_CROP_S, format, data_p))
												{
													if (AddValidCropToJSON (study_p -> st_previous_crop_p, study_json_p, ST_PREVIOUS_CROP_S, format, data_p))
														{
															if (AddParentFieldTrialToJSON (study_p, study_json_p, data_p))
																{
																	if (AddGrandParentProgramToJSON (study_p, study_json_p, format, data_p))
																		{
																			AddFrictionlessDataLink (study_p, study_json_p, data_p);

																			add_item_flag = true;
																		}
																	else
																		{
																			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, study_json_p, "Failed to add grandparent program to study \"%s\"", study_p -> st_parent_p -> ft_name_s);
																		}
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
											if (AddValidCropToJSON (study_p -> st_current_crop_p, study_json_p, ST_CURRENT_CROP_S, VF_STORAGE, data_p))
												{
													if (AddValidCropToJSON (study_p -> st_previous_crop_p, study_json_p, ST_PREVIOUS_CROP_S, VF_STORAGE, data_p))
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
									if (AddValidDateToJSON (study_p -> st_sowing_date_p, study_json_p, ST_SOWING_DATE_S, false))
										{
											if (AddValidDateToJSON (study_p -> st_harvest_date_p, study_json_p, ST_HARVEST_DATE_S, false))
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

				}		/* if (AddCommonStudyJSONValues (study_p, study_json_p, format, data_p)) */

		}		/* if (study_json_p) */

	return NULL;
}


Study *GetStudyFromJSON (const json_t *json_p, const ViewFormat format, const FieldTrialServiceData *data_p)
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
																			char *id_s = GetBSONOidAsString (location_id_p);

																			if (id_s)
																				{
																					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, json_p, "GetLocationById failed for %s", id_s);
																					FreeCopiedString (id_s);
																				}
																			else
																				{
																					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, json_p, "GetLocationById failed");
																				}

																			success_flag = false;
																		}
																	else if (! (trial_p = GetFieldTrialById (parent_field_trial_id_p, format, data_p)))
																		{
																			char *id_s = GetBSONOidAsString (parent_field_trial_id_p);

																			if (id_s)
																				{
																					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, json_p, "GetFieldTrialById failed for %s", id_s);
																					FreeCopiedString (id_s);
																				}
																			else
																				{
																					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, json_p, "GetFieldTrialById failed");
																				}


																			success_flag = false;
																		}

																}		/* if ((format == VF_CLIENT_FULL) || (format == VF_CLIENT_MINIMAL)) */

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
																					uint32 *num_replicates_p = NULL;
																					double64 *plot_horizontal_gap_p = NULL;
																					double64 *plot_vertical_gap_p = NULL;
																					uint32 *plot_rows_per_block_p = NULL;
																					uint32 *plots_columns_per_block_p = NULL;
																					double64 *plot_block_horizontal_gap_p = NULL;
																					double64 *plot_block_vertical_gap_p = NULL;


																					const char *weather_s = GetJSONString (json_p, ST_WEATHER_S);
																					const json_t *shape_p = json_object_get (json_p, ST_SHAPE_S);

																					Person *curator_p = GetPersonFromCompoundJSON (json_p, ST_CURATOR_S, format, data_p);
																					Person *contact_p = GetPersonFromCompoundJSON (json_p, ST_CONTACT_S, format, data_p);


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
																					GetValidUnsignedIntFromJSON (json_p, ST_NUMBER_OF_REPLICATES_S, &num_replicates_p);

																					GetValidRealFromJSON (json_p, ST_PLOT_WIDTH_S, &plot_width_p);
																					GetValidRealFromJSON (json_p, ST_PLOT_LENGTH_S, &plot_length_p);


																					GetValidRealFromJSON (json_p, ST_PLOT_H_GAP_S, &plot_horizontal_gap_p);
																					GetValidRealFromJSON (json_p, ST_PLOT_V_GAP_S, &plot_vertical_gap_p);

																					GetValidUnsignedIntFromJSON (json_p, ST_PLOT_ROWS_PER_BLOCK_S, &plot_rows_per_block_p);
																					GetValidUnsignedIntFromJSON (json_p, ST_PLOT_COLS_PER_BLOCK_S, &plots_columns_per_block_p);

																					GetValidRealFromJSON (json_p, ST_PLOT_BLOCK_H_GAP_S, &plot_block_horizontal_gap_p);
																					GetValidRealFromJSON (json_p, ST_PLOT_BLOCK_V_GAP_S, &plot_block_vertical_gap_p);


																					study_p = AllocateStudy (id_p, name_s, soil_s, data_url_s, aspect_s, slope_s, sowing_date_p, harvest_date_p, location_p, trial_p, MF_SHALLOW_COPY, current_crop_p, previous_crop_p,
																																	 min_ph_p, max_ph_p, description_s, design_s, growing_conditions_s, phenotype_gathering_notes_s,
																																	 num_plot_rows_p, num_plot_columns_p, num_replicates_p, plot_width_p, plot_length_p,
																																	 weather_s, shape_p, plot_horizontal_gap_p, plot_vertical_gap_p, plot_rows_per_block_p, plots_columns_per_block_p, plot_block_horizontal_gap_p,
																																	 plot_block_vertical_gap_p,
																																	 curator_p, contact_p,
																																	 data_p);

																					if (study_p)
																						{
																							if (AddTreatmentsFromJSON (study_p, json_p, data_p))
																								{
																									return study_p;
																								}
																						}
																					/*
																					 *
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


																					if (num_replicates_p)
																						{
																							FreeMemory (num_replicates_p);
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
																							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, json_p, "AllocateStudy failed");

																							if (current_crop_p)
																								{
																									FreeCrop (current_crop_p);
																								}

																							if (previous_crop_p)
																								{
																									FreeCrop (previous_crop_p);
																								}

																							if (contact_p)
																								{
																									FreePerson (contact_p);
																								}

																							if (curator_p)
																								{
																									FreePerson (curator_p);
																								}

																						}		/* if (!study_p) */

																				}		/* if (CreateValidDateFromJSON (json_p, ST_HARVEST_DATE_S, &harvest_date_p)) */
																			else
																				{
																					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, json_p, "CreateValidDateFromJSON failed for %s", ST_HARVEST_DATE_S);
																				}

																			/*
																			 * The dates are copied by AllocateStudy so we can free our values.
																			 */
																			if (sowing_date_p)
																				{
																					FreeTime (sowing_date_p);
																				}

																		}		/* if (CreateValidDateFromJSON (json_p, ST_SOWING_DATE_S, &sowing_date_p)) */
																	else
																		{
																			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, json_p, "CreateValidDateFromJSON failed for %s", ST_SOWING_DATE_S);
																		}

																}		/* if (success_flag) */

														}		/* if (GetMongoIdFromJSON (json_p, id_p)) */
													else
														{
															PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, json_p, "GetMongoIdFromJSON failed to get %s", ST_LOCATION_S);
														}

													if (!study_p)
														{
															FreeBSONOid (id_p);
														}

												}
											else
												{
													PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "GetNewUnitialisedBSONOid failed for the study id for %s", name_s);
												}

										}		/* if (GetNamedIdFromJSON (json_p, ST_LOCATION_S, address_id_p)) */
									else
										{
											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, json_p, "GetNamedIdFromJSON failed to get %s", ST_LOCATION_S);
										}

									FreeBSONOid (location_id_p);
								}		/* if (location_id_p) */
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "GetNewUnitialisedBSONOid failed for location for %s", name_s);
								}

						}		/* if (GetNamedIdFromJSON (json_p, ST_PARENT_FIELD_TRIAL_S, parent_field_trial_id_p)) */
					else
						{
							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, json_p, "GetNamedIdFromJSON failed to get %s", ST_PARENT_FIELD_TRIAL_S);
						}

					FreeBSONOid (parent_field_trial_id_p);
				}		/* if (parent_field_trial_id_p) */
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "GetNewUnitialisedBSONOid failed for parent field trial for %s", name_s);
				}

		}		/* if (name_s) */
	else
		{
			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, json_p, "Failed to get %s", ST_NAME_S);
		}

	return study_p;
}


Study *GetStudyByIdString (const char *study_id_s, const ViewFormat format, const FieldTrialServiceData *data_p)
{
	Study *study_p = GetDFWObjectByIdString (study_id_s, DFTD_STUDY, GetStudyCallback, format, data_p);

	return study_p;
}


Study *GetStudyById (bson_oid_t *study_id_p, const ViewFormat format, const FieldTrialServiceData *data_p)
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



static bool AddValidCropToStorageJSON (Crop *crop_p, json_t *study_json_p, const char * const key_s, const ViewFormat format, const FieldTrialServiceData *data_p)
{
	bool success_flag = false;

	if (crop_p)
		{
			json_t *crop_json_p = GetCropAsJSON (crop_p, format, data_p);

			if (crop_json_p)
				{
					if (json_object_set_new (study_json_p, key_s, crop_json_p) == 0)
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
			if (json_object_set_new (study_json_p, key_s, json_null ()) == 0)
				{
					success_flag = true;
				}
		}

	return success_flag;
}




static bool AddValidCropToJSON (Crop *crop_p, json_t *study_json_p, const char * const key_s, const ViewFormat format, const FieldTrialServiceData *data_p)
{
	bool success_flag = false;

	if (crop_p)
		{
			if (format == VF_STORAGE)
				{
					if (AddNamedCompoundIdToJSON (study_json_p, crop_p -> cr_id_p, key_s))
						{
							success_flag = true;
						}
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add crop \"%s\" to study", crop_p -> cr_name_s);
						}
				}
			else
				{
					json_t *crop_json_p = crop_json_p = GetCropAsJSON (crop_p, format, data_p);

					if (crop_json_p)
						{
							if (json_object_set_new (study_json_p, key_s, crop_json_p) == 0)
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
		}
	else
		{
			if (json_object_set_new (study_json_p, key_s, json_null ()) == 0)
				{
					success_flag = true;
				}
		}

	return success_flag;
}



static void *GetStudyCallback (const json_t *json_p, const ViewFormat format, const FieldTrialServiceData *data_p)
{
	return GetStudyFromJSON (json_p, format, data_p);
}


static bool AddPlotsToJSON (Study *study_p, json_t *study_json_p, const ViewFormat format, JSONProcessor *processor_p, const FieldTrialServiceData *data_p)
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




static int32 GetNumberOfPlotsInStudy (const Study *study_p, const FieldTrialServiceData *data_p)
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
static bool AddParentFieldTrialToJSON (Study *study_p, json_t *study_json_p, const FieldTrialServiceData *data_p)
{
	if (study_p -> st_parent_p)
		{
			json_t *field_trial_json_p = json_object ();

			if (field_trial_json_p)
				{
					if (AddCompoundIdToJSON (field_trial_json_p, study_p -> st_parent_p -> ft_id_p))
						{
							if (SetJSONString (field_trial_json_p, FT_NAME_S, study_p -> st_parent_p -> ft_name_s))
								{
									if (SetJSONString (field_trial_json_p, FT_TEAM_S, study_p -> st_parent_p -> ft_team_s))
										{
											if (json_object_set_new (study_json_p, ST_PARENT_FIELD_TRIAL_S, field_trial_json_p) == 0)
												{
													return true;
												}
										}
								}
						}

					json_decref (field_trial_json_p);
				}
		}

	return false;
}


/*
 * For Client formats
 */
static bool AddGrandParentProgramToJSON (Study *study_p, json_t *study_json_p, const ViewFormat format, const FieldTrialServiceData *data_p)
{
	FieldTrial *trial_p = study_p -> st_parent_p;

	if (trial_p)
		{
			Programme *program_p = trial_p -> ft_parent_p;

			if (program_p)
				{
					json_t *program_json_p = GetProgrammeAsJSON (program_p, format, data_p);

					if (program_json_p)
						{
							if (json_object_set_new (study_json_p, FT_PARENT_PROGRAM_S, program_json_p) == 0)
								{
									return true;
								}
							else
								{
									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, program_json_p, "Failed to add program to study \"%s\"", study_p -> st_name_s);
									json_decref (program_json_p);
								}
						}
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get program \"%s\" as json", program_p  -> pr_name_s);
						}
				}
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "no program for trial \"%s\"", trial_p -> ft_name_s);
				}
		}
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "no parent program for study \"%s\"", study_p -> st_name_s);
		}

	return false;
}



static bool AddDefaultPlotValuesToJSON (const Study *study_p, json_t *study_json_p, const FieldTrialServiceData *data_p)
{
	if (SetNonTrivialDouble (study_json_p, ST_PLOT_WIDTH_S, study_p -> st_default_plot_width_p, true))
		{
			if (SetNonTrivialDouble (study_json_p, ST_PLOT_LENGTH_S, study_p -> st_default_plot_length_p, true))
				{
					if (SetNonTrivialUnsignedInt (study_json_p, ST_NUMBER_OF_PLOT_ROWS_S, study_p -> st_num_rows_p, true))
						{
							if (SetNonTrivialUnsignedInt (study_json_p, ST_NUMBER_OF_PLOT_COLUMN_S, study_p -> st_num_columns_p, true))
								{
									if (SetNonTrivialUnsignedInt (study_json_p, ST_NUMBER_OF_REPLICATES_S, study_p -> st_num_replicates_p, true))
										{
											return true;
										}
								}
						}
				}
		}

	return false;
}


static bool AddTreatmentsToJSON (const Study *study_p, json_t *study_json_p, const ViewFormat format)
{
	bool success_flag = false;

	if (study_p -> st_treatments_p)
		{
			json_t *treatments_json_p = json_array ();

			if (treatments_json_p)
				{
					bool b = true;
					TreatmentFactorNode *node_p = (TreatmentFactorNode *) (study_p -> st_treatments_p -> ll_head_p);

					while (node_p && b)
						{
							json_t *treatment_json_p = GetTreatmentFactorAsJSON (node_p -> tfn_p, format);

							if (treatment_json_p)
								{
									if (json_array_append_new (treatments_json_p, treatment_json_p) == 0)
										{
											node_p = (TreatmentFactorNode *) (node_p -> tfn_node.ln_next_p);
										}
									else
										{
											json_decref (treatment_json_p);
											b = false;
										}
								}
							else
								{
									b = false;
								}

						}

					if (json_array_size (treatments_json_p) == study_p -> st_treatments_p -> ll_size)
						{
							if (json_object_set_new (study_json_p, ST_TREATMENTS_S, treatments_json_p) == 0)
								{
									return true;
								}
						}

					json_decref (treatments_json_p);
				}		/* if (treatments_json_p) */

		}		/* if (study_p -> st_treatments_p) */
	else
		{
			return true;
		}

	return false;
}


static bool AddCommonStudyJSONValues (Study *study_p, json_t *study_json_p, const ViewFormat format, const FieldTrialServiceData *data_p)
{
	bool success_flag = false;

	if (SetJSONString (study_json_p, ST_NAME_S, study_p -> st_name_s))
		{
			if (SetNonTrivialDouble (study_json_p, ST_MIN_PH_S, study_p -> st_min_ph_p, true))
				{
					if (SetNonTrivialDouble (study_json_p, ST_MAX_PH_S, study_p -> st_max_ph_p, true))
						{
							if (SetNonTrivialString (study_json_p, ST_DESCRIPTION_S, study_p -> st_description_s, true))
								{
									if (SetNonTrivialString (study_json_p, ST_GROWING_CONDITIONS_S, study_p -> st_growing_conditions_s, true))
										{
											if (SetNonTrivialString (study_json_p, ST_DESIGN_S, study_p -> st_design_s, true))
												{
													if (SetNonTrivialString (study_json_p, ST_GROWING_CONDITIONS_S, study_p -> st_growing_conditions_s, true))
														{
															if (SetNonTrivialString (study_json_p, ST_PHENOTYPE_GATHERING_NOTES_S, study_p -> st_phenotype_gathering_notes_s, true))
																{
																	if (SetNonTrivialString (study_json_p, ST_DATA_LINK_S, study_p -> st_data_url_s, true))
																		{
																			if (SetNonTrivialString (study_json_p, ST_SOIL_S, study_p -> st_soil_type_s, true))
																				{
																					if (SetNonTrivialString (study_json_p, ST_SLOPE_S, study_p -> st_slope_s, true))
																						{
																							if (SetNonTrivialString (study_json_p, ST_WEATHER_S, study_p -> st_weather_link_s, true))
																								{
																									if (AddValidAspectToJSON (study_p, study_json_p))
																										{
																											if (AddDefaultPlotValuesToJSON (study_p, study_json_p, data_p))
																												{
																													if ((!study_p -> st_shape_p) || (json_object_set (study_json_p, ST_SHAPE_S, study_p -> st_shape_p) == 0))
																														{
																															if (SetNonTrivialDouble (study_json_p, ST_PLOT_H_GAP_S, study_p -> st_plot_horizontal_gap_p, true))
																																{
																																	if (SetNonTrivialDouble (study_json_p, ST_PLOT_V_GAP_S, study_p -> st_plot_vertical_gap_p, true))
																																		{
																																			if (SetNonTrivialUnsignedInt (study_json_p, ST_PLOT_ROWS_PER_BLOCK_S, study_p -> st_plots_rows_per_block_p, true))
																																				{
																																					if (SetNonTrivialUnsignedInt (study_json_p, ST_PLOT_COLS_PER_BLOCK_S, study_p -> st_plots_columns_per_block_p, true))
																																						{
																																							if (SetNonTrivialDouble (study_json_p, ST_PLOT_BLOCK_H_GAP_S, study_p -> st_plot_block_horizontal_gap_p, true))
																																								{
																																									if (SetNonTrivialDouble (study_json_p, ST_PLOT_BLOCK_V_GAP_S, study_p -> st_plot_block_vertical_gap_p, true))
																																										{
																																											if (AddTreatmentsToJSON (study_p, study_json_p, format))
																																												{
																																													if ((study_p -> st_curator_p == NULL) || (AddPersonToCompoundJSON (study_p -> st_curator_p, study_json_p, ST_CURATOR_S, format, data_p)))
																																														{
																																															if ((study_p -> st_contact_p == NULL) || (AddPersonToCompoundJSON (study_p -> st_contact_p, study_json_p, ST_CONTACT_S, format, data_p)))
																																																{
																																																	success_flag = true;
																																																}

																																														}


																																												}		/* if (AddTreatmentsToJSON (study_p, study_json_p, format)) */

																																										}

																																								}


																																						}

																																				}


																																		}

																																}


																														}		/* if ((!study_p -> st_shape_p) || (json_object_set (study_json_p, ST_SHAPE_S, study_p -> st_shape_p) == 0)) */


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

										}		/* if ((study_p -> st_min_ph == ST_UNSET_PH) || (SetJSONInteger (study_json_p, ST_MIN_PH_S, study_p -> st_min_ph))) */
								}
						}

				}		/* if ((study_p -> st_min_ph == ST_UNSET_PH) || (SetJSONInteger (study_json_p, ST_MIN_PH_S, study_p -> st_min_ph))) */

		}		/* if (SetJSONString (study_json_p, ST_NAME_S, study_p -> st_name_s) == 0) */

	return success_flag;
}



static bool AddTreatmentsFromJSON (Study *study_p, const json_t *study_json_p, const FieldTrialServiceData *data_p)
{
	bool success_flag = true;
	const json_t *treatment_factors_json_p = json_object_get (study_json_p, ST_TREATMENTS_S);

	if (treatment_factors_json_p)
		{
			size_t i;
			const size_t size = json_array_size (treatment_factors_json_p);

			for (i = 0; i < size; ++ i)
				{
					const json_t *treatment_factor_json_p = json_array_get (treatment_factors_json_p, i);
					TreatmentFactor *tf_p = GetTreatmentFactorFromJSON (treatment_factor_json_p, study_p, data_p);

					if (tf_p)
						{
							TreatmentFactorNode *node_p = AllocateTreatmentFactorNode (tf_p);

							if (node_p)
								{
									LinkedListAddTail (study_p -> st_treatments_p, & (node_p -> tfn_node));
								}
							else
								{
									i = size;		/* force exit from loop */
								}
						}
					else
						{
							i = size;		/* force exit from loop */
						}

				}		/* for (i = 0; i < size; ++ i) */

			if (study_p -> st_treatments_p -> ll_size != size)
				{
					success_flag = false;
				}

		}		/* if (treatments_json_p) */

	return success_flag;
}


static bool AddFrictionlessDataLink (const Study * const study_p, json_t *study_json_p, const FieldTrialServiceData *data_p)
{
	bool success_flag = false;

	char *fd_url_s = GetStudyFrictionlessDataURL (study_p, data_p);

	if (fd_url_s)
		{
			if (SetJSONString (study_json_p, ST_FRICTIONLESS_DATA_LINK_S, fd_url_s))
				{
					success_flag = true;
				}

			FreeCopiedString (fd_url_s);
		}

	return success_flag;
}


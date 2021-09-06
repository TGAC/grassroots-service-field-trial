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
 * row.c
 *
 *  Created on: 28 Sep 2018
 *      Author: billy
 */

#define ALLOCATE_ROW_TAGS (1)
#include "row.h"

#include "memory_allocations.h"
#include "string_utils.h"
#include "streams.h"
#include "observation.h"
#include "dfw_util.h"
#include "plot_jobs.h"
#include "treatment_factor_value.h"


static bool AddObservationsToJSON (json_t *row_json_p, LinkedList *observations_p, const ViewFormat format);

static bool GetObservationsFromJSON (const json_t *row_json_p, Row *row_p, const FieldTrialServiceData *data_p);


static bool GetTreatmentFactorValuesFromJSON (const json_t *row_json_p, Row *row_p, const Study *study_p, const FieldTrialServiceData *data_p);

static bool AddTreatmentFactorsToJSON (json_t *row_json_p, LinkedList *treatment_factors_p, const Study *study_p, const ViewFormat format);


Row *AllocateRow (bson_oid_t *id_p, const uint32 rack_index, const uint32 study_index, const uint32 replicate, Material *material_p, MEM_FLAG material_mem, Plot *parent_plot_p)
{
	if (material_p)
		{
			LinkedList *observations_p = AllocateLinkedList (FreeObservationNode);

			if (observations_p)
				{
					LinkedList *tf_values_p = AllocateLinkedList (FreeTreatmentFactorValueNode);

					if (tf_values_p)
						{
							Row *row_p = (Row *) AllocMemory (sizeof (Row));

							if (row_p)
								{
									bool success_flag = true;

									if (!id_p)
										{
											id_p = GetNewBSONOid ();

											if (!id_p)
												{
													PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate BSON oid for row at [" UINT32_FMT ", " UINT32_FMT "] for study \"%s\"", parent_plot_p -> pl_parent_p -> st_name_s);
													success_flag = false;
												}
										}

									if (success_flag)
										{
											row_p -> ro_id_p = id_p;

											row_p -> ro_rack_index = rack_index;
											row_p -> ro_by_study_index = study_index;
											row_p -> ro_material_p = material_p;
											row_p -> ro_material_mem = material_mem;
											row_p -> ro_plot_p = parent_plot_p;
											row_p -> ro_study_p = parent_plot_p -> pl_parent_p;
											row_p -> ro_observations_p = observations_p;
											row_p -> ro_treatment_factor_values_p = tf_values_p;
											row_p -> ro_replicate_index = replicate;
											row_p -> ro_replicate_control_flag = false;

											return row_p;
										}

									FreeMemory (row_p);
								}
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate row " UINT32_FMT " at [" UINT32_FMT "," UINT32_FMT "]", parent_plot_p -> pl_row_index, parent_plot_p -> pl_column_index, index);
								}

							FreeLinkedList (tf_values_p);
						}		/* if (tf_values_p) */
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate treatment factors list " UINT32_FMT " at [" UINT32_FMT "," UINT32_FMT "]", parent_plot_p -> pl_row_index, parent_plot_p -> pl_column_index, index);
						}


					FreeLinkedList (observations_p);
				}
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate observations list " UINT32_FMT " at [" UINT32_FMT "," UINT32_FMT "]", parent_plot_p -> pl_row_index, parent_plot_p -> pl_column_index, index);
				}
		}
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "No valid material for row " UINT32_FMT " at [" UINT32_FMT "," UINT32_FMT "]", parent_plot_p -> pl_row_index, parent_plot_p -> pl_column_index, index);
		}

	return NULL;
}


void FreeRow (Row *row_p)
{
	FreeLinkedList (row_p -> ro_treatment_factor_values_p);

	FreeLinkedList (row_p -> ro_observations_p);

	if ((row_p -> ro_material_mem == MF_DEEP_COPY) || (row_p -> ro_material_mem == MF_SHALLOW_COPY))
		{
			if (row_p -> ro_material_p)
				{
					FreeMaterial (row_p -> ro_material_p);
				}
		}

	FreeBSONOid (row_p -> ro_id_p);

	FreeMemory (row_p);
}


RowNode *AllocateRowNode (Row *row_p)
{
	RowNode *ro_node_p = (RowNode *) AllocMemory (sizeof (RowNode));

	if (ro_node_p)
		{
			InitListItem (& (ro_node_p -> rn_node));

			ro_node_p -> rn_row_p = row_p;
		}

	return ro_node_p;
}



void FreeRowNode (ListItem *node_p)
{
	RowNode *ro_node_p = (RowNode *) node_p;

	if (ro_node_p -> rn_row_p)
		{
			FreeRow (ro_node_p -> rn_row_p);
		}

	FreeMemory (ro_node_p);
}



json_t *GetRowAsJSON (const Row *row_p, const ViewFormat format, JSONProcessor *processor_p, const FieldTrialServiceData *data_p)
{
	json_t *row_json_p = json_object ();

	if (row_json_p)
		{
			bool success_flag = false;

			if (row_p -> ro_replicate_control_flag)
				{
					success_flag = SetJSONString (row_json_p, RO_REPLICATE_S, RO_REPLICATE_CONTROL_S);
				}
			else
				{
					success_flag = SetJSONInteger (row_json_p, RO_REPLICATE_S, row_p -> ro_replicate_index);
				}

			if (success_flag)
				{
					if (row_p -> ro_material_p)
						{
							switch (format)
								{
									case VF_CLIENT_FULL:
										{
											json_t *material_json_p = GetMaterialAsJSON (row_p -> ro_material_p, true, data_p);

											if (material_json_p)
												{
													if (json_object_set_new (row_json_p, RO_MATERIAL_S, material_json_p) == 0)
														{
															success_flag = true;
														}
													else
														{
															PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, material_json_p, "Failed to add material to row json");
															json_decref (material_json_p);
														}
												}
											else
												{
													PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "GetMaterialAsJSON failed for \"%s\"", row_p -> ro_material_p -> ma_accession_s);
												}
										}
										break;

									case VF_STORAGE:
										{
											if (AddNamedCompoundIdToJSON (row_json_p, row_p -> ro_material_p -> ma_id_p, RO_MATERIAL_ID_S))
												{
													success_flag = true;
												}
											else
												{
													PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, row_json_p, "Failed to add material \"%s\" to row json", row_p -> ro_material_p -> ma_accession_s);
												}
										}
										break;

									default:
										break;
								}
						}
					else
						{
							success_flag = true;
						}

					if (success_flag)
						{
							if (SetJSONInteger (row_json_p, RO_RACK_INDEX_S, row_p -> ro_rack_index))
								{
									if (SetJSONInteger (row_json_p, RO_STUDY_INDEX_S, row_p -> ro_by_study_index))
										{
											/*
											 * We only need to store the parent plot id if the JSON is for the backend
											 */
											if (format == VF_STORAGE)
												{
													if (AddNamedCompoundIdToJSON (row_json_p, row_p -> ro_plot_p -> pl_id_p, RO_PLOT_ID_S))
														{
															if (AddNamedCompoundIdToJSON (row_json_p, row_p -> ro_study_p -> st_id_p, RO_STUDY_ID_S))
																{

																}		/* if (AddNamedCompoundIdToJSON (row_json_p, row_p -> ro_plot_p -> pl_id_p, RO_PLOT_ID_S)) */
															else
																{
																	success_flag = false;
																	PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, row_json_p, "Failed to add id for for plot [" UINT32_FMT, ", " UINT32_FMT "] in study \"%s\"",
																										 row_p -> ro_plot_p -> pl_row_index, row_p -> ro_plot_p -> pl_column_index, row_p -> ro_plot_p -> pl_parent_p -> st_name_s);
																}

														}		/* if (AddNamedCompoundIdToJSON (row_json_p, row_p -> ro_plot_p -> pl_id_p, RO_PLOT_ID_S)) */
													else
														{
															success_flag = false;
															PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, row_json_p, "Failed to add id for for plot [" UINT32_FMT, ", " UINT32_FMT "] in study \"%s\"",
																								 row_p -> ro_plot_p -> pl_row_index, row_p -> ro_plot_p -> pl_column_index, row_p -> ro_plot_p -> pl_parent_p -> st_name_s);
														}
												}		/* if (format == VF_STORAGE) */

											if (success_flag)
												{
													if (AddCompoundIdToJSON (row_json_p, row_p -> ro_id_p))
														{
															if (AddObservationsToJSON (row_json_p, row_p -> ro_observations_p, format))
																{
																	if (AddTreatmentFactorsToJSON (row_json_p, row_p -> ro_treatment_factor_values_p,  row_p -> ro_study_p, format))
																		{
																			return row_json_p;
																		}		/* if (AddTreatmentFactorsToJSON (row_json_p, row_p -> ro_treatment_factor_values_p, format)) */
																	else
																		{
																			char *id_s = GetBSONOidAsString (row_p -> ro_id_p);

																			if (id_s)
																				{
																					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, row_json_p, "AddTreatmentFactorsToJSON failed for row \"%s\"", id_s);
																					FreeCopiedString (id_s);
																				}
																			else
																				{
																					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, row_json_p, "AddTreatmentFactorsToJSON failed for row");
																				}

																		}


																}		/* if (AddObservationsToJSON (row_json_p, row_p -> ro_observations_p, format)) */
															else
																{
																	char *id_s = GetBSONOidAsString (row_p -> ro_id_p);

																	if (id_s)
																		{
																			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, row_json_p, "AddObservationsToJSON failed for row \"%s\"", id_s);
																			FreeCopiedString (id_s);
																		}
																	else
																		{
																			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, row_json_p, "AddObservationsToJSON failed for row");
																		}

																}

														}		/* if (AddCompoundIdToJSON (row_json_p, row_p -> ro_id_p)) */
													else
														{
															char *id_s = GetBSONOidAsString (row_p -> ro_id_p);

															if (id_s)
																{
																	PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, row_json_p, "Failed to add row compound id \"%s\"", id_s);
																	FreeCopiedString (id_s);
																}
															else
																{
																	PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, row_json_p, "Failed to row add compound id");
																}
														}

												}		/* if (success_flag) */

										}		/* if (SetJSONInteger (row_json_p, RO_STUDY_INDEX_S, row_p -> ro_by_study_index)) */
									else
										{
											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, row_json_p, "Failed to add \"%s\": " UINT32_FMT, RO_STUDY_INDEX_S, row_p -> ro_by_study_index);
										}


								}		/* if (SetJSONInteger (row_json_p, RO_INDEX_S, row_p -> ro_index)) */
							else
								{
									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, row_json_p, "Failed to add \"%s\": " UINT32_FMT, RO_RACK_INDEX_S, row_p -> ro_rack_index);
								}
						}
				}		/* if (success_flag) */

			json_decref (row_json_p);
		}		/* if (row_json_p) */

	return NULL;
}


Row *GetRowFromJSON (const json_t *json_p, Plot *plot_p, Material *material_p, const Study *study_p, const ViewFormat format, const FieldTrialServiceData *data_p)
{
	Row *row_p = NULL;
	Material *material_to_use_p = material_p;

	if (!plot_p)
		{
			bson_oid_t *plot_id_p = GetNewUnitialisedBSONOid ();

			if (plot_id_p)
				{
					if (GetNamedIdFromJSON (json_p, RO_PLOT_ID_S, plot_id_p))
						{
							plot_p = GetPlotById (plot_id_p, NULL, data_p);
						}
				}
		}

	if (plot_p)
		{
			bool success_flag = true;

			if (format == VF_CLIENT_FULL)
				{
					/*
					 * If we haven't already got the material, get it!
					 */
					if (!material_to_use_p)
						{
							bson_oid_t *material_id_p = GetNewUnitialisedBSONOid ();

							success_flag = true;

							if (material_id_p)
								{
									if (GetNamedIdFromJSON (json_p, RO_MATERIAL_ID_S, material_id_p))
										{
											material_to_use_p = GetMaterialById (material_id_p, data_p);

											if (material_to_use_p)
												{
													success_flag = true;
												}
											else
												{
													char id_s [MONGO_OID_STRING_BUFFER_SIZE];

													bson_oid_to_string (material_id_p, id_s);
													PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, json_p, "Failed to get material with id %s", id_s);
												}

										}		/* if (GetNamedIdFromJSON (json_p, RO_MATERIAL_ID_S, material_id_p)) */
									else
										{
											char plot_id_s [MONGO_OID_STRING_BUFFER_SIZE];

											bson_oid_to_string (plot_p -> pl_id_p, plot_id_s);

											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, json_p, "Failed to get row's material id for plot \"%s\" at [" UINT32_FMT ", " UINT32_FMT "]", plot_id_s, plot_p -> pl_row_index, plot_p -> pl_column_index);
										}

									FreeBSONOid (material_id_p);
								}		/* if (material_id_p) */
							else
								{
									char id_s [MONGO_OID_STRING_BUFFER_SIZE];

									bson_oid_to_string (plot_p -> pl_id_p, id_s);
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate row's material id for plot \"%s\" at [" UINT32_FMT ", " UINT32_FMT "]", id_s, plot_p -> pl_row_index, plot_p -> pl_column_index);
								}

						}		/* if (!material_p) */

					success_flag = (material_to_use_p != NULL);
				}		/* if (format == VF_CLIENT_FULL) */

			if (success_flag)
				{
					bson_oid_t *id_p = GetNewUnitialisedBSONOid ();

					if (id_p)
						{
							if (GetMongoIdFromJSON (json_p, id_p))
								{
									json_int_t rack_index = -1;

									if (GetJSONInteger (json_p, RO_RACK_INDEX_S, &rack_index))
										{
											json_int_t study_index = -1;

											if (GetJSONInteger (json_p, RO_STUDY_INDEX_S, &study_index))
												{
													bool rep_control_flag = false;
													uint32 replicate = 1;
													const json_t *rep_json_p = json_object_get (json_p, RO_REPLICATE_S);

													if (rep_json_p)
														{
															if (json_is_string (rep_json_p))
																{
																	const char *rep_s = json_string_value (rep_json_p);

																	if (rep_s)
																		{
																			if (Stricmp (rep_s, RO_REPLICATE_CONTROL_S) == 0)
																				{
																					rep_control_flag = true;
																				}
																			else
																				{
																					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, json_p, "Invalid replicate value \"%s\"", rep_s);
																				}
																		}
																	else
																		{
																			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, json_p, "Missing replicate value");
																		}
																}
															else if (json_is_integer (rep_json_p))
																{
																	replicate = json_integer_value (rep_json_p);
																}
														}

													if ((replicate != 0) || (rep_control_flag))
														{
															MEM_FLAG mf = material_to_use_p == material_p ? MF_SHADOW_USE : MF_SHALLOW_COPY;
															row_p = AllocateRow (id_p, rack_index, study_index, replicate, material_to_use_p, mf, plot_p);

															if (row_p)
																{
																	if (rep_control_flag)
																		{
																			SetRowGenotypeControl (row_p, true);
																		}

																	if (!GetObservationsFromJSON (json_p, row_p, data_p))
																		{
																			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, json_p, "GetObservationsFromJSON failed");
																			FreeRow (row_p);
																			row_p = NULL;
																		}


																	if (!GetTreatmentFactorValuesFromJSON (json_p, row_p, study_p, data_p))
																		{
																			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, json_p, "GetTreatmentFactorValuesFromJSON failed");
																			FreeRow (row_p);
																			row_p = NULL;
																		}


																}

														}		/* if ((replicate != 0) || (rep_control_flag)) */
												}
										}

								}

							if (!row_p)
								{
									FreeBSONOid (id_p);
								}
						}		/* if (id_p) */

				}		/* if (success_flag) */
			else
				{
					char id_s [MONGO_OID_STRING_BUFFER_SIZE];

					bson_oid_to_string (plot_p -> pl_id_p, id_s);
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get row's material for plot \"%s\" at [" UINT32_FMT ", " UINT32_FMT "]", id_s, plot_p -> pl_row_index, plot_p -> pl_column_index);
				}
		}

	if (!row_p)
		{
			if (material_to_use_p && (material_to_use_p != material_p))
				{
					FreeMaterial (material_to_use_p);
				}
		}

	return row_p;
}

//
//bool SaveRow (Row *row_p, const FieldTrialServiceData *data_p, bool insert_flag)
//{
//	bson_t *selector_p = NULL;
//	bool success_flag = false;
//
//	if (PrepareSaveData (& (row_p -> ro_id_p), &selector_p))
//		{
//			json_t *row_json_p = GetRowAsJSON (row_p, VF_STORAGE, NULL, data_p);
//
//			if (row_json_p)
//				{
//					if (SaveMongoData (data_p -> dftsd_mongo_p, row_json_p, data_p -> dftsd_collection_ss [DFTD_ROW], selector_p))
//						{
//							success_flag = true;
//						}
//					else
//						{
//							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, row_json_p, "SaveMongoData failed");
//						}
//
//					json_decref (row_json_p);
//				}		/* if (row_json_p) */
//			else
//				{
//					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "GetRowAsJSON failed for row " UINT32_FMT " for plot [" UINT32_FMT ", " UINT32_FMT "] in study \"%s\"", row_p -> ro_rack_index, row_p -> ro_plot_p -> pl_row_index, row_p -> ro_plot_p -> pl_column_index, row_p -> ro_plot_p -> pl_parent_p -> st_name_s);
//				}
//
//		}		/* if (row_p -> ro_id_p) */
//	else
//		{
//			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "PrepareSaveData failed for row " UINT32_FMT " for plot [" UINT32_FMT ", " UINT32_FMT "] in study \"%s\"", row_p -> ro_rack_index, row_p -> ro_plot_p -> pl_row_index, row_p -> ro_plot_p -> pl_column_index, row_p -> ro_plot_p -> pl_parent_p -> st_name_s);
//		}
//
//	return success_flag;
//}
//




bool AddObservationToRow (Row *row_p, Observation *observation_p)
{
	bool success_flag = false;
	ObservationNode *node_p = NULL;

	/*
	 * If the observation has the same phenotype and date as an existing one,
	 * then replace it. If not, then simply add it.
	 */
	node_p = (ObservationNode *) (row_p -> ro_observations_p -> ll_head_p);
	while (node_p && (!success_flag))
		{
			Observation *existing_observation_p = node_p -> on_observation_p;

			if (observation_p == existing_observation_p)
				{
					success_flag = true;
				}
			else if (AreObservationsMatching (existing_observation_p, observation_p))
				{
					node_p -> on_observation_p = observation_p;
					FreeObservation (existing_observation_p);
					success_flag = true;
				}
			else
				{
					node_p = (ObservationNode *) (node_p -> on_node.ln_next_p);
				}
		}

	if (!success_flag)
		{
			node_p = AllocateObservationNode (observation_p);

			if (node_p)
				{
					LinkedListAddTail (row_p -> ro_observations_p, & (node_p -> on_node));
					success_flag = true;
				}
			else
				{
					char row_id_s [MONGO_OID_STRING_BUFFER_SIZE];
					char observation_id_s [MONGO_OID_STRING_BUFFER_SIZE];

					bson_oid_to_string (row_p -> ro_id_p, row_id_s);
					bson_oid_to_string (observation_p -> ob_id_p, observation_id_s);

					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add observation \"%s\" to row \"%s\"", observation_id_s, row_id_s);
				}
		}

	return success_flag;
}



bool AddTreatmentFactorValueToRow (Row *row_p, TreatmentFactorValue *tf_value_p)
{
	bool success_flag = false;
	TreatmentFactorValueNode *node_p = NULL;

	/*
	 * If the treatment factor has the same treatment factor and date as an existing one,
	 * then replace it. If not, then simply add it.
	 */
	node_p = (TreatmentFactorValueNode *) (row_p -> ro_treatment_factor_values_p -> ll_head_p);
	while (node_p)
		{
			TreatmentFactorValue *existing_tf_value_p = node_p -> tfvn_value_p;

			if (AreTreatmentFactorValuesMatching (existing_tf_value_p, tf_value_p))
				{
					node_p -> tfvn_value_p = tf_value_p;
					FreeTreatmentFactorValue (existing_tf_value_p);
					success_flag = true;
					node_p = NULL;		/* force exit from loop */
				}
			else
				{
					node_p = (TreatmentFactorValueNode *) (node_p -> tfvn_node.ln_next_p);
				}
		}

	if (!success_flag)
		{
			node_p = AllocateTreatmentFactorValueNode (tf_value_p);

			if (node_p)
				{
					LinkedListAddTail (row_p -> ro_treatment_factor_values_p, & (node_p -> tfvn_node));
					success_flag = true;
				}
			else
				{
					char row_id_s [MONGO_OID_STRING_BUFFER_SIZE];
					char treatment_id_s [MONGO_OID_STRING_BUFFER_SIZE];
					const bson_oid_t *treatment_id_p = GetTreatmentIdForTreatmentFactorValue (tf_value_p);


					bson_oid_to_string (row_p -> ro_id_p, row_id_s);
					bson_oid_to_string (treatment_id_p, treatment_id_s);

					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add observation \"%s\" to row \"%s\"", treatment_id_s, row_id_s);
				}
		}

	return success_flag;
}




void UpdateRow (Row *row_p, const uint32 rack_plotwise_index, Material *material_p, MEM_FLAG material_mem, const bool control_rep_flag, const uint32 replicate)
{
	if (row_p -> ro_rack_index != rack_plotwise_index)
		{
			row_p -> ro_rack_index = rack_plotwise_index;
		}

	if (row_p -> ro_material_p -> ma_id_p != material_p -> ma_id_p)
		{
			if ((row_p -> ro_material_mem == MF_DEEP_COPY) || (row_p -> ro_material_mem == MF_SHALLOW_COPY))
				{
					if (row_p -> ro_material_p)
						{
							FreeMaterial (row_p -> ro_material_p);
						}
				}

			row_p -> ro_material_p = material_p;
			row_p -> ro_material_mem = material_mem;
		}

	if (control_rep_flag != IsRowGenotypeControl (row_p))
		{
			SetRowGenotypeControl (row_p, control_rep_flag);
		}

	if (!control_rep_flag)
		{
			if (row_p -> ro_replicate_index != replicate)
				{
					row_p -> ro_replicate_index = replicate;
				}
		}

}


static bool AddObservationsToJSON (json_t *row_json_p, LinkedList *observations_p, const ViewFormat format)
{
	bool success_flag = false;

	if (observations_p -> ll_size > 0)
		{
			json_t *observations_json_p = json_array ();

			if (observations_json_p)
				{
					if (json_object_set_new (row_json_p, RO_OBSERVATIONS_S, observations_json_p) == 0)
						{
							ObservationNode *node_p = (ObservationNode *) (observations_p -> ll_head_p);

							success_flag = true;

							while (node_p && success_flag)
								{
									const Observation *observation_p = node_p -> on_observation_p;
									json_t *observation_json_p = GetObservationAsJSON (observation_p, format);

									if (observation_json_p)
										{
											if (json_array_append_new (observations_json_p, observation_json_p) == 0)
												{
													node_p = (ObservationNode *) (node_p -> on_node.ln_next_p);
												}
											else
												{
													success_flag = false;
													json_decref (observation_json_p);
												}
										}
									else
										{
											success_flag = false;
										}
								}		/* while (node_p && success_flag) */

						}		/* if (json_object_set_new (row_json_p, RO_PHENOTYPES_S, phenotypes_array_p) == 0) */
					else
						{
							json_decref (observations_json_p);
						}

				}		/* if (phenotypes_array_p) */

		}
	else
		{
			success_flag = true;
		}

	return success_flag;
}



static bool GetObservationsFromJSON (const json_t *row_json_p, Row *row_p, const FieldTrialServiceData *data_p)
{
	bool success_flag = false;
	const json_t *observations_json_p = json_object_get (row_json_p, RO_OBSERVATIONS_S);

	if (observations_json_p)
		{
			size_t size = json_array_size (observations_json_p);
			size_t i;

			success_flag = true;

			for (i = 0; i < size; ++ i)
				{
					const json_t *observation_json_p = json_array_get (observations_json_p, i);
					Observation *observation_p = GetObservationFromJSON (observation_json_p, data_p);

					if (observation_p)
						{
							if (!AddObservationToRow (row_p, observation_p))
								{
									char row_id_s [MONGO_OID_STRING_BUFFER_SIZE];

									bson_oid_to_string (row_p -> ro_id_p, row_id_s);
									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, observation_json_p, "Failed to add observation to for row \"%s\"", row_id_s);

									FreeObservation (observation_p);
									success_flag = false;
									i = size;		/* force exit from loop */
								}

						}		/* if (observation_p) */
					else
						{
							char row_id_s [MONGO_OID_STRING_BUFFER_SIZE];

							bson_oid_to_string (row_p -> ro_id_p, row_id_s);
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "GetObservationFromJSON failed for row \"%s\"", row_id_s);


							success_flag = false;
							i = size;		/* force exit from loop */
						}

				}		/* for (i = 0; i < size; ++ i) */

		}		/* if (phenotypes_json_p) */
	else
		{
			success_flag = true;
		}

	return success_flag;
}


static bool AddTreatmentFactorsToJSON (json_t *row_json_p, LinkedList *treatment_factors_p, const Study *study_p, const ViewFormat format)
{
	bool success_flag = false;

	if (treatment_factors_p -> ll_size > 0)
		{
			json_t *treatment_factors_json_p = json_array ();

			if (treatment_factors_json_p)
				{
					if (json_object_set_new (row_json_p, RO_TREATMENTS_S, treatment_factors_json_p) == 0)
						{
							TreatmentFactorValueNode *node_p = (TreatmentFactorValueNode *) (treatment_factors_p -> ll_head_p);

							success_flag = true;

							while (node_p && success_flag)
								{
									const TreatmentFactorValue *tfv_p = node_p -> tfvn_value_p;
									json_t *treatment_factor_json_p = GetTreatmentFactorValueAsJSON (tfv_p, study_p);

									if (treatment_factor_json_p)
										{
											if (json_array_append_new (treatment_factors_json_p, treatment_factor_json_p) == 0)
												{
													node_p = (TreatmentFactorValueNode *) (node_p -> tfvn_node.ln_next_p);
												}
											else
												{
													success_flag = false;
													json_decref (treatment_factor_json_p);
												}
										}
									else
										{
											success_flag = false;
										}
								}		/* while (node_p && success_flag) */

						}		/* if (json_object_set_new (row_json_p, RO_PHENOTYPES_S, phenotypes_array_p) == 0) */
					else
						{
							json_decref (treatment_factors_json_p);
						}

				}		/* if (phenotypes_array_p) */

		}
	else
		{
			success_flag = true;
		}

	return success_flag;
}



static bool GetTreatmentFactorValuesFromJSON (const json_t *row_json_p, Row *row_p, const Study *study_p, const FieldTrialServiceData *data_p)
{
	bool success_flag = false;
	const json_t *tf_values_json_p = json_object_get (row_json_p, RO_TREATMENTS_S);

	if (tf_values_json_p)
		{
			size_t size = json_array_size (tf_values_json_p);
			size_t i;

			success_flag = true;

			for (i = 0; i < size; ++ i)
				{
					const json_t *tf_value_json_p = json_array_get (tf_values_json_p, i);
					TreatmentFactorValue *tf_value_p = GetTreatmentFactorValueFromJSON (tf_value_json_p, study_p, data_p);

					if (tf_value_p)
						{
							if (!AddTreatmentFactorValueToRow (row_p, tf_value_p))
								{
									char row_id_s [MONGO_OID_STRING_BUFFER_SIZE];

									bson_oid_to_string (row_p -> ro_id_p, row_id_s);
									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, tf_value_json_p, "Failed to add TreatmentFactorValue to for row \"%s\"", row_id_s);

									FreeTreatmentFactorValue (tf_value_p);
									success_flag = false;
									i = size;		/* force exit from loop */
								}

						}		/* if (observation_p) */
					else
						{
							char row_id_s [MONGO_OID_STRING_BUFFER_SIZE];

							bson_oid_to_string (row_p -> ro_id_p, row_id_s);
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "GetTreatmentFactorValueFromJSON failed for row \"%s\"", row_id_s);


							success_flag = false;
							i = size;		/* force exit from loop */
						}

				}		/* for (i = 0; i < size; ++ i) */

		}		/* if (phenotypes_json_p) */
	else
		{
			success_flag = true;
		}

	return success_flag;
}



void SetRowGenotypeControl (Row *row_p, bool control_flag)
{
	row_p -> ro_replicate_control_flag = control_flag;
}


bool IsRowGenotypeControl (const Row *row_p)
{
	return row_p -> ro_replicate_control_flag;
}



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


static bool AddObservationsToJSON (json_t *row_json_p, LinkedList *observations_p, const bool expand_fields_flag);

static bool GetObservationsFromJSON (const json_t *row_json_p, Row *row_p, const DFWFieldTrialServiceData *data_p);



Row *AllocateRow (bson_oid_t *id_p, const uint32 index, Material *material_p, Plot *parent_plot_p)
{
	if (material_p)
		{
			LinkedList *observations_p = AllocateLinkedList (FreeObservationNode);

			if (observations_p)
				{
					Row *row_p = (Row *) AllocMemory (sizeof (Row));

					if (row_p)
						{
							row_p -> ro_id_p = id_p;
							row_p -> ro_index = index;
							row_p -> ro_material_p = material_p;
							row_p -> ro_plot_p = parent_plot_p;
							row_p -> ro_material_s = NULL;
							row_p -> ro_observations_p = observations_p;

							return row_p;
						}
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate row " UINT32_FMT " at [" UINT32_FMT "," UINT32_FMT "]", parent_plot_p -> pl_row_index, parent_plot_p -> pl_column_index, index);
						}
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
	if (row_p -> ro_material_s)
		{
			FreeCopiedString (row_p -> ro_material_s);
		}

	FreeLinkedList (row_p -> ro_observations_p);

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



json_t *GetRowAsJSON (const Row *row_p, const bool expand_fields_flag, const DFWFieldTrialServiceData *data_p)
{
	json_t *row_json_p = json_object ();

	if (row_json_p)
		{
			bool success_flag = false;

			if (row_p -> ro_material_p)
				{
					if (expand_fields_flag)
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
						}
					else
						{
							if (AddNamedCompoundIdToJSON (row_json_p, row_p -> ro_material_p -> ma_id_p, RO_MATERIAL_ID_S))
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
					if (AddNamedCompoundIdToJSON (row_json_p, row_p -> ro_plot_p -> pl_id_p, RO_PLOT_ID_S))
						{
							if (AddCompoundIdToJSON (row_json_p, row_p -> ro_id_p))
								{
									if (SetJSONInteger (row_json_p, RO_INDEX_S, row_p -> ro_index))
										{
											if (AddObservationsToJSON (row_json_p, row_p -> ro_observations_p, expand_fields_flag))
												{
													return row_json_p;
												}

										}
								}
						}
				}

			json_decref (row_json_p);
		}		/* if (row_json_p) */

	return NULL;
}


Row *GetRowFromJSON (const json_t *json_p, Plot *plot_p, Material *material_p, const bool expand_fields_flag, const DFWFieldTrialServiceData *data_p)
{
	if (!plot_p)
		{
			bson_oid_t *plot_id_p = GetNewUnitialisedBSONOid ();

			if (plot_id_p)
				{
					if (GetNamedIdFromJSON (json_p, RO_PLOT_ID_S, plot_id_p))
						{

						}
				}
		}

	if (plot_p)
		{
			bool success_flag = true;

			if (expand_fields_flag)
				{
					/*
					 * If we haven't already got the material, get it!
					 */
					if (!material_p)
						{
							bson_oid_t *material_id_p = GetNewUnitialisedBSONOid ();
							const char *material_s = NULL;

							if (material_id_p)
								{
									if (GetNamedIdFromJSON (json_p, RO_MATERIAL_ID_S, material_id_p))
										{
											if (plot_p -> pl_parent_p)
												{
													material_p = GetMaterialById (material_id_p, plot_p -> pl_parent_p, data_p);

													if (!material_p)
														{
															char id_s [MONGO_OID_STRING_BUFFER_SIZE];

															bson_oid_to_string (material_id_p, id_s);
															PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, json_p, "Failed to get material with id %s", id_s);
														}
												}		/* if (plot_p -> pl_parent_p) */
											else
												{
													char id_s [MONGO_OID_STRING_BUFFER_SIZE];

													bson_oid_to_string (plot_p -> pl_id_p, id_s);
													PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Plot %s has no parent id", id_s);
												}
										}

									FreeBSONOid (material_id_p);
								}

							if (!material_p)
								{
									material_s = GetJSONString (json_p, RO_TRIAL_MATERIAL_S);

									if (material_s)
										{
											material_p = GetMaterialByInternalName (material_s, plot_p -> pl_parent_p, data_p);
										}
								}

						}		/* if (!material_p) */

					success_flag = (material_p != NULL);
				}		/* if (expand_fields_flag) */

			if (success_flag)
				{
					bson_oid_t *id_p = GetNewUnitialisedBSONOid ();

					if (id_p)
						{
							if (GetMongoIdFromJSON (json_p, id_p))
								{
									int index = -1;

									if (GetJSONInteger (json_p, RO_INDEX_S, &index))
										{
											Row *row_p = AllocateRow (id_p, index, material_p, plot_p);

											if (row_p)
												{
													if (GetObservationsFromJSON (json_p, row_p, data_p))
														{
															return row_p;
														}

													FreeRow (row_p);
												}
										}
								}

							FreeBSONOid (id_p);
						}		/* if (id_p) */

				}		/* if (success_flag) */
		}

	return NULL;
}


bool SaveRow (Row *row_p, const DFWFieldTrialServiceData *data_p, bool insert_flag)
{
	bson_t *selector_p = NULL;
	bool success_flag = PrepareSaveData (& (row_p -> ro_id_p), &selector_p);

	if (success_flag)
		{
			json_t *row_json_p = GetRowAsJSON (row_p, false, data_p);

			if (row_json_p)
				{
					success_flag = SaveMongoData (data_p -> dftsd_mongo_p, row_json_p, data_p -> dftsd_collection_ss [DFTD_ROW], selector_p);

					json_decref (row_json_p);
				}		/* if (row_json_p) */

		}		/* if (row_p -> ro_id_p) */

	return success_flag;
}


bool AddObservationToRow (Row *row_p, Observation *observation_p)
{
	bool success_flag = false;
	ObservationNode *node_p = AllocateObservationNode (observation_p);

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
			success_flag = false;
		}

	return success_flag;
}



static bool AddObservationsToJSON (json_t *row_json_p, LinkedList *observations_p, const bool expand_fields_flag)
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
									json_t *observation_json_p = GetObservationAsJSON (observation_p, expand_fields_flag);

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



static bool GetObservationsFromJSON (const json_t *row_json_p, Row *row_p, const DFWFieldTrialServiceData *data_p)
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
									FreeObservation (observation_p);
									success_flag = false;
									i = size;		/* force exit from loop */
								}

						}		/* if (observation_p) */
					else
						{
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


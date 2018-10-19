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


Row *AllocateRow (bson_oid_t *id_p, const uint32 index, Material *material_p,  const char *internal_material_s, Plot *parent_plot_p)
{
	char *copied_internal_material_s = NULL;

	/*
	 * Make sure we have a valid material
	 */
	bool valid_material_flag = (material_p != NULL);

	if (!valid_material_flag)
		{
			if (internal_material_s)
				{
					if ((copied_internal_material_s = EasyCopyToNewString (internal_material_s)) != NULL)
						{
							valid_material_flag = true;
						}
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to copy material name \"%s\"", internal_material_s);
						}
				}
		}


	if (valid_material_flag)
		{
			Row *row_p = (Row *) AllocMemory (sizeof (Row));

			if (row_p)
				{
					row_p -> ro_id_p = id_p;
					row_p -> ro_index = index;
					row_p -> ro_material_p = material_p;
					row_p -> ro_material_s = copied_internal_material_s;
					row_p -> ro_plot_p = parent_plot_p;

					return row_p;
				}
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate row " UINT32_FMT " at [" UINT32_FMT "," UINT32_FMT "]", parent_plot_p -> pl_row_index, parent_plot_p -> pl_column_index, index);
				}
		}
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "No valid material for row " UINT32_FMT " at [" UINT32_FMT "," UINT32_FMT "]", parent_plot_p -> pl_row_index, parent_plot_p -> pl_column_index, index);
		}

	if (copied_internal_material_s)
		{
			FreeCopiedString (copied_internal_material_s);
		}

	return NULL;
}


void FreeRow (Row *row_p)
{
	if (row_p -> ro_material_s)
		{
			FreeCopiedString (row_p -> ro_material_s);
		}

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



json_t *GetRowAsJSON (const Row *row_p)
{
	json_t *row_json_p = json_object ();

	if (row_json_p)
		{
			if ((row_p -> ro_material_p == NULL) || (AddNamedCompoundIdToJSON (row_json_p, row_p -> ro_material_p -> ma_id_p, RO_MATERIAL_ID_S)))
				{
					if ((row_p -> ro_material_s == NULL) || (SetJSONString (row_json_p, RO_MATERIAL_S, row_p -> ro_material_s)))
						{
							if (AddNamedCompoundIdToJSON (row_json_p, row_p -> ro_plot_p -> pl_id_p, RO_PLOT_ID_S))
								{
									if (AddCompoundIdToJSON (row_json_p, row_p -> ro_id_p))
										{
											if (SetJSONInteger (row_json_p, RO_INDEX_S, row_p -> ro_index))
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


Row *GetRowFromJSON (const json_t *json_p, const bool expand_fields_flag)
{
	bson_oid_t *plot_id_p = GetNewUnitialisedBSONOid ();

	if (plot_id_p)
		{
			if (GetNamedIdFromJSON (json_p, RO_PLOT_ID_S, plot_id_p))
				{
					bson_oid_t *id_p = GetNewUnitialisedBSONOid ();

					if (id_p)
						{
							if (GetMongoIdFromJSON (json_p, id_p))
								{
									bool valid_material_flag = false;
									bson_oid_t *material_id_p = GetNewUnitialisedBSONOid ();
									const char *material_s = NULL;

									if (material_id_p)
										{
											if (GetNamedIdFromJSON (json_p, RO_MATERIAL_ID_S, material_id_p))
												{
													valid_material_flag = true;
												}
											else
												{
													FreeBSONOid (material_id_p);
													material_id_p = NULL;
												}
										}

									if (!valid_material_flag)
										{
											material_s = GetJSONString (json_p, RO_MATERIAL_S);

											if (material_s)
												{
													valid_material_flag = true;
												}
										}

									if (valid_material_flag)
										{
											int index = -1;

											if (GetJSONInteger (json_p, RO_INDEX_S, &index))
												{

												}
										}

									if (material_id_p)
										{
											FreeBSONOid (material_id_p);
										}
								}

							FreeBSONOid (id_p);
						}
				}

			FreeBSONOid (plot_id_p);
		}

	return NULL;
}

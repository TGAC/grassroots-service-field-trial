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


#include "row.h"

#include "memory_allocations.h"


Row *AllocateRow (bson_oid_t *id_p, const uint32 index, Material *material_p, Plot *parent_plot_p)
{
	Row *row_p = (Row *) AllocMemory (sizeof (Row));

	if (row_p)
		{
			row_p -> ro_id_p = id_p;
			row_p -> ro_index = index;
			row_p -> ro_material_p = material_p;
			row_p -> ro_plot_p = parent_plot_p;
		}

	return row_p;
}


void FreeRow (Row *row_p)
{
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
			if (AddNamedCompoundIdToJSON (row_json_p, row_p -> ro_material_p -> ma_id_p, RO_MATERIAL_ID_S))
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

			json_decref (row_json_p);
		}		/* if (row_json_p) */

	return NULL;
}


Row *GetRowFromJSON (const json_t *json_p)
{
	return NULL;
}

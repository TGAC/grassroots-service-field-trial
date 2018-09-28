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


Plot *AllocatePlot (bson_oid_t *id_p, const uint32 sowing_date, const uint32 harvest_date, const double64 width, const double64 length, const uint32 row_index,
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
							LinkedList *rows_p = AllocateLinkedList (FreeRowNode);

							if (rows_p)
								{
									Plot *plot_p = (Plot *) AllocMemory (sizeof (Plot));

									if (plot_p)
										{
											plot_p -> pl_id_p = id_p;
											plot_p -> pl_sowing_date = sowing_date;
											plot_p -> pl_harvest_date = harvest_date;
											plot_p -> pl_width = width;
											plot_p -> pl_length = length;
											plot_p -> pl_row_index = row_index;
											plot_p -> pl_column_index = column_index;
											plot_p -> pl_parent_p = parent_p;
											plot_p -> pl_rows_p = rows_p;

											return plot_p;
										}		/* if (plot_p) */

									FreeLinkedList (rows_p);
								}		/* if (rows_p) */

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

/*
 * phenotype_statistics.c
 *
 *  Created on: 27 May 2022
 *      Author: billy
 */


#include "phenotype_statistics.h"
#include "memory_allocations.h"

#include "string_utils.h"




PhenotypeStatisticsNode *AllocatePhenotypeStatisticsNode (const char *measured_variable_name_s, const Statistics *src_p)
{
	char *mv_s = EasyCopyToNewString (measured_variable_name_s);

	if (mv_s)
		{
			Statistics *copied_stats_p = CopyStatistics (src_p);

			if (copied_stats_p)
				{
					PhenotypeStatisticsNode *node_p = (PhenotypeStatisticsNode *) AllocMemory (sizeof (PhenotypeStatisticsNode));

					if (node_p)
						{
							InitListItem (& (node_p -> psn_node));
							node_p -> psn_measured_variable_name_s = mv_s;
							node_p -> psn_stats_p = copied_stats_p;

							return node_p;
						}
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate stats node for measured variable name \"%s\"", measured_variable_name_s);
						}

					FreeStatistics (copied_stats_p);
				}
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to copy statistics for measured variable name \"%s\"", measured_variable_name_s);
				}

			FreeCopiedString (mv_s);
		}
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to copy measured variable name \"%s\"", measured_variable_name_s);
		}

	return NULL;
}


void FreePhenotypeStatisticsNode (ListItem *node_p)
{
	PhenotypeStatisticsNode *psn_p = (PhenotypeStatisticsNode *) node_p;

	FreeCopiedString (psn_p -> psn_measured_variable_name_s);
	FreeStatistics (psn_p -> psn_stats_p);

	FreeMemory (psn_p);
}


bool GetPhenotypeStatisticsNodeAsJSON (const PhenotypeStatisticsNode *psn_p, json_t *parent_p)
{
	bool success_flag = false;
	json_t *stats_json_p = GetStatisticsAsJSON (psn_p -> psn_stats_p);

	if (stats_json_p)
		{
			if (json_object_set_new (parent_p, psn_p -> psn_measured_variable_name_s, stats_json_p) == 0)
				{
					success_flag = true;
				}
			else
				{
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, stats_json_p, "Failed to add phenotype stats for \"%s\" to json", psn_p -> psn_measured_variable_name_s);
					json_decref (stats_json_p);
				}
		}
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "GetStatisticsAsJSON () failed for \"%s\"", psn_p -> psn_measured_variable_name_s);
		}

	return success_flag;
}


bool AddPhenotypeStatisticsNodeFromJSON (LinkedList *nodes_p, const json_t *json_p)
{
	bool success_flag = false;

	return success_flag;
}

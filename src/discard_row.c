/*
 * discard_row.c
 *
 *  Created on: 30 Jun 2022
 *      Author: billy
 */

#include "discard_row.h"


static bool AddDiscardRowToJSON (const Row *row_p, json_t *row_json_p, const ViewFormat format, const FieldTrialServiceData *data_p);

static bool AddDiscardRowToFD (const Row *row_p, json_t *row_fd_p, const FieldTrialServiceData *service_data_p, const char * const null_sequence_s);




Row *AllocateDiscardRow (bson_oid_t *id_p, const uint32 study_index, Plot *parent_plot_p)
{
	DiscardRow *row_p = (DiscardRow *) AllocMemory (sizeof (DiscardRow));

	if (row_p)
		{
			if (InitRow (& (row_p -> dr_base), id_p, study_index, parent_plot_p, RT_BLANK, NULL, AddDiscardRowToJSON, NULL, AddDiscardRowToFD))
				{
					return (& (row_p -> dr_base));
				}

			FreeMemory (row_p);
		}

	return NULL;
}



DiscardRow *GetDiscardRowFromJSON (const json_t *row_json_p, Plot *plot_p, const Study *study_p, const ViewFormat format, FieldTrialServiceData *data_p)
{
	DiscardRow *row_p = (DiscardRow *) AllocMemory (sizeof (DiscardRow));

	if (row_p)
		{

			if (PopulateRowFromJSON (& (row_p -> dr_base), plot_p, row_json_p, format, data_p))
				{
					return row_p;
				}

			FreeMemory (row_p);
		}

	return NULL;
}



static bool AddDiscardRowToJSON (const Row *row_p, json_t *row_json_p, const ViewFormat format, const FieldTrialServiceData *data_p)
{
	DiscardRow *discard_row_p = (DiscardRow *) row_p;
	bool success_flag = false;

	if (SetJSONBoolean (row_json_p, RO_DISCARD_S, true))
		{
			success_flag = true;
		}
	else
		{
			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, row_json_p, "Failed to add \"%s\": true", RO_DISCARD_S);
		}

	return success_flag;
}


static bool AddDiscardRowToFD (const Row *row_p, json_t *row_fd_p, const FieldTrialServiceData *service_data_p, const char * const null_sequence_s)
{
	DiscardRow *discard_row_p = (DiscardRow *) row_p;
	bool success_flag = false;

	if (SetJSONBoolean (row_fd_p, RO_DISCARD_S, true))
		{
			success_flag = true;
		}
	else
		{
			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, row_fd_p, "Failed to add \"%s\": true", RO_DISCARD_S);
		}

	return success_flag;
}

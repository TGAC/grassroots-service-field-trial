/*
 * discard_row.c
 *
 *  Created on: 30 Jun 2022
 *      Author: billy
 */

#include "discard_row.h"


static bool AddDiscardRowToJSON (const BaseRow *row_p, json_t *row_json_p, const ViewFormat format, const FieldTrialServiceData *data_p);

static bool AddDiscardRowToFD (const BaseRow *row_p, json_t *row_fd_p, const FieldTrialServiceData *service_data_p, const char * const null_sequence_s);



BaseRow *AllocateDiscardRow (bson_oid_t *id_p, const uint32 study_index, Plot *parent_plot_p)
{
	return AllocateBaseRow (id_p, study_index, parent_plot_p, RT_DISCARD, NULL, AddDiscardRowToJSON, AddDiscardRowToFD);
}


static bool AddDiscardRowToJSON (const BaseRow *row_p, json_t *row_json_p, const ViewFormat format, const FieldTrialServiceData *data_p)
{
	DiscardRow *discard_row_p = (DiscardRow *) row_p;
	bool success_flag = false;

	if (SetJSONBoolean (row_json_p, BR_DISCARD_S, true))
		{
			success_flag = true;
		}
	else
		{
			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, row_json_p, "Failed to add \"%s\": true", BR_DISCARD_S);
		}

	return success_flag;
}


static bool AddDiscardRowToFD (const BaseRow *row_p, json_t *row_fd_p, const FieldTrialServiceData *service_data_p, const char * const null_sequence_s)
{
	DiscardRow *discard_row_p = (DiscardRow *) row_p;
	bool success_flag = false;

	if (SetJSONBoolean (row_fd_p, BR_DISCARD_S, true))
		{
			success_flag = true;
		}
	else
		{
			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, row_fd_p, "Failed to add \"%s\": true", BR_DISCARD_S);
		}

	return success_flag;
}

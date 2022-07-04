/*
 * blank_row.c
 *
 *  Created on: 30 Jun 2022
 *      Author: billy
 */


#include "blank_row.h"


static bool AddBlankRowToJSON (const Row *row_p, json_t *row_json_p, const ViewFormat format, const FieldTrialServiceData *data_p);

static bool AddBlankRowToFD (const Row *row_p, json_t *row_fd_p, const FieldTrialServiceData *service_data_p, const char * const null_sequence_s);



Row *AllocateBlankRow (bson_oid_t *id_p, const uint32 study_index, Plot *parent_plot_p)
{
	BlankRow *row_p = (BlankRow *) AllocMemory (sizeof (BlankRow));

	if (row_p)
		{
			if (InitRow (& (row_p -> br_base), id_p, study_index, parent_plot_p, RT_BLANK, NULL, AddBlankRowToJSON, NULL, AddBlankRowToFD))
				{
					return (& (row_p -> br_base));
				}

			FreeMemory (row_p);
		}

	return NULL;
}



BlankRow *GetBlankRowFromJSON (const json_t *row_json_p, Plot *plot_p, const Study *study_p, const ViewFormat format, FieldTrialServiceData *data_p)
{
	BlankRow *row_p = (BlankRow *) AllocMemory (sizeof (BlankRow));

	if (row_p)
		{
			if (PopulateRowFromJSON (& (row_p -> br_base), plot_p, row_json_p, format, data_p))
				{
					return row_p;
				}

			FreeMemory (row_p);
		}

	return NULL;
}




static bool AddBlankRowToJSON (const Row *row_p, json_t *row_json_p, const ViewFormat format, const FieldTrialServiceData *data_p)
{
	BlankRow *blank_row_p = (BlankRow *) row_p;
	bool success_flag = false;

	if (SetJSONBoolean (row_json_p, RO_BLANK_S, true))
		{
			success_flag = true;
		}
	else
		{
			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, row_json_p, "Failed to add \"%s\": true", RO_BLANK_S);
		}

	return success_flag;
}


static bool AddBlankRowToFD (const Row *row_p, json_t *row_fd_p, const FieldTrialServiceData *service_data_p, const char * const null_sequence_s)
{
	BlankRow *blank_row_p = (BlankRow *) row_p;
	bool success_flag = false;

	if (SetJSONBoolean (row_fd_p, RO_BLANK_S, true))
		{
			success_flag = true;
		}
	else
		{
			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, row_fd_p, "Failed to add \"%s\": true", RO_BLANK_S);
		}

	return success_flag;

}


/*
 * field_trial_sqlite.c
 *
 *  Created on: 21 Sep 2018
 *      Author: billy
 */



#include "field_trial_sqlite.h"
#include "sqlite_tool.h"
#include "string_utils.h"

//static const char * const S_ID_COLUMN_S = "fieldTrialId";


DFW_FIELD_TRIAL_SERVICE_LOCAL LinkedList *GetFieldTrialsByNameFromSQLite (DFWFieldTrialServiceData *data_p, const char *name_s);


bool AddFieldTrialToSQLite (DFWFieldTrialServiceData *data_p, FieldTrial *trial_p)
{
	bool success_flag = false;
	SQLiteTool *tool_p = data_p -> dftsd_sqlite_p;

	if (IsIdSet  (& (trial_p -> ft_id), data_p))
		{

		}
	else
		{
			json_t *field_trial_json_p = GetFieldTrialAsJSON (trial_p);

			if (field_trial_json_p)
				{
					char *error_s = NULL;

					success_flag = InsertSQLiteRow (tool_p, field_trial_json_p, &error_s);

					if (error_s)
						{
							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, field_trial_json_p, "Failed to insert into sqlite db, error \"%s\"", error_s);
							FreeCopiedString (error_s);
						}
					else
						{
							/* we need to get the id for the newly inserted trial */

						}

					json_decref (field_trial_json_p);
				}
		}

	return success_flag;
}

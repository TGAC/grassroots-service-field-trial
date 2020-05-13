/*
 * field_trial_sqlite.c
 *
 *  Created on: 21 Sep 2018
 *      Author: billy
 */



#include "field_trial_sqlite.h"

#include "sqlite_tool.h"
#include "string_utils.h"


static const char * const S_ID_COLUMN_S = "id";
static const char * const S_NAME_COLUMN_S = "name";
static const char * const S_TEAM_COLUMN_S = "team";


static json_t *GetFieldTrialAsDBJSON (const FieldTrial *trial_p);


bool AddFieldTrialToSQLite (FieldTrialServiceData *data_p, FieldTrial *trial_p)
{
	bool success_flag = false;
	SQLiteTool *tool_p = data_p -> dftsd_sqlite_p;

	if (SetSQLiteToolTable (tool_p, data_p -> dftsd_collection_ss [DFTD_FIELD_TRIAL]))
		{
			if (IsIdSet  (& (trial_p -> ft_id), data_p))
				{

				}
			else
				{
					json_t *field_trial_json_p = GetFieldTrialAsDBJSON (trial_p);

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
		}

	return success_flag;
}


static json_t *GetFieldTrialAsDBJSON (const FieldTrial *trial_p)
{
	return GetFieldTrialAsConfiguredJSON (trial_p, S_NAME_COLUMN_S, S_TEAM_COLUMN_S);
}


FieldTrial *GetFieldTrialByIDFromSQLiteDB (const uint32 id, FieldTrialServiceData *data_p)
{
	FieldTrial *trial_p = NULL;
	char *id_s = ConvertIntegerToString (id);

	if (id_s)
		{
			char *sql_s = ConcatenateVarargsStrings ("SELECT FROM ", data_p -> dftsd_collection_ss [DFTD_FIELD_TRIAL], " WHERE ", S_ID_COLUMN_S, " = ", id_s, ";", NULL);

			if (sql_s)
				{
					int (*callback_fn) (void *data_p, int num_columns, char **columns_aa_text_ss, char **column_names_ss) = NULL;

					RunSQLiteToolStatement (data_p -> dftsd_sqlite_p, sql_s, callback_fn, &trial_p);

					FreeCopiedString (sql_s);
				}

			FreeCopiedString (id_s);
		}		/* if (id_s) */

	return trial_p;
}


static int GetFieldTrial (void *data_p, int num_columns, char **columns_as_text_ss, char **column_names_ss)
{
	int res = 0;
	FieldTrial **trial_pp = (FieldTrial **) data_p;
	int i = 0;
	char **current_column_name_ss = column_names_ss;
	char **current_column_value_ss = columns_as_text_ss;
	char *name_s = NULL;
	char *id_s = NULL;
	char *team_s = NULL;

	for (i = num_columns; i > 0; -- i, ++ current_column_name_ss, ++ current_column_value_ss)
		{
			if (strcmp (*current_column_name_ss, S_ID_COLUMN_S) == 0)
				{
					id_s = *current_column_value_ss;
				}
			else if (strcmp (*current_column_name_ss, S_NAME_COLUMN_S) == 0)
				{
					name_s = *current_column_value_ss;
				}
			else if (strcmp (*current_column_name_ss, S_TEAM_COLUMN_S) == 0)
				{
					team_s = *current_column_value_ss;
				}
		}

	if (name_s && team_s && id_s)
		{
			int id;

			if (GetValidInteger (&id_s, &id))
				{
					FieldTrial *trial_p = AllocateFieldTrial (name_s, team_s, data_p);

					if (trial_p)
						{
							SetIdIndex (& (trial_p -> ft_id), id);

							*trial_pp = trial_p;
						}
				}
		}		/* if (name_s && team_s && id_s) */

	return res;
}

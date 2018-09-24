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
 * field_trial.c
 *
 *  Created on: 11 Sep 2018
 *      Author: billy
 */


#define ALLOCATE_FIELD_TRIAL_TAGS (1)
#include "field_trial.h"
#include "field_trial_mongodb.h"
#include "dfw_field_trial_service_data.h"
#include "string_utils.h"
#include "experimental_area.h"
#include "memory_allocations.h"



static bool LoadFieldTrialByName (const char *name_s);



FieldTrial *AllocateFieldTrial (const char *name_s, const char *team_s, const char *id_s)
{
	char *copied_name_s = EasyCopyToNewString (name_s);

	if (copied_name_s)
		{
			char *copied_team_s = EasyCopyToNewString (team_s);

			if (copied_team_s)
				{
					bool success_flag = true;
					bson_oid_t *id_p = NULL;

					if (id_s)
						{
							id_p = GetBSONOidFromString (id_s);

							success_flag = (id_p != NULL);
						}

					if (success_flag)
						{
							FieldTrial *trial_p = (FieldTrial *) AllocMemory (sizeof (FieldTrial));

							if (trial_p)
								{
									trial_p -> ft_name_s = copied_name_s;
									trial_p -> ft_team_s = copied_team_s;
									trial_p -> ft_id_p = id_p;

									return trial_p;
								}

							if (id_p)
								{
									FreeMemory (id_p);
								}
						}

					FreeCopiedString (copied_team_s);
				}		/* if (copied_team_s) */

			FreeCopiedString (copied_name_s);
		}		/* if (copied_name_s) */

	return NULL;
}



bool GenerateIdFromString (bson_oid_t **id_pp, const char *id_s)
{
	bool success_flag = false;

	if (bson_oid_is_valid (id_s, strlen (id_s)))
		{
			bson_oid_t *id_p = AllocMemory (sizeof (bson_oid_t));

			if (id_p)
				{
					bson_oid_init_from_string (id_p, id_s);
					*id_pp = id_p;
					success_flag = true;
				}
		}

	return success_flag;
}


bson_oid_t *GetNewId (void)
{
	bson_oid_t *id_p = AllocMemory (sizeof (bson_oid_t));

	if (id_p)
		{
			bson_oid_init (id_p, bson_context_get_default ());
		}

	return id_p;
}


char *GetFieldTrialIdAsString (const FieldTrial *trial_p)
{
	char *id_s = NULL;

	if (trial_p -> ft_id_p)
		{
			id_s = GetBSONOidAsString (trial_p -> ft_id_p);
		}

	return id_s;
}


bool SaveFieldTrial (FieldTrial *trial_p, DFWFieldTrialServiceData *data_p)
{
	bool success_flag = false;
	bool insert_flag = false;
	json_t *field_trial_json_p = NULL;

	if (! (trial_p -> ft_id_p))
		{
			trial_p -> ft_id_p  = GetNewId ();

			if (trial_p -> ft_id_p)
				{
					insert_flag = true;
				}
		}

	if (trial_p -> ft_id_p)
		{
			field_trial_json_p = GetFieldTrialAsJSON (trial_p);

			if (field_trial_json_p)
				{
					success_flag = SaveMongoData (data_p -> dftsd_mongo_p, field_trial_json_p, data_p -> dftsd_collection_ss [DFTD_FIELD_TRIAL], insert_flag);

					json_decref (field_trial_json_p);
				}		/* if (field_trial_json_p) */

		}		/* if (trial_p -> ft_id_p) */

	return success_flag;
}



void FreeFieldTrial (FieldTrial *trial_p)
{
	FreeCopiedString (trial_p -> ft_name_s);
	FreeCopiedString (trial_p -> ft_team_s);

	if (trial_p -> ft_id_p)
		{
			FreeMemory (trial_p -> ft_id_p);
		}


	FreeMemory (trial_p);
}


LinkedList *GetFieldTrialsByName (DFWFieldTrialServiceData *data_p, const char *name_s)
{
	LinkedList *trials_p = NULL;

	switch (data_p -> dftsd_backend)
		{
			case DB_MONGO_DB:
				trials_p = GetFieldTrialsByNameFromMongoDB (data_p, name_s);
				break;

			default:
				break;
		}

	return trials_p;
}



json_t *GetFieldTrialAsJSON (const FieldTrial *trial_p)
{
	return GetFieldTrialAsConfiguredJSON (trial_p, FT_NAME_S, FT_TEAM_S);
}



json_t *GetFieldTrialAsConfiguredJSON (const FieldTrial *trial_p, const char * const name_key_s, const char * const team_key_s)
{
	json_t *trial_json_p = json_object ();

	if (trial_json_p)
		{
			if (json_object_set_new (trial_json_p, name_key_s, json_string (trial_p -> ft_name_s)) == 0)
				{
					if (json_object_set_new (trial_json_p, team_key_s, json_string (trial_p -> ft_team_s)) == 0)
						{

							return trial_json_p;
						}		/* if (json_object_set_new (trial_json_p, team_key_s, json_string (trial_p -> ft_team_s)) == 0) */

				}		/* if (json_object_set_new (trial_json_p, name_key_s, json_string (trial_p -> ft_name_s)) == 0) */

			json_decref (trial_json_p);
		}		/* if (trial_json_p) */

	return NULL;
}


FieldTrial *GetFieldTrialFromJSON (const json_t *json_p, DFWFieldTrialServiceData *data_p)
{
	const char *name_s = GetJSONString (json_p, FT_NAME_S);

	if (name_s)
		{
			const char *team_s = GetJSONString (json_p, FT_TEAM_S);

			if (team_s)
				{
					const char *id_s = GetJSONString (json_p, FT_ID_S);

					FieldTrial *trial_p = AllocateFieldTrial (name_s, team_s, id_s);

					return trial_p;
				}

		}

	return NULL;
}


FieldTrialNode *AllocateFieldTrialNodeByParts (const char *name_s, const char *team_s, const char *id_s)
{
	FieldTrial *trial_p = AllocateFieldTrial (name_s, team_s, id_s);

	if (trial_p)
		{
			FieldTrialNode *node_p = AllocateFieldTrialNode (trial_p);

			if (node_p)
				{
					return node_p;
				}

			FreeFieldTrial (trial_p);
		}		/* if (trial_p) */

	return NULL;
}


FieldTrialNode *AllocateFieldTrialNode (FieldTrial *trial_p)
{
	FieldTrialNode *node_p = (FieldTrialNode *) AllocMemory (sizeof (FieldTrialNode));

	if (node_p)
		{
			InitListItem (& (node_p -> ftn_node));
			node_p -> ftn_field_trial_p = trial_p;
		}

	return node_p;
}


void FreeFieldTrialNode (ListItem *node_p)
{
	FieldTrialNode *field_trial_node_p = (FieldTrialNode *) node_p;

	if (field_trial_node_p -> ftn_field_trial_p)
		{
			FreeFieldTrial (field_trial_node_p -> ftn_field_trial_p);
		}

	FreeMemory (field_trial_node_p);
}


bool AddFieldTrialExperimentalArea (FieldTrial *trial_p, ExperimentalArea *area_p, DFWFieldTrialServiceData *data_p)
{
	bool success_flag = false;
	json_t *area_json_p = GetExperimentalAreaAsJSON (area_p);

	if (area_json_p)
		{


			json_decref (area_json_p);
		}		/* if (area_json_p) */

	return success_flag;
}



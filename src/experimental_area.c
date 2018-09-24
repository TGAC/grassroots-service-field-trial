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
 * experimental_area.c
 *
 *  Created on: 18 Sep 2018
 *      Author: billy
 */

#define ALLOCATE_EXPERIMENTAL_AREA_TAGS (1)
#include "experimental_area.h"
#include "memory_allocations.h"
#include "string_utils.h"

/*
 * DB COLUMN NAMES
 */

static const char *S_ID_COLUMN_S = "id";

static const char *S_NAME_COLUMN_S = "name";

static const char *S_LOCATION_COLUMN_S = "location";

static const char *S_SOIL_COLUMN_S = "soil";

static const char *S_YEAR_COLUMN_S = "year";

static const char *S_PARENT_FIELD_TRIAL_COLUMN_S = "parent_field_trial_id";


/*
 * STATIC PROTOTYPES
 */
static json_t *GetExperimentalAreaAsJSONWithNamedKeys (const ExperimentalArea *area_p, const char *name_key_s, const char *soil_key_s, const char *location_key_s, const char *year_key_s, const char *field_trial_key_s, const char *id_key_s);



/*
 * API FUNCTIONS
 */

ExperimentalArea *AllocateExperimentalArea ()
{
	ExperimentalArea *area_p = NULL;

	return area_p;
}


void FreeExperimentalArea (ExperimentalArea *area_p)
{
	if (area_p -> ea_name_s)
		{
			FreeCopiedString (area_p -> ea_name_s);
		}

	if (area_p -> ea_soil_type_s)
		{
			FreeCopiedString (area_p -> ea_soil_type_s);
		}

	if (area_p -> ea_location_s)
		{
			FreeCopiedString (area_p -> ea_location_s);
		}

	if (area_p -> ea_plots_p)
		{
			FreeLinkedList (area_p -> ea_plots_p);
		}

	FreeMemory (area_p);
}


json_t *GetExperimentalAreaAsJSON (const ExperimentalArea *area_p)
{
	return GetExperimentalAreaAsJSONWithNamedKeys (area_p, EA_NAME_S, EA_SOIL_S, EA_LOCATION_S, EA_YEAR_S, EA_PARENT_FIELD_TRIAL_S, EA_ID_S);
}


json_t *GetExperimentalAreaAsJSONForDB (const ExperimentalArea *area_p)
{
	return GetExperimentalAreaAsJSONWithNamedKeys (area_p, S_NAME_COLUMN_S, S_SOIL_COLUMN_S, S_LOCATION_COLUMN_S, S_YEAR_COLUMN_S, S_PARENT_FIELD_TRIAL_COLUMN_S, S_ID_COLUMN_S);
}



LinkedList *GetExperimentalAreaPlots (ExperimentalArea *area_p)
{
	LinkedList *plots_list_p = NULL;

	if (! (area_p -> ea_plots_p))
		{

		}		/* if (! (area_p -> ea_plots_p)) */

	return plots_list_p;
}



bool SaveExperimentalArea (ExperimentalArea *area_p, DFWFieldTrialServiceData *data_p)
{
	bool success_flag = false;
	bool insert_flag = false;

	if (! (area_p -> ea_id_p))
		{
			area_p -> ea_id_p  = GetNewId ();

			if (area_p -> ea_id_p)
				{
					insert_flag = true;
				}
		}

	if (area_p -> ea_id_p)
		{
			json_t *area_json_p = GetExperimentalAreaAsJSON (area_p);

			if (area_json_p)
				{
					success_flag = SaveMongoData (data_p -> dftsd_mongo_p, area_json_p, data_p -> dftsd_collection_ss [DFTD_EXPERIMENTAL_AREA], insert_flag);

					json_decref (area_json_p);
				}		/* if (area_json_p) */

		}		/* if (area_p -> ea_id_p) */

	return success_flag;
}

ExperimentalArea *LoadExperimentalArea (const int32 area_id, DFWFieldTrialServiceData *data_p)
{
	ExperimentalArea *area_p = NULL;


	return area_p;
}




/*
 * STATIC FUNCTIONS
 */

static json_t *GetExperimentalAreaAsJSONWithNamedKeys (const ExperimentalArea *area_p, const char *name_key_s, const char *soil_key_s, const char *location_key_s, const char *year_key_s, const char *field_trial_key_s, const char *id_key_s)
{
	json_t *area_json_p = json_object ();

	if (area_json_p)
		{
			if (json_object_set_new (area_json_p, name_key_s, json_string (area_p -> ea_name_s)) == 0)
				{
					if (json_object_set_new (area_json_p, soil_key_s, json_string (area_p -> ea_soil_type_s)) == 0)
						{
							if (json_object_set_new (area_json_p, location_key_s, json_string (area_p -> ea_location_s)) == 0)
								{
									if (json_object_set_new (area_json_p, year_key_s, json_integer (area_p -> ea_year)) == 0)
										{
											char *parent_field_trial_id_s = GetFieldTrialIdAsString (area_p -> ea_parent_p);

											if (parent_field_trial_id_s)
												{
													if (json_object_set_new (area_json_p, field_trial_key_s, json_string (parent_field_trial_id_s)) == 0)
														{
															char *id_s = GetBSONOidAsString (area_p -> ea_id_p);

															if (id_s)
																{
																	if (json_object_set_new (area_json_p, id_key_s, json_string (id_s)) == 0)
																		{
																			return area_json_p;
																		}

																	FreeCopiedString (id_s);
																}
														}

													FreeCopiedString (parent_field_trial_id_s);
												}
										}
								}
						}
				}

			json_decref (area_json_p);
		}		/* if (area_json_p) */

	return NULL;
}

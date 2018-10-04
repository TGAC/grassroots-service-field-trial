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
#include "plot.h"

/*
 * DB COLUMN NAMES
 */

/*
 * STATIC PROTOTYPES
 */



/*
 * API FUNCTIONS
 */

ExperimentalArea *AllocateExperimentalArea (bson_oid_t *id_p, const char *name_s, const char *soil_s, const uint32 sowing_year, const uint32 harvest_year, Address *address_p, FieldTrial *parent_field_trial_p, DFWFieldTrialServiceData *data_p)
{
	char *copied_name_s = EasyCopyToNewString (name_s);

	if (copied_name_s)
		{
			char *copied_soil_s = EasyCopyToNewString (soil_s);

			if (copied_soil_s)
				{
					LinkedList *plots_p = AllocateLinkedList (FreePlotNode);

					if (plots_p)
						{
							ExperimentalArea *area_p = (ExperimentalArea *) AllocMemory (sizeof (ExperimentalArea));

							if (area_p)
								{
									area_p -> ea_id_p = id_p;
									area_p -> ea_address_p = address_p;
									area_p -> ea_name_s = copied_name_s;
									area_p -> ea_soil_type_s = copied_soil_s;
									area_p -> ea_sowing_year = sowing_year;
									area_p -> ea_harvest_year = harvest_year;
									area_p -> ea_parent_p = parent_field_trial_p;
									area_p -> ea_plots_p = NULL;

									return area_p;
								}

							FreeLinkedList (plots_p);
						}		/* if (plots_p) */


					FreeCopiedString (copied_soil_s);
				}		/* if (copied_soil_s) */
			else
				{

				}

			FreeCopiedString (copied_name_s);
		}		/* if (copied_name_s) */
	else
		{

		}

	return NULL;
}


ExperimentalArea *AllocateExperimentalAreaByIDString (bson_oid_t *id_p, const char *name_s, const char *location_s, const char *soil_s, const uint32 sowing_year, const uint32 harvest_year, const char *parent_field_trial_id_s, DFWFieldTrialServiceData *data_p)
{
	FieldTrial *parent_field_trial_p = GetFieldTrialByIdString (parent_field_trial_id_s, data_p);

	if (parent_field_trial_p)
		{
			ExperimentalArea *area_p = AllocateExperimentalArea (id_p, name_s, location_s, soil_s, sowing_year, harvest_year, parent_field_trial_p, data_p);

			if (area_p)
				{
					return area_p;
				}

			FreeFieldTrial (parent_field_trial_p);
		}		/* if (parent_field_trial_p) */

	return NULL;
}


void FreeExperimentalArea (ExperimentalArea *area_p)
{
	if (area_p -> ea_id_p)
		{
			FreeBSONOid (area_p -> ea_id_p);
		}

	if (area_p -> ea_name_s)
		{
			FreeCopiedString (area_p -> ea_name_s);
		}

	if (area_p -> ea_soil_type_s)
		{
			FreeCopiedString (area_p -> ea_soil_type_s);
		}

	if (area_p -> ea_address_p)
		{
			FreeAddress (area_p -> ea_address_p);
		}

	if (area_p -> ea_plots_p)
		{
			FreeLinkedList (area_p -> ea_plots_p);
		}

	FreeMemory (area_p);
}


ExperimentalAreaNode *AllocateExperimentalAreaNode (ExperimentalArea *area_p)
{
	ExperimentalAreaNode *ea_node_p = (ExperimentalAreaNode *) AllocMemory (sizeof (ExperimentalAreaNode));

	if (ea_node_p)
		{
			InitListItem (& (ea_node_p -> ean_node));

			ea_node_p -> ean_experimental_area_p = area_p;
		}

	return ea_node_p;
}

void FreeExperimentalAreaNode (ListItem *node_p)
{
	ExperimentalAreaNode *ea_node_p = (ExperimentalAreaNode *) node_p;

	if (ea_node_p -> ean_experimental_area_p)
		{
			FreeExperimentalArea (ea_node_p -> ean_experimental_area_p);
		}

	FreeMemory (ea_node_p);
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


json_t *GetExperimentalAreaAsJSON (const ExperimentalArea *area_p)
{
	json_t *area_json_p = json_object ();

	if (area_json_p)
		{
			if (json_object_set_new (area_json_p, EA_NAME_S, json_string (area_p -> ea_name_s)) == 0)
				{
					if (json_object_set_new (area_json_p, EA_SOIL_S, json_string (area_p -> ea_soil_type_s)) == 0)
						{
							json_t *address_json_p = GetAddressAsJSON (area_p -> ea_address_p);

							if (address_json_p)
								{
									if (json_object_set_new (area_json_p, EA_LOCATION_S, address_json_p) == 0)
										{
											if ((area_p -> ea_sowing_year == 0) || json_object_set_new (area_json_p, EA_SOWING_YEAR_S, json_integer (area_p -> ea_sowing_year)) == 0)
												{
													if ((area_p -> ea_harvest_year == 0) || json_object_set_new (area_json_p, EA_HARVEST_YEAR_S, json_integer (area_p -> ea_harvest_year)) == 0)
														{
															if (AddCompoundIdToJSON (area_json_p, area_p -> ea_id_p))
																{
																	if (AddIdToJSON (area_json_p, area_p -> ea_parent_p -> ft_id_p, EA_PARENT_FIELD_TRIAL_S))
																		{
																			return area_json_p;
																		}
																}
														}
												}
										}		/* if (json_object_set_new (area_json_p, EA_LOCATION_S, address_json_p) == 0) */
									else
										{
											json_decref (address_json_p);
										}

								}		/* if (address_json_p) */

						}
				}

			json_decref (area_json_p);
		}		/* if (area_json_p) */

	return NULL;
}



ExperimentalArea *GetExperimentalAreaFromJSON (const json_t *json_p, DFWFieldTrialServiceData *data_p)
{
	const char *name_s = GetJSONString (json_p, EA_NAME_S);

	if (name_s)
		{
			const char *soil_s = GetJSONString (json_p, EA_SOIL_S);

			if (soil_s)
				{
					const char *location_s = GetJSONString (json_p, EA_LOCATION_S);

					if (location_s)
						{
							bson_oid_t *id_p = AllocMemory (sizeof (bson_oid_t));

							if (id_p)
								{
									if (GetCompoundIdFromJSON (json_p, id_p))
										{
											ExperimentalArea *area_p = NULL;
											uint32 sowing_year = 0;
											uint32 harvest_year = 0;
											const char *parent_field_trial_id_s = NULL;

											GetJSONInteger (json_p, EA_SOWING_YEAR_S, (int *) &sowing_year);
											GetJSONInteger (json_p, EA_HARVEST_YEAR_S,(int *) &harvest_year);

											area_p = AllocateExperimentalArea (id_p, name_s, location_s, soil_s, sowing_year, harvest_year, NULL, data_p);

											return area_p;
										}

									FreeMemory (id_p);
								}

						}		/* if (location_s) */

				}		/* if (soil_s) */

		}		/* if (name_s) */

	return NULL;
}
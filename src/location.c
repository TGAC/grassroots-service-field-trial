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
 * location.c
 *
 *  Created on: 4 Oct 2018
 *      Author: billy
 */

#define ALLOCATE_LOCATION_TAGS (1)
#include "location.h"
#include "memory_allocations.h"


Location *AllocateLocation (Address *address_p, const uint32 order, ExperimentalArea *parent_area_p, bson_oid_t *id_p)
{
	Location *location_p = (Location *) AllocMemory (sizeof (Location));

	if (location_p)
		{
			location_p -> lo_parent_area_p = parent_area_p;
			location_p -> lo_address_p = address_p;
			location_p -> lo_order = order;
			location_p -> lo_id_p = id_p;
		}		/* if (location_p) */

	return location_p;
}


void FreeLocation (Location *location_p)
{
	if (location_p -> lo_address_p)
		{
			FreeAddress (location_p -> lo_address_p);
		}

	if (location_p -> lo_id_p)
		{
			FreeBSONOid (location_p -> lo_id_p);
		}

	FreeMemory (location_p);
}



json_t *GetLocationAsJSON (Location *location_p, DFWFieldTrialServiceData *data_p)
{
	json_t *location_json_p = json_object ();

	if (location_json_p)
		{
			if (AddIdToJSON (location_json_p, location_p -> lo_id_p, MONGO_ID_S))
				{
					if (AddIdToJSON (location_json_p, location_p -> lo_parent_area_p -> ea_id_p, LO_PARENT_EXPERIMENTAL_AREA_S))
						{
							if (SetJSONInteger (location_json_p, LO_ORDER_S, location_p -> lo_order))
								{
									json_t *address_json_p = GetAddressAsJSON (location_p -> lo_address_p);

									if (address_json_p)
										{
											if (json_object_set_new (location_json_p, LO_ADDRESS_S, address_json_p) == 0)
												{
													return location_json_p;
												}		/* if (json_object_set_new (location_json_p, LO_ADDRESS_S, address_json_p) == 0) */
											else
												{
													json_decref (address_json_p);
												}

										}		/* if (geo_json_p) */

								}		/* if (SetJSONInteger (location_json_p, LO_ORDER_S, location_p -> lo_order)) */

						}		/* if (AddIdToJSON (location_json_p, location_p -> lo_parent_area_p -> ea_id_p, LO_PARENT_EXPERIMENTAL_AREA_S)) */

				}		/* if (AddIdToJSON (location_json_p, location_p -> lo_id_p, MONGO_ID_S)) */

			json_decref (location_json_p);
		}		/* if (location_json_p) */

	return NULL;
}


Location *GetLocationFromJSON (const json_t *location_json_p, const DFWFieldTrialServiceData *data_p)
{
	uint32 order;

	if (GetJSONInteger (location_json_p, LO_ORDER_S, (int *) &order))
		{
			bson_oid_t *id_p = AllocMemory (sizeof (bson_oid_t));

			if (id_p)
				{
					if (GetMongoIdFromJSON (location_json_p, id_p))
						{
							const json_t *address_json_p = json_object_get (location_json_p, LO_ADDRESS_S);

							if (address_json_p)
								{
									Address *address_p = GetAddressFromJSON (address_json_p);

									if (address_p)
										{
											Location *location_p = AllocateLocation (address_p, order, NULL, id_p);

											if (location_p)
												{
													return location_p;
												}

											FreeAddress (address_p);
										}		/* if (address_p) */


								}		/* if (address_json_p) */

						}		/* if (GetCompoundIdFromJSON (location_json_p, id_p)) */

					FreeMemory (id_p);
				}		/* if (id_p) */

		}		/* if (GetJSONInteger (location_json_p, LO_ORDER_S, (int *) &order)) */

	return NULL;
}




char *GetLocationAsString (const Location *location_p)
{
	char *location_s = NULL;

	if (location_p -> lo_address_p)
		{
			location_s = GetAddressAsString (location_p -> lo_address_p);
		}

	return location_s;
}

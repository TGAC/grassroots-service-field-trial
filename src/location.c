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
#include "study.h"
#include "dfw_util.h"


static void *GetLocationFromJSONCallback (const json_t *location_json_p, const DFWFieldTrialServiceData *data_p);



Location *AllocateLocation (Address *address_p, const uint32 order, bson_oid_t *id_p)
{
	Location *location_p = (Location *) AllocMemory (sizeof (Location));

	if (location_p)
		{
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



json_t *GetLocationAsJSON (Location *location_p)
{
	json_t *location_json_p = json_object ();

	if (location_json_p)
		{
			if (AddCompoundIdToJSON (location_json_p, location_p -> lo_id_p))
				{
					if (SetJSONInteger (location_json_p, LO_ORDER_S, location_p -> lo_order))
						{
							json_t *address_json_p = GetAddressAsJSON (location_p -> lo_address_p);

							if (address_json_p)
								{
									if (json_object_set_new (location_json_p, LO_ADDRESS_S, address_json_p) == 0)
										{
											if (AddDatatype (location_json_p, DFTD_LOCATION))
												{
													return location_json_p;
												}

										}		/* if (json_object_set_new (location_json_p, LO_ADDRESS_S, address_json_p) == 0) */
									else
										{
											json_decref (address_json_p);
										}

								}		/* if (geo_json_p) */

						}		/* if (SetJSONInteger (location_json_p, LO_ORDER_S, location_p -> lo_order)) */

				}		/* if (AddCompoundIdToJSON (location_json_p, location_p -> lo_id_p)) */

			json_decref (location_json_p);
		}		/* if (location_json_p) */

	return NULL;
}


Location *GetLocationFromJSON (const json_t *location_json_p, const DFWFieldTrialServiceData * UNUSED_PARAM (data_p))
{
	uint32 order;

	if (GetJSONInteger (location_json_p, LO_ORDER_S, (int *) &order))
		{
			bson_oid_t *id_p = GetNewUnitialisedBSONOid ();

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
											Location *location_p = AllocateLocation (address_p, order, id_p);

											if (location_p)
												{
													return location_p;
												}

											FreeAddress (address_p);
										}		/* if (address_p) */


								}		/* if (address_json_p) */

						}		/* if (GetCompoundIdFromJSON (location_json_p, id_p)) */

					FreeBSONOid (id_p);
				}		/* if (id_p) */

		}		/* if (GetJSONInteger (location_json_p, LO_ORDER_S, (int *) &order)) */

	return NULL;
}




bool SaveLocation (Location *location_p, DFWFieldTrialServiceData *data_p)
{
	bson_t *selector_p = NULL;
	bool success_flag = PrepareSaveData (& (location_p -> lo_id_p), &selector_p);

	if (success_flag)
		{
			json_t *location_json_p = GetLocationAsJSON (location_p);

			if (location_json_p)
				{
					success_flag = SaveMongoData (data_p -> dftsd_mongo_p, location_json_p, data_p -> dftsd_collection_ss [DFTD_LOCATION], selector_p);

					json_decref (location_json_p);
				}		/* if (area_json_p) */
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get location \"%s\" as JSON", location_p -> lo_address_p -> ad_name_s);
					success_flag = false;
				}

		}		/* if (location_p -> lo_id_p) */

	return success_flag;
}



Location *GetLocationById (bson_oid_t *id_p, const ViewFormat format, const DFWFieldTrialServiceData *data_p)
{
	Location *location_p = GetDFWObjectById (id_p, DFTD_LOCATION, GetLocationFromJSONCallback, format, data_p);

	return location_p;
}


Location *GetLocationByIdString (const char *location_id_s, const ViewFormat format, const DFWFieldTrialServiceData *data_p)
{
	Location *location_p = GetDFWObjectByIdString (location_id_s, DFTD_LOCATION, GetLocationFromJSONCallback, format, data_p);

	return location_p;
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



static void *GetLocationFromJSONCallback (const json_t *location_json_p, const DFWFieldTrialServiceData *data_p)
{
	return GetLocationFromJSON (location_json_p, data_p);
}


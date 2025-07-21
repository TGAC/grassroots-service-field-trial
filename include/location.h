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
 * location.h
 *
 *  Created on: 11 Sep 2018
 *      Author: billy
 */

#ifndef SERVICES_FIELD_TRIALS_INCLUDE_LOCATION_H_
#define SERVICES_FIELD_TRIALS_INCLUDE_LOCATION_H_

#include "study.h"
#include "jansson.h"

#include "address.h"

struct Study;

typedef enum
{
	LT_UNKNOWN,
	LT_FARM,
	LT_SITE,
	LT_NUM_TYPES
} LocationType;





typedef struct Location
{
	bson_oid_t *lo_id_p;

	uint32 lo_order;

	char *lo_soil_s;

	double64 *lo_min_ph_p;

	double64 *lo_max_ph_p;

	LocationType lo_type;

	Address *lo_address_p;
} Location;


typedef struct LocationNode
{
	ListItem ln_node;

	Location *ln_location_p;
} LocationNode;


#ifndef DOXYGEN_SHOULD_SKIP_THIS

#ifdef ALLOCATE_LOCATION_TAGS
	#define LOCATION_PREFIX DFW_FIELD_TRIAL_SERVICE_LOCAL
	#define LOCATION_VAL(x)	= x
	#define LOCATION_CONCAT_VAL(x,y)	= x y
#else
	#define LOCATION_PREFIX extern
	#define LOCATION_VAL(x)
	#define LOCATION_CONCAT_VAL(x,y)
#endif

#endif 		/* #ifndef DOXYGEN_SHOULD_SKIP_THIS */




/* Location */



LOCATION_PREFIX const char *LO_ADDRESS_S LOCATION_VAL ("address");

LOCATION_PREFIX const char *LO_ORDER_S LOCATION_VAL ("order");

LOCATION_PREFIX const char *LO_NAME_S LOCATION_VAL ("name");


LOCATION_PREFIX const char *LO_SOIL_S LOCATION_VAL ("soil");


LOCATION_PREFIX const char *LO_MIN_PH_S LOCATION_VAL ("min_ph");

LOCATION_PREFIX const char *LO_MAX_PH_S LOCATION_VAL ("max_ph");

LOCATION_PREFIX const char *LO_TYPE_S LOCATION_VAL ("type");

LOCATION_PREFIX const char *LT_FARM_S LOCATION_VAL ("farm");

LOCATION_PREFIX const char *LT_SITE_S LOCATION_VAL ("site");

LOCATION_PREFIX const char *LT_UNKNOWN_S LOCATION_VAL ("unknown");




#ifdef __cplusplus
extern "C"
{
#endif



DFW_FIELD_TRIAL_SERVICE_LOCAL Location *AllocateLocation (Address *address_p, const uint32 order, const char *soil_s, const double64 *min_ph_p, const double64 *max_ph_p, const LocationType loc_type, bson_oid_t *id_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL void FreeLocation (Location *location_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL Location *CopyLocation (const Location * const src_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL LocationNode *AllocateLocationNode (Location *location_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL void FreeLocationNode (ListItem *node_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL json_t *GetLocationAsJSON (Location *location_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL Location *GetLocationFromJSON (const json_t *location_json_p, const FieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL char *GetLocationAsString (const Location *location_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL Location *GetLocationById (bson_oid_t *id_p, const ViewFormat format, const FieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL Location *GetLocationByIdString (const char *location_id_s, const ViewFormat format, const FieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL Location *GetUniqueLocationBySearchString (const char *location_s, const ViewFormat format, const FieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL LinkedList *GetLocationsByName (FieldTrialServiceData *data_p, const char *location_s);


DFW_FIELD_TRIAL_SERVICE_LOCAL OperationStatus SaveLocation (Location *location_p, ServiceJob *job_p, FieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL const char *GetLocationTypeAsString (const LocationType loc_type);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool GetLocationTypeFromString (const char *loc_type_s, LocationType *loc_type_p);

#ifdef __cplusplus
}
#endif


#endif /* SERVICES_FIELD_TRIALS_INCLUDE_LOCATION_H_ */

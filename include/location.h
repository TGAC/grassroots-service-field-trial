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

#ifndef SERVICES_DFW_FIELD_TRIAL_SERVICE_INCLUDE_LOCATION_H_
#define SERVICES_DFW_FIELD_TRIAL_SERVICE_INCLUDE_LOCATION_H_

#include "study.h"
#include "jansson.h"

#include "address.h"

struct Study;

typedef struct Location
{
	bson_oid_t *lo_id_p;

	// struct ExperimentalArea *lo_parent_area_p;

	uint32 lo_order;

	Address *lo_address_p;
} Location;



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

LOCATION_PREFIX const char *LO_PARENT_EXPERIMENTAL_AREA_S LOCATION_VAL ("parent_experimental_area_id");


#ifdef __cplusplus
extern "C"
{
#endif


DFW_FIELD_TRIAL_SERVICE_LOCAL Location *AllocateLocation (Address *address_p, const uint32 order, bson_oid_t *id_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL void FreeLocation (Location *location_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL json_t *GetLocationAsJSON (Location *location_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL Location *GetLocationFromJSON (const json_t *location_json_p, const DFWFieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL char *GetLocationAsString (const Location *location_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL Location *GetLocationById (bson_oid_t *id_p, const ViewFormat format, const DFWFieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL Location *GetLocationByIdString (const char *location_id_s, const ViewFormat format, const DFWFieldTrialServiceData *data_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL bool SaveLocation (Location *location_p, DFWFieldTrialServiceData *data_p);


#ifdef __cplusplus
}
#endif


#endif /* SERVICES_DFW_FIELD_TRIAL_SERVICE_INCLUDE_LOCATION_H_ */

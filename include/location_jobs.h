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
 * location_jobs.h
 *
 *  Created on: 5 Oct 2018
 *      Author: billy
 */

#ifndef SERVICES_FIELD_TRIALS_INCLUDE_LOCATION_JOBS_H_
#define SERVICES_FIELD_TRIALS_INCLUDE_LOCATION_JOBS_H_

#include "dfw_field_trial_service_data.h"
#include "dfw_field_trial_service_library.h"
#include "address.h"
#include "location.h"

#include "string_parameter.h"



#ifndef DOXYGEN_SHOULD_SKIP_THIS

#ifdef ALLOCATE_LOCATION_JOB_CONSTANTS
	#define LOCATION_JOB_PREFIX DFW_FIELD_TRIAL_SERVICE_API
	#define LOCATION_JOB_STRUCT_VAL(x,y)	= { x, y}
#else
	#define LOCATION_JOB_PREFIX extern
	#define LOCATION_JOB_STRUCT_VAL(x,y)
#endif

#endif 		/* #ifndef DOXYGEN_SHOULD_SKIP_THIS */




LOCATION_JOB_PREFIX NamedParameterType LOCATION_NAME LOCATION_JOB_STRUCT_VAL("LO Name", PT_STRING);
LOCATION_JOB_PREFIX NamedParameterType LOCATION_STREET LOCATION_JOB_STRUCT_VAL("LO Street", PT_STRING);
LOCATION_JOB_PREFIX NamedParameterType LOCATION_TOWN LOCATION_JOB_STRUCT_VAL("LO Town", PT_STRING);
LOCATION_JOB_PREFIX NamedParameterType LOCATION_COUNTY LOCATION_JOB_STRUCT_VAL("LO County", PT_STRING);
LOCATION_JOB_PREFIX NamedParameterType LOCATION_COUNTRY LOCATION_JOB_STRUCT_VAL("LO Country", PT_STRING);
LOCATION_JOB_PREFIX NamedParameterType LOCATION_POSTCODE LOCATION_JOB_STRUCT_VAL("LO Postcode", PT_STRING);
LOCATION_JOB_PREFIX NamedParameterType LOCATION_USE_GPS LOCATION_JOB_STRUCT_VAL("LO Use GPS", PT_BOOLEAN);
LOCATION_JOB_PREFIX NamedParameterType LOCATION_LATITUDE LOCATION_JOB_STRUCT_VAL("LO Latitude", PT_SIGNED_REAL);
LOCATION_JOB_PREFIX NamedParameterType LOCATION_LONGITUDE LOCATION_JOB_STRUCT_VAL("LO Longitude", PT_SIGNED_REAL);
LOCATION_JOB_PREFIX NamedParameterType LOCATION_ALTITUDE LOCATION_JOB_STRUCT_VAL("LO Altitude", PT_SIGNED_REAL);


LOCATION_JOB_PREFIX NamedParameterType LOCATION_MIN_PH LOCATION_JOB_STRUCT_VAL("LO pH Minimum", PT_UNSIGNED_REAL);
LOCATION_JOB_PREFIX NamedParameterType LOCATION_MAX_PH LOCATION_JOB_STRUCT_VAL("LO pH Maximum", PT_UNSIGNED_REAL);
LOCATION_JOB_PREFIX NamedParameterType LOCATION_SOIL LOCATION_JOB_STRUCT_VAL("LO Soil", PT_STRING);

LOCATION_JOB_PREFIX NamedParameterType LOCATION_TYPE LOCATION_JOB_STRUCT_VAL("LO Type", PT_STRING);


LOCATION_JOB_PREFIX NamedParameterType LOCATION_ID LOCATION_JOB_STRUCT_VAL("Location ID", PT_STRING);
LOCATION_JOB_PREFIX NamedParameterType LOCATION_GET_ALL_LOCATIONS LOCATION_JOB_STRUCT_VAL("Get all Locations", PT_BOOLEAN);



#ifdef __cplusplus
extern "C"
{
#endif



DFW_FIELD_TRIAL_SERVICE_LOCAL bool AddSubmissionLocationParams (ServiceData *data_p, ParameterSet *param_set_p, DataResource *resource_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool RunForSubmissionLocationParams (FieldTrialServiceData *data_p, ParameterSet *param_set_p, ServiceJob *job_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool GetSubmissionLocationParameterTypeForNamedParameter (const char *param_name_s, ParameterType *pt_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL Address *GetAddressFromLocationString (const char *location_s);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool SetUpLocationsListParameter (const FieldTrialServiceData *data_p, StringParameter *param_p, const Location *active_location_p, const char *extra_option_s);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool AddSearchLocationParams (ServiceData *data_p, ParameterSet *param_set_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool RunForSearchLocationParams (FieldTrialServiceData *data_p, ParameterSet *param_set_p, ServiceJob *job_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool GetSearchLocationParameterTypeForNamedParameter (const char *param_name_s, ParameterType *pt_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL const char *GetUnsetLocationValue (void);

DFW_FIELD_TRIAL_SERVICE_LOCAL json_t *GetAllLocationsAsJSON (const FieldTrialServiceData *data_p, bson_t *opts_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool AddLocationToServiceJob (ServiceJob *job_p, Location *location_p, const ViewFormat format, FieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL Location *GetLocationFromResource (DataResource *resource_p, const NamedParameterType location_param_type, FieldTrialServiceData *dfw_data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL json_t *GetLocationIndexingData (Service *service_p);

#ifdef __cplusplus
}
#endif




#endif /* SERVICES_FIELD_TRIALS_INCLUDE_LOCATION_JOBS_H_ */

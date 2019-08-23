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

#ifndef SERVICES_DFW_FIELD_TRIAL_SERVICE_INCLUDE_LOCATION_JOBS_H_
#define SERVICES_DFW_FIELD_TRIAL_SERVICE_INCLUDE_LOCATION_JOBS_H_

#include "dfw_field_trial_service_data.h"
#include "dfw_field_trial_service_library.h"
#include "address.h"





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



#ifdef __cplusplus
extern "C"
{
#endif



DFW_FIELD_TRIAL_SERVICE_LOCAL bool AddSubmissionLocationParams (ServiceData *data_p, ParameterSet *param_set_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool RunForSubmissionLocationParams (DFWFieldTrialServiceData *data_p, ParameterSet *param_set_p, ServiceJob *job_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool GetSubmissionLocationParameterTypeForNamedParameter (const char *param_name_s, ParameterType *pt_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL Address *GetAddressFromLocationString (const char *location_s);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool SetUpLocationsListParameter (const DFWFieldTrialServiceData *data_p, Parameter *param_p, const bool add_any_flag);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool AddSearchLocationParams (ServiceData *data_p, ParameterSet *param_set_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool RunForSearchLocationParams (DFWFieldTrialServiceData *data_p, ParameterSet *param_set_p, ServiceJob *job_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool GetSearchLocationParameterTypeForNamedParameter (const char *param_name_s, ParameterType *pt_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL const char *GetUnsetLocationValue (void);

DFW_FIELD_TRIAL_SERVICE_LOCAL json_t *GetAllLocationsAsJSON (const DFWFieldTrialServiceData *data_p, bson_t *opts_p);


#ifdef __cplusplus
}
#endif




#endif /* SERVICES_DFW_FIELD_TRIAL_SERVICE_INCLUDE_LOCATION_JOBS_H_ */

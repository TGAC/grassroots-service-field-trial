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
 * field_trial_jobs.h
 *
 *  Created on: 1 Oct 2018
 *      Author: billy
 */

#ifndef SERVICES_DFW_FIELD_TRIAL_SERVICE_INCLUDE_FIELD_TRIAL_JOBS_H_
#define SERVICES_DFW_FIELD_TRIAL_SERVICE_INCLUDE_FIELD_TRIAL_JOBS_H_


#include "dfw_field_trial_service_data.h"
#include "dfw_field_trial_service_library.h"

#include "field_trial.h"


#ifndef DOXYGEN_SHOULD_SKIP_THIS

#ifdef ALLOCATE_FIELD_TRIAL_CONSTANTS
	#define FIELD_TRIAL_JOB_PREFIX DFW_FIELD_TRIAL_SERVICE_API
	#define FIELD_TRIAL_JOB_STRUCT_VAL(x,y)	= { x, y}
#else
	#define FIELD_TRIAL_JOB_PREFIX extern
	#define FIELD_TRIAL_JOB_STRUCT_VAL(x,y)
#endif

#endif 		/* #ifndef DOXYGEN_SHOULD_SKIP_THIS */




FIELD_TRIAL_JOB_PREFIX NamedParameterType FIELD_TRIAL_NAME FIELD_TRIAL_JOB_STRUCT_VAL("FT Name", PT_STRING);
FIELD_TRIAL_JOB_PREFIX NamedParameterType FIELD_TRIAL_TEAM FIELD_TRIAL_JOB_STRUCT_VAL("FT Team", PT_STRING);
FIELD_TRIAL_JOB_PREFIX NamedParameterType FIELD_TRIAL_ADD FIELD_TRIAL_JOB_STRUCT_VAL("FT Add", PT_BOOLEAN);


#ifdef __cplusplus
extern "C"
{
#endif

DFW_FIELD_TRIAL_SERVICE_LOCAL bool AddSubmissionFieldTrialParams (ServiceData *data_p, ParameterSet *param_set_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL bool RunForSubmissionFieldTrialParams (DFWFieldTrialServiceData *data_p, ParameterSet *param_set_p, ServiceJob *job_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL bool GetSubmissionFieldTrialParameterTypeForNamedParameter (const char *param_name_s, ParameterType *pt_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool AddSearchFieldTrialParams (ServiceData *data_p, ParameterSet *param_set_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL bool RunForSearchFieldTrialParams (DFWFieldTrialServiceData *data_p, ParameterSet *param_set_p, ServiceJob *job_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL bool GetSearchFieldTrialParameterTypeForNamedParameter (const char *param_name_s, ParameterType *pt_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool SetUpFieldTrialsListParameter (const DFWFieldTrialServiceData *data_p, Parameter *param_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL json_t *GetAllFieldTrialsAsJSON (const DFWFieldTrialServiceData *data_p, bson_t *opts_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool AddFieldTrialToServiceJobFromJSON (ServiceJob *job_p, json_t *trial_json_p, const ViewFormat format, DFWFieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool AddFieldTrialToServiceJob (ServiceJob *job_p, FieldTrial *trial_p, const ViewFormat format, DFWFieldTrialServiceData *data_p);


#ifdef __cplusplus
}
#endif

#endif /* SERVICES_DFW_FIELD_TRIAL_SERVICE_INCLUDE_FIELD_TRIAL_JOBS_H_ */

/*
** Copyright 2014-2020 The Earlham Institute
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
 * program_jobs.h
 *
 *  Created on: 13 Nov 2020
 *      Author: billy
 */

#ifndef SERVICES_FIELD_TRIALS_INCLUDE_PROGRAM_JOBS_H_
#define SERVICES_FIELD_TRIALS_INCLUDE_PROGRAM_JOBS_H_

#include "dfw_field_trial_service_data.h"
#include "dfw_field_trial_service_library.h"

#include "program.h"
#include "string_parameter.h"


#ifndef DOXYGEN_SHOULD_SKIP_THIS

#ifdef ALLOCATE_PROGRAM_JOB_CONSTANTS
	#define PROGRAM_JOB_PREFIX DFW_FIELD_TRIAL_SERVICE_API
	#define PROGRAM_JOB_STRUCT_VAL(x,y)	= { x, y}
#else
	#define PROGRAM_JOB_PREFIX extern
	#define PROGRAM_JOB_STRUCT_VAL(x,y)
#endif

#endif 		/* #ifndef DOXYGEN_SHOULD_SKIP_THIS */


/*
 * Study parameters
 */
PROGRAM_JOB_PREFIX NamedParameterType PROGRAM_ID PROGRAM_JOB_STRUCT_VAL("PR Id", PT_STRING);
PROGRAM_JOB_PREFIX NamedParameterType PROGRAM_ABBREVIATION PROGRAM_JOB_STRUCT_VAL("PR Abbreviation", PT_STRING);
PROGRAM_JOB_PREFIX NamedParameterType PROGRAM_CROP PROGRAM_JOB_STRUCT_VAL("PR Crop", PT_STRING);
PROGRAM_JOB_PREFIX NamedParameterType PROGRAM_URL PROGRAM_JOB_STRUCT_VAL("PR Url", PT_STRING);
PROGRAM_JOB_PREFIX NamedParameterType PROGRAM_NAME PROGRAM_JOB_STRUCT_VAL("PR Name", PT_STRING);
PROGRAM_JOB_PREFIX NamedParameterType PROGRAM_OBJECTIVE PROGRAM_JOB_STRUCT_VAL("PR Objective", PT_LARGE_STRING);
PROGRAM_JOB_PREFIX NamedParameterType PROGRAM_PI_NAME PROGRAM_JOB_STRUCT_VAL("PR PI Name", PT_STRING);



#ifdef __cplusplus
extern "C"
{
#endif


DFW_FIELD_TRIAL_SERVICE_LOCAL bool SetUpProgramsListParameter (const FieldTrialServiceData *data_p, StringParameter *param_p, const Program *active_program_p, const bool empty_option_flag);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool AddSubmissionProgramParams (ServiceData *data_p, ParameterSet *param_set_p, Resource *resource_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL json_t *GetAllProgramsAsJSON (const FieldTrialServiceData *data_p, bson_t *opts_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool GetSubmissionProgramParameterTypeForNamedParameter (const char *param_name_s, ParameterType *pt_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool RunForSubmissionProgramParams (FieldTrialServiceData *data_p, ParameterSet *param_set_p, ServiceJob *job_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool AddProgramToServiceJob (ServiceJob *job_p, Program *program_p, const ViewFormat format, FieldTrialServiceData *data_p);


#ifdef __cplusplus
}
#endif


#endif /* SERVICES_FIELD_TRIALS_INCLUDE_PROGRAM_JOBS_H_ */

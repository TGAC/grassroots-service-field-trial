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

#ifndef SERVICES_FIELD_TRIALS_INCLUDE_PROGRAMME_JOBS_H_
#define SERVICES_FIELD_TRIALS_INCLUDE_PROGRAMME_JOBS_H_

#include "dfw_field_trial_service_data.h"
#include "dfw_field_trial_service_library.h"

#include "programme.h"
#include "string_parameter.h"


#ifndef DOXYGEN_SHOULD_SKIP_THIS

#ifdef ALLOCATE_PROGRAMME_JOB_CONSTANTS
	#define PROGRAMME_JOB_PREFIX DFW_FIELD_TRIAL_SERVICE_API
	#define PROGRAMME_JOB_STRUCT_VAL(x,y)	= { x, y}
#else
	#define PROGRAMME_JOB_PREFIX extern
	#define PROGRAMME_JOB_STRUCT_VAL(x,y)
#endif

#endif 		/* #ifndef DOXYGEN_SHOULD_SKIP_THIS */


/*
 * Programme parameters
 */
PROGRAMME_JOB_PREFIX NamedParameterType PROGRAMME_ID PROGRAMME_JOB_STRUCT_VAL("PR Id", PT_STRING);
PROGRAMME_JOB_PREFIX NamedParameterType PROGRAMME_ABBREVIATION PROGRAMME_JOB_STRUCT_VAL("PR Abbreviation", PT_STRING);
PROGRAMME_JOB_PREFIX NamedParameterType PROGRAMME_CROP PROGRAMME_JOB_STRUCT_VAL("PR Crop", PT_STRING);
PROGRAMME_JOB_PREFIX NamedParameterType PROGRAMME_URL PROGRAMME_JOB_STRUCT_VAL("PR Url", PT_STRING);
PROGRAMME_JOB_PREFIX NamedParameterType PROGRAMME_NAME PROGRAMME_JOB_STRUCT_VAL("PR Name", PT_STRING);
PROGRAMME_JOB_PREFIX NamedParameterType PROGRAMME_OBJECTIVE PROGRAMME_JOB_STRUCT_VAL("PR Objective", PT_LARGE_STRING);
PROGRAMME_JOB_PREFIX NamedParameterType PROGRAMME_LOGO PROGRAMME_JOB_STRUCT_VAL("PR Logo", PT_STRING);
PROGRAMME_JOB_PREFIX NamedParameterType PROGRAMME_FUNDER PROGRAMME_JOB_STRUCT_VAL("PR Funder", PT_STRING);
PROGRAMME_JOB_PREFIX NamedParameterType PROGRAMME_USER PROGRAMME_JOB_STRUCT_VAL("PR User", PT_STRING);
PROGRAMME_JOB_PREFIX NamedParameterType PROGRAMME_CODE PROGRAMME_JOB_STRUCT_VAL("PR Code", PT_STRING);


PROGRAMME_JOB_PREFIX NamedParameterType PROGRAMME_PI_NAME PROGRAMME_JOB_STRUCT_VAL("PR PI Name", PT_STRING);
PROGRAMME_JOB_PREFIX NamedParameterType PROGRAMME_PI_EMAIL PROGRAMME_JOB_STRUCT_VAL("PR PI Email", PT_STRING);
PROGRAMME_JOB_PREFIX NamedParameterType PROGRAMME_PI_ROLE PROGRAMME_JOB_STRUCT_VAL("PR PI Role", PT_STRING);
PROGRAMME_JOB_PREFIX NamedParameterType PROGRAMME_PI_AFFILATION PROGRAMME_JOB_STRUCT_VAL("PR PI Affiliation", PT_STRING);
PROGRAMME_JOB_PREFIX NamedParameterType PROGRAMME_PI_ORCID PROGRAMME_JOB_STRUCT_VAL("PR PI Orcid", PT_STRING);


PROGRAMME_JOB_PREFIX NamedParameterType PROGRAMME_SEARCH PROGRAMME_JOB_STRUCT_VAL("PR Search", PT_STRING);


#ifdef __cplusplus
extern "C"
{
#endif


DFW_FIELD_TRIAL_SERVICE_LOCAL bool SetUpProgrammesListParameter (const FieldTrialServiceData *data_p, StringParameter *param_p, const Programme *active_program_p, const bool empty_option_flag);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool AddSubmissionProgrammeParams (ServiceData *data_p, ParameterSet *param_set_p, DataResource *resource_p, const bool read_only_flag);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool AddSearchProgrammeParams (ServiceData *data_p, ParameterSet *param_set_p, DataResource *resource_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL json_t *GetAllProgrammesAsJSON (const FieldTrialServiceData *data_p, const bool full_data_flag);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool GetSubmissionProgrammeParameterTypeForNamedParameter (const char *param_name_s, ParameterType *pt_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool GetSearchProgrammeParameterTypeForNamedParameter (const char *param_name_s, ParameterType *pt_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool RunForSubmissionProgrammeParams (FieldTrialServiceData *data_p, ParameterSet *param_set_p, ServiceJob *job_p, User *user_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool RunForSearchProgrammeParams (FieldTrialServiceData *data_p, ParameterSet *param_set_p, ServiceJob *job_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool AddProgrammeToServiceJob (ServiceJob *job_p, Programme *program_p, const ViewFormat format, FieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL Programme *GetProgrammeFromResource (DataResource *resource_p, const NamedParameterType program_param_type, FieldTrialServiceData *dfw_data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL json_t *GetProgrammeIndexingData (Service *service_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL json_t *GetProgrammeAsFrictionlessDataResource (const Programme *programme_p, const FieldTrialServiceData *data_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL json_t *GetProgrammeAsFrictionlessDataPackage (const Programme *programme_p, const FieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool SaveProgrammeAsFrictionlessData (const Programme *programme_p, const FieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool AddProgrammesListFromJSON (const char *id_s, json_t *programmes_json_p, ParameterSet *param_set_p, ParameterGroup *group_p, const bool read_only_flag, const char *empty_option_s, FieldTrialServiceData *ft_data_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL bool AddProgrammesList (const char *id_s, ParameterSet *param_set_p, ParameterGroup *group_p, const bool read_only_flag, const char * const empty_option_s, FieldTrialServiceData *data_p);



DFW_FIELD_TRIAL_SERVICE_LOCAL bool AddProgrammeEditor (Programme *programme_p, const char *id_s,
																											 ParameterSet *param_set_p, const bool read_only_flag, FieldTrialServiceData *dfw_data_p);



#ifdef __cplusplus
}
#endif


#endif /* SERVICES_FIELD_TRIALS_INCLUDE_PROGRAMME_JOBS_H_ */

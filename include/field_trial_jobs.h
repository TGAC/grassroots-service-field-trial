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

#ifndef SERVICES_FIELD_TRIALS_INCLUDE_FIELD_TRIAL_JOBS_H_
#define SERVICES_FIELD_TRIALS_INCLUDE_FIELD_TRIAL_JOBS_H_


#include "dfw_field_trial_service_data.h"
#include "dfw_field_trial_service_library.h"

#include "field_trial.h"
#include "json_processor.h"
#include "string_parameter.h"


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
FIELD_TRIAL_JOB_PREFIX NamedParameterType FIELD_TRIAL_ID FIELD_TRIAL_JOB_STRUCT_VAL("FT Id", PT_STRING);
FIELD_TRIAL_JOB_PREFIX NamedParameterType FIELD_TRIAL_PARENT_ID FIELD_TRIAL_JOB_STRUCT_VAL("FT Parent Id", PT_STRING);

FIELD_TRIAL_JOB_PREFIX NamedParameterType FIELD_TRIAL_ADD FIELD_TRIAL_JOB_STRUCT_VAL("FT Add", PT_BOOLEAN);

FIELD_TRIAL_JOB_PREFIX NamedParameterType FIELD_TRIAL_SEARCH FIELD_TRIAL_JOB_STRUCT_VAL("FT Search", PT_BOOLEAN);

#ifdef __cplusplus
extern "C"
{
#endif

DFW_FIELD_TRIAL_SERVICE_LOCAL bool AddSubmissionFieldTrialParams (ServiceData *data_p, ParameterSet *param_set_p, FieldTrial *active_trial_p, const bool read_only_flag);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool RunForSubmissionFieldTrialParams (FieldTrialServiceData *data_p, ParameterSet *param_set_p, ServiceJob *job_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL bool GetSubmissionFieldTrialParameterTypeForNamedParameter (const char *param_name_s, ParameterType *pt_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool AddSearchFieldTrialParams (ServiceData *data_p, ParameterSet *param_set_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL bool RunForSearchFieldTrialParams (FieldTrialServiceData *data_p, ParameterSet *param_set_p, ServiceJob *job_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL bool GetSearchFieldTrialParameterTypeForNamedParameter (const char *param_name_s, ParameterType *pt_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool SetUpFieldTrialsListParameter (const FieldTrialServiceData *data_p, StringParameter *param_p,  const char *active_trial_id_s, const bool empty_option_flag);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool SetUpFieldTrialsListParameterFromJSON (const FieldTrialServiceData *data_p, StringParameter *param_p,  const char *active_trial_id_s, const bool empty_option_flag, json_t *trials_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL json_t *GetFieldTrialIndexingData (Service *service_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL json_t *GetAllFieldTrialsAsJSON (const FieldTrialServiceData *data_p, const bool full_data_flag);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool AddFieldTrialToServiceJobFromJSON (ServiceJob *job_p, json_t *trial_json_p, const ViewFormat format, FieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool AddFieldTrialToServiceJob (ServiceJob *job_p, FieldTrial *trial_p, const ViewFormat format, FieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL json_t *GetFieldTrialJSONForId (const char *id_s, const ViewFormat format, JSONProcessor *processor_p, char **name_ss, FieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL FieldTrial *GetFieldTrialFromResource (DataResource *resource_p, const NamedParameterType trial_param_type, FieldTrialServiceData *dfw_data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL json_t *GetFieldTrialAsFrictionlessDataResource (const FieldTrial *trial_p, const FieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool SearchFieldTrials (ServiceJob *job_p, const char *name_s, const char *team_s, const bool regex_flag, const ViewFormat format, FieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool PopulaterActiveTrialValues (FieldTrial *active_trial_p, char **id_ss, char **programme_id_ss, const char **name_ss, const char **team_ss, LinkedList **existing_people_pp, ParameterSet *param_set_p, FieldTrialServiceData *dfw_data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool AddTrialsList (const char *id_s, ParameterSet *param_set_p, ParameterGroup *group_p, const bool read_only_flag, const char * const empty_option_s, FieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool AddTrialsListFromJSON (const char *id_s, json_t *trials_json_p, ParameterSet *param_set_p, ParameterGroup *group_p, const bool read_only_flag, const char *empty_option_s, FieldTrialServiceData *dfw_data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool AddTrialEditor (const char * const name_s, const char * const team_s, const char * const programme_id_s, 	LinkedList *existing_people_p, ParameterSet *param_set_p, ParameterGroup *group_p, const bool read_only_flag, FieldTrialServiceData *dfw_data_p);



#ifdef __cplusplus
}
#endif

#endif /* SERVICES_FIELD_TRIALS_INCLUDE_FIELD_TRIAL_JOBS_H_ */

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
 * experimental_area_jobs.h
 *
 *  Created on: 1 Oct 2018
 *      Author: billy
 */

#ifndef SERVICES_DFW_FIELD_TRIAL_SERVICE_INCLUDE_EXPERIMENTAL_AREA_JOBS_H_
#define SERVICES_DFW_FIELD_TRIAL_SERVICE_INCLUDE_EXPERIMENTAL_AREA_JOBS_H_


#include "dfw_field_trial_service_data.h"
#include "dfw_field_trial_service_library.h"

#include "study.h"


#ifndef DOXYGEN_SHOULD_SKIP_THIS

#ifdef ALLOCATE_STUDY_JOB_CONSTANTS
	#define STUDY_JOB_PREFIX DFW_FIELD_TRIAL_SERVICE_API
	#define STUDY_JOB_STRUCT_VAL(x,y)	= { x, y}
#else
	#define STUDY_JOB_PREFIX extern
	#define STUDY_JOB_STRUCT_VAL(x,y)
#endif

#endif 		/* #ifndef DOXYGEN_SHOULD_SKIP_THIS */

/*
 * Study parameters
 */
STUDY_JOB_PREFIX NamedParameterType STUDY_ID STUDY_JOB_STRUCT_VAL("ST Id", PT_STRING);

STUDY_JOB_PREFIX NamedParameterType STUDY_NAME STUDY_JOB_STRUCT_VAL("ST Name", PT_STRING);
STUDY_JOB_PREFIX NamedParameterType STUDY_SOIL STUDY_JOB_STRUCT_VAL("ST Soil", PT_STRING);

STUDY_JOB_PREFIX NamedParameterType STUDY_LINK STUDY_JOB_STRUCT_VAL("ST Link", PT_STRING);
STUDY_JOB_PREFIX NamedParameterType STUDY_SOWING_YEAR STUDY_JOB_STRUCT_VAL("ST Sowing Year", PT_TIME);
STUDY_JOB_PREFIX NamedParameterType STUDY_HARVEST_YEAR STUDY_JOB_STRUCT_VAL("ST Harvest Year", PT_TIME);

STUDY_JOB_PREFIX NamedParameterType STUDY_ASPECT STUDY_JOB_STRUCT_VAL("ST Field Aspect", PT_STRING);
STUDY_JOB_PREFIX NamedParameterType STUDY_SLOPE STUDY_JOB_STRUCT_VAL("ST Slope", PT_STRING);

STUDY_JOB_PREFIX NamedParameterType STUDY_GET_ALL_PLOTS STUDY_JOB_STRUCT_VAL("Get all Plots for Study", PT_BOOLEAN);


STUDY_JOB_PREFIX NamedParameterType STUDY_ADD_STUDY STUDY_JOB_STRUCT_VAL("Add Study", PT_BOOLEAN);

STUDY_JOB_PREFIX NamedParameterType STUDY_THIS_CROP STUDY_JOB_STRUCT_VAL("This Crop", PT_STRING);
STUDY_JOB_PREFIX NamedParameterType STUDY_PREVIOUS_CROP STUDY_JOB_STRUCT_VAL("Previous Crop", PT_STRING);


STUDY_JOB_PREFIX NamedParameterType STUDY_MIN_PH STUDY_JOB_STRUCT_VAL("pH Minimum", PT_UNSIGNED_REAL);
STUDY_JOB_PREFIX NamedParameterType STUDY_MAX_PH STUDY_JOB_STRUCT_VAL("pH Maximum", PT_UNSIGNED_REAL);

STUDY_JOB_PREFIX NamedParameterType STUDY_FIELD_TRIALS_LIST STUDY_JOB_STRUCT_VAL("Field Trials", PT_STRING);
STUDY_JOB_PREFIX NamedParameterType STUDY_LOCATIONS_LIST STUDY_JOB_STRUCT_VAL("Locations", PT_STRING);




#ifdef __cplusplus
extern "C"
{
#endif



DFW_FIELD_TRIAL_SERVICE_LOCAL bool AddSubmissionStudyParams (ServiceData *data_p, ParameterSet *param_set_p, Resource *resource_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool RunForSubmissionStudyParams (DFWFieldTrialServiceData *data_p, ParameterSet *param_set_p, ServiceJob *job_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool GetSubmissionStudyParameterTypeForNamedParameter (const char *param_name_s, ParameterType *pt_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool AddSearchStudyParams (ServiceData *data_p, ParameterSet *param_set_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool RunForSearchStudyParams (DFWFieldTrialServiceData *data_p, ParameterSet *param_set_p, ServiceJob *job_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool GetSearchStudyParameterTypeForNamedParameter (const char *param_name_s, ParameterType *pt_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL json_t *GetAllStudiesAsJSON (const DFWFieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool SetUpStudiesListParameter (const DFWFieldTrialServiceData *data_p, Parameter *param_p, const char *empty_option_s);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool AddStudyToServiceJob (ServiceJob *job_p, Study *study_p, const ViewFormat format, DFWFieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL char *GetStudyAsString (const Study *study_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL const KeyValuePair *GetAspect (const char *aspect_value_s);


#ifdef __cplusplus
}
#endif


#endif /* SERVICES_DFW_FIELD_TRIAL_SERVICE_INCLUDE_EXPERIMENTAL_AREA_JOBS_H_ */

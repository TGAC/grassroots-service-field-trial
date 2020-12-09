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
 * treatment_factor_jobs.h
 *
 *  Created on: 7 Dec 2020
 *      Author: billy
 */

#ifndef SERVICES_FIELD_TRIALS_INCLUDE_TREATMENT_FACTOR_JOBS_H_
#define SERVICES_FIELD_TRIALS_INCLUDE_TREATMENT_FACTOR_JOBS_H_


#include "dfw_field_trial_service_data.h"
#include "dfw_field_trial_service_library.h"

#include "treatment_factor.h"

#include "string_parameter.h"

#ifdef __cplusplus
extern "C"
{
#endif


DFW_FIELD_TRIAL_SERVICE_LOCAL bool AddSubmissionTreatmentFactorParams (ServiceData *data_p, ParameterSet *param_set_p, Resource *resource_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool SetUpTreatmentFactorsListParameter (const FieldTrialServiceData *data_p, StringParameter *param_p, const struct Study *active_study_p, const TreatmentFactor *active_tf_p, const bool empty_option_flag);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool GetSubmissionTreatmentFactorParameterTypeForNamedParameter (const char *param_name_s, ParameterType *pt_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool RunForSubmissionTreatmentFactorParams (FieldTrialServiceData *data_p, ParameterSet *param_set_p, ServiceJob *job_p);


#ifdef __cplusplus
}
#endif



#endif /* SERVICES_FIELD_TRIALS_INCLUDE_TREATMENT_FACTOR_JOBS_H_ */

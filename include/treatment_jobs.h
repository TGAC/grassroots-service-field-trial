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
 * phenotype_jobs.h
 *
 *  Created on: 26 Oct 2018
 *      Author: billy
 */

#ifndef SERVICES_DFW_FIELD_TRIAL_SERVICE_INCLUDE_TREATMENT_JOBS_H_
#define SERVICES_DFW_FIELD_TRIAL_SERVICE_INCLUDE_TREATMENT_JOBS_H_


#include "dfw_field_trial_service_data.h"
#include "dfw_field_trial_service_library.h"

#include "treatment.h"


#ifdef __cplusplus
extern "C"
{
#endif


DFW_FIELD_TRIAL_SERVICE_LOCAL bool AddSubmissionTreatmentParams (ServiceData *data_p, ParameterSet *param_set_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool RunForSubmissionTreatmentParams (DFWFieldTrialServiceData *data_p, ParameterSet *param_set_p, ServiceJob *job_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool GetSubmissionTreatmentParameterTypeForNamedParameter (const char *param_name_s, ParameterType *pt_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL Treatment *GetTreatmentByInternalName (const char *name_s, const DFWFieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL json_t *GetAllTreatmentsAsJSON (const DFWFieldTrialServiceData *data_p, bson_t *opts_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL char *GetTreatmentAsString (const Treatment *treatment_p);


#ifdef __cplusplus
}
#endif



#endif /* SERVICES_DFW_FIELD_TRIAL_SERVICE_INCLUDE_TREATMENT_JOBS_H_ */

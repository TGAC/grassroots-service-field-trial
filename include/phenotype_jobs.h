/*
 * phenotype_jobs.h
 *
 *  Created on: 6 Nov 2019
 *      Author: billy
 */

#ifndef SERVICES_FIELD_TRIALS_INCLUDE_PHENOTYPE_JOBS_H_
#define SERVICES_FIELD_TRIALS_INCLUDE_PHENOTYPE_JOBS_H_



#include "dfw_field_trial_service_data.h"
#include "dfw_field_trial_service_library.h"



#ifdef __cplusplus
extern "C"
{
#endif



DFW_FIELD_TRIAL_SERVICE_LOCAL bool AddSubmissionPhenotypeParams (ServiceData *data_p, ParameterSet *param_set_p, Resource *resource_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool RunForSubmissionPhenotypesParams (DFWFieldTrialServiceData *data_p, ParameterSet *param_set_p, ServiceJob *job_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool GetSubmissionPhenotypesParameterTypeForNamedParameter (const char *param_name_s, ParameterType *pt_p);



#ifdef __cplusplus
}
#endif


#endif /* SERVICES_FIELD_TRIALS_INCLUDE_PHENOTYPE_JOBS_H_ */

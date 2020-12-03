/*
 * submit_treatment.h
 *
 *  Created on: 3 Dec 2020
 *      Author: billy
 */

#ifndef SERVICES_FIELD_TRIALS_INCLUDE_SUBMIT_TREATMENT_H_
#define SERVICES_FIELD_TRIALS_INCLUDE_SUBMIT_TREATMENT_H_

#include "dfw_field_trial_service_data.h"
#include "dfw_field_trial_service_library.h"



#ifdef __cplusplus
extern "C"
{
#endif


DFW_FIELD_TRIAL_SERVICE_LOCAL Service *GetTreatmentSubmissionService (GrassrootsServer *grassroots_p);



#ifdef __cplusplus
}
#endif

#endif /* SERVICES_FIELD_TRIALS_INCLUDE_SUBMIT_TREATMENT_H_ */

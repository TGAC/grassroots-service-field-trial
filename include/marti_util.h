/*
 * marti_util.h
 *
 *  Created on: 3 Oct 2024
 *      Author: billy
 */

#ifndef SERVICES_FIELD_TRIALS_INCLUDE_MARTI_UTIL_H_
#define SERVICES_FIELD_TRIALS_INCLUDE_MARTI_UTIL_H_

#include "dfw_field_trial_service_data.h"
#include "dfw_field_trial_service_library.h"
#include "study.h"



#ifdef __cplusplus
extern "C"
{
#endif


DFW_FIELD_TRIAL_SERVICE_LOCAL OperationStatus AddMartiResults (Study *study_p, json_t *study_json_p, const ViewFormat format, const FieldTrialServiceData *data_p);




#ifdef __cplusplus
}
#endif


#endif /* SERVICES_FIELD_TRIALS_INCLUDE_MARTI_UTIL_H_ */

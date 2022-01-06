/*
 * handbook_generator.h
 *
 *  Created on: 5 Jan 2022
 *      Author: billy
 */

#ifndef SERVICES_FIELD_TRIALS_INCLUDE_HANDBOOK_GENERATOR_H_
#define SERVICES_FIELD_TRIALS_INCLUDE_HANDBOOK_GENERATOR_H_

#include "dfw_field_trial_service_library.h"
#include "dfw_field_trial_service_data.h"

#include "study.h"
#include "operation.h"

#ifdef __cplusplus
extern "C"
{
#endif


DFW_FIELD_TRIAL_SERVICE_LOCAL OperationStatus GenerateStudyAsPDF (const Study *study_p, FieldTrialServiceData *data_p);


#ifdef __cplusplus
}
#endif

#endif /* SERVICES_FIELD_TRIALS_INCLUDE_HANDBOOK_GENERATOR_H_ */

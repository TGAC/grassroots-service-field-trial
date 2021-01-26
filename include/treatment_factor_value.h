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
 * treatment_factor_value.h
 *
 *  Created on: 25 Jan 2021
 *      Author: billy
 */

#ifndef SERVICES_FIELD_TRIALS_INCLUDE_TREATMENT_FACTOR_VALUE_H_
#define SERVICES_FIELD_TRIALS_INCLUDE_TREATMENT_FACTOR_VALUE_H_

#include "dfw_field_trial_service_library.h"
#include "dfw_field_trial_service_data.h"

#include "treatment_factor.h"


typedef struct TreatmentFactorValue
{
	TreatmentFactor *tfv_factor_p;

	char *tfv_label_s;

} TreatmentFactorValue;


typedef struct TreatmentFactorValueNode
{
	ListItem tfvn_node;

	TreatmentFactorValue *tfvn_value_p;
} TreatmentFactorValueNode;


#ifdef __cplusplus
extern "C"
{
#endif


DFW_FIELD_TRIAL_SERVICE_LOCAL TreatmentFactorValue *AllocateTreatmentFactorValue (TreatmentFactor *treatment_factor_p, const char *label_s);

DFW_FIELD_TRIAL_SERVICE_LOCAL void FreeTreatmentFactorValue (TreatmentFactorValue *treatment_factor_value_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL TreatmentFactorValueNode *AllocateTreatmentFactorValueNode (TreatmentFactorValue *treatment_factor_value_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL void FreeTreatmentFactorValueNode (ListItem *treatment_factor_value_node_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL const char *GetTreatmentFactorLabelValue (const TreatmentFactorValue *treatment_factor_value_p);


#ifdef _cplusplus
}
#endif



#endif /* SERVICES_FIELD_TRIALS_INCLUDE_TREATMENT_FACTOR_VALUE_H_ */

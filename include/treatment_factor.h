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
 * treatment_factor..h
 *
 *  Created on: 4 Dec 2020
 *      Author: billy
 */

#ifndef SERVICES_FIELD_TRIALS_INCLUDE_TREATMENT_FACTOR_H_
#define SERVICES_FIELD_TRIALS_INCLUDE_TREATMENT_FACTOR_H_


#include "dfw_field_trial_service_library.h"
#include "dfw_field_trial_service_data.h"

#include "treatment.h"
#include "linked_list.h"


/* forward declarations */
struct Study;


/**
 * A TreatmentFactor is a set of values for a
 * Treatment applied to plots within a Study.
 */
typedef struct TreatmentFactor
{
	/** The Treatment that is applied. */
	Treatment *tf_treatment_p;

	/** The Study that this TreatmentFactor is applied in. */
	struct Study *tf_study_p;

	/**
	 * A list of KeyValuePairNodes where the keys are the labels
	 * for the different levels of the applied Treatment and the
	 * values are the descriptions of what has been applied. This
	 * could be a numeric range or a textual description.
	 */
	LinkedList *tf_values_p;

} TreatmentFactor;


typedef struct TreatmentFactorNode
{
	ListItem tfn_node;

	TreatmentFactor *tfn_p;
} TreatmentFactorNode;



#ifdef __cplusplus
extern "C"
{
#endif


DFW_FIELD_TRIAL_SERVICE_LOCAL TreatmentFactor *AllocateTreatmentFactor (Treatment *treatment_factor_p, struct Study *study_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL void FreeTreatmentFactor (TreatmentFactor *treatment_factor_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL bool AddTreatmentFactorValue (TreatmentFactor *treatment_p, const char *name_s, const char *value_s);

DFW_FIELD_TRIAL_SERVICE_LOCAL TreatmentFactorNode *AllocateTreatmentFactorNode (TreatmentFactor *treatment_factor_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL void FreeTreatmentFactorNode (ListItem *treatment_node_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL size_t *GetNumberOfTreatmentFactorValues (const TreatmentFactor *treatment_factor_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL json_t *GetTreatmentFactorAsJSON (const TreatmentFactor *treatment_factor_p, const ViewFormat format);

DFW_FIELD_TRIAL_SERVICE_LOCAL TreatmentFactor *GetTreatmentFactorFromJSON (const json_t *treatment_factor_json_p, struct Study *parent_study_p, const FieldTrialServiceData *data_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL const char *GetTreatmentFactorValue (TreatmentFactor *treatment_p, const char *name_s);

DFW_FIELD_TRIAL_SERVICE_LOCAL const char *GetTreatmentFactorName (const TreatmentFactor *treatment_factor_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL const char *GetTreatmentFactorUrl (const TreatmentFactor *treatment_factor_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL json_t *GetTreatmentFactorValuesAsJSON (const TreatmentFactor *treatment_factor_p);

#ifdef _cplusplus
}
#endif


#endif /* SERVICES_FIELD_TRIALS_INCLUDE_TREATMENT_FACTOR_H_ */

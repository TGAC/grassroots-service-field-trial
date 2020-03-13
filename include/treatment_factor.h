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
 * study_treatment.h
 *
 *  Created on: 5 Mar 2020
 *      Author: billy
 */

#ifndef SERVICES_FIELD_TRIALS_INCLUDE_STUDY_TREATMENT_H_
#define SERVICES_FIELD_TRIALS_INCLUDE_STUDY_TREATMENT_H_


#include "dfw_field_trial_service_library.h"
#include "dfw_field_trial_service_data.h"

#include "key_value_pair.h"
#include "schema_keys.h"
#include "jansson.h"


/**
 * A treatmnet applied to the plots within a study.
 */
typedef struct TreatmentFactor
{
	//Study *parent_study_p;

	SchemaTerm *tf_ontology_term_p;

	char *tf_notes_s;

	char *tf_abbreviation_s;



	/**
	 * A list of KeyValuePairNodes defining the different treatment levels
	 *
	 */
	LinkedList *tf_values_p;
} TreatmentFactor;



typedef struct TreatmentFactorNode
{
	ListItem tfn_node;

	TreatmentFactor *tfn_treatment_p;
} TreatmentFactorNode;


#ifndef DOXYGEN_SHOULD_SKIP_THIS

#ifdef ALLOCATE_TREATMENT_FACTOR_TAGS
	#define TREATMENT_FACTOR_PREFIX DFW_FIELD_TRIAL_SERVICE_LOCAL
	#define TREATMENT_FACTOR_VAL(x)	= x
	#define TREATMENT_FACTOR_CONCAT_VAL(x,y)	= x y
#else
	#define TREATMENT_FACTOR_PREFIX extern
	#define TREATMENT_FACTOR_VAL(x)
	#define TREATMENT_FACTOR_CONCAT_VAL(x,y)
#endif

#endif 		/* #ifndef DOXYGEN_SHOULD_SKIP_THIS */


TREATMENT_FACTOR_PREFIX const char *TF_NAME_S TREATMENT_FACTOR_CONCAT_VAL (CONTEXT_PREFIX_SCHEMA_ORG_S, "name");
TREATMENT_FACTOR_PREFIX const char *TF_VALUES_S TREATMENT_FACTOR_VAL ("values");
TREATMENT_FACTOR_PREFIX const char *TF_TERM_S TREATMENT_FACTOR_VAL ("term");




#ifdef __cplusplus
extern "C"
{
#endif


DFW_FIELD_TRIAL_SERVICE_LOCAL TreatmentFactor *AllocateTreatmentFactor (SchemaTerm *term_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL void FreeTreatmentFactor (TreatmentFactor *treatment_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL bool AddTreatmentFactorValueByParts (TreatmentFactor *treatment_p, const char *name_s, const char *value_s);

DFW_FIELD_TRIAL_SERVICE_LOCAL bool AddTreatmentFactorValue (TreatmentFactor *treatment_p, KeyValuePair *pair_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL TreatmentFactorNode *AllocateTreatmentFactorNode (TreatmentFactor *treatment_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL void FreeTreatmentFactorNode (ListItem *treatment_node_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL json_t *GetTreatmentFactorAsJSON (const TreatmentFactor *treatmnent_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL TreatmentFactor *GetTreatmentFactorFromJSON (const json_t *treatmnent_json_p);

#ifdef __cplusplus
}
#endif


#endif /* SERVICES_FIELD_TRIALS_INCLUDE_STUDY_TREATMENT_H_ */

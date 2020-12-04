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

typedef struct Treatment Treatment;

/**
 * A treatment applied to the plots within a study.
 */
struct Treatment
{
	bson_oid_t *tr_id_p;

	SchemaTerm *tr_ontology_term_p;

	char **tr_parent_names_ss;

	char **tr_synonyms_ss;
};




///**
// * A treatment applied to the plots within a study.
// */
//typedef struct TreatmentLevels
//{
//	bson_oid_t *tfl_treatment_id_p;
//
//	Study *tfl_parent_study_p;
//
//
//	/**
//	 * A list of KeyValuePairNodes defining the different treatment levels
//	 *
//	 */
//	LinkedList *tlf_values_p;
//} TreatmentLevels;



//typedef struct TreatmentLevelsNode
//{
//	ListItem tfln_node;
//
//	TreatmentLevels *tfln_p;
//} TreatmentLevelsNode;



#ifndef DOXYGEN_SHOULD_SKIP_THIS

#ifdef ALLOCATE_TREATMENT_TAGS
	#define TREATMENT_PREFIX DFW_FIELD_TRIAL_SERVICE_LOCAL
	#define TREATMENT_VAL(x)	= x
	#define TREATMENT_CONCAT_VAL(x,y)	= x y
#else
	#define TREATMENT_PREFIX extern
	#define TREATMENT_VAL(x)
	#define TREATMENT_CONCAT_VAL(x,y)
#endif

#endif 		/* #ifndef DOXYGEN_SHOULD_SKIP_THIS */


TREATMENT_PREFIX const char *TR_NAME_S TREATMENT_CONCAT_VAL (CONTEXT_PREFIX_SCHEMA_ORG_S, "name");
TREATMENT_PREFIX const char *TR_TERM_S TREATMENT_VAL ("term");
TREATMENT_PREFIX const char *TR_PARENTS_S TREATMENT_VAL ("parents");
TREATMENT_PREFIX const char *TR_SYNONYMS_S TREATMENT_VAL ("synonyms");



#ifdef __cplusplus
extern "C"
{
#endif


DFW_FIELD_TRIAL_SERVICE_LOCAL Treatment *AllocateTreatment (SchemaTerm *term_p, char **parent_names_ss, const bool copy_parents_flag, char **synonyms_ss, const bool copy_synonyms_flag, bson_oid_t *id_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL void FreeTreatment (Treatment *treatment_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL bool AddTreatmentValueByParts (Treatment *treatment_p, const char *name_s, const char *value_s);

DFW_FIELD_TRIAL_SERVICE_LOCAL bool AddTreatmentValue (Treatment *treatment_p, KeyValuePair *pair_p);

//DFW_FIELD_TRIAL_SERVICE_LOCAL TreatmentNode *AllocateTreatmentNode (Treatment *treatment_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL void FreeTreatmentNode (ListItem *treatment_node_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL json_t *GetTreatmentAsJSON (const Treatment *treatmnent_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL Treatment *GetTreatmentFromJSON (const json_t *treatmnent_json_p);

#ifdef _cplusplus
}
#endif


#endif /* SERVICES_FIELD_TRIALS_INCLUDE_STUDY_TREATMENT_H_ */

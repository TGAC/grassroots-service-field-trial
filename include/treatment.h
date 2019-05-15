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
 * phenotype.h
 *
 *  Created on: 11 Sep 2018
 *      Author: billy
 */

#ifndef SERVICES_DFW_FIELD_TRIAL_SERVICE_INCLUDE_TREATMENT_H_
#define SERVICES_DFW_FIELD_TRIAL_SERVICE_INCLUDE_TREATMENT_H_

#include <time.h>

#include "dfw_field_trial_service_data.h"
#include "dfw_field_trial_service_library.h"
#include "instrument.h"
#include "typedefs.h"
#include "jansson.h"
#include "schema_term.h"




typedef struct Treatment
{
	bson_oid_t *tr_id_p;

	SchemaTerm *tr_trait_term_p;

	SchemaTerm *tr_measurement_term_p;

	SchemaTerm *tr_unit_term_p;

	SchemaTerm *tr_form_term_p;

	char *tr_internal_name_s;

} Treatment;


#ifndef DOXYGEN_SHOULD_SKIP_THIS

#ifdef ALLOCATE_PHENOTYPE_TAGS
	#define TREATMENT_PREFIX DFW_FIELD_TRIAL_SERVICE_LOCAL
	#define TREATMENT_VAL(x)	= x
	#define TREATMENT_CONCAT_VAL(x,y)	= x y
#else
	#define TREATMENT_PREFIX extern
	#define TREATMENT_VAL(x)
	#define TREATMENTE_CONCAT_VAL(x,y)
#endif

#endif 		/* #ifndef DOXYGEN_SHOULD_SKIP_THIS */


TREATMENT_PREFIX const char *TR_TRAIT_S TREATMENT_VAL ("trait");

TREATMENT_PREFIX const char *TR_MEASUREMENT_S TREATMENT_VAL ("measurement");

TREATMENT_PREFIX const char *TR_UNIT_S TREATMENT_VAL ("unit");

TREATMENT_PREFIX const char *TR_FORM_S TREATMENT_VAL ("form");

TREATMENT_PREFIX const char *TR_VALUE_S TREATMENT_VAL ("value");

TREATMENT_PREFIX const char *TR_INTERNAL_NAME_S TREATMENT_VAL ("internal_name");



#ifdef __cplusplus
extern "C"
{
#endif



DFW_FIELD_TRIAL_SERVICE_LOCAL Treatment *AllocateTreatment (bson_oid_t *id_p, SchemaTerm *trait_p, SchemaTerm *measurement_p, SchemaTerm *unit_p, SchemaTerm *form_p, const char *internal_name_s);

DFW_FIELD_TRIAL_SERVICE_LOCAL void FreeTreatment (Treatment *treatment_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL json_t *GetTreatmentAsJSON (const Treatment *treatment_p, const ViewFormat format);

DFW_FIELD_TRIAL_SERVICE_LOCAL Treatment *GetTreatmentFromJSON (const json_t *phenotype_json_p, const DFWFieldTrialServiceData *data_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL bool SaveTreatment (Treatment *treatment_p, const DFWFieldTrialServiceData *data_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL Treatment *GetTreatmentById (const bson_oid_t *id_p, const DFWFieldTrialServiceData *data_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL Treatment *GetTreatmentByIdString (const char *id_s, const DFWFieldTrialServiceData *data_p);



#ifdef __cplusplus
}
#endif



#endif /* SERVICES_DFW_FIELD_TRIAL_SERVICE_INCLUDE_TREATMENT_H_ */

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
 * person.h
 *
 *  Created on: 19 Dec 2018
 *      Author: billy
 */

#ifndef SERVICES_FIELD_TRIALS_INCLUDE_PERSON_H_
#define SERVICES_FIELD_TRIALS_INCLUDE_PERSON_H_

#include "dfw_field_trial_service_data.h"
#include "dfw_field_trial_service_library.h"
#include "jansson.h"

#include "address.h"
#include "typedefs.h"


#ifndef DOXYGEN_SHOULD_SKIP_THIS

#ifdef ALLOCATE_STUDY_TAGS
	#define PERSON_PREFIX DFW_FIELD_TRIAL_SERVICE_LOCAL
	#define PERSON_VAL(x)	= x
	#define PERSON_CONCAT_VAL(x,y)	= x y
#else
	#define PERSON_PREFIX extern
	#define PERSON_VAL(x)
	#define PERSON_CONCAT_VAL(x,y)
#endif

#endif 		/* #ifndef DOXYGEN_SHOULD_SKIP_THIS */



typedef struct Person
{
	char *pe_name_s;

	char *pe_email_s;

} Person;


PERSON_PREFIX const char *PE_NAME_S PERSON_CONCAT_VAL (CONTEXT_PREFIX_SCHEMA_ORG_S, "name");

PERSON_PREFIX const char *PE_EMAIL_S PERSON_CONCAT_VAL (CONTEXT_PREFIX_SCHEMA_ORG_S, "email");


#ifdef __cplusplus
extern "C"
{
#endif


DFW_FIELD_TRIAL_SERVICE_LOCAL Person *AllocatePerson (const char *name_s, const char *email_s);


DFW_FIELD_TRIAL_SERVICE_LOCAL void FreePerson (Person *person_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL json_t *GetPersonAsJSON (const Person * const person_p, const ViewFormat format, const FieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL Person *GetPersonFromJSON (const json_t *parent_p, const ViewFormat format, const FieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool AddPersonToCompoundJSON (const Person *person_p, json_t *parent_json_p, const char * const key_s,  const ViewFormat format, const FieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL Person *GetPersonFromCompoundJSON (const json_t *json_p, const char * const key_s, const ViewFormat format, const FieldTrialServiceData *data_p);


#ifdef __cplusplus
}
#endif


#endif /* SERVICES_FIELD_TRIALS_INCLUDE_PERSON_H_ */

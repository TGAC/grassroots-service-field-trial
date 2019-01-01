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

#ifndef SERVICES_DFW_FIELD_TRIAL_SERVICE_INCLUDE_PERSON_H_
#define SERVICES_DFW_FIELD_TRIAL_SERVICE_INCLUDE_PERSON_H_

#include "dfw_field_trial_service_data.h"
#include "dfw_field_trial_service_library.h"
#include "jansson.h"

#include "typedefs.h"

/*
 *    "description": "Example Person",
        "emailAddress": "bob@bob.com",
        "firstName": "Bob",
        "lastName": "Robertson",
        "mailingAddress": "123 Road Street, City, State, Country, 98765",
        "middleName": "Danger",
        "personDbId": "person1",
        "phoneNumber": "+19876543210",
        "userID": "bdr45"
 */

typedef struct Person
{
	bson_oid_t *pe_id_p;

	char *pe_first_name_s;

	char *pe_middle_name_s;

	char *pe_last_name_s;

	char *pe_email_s;

	char *pe_description_s;

	char *pe_phone_s;


} Person;



#endif /* SERVICES_DFW_FIELD_TRIAL_SERVICE_INCLUDE_PERSON_H_ */

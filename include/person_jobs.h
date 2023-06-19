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

#ifndef SERVICES_FIELD_TRIALS_INCLUDE_PERSON_JOBS_H_
#define SERVICES_FIELD_TRIALS_INCLUDE_PERSON_JOBS_H_

#include "dfw_field_trial_service_data.h"
#include "dfw_field_trial_service_library.h"
#include "jansson.h"

#include "person.h"
#include "parameter_set.h"
#include "parameter_group.h"
#include "linked_list.h"



#ifdef ALLOCATE_PERSON_JOB_TAGS
	#define PERSON_JOB_PREFIX DFW_FIELD_TRIAL_SERVICE_LOCAL
	#define PERSON_JOB_VAL(x)	= x
	#define PERSON_JOB_STRUCT_VAL(x,y)	= {x, y}
#else
	#define PERSON_JOB_PREFIX extern
	#define PERSON_JOB_VAL(x)
	#define PERSON_JOB_STRUCT_VAL(x,y)
#endif


PERSON_JOB_PREFIX NamedParameterType PERSON_NAME PERSON_JOB_STRUCT_VAL("PE Name", PT_STRING);
PERSON_JOB_PREFIX NamedParameterType PERSON_EMAIL PERSON_JOB_STRUCT_VAL("PE Email", PT_STRING);
PERSON_JOB_PREFIX NamedParameterType PERSON_ROLE PERSON_JOB_STRUCT_VAL("PE Role", PT_STRING);
PERSON_JOB_PREFIX NamedParameterType PERSON_AFFILATION PERSON_JOB_STRUCT_VAL("PE Affiliation", PT_STRING);
PERSON_JOB_PREFIX NamedParameterType PERSON_ORCID PERSON_JOB_STRUCT_VAL("PE Orcid", PT_STRING);


#ifdef __cplusplus
extern "C"
{
#endif


bool AddMultiplePeopleParameters (ParameterSet *param_set_p, const char *group_s, LinkedList *existing_people_p, FieldTrialServiceData *data_p);


#ifdef __cplusplus
}
#endif


#endif /* SERVICES_FIELD_TRIALS_INCLUDE_PERSON_JOBS_H_ */

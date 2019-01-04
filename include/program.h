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
 * program.h
 *
 *  Created on: 19 Dec 2018
 *      Author: billy
 */

#ifndef SERVICES_DFW_FIELD_TRIAL_SERVICE_INCLUDE_PROGRAM_H_
#define SERVICES_DFW_FIELD_TRIAL_SERVICE_INCLUDE_PROGRAM_H_


#include <time.h>

#include "dfw_field_trial_service_data.h"
#include "dfw_field_trial_service_library.h"
#include "field_trial.h"
#include "location.h"
#include "jansson.h"

#include "typedefs.h"
#include "person.h"

/*
 *
 *         "abbreviation": "P1",
        "commonCropName": "Tomatillo",
        "documentationURL": "https://brapi.org",
        "leadPerson": "Name Nameson",
        "leadPersonDbId": "person1",
        "leadPersonName": "Name Nameson",
        "name": "Program 1",
        "objective": "Global Population Improvement",
        "programDbId": "1",
        "programName": "Program 1"
 */

typedef struct Program
{
	bson_oid_t *pr_id_p;

	char *pr_common_crop_name_s;

	char *pr_documentation_url_s;

	char *pr_abbreviation_s;

	char *pr_name_s;

	char *pr_objective_s;

	Person *pr_lead_person_p;

	/**
	 * A LinkedList of FieldTrialNodes
	 * for all of the FieldTrials in this
	 * Program.
	 */
	LinkedList *ea_trials_p;

} Program;

#endif /* SERVICES_DFW_FIELD_TRIAL_SERVICE_INCLUDE_PROGRAM_H_ */

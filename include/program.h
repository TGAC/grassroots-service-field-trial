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

#ifndef SERVICES_FIELD_TRIALS_INCLUDE_PROGRAM_H_
#define SERVICES_FIELD_TRIALS_INCLUDE_PROGRAM_H_


#include <time.h>

#include "dfw_field_trial_service_data.h"
#include "dfw_field_trial_service_library.h"
#include "field_trial.h"
#include "location.h"
#include "jansson.h"

#include "typedefs.h"
#include "person.h"



typedef struct Programme
{
	bson_oid_t *pr_id_p;

	char *pr_abbreviation_s;

	Crop *pr_crop_p;

	char *pr_documentation_url_s;

	char *pr_name_s;

	char *pr_objective_s;

	/**
	 * The name of the program leader
	 */
	char *pr_pi_name_s;

	/**
	 * A LinkedList of FieldTrialNodes
	 * for all of the FieldTrials in this
	 * Program.
	 */
	LinkedList *pr_trials_p;

} Programme;


typedef struct ProgrammeNode
{
	ListItem pn_node;
	Programme *pn_programme_p;
} ProgrammeNode;


#ifndef DOXYGEN_SHOULD_SKIP_THIS

#ifdef ALLOCATE_PROGRAM_TAGS
	#define PROGRAMME_PREFIX DFW_FIELD_TRIAL_SERVICE_LOCAL
	#define PROGRAMME_VAL(x)	= x
	#define PROGRAMME_CONCAT_VAL(x,y)	= x y
#else
	#define PROGRAMME_PREFIX extern
	#define PROGRAMME_VAL(x)
	#define PROGRAMME_CONCAT_VAL(x,y)
#endif

#endif 		/* #ifndef DOXYGEN_SHOULD_SKIP_THIS */



PROGRAMME_PREFIX const char *PR_NAME_S PROGRAMME_CONCAT_VAL (CONTEXT_PREFIX_SCHEMA_ORG_S, "name");

PROGRAMME_PREFIX const char *PR_OBJECTIVE_S PROGRAMME_CONCAT_VAL (CONTEXT_PREFIX_SCHEMA_ORG_S, "description");

PROGRAMME_PREFIX const char *PR_ID_S PROGRAMME_VAL ("_id");

PROGRAMME_PREFIX const char *PR_TRIALS_S PROGRAMME_VAL ("trials");

PROGRAMME_PREFIX const char *PR_DOCUMENTATION_URL_S PROGRAMME_CONCAT_VAL (CONTEXT_PREFIX_SCHEMA_ORG_S, "url");

PROGRAMME_PREFIX const char *PR_PI_NAME_S PROGRAMME_VAL ("principal_investigator");

PROGRAMME_PREFIX const char *PR_CROP_S PROGRAMME_VAL ("crop");

PROGRAMME_PREFIX const char *PR_ABBREVIATION_S PROGRAMME_CONCAT_VAL (CONTEXT_PREFIX_SCHEMA_ORG_S, "alternateName");



#ifdef __cplusplus
extern "C"
{
#endif


DFW_FIELD_TRIAL_SERVICE_LOCAL Programme *AllocateProgramme (bson_oid_t *id_p, const char *abbreviation_s, Crop *crop_p, const char *documentation_url_s, const char *name_s, const char *objective_s, const char *pi_name_s);


DFW_FIELD_TRIAL_SERVICE_LOCAL void FreeProgramme (Programme *programme_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool AddProgrammeFieldTrial (Programme *programme_p, FieldTrial *trial_p, MEM_FLAG mf);


DFW_FIELD_TRIAL_SERVICE_LOCAL uint32 GetNumberOfProgrammeFieldTrials (const Programme *programme_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL json_t *GetProgrammeAsJSON (Programme *programme_p, const ViewFormat format, const FieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL Programme *GetProgrammeFromJSON (const json_t *json_p, const ViewFormat format, const FieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool AddFieldTrialsToProgrammeJSON (Programme *programme_p, json_t *program_json_p, const ViewFormat format, const FieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL Programme *GetUniqueProgrammeBySearchString (const char *programme_s, const ViewFormat format, const FieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL Programme *GetProgrammeById (const bson_oid_t *id_p, const ViewFormat format, const FieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL Programme *GetProgrammeByIdString (const char *program_id_s, const ViewFormat format, const FieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL OperationStatus SaveProgramme (Programme *programme_p, ServiceJob *job_p, FieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool RemoveProgrammeFieldTrial (Programme *programme_p, FieldTrial *trial_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL LinkedList *GetProgrammesByName (const char * const programme_s, const FieldTrialServiceData *data_p);


#ifdef __cplusplus
}
#endif


#endif /* SERVICES_FIELD_TRIALS_INCLUDE_PROGRAM_H_ */





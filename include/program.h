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



typedef struct Program
{
	bson_oid_t *pr_id_p;

	char *pr_abbreviation_s;

	char *pr_common_crop_name_s;

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

} Program;


typedef struct ProgramNode
{
	ListItem pn_node;
	Program *pn_program_p;
} ProgramNode;


#ifndef DOXYGEN_SHOULD_SKIP_THIS

#ifdef ALLOCATE_PROGRAM_TAGS
	#define PROGRAM_PREFIX DFW_FIELD_TRIAL_SERVICE_LOCAL
	#define PROGRAM_VAL(x)	= x
	#define PROGRAM_CONCAT_VAL(x,y)	= x y
#else
	#define PROGRAM_PREFIX extern
	#define PROGRAM_VAL(x)
	#define PROGRAM_CONCAT_VAL(x,y)
#endif

#endif 		/* #ifndef DOXYGEN_SHOULD_SKIP_THIS */



PROGRAM_PREFIX const char *PR_NAME_S PROGRAM_CONCAT_VAL (CONTEXT_PREFIX_SCHEMA_ORG_S, "name");

PROGRAM_PREFIX const char *PR_OBJECTIVE_S PROGRAM_CONCAT_VAL (CONTEXT_PREFIX_SCHEMA_ORG_S, "description");

PROGRAM_PREFIX const char *PR_ID_S PROGRAM_VAL ("_id");

PROGRAM_PREFIX const char *PR_TRIALS_S PROGRAM_VAL ("trials");

PROGRAM_PREFIX const char *PR_DOCUMENTATION_URL_S PROGRAM_CONCAT_VAL (CONTEXT_PREFIX_SCHEMA_ORG_S, "url");

PROGRAM_PREFIX const char *PR_PI_NAME_S PROGRAM_VAL ("principal_investigator");

PROGRAM_PREFIX const char *PR_CROP_S PROGRAM_VAL ("crop");

PROGRAM_PREFIX const char *PR_ABBREVIATION_S PROGRAM_CONCAT_VAL (CONTEXT_PREFIX_SCHEMA_ORG_S, "alternateName");



#ifdef __cplusplus
extern "C"
{
#endif


DFW_FIELD_TRIAL_SERVICE_LOCAL Program *AllocateProgram (bson_oid_t *id_p, const char *abbreviation_s, const char *common_crop_name_s, const char *documentation_url_s, const char *name_s, const char *objective_s, const char *pi_name_s);


DFW_FIELD_TRIAL_SERVICE_LOCAL void FreeProgram (Program *program_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool AddProgramFieldTrial (Program *program_p, FieldTrial *trial_p, MEM_FLAG mf);


DFW_FIELD_TRIAL_SERVICE_LOCAL uint32 GetNumberOfProgramFieldTrials (const Program *program_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL json_t *GetProgramAsJSON (Program *program_p, const ViewFormat format, const FieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL Program *GetProgramFromJSON (const json_t *json_p, const FieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool AddFieldTrialsToProgramJSON (Program *program_p, json_t *program_json_p, const ViewFormat format, const FieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL Program *GetUniqueProgramBySearchString (const char *program_s, const ViewFormat format, const FieldTrialServiceData *data_p);


#ifdef __cplusplus
}
#endif


#endif /* SERVICES_FIELD_TRIALS_INCLUDE_PROGRAM_H_ */





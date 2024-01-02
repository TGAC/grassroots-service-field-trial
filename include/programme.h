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
#include "permission.h"


/**
 * The datatype representing a Programme of Field Trials and Studies.
 *
 * @defgroup programme
 * @ingroup field_trials_service
 */
typedef struct Programme
{
	bson_oid_t *pr_id_p;


	PermissionsGroup *pr_permissions_p;

	/**
	 * The time when this Field Trial was saved.
	 */
	char *pr_timestamp_s;


	/**
	 * The abbreviation for this Programme.
	 */
	char *pr_abbreviation_s;

	/**
	 * The Crop that this Programme is researching.
	 */
	Crop *pr_crop_p;

	/**
	 * The web address for any documentaion about this Programme.
	 */
	char *pr_documentation_url_s;

	/**
	 * The name of this Programme.
	 */
	char *pr_name_s;

	/**
	 * The objective of this Programme.
	 */
	char *pr_objective_s;

	/**
	 * The Person leading the Programme.
	 */
	Person *pr_pi_p;

	/**
	 * A LinkedList of FieldTrialNodes
	 * for all of the FieldTrials in this
	 * Programme.
	 */
	LinkedList *pr_trials_p;


	/**
	 * Web address of on optional logo.
	 */
	char *pr_logo_url_s;


	/**
	 *  The name of the funding organisation.
	 */
	char *pr_funding_organisation_s;

	/**
	 *  The programme's pproject or grant code.
	 */
	char *pr_project_code_s;

} Programme;


typedef struct ProgrammeNode
{
	ListItem pn_node;
	Programme *pn_programme_p;
} ProgrammeNode;


#ifndef DOXYGEN_SHOULD_SKIP_THIS

#ifdef ALLOCATE_PROGRAMME_TAGS
	#define PROGRAMME_PREFIX DFW_FIELD_TRIAL_SERVICE_API
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

PROGRAMME_PREFIX const char *PR_PI_S PROGRAMME_VAL ("principal_investigator");

PROGRAMME_PREFIX const char *PR_CROP_S PROGRAMME_VAL ("crop");

PROGRAMME_PREFIX const char *PR_ABBREVIATION_S PROGRAMME_CONCAT_VAL (CONTEXT_PREFIX_SCHEMA_ORG_S, "alternateName");

PROGRAMME_PREFIX const char *PR_LOGO_S PROGRAMME_CONCAT_VAL (CONTEXT_PREFIX_SCHEMA_ORG_S, "image");


PROGRAMME_PREFIX const char *PR_FD_NAME_S PROGRAMME_VAL ("name");

PROGRAMME_PREFIX const char *PR_FUNDERS_S PROGRAMME_VAL("funders");

PROGRAMME_PREFIX const char *PR_CODE_S PROGRAMME_VAL("code");


PROGRAMME_PREFIX const char *PR_FUNDING_IDENTIFIER_S PROGRAMME_VAL("identifier");
PROGRAMME_PREFIX const char *PR_FUNDING_ORG_NAME_S PROGRAMME_VAL("name");
PROGRAMME_PREFIX const char *PR_GRANT_FUNDER_S PROGRAMME_VAL("funder");
PROGRAMME_PREFIX const char *PR_FUNDING_S PROGRAMME_VAL("funding");

#ifdef __cplusplus
extern "C"
{
#endif


DFW_FIELD_TRIAL_SERVICE_LOCAL Programme *AllocateProgramme (bson_oid_t *id_p, const char *abbreviation_s, Crop *crop_p, const char *documentation_url_s, const char *name_s, const char *objective_s, Person *pi_p, const char *logo_url_s, const char *funders_s, const char *project_code_s, const char *timestamp_s);


/**
 * Free a given Programme.
 *
 * @param programme_p The Programme to free.
 * @ingroup programme
 */
DFW_FIELD_TRIAL_SERVICE_LOCAL void FreeProgramme (Programme *programme_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL ProgrammeNode *AllocateProgrammeNode (Programme *programme_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL void FreeProgrammeNode (ListItem *node_p);



DFW_FIELD_TRIAL_SERVICE_LOCAL bool AddProgrammeFieldTrial (Programme *programme_p, FieldTrial *trial_p, MEM_FLAG mf);


DFW_FIELD_TRIAL_SERVICE_LOCAL uint32 GetNumberOfProgrammeFieldTrials (const Programme *programme_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL json_t *GetProgrammeAsJSON (Programme *programme_p, const ViewFormat format, const FieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL Programme *GetProgrammeFromJSON (const json_t *json_p, const ViewFormat format, const FieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool AddFieldTrialsToProgrammeJSON (Programme *programme_p, json_t *program_json_p, const ViewFormat format, FieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL Programme *GetUniqueProgrammeBySearchString (const char *programme_s, const ViewFormat format, const FieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL Programme *GetProgrammeById (const bson_oid_t *id_p, const ViewFormat format, const FieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL Programme *GetProgrammeByIdString (const char *program_id_s, const ViewFormat format, const FieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL OperationStatus SaveProgramme (Programme *programme_p, ServiceJob *job_p, FieldTrialServiceData *data_p, User *user_p);



/**
 * Remove a FieldTrial from a given Programme.
 *
 * @param programme_p The Programme to remove the FieldTrial from.
 * @param trial_p The FieldTrial to remove.
 * @return <code>true</code> if the FieldTrial was removed from the Programme,
 * code>false</code> if the FieldTrial was not on the list of Trials belonging
 * to the given Programme.
 * @ingroup programme
 */
DFW_FIELD_TRIAL_SERVICE_LOCAL bool RemoveProgrammeFieldTrial (Programme *programme_p, FieldTrial *trial_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL LinkedList *GetProgrammesByName (const char * const programme_s, const FieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL Programme *GetVersionedProgramme (const char *programme_id_s, const char *timestamp_s, const ViewFormat format, const FieldTrialServiceData *data_p);


#ifdef __cplusplus
}
#endif


#endif /* SERVICES_FIELD_TRIALS_INCLUDE_PROGRAM_H_ */





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
 * field_trial.h
 *
 *  Created on: 11 Sep 2018
 *      Author: billy
 */

#ifndef SERVICES_FIELD_TRIALS_INCLUDE_FIELD_TRIAL_H_
#define SERVICES_FIELD_TRIALS_INCLUDE_FIELD_TRIAL_H_

#include "dfw_field_trial_service_data.h"
#include "dfw_field_trial_service_library.h"
#include "jansson.h"

#include "typedefs.h"
#include "linked_list.h"

/* forward declarations */
struct Study;
struct Programme;


typedef struct FieldTrial
{
	bson_oid_t *ft_id_p;

	struct Programme *ft_parent_p;

	MEM_FLAG ft_parent_program_mem;


	/**
	 * Name of the field trial
	 */
	char *ft_name_s;

	char *ft_team_s;

	/**
	 * A LinkedList of StudyNodes
	 * for all of the Studies in this
	 * FieldTrial.
	 */
	LinkedList *ft_studies_p;

} FieldTrial;


typedef struct FieldTrialNode
{
	ListItem ftn_node;

	FieldTrial *ftn_field_trial_p;

} FieldTrialNode;


#ifndef DOXYGEN_SHOULD_SKIP_THIS

#ifdef ALLOCATE_FIELD_TRIAL_TAGS
	#define FIELD_TRIAL_PREFIX DFW_FIELD_TRIAL_SERVICE_LOCAL
	#define FIELD_TRIAL_VAL(x)	= x
	#define FIELD_TRIAL_CONCAT_VAL(x,y)	= x y
#else
	#define FIELD_TRIAL_PREFIX extern
	#define FIELD_TRIAL_VAL(x)
	#define FIELD_TRIAL_CONCAT_VAL(x,y)
#endif

#endif 		/* #ifndef DOXYGEN_SHOULD_SKIP_THIS */




/* Field Trial */


FIELD_TRIAL_PREFIX const char *FT_NAME_S FIELD_TRIAL_CONCAT_VAL (CONTEXT_PREFIX_SCHEMA_ORG_S, "name");

FIELD_TRIAL_PREFIX const char *FT_TEAM_S FIELD_TRIAL_VAL ("team");

FIELD_TRIAL_PREFIX const char *FT_ID_S FIELD_TRIAL_VAL ("_id");

FIELD_TRIAL_PREFIX const char *FT_STUDIES_S FIELD_TRIAL_VAL ("studies");

FIELD_TRIAL_PREFIX const char *FT_PARENT_PROGRAM_S FIELD_TRIAL_VAL ("parent_program");


#ifdef __cplusplus
extern "C"
{
#endif


DFW_FIELD_TRIAL_SERVICE_LOCAL FieldTrial *AllocateFieldTrial (const char *name_s, const char *team_s, struct Programme *parent_program_p, MEM_FLAG parent_program_mem, bson_oid_t *id_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL void FreeFieldTrial (FieldTrial *trial_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL FieldTrialNode *AllocateFieldTrialNode (FieldTrial *trial_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL void FreeFieldTrialNode (ListItem *node_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL json_t *GetFieldTrialAsJSON (FieldTrial *trial_p, const ViewFormat format, const FieldTrialServiceData *data_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL FieldTrial *GetFieldTrialFromJSON (const json_t *json_p, const ViewFormat format, const FieldTrialServiceData *data_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL LinkedList *GetFieldTrialStudies (FieldTrial *trial_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL char *GetFieldTrialIdAsString (const FieldTrial *trial_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL FieldTrial *GetUniqueFieldTrialBySearchString (const char *trial_s, const ViewFormat format, const FieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool AddFieldTrialStudy (FieldTrial *trial_p, struct Study *study_p, MEM_FLAG mf);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool SaveFieldTrial (FieldTrial *trial_p, ServiceJob *job_p, FieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL FieldTrial *GetFieldTrialByIdString (const char *field_trial_id_s, const ViewFormat format, const FieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool GetAllFieldTrialStudies (FieldTrial *trial_p, const ViewFormat format, const FieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL char *GetFieldTrialAsString (const FieldTrial *trial_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool AddStudiesToFieldTrialJSON (FieldTrial *trial_p, json_t *trial_json_p, const ViewFormat format, const FieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL LinkedList *GetFieldTrialsByName (const char * const trial_s, const FieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL FieldTrial *GetFieldTrialById (const bson_oid_t *id_p, const ViewFormat format, const FieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool RemoveFieldTrialStudy (FieldTrial *trial_p, struct Study *study_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL uint32 GetNumberOfFieldTrialStudies (const FieldTrial *trial_p);


#ifdef __cplusplus
}
#endif





#endif /* SERVICES_FIELD_TRIALS_INCLUDE_FIELD_TRIAL_H_ */

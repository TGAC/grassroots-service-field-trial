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

#ifndef SERVICES_DFW_FIELD_TRIAL_SERVICE_INCLUDE_FIELD_TRIAL_H_
#define SERVICES_DFW_FIELD_TRIAL_SERVICE_INCLUDE_FIELD_TRIAL_H_

#include "jansson.h"

#include "typedefs.h"
#include "dfw_field_trial_service_library.h"
#include "dfw_field_trial_service_data.h"
#include "linked_list.h"

/* forward declarations */
struct ExperimentalArea;

typedef struct FieldTrial
{
	DFWId ft_id;

	char *ft_name_s;

	char *ft_team_s;

	LinkedList *experimental_areas_p;

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
#else
	#define FIELD_TRIAL_PREFIX extern
	#define FIELD_TRIAL_VAL(x)
#endif

#endif 		/* #ifndef DOXYGEN_SHOULD_SKIP_THIS */


FIELD_TRIAL_PREFIX const char *FT_NAME_S FIELD_TRIAL_VAL ("name");


FIELD_TRIAL_PREFIX const char *FT_TEAM_S FIELD_TRIAL_VAL ("team");


#ifdef __cplusplus
extern "C"
{
#endif


DFW_FIELD_TRIAL_SERVICE_LOCAL FieldTrial *AllocateFieldTrial (const char *name_s, const char *team_s, DFWFieldTrialServiceData *data_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL void FreeFieldTrial (FieldTrial *trial_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL FieldTrialNode *AllocateFieldTrialNodeByParts (const char *name_s, const char *team_s, DFWFieldTrialServiceData *data_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL FieldTrialNode *AllocateFieldTrialNode (FieldTrial *trial_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL void FreeFieldTrialNode (ListItem *node_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL json_t *GetFieldTrialAsJSON (const FieldTrial *trial_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL FieldTrial *GetFieldTrialFromJSON (const json_t *json_p, DFWFieldTrialServiceData *data_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL LinkedList *GetFieldTrialExperimentalAreas (FieldTrial *trial_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool AddFieldTrialExperimentalArea (FieldTrial *trial_p, struct ExperimentalArea *area_p, DFWFieldTrialServiceData *data_p);



#ifdef __cplusplus
}
#endif





#endif /* SERVICES_DFW_FIELD_TRIAL_SERVICE_INCLUDE_FIELD_TRIAL_H_ */

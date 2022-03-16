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
 * crop_ontology_tool.h
 *
 *  Created on: 27 Oct 2018
 *      Author: billy
 */

#ifndef SERVICES_FIELD_TRIALS_INCLUDE_CROP_ONTOLOGY_TOOL_H_
#define SERVICES_FIELD_TRIALS_INCLUDE_CROP_ONTOLOGY_TOOL_H_



#include "schema_term.h"
#include "dfw_field_trial_service_library.h"
#include "mongodb_tool.h"
#include "parameter.h"

#include "operation.h"
#include "dfw_field_trial_service_data.h"
#include "curl_tools.h"


typedef enum
{
	TT_TRAIT,
	TT_METHOD,
	TT_UNIT,
	TT_VARIABLE,
	TT_NUM_TYPES
} TermType;



#ifdef __cplusplus
extern "C"
{
#endif


DFW_FIELD_TRIAL_SERVICE_API SchemaTerm *GetCropOnotologySchemaTerm (const char *crop_ontology_term_s, TermType expected_type, TermType *found_type_p, MongoTool *mongo_p);


DFW_FIELD_TRIAL_SERVICE_API json_t *GetScaleClassAsJSON (const ScaleClass *class_p);


DFW_FIELD_TRIAL_SERVICE_API OperationStatus StoreAllScaleUnits (FieldTrialServiceData *data_p);



DFW_FIELD_TRIAL_SERVICE_LOCAL const ScaleClass *GetScaleClassByName (const char * const name_s);



#ifdef __cplusplus
}
#endif



#endif /* SERVICES_FIELD_TRIALS_INCLUDE_CROP_ONTOLOGY_TOOL_H_ */

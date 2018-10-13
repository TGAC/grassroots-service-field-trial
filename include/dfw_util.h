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
 * dfw_util.h
 *
 *  Created on: 8 Oct 2018
 *      Author: billy
 */


#ifndef DFW_FIELD_TRIAL_SERVICE_DFW_UTIL_H_
#define DFW_FIELD_TRIAL_SERVICE_DFW_UTIL_H_

#include "dfw_field_trial_service_data.h"



#ifdef __cplusplus
extern "C"
{
#endif


DFW_FIELD_TRIAL_SERVICE_LOCAL	void *GetDFWObjectById (const bson_oid_t *id_p, DFWFieldTrialData collection_type, void *(*get_obj_from_json_fn) (const json_t *json_p, const DFWFieldTrialServiceData *data_p), const DFWFieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL void *GetDFWObjectByIdString (const char *object_id_s, DFWFieldTrialData collection_type, void *(*get_obj_from_json_fn) (const json_t *json_p, const DFWFieldTrialServiceData *data_p), const DFWFieldTrialServiceData *data_p);

#ifdef __cplusplus
}
#endif


#endif		/* #ifndef DFW_FIELD_TRIAL_SERVICE_DFW_UTIL_H_ */

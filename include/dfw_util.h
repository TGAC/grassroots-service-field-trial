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


DFW_FIELD_TRIAL_SERVICE_LOCAL	void *GetDFWObjectById (const bson_oid_t *id_p, DFWFieldTrialData collection_type, void *(*get_obj_from_json_fn) (const json_t *json_p, const ViewFormat format, const DFWFieldTrialServiceData *data_p), const ViewFormat format, const DFWFieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL void *GetDFWObjectByIdString (const char *object_id_s, DFWFieldTrialData collection_type, void *(*get_obj_from_json_fn) (const json_t *json_p, const ViewFormat format, const DFWFieldTrialServiceData *data_p), const ViewFormat format, const DFWFieldTrialServiceData *data_p);



DFW_FIELD_TRIAL_SERVICE_LOCAL bool CopyValidDate (const struct tm *src_p, struct tm **dest_pp);

DFW_FIELD_TRIAL_SERVICE_LOCAL bool CreateValidDateFromJSON (const json_t *json_p, const char *key_s, struct tm **time_pp);

DFW_FIELD_TRIAL_SERVICE_LOCAL bool AddValidDateToJSON (struct tm *time_p, json_t *json_p, const char *key_s);

DFW_FIELD_TRIAL_SERVICE_LOCAL bool AddValidDateAsEpochToJSON (struct tm *time_p, json_t *json_p, const char *key_s);

DFW_FIELD_TRIAL_SERVICE_LOCAL bool PrepareSaveData (bson_oid_t **id_pp, bson_t **selector_pp);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool AddContext (json_t *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool AddDatatype (json_t *doc_p, const DFWFieldTrialData data_type);


DFW_FIELD_TRIAL_SERVICE_LOCAL LinkedList *SearchObjects (const DFWFieldTrialServiceData *data_p, const char **keys_ss, const char **values_ss, void (*free_list_item_fn) (ListItem * const item_p), bool (*add_result_to_list_fn) (const json_t *result_p, LinkedList *list_p, const DFWFieldTrialServiceData *service_data_p));


#ifdef __cplusplus
}
#endif


#endif		/* #ifndef DFW_FIELD_TRIAL_SERVICE_DFW_UTIL_H_ */

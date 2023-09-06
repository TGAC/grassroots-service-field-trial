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

#include "json_processor.h"

#ifdef __cplusplus
extern "C"
{
#endif




DFW_FIELD_TRIAL_SERVICE_LOCAL	void *GetDFWObjectById (const bson_oid_t *id_p, DFWFieldTrialData collection_type, void *(*get_obj_from_json_fn) (const json_t *json_p, const ViewFormat format, const FieldTrialServiceData *data_p), const ViewFormat format, const FieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL	void *GetDFWObjectByNamedId (const bson_oid_t *id_p, DFWFieldTrialData collection_type, const char *id_key_s, void *(*get_obj_from_json_fn) (const json_t *json_p, const ViewFormat format, const FieldTrialServiceData *data_p), const ViewFormat format, const FieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL void *GetDFWObjectByIdString (const char *object_id_s, DFWFieldTrialData collection_type, void *(*get_obj_from_json_fn) (const json_t *json_p, const ViewFormat format, const FieldTrialServiceData *data_p), const ViewFormat format, const FieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL void *GetDFWObjectByNamedIdString (const char *object_id_s, DFWFieldTrialData collection_type, const char *id_key_s, void *(*get_obj_from_json_fn) (const json_t *json_p, const ViewFormat format, const FieldTrialServiceData *data_p), const ViewFormat format, const FieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool CopyValidDate (const struct tm *src_p, struct tm **dest_pp);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool CopyValidReal (const double64 *src_p, double64 **dest_pp);

DFW_FIELD_TRIAL_SERVICE_LOCAL bool CopyValidUnsignedInteger (const uint32 *src_p, uint32 **dest_pp);

DFW_FIELD_TRIAL_SERVICE_LOCAL bool CopyValidInteger (const int32 *src_p, int32 **dest_pp);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool CreateValidDateFromJSON (const json_t *json_p, const char *key_s, struct tm **time_pp);

DFW_FIELD_TRIAL_SERVICE_LOCAL bool AddValidDateToJSON (struct tm *time_p, json_t *json_p, const char *key_s, const bool add_time_flag);

DFW_FIELD_TRIAL_SERVICE_LOCAL bool AddValidDateAsEpochToJSON (struct tm *time_p, json_t *json_p, const char *key_s);

DFW_FIELD_TRIAL_SERVICE_LOCAL bool PrepareSaveData (bson_oid_t **id_pp, bson_t **selector_pp);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool AddContext (json_t *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool AddImage (json_t *doc_p, const DFWFieldTrialData data_type, const FieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool AddDatatype (json_t *doc_p, const DFWFieldTrialData data_type);


DFW_FIELD_TRIAL_SERVICE_LOCAL LinkedList *SearchObjects (const FieldTrialServiceData *data_p, const DFWFieldTrialData collection_type, const char **keys_ss, const char **values_ss, void (*free_list_item_fn) (ListItem * const item_p), bool (*add_result_to_list_fn) (const json_t *result_p, LinkedList *list_p, const FieldTrialServiceData *service_data_p));


DFW_FIELD_TRIAL_SERVICE_LOCAL bool CacheStudy (const char *id_s, const json_t *study_json_p, const FieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL json_t *GetCachedStudy (const char *id_s, const FieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool ClearCachedStudy (const char *id_s, const FieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool FindAndAddResultToServiceJob (const char *id_s, const ViewFormat format, ServiceJob *job_p, JSONProcessor *processor_p,
																																 json_t *(get_json_fn) (const char *id_s, const ViewFormat format, JSONProcessor *processor_p, char **name_ss, const FieldTrialServiceData *data_p),
																																 const DFWFieldTrialData datatype, const FieldTrialServiceData *data_p);



DFW_FIELD_TRIAL_SERVICE_LOCAL bool GetValidRealFromJSON (const json_t *study_json_p, const char *key_s, double64 **ph_pp);

DFW_FIELD_TRIAL_SERVICE_LOCAL bool GetValidUnsignedIntFromJSON (const json_t *study_json_p, const char *key_s, uint32 **value_pp);


DFW_FIELD_TRIAL_SERVICE_LOCAL json_t *GetImageObject (const char *image_url_s, const char *thumbnail_url_s);


DFW_FIELD_TRIAL_SERVICE_LOCAL const char *GetNamedParameterDefaultValueFromJSON (const char *id_param_s, const json_t *params_json_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL char *GetFrictionlessDataFilename (const char * const name_s, const FieldTrialServiceData *data_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL char *GetFrictionlessDataURL (const char *const name_s, const FieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL char *GetStudyHandbookURL (const char *const name_s, const FieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL char *GetBackupFilename (const char *id_s, const FieldTrialServiceData *data_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL char *GetPlotsUploadsFilename (const char *id_s, const FieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool SetFieldTrialServiceJobURL (ServiceJob *job_p, const char * const url_prefix_s, const char * const id_s);


DFW_FIELD_TRIAL_SERVICE_LOCAL const char *GetStringDefaultValueFromJSON (const char *param_s, const json_t *params_json_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL json_t *GetAllVersionsOfObject (const char *id_s, DFWFieldTrialData collection_type, const FieldTrialServiceData *data_p);


#ifdef __cplusplus
}
#endif


#endif		/* #ifndef DFW_FIELD_TRIAL_SERVICE_DFW_UTIL_H_ */

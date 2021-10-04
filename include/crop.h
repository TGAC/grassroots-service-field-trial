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
 * crop.h
 *
 *  Created on: 12 Apr 2019
 *      Author: billy
 */

#ifndef SERVICES_FIELD_TRIALS_INCLUDE_CROP_H_
#define SERVICES_FIELD_TRIALS_INCLUDE_CROP_H_

#include "dfw_field_trial_service_library.h"
#include "dfw_field_trial_service_data.h"

#include "jansson.h"

/**
 * @ingroup field_trials_service
 */
typedef struct Crop
{
	bson_oid_t *cr_id_p;

	char *cr_name_s;

	char *cr_argovoc_preferred_term_s;

	char *cr_agrovoc_uri_s;

	/*
	 * A NULL-terminated array of synonyms for this crop variety.
	 * This can be NULL meaning there are no synonyms.
	 */
	char **cr_synonyms_ss;

} Crop;


#ifndef DOXYGEN_SHOULD_SKIP_THIS

#ifdef ALLOCATE_CROP_TAGS
	#define CROP_PREFIX DFW_FIELD_TRIAL_SERVICE_LOCAL
	#define CROP_VAL(x)	= x
	#define CROP_CONCAT_VAL(x,y)	= x y
#else
	#define CROP_PREFIX extern
	#define CROP_VAL(x)
	#define CROP_CONCAT_VAL(x,y)
#endif

#endif 		/* #ifndef DOXYGEN_SHOULD_SKIP_THIS */


CROP_PREFIX const char *CR_NAME_S CROP_CONCAT_VAL (CONTEXT_PREFIX_SCHEMA_ORG_S, "name");

CROP_PREFIX const char *CR_AGROVOC_URL_S CROP_CONCAT_VAL (CONTEXT_PREFIX_SCHEMA_ORG_S, "url");

CROP_PREFIX const char *CR_AGROVOC_PREFERRED_TERM_S CROP_VAL ("preferred_term");

CROP_PREFIX const char *CR_SYNONYMS_S CROP_VAL ("synonyms");


#ifdef __cplusplus
extern "C"
{
#endif


DFW_FIELD_TRIAL_SERVICE_LOCAL Crop *AllocateCrop (bson_oid_t *id_p, const char *name_s, const char *argovoc_preferred_term_s, const char *agrovoc_uri_s, char **synonyms_ss);


DFW_FIELD_TRIAL_SERVICE_LOCAL void FreeCrop (Crop *crop_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL json_t *GetCropAsJSON (Crop *crop_p, const ViewFormat format, const FieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL Crop *GetCropFromJSON (const json_t *crop_json_p, const FieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool SaveCrop (Crop *crop_p, const FieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL Crop *GetCropByIdString (const char *id_s, const FieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL Crop *GetCropById (const bson_oid_t *id_p, const FieldTrialServiceData *data_p);


#ifdef __cplusplus
}
#endif

#endif /* SERVICES_FIELD_TRIALS_INCLUDE_CROP_H_ */

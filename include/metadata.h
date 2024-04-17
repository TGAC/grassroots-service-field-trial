/*
 * db_ownership.h
 *
 *  Created on: 3 Mar 2024
 *      Author: billy
 */

#ifndef SERVICES_FIELD_TRIALS_INCLUDE_METADATA_H_
#define SERVICES_FIELD_TRIALS_INCLUDE_METADATA_H_

#include "dfw_field_trial_service_library.h"

#include "service.h"

#include "permission.h"

typedef struct Metadata
{
	PermissionsGroup *me_permissions_p;

	/**
	 * The User that saved this version of the object.
	 */
	User *me_user_p;

	bool me_owns_user_flag;

	/**
	 * The time when this object was saved.
	 */
	char *me_timestamp_s;


} Metadata;




#ifdef __cplusplus
extern "C"
{
#endif


DFW_FIELD_TRIAL_SERVICE_LOCAL Metadata *AllocateMetadata (PermissionsGroup *permissions_group_p, User *user_p, const bool owns_user_flag, const char *timestamp_s);


DFW_FIELD_TRIAL_SERVICE_LOCAL void FreeMetadata (Metadata *metadata_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool AddMetadataToJSON (const Metadata * const metadata_p, json_t *parent_json_p, const ViewFormat vf);


DFW_FIELD_TRIAL_SERVICE_LOCAL Metadata *GetMetadataFromJSON (const json_t * const json_p, const ServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL Metadata *GetMetadataFromDefaultChildJSON (const json_t * const json_p, const ServiceData *data_p);



DFW_FIELD_TRIAL_SERVICE_LOCAL json_t *GetMetadataAsJSON (const Metadata * const metadata_p, const ViewFormat vf);


DFW_FIELD_TRIAL_SERVICE_LOCAL void SetMetadataUser (Metadata *metadata_p, User *user_p, bool owns_user_flag);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool MetadataHasUser (Metadata *metadata_p);



#ifdef __cplusplus
}
#endif



#endif /* SERVICES_FIELD_TRIALS_INCLUDE_METADATA_H_ */

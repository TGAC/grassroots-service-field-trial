/*
 * metadata.c
 *
 *  Created on: 3 Mar 2024
 *      Author: billy
 */


#include "metadata.h"


static const char * const S_METADATA_JSON_KEY_S = "metadata";


Metadata *AllocateMetadata (bson_oid_t *id_p, PermissionsGroup *permissions_group_p, User *user_p, const bool owns_user_flag, const char *timestamp_s)
{
	char *copied_timestamp_s = NULL;

	if ((timestamp_s == NULL) || (copied_timestamp_s = EasyCopyToNewString (timestamp_s)))
		{
			bool alloc_perms_flag = false;

			if (!permissions_group_p)
				{
					permissions_group_p = AllocatePermissionsGroup ();

					if (permissions_group_p)
						{
							alloc_perms_flag = true;
						}
				}

			if (permissions_group_p)
				{
					Metadata *metadata_p = (Metadata *) AllocMemory (sizeof (Metadata));

					if (metadata_p)
						{
							metadata_p -> me_id_p = id_p;
							metadata_p -> me_timestamp_s = copied_timestamp_s;
							metadata_p -> me_user_p = user_p;
							metadata_p -> me_owns_user_flag = owns_user_flag;

							metadata_p -> me_permissions_p = permissions_group_p;

							return metadata_p;
						}

					if (alloc_perms_flag)
						{
							FreePermissionsGroup (permissions_group_p);
						}
				}


			if (copied_timestamp_s)
				{
					FreeCopiedString (copied_timestamp_s);
				}
		}		/* if ((timestamp_s == NULL) || (copied_timestamp_s = EasyCopyToNewString (timestamp_s))) */

	return NULL;
}


void FreeMetadata (Metadata *metadata_p)
{
	if (metadata_p -> me_id_p)
		{
			FreeBSONOid (metadata_p -> me_id_p);
		}

	if (metadata_p -> me_permissions_p)
		{
			FreePermissionsGroup (metadata_p -> me_permissions_p);
		}

	if ((metadata_p -> me_user_p) && (metadata_p -> me_owns_user_flag))
		{
			FreeUser (metadata_p -> me_user_p);
		}

	if (metadata_p -> me_timestamp_s)
		{
			FreeCopiedString (metadata_p -> me_timestamp_s);
		}

	FreeMetadata (metadata_p);
}



bool AddMetadataToJSON (const Metadata * const metadata_p, json_t *parent_json_p)
{
	bool success_flag = false;
	json_t *metadata_json_p = GetMetadataAsJSON (metadata_p);

	if (metadata_json_p)
		{
			if (json_object_set_new (parent_json_p, S_METADATA_JSON_KEY_S, metadata_json_p) == 0)
				{
					success_flag = true;
				}
			else
				{
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, metadata_json_p, "Failed to add metadata to json");
					json_decref (metadata_json_p);
				}
		}
	else
		{
			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, parent_json_p, "GetMetadataAsJSON () failed");
		}

	return success_flag;
}


Metadata *GetMetadataFromJSON (const json_t * const json_p, const ServiceData *data_p)
{
	Metadata *metadata_p = NULL;
	bson_oid_t *id_p = GetNewUnitialisedBSONOid ();

	if (id_p)
		{
			if (GetMongoIdFromJSON (json_p, id_p))
				{
					const char *timestamp_s = GetJSONString (json_p, MONGO_TIMESTAMP_S);
					User *user_p = NULL;
					GrassrootsServer *grassroots_p = data_p -> sd_service_p -> se_grassroots_p;
					PermissionsGroup *perms_group_p = GetPermissionsGroupFromChildJSON (json_p, FT_PERMISSIONS_S, grassroots_p);
					bson_oid_t *temp_id_p = GetNewUnitialisedBSONOid ();

					if (temp_id_p)
						{
							json_t *user_json_p = json_object_get (json_p, FT_USER_S);

							if (user_json_p)
								{
									if (GetMongoIdFromJSON (user_json_p, temp_id_p))
										{
											user_p = GetUserById (grassroots_p, temp_id_p);

											if (user_p)
												{

												}
										}
								}

							FreeBSONOid (temp_id_p);
						}

					if (!metadata_p)
						{
							if (perms_group_p)
								{
									FreePermissionsGroup (perms_group_p);
								}
						}
				}

			if (!metadata_p)
				{
					FreeBSONOid (id_p);

				}
		}

	return metadata_p;
}



json_t *GetMetadataAsJSON (const Metadata * const metadata_p, const bool add_id_flag)
{
	json_t *metadata_json_p = json_object ();

	if (metadata_json_p)
		{
			if (SetNonTrivialString (metadata_json_p, MONGO_TIMESTAMP_S, metadata_p -> me_timestamp_s, true))
				{
					if ((!add_id_flag) || (AddCompoundIdToJSON (metadata_json_p, metadata_p -> me_id_p)))
						{
							if ((metadata_p -> me_user_p == NULL) || (AddUserToJSON (metadata_p -> me_user_p, metadata_json_p, FT_USER_S, true)))
								{
									if ((metadata_p -> me_permissions_p == NULL) || (AddPermissionsGroupToJSON (metadata_p -> me_permissions_p, metadata_json_p, FT_PERMISSIONS_S, VF_REFERENCE)))
										{
											return metadata_json_p;
										}
									else
										{
											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, metadata_json_p, "AddPermissionsGroupToJSON () failed");
										}
								}
							else
								{
									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, metadata_json_p, "AddUserToJSON () failed for \"%s\"", metadata_p -> me_user_p -> us_email_s);
								}


						}

				}		/* if (SetNonTrivialString (metadata_json_p, MONGO_TIMESTAMP_S, metadata_p -> me_timestamp_s, true)) */

			json_decref (metadata_json_p);
		}

	return NULL;
}

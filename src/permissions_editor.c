/*
 * permissions_editor.c
 *
 *  Created on: 10 Jan 2024
 *      Author: billy
 */

#include "permissions_editor.h"



static NamedParameterType PERMISSION_READ = { "PE Read", PT_STRING_ARRAY } ;


bool AddPermissionsEditor (PermissionsGroup *permissions_group_p, const char *id_s, ParameterSet *param_set_p, const bool read_only_flag, FieldTrialServiceData *dfw_data_p)
{
	bool success_flag = false;
	const GrassrootsServer *grassroots_p = dfw_data_p -> dftsd_base_data.sd_service_p -> se_grassroots_p;

	LinkedList *users_p = GetAllUsers (grassroots_p);

	if (users_p)
		{
			bool success_flag = true;
			UserNode *node_p = (UserNode *) (users_p -> ll_head_p);

			while (node_p && success_flag)
				{

					if (success_flag)
						{
							node_p = (UserNode *) (node_p -> un_node.ln_next_p);
						}
				}

		}

	return success_flag;
}

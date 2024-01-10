/*
 * permissions_editor.h
 *
 *  Created on: 10 Jan 2024
 *      Author: billy
 */

#ifndef SERVICES_FIELD_TRIALS_INCLUDE_PERMISSIONS_EDITOR_H_
#define SERVICES_FIELD_TRIALS_INCLUDE_PERMISSIONS_EDITOR_H_

#include "dfw_field_trial_service_data.h"
#include "dfw_field_trial_service_library.h"

#include "permission.h"

#include "parameter_set.h"

#ifdef __cplusplus
extern "C"
{
#endif


DFW_FIELD_TRIAL_SERVICE_LOCAL bool AddPermissionsEditor (PermissionsGroup *permissions_group_p, const char *id_s, ParameterSet *param_set_p, const bool read_only_flag, FieldTrialServiceData *dfw_data_p);


#ifdef __cplusplus
}
#endif



#endif /* SERVICES_FIELD_TRIALS_INCLUDE_PERMISSIONS_EDITOR_H_ */

/*
 * field_trial_sqlite.h
 *
 *  Created on: 21 Sep 2018
 *      Author: billy
 */

#ifndef SERVICES_DFW_FIELD_TRIAL_SERVICE_INCLUDE_FIELD_TRIAL_SQLITE_H_
#define SERVICES_DFW_FIELD_TRIAL_SERVICE_INCLUDE_FIELD_TRIAL_SQLITE_H_


#include "field_trial.h"
#include "dfw_field_trial_service_data.h"

#ifdef __cplusplus
extern "C"
{
#endif


DFW_FIELD_TRIAL_SERVICE_LOCAL LinkedList *GetFieldTrialsByNameFromSQLite (DFWFieldTrialServiceData *data_p, const char *name_s);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool AddFieldTrialByNameToSQLite (DFWFieldTrialServiceData *data_p, FieldTrial *trial_p);


#ifdef __cplusplus
}
#endif


#endif /* SERVICES_DFW_FIELD_TRIAL_SERVICE_INCLUDE_FIELD_TRIAL_SQLITE_H_ */

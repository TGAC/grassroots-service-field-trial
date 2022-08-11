/*
 * study_manager.c
 *
 *  Created on: 5 Aug 2022
 *      Author: billy
 */

#ifndef SERVICES_FIELD_TRIALS_INCLUDE_STUDY_MANAGER_H_
#define SERVICES_FIELD_TRIALS_INCLUDE_STUDY_MANAGER_H_




#include "dfw_field_trial_service_data.h"
#include "dfw_field_trial_service_library.h"



#ifdef __cplusplus
extern "C"
{
#endif


DFW_FIELD_TRIAL_SERVICE_LOCAL Service *GetStudyManagerService (GrassrootsServer *grassroots_p);


#ifdef __cplusplus
}
#endif


#endif /* SERVICES_FIELD_TRIALS_INCLUDE_STUDY_MANAGER_H_ */

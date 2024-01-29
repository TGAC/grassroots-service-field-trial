/*
 * browse_study_history.h
 *
 *  Created on: 5 Oct 2023
 *      Author: billy
 */

#ifndef SERVICES_FIELD_TRIALS_INCLUDE_BROWSE_STUDY_HISTORY_H_
#define SERVICES_FIELD_TRIALS_INCLUDE_BROWSE_STUDY_HISTORY_H_

#include "dfw_field_trial_service_data.h"
#include "dfw_field_trial_service_library.h"



#ifdef __cplusplus
extern "C"
{
#endif


DFW_FIELD_TRIAL_SERVICE_LOCAL Service *GetBrowseStudyHistoryService (GrassrootsServer *grassroots_p);


#ifdef __cplusplus
}
#endif

#endif /* SERVICES_FIELD_TRIALS_INCLUDE_BROWSE_STUDY_HISTORY_H_ */

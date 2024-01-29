/*
 * dfw_field_trial_service.h
 *
 *  Created on: 13 Jul 2018
 *      Author: billy
 */

#ifndef SERVICES_FIELD_TRIALS_INCLUDE_DFW_FIELD_TRIAL_SERVICE_H_
#define SERVICES_FIELD_TRIALS_INCLUDE_DFW_FIELD_TRIAL_SERVICE_H_


#include "dfw_field_trial_service_library.h"
#include "service.h"




#ifdef __cplusplus
extern "C"
{
#endif


/**
 * Get the Service available for running the DFW Field Trial Service.
 *
 * @param user_p The User for the user trying to access the services.
 * This can be <code>NULL</code>.
 * @return The ServicesArray containing the DFW Field Trial Service. or
 * <code>NULL</code> upon error.
 *
 * @ingroup field_trials_service
 */
DFW_FIELD_TRIAL_SERVICE_API ServicesArray *GetServices (User *user_p, GrassrootsServer *grassroots_p);


/**
 * Free the ServicesArray and its associated DFW Field Trial Service.
 *
 * @param services_p The ServicesArray to free.
 *
 * @ingroup field_trials_service
 */
DFW_FIELD_TRIAL_SERVICE_API void ReleaseServices (ServicesArray *services_p);



#ifdef __cplusplus
}
#endif


#endif /* SERVICES_FIELD_TRIALS_INCLUDE_DFW_FIELD_TRIAL_SERVICE_H_ */

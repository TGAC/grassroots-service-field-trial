/*
 * dfw_field_trial_service.h
 *
 *  Created on: 13 Jul 2018
 *      Author: billy
 */

#ifndef SERVICES_DFW_FIELD_TRIAL_SERVICE_INCLUDE_DFW_FIELD_TRIAL_SERVICE_H_
#define SERVICES_DFW_FIELD_TRIAL_SERVICE_INCLUDE_DFW_FIELD_TRIAL_SERVICE_H_


#include "service.h"

#include "dfw_field_trial_service_library.h"




#ifndef DOXYGEN_SHOULD_SKIP_THIS

#ifdef ALLOCATE_DFW_FIELD_TRIAL_SERVICE_TAGS
	#define DFW_FIELD_TRIAL_PREFIX DFW_FIELD_TRIAL_SERVICE_LOCAL
	#define DFW_FIELD_TRIAL_VAL(x)	= x
	#define DFW_FIELD_TRIAL_CONCAT_VAL(x,y) = x y
#else
	#define DFW_FIELD_TRIAL_PREFIX extern
	#define DFW_FIELD_TRIAL_VAL(x)
	#define DFW_FIELD_TRIAL_CONCAT_VAL(x,y) = x y
#endif

#endif 		/* #ifndef DOXYGEN_SHOULD_SKIP_THIS */


/**
 * The key for specifying the object containing the fields data
 *
 * @ingroup dfw_field_trial_service
 */
DFW_FIELD_TRIAL_PREFIX const char *DFT_FIELD_S DFW_FIELD_TRIAL_VAL ("field");

/**
 * The key for specifying the object containing the plot data
 *
 * @ingroup dfw_field_trial_service
 */
DFW_FIELD_TRIAL_PREFIX const char *DFT_PLOT_S DFW_FIELD_TRIAL_VAL ("plot");

/**
 * The key for specifying the object containing the drilling data
 *
 * @ingroup dfw_field_trial_service
 */
DFW_FIELD_TRIAL_PREFIX const char *DFT_DRILLING_S DFW_FIELD_TRIAL_VAL ("drilling");


/**
 * The key for specifying the object containing the raw phenotype data
 *
 * @ingroup dfw_field_trial_service
 */
DFW_FIELD_TRIAL_PREFIX const char *DFT_RAW_PHENOTYPE_S DFW_FIELD_TRIAL_VAL ("raw_phenotype");


/**
 * The key for specifying the object containing the corrected phenotype data
 *
 * @ingroup dfw_field_trial_service
 */
DFW_FIELD_TRIAL_PREFIX const char *DFT_CORRECTED_PHENOTYPE_S DFW_FIELD_TRIAL_VAL ("corrected_phenotype");




#ifdef __cplusplus
extern "C"
{
#endif


/**
 * Get the Service available for running the DFW Field Trial Service.
 *
 * @param user_p The UserDetails for the user trying to access the services.
 * This can be <code>NULL</code>.
 * @return The ServicesArray containing the DFW Field Trial Service. or
 * <code>NULL</code> upon error.
 *
 * @ingroup dfw_field_trial_service
 */
DFW_FIELD_TRIAL_SERVICE_API ServicesArray *GetServices (UserDetails *user_p);


/**
 * Free the ServicesArray and its associated DFW Field Trial Service.
 *
 * @param services_p The ServicesArray to free.
 *
 * @ingroup dfw_field_trial_service
 */
DFW_FIELD_TRIAL_SERVICE_API void ReleaseServices (ServicesArray *services_p);



DFW_FIELD_TRIAL_SERVICE_LOCAL bool AddErrorMessage (ServiceJob *job_p, const json_t *value_p, const char *error_s, const int index);

#ifdef __cplusplus
}
#endif


#endif /* SERVICES_DFW_FIELD_TRIAL_SERVICE_INCLUDE_DFW_FIELD_TRIAL_SERVICE_H_ */

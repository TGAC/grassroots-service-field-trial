/*
 * observation_metadata.h
 *
 *  Created on: 13 Nov 2024
 *      Author: billy
 */

#ifndef SERVICES_FIELD_TRIALS_INCLUDE_OBSERVATION_METADATA_H_
#define SERVICES_FIELD_TRIALS_INCLUDE_OBSERVATION_METADATA_H_


#include <time.h>

#include "typedefs.h"

#include "dfw_field_trial_service_library.h"
#include "dfw_field_trial_service_data.h"


typedef struct ObservationMetadata
{
	struct tm *om_start_date_p;
	struct tm *om_end_date_p;
	bool om_corrected_flag;
	uint32 om_index;
} ObservationMetadata;


#ifdef __cplusplus
extern "C"
{
#endif


DFW_FIELD_TRIAL_SERVICE_LOCAL void ClearObservationMetadata (ObservationMetadata *obs_metadata_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL void FreeObservationMetadata (ObservationMetadata *obs_metadata_p);


/**
 * Extract the Observation metadata from the column heading
 */

DFW_FIELD_TRIAL_SERVICE_LOCAL OperationStatus GetObservationMetadata (const char *key_s, MeasuredVariable **measured_variable_pp, ObservationMetadata **metadata_pp, bool *notes_flag_p, ServiceJob *job_p, const uint32 row_index, MEM_FLAG *mf_p, FieldTrialServiceData *data_p);



DFW_FIELD_TRIAL_SERVICE_LOCAL bool SetObservationMetadataStartDate (ObservationMetadata * const metadata_p, const struct tm * const time_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool SetObservationMetadataEndDate (ObservationMetadata * const metadata_p, const struct tm * const time_p);


#ifdef __cplusplus
}
#endif


#endif /* SERVICES_FIELD_TRIALS_INCLUDE_OBSERVATION_METADATA_H_ */

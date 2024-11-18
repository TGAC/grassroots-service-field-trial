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




typedef struct ObservationMetadata
{
	struct tm *om_start_date_p;
	struct tm *om_end_date_p;
	bool om_corrected_flag;
	uint32 om_index;
} ObservationMetadata;


typedef struct ObservationMetadata
{
	struct tm *om_start_date_p;
	struct tm *om_end_date_p;
	bool om_corrected_flag;
	uint32 om_index;
} ObservationMetadata;




void ClearObservationMetadata (ObservationMetadata *obs_metadata_p);

void FreeObservationMetadata (ObservationMetadata *obs_metadata_p);


/**
 * Extract the Observation metadata from the column heading
 */

OperationStatus GetObservationMetadata (const char *key_s, MeasuredVariable **measured_variable_pp, ObservationMetadata **metadata_pp, bool *notes_flag_p, ServiceJob *job_p, const uint32 row_index, MEM_FLAG *mf_p, FieldTrialServiceData *data_p);



#endif /* SERVICES_FIELD_TRIALS_INCLUDE_OBSERVATION_METADATA_H_ */

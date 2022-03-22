/*
 * signed_int_observation.h
 *
 *  Created on: 21 Mar 2022
 *      Author: billy
 */

#ifndef SERVICES_FIELD_TRIALS_SRC_SIGNED_INT_OBSERVATION_H_
#define SERVICES_FIELD_TRIALS_SRC_SIGNED_INT_OBSERVATION_H_


#include "observation.h"
#include "typedefs.h"

typedef struct IntegerObservation
{
	Observation io_base_observation;

	/**
	 * The raw phenotypic value for this Observation.
	 */
	int32 *io_raw_value_p;

	/**
	 * The corrected phenotypic value for this Observation.
	 */
	int32 *io_corrected_value_p;


} IntegerObservation;



#ifdef __cplusplus
extern "C"
{
#endif



DFW_FIELD_TRIAL_SERVICE_LOCAL IntegerObservation *AllocateIntegerObservation (bson_oid_t *id_p, const struct tm *start_date_p, const struct tm *end_date_p, MeasuredVariable *phenotype_p, MEM_FLAG phenotype_mem, const int32 *raw_value_p, const int32 *corrected_value_p,
	const char *growth_stage_s, const char *method_s, Instrument *instrument_p, const ObservationNature nature, const uint32 *index_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL void FreeIntegerObservation (IntegerObservation *observation_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL json_t *GetIntegerObservationAsJSON (const IntegerObservation *observation_p, const ViewFormat format);


DFW_FIELD_TRIAL_SERVICE_LOCAL IntegerObservation *GetIntegerObservationFromJSON (const json_t *phenotype_json_p, FieldTrialServiceData *data_p);



DFW_FIELD_TRIAL_SERVICE_LOCAL bool SetIntegerObservationRawValueFromString (IntegerObservation *observation_p, const char *value_s);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool SetIntegerObservationCorrectedValueFromString (IntegerObservation *observation_p, const char *value_s);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool AddIntegerObservationRawValueToJSON (const IntegerObservation *obs_p, const char *key_s, json_t *json_p, const char *null_sequence_s, bool only_if_exists_flag);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool AddIntegerObservationCorrectedValueToJSON (const IntegerObservation *obs_p, const char *key_s, json_t *json_p, const char *null_sequence_s, bool only_if_exists_flag);


#ifdef __cplusplus
}
#endif


#endif /* SERVICES_FIELD_TRIALS_SRC_SIGNED_INT_OBSERVATION_H_ */

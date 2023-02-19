/*
 * time_observation.h
 *
 *  Created on: 7 Apr 2022
 *      Author: billy
 */

#ifndef SERVICES_FIELD_TRIALS_INCLUDE_TIME_OBSERVATION_H_
#define SERVICES_FIELD_TRIALS_INCLUDE_TIME_OBSERVATION_H_


#include "observation.h"
#include "typedefs.h"

#include <time.h>

typedef struct TimeObservation
{
	Observation to_base_observation;

	/**
	 * The raw phenotypic value for this Observation.
	 */
	struct tm *to_raw_value_p;

	/**
	 * The corrected phenotypic value for this Observation.
	 */
	struct tm *to_corrected_value_p;


} TimeObservation;



#ifdef __cplusplus
extern "C"
{
#endif



DFW_FIELD_TRIAL_SERVICE_LOCAL TimeObservation *AllocateTimeObservation (bson_oid_t *id_p, const struct tm *start_date_p, const struct tm *end_date_p, MeasuredVariable *phenotype_p, MEM_FLAG phenotype_mem, const struct tm *raw_value_p, const struct tm *corrected_value_p,
	const char *growth_stage_s, const char *method_s, Instrument *instrument_p, const ObservationNature nature, const uint32 *index_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL void ClearTimeObservation (Observation *observation_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL json_t *GetTimeObservationAsJSON (const TimeObservation *observation_p, const ViewFormat format);


DFW_FIELD_TRIAL_SERVICE_LOCAL TimeObservation *GetTimeObservationFromJSON (const json_t *phenotype_json_p, FieldTrialServiceData *data_p);



DFW_FIELD_TRIAL_SERVICE_LOCAL bool SetTimeObservationRawValueFromString (TimeObservation *observation_p, const char *value_s);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool SetTimeObservationCorrectedValueFromString (TimeObservation *observation_p, const char *value_s);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool SetTimeObservationRawValueFromJSON (TimeObservation *observation_p, const json_t *value_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool SetTimeObservationCorrectedValueFromJSON (TimeObservation *observation_p, const json_t *value_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool AddTimeObservationRawValueToJSON (const TimeObservation *obs_p, const char *key_s, json_t *json_p, const char *null_sequence_s, bool only_if_exists_flag);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool AddTimeObservationCorrectedValueToJSON (const TimeObservation *obs_p, const char *key_s, json_t *json_p, const char *null_sequence_s, bool only_if_exists_flag);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool AddTimeObservationValuesToJSON (const Observation *obs_p, const char *raw_key_s, const char *corrected_key_s, json_t *json_p, const char *null_sequence_s, bool only_if_exists_flag);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool GetTimeObservationRawValueAsString (TimeObservation *observation_p, char **value_ss, bool *free_flag_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool GetTimeObservationCorrectedValueAsString (TimeObservation *observation_p, char **value_ss, bool *free_flag_p);


#ifdef __cplusplus
}
#endif




#endif /* SERVICES_FIELD_TRIALS_INCLUDE_TIME_OBSERVATION_H_ */

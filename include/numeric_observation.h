/*
 * numeric_observation.h
 *
 *  Created on: 16 Mar 2022
 *      Author: billy
 */

#ifndef SERVICES_FIELD_TRIALS_INCLUDE_NUMERIC_OBSERVATION_H_
#define SERVICES_FIELD_TRIALS_INCLUDE_NUMERIC_OBSERVATION_H_

#include "observation.h"
#include "typedefs.h"

typedef struct NumericObservation
{
	Observation no_base_observation;


	/**
	 * The raw phenotypic value for this Observation.
	 */
	double64 *no_raw_value_p;

	/**
	 * The corrected phenotypic value for this Observation.
	 */
	double64 *no_corrected_value_p;


} NumericObservation;



#ifdef __cplusplus
extern "C"
{
#endif



DFW_FIELD_TRIAL_SERVICE_LOCAL NumericObservation *AllocateNumericObservation (bson_oid_t *id_p, const struct tm *start_date_p, const struct tm *end_date_p, MeasuredVariable *phenotype_p, MEM_FLAG phenotype_mem, const double *raw_value_p, const double *corrected_value_p,
	const char *growth_stage_s, const char *method_s, Instrument *instrument_p, const ObservationNature nature, const uint32 *index_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL void ClearNumericObservation (NumericObservation *observation_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL json_t *GetNumericObservationAsJSON (const NumericObservation *observation_p, const ViewFormat format);


DFW_FIELD_TRIAL_SERVICE_LOCAL NumericObservation *GetNumericObservationFromJSON (const json_t *phenotype_json_p, FieldTrialServiceData *data_p);


/**
 * Set the raw value for a given Observation.
 *
 * The value is deep-copied to the Observation so the value passed in can be freed without causing any
 * memory corruption.
 *
 * @param observation_p The Observation that will have its raw value updated.
 * @param value_p The new raw value to store, this can be <code>NULL</code> which will clear the
 * Observation's existing raw value if it has been previously set.
 * @return <code>true</code> if the Observation's raw value was updated successfully, <code>false</code>
 * if there errors.
 * @memberof NumericObservation
 * @ingroup field_trials_service
 */
DFW_FIELD_TRIAL_SERVICE_LOCAL bool SetNumericObservationRawValue (NumericObservation *observation_p, const double64 *value_p);



DFW_FIELD_TRIAL_SERVICE_LOCAL bool SetNumericObservationRawValueFromString (NumericObservation *observation_p, const char *value_s);


/**
 * Set the corrected value for a given Observation.
 *
 * The value is deep-copied to the Observation so the value passed in can be freed without causing any
 * memory corruption.
 *
 * @param observation_p The Observation that will have its corrected value updated.
 * @param value_p The new corrected value to store, this can be <code>NULL</code> which will clear the
 * Observation's existing corrected value if it has been previously set.
 * @return <code>true</code> if the Observation's corrected value was updated successfully, <code>false</code>
 * if there errors.
 * @memberof NumericObservation
 * @ingroup field_trials_service
 */
DFW_FIELD_TRIAL_SERVICE_LOCAL bool SetNumericObservationCorrectedValue (NumericObservation *observation_p, const double64 *value_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool SetNumericObservationCorrectedValueFromString (NumericObservation *observation_p, const char *value_s);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool SetNumericObservationRawValueFromJSON (NumericObservation *observation_p, const json_t *value_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool SetNumericObservationCorrectedValueFromJSON (NumericObservation *observation_p, const json_t *value_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool AddNumericObservationRawValueToJSON (const NumericObservation *obs_p, const char *key_s, json_t *json_p, const char *null_sequence_s, bool only_if_exists_flag);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool AddNumericObservationCorrectedValueToJSON (const NumericObservation *obs_p, const char *key_s, json_t *json_p, const char *null_sequence_s, bool only_if_exists_flag);



#ifdef __cplusplus
}
#endif



#endif /* SERVICES_FIELD_TRIALS_INCLUDE_NUMERIC_OBSERVATION_H_ */

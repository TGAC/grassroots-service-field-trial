/*
 * string_observation.h
 *
 *  Created on: 16 Mar 2022
 *      Author: billy
 */

#ifndef SERVICES_FIELD_TRIALS_INCLUDE_STRING_OBSERVATION_H_
#define SERVICES_FIELD_TRIALS_INCLUDE_STRING_OBSERVATION_H_

#include "observation.h"


typedef struct StringObservation
{
	Observation so_base_observation;

	/**
	 * The raw phenotypic value for this Observation.
	 */
	char *so_raw_value_s;

	/**
	 * The corrected phenotypic value for this Observation.
	 */
	char *so_corrected_value_s;



} StringObservation;



#ifdef __cplusplus
extern "C"
{
#endif



DFW_FIELD_TRIAL_SERVICE_LOCAL StringObservation *AllocateStringObservation (bson_oid_t *id_p, ObservationMetadata *metadata_p, MeasuredVariable *phenotype_p,
																																						MEM_FLAG phenotype_mem, const char * const raw_value_s,
																																						const char * const corrected_value_s, const char *growth_stage_s,
																																						const char *method_s, Instrument *instrument_p, const ObservationNature nature,
																																						const char *notes_s);



DFW_FIELD_TRIAL_SERVICE_LOCAL void ClearStringObservation (Observation *observation_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL json_t *GetStringObservationAsJSON (const StringObservation *observation_p, const ViewFormat format, const FieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL StringObservation *GetStringObservationFromJSON (const json_t *phenotype_json_p, FieldTrialServiceData *data_p);



/**
 * Set the raw value for a given Observation.
 *
 * The value is deep-copied to the Observation so the value passed in can be freed without causing any
 * memory corruption.
 *
 * @param observation_p The Observation that will have its raw value updated.
 * @param value_s The new raw value to store, this can be <code>NULL</code> which will clear the
 * Observation's existing raw value if it has been previously set.
 * @return <code>true</code> if the Observation's raw value was updated successfully, <code>false</code>
 * if there errors.
 * @memberof StringObservation
 * @ingroup field_trials_service
 */
DFW_FIELD_TRIAL_SERVICE_LOCAL bool SetStringObservationRawValue (StringObservation *observation_p, const char *value_s);


/**
 * Set the corrected value for a given Observation.
 *
 * The value is deep-copied to the Observation so the value passed in can be freed without causing any
 * memory corruption.
 *
 * @param observation_p The Observation that will have its corrected value updated.
 * @param value_s The new corrected value to store, this can be <code>NULL</code> which will clear the
 * Observation's existing corrected value if it has been previously set.
 * @return <code>true</code> if the Observation's corrected value was updated successfully, <code>false</code>
 * if there errors.
 * @memberof StringObservation
 * @ingroup field_trials_service
 */
DFW_FIELD_TRIAL_SERVICE_LOCAL bool SetStringObservationCorrectedValue (StringObservation *observation_p, const char *value_s);



DFW_FIELD_TRIAL_SERVICE_LOCAL bool SetStringObservationRawValueFromJSON (StringObservation *observation_p, const json_t *value_p);


/**
 * Set the corrected value for a given Observation.
 *
 * The value is deep-copied to the Observation so the value passed in can be freed without causing any
 * memory corruption.
 *
 * @param observation_p The Observation that will have its corrected value updated.
 * @param value_s The new corrected value to store, this can be <code>NULL</code> which will clear the
 * Observation's existing corrected value if it has been previously set.
 * @return <code>true</code> if the Observation's corrected value was updated successfully, <code>false</code>
 * if there errors.
 * @memberof StringObservation
 * @ingroup field_trials_service
 */
DFW_FIELD_TRIAL_SERVICE_LOCAL bool SetStringObservationCorrectedValueFromJSON (StringObservation *observation_p, const json_t *value_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool AddStringObservationRawValueToJSON (const StringObservation *obs_p, const char *key_s, json_t *json_p, const char *null_sequence_s, bool only_if_exists_flag);



DFW_FIELD_TRIAL_SERVICE_LOCAL bool AddStringObservationCorrectedValueToJSON (const StringObservation *obs_p, const char *key_s, json_t *json_p, const char *null_sequence_s, bool only_if_exists_flag);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool AddStringObservationValuesToJSON (const Observation *obs_p, const char *raw_key_s, const char *corrected_key_s, json_t *json_p, const char *null_sequence_s, bool only_if_exists_flag);


#ifdef __cplusplus
}
#endif



#endif /* SERVICES_FIELD_TRIALS_INCLUDE_STRING_OBSERVATION_H_ */

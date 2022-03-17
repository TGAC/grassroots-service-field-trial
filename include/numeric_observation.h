/*
 * numeric_observation.h
 *
 *  Created on: 16 Mar 2022
 *      Author: billy
 */

#ifndef SERVICES_FIELD_TRIALS_INCLUDE_NUMERIC_OBSERVATION_H_
#define SERVICES_FIELD_TRIALS_INCLUDE_NUMERIC_OBSERVATION_H_

#include "observation.h"


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


DFW_FIELD_TRIAL_SERVICE_LOCAL void FreeNumericObservation (NumericObservation *observation_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL json_t *GetNumericObservationAsJSON (const NumericObservation *observation_p, const ViewFormat format);


DFW_FIELD_TRIAL_SERVICE_LOCAL NumericObservation *GetNumericObservationFromJSON (const json_t *phenotype_json_p, FieldTrialServiceData *data_p);


#ifdef __cplusplus
}
#endif



#endif /* SERVICES_FIELD_TRIALS_INCLUDE_NUMERIC_OBSERVATION_H_ */

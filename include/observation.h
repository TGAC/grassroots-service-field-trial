/*
** Copyright 2014-2018 The Earlham Institute
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/
/*
 * observation.h
 *
 *  Created on: 30 Oct 2018
 *      Author: billy
 */

#ifndef SERVICES_FIELD_TRIALS_INCLUDE_OBSERVATION_H_
#define SERVICES_FIELD_TRIALS_INCLUDE_OBSERVATION_H_

#include "dfw_field_trial_service_data.h"
#include "dfw_field_trial_service_library.h"
#include "measured_variable.h"



typedef enum ObservationNature
{
	ON_UNSET = -1,
	ON_ROW,
	ON_EXPERIMENTAL_AREA,
	ON_NUM_PHENOTYPE_NATURES
} ObservationNature;



/**
 * A datatype for storing a phneotypic observation within
 * an experiment.
 *
 * @ingroup field_trials_service
 */
typedef struct BaseObservation
{
	bson_oid_t *ob_id_p;

	/**
	 * The date and time when the measuring for this Observation was started.
	 * This can be <code>NULL</code> meaning that a date isn't applicable.
	 */
	struct tm *ob_start_date_p;

	/**
	 * The date and time when the measuring for this Observation was started.
	 * This can be <code>NULL</code> meaning that a date isn't applicable.
	 */
	struct tm *ob_end_date_p;

	/**
	 * The MeasuredVariable that this Observation is measuring.
	 */
	MeasuredVariable *ob_phenotype_p;


	MEM_FLAG ob_phenotype_mem;

	Instrument *ob_instrument_p;

	/**
	 *
	 */
	char *ob_growth_stage_s;

	char *ob_method_s;

	ObservationNature ob_type;

	/**
	 * For some phenotype data multiple data points are taken from the same plot e.g.
	 * yield data from two separate strips within a plot. So having the ability to
	 * store the same phenotypic term but with more than one data point is needed.
	 *
	 * By default this value is set to OB_DEFAULT_INDEX which means that this is
	 * the first, and most often, the only data point for a given non-timed
	 * phenotypic observation.
	 */
	uint32 ob_index;

	ParameterType ob_datatype;

} BaseObservation;


/**
 * A datatype for storing a phneotypic observation within
 * an experiment.
 *
 * @ingroup field_trials_service
 */
typedef struct NumericObservation
{
	BaseObservation no_base_observation;

	/**
	 * The raw phenotypic value for this Observation.
	 */
	double64 *no_raw_value_p;

	/**
	 * The corrected phenotypic value for this Observation.
	 */
	double64 *no_corrected_value_p;


} NumericObservation;


/**
 * A datatype for storing a phneotypic observation within
 * an experiment.
 *
 * @ingroup field_trials_service
 */
typedef struct TextObservation
{
	BaseObservation to_base_observation;

	/**
	 * The raw phenotypic value for this Observation.
	 */
	char *to_raw_value_s;

	/**
	 * The corrected phenotypic value for this Observation.
	 */
	char *to_corrected_value_s;


} TextObservation;


typedef struct ObservationNode
{
	ListItem on_node;

	Observation *on_observation_p;
} ObservationNode;



#ifndef DOXYGEN_SHOULD_SKIP_THIS

#ifdef ALLOCATE_OBSERVATION_TAGS
	#define OBSERVATION_PREFIX DFW_FIELD_TRIAL_SERVICE_LOCAL
	#define OBSERVATION_VAL(x)	= x
	#define OBSERVATION_CONCAT_VAL(x,y)	= x y
#else
	#define OBSERVATION_PREFIX extern
	#define OBSERVATION_VAL(x)
	#define OBSERVATION_CONCAT_VAL(x,y)
#endif

#endif 		/* #ifndef DOXYGEN_SHOULD_SKIP_THIS */


OBSERVATION_PREFIX const char *OB_PHENOTYPE_S OBSERVATION_VAL ("phenotype");

OBSERVATION_PREFIX const char *OB_PHENOTYPE_ID_S OBSERVATION_VAL ("phenotype_id");

OBSERVATION_PREFIX const char *OB_START_DATE_S OBSERVATION_VAL ("date");

OBSERVATION_PREFIX const char *OB_END_DATE_S OBSERVATION_VAL ("end_date");

OBSERVATION_PREFIX const char *OB_RAW_VALUE_S OBSERVATION_VAL ("raw_value");

OBSERVATION_PREFIX const char *OB_INSTRUMENT_ID_S OBSERVATION_VAL ("instrument_id");

OBSERVATION_PREFIX const char *OB_INSTRUMENT_S OBSERVATION_VAL ("instrument");

OBSERVATION_PREFIX const char *OB_GROWTH_STAGE_S OBSERVATION_VAL ("growth_stage");

OBSERVATION_PREFIX const char *OB_CORRECTED_VALUE_S OBSERVATION_VAL ("corrected_value");

OBSERVATION_PREFIX const char *OB_METHOD_S OBSERVATION_VAL ("method");

OBSERVATION_PREFIX const char *OB_NATURE_S OBSERVATION_VAL ("nature");

OBSERVATION_PREFIX const char *OB_INDEX_S OBSERVATION_VAL ("index");


OBSERVATION_PREFIX const uint32 OB_DEFAULT_INDEX OBSERVATION_VAL (1);


#ifdef __cplusplus
extern "C"
{
#endif


DFW_FIELD_TRIAL_SERVICE_LOCAL Observation *AllocateObservation (bson_oid_t *id_p, const struct tm *start_date_p, const struct tm *end_date_p, MeasuredVariable *phenotype_p, MEM_FLAG phenotype_mem, const char *raw_value_s, const char *corrected_value_s,
																	const char *growth_stage_s, const char *method_s, Instrument *instrument_p, const ObservationNature nature, const uint32 *index_p);


/**
 * Free an Observation
 *
 * @param observation_p The Observation to free.
 * @ingroup field_trials_service
 */
DFW_FIELD_TRIAL_SERVICE_LOCAL void FreeObservation (Observation *observation_p);


/**
 * Allocate an ObservationNode for a given Observation.
 *
 * @param observation_p The Observation that will be stored upon the newly-allocated ObservationNode.
 * @return The newly-allocated ObservationNode or <code>NULL</code> upon error.
 * @ingroup field_trials_service
 */
DFW_FIELD_TRIAL_SERVICE_LOCAL ObservationNode *AllocateObservationNode (Observation *observation_p);


/**
 * Free an ObservationNode
 *
 * @param node_p The ObservationNode to free. This will also call FreeObservation() on the
 * ObservationNode's Observation too.
 * @ingroup field_trials_service
 */
DFW_FIELD_TRIAL_SERVICE_LOCAL void FreeObservationNode (ListItem *node_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL json_t *GetObservationAsJSON (const Observation *observation_p, const ViewFormat format);

DFW_FIELD_TRIAL_SERVICE_LOCAL Observation *GetObservationFromJSON (const json_t *phenotype_json_p, FieldTrialServiceData *data_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL bool SaveObservation (Observation *observation_p, const FieldTrialServiceData *data_p);


/**
 * Test whether two Observations are the same in terms of phenotypes, dates and observation indexes
 *
 * @param observation_0_p The first Observation.
 * @param observation_1_p The second Observation.
 * @return <code>true</code> if the Observations match, <code>false</code> otherwise.
 * @ingroup field_trials_service
 */
DFW_FIELD_TRIAL_SERVICE_LOCAL bool AreObservationsMatching (const Observation *observation_0_p, const Observation *observation_1_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool AreObservationsMatchingByParts (const Observation *observation_p, const MeasuredVariable *variable_p, const struct tm *start_date_p, const struct tm *end_date_p, const uint32 index);


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
 * @ingroup field_trials_service
 */
DFW_FIELD_TRIAL_SERVICE_LOCAL bool SetObservationRawValue (Observation *observation_p, const char *value_s);


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
 * @ingroup field_trials_service
 */
DFW_FIELD_TRIAL_SERVICE_LOCAL bool SetObservationCorrectedValue (Observation *observation_p, const char *value_s);

#ifdef __cplusplus
}
#endif



#endif /* SERVICES_FIELD_TRIALS_INCLUDE_OBSERVATION_H_ */

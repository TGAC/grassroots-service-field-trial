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


typedef struct Observation
{
	bson_oid_t *ob_id_p;

	char *ob_raw_value_s;

	char *ob_corrected_value_s;

	struct tm *ob_start_date_p;

	struct tm *ob_end_date_p;

	MeasuredVariable *ob_phenotype_p;

	Instrument *ob_instrument_p;

	char *ob_growth_stage_s;

	char *ob_method_s;

	ObservationNature ob_type;

} Observation;


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



#ifdef __cplusplus
extern "C"
{
#endif


DFW_FIELD_TRIAL_SERVICE_LOCAL Observation *AllocateObservation (bson_oid_t *id_p, const struct tm *start_date_p, const struct tm *end_date_p, MeasuredVariable *phenotype_p, const char *value_s, const char *corrected_value_s,
																	const char *growth_stage_s, const char *method_s, Instrument *instrument_p, const ObservationNature nature);


DFW_FIELD_TRIAL_SERVICE_LOCAL void FreeObservation (Observation *observation_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL ObservationNode *AllocateObservationNode (Observation *observation_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL void FreeObservationNode (ListItem *node_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL json_t *GetObservationAsJSON (const Observation *observation_p, const ViewFormat format);

DFW_FIELD_TRIAL_SERVICE_LOCAL Observation *GetObservationFromJSON (const json_t *phenotype_json_p, const FieldTrialServiceData *data_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL bool SaveObservation (Observation *observation_p, const FieldTrialServiceData *data_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL bool SetObservationValue (Observation *observation_p, const char *value_s);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool AreObservationsMatching (const Observation *observation_0_p, const Observation *observation_1_p);


#ifdef __cplusplus
}
#endif



#endif /* SERVICES_FIELD_TRIALS_INCLUDE_OBSERVATION_H_ */

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
 * instrument.h
 *
 *  Created on: 11 Sep 2018
 *      Author: billy
 */

#ifndef SERVICES_FIELD_TRIALS_INCLUDE_INSTRUMENT_H_
#define SERVICES_FIELD_TRIALS_INCLUDE_INSTRUMENT_H_



#include "dfw_field_trial_service_data.h"
#include "dfw_field_trial_service_library.h"
#include "typedefs.h"


typedef struct Instrument
{
	bson_oid_t *in_id_p;

	char *in_name_s;

	char *in_model_s;

} Instrument;



#ifndef DOXYGEN_SHOULD_SKIP_THIS

#ifdef ALLOCATE_INSTRUMENT_TAGS
	#define INSTRUMENT_PREFIX DFW_FIELD_TRIAL_SERVICE_LOCAL
	#define INSTRUMENT_VAL(x)	= x
	#define INSTRUMENT_CONCAT_VAL(x,y)	= x y
#else
	#define INSTRUMENT_PREFIX extern
	#define INSTRUMENT_VAL(x)
	#define INSTRUMENT_CONCAT_VAL(x,y)
#endif

#endif 		/* #ifndef DOXYGEN_SHOULD_SKIP_THIS */


INSTRUMENT_PREFIX const char *IN_NAME_S INSTRUMENT_CONCAT_VAL (CONTEXT_PREFIX_SCHEMA_ORG_S, "name");

INSTRUMENT_PREFIX const char *IN_MODEL_S INSTRUMENT_VAL ("model");


#ifdef __cplusplus
extern "C"
{
#endif



DFW_FIELD_TRIAL_SERVICE_LOCAL Instrument *AllocateInstrument (bson_oid_t *id_p, const char *name_s, const char *model_s);

DFW_FIELD_TRIAL_SERVICE_LOCAL void FreeInstrument (Instrument *instrument_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL json_t *GetInstrumentAsJSON (const Instrument *instrument_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL Instrument *GetInstrumentFromJSON (const json_t *phenotype_json_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool SaveInstrument (Instrument *instrument_p, const FieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL Instrument *GetInstrumentById (const bson_oid_t *instrument_id_p, const FieldTrialServiceData *data_p);

#ifdef __cplusplus
}
#endif




#endif /* SERVICES_FIELD_TRIALS_INCLUDE_INSTRUMENT_H_ */

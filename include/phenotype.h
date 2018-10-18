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
 * phenotype.h
 *
 *  Created on: 11 Sep 2018
 *      Author: billy
 */

#ifndef SERVICES_DFW_FIELD_TRIAL_SERVICE_INCLUDE_PHENOTYPE_H_
#define SERVICES_DFW_FIELD_TRIAL_SERVICE_INCLUDE_PHENOTYPE_H_

#include <time.h>

#include "dfw_field_trial_service_data.h"
#include "dfw_field_trial_service_library.h"
#include "instrument.h"
#include "typedefs.h"
#include "jansson.h"



typedef struct Phenotype
{
	bson_oid_t *ph_id_p;

	char *ph_trait_s;

	char *ph_trait_abbreviation_s;

	char *ph_measurement_s;

	char *ph_unit_s;

	struct tm *ph_date_p;

	Instrument *ph_instrument_p;

	char *ph_growth_stage_s;

	bool ph_corrected_flag;

	char *ph_method_s;

} Phenotype;


#ifndef DOXYGEN_SHOULD_SKIP_THIS

#ifdef ALLOCATE_PHENOTYPE_TAGS
	#define PHENOTYPE_PREFIX DFW_FIELD_TRIAL_SERVICE_LOCAL
	#define PHENOTYPE_VAL(x)	= x
	#define PHENOTYPE_CONCAT_VAL(x,y)	= x y
#else
	#define PHENOTYPE_PREFIX extern
	#define PHENOTYPE_VAL(x)
	#define PHENOTYPE_CONCAT_VAL(x,y)
#endif

#endif 		/* #ifndef DOXYGEN_SHOULD_SKIP_THIS */


PHENOTYPE_PREFIX const char *PH_TRAIT_S PHENOTYPE_VAL ("trait");

PHENOTYPE_PREFIX const char *PH_TRAIT_ABBREVIATION_S PHENOTYPE_VAL ("trait_abbreviation");

PHENOTYPE_PREFIX const char *PH_MEASUREMENT_S PHENOTYPE_VAL ("measurement");

PHENOTYPE_PREFIX const char *PH_UNIT_S PHENOTYPE_VAL ("unit");

PHENOTYPE_PREFIX const char *PH_DATE_S PHENOTYPE_VAL ("date");

PHENOTYPE_PREFIX const char *PH_INSTRUMENT_ID_S PHENOTYPE_VAL ("instrument_id");

PHENOTYPE_PREFIX const char *PH_INSTRUMENT_S PHENOTYPE_VAL ("instrument");

PHENOTYPE_PREFIX const char *PH_GROWTH_STAGE_S PHENOTYPE_VAL ("growth_stage");

PHENOTYPE_PREFIX const char *PH_CORRECTED_S PHENOTYPE_VAL ("corrected");

PHENOTYPE_PREFIX const char *PH_METHOD_S PHENOTYPE_VAL ("method");

#ifdef __cplusplus
extern "C"
{
#endif



DFW_FIELD_TRIAL_SERVICE_LOCAL Phenotype *AllocatePhenotype (bson_oid_t *id_p, const struct tm *date_p, const char *trait_s, const char *trait_abbreviation_s, const char *measurement_s,
																														const char *unit_s, const char *growth_stage_s, const bool corrected_value_flag, const char *method_s, Instrument *instrument_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL void FreePhenotype (Phenotype *phenotype_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL json_t *GetPhenotypeAsJSON (const Phenotype *phenotype_p, const bool expand_fields_flag);

DFW_FIELD_TRIAL_SERVICE_LOCAL Phenotype *GetPhenotypeFromJSON (const json_t *phenotype_json_p, const DFWFieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool SavePhenotype (Phenotype *phenotype_p, const DFWFieldTrialServiceData *data_p, bool corrected_value_flag);


#ifdef __cplusplus
}
#endif



#endif /* SERVICES_DFW_FIELD_TRIAL_SERVICE_INCLUDE_PHENOTYPE_H_ */

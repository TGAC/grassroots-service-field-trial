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


#include "instrument.h"
#include "typedefs.h"




typedef struct Phenotype
{
	uint32 ph_id;

	char *ph_trait_s;

	char *ph_trait_abbreviation_s;

	char *ph_measurement_s;

	char *ph_unit_s;

	char *ph_date_s;

	Instrument *ph_instrument_p;

	char *growth_stage_s;

} Phenotype;

#endif /* SERVICES_DFW_FIELD_TRIAL_SERVICE_INCLUDE_PHENOTYPE_H_ */

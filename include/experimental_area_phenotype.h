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
 * experimental_area_phenotype.h
 *
 *  Created on: 11 Sep 2018
 *      Author: billy
 */

#ifndef SERVICES_FIELD_TRIALS_INCLUDE_EXPERIMENTAL_AREA_PHENOTYPE_H_
#define SERVICES_FIELD_TRIALS_INCLUDE_EXPERIMENTAL_AREA_PHENOTYPE_H_


#include "experimental_area.h"
#include "typedefs.h"
#include "treatment.h"


typedef struct ExperimentalAreaPhenotype
{
	uint32 eap_id;

	Treatment *eap_phenotype_p;

	bool eap_corrected_flag;

	char *eap_method_s;

	Study *eap_area_p;

} ExperimentalAreaPhenotype;


#endif /* SERVICES_FIELD_TRIALS_INCLUDE_EXPERIMENTAL_AREA_PHENOTYPE_H_ */

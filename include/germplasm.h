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
 * germplasm.h
 *
 *  Created on: 23 Jul 2018
 *      Author: billy
 */

#ifndef SERVICES_DFW_FIELD_TRIAL_SERVICE_INCLUDE_GERMPLASM_H_
#define SERVICES_DFW_FIELD_TRIAL_SERVICE_INCLUDE_GERMPLASM_H_


#include "dfw_field_trial_service_library.h"
#include "typedefs.h"

#include "jansson.h"


typedef struct Germplasm
{
	char **ge_sources_ss;

	char **ge_accessions_ss;

	uint32 ge_num_crosses;
} Germplasm;



#ifdef __cplusplus
extern "C"
{
#endif

DFW_FIELD_TRIAL_SERVICE_LOCAL Germplasm *AllocateGermplasm ();


DFW_FIELD_TRIAL_SERVICE_LOCAL void FreeGermplasm (Germplasm *germplasm_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL json_t *GetGermplasmAsJSON (const Germplasm *germplasm_p);


#ifdef __cplusplus
}
#endif

#endif /* SERVICES_DFW_FIELD_TRIAL_SERVICE_INCLUDE_GERMPLASM_H_ */

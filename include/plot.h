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
 * plot.h
 *
 *  Created on: 23 Jul 2018
 *      Author: billy
 */

#ifndef SERVICES_DFW_FIELD_TRIAL_SERVICE_INCLUDE_PLOT_H_
#define SERVICES_DFW_FIELD_TRIAL_SERVICE_INCLUDE_PLOT_H_

#include "typedefs.h"

typedef struct Plot
{
	uint32 pl_id;

	uint32 pl_sowing_date;

	uint32 pl_harvest_date;

	char *pl_row_factor_s;

	bool pl_replication_flag;

	char *pl_trial_design_s;

	Size pl_size;

	char *pl_growing_conditions_s;

	char *pl_treatments_s;


} Plot;


#ifdef __cplusplus
extern "C"
{
#endif


#ifdef __cplusplus
}
#endif



#endif /* SERVICES_DFW_FIELD_TRIAL_SERVICE_INCLUDE_PLOT_H_ */

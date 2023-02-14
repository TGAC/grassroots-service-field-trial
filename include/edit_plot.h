/*
** Copyright 2014-2023 The Earlham Institute
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
 * submit_plot.h
 *
 *  Created on: 9 Feb 2023
 *      Author: billy
 */

#ifndef SERVICES_FIELD_TRIALS_INCLUDE_EDIT_PLOT_H_
#define SERVICES_FIELD_TRIALS_INCLUDE_EDIT_PLOT_H_

#include "dfw_field_trial_service_data.h"
#include "dfw_field_trial_service_library.h"



#ifdef __cplusplus
extern "C"
{
#endif


DFW_FIELD_TRIAL_SERVICE_LOCAL Service *GetPlotEditingService (GrassrootsServer *grassroots_p);


#ifdef __cplusplus
}
#endif


#endif /* SERVICES_FIELD_TRIALS_INCLUDE_EDIT_PLOT_H_ */

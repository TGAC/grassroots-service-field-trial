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
 * indexing.h
 *
 *  Created on: 23 Aug 2019
 *      Author: billy
 */

#ifndef SERVICES_DFW_FIELD_TRIAL_SERVICE_INCLUDE_INDEXING_H_
#define SERVICES_DFW_FIELD_TRIAL_SERVICE_INCLUDE_INDEXING_H_


#include "dfw_field_trial_service_data.h"
#include "dfw_field_trial_service_library.h"
#include "lucene_tool.h"


#ifdef __cplusplus
extern "C"
{
#endif


DFW_FIELD_TRIAL_SERVICE_LOCAL	bool IndexData (ServiceJob *job_p, const json_t *data_to_index_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL OperationStatus ReindexAllData (ServiceJob *job_p, const DFWFieldTrialServiceData *service_data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool ReindexStudies (ServiceJob *job_p, LuceneTool *lucene_p, bool update_flag, const DFWFieldTrialServiceData *service_data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool ReindexTrials (ServiceJob *job_p, LuceneTool *lucene_p, bool update_flag, const DFWFieldTrialServiceData *service_data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool ReindexLocations (ServiceJob *job_p, LuceneTool *lucene_p, bool update_flag, const DFWFieldTrialServiceData *service_data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool ReindexMeasuredVariables (ServiceJob *job_p, LuceneTool *lucene_p, bool update_flag, const DFWFieldTrialServiceData *service_data_p);


#ifdef __cplusplus
}
#endif



#endif /* SERVICES_DFW_FIELD_TRIAL_SERVICE_INCLUDE_INDEXING_H_ */

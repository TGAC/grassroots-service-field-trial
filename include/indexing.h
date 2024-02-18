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

#ifndef SERVICES_FIELD_TRIALS_INCLUDE_INDEXING_H_
#define SERVICES_FIELD_TRIALS_INCLUDE_INDEXING_H_


#include "dfw_field_trial_service_data.h"
#include "dfw_field_trial_service_library.h"
#include "lucene_tool.h"


#ifdef __cplusplus
extern "C"
{
#endif


DFW_FIELD_TRIAL_SERVICE_LOCAL	Service *GetFieldTrialIndexingService (GrassrootsServer *grassroots_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL OperationStatus ReindexAllData (ServiceJob *job_p, const bool update_flag, const FieldTrialServiceData *service_data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL OperationStatus ReindexStudies (ServiceJob *job_p, LuceneTool *lucene_p, bool update_flag, const FieldTrialServiceData *service_data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL OperationStatus ReindexTrials (ServiceJob *job_p, LuceneTool *lucene_p, bool update_flag, const FieldTrialServiceData *service_data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL OperationStatus ReindexLocations (ServiceJob *job_p, LuceneTool *lucene_p, bool update_flag, const FieldTrialServiceData *service_data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL OperationStatus ReindexMeasuredVariables (ServiceJob *job_p, LuceneTool *lucene_p, bool update_flag, const FieldTrialServiceData *service_data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL OperationStatus ReindexProgrammes (ServiceJob *job_p, LuceneTool *lucene_p, bool update_flag, const FieldTrialServiceData *service_data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL OperationStatus ReindexTreatments (ServiceJob *job_p, LuceneTool *lucene_p, bool update_flag, const FieldTrialServiceData *service_data_p);


#ifdef __cplusplus
}
#endif



#endif /* SERVICES_FIELD_TRIALS_INCLUDE_INDEXING_H_ */

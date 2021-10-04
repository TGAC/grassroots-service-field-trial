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
 * row_jobs.h
 *
 *  Created on: 28 Oct 2018
 *      Author: billy
 */

#ifndef SERVICES_FIELD_TRIALS_INCLUDE_ROW_JOBS_H_
#define SERVICES_FIELD_TRIALS_INCLUDE_ROW_JOBS_H_

#include "dfw_field_trial_service_data.h"
#include "dfw_field_trial_service_library.h"

#include "row.h"
#include "plot.h"
#include "treatment_factor_value.h"



#ifdef __cplusplus
extern "C"
{
#endif



DFW_FIELD_TRIAL_SERVICE_LOCAL bool AddSubmissionRowPhenotypeParams (ServiceData *data_p, ParameterSet *param_set_p, Resource *resource_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool RunForSubmissionRowPhenotypeParams (FieldTrialServiceData *data_p, ParameterSet *param_set_p, ServiceJob *job_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool GetSubmissionRowPhenotypeParameterTypeForNamedParameter (const char *param_name_s, ParameterType *pt_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL Row *GetRowByRackIndex (const int32 row, Plot *plot_p, const bool expand_fields_flag, const FieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL Row *GetRowByStudyIndex (const int32 by_study_index, Study *study_p, const FieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL LinkedList *GetAllRowsContainingMaterial (Material *material_p, const FieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL OperationStatus AddObservationValuesToRow (Row *row_p, json_t *observation_json_p, Study *study_p, ServiceJob *job_p, const uint32 row_index, const FieldTrialServiceData *data_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL OperationStatus AddTreatmentFactorValuesToRow (Row *row_p, json_t *plot_json_p, Study *study_p, const FieldTrialServiceData *data_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL bool AddTreatmentFactorValueToRowByParts (Row *row_p, TreatmentFactor *tf_p, const char *value_s);

DFW_FIELD_TRIAL_SERVICE_LOCAL json_t *GetRowAsFrictionlessData (const Row *row_p, const Study * const study_p, const FieldTrialServiceData *service_data_p, const char * const null_sequence_s);

DFW_FIELD_TRIAL_SERVICE_LOCAL bool AddRowFrictionlessDataDetails (const Row *row_p, json_t *row_fd_p, const FieldTrialServiceData *service_data_p, const char * const null_sequence_s);


DFW_FIELD_TRIAL_SERVICE_LOCAL Observation *GetMatchingObservation (const Row *row_p, const MeasuredVariable *variable_p, const struct tm *start_date_p, const struct tm *end_date_p, const uint32 index);


#ifdef __cplusplus
}
#endif



#endif /* SERVICES_FIELD_TRIALS_INCLUDE_ROW_JOBS_H_ */

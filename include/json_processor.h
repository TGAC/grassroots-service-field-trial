/*
 * json_processor.h
 *
 *  Created on: 2 Dec 2019
 *      Author: billy
 */

#ifndef SERVICES_FIELD_TRIALS_INCLUDE_JSON_PROCESSOR_H_
#define SERVICES_FIELD_TRIALS_INCLUDE_JSON_PROCESSOR_H_

#include "dfw_field_trial_service_data.h"
#include "dfw_field_trial_service_library.h"
#include "jansson.h"


struct Plot;

struct JSONProcessor;


typedef struct JSONProcessor
{

	json_t *(*jp_process_plot_json_fn) (struct JSONProcessor *processor_p, struct Plot *plot_p, ViewFormat format, const DFWFieldTrialServiceData *service_data_p);

	void (*jp_free_fn) (struct JSONProcessor *processor_p);
} JSONProcessor;

#ifdef __cplusplus
extern "C"
{
#endif



DFW_FIELD_TRIAL_SERVICE_LOCAL void FreeJSONProcessor (struct JSONProcessor *processor_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL json_t *ProcessPlotJSON (struct JSONProcessor *processor_p, struct Plot *plot_p, ViewFormat format, const DFWFieldTrialServiceData *service_data_p);


#ifdef __cplusplus
}
#endif

#endif /* SERVICES_FIELD_TRIALS_INCLUDE_JSON_PROCESSOR_H_ */

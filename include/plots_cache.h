/*
 * plots_cache.h
 *
 *  Created on: 28 Jun 2022
 *      Author: billy
 */

#ifndef SERVICES_FIELD_TRIALS_INCLUDE_PLOTS_CACHE_H_
#define SERVICES_FIELD_TRIALS_INCLUDE_PLOTS_CACHE_H_

#include "jansson.h"


#include "dfw_field_trial_service_data.h"
#include "dfw_field_trial_service_library.h"


typedef struct
{
	json_t *pc_grid_cache_p;

	json_t *pc_index_cache_p;
} PlotsCache;



#ifdef __cplusplus
extern "C"
{
#endif



DFW_FIELD_TRIAL_SERVICE_LOCAL PlotsCache *AllocatePlotsCache (void);

DFW_FIELD_TRIAL_SERVICE_LOCAL void FreePlotsCache (PlotsCache *plots_cache_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL bool CheckPlotRequirements (PlotsCache *plots_cache_p, const json_t *table_row_json_p, const size_t row_index, ServiceJob *job_p, int32 *row_p, int32 *column_p, int32 *index_p);


#ifdef __cplusplus
}
#endif

#endif /* SERVICES_FIELD_TRIALS_INCLUDE_PLOTS_CACHE_H_ */

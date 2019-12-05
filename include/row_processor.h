/*
 * row_processor.h
 *
 *  Created on: 5 Dec 2019
 *      Author: billy
 */

#ifndef SERVICES_FIELD_TRIALS_INCLUDE_ROW_PROCESSOR_H_
#define SERVICES_FIELD_TRIALS_INCLUDE_ROW_PROCESSOR_H_

#include "json_processor.h"




typedef struct RowProcessor
{
	JSONProcessor rp_base_processor;

	struct Material *rp_material_p;

} RowProcessor;



#ifdef __cplusplus
extern "C"
{
#endif


DFW_FIELD_TRIAL_SERVICE_LOCAL JSONProcessor *AllocateRowProcessor (struct Material *material_p);


#ifdef __cplusplus
}
#endif


#endif /* SERVICES_FIELD_TRIALS_INCLUDE_ROW_PROCESSOR_H_ */

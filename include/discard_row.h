/*
 * discard_row.h
 *
 *  Created on: 30 Jun 2022
 *      Author: billy
 */

#ifndef SERVICES_FIELD_TRIALS_INCLUDE_DISCARD_ROW_H_
#define SERVICES_FIELD_TRIALS_INCLUDE_DISCARD_ROW_H_

#include "base_row.h"


typedef struct DiscardRow
{
	BaseRow dr_base;
} DiscardRow;


#ifdef __cplusplus
extern "C"
{
#endif


DFW_FIELD_TRIAL_SERVICE_LOCAL BaseRow *AllocateDiscardRow (bson_oid_t *id_p, const uint32 study_index, Plot *parent_plot_p);


#ifdef __cplusplus
}
#endif

#endif /* SERVICES_FIELD_TRIALS_INCLUDE_DISCARD_ROW_H_ */

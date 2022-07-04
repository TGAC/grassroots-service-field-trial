/*
 * discard_row.h
 *
 *  Created on: 30 Jun 2022
 *      Author: billy
 */

#ifndef SERVICES_FIELD_TRIALS_INCLUDE_DISCARD_ROW_H_
#define SERVICES_FIELD_TRIALS_INCLUDE_DISCARD_ROW_H_

#include "row.h"


typedef struct DiscardRow
{
	Row dr_base;
} DiscardRow;


#ifdef __cplusplus
extern "C"
{
#endif


DFW_FIELD_TRIAL_SERVICE_LOCAL Row *AllocateDiscardRow (bson_oid_t *id_p, const uint32 study_index, Plot *parent_plot_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL DiscardRow *GetDiscardRowFromJSON (const json_t *row_json_p, Plot *plot_p, const Study *study_p, const ViewFormat format, FieldTrialServiceData *data_p);


#ifdef __cplusplus
}
#endif

#endif /* SERVICES_FIELD_TRIALS_INCLUDE_DISCARD_ROW_H_ */

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
 * row.h
 *
 *  Created on: 11 Sep 2018
 *      Author: billy
 */

#ifndef SERVICES_FIELD_TRIALS_INCLUDE_ROW_H_
#define SERVICES_FIELD_TRIALS_INCLUDE_ROW_H_


#include "material.h"
#include "plot.h"
#include "observation.h"
#include "treatment_factor_value.h"


struct Study;


typedef struct Row
{
	bson_oid_t *ro_id_p;

	const Study *ro_study_p;

	bson_oid_t *ro_study_id_p;


	Plot *ro_plot_p;

	/**
	 * The unique index for this row within
	 * its parent study.
	 */
	uint32 ro_by_study_index;

	/**
	 * The row factor/category for this rack within
	 * its parent plot.
	 */
	uint32 ro_rack_index;

	Material *ro_material_p;

	MEM_FLAG ro_material_mem;

	LinkedList *ro_observations_p;

	LinkedList *ro_treatment_factor_values_p;

	uint32 ro_replicate_index;

	bool ro_replicate_control_flag;



} Row;


typedef struct RowNode
{
	ListItem rn_node;

	Row *rn_row_p;
} RowNode;


#ifndef DOXYGEN_SHOULD_SKIP_THIS

#ifdef ALLOCATE_ROW_TAGS
	#define ROW_PREFIX DFW_FIELD_TRIAL_SERVICE_API
	#define ROW_VAL(x)	= x
	#define ROW_CONCAT_VAL(x,y)	= x y
#else
	#define ROW_PREFIX extern
	#define ROW_VAL(x)
	#define ROW_CONCAT_VAL(x,y)
#endif

#endif 		/* #ifndef DOXYGEN_SHOULD_SKIP_THIS */


ROW_PREFIX const char *RO_ID_S ROW_VAL ("id");

ROW_PREFIX const char *RO_RACK_INDEX_S ROW_VAL ("rack_index");

ROW_PREFIX const char *RO_STUDY_INDEX_S ROW_VAL ("study_index");

ROW_PREFIX const char *RO_PLOT_ID_S ROW_VAL ("plot_id");

ROW_PREFIX const char *RO_STUDY_ID_S ROW_VAL ("study_id");

ROW_PREFIX const char *RO_REPLICATE_S ROW_VAL ("replicate");

ROW_PREFIX const char *RO_MATERIAL_ID_S ROW_VAL ("material_id");

/**
 * The internal name for a field trial's material. This is
 * used as a temporary holding value until the material pointer
 * has been linked up
 */
ROW_PREFIX const char *RO_TRIAL_MATERIAL_S ROW_VAL ("trial_material");


ROW_PREFIX const char *RO_MATERIAL_S ROW_VAL ("material");


ROW_PREFIX const char *RO_OBSERVATIONS_S  ROW_VAL ("observations");

ROW_PREFIX const char *RO_TREATMENTS_S  ROW_VAL ("treatments");


ROW_PREFIX const char *RO_REPLICATE_CONTROL_S ROW_VAL ("control");



ROW_PREFIX const char *RO_IMPORT_RACK_S ROW_VAL ("Rack");


ROW_PREFIX const char *RO_PLOT_INDEX_S ROW_VAL ("Plot index");


#ifdef __cplusplus
extern "C"
{
#endif


DFW_FIELD_TRIAL_SERVICE_LOCAL Row *AllocateRow (bson_oid_t *id_p, const uint32 rack_index, const uint32 study_index, const uint32 replicate, Material *material_p, MEM_FLAG material_mem, Plot *parent_plot_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL void FreeRow (Row *row_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL RowNode *AllocateRowNode (Row *row_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL void FreeRowNode (ListItem *node_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL json_t *GetRowAsJSON (const Row *row_p, const ViewFormat format, JSONProcessor *processor_p, const FieldTrialServiceData *data_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL Row *GetRowFromJSON (const json_t *json_p, Plot *plot_p, Material *material_p, const struct Study *study_p, const ViewFormat format, FieldTrialServiceData *data_p);

//DFW_FIELD_TRIAL_SERVICE_LOCAL bool SaveRow (Row *row_p, const FieldTrialServiceData *data_p, bool insert_flag);

DFW_FIELD_TRIAL_SERVICE_LOCAL bool AddObservationToRow (Row *row_p, Observation *observation_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL void SetRowGenotypeControl (Row *row_p, bool control_flag);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool IsRowGenotypeControl (const Row *row_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL void UpdateRow (Row *row_p, const uint32 rack_plotwise_index, Material *material_p, MEM_FLAG material_mem, const bool control_rep_flag, const uint32 replicate);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool AddTreatmentFactorValueToRow (Row *row_p, TreatmentFactorValue *tf_value_p);

#ifdef __cplusplus
}
#endif


#endif /* SERVICES_FIELD_TRIALS_INCLUDE_ROW_H_ */

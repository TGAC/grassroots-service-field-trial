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

#ifndef SERVICES_DFW_FIELD_TRIAL_SERVICE_INCLUDE_ROW_H_
#define SERVICES_DFW_FIELD_TRIAL_SERVICE_INCLUDE_ROW_H_


#include "material.h"
#include "plot.h"




typedef struct Row
{
	bson_oid_t *ro_id_p;

	Plot *ro_plot_p;

	uint32 ro_index;

	Material *ro_material_p;

	char *ro_material_s;

	LinkedList *ro_phenotypes_p;

} Row;


typedef struct RowNode
{
	ListItem rn_node;

	Row *rn_row_p;
} RowNode;


#ifndef DOXYGEN_SHOULD_SKIP_THIS

#ifdef ALLOCATE_ROW_TAGS
	#define ROW_PREFIX DFW_FIELD_TRIAL_SERVICE_LOCAL
	#define ROW_VAL(x)	= x
	#define ROW_CONCAT_VAL(x,y)	= x y
#else
	#define ROW_PREFIX extern
	#define ROW_VAL(x)
	#define ROW_CONCAT_VAL(x,y)
#endif

#endif 		/* #ifndef DOXYGEN_SHOULD_SKIP_THIS */


ROW_PREFIX const char *RO_ID_S ROW_VAL ("id");

ROW_PREFIX const char *RO_INDEX_S ROW_VAL ("index");

ROW_PREFIX const char *RO_PLOT_ID_S ROW_VAL ("plot_id");

ROW_PREFIX const char *RO_MATERIAL_ID_S ROW_VAL ("material_id");

/**
 * The internal name for a field trial's material. This is
 * used as a temporary holding value until the material pointer
 * has been linked up
 */
ROW_PREFIX const char *RO_TRIAL_MATERIAL_S ROW_VAL ("trial_material_s");


ROW_PREFIX const char *RO_MATERIAL_S ROW_VAL ("material_s");



#ifdef __cplusplus
extern "C"
{
#endif


DFW_FIELD_TRIAL_SERVICE_LOCAL Row *AllocateRow (bson_oid_t *id_p, const uint32 index, Material *material_p, Plot *parent_plot_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL void FreeRow (Row *row_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL RowNode *AllocateRowNode (Row *row_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL void FreeRowNode (ListItem *node_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL json_t *GetRowAsJSON (const Row *row_p, const bool expand_material_flag, const DFWFieldTrialServiceData *data_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL Row *GetRowFromJSON (const json_t *json_p, Plot *plot_p, Material *material_p, const bool expand_fields_flag, const DFWFieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool SaveRow (Row *row_p, const DFWFieldTrialServiceData *data_p);


#ifdef __cplusplus
}
#endif


#endif /* SERVICES_DFW_FIELD_TRIAL_SERVICE_INCLUDE_ROW_H_ */

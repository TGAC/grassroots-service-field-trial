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
 * plot.h
 *
 *  Created on: 23 Jul 2018
 *      Author: billy
 */

#ifndef SERVICES_DFW_FIELD_TRIAL_SERVICE_INCLUDE_PLOT_H_
#define SERVICES_DFW_FIELD_TRIAL_SERVICE_INCLUDE_PLOT_H_

#include "dfw_field_trial_service_library.h"
#include "experimental_area.h"
#include "jansson.h"


#include "linked_list.h"


#ifndef DOXYGEN_SHOULD_SKIP_THIS

#ifdef ALLOCATE_PLOT_TAGS
	#define PLOT_PREFIX DFW_FIELD_TRIAL_SERVICE_LOCAL
	#define PLOT_VAL(x)	= x
	#define PLOT_CONCAT_VAL(x,y)	= x y
#else
	#define PLOT_PREFIX extern
	#define PLOT_VAL(x)
	#define PLOT_CONCAT_VAL(x,y)
#endif

#endif 		/* #ifndef DOXYGEN_SHOULD_SKIP_THIS */


PLOT_PREFIX const char *PL_ID_S PLOT_VAL ("id");

PLOT_PREFIX const char *PL_PARENT_EXPERIMENTAL_AREA_S PLOT_VAL ("parent_experimental_area_id");

PLOT_PREFIX const char *PL_SOWING_DATE_S PLOT_VAL ("sowing_date");

PLOT_PREFIX const char *PL_HARVEST_DATE_S PLOT_VAL ("harvest_date");

PLOT_PREFIX const char *PL_WIDTH_S PLOT_VAL ("width");

PLOT_PREFIX const char *PL_LENGTH_S PLOT_VAL ("length");

PLOT_PREFIX const char *PL_TRIAL_DESIGN_S PLOT_VAL ("trial_desgin");

PLOT_PREFIX const char *PL_GROWING_CONDITION_S PLOT_VAL ("growing_condition");

PLOT_PREFIX const char *PL_TREATMENT_S PLOT_VAL ("treatment");

PLOT_PREFIX const char *PL_ROW_INDEX_S PLOT_VAL ("row_index");

PLOT_PREFIX const char *PL_COLUMN_INDEX_S PLOT_VAL ("column_index");

PLOT_PREFIX const char *PL_ROWS_S PLOT_VAL ("rows");



typedef struct Plot
{
	bson_oid_t *pl_id_p;

	ExperimentalArea *pl_parent_p;

	struct tm *pl_sowing_date_p;

	struct tm *pl_harvest_date_p;

	double64 pl_width;

	double64 pl_length;

	uint32 pl_row_index;

	uint32 pl_column_index;

	char *pl_trial_design_s;

	char *pl_growing_conditions_s;

	char *pl_treatments_s;

	/**
	 * A LinkedList of RowNodes
	 * for all of the Rows in this
	 * Plot.
	 */
	LinkedList *pl_rows_p;

} Plot;


typedef struct PlotNode
{
	ListItem pn_node;

	Plot *pn_plot_p;
} PlotNode;



#ifdef __cplusplus
extern "C"
{
#endif


DFW_FIELD_TRIAL_SERVICE_LOCAL Plot *AllocatePlot (bson_oid_t *id_p, const struct tm *sowing_date_p, const struct tm *harvest_date_p, const double64 width, const double64 length, const uint32 row_index,
																									const uint32 column_index, const char *trial_design_s, const char *growing_conditions_s, const char *treatments_s, ExperimentalArea *parent_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL void FreePlot (Plot *plot_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL PlotNode *AllocatePlotNode (Plot *plot_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL void FreePlotNode (ListItem *node_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL json_t *GetPlotAsJSON (Plot *plot_p, const bool expand_fields_flag, const DFWFieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL Plot *GetPlotFromJSON (const json_t *plot_json_p, ExperimentalArea *parent_area_p, const DFWFieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool GetPlotRows (Plot *plot_p, const DFWFieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool SavePlot (Plot *plot_p, const DFWFieldTrialServiceData *data_p);


#ifdef __cplusplus
}
#endif



#endif /* SERVICES_DFW_FIELD_TRIAL_SERVICE_INCLUDE_PLOT_H_ */

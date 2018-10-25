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
 * experimental_area.h
 *
 *  Created on: 11 Sep 2018
 *      Author: billy
 */

#ifndef SERVICES_DFW_FIELD_TRIAL_SERVICE_INCLUDE_EXPERIMENTAL_AREA_H_
#define SERVICES_DFW_FIELD_TRIAL_SERVICE_INCLUDE_EXPERIMENTAL_AREA_H_


#include <time.h>

#include "dfw_field_trial_service_data.h"
#include "dfw_field_trial_service_library.h"
#include "field_trial.h"
#include "location.h"
#include "jansson.h"

#include "typedefs.h"
#include "address.h"


#ifndef DOXYGEN_SHOULD_SKIP_THIS

#ifdef ALLOCATE_EXPERIMENTAL_AREA_TAGS
	#define EXPERIMENTAL_AREA_PREFIX DFW_FIELD_TRIAL_SERVICE_LOCAL
	#define EXPERIMENTAL_AREA_VAL(x)	= x
	#define EXPERIMENTAL_AREA_CONCAT_VAL(x,y)	= x y
#else
	#define EXPERIMENTAL_AREA_PREFIX extern
	#define EXPERIMENTAL_AREA_VAL(x)
	#define EXPERIMENTAL_AREA_CONCAT_VAL(x,y)
#endif

#endif 		/* #ifndef DOXYGEN_SHOULD_SKIP_THIS */


EXPERIMENTAL_AREA_PREFIX const char *EA_ID_S EXPERIMENTAL_AREA_VAL ("id");

EXPERIMENTAL_AREA_PREFIX const char *EA_NAME_S EXPERIMENTAL_AREA_CONCAT_VAL (CONTEXT_PREFIX_SCHEMA_ORG_S, "name");

EXPERIMENTAL_AREA_PREFIX const char *EA_LOCATION_ID_S EXPERIMENTAL_AREA_VAL ("address_id");

EXPERIMENTAL_AREA_PREFIX const char *EA_LOCATION_S EXPERIMENTAL_AREA_VAL ("address");


EXPERIMENTAL_AREA_PREFIX const char *EA_SOIL_S EXPERIMENTAL_AREA_VAL ("soil");

EXPERIMENTAL_AREA_PREFIX const char *EA_SOWING_DATE_S EXPERIMENTAL_AREA_VAL ("sowing_date");

EXPERIMENTAL_AREA_PREFIX const char *EA_HARVEST_DATE_S EXPERIMENTAL_AREA_VAL ("harvest_date");


EXPERIMENTAL_AREA_PREFIX const char *EA_PARENT_FIELD_TRIAL_S EXPERIMENTAL_AREA_VAL ("parent_field_trial_id");

EXPERIMENTAL_AREA_PREFIX const char *EA_PLOTS_S EXPERIMENTAL_AREA_VAL ("plots");



typedef struct ExperimentalArea
{
	bson_oid_t *ea_id_p;

	FieldTrial *ea_parent_p;

	//char *ea_location_s;
	struct Location *ea_location_p;

	//Address *ea_address_p;

	char *ea_soil_type_s;

	char *ea_name_s;

	struct tm *ea_sowing_date_p;

	struct tm *ea_harvest_date_p;

	/**
	 * A LinkedList of PlotNodes
	 * for all of the Plots in this
	 * ExperimentalArea.
	 */
	LinkedList *ea_plots_p;

} ExperimentalArea;



typedef struct ExperimentalAreaNode
{
	ListItem ean_node;

	ExperimentalArea *ean_experimental_area_p;

} ExperimentalAreaNode;



#ifdef __cplusplus
extern "C"
{
#endif


DFW_FIELD_TRIAL_SERVICE_LOCAL	ExperimentalArea *AllocateExperimentalAreaByIDString (bson_oid_t *id_p, const char *name_s, const char *location_s, const char *soil_s, const uint32 sowing_year, const uint32 harvest_year, const char *parent_field_trial_id_s, const DFWFieldTrialServiceData *data_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL ExperimentalArea *AllocateExperimentalArea (bson_oid_t *id_p, const char *name_s, const char *soil_s, const struct tm *sowing_date_p, const struct tm *harvest_date_p, struct Location *location_p, FieldTrial *parent_field_trial_p, const DFWFieldTrialServiceData *data_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL void FreeExperimentalArea (ExperimentalArea *area_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL ExperimentalAreaNode *AllocateExperimentalAreaNode (ExperimentalArea *area_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL void FreeExperimentalAreaNode (ListItem *node_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL json_t *GetExperimentalAreaAsJSON (ExperimentalArea *area_p, const bool expand_fields_flag, DFWFieldTrialServiceData *data_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL ExperimentalArea *GetExperimentalAreaFromJSON (const json_t *json_p, const bool full_location_flag, const DFWFieldTrialServiceData *data_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL bool GetExperimentalAreaPlots (ExperimentalArea *area_p, DFWFieldTrialServiceData *data_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL bool SaveExperimentalArea (ExperimentalArea *area_p, DFWFieldTrialServiceData *data_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL ExperimentalArea *LoadExperimentalArea (const int32 area_id, DFWFieldTrialServiceData *data_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL ExperimentalArea *GetExperimentalAreaByIdString (const char *area_id_s, const DFWFieldTrialServiceData *data_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL ExperimentalArea *GetExperimentalAreaById (bson_oid_t *area_id_p, const DFWFieldTrialServiceData *data_p);


#ifdef __cplusplus
}
#endif


#endif /* SERVICES_DFW_FIELD_TRIAL_SERVICE_INCLUDE_EXPERIMENTAL_AREA_H_ */

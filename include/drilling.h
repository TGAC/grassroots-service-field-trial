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
 * drilling.h
 *
 *  Created on: 23 Jul 2018
 *      Author: billy
 */

#ifndef SERVICES_DFW_FIELD_TRIAL_SERVICE_INCLUDE_DRILLING_H_
#define SERVICES_DFW_FIELD_TRIAL_SERVICE_INCLUDE_DRILLING_H_


#include "dfw_field_trial_service_library.h"
#include "germplasm.h"
#include "typedefs.h"
#include "coordinate.h"



typedef struct DrillingPlot
{
	bson_oid_t *dp_id_p;

	Germplasm *dr_germplasm_p;

	uint32 dr_row_index;

	uint32 dr_column_index;

	uint32 dr_replicate_index;

	struct tm *dr_sowing_date_p;

	double64 dr_sowing_rate;

} DrillingPlot;



typedef struct Drilling
{
	bson_oid_t *dr_plot_id_p;

	bson_oid_t *dr_parent_study_p;

	Germplasm *dr_germplasm_p;

	uint32 dr_row_index;

	uint32 dr_column_index;

	uint32 dr_replicate_index;

	struct tm *dr_sowing_date_p;

	double64 dr_sowing_rate;

} Drilling;


#ifndef DOXYGEN_SHOULD_SKIP_THIS

#ifdef ALLOCATE_DRILLING_TAGS
	#define DRILLING_PREFIX DFW_FIELD_TRIAL_SERVICE_LOCAL
	#define DRILLING_VAL(x)	= x
	#define DRILLING_CONCAT_VAL(x,y)	= x y
#else
	#define DRILLING_PREFIX extern
	#define DRILLING_VAL(x)
	#define DRILLING_CONCAT_VAL(x,y)
#endif

#endif 		/* #ifndef DOXYGEN_SHOULD_SKIP_THIS */


DRILLING_PREFIX const char *DR_SOWING_RATE_S DRILLING_CONCAT_VAL (CONTEXT_PREFIX_AGRONOMY_ONTOLOGY_S, "00000202");

DRILLING_PREFIX const char *IN_MODEL_S DRILLING_VAL ("model");




#ifdef __cplusplus
extern "C"
{
#endif


#ifdef __cplusplus
}
#endif

#endif /* SERVICES_DFW_FIELD_TRIAL_SERVICE_INCLUDE_DRILLING_H_ */

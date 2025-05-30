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
 * phenotype.h
 *
 *  Created on: 11 Sep 2018
 *      Author: billy
 */

#ifndef SERVICES_FIELD_TRIALS_INCLUDE_MEASURED_VARIABLE_H_
#define SERVICES_FIELD_TRIALS_INCLUDE_MEASURED_VARIABLE_H_

#include <time.h>

#include "dfw_field_trial_service_data.h"
#include "dfw_field_trial_service_library.h"
#include "instrument.h"
#include "typedefs.h"
#include "jansson.h"
#include "schema_term.h"
#include "schema_keys.h"
#include "crop_ontology_tool.h"
#include "crop_ontology.h"


typedef struct MeasuredVariable
{
	bson_oid_t *mv_id_p;

	SchemaTerm *mv_variable_term_p;

	SchemaTerm *mv_trait_term_p;

	SchemaTerm *mv_measurement_term_p;

	SchemaTerm *mv_unit_term_p;

	const ScaleClass *mv_scale_class_p;

	/**
	 * The Ontology that this Measured Variable is a part of.
	 */
	struct CropOntology *mv_parent_p;

	MEM_FLAG mv_parent_mem;


} MeasuredVariable;


typedef struct MeasuredVariableNode
{
	ListItem mvn_node;

	MeasuredVariable *mvn_measured_variable_p;

	MEM_FLAG mvn_measured_variable_mem;

	char *mvn_id_s;

} MeasuredVariableNode;

#ifndef DOXYGEN_SHOULD_SKIP_THIS

#ifdef ALLOCATE_MEASURED_VARIABLE_TAGS
	#define MEASURED_VARIABLE_PREFIX DFW_FIELD_TRIAL_SERVICE_LOCAL
	#define MEASURED_VARIABLE_VAL(x)	= x
	#define MEASURED_VARIABLE_CONCAT_VAL(x,y)	= x y
#else
	#define MEASURED_VARIABLE_PREFIX extern
	#define MEASURED_VARIABLE_VAL(x)
	#define MEASURED_VARIABLE_CONCAT_VAL(x,y)
#endif

#endif 		/* #ifndef DOXYGEN_SHOULD_SKIP_THIS */


MEASURED_VARIABLE_PREFIX const char *MV_TRAIT_S MEASURED_VARIABLE_VAL ("trait");

MEASURED_VARIABLE_PREFIX const char *MV_MEASUREMENT_S MEASURED_VARIABLE_VAL ("measurement");

MEASURED_VARIABLE_PREFIX const char *MV_UNIT_S MEASURED_VARIABLE_VAL ("unit");

MEASURED_VARIABLE_PREFIX const char *MV_VARIABLE_S MEASURED_VARIABLE_VAL ("variable");

MEASURED_VARIABLE_PREFIX const char *MV_FORM_S MEASURED_VARIABLE_VAL ("form");

MEASURED_VARIABLE_PREFIX const char *MV_VALUE_S MEASURED_VARIABLE_VAL ("value");

MEASURED_VARIABLE_PREFIX const char *MV_INTERNAL_NAME_S MEASURED_VARIABLE_VAL ("internal_name");

MEASURED_VARIABLE_PREFIX const char *MV_SCALE_S MEASURED_VARIABLE_VAL ("scale");

MEASURED_VARIABLE_PREFIX const char *MV_NAME_S MEASURED_VARIABLE_CONCAT_VAL (CONTEXT_PREFIX_SCHEMA_ORG_S, "name");

MEASURED_VARIABLE_PREFIX const char *MV_ONTOLOGY_S MEASURED_VARIABLE_VAL ("ontology");


#ifdef __cplusplus
extern "C"
{
#endif


DFW_FIELD_TRIAL_SERVICE_LOCAL MeasuredVariable *AllocateMeasuredVariable (bson_oid_t *id_p, SchemaTerm *trait_p, SchemaTerm *measurement_p, SchemaTerm *unit_p, SchemaTerm *variable_p, const ScaleClass *class_p,
																						CropOntology *parent_ontology_p, MEM_FLAG parent_ontology_mem);


DFW_FIELD_TRIAL_SERVICE_LOCAL void FreeMeasuredVariable (MeasuredVariable *treatment_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL json_t *GetMeasuredVariableAsJSON (const MeasuredVariable *treatment_p, const ViewFormat format, const FieldTrialServiceData *data_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL MeasuredVariable *GetMeasuredVariableFromJSON (const json_t *phenotype_json_p, const FieldTrialServiceData *data_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL OperationStatus SaveMeasuredVariable (MeasuredVariable *mv_p, ServiceJob *job_p, const char * const job_name_s, const FieldTrialServiceData *data_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL MeasuredVariable *GetMeasuredVariableById (const bson_oid_t *id_p, const FieldTrialServiceData *data_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL MeasuredVariable *GetMeasuredVariableByName (const char * const name_s, const FieldTrialServiceData *data_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL MeasuredVariable *GetMeasuredVariableByIdString (const char *id_s, const FieldTrialServiceData *data_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL MeasuredVariable *GetMeasuredVariableBySchemaURLs (const char *trait_url_s, const char *method_url_s, const char *unit_url_s, const FieldTrialServiceData *data_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL const char *GetMeasuredVariableName (const MeasuredVariable *mv_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL const char *GetMeasuredVariableURL (const MeasuredVariable *mv_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL MeasuredVariableNode *AllocateMeasuredVariableNode (MeasuredVariable *variable_p, MEM_FLAG mv_mem);

DFW_FIELD_TRIAL_SERVICE_LOCAL void FreeMeasuredVariableNode (ListItem *node_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL const struct ScaleClass *GetMeasuredVariableScaleClass (const MeasuredVariable * const variable_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL OperationStatus IndexMeasuredVariable (MeasuredVariable *mv_p, ServiceJob *job_p, const char *job_name_s, FieldTrialServiceData *data_p);


#ifdef __cplusplus
}
#endif



#endif /* SERVICES_FIELD_TRIALS_INCLUDE_MEASURED_VARIABLE_H_ */

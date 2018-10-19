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
 * material.h
 *
 *  Created on: 11 Sep 2018
 *      Author: billy
 */

#ifndef SERVICES_DFW_FIELD_TRIAL_SERVICE_INCLUDE_MATERIAL_H_
#define SERVICES_DFW_FIELD_TRIAL_SERVICE_INCLUDE_MATERIAL_H_





#include "dfw_field_trial_service_data.h"
#include "dfw_field_trial_service_library.h"
#include "jansson.h"
#include "typedefs.h"


typedef struct Material
{
	bson_oid_t *ma_id_p;

	bson_oid_t *ma_germplasm_id_p;

	char *ma_source_s;

	char *ma_accession_s;

	char *ma_pedigree_s;

	char *ma_barcode_s;

} Material;





#ifndef DOXYGEN_SHOULD_SKIP_THIS

#ifdef ALLOCATE_MATERIAL_TAGS
	#define MATERIAL_PREFIX DFW_FIELD_TRIAL_SERVICE_LOCAL
	#define MATERIAL_VAL(x)	= x
	#define MATERIAL_CONCAT_VAL(x,y)	= x y
#else
	#define MATERIAL_PREFIX extern
	#define MATERIAL_VAL(x)
	#define MATERIAL_CONCAT_VAL(x,y)
#endif

#endif 		/* #ifndef DOXYGEN_SHOULD_SKIP_THIS */


MATERIAL_PREFIX const char *MA_ID_S MATERIAL_VAL ("id");

MATERIAL_PREFIX const char *MA_ACCESSION_S MATERIAL_VAL ("accession");

MATERIAL_PREFIX const char *MA_BARCODE_S MATERIAL_VAL ("barcode");

MATERIAL_PREFIX const char *MA_SOURCE_S MATERIAL_VAL ("source");


MATERIAL_PREFIX const char *MA_PEDIGREE_S MATERIAL_VAL ("pedigree");


MATERIAL_PREFIX const char *MA_GERMPLASM_ID_S MATERIAL_VAL ("germplasm_id");

MATERIAL_PREFIX const char *MA_EXPERIMENTAL_AREA_ID_S MATERIAL_VAL ("area_id");

MATERIAL_PREFIX const char *MA_INTERNAL_NAME_S MATERIAL_VAL ("internal_name");




#ifdef __cplusplus
extern "C"
{
#endif


DFW_FIELD_TRIAL_SERVICE_LOCAL Material *AllocateMaterial (bson_oid_t *id_p, const char *source_s, const char *accession_s, const char *pedigree_s, const char *barcode_s, const char *internal_name_s, const ExperimentalArea *area_p, bson_oid_t *germplasm_id_p, const DFWFieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL void FreeMaterial (Material *material_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL json_t *GetMaterialAsJSON (const Material *material_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL Material *GetMaterialFromJSON (const json_t *json_p, const DFWFieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool SaveMaterial (Material *material_p, DFWFieldTrialServiceData *data_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL Material *LoadMaterial (const int32 material_id, DFWFieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL Material *GetMaterialByInternalName (const char *material_s, ExperimentalArea *area_p, const DFWFieldTrialServiceData *data_p);


#ifdef __cplusplus
}
#endif


#endif /* SERVICES_DFW_FIELD_TRIAL_SERVICE_INCLUDE_MATERIAL_H_ */

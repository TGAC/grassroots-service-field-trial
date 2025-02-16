/*
 * crop_ontology.h
 *
 *  Created on: 15 Feb 2025
 *      Author: billy
 */

#ifndef SERVICES_FIELD_TRIALS_INCLUDE_CROP_ONTOLOGY_H_
#define SERVICES_FIELD_TRIALS_INCLUDE_CROP_ONTOLOGY_H_

#include "dfw_field_trial_service_library.h"
#include "dfw_field_trial_service_data.h"

#include "jansson.h"


typedef struct CropOntology
{
	bson_oid_t *co_id_p;

	char *co_name_s;

	char *co_url_s;

	char *co_crop_s;

	/**
	 * The URL to the logo image for this ontology
	 */
	char *co_image_s;

} CropOntology;



#ifndef DOXYGEN_SHOULD_SKIP_THIS

#ifdef ALLOCATE_CROP_ONTOLOGY_TAGS
	#define CROP_ONTOLOGY_PREFIX DFW_FIELD_TRIAL_SERVICE_LOCAL
	#define CROP_ONTOLOGY_VAL(x)	= x
	#define CROP_ONTOLOGY_CONCAT_VAL(x,y)	= x y
#else
	#define CROP_ONTOLOGY_PREFIX extern
	#define CROP_ONTOLOGY_VAL(x)
	#define CROP_ONTOLOGY_CONCAT_VAL(x,y)
#endif

#endif 		/* #ifndef DOXYGEN_SHOULD_SKIP_THIS */


CROP_ONTOLOGY_PREFIX const char *CO_NAME_S CROP_ONTOLOGY_CONCAT_VAL (CONTEXT_PREFIX_SCHEMA_ORG_S, "name");

CROP_ONTOLOGY_PREFIX const char *CO_URL_S CROP_ONTOLOGY_CONCAT_VAL (CONTEXT_PREFIX_SCHEMA_ORG_S, "url");

CROP_ONTOLOGY_PREFIX const char *CO_CROP_S CROP_ONTOLOGY_VAL ("crop");

CROP_ONTOLOGY_PREFIX const char *CO_IMAGE_S CROP_ONTOLOGY_CONCAT_VAL (CONTEXT_PREFIX_SCHEMA_ORG_S, "image");

#ifdef __cplusplus
extern "C"
{
#endif


DFW_FIELD_TRIAL_SERVICE_LOCAL CropOntology *AllocateCropOntology (bson_oid_t *id_p, const char *name_s, const char *url_s, const char *crop_s, const char *image_s);

DFW_FIELD_TRIAL_SERVICE_LOCAL void FreeCropOntology (CropOntology *co_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL json_t *GetCropOntologyAsJSON (CropOntology *co_p, const ViewFormat format, const FieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL CropOntology *GetCropOntologyFromJSON (const json_t *crop_json_p, const FieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool SaveCropOntology (CropOntology *crop_p, const FieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL CropOntology *GetCropOntologyByIdString (const char *id_s, const FieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL CropOntology *GetCropOntologyById (const bson_oid_t *id_p, const FieldTrialServiceData *data_p);


#ifdef __cplusplus
}
#endif


#endif /* SERVICES_FIELD_TRIALS_INCLUDE_CROP_ONTOLOGY_H_ */

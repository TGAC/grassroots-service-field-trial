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

	char *co_id_s;

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

CROP_ONTOLOGY_PREFIX const char *CO_ID_S CROP_ONTOLOGY_CONCAT_VAL (CONTEXT_PREFIX_SCHEMA_ORG_S, "sameAs");

CROP_ONTOLOGY_PREFIX const char *CO_CROP_S CROP_ONTOLOGY_VAL ("crop");

CROP_ONTOLOGY_PREFIX const char *CO_IMAGE_S CROP_ONTOLOGY_CONCAT_VAL (CONTEXT_PREFIX_SCHEMA_ORG_S, "image");

#ifdef __cplusplus
extern "C"
{
#endif


DFW_FIELD_TRIAL_SERVICE_LOCAL CropOntology *AllocateCropOntology (bson_oid_t *id_p, const char *name_s, const char *url_s, const char *crop_s, const char *image_s);


DFW_FIELD_TRIAL_SERVICE_LOCAL void FreeCropOntology (CropOntology *co_p);


/**
 * Make a deep copy of a CropOntology.
 *
 * @param src_p The CropOntology to make a deep copy of.
 * @return The newly-allocated deep copy of the input CropOntology or <code>NULL</code> upon error.
 */
DFW_FIELD_TRIAL_SERVICE_LOCAL CropOntology *DuplicateCropOntology (const CropOntology * const src_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL json_t *GetCropOntologyAsJSON (CropOntology *co_p, const ViewFormat format, const FieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL CropOntology *GetCropOntologyFromJSON (const json_t *crop_json_p, const FieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool SaveCropOntology (CropOntology *crop_p, const FieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL CropOntology *GetCropOntologyByIdString (const char *id_s, const FieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL CropOntology *GetCropOntologyById (const bson_oid_t *id_p, const FieldTrialServiceData *data_p);


/**
 * Get an existing Ontology from the database by its id.
 *
 * This id is not the bson_oid_t, instead it is its own given id or name by its curators.
 * For instance the Crop Ontology's Wheat Ontology is CO_321, see https://cropontology.org/term/CO_321:ROOT for details.
 *
 * @param ontology_id_s The id of the Ontology to search for.
 * @param data_p The FieldTrialServiceData
 * @return The existing CropOntology from the database or <code>NULL</code> if it does not have an entry.
 */
DFW_FIELD_TRIAL_SERVICE_LOCAL CropOntology *GetExistingCropOntologyByOntologyID (const char * const ontology_id_s, const FieldTrialServiceData *data_p);

#ifdef __cplusplus
}
#endif


#endif /* SERVICES_FIELD_TRIALS_INCLUDE_CROP_ONTOLOGY_H_ */

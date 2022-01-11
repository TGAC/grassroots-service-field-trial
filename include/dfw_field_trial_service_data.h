/*
** Copyright 2014-2016 The Earlham Institute
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

/**
 * @file
 * @brief
 */
/*
 * dfw_field_trial_service_data.h
 *
 *  Created on: 8 Jul 2018
 *      Author: tyrrells
 */

#ifndef DFW_FIELD_TRIAL_SERVICE_DATA_H_
#define DFW_FIELD_TRIAL_SERVICE_DATA_H_

#include "dfw_field_trial_service_library.h"
#include "jansson.h"

#include "service.h"
#include "mongodb_tool.h"
#include "sqlite_tool.h"


typedef enum
{
	DFTD_PROGRAMME,
	DFTD_FIELD_TRIAL,
	DFTD_STUDY,
	DFTD_LOCATION,
	DFTD_PLOT,
/*	DFTD_ROW, */
	DFTD_MATERIAL,
	DFTD_DRILLING,
	DFTD_MEASURED_VARIABLE,
	DFTD_OBSERVATION,
	DFTD_INSTRUMENT,
	DFTD_GENE_BANK,
	DFTD_CROP,
	DFTD_TREATMENT,
	DFTD_NUM_TYPES
} DFWFieldTrialData;



/**
 * An indicator of what the output destination
 * is for the JSON data values. This lets us
 * know whether we need to e.g. expand fields
 * to full objects from their ids, or omit
 * certain fields entirely
 *
 * @ingroup field_trials_service
 */
typedef enum
{
	/**
	 * This is for generating JSON to be stored in the
	 * server-side mongo db.
	 */
	VF_STORAGE,

	/**
	 * This is for generating a full data set for displaying
	 * within a client.
	 */
	VF_CLIENT_FULL,

	/**
	 * This is for generating a minimal data set for displaying
	 * within a client. This is used when doing LinkedService
	 * calls to get subsequent child data.
	 */
	VF_CLIENT_MINIMAL,

	/**
	 * The number of available formats
	 */
	VF_NUM_FORMATS
} ViewFormat;



/**
 * The configuration data used by the DFW Field Trial Service.
 *
 * @extends ServiceData
 * @ingroup field_trials_service
 */
typedef struct /*DFW_FIELD_TRIAL_SERVICE_LOCAL*/ FieldTrialServiceData
{
	/** The base ServiceData. */
	ServiceData dftsd_base_data;


	/**
	 * @private
	 *
	 * The MongoTool to connect to the database where our data is stored.
	 */
	MongoTool *dftsd_mongo_p;


	/**
	 * @private
	 *
	 * The name of the database to use.
	 */
	const char *dftsd_database_s;

	/**
	 * @private
	 *
	 * The collection name of use for each of the different types of data.
	 */
	const char *dftsd_collection_ss [DFTD_NUM_TYPES];

	/**
	 * @private
	 *
	 * The key used for facets in Lucene
	 */
	const char *dftsd_facet_key_s;


	/**
	 * @private
	 *
	 * The path to where cached studies are saved.
	 */
	const char *dftsd_study_cache_path_s;


	/**
	 * @private
	 *
	 * The filesystem path to where to save the Studies will be backed up to
	 * fwhen they are deleted
	 */
	const char *dftsd_backup_path_s;


	/**
	 * @private
	 *
	 * The filesystem path to where to save the Frictionless Data Packages
	 * for the Studies.
	 */
	const char *dftsd_fd_path_s;

	/**
	 * @private
	 *
	 * The base url to where the Frictionless Data Packages
	 * for the Studies will be served from.
	 */
	const char *dftsd_fd_url_s;


	/**
	 * @private
	 *
	 * The base url for where each study can be viewed on the
	 * front end.
	 */
	const char *dftsd_view_study_url_s;

	json_t *dftsd_observations_cache_p;

} FieldTrialServiceData;



#ifndef DOXYGEN_SHOULD_SKIP_THIS

#ifdef ALLOCATE_DFW_FIELD_TRIAL_SERVICE_TAGS
	#define DFW_FIELD_TRIAL_PREFIX DFW_FIELD_TRIAL_SERVICE_API
	#define DFW_FIELD_TRIAL_VAL(x)	= x
	#define DFW_FIELD_TRIAL_CONCAT_VAL(x,y) = x y
#else
	#define DFW_FIELD_TRIAL_PREFIX extern
	#define DFW_FIELD_TRIAL_VAL(x)
	#define DFW_FIELD_TRIAL_CONCAT_VAL(x,y) = x y
#endif

#endif 		/* #ifndef DOXYGEN_SHOULD_SKIP_THIS */


/**
 * The key for specifying the object containing the fields data
 *
 * @ingroup field_trials_service
 */
DFW_FIELD_TRIAL_PREFIX const char *DFT_PROGRAM_S DFW_FIELD_TRIAL_VAL ("Programs");


/**
 * The key for specifying the object containing the fields data
 *
 * @ingroup field_trials_service
 */
DFW_FIELD_TRIAL_PREFIX const char *DFT_FIELD_TRIALS_S DFW_FIELD_TRIAL_VAL ("FieldTrials");

/**
 * The key for specifying the object containing the experimental area data
 *
 * @ingroup field_trials_service
 */
DFW_FIELD_TRIAL_PREFIX const char *DFT_STUDIES_S DFW_FIELD_TRIAL_VAL ("Studies");


/**
 * The key for specifying the object containing the location data
 *
 * @ingroup field_trials_service
 */
DFW_FIELD_TRIAL_PREFIX const char *DFT_LOCATION_S DFW_FIELD_TRIAL_VAL ("Locations");


/**
 * The key for specifying the object containing the plot data
 *
 * @ingroup field_trials_service
 */
DFW_FIELD_TRIAL_PREFIX const char *DFT_PLOT_S DFW_FIELD_TRIAL_VAL ("Plots");


/**
 * The key for specifying the object containing the drilling data
 *
 * @ingroup field_trials_service
 */
DFW_FIELD_TRIAL_PREFIX const char *DFT_DRILLING_S DFW_FIELD_TRIAL_VAL ("Drillings");



/**
 * The key for specifying the object containing the materials data
 *
 * @ingroup field_trials_service
 */
DFW_FIELD_TRIAL_PREFIX const char *DFT_MATERIAL_S DFW_FIELD_TRIAL_VAL ("Materials");


/**
 * The key for specifying the object containing the  phenotype data
 *
 * @ingroup field_trials_service
 */
DFW_FIELD_TRIAL_PREFIX const char *DFT_PHENOTYPE_S DFW_FIELD_TRIAL_VAL ("Phenotypes");


/**
 * The key for specifying the object containing the observation data
 *
 * @ingroup field_trials_service
 */
DFW_FIELD_TRIAL_PREFIX const char *DFT_OBSERVATION_S DFW_FIELD_TRIAL_VAL ("Observations");


/**
 * The key for specifying the object containing the instruments
 *
 * @ingroup field_trials_service
 */
DFW_FIELD_TRIAL_PREFIX const char *DFT_INSTRUMENT_S DFW_FIELD_TRIAL_VAL ("Instruments");


/**
 * The key for specifying the object containing the Gene Banks
 *
 * @ingroup field_trials_service
 */
DFW_FIELD_TRIAL_PREFIX const char *DFT_GENE_BANK_S DFW_FIELD_TRIAL_VAL ("GeneBanks");



/**
 * The key for specifying the object containing the rows within the plots.
 *
 * @ingroup field_trials_service
 */
//DFW_FIELD_TRIAL_PREFIX const char *DFT_ROW_S DFW_FIELD_TRIAL_VAL ("Rows");


/**
 * The key for specifying the object containing the crops.
 *
 * @ingroup field_trials_service
 */
DFW_FIELD_TRIAL_PREFIX const char *DFT_CROP_S DFW_FIELD_TRIAL_VAL ("Crops");


/**
 * The key for specifying the object containing the treatments.
 *
 * @ingroup field_trials_service
 */
DFW_FIELD_TRIAL_PREFIX const char *DFT_TREATMENT_S DFW_FIELD_TRIAL_VAL ("Treatments");



/**
 * The key for specifying whether a particular object in a JSON tree is
 * selected, e.g. matched a search.
 *
 * @ingroup field_trials_service
 */
DFW_FIELD_TRIAL_PREFIX const char *DFT_SELECTED_S DFW_FIELD_TRIAL_VAL ("selected");

/** The prefix to use for Field Trial Service aliases. */
#define DFT_GROUP_ALIAS_PREFIX_S "field_trial"

/*
 * forward declaration
 */
struct MeasuredVafriable;


#ifdef __cplusplus
extern "C"
{
#endif

DFW_FIELD_TRIAL_SERVICE_LOCAL FieldTrialServiceData *AllocateFieldTrialServiceData (void);


DFW_FIELD_TRIAL_SERVICE_LOCAL void FreeFieldTrialServiceData (FieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool ConfigureFieldTrialService (FieldTrialServiceData *data_p, GrassrootsServer *grassroots_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL const char *GetDatatypeAsString (const DFWFieldTrialData data_type);


DFW_FIELD_TRIAL_SERVICE_LOCAL const char *GetDatatypeDescriptionAsString (const DFWFieldTrialData data_type);


DFW_FIELD_TRIAL_SERVICE_LOCAL DFWFieldTrialData GetDatatypeFromString (const char *type_s);


DFW_FIELD_TRIAL_SERVICE_LOCAL const char *GetImageForDatatype (const FieldTrialServiceData *data_p, const char *data_type_s);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool EnableObservationsCache (FieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL void ClearObservationsCache (FieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL struct MeasuredVariable *GetCachedMeasuredVariable (FieldTrialServiceData *data_p, const char *mv_id_s);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool AddMeasuredVariableToCache (FieldTrialServiceData *data_p, const char *id_s, struct MeasuredVariable *mv_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool HasMeasuredVariableCache (FieldTrialServiceData *data_p);

#ifdef __cplusplus
}
#endif


#endif /* DFW_FIELD_TRIAL_SERVICE_DATA_H_ */

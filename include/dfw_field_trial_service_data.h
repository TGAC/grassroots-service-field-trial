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
	DFTD_FIELD_TRIAL,
	DFTD_EXPERIMENTAL_AREA,
	DFTD_LOCATION,
	DFTD_PLOT,
	DFTD_ROW,
	DFTD_MATERIAL,
	DFTD_DRILLING,
	DFTD_RAW_PHENOTYPE,
	DFTD_CORRECTED_PHENOTYPE,
	DFTD_INSTRUMENT,
	DFTD_GENE_BANK,
	DFTD_NUM_TYPES
} DFWFieldTrialData;


typedef enum
{
	DB_MONGO_DB,
	DB_SQLITE,
	DB_NUM_BACKENDS
} DFWBackend;





/**
 * The configuration data used by the DFW Field Trial Service.
 *
 * @extends ServiceData
 */
typedef struct /*DFW_FIELD_TRIAL_SERVICE_LOCAL*/ DFWFieldTrialServiceData
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

} DFWFieldTrialServiceData;



#ifndef DOXYGEN_SHOULD_SKIP_THIS

#ifdef ALLOCATE_DFW_FIELD_TRIAL_SERVICE_TAGS
	#define DFW_FIELD_TRIAL_PREFIX DFW_FIELD_TRIAL_SERVICE_LOCAL
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
 * @ingroup dfw_field_trial_service
 */
DFW_FIELD_TRIAL_PREFIX const char *DFT_FIELD_S DFW_FIELD_TRIAL_VAL ("FieldTrials");

/**
 * The key for specifying the object containing the experimental area data
 *
 * @ingroup dfw_field_trial_service
 */
DFW_FIELD_TRIAL_PREFIX const char *DFT_EXPERMIENTAL_AREA_S DFW_FIELD_TRIAL_VAL ("ExperimentalAreas");


/**
 * The key for specifying the object containing the location data
 *
 * @ingroup dfw_field_trial_service
 */
DFW_FIELD_TRIAL_PREFIX const char *DFT_LOCATION_S DFW_FIELD_TRIAL_VAL ("Locations");


/**
 * The key for specifying the object containing the plot data
 *
 * @ingroup dfw_field_trial_service
 */
DFW_FIELD_TRIAL_PREFIX const char *DFT_PLOT_S DFW_FIELD_TRIAL_VAL ("Plots");


/**
 * The key for specifying the object containing the drilling data
 *
 * @ingroup dfw_field_trial_service
 */
DFW_FIELD_TRIAL_PREFIX const char *DFT_DRILLING_S DFW_FIELD_TRIAL_VAL ("Drillings");



/**
 * The key for specifying the object containing the materials data
 *
 * @ingroup dfw_field_trial_service
 */
DFW_FIELD_TRIAL_PREFIX const char *DFT_MATERIAL_S DFW_FIELD_TRIAL_VAL ("Materials");


/**
 * The key for specifying the object containing the raw phenotype data
 *
 * @ingroup dfw_field_trial_service
 */
DFW_FIELD_TRIAL_PREFIX const char *DFT_RAW_PHENOTYPE_S DFW_FIELD_TRIAL_VAL ("RawPhenotypes");


/**
 * The key for specifying the object containing the corrected phenotype data
 *
 * @ingroup dfw_field_trial_service
 */
DFW_FIELD_TRIAL_PREFIX const char *DFT_CORRECTED_PHENOTYPE_S DFW_FIELD_TRIAL_VAL ("CorrectedPhenotypes");

/**
 * The key for specifying the object containing the instruments
 *
 * @ingroup dfw_field_trial_service
 */
DFW_FIELD_TRIAL_PREFIX const char *DFT_INSTRUMENT_S DFW_FIELD_TRIAL_VAL ("Instruments");


/**
 * The key for specifying the object containing the Gene Banks
 *
 * @ingroup dfw_field_trial_service
 */
DFW_FIELD_TRIAL_PREFIX const char *DFT_GENE_BANK_S DFW_FIELD_TRIAL_VAL ("GeneBanks");



/**
 * The key for specifying the object containing the rows within the plots.
 *
 * @ingroup dfw_field_trial_service
 */
DFW_FIELD_TRIAL_PREFIX const char *DFT_ROW_S DFW_FIELD_TRIAL_VAL ("Rows");





#ifdef __cplusplus
extern "C"
{
#endif

DFW_FIELD_TRIAL_SERVICE_LOCAL DFWFieldTrialServiceData *AllocateDFWFieldTrialServiceData (void);


DFW_FIELD_TRIAL_SERVICE_LOCAL void FreeDFWFieldTrialServiceData (DFWFieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool ConfigureDFWFieldTrialService (DFWFieldTrialServiceData *data_p);

#ifdef __cplusplus
}
#endif


#endif /* DFW_FIELD_TRIAL_SERVICE_DATA_H_ */

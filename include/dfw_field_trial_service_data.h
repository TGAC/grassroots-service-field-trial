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
	DFTD_DRILLING,
	DFTD_RAW_PHENOTYPE,
	DFTD_CORRECTED_PHENOTYPE,
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



	DFWBackend dftsd_backend;

	/**
	 * @private
	 *
	 * The MongoTool to connect to the database where our data is stored.
	 */
	MongoTool *dftsd_mongo_p;



	SQLiteTool *dftsd_sqlite_p;


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


#ifdef __cplusplus
extern "C"
{
#endif


/*
DFW_FIELD_TRIAL_SERVICE_LOCAL void InitialiseId (DFWId *id_p);

DFW_FIELD_TRIAL_SERVICE_LOCAL void ClearId (DFWId *id_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool SetIdString (DFWId *id_p, const char *id_s);


DFW_FIELD_TRIAL_SERVICE_LOCAL void SetIdIndex (DFWId *id_p, const uint32 id_index);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool IsIdSet (const DFWId *id_p, const DFWFieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool AddIdToJSON (json_t *json_p, const char * const key_s, DFWId *id_p, DFWFieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool GetIdFromJSON (json_t *json_p, const char * const key_s, DFWId *id_p, DFWFieldTrialServiceData *data_p);

*/

#ifdef __cplusplus
}
#endif


#endif /* DFW_FIELD_TRIAL_SERVICE_DATA_H_ */

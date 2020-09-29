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
 * dfw_field_trial_service_data.c
 *
 *  Created on: 18 Sep 2018
 *      Author: billy
 */

#define ALLOCATE_DFW_FIELD_TRIAL_SERVICE_TAGS (1)
#include "dfw_field_trial_service_data.h"

#include "streams.h"
#include "string_utils.h"


static const char *S_TYPES_SS [DFTD_NUM_TYPES] =
{
	"Grassroots:FieldTrial",
	"Grassroots:Study",
	"Grassroots:Location",
	"Grassroots:Plot",
/*	"Grassroots:Row", */
 	"Grassroots:Material",
	"Grassroots:Drilling",
	"Grassroots:Phenotype",
	"Grassroots:Observation",
	"Grassroots:Instrument",
	"Grassroots:GeneBank",
	"Grassroots:Crop"
};


FieldTrialServiceData *AllocateFieldTrialServiceData (void)
{
	FieldTrialServiceData *data_p = (FieldTrialServiceData *) AllocMemory (sizeof (FieldTrialServiceData));

	if (data_p)
		{
			data_p -> dftsd_mongo_p =  NULL;
			data_p -> dftsd_database_s = NULL;
			data_p -> dftsd_facet_key_s = NULL;
			data_p -> dftsd_study_cache_path_s = NULL;

			memset (data_p -> dftsd_collection_ss, 0, DFTD_NUM_TYPES * sizeof (const char *));

			return data_p;
		}

	return NULL;
}


void FreeFieldTrialServiceData (FieldTrialServiceData *data_p)
{
	if (data_p -> dftsd_mongo_p)
		{
			FreeMongoTool (data_p -> dftsd_mongo_p);
		}

	FreeMemory (data_p);
}


bool ConfigureFieldTrialService (FieldTrialServiceData *data_p, GrassrootsServer *grassroots_p)
{
	bool success_flag = false;
	const json_t *service_config_p = data_p -> dftsd_base_data.sd_config_p;

	data_p -> dftsd_database_s = GetJSONString (service_config_p, "database");

	if (data_p -> dftsd_database_s)
		{
			if ((data_p -> dftsd_mongo_p = AllocateMongoTool (NULL, grassroots_p -> gs_mongo_manager_p)) != NULL)
				{
					if (SetMongoToolDatabase (data_p -> dftsd_mongo_p, data_p -> dftsd_database_s))
						{
							success_flag = true;

							data_p -> dftsd_study_cache_path_s = GetJSONString (service_config_p, "cache_path");

							if (data_p -> dftsd_study_cache_path_s)
								{
									if (!EnsureDirectoryExists (data_p -> dftsd_study_cache_path_s))
										{
											PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to create studies cache directory \"%s\"", data_p -> dftsd_study_cache_path_s);
											data_p -> dftsd_study_cache_path_s = NULL;
										}
								}

							* ((data_p -> dftsd_collection_ss) + DFTD_FIELD_TRIAL) = DFT_FIELD_S;
							* ((data_p -> dftsd_collection_ss) + DFTD_STUDY) = DFT_STUDIES_S;
							* ((data_p -> dftsd_collection_ss) + DFTD_LOCATION) = DFT_LOCATION_S;
							* ((data_p -> dftsd_collection_ss) + DFTD_PLOT) = DFT_PLOT_S;
							* ((data_p -> dftsd_collection_ss) + DFTD_MATERIAL) = DFT_MATERIAL_S;
							* ((data_p -> dftsd_collection_ss) + DFTD_DRILLING) = DFT_DRILLING_S;
							* ((data_p -> dftsd_collection_ss) + DFTD_TREATMENT) = DFT_PHENOTYPE_S;
							* ((data_p -> dftsd_collection_ss) + DFTD_OBSERVATION) = DFT_OBSERVATION_S;
							* ((data_p -> dftsd_collection_ss) + DFTD_INSTRUMENT) = DFT_INSTRUMENT_S;
							* ((data_p -> dftsd_collection_ss) + DFTD_GENE_BANK) = DFT_GENE_BANK_S;
							// * ((data_p -> dftsd_collection_ss) + DFTD_ROW) = DFT_ROW_S;
							* ((data_p -> dftsd_collection_ss) + DFTD_CROP) = DFT_CROP_S;
						}
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to set db to \"%s\"", data_p -> dftsd_database_s);
						}

				}		/* if ((data_p -> dftsd_mongo_p = AllocateMongoTool (NULL, grassroots_p -> gs_mongo_manager_p)) != NULL) */
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate MongoTool");
				}


		} /* if (data_p -> psd_database_s) */

	return success_flag;
}



const char *GetDatatypeAsString (const DFWFieldTrialData data_type)
{
	const char *type_s = NULL;

	if (data_type < DFTD_NUM_TYPES)
		{
			type_s =  * (S_TYPES_SS + data_type);
		}

	return type_s;
}


DFWFieldTrialData GetDatatypeFromString (const char *type_s)
{
	if (type_s)
		{
			DFWFieldTrialData i = 0;

			for ( ; i < DFTD_NUM_TYPES; ++ i)
				{
					if (strcmp (* (S_TYPES_SS + i), type_s) == 0)
						{
							return i;
						}
				}

		}		/* if (type_s) */

	return DFTD_NUM_TYPES;
}



const char *GetImageForDatatype (const FieldTrialServiceData *data_p, const char *data_type_s)
{
	const char *image_s = NULL;
	const json_t *images_config_p = json_object_get (data_p -> dftsd_base_data.sd_config_p, "images");

	if (images_config_p)
		{
			image_s = GetJSONString (images_config_p, data_type_s);
		}


	return image_s;
}


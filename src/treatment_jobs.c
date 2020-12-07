/*
** Copyright 2014-2020 The Earlham Institute
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
 * treatment_jobs.c
 *
 *  Created on: 6 Mar 2020
 *      Author: billy
 */


#include "treatment_jobs.h"
#include "dfw_util.h"

#include "json_parameter.h"
#include "string_parameter.h"
#include "parameter_group.h"

/**
 * The NamedParameterType for the target chromosome parameter.
 */
static NamedParameterType TR_NAME = { "TR Name", PT_STRING };
static NamedParameterType TR_DESCRIPTION = { "TR Description", PT_LARGE_STRING };
static NamedParameterType TR_ONOTOLOGY_URL = { "TR Web Address", PT_STRING };
static NamedParameterType TR_PARENT_NAME = { "TR Parent", PT_STRING };
static NamedParameterType TR_SYNONYMS = { "TR Parent", PT_LARGE_STRING };



/*
 fertilizer exposure


    Spacer chemical fertilizer exposure

        Spacer nitrogen fertilizer exposure

            ammonium sulfate exposure

            Spacer nitrate exposure

                ammonium nitrate exposure

                calcium nitrate exposure

                limited nitrate exposure

                potassium nitrate exposure

                sodium nitrate exposure

            NPK fertilizer exposure

            urea exposure

        Spacer phosphate fertilizer exposure

            NPK fertilizer exposure

            superphosphate exposure

        Spacer sulfate fertilizer exposure

            ammonium sulfate exposure

            ferrous sulfate exposure

            potassium sulfate exposure
 */

static const KeyValuePair *S_FERTILIZERS_P [] =
{

};





json_t *GetAllTreatmentsAsJSON (const FieldTrialServiceData *data_p, bson_t *opts_p)
{
	json_t *results_p = NULL;

	if (SetMongoToolCollection (data_p -> dftsd_mongo_p, data_p -> dftsd_collection_ss [DFTD_TREATMENT]))
		{
			bson_t *query_p = NULL;

			results_p = GetAllMongoResultsAsJSON (data_p -> dftsd_mongo_p, query_p, opts_p);
		}		/* if (SetMongoToolCollection (data_p -> dftsd_mongo_p, data_p -> dftsd_collection_ss [DFTD_TREATMENT])) */

	return results_p;
}



json_t *GetTreatmentIndexingData (Service *service_p)
{
	FieldTrialServiceData *data_p = (FieldTrialServiceData *) (service_p -> se_data_p);
	json_t *src_treatments_p = GetAllTreatmentsAsJSON (data_p, NULL);

	if (src_treatments_p)
		{
			if (json_is_array (src_treatments_p))
				{
					size_t i;
					json_t *src_treatment_p;
					size_t num_added = 0;

					json_array_foreach (src_treatments_p, i, src_treatment_p)
						{
							if (AddDatatype (src_treatment_p, DFTD_TREATMENT))
								{

								}


						}		/* json_array_foreach (src_treatments_p, i, src_study_p) */

				}		/* if (json_is_array (src_treatments_p)) */

			return src_treatments_p;
		}		/* if (src_treatments_p) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "No programs for \"%s\"", GetServiceName (service_p));
		}

	return NULL;
}


bool GetSubmissionTreatmentParameterTypeForNamedParameter (const char *param_name_s, ParameterType *pt_p)
{
	bool success_flag = true;


	if (strcmp (param_name_s, TR_NAME.npt_name_s) == 0)
		{
			*pt_p = TR_NAME.npt_type;
		}
	else if (strcmp (param_name_s, TR_DESCRIPTION.npt_name_s) == 0)
		{
			*pt_p = TR_DESCRIPTION.npt_type;
		}
	else if (strcmp (param_name_s, TR_ONOTOLOGY_URL.npt_name_s) == 0)
		{
			*pt_p = TR_ONOTOLOGY_URL.npt_type;
		}
	else if (strcmp (param_name_s, TR_PARENT_NAME.npt_name_s) == 0)
		{
			*pt_p = TR_PARENT_NAME.npt_type;
		}
	else if (strcmp (param_name_s, TR_SYNONYMS.npt_name_s) == 0)
		{
			*pt_p = TR_SYNONYMS.npt_type;
		}
	else
		{
			success_flag = false;
		}


	return success_flag;
}



bool AddSubmissionTreatmentParams (ServiceData *data_p, ParameterSet *param_set_p, Resource *resource_p)
{
	bool success_flag = false;
	ParameterGroup *group_p = CreateAndAddParameterGroupToParameterSet ("Treatmemts", true, data_p, param_set_p);

	if (group_p)
		{
			Parameter *param_p = NULL;

			if ((param_p = EasyCreateAndAddStringParameterToParameterSet (data_p, param_set_p, group_p, TR_NAME.npt_type, TR_NAME.npt_name_s, "Treatment Factor", "Create a Treatment Factor for this Study", NULL, PL_ALL)) != NULL)
				{
					success_flag = true;
				}

		}		/* if (group_p) */

	return success_flag;
}



bool RunForSubmissionTreatmentParams (FieldTrialServiceData *data_p, ParameterSet *param_set_p, ServiceJob *job_p)
{
	bool job_done_flag = false;
	const char *name_s = NULL;

	if (GetCurrentStringParameterValueFromParameterSet (param_set_p, TR_NAME.npt_name_s, &name_s))
		{
			const char *description_s = NULL;

			if (GetCurrentStringParameterValueFromParameterSet (param_set_p, TR_DESCRIPTION.npt_name_s, &description_s))
				{

				}		/* if (GetCurrentParameterValueFromParameterSet (param_set_p, TR_DESCRIPTION.npt_name_s, &description_s)) */
			else
				{
					PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to get param \%s\"", TR_DESCRIPTION.npt_name_s);
				}

		}		/* if (GetCurrentParameterValueFromParameterSet (param_set_p, TR_NAME.npt_name_s, &name_s)) */
	else
		{
			PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to get param \%s\"", TR_NAME.npt_name_s);
		}

	return job_done_flag;
}


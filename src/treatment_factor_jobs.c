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
 * treatment_factor_jobs.c
 *
 *  Created on: 6 Mar 2020
 *      Author: billy
 */


#include "/treatment_factor_jobs.h"

#include "json_parameter.h"
#include "string_parameter.h"
#include "parameter_group.h"

/**
 * The NamedParameterType for the target chromosome parameter.
 */
static NamedParameterType TF_NAME = { "Name", PT_STRING };
static NamedParameterType TF_DESCRIPTION = { "Description", PT_STRING };
static NamedParameterType TF_VALUES = { "Levels", PT_JSON_TABLE };

static const char * const S_LABEL_TITLE_S = "Label";
static const char * const S_VALUE_TITLE_S = "Value";


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



static Parameter *GetTableParameter (ParameterSet *param_set_p, ParameterGroup *group_p, Study *active_study_p, const FieldTrialServiceData *data_p);
static json_t *GetTableParameterHints (void);







bool AddTreatmentFactorParameters (ParameterSet *param_set_p, Study *active_study_p, const ServiceData *data_p)
{
	bool success_flag = false;
	ParameterGroup *group_p = CreateAndAddParameterGroupToParameterSet ("TreatmemtFactors", true, data_p, param_set_p);

	if (group_p)
		{
			Parameter *param_p = NULL;

			if ((param_p = EasyCreateAndAddStringParameterToParameterSet (data_p, param_set_p, group_p, TF_NAME.npt_type, TF_NAME.npt_name_s, "Treatment Factor", "Create a Treatment Factor for this Study", NULL, PL_ALL)) != NULL)
				{
					if ((param_p = GetTableParameter (param_set_p, group_p, active_study_p,  (const FieldTrialServiceData *) data_p)) != NULL)
						{
							success_flag = true;
						}
				}

		}		/* if (group_p) */

	return success_flag;
}



static Parameter *GetTableParameter (ParameterSet *param_set_p, ParameterGroup *group_p, Study *active_study_p, const FieldTrialServiceData *data_p)
{
	Parameter *param_p = NULL;
	bool success_flag = false;
	json_t *hints_p = GetTableParameterHints ();

	if (hints_p)
		{
			json_t *treatmnent_factors_json_p = NULL;

			if (active_study_p)
				{

					success_flag = true;

				}
			else
				{
					success_flag = true;
				}

			if (success_flag)
				{
					param_p = EasyCreateAndAddJSONParameterToParameterSet (& (data_p -> dftsd_base_data), param_set_p, group_p, TF_VALUES.npt_type, TF_VALUES.npt_name_s, "Plot data to upload", "The data to upload", NULL, PL_ALL);

					if (param_p)
						{
							success_flag = false;

							if (AddParameterKeyJSONValuePair (param_p, PA_TABLE_COLUMN_HEADINGS_S, hints_p))
								{
									success_flag = true;
								}

							if (!success_flag)
								{
									FreeParameter (param_p);
									param_p = NULL;
								}

						}		/* if (param_p) */

				}		/* if (success_flag) */

			json_decref (hints_p);
		}		/* if (hints_p) */


	return param_p;
}


static json_t *GetTableParameterHints (void)
{
	json_t *hints_p = json_array ();

	if (hints_p)
		{
			if (AddColumnParameterHint (S_LABEL_TITLE_S, PT_STRING, hints_p))
				{
					if (AddColumnParameterHint (S_VALUE_TITLE_S, PT_STRING, hints_p))
						{
							return hints_p;
						}
				}

			json_decref (hints_p);
		}

	return NULL;
}



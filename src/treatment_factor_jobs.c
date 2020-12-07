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
 *  Created on: 7 Dec 2020
 *      Author: billy
 */

#include "treatment_factor_jobs.h"

#include "string_parameter.h"
#include "json_parameter.h"

#include "study_jobs.h"


static NamedParameterType TFJ_STUDY = { "Study", PT_STRING };
static NamedParameterType TFJ_TREATMENT = { "Treatment", PT_STRING };

static NamedParameterType TFJ_VALUES = { "Levels", PT_JSON_TABLE };

static const char * const S_LABEL_TITLE_S = "Label";
static const char * const S_VALUE_TITLE_S = "Value";

static const char * const S_EMPTY_LIST_OPTION_S = "<empty>";



static Parameter *GetTableParameter (ParameterSet *param_set_p, ParameterGroup *group_p, TreatmentFactor *active_tf_p, const FieldTrialServiceData *data_p);

static json_t *GetTableParameterHints (void);


bool AddSubmissionTreatmentFactorParams (ServiceData *data_p, ParameterSet *param_set_p, Resource *resource_p)
{
	Parameter *param_p;
	const char *study_id_s = (char *) S_EMPTY_LIST_OPTION_S;
	TreatmentFactor *active_tf_p = NULL;

	if ((param_p = EasyCreateAndAddStringParameterToParameterSet (data_p, param_set_p, NULL, TFJ_STUDY.npt_type, TFJ_STUDY.npt_name_s, "Study", "Study to load Treatment Factors for", study_id_s, PL_ALL)) != NULL)
		{
			FieldTrialServiceData *ft_data_p = (FieldTrialServiceData *) data_p;

			if (SetUpStudiesListParameter (ft_data_p, (StringParameter *) param_p, NULL, true))
				{
					if (GetTableParameter (param_set_p, NULL, active_tf_p, ft_data_p))
						{
							return true;
						}

				}		/* if (SetUpStudiesListParameter (ft_data_p, (StringParameter *) param_p, NULL, true)) */
		}

	return false;
}




bool SetUpTreatmentFactorsListParameter (const FieldTrialServiceData *data_p, StringParameter *param_p, const Study *active_study_p, const bool empty_option_flag)
{
	bool success_flag = false;
	bool value_set_flag = false;

	if (active_study_p -> st_treatments_p)
		{
			bool loop_flag = true;
			TreatmentFactorNode *node_p = (TreatmentFactorNode *) (active_study_p -> st_treatments_p -> ll_head_p);

			while (node_p && loop_flag)
				{
					TreatmentFactor *tf_p = node_p -> tfn_p;
					const char *name_s = GetTreatmentFactorName (tf_p);
					char *id_s = GetBSONOidAsString (tf_p -> tf_treatment_p -> tr_id_p);

					if (id_s)
						{
							if (!CreateAndAddStringParameterOption (param_p, name_s, id_s))
								{
									loop_flag = false;
								}

							FreeCopiedString (id_s);
						}

					if (loop_flag)
						{
							node_p = (TreatmentFactorNode *) (node_p -> tfn_node.ln_next_p);
						}

				}		/* while (node_p && loop_flag) */
		}

	return success_flag;
}



static Parameter *GetTableParameter (ParameterSet *param_set_p, ParameterGroup *group_p, TreatmentFactor *active_tf_p, const FieldTrialServiceData *data_p)
{
	Parameter *param_p = NULL;
	bool success_flag = false;
	json_t *hints_p = GetTableParameterHints ();

	if (hints_p)
		{
			json_t *tf_json_p = NULL;

			if (active_tf_p)
				{
					tf_json_p = GetTreatmentFactorValuesAsJSON (active_tf_p);

					if (tf_json_p)
						{
							success_flag = true;
						}
					else
						{
							/*
							 * Are there default values for the study
							 */
						}
				}
			else
				{
					success_flag = true;
				}

			if (success_flag)
				{
					param_p = EasyCreateAndAddJSONParameterToParameterSet (& (data_p -> dftsd_base_data), param_set_p, group_p, TFJ_VALUES.npt_type, TFJ_VALUES.npt_name_s, "Treatment Factors to upload", "The data to upload", tf_json_p, PL_ALL);

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

			if (tf_json_p)
				{
					json_decref (tf_json_p);
				}

			json_decref (hints_p);
		}		/* if (hints_p) */


	return param_p;
}


static json_t *GetTableParameterHints (void)
{
	json_t *hints_p = json_array ();

	if (hints_p)
		{
			if (AddColumnParameterHint (S_LABEL_TITLE_S, "The label to use for the Treatment Factor level.", PT_STRING, false, hints_p))
				{
					if (AddColumnParameterHint (S_VALUE_TITLE_S, "The value or description for the Treatment Factor level", PT_STRING, false, hints_p))
						{
							return hints_p;
						}
				}

			json_decref (hints_p);
		}

	return NULL;
}




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



static NamedParameterType TFJ_STUDY = { "Study", PT_STRING };
static NamedParameterType TFJ_TREATMENT = { "Treatment", PT_STRING };

static NamedParameterType TFJ_VALUES = { "Levels", PT_JSON_TABLE };

static const char * const S_LABEL_TITLE_S = "Label";
static const char * const S_VALUE_TITLE_S = "Value";

static const char * const S_EMPTY_LIST_OPTION_S = "<empty>";


bool AddSubmissionTreatmentFactorParams (ServiceData *data_p, ParameterSet *param_set_p, Resource *resource_p)
{
	Parameter *param_p;
	const char *study_id_s = (char *) S_EMPTY_LIST_OPTION_S;

	if ((param_p = EasyCreateAndAddStringParameterToParameterSet (data_p, param_set_p, NULL, TFJ_STUDY.npt_type, TFJ_STUDY.npt_name_s, "Study", "Study to load Treatment Factors for", study_id_s, PL_ALL)) != NULL)
		{
			FieldTrialServiceData *ft_data_p = (FieldTrialServiceData *) data_p;

			if (SetUpStudiesListParameter (ft_data_p, (StringParameter *) param_p, NULL, true))
				{

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

					if (loop_flag)
						{
							node_p = (TreatmentFactorNode *) (node_p -> tfn_node.ln_next_p);
						}

				}		/* while (node_p && loop_flag) */

			if (json_is_array (results_p))
				{
					const size_t num_results = json_array_size (results_p);

					success_flag = true;

					/*
					 * If there's an empty option, add it
					 */
					if (empty_option_flag)
						{
							success_flag = CreateAndAddStringParameterOption (param_p, S_EMPTY_LIST_OPTION_S, S_EMPTY_LIST_OPTION_S);
						}

					if (success_flag)
						{
							if (num_results > 0)
								{
									size_t i = 0;
									const char *param_value_s = GetStringParameterCurrentValue (param_p);

									while ((i < num_results) && success_flag)
										{
											json_t *entry_p = json_array_get (results_p, i);
											Study *study_p = GetStudyFromJSON (entry_p, VF_CLIENT_MINIMAL, data_p);

											if (study_p)
												{
													char *id_s = GetBSONOidAsString (study_p -> st_id_p);

													if (id_s)
														{
															if (param_value_s && (strcmp (param_value_s, id_s) == 0))
																{
																	value_set_flag = true;
																}

															if (!CreateAndAddStringParameterOption (param_p, id_s, study_p -> st_name_s))
																{
																	success_flag = false;
																	PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add param option \"%s\": \"%s\"", id_s,  study_p -> st_name_s);
																}

															FreeCopiedString (id_s);
														}
													else
														{
															success_flag = false;
															PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, entry_p, "Failed to get Study BSON oid");
														}

													FreeStudy (study_p);
												}		/* if (study_p) */


											if (success_flag)
												{
													++ i;
												}

										}		/* while ((i < num_results) && success_flag) */

									/*
									 * If the parameter's value isn't on the list, reset it
									 */
									if (!value_set_flag)
										{
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "param value \"%s\" not on list of existing studies", param_value_s);
										}

								}		/* if (num_results > 0) */
							else
								{
									/* nothing to add */
									success_flag = true;
								}

						}		/* if (success_flag) */


				}		/* if (json_is_array (results_p)) */

			json_decref (results_p);
		}		/* if (results_p) */

	if (success_flag)
		{
			if (active_study_p)
				{
					char *id_s = GetBSONOidAsString (active_study_p -> st_id_p);

					if (id_s)
						{
							success_flag = SetStringParameterDefaultValue (param_p, id_s);
							FreeCopiedString (id_s);
						}
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get id string for active study \"%s\"", active_study_p -> st_name_s);
							success_flag = false;
						}
				}
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
					tf_json_p = GetStudyPlotsForSubmissionTable (active_tf_p, data_p);

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

			if (plots_json_p)
				{
					json_decref (plots_json_p);
				}

			json_decref (hints_p);
		}		/* if (hints_p) */


	return param_p;
}



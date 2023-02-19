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
 * submit_plot.c
 *
 *  Created on: 07 Feb 2023
 *      Author: billy
 */


#include "edit_plot.h"

#include "audit.h"

#include "plot.h"
#include "row.h"

#include "string_parameter.h"
#include "string_array_parameter.h"
#include "time_array_parameter.h"
#include "json_parameter.h"
#include "time_parameter.h"
#include "boolean_parameter.h"

#include "dfw_util.h"

#include "observation.h"
#include "standard_row.h"

/*
 * Static declarations
 */


static NamedParameterType S_PLOT_ID = { "PL Id", PT_STRING };

static NamedParameterType S_MEASURED_VARIABLE_NAME  = { "PL Measured Variable Name", PT_STRING };

static NamedParameterType S_PHENOTYPE_VALUE  = { "PL Phenotype Value", PT_STRING };

static NamedParameterType S_PHENOTYPE_START_DATE  = { "PL Phenotype Start Date", PT_TIME };

static NamedParameterType S_PHENOTYPE_END_DATE  = { "PL Phenotype Start Date", PT_TIME };

static NamedParameterType S_PHENOTYPE_CORRECTED  = { "PL Phenotype Corrected", PT_BOOLEAN };



static NamedParameterType S_STUDY_NAME  = { "PL Study Name", PT_STRING };

static NamedParameterType S_STUDY_ID  = { "PL Study Id", PT_STRING };

static NamedParameterType S_OBSERVATIONS = { "PL Observations", PT_JSON_TABLE };




static const char *GetPlotEditingServiceName (const Service *service_p);

static const char *GetPlotEditingServiceDescription (const Service *service_p);

static const char *GetPlotEditingServiceAlias (const Service *service_p);

static const char *GetPlotEditingServiceInformationUri (const Service *service_p);

static ParameterSet *GetPlotEditingServiceParameters (Service *service_p, DataResource *resource_p, UserDetails *user_p);

static bool GetPlotEditingServiceParameterTypesForNamedParameters (const Service *service_p, const char *param_name_s, ParameterType *pt_p);

static void ReleasePlotEditingServiceParameters (Service *service_p, ParameterSet *params_p);

static ServiceJobSet *RunPlotEditingService (Service *service_p, ParameterSet *param_set_p, UserDetails *user_p, ProvidersStateTable *providers_p);


static bool ClosePlotEditingService (Service *service_p);

static ServiceMetadata *GetPlotEditingServiceMetadata (Service *service_p);

static bool AddEditPlotParams (ServiceData *data_p, ParameterSet *param_set_p, DataResource *resource_p);


static const char *GetPlotIdFromResource (DataResource *resource_p, const NamedParameterType plot_param_type);

static bool AddDefaultsFromPlot (Plot *plot_p, ServiceData *data_p, ParameterSet *params_p);

static json_t *GetTableParameterHints (void);

static bool AddExistingPhenotypeParameters (ParameterSet *params_p, const StandardRow *row_p, FieldTrialServiceData *data_p);

static bool AddPhenotypeParameters (Plot *active_plot_p, const char *child_group_name_s, ParameterSet *param_set_p, ParameterGroup *parent_group_p, ServiceData *data_p);


static bool AddStringToJSONArray (json_t *array_p, const char *value_s);


static bool AddValidDateToJSONArray (json_t *array_p, const struct tm *time_p);


static bool AddStringToJSONArray (json_t *array_p, const char *value_s);

static bool AddBooleanToJSONArray (json_t *array_p, const bool corrected_flag);

static bool AddStringPhenotypeValueToJSON (json_t *array_p, Observation *observation_p, ObservationValueType ovt);

static bool PopulateExistingValuesInJSON (Row *active_row_p, json_t **existing_mv_names_pp, json_t **existing_phenotype_values_pp,
																					json_t **existing_phenotype_start_dates_pp, json_t **existing_phenotype_end_dates_pp,
																					json_t **existing_phenotype_corrected_pp);

/*
 * API definitions
 */


Service *GetPlotEditingService (GrassrootsServer *grassroots_p)
{
	Service *service_p = (Service *) AllocMemory (sizeof (Service));

	if (service_p)
		{
			FieldTrialServiceData *data_p = AllocateFieldTrialServiceData ();

			if (data_p)
				{
					if (InitialiseService (service_p,
														 GetPlotEditingServiceName,
														 GetPlotEditingServiceDescription,
														 GetPlotEditingServiceAlias,
														 GetPlotEditingServiceInformationUri,
														 RunPlotEditingService,
														 NULL,
														 GetPlotEditingServiceParameters,
														 GetPlotEditingServiceParameterTypesForNamedParameters,
														 ReleasePlotEditingServiceParameters,
														 ClosePlotEditingService,
														 NULL,
														 false,
														 SY_SYNCHRONOUS,
														 (ServiceData *) data_p,
														 GetPlotEditingServiceMetadata,
														 NULL,
														 grassroots_p))
						{

							if (ConfigureFieldTrialService (data_p, grassroots_p))
								{
									return service_p;
								}

						}		/* if (InitialiseService (.... */

					FreeFieldTrialServiceData (data_p);
				}

			FreeMemory (service_p);
		}		/* if (service_p) */

	return NULL;
}





static const char *GetPlotEditingServiceName (const Service * UNUSED_PARAM (service_p))
{
	return "Edit Field Trial Plot";
}


static const char *GetPlotEditingServiceDescription (const Service * UNUSED_PARAM (service_p))
{
	return "A service to edit an individual field trial plot.";
}


static const char *GetPlotEditingServiceAlias (const Service * UNUSED_PARAM (service_p))
{
	return DFT_GROUP_ALIAS_PREFIX_S SERVICE_GROUP_ALIAS_SEPARATOR "edit_plot";
}


static const char *GetPlotEditingServiceInformationUri (const Service *service_p)
{
	const char *url_s = GetServiceInformationPage (service_p);

	if (!url_s)
		{
			url_s = "https://grassroots.tools/docs/user/services/field_trial/edit_plot.md";
		}

	return url_s;
}



static bool GetPlotEditingServiceParameterTypesForNamedParameters (const Service *service_p, const char *param_name_s, ParameterType *pt_p)
{
	const NamedParameterType params [] =
		{
			S_PLOT_ID,
			S_MEASURED_VARIABLE_NAME,
			S_PHENOTYPE_VALUE,
			S_PHENOTYPE_START_DATE,
			S_PHENOTYPE_END_DATE,
			S_PHENOTYPE_CORRECTED,
			S_STUDY_NAME,
			S_STUDY_ID,
			NULL
		};

	return DefaultGetParameterTypeForNamedParameter (param_name_s, pt_p, params);
}



static ParameterSet *GetPlotEditingServiceParameters (Service *service_p, DataResource *resource_p, UserDetails * UNUSED_PARAM (user_p))
{
	ParameterSet *params_p = AllocateParameterSet ("Plot edit service parameters", "The parameters used for the plot editing service");

	if (params_p)
		{
			ServiceData *data_p = service_p -> se_data_p;

			if (AddEditPlotParams (data_p, params_p, resource_p))
				{
					return params_p;
				}
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "AddEditingFieldTrialParams failed");
				}

			FreeParameterSet (params_p);
		}
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate %s ParameterSet", GetPlotEditingServiceName (service_p));
		}

	return NULL;
}




static void ReleasePlotEditingServiceParameters (Service * UNUSED_PARAM (service_p), ParameterSet *params_p)
{
	FreeParameterSet (params_p);
}




static bool ClosePlotEditingService (Service *service_p)
{
	bool success_flag = true;

	FreeFieldTrialServiceData ((FieldTrialServiceData *) (service_p -> se_data_p));

	return success_flag;
}



static ServiceJobSet *RunPlotEditingService (Service *service_p, ParameterSet *param_set_p, UserDetails * UNUSED_PARAM (user_p), ProvidersStateTable * UNUSED_PARAM (providers_p))
{
	FieldTrialServiceData *data_p = (FieldTrialServiceData *) (service_p -> se_data_p);

	service_p -> se_jobs_p = AllocateSimpleServiceJobSet (service_p, NULL, "Submit Plot");

	if (service_p -> se_jobs_p)
		{
			ServiceJob *job_p = GetServiceJobFromServiceJobSet (service_p -> se_jobs_p, 0);

			LogParameterSet (param_set_p, job_p);

			SetServiceJobStatus (job_p, OS_FAILED_TO_START);

			//if (!RunForEditPlotParams (data_p, param_set_p, job_p))
				{

				}		/* if (!RunForEditingPlotParams (data_p, param_set_p, job_p)) */


			LogServiceJob (job_p);
		}		/* if (service_p -> se_jobs_p) */

	return service_p -> se_jobs_p;
}



static ServiceMetadata *GetPlotEditingServiceMetadata (Service *service_p)
{
	const char *term_url_s = CONTEXT_PREFIX_EDAM_ONTOLOGY_S "topic_3810";
	SchemaTerm *category_p = AllocateSchemaTerm (term_url_s, "Agricultural science",
		"Multidisciplinary study, research and development within the field of agriculture.");

	if (category_p)
		{
			SchemaTerm *subcategory_p;

			term_url_s = CONTEXT_PREFIX_EDAM_ONTOLOGY_S "operation_3431";
			subcategory_p = AllocateSchemaTerm (term_url_s, "Deposition", "Deposit some data in a database or some other type of repository or software system.");

			if (subcategory_p)
				{
					ServiceMetadata *metadata_p = AllocateServiceMetadata (category_p, subcategory_p);

					if (metadata_p)
						{
							SchemaTerm *input_p;

							term_url_s = CONTEXT_PREFIX_EXPERIMENTAL_FACTOR_ONTOLOGY_S "EFO_0000651";
							input_p = AllocateSchemaTerm (term_url_s, "phenotype", "The observable form taken by some character (or group of characters) "
																						"in an individual or an organism, excluding pathology and disease. The detectable outward manifestations of a specific genotype.");

							if (input_p)
								{
									if (AddSchemaTermToServiceMetadataInput (metadata_p, input_p))
										{
											/* Genotype */
											term_url_s = CONTEXT_PREFIX_EXPERIMENTAL_FACTOR_ONTOLOGY_S "EFO_0000513";
											input_p = AllocateSchemaTerm (term_url_s, "genotype", "Information, making the distinction between the actual physical material "
												"(e.g. a cell) and the information about the genetic content (genotype).");

											if (input_p)
												{
													if (AddSchemaTermToServiceMetadataInput (metadata_p, input_p))
														{
															return metadata_p;
														}		/* if (AddSchemaTermToServiceMetadataOutput (metadata_p, output_p)) */
													else
														{
															PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add output term %s to service metadata", term_url_s);
															FreeSchemaTerm (input_p);
														}

												}
										}		/* if (AddSchemaTermToServiceMetadataInput (metadata_p, input_p)) */
									else
										{
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add input term %s to service metadata", term_url_s);
											FreeSchemaTerm (input_p);
										}

								}		/* if (input_p) */
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate input term %s for service metadata", term_url_s);
								}

						}		/* if (metadata_p) */
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate service metadata");
						}

				}		/* if (subcategory_p) */
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate sub-category term %s for service metadata", term_url_s);
				}

		}		/* if (category_p) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate category term %s for service metadata", term_url_s);
		}

	return NULL;
}


static bool AddEditPlotParams (ServiceData *data_p, ParameterSet *param_set_p, DataResource *resource_p)
{
	FieldTrialServiceData *dfw_data_p = (FieldTrialServiceData *) data_p;
	bool success_flag = false;
	Parameter *param_p = NULL;
	ParameterGroup *group_p = CreateAndAddParameterGroupToParameterSet ("Plot", false, data_p, param_set_p);
	Plot *active_plot_p = NULL;
	const char *active_plot_id_s = GetPlotIdFromResource (resource_p, S_PLOT_ID);


	if (active_plot_id_s)
		{
			active_plot_p = GetPlotByIdString (active_plot_id_s, VF_CLIENT_FULL, dfw_data_p);
		}


	if ((param_p = EasyCreateAndAddStringParameterToParameterSet (data_p, param_set_p, group_p, S_PLOT_ID.npt_type, S_PLOT_ID.npt_name_s, "Plot ID", "The Plot to edit", active_plot_id_s, PL_ALL)) != NULL)
		{
//			param_p -> pa_read_only_flag = true;

			param_p -> pa_refresh_service_flag = true;

			if (AddDefaultsFromPlot (active_plot_p, data_p, param_set_p))
				{
					const char *study_name_s = NULL;
					char *study_id_s = NULL;

					if (active_plot_p && (active_plot_p -> pl_parent_p))
						{
							study_name_s = active_plot_p -> pl_parent_p -> st_name_s;
							study_id_s = GetBSONOidAsString (active_plot_p -> pl_parent_p -> st_id_p);
						}

					if ((param_p = EasyCreateAndAddStringParameterToParameterSet (data_p, param_set_p, group_p, S_STUDY_NAME.npt_type, S_STUDY_NAME.npt_name_s, "Study", "The Study that contains this plot", study_name_s, PL_ALL)) != NULL)
						{
							param_p -> pa_read_only_flag = true;

							if ((param_p = EasyCreateAndAddStringParameterToParameterSet (data_p, param_set_p, group_p, S_STUDY_ID.npt_type, S_STUDY_ID.npt_name_s, "Study ID", "The Study ID", study_id_s, PL_ALL)) != NULL)
								{
									const char *child_group_name_s = "child";

									param_p -> pa_read_only_flag = true;

									if (AddPhenotypeParameters (active_plot_p, child_group_name_s, param_set_p, group_p, data_p))
										{
											success_flag = true;
										}		/* if (AddPhenotypeParameters (active_plot_p, child_group_name_s, param_set_p, group_p, dfw_data_p)) */
									else
										{
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "AddPhenotypeParameters () failed");
										}

								}
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", S_STUDY_ID.npt_name_s);
								}

						}
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", S_STUDY_NAME.npt_name_s);
						}


					if (study_id_s)
						{
							FreeBSONOidString (study_id_s);
						}

				}		/* if (AddPlotDefaultsFromStudy (active_plot_p, data_p, param_set_p)) */


		}
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", S_PLOT_ID.npt_name_s);
		}

	if (active_plot_p)
		{
			FreePlot (active_plot_p);
		}

	return success_flag;
}



static const char *GetPlotIdFromResource (DataResource *resource_p, const NamedParameterType plot_param_type)
{
	const char *plot_id_s = NULL;

	/*
	 * Have we been set some parameter values to refresh from?
	 */
	if (resource_p && (resource_p -> re_data_p))
		{
			const json_t *param_set_json_p = json_object_get (resource_p -> re_data_p, PARAM_SET_KEY_S);

			if (param_set_json_p)
				{
					json_t *params_json_p = json_object_get (param_set_json_p, PARAM_SET_PARAMS_S);

					if (params_json_p)
						{
							plot_id_s = GetStringDefaultValueFromJSON (plot_param_type.npt_name_s, params_json_p);
						}
				}
		}

	return plot_id_s;
}



static bool AddDefaultsFromPlot (Plot *plot_p, ServiceData *data_p, ParameterSet *params_p)
{
	return true;
}



static bool AddExistingPhenotypeParameters (ParameterSet *params_p, const StandardRow *row_p, FieldTrialServiceData *data_p)
{
	ParameterGroup *group_p = CreateAndAddParameterGroupToParameterSet ("Phenotypes", false, & (data_p -> dftsd_base_data), params_p);

	if (group_p)
		{
			Parameter *param_p = NULL;
			json_t *observations_array_p = NULL;

			if (row_p && (row_p -> sr_observations_p))
				{
					if (row_p -> sr_observations_p -> ll_size > 0)
						{
							observations_array_p = json_array ();

							if (observations_array_p)
								{
									bool success_flag = true;
									ObservationNode *node_p = ((ObservationNode *) (row_p -> sr_observations_p -> ll_head_p));

									while (node_p && success_flag)
										{
											Observation *observation_p = node_p -> on_observation_p;
											json_t *observation_json_p = GetObservationAsJSON (observation_p, VF_CLIENT_FULL);

											if (observation_json_p)
												{
													if (json_array_append_new (observations_array_p, observation_json_p) == 0)
														{
															node_p = (ObservationNode *) (node_p -> on_node.ln_next_p);
														}
													else
														{
															PrintJSONToErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, observation_json_p, "Failed to add observation from row " UINT32_FMT " as JSON", row_p -> sr_base.ro_by_study_index);

															json_decref (observation_json_p);

															success_flag = false;
														}
												}		/* if (factors_p) */
											else
												{
													PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "GetObservationAsJSON () failed");
													success_flag = false;
												}

										}		/* while (node_p) */

									if (!success_flag)
										{
											json_decref (observations_array_p);
											observations_array_p = NULL;
										}

								}		/* if (factors_array_p) */

						}		/* if (num_treatments > 1) */
				}


			param_p = EasyCreateAndAddJSONParameterToParameterSet (& (data_p -> dftsd_base_data), params_p, group_p, S_OBSERVATIONS.npt_type, S_OBSERVATIONS.npt_name_s, "Observations", "The current Observations", observations_array_p, PL_ALL);

			if (param_p)
				{
					json_t *hints_p = GetTableParameterHints ();

					if (hints_p)
						{
							if (AddParameterKeyJSONValuePair (param_p, PA_TABLE_COLUMN_HEADINGS_S, hints_p))
								{
									return true;
								}

							json_decref (hints_p);
						}
				}


		}

	return false;
}


static json_t *GetTableParameterHints (void)
{
	/*
	headers_s = ConcatenateVarargsStrings (S_SOWING_TITLE_S, delim_s, S_HARVEST_TITLE_S, delim_s, S_WIDTH_TITLE_S, delim_s, S_LENGTH_TITLE_S, delim_s, S_ROW_TITLE_S, delim_s, S_COLUMN_TITLE_S, delim_s,
																				 PL_REPLICATE_TITLE_S, delim_s, S_RACK_TITLE_S, delim_s, S_MATERIAL_TITLE_S, delim_s, S_TRIAL_DESIGN_TITLE_S, delim_s, S_GROWING_CONDITION_TITLE_S, delim_s, S_TREATMENT_TITLE_S, delim_s, NULL);
	 */
	json_t *hints_p = json_array ();

	if (hints_p)
		{
			if (AddColumnParameterHint (OB_PHENOTYPE_S, "If left blank, then the *Sowing date* specified for the Study will be used.", PT_STRING, false, hints_p))
				{
					if (AddColumnParameterHint (OB_START_DATE_S, "Harvest date of the plot. If this is blank, then the *Harvest date* specified for the Study will be used.", PT_STRING, false, hints_p))
						{
							if (AddColumnParameterHint (OB_END_DATE_S, " If this is blank, then the *Plot width* specified for the Study will be used.", PT_STRING, false, hints_p))
								{
									if (AddColumnParameterHint (OB_RAW_VALUE_S, " If this is blank, then the *Plot height* specified for the Study will be used.", PT_TIME, false, hints_p))
										{
											if (AddColumnParameterHint (OB_CORRECTED_VALUE_S, ".If this is blank, then the *Plot height* specified for the Study will be used.", PT_TIME, false, hints_p))
												{
													return hints_p;
												}
										}
								}
						}
				}

			json_decref (hints_p);
		}

	return NULL;
}




static bool AddValidDateToJSONArray (json_t *array_p, const struct tm *time_p)
{
	bool b = false;

	if (time_p)
		{
			char *time_s = GetTimeAsString (time_p, true, NULL);

			if (time_s)
				{
					if (AddStringToJSONArray (array_p, time_s))
						{
							b = true;
						}

					FreeTimeString (time_s);
				}
		}
	else
		{
			if (json_array_append_new (array_p, json_null ()) == 0)
				{
					b = true;
				}
		}

	return b;
}


static bool AddBooleanToJSONArray (json_t *array_p, const bool corrected_flag)
{
	json_t *corrected_p = NULL;

	if (corrected_flag)
		{
			corrected_p = json_true ();
		}
	else
		{
			corrected_p = json_false ();
		}


	if (corrected_p)
		{
			if (json_array_append_new (array_p, corrected_p) == 0)
				{
					return true;
				}
			else
				{
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, array_p, "Failed to append \"%s\"", corrected_flag ? "true" : "false");
				}

			json_decref (corrected_p);
		}
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to create json object for \"%s\"", corrected_flag ? "true" : "false");
		}

	return false;
}


static bool AddStringPhenotypeValueToJSON (json_t *array_p, Observation *observation_p, ObservationValueType ovt)
{
	char *value_s = NULL;
	bool free_value_flag = false;

	if (observation_p -> ob_get_value_as_string_fn (observation_p, ovt, &value_s, &free_value_flag))
		{
			if (value_s)
				{
					if (AddStringToJSONArray (array_p, value_s))
						{
							return true;
						}

					if (free_value_flag)
						{
							FreeCopiedString (value_s);
						}
				}
			else
				{
					if (json_array_append_new (array_p, json_null ()) == 0)
						{
							return true;
						}
				}
		}

	return false;
}


static bool AddStringToJSONArray (json_t *array_p, const char *value_s)
{
	if (value_s)
		{
			json_t *value_p = json_string (value_s);

			if (value_p)
				{
					if (json_array_append_new (array_p, value_p) == 0)
						{
							return true;
						}
					else
						{
							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, array_p, "Failed to append \"%s\"", value_s);
						}

					json_decref (value_p);
				}
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to create json string for \"%s\"", value_s);
				}

		}
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "input value is null");
		}

	return false;
}


static bool PopulateExistingValuesInJSON (Row *active_row_p, json_t **existing_mv_names_pp, json_t **existing_phenotype_values_pp,
																					json_t **existing_phenotype_start_dates_pp, json_t **existing_phenotype_end_dates_pp,
																					json_t **existing_phenotype_corrected_pp)
{
	bool success_flag = false;

	if (active_row_p -> ro_type == RT_STANDARD)
		{
			StandardRow *row_p = (StandardRow *) active_row_p;
			const uint32 num_entries = row_p -> sr_observations_p -> ll_size;

			if (num_entries > 0)
				{
					char **existing_mv_names_ss = (char **) AllocMemoryArray (num_entries, sizeof (char *));

					if (existing_mv_names_ss)
						{
							json_t *existing_phenotype_values_p = json_array ();

							if (existing_phenotype_values_p)
									{
										json_t *existing_phenotype_start_dates_p = json_array ();

										if (existing_phenotype_start_dates_p)
											{
												json_t *existing_phenotype_end_dates_p = json_array ();

												if (existing_phenotype_end_dates_p)
													{
														json_t *existing_phenotype_corrected_p = json_array ();

														if (existing_phenotype_corrected_p)
															{
																ObservationNode *obs_node_p = (ObservationNode *) (row_p -> sr_observations_p -> ll_head_p);

																success_flag = true;

																while (obs_node_p && success_flag)
																	{
																		Observation *observation_p = obs_node_p -> on_observation_p;
																		const char *mv_s = GetMeasuredVariableName (observation_p -> ob_phenotype_p);

																		if (AddStringToJSONArray (existing_mv_names_p, mv_s))
																			{
																				if (AddValidDateToJSONArray (existing_phenotype_start_dates_p, observation_p -> ob_start_date_p))
																					{
																						if (AddValidDateToJSONArray (existing_phenotype_start_dates_p, observation_p -> ob_end_date_p))
																							{
																								if (AddStringPhenotypeValueToJSON (existing_phenotype_corrected_p, observation_p, OVT_CORRECTED_VALUE))
																									{
																										if (AddStringPhenotypeValueToJSON (existing_phenotype_values_p, observation_p, OVT_RAW_VALUE))
																											{
																												obs_node_p = (ObservationNode *) (obs_node_p -> on_node.ln_next_p);
																											}
																										else
																											{
																												success_flag = false;
																											}
																									}
																								else
																									{
																										success_flag = false;
																									}
																							}
																						else
																							{
																								success_flag = false;
																							}
																					}
																				else
																					{
																						success_flag = false;
																					}

																			}		/* if (AddStringToJSONArray (existing_mv_names_p, mv_s)) */

																	}		/* while (obs_node_p && loop_flag) */

																if (success_flag)
																	{
																		*existing_mv_names_pp = existing_mv_names_p;
																		*existing_phenotype_values_pp = existing_phenotype_values_p;
																		*existing_phenotype_start_dates_pp = existing_phenotype_start_dates_p;
																		*existing_phenotype_end_dates_pp = existing_phenotype_end_dates_p;
																		*existing_phenotype_corrected_pp = existing_phenotype_corrected_p;

																		return true;
																	}		/* if (success_flag) */

																json_decref (existing_phenotype_corrected_p);
															}		/* if (existing_phenotype_corrected_p) */
														else
															{
																PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate existing_phenotype_corrected_p");
															}

														json_decref (existing_phenotype_end_dates_p);
													}		/* if (existing_phenotype_end_dates_p) */
												else
													{
														PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate existing_phenotype_end_dates_p");
													}

												json_decref (existing_phenotype_start_dates_p);
											}		/* if (existing_phenotype_start_dates_p) */
										else
											{
												PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate existing_phenotype_start_dates_p");
											}

										json_decref (existing_phenotype_values_p);
									}		/* if (existing_phenotype_values_p) */
								else
									{
										PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate existing_phenotype_values_p");
									}

							json_decref (existing_mv_names_p);
						}		/* if (existing_mv_names_p) */
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate existing_mv_names_p");
						}


				}		/* if (row_p -> sr_observations_p -> ll_size > 0) */
			else
				{
					return true;
				}
		}

	return false;
}


static bool AddPhenotypeParameters (Plot *active_plot_p, const char *child_group_name_s, ParameterSet *param_set_p, ParameterGroup *parent_group_p, ServiceData *data_p)
{
	bool success_flag = false;
	Row *active_row_p;

	json_t *existing_mv_names_p;
	json_t *existing_phenotype_values_p;
	json_t *existing_phenotype_start_dates_p;
	json_t *existing_phenotype_end_dates_p;
	json_t *existing_phenotype_corrected_p;


	if (PopulateExistingValuesInJSON (active_row_p, &existing_mv_names_p, &existing_phenotype_values_p,
																		&existing_phenotype_start_dates_p, &existing_phenotype_end_dates_p,
																		&existing_phenotype_corrected_p))
		{
			ParameterGroup *child_group_p = CreateAndAddParameterGroupChild (parent_group_p, child_group_name_s, true, false);

			if (child_group_p)
				{
					const size_t num_entries = json_array_size (existing_mv_names_p);
					Parameter *param_p = EasyCreateAndAddStringArrayParameterToParameterSet (data_p, param_set_p, child_group_p, S_MEASURED_VARIABLE_NAME.npt_type, S_MEASURED_VARIABLE_NAME.npt_name_s, "Measured Variable Name", "The Name of the Measured Variable to add a phenotype for", existing_mv_names_p, PL_ALL);

					if (param_p)
						{
							child_group_p -> pg_repeatable_param_p = param_p;

							if ((param_p = EasyCreateAndAddStringArrayParameterToParameterSet (data_p, param_set_p, child_group_p, S_PHENOTYPE_VALUE.npt_type, S_PHENOTYPE_VALUE.npt_name_s, "Phenotype", "The observed phenotypic value", existing_phenotype_values_p, num_entries, PL_ALL)) != NULL)
								{
									if ((param_p = EasyCreateAndAddTimeArrayParameterToParameterSet (data_p, param_set_p, child_group_p, S_PHENOTYPE_START_DATE.npt_name_s, "Start Date", "The date when the observation happened/started", , num_entries, PL_ALL)) != NULL)
										{
											if ((param_p = EasyCreateAndAddTimeArrayParameterToParameterSet (data_p, param_set_p, child_group_p, S_PHENOTYPE_END_DATE.npt_name_s, "End Date", "The date when the observation finished if it differs from the start date", existing_phenotype_end_dates_p, num_entries, PL_ALL)) != NULL)
												{
													if ((param_p = EasyCreateAndAddBooleanParameterToParameterSet (data_p, param_set_p, child_group_p, S_PHENOTYPE_CORRECTED.npt_name_s, "Corrected?", "Tick this to specify if this Phenotypic value is a correction of a previous raw value", NULL, PL_ALL)) != NULL)
														{
															success_flag = true;
														}
													else
														{
															PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", S_PHENOTYPE_CORRECTED.npt_name_s);
														}

												}
											else
												{
													PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", S_PHENOTYPE_END_DATE.npt_name_s);
												}

										}
									else
										{
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", S_PHENOTYPE_START_DATE.npt_name_s);
										}

								}
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", S_PHENOTYPE_VALUE.npt_name_s);
								}

						}
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", S_MEASURED_VARIABLE_NAME.npt_name_s);
						}

				}		/* if (child_group_p) */
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to create child parameter group \"%s\"", child_group_name_s);
				}

			if (existing_mv_names_p)
				{
					json_decref (existing_mv_names_p);
				}

			if (existing_phenotype_values_p)
				{
					json_decref (existing_phenotype_values_p);
				}

			if (existing_phenotype_start_dates_p)
				{
					json_decref (existing_phenotype_start_dates_p);
				}

			if (existing_phenotype_end_dates_p)
				{
					json_decref (existing_phenotype_end_dates_p);
				}

			if (existing_phenotype_corrected_p)
				{
					json_decref (existing_phenotype_corrected_p);
				}


		}		/* 	if (PopulateExistingValuesInJSON (active_row_p, &existing_mv_names_p, &existing_phenotype_values_p,
																		&existing_phenotype_start_dates_p, &existing_phenotype_end_dates_p,
																		&existing_phenotype_corrected_p)) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "PopulateExistingValuesInJSON () failed for " UNIT32_FMT " in \"%s\"", active_row_p -> ro_by_study_index, active_row_p -> ro_study_p -> st_name_s);
		}


	return success_flag;
}

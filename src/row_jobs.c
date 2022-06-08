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
 * row_jobs.c
 *
 *  Created on: 28 Oct 2018
 *      Author: billy
 */


#include "measured_variable_jobs.h"
#include "row_jobs.h"
#include "plot_jobs.h"
#include "string_utils.h"
#include "study_jobs.h"
#include "math_utils.h"
#include "time_util.h"
#include "observation.h"
#include "treatment.h"
#include "treatment_factor.h"
#include "treatment_jobs.h"
#include "treatment_factor_value.h"

#include "char_parameter.h"
#include "json_parameter.h"
#include "string_parameter.h"

#include "frictionless_data_util.h"

/*
 * static declarations
 */
static const char * const S_RACK_S = "Rack";
static const char * const S_PLOT_INDEX_S = "Plot index";



/**
 * Extract the Observation metadata from the column heading
 */
static OperationStatus GetObservationMetadata (const char *key_s, MeasuredVariable **measured_variable_pp, struct tm **start_date_pp, struct tm **end_date_pp, bool *corrected_value_flag_p, uint32 *ob_index_p, ServiceJob *job_p, const uint32 row_index, MEM_FLAG *measured_variable_mem_p,  FieldTrialServiceData *data_p);


/**
 * Report any errors from GetObservationMetadata ()
 */
static void ReportObservationMetadataError (ServiceJob *job_p, const char *prefix_s, const char *key_s, const char *value_s);


/*
 * API Definitions
 */


bool AddRowFrictionlessDataDetails (const Row *row_p, json_t *row_fd_p, const FieldTrialServiceData *service_data_p, const char * const null_sequence_s)
{
	bool success_flag = true;

	if (SetJSONInteger (row_fd_p, PL_INDEX_TABLE_TITLE_S, row_p -> ro_by_study_index))
		{
			if (SetJSONInteger (row_fd_p, S_RACK_S, row_p -> ro_rack_index))
				{
					if ((! (row_p -> ro_material_p)) || (SetJSONString (row_fd_p, PL_ACCESSION_TABLE_TITLE_S, row_p -> ro_material_p -> ma_accession_s)))
						{
							if (row_p -> ro_replicate_control_flag)
								{
									success_flag = SetJSONString (row_fd_p, PL_REPLICATE_TITLE_S, RO_REPLICATE_CONTROL_S);
								}
							else
								{
									success_flag = SetJSONInteger (row_fd_p, PL_REPLICATE_TITLE_S, row_p -> ro_rack_index);
								}

							if (success_flag)
								{
									/*
									 * Add the treatment factors
									 */

									if (row_p -> ro_treatment_factor_values_p)
										{
											uint32 num_added = 0;
											TreatmentFactorValueNode *tfv_node_p = (TreatmentFactorValueNode *) (row_p -> ro_treatment_factor_values_p -> ll_head_p);

											while (tfv_node_p && success_flag)
												{
													TreatmentFactorValue *tf_value_p = tfv_node_p -> tfvn_value_p;

													const char *url_s = GetTreatmentFactorUrl (tf_value_p -> tfv_factor_p);

													if (url_s)
														{
															if (SetFDTableString (row_fd_p, url_s, tf_value_p -> tfv_label_s, null_sequence_s))
																{
																	++ num_added;
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

													if (success_flag)
														{
															tfv_node_p = (TreatmentFactorValueNode *) (tfv_node_p -> tfvn_node.ln_next_p);
														}
												}

										}		/* if (row_p -> ro_treatment_factor_values_p) */


									if (success_flag)
										{
											if (row_p -> ro_observations_p)
												{
													uint32 num_added = 0;
													ObservationNode *obs_node_p = (ObservationNode *) (row_p -> ro_observations_p -> ll_head_p);

													while (obs_node_p && success_flag)
														{
															Observation *obs_p = obs_node_p -> on_observation_p;

															if (AddObservationValuesToFrictionlessData (obs_p, row_fd_p))
																{
																	obs_node_p = (ObservationNode *) (obs_node_p -> on_node.ln_next_p);
																	++ num_added;
																}
															else
																{
																	success_flag = false;
																}

														}		/* while (obs_node_p && success_flag) */

												}		/* if (row_p -> ro_observations_p) */



										}		/* if (success_flag) */

								}		/* if (success_flag) */

						}		/* if ((! (row_p -> ro_material_p)) || (SetJSONString (row_fd_p, PL_ACCESSION_TABLE_TITLE_S, row_p -> ro_material_p -> ma_accession_s))) */


				}		/* if (SetJSONInteger (row_fd_p, RO_RACK_INDEX_S, row_p -> ro_rack_index)) */

		}



	return success_flag;
}




/*
 * static definitions
 */




OperationStatus AddTreatmentFactorValuesToRow (Row *row_p, json_t *plot_json_p, Study *study_p, FieldTrialServiceData *data_p)
{
	OperationStatus status = OS_IDLE;
	void *temp_p = NULL;
	const char *key_s;
	json_t *value_p;
	size_t num_treatments = 0;
	size_t num_added = 0;

	json_object_foreach_safe (plot_json_p, temp_p, key_s, value_p)
		{
			/* Is it a treatment? */
			Treatment *treatment_p = GetTreatmentByURL (key_s, VF_STORAGE, data_p);


			if (treatment_p)
				{
					/*
					 * Does the Study have a TreatmentFactor for this
					 * Treatment?
					 */
					TreatmentFactor *tf_p = GetTreatmentFactorForStudy (study_p, treatment_p -> tr_id_p, data_p);

					if (tf_p)
						{
							if (json_is_string (value_p))
								{
									const char *name_s = json_string_value (value_p);
									const char *value_s = GetTreatmentFactorValue (tf_p, name_s);

									/* Is it a valid defined label? */
									if (value_s)
										{
											if (AddTreatmentFactorValueToRowByParts (row_p, tf_p, name_s))
												{
													++ num_added;
												}
										}
								}


							//FreeTreatmentFactor (tf_p);
						}

					FreeTreatment (treatment_p);

					/*
					 * We know that it's a Treatment so remove it from any later processing
					 * and increment the number that we've seen
					 */
					json_object_del (plot_json_p, key_s);
					++ num_treatments;

				}		/* if (treatment_p) */

		}		/* json_array_foreach_safe (plot_json_p, temp_p, key_s, value_p) */

	if (num_treatments > 0)
		{
			if (num_added == num_treatments)
				{
					status = OS_SUCCEEDED;
				}
			else if (num_added > 0)
				{
					status = OS_PARTIALLY_SUCCEEDED;
				}
			else
				{
					status = OS_FAILED;
				}
		}

	return status;
}



//OperationStatus OldAddObservationValuesToRow (Row *row_p, json_t *observation_json_p, Study *study_p, const FieldTrialServiceData *data_p)
//{
//	OperationStatus status = OS_FAILED;
//
//	bool loop_success_flag = true;
//	void *iterator_p = json_object_iter (observation_json_p);
//	size_t imported_obs = 0;
//	size_t total_obs = 0;
//	LinkedList *processed_keys_p = AllocateStringLinkedList ();
//
//	if (processed_keys_p)
//		{
//			while (iterator_p && loop_success_flag)
//				{
//					const char *key_s = json_object_iter_key (iterator_p);
//					json_t *value_p = json_object_iter_value (iterator_p);
//
//					/*
//					 * ignore our column names
//					 */
//					if ((strcmp (key_s, S_PLOT_INDEX_S) != 0) && (strcmp (key_s, S_RACK_S) != 0))
//						{
//							/*
//							 * make sure it isn't a date column
//							 */
//							const char * const DATE_ENDING_S = " date";
//							const char * const CORRECTED_ENDING_S = " corrected";
//
//							if ((!DoesStringEndWith (key_s, DATE_ENDING_S)) && (!DoesStringEndWith (key_s, CORRECTED_ENDING_S)))
//								{
//									MeasuredVariable *measured_variable_p = GetMeasuredVariableByVariableName (key_s, data_p);
//
//									if (measured_variable_p)
//										{
//											Observation *observation_p = NULL;
//											bool added_phenotype_flag = false;
//											const char *raw_value_s = json_string_value (value_p);
//											const char *corrected_value_s = NULL;
//											char *column_header_s = NULL;
//
//											/* corrected value */
//											column_header_s = ConcatenateStrings (key_s, CORRECTED_ENDING_S);
//											if (column_header_s)
//												{
//													corrected_value_s = GetJSONString (observation_json_p, column_header_s);
//													FreeCopiedString (column_header_s);
//												}		/* if (column_header_s) */
//
//
//											if ((!IsStringEmpty (raw_value_s)) || (!IsStringEmpty (corrected_value_s)))
//												{
//													const char *growth_stage_s = NULL;
//													const char *method_s = NULL;
//													ObservationNature nature = ON_ROW;
//													Instrument *instrument_p = NULL;
//													bson_oid_t *observation_id_p = GetNewBSONOid ();
//													struct tm *observation_date_p = NULL;
//
//													++ total_obs;
//
//													/*
//													 * assume failure to import
//													 */
//													loop_success_flag = false;
//
//													/* date */
//													column_header_s = ConcatenateStrings (key_s, DATE_ENDING_S);
//													if (column_header_s)
//														{
//															const char *date_s = GetJSONString (observation_json_p, column_header_s);
//
//															if (date_s)
//																{
//																	observation_date_p = GetTimeFromString (date_s);
//
//																	if (observation_date_p)
//																		{
//																			if (!AddStringToStringLinkedList (processed_keys_p, column_header_s, MF_DEEP_COPY))
//																				{
//
//																				}
//																		}
//																}
//
//															FreeCopiedString (column_header_s);
//														}		/* if (column_header_s) */
//
//													if (observation_id_p)
//														{
//															observation_p = AllocateObservation (observation_id_p, observation_date_p, measured_variable_p, raw_value_s, corrected_value_s, growth_stage_s, method_s, instrument_p, nature);
//
//															if (observation_p)
//																{
//																	if (AddObservationToRow (row_p, observation_p))
//																		{
//																			++ imported_obs;
//																			added_phenotype_flag = true;
//																			loop_success_flag = true;
//																		}
//																	else
//																		{
//																			char id_s [MONGO_OID_STRING_BUFFER_SIZE];
//
//																			bson_oid_to_string (row_p -> ro_id_p, id_s);
//
//																			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, observation_json_p, "AddObservationToRow failed for row \"%s\" and key \"%s\"", id_s, key_s);
//																			FreeObservation (observation_p);
//																		}
//
//																}		/* if (observation_p) */
//															else
//																{
//																	char id_s [MONGO_OID_STRING_BUFFER_SIZE];
//
//																	bson_oid_to_string (row_p -> ro_id_p, id_s);
//
//																	PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, observation_json_p, "Failed to allocate Observation for row \"%s\" and key \"%s\"", id_s, key_s);
//
//																	FreeBSONOid (observation_id_p);
//																}
//
//														}		/* if (observation_id_p) */
//													else
//														{
//															PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, observation_json_p, "Failed to allocate observation id");
//														}
//
//													if (observation_date_p)
//														{
//															FreeTime (observation_date_p);
//														}
//												}		/* if ((!IsStringEmpty (raw_value_s)) || (!IsStringEmpty (corrected_value_s))) */
//											else
//												{
//													PrintJSONToLog (STM_LEVEL_INFO, __FILE__, __LINE__, observation_json_p, "No measured value for \"%s\", skipping", key_s);
//												}
//
//											if (!added_phenotype_flag)
//												{
//													FreeMeasuredVariable (measured_variable_p);
//												}
//
//										}		/* if (phenotype_p) */
//									else
//										{
//											PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to get phenotype with variable name \"%s\"", key_s);
//										}
//
//								}		/* if (! (DoesStringEndWith (mapped_key_s, "date"))) */
//
//						}		/* if ((strcmp (key_s, S_PLOT_INDEX_S) != 0) && (strcmp (key_s, S_RACK_S) != 0)) */
//
//					iterator_p = json_object_iter_next (observation_json_p, iterator_p);
//				}		/* while (iterator_p && loop_success_flag) */
//
//
//			FreeLinkedList (processed_keys_p);
//		}		/* if (processed_keys_p) */
//
//
//	if (imported_obs == total_obs)
//		{
//			status = OS_SUCCEEDED;
//		}
//	else if (imported_obs > 0)
//		{
//			status = OS_PARTIALLY_SUCCEEDED;
//		}
//
//	return status;
//}


OperationStatus AddStatsValuesToRow (Row *row_p, json_t *stas_json_p, Study *study_p, ServiceJob *job_p, const uint32 row_index, FieldTrialServiceData *data_p)
{
	OperationStatus status = OS_FAILED;

	return status;
}



OperationStatus AddObservationValuesToRow (Row *row_p, json_t *observations_json_p, Study *study_p, ServiceJob *job_p, const uint32 row_index, FieldTrialServiceData *data_p)
{
	OperationStatus status = OS_FAILED;

	bool loop_success_flag = true;
	void *iterator_p = json_object_iter (observations_json_p);
	size_t imported_obs = 0;
	size_t total_obs = 0;
	LinkedList *processed_keys_p = AllocateStringLinkedList ();

	if (processed_keys_p)
		{
			while (iterator_p && loop_success_flag)
				{
					const char *key_s = json_object_iter_key (iterator_p);
					json_t *value_p = json_object_iter_value (iterator_p);
					bool observation_key_flag = false;


					/*
					 * ignore our column names
					 */
					if ((strcmp (key_s, S_PLOT_INDEX_S) != 0) && (strcmp (key_s, S_RACK_S) != 0))
						{
							MeasuredVariable *measured_variable_p = NULL;
							MEM_FLAG measured_variable_mem = MF_ALREADY_FREED;
							struct tm *start_date_p = NULL;
							struct tm *end_date_p = NULL;
							bool corrected_value_flag = false;
							uint32 observation_index = OB_DEFAULT_INDEX;

							if (GetObservationMetadata (key_s, &measured_variable_p, &start_date_p, &end_date_p, &corrected_value_flag, &observation_index, job_p, row_index, &measured_variable_mem, data_p))
								{
									Observation *observation_p = NULL;
									bool added_phenotype_flag = false;
									bool free_measured_variable_flag = false;
									const char *value_s = json_string_value (value_p);

									if (!IsStringEmpty (value_s))
										{
											const char *growth_stage_s = NULL;
											const char *method_s = NULL;
											ObservationNature nature = ON_ROW;
											Instrument *instrument_p = NULL;
											json_t *raw_value_p = NULL;
											json_t *corrected_value_p = NULL;

											if (corrected_value_flag)
												{
													corrected_value_p = value_p;
												}
											else
												{
													raw_value_p = value_p;
												}

											observation_p = GetMatchingObservation (row_p, measured_variable_p, start_date_p, end_date_p, &observation_index);

											if (observation_p)
												{
													if (corrected_value_flag)
														{
															if (!SetObservationCorrectedValueFromString (observation_p, value_s))
																{
																	char id_s [MONGO_OID_STRING_BUFFER_SIZE];

																	bson_oid_to_string (row_p -> ro_id_p, id_s);

																	PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, observations_json_p, "SetObservationCorrectedValue failed for row \"%s\" and key \"%s\" with value \"%s\"", id_s, key_s, value_s);
																	FreeObservation (observation_p);
																}
														}
													else
														{
															if (!SetObservationRawValueFromString (observation_p, value_s))
																{
																	char id_s [MONGO_OID_STRING_BUFFER_SIZE];

																	bson_oid_to_string (row_p -> ro_id_p, id_s);

																	PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, observations_json_p, "SetObservationRawValue failed for row \"%s\" and key \"%s\" with value \"%s\"", id_s, key_s, value_s);
																	FreeObservation (observation_p);
																}

														}

													free_measured_variable_flag = true;

													++ imported_obs;

												}		/* if (observation_p) */
											else
												{
													bson_oid_t *observation_id_p = GetNewBSONOid ();

													if (observation_id_p)
														{
															const ScaleClass *class_p = GetMeasuredVariableScaleClass (measured_variable_p);
															ObservationType obs_type = GetObservationTypeForScaleClass (class_p);

															if (obs_type != OT_NUM_TYPES)
																{
																	observation_p = AllocateObservation (observation_id_p, start_date_p, end_date_p, measured_variable_p, MF_SHALLOW_COPY, raw_value_p, corrected_value_p, growth_stage_s, method_s, instrument_p, nature, &observation_index, obs_type);
																}

															if (observation_p)
																{
																	if (AddObservationToRow (row_p, observation_p))
																		{
																			added_phenotype_flag = true;
																			loop_success_flag = true;
																		}
																	else
																		{
																			char id_s [MONGO_OID_STRING_BUFFER_SIZE];

																			bson_oid_to_string (row_p -> ro_id_p, id_s);

																			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, observations_json_p, "AddObservationToRow failed for row \"%s\" and key \"%s\"", id_s, key_s);
																			FreeObservation (observation_p);
																		}

																}		/* if (observation_p) */
															else
																{
																	char id_s [MONGO_OID_STRING_BUFFER_SIZE];

																	bson_oid_to_string (row_p -> ro_id_p, id_s);

																	PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, observations_json_p, "Failed to allocate Observation for row \"%s\" and key \"%s\"", id_s, key_s);


																	free_measured_variable_flag = true;
																}

														}
													else
														{
															PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, observations_json_p, "Failed to allocate observation id");
														}
												}

											++ total_obs;



										}		/* if ((!IsStringEmpty (raw_value_s)) */
									else
										{
											PrintJSONToLog (STM_LEVEL_INFO, __FILE__, __LINE__, observations_json_p, "No measured value for \"%s\", skipping", key_s);
										}


									if (start_date_p)
										{
											FreeTime (start_date_p);
											start_date_p = NULL;
										}

									if (end_date_p)
										{
											FreeTime (end_date_p);
											end_date_p = NULL;
										}

									/*
									 * If the Observation failed to be allocated then, we need to free the
									 * Measured Variable as FreeObservation () would normally take care of
									 * that when the
									 */
									if (free_measured_variable_flag && measured_variable_p)
										{
											FreeMeasuredVariable (measured_variable_p);
											measured_variable_p = NULL;
										}

									if (!AddStringToStringLinkedList (processed_keys_p, key_s, MF_DEEP_COPY))
										{
											PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "AddStringToStringLinkedList () failed for \"%s\"", key_s);
										}


								}		/* if (GetObservationMetadata (key_s, &measured_variable_p, &start_date_p, &end_date_p, data_p)) */


						}		/* if ((strcmp (key_s, S_PLOT_INDEX_S) != 0) && (strcmp (key_s, S_RACK_S) != 0)) */


					iterator_p = json_object_iter_next (observations_json_p, iterator_p);
				}		/* while (iterator_p && loop_success_flag) */


			/*
			 * Remove all of the processed observation keys from the JSON
			 */
			if (processed_keys_p -> ll_size > 0)
				{
					StringListNode *node_p = (StringListNode *) (processed_keys_p -> ll_head_p);

					while (node_p)
						{
							PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Removing observation \"%s\" from the json", node_p -> sln_string_s);

							json_object_del (observations_json_p, node_p -> sln_string_s);

							node_p = (StringListNode *) (node_p -> sln_node.ln_next_p);
						}
				}

			FreeLinkedList (processed_keys_p);
		}		/* if (processed_keys_p) */


	if (imported_obs == total_obs)
		{
			status = OS_SUCCEEDED;
		}
	else if (imported_obs > 0)
		{
			status = OS_PARTIALLY_SUCCEEDED;
		}

	return status;
}




OperationStatus AddObservationValueToRow (Row *row_p, const char *key_s, const json_t *value_p, Study *study_p, ServiceJob *job_p, const uint32 row_index, FieldTrialServiceData *data_p)
{
	OperationStatus status = OS_IDLE;

	/*
	 * ignore our column names
	 */
	if ((strcmp (key_s, S_PLOT_INDEX_S) != 0) && (strcmp (key_s, S_RACK_S) != 0))
		{
			MeasuredVariable *measured_variable_p = NULL;
			MEM_FLAG mv_mem = MF_ALREADY_FREED;
			struct tm *start_date_p = NULL;
			struct tm *end_date_p = NULL;
			bool corrected_value_flag = false;
			uint32 observation_index = OB_DEFAULT_INDEX;

			status = GetObservationMetadata (key_s, &measured_variable_p, &start_date_p, &end_date_p, &corrected_value_flag, &observation_index, job_p, row_index, &mv_mem, data_p);

			if (status == OS_SUCCEEDED)
				{
					Observation *observation_p = NULL;
					bool added_phenotype_flag = false;
					bool free_measured_variable_flag = false;
					const char *growth_stage_s = NULL;
					const char *method_s = NULL;
					ObservationNature nature = ON_ROW;
					Instrument *instrument_p = NULL;
					const json_t *raw_value_p = NULL;
					const json_t *corrected_value_p = NULL;


					/* reset status */
					status = OS_FAILED;

					if (corrected_value_flag)
						{
							corrected_value_p = value_p;
						}
					else
						{
							raw_value_p = value_p;
						}

					observation_p = GetMatchingObservation (row_p, measured_variable_p, start_date_p, end_date_p, &observation_index);

					if (observation_p)
						{
							if (corrected_value_flag)
								{
									if (SetObservationCorrectedValueFromJSON (observation_p, value_p))
										{
											status = OS_SUCCEEDED;
										}
									else
										{
											char id_s [MONGO_OID_STRING_BUFFER_SIZE];

											bson_oid_to_string (row_p -> ro_id_p, id_s);

											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, value_p, "SetObservationCorrectedValue failed for row \"%s\" and key \"%s\"", id_s, key_s);
											FreeObservation (observation_p);
										}
								}
							else
								{
									if (SetObservationRawValueFromJSON (observation_p, value_p))
										{
											status = OS_SUCCEEDED;
										}
									else
										{
											char id_s [MONGO_OID_STRING_BUFFER_SIZE];

											bson_oid_to_string (row_p -> ro_id_p, id_s);

											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, value_p, "SetObservationRawValue failed for row \"%s\" and key \"%s\"", id_s, key_s);
											FreeObservation (observation_p);
										}

								}

							free_measured_variable_flag = true;
						}		/* if (observation_p) */
					else
						{

							/*
							 * Are the json values non-trivial?
							 */
							if (! ((IsJSONEmpty (raw_value_p)) && (IsJSONEmpty (corrected_value_p))))
								{
									bson_oid_t *observation_id_p = GetNewBSONOid ();

									if (observation_id_p)
										{
											const ScaleClass *class_p = GetMeasuredVariableScaleClass (measured_variable_p);
											ObservationType obs_type = GetObservationTypeForScaleClass (class_p);

											if (obs_type != OT_NUM_TYPES)
												{
													observation_p = AllocateObservation (observation_id_p, start_date_p, end_date_p, measured_variable_p, mv_mem, raw_value_p, corrected_value_p, growth_stage_s, method_s, instrument_p, nature, &observation_index, obs_type);
												}
											else
												{
													PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "GetObservationTypeForScaleClass () failed for \"%s\" and variable \"%s\"", class_p -> sc_name_s, GetMeasuredVariableName (measured_variable_p));
												}


											if (observation_p)
												{
													if (AddObservationToRow (row_p, observation_p))
														{
															added_phenotype_flag = true;
															status = OS_SUCCEEDED;
														}
													else
														{
															char id_s [MONGO_OID_STRING_BUFFER_SIZE];

															bson_oid_to_string (row_p -> ro_id_p, id_s);

															PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "AddObservationToRow failed for row \"%s\" and key \"%s\"", id_s, key_s);
															FreeObservation (observation_p);
														}

												}		/* if (observation_p) */
											else
												{
													char id_s [MONGO_OID_STRING_BUFFER_SIZE];

													bson_oid_to_string (row_p -> ro_id_p, id_s);

													PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate Observation failed for row \"%s\" and key \"%s\"", id_s, key_s);

													free_measured_variable_flag = true;
												}

										}		/* if (observation_id_p) */
									else
										{
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate observation id for row number " UINT32_FMT " and key \"%s\"", row_index, key_s);
										}
								}		/* if (! ((IsJSONEmpty (raw_value_p)) && (IsJSONEmpty (corrected_value_p)))) */
							else
								{
									/*
									 * A new Observation but with no value e.g. empty row-column entry in the
									 * spreadsheet.
									 */
									status = OS_SUCCEEDED;
								}
						}



					if (start_date_p)
						{
							FreeTime (start_date_p);
							start_date_p = NULL;
						}

					if (end_date_p)
						{
							FreeTime (end_date_p);
							end_date_p = NULL;
						}

					/*
					 * If the Observation failed to be allocated then, we need to free the
					 * Measured Variable as FreeObservation () would normally take care of
					 * that when the
					 */
					if (free_measured_variable_flag && measured_variable_p)
						{
							if ((mv_mem == MF_SHALLOW_COPY) || (mv_mem == MF_DEEP_COPY))
								{
									FreeMeasuredVariable (measured_variable_p);
									measured_variable_p = NULL;
								}
						}

				}		/* if (GetObservationMetadata (key_s, &measured_variable_p, &start_date_p, &end_date_p, data_p)) */


		}		/* if ((strcmp (key_s, S_PLOT_INDEX_S) != 0) && (strcmp (key_s, S_RACK_S) != 0)) */

	return status;
}



OperationStatus AddSingleTreatmentFactorValueToRow  (Row *row_p, const char *key_s, const char *name_s, Study *study_p, ServiceJob *job_p, const uint32 row_index, FieldTrialServiceData *data_p)
{
	OperationStatus status = OS_IDLE;
	void *temp_p = NULL;
	json_t *value_p;
	Treatment *treatment_p = NULL;
	bool cached_treatment_flag = false;

	if (data_p -> dftsd_treatments_cache_p)
		{
			treatment_p = GetCachedTreatmentByURL (data_p, name_s);
		}

	if (!treatment_p)
		{
			treatment_p = GetTreatmentByURL (key_s, VF_STORAGE, data_p);

			if (treatment_p)
				{
					if (data_p -> dftsd_treatments_cache_p)
						{
							if (AddTreatmentToCache (data_p, treatment_p, MF_SHALLOW_COPY))
								{
									cached_treatment_flag = true;
								}
							else
								{

								}
						}
				}
		}

	if (treatment_p)
		{
			/*
			 * Does the Study have a TreatmentFactor for this
			 * Treatment?
			 */
			TreatmentFactor *tf_p = GetTreatmentFactorForStudy (study_p, treatment_p -> tr_id_p, data_p);

			if (tf_p)
				{
					const char *value_s = GetTreatmentFactorValue (tf_p, name_s);

					/* Is it a valid defined label? */
					if (IsStringEmpty (value_s))
						{
							/* nothing to do */
							status = OS_SUCCEEDED;
						}
					else
						{
							if (AddTreatmentFactorValueToRowByParts (row_p, tf_p, name_s))
								{
									status = OS_SUCCEEDED;
								}
						}

					//FreeTreatmentFactor (tf_p);
				}

			if (!cached_treatment_flag)
				{
					FreeTreatment (treatment_p);
				}

		}		/* if (treatment_p) */


	return status;
}



Observation *GetMatchingObservation (const Row *row_p, const MeasuredVariable *variable_p, const struct tm *start_date_p, const struct tm *end_date_p, const uint32 *index_p)
{
	ObservationNode *node_p = (ObservationNode *) (row_p -> ro_observations_p -> ll_head_p);

	while (node_p)
		{
			Observation *existing_observation_p = node_p -> on_observation_p;

			if (AreObservationsMatchingByParts (existing_observation_p, variable_p, start_date_p, end_date_p, index_p))
				{
					return existing_observation_p;
				}
			else
				{
					node_p = (ObservationNode *) (node_p -> on_node.ln_next_p);
				}
		}

	return NULL;
}


Row *GetRowByStudyIndex (const int32 by_study_index, Study *study_p, FieldTrialServiceData *data_p)
{
	Row *row_p = NULL;
	char *index_key_s = ConcatenateVarargsStrings (PL_ROWS_S, ".", RO_STUDY_INDEX_S, NULL);

	if (index_key_s)
		{
			char *study_key_s = ConcatenateVarargsStrings (PL_ROWS_S, ".", RO_STUDY_ID_S, NULL);

			if (study_key_s)
				{
					bson_t *query_p = BCON_NEW (index_key_s, BCON_INT32 (by_study_index), study_key_s, BCON_OID (study_p -> st_id_p));

					if (query_p)
						{
							if (SetMongoToolCollection (data_p -> dftsd_mongo_p, data_p -> dftsd_collection_ss [DFTD_PLOT]))
								{
									json_t *results_p = GetAllMongoResultsAsJSON (data_p -> dftsd_mongo_p, query_p, NULL);

									if (results_p)
										{
											if (json_is_array (results_p))
												{
													const size_t num_results = json_array_size (results_p);

													if (num_results == 1)
														{
															size_t i = 0;
															json_t *plot_json_p = json_array_get (results_p, i);
															Plot *plot_p = GetPlotFromJSON (plot_json_p, study_p, data_p);

															if (plot_p)
																{
																	row_p = GetRowFromPlotByStudyIndex (plot_p, by_study_index);

																	if (!row_p)
																		{
																			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, plot_json_p, "Failed to get row with study index " UINT32_FMT,  by_study_index);
																			FreePlot (plot_p);
																		}
																}
															else
																{
																	PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, plot_json_p, "Failed to create plot");
																}

														}		/* if (num_results == 1) */
													else
														{
															char *query_json_s = bson_as_relaxed_extended_json (query_p, NULL);

															if (query_json_s)
																{
																	PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, results_p, UINT32_FMT " results for \"%s\"", query_json_s);
																	free (query_json_s);
																}
															else
																{
																	PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, results_p, UINT32_FMT " for row " UINT32_FMT " in study \"%s\"", by_study_index, study_p -> st_name_s);
																}
														}

												}		/* if (json_is_array (results_p)) */

											json_decref (results_p);
										}		/* if (results_p) */
									else
										{
											PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "No results for row " UINT32_FMT " in study \"%s\"", by_study_index, study_p -> st_name_s);
										}

								}		/* if (SetMongoToolCollection (data_p -> dftsd_mongo_p, data_p -> dftsd_collection_ss [DFTD_ROW]) */

							bson_destroy (query_p);
						}		/* if (query_p) */
					else
						{
							char *study_id_s = GetBSONOidAsString (study_p -> st_id_p);

							if (study_id_s)
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to create query for row " UINT32_FMT " in study \"%s\"", by_study_index, study_id_s);
									FreeCopiedString (study_id_s);
								}
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to create query for row " UINT32_FMT " in study \"%s\"", study_p -> st_name_s);
								}
						}

					FreeCopiedString (study_key_s);
				}		/* if (study_key_s) */
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to concatenate \"%s\", \".\" and \"%s\"", PL_ROWS_S, RO_STUDY_ID_S);
				}

			FreeCopiedString (index_key_s);
		}		/* if (index_key_s) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to concatenate \"%s\", \".\" and \"%s\"", PL_ROWS_S, RO_STUDY_INDEX_S);
		}

	return row_p;
}


bool AddTreatmentFactorValueToRowByParts (Row *row_p, TreatmentFactor *tf_p, const char *value_s)
{
	bool success_flag = false;
	TreatmentFactorValue *tf_value_p = AllocateTreatmentFactorValue (tf_p, value_s);

	if (tf_value_p)
		{
			success_flag = AddTreatmentFactorValueToRow (row_p, tf_value_p);
		}
	else
		{
			const char * const tf_name_s = GetTreatmentFactorName (tf_p);

			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "AllocateTreatmentFactorValue () failed for \"%s\", and \"%s\"", tf_name_s, value_s);
		}

	return success_flag;
}



bool GetDiscardValueFromSubmissionJSON (const json_t *row_json_p)
{
	const char *value_s = GetJSONString (row_json_p, RO_DISCARD_S);

	if (value_s)
		{
			if (Stricmp (value_s, "1") == 0)
				{
					return true;
				}
		}		/* if (value_s) */

	value_s = GetJSONString (row_json_p, PL_ACCESSION_TABLE_TITLE_S);
	if (value_s)
		{
			if (Stricmp (value_s, "discard") == 0)
				{
					return true;
				}
		}		/* if (value_s) */

	return false;
}



bool GetBlankValueFromSubmissionJSON (const json_t *row_json_p)
{
	const char *value_s = GetJSONString (row_json_p, RO_BLANK_S);

	if (value_s)
		{
			if (Stricmp (value_s, "1") == 0)
				{
					return true;
				}
		}		/* if (value_s) */

	value_s = GetJSONString (row_json_p, PL_ACCESSION_TABLE_TITLE_S);
	if (value_s)
		{
			if (Stricmp (value_s, "blank") == 0)
				{
					return true;
				}
		}		/* if (value_s) */

	return false;
}




static OperationStatus GetObservationMetadata (const char *key_s, MeasuredVariable **measured_variable_pp, struct tm **start_date_pp, struct tm **end_date_pp, bool *corrected_value_flag_p, uint32 *ob_index_p, ServiceJob *job_p, const uint32 row_index, MEM_FLAG *mf_p, FieldTrialServiceData *data_p)
{
	OperationStatus status = OS_IDLE;
	LinkedList *tokens_p = ParseStringToStringLinkedList (key_s, " ", true);


	if (tokens_p)
		{
			StringListNode *node_p = (StringListNode *) (tokens_p -> ll_head_p);
			MeasuredVariable *measured_variable_p = NULL;
			measured_variable_p = GetMeasuredVariableByVariableName (node_p -> sln_string_s, mf_p, data_p);

			if (measured_variable_p)
				{
					struct tm *start_date_p = NULL;
					struct tm *end_date_p = NULL;
					const char * const CORRECTED_KEY_S = "corrected";
					const char * const INDEX_PREFIX_KEY_S = "sample_";
					const size_t INDEX_PREFIX_KEY_LENGTH = strlen (INDEX_PREFIX_KEY_S);
					uint32 index = OB_DEFAULT_INDEX;
					bool start_date_flag = true;
					bool loop_flag = true;

					*corrected_value_flag_p = false;
					node_p = (StringListNode *) (node_p -> sln_node.ln_next_p);

					status = OS_SUCCEEDED;

					while (node_p && loop_flag && (status == OS_SUCCEEDED))
						{
							const char *value_s = node_p -> sln_string_s;

							if (Stricmp (value_s, CORRECTED_KEY_S) == 0)
								{
									*corrected_value_flag_p = true;
								}
							else if (Strnicmp (value_s, INDEX_PREFIX_KEY_S, INDEX_PREFIX_KEY_LENGTH) == 0)
								{
									const char *temp_s = value_s + INDEX_PREFIX_KEY_LENGTH;
									int32 answer;

									if (GetValidInteger (&temp_s, &answer))
										{
											if (answer >= 1)
												{
													index = (uint32) answer;
												}
											else
												{
													ReportObservationMetadataError (job_p, "Invalid sample index from", key_s, value_s);
													status = OS_FAILED;
												}
										}
									else
										{
											ReportObservationMetadataError (job_p, "Failed to create sample index from", key_s, value_s);
											status = OS_FAILED;
										}
								}
							else
								{
									if (start_date_flag)
										{
											start_date_p = GetTimeFromString (value_s);

											if (start_date_p)
												{
													start_date_flag = false;
												}
											else
												{
													ReportObservationMetadataError (job_p, "Failed to create start date from", key_s, value_s);
													status = OS_FAILED;
												}

										}
									else
										{
											end_date_p = GetTimeFromString (value_s);

											if (!end_date_p)
												{
													ReportObservationMetadataError (job_p, "Failed to create end date from", key_s, value_s);
													status = OS_FAILED;
												}
										}
								}

							if (start_date_p && end_date_p && (*corrected_value_flag_p))
								{
									loop_flag = false;
								}
							else
								{
									node_p = (StringListNode *) (node_p -> sln_node.ln_next_p);
								}
						}		/* while (node_p) */


					if (status == OS_SUCCEEDED)
						{
							*measured_variable_pp = measured_variable_p;
							*start_date_pp = start_date_p;
							*end_date_pp = end_date_p;
							*ob_index_p = index;
						}
					else
						{
							if (start_date_p)
								{
									FreeTime (start_date_p);
								}

							if (end_date_p)
								{
									FreeTime (end_date_p);
								}

							FreeMeasuredVariable (measured_variable_p);
						}

				}		/* if (measured_variable_p) */
			else
				{
					PrintErrors (STM_LEVEL_FINE, __FILE__, __LINE__, "Failed to get Measured Variable for \"%s\"", node_p -> sln_string_s);
				}


			FreeLinkedList (tokens_p);
		}		/* if (tokens_p) */


	return status;
}



static void ReportObservationMetadataError (ServiceJob *job_p, const char *prefix_s, const char *key_s, const char *value_s)
{
	char *error_s = ConcatenateVarargsStrings (prefix_s, " \"", value_s, "\" in column \"", key_s, "\"", NULL);

	if (error_s)
		{
			AddParameterErrorMessageToServiceJob (job_p, PL_PLOT_TABLE.npt_name_s, PL_PLOT_TABLE.npt_type, error_s);
			FreeCopiedString (error_s);
		}
	else
		{
			AddParameterErrorMessageToServiceJob (job_p, PL_PLOT_TABLE.npt_name_s, PL_PLOT_TABLE.npt_type, prefix_s);
		}

	PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "%s \"%s\" in column \"%s\"", prefix_s, value_s, key_s);
}

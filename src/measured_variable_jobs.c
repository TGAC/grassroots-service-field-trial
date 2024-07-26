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
 * phenotype_jobs.c
 *
 *  Created on: 26 Oct 2018
 *      Author: billy
 */

#define ALLOCATE_MEASURED_VARIABLE_CONSTANTS (1)
#include "measured_variable_jobs.h"
#include "string_utils.h"
#include "math_utils.h"
#include "crop_ontology_tool.h"
#include "dfw_util.h"
#include "io_utils.h"


#include "char_parameter.h"
#include "json_parameter.h"

/*
 * static declarations
 */


static const char * const S_VARIABLE_ID_S = "Variable Identifier";
static const char * const S_VARIABLE_NAME_S = "Variable Name";
static const char * const S_VARIABLE_DESCRIPTION_S = "Variable Description";
static const char * const S_VARIABLE_ABBREVIATION_S = "Variable Abbreviation";
static const char * const S_TRAIT_ID_S = "Trait Identifier";
static const char * const S_TRAIT_ABBREVIATION_S = "Trait Abbreviation";
static const char * const S_TRAIT_NAME_S = "Trait Name";
static const char * const S_TRAIT_DESCRIPTION_S = "Trait Description";
static const char * const S_METHOD_ID_S = "Method Identifier";
static const char * const S_METHOD_ABBREVIATION_S = "Method Abbreviation";
static const char * const S_METHOD_NAME_S = "Method Name";
static const char * const S_METHOD_DESCRIPTION_S = "Method Description";
static const char * const S_UNIT_ID_S = "Unit Identifier";
static const char * const S_UNIT_ABBREVIATION_S = "Unit Abbreviation";
static const char * const S_UNIT_NAME_S = "Unit Name";
static const char * const S_UNIT_DESCRIPTION_S = "Unit Description";
static const char * const S_FORM_ID_S = "Form Identifier";
static const char * const S_FORM_ABBREVIATION_S = "Form Abbreviation";
static const char * const S_FORM_NAME_S = "Form Name";
static const char * const S_FORM_DESCRIPTION_S = "Form Description";
static const char * const S_SCALE_CLASS_NAME_S = "Scale Class";


static NamedParameterType S_PHENOTYPE_TABLE_COLUMN_DELIMITER = { "PH Data delimiter", PT_CHAR };
static NamedParameterType S_PHENOTYPE_TABLE = { "PH Upload", PT_JSON_TABLE};


static Parameter *GetMeasuredVariablesDataTableParameter (ParameterSet *param_set_p, ParameterGroup *group_p, const FieldTrialServiceData *data_p);


static bool AddMeasuredVariablesFromJSON (ServiceJob *job_p, const json_t *phenotypes_json_p, const FieldTrialServiceData *data_p);


static SchemaTerm *GetSchemaTerm (const json_t *json_p, const char *id_key_s, const char *name_key_s, const char *description_key_s, const char *abbreviation_key_s, TermType expected_type, MongoTool *mongo_p);

static json_t *GetTableParameterHints (void);

static char *GetRowAsString (const int32 row);



bool AddSubmissionMeasuredVariableParams (ServiceData *data_p, ParameterSet *param_set_p)
{
	bool success_flag = false;

	ParameterGroup *group_p = CreateAndAddParameterGroupToParameterSet ("MeasuredVariables", false, data_p, param_set_p);

	if (group_p)
		{
			Parameter *param_p = NULL;
			const char c = DFT_DEFAULT_COLUMN_DELIMITER;

			if ((param_p = EasyCreateAndAddCharParameterToParameterSet (data_p, param_set_p, group_p, S_PHENOTYPE_TABLE_COLUMN_DELIMITER.npt_name_s, "Delimiter", "The character delimiting columns", &c, PL_ADVANCED)) != NULL)
				{
					const FieldTrialServiceData *dfw_service_data_p = (FieldTrialServiceData *) data_p;

					if ((param_p = GetMeasuredVariablesDataTableParameter (param_set_p, group_p, dfw_service_data_p)) != NULL)
						{
							success_flag = true;
						}
				}

		}		/* if (group_p) */


	return success_flag;
}


bool RunForSubmissionMeasuredVariableParams (FieldTrialServiceData *data_p, ParameterSet *param_set_p, ServiceJob *job_p)
{
	bool job_done_flag = false;
	const json_t *phenotypes_json_p = NULL;


	if (GetCurrentJSONParameterValueFromParameterSet (param_set_p, S_PHENOTYPE_TABLE.npt_name_s, &phenotypes_json_p))
		{
			/*
			 * Has a spreadsheet been uploaded?
			 */
			if (phenotypes_json_p && (json_array_size (phenotypes_json_p) > 0))
				{
					if (!AddMeasuredVariablesFromJSON (job_p, phenotypes_json_p, data_p))
						{
							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, phenotypes_json_p, "AddMeasuredVariablesFromJSON for failed");
						}
				}		/* if (phenotpnes_json_p) */

			job_done_flag = true;

		}		/* if (GetParameterValueFromParameterSet (param_set_p, S_PHENOTYPE_TABLE.npt_name_s, &value, true)) */


	return job_done_flag;
}


bool GetSubmissionMeasuredVariableParameterTypeForNamedParameter (const char *param_name_s, ParameterType *pt_p)
{
	bool success_flag = true;

	if (strcmp (param_name_s, S_PHENOTYPE_TABLE_COLUMN_DELIMITER.npt_name_s) == 0)
		{
			*pt_p = S_PHENOTYPE_TABLE_COLUMN_DELIMITER.npt_type;
		}
	else if (strcmp (param_name_s, S_PHENOTYPE_TABLE.npt_name_s) == 0)
		{
			*pt_p = S_PHENOTYPE_TABLE.npt_type;
		}
	else
		{
			success_flag = false;
		}

	return success_flag;
}




bool AddSearchTraitParams (ServiceData *data_p, ParameterSet *param_set_p)
{
	bool success_flag = false;
	ParameterGroup *group_p = CreateAndAddParameterGroupToParameterSet ("Traits", false, data_p, param_set_p);

	if (group_p)
		{
			const char c = DFT_DEFAULT_COLUMN_DELIMITER;
			Parameter *param_p = NULL;

			if ((param_p = EasyCreateAndAddCharParameterToParameterSet (data_p, param_set_p, group_p, S_PHENOTYPE_TABLE_COLUMN_DELIMITER.npt_name_s, "Delimiter", "The character delimiting columns", &c, PL_ADVANCED)) != NULL)
				{
					const FieldTrialServiceData *dfw_service_data_p = (FieldTrialServiceData *) data_p;

					if ((param_p = GetMeasuredVariablesDataTableParameter (param_set_p, group_p, dfw_service_data_p)) != NULL)
						{
							success_flag = true;
						}
				}

		}		/* if (group_p) */


	return success_flag;
}


json_t *GetAllTraitsAsJSON (const FieldTrialServiceData *data_p)
{
	json_t *traits_p = NULL;

	if (SetMongoToolCollection (data_p -> dftsd_mongo_p, data_p -> dftsd_collection_ss [DFTD_MEASURED_VARIABLE]))
		{
			bson_t *query_p = NULL;
			bson_t *opts_p =  BCON_NEW ( "sort", "{", MONGO_ID_S, BCON_INT32 (1), "}");
			json_t *results_p = GetAllMongoResultsAsJSON (data_p -> dftsd_mongo_p, query_p, opts_p);

			if (results_p)
				{
					traits_p = json_array ();

					if (traits_p)
						{
							const size_t num_results = json_array_size (results_p);
							size_t i;

							for (i = 0; i < num_results; ++ i)
								{
									json_t *result_p = json_array_get (results_p, i);
									json_t *trait_p = json_object_get (result_p, MV_TRAIT_S);

									if (trait_p)
										{
											if (json_array_append (traits_p, trait_p) != 0)
												{
													PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, trait_p, "Failed to append trait to array");
												}		/* if (json_array_append (traits_p, trait_p) != 0) */

										}		/* if (trait_p) */
									else
										{
											PrintJSONToErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, result_p, "No \"%s\" key found");
										}

								}		/* for (i = 0; i < num_results; ++ i) */

						}		/* if (traits_p) */
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to create traits array");
						}

					json_decref (results_p);
				}

			if (opts_p)
				{
					bson_destroy (opts_p);
				}
		}

	return traits_p;
}


MeasuredVariable *GetMeasuredVariableByVariableName (const char *name_s, MEM_FLAG *mv_mem_p, FieldTrialServiceData *data_p)
{
	MeasuredVariable *phenotype_p = NULL;

	if (HasMeasuredVariableCache (data_p))
		{
			phenotype_p = GetCachedMeasuredVariableByName (data_p, name_s);

			if (phenotype_p)
				{
					*mv_mem_p = MF_SHADOW_USE;
				}
		}

	if (!phenotype_p)
		{
			if (SetMongoToolCollection (data_p -> dftsd_mongo_p, data_p -> dftsd_collection_ss [DFTD_MEASURED_VARIABLE]))
				{
					char *key_s = GetMeasuredVariablesNameKey ();

					if (key_s)
						{
							bson_t *query_p = BCON_NEW (key_s, BCON_UTF8 (name_s));

							if (query_p)
								{
									json_t *results_p = GetAllMongoResultsAsJSON (data_p -> dftsd_mongo_p, query_p, NULL);

									if (results_p)
										{
											if (json_is_array (results_p))
												{
													const size_t num_results = json_array_size (results_p);

													if (num_results > 1)
														{
															PrintJSONToLog (STM_LEVEL_INFO, __FILE__, __LINE__, results_p, "Multiple matching measured variables " SIZET_FMT ", using the first", num_results);
														}

													if (num_results != 0)
														{
															size_t i = 0;
															json_t *entry_p = json_array_get (results_p, i);

															phenotype_p = GetMeasuredVariableFromJSON (entry_p, data_p);

															if (phenotype_p)
																{
																	MEM_FLAG mf = MF_SHALLOW_COPY;

																	if (HasMeasuredVariableCache (data_p))
																		{
																			if (AddMeasuredVariableToCache (data_p, phenotype_p, MF_SHALLOW_COPY))
																				{
																					mf = MF_SHADOW_USE;
																				}
																		}

																	*mv_mem_p = mf;
																}

															if (!phenotype_p)
																{
																	PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, results_p, "GetMeasuredVariableFromJSON failed for \"%s\": \"%s\"", key_s, name_s);
																}

														}		/* if (num_results != 0) */
													else
														{
															PrintBSONToErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, query_p, "No measured variables found for \"%s\": \"%s\"", key_s, name_s);
														}

												}		/* if (json_is_array (results_p)) */
											else
												{
													PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, results_p, "Results are not an array for \"%s\": \"%s\"", key_s, name_s);
												}

											json_decref (results_p);
										}		/* if (results_p) */
									else
										{
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "NULL results for \"%s\": \"%s\"", key_s, name_s);
										}

									bson_destroy (query_p);
								}		/* if (query_p) */
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to create query for \"%s\": \"%s\"", key_s, name_s);
								}

							FreeMeasuredVariablesNameKey (key_s);
						}
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "GetMeasuredVariablesNameKey () failed", data_p -> dftsd_collection_ss [DFTD_MEASURED_VARIABLE]);
						}

				}		/* if (SetMongoToolCollection (data_p -> dftsd_mongo_p, data_p -> dftsd_collection_ss [DFTD_RAW_PHENOTYPE])) */
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to set mongo collection to \"%s\"", data_p -> dftsd_collection_ss [DFTD_MEASURED_VARIABLE]);
				}

		}		/* if (!phenotype_p) */


	return phenotype_p;
}


char *GetMeasuredVariablesNameKey (void)
{
	char *key_s = ConcatenateVarargsStrings (MV_VARIABLE_S, ".", SCHEMA_TERM_NAME_S, NULL);

	if (key_s)
		{
			return key_s;
		}
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "ConcatenateVarargsStrings () failed for \"%s\", \".\" and \"%s\"", MV_VARIABLE_S, SCHEMA_TERM_NAME_S);
		}

	return NULL;
}


void FreeMeasuredVariablesNameKey (char *key_s)
{
	FreeCopiedString (key_s);
}


json_t *GetMeasuredVariableIndexingData (Service *service_p)
{
	FieldTrialServiceData *data_p = (FieldTrialServiceData *) (service_p -> se_data_p);
	json_t *measured_variables_p = GetAllMeasuredVariablesAsJSON (data_p, NULL);

	if (measured_variables_p)
		{
			if (json_is_array (measured_variables_p))
				{
					size_t i;
					json_t *measured_variable_p;
					size_t num_added = 0;

					json_array_foreach (measured_variables_p, i, measured_variable_p)
						{
							if (AddDatatype (measured_variable_p, DFTD_MEASURED_VARIABLE))
								{
									json_t *variable_p = json_object_get (measured_variable_p, MV_VARIABLE_S);

									if (variable_p)
										{
											const char *name_s = GetJSONString (variable_p, SCHEMA_TERM_NAME_S);

											if (name_s)
												{
													if (SetJSONString (measured_variable_p, MV_NAME_S, name_s))
														{
															++ num_added;
														}
												}
										}
								}


						}		/* json_array_foreach (src_studies_p, i, src_study_p) */

				}		/* if (json_is_array (src_studies_p)) */

			return measured_variables_p;
		}		/* if (src_studies_p) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "No measured variables for \"%s\"", GetServiceName (service_p));
		}

	return NULL;
}


json_t *GetAllMeasuredVariablesAsJSON (const FieldTrialServiceData *data_p, bson_t *opts_p)
{
	json_t *results_p = NULL;

	if (SetMongoToolCollection (data_p -> dftsd_mongo_p, data_p -> dftsd_collection_ss [DFTD_MEASURED_VARIABLE]))
		{
			bson_t *query_p = NULL;

			results_p = GetAllMongoResultsAsJSON (data_p -> dftsd_mongo_p, query_p, opts_p);
		}		/* if (SetMongoToolCollection (data_p -> dftsd_mongo_p, data_p -> dftsd_collection_ss [DFTD_PHENOTYPE])) */

	return results_p;
}


char *GetMeasuredVariableAsString (const MeasuredVariable *treatment_p)
{
	char *title_s = NULL;

	if (treatment_p -> mv_variable_term_p)
		{
			title_s = EasyCopyToNewString (treatment_p -> mv_variable_term_p -> st_name_s);
		}
	else
		{
			ByteBuffer *buffer_p = AllocateByteBuffer (1024);

			if (buffer_p)
				{
					bool success_flag = true;

					if (treatment_p -> mv_trait_term_p)
						{
							success_flag = AppendStringToByteBuffer (buffer_p, treatment_p -> mv_trait_term_p -> st_name_s);
						}

					if (success_flag)
						{
							if (treatment_p -> mv_measurement_term_p)
								{
									success_flag = AppendStringsToByteBuffer (buffer_p, " - ", treatment_p -> mv_measurement_term_p -> st_name_s, NULL);
								}
						}

					if (success_flag)
						{
							if (treatment_p -> mv_unit_term_p)
								{
									success_flag = AppendStringsToByteBuffer (buffer_p, " - ", treatment_p -> mv_unit_term_p -> st_name_s, NULL);
								}
						}


					if (success_flag)
						{
							title_s = DetachByteBufferData (buffer_p);
						}
					else
						{
							FreeByteBuffer (buffer_p);
						}

				}		/* if (buffer_p) */

		}

	return title_s;
}


bool AddMeasuredVariableToServiceJob (ServiceJob *job_p, MeasuredVariable *treatment_p, const ViewFormat format, FieldTrialServiceData *data_p)
{
	bool success_flag = false;
	json_t *treatment_json_p = GetMeasuredVariableAsJSON (treatment_p, format);

	if (treatment_json_p)
		{
			char *title_s = GetMeasuredVariableAsString (treatment_p);

			if (title_s)
				{
					if (AddContext (treatment_json_p))
						{
							json_t *dest_record_p = GetDataResourceAsJSONByParts (PROTOCOL_INLINE_S, NULL, title_s, treatment_json_p);

							if (dest_record_p)
								{
									AddImage (dest_record_p, DFTD_MEASURED_VARIABLE, data_p);

									if (AddResultToServiceJob (job_p, dest_record_p))
										{
											success_flag = true;
										}
									else
										{
											json_decref (dest_record_p);
										}

								}		/* if (dest_record_p) */

						}		/* if (AddContext (treatment_json_p)) */

					FreeCopiedString (title_s);
				}		/* if (title_s) */

		}		/* if (treatment_json_p) */

	return success_flag;
}




/*
 * static definitions
 */



static json_t *GetTableParameterHints (void)
{
	/*
	headers_s = ConcatenateVarargsStrings (S_INTERNAL_NAME_TITLE_S, delim_s, S_TRAIT_ID_S, delim_s, S_TRAIT_ABBREVIATION_S, delim_s, S_TRAIT_NAME_S, delim_s, S_TRAIT_DESCRIPTION_S, delim_s,
																				 S_METHOD_ID_S, delim_s, S_METHOD_ABBREVIATION_S, delim_s, S_METHOD_NAME_S, delim_s, S_METHOD_DESCRIPTION_S, delim_s,
																				 S_UNIT_ID_S, delim_s, S_UNIT_ABBREVIATION_S, delim_s, S_UNIT_NAME_S, delim_s, S_UNIT_DESCRIPTION_S, delim_s,
																				 S_FORM_ID_S, delim_s, S_FORM_ABBREVIATION_S, delim_s, S_FORM_NAME_S, delim_s, S_FORM_DESCRIPTION_S, delim_s,
																				 NULL);

	 */
	json_t *hints_p = json_array ();

	if (hints_p)
		{
			if (AddColumnParameterHint (S_VARIABLE_ID_S, NULL, PT_STRING, true, hints_p))
				{
					if (AddColumnParameterHint (S_VARIABLE_NAME_S, NULL, PT_STRING, true, hints_p))
						{
							if (AddColumnParameterHint (S_VARIABLE_DESCRIPTION_S, NULL, PT_STRING, true, hints_p))
								{
									if (AddColumnParameterHint (S_TRAIT_ID_S, NULL, PT_STRING, true, hints_p))
										{
											if (AddColumnParameterHint (S_TRAIT_ABBREVIATION_S, NULL, PT_STRING, true, hints_p))
												{
													if (AddColumnParameterHint (S_TRAIT_NAME_S, NULL, PT_STRING, true, hints_p))
														{
															if (AddColumnParameterHint (S_TRAIT_DESCRIPTION_S, NULL, PT_STRING, true, hints_p))
																{
																	if (AddColumnParameterHint (S_METHOD_ID_S, NULL, PT_STRING, true, hints_p))
																		{
																			if (AddColumnParameterHint (S_METHOD_ABBREVIATION_S, NULL, PT_STRING, true, hints_p))
																				{
																					if (AddColumnParameterHint (S_METHOD_NAME_S, NULL, PT_STRING, true, hints_p))
																						{
																							if (AddColumnParameterHint (S_METHOD_DESCRIPTION_S, NULL, PT_STRING, true, hints_p))
																								{
																									if (AddColumnParameterHint (S_UNIT_ID_S, NULL, PT_STRING, true, hints_p))
																										{
																											if (AddColumnParameterHint (S_UNIT_ABBREVIATION_S, NULL, PT_STRING, true, hints_p))
																												{
																													if (AddColumnParameterHint (S_UNIT_NAME_S, NULL, PT_STRING, true, hints_p))
																														{
																															if (AddColumnParameterHint (S_UNIT_DESCRIPTION_S, NULL, PT_STRING, true, hints_p))
																																{
																																	if (AddColumnParameterHint (S_FORM_ID_S, NULL, PT_STRING, true, hints_p))
																																		{
																																			if (AddColumnParameterHint (S_FORM_ABBREVIATION_S, NULL, PT_STRING, true, hints_p))
																																				{
																																					if (AddColumnParameterHint (S_FORM_NAME_S, NULL, PT_STRING, true, hints_p))
																																						{
																																							if (AddColumnParameterHint (S_FORM_DESCRIPTION_S, NULL, PT_STRING, true, hints_p))
																																								{
																																									if (AddColumnParameterHint (S_SCALE_CLASS_NAME_S, NULL, PT_STRING, true, hints_p))
																																										{
																																											return hints_p;
																																										}
																																								}
																																						}
																																				}
																																		}
																																}
																														}
																												}
																										}
																								}
																						}
																				}
																		}
																}
														}
												}

										}
								}


						}
				}
			json_decref (hints_p);
		}

	return NULL;
}



static Parameter *GetMeasuredVariablesDataTableParameter (ParameterSet *param_set_p, ParameterGroup *group_p, const FieldTrialServiceData *data_p)
{
	Parameter *param_p = EasyCreateAndAddJSONParameterToParameterSet (& (data_p -> dftsd_base_data), param_set_p, group_p, S_PHENOTYPE_TABLE.npt_type, S_PHENOTYPE_TABLE.npt_name_s, "MeasuredVariable data to upload", "The data to upload", NULL, PL_ALL);

	if (param_p)
		{
			bool success_flag = false;

			json_t *hints_p = GetTableParameterHints ();

			if (hints_p)
				{
					if (AddParameterKeyJSONValuePair (param_p, PA_TABLE_COLUMN_HEADINGS_S, hints_p))
						{
							const char delim_s [2] = { DFT_DEFAULT_COLUMN_DELIMITER, '\0' };

							if (AddParameterKeyStringValuePair (param_p, PA_TABLE_COLUMN_DELIMITER_S, delim_s))
								{
									success_flag = true;
								}
						}

					json_decref (hints_p);
				}

			if (!success_flag)
				{
					FreeParameter (param_p);
					param_p = NULL;
				}

		}		/* if (param_p) */

	return param_p;
}


/*
{
	"Accession": "EM",
	"Trait Identifier": "CO_321:0000007",
	"Trait Abbreviation": "Hd",
	"Trait Name": "Heading time",
	"Trait Description": "Heading time extends from the time of emergence of the tip of the spike from the flag leaf sheath to when the spike has completely emerged but has not yet started to flower.",
	"Method Identifier": "CO_321:0000840",
	"Method Abbreviation": "",
	"Method Name": "Hd DS55 date Estimation",
	"Method Description": "Record date of heading (DS55) when 50% of the spike is emerged (i.e., middle of the spike at the flag leaf ligule) on 50% of all stems.",
	"Unit Identifier": "CO_321:0000855",
	"Unit Abbreviation": "",
	"Unit Name": "Julian date (JD)"
}
 */
static bool AddMeasuredVariablesFromJSON (ServiceJob *job_p, const json_t *phenotypes_json_p, const FieldTrialServiceData *data_p)
{
	bool success_flag	= true;
	OperationStatus status = OS_FAILED;

	if (json_is_array (phenotypes_json_p))
		{
			const size_t num_rows = json_array_size (phenotypes_json_p);
			size_t i;
			size_t num_imported = 0;
			size_t num_empty_rows = 0;
			size_t num_existing = 0;

			for (i = 0; i < num_rows; ++ i)
				{
					json_t *table_row_json_p = json_array_get (phenotypes_json_p, i);

					const size_t row_size =  json_object_size (table_row_json_p);

					if (row_size > 0)
						{
							MeasuredVariable *mv_p = NULL;
							MongoTool *mongo_p = data_p -> dftsd_mongo_p;
							SchemaTerm *trait_p = GetSchemaTerm (table_row_json_p, S_TRAIT_ID_S, S_TRAIT_NAME_S, S_TRAIT_DESCRIPTION_S, S_TRAIT_ABBREVIATION_S, TT_TRAIT, mongo_p);

							if (trait_p)
								{
									SchemaTerm *method_p = GetSchemaTerm (table_row_json_p, S_METHOD_ID_S, S_METHOD_NAME_S, S_METHOD_DESCRIPTION_S, S_METHOD_ABBREVIATION_S, TT_METHOD, mongo_p);

									if (method_p)
										{
											SchemaTerm *unit_p = GetSchemaTerm (table_row_json_p, S_UNIT_ID_S, S_UNIT_NAME_S, S_UNIT_DESCRIPTION_S, S_UNIT_ABBREVIATION_S, TT_UNIT, mongo_p);

											if (unit_p)
												{
													SchemaTerm *variable_p = GetSchemaTerm (table_row_json_p, S_VARIABLE_ID_S, S_VARIABLE_NAME_S, S_VARIABLE_DESCRIPTION_S, S_VARIABLE_ABBREVIATION_S, TT_VARIABLE, mongo_p);

													if (variable_p)
														{
															/*
															 * Variable names must not contain any whitespace
															 */
															if (!DoesStringContainWhitespace (variable_p -> st_name_s))
																{
																	const char *scale_s = GetJSONString (table_row_json_p, S_SCALE_CLASS_NAME_S);

																	if (scale_s)
																		{
																			const ScaleClass *scale_p = GetScaleClassByName (scale_s);

																			if (scale_p)
																				{
																					mv_p = AllocateMeasuredVariable (NULL, trait_p, method_p, unit_p, variable_p, scale_p);

																					if (mv_p)
																						{
																							int res = CheckMeasuredVariable (mv_p, data_p);

																							if (res == 0)
																								{
																									OperationStatus import_status;

																									PrintJSONToLog (STM_LEVEL_FINER, __FILE__, __LINE__, table_row_json_p, "Adding MeasuredVariable for row " SIZET_FMT, i);

																									import_status = SaveMeasuredVariable (mv_p, job_p, data_p);

																									if ((import_status == OS_SUCCEEDED) || (import_status == OS_PARTIALLY_SUCCEEDED))
																										{
																											++ num_imported;
																										}
																									else
																										{
																											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, table_row_json_p, "Failed to save MeasuredVariable for row " SIZET_FMT, i);
																											AddTabularParameterErrorMessageToServiceJob (job_p, S_PHENOTYPE_TABLE.npt_name_s, S_PHENOTYPE_TABLE.npt_type, "Failed to save measured variable", i, NULL);
																											success_flag = false;
																										}

																								}		/* if (res == 0) */
																							else if (res == 1)
																								{
																									++ num_existing;
																									PrintJSONToLog (STM_LEVEL_FINER, __FILE__, __LINE__, table_row_json_p, "Ignoring existing MeasuredVariable for row " SIZET_FMT, i);
																								}
																							else if (res == -1)
																								{
																									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, table_row_json_p, "MeasuredVariable Trait, Measurement and Unit Combination already exist for different Variable " SIZET_FMT, i);
																									AddTabularParameterErrorMessageToServiceJob (job_p, S_PHENOTYPE_TABLE.npt_name_s, S_PHENOTYPE_TABLE.npt_type, "MeasuredVariable Trait, Measurement and Unit Combination already exist for different Variable", i, NULL);
																								}
																						}		/* if (mv_p) */
																					else
																						{
																							PrintJSONToErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, table_row_json_p, "AllocateMeasuredVariable failed with name \"%s\"  for row " SIZET_FMT, variable_p -> st_name_s, i);
																							AddTabularParameterErrorMessageToServiceJob (job_p, S_PHENOTYPE_TABLE.npt_name_s, S_PHENOTYPE_TABLE.npt_type, "Failed to create measured variable", i, NULL);
																						}

																				}		/*  if (scale_p) */
																			else
																				{
																					PrintErrors (STM_LEVEL_INFO, __FILE__, __LINE__, "Failed to get scale class for \"%s\" for row " SIZET_FMT, scale_s, i);
																					AddTabularParameterErrorMessageToServiceJob (job_p, S_PHENOTYPE_TABLE.npt_name_s, S_PHENOTYPE_TABLE.npt_type, "Failed to get scale class", i, NULL);
																				}

																		}		/* if (scale_s) */
																	else
																		{
																			PrintJSONToErrors (STM_LEVEL_INFO, __FILE__, __LINE__, table_row_json_p, "No scale class specified for row " SIZET_FMT, i);
																			AddTabularParameterErrorMessageToServiceJob (job_p, S_PHENOTYPE_TABLE.npt_name_s, S_PHENOTYPE_TABLE.npt_type, "No scale class specified", i, NULL);
																		}

																}
															else
																{
																	PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, table_row_json_p, "Variable name \"%s\" on line " SIZET_FMT " contains whitespace", variable_p -> st_name_s, i);
																	AddTabularParameterErrorMessageToServiceJob (job_p, S_PHENOTYPE_TABLE.npt_name_s, S_PHENOTYPE_TABLE.npt_type, "Variable name contains whitespace", i, NULL);
																}



														}		/* if (variable_p */
													else
														{
															char *row_s = GetRowAsString (i);
															PrintJSONToErrors (STM_LEVEL_INFO, __FILE__, __LINE__, table_row_json_p, "Failed to get Variable for row " SIZET_FMT, i);

															if (row_s)
																{
																	const char *prefix_s = "Failed to get Method";
																	char *error_s = ConcatenateVarargsStrings (prefix_s, " for ", row_s, NULL);

																	if (error_s)
																		{
																			AddParameterErrorMessageToServiceJob (job_p, S_PHENOTYPE_TABLE.npt_name_s, S_PHENOTYPE_TABLE.npt_type, error_s);
																			FreeCopiedString (error_s);
																		}
																	else
																		{
																			AddParameterErrorMessageToServiceJob (job_p, S_PHENOTYPE_TABLE.npt_name_s, S_PHENOTYPE_TABLE.npt_type, prefix_s);
																		}

																	FreeCopiedString (row_s);
																}
														}

													if (!mv_p)
														{
															FreeSchemaTerm (unit_p);
														}

												}		/* if (unit_p) */
											else
												{
													PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, table_row_json_p, "Failed to get Unit for row " SIZET_FMT, i);
													AddTabularParameterErrorMessageToServiceJob (job_p, S_PHENOTYPE_TABLE.npt_name_s, S_PHENOTYPE_TABLE.npt_type, "Failed to create Unit", i, NULL);
												}

											if (!mv_p)
												{
													FreeSchemaTerm (method_p);
												}

										}		/* if (method_p) */
									else
										{
											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, table_row_json_p, "Failed to get Method for row " SIZET_FMT, i);
											AddTabularParameterErrorMessageToServiceJob (job_p, S_PHENOTYPE_TABLE.npt_name_s, S_PHENOTYPE_TABLE.npt_type, "Failed to create Method", i, NULL);
										}

									if (!mv_p)
										{
											FreeSchemaTerm (trait_p);
										}

								}		/* if (trait_p) */
							else
								{
									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, table_row_json_p, "Failed to get Trait for row " SIZET_FMT, i);
									AddTabularParameterErrorMessageToServiceJob (job_p, S_PHENOTYPE_TABLE.npt_name_s, S_PHENOTYPE_TABLE.npt_type, "Failed to create Trait", i, NULL);
								}

							if (mv_p)
								{
									FreeMeasuredVariable (mv_p);
								}

						}		/* if (row_size > 0) */
					else
						{
							++ num_empty_rows;
						}
				}		/* for (i = 0; i < num_rows; ++ i) */


			if (num_imported + num_empty_rows  + num_existing == num_rows)
				{
					status = OS_SUCCEEDED;
				}
			else if (num_imported > 0)
				{
					status = OS_PARTIALLY_SUCCEEDED;
				}

		}		/* if (json_is_array (plots_json_p)) */

	SetServiceJobStatus (job_p, status);

	return success_flag;
}


static char *GetRowAsString (const int32 row)
{
	char *row_s = NULL;
	char *num_s = ConvertIntegerToString (row);

	if (num_s)
		{
			row_s = ConcatenateStrings ("row ", row_s);

			if (!row_s)
				{
					PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to concatenate row and " UINT32_FMT, row);
				}

			FreeCopiedString (num_s);
		}		/* if (num_s) */

	return row_s;
}


static SchemaTerm *GetSchemaTerm (const json_t *json_p, const char *id_key_s, const char *name_key_s, const char *description_key_s, const char *abbreviation_key_s, TermType expected_type, MongoTool *mongo_p)
{
	SchemaTerm *term_p = NULL;
	const char *id_s = GetJSONString (json_p, id_key_s);

	if (id_s)
		{
			const char *name_s =  GetJSONString (json_p, name_key_s);
			const char *description_s =  GetJSONString (json_p, description_key_s);
			const char *abbreviation_s =  GetJSONString (json_p, abbreviation_key_s);

			if (IsStringEmpty (name_s) || IsStringEmpty (description_s) || IsStringEmpty (abbreviation_s))
				{
					if (strncmp (id_s, "CO_", strlen (CONTEXT_PREFIX_CROP_ONTOLOGY_S)) == 0)
						{
							TermType found_type = TT_NUM_TYPES;
							term_p = GetCropOnotologySchemaTerm (id_s, expected_type, &found_type, mongo_p);

							if (!term_p)
								{
									PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "GetCropOnotologySchemaTerm failed for \"%s\"", id_s);
								}
						}
				}

			if (!term_p)
				{
					if (name_s)
						{
							term_p = AllocateExtendedSchemaTerm (id_s, name_s, description_s, abbreviation_s);
						}
				}

		}		/* if (id_s) */
	else
		{
			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, json_p, "Failed to get \"%s\"", id_key_s);
		}

	return term_p;
}


int CheckMeasuredVariable (MeasuredVariable *var_p, const FieldTrialServiceData *data_p)
{
	int res = 0;

	if ((var_p -> mv_trait_term_p) && (var_p -> mv_measurement_term_p) && (var_p -> mv_unit_term_p))
		{
			MeasuredVariable *saved_treatment_p = GetMeasuredVariableBySchemaURLs (var_p -> mv_trait_term_p -> st_url_s, var_p -> mv_measurement_term_p -> st_url_s, var_p -> mv_unit_term_p -> st_url_s, data_p);

			if (saved_treatment_p)
				{

					/*
					 * Does the Variable match?
					 */

					const char *var_url_s = GetMeasuredVariableURL (var_p);
					const char *saved_treatment_url_s = GetMeasuredVariableURL (saved_treatment_p);

					if (DoStringsMatch (var_url_s, saved_treatment_url_s))
						{
							if (var_p -> mv_scale_class_p == NULL)
								{
									var_p -> mv_scale_class_p = saved_treatment_p -> mv_scale_class_p;
								}

							res = 1;
						}
					else
						{
							res = -1;
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__,  "\"%s\" and \"%s\" have matching traits, measurements and units",
													 GetMeasuredVariableName (var_p), GetMeasuredVariableName (saved_treatment_p));

						}

					FreeMeasuredVariable (saved_treatment_p);
				}
		}

	return res;
}


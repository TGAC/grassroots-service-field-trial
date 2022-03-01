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
 * program_jobs.c
 *
 *  Created on: 13 Nov 2020
 *      Author: billy
 */

#define ALLOCATE_PROGRAMME_JOB_CONSTANTS (1)
#include "programme_jobs.h"
#include "crop_jobs.h"
#include "dfw_util.h"

#include "frictionless_data_util.h"

static const char * const S_EMPTY_LIST_OPTION_S = "<empty>";


static bool AddProgramme (ServiceJob *job_p, ParameterSet *param_set_p, FieldTrialServiceData *data_p);

static bool AddProgrammeToServiceJobResult (ServiceJob *job_p, Programme *program_p, json_t *program_json_p, const ViewFormat format, FieldTrialServiceData *data_p);

static bool SetUpDefaultsFromExistingProgramme (const Programme *programme_p, char **id_ss,  char **name_ss, char **abbreviation_ss, Crop **crop_pp, char **documentation_url_ss, char **objective_ss, char **pi_name_ss, char **pi_email_ss, char **logo_ss);


bool AddSubmissionProgrammeParams (ServiceData *data_p, ParameterSet *param_set_p, Resource *resource_p)
{
	FieldTrialServiceData *dfw_data_p = (FieldTrialServiceData *) data_p;
	bool success_flag = false;
	Parameter *param_p = NULL;
	char *id_s = NULL;
	char *abbreviation_s = NULL;
	char *documentation_url_s = NULL;
	char *name_s = NULL;
	char *objective_s = NULL;
	char *pi_name_s = NULL;
	char *pi_email_s = NULL;
	char *logo_s = NULL;
	Crop *crop_p = NULL;
	Programme *active_program_p = GetProgrammeFromResource (resource_p, PROGRAMME_ID, dfw_data_p);
	bool defaults_flag = false;

	if (active_program_p)
		{
			if (SetUpDefaultsFromExistingProgramme (active_program_p, &id_s,  &name_s, &abbreviation_s, &crop_p, &documentation_url_s, &objective_s, &pi_name_s, &pi_email_s, &logo_s))
				{
					defaults_flag = true;
				}
		}
	else
		{
			id_s = (char *) S_EMPTY_LIST_OPTION_S;
			defaults_flag = true;
		}


	if (defaults_flag)
		{
			if ((param_p = EasyCreateAndAddStringParameterToParameterSet (data_p, param_set_p, NULL, PROGRAMME_ID.npt_type, PROGRAMME_ID.npt_name_s, "Load Programme", "Edit an existing Programme", id_s, PL_ALL)) != NULL)
				{
					if (SetUpProgrammesListParameter (dfw_data_p, (StringParameter *) param_p, active_program_p, true))
						{
							/*
							 * We want to update all of the values in the form
							 * when a user selects a study from the list so
							 * we need to make the parameter automatically
							 * refresh the values. So we set the
							 * pa_refresh_service_flag to true.
							 */
							param_p -> pa_refresh_service_flag = true;

							if ((param_p = EasyCreateAndAddStringParameterToParameterSet (data_p, param_set_p, NULL, PROGRAMME_NAME.npt_type, PROGRAMME_NAME.npt_name_s, "Name", "The name of the Programme", name_s, PL_ALL)) != NULL)
								{
									param_p -> pa_required_flag = true;

									if ((param_p = EasyCreateAndAddStringParameterToParameterSet (data_p, param_set_p, NULL, PROGRAMME_ABBREVIATION.npt_type, PROGRAMME_ABBREVIATION.npt_name_s, "Abbreviation", "The abbreviation for the Programme", abbreviation_s, PL_ALL)) != NULL)
										{
											if ((param_p = EasyCreateAndAddStringParameterToParameterSet (data_p, param_set_p, NULL, PROGRAMME_CROP.npt_type, PROGRAMME_CROP.npt_name_s, "Crop", "The crop for the Programme", NULL, PL_ALL)) != NULL)
												{
													if (SetUpCropsListParameter (dfw_data_p, (StringParameter *) param_p, crop_p, S_EMPTY_LIST_OPTION_S, active_program_p ? false : true))
														{
															if ((param_p = EasyCreateAndAddStringParameterToParameterSet (data_p, param_set_p, NULL, PROGRAMME_OBJECTIVE.npt_type, PROGRAMME_OBJECTIVE.npt_name_s, "Objective", "The Programme's objective", objective_s, PL_ALL)) != NULL)
																{
																	if ((param_p = EasyCreateAndAddStringParameterToParameterSet (data_p, param_set_p, NULL, PROGRAMME_URL.npt_type, PROGRAMME_URL.npt_name_s, "Web address", "The web page documenting this Programme", documentation_url_s, PL_ALL)) != NULL)
																		{
																			if ((param_p = EasyCreateAndAddStringParameterToParameterSet (data_p, param_set_p, NULL, PROGRAMME_PI_NAME.npt_type, PROGRAMME_PI_NAME.npt_name_s, "Principal Investigator Name", "The name of the Programme's lead", pi_name_s, PL_ALL)) != NULL)
																				{
																					param_p -> pa_required_flag = true;

																					if ((param_p = EasyCreateAndAddStringParameterToParameterSet (data_p, param_set_p, NULL, PROGRAMME_PI_EMAIL.npt_type, PROGRAMME_PI_EMAIL.npt_name_s, "Principal Investigator Email", "The email address of the Programme's lead", pi_email_s, PL_ALL)) != NULL)
																						{
																							param_p -> pa_required_flag = true;


																							if ((param_p = EasyCreateAndAddStringParameterToParameterSet (data_p, param_set_p, NULL, PROGRAMME_LOGO.npt_type, PROGRAMME_LOGO.npt_name_s, "Logo", "The web address of the programme logo", logo_s, PL_ALL)) != NULL)
																								{
																									success_flag = true;
																								}
																							else
																								{
																									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", PROGRAMME_LOGO.npt_name_s);
																								}

																						}
																					else
																						{
																							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", PROGRAMME_PI_EMAIL.npt_name_s);
																						}
																				}
																			else
																				{
																					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", PROGRAMME_PI_NAME.npt_name_s);
																				}
																		}
																	else
																		{
																			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", PROGRAMME_URL.npt_name_s);
																		}
																}

														}

												}
										}
								}

						}		/* if (SetUpFieldTrialsListParameter ((FieldTrialServiceData *) data_p, (StringParameter *) param_p, NULL, true)) */

				}		/* if ((param_p = EasyCreateAndAddStringParameterToParameterSet (data_p, param_set_p, NULL, FIELD_TRIAL_ID.npt_type, FIELD_TRIAL_ID.npt_name_s, "Load Field Trial", "Edit an existing Field Trial", id_s, PL_ADVANCED)) != NULL) */

		}		/* if (defaults_flag) */


	if (active_program_p)
		{
			FreeProgramme (active_program_p);
		}

	return success_flag;
}



json_t *GetAllProgrammesAsJSON (const FieldTrialServiceData *data_p, bson_t *opts_p)
{
	json_t *results_p = NULL;

	if (SetMongoToolCollection (data_p -> dftsd_mongo_p, data_p -> dftsd_collection_ss [DFTD_PROGRAMME]))
		{
			bson_t *query_p = NULL;

			results_p = GetAllMongoResultsAsJSON (data_p -> dftsd_mongo_p, query_p, opts_p);
		}		/* if (SetMongoToolCollection (data_p -> dftsd_mongo_p, data_p -> dftsd_collection_ss [DFTD_PROGRAMME])) */

	return results_p;
}


bool SetUpProgrammesListParameter (const FieldTrialServiceData *data_p, StringParameter *param_p, const Programme *active_program_p, const bool empty_option_flag)
{
	bool success_flag = false;
	json_t *results_p = GetAllProgrammesAsJSON (data_p, NULL);
	bool value_set_flag = false;

	if (results_p)
		{
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
											Programme *program_p = GetProgrammeFromJSON (entry_p, VF_CLIENT_MINIMAL, data_p);

											if (program_p)
												{
													char *id_s = GetBSONOidAsString (program_p -> pr_id_p);

													if (id_s)
														{
															if (param_value_s && (strcmp (param_value_s, id_s) == 0))
																{
																	value_set_flag = true;
																}

															if (!CreateAndAddStringParameterOption (param_p, id_s, program_p -> pr_name_s))
																{
																	success_flag = false;
																	PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add param option \"%s\": \"%s\"", id_s, program_p -> pr_name_s);
																}

															FreeCopiedString (id_s);
														}
													else
														{
															success_flag = false;
															PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, entry_p, "Failed to get Programme BSON oid");
														}

													FreeProgramme (program_p);
												}		/* if (program_p) */


											if (success_flag)
												{
													++ i;
												}

										}		/* while ((i < num_results) && success_flag) */

									/*
									 * If the parameter's value isn't on the list, reset it
									 */
									if ((param_value_s != NULL) && (strcmp (param_value_s, S_EMPTY_LIST_OPTION_S) != 0) && (value_set_flag == false))
										{
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "param value \"%s\" not on list of existing programmes", param_value_s);
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
			if (active_program_p)
				{
					char *id_s = GetBSONOidAsString (active_program_p -> pr_id_p);

					if (id_s)
						{
							success_flag = SetStringParameterDefaultValue (param_p, id_s);
							FreeCopiedString (id_s);
						}
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get id string for active program \"%s\"", active_program_p -> pr_name_s);
							success_flag = false;
						}
				}
		}

	return success_flag;
}


bool RunForSubmissionProgrammeParams (FieldTrialServiceData *data_p, ParameterSet *param_set_p, ServiceJob *job_p)
{
	bool success_flag = AddProgramme (job_p, param_set_p, data_p);

	return success_flag;
}



bool GetSubmissionProgrammeParameterTypeForNamedParameter (const char *param_name_s, ParameterType *pt_p)
{
	bool success_flag = true;

	if (strcmp (param_name_s, PROGRAMME_ID.npt_name_s) == 0)
		{
			*pt_p = PROGRAMME_ID.npt_type;
		}
	else if (strcmp (param_name_s, PROGRAMME_ABBREVIATION.npt_name_s) == 0)
		{
			*pt_p = PROGRAMME_ABBREVIATION.npt_type;
		}
	else if (strcmp (param_name_s, PROGRAMME_CROP.npt_name_s) == 0)
		{
			*pt_p = PROGRAMME_CROP.npt_type;
		}
	else if (strcmp (param_name_s, PROGRAMME_NAME.npt_name_s) == 0)
		{
			*pt_p = PROGRAMME_NAME.npt_type;
		}
	else if (strcmp (param_name_s, PROGRAMME_OBJECTIVE.npt_name_s) == 0)
		{
			*pt_p = PROGRAMME_OBJECTIVE.npt_type;
		}
	else if (strcmp (param_name_s, PROGRAMME_PI_NAME.npt_name_s) == 0)
		{
			*pt_p = PROGRAMME_PI_NAME.npt_type;
		}
	else if (strcmp (param_name_s, PROGRAMME_PI_EMAIL.npt_name_s) == 0)
		{
			*pt_p = PROGRAMME_PI_EMAIL.npt_type;
		}
	else if (strcmp (param_name_s, PROGRAMME_URL.npt_name_s) == 0)
		{
			*pt_p = PROGRAMME_URL.npt_type;
		}
	else if (strcmp (param_name_s, PROGRAMME_LOGO.npt_name_s) == 0)
		{
			*pt_p = PROGRAMME_LOGO.npt_type;
		}
	else
		{
			success_flag = false;
		}

	return success_flag;
}


json_t *GetProgrammeIndexingData (Service *service_p)
{
	FieldTrialServiceData *data_p = (FieldTrialServiceData *) (service_p -> se_data_p);
	json_t *src_programs_p = GetAllProgrammesAsJSON (data_p, NULL);

	if (src_programs_p)
		{
			if (json_is_array (src_programs_p))
				{
					size_t i;
					json_t *src_program_p;

					json_array_foreach (src_programs_p, i, src_program_p)
						{
							bson_oid_t id;

							if (AddDatatype (src_program_p, DFTD_PROGRAMME))
								{
									if (GetMongoIdFromJSON (src_program_p, &id))
										{
											Crop *crop_p = GetStoredCropValue (src_program_p, PR_CROP_S, data_p);

											if (crop_p)
												{
													SetJSONString (src_program_p, PR_CROP_S, crop_p -> cr_name_s);
													FreeCrop (crop_p);
												}

										}		/* if (GetMongoIdFromJSON (entry_p, &id)) */

								}


						}		/* json_array_foreach (src_studies_p, i, src_study_p) */

				}		/* if (json_is_array (src_studies_p)) */

			return src_programs_p;
		}		/* if (src_studies_p) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "No programs for \"%s\"", GetServiceName (service_p));
		}

	return NULL;
}


bool SaveProgrammeAsFrictionlessData (const Programme *programme_p, const FieldTrialServiceData *data_p)
{
	bool success_flag = false;
	char *full_filename_s = GetFrictionlessDataFilename (programme_p -> pr_name_s, data_p);

	if (full_filename_s)
		{
			json_t *programme_fd_p = GetProgrammeAsFrictionlessDataPackage (programme_p, data_p);

			if (programme_fd_p)
				{
					if (json_dump_file (programme_fd_p, full_filename_s, JSON_INDENT (2)) == 0)
						{
							success_flag = true;
						}

					json_decref (programme_fd_p);
				}		/* if (programme_fd_p) */

			FreeCopiedString (full_filename_s);
		}		/* if (full_filename_s) */


	return success_flag;

}



json_t *GetProgrammeAsFrictionlessDataPackage (const Programme *programme_p, const FieldTrialServiceData *data_p)
{
	json_t *package_p = json_object ();

	if (package_p)
		{
			if (SetJSONString (package_p, FD_PROFILE_S, FD_PROFILE_DATA_PACKAGE_S))
				{
					if (SetJSONString (package_p, FD_NAME_S, programme_p -> pr_name_s))
						{
							json_t *resources_p = json_array ();

							if (resources_p)
								{
									if (json_object_set_new (package_p, FD_RESOURCES_S, resources_p) == 0)
										{
											json_t *programme_fd_p = GetProgrammeAsFrictionlessDataResource (programme_p, data_p);

											if (programme_fd_p)
												{
													if (json_array_append_new (resources_p, programme_fd_p) == 0)
														{
															return package_p;
														}

												}

										}		/* if (json_object_set_new (package_p, FD_RESOURCES_S, resources_p) == 0) */
									else
										{
											json_decref (resources_p);
										}
								}
						}

				}		/* if (SetJSONString (package_p, FD_PROFILE_S, FD_PROFILE_DATA_S)) */

			json_decref (package_p);
		}


	return NULL;
}



json_t *GetProgrammeAsFrictionlessDataResource (const Programme *programme_p, const FieldTrialServiceData *data_p)
{
	json_t *programme_fd_p = json_object ();

	if (programme_fd_p)
		{
			bool success_flag = true;

			const char * const FD_SCHEMA_URL_S = "https://grassroots.tools/frictionless-data/schemas/field-trials/programme-resource.json";

			if (SetJSONString (programme_fd_p, FD_PROFILE_S, FD_SCHEMA_URL_S))
				{
					char *id_s = GetBSONOidAsString (programme_p -> pr_id_p);

					if (id_s)
						{
							if (SetJSONString (programme_fd_p, FD_ID_S, id_s))
								{
									if (SetJSONString (programme_fd_p, FD_NAME_S, programme_p -> pr_name_s))
										{
											const char * const FD_ABBREVIATION_S = "abbreviation";

											if (SetNonTrivialString (programme_fd_p, FD_ABBREVIATION_S, programme_p -> pr_abbreviation_s, false))
												{

													if (programme_p -> pr_crop_p)
														{
														}		/* if (programme_p -> pr_crop_p) */

													if (success_flag)
														{
															const char * const URL_S = "url";

															if (SetNonTrivialString (programme_fd_p, URL_S, programme_p -> pr_documentation_url_s, false))
																{

																	if (programme_p -> pr_pi_p)
																		{
																			const char * const FD_PI_NAME_S = "pi_name";

																			if (SetNonTrivialString (programme_fd_p, FD_PI_NAME_S, programme_p -> pr_pi_p -> pe_name_s, false))
																				{
																					const char * const FD_PI_EMAIL_S = "pi_email";

																					if (SetNonTrivialString (programme_fd_p, FD_PI_EMAIL_S, programme_p -> pr_pi_p -> pe_email_s, false))
																						{

																						}		/* if (SetNonTrivialString (programme_fd_p, FD_PI_EMAIL_S, programme_p -> pr_pi_p -> pe_email_s, false)) */
																					else
																						{
																							success_flag = false;
																							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, programme_fd_p, "Failed to set \"%s\": \"%s\"", FD_PI_EMAIL_S, programme_p -> pr_pi_p -> pe_email_s);
																						}

																				}		/* if (SetNonTrivialString (programme_fd_p, FD_PI_NAME_S, programme_p -> pr_pi_p -> pe_name_s, false)) */
																			else
																				{
																					success_flag = false;
																					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, programme_fd_p, "Failed to set \"%s\": \"%s\"", FD_PI_NAME_S, programme_p -> pr_pi_p -> pe_name_s);
																				}

																		}		/* if (programme_p -> pr_pi_p) */

																	if (success_flag)
																		{
																			const char * const FD_LOGO_S = "logo";

																			if (SetNonTrivialString (programme_fd_p, FD_LOGO_S, programme_p -> pr_logo_url_s, false))
																				{

																				}		/* if (SetNonTrivialString (programme_fd_p, FD_LOGO_S, programme_p -> pr_logo_url_s, false)) */
																			else
																				{
																					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, programme_fd_p, "Failed to set \"%s\": \"%s\"", FD_LOGO_S, programme_p -> pr_logo_url_s);
																				}

																		}		/* if (success_flag) */

																}		/* if (SetNonTrivialString (programme_fd_p, URL_S, programme_p -> pr_documentation_url_s, false)) */
															else
																{
																	PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, programme_fd_p, "Failed to set \"%s\": \"%s\"", URL_S, programme_p -> pr_documentation_url_s);
																}

														}		/* if (success_flag) */

												}		/* if (SetNonTrivialString (study_fd_p, FD_ABBREVIATION_S, study_p -> st_description_s, false)) */
											else
												{
													PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, programme_fd_p, "Failed to set \"%s\": \"%s\"", FD_ABBREVIATION_S, programme_p -> pr_abbreviation_s);
												}

										}		/* if (SetJSONString (programme_fd_p, FD_NAME_S, programme_p -> pr_name_s)) */
									else
										{
											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, programme_fd_p, "Failed to set \"%s\": \"%s\"", FD_NAME_S, programme_p -> pr_name_s);
										}

								}		/* if (SetJSONString (programme_fd_p, FD_ID_S, id_s)) */

							FreeCopiedString (id_s);
						}		/* if (id_s) */
					else
						{

						}

				}		/* if (SetJSONString (programme_fd_p, FD_PROFILE_S, FD_SCHEMA_URL_S)) */
			else
				{
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, programme_fd_p, "Failed to set \"%s\": \"%s\"", FD_PROFILE_S, FD_SCHEMA_URL_S);
				}

			if (!success_flag)
				{
					json_decref (programme_fd_p);
					programme_fd_p = NULL;
				}

		}		/* if (programme_fd_p) */

	return programme_fd_p;
}



static bool AddProgramme (ServiceJob *job_p, ParameterSet *param_set_p, FieldTrialServiceData *data_p)
{
	OperationStatus status = OS_FAILED;
	const char *id_s = NULL;
	const char *name_s = NULL;
	bson_oid_t *programme_id_p = NULL;

	/*
	 * Get the existing program id if specified
	 */
	GetCurrentStringParameterValueFromParameterSet (param_set_p, PROGRAMME_ID.npt_name_s, &id_s);

	if (id_s)
		{
			if (strcmp (S_EMPTY_LIST_OPTION_S, id_s) != 0)
				{
					programme_id_p = GetBSONOidFromString (id_s);

					if (!programme_id_p)
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to load Programme \"%s\" for editing", id_s);
							return false;
						}
				}
		}		/* if (id_value.st_string_value_s) */


	if (GetCurrentStringParameterValueFromParameterSet (param_set_p, PROGRAMME_NAME.npt_name_s, &name_s))
		{
			const char *pi_name_s = NULL;

			if (GetCurrentStringParameterValueFromParameterSet (param_set_p, PROGRAMME_PI_NAME.npt_name_s, &pi_name_s))
				{
					const char *pi_email_s = NULL;

					if (GetCurrentStringParameterValueFromParameterSet (param_set_p, PROGRAMME_PI_EMAIL.npt_name_s, &pi_email_s))
						{
							Person *person_p = AllocatePerson (pi_name_s, pi_email_s);

							if (person_p)
								{
									bool freed_person_flag = false;
									Programme *programme_p = NULL;
									const char *abbreviation_s = NULL;
									const char *crop_id_s = NULL;
									const char *url_s = NULL;
									const char *objective_s = NULL;
									const char *logo_s = NULL;
									Crop *crop_p = NULL;

									GetCurrentStringParameterValueFromParameterSet (param_set_p, PROGRAMME_ABBREVIATION.npt_name_s, &abbreviation_s);
									GetCurrentStringParameterValueFromParameterSet (param_set_p, PROGRAMME_CROP.npt_name_s, &crop_id_s);
									GetCurrentStringParameterValueFromParameterSet (param_set_p, PROGRAMME_URL.npt_name_s, &url_s);
									GetCurrentStringParameterValueFromParameterSet (param_set_p, PROGRAMME_OBJECTIVE.npt_name_s, &objective_s);
									GetCurrentStringParameterValueFromParameterSet (param_set_p, PROGRAMME_LOGO.npt_name_s, &logo_s);

									crop_p = GetCropByIdString (crop_id_s, data_p);

									programme_p = AllocateProgramme (programme_id_p, abbreviation_s, crop_p, url_s, name_s, objective_s, person_p, logo_s);

									if (programme_p)
										{
											status = SaveProgramme (programme_p, job_p, data_p);

											if (status == OS_FAILED)
												{
													PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to save Programme named \"%s\"", name_s);
												}

											FreeProgramme (programme_p);
											freed_person_flag = true;
										}
									else
										{
											if (crop_p)
												{
													FreeCrop (crop_p);
												}
										}

									if (!freed_person_flag)
										{
											FreePerson (person_p);
										}

								}		/* if (person_p) */
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate Person for %s, %s in programme %s", pi_name_s, pi_email_s, name_s);
								}

						}
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get Programme parameter %s", PROGRAMME_PI_EMAIL.npt_name_s);
						}

				}
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get Programme parameter %s", PROGRAMME_PI_NAME.npt_name_s);
				}

		}
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get Programme parameter %s", PROGRAMME_NAME.npt_name_s);
		}


	SetServiceJobStatus (job_p, status);

	return ((status == OS_SUCCEEDED) || (status == OS_PARTIALLY_SUCCEEDED));
}


bool AddProgrammeToServiceJob (ServiceJob *job_p, Programme *program_p, const ViewFormat format, FieldTrialServiceData *data_p)
{
	bool success_flag = false;
	json_t *program_json_p = GetProgrammeAsJSON (program_p, format, data_p);

	if (program_json_p)
		{
			if (AddProgrammeToServiceJobResult (job_p, program_p, program_json_p, format, data_p))
				{
					success_flag = true;
				}

			json_decref (program_json_p);
		}		/* if (program_json_p) */

	return success_flag;
}



static bool SetUpDefaultsFromExistingProgramme (const Programme *programme_p, char **id_ss,  char **name_ss, char **abbreviation_ss, Crop **crop_pp, char **documentation_url_ss, char **objective_ss, char **pi_name_ss, char **pi_email_ss, char **logo_ss)
{
	char *program_id_s = GetBSONOidAsString (programme_p -> pr_id_p);

	if (program_id_s)
		{
			*id_ss = program_id_s;
			*name_ss = programme_p -> pr_name_s;
			*abbreviation_ss = programme_p -> pr_abbreviation_s;
			*crop_pp = programme_p -> pr_crop_p;
			*documentation_url_ss = programme_p -> pr_documentation_url_s;
			*objective_ss = programme_p -> pr_objective_s;
			*logo_ss = programme_p -> pr_logo_url_s;

			if (programme_p -> pr_pi_p)
				{
					*pi_name_ss = programme_p -> pr_pi_p -> pe_name_s;
					*pi_email_ss = programme_p -> pr_pi_p -> pe_email_s;
				}

			return true;
		}		/* if (tprogrma_id_s) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to copy programme id for \"%s\"", programme_p -> pr_name_s);
		}

	return false;

}



static bool AddProgrammeToServiceJobResult (ServiceJob *job_p, Programme *program_p, json_t *program_json_p, const ViewFormat format, FieldTrialServiceData *data_p)
{
	bool success_flag = false;

	json_t *dest_record_p = GetResourceAsJSONByParts (PROTOCOL_INLINE_S, NULL, program_p -> pr_name_s, program_json_p);

	if (dest_record_p)
		{
			AddImage (dest_record_p, DFTD_PROGRAMME, data_p);

			if (AddResultToServiceJob (job_p, dest_record_p))
				{
					success_flag = true;
				}
			else
				{
					json_decref (dest_record_p);
				}

		}		/* if (dest_record_p) */

	return success_flag;
}



Programme *GetProgrammeFromResource (Resource *resource_p, const NamedParameterType program_param_type, FieldTrialServiceData *dfw_data_p)
{
	Programme *program_p = NULL;

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
							const char *program_id_s = GetIDDefaultValueFromJSON (program_param_type.npt_name_s, params_json_p);

							/*
							 * Do we have an existing program id?
							 */
							if (program_id_s)
								{
									program_p = GetProgrammeByIdString (program_id_s, VF_CLIENT_MINIMAL, dfw_data_p);

									if (!program_p)
										{
											PrintJSONToErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, params_json_p, "Failed to load Programme with id \"%s\"", program_id_s);
										}

								}		/* if (study_id_s) */

						}
					else
						{
							PrintJSONToErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, param_set_json_p, "Failed to get params with key \"%s\"", PARAM_SET_PARAMS_S);
						}
				}
			else
				{
					PrintJSONToErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, resource_p -> re_data_p, "Failed to get param set with key \"%s\"", PARAM_SET_KEY_S);
				}

		}		/* if (resource_p && (resource_p -> re_data_p)) */

	return program_p;
}



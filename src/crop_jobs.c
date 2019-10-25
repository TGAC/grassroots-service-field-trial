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
 * crop_jobs.c
 *
 *  Created on: 17 Apr 2019
 *      Author: billy
 */



#include "crop.h"
#include "crop_jobs.h"
#include "string_utils.h"



/*
 * static declarations
 */

static NamedParameterType S_NAME = { "CR Name", PT_KEYWORD };
static NamedParameterType S_PREFERRED_TERM = { "CR Preferred Term", PT_KEYWORD};
static NamedParameterType S_ONTOLOGY_URL = { "CR Ontology URL", PT_STRING };
static NamedParameterType S_SYNONYMS = { "CR Synonyms", PT_LARGE_STRING };


static bool SetDefaultCropValue (Parameter *param_p, const char *crop_s);


static const char * const S_EMPTY_LIST_OPTION_S = "<empty>";

/*
 * API definitions
 */

bool AddSubmissionCropParams (ServiceData *data_p, ParameterSet *param_set_p)
{
	bool success_flag = false;

	ParameterGroup *group_p = CreateAndAddParameterGroupToParameterSet ("Crops", false, data_p, param_set_p);

	if (group_p)
		{
			Parameter *param_p = NULL;
			SharedType def;

			InitSharedType (&def);

			def.st_string_value_s = NULL;

			if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_NAME.npt_type, S_NAME.npt_name_s, "Name", "The crop name", def, PL_ALL)) != NULL)
				{
					if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_PREFERRED_TERM.npt_type, S_PREFERRED_TERM.npt_name_s, "Preferred name", "The AgroVOC preferred term for the crop", def, PL_ALL)) != NULL)
						{
							if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_ONTOLOGY_URL.npt_type, S_ONTOLOGY_URL.npt_name_s, "Definition", "The URL of the ontology definition for this crop", def, PL_ALL)) != NULL)
								{
									if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_SYNONYMS.npt_type, S_SYNONYMS.npt_name_s, "Synonyms", "The comma-separated list of synonyms for this crop", def, PL_ALL)) != NULL)
										{
											success_flag = true;
										}
								}
						}
				}
		}		/* if (group_p) */


	return success_flag;
}


bool RunForSubmissionCropParams (DFWFieldTrialServiceData *data_p, ParameterSet *param_set_p, ServiceJob *job_p)
{
	bool job_done_flag = false;

	SharedType name_value;
	InitSharedType (&name_value);

	if (GetParameterValueFromParameterSet (param_set_p, S_NAME.npt_name_s, &name_value, true))
		{
			/*
			 * The name is required
			 */
			if (! (IsStringEmpty (name_value.st_string_value_s)))
				{
					SharedType term_value;
					InitSharedType (&term_value);
					OperationStatus status = OS_FAILED_TO_START;

					if (GetParameterValueFromParameterSet (param_set_p, S_PREFERRED_TERM.npt_name_s, &term_value, true))
						{
							SharedType ontology_value;
							InitSharedType (&ontology_value);

							if (GetParameterValueFromParameterSet (param_set_p, S_ONTOLOGY_URL.npt_name_s, &ontology_value, true))
								{
									SharedType synonyms_value;
									InitSharedType (&synonyms_value);

									if (GetParameterValueFromParameterSet (param_set_p, S_SYNONYMS.npt_name_s, &synonyms_value, true))
										{
											bool success_flag = true;
											char **synonyms_ss = NULL;

											if (!IsStringEmpty (synonyms_value.st_string_value_s))
												{
													LinkedList *synonyms_p = ParseStringToStringLinkedList (synonyms_value.st_string_value_s, ",", false);

													if (synonyms_p)
														{
															synonyms_ss = (char **) AllocMemoryArray (1 + synonyms_p -> ll_size, sizeof (char *));

															if (synonyms_ss)
																{
																	StringListNode *node_p = (StringListNode *) synonyms_p -> ll_head_p;
																	char **synonym_ss = synonyms_ss;

																	while (node_p)
																		{
																			*synonym_ss = DetachStringFromStringListNode (node_p);

																			++ synonym_ss;
																			node_p = (StringListNode *) (node_p -> sln_node.ln_next_p);
																		}

																}

															FreeLinkedList (synonyms_p);
														}
													else
														{
															PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to parse \"%s\" for synonyms", synonyms_value.st_string_value_s);
															success_flag = false;
														}

												}		/* if (!IsStringEmpty (synonyms_value.st_string_value_s)) */

											if (success_flag)
												{
													Crop *crop_p = AllocateCrop (NULL, name_value.st_string_value_s, term_value.st_string_value_s, ontology_value.st_string_value_s, synonyms_ss);

													if (crop_p)
														{
															if (SaveCrop (crop_p, data_p))
																{
																	status = OS_SUCCEEDED;
																}
														}
													else
														{

															status = OS_FAILED;
															if (synonyms_ss)
																{
																	char **synonym_ss = synonyms_ss;

																	while (*synonym_ss)
																		{
																			FreeCopiedString (*synonym_ss);
																			++ synonym_ss;
																		}
																}
														}

												}

										}		/* if (GetParameterValueFromParameterSet (param_set_p, S_SYNONYMS.npt_name_s, &synonyms_value, true)) */
									else
										{
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get \"%s\" parameter value", S_SYNONYMS.npt_name_s);
										}

								}		/* if (GetParameterValueFromParameterSet (param_set_p, S_ONTOLOGY_URL.npt_name_s, &ontology_value, true)) */
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get \"%s\" parameter value", S_ONTOLOGY_URL.npt_name_s);
								}

						}		/* if (GetParameterValueFromParameterSet (param_set_p, S_PREFERRED_TERM.npt_name_s, &term_value, true)) */
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get \"%s\" parameter value", S_PREFERRED_TERM.npt_name_s);
						}

					job_done_flag = true;

					SetServiceJobStatus (job_p, status);

				}		/* if (! (IsStringEmpty (value.st_string_value_s))) */

		}		/* if (GetParameterValueFromParameterSet (param_set_p, S_NAME.npt_name_s, &name_value, true)) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get \"%s\" parameter value", S_NAME.npt_name_s);
		}


	return job_done_flag;
}


bool GetSubmissionCropParameterTypeForNamedParameter (const char *param_name_s, ParameterType *pt_p)
{
	bool success_flag = true;

	if (strcmp (param_name_s, S_NAME.npt_name_s) == 0)
		{
			*pt_p = S_NAME.npt_type;
		}
	else if (strcmp (param_name_s, S_PREFERRED_TERM.npt_name_s) == 0)
		{
			*pt_p = S_PREFERRED_TERM.npt_type;
		}
	else if (strcmp (param_name_s, S_ONTOLOGY_URL.npt_name_s) == 0)
		{
			*pt_p = S_ONTOLOGY_URL.npt_type;
		}
	else if (strcmp (param_name_s, S_SYNONYMS.npt_name_s) == 0)
		{
			*pt_p = S_SYNONYMS.npt_type;
		}
	else
		{
			success_flag = false;
		}

	return success_flag;
}


bool SetUpCropsListParameter (const DFWFieldTrialServiceData *data_p, Parameter *param_p, const char *empty_option_s)
{
	bool success_flag = false;

	if (SetMongoToolCollection (data_p -> dftsd_mongo_p, data_p -> dftsd_collection_ss [DFTD_CROP]))
		{
			bson_t *query_p = NULL;
			bson_t *opts_p =  BCON_NEW ( "sort", "{", CR_NAME_S, BCON_INT32 (1), "}", "collation", "{", "locale", BCON_UTF8 ("en"), "}");
			json_t *results_p = GetAllMongoResultsAsJSON (data_p -> dftsd_mongo_p, query_p, opts_p);

			if (results_p)
				{
					if (json_is_array (results_p))
						{
							const size_t num_results = json_array_size (results_p);

							if (num_results > 0)
								{
									size_t i;
									const json_t *service_config_p = data_p -> dftsd_base_data.sd_config_p;
									const char *default_crop_s = GetJSONString (service_config_p, "default_crop");

									SharedType def;

									InitSharedType (&def);

									/*
									 * If there's an empty option, add it
									 */
									if (empty_option_s)
										{
											def.st_string_value_s = (char *) S_EMPTY_LIST_OPTION_S;

											success_flag = CreateAndAddParameterOptionToParameter (param_p, def, S_EMPTY_LIST_OPTION_S);
										}

									for (i = 0; i < num_results; ++ i)
										{
											json_t *entry_p = json_array_get (results_p, i);
											Crop *crop_p = GetCropFromJSON (entry_p, data_p);

											if (crop_p)
												{
													char *id_s = GetBSONOidAsString (crop_p -> cr_id_p);

													if (id_s)
														{
															InitSharedType (&def);

															def.st_string_value_s = id_s;

															if (param_p)
																{
																	success_flag  = true;

																	if ((!default_crop_s) && (i == 0))
																		{
																			success_flag = SetDefaultCropValue (param_p, crop_p -> cr_name_s);
																		}

																	success_flag = CreateAndAddParameterOptionToParameter (param_p, def, crop_p -> cr_name_s);
																}

															FreeCopiedString (id_s);
														}

													FreeCrop (crop_p);
												}		/* if (crop_p) */

										}		/* for (i = 0; i < num_results; ++ i)) */


									if (success_flag)
										{
											if (default_crop_s)
												{
													success_flag = SetDefaultCropValue (param_p, default_crop_s);
												}
										}

								}		/* if (num_results > 0) */
							else
								{
									/* nothing to add */
									success_flag = true;
								}

						}		/* if (json_is_array (results_p)) */

					json_decref (results_p);
				}		/* if (results_p) */

			if (opts_p)
				{
					bson_destroy (opts_p);
				}

		}		/* if (SetMongoToolCollection (data_p -> dftsd_mongo_p, data_p -> dftsd_collection_ss [DFTD_FIELD_TRIAL])) */

	return success_flag;
}


static bool SetDefaultCropValue (Parameter *param_p, const char *crop_s)
{
	bool success_flag = false;
	SharedType def;

	InitSharedType (&def);

	def.st_string_value_s = (char *) crop_s;

	if (SetParameterValueFromSharedType (param_p, &def, false))
		{
			if (SetParameterValueFromSharedType (param_p, &def, true))
				{
					success_flag = true;
				}
		}

	return success_flag;
}

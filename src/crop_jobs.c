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

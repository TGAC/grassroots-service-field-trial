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
 * treatment_factor.c
 *
 *  Created on: 5 Mar 2020
 *      Author: billy
 */


#include "treatment_factor.h"
#include "string_utils.h"
#include "memory_allocations.h"
#include "streams.h"



TreatmentFactor *AllocateTreatmentFactor (const char * const name_s)
{
	char *copied_name_s = EasyCopyToNewString (name_s);

	if (copied_name_s)
		{
			LinkedList *list_p = AllocateLinkedList (FreeTreatmentFactorNode);

			if (list_p)
				{
					TreatmentFactor *treatment_factor_p = (TreatmentFactor *) AllocMemory (sizeof (TreatmentFactor));

					if (treatment_factor_p)
						{
							treatment_factor_p -> tf_name_s = copied_name_s;
							treatment_factor_p -> tf_values_p = list_p;

							return treatment_factor_p;
						}		/* if (treatment_factor_p) */
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate memory for TreatmentFactor");
						}

					FreeLinkedList (list_p);
				}		/* if (list_p) */
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate list for TreatmentFactor");
				}

			FreeCopiedString (copied_name_s);
		}		/* if (copied_name_s) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to copy TreatmnetFactor name \"%s\"", name_s);
		}

	return NULL;
}


void FreeTreatmentFactor (TreatmentFactor *treatment_p)
{
	FreeCopiedString (treatment_p -> tf_name_s);
	FreeLinkedList (treatment_p -> tf_values_p);

	FreeMemory (treatment_p);
}


bool AddTreatmentFactorValue (TreatmentFactor *treatment_p, const char *name_s, const char *value_s)
{
	bool success_flag = false;
	KeyValuePairNode *node_p = AllocateKeyValuePairNode (name_s, value_s);

	if (node_p)
		{
			LinkedListAddTail (treatment_p -> tf_values_p, node_p);
			success_flag = true;
		}
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate TreatmnetFactor value for \"%s\": \"s\"", name_s, value_s);
		}

	return success_flag;
}


TreatmentFactorNode *AllocateTreatmentFactorNode (TreatmentFactor *treatment_p)
{
	TreatmentFactorNode *node_p = (TreatmentFactorNode *) AllocMemory (sizeof (TreatmentFactorNode));

	if (node_p)
		{
			InitListItem (& (node_p -> tfn_node));
			node_p -> tfn_treatment_p = treatment_p;

			return treatment_p;
		}
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate TreatmnetFactorNode for treatment \"%s\"", treatment_p -> tf_name_s);
		}

	return NULL;
}


void FreeTreatmentFactorNode (ListItem *node_p)
{
	TreatmentFactorNode *tf_node_p = (TreatmentFactorNode *) node_p;

	FreeTreatmentFactor (tf_node_p -> tfn_treatment_p);
	FreeMemory (tf_node_p);
}



json_t *GetTreatmentFactorAsJSON (const TreatmentFactor *treatment_p)
{
	json_t *tf_json_p = json_object ();

	if (tf_json_p)
		{
			if (SetJSONString (tf_json_p, TF_NAME_S, treatment_p -> tf_name_s))
				{
					bool success_flag = true;

					if (treatment_p -> tf_values_p -> ll_size)
						{
							json_t *factors_json_p = json_array ();

							if (factors_json_p)
								{
									if (json_object_set_new (tf_json_p, TF_VALUES_S, factors_json_p) == 0)
										{
											KeyValuePairNode *node_p = (KeyValuePairNode *) (treatment_p -> tf_values_p -> ll_head_p);

											while (success_flag && node_p)
												{
													const KeyValuePair *kvp_p = node_p -> kvpn_pair_p;
													json_t *kvp_json_p = GetKeyValuePairAsJSON (kvp_p);

													if (kvp_json_p)
														{
															if (json_array_append_new (factors_json_p, kvp_json_p) == 0)
																{
																	node_p = (KeyValuePairNode *) (node_p -> kvpn_node.ln_next_p);
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
										}
									else
										{
											json_decref (factors_json_p);
										}

								}		/* if (factors_json_p) */
							else
								{

								}

						}		/* if (treatment_p -> tf_values_p -> ll_size) */

					if (success_flag)
						{
							return tf_json_p;
						}

				}		/* if (SetJSONString (tf_json_p, TF_NAME_S, treatment_p -> tf_name_s)) */

			json_decref (tf_json_p);
		}		/* if (tf_json_p) */

	return NULL;
}


TreatmentFactor *GetTreatmentFactorFromJSON (const json_t *treatmnent_json_p)
{
	const char *name_s = GetJSONString (treatmnent_json_p, TF_NAME_S);

	if (name_s)
		{
			TreatmentFactor *tf_p = AllocateTreatmentFactor (name_s);

			json_t *values_p = json_object_get (treatmnent_json_p, TF_VALUES_S);

			if (values_p)
				{

				}		/* if (values_p) */

		}		/* if (name_s) */

	return NULL;
}


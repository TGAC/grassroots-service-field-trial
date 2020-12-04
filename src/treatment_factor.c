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
 *  Created on: 4 Dec 2020
 *      Author: billy
 */

#include "treatment_factor.h"
#include "memory_allocations.h"
#include "key_value_pair.h"


static const char *S_STUDY_ID_S = "study_id";
static const char *S_STUDY_NAME_S = "study_name";
static const char *S_TREATMENT_S = "treatment";
static const char *S_TREATMENT_ID_S = "treatment_id";
static const char *S_VALUES_S = "values";
static const char *S_VALUES_KEY_S = "so:name";
static const char *S_VALUES_VALUE_S = "value";


/*
 * Static Declarations
 */

static json_t *GetValuesAsJSON (LinkedList *values_p);

static bool AddKeyValuePairAsJSON (const KeyValuePair *pair_p, json_t *array_p);



/*
 * API Definitions
 */

TreatmentFactor *AllocateTreatmentFactor (Treatment *treatment_p, Study *study_p)
{
	LinkedList *list_p = AllocateLinkedList (FreeKeyValuePairNode);

	if (list_p)
		{
			TreatmentFactor *tf_p = (TreatmentFactor *) AllocMemory (sizeof (TreatmentFactor));

			if (tf_p)
				{
					tf_p -> tf_study_p = study_p;
					tf_p -> tf_treatment_p = treatment_p;
					tf_p -> tf_values_p = list_p;

					return tf_p;
				}

			FreeLinkedList (list_p);
		}

	return NULL;
}


void FreeTreatmentFactor (TreatmentFactor *treatment_factor_p)
{
	FreeTreatment (treatment_factor_p -> tf_treatment_p);
	FreeLinkedList (treatment_factor_p -> tf_values_p);

	FreeMemory (treatment_factor_p);
}


bool AddTreatmentFactorValue (TreatmentFactor *treatment_factor_p, const char *name_s, const char *value_s)
{
	bool success_flag = false;
	KeyValuePairNode *node_p = AllocateKeyValuePairNodeByParts (name_s, value_s);

	if (node_p)
		{
			LinkedListAddTail (treatment_factor_p -> tf_values_p, & (node_p -> kvpn_node));
			success_flag = true;
		}

	return success_flag;
}


TreatmentFactorNode *AllocateTreatmentFactortNode (TreatmentFactor *treatment_factor_p)
{
	TreatmentFactorNode *node_p = (TreatmentFactorNode *) AllocMemory (sizeof (TreatmentFactorNode));

	if (node_p)
		{
			InitListItem (& (node_p -> tfn_node));
			node_p -> tfn_p = treatment_factor_p;
		}

	return node_p;
}


void FreeTreatmentNode (ListItem *node_p)
{
	TreatmentFactorNode *tf_node_p = (TreatmentFactorNode *) node_p;

	FreeTreatmentFactor (tf_node_p -> tfn_p);

	FreeMemory (node_p);
}


json_t *GetTreatmentFactorAsJSON (const TreatmentFactor *treatment_factor_p, const ViewFormat format)
{
	json_t *tf_json_p = json_object ();

	if (tf_json_p)
		{
			json_t *values_json_p = GetValuesAsJSON (treatment_factor_p -> tf_values_p);

			if (values_json_p)
				{
					if (json_object_set_new (tf_json_p, S_VALUES_S, values_json_p) == 0)
						{
							bool success_flag = false;

							if (AddNamedCompoundIdToJSON (tf_json_p, treatment_factor_p -> tf_study_p -> st_id_p, S_STUDY_ID_S))
								{
									if (format == VF_STORAGE)
										{
											if (AddNamedCompoundIdToJSON (tf_json_p, treatment_factor_p -> tf_treatment_p -> tr_id_p, S_TREATMENT_ID_S))
												{
													success_flag = true;
												}
										}
									else
										{
											json_t *treatment_json_p = GetTreatmentAsJSON (treatment_factor_p -> tf_treatment_p);

											if (treatment_json_p)
												{
													if (json_object_set_new (tf_json_p, S_TREATMENT_S, treatment_json_p) == 0)
														{
															if (SetJSONString (tf_json_p, S_STUDY_NAME_S, treatment_factor_p -> tf_study_p -> st_name_s))
																{
																	success_flag = true;
																}
														}
													else
														{
															json_decref (treatment_json_p);
														}
												}
										}
								}

							if (success_flag)
								{
									return tf_json_p;
								}
						}
					else
						{
							json_decref (values_json_p);
						}

				}		/* if (values_json_p) */

			json_decref (tf_json_p);
		}		/* if (tf_json_p) */

	return NULL;
}


TreatmentFactor *GetTreatmentFactorFromJSON (const json_t *treatment_factor_json_p)
{
	TreatmentFactor *tf_p = NULL;
	bson_oid_t *study_id_p = GetNewUnitialisedBSONOid ();

	if (study_id_p)
		{
			if (GetNamedIdFromJSON (treatment_factor_json_p, S_STUDY_ID_S, study_id_p))
				{

				}		/* if (GetNamedIdFromJSON (treatment_factor_json_p, S_STUDY_ID_S, study_id_p)) */

		}		/* if (study_id_p) */

	return tf_p;
}



/*
 * Static Definitions
 */

static json_t *GetValuesAsJSON (LinkedList *values_p)
{
	json_t *values_json_p = json_array ();

	if (values_json_p)
		{
			bool success_flag = true;
			KeyValuePairNode *node_p = (KeyValuePairNode *) (values_p -> ll_head_p);

			while (node_p && success_flag)
				{
					if (AddKeyValuePairAsJSON (node_p -> kvpn_pair_p, values_json_p))
						{
							node_p = (KeyValuePairNode *) (node_p -> kvpn_node.ln_next_p);
						}
					else
						{
							success_flag = false;
						}
				}

			if (success_flag)
				{
					return values_json_p;
				}

			json_decref (values_json_p);
		}		/* if (values_json_p) */

	return NULL;
}


static bool AddKeyValuePairAsJSON (const KeyValuePair *pair_p, json_t *array_p)
{
	json_t *entry_p = json_object ();

	if (entry_p)
		{
			if (SetJSONString (entry_p, S_VALUES_KEY_S, pair_p -> kvp_key_s))
				{
					if (SetJSONString (entry_p, S_VALUES_VALUE_S, pair_p -> kvp_value_s))
						{
							return entry_p;
						}
				}

			json_decref (entry_p);
		}

	return NULL;
}

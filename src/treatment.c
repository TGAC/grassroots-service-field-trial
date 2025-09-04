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

#define ALLOCATE_TREATMENT_TAGS (1)
#include "treatment.h"
#include "string_utils.h"
#include "memory_allocations.h"
#include "streams.h"
#include "jansson.h"
#include "dfw_util.h"
#include "string_array_parameter.h"


static char **GetStringsFromJSON (const json_t *treatment_json_p,  const char *key_s, size_t *num_values_p);


static bool AddStringsToJSON (const char *key_s, char **values_ss, size_t num_values, json_t *json_p);



Treatment *AllocateTreatment (SchemaTerm *term_p, char **parent_names_ss, const size_t num_parents, const bool copy_parents_flag, char **synonyms_ss, const size_t num_synonyms, const bool copy_synonyms_flag, bson_oid_t *id_p)
{
	Treatment *treatment_p = (Treatment *) AllocMemory (sizeof (Treatment));

	if (treatment_p)
		{
			bool success_flag = true;
			char **copied_parents_ss = NULL;

			if (parent_names_ss && copy_parents_flag)
				{
					copied_parents_ss = CopyStringArray (parent_names_ss, num_parents);

					if (!copied_parents_ss)
						{
							success_flag = false;
						}

				}		/* if (parent_names_ss) */
			else
				{
					copied_parents_ss = parent_names_ss;
				}

			if (success_flag)
				{
					char **copied_synonyms_ss = NULL;

					if (synonyms_ss && copy_synonyms_flag)
						{
							copied_synonyms_ss = CopyStringArray (synonyms_ss, num_synonyms);

							if (!copied_synonyms_ss)
								{
									success_flag = false;
								}
						}		/* if (parent_names_ss) */
					else
						{
							copied_synonyms_ss = synonyms_ss;
						}

					if (success_flag)
						{
							treatment_p -> tr_ontology_term_p = term_p;
							treatment_p -> tr_id_p = id_p;
							treatment_p -> tr_parent_names_ss = copied_parents_ss;
							treatment_p -> tr_num_parents = num_parents;
							treatment_p -> tr_synonyms_ss = copied_synonyms_ss;
							treatment_p -> tr_num_synonyms = num_synonyms;

							return treatment_p;
						}

				}		/* if (success_flag) */

			if (copied_parents_ss && copy_parents_flag)
				{
					FreeStringArray (copied_parents_ss, num_parents);
				}

			FreeMemory (treatment_p);
		}		/* if (treatment_p) */

	return NULL;
}



Treatment *CopyTreatment (const Treatment * const src_p)
{
	SchemaTerm *term_p = CopySchemaTerm (src_p -> tr_ontology_term_p);

	if (term_p)
		{
			bson_oid_t *id_p = CopyBSONOid (src_p -> tr_id_p);

			if (id_p)
				{
					Treatment *dest_p = AllocateTreatment (term_p, src_p -> tr_parent_names_ss, src_p -> tr_num_parents, true, src_p -> tr_synonyms_ss, src_p -> tr_num_synonyms, true, id_p);

					if (dest_p)
						{
							return dest_p;
						}

					FreeBSONOid (id_p);
				}

			FreeSchemaTerm (term_p);
		}

	return NULL;
}


void FreeTreatment (Treatment *treatment_p)
{
	FreeSchemaTerm (treatment_p -> tr_ontology_term_p);

	if (treatment_p -> tr_parent_names_ss)
		{
			FreeStringArray (treatment_p -> tr_parent_names_ss, treatment_p -> tr_num_parents);
		}


	if (treatment_p -> tr_synonyms_ss)
		{
			FreeStringArray (treatment_p -> tr_synonyms_ss, treatment_p -> tr_num_synonyms);
		}

	FreeBSONOid (treatment_p -> tr_id_p);

	FreeMemory (treatment_p);
}



//bool AddTreatmentValueByParts (Treatment *treatment_p, const char *name_s, const char *value_s)
//{
//	bool success_flag = false;
//	KeyValuePair *pair_p = AllocateKeyValuePair (name_s, value_s);
//
//	if (pair_p)
//		{
//			if (AddTreatmentValue (treatment_p, pair_p))
//				{
//					success_flag = true;
//				}
//			else
//				{
//					FreeKeyValuePair (pair_p);
//				}
//		}
//	else
//		{
//			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate Treatment value for \"%s\": \"s\"", name_s, value_s);
//		}
//
//	return success_flag;
//}
//
//
//bool AddTreatmentValue (Treatment *treatment_p, KeyValuePair *pair_p)
//{
//	bool success_flag = false;
//	KeyValuePairNode *node_p = AllocateKeyValuePairNode (pair_p);
//
//	if (node_p)
//		{
//			LinkedListAddTail (treatment_p -> tr_values_p, & (node_p -> kvpn_node));
//		}
//	else
//		{
//			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate Treatment value for \"%s\": \"s\"", pair_p -> kvp_key_s, pair_p -> kvp_value_s);
//		}
//
//	return success_flag;
//}


//TreatmentNode *AllocateTreatmentNode (Treatment *treatment_p)
//{
//	TreatmentNode *node_p = (TreatmentNode *) AllocMemory (sizeof (TreatmentNode));
//
//	if (node_p)
//		{
//			InitListItem (& (node_p -> tfn_node));
//			node_p -> tfn_treatment_p = treatment_p;
//
//			return node_p;
//		}
//	else
//		{
//			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate TreatmentNode for treatment \"%s\"", treatment_p -> tr_ontology_term_p -> st_name_s);
//		}
//
//	return NULL;
//}
//
//
//void FreeTreatmentNode (ListItem *node_p)
//{
//	TreatmentNode *tr_node_p = (TreatmentNode *) node_p;
//
//	FreeTreatment (tr_node_p -> tfn_treatment_p);
//	FreeMemory (tr_node_p);
//}


bool AddTreatmentToJSON (const Treatment *treatment_p, json_t *root_p)
{
	bool success_flag = false;

	if (AddSchemaTermToJSON (treatment_p -> tr_ontology_term_p, root_p))
		{
			if ((! (treatment_p -> tr_parent_names_ss)) || (AddStringsToJSON (TR_PARENTS_S, treatment_p -> tr_parent_names_ss, treatment_p -> tr_num_parents, root_p)))
				{
					if ((! (treatment_p -> tr_synonyms_ss)) || (AddStringsToJSON (TR_SYNONYMS_S, treatment_p -> tr_synonyms_ss, treatment_p -> tr_num_synonyms, root_p)))
						{
							if (AddCompoundIdToJSON (root_p, treatment_p -> tr_id_p))
								{
									if (AddDatatype (root_p, DFTD_TREATMENT))
										{
											success_flag = true;
										}
								}

						}

				}

		}		/* if (term_json_p) */

	return success_flag;
}



json_t *GetTreatmentAsJSON (const Treatment *treatment_p)
{
	json_t *term_json_p = GetSchemaTermAsJSON (treatment_p -> tr_ontology_term_p);

	if (term_json_p)
		{
			if (AddTreatmentToJSON (treatment_p, term_json_p))
				{
					return term_json_p;
				}

			json_decref (term_json_p);
		}		/* if (term_json_p) */

	return NULL;
}




Treatment *GetTreatmentFromJSON (const json_t *treatment_json_p)
{
	bson_oid_t *id_p = GetNewUnitialisedBSONOid ();

	if (id_p)
		{
			if (GetMongoIdFromJSON (treatment_json_p, id_p))
				{
					SchemaTerm *term_p = GetSchemaTermFromJSON (treatment_json_p);

					if (term_p)
						{
							size_t num_parents = 0;
							size_t num_synonyms = 0;

							char **parents_ss = GetStringsFromJSON (treatment_json_p, TR_PARENTS_S, &num_parents);
							char **synonyms_ss = GetStringsFromJSON (treatment_json_p, TR_SYNONYMS_S, &num_synonyms);
							Treatment *tr_p = AllocateTreatment (term_p, parents_ss, num_parents, false, synonyms_ss, num_synonyms, false, id_p);

							if (tr_p)
								{
									return tr_p;
								}

							if (parents_ss)
								{
									FreeStringArray (parents_ss, num_parents);
								}		/* if (parents_ss) */

							if (synonyms_ss)
								{
									FreeStringArray (synonyms_ss, num_synonyms);
								}		/* if (synonyms_ss) */


							FreeSchemaTerm (term_p);
						}		/* if (term_p) */

				}		/* if (GetMongoIdFromJSON (treatment_json_p, id_p)) */


			FreeBSONOid (id_p);
		}		/* if (id_p) */

	return NULL;
}


TreatmentNode *AllocateTreatmentNode (Treatment *treatment_p, MEM_FLAG treatment_mem)
{
	TreatmentNode *node_p = NULL;

	if ((treatment_mem == MF_SHALLOW_COPY) || (treatment_mem == MF_SHADOW_USE))
		{
			node_p = (TreatmentNode *) 	AllocMemory (sizeof (TreatmentNode));

			if (node_p)
				{
					InitListItem (& (node_p -> tn_node));

					node_p -> tn_treatment_mem = treatment_mem;
					node_p -> tn_treatment_p = treatment_p;
					node_p -> tn_treatment_url_s = treatment_p -> tr_ontology_term_p -> st_url_s;
				}
		}

	return node_p;
}


void FreeTreatmentNode (ListItem *node_p)
{
	TreatmentNode *treatment_node_p = (TreatmentNode *) node_p;

	if ((treatment_node_p -> tn_treatment_mem == MF_SHALLOW_COPY) || (treatment_node_p -> tn_treatment_mem == MF_DEEP_COPY))
		{
			FreeTreatment (treatment_node_p -> tn_treatment_p);
		}

	FreeMemory (treatment_node_p);
}



static bool AddStringsToJSON (const char *key_s, char **values_ss, size_t num_values, json_t *json_p)
{
	json_t *strings_p = json_array ();

	if (strings_p)
		{
			size_t i = 0;
			bool error_flag = false;

			for (i = 0; i < num_values; ++ i, ++ values_ss)
				{
					json_t *value_p = json_string (*values_ss);

					if (value_p)
						{
							if (json_array_append_new (strings_p, value_p) != 0)
								{
									error_flag = true;
								}
						}
					else
						{
							error_flag = true;
						}
				}

			if (!error_flag)
				{
					if (json_object_set_new (json_p, key_s, strings_p) == 0)
						{
							return true;
						}
				}

			json_decref (strings_p);
		}		/* if (strings_p) */


	return false;
}


static char **GetStringsFromJSON (const json_t *treatment_json_p,  const char *key_s, size_t *num_values_p)
{
	json_t *json_p = json_object_get (treatment_json_p, key_s);

	if (json_p)
		{
			if (json_is_array (json_p))
				{
					const size_t size = json_array_size (json_p);
					char **values_ss = (char **) AllocMemoryArray (size, sizeof (char *));

					if (values_ss)
						{
							bool success_flag = true;
							size_t i = 0;
							char **value_ss = values_ss;

							while (success_flag && (i < size))
								{
									const json_t *str_json_p = json_array_get (json_p, i);
									const char *src_s = json_string_value (str_json_p);

									if (src_s)
										{
											*value_ss = EasyCopyToNewString (src_s);

											if (! (*value_ss))
												{
													size_t j = 0;

													for (j = i; j > 0; -- j, -- value_ss)
														{
															FreeCopiedString (*value_ss);
														}

													success_flag = false;
												}
										}

									++ value_ss;
									++ i;
								}

							if (success_flag)
								{
									*num_values_p = size;
									return values_ss;
								}

							FreeMemory (values_ss);
						}		/* if (values_ss) */


				}		/* if (json_is_array (json_p)) */

		}		/* if (json_p) */


	return NULL;
}



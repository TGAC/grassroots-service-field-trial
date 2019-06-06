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
 * crop_ontology_tool.c
 *
 *  Created on: 25 Oct 2018
 *      Author: billy
 */


/*
 * http://www.cropontology.org/get-attributes/CO_321:0000007
 */
/*
[
  {
    "key": "name",
    "value": "{\"english\":\"Heading time\"}"
  },
  {
    "key": "Trait description",
    "value": "{\"english\":\"Heading time extends from the time of emergence of the tip of the spike from the flag leaf sheath to when the spike has completely emerged but has not yet started to flower.\"}"
  },
  {
    "key": "Trait class",
    "value": "{\"english\":\"Phenological traits\"}"
  },
  {
    "key": "created_at",
    "value": "Fri Dec 02 08:20:19 UTC 2016"
  },
  {
    "key": "ontology_id",
    "value": "CO_321"
  },
  {
    "key": "ontology_name",
    "value": "Wheat"
  },
  {
    "key": "Main trait abbreviation",
    "value": "{\"english\":\"Hd\"}"
  },
  {
    "key": "Trait name",
    "value": "{\"english\":\"Heading time\"}"
  },
  {
    "key": "Alternative trait abbreviations",
    "value": "{\"english\":\"Head, Heading, H\"}"
  },
  {
    "key": "ibfieldbook",
    "value": "{\"english\":\"Obsolete/legacy\"}"
  },
  {
    "key": "Attribute",
    "value": "{\"english\":\"Heading time\"}"
  },
  {
    "key": "language",
    "value": "EN"
  },
  {
    "key": "Entity",
    "value": "{\"english\":\"Plant\"}"
  }
]
*/

#include <string.h>

#include "crop_ontology_tool.h"
#include "treatment.h"
#include "curl_tools.h"
#include "jansson.h"
#include "string_utils.h"
#include "streams.h"
#include "schema_term.h"
#include "json_util.h"


/*
 * static declarations
 */

static const char * const S_CROP_ONTOLOGY_API_URL_S = "http://www.cropontology.org/get-attributes/";



static char *GetTermEnglishValue (const json_t *entry_p);

static SchemaTerm *GetSchemaTerm (json_t *values_p, const char *term_s, const char *name_key_s, const char *abbreviation_key_s, const char *description_key_s);

static SchemaTerm *GetCachedCropOnotologySchemaTerm (const char *term_s, MongoTool *tool_p);

static SchemaTerm *FindCachedCropOnotologySchemaTerm (const char *term_s, const char *key_stem_s, MongoTool *tool_p, bson_t *opts_p);



/*
 * API definitions
 */



SchemaTerm *GetCropOnotologySchemaTerm (const char *crop_ontology_term_s, TermType expected_type, TermType *found_type_p, MongoTool *mongo_p)
{
	SchemaTerm *term_p = NULL;
	const size_t TERMS_URL_LENGTH = strlen (CONTEXT_URL_CROP_ONTOLOGY_S);
	const char *term_s = NULL;

	if (strncmp (crop_ontology_term_s, CONTEXT_URL_CROP_ONTOLOGY_S, TERMS_URL_LENGTH) == 0)
		{
			term_s = crop_ontology_term_s + TERMS_URL_LENGTH;
		}
	else
		{
			term_s = crop_ontology_term_s;
		}

	if (mongo_p)
		{
			term_p = GetCachedCropOnotologySchemaTerm (term_s, mongo_p);
		}

	if (!term_p)
		{
			char *url_s = ConcatenateStrings (S_CROP_ONTOLOGY_API_URL_S, term_s);

			if (url_s)
				{
					CurlTool *tool_p = AllocateCurlTool (CM_MEMORY);

					if (tool_p)
						{
							if (SetUriForCurlTool (tool_p, url_s))
								{
									CURLcode c = RunCurlTool (tool_p);

									if (c == CURLE_OK)
										{
											const char *results_s = GetCurlToolData (tool_p);

											if (results_s)
												{
													json_error_t err;
													json_t *res_p = json_loads (results_s, JSON_DECODE_ANY, &err);

													if (res_p)
														{
															if (json_is_array (res_p))
																{
																	const size_t num_results = json_array_size (res_p);
																	size_t i = 0;
																	TermType tt = TT_NUM_TYPES;

																	/* Determine the type of data */
																	while ((i < num_results) && (tt == TT_NUM_TYPES))
																		{
																			json_t *entry_p = json_array_get (res_p, i);
																			const char *key_s = GetJSONString (entry_p, "key");

																			if (key_s)
																				{
																					if (strcmp (key_s, "Method name") == 0)
																						{
																							tt = TT_METHOD;
																						}
																					else if (strcmp (key_s, "Variable name") == 0)
																						{
																							tt = TT_VARIABLE;
																						}
																					else if (strcmp (key_s, "Trait name") == 0)
																						{
																							tt = TT_TRAIT;
																						}
																					else if (strcmp (key_s, "Scale name") == 0)
																						{
																							tt = TT_UNIT;
																						}

																				}		/* if (key_s) */

																			++ i;
																		}		/* while ((i < num_results) && (var_type == TT_NUM_TYPES)) */


																	if (tt != TT_NUM_TYPES)
																		{
																			/*
																			 * Does the term type match what we expected to get?
																			 */
																			if ((expected_type == TT_NUM_TYPES) || (tt == expected_type))
																				{
																					const char *name_key_s = NULL;
																					const char *abbr_key_s = NULL;
																					const char *desc_key_s = NULL;

																					switch (tt)
																						{
																							case TT_TRAIT:
																								name_key_s = "Trait name";
																								abbr_key_s = "Main trait abbreviation";
																								desc_key_s = "Trait description";
																								break;

																							case TT_METHOD:
																								name_key_s = "Method name";
																								desc_key_s = "Method description";
																								break;

																							case TT_UNIT:
																								name_key_s = "Scale name";
																								break;

																							case TT_VARIABLE:
																								name_key_s = "Variable name";
																								break;

																							default:
																								break;
																						}

																					term_p = GetSchemaTerm (res_p, term_s, name_key_s, abbr_key_s, desc_key_s);

																					if (!term_p)
																						{
																							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, res_p, "GetSchemaTerm failed for term \"%s\", name_key \"%s\", abbr_key \"%s\", desc_key \"%s\"", term_s, name_key_s ? name_key_s : "", abbr_key_s ? abbr_key_s : "", desc_key_s ? desc_key_s : "");
																						}
																					else
																						{
																							*found_type_p = tt;
																						}

																				}		/* if (tt == expected_type) */
																			else
																				{
																					PrintJSONToLog (STM_LEVEL_WARNING, __FILE__, __LINE__, res_p, "term is of type %d, expected is %d", tt, expected_type);
																				}



																		}		/* if (tt != TT_NUM_TYPES) */
																	else
																		{
																			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, res_p, "Failed to determine term type");
																		}

																}		/* if (json_is_array (res_p)) */

															json_decref (res_p);
														}		/* if (res_p) */
													else
														{
															PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "failed to load \"%s\" as JSON, err at %d, %d", results_s, err.line, err.column);
														}

												}		/* if (results_s) */
											else
												{
													PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "failed to get data for \"%s\"", url_s);
												}

										}		/* if (c == CURLE_OK) */
									else
										{
											const char *error_s = curl_easy_strerror (c);

											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "failed to run \"%s\": \"%s\"", url_s, error_s);
										}

								}		/* if (SetUriForCurlTool (tool_p, url_s)) */
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "failed to get set curl tool's url to \"%s\"", url_s);
								}

							FreeCurlTool (tool_p);
						}		/* if (tool_p) */
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "failed to allocate curl tool for \"%s\"", url_s);
						}

					FreeCopiedString (url_s);
				}		/* if (url_s) */
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "failed to make url from \"%s\" and \"%s\"", S_CROP_ONTOLOGY_API_URL_S, crop_ontology_term_s);
				}

		}		/* if (!term_p) */


	return term_p;
}


/*
 * static definitions
 */


static char *GetTermEnglishValue (const json_t *entry_p)
{
	char *term_value_s = NULL;
	const char *value_s = GetJSONString (entry_p, "value");

	if (value_s)
		{
			json_error_t err;
			json_t *data_p = json_loads (value_s, 0, &err);

			if (data_p)
				{
					value_s = GetJSONString (data_p, "english");

					if (value_s)
						{
							term_value_s = EasyCopyToNewString (value_s);
						}

					json_decref (data_p);
				}

		}

	return term_value_s;
}


static SchemaTerm *GetSchemaTerm (json_t *values_p, const char *term_s, const char *name_key_s, const char *abbreviation_key_s, const char *description_key_s)
{
	SchemaTerm *term_p = NULL;
	const size_t num_results = json_array_size (values_p);
	size_t i = 0;
	size_t matched_count = 0;
	char *name_s = NULL;
	char *description_s = NULL;
	char *abbreviation_s = NULL;
	size_t num_to_match = 0;

	if (name_key_s)
		{
			++ num_to_match;
		}

	if (abbreviation_key_s)
		{
			++ num_to_match;
		}

	if (description_key_s)
		{
			++ num_to_match;
		}


	while ((i < num_results) && (matched_count < num_to_match))
		{
			json_t *entry_p = json_array_get (values_p, i);
			const char *key_s = GetJSONString (entry_p, "key");

			if (key_s)
				{
					if ((name_key_s != NULL) && (strcmp (key_s, name_key_s) == 0))
						{
							name_s = GetTermEnglishValue (entry_p);
							++ matched_count;
						}
					else if ((description_key_s != NULL) && (strcmp (key_s, description_key_s) == 0))
						{
							description_s = GetTermEnglishValue (entry_p);
							++ matched_count;
						}
					else if ((abbreviation_key_s != NULL) && (strcmp (key_s, abbreviation_key_s) == 0))
						{
							abbreviation_s = GetTermEnglishValue (entry_p);
							++ matched_count;
						}
				}

			++ i;
		}		/* while ((i < num_results) && (matched_count < num_to_match)) */


	if (num_to_match == matched_count)
		{
			term_p = AllocateExtendedSchemaTerm (term_s, name_s, description_s, abbreviation_s);

			if (!term_p)
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "failed to allocate term for \"%s\", \"%s\", \"%s\", \"%s\"", term_s, name_s, description_s, abbreviation_s);
				}

			if (name_s)
				{
					FreeCopiedString (name_s);
				}

			if (description_s)
				{
					FreeCopiedString (description_s);
				}

			if (abbreviation_s)
				{
					FreeCopiedString (abbreviation_s);
				}

		}

	return term_p;
}


static SchemaTerm *FindCachedCropOnotologySchemaTerm (const char *term_s, const char *key_stem_s, MongoTool *tool_p, bson_t *opts_p)
{
	SchemaTerm *term_p = NULL;
	json_t *query_p = json_object ();

	if (query_p)
		{
			char *key_s = ConcatenateVarargsStrings (key_stem_s, ".", SCHEMA_TERM_URL_S, NULL);

			if (key_s)
				{
					if (SetJSONString (query_p, key_s, term_s))
						{
							if (FindMatchingMongoDocumentsByJSON (tool_p, query_p, NULL, opts_p))
								{
									json_t *results_p = GetAllExistingMongoResultsAsJSON (tool_p);

									if (results_p)
										{
											if (json_is_array (results_p))
												{
													if (json_array_size (results_p) > 0)
														{
															json_t *result_p = json_array_get (results_p, 0);
															json_t *term_json_p = json_object_get (result_p, key_stem_s);

															if (term_json_p)
																{
																	term_p = GetSchemaTermFromJSON (term_json_p);

																	if (!term_p)
																		{
																			PrintJSONToErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, term_json_p, "GetSchemaTermFromJSON failed for \"%s\": \"%s\" query", key_s, term_s);
																		}
																}		/* if (term_json_p) */
															else
																{
																	PrintJSONToErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, result_p, "Failed to get \"%s\": from result", key_stem_s);
																}

														}		/* if (json_array_size (results_p) > 0) */

												}		/* 	if (json_is_array (results_p)) */
											else
												{
													PrintJSONToErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, results_p, "results_p is not an array for \"%s\": \"%s\" query", key_s, term_s);
												}

										}		/* if (results_p) */
									else
										{

											PrintJSONToErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, query_p, "GetAllExistingMongoResultsAsJSON failed for query");
										}

								}		/* if (FindMatchingMongoDocumentsByJSON (tool_p, query_p, NULL, opts_p)) */
							else
								{
									PrintJSONToErrors (STM_LEVEL_FINER, __FILE__, __LINE__, query_p, "FindMatchingMongoDocumentsByJSON failed");
								}

						}		/* if (SetJSONString (query_p, key_s, term_s)) */
					else
						{
							PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to set \"%s\": \"%s\" in query JSON", key_s, term_s);
						}

					FreeCopiedString (key_s);
				}

			json_decref (query_p);
		}

	return term_p;
}



static SchemaTerm *GetCachedCropOnotologySchemaTerm (const char *term_s, MongoTool *tool_p)
{
	SchemaTerm *term_p = NULL;

	/* Any hit will do, so limit our results size to 1 */
	bson_t *opts_p = BCON_NEW ("limit", BCON_INT64 (1));

	if (opts_p)
		{
			if ((term_p = FindCachedCropOnotologySchemaTerm (term_s, TR_TRAIT_S, tool_p, opts_p)) == NULL)
				{
					if ((term_p = FindCachedCropOnotologySchemaTerm (term_s, TR_MEASUREMENT_S, tool_p, opts_p)) == NULL)
						{
							if ((term_p = FindCachedCropOnotologySchemaTerm (term_s, TR_UNIT_S, tool_p, opts_p)) == NULL)
								{
									if ((term_p = FindCachedCropOnotologySchemaTerm (term_s, TR_VARIABLE_S, tool_p, opts_p)) == NULL)
										{

										}

								}

						}

				}

			bson_destroy (opts_p);
		}		/* if (opts_p) */

	return term_p;
}


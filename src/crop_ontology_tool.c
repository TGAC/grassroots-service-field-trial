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

static const char *GetTermEnglishValue (const json_t *co_data_p, const char *key_s);

static const char *GetCropOntologyTraitName (const json_t *co_data_p);

static const char *GetCropOntologyTraitDescription (const json_t *co_data_p);

static const char *GetCropOntologyTraitAbbreviation (const json_t *co_data_p);

/*
 * API definitions
 */

SchemaTerm *GetCropOnotologySchemaTerm (const char *crop_ontology_term_s)
{
	SchemaTerm *term_p = NULL;
	char *url_s = NULL;
	const size_t TERMS_URL_LENGTH = strlen (CONTEXT_URL_CROP_ONOTOLOGY_S);
	const char *term_s = NULL;

	if (strncmp (crop_ontology_term_s, CONTEXT_URL_CROP_ONOTOLOGY_S, TERMS_URL_LENGTH) == 0)
		{
			term_s = crop_ontology_term_s + TERMS_URL_LENGTH;
		}
	else
		{
			term_s = crop_ontology_term_s;
		}

	url_s = ConcatenateStrings (S_CROP_ONTOLOGY_API_URL_S, term_s);

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
													const char *trait_name_s = GetCropOntologyTraitName (res_p);

													if (trait_name_s)
														{
															const char *trait_description_s = GetCropOntologyTraitDescription (res_p);

															if (trait_description_s)
																{
																	const char *abbreviation_s = GetCropOntologyTraitAbbreviation (res_p);

																	term_p = AllocateExtendedSchemaTerm (term_s, trait_name_s, trait_description_s, abbreviation_s);

																	if (!term_p)
																		{
																			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, res_p, "Failed to allocate SchemaTerm");
																		}

																}		/* if (trait_description_s) */
															else
																{
																	PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, res_p, "Failed to get trait description");
																}
														}		/* if (trait_name_s) */
													else
														{
															PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, res_p, "Failed to get trait name");
														}

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


	return term_p;
}


/*
 * static definitions
 */


static const char *GetCropOntologyTraitName (const json_t *co_data_p)
{
	return GetTermEnglishValue (co_data_p, "name");
}


static const char *GetCropOntologyTraitDescription (const json_t *co_data_p)
{
	return GetTermEnglishValue (co_data_p, "description");
}


static const char *GetCropOntologyTraitAbbreviation (const json_t *co_data_p)
{
	return GetTermEnglishValue (co_data_p, "Main trait abbreviation");
}


static const char *GetTermEnglishValue (const json_t *co_data_p, const char *key_s)
{
	const char *value_s = NULL;
	const json_t *value_p = json_object_get (co_data_p, key_s);

	if (value_p)
		{
			value_s = GetJSONString (value_p, "english");
		}

	return value_s;
}

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
 * treatment_sanitiser.c
 *
 *  Created on: 6 Jun 2019
 *      Author: billy
 */

#include <stdio.h>

#include "jansson.h"
#include "typedefs.h"
#include "schema_term.h"
#include "/crop_ontology_tool.h"
#include "json_util.h"


int main (int argc, char **argv)
{
	if (argc == 2)
		{
			/*
			 * Read in the json file
			 */
			json_error_t err;
			json_t *treatments_p = json_load_file (argv [1], 0, &err);

			if (treatments_p)
				{
					if (json_is_array (treatments_p))
						{
							const size_t size = json_array_size (treatments_p);
							size_t i;
							size_t processed_count = 0;
							const size_t JSON_OUT_FLAGS = JSON_INDENT (2);
							const char * const UNIT_ID_KEY_S = "Unit ID";
							const char * const UNIT_NAME_KEY_S = "Unit Name";
							const char * const VARIABLE_ID_KEY_S = "Variable ID";
							const char * const VARIABLE_NAME_KEY_S = "Variable Name";
							const char * const COMBINED_ID_KEY_S = "CO scale/variable";
							const char * const COMBINED_NAME_KEY_S = "Scale/variable name";

							/*
							 * 	For each line, get the CO definition for the CO Scale/Variable Id key
							 */

							for (i = 0; i < size; ++ i)
								{
									json_t *row_p = json_array_get (treatments_p, i);
									const char *id_s = GetJSONString (row_p, COMBINED_ID_KEY_S);

									if (id_s)
										{
											TermType found_type = TT_NUM_TYPES;

											/*
											 * Get the term
 											*/
											SchemaTerm *term_p = GetCropOnotologySchemaTerm (id_s, TT_UNIT, &found_type, NULL);

											if (term_p)
												{
													bool success_flag = false;

													switch (found_type)
														{
															/*
															 * If it's a unit, add it as a unit explicitly
															 */
															case TT_UNIT:
																{
																	if (SetJSONString (row_p, UNIT_ID_KEY_S, id_s))
																		{
																			if (SetJSONString (row_p, UNIT_NAME_KEY_S, term_p -> st_name_s))
																				{
																					success_flag = true;
																				}
																			else
																				{
																					printf ("Failed to add \"%s\": \"%s\": ", UNIT_NAME_KEY_S, term_p -> st_name_s);
																					json_dumpf (row_p, stdout, JSON_OUT_FLAGS);
																				}
																		}
																	else
																		{
																			printf ("Failed to add \"%s\": \"%s\": ", UNIT_ID_KEY_S, id_s);
																			json_dumpf (row_p, stdout, JSON_OUT_FLAGS);
																		}
																}
																break;

																/*
																 * If it's a variable, add it as a variable explicitly
																 */
															case TT_VARIABLE:
																{
																	if (SetJSONString (row_p, VARIABLE_ID_KEY_S, id_s))
																		{
																			if (SetJSONString (row_p, VARIABLE_NAME_KEY_S, term_p -> st_name_s))
																				{
																					success_flag = true;
																				}
																			else
																				{
																					printf ("Failed to add \"%s\": \"%s\": ", VARIABLE_NAME_KEY_S, term_p -> st_name_s);
																					json_dumpf (row_p, stdout, JSON_OUT_FLAGS);
																				}
																		}
																	else
																		{
																			printf ("Failed to add \"%s\": \"%s\": ", VARIABLE_ID_KEY_S, id_s);
																			json_dumpf (row_p, stdout, JSON_OUT_FLAGS);
																		}
																}
																break;

																/*
																 * It's not a unit or a variable
																 */
															default:
																printf ("Failed to get type for obj %lu: ", i);
																json_dumpf (row_p, stdout, JSON_OUT_FLAGS);
																break;
														}		/* switch (found_type) */


													if (success_flag)
														{
															/*
															 * Remove the combined data
															 */
															json_object_del (row_p, COMBINED_ID_KEY_S);
															json_object_del (row_p, COMBINED_NAME_KEY_S);

															++ processed_count;
														}

												}		/* if (term_p) */
											else
												{

												}

										}		/* if (id_s) */
									else
										{

										}

								}		/* for (i = 0; i < size; ++ i) */


							printf ("processed %lu out of %lu successfully\n", processed_count, size);

							if (json_dump_file (treatments_p, * (argv + 2), JSON_OUT_FLAGS) == 0)
								{
									printf ("Saved sanitised treatments to \"%s\"\n", * (argv + 2));
								}
							else
								{
									printf ("Failed to save sanitised treatments to \"%s\"\n", * (argv + 2));
								}

						}		/* if (json_is_array (in_p)) */

					json_decref (treatments_p);
				}		/* if (treatments_p) */

		}		/* if (argc == 2) */

	return 0;
}

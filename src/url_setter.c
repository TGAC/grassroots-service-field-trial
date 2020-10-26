/*
 * url_setter.c
 *
 *  Created on: 26 Oct 2020
 *      Author: billy
 */


#include <stdio.h>
#include "jansson.h"


int main (int argc, char *argv [])
{
	int res = 0;

	if (argc == 5)
		{
			json_error_t err;
			json_t *data_package_p = json_load_file (argv [1], 0, &err);

			if (data_package_p)
				{
					json_t *resources_p = json_object_get (data_package_p, "resources");

					if (resources_p)
						{
							if (json_is_array (resources_p))
								{
									FILE *out_f = fopen (argv [4], "w");

									if (out_f)
										{
											const char *base_url_s = argv [2];
											const char *study_id_s = argv [3];
											json_t *resource_p;
											size_t i;

											json_array_foreach (resources_p, i, resource_p)
												{
													json_t *local_path_p = json_object_get (resource_p, "path");

													if (local_path_p)
														{
															if (json_is_string (local_path_p))
																{
																	const char *local_path_s = json_string_value (local_path_p);
																	//db.getCollection('Plots').update({"parent_study_id": ObjectId ("5f88661a02700f47a0101e23"), "study_index": 1},{$set: { "so:url": "testing" }});

																	fprintf (out_f, "db.Plots.update({\"parent_study_id\": ObjectId (\"%s\"), \"rows.study_index\": %lu},{$set: { \"so:url\": \"%s/%s\" }});\n",
																					 study_id_s, i + 1, base_url_s, local_path_s);
																}
														}
												}

											fclose (out_f);
										}

								}
						}

					json_decref (data_package_p);
				}

		}


	return res;
}

/*
 * frictionless_data_util.c
 *
 *  Created on: 22 Mar 2021
 *      Author: billy
 */

#include "typedefs.h"

#define ALLOCATE_FD_UTIL_TAGS (1)
#include "frictionless_data_util.h"
#include "json_util.h"
#include "streams.h"

/*
 * https://specs.frictionlessdata.io/table-schema/#descriptor
 *
 * TODO add constraints
 * */
bool AddTableField (json_t *fields_p, const char *name_s, const char *title_s, const char *type_s, const char *format_s, const char *description_s, const char *rdf_type_s)
{
	json_t *field_p = json_object ();

	if (field_p)
		{
			if ((name_s != NULL) && (SetJSONString (field_p, FD_TABLE_FIELD_NAME, name_s)))
				{
					if (SetNonTrivialString (field_p, FD_TABLE_FIELD_TITLE, title_s, false))
						{
							if (SetNonTrivialString (field_p, FD_TABLE_FIELD_TYPE, type_s, false))
								{
									if (SetNonTrivialString (field_p, FD_TABLE_FIELD_FORMAT, format_s, false))
										{
											if (SetNonTrivialString (field_p, FD_TABLE_FIELD_DESCRIPTION, description_s, false))
												{
													if (SetNonTrivialString (field_p, FD_TABLE_FIELD_RDF_TYPE, rdf_type_s, false))
														{
															if (json_array_append_new (fields_p, field_p) == 0)
																{
																	return true;
																}
															else
																{
																	PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, field_p, "Failed to append field to array");
																}

														}		/* if (SetNonTrivialString (field_p, FD_TABLE_FIELD_RDF_TYPE, rdf_type_s)) */
													else
														{
															PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, field_p, "Failed to add %s: %s", FD_TABLE_FIELD_RDF_TYPE, rdf_type_s ? rdf_type_s : "NULL");
														}

												}		/* if (SetNonTrivialString (field_p, FD_TABLE_FIELD_DESCRIPTION, description_s)) */
											else
												{
													PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, field_p, "Failed to add %s: %s", FD_TABLE_FIELD_TYPE, description_s ? description_s : "NULL");
												}

										}		/* if (SetNonTrivialString (field_p, FD_TABLE_FIELD_FORMAT, format_s)) */
									else
										{
											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, field_p, "Failed to add %s: %s", FD_TABLE_FIELD_FORMAT, format_s ? format_s : "NULL");
										}

								}		/* if (SetNonTrivialString (field_p, FD_TABLE_FIELD_TYPE, type_s)) */
							else
								{
									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, field_p, "Failed to add %s: %s", FD_TABLE_FIELD_TYPE, type_s ? type_s : "NULL");
								}

						}		/* if (SetNonTrivialString (field_p, FD_TABLE_FIELD_TITLE, title_s)) */
					else
						{
							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, field_p, "Failed to add %s: %s", FD_TABLE_FIELD_TITLE, title_s ? title_s : "NULL");
						}

				}		/* if ((name_s != NULL) && (SetJSON (field_p, FD_TABLE_FIELD_NAME, name_s))) */

			json_decref (field_p);
		}		/* if (field_p) */

	return false;
}

/*
 * https://specs.frictionlessdata.io/csv-dialect/#usage
 */
static json_t *GetCSVDialect (const char *delimter_s, const char *line_terminator_s,  const char *comment_char_p, const char *escape_char_p, const char *null_seqeunce_s, const bool has_header_row_flag, const bool case_sensitive_header_flag)
{
	json_t *dialect_p = json_object ();

	if (dialect_p)
		{
			if (SetJSONString (dialect_p, FD_CSV_DIALECT_DELIMITER, delimter_s, false))
				{
					if (SetJSONString (dialect_p, FD_CSV_DIALECT_LINE_TERMINATOR, line_terminator_s, false))
						{
							if (SetNonTrivialString (dialect_p, FD_TABLE_FIELD_DESCRIPTION, description_s, false))
								{
									if (SetNonTrivialString (dialect_p, FD_TABLE_FIELD_RDF_TYPE, rdf_type_s, false))
										{

										}		/* if (SetNonTrivialString (dialect_p, FD_TABLE_FIELD_RDF_TYPE, rdf_type_s)) */
									else
										{
											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, dialect_p, "Failed to add %s: %s", FD_TABLE_FIELD_RDF_TYPE, rdf_type_s ? rdf_type_s : "NULL");
										}

								}		/* if (SetNonTrivialString (dialect_p, FD_TABLE_FIELD_DESCRIPTION, description_s)) */
							else
								{
									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, dialect_p, "Failed to add %s: %s", FD_TABLE_FIELD_TYPE, description_s ? description_s : "NULL");
								}

						}		/* if (SetNonTrivialString (dialect_p, FD_CSV_DIALECT_LINE_TERMINATOR, line_terminator_s)) */
					else
						{
							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, dialect_p, "Failed to add %s: %s", FD_CSV_DIALECT_LINE_TERMINATOR, line_terminator_s ? line_terminator_s : "NULL");
						}

				}		/* if (SetNonTrivialString (dialect_p, FD_CSV_DIALECT_DELIMITER, delimter_s)) */
			else
				{
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, dialect_p, "Failed to add %s: %s", FD_CSV_DIALECT_DELIMITER, delimter_s ? delimter_s : "NULL");
				}


			json_decref (dialect_p);
		}		/* if (dialect_p) */

	return NULL;
}




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
#include "dfw_util.h"

#include "math_utils.h"

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


bool SetFDTableReal (json_t *row_p, const char * const key_s, const double64 *value_p, const char * const null_sequence_s)
{
	bool success_flag = false;

	if (value_p)
		{
			success_flag = SetJSONReal (row_p, key_s, *value_p);
		}
	else
		{
			success_flag = SetJSONString (row_p, key_s, null_sequence_s);
		}

	return success_flag;
}


bool SetFDTableString (json_t *row_p, const char * const key_s, const char * const value_s, const char * const null_sequence_s)
{
	bool success_flag = false;

	if (value_s)
		{
			success_flag = SetJSONString (row_p, key_s,value_s);
		}
	else
		{
			success_flag = SetJSONString (row_p, key_s, null_sequence_s);
		}

	return success_flag;

}


/*
 * https://specs.frictionlessdata.io/csv-dialect/#usage
 */
json_t *GetCSVDialect (const char *delimter_s, const char *line_terminator_s, const char *comment_char_p, const char *escape_char_p, const char *null_seqeunce_s,
											 const bool has_header_row_flag, const bool case_sensitive_header_flag, const bool double_quote_flag, const bool skip_initial_space_flag,
											 const char *quote_p, const uint32 major_version, const uint32 minor_version)
{
	json_t *dialect_p = NULL;

	if (! (quote_p && escape_char_p))
		{
			dialect_p = json_object ();

			if (dialect_p)
				{
					/*
					 * Set up defaults
					 */
					char buffer_s [2];

					if (!delimter_s)
						{
							delimter_s = ",";
						}

					if (!line_terminator_s)
						{
							line_terminator_s = "\r\n";
						}


					* (buffer_s + 1) = '\0';


					if (SetJSONString (dialect_p, FD_CSV_DIALECT_DELIMITER, delimter_s))
						{
							if (SetJSONString (dialect_p, FD_CSV_DIALECT_LINE_TERMINATOR, line_terminator_s))
								{
									if (SetJSONBoolean (dialect_p, FD_CSV_DIALECT_HEADER_ROW, has_header_row_flag))
										{
											if (SetJSONBoolean (dialect_p, FD_CSV_DIALECT_CASE_SENSITIVE_HEADER, case_sensitive_header_flag))
												{
													if ((null_seqeunce_s == NULL) || (SetJSONString (dialect_p, FD_CSV_DIALECT_NULL_VALUE, null_seqeunce_s)))
														{
															if (SetJSONBoolean (dialect_p, FD_CSV_DIALECT_DOUBLE_QUOTE, double_quote_flag))
																{
																	if (SetJSONBoolean (dialect_p, FD_CSV_DIALECT_SKIP_INITIAL_SPACE, skip_initial_space_flag))
																		{
																			bool success_flag =true;

																			if (quote_p)
																				{
																					*buffer_s = *quote_p;

																					success_flag = SetJSONString (dialect_p, FD_CSV_DIALECT_QUOTE_CHAR, buffer_s);
																				}
																			else if (escape_char_p)
																				{
																					*buffer_s = *escape_char_p;

																					success_flag = SetJSONString (dialect_p, FD_CSV_DIALECT_ESCAPE_CHAR, buffer_s);
																				}

																			if (success_flag)
																				{
																					if (comment_char_p)
																						{
																							*buffer_s = *comment_char_p;

																							success_flag = SetJSONString (dialect_p, FD_CSV_DIALECT_COMMENT_CHAR, buffer_s);
																						}

																					if (success_flag)
																						{
																							if ((major_version > 0) || (minor_version > 0))
																								{
																									char *major_s;

																									success_flag = false;

																									major_s = ConvertUnsignedIntegerToString (major_version);

																									if (major_s)
																										{
																											char *minor_s = ConvertUnsignedIntegerToString (minor_version);

																											if (minor_s)
																												{
																													char *version_s = ConcatenateVarargsStrings (major_s, ".", minor_s, NULL);

																													if (version_s)
																														{
																															success_flag = SetJSONString (dialect_p, FD_CSV_DIALECT_VERSION, version_s);

																															FreeCopiedString (version_s);
																														}

																													FreeCopiedString (minor_s);
																												}

																											FreeCopiedString (major_s);
																										}

																								}
																						}

																					if (success_flag)
																						{
																							return dialect_p;
																						}

																				}
																		}
																}
														}
												}
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
					dialect_p = NULL;
				}		/* if (dialect_p) */

		}
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Only one of either quote (%c) or escape (%c) can be set at any time", *quote_p, *escape_char_p);
		}


	return dialect_p;
}




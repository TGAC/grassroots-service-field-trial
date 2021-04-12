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


static json_t *GetOrCreateConstraints (json_t *field_p);


/*
 * https://specs.frictionlessdata.io/table-schema/#descriptor
 *
 * TODO add constraints
 * */
json_t *AddTableField (json_t *fields_p, const char *name_s, const char *title_s, const char *type_s, const char *format_s, const char *description_s, const char *rdf_type_s)
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
																	return field_p;
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

	return NULL;
}


json_t *AddIntegerField (json_t *fields_p, const char *name_s, const char *title_s, const char *format_s, const char *description_s, const char *rdf_type_s, const int *min_value_p)
{
	json_t *field_p = AddTableField (fields_p, name_s, title_s, FD_TYPE_INTEGER, format_s, description_s, rdf_type_s);

	if (field_p)
		{
			if ((min_value_p == NULL) || (SetTableFieldMinimumInteger (field_p, *min_value_p)))
				{
					return field_p;
				}
			else
				{
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, fields_p, "Failed to set %s constraints", name_s);
				}
		}
	else
		{
			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, fields_p, "Failed to add %s field", name_s);
		}

	return NULL;
}


json_t *AddNumberField (json_t *fields_p, const char *name_s, const char *title_s, const char *format_s, const char *description_s, const char *rdf_type_s, const double *min_value_p)
{
	json_t *field_p = AddTableField (fields_p, name_s, title_s, FD_TYPE_NUMBER, format_s, description_s, rdf_type_s);

	if (field_p)
		{
			if ((min_value_p == NULL) || (SetTableFieldMinimumNumber (field_p, *min_value_p)))
				{
					return field_p;
				}
			else
				{
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, fields_p, "Failed to set %s constraints", name_s);
				}
		}
	else
		{
			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, fields_p, "Failed to add %s field", name_s);
		}

	return NULL;
}




bool SetTableFieldRequired (json_t *field_p)
{
	bool success_flag = false;
	json_t *constraints_p = GetOrCreateConstraints (field_p);

	if (constraints_p)
		{
			if (SetJSONBoolean (constraints_p, FD_TABLE_FIELD_CONSTRAINT_REQUIRED, true))
				{
					success_flag = true;
				}
		}

	return success_flag;
}



bool SetTableFieldUnique (json_t *field_p)
{
	bool success_flag = false;
	json_t *constraints_p = GetOrCreateConstraints (field_p);

	if (constraints_p)
		{
			if (SetJSONBoolean (constraints_p, FD_TABLE_FIELD_CONSTRAINT_UNIQUE, true))
				{
					success_flag = true;
				}
		}

	return success_flag;
}



bool SetTableFieldMinimumInteger (json_t *field_p, const json_int_t value)
{
	bool success_flag = false;
	json_t *constraints_p = GetOrCreateConstraints (field_p);

	if (constraints_p)
		{
			if (SetJSONInteger (constraints_p, FD_TABLE_FIELD_CONSTRAINT_MIN, value))
				{
					success_flag = true;
				}
		}

	return success_flag;
}


bool SetTableFieldMinimumNumber (json_t *field_p, const double value)
{
	bool success_flag = false;
	json_t *constraints_p = GetOrCreateConstraints (field_p);

	if (constraints_p)
		{
			if (SetJSONReal (constraints_p, FD_TABLE_FIELD_CONSTRAINT_MIN, value))
				{
					success_flag = true;
				}
		}

	return success_flag;
}


bool SetTableFieldMaximumInteger (json_t *field_p, const json_int_t value)
{
	bool success_flag = false;
	json_t *constraints_p = GetOrCreateConstraints (field_p);

	if (constraints_p)
		{
			if (SetJSONInteger (constraints_p, FD_TABLE_FIELD_CONSTRAINT_MAX, value))
				{
					success_flag = true;
				}
		}

	return success_flag;
}


bool SetTableFieldMaximumNumber (json_t *field_p, const double value)
{
	bool success_flag = false;
	json_t *constraints_p = GetOrCreateConstraints (field_p);

	if (constraints_p)
		{
			if (SetJSONReal (constraints_p, FD_TABLE_FIELD_CONSTRAINT_MAX, value))
				{
					success_flag = true;
				}
		}

	return success_flag;
}


bool SetTableFieldPattern (json_t *field_p, const char * const pattern_s)
{
	bool success_flag = false;
	json_t *constraints_p = GetOrCreateConstraints (field_p);

	if (constraints_p)
		{
			if (SetJSONString (constraints_p, FD_TABLE_FIELD_CONSTRAINT_PATTERN, pattern_s))
				{
					success_flag = true;
				}
		}

	return success_flag;
}


bool SetTableFieldEnum (json_t *field_p, json_t *enum_p)
{
	bool success_flag = false;
	json_t *constraints_p = GetOrCreateConstraints (field_p);

	if (constraints_p)
		{
			if (json_object_set_new (constraints_p, FD_TABLE_FIELD_CONSTRAINT_ENUM, enum_p) == 0)
				{
					success_flag = true;
				}
		}

	return success_flag;
}



json_t *GetDataPackage (const char *name_s, const char *description_s, const bson_oid_t *id_p)
{
	json_t *data_package_p = json_object ();

	if (data_package_p)
		{
			bool success_flag = false;

			if (SetJSONString (data_package_p, FD_NAME_S, name_s))
				{
					char *id_s = GetBSONOidAsString (id_p);

					if (id_s)
						{
							if (SetJSONString (data_package_p, FD_ID_S, id_s))
								{
									if (SetJSONString (data_package_p, FD_DESCRIPTION_S, description_s))
										{
											if (SetJSONString (data_package_p, FD_PROFILE_S, FD_PROFILE_DATA_PACKAGE_S))
												{
													json_t *resources_p = json_array ();

													if (resources_p)
														{
															if (json_object_set_new (data_package_p, FD_RESOURCES_S, resources_p) == 0)
																{
																	success_flag = true;
																}		/* if (json_object_set_new (data_package_p, FD_RESOURCES_S, resources_p) == 0) */
															else
																{
																	PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, data_package_p, "Failed to add resources array");
																}

														}		/* if (resources_p) */
													else
														{
															PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, data_package_p, "Failed to allocate resources array");
														}

												}		/*  if (SetJSONString (data_package_p, FD_PROFILE_S, FD_PROFILE_DATA_PACKAGE_S)) */
											else
												{
													PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, data_package_p, "Failed to set \"%s\": \"%s\"", FD_PROFILE_S, FD_PROFILE_DATA_PACKAGE_S);
												}


										}		/* if (SetJSONString (data_package_p, FD_DESCRIPTION_S, description_s)) */
									else
										{
											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, data_package_p, "Failed to set \"%s\": \"%s\"", FD_DESCRIPTION_S, description_s);
										}

								}		/*  if (SetJSONString (data_package_p, FD_ID_S, id_s)) */
							else
								{
									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, data_package_p, "Failed to set \"%s\": \"%s\"", FD_ID_S, id_s);
								}

							FreeCopiedString (id_s);
						}		/* if (id_s) */
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to copy id to string for \"%s\"", name_s);
						}

				}		/* if (SetJSONString (data_package_p, FD_NAME_S, name_s)) */
			else
				{
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, data_package_p, "Failed to set \"%s\": \"%s\"", FD_NAME_S, name_s);
				}

			if (!success_flag)
				{
					json_decref (data_package_p);
					data_package_p = NULL;
				}

		}		/* if (data_package_p) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate JSON for \"%s\"", name_s);
		}

	return data_package_p;
}



static json_t *GetOrCreateConstraints (json_t *field_p)
{
	json_t *constraints_p = json_object_get (field_p, FD_TABLE_FIELD_CONSTRAINTS);

	if (!constraints_p)
		{
			constraints_p = json_object ();

			if (constraints_p)
				{
					if (json_object_set_new (field_p, FD_TABLE_FIELD_CONSTRAINTS, constraints_p) != 0)
						{
							json_decref (constraints_p);
							constraints_p = NULL;
						}
				}
		}

	return constraints_p;
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




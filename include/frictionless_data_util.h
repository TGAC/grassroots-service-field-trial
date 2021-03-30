/*
 * frictionless_data_util.h
 *
 *  Created on: 22 Mar 2021
 *      Author: billy
 */

#ifndef SERVICES_FIELD_TRIALS_INCLUDE_FRICTIONLESS_DATA_UTIL_H_
#define SERVICES_FIELD_TRIALS_INCLUDE_FRICTIONLESS_DATA_UTIL_H_

#include "dfw_field_trial_service_library.h"

#include "jansson.h"

#ifndef DOXYGEN_SHOULD_SKIP_THIS


#ifdef ALLOCATE_FD_UTIL_TAGS
	#define FD_UTIL_PREFIX DFW_FIELD_TRIAL_SERVICE_API
	#define FD_UTIL_VAL(x)	= x
#else
	#define FD_UTIL_PREFIX extern
	#define FD_UTIL_VAL(x)
#endif

#endif 		/* #ifndef DOXYGEN_SHOULD_SKIP_THIS */


FD_UTIL_PREFIX const char * const FD_PROFILE_S FD_UTIL_VAL ("profile");
FD_UTIL_PREFIX const char * const FD_PROFILE_TABULAR_PACKAGE_S FD_UTIL_VAL ("tabular-data-package");
FD_UTIL_PREFIX const char * const FD_PROFILE_TABULAR_RESOURCE_S FD_UTIL_VAL ("tabular-data-resource");
FD_UTIL_PREFIX const char * const FD_PROFILE_DATA_S FD_UTIL_VAL ("data-package");
FD_UTIL_PREFIX const char * const FD_NAME_S FD_UTIL_VAL ("name");
FD_UTIL_PREFIX const char * const FD_ID_S FD_UTIL_VAL ("id");
FD_UTIL_PREFIX const char * const FD_LICENSES_S FD_UTIL_VAL ("licenses");
FD_UTIL_PREFIX const char * const FD_DESCRIPTION_S FD_UTIL_VAL ("description");
FD_UTIL_PREFIX const char * const FD_URL_S FD_UTIL_VAL ("homepage");
FD_UTIL_PREFIX const char * const FD_RESOURCES_S FD_UTIL_VAL ("resources");


FD_UTIL_PREFIX const char * const FD_SCHEMA_S FD_UTIL_VAL ("schema");



/*
 * Table fields
 */
FD_UTIL_PREFIX const char *FD_TABLE_FIELD_NAME FD_UTIL_VAL ("name");

FD_UTIL_PREFIX const char *FD_TABLE_FIELD_TITLE FD_UTIL_VAL ("title");

FD_UTIL_PREFIX const char *FD_TABLE_FIELD_TYPE FD_UTIL_VAL ("type");

FD_UTIL_PREFIX const char *FD_TABLE_FIELD_FORMAT FD_UTIL_VAL ("format");

FD_UTIL_PREFIX const char *FD_TABLE_FIELD_DESCRIPTION FD_UTIL_VAL ("description");

FD_UTIL_PREFIX const char *FD_TABLE_FIELD_CONSTRAINTS FD_UTIL_VAL ("constraints");

FD_UTIL_PREFIX const char *FD_TABLE_FIELD_RDF_TYPE FD_UTIL_VAL ("rdfType");

FD_UTIL_PREFIX const char *FD_TABLE_FIELD_NULL_VALUE FD_UTIL_VAL ("missingValues");


/*
 * CSV Dialect
 */

FD_UTIL_PREFIX const char *FD_CSV_DIALECT FD_UTIL_VAL ("dialect");

FD_UTIL_PREFIX const char *FD_CSV_DIALECT_DELIMITER FD_UTIL_VAL ("delimiter");

FD_UTIL_PREFIX const char *FD_CSV_DIALECT_LINE_TERMINATOR FD_UTIL_VAL ("lineTerminator");

FD_UTIL_PREFIX const char *FD_CSV_DIALECT_QUOTE_CHAR FD_UTIL_VAL ("type");

FD_UTIL_PREFIX const char *FD_CSV_DIALECT_DOUBLE_QUOTE FD_UTIL_VAL ("format");

FD_UTIL_PREFIX const char *FD_CSV_DIALECT_ESCAPE_CHAR FD_UTIL_VAL ("escapeChar");

FD_UTIL_PREFIX const char *FD_CSV_DIALECT_NULL_VALUE FD_UTIL_VAL ("nullSequence");

FD_UTIL_PREFIX const char *FD_CSV_DIALECT_SKIP_INITIAL_SPACE FD_UTIL_VAL ("skipInitialSpace");

FD_UTIL_PREFIX const char *FD_CSV_DIALECT_HEADER_ROW FD_UTIL_VAL ("header");

FD_UTIL_PREFIX const char *FD_CSV_DIALECT_COMMENT_CHAR FD_UTIL_VAL ("commentChar");

FD_UTIL_PREFIX const char *FD_CSV_DIALECT_CASE_SENSITIVE_HEADER FD_UTIL_VAL ("caseSensitiveHeader");

FD_UTIL_PREFIX const char *FD_CSV_DIALECT_VERSION FD_UTIL_VAL ("csvddfVersion");


/*
 * Types
 */

/** The field contains strings, that is, sequences of characters. */
FD_UTIL_PREFIX const char *FD_TYPE_STRING FD_UTIL_VAL ("string");

/** Any valid string.*/
FD_UTIL_PREFIX const char *FD_TYPE_STRING_FORMAT_DEFAULT FD_UTIL_VAL ("default");

/** A valid email address. */
FD_UTIL_PREFIX const char *FD_TYPE_STRING_FORMAT_EMAIL FD_UTIL_VAL ("email");

/** A valid URI. */
FD_UTIL_PREFIX const char *FD_TYPE_STRING_FORMAT_URI FD_UTIL_VAL ("uri");

/** A base64 encoded string representing binary data. */
FD_UTIL_PREFIX const char *FD_TYPE_STRING_FORMAT_BINARY FD_UTIL_VAL ("binary");

/** A string that is a uuid. */
FD_UTIL_PREFIX const char *FD_TYPE_STRING_FORMAT_UUID FD_UTIL_VAL ("uuid");



/** The field contains numbers of any kind including decimals. */
FD_UTIL_PREFIX const char *FD_TYPE_NUMBER FD_UTIL_VAL ("number");



/**
 * The field contains integers - that is whole numbers.
 *
 * Integer values are indicated in the standard way for any valid integer.
 */
FD_UTIL_PREFIX const char *FD_TYPE_INTEGER FD_UTIL_VAL ("integer");


/** The field contains boolean (true/false) data. */
FD_UTIL_PREFIX const char *FD_TYPE_BOOLEAN FD_UTIL_VAL ("boolean");


/**
 * A date without a time.
 */
FD_UTIL_PREFIX const char *FD_TYPE_DATE FD_UTIL_VAL ("date");


/**
 * A time without a date.
 */
FD_UTIL_PREFIX const char *FD_TYPE_TIME FD_UTIL_VAL ("time");


/**
 * A date with a time.
 */
FD_UTIL_PREFIX const char *FD_TYPE_DATETIME FD_UTIL_VAL ("datetime");


/**
 * The field contains a JSON object according to GeoJSON or TopoJSON spec.
 */
FD_UTIL_PREFIX const char *FD_TYPE_GEOJSON FD_UTIL_VAL ("geojson");



#ifdef __cplusplus
extern "C"
{
#endif

DFW_FIELD_TRIAL_SERVICE_LOCAL bool AddTableField (json_t *fields_p, const char *name_s, const char *title_s, const char *type_s, const char *format_s, const char *description_s, const char *rdf_type_s);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool SetFDTableReal (json_t *row_p, const char * const key_s, const double64 *value_p, const char * const null_sequence_s);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool SetFDTableString (json_t *row_p, const char * const key_s, const char * const value_s, const char * const null_sequence_s);

#ifdef __cplusplus
}
#endif


#endif /* SERVICES_FIELD_TRIALS_INCLUDE_FRICTIONLESS_DATA_UTIL_H_ */

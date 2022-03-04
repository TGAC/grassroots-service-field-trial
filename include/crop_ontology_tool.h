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
 * crop_ontology_tool.h
 *
 *  Created on: 27 Oct 2018
 *      Author: billy
 */

#ifndef SERVICES_FIELD_TRIALS_INCLUDE_CROP_ONTOLOGY_TOOL_H_
#define SERVICES_FIELD_TRIALS_INCLUDE_CROP_ONTOLOGY_TOOL_H_



#include "schema_term.h"
#include "dfw_field_trial_service_library.h"
#include "mongodb_tool.h"
#include "parameter.h"



typedef enum
{
	TT_TRAIT,
	TT_METHOD,
	TT_UNIT,
	TT_VARIABLE,
	TT_NUM_TYPES
} TermType;

/*
	Crop Ontology Scale Class types

1. Date
The date class is for events expressed in a time format, e.g. “yyyymmdd hh:mm:ss –
UTC” or “dd-mm-yy”. A good practice recommended by the Breeding API (BrAPI) is to
use the Date and timestamp fields coded in the ISO 8601 standard, extended format.
Check
https://github.com/plantbreeding/API/blob/master/Specification/GeneralInfo/Date_Time
_Encoding.md)

2. Duration
The duration class is for time elapsed between two events expressed in a time format,
e.g. “days”, “hours”, “months”.

3. Nominal
Categorical scale that can take one of a limited number of categories. There is no
intrinsic ordering to the categories e.g. r=“red”, g=“green”, p=“purple”.

4. Numerical
Numerical scales express the trait with real numbers. The numerical scale defines the
unit e.g. centimetre, ton per hectare, number of branches.

5. Ordinal
Ordinal scales are composed of ordered and fixed number of categories e.g. 1=low,
2=moderate, 3=high

6. Text
A free text is used to express the scale value. Also known as Character variable
(varchar)
e.g. “Preferred when slightly undercooked”.

7. Code
This scale class is exceptionally used to express complex traits. Code is a nominal
scale that combines the expressions of the different traits composing the complex trait.
For example, a disease related code might be expressed by a 2-digit code for intensity
and 2-character code for severity. The first 2 digits are the proportion of plants affected
by a fungus and the 2 characters refer to the severity, e.g. “75HD” means “75% of the
plants are infected and plants are highly damaged”. It is recommended to create
variables for every component of the code.
*/

typedef struct COScaleClass
{
	const char *cosc_name_s;
	ParameterType cosc_type;
} COScaleClass;





typedef struct COTerm
{
	TermType cot_type;

	char *cot_name_s;

	char *cot_url_s;

} COTerm;

typedef struct COTraitTerm
{
	COTerm cott_term;

	char *cott_description_s;

	char *cott_abbreviation_s;

} COTraitTerm;


typedef struct COMethodTerm
{
	COTerm comt_term;

	char *comt_description_s;

} COMethodTerm;


typedef struct COUnitTerm
{
	COTerm cout_term;

	COScaleClass *cout_class_p;


} COUnitTerm;





#ifdef __cplusplus
extern "C"
{
#endif


DFW_FIELD_TRIAL_SERVICE_API SchemaTerm *GetCropOnotologySchemaTerm (const char *crop_ontology_term_s, TermType expected_type, TermType *found_type_p, MongoTool *mongo_p);


DFW_FIELD_TRIAL_SERVICE_API json_t *GetScaleClassAsJSON (const COScaleClass *class_p);



#ifdef __cplusplus
}
#endif



#endif /* SERVICES_FIELD_TRIALS_INCLUDE_CROP_ONTOLOGY_TOOL_H_ */

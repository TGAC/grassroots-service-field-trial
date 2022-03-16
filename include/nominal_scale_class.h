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
 * nominal_scale_class.h
 *
 *  Created on: 28 Apr 2020
 *      Author: billy
 */

#ifndef SERVICES_FIELD_TRIALS_INCLUDE_NOMINAL_SCALE_CLASS_H_
#define SERVICES_FIELD_TRIALS_INCLUDE_NOMINAL_SCALE_CLASS_H_



typedef struct NominalCOScaleClass
{
	ScaleClass cons_base;

	size_t cons_num_categories;
	char **cons_categories_ss;

} NominalCOScaleClass;




#ifdef __cplusplus
extern "C"
{
#endif

NominalCOScaleClass *AllocateNominalCOScaleClass (const json_t *def_p);


#ifdef __cplusplus
}
#endif



#endif /* SERVICES_FIELD_TRIALS_INCLUDE_NOMINAL_SCALE_CLASS_H_ */

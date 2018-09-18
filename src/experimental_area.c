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
 * experimental_area.c
 *
 *  Created on: 18 Sep 2018
 *      Author: billy
 */

#include "experimental_area.h"
#include "memory_allocations.h"





ExperimentalArea *AllocateExperimentalArea ()
{
	ExperimentalArea *area_p = NULL;

	return area_p;
}


void FreeExperimentalArea (ExperimentalArea *area_p)
{

}


json_t *GetExperimentalAreaAsJSON (const ExperimentalArea *area_p, DFWFieldTrialServiceData *data_p)
{
	json_t *area_json_p = json_object ();

	if (json_object_set_new (area_json_p, EA_NAME_S, json_string (area_p -> ea_name_s)) == 0)
		{
			if (json_object_set_new (area_json_p, EA_SOIL_TYPE_S, json_string (area_p -> ea_soil_type_s)) == 0)
				{
					if (json_object_set_new (area_json_p, EA_LOCATION_S, json_string (area_p -> ea_location_s)) == 0)
						{
							if (json_object_set_new (area_json_p, EA_YEAR_S, json_integer (area_p -> ea_year)) == 0)
								{
									if (AddIdToJSON (area_json_p, EA_PARENT_FIELD_TRIAL_S, & (area_p -> ea_parent_p -> ft_id), data_p))
										{

										}

								}

						}

				}

		}

	return area_json_p;
}


LinkedList *GetExperimentalAreaPlots (ExperimentalArea *area_p)
{
	LinkedList *plots_list_p = NULL;

	return plots_list_p;
}



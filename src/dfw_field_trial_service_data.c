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
 * dfw_field_trial_service_data.c
 *
 *  Created on: 18 Sep 2018
 *      Author: billy
 */


#include "dfw_field_trial_service_data.h"
#include "streams.h"


bool SetUnitialisedId (DFWId *id_p, const DFWFieldTrialServiceData *data_p)
{
	bool success_flag = false;

	switch (data_p -> dftsd_backend)
		{
			case DB_MONGO_DB:
				id_p -> di_index = UINT32_MAX;
				success_flag = true;
				break;

			case DB_SQLITE:
				id_p -> di_id_s = NULL;
				success_flag = true;
				break;

			default:
				break;

		}

	return success_flag;
}



bool AddIdToJSON (json_t *json_p, const char * const key_s, DFWId *id_p, DFWFieldTrialServiceData *data_p)
{
	bool success_flag = false;
	json_t *value_p = NULL;

	switch (data_p -> dftsd_backend)
		{
			case DB_MONGO_DB:
				value_p = json_integer (id_p -> di_index);
				if (!value_p)
					{

					}
				break;

			case DB_SQLITE:
				value_p = json_string (id_p -> di_id_s);
				if (!value_p)
					{

					}
				break;

			default:
				break;
		}


	if (value_p)
		{
			if (json_object_set_new (json_p, key_s, value_p) == 0)
				{
					success_flag = true;
				}
			else
				{

				}
		}

	return success_flag;
}



bool GetIdFromJSON (json_t *json_p, const char * const key_s, DFWId *id_p, DFWFieldTrialServiceData *data_p)
{
	bool success_flag = false;

	switch (data_p -> dftsd_backend)
		{
			case DB_MONGO_DB:
				if (GetJSONInteger (json_p, key_s, & (id_p -> di_index)))
					{
						success_flag = true;
					}
				break;

			case DB_SQLITE:
				id_p -> di_id_s = GetJSONString (json_p, key_s);

				if (! (id_p -> di_id_s))
					{
						success_flag = true;
					}
				break;

			default:
				break;
		}


	return success_flag;

}

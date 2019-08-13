/*
** Copyright 2014-2016 The Earlham Institute
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


/**
 * @file
 * @brief
 */
#include <stdio.h>

#include "jansson.h"

#include "typedefs.h"
#include "streams.h"
#include "json_util.h"
#include "byte_buffer.h"


#include "location_jobs.h"
#include "field_trial.h"

typedef enum
{
	IM_LOCATIONS,
	IM_TRIALS,
	IM_NUM_MODES
} ImportMode;



/*
 * STATIC DECLARATIONS
 */

static bool ImportTrials (const json_t *trial_p, const char *grassroots_url_s);

static bool ImportLocations (const json_t *locations_p, const char *grassroots_url_s);

static bool ImportLocation (const json_t *location_p, const char *grassroots_url_s);



/*
 * DEFINITIONS
 */

int main (int argc, char **argv)
{
	int res = 0;
	const char *grassroots_url_s = NULL;
	const char *filename_s = NULL;
	ImportMode im = IM_NUM_MODES;

	GetAddressFromLocationString  (NULL);

	if (im != IM_NUM_MODES)
		{
			json_error_t err;
			json_t *data_p = json_load_file (filename_s, 0, &err);

			if (data_p)
				{
					switch (im)
						{
							case IM_LOCATIONS:
								ImportLocations (data_p, grassroots_url_s);
								break;

							case IM_TRIALS:
								ImportTrials (data_p, grassroots_url_s);
								break;

							default:
								break;
						}

					json_decref (data_p);
				}		/* if (data_p) */

		}		/* if (im != IM_NUM_MODES) */


	return res;
}


/*
 * STATIC DEFINITIONS
 */


static bool ImportTrials (const json_t *trials_p, const char *grassroots_url_s)
{
	bool success_flag = false;

	if (json_is_array (trials_p))
		{
			json_t *locations_cache_p = json_object ();

			if (locations_cache_p)
				{

				}		/* if (locations_cache_p) */

		}
	else
		{

		}

	return success_flag;
}



static bool ImportLocations (const json_t *locations_p, const char *grassroots_url_s)
{
	bool success_flag = false;

	if (json_is_array (locations_p))
		{
			json_t *location_p;
			size_t i;

			json_array_foreach (locations_p, i, location_p)
				{
					if (!ImportLocation (location_p, grassroots_url_s))
						{
						}
				}
		}
	else
		{

		}

	return success_flag;
}


static bool ImportLocation (const json_t *location_p, const char *grassroots_url_s)
{
	bool success_flag = false;
	const char *field_s = GetJSONString (location_p, "Field");

	if (field_s)
		{
			const char *lat_s = GetJSONString (location_p, "Lat");

			if (lat_s)
				{
					const char *long_s = GetJSONString (location_p, "Long");

					if (long_s)
						{
							/*
							 * Build the request
							 */
							ByteBuffer *buffer_p = AllocateByteBuffer (1024);

							if (buffer_p)
								{
									//AppendStringsToByteBuffer (buffer_p, "?", LOCATION_NAME.npt_name_s, "=", field_s, NULL);
									//AppendStringsToByteBuffer (buffer_p, "&", LOCATION_LATITUDE.npt_name_s, "=", lat_s, NULL);
									//AppendStringsToByteBuffer (buffer_p, "&", LOCATION_LONGITUDE.npt_name_s, "=", long_s, NULL);

									/*
									 * https://grassroots.tools/grassroots-test/5/controller/service/DFWFieldTrial%20search%20service?FT%20Keyword%20Search=simon
									 */

									FreeByteBuffer (buffer_p);
								}
						}
					else
						{
							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, location_p, "Failed to get Long");
						}

				}
			else
				{
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, location_p, "Failed to get Lat");
				}

		}
	else
		{
			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, location_p, "Failed to get Field");
		}

	return success_flag;
}

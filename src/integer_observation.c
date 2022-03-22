/*
 * integer_observation.c
 *
 *  Created on: 21 Mar 2022
 *      Author: billy
 */




#include "integer_observation.h"
#include "dfw_util.h"
#include "typedefs.h"


static bool AddIntegerValueToJSON (json_t *json_p, const char *key_s, const int32 *value_p, const char *null_sequence_s);



IntegerObservation *AllocateIntegerObservation (bson_oid_t *id_p, const struct tm *start_date_p, const struct tm *end_date_p, MeasuredVariable *phenotype_p, MEM_FLAG phenotype_mem, const int32 *raw_value_p, const int32 *corrected_value_p,
	const char *growth_stage_s, const char *method_s, Instrument *instrument_p, const ObservationNature nature, const uint32 *index_p)
{
	int32 *copied_raw_value_p = NULL;

	if (CopyValidInteger (raw_value_p, &copied_raw_value_p))
		{
			int32 *copied_corrected_value_p = NULL;

			if (CopyValidInteger (corrected_value_p, &copied_corrected_value_p))
				{
					IntegerObservation *observation_p = (IntegerObservation *) AllocMemory (sizeof (IntegerObservation));

					if (observation_p)
						{
							if (InitObservation (& (observation_p -> io_base_observation), id_p, start_date_p, end_date_p, phenotype_p, phenotype_mem, growth_stage_s, method_s, instrument_p, nature, index_p, OT_SIGNED_INTEGER))
								{
									observation_p -> io_raw_value_p = copied_raw_value_p;
									observation_p -> io_corrected_value_p = copied_corrected_value_p;

									return observation_p;
								}

							FreeIntegerObservation (observation_p);
						}

					if (copied_corrected_value_p)
						{
							FreeMemory (copied_corrected_value_p);
						}
				}

			if (copied_raw_value_p)
				{
					FreeMemory (copied_raw_value_p);
				}
		}

	return NULL;
}



void FreeIntegerObservation (IntegerObservation *observation_p)
{
	ClearObservation (& (observation_p -> io_base_observation));

	if (observation_p -> io_raw_value_p)
		{
			FreeMemory (observation_p -> io_raw_value_p);
		}

	if (observation_p -> io_corrected_value_p)
		{
			FreeMemory (observation_p -> io_corrected_value_p);
		}

	FreeMemory (observation_p);
}



json_t *GetIntegerObservationAsJSON (const IntegerObservation *observation_p, const ViewFormat format)
{
	json_t *obs_json_p = GetObservationAsJSON (& (observation_p -> io_base_observation), format);

	if (obs_json_p)
		{
			if ((! (observation_p -> io_raw_value_p)) || (SetJSONInteger (obs_json_p, OB_RAW_VALUE_S, * (observation_p -> io_raw_value_p))))
				{
					if ((! (observation_p -> io_corrected_value_p)) || (SetJSONInteger (obs_json_p, OB_CORRECTED_VALUE_S, * (observation_p -> io_corrected_value_p))))
						{
							return obs_json_p;
						}
				}

			json_decref (obs_json_p);
		}

	return NULL;
}



IntegerObservation *GetIntegerObservationFromJSON (const json_t *phenotype_json_p, FieldTrialServiceData *data_p)
{
	IntegerObservation *observation_p = (IntegerObservation *) AllocMemory (sizeof (IntegerObservation));

	if (observation_p)
		{

		}
	return NULL;
}





bool AddIntegerObservationRawValueToJSON (const IntegerObservation *obs_p, const char *key_s, json_t *json_p, const char *null_sequence_s)
{
	return AddIntegerValueToJSON (json_p, key_s, obs_p -> io_raw_value_p, null_sequence_s);
}


bool AddIntegerObservationCorrectedValueToJSON (const IntegerObservation *obs_p, const char *key_s, json_t *json_p, const char *null_sequence_s)
{
	return AddIntegerValueToJSON (json_p, key_s, obs_p -> io_corrected_value_p, null_sequence_s);
}


static bool AddIntegerValueToJSON (json_t *json_p, const char *key_s, const int32 *value_p, const char *null_sequence_s)
{
	bool success_flag = false;

	if (value_p)
		{
			if (SetJSONInteger (json_p, key_s, *value_p))
				{
					success_flag = true;
				}
			else
				{
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, json_p, "Failed to set \"%s\": " INT32_FMT " in JSON", key_s, *value_p);
				}
		}
	else
		{
			if (SetJSONNull (json_p, key_s))
				{
					success_flag = true;
				}
			else
				{
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, json_p, "Failed to set \"%s\": null in JSON", key_s);
				}
		}

	return success_flag;
}


/*
 * integer_observation.c
 *
 *  Created on: 21 Mar 2022
 *      Author: billy
 */




#include "integer_observation.h"
#include "dfw_util.h"
#include "math_utils.h"


static bool AddIntegerValueToJSON (json_t *json_p, const char *key_s, const int32 *value_p, const char *null_sequence_s);

static bool SetValueFromString (int32 **store_pp, const char *value_s);

static bool SetValueFromJSON (int32 **store_pp, const json_t *value_p);

static bool SetIntegerObservationValueFromString (Observation *observation_p, ObservationValueType ovt, const char *value_s);

static bool SetIntegerObservationValueFromJSON (Observation *observation_p, ObservationValueType ovt, const json_t *value_p);



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
							memset (observation_p, 0, sizeof (IntegerObservation));

							if (InitObservation (& (observation_p -> io_base_observation), id_p, start_date_p, end_date_p, phenotype_p, phenotype_mem, growth_stage_s, method_s, instrument_p, nature, index_p, OT_INTEGER,
																	 ClearIntegerObservation, AddIntegerObservationValuesToJSON, SetIntegerObservationValueFromJSON, SetIntegerObservationValueFromString))
								{
									observation_p -> io_raw_value_p = copied_raw_value_p;
									observation_p -> io_corrected_value_p = copied_corrected_value_p;

									return observation_p;
								}

							ClearObservation (& (observation_p -> io_base_observation));
							ClearIntegerObservation (& (observation_p -> io_base_observation));
							FreeMemory (observation_p);
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



void ClearIntegerObservation (Observation *observation_p)
{
	IntegerObservation *int_obs_p = (IntegerObservation *) observation_p;

	if (int_obs_p -> io_raw_value_p)
		{
			FreeMemory (int_obs_p -> io_raw_value_p);
		}

	if (int_obs_p -> io_corrected_value_p)
		{
			FreeMemory (int_obs_p -> io_corrected_value_p);
		}
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



bool AddIntegerObservationValuesToJSON (const Observation *obs_p, const char *raw_key_s, const char *corrected_key_s, json_t *json_p, const char *null_sequence_s, bool only_if_exists_flag)
{
	bool success_flag = false;
	IntegerObservation *int_obs_p = (IntegerObservation *) obs_p;

	if (AddIntegerObservationRawValueToJSON (int_obs_p, raw_key_s, json_p, null_sequence_s, only_if_exists_flag))
		{
			if (AddIntegerObservationCorrectedValueToJSON (int_obs_p, corrected_key_s, json_p, null_sequence_s, only_if_exists_flag))
				{
					success_flag = true;
				}
		}

	return success_flag;
}


IntegerObservation *GetIntegerObservationFromJSON (const json_t *phenotype_json_p, FieldTrialServiceData *data_p)
{
	IntegerObservation *observation_p = (IntegerObservation *) AllocMemory (sizeof (IntegerObservation));

	if (observation_p)
		{

		}

	return NULL;
}



bool AddIntegerObservationRawValueToJSON (const IntegerObservation *obs_p, const char *key_s, json_t *json_p, const char *null_sequence_s, bool only_if_exists_flag)
{
	bool success_flag = false;

	if (only_if_exists_flag)
		{
			if ((! (obs_p -> io_raw_value_p)) || (AddIntegerValueToJSON (json_p, key_s, obs_p -> io_raw_value_p, null_sequence_s)))
				{
					success_flag = true;
				}
		}
	else
		{
			success_flag = AddIntegerValueToJSON (json_p, key_s, obs_p -> io_raw_value_p, null_sequence_s);
		}

	return success_flag;
}


bool AddIntegerObservationCorrectedValueToJSON (const IntegerObservation *obs_p, const char *key_s, json_t *json_p, const char *null_sequence_s, bool only_if_exists_flag)
{
	bool success_flag = false;

	if (only_if_exists_flag)
		{
			if ((! (obs_p -> io_corrected_value_p)) || (AddIntegerValueToJSON (json_p, key_s, obs_p -> io_corrected_value_p, null_sequence_s)))
				{
					success_flag = true;
				}
		}
	else
		{
			success_flag = AddIntegerValueToJSON (json_p, key_s, obs_p -> io_corrected_value_p, null_sequence_s);
		}

	return success_flag;
}


bool SetIntegerObservationRawValueFromString (IntegerObservation *observation_p, const char *value_s)
{
	return SetValueFromString (& (observation_p -> io_raw_value_p), value_s);
}


bool SetIntegerObservationCorrectedValueFromString (IntegerObservation *observation_p, const char *value_s)
{
	return SetValueFromString (& (observation_p -> io_corrected_value_p), value_s);
}


bool SetIntegerObservationRawValueFromJSON (IntegerObservation *observation_p, const json_t *value_p)
{
	return SetValueFromJSON (& (observation_p -> io_raw_value_p), value_p);
}


bool SetIntegerObservationCorrectedValueFromJSON (IntegerObservation *observation_p, const json_t *value_p)
{
	return SetValueFromJSON (& (observation_p -> io_raw_value_p), value_p);
}

/*
 * STATIC DEFINITIONS
 */



static bool SetIntegerObservationValueFromString (Observation *observation_p, ObservationValueType ovt, const char *value_s)
{
	bool success_flag = false;
	IntegerObservation *int_obs_p = (IntegerObservation *) observation_p;

	switch (ovt)
		{
			case OVT_RAW_VALUE:
				success_flag = SetIntegerObservationRawValueFromString (int_obs_p, value_s);
				break;

			case OVT_CORRECTED_VALUE:
				success_flag = SetIntegerObservationCorrectedValueFromString (int_obs_p, value_s);
				break;

			default:
				break;
		}

	return success_flag;
}


static bool SetIntegerObservationValueFromJSON (Observation *observation_p, ObservationValueType ovt, const json_t *value_p)
{
	bool success_flag = false;
	IntegerObservation *int_obs_p = (IntegerObservation *) observation_p;

	switch (ovt)
		{
			case OVT_RAW_VALUE:
				success_flag = SetIntegerObservationRawValueFromJSON (int_obs_p, value_p);
				break;

			case OVT_CORRECTED_VALUE:
				success_flag = SetIntegerObservationCorrectedValueFromJSON (int_obs_p, value_p);
				break;

			default:
				break;
		}

	return success_flag;
}


static bool SetValueFromString (int32 **store_pp, const char *value_s)
{
	bool success_flag = false;

	if (!IsStringEmpty (value_s))
		{
			int32 i;

			if (GetValidInteger (&value_s, &i))
				{
					if (! (*store_pp))
						{
							int32 *i_p = (int32 *) AllocMemory (sizeof (int32));

							if (i_p)
								{
									*store_pp = i_p;
								}
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate memory for storing raw value " INT32_FMT " from \"%s\"", i, value_s);
									return false;
								}
						}

					**store_pp = i;
					success_flag = true;
				}

		}
	else
		{
			if (*store_pp)
				{
					FreeMemory (*store_pp);
					*store_pp = NULL;
				}

			success_flag = true;
		}

	return success_flag;
}


static bool SetValueFromJSON (int32 **store_pp, const json_t *value_p)
{
	bool success_flag = false;

	if (json_is_integer (value_p))
		{
			int32 i = json_integer_value (value_p);

			if (*store_pp)
				{
					**store_pp = i;
					success_flag = true;
				}
			else
				{
					int32 *i_p = (int32 *) AllocMemory (sizeof (int32));

					if (i_p)
						{
							*i_p = i;
							*store_pp = i_p;
							success_flag = true;
						}
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate memory for storing value " INT32_FMT, i);
						}
				}
		}
	else
		{
			if (*store_pp)
				{
					FreeMemory (*store_pp);
					*store_pp = NULL;
				}

			success_flag = true;
		}

	return success_flag;
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


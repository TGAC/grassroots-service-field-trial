/*
 * time_observation.c
 *
 *  Created on: 7 Apr 2022
 *      Author: billy
 */


#include "time_observation.h"
#include "dfw_util.h"
#include "math_utils.h"
#include "time_util.h"


static bool AddTimeValueToJSON (json_t *json_p, const char *key_s, const struct tm *value_p, const char *null_sequence_s);

static bool SetValueFromString (struct tm **store_pp, const char *value_s);

static bool SetValueFromJSON (struct tm **store_pp, const json_t *value_p);

static bool CopyValidTime (const struct tm *src_p, struct tm **dest_p);

static bool SetTimeObservationValueFromString (Observation *observation_p, ObservationValueType ovt, const char *value_s);

static bool SetTimeObservationValueFromJSON (Observation *observation_p, ObservationValueType ovt, const json_t *value_p);

static bool SetJSONTime (json_t *json_p, const char *key_s, const struct tm *time_p);


static bool GetValueAsString (const struct tm *value_p, char **value_ss, bool *free_flag_p);

static bool GetTimeObservationValueAsString (const struct Observation *observation_p, ObservationValueType ovt, char **value_ss, bool *free_value_flag_p);



TimeObservation *AllocateTimeObservation (bson_oid_t *id_p, ObservationMetadata *metadata_p, MeasuredVariable *phenotype_p, MEM_FLAG phenotype_mem, const struct tm *raw_value_p, const struct tm *corrected_value_p,
	const char *growth_stage_s, const char *method_s, Instrument *instrument_p, const ObservationNature nature, const char *notes_s)
{
	struct tm *copied_raw_value_p = NULL;

	if (CopyValidTime (raw_value_p, &copied_raw_value_p))
		{
			struct tm *copied_corrected_value_p = NULL;

			if (CopyValidTime (corrected_value_p, &copied_corrected_value_p))
				{
					TimeObservation *observation_p = (TimeObservation *) AllocMemory (sizeof (TimeObservation));

					if (observation_p)
						{
							if (InitObservation (& (observation_p -> to_base_observation), id_p, metadata_p, phenotype_p, phenotype_mem, growth_stage_s, method_s, instrument_p, nature, notes_s,
																	 OT_TIME,
									ClearTimeObservation,
									AddTimeObservationValuesToJSON,
									SetTimeObservationValueFromJSON,
									SetTimeObservationValueFromString,
									GetTimeObservationValueAsString))
								{
									observation_p -> to_raw_value_p = copied_raw_value_p;
									observation_p -> to_corrected_value_p = copied_corrected_value_p;

									return observation_p;
								}

							ClearObservation (& (observation_p -> to_base_observation));
							ClearTimeObservation (& (observation_p -> to_base_observation));
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



void ClearTimeObservation (Observation *observation_p)
{
	TimeObservation *time_obs_p = (TimeObservation *) observation_p;

	if (time_obs_p -> to_raw_value_p)
		{
			FreeTime (time_obs_p -> to_raw_value_p);
			time_obs_p -> to_raw_value_p = NULL;
		}

	if (time_obs_p -> to_corrected_value_p)
		{
			FreeTime (time_obs_p -> to_corrected_value_p);
			time_obs_p -> to_corrected_value_p = NULL;
		}
}


json_t *GetTimeObservationAsJSON (const TimeObservation *observation_p, const ViewFormat format, const FieldTrialServiceData *data_p)
{
	json_t *obs_json_p = GetObservationAsJSON (& (observation_p -> to_base_observation), format, data_p);

	if (obs_json_p)
		{
			if ((! (observation_p -> to_raw_value_p)) || (SetJSONTime (obs_json_p, OB_RAW_VALUE_S, observation_p -> to_raw_value_p)))
				{
					if ((! (observation_p -> to_corrected_value_p)) || (SetJSONTime (obs_json_p, OB_CORRECTED_VALUE_S, observation_p -> to_corrected_value_p)))
						{
							return obs_json_p;
						}
				}

			json_decref (obs_json_p);
		}

	return NULL;
}


bool AddTimeObservationValuesToJSON (const Observation *obs_p, const char *raw_key_s, const char *corrected_key_s, json_t *json_p, const char *null_sequence_s, bool only_if_exists_flag)
{
	bool success_flag = false;
	TimeObservation *time_obs_p = (TimeObservation *) obs_p;

	if (AddTimeObservationRawValueToJSON (time_obs_p, raw_key_s, json_p, null_sequence_s, only_if_exists_flag))
		{
			if (AddTimeObservationCorrectedValueToJSON (time_obs_p, corrected_key_s, json_p, null_sequence_s, only_if_exists_flag))
				{
					success_flag = true;
				}
		}

	return success_flag;
}


TimeObservation *GetTimeObservationFromJSON (const json_t *phenotype_json_p, FieldTrialServiceData *data_p)
{
	TimeObservation *observation_p = (TimeObservation *) AllocMemory (sizeof (TimeObservation));

	if (observation_p)
		{

		}

	return NULL;
}



bool AddTimeObservationRawValueToJSON (const TimeObservation *obs_p, const char *key_s, json_t *json_p, const char *null_sequence_s, bool only_if_exists_flag)
{
	bool success_flag = false;

	if (only_if_exists_flag)
		{
			if ((! (obs_p -> to_raw_value_p)) || (AddTimeValueToJSON (json_p, key_s, obs_p -> to_raw_value_p, null_sequence_s)))
				{
					success_flag = true;
				}
		}
	else
		{
			success_flag = AddTimeValueToJSON (json_p, key_s, obs_p -> to_raw_value_p, null_sequence_s);
		}

	return success_flag;
}


bool AddTimeObservationCorrectedValueToJSON (const TimeObservation *obs_p, const char *key_s, json_t *json_p, const char *null_sequence_s, bool only_if_exists_flag)
{
	bool success_flag = false;

	if (only_if_exists_flag)
		{
			if ((! (obs_p -> to_corrected_value_p)) || (AddTimeValueToJSON (json_p, key_s, obs_p -> to_corrected_value_p, null_sequence_s)))
				{
					success_flag = true;
				}
		}
	else
		{
			success_flag = AddTimeValueToJSON (json_p, key_s, obs_p -> to_corrected_value_p, null_sequence_s);
		}

	return success_flag;
}


bool SetTimeObservationRawValueFromString (TimeObservation *observation_p, const char *value_s)
{
	return SetValueFromString (& (observation_p -> to_raw_value_p), value_s);
}


bool SetTimeObservationCorrectedValueFromString (TimeObservation *observation_p, const char *value_s)
{
	return SetValueFromString (& (observation_p -> to_corrected_value_p), value_s);
}


bool SetTimeObservationRawValueFromJSON (TimeObservation *observation_p, const json_t *value_p)
{
	return SetValueFromJSON (& (observation_p -> to_raw_value_p), value_p);
}


bool SetTimeObservationCorrectedValueFromJSON (TimeObservation *observation_p, const json_t *value_p)
{
	return SetValueFromJSON (& (observation_p -> to_corrected_value_p), value_p);
}


bool GetTimeObservationRawValueAsString (const TimeObservation *observation_p, char **value_ss, bool *free_flag_p)
{
	return GetValueAsString (observation_p -> to_raw_value_p, value_ss, free_flag_p);
}


bool GetTimeObservationCorrectedValueAsString (const TimeObservation *observation_p, char **value_ss, bool *free_flag_p)
{
	return GetValueAsString (observation_p -> to_corrected_value_p, value_ss, free_flag_p);
}


/*
 * STATIC DEFINITIONS
 */



static bool SetTimeObservationValueFromString (Observation *observation_p, ObservationValueType ovt, const char *value_s)
{
	bool success_flag = false;
	TimeObservation *time_obs_p = (TimeObservation *) observation_p;

	switch (ovt)
		{
			case OVT_RAW_VALUE:
				success_flag = SetTimeObservationRawValueFromString (time_obs_p, value_s);
				break;

			case OVT_CORRECTED_VALUE:
				success_flag = SetTimeObservationCorrectedValueFromString (time_obs_p, value_s);
				break;

			default:
				break;
		}

	return success_flag;
}


static bool SetTimeObservationValueFromJSON (Observation *observation_p, ObservationValueType ovt, const json_t *value_p)
{
	bool success_flag = false;
	TimeObservation *time_obs_p = (TimeObservation *) observation_p;

	switch (ovt)
		{
			case OVT_RAW_VALUE:
				success_flag = SetTimeObservationRawValueFromJSON (time_obs_p, value_p);
				break;

			case OVT_CORRECTED_VALUE:
				success_flag = SetTimeObservationCorrectedValueFromJSON (time_obs_p, value_p);
				break;

			default:
				break;
		}

	return success_flag;
}


static bool GetTimeObservationValueAsString (const struct Observation *observation_p, ObservationValueType ovt, char **value_ss, bool *free_value_flag_p)
{
	bool success_flag = false;
	const TimeObservation *time_obs_p = (const TimeObservation *) observation_p;

	switch (ovt)
		{
			case OVT_RAW_VALUE:
				success_flag = GetTimeObservationRawValueAsString (time_obs_p, value_ss, free_value_flag_p);
				break;

			case OVT_CORRECTED_VALUE:
				success_flag = GetTimeObservationCorrectedValueAsString (time_obs_p, value_ss, free_value_flag_p);
				break;

			default:
				break;
		}

	return success_flag;
}


static bool SetValueFromString (struct tm **store_pp, const char *value_s)
{
	bool success_flag = false;

	if (!IsStringEmpty (value_s))
		{
			if (*store_pp)
				{
					if (SetTimeFromString (*store_pp, value_s))
						{
							success_flag = true;
						}
				}
			else
				{
					struct tm *time_p = GetTimeFromString (value_s);

					if (time_p)
						{
							*store_pp = time_p;
							success_flag = true;
						}
				}
		}
	else
		{
			if (*store_pp)
				{
					FreeTime (*store_pp);
					*store_pp = NULL;
				}

			success_flag = true;
		}

	return success_flag;
}


static bool SetValueFromJSON (struct tm **store_pp, const json_t *value_p)
{
	bool success_flag = false;

	if ((!value_p) || (json_is_null (value_p)))
		{
			if (*store_pp)
				{
					FreeTime (*store_pp);
					*store_pp = NULL;
				}

			success_flag = true;
		}
	else if (json_is_string (value_p))
		{
			const char *value_s = json_string_value (value_p);

			success_flag = SetValueFromString (store_pp, value_s);
		}

	return success_flag;
}


static bool CopyValidTime (const struct tm *src_p, struct tm **dest_pp)
{
	bool success_flag = false;

	if (src_p)
		{
			struct tm *dest_p = DuplicateTime (src_p);

			if (dest_p)
				{
					*dest_pp = dest_p;
					success_flag = true;
				}
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to copy time");
				}
		}
	else
		{
			success_flag = true;
		}

	return success_flag;
}


static bool AddTimeValueToJSON (json_t *json_p, const char *key_s, const struct tm *time_p, const char *null_sequence_s)
{
	bool success_flag = false;

	if (time_p)
		{
			char *time_s = GetTimeAsString (time_p, true, NULL);

			if (time_s)
				{
					if (SetJSONString (json_p, key_s, time_s))
						{
							success_flag = true;
						}
					else
						{
							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, json_p, "Failed to set \"%s\": \"%s\" in JSON", key_s, *time_s);
						}

					FreeTimeString (time_s);
				}
			else
				{
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, json_p, "Failed to get \"%s\" value as string", key_s);
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


static bool SetJSONTime (json_t *json_p, const char *key_s, const struct tm *time_p)
{
	bool success_flag = false;
	char *time_s = GetTimeAsString (time_p, true, NULL);

	if (time_s)
		{
			if (SetJSONString (json_p, key_s, time_s))
				{
					success_flag = true;
				}
			else
				{
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, json_p, "SetJSONString () failed for \"%s\": \"%s\"", key_s, time_s);
				}

			FreeCopiedString (time_s);
		}
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "GetTimeAsString () failed");
		}

	return success_flag;
}


static bool GetValueAsString (const struct tm *value_p, char **value_ss, bool *free_flag_p)
{
	bool success_flag = false;

	if (value_p)
		{
			char *value_s = GetTimeAsString (value_p, true, NULL);

			if (value_s)
				{
					*value_ss = value_s;
					*free_flag_p = true;
					success_flag = true;
				}
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "GetTimeAsString () failed ");
				}

		}
	else
		{
			*value_ss = NULL;
			*free_flag_p = false;
			success_flag = true;
		}

	return success_flag;
}




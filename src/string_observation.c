/*
 * string_observation.c
 *
 *  Created on: 17 Mar 2022
 *      Author: billy
 */


#include "string_observation.h"


static bool AddStringValueToJSON (json_t *json_p, const char *key_s, const char *value_s, const char *null_sequence_s);

static bool SetValueFromJSON (char **store_ss, const json_t *value_p);


static bool SetStringObservationValueFromString (Observation *observation_p, ObservationValueType ovt, const char *value_s);

static bool SetStringObservationValueFromJSON (Observation *observation_p, ObservationValueType ovt, const json_t *value_p);

static bool GetStringObservationValueAsString (const struct Observation *observation_p, ObservationValueType ovt, char **value_ss, bool *free_value_flag_p);


StringObservation *AllocateStringObservation (bson_oid_t *id_p, ObservationMetadata *metadata_p, MeasuredVariable *phenotype_p, MEM_FLAG phenotype_mem,
																							const char * const raw_value_s, const char * const corrected_value_s, const char *growth_stage_s,
																							const char *method_s, Instrument *instrument_p, const ObservationNature nature, const char *notes_s)
{
	char *copied_raw_value_s = NULL;

	if ((!raw_value_s) || ((copied_raw_value_s = EasyCopyToNewString (raw_value_s)) != NULL))
		{
			char *copied_corrected_value_s = NULL;

			if ((!corrected_value_s) || ((copied_corrected_value_s = EasyCopyToNewString (corrected_value_s)) != NULL))
				{
					StringObservation *observation_p = (StringObservation *) AllocMemory (sizeof (StringObservation));

					if (observation_p)
						{
							memset (observation_p, 0, sizeof (StringObservation));

							if (InitObservation (& (observation_p -> so_base_observation), id_p, metadata_p, phenotype_p, phenotype_mem, growth_stage_s, method_s, instrument_p, nature, notes_s,
																	 OT_STRING,
																	 ClearStringObservation, AddStringObservationValuesToJSON, SetStringObservationValueFromJSON,
																	 SetStringObservationValueFromString, GetStringObservationValueAsString))
								{
									observation_p -> so_raw_value_s = copied_raw_value_s;
									observation_p -> so_corrected_value_s = copied_corrected_value_s;

									return observation_p;
								}

							ClearObservation (& (observation_p -> so_base_observation));
							ClearStringObservation (& (observation_p -> so_base_observation));
							FreeMemory (observation_p);
						}

					if (copied_corrected_value_s)
						{
							FreeCopiedString (copied_corrected_value_s);
						}
				}

			if (copied_raw_value_s)
				{
					FreeCopiedString (copied_raw_value_s);
				}
		}

	return NULL;
}



void ClearStringObservation (Observation *observation_p)
{
	StringObservation *string_obs_p = (StringObservation *) observation_p;

	if (string_obs_p -> so_raw_value_s)
		{
			FreeCopiedString (string_obs_p -> so_raw_value_s);
			string_obs_p -> so_raw_value_s = NULL;
		}

	if (string_obs_p -> so_corrected_value_s)
		{
			FreeCopiedString (string_obs_p -> so_corrected_value_s);
			string_obs_p -> so_corrected_value_s = NULL;
		}
}



bool AddStringObservationValuesToJSON (const Observation *obs_p, const char *raw_key_s, const char *corrected_key_s, json_t *json_p, const char *null_sequence_s, bool only_if_exists_flag)
{
	bool success_flag = false;
	StringObservation *string_obs_p = (StringObservation *) obs_p;

	if (AddStringObservationRawValueToJSON (string_obs_p, raw_key_s, json_p, null_sequence_s, only_if_exists_flag))
		{
			if (AddStringObservationCorrectedValueToJSON (string_obs_p, corrected_key_s, json_p, null_sequence_s, only_if_exists_flag))
				{
					success_flag = true;
				}
		}

	return success_flag;
}




json_t *GetStringObservationAsJSON (const StringObservation *observation_p, const ViewFormat format, const FieldTrialServiceData *data_p)
{
	json_t *observation_json_p = GetObservationAsJSON (& (observation_p -> so_base_observation), format, data_p);

	if (observation_json_p)
		{
			if ((IsStringEmpty (observation_p -> so_raw_value_s)) || (SetJSONString (observation_json_p, OB_RAW_VALUE_S, observation_p -> so_raw_value_s)))
				{
					if ((IsStringEmpty (observation_p -> so_corrected_value_s)) || (SetJSONString (observation_json_p, OB_CORRECTED_VALUE_S, observation_p -> so_corrected_value_s)))
						{
							return observation_json_p;
						}		/* if ((IsStringEmpty (observation_p -> ob_corrected_value_s)) || (SetJSONString (observation_json_p, OB_CORRECTED_VALUE_S, observation_p -> ob_corrected_value_s))) */
					else
						{
							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, observation_json_p, "Failed to set \"%s\": \"%d\" to JSON", OB_CORRECTED_VALUE_S, observation_p -> so_corrected_value_s);
						}

				}		/* if (SetJSONString (observation_json_p, OB_VALUE_S, observation_p -> ob_measured_value_s)) */
			else
				{
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, observation_json_p, "Failed to add \"%s\": \"%s\" to JSON", OB_RAW_VALUE_S, observation_p -> so_raw_value_s);
				}

			json_decref (observation_json_p);
		}

	return NULL;
}



StringObservation *GetStringObservationFromJSON (const json_t *phenotype_json_p, FieldTrialServiceData *data_p)
{
	StringObservation *observation_p = (StringObservation *) AllocMemory (sizeof (StringObservation));

	if (observation_p)
		{

		}
	return NULL;
}


static bool SetStringObservationValueFromString (Observation *observation_p, ObservationValueType ovt, const char *value_s)
{
	bool success_flag = false;
	StringObservation *string_obs_p = (StringObservation *) observation_p;

	switch (ovt)
		{
			case OVT_RAW_VALUE:
				success_flag = SetStringObservationRawValue (string_obs_p, value_s);
				break;

			case OVT_CORRECTED_VALUE:
				success_flag = SetStringObservationCorrectedValue (string_obs_p, value_s);
				break;

			default:
				break;
		}

	return success_flag;
}


static bool GetStringObservationValueAsString (const struct Observation *observation_p, ObservationValueType ovt, char **value_ss, bool *free_value_flag_p)
{
	bool success_flag = false;
	const StringObservation *string_obs_p = (const StringObservation *) observation_p;

	switch (ovt)
		{
			case OVT_RAW_VALUE:
				*value_ss = string_obs_p -> so_raw_value_s;
				*free_value_flag_p = false;
				success_flag = true;
				break;

			case OVT_CORRECTED_VALUE:
				*value_ss = string_obs_p -> so_corrected_value_s;
				*free_value_flag_p = false;
				success_flag = true;
				break;

			default:
				break;
		}

	return success_flag;

}


static bool SetStringObservationValueFromJSON (Observation *observation_p, ObservationValueType ovt, const json_t *value_p)
{
	bool success_flag = false;
	StringObservation *string_obs_p = (StringObservation *) observation_p;

	switch (ovt)
		{
			case OVT_RAW_VALUE:
				success_flag = SetStringObservationRawValueFromJSON (string_obs_p, value_p);
				break;

			case OVT_CORRECTED_VALUE:
				success_flag = SetStringObservationCorrectedValueFromJSON (string_obs_p, value_p);
				break;

			default:
				break;
		}

	return success_flag;
}


bool SetStringObservationRawValue (StringObservation *observation_p, const char *value_s)
{
	bool success_flag = true;

	if (value_s)
		{
			success_flag = ReplaceStringValue (& (observation_p -> so_raw_value_s), value_s);
		}
	else
		{
			if (observation_p -> so_raw_value_s)
				{
					FreeCopiedString (observation_p -> so_raw_value_s);
					observation_p -> so_raw_value_s = NULL;
				}
		}

	return success_flag;
}


bool SetStringObservationCorrectedValue (StringObservation *observation_p, const char *value_s)
{
	bool success_flag = true;

	if (value_s)
		{
			success_flag = ReplaceStringValue (& (observation_p -> so_corrected_value_s), value_s);
		}
	else
		{
			if (observation_p -> so_corrected_value_s)
				{
					FreeCopiedString (observation_p -> so_corrected_value_s);
					observation_p -> so_corrected_value_s = NULL;
				}
		}

	return success_flag;
}




bool SetStringObservationRawValueFromJSON (StringObservation *observation_p, const json_t *value_p)
{
	return SetValueFromJSON (& (observation_p -> so_raw_value_s), value_p);
}


bool SetStringObservationCorrectedValueFromJSON (StringObservation *observation_p, const json_t *value_p)
{
	return SetValueFromJSON (& (observation_p -> so_corrected_value_s), value_p);
}



bool AddStringObservationRawValueToJSON (const StringObservation *obs_p, const char *key_s, json_t *json_p, const char *null_sequence_s, bool only_if_exists_flag)
{
	bool success_flag = false;

	if (only_if_exists_flag)
		{
			if ((! (obs_p -> so_raw_value_s)) || (AddStringValueToJSON (json_p, key_s, obs_p -> so_raw_value_s, null_sequence_s)))
				{
					success_flag = true;
				}
		}
	else
		{
			success_flag = AddStringValueToJSON (json_p, key_s, obs_p -> so_raw_value_s, null_sequence_s);
		}

	return success_flag;
}


bool AddStringObservationCorrectedValueToJSON (const StringObservation *obs_p, const char *key_s, json_t *json_p, const char *null_sequence_s, bool only_if_exists_flag)
{
	bool success_flag = false;

	if (only_if_exists_flag)
		{
			if ((! (obs_p -> so_raw_value_s)) || (AddStringValueToJSON (json_p, key_s, obs_p -> so_corrected_value_s, null_sequence_s)))
				{
					success_flag = true;
				}
		}
	else
		{
			success_flag = AddStringValueToJSON (json_p, key_s, obs_p -> so_corrected_value_s, null_sequence_s);
		}

	return success_flag;
}


static bool SetValueFromJSON (char **store_ss, const json_t *value_p)
{
	bool success_flag = false;

	if (json_is_string (value_p))
		{
			const char *value_s = json_string_value (value_p);

			if (value_s)
				{
					success_flag = ReplaceStringValue (store_ss, value_s);
				}
			else
				{
					if (*store_ss)
						{
							FreeCopiedString (*store_ss);
							store_ss = NULL;
						}
				}
		}

	return success_flag;
}



static bool AddStringValueToJSON (json_t *json_p, const char *key_s, const char *value_s, const char *null_sequence_s)
{
	bool success_flag = false;

	if (value_s)
		{
			if (SetJSONString (json_p, key_s, value_s))
				{
					success_flag = true;
				}
			else
				{
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, json_p, "Failed to set \"%s\": \"%s\" in JSON", key_s, value_s);
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

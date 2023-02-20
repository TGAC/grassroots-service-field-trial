/*
 * numeric_observation.c
 *
 *  Created on: 16 Mar 2022
 *      Author: billy
 */


#include "numeric_observation.h"
#include "dfw_util.h"
#include "math_utils.h"
#include "memory_allocations.h"
#include "typedefs.h"


static bool AddNumericValueToJSON (json_t *json_p, const char *key_s, const double64 *value_p, const char *null_sequence_s);

static bool SetValueFromString (double64 **store_pp, const char *value_s);

static bool SetValueFromJSON (double64 **store_pp, const json_t *value_p);

static bool SetNumericObservationValueFromString (Observation *observation_p, ObservationValueType ovt, const char *value_s);

static bool SetNumericObservationValueFromJSON (Observation *observation_p, ObservationValueType ovt, const json_t *value_p);

static bool GetValueAsString (const double64 *value_p, char **value_ss, bool *free_flag_p);

static bool GetNumericObservationValueAsString (struct Observation *observation_p, ObservationValueType ovt, char **value_ss, bool *free_value_flag_p);


NumericObservation *AllocateNumericObservation (bson_oid_t *id_p, const struct tm *start_date_p, const struct tm *end_date_p, MeasuredVariable *phenotype_p, MEM_FLAG phenotype_mem, const double *raw_value_p, const double *corrected_value_p,
	const char *growth_stage_s, const char *method_s, Instrument *instrument_p, const ObservationNature nature, const uint32 *index_p)
{
	double64 *copied_raw_value_p = NULL;

	if (CopyValidReal (raw_value_p, &copied_raw_value_p))
		{
			double64 *copied_corrected_value_p = NULL;

			if (CopyValidReal (corrected_value_p, &copied_corrected_value_p))
				{
					NumericObservation *observation_p = (NumericObservation *) AllocMemory (sizeof (NumericObservation));

					if (observation_p)
						{
							memset (observation_p, 0, sizeof (NumericObservation));

							if (InitObservation (& (observation_p -> no_base_observation), id_p, start_date_p, end_date_p, phenotype_p, phenotype_mem, growth_stage_s, method_s, instrument_p, nature, index_p, OT_NUMERIC,
																	 ClearNumericObservation,
																	 AddNumericObservationValuesToJSON,
																	 SetNumericObservationValueFromJSON,
																	 SetNumericObservationValueFromString,
																	 GetNumericObservationValueAsString))
								{
									observation_p -> no_raw_value_p = copied_raw_value_p;
									observation_p -> no_corrected_value_p = copied_corrected_value_p;

									return observation_p;
								}

							ClearObservation (& (observation_p -> no_base_observation));
							ClearNumericObservation (& (observation_p -> no_base_observation));
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





void ClearNumericObservation (Observation *observation_p)
{
	NumericObservation *numeric_obs_p = (NumericObservation *) observation_p;

	if (numeric_obs_p -> no_raw_value_p)
		{
			FreeMemory (numeric_obs_p -> no_raw_value_p);
			numeric_obs_p -> no_raw_value_p = NULL;
		}

	if (numeric_obs_p -> no_corrected_value_p)
		{
			FreeMemory (numeric_obs_p -> no_corrected_value_p);
			numeric_obs_p -> no_corrected_value_p = NULL;
		}
}



json_t *GetNumericObservationAsJSON (const NumericObservation *observation_p, const ViewFormat format)
{
	json_t *obs_json_p = GetObservationAsJSON (& (observation_p -> no_base_observation), format);

	if (obs_json_p)
		{
			if ((! (observation_p -> no_raw_value_p)) || (SetJSONReal (obs_json_p, OB_RAW_VALUE_S, * (observation_p -> no_raw_value_p))))
				{
					if ((! (observation_p -> no_corrected_value_p)) || (SetJSONReal (obs_json_p, OB_CORRECTED_VALUE_S, * (observation_p -> no_corrected_value_p))))
						{
							return obs_json_p;
						}
				}

			json_decref (obs_json_p);
		}

	return NULL;
}



NumericObservation *GetNumericObservationFromJSON (const json_t *phenotype_json_p, FieldTrialServiceData *data_p)
{
	NumericObservation *observation_p = (NumericObservation *) AllocMemory (sizeof (NumericObservation));

	if (observation_p)
		{

		}
	return NULL;
}


bool SetNumericObservationRawValueFromString (NumericObservation *observation_p, const char *value_s)
{
	return SetValueFromString (& (observation_p -> no_raw_value_p), value_s);
}


bool SetNumericObservationCorrectedValueFromString (NumericObservation *observation_p, const char *value_s)
{
	return SetValueFromString (& (observation_p -> no_corrected_value_p), value_s);
}

bool SetNumericObservationRawValueFromJSON (NumericObservation *observation_p, const json_t *value_p)
{
	return SetValueFromJSON (& (observation_p -> no_raw_value_p), value_p);
}


bool SetNumericObservationCorrectedValueFromJSON (NumericObservation *observation_p, const json_t *value_p)
{
	return SetValueFromJSON (& (observation_p -> no_raw_value_p), value_p);
}


bool GetNumericObservationRawValueAsString (NumericObservation *observation_p, char **value_ss, bool *free_flag_p)
{
	return GetValueAsString (observation_p -> no_raw_value_p, value_ss, free_flag_p);
}


bool GetNumericObservationCorrectedValueAsString (NumericObservation *observation_p, char **value_ss, bool *free_flag_p)
{
	return GetValueAsString (observation_p -> no_corrected_value_p, value_ss, free_flag_p);
}


bool AddNumericObservationValuesToJSON (const Observation *obs_p, const char *raw_key_s, const char *corrected_key_s, json_t *json_p, const char *null_sequence_s, bool only_if_exists_flag)
{
	const NumericObservation *num_obs_p = (const NumericObservation *) obs_p;
	bool success_flag = false;

	if (AddNumericObservationRawValueToJSON (num_obs_p, raw_key_s, json_p, null_sequence_s, only_if_exists_flag))
		{
			if (AddNumericObservationCorrectedValueToJSON (num_obs_p, corrected_key_s, json_p, null_sequence_s, only_if_exists_flag))
				{
					success_flag = true;
				}
		}

	return success_flag;
}


bool AddNumericObservationRawValueToJSON (const NumericObservation *obs_p, const char *key_s, json_t *json_p, const char *null_sequence_s, bool only_if_exists_flag)
{
	bool success_flag = false;

	if (only_if_exists_flag)
		{
			if ((! (obs_p -> no_raw_value_p)) || (AddNumericValueToJSON (json_p, key_s, obs_p -> no_raw_value_p, null_sequence_s)))
				{
					success_flag = true;
				}
		}
	else
		{
			success_flag = AddNumericValueToJSON (json_p, key_s, obs_p -> no_raw_value_p, null_sequence_s);
		}

	return success_flag;
}


bool AddNumericObservationCorrectedValueToJSON (const NumericObservation *obs_p, const char *key_s, json_t *json_p, const char *null_sequence_s, bool only_if_exists_flag)
{
	bool success_flag = false;

	if (only_if_exists_flag)
		{
			if ((! (obs_p -> no_corrected_value_p)) || (AddNumericValueToJSON (json_p, key_s, obs_p -> no_corrected_value_p, null_sequence_s)))
				{
					success_flag = true;
				}
		}
	else
		{
			success_flag = AddNumericValueToJSON (json_p, key_s, obs_p -> no_corrected_value_p, null_sequence_s);
		}

	return success_flag;
}


/*
 * STATIC DEFIINITIONS
 */




static bool SetNumericObservationValueFromString (Observation *observation_p, ObservationValueType ovt, const char *value_s)
{
	bool success_flag = false;
	NumericObservation *numeric_obs_p = (NumericObservation *) observation_p;

	switch (ovt)
		{
			case OVT_RAW_VALUE:
				success_flag = SetNumericObservationRawValueFromString (numeric_obs_p, value_s);
				break;

			case OVT_CORRECTED_VALUE:
				success_flag = SetNumericObservationCorrectedValueFromString (numeric_obs_p, value_s);
				break;

			default:
				break;
		}

	return success_flag;
}


static bool SetNumericObservationValueFromJSON (Observation *observation_p, ObservationValueType ovt, const json_t *value_p)
{
	bool success_flag = false;
	NumericObservation *numeric_obs_p = (NumericObservation *) observation_p;

	switch (ovt)
		{
			case OVT_RAW_VALUE:
				success_flag = SetNumericObservationRawValueFromJSON (numeric_obs_p, value_p);
				break;

			case OVT_CORRECTED_VALUE:
				success_flag = SetNumericObservationCorrectedValueFromJSON (numeric_obs_p, value_p);
				break;

			default:
				break;
		}

	return success_flag;
}



static bool SetValueFromString (double64 **store_pp, const char *value_s)
{
	bool success_flag = false;

	if (!IsStringEmpty (value_s))
		{
			double d;

			if (GetValidRealNumber (&value_s, &d, NULL))
				{
					if (! (*store_pp))
						{
							double64 *d_p = (double64 *) AllocMemory (sizeof (double64));

							if (d_p)
								{
									*store_pp = d_p;
								}
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate memory for storing raw value " DOUBLE64_FMT " from \"%s\"", d, value_s);
									return false;
								}
						}

					**store_pp = d;
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


static bool SetValueFromJSON (double64 **store_pp, const json_t *value_p)
{
	bool success_flag = false;

	if (json_is_number (value_p))
		{
			double d = json_real_value (value_p);

			if (*store_pp)
				{
					**store_pp = d;
					success_flag = true;
				}
			else
				{
					double64 *d_p = (double64 *) AllocMemory (sizeof (double64));

					if (d_p)
						{
							*d_p = d;
							*store_pp = d_p;
							success_flag = true;
						}
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate memory for storing value %lf", d);
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


static bool AddNumericValueToJSON (json_t *json_p, const char *key_s, const double64 *value_p, const char *null_sequence_s)
{
	bool success_flag = false;

	if (value_p)
		{
			if (SetJSONReal (json_p, key_s, *value_p))
				{
					success_flag = true;
				}
			else
				{
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, json_p, "Failed to set \"%s\": " DOUBLE64_FMT " in JSON", key_s, *value_p);
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


static bool GetNumericObservationValueAsString (struct Observation *observation_p, ObservationValueType ovt, char **value_ss, bool *free_value_flag_p)
{
	bool success_flag = false;
	NumericObservation *num_obs_p = (NumericObservation *) observation_p;

	switch (ovt)
		{
			case OVT_RAW_VALUE:
				success_flag = GetNumericObservationRawValueAsString (num_obs_p, value_ss, free_value_flag_p);
				break;

			case OVT_CORRECTED_VALUE:
				success_flag = GetNumericObservationCorrectedValueAsString (num_obs_p, value_ss, free_value_flag_p);
				break;

			default:
				break;
		}

	return success_flag;

}


static bool GetValueAsString (const double64 *value_p, char **value_ss, bool *free_flag_p)
{
	bool success_flag = false;

	if (value_p)
		{
			char *value_s = ConvertDoubleToString (*value_p);

			if (value_s)
				{
					*value_ss = value_s;
					*free_flag_p = true;
					success_flag = true;
				}
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate memory for storing value " INT32_FMT, *value_p);
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




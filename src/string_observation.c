/*
 * string_observation.c
 *
 *  Created on: 17 Mar 2022
 *      Author: billy
 */


#include "string_observation.h"





StringObservation *AllocateStringObservation (bson_oid_t *id_p, const struct tm *start_date_p, const struct tm *end_date_p, MeasuredVariable *phenotype_p, MEM_FLAG phenotype_mem, const char * const raw_value_s, const char * const corrected_value_s,
	const char *growth_stage_s, const char *method_s, Instrument *instrument_p, const ObservationNature nature, const uint32 *index_p)
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
							if (InitObservation (& (observation_p -> so_base_observation), id_p, start_date_p, end_date_p, phenotype_p, phenotype_mem, growth_stage_s, method_s, instrument_p, nature, index_p, OT_STRING))
								{
									observation_p -> so_raw_value_s = copied_raw_value_s;
									observation_p -> so_corrected_value_s = copied_corrected_value_s;

									return observation_p;
								}

							FreeStringObservation (observation_p);
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



void FreeStringObservation (StringObservation *observation_p)
{
	ClearObservation (& (observation_p -> so_base_observation));

	if (observation_p -> so_raw_value_s)
		{
			FreeCopiedString (observation_p -> so_raw_value_s);
		}

	if (observation_p -> so_corrected_value_s)
		{
			FreeCopiedString (observation_p -> so_corrected_value_s);
		}


	FreeMemory (observation_p);
}



json_t *GetStringObservationAsJSON (const StringObservation *observation_p, const ViewFormat format)
{
	json_t *observation_json_p = GetObservationAsJSON (& (observation_p -> so_base_observation), format);

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


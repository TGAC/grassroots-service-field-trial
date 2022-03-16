/*
 * numeric_observation.c
 *
 *  Created on: 16 Mar 2022
 *      Author: billy
 */


#include "numeric_observation.h"



NumericObservation *AllocateNumericObservation (bson_oid_t *id_p, const struct tm *start_date_p, const struct tm *end_date_p, MeasuredVariable *phenotype_p, MEM_FLAG phenotype_mem, const double *raw_value_p, const double *corrected_value_p,
	const char *growth_stage_s, const char *method_s, Instrument *instrument_p, const ObservationNature nature, const uint32 *index_p, const ObservationType obs_type)
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
							if (InitObservation (observation_p, id_p, start_date_p, end_date_p, phenotype_p, phenotype_mem, growth_stage_s, method_s, instrument_p, nature, index_p, obs_type))
								{
									observation_p -> no_raw_value_p = copied_raw_value_p;
									observation_p -> no_corrected_value_p = copied_corrected_value_p;

									return observation_p;
								}

							FreeNumericObservation (observation_p);
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



void FreeNumericObservation (NumericObservation *observation_p)
{
	ClearObservation (& (observation_p -> no_base_observation));

	if (observation_p -> no_raw_value_p)
		{
			FreeMemory (observation_p -> no_raw_value_p);
		}

	if (observation_p -> no_corrected_value_p)
		{
			FreeMemory (observation_p -> no_corrected_value_p);
		}

	FreeMemory (observation_p);
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





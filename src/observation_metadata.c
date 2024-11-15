/*
 * observation_metadata.c
 *
 *  Created on: 13 Nov 2024
 *      Author: billy
 */


#include "observation_metadata.h"




OperationStatus GetObservationMetadata (const char *key_s, MeasuredVariable **measured_variable_pp, ObservtionMetadata *metadata_p, bool *notes_flag_p, ServiceJob *job_p, const uint32 row_index, MEM_FLAG *mf_p, FieldTrialServiceData *data_p);


void ClearObservationMetadata (ObservationMetadata *obs_metadata_p)
{
	if (obs_metadata_p -> om_start_date_p)
		{
			FreeTime (obs_metadata_p -> om_start_date_p);
			obs_metadata_p -> om_start_date_p = NULL;
		}

	if (obs_metadata_p -> om_end_date_p)
		{
			FreeTime (obs_metadata_p -> om_end_date_p);
			obs_metadata_p -> om_end_date_p = NULL;
		}

	obs_metadata_p -> om_corrected_flag = false;
	obs_metadata_p -> om_index = 1;
}


void FreeObservationMetadata (ObservationMetadata *obs_metadata_p)
{
	ClearObservationMetadata (obs_metadata_p);
	FreeMemory (obs_metadata_p);
}


int CompareObservationMetadata (const ObservationMetadata * const om_0_p, const ObservationMetadata * const om_1_p)
{
}





static int CompareDatePointers (const struct tm *tm_0_p, const struct tm *tm_1_p)
{
	int res = 0;

	if (tm_0_p)
		{
			if (tm_1_p)
				{
					res = CompareDates (tm_0_p, tm_1_p, true);
				}
			else
				{
					res = -1;
				}
		}
	else if (tm_1_p)
		{
			res = 1;
		}

	return res;
}

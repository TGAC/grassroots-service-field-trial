/*
 * observation_metadata.c
 *
 *  Created on: 13 Nov 2024
 *      Author: billy
 */

#include <stdlib.h>

#include "observation_metadata.h"

#include "time_util.h"


static int CompareDatePointers (const struct tm *tm_0_p, const struct tm *tm_1_p);


static bool FillObservationMetadataHashBucket (HashBucket * const bucket_p, const void * const key_p, const void * const value_p);



/**
	* Get A HashTable of HashBuckets that have strings for their keys and values.
	*
	*
	*/
HashTable *GetHashTableOfObservationMetadata (const uint32 initial_capacity, const uint8 load_percentage)
{
	return AllocateHashTable (initial_capacity, load_percentage, HashString, CreateDeepCopyHashBuckets, NULL, FillObservationMetadataHashBucket, CompareStringHashBuckets, NULL, NULL);
}


ObservationMetadata *AllocateObservationMetadata (const struct tm * const start_date_p, const struct tm * const end_date_p, const bool corrected_flag, const uint32 index)
{
	struct tm *copied_start_date_p = NULL;

	if ((start_date_p == NULL) || ((copied_start_date_p = DuplicateTime (start_date_p)) != NULL))
		{
			struct tm *copied_end_date_p = NULL;

			if ((end_date_p == NULL) || ((copied_end_date_p = DuplicateTime (end_date_p)) != NULL))
				{
					ObservationMetadata *metadata_p = (ObservationMetadata *) AllocMemory (sizeof (ObservationMetadata));

					if (metadata_p)
						{
							metadata_p -> om_start_date_p = copied_start_date_p;
							metadata_p -> om_end_date_p = copied_end_date_p;
							metadata_p -> om_corrected_flag = corrected_flag;
							metadata_p -> om_index = index;

							return metadata_p;
						}

					if (copied_end_date_p)
						{
							FreeTime (copied_end_date_p);
						}
				}

			if (copied_start_date_p)
				{
					FreeTime (copied_start_date_p);
				}
		}

	return NULL;
}


/**
 * Extract the Observation metadata from the column heading
 */

OperationStatus GetObservationMetadata (const char *key_s, MeasuredVariable **measured_variable_pp, ObservationMetadata **metadata_pp, bool *notes_flag_p, ServiceJob *job_p, const uint32 row_index, MEM_FLAG *mf_p, FieldTrialServiceData *data_p)
{
	OperationStatus status = OS_IDLE;
	LinkedList *tokens_p = ParseStringToStringLinkedList (key_s, " ", true);


	if (tokens_p)
		{
			StringListNode *node_p = (StringListNode *) (tokens_p -> ll_head_p);
			MeasuredVariable *measured_variable_p = NULL;
			measured_variable_p = GetMeasuredVariableByVariableName (node_p -> sln_string_s, mf_p, data_p);

			if (measured_variable_p)
				{
					struct tm *start_date_p = NULL;
					struct tm *end_date_p = NULL;
					const char * const NOTES_KEY_S = "notes";
					const char * const CORRECTED_KEY_S = "corrected";
					const char * const INDEX_PREFIX_KEY_S = "sample_";
					const size_t INDEX_PREFIX_KEY_LENGTH = strlen (INDEX_PREFIX_KEY_S);
					uint32 index = OB_DEFAULT_INDEX;
					bool start_date_flag = true;
					bool loop_flag = true;
					bool corrected_flag = false;

					node_p = (StringListNode *) (node_p -> sln_node.ln_next_p);

					status = OS_SUCCEEDED;

					while (node_p && loop_flag && (status == OS_SUCCEEDED))
						{
							const char *value_s = node_p -> sln_string_s;

							if (Stricmp (value_s, CORRECTED_KEY_S) == 0)
								{
									corrected_flag = true;
								}
							else if (Stricmp (value_s, NOTES_KEY_S) == 0)
								{
									*notes_flag_p = true;
								}
							else if (Strnicmp (value_s, INDEX_PREFIX_KEY_S, INDEX_PREFIX_KEY_LENGTH) == 0)
								{
									const char *temp_s = value_s + INDEX_PREFIX_KEY_LENGTH;
									int32 answer;

									if (GetValidInteger (&temp_s, &answer))
										{
											if (answer >= 1)
												{
													index = (uint32) answer;
												}
											else
												{
													ReportObservationMetadataError (job_p, "Invalid sample index from", key_s, value_s);
													status = OS_FAILED;
												}
										}
									else
										{
											ReportObservationMetadataError (job_p, "Failed to create sample index from", key_s, value_s);
											status = OS_FAILED;
										}
								}
							else
								{
									if (start_date_flag)
										{
											start_date_p = GetTimeFromString (value_s);

											if (start_date_p)
												{
													start_date_flag = false;
												}
											else
												{
													ReportObservationMetadataError (job_p, "Failed to create start date from", key_s, value_s);
													status = OS_FAILED;
												}

										}
									else
										{
											end_date_p = GetTimeFromString (value_s);

											if (!end_date_p)
												{
													ReportObservationMetadataError (job_p, "Failed to create end date from", key_s, value_s);
													status = OS_FAILED;
												}
										}
								}

							if (start_date_p && end_date_p && corrected_flag)
								{
									loop_flag = false;
								}
							else
								{
									node_p = (StringListNode *) (node_p -> sln_node.ln_next_p);
								}
						}		/* while (node_p) */


					if (status == OS_SUCCEEDED)
						{
							ObservationMetadata *metadata_p = AllocateObservationMetadata (start_date_p, end_date_p, corrected_flag, index);

							if (metadata_p)
								{
									*metadata_pp = metadata_p;
									*measured_variable_pp = measured_variable_p;
								}
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "AllocateObservationMetadata () failed for \"%s\"", key_s);
								}

						}
					else
						{
							if (start_date_p)
								{
									FreeTime (start_date_p);
								}

							if (end_date_p)
								{
									FreeTime (end_date_p);
								}

							FreeMeasuredVariable (measured_variable_p);
						}

				}		/* if (measured_variable_p) */
			else
				{
					PrintErrors (STM_LEVEL_FINE, __FILE__, __LINE__, "Failed to get Measured Variable for \"%s\"", node_p -> sln_string_s);
				}


			FreeLinkedList (tokens_p);
		}		/* if (tokens_p) */


	return status;
}


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


ObservationMetadata *CopyObservationMetadata (const ObservationMetadata * const src_p)
{
	return AllocateObservationMetadata (src_p -> om_start_date_p, src_p -> om_start_date_p, src_p -> om_corrected_flag, src_p -> om_index);
}



int CompareObservationMetadata (const ObservationMetadata * const om_0_p, const ObservationMetadata * const om_1_p)
{
	int res = CompareDatePointers (om_0_p, om_1_p);

	if (res == 0)
		{
			if (om_0_p -> om_corrected_flag == om_1_p -> om_corrected_flag)
				{
					if (om_0_p -> om_index < om_1_p -> om_index)
						{
							res = -1;
						}
					else if (om_0_p -> om_index > om_1_p -> om_index)
						{
							res = 1;
						}

				}
			else if (om_0_p -> om_corrected_flag)
				{
					res = 1;
				}
			else
				{
					res = -1;
				}

		}

	return res;
}


bool AddObservationMetadataToJSON (ObservationMetadata * const metadata_p, json_t *observation_json_p)
{
	bool success_flag = false;

	return success_flag;
}


bool SetObservationMetadataStartDate (ObservationMetadata * const metadata_p, const struct tm * const time_p)
{
	return SetObservationMetadataDate (time_p, & (metadata_p -> om_start_date_p));
}


bool SetObservationMetadataEndDate (ObservationMetadata * const metadata_p, const struct tm * const time_p)
{
	return SetObservationMetadataDate (time_p, & (metadata_p -> om_end_date_p));
}


static bool SetObservationMetadataDate (const struct tm * const src_p, struct tm **dest_pp)
{
	bool success_flag = false;

	if (*dest_pp)
		{
			CopyTime (src_p, *dest_pp);
			success_flag = true;
		}
	else
		{
			struct tm *dest_p = DuplicateTime (src_p);

			if (dest_p)
				{
					*dest_pp = dest_p;
					success_flag = true;
				}
		}

	return success_flag;
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



static bool FillObservationMetadataHashBucket (HashBucket * const bucket_p, const void * const key_p, const void * const value_p)
{
	bool success_flag = false;

	if (FillStringValue (key_p, & (bucket_p -> hb_key_p), bucket_p -> hb_owns_key))
		{
			if (FillObservationMetadataValue (value_p, & (bucket_p -> hb_value_p), bucket_p -> hb_owns_value))
				{
					success_flag = true;
				}
		}

	return success_flag;

}



static bool FillObservationMetadataValue (const void *src_p, const void **dest_pp, const MEM_FLAG mf)
{
	bool success_flag = true;

	switch (mf)
		{
			case MF_DEEP_COPY:
				{
					ObservationMetadata *dest_p = CopyObservationMetadata ((const ObservationMetadata * const) src_p);

					if (dest_p)
						{
							*dest_pp = dest_p;
						}
					else
						{
							success_flag = false;
						}
				}
				break;

			case MF_SHALLOW_COPY:
			case MF_SHADOW_USE:
			case MF_ALREADY_FREED:
				*dest_pp = src_p;
				break;
		}

	return success_flag;
}





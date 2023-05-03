/*
 * plots_cache.c
 *
 *  Created on: 28 Jun 2022
 *      Author: billy
 */


#include "plots_cache.h"
#include "plot_jobs.h"

#include "math_utils.h"


static int IsCachedEntry (json_t *cache_p, const char * const key_s, const size_t row_index, size_t *duplicate_value_p);



PlotsCache *AllocatePlotsCache (void)
{
	json_t *grid_cache_p = json_object ();

	if (grid_cache_p)
		{
			json_t *id_cache_p = json_object ();

			if (id_cache_p)
				{
					PlotsCache * pc_p = (PlotsCache *) AllocMemory (sizeof (PlotsCache));

					if (pc_p)
						{
							pc_p -> pc_grid_cache_p = grid_cache_p;
							pc_p -> pc_index_cache_p = id_cache_p;

							return pc_p;
						}

					json_decref (id_cache_p);
				}

			json_decref (grid_cache_p);
		}

	return NULL;
}


void FreePlotsCache (PlotsCache *plots_cache_p)
{
	json_decref (plots_cache_p -> pc_grid_cache_p);
	json_decref (plots_cache_p -> pc_index_cache_p);

	FreeMemory (plots_cache_p);
}


bool CheckPlotRequirements (PlotsCache *plots_cache_p, const json_t *table_row_json_p, const size_t row_index, ServiceJob *job_p, int32 *row_p, int32 *column_p, int32 *index_p, int32 *rack_p)
{
	bool success_flag = false;
	const char *row_s = GetJSONString (table_row_json_p, PL_ROW_TITLE_S);

	if (row_s)
		{
			const char *column_s = GetJSONString (table_row_json_p, PL_COLUMN_TITLE_S);

			if (column_s)
				{
					const char * const sep_s = " - ";
					const char *rack_s = GetJSONString (table_row_json_p, PL_RACK_TITLE_S);
					char *row_and_column_s = NULL;

					if (!rack_s)
						{
							rack_s = "1";
						}


					char *row_and_column_and_rack_s = ConcatenateVarargsStrings (row_s, sep_s, column_s, sep_s, rack_s, NULL);

					if (row_and_column_and_rack_s)
						{
							const char *index_s = GetJSONString (table_row_json_p, PL_INDEX_TABLE_TITLE_S);

							if (index_s)
								{
									size_t matched_row;
									int res = IsCachedEntry (plots_cache_p -> pc_index_cache_p, index_s, row_index, &matched_row);

									if (res == 0)
										{
											res = IsCachedEntry (plots_cache_p -> pc_grid_cache_p, row_and_column_and_rack_s, row_index, &matched_row);

											if (res == 0)
												{
													if (GetValidInteger (&row_s, row_p))
														{
															if (GetValidInteger (&column_s, column_p))
																{
																	if (GetValidInteger (&index_s, index_p))
																		{
																			if (GetValidInteger (&rack_s, rack_p))
																				{
																					success_flag = true;
																				}		/* if (GetValidInteger (&index_s, index_p)) */
																			else
																				{
																					AddTabularParameterErrorMessageToServiceJob (job_p, PL_PLOT_TABLE.npt_name_s, PL_PLOT_TABLE.npt_type, "Failed to get rack as a number", row_index, PL_RACK_TITLE_S);
																					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, table_row_json_p, "Failed to get \"%s\" as a number from \"%s\"", rack_s, PL_RACK_TITLE_S);
																				}

																		}		/* if (GetValidInteger (&index_s, index_p)) */
																	else
																		{
																			AddTabularParameterErrorMessageToServiceJob (job_p, PL_PLOT_TABLE.npt_name_s, PL_PLOT_TABLE.npt_type, "Failed to get index as a number", row_index, PL_INDEX_TABLE_TITLE_S);
																			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, table_row_json_p, "Failed to get \"%s\" as a number from \"%s\"", index_s, PL_INDEX_TABLE_TITLE_S);
																		}

																}		/* if (GetValidInteger (&column_s, column_p)) */
															else
																{
																	AddTabularParameterErrorMessageToServiceJob (job_p, PL_PLOT_TABLE.npt_name_s, PL_PLOT_TABLE.npt_type, "Failed to get column as a number", row_index, PL_COLUMN_TITLE_S);
																	PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, table_row_json_p, "Failed to get \"%s\" as a number from \"%s\"", column_s, PL_COLUMN_TITLE_S);
																}

														}		/* if (GetValidInteger (&row_s, row_p)) */
													else
														{
															AddTabularParameterErrorMessageToServiceJob (job_p, PL_PLOT_TABLE.npt_name_s, PL_PLOT_TABLE.npt_type, "Failed to get row as a number", row_index, PL_ROW_TITLE_S);
															PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, table_row_json_p, "Failed to get \"%s\" as a number from \"%s\"", row_s, PL_ROW_TITLE_S);
														}

												}		/* if (res == 0) */
											else if (res == 1)
												{
													bool done_error_flag = false;
													char *matched_row_s = ConvertSizeTToString (matched_row);

													if (matched_row_s)
														{
															char *full_error_s = ConcatenateVarargsStrings ("Row, column and rack values are duplicates of row ", matched_row_s, " in the spreadsheet", NULL);

															if (full_error_s)
																{
																	AddTabularParameterErrorMessageToServiceJob (job_p, PL_PLOT_TABLE.npt_name_s, PL_PLOT_TABLE.npt_type, full_error_s, row_index, PL_ROW_TITLE_S);
																	done_error_flag = true;

																	FreeCopiedString (full_error_s);
																}

															FreeCopiedString (matched_row_s);
														}

													if (!done_error_flag)
														{
															AddTabularParameterErrorMessageToServiceJob (job_p, PL_PLOT_TABLE.npt_name_s, PL_PLOT_TABLE.npt_type, "Row, column and rack are duplicates of another row in the spreadsheet", row_index, PL_ROW_TITLE_S);
														}

													PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, table_row_json_p, "Row \"%s\", column \"%s\" and rack \"%s\" on row " SIZET_FMT " are duplicates of row " SIZET_FMT " in the spreadsheet", row_s, column_s, rack_s, row_index, matched_row, PL_ROW_TITLE_S);
												}		/* else if (res == 1) */
											else if (res == -1)
												{
													AddTabularParameterErrorMessageToServiceJob (job_p, PL_PLOT_TABLE.npt_name_s, PL_PLOT_TABLE.npt_type, "Internal error parsing table", row_index, PL_INDEX_TABLE_TITLE_S);
													PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, table_row_json_p, "Internal error parsing table", PL_INDEX_TABLE_TITLE_S);
												}		/* else if (res == -1) */

										}		/* if (res == 0) */
									else if (res == 1)
										{
											bool done_error_flag = false;
											char *matched_row_s = ConvertSizeTToString (matched_row);

											if (matched_row_s)
												{
													char *full_error_s = ConcatenateVarargsStrings ("Plot Id is duplicate of value on row ", matched_row_s, " in the spreadsheet", NULL);

													if (full_error_s)
														{
															AddTabularParameterErrorMessageToServiceJob (job_p, PL_PLOT_TABLE.npt_name_s, PL_PLOT_TABLE.npt_type, full_error_s, row_index, PL_ROW_TITLE_S);
															done_error_flag = true;

															FreeCopiedString (full_error_s);
														}

													FreeCopiedString (matched_row_s);
												}

											if (!done_error_flag)
												{
													AddTabularParameterErrorMessageToServiceJob (job_p, PL_PLOT_TABLE.npt_name_s, PL_PLOT_TABLE.npt_type, "Plot Id is duplicate of value on row in the spreadsheet", row_index, PL_ROW_TITLE_S);
												}

											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, table_row_json_p, "Plot Id \"%s\" on row " SIZET_FMT " is a duplicate of row " SIZET_FMT " in the spreadsheet", index_s, row_index, matched_row, PL_ROW_TITLE_S);

										}		/* else if (res == 1) */
									else if (res == -1)
										{
											AddTabularParameterErrorMessageToServiceJob (job_p, PL_PLOT_TABLE.npt_name_s, PL_PLOT_TABLE.npt_type, "Internal error parsing table", row_index, PL_INDEX_TABLE_TITLE_S);
											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, table_row_json_p, "Internal error parsing table", PL_INDEX_TABLE_TITLE_S);
										}		/* else if (res == -1) */

								}		/* if (index_s) */
							else
								{
									AddTabularParameterErrorMessageToServiceJob (job_p, PL_PLOT_TABLE.npt_name_s, PL_PLOT_TABLE.npt_type, "Value not set", row_index, PL_INDEX_TABLE_TITLE_S);
									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, table_row_json_p, "Failed to get \"%s\"", PL_INDEX_TABLE_TITLE_S);
								}

							FreeCopiedString (row_and_column_s);
						}		/* if (row_and_column_s) */
					else
						{
							AddTabularParameterErrorMessageToServiceJob (job_p, PL_PLOT_TABLE.npt_name_s, PL_PLOT_TABLE.npt_type, "Internal error parsing table", row_index, NULL);
							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, table_row_json_p, "Failed to concatenate \"%s\" and \"%s\"", row_s, column_s);
						}

				}		/* if (column_s) */
			else
				{
					AddTabularParameterErrorMessageToServiceJob (job_p, PL_PLOT_TABLE.npt_name_s, PL_PLOT_TABLE.npt_type, "Failed to get value", row_index, PL_COLUMN_TITLE_S);
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, table_row_json_p, "Could not parse \"%s\" to an integer", PL_COLUMN_TITLE_S);
				}

		}		/* if (row_s) */
	else
		{
			AddTabularParameterErrorMessageToServiceJob (job_p, PL_PLOT_TABLE.npt_name_s, PL_PLOT_TABLE.npt_type, "Failed to get value", row_index, PL_ROW_TITLE_S);
			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, table_row_json_p, "Could not parse \"%s\" to an integer", PL_ROW_TITLE_S);
		}

	return success_flag;
}



static int IsCachedEntry (json_t *cache_p, const char * const key_s, const size_t row_index, size_t *duplicate_value_p)
{
	int cached_res = 0;

	/*
	 * Is this combo unique within the data that we are currently
	 * importing?
	 */
	json_t *child_obj_p = json_object_get (cache_p, key_s);

	if (child_obj_p)
		{
			*duplicate_value_p =  (size_t) json_integer_value (child_obj_p);
			cached_res = 1;
		}
	else
		{
			if (!SetJSONInteger (cache_p, key_s, row_index))
				{
					cached_res = -1;
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, cache_p, "Failed to add cache entry for \"%s\": " SIZET_FMT, key_s, row_index);
				}
		}

	return cached_res;
}


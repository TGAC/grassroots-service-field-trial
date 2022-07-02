/*
 * base_row.c
 *
 *  Created on: 29 Jun 2022
 *      Author: billy
 */

#define ALLOCATE_BASE_ROW_TAGS (1)
#include "base_row.h"

#include "memory_allocations.h"
#include "streams.h"
#include "mongodb_tool.h"
#include "plot.h"


static const char *S_ROW_TYPES_SS [] = { "Standard", "Discard", "Blank" };



BaseRow *AllocateBaseRow (bson_oid_t *id_p, const uint32 study_index, Plot *parent_plot_p, RowType rt,
													void (*clear_fn) (BaseRow *row_p),
													bool (*add_to_json_fn) (const BaseRow *row_p, json_t *row_json_p, const ViewFormat format, const FieldTrialServiceData *data_p),
													bool (*add_to_fd_fn) (const BaseRow *row_p, json_t *row_fd_p, const FieldTrialServiceData *service_data_p, const char * const null_sequence_s))
{
	BaseRow *row_p = (BaseRow *) AllocMemory (sizeof (BaseRow));

	if (row_p)
		{
			if (InitBaseRow (row_p, id_p, study_index, parent_plot_p, rt, clear_fn, add_to_json_fn, add_to_fd_fn))
				{
					return row_p;
				}

			FreeMemory (row_p);
		}

	return NULL;
}


bool InitBaseRow (BaseRow *row_p, bson_oid_t *id_p, const uint32 study_index, Plot *parent_plot_p, RowType rt,
									void (*clear_fn) (BaseRow *row_p),
									bool (*add_to_json_fn) (const BaseRow *row_p, json_t *row_json_p, const ViewFormat format, const FieldTrialServiceData *data_p),
									bool (*add_to_fd_fn) (const BaseRow *row_p, json_t *row_fd_p, const FieldTrialServiceData *service_data_p, const char * const null_sequence_s))

{
	if (!id_p)
		{
			id_p = GetNewBSONOid ();

			if (!id_p)
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate BSON oid for row at [" UINT32_FMT ", " UINT32_FMT "] for study \"%s\"", parent_plot_p -> pl_parent_p -> st_name_s);
					return false;
				}
		}

	row_p -> br_type = rt;
	row_p -> br_id_p = id_p;
	row_p -> br_by_study_index = study_index;
	row_p -> br_plot_p = parent_plot_p;
	row_p -> br_study_p = parent_plot_p -> pl_parent_p;
	row_p -> br_clear_fn = clear_fn;
	row_p -> br_add_to_json_fn = add_to_json_fn;
	row_p -> br_add_to_fd_fn = add_to_fd_fn;


	return true;
}


void ClearBaseRow (BaseRow *row_p)
{
	FreeBSONOid (row_p -> br_id_p);
}


void FreeBaseRow (BaseRow *row_p)
{
	ClearBaseRow (row_p);

	if (row_p -> br_clear_fn)
		{
			row_p -> br_clear_fn (row_p);
		}

	FreeMemory (row_p);
}



void SetRowType (BaseRow *row_p, RowType rt)
{
	row_p -> br_type = rt;
}


RowType GetRowType (const BaseRow *row_p)
{
	return (row_p -> br_type);
}



bool AddBaseRowToJSON (const BaseRow *row_p, json_t *row_json_p, const ViewFormat format, const FieldTrialServiceData *data_p)
{
	bool success_flag = false;

	if (SetJSONInteger (row_json_p, BR_STUDY_INDEX_S, row_p -> br_by_study_index))
		{
			/*
			 * We only need to store the parent plot id if the JSON is for the backend
			 */
			if (format == VF_STORAGE)
				{
					if (AddCompoundIdToJSON (row_json_p, row_p -> br_id_p))
						{
							if (AddNamedCompoundIdToJSON (row_json_p, row_p -> br_plot_p -> pl_id_p, BR_PLOT_ID_S))
								{
									if (AddNamedCompoundIdToJSON (row_json_p, row_p -> br_study_p -> st_id_p, BR_STUDY_ID_S))
										{
											const char *type_s = GetRowTypeAsString (row_p -> br_type);

											if (type_s)
												{
													if (SetJSONString (row_json_p, BR_ROW_TYPE_S, type_s))
														{
															if (row_p -> br_add_to_json_fn)
																{
																	success_flag = row_p -> br_add_to_json_fn (row_p, row_json_p, format, data_p);
																}
															else
																{
																	success_flag = true;
																}
														}		/* if (SetJSONString (row_json_p, BR_ROW_TYPE_S, type_s)) */
													else
														{
															PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, row_json_p, "SetJSONString() failed for row " UINT32_FMT " in study \"%s\", \"%s\": \"%s\"",
																								 row_p -> br_by_study_index, row_p -> br_plot_p -> pl_parent_p -> st_name_s,
																								 BR_ROW_TYPE_S, type_s);
														}

												}		/* if (type_s) */
											else
												{
													PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "GetRowTypeAsString () failed for row " UINT32_FMT " in study \"%s\" with type %d",
																				 row_p -> br_by_study_index, row_p -> br_plot_p -> pl_parent_p -> st_name_s, row_p -> br_type);
												}

										}		/* if (AddNamedCompoundIdToJSON (row_json_p, row_p -> ro_plot_p -> pl_id_p, RO_PLOT_ID_S)) */
									else
										{
											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, row_json_p, "Failed to add study id for row " UINT32_FMT " in study \"%s\"",
																				 row_p -> br_by_study_index, row_p -> br_plot_p -> pl_parent_p -> st_name_s);
										}

								}		/* if (AddNamedCompoundIdToJSON (row_json_p, row_p -> ro_plot_p -> pl_id_p, RO_PLOT_ID_S)) */
							else
								{
									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, row_json_p, "Failed to add plot id for for row " UINT32_FMT " in study \"%s\"",
																		 row_p -> br_by_study_index, row_p -> br_plot_p -> pl_parent_p -> st_name_s);
								}

						}		/* if (AddCompoundIdToJSON (row_json_p, row_p -> ro_id_p)) */
					else
						{
							char *id_s = GetBSONOidAsString (row_p -> br_id_p);

							if (id_s)
								{
									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, row_json_p, "Failed to add row compound id \"%s\"", id_s);
									FreeBSONOidString (id_s);
								}
							else
								{
									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, row_json_p, "Failed to row add compound id");
								}
						}


				}		/* if (format == VF_STORAGE) */
			else
				{
					success_flag = true;
				}

		}		/* if (SetJSONInteger (row_json_p, RO_STUDY_INDEX_S, row_p -> ro_by_study_index)) */
	else
		{
			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, row_json_p, "Failed to add \"%s\": " UINT32_FMT, BR_STUDY_INDEX_S, row_p -> br_by_study_index);
		}

	return success_flag;
}


json_t *GetRowAsJSON (const BaseRow *row_p, const ViewFormat format, JSONProcessor *processor_p, const FieldTrialServiceData *data_p)
{
	json_t *row_json_p = json_object ();

	if (row_json_p)
		{
			if (AddBaseRowToJSON (row_p, row_json_p, format, data_p))
				{
					bool success_flag = true;

					if (row_p -> br_add_to_json_fn)
						{
							success_flag = row_p -> br_add_to_json_fn (row_p, row_json_p, format, data_p);
						}

					if (success_flag)
						{
							return row_json_p;
						}

				}		/* if (AddBaseRowToJSON (& (row_p -> sr_base), row_json_p, format)) */
			else
				{

				}

			json_decref (row_json_p);
		}		/* if (row_json_p) */

	return NULL;
}


BaseRow *GetBaseRowFromJSON (const json_t *json_p, Plot *plot_p, const Study *study_p, const ViewFormat format, FieldTrialServiceData *data_p)
{
	BaseRow *row_p = NULL;

	return row_p;
}



bool PopulateBaseRowFromJSON (BaseRow *row_p, const json_t *row_json_p)
{
	bool success_flag = false;

	RowType rt = RT_STANDARD;
	if ()

	return success_flag;
}


RowNode *AllocateRowNode (BaseRow *row_p)
{
	RowNode *sr_node_p = (RowNode *) AllocMemory (sizeof (RowNode));

	if (sr_node_p)
		{
			InitListItem (& (sr_node_p -> rn_node));

			sr_node_p -> rn_row_p = row_p;
		}

	return sr_node_p;
}



void FreeRowNode (ListItem *node_p)
{
	RowNode *sr_node_p = (RowNode *) node_p;

	if (sr_node_p -> rn_row_p)
		{
			FreeBaseRow (sr_node_p -> rn_row_p);
		}

	FreeMemory (sr_node_p);
}


const char *GetRowTypeAsString (const RowType rt)
{
	if (rt <= RT_NUM_VALUES)
		{
			return (* (S_ROW_TYPES_SS + rt));
		}

	return NULL;
}


bool SetRowTypeFromString (RowType *rt_p, const char *value_s)
{
	const char **type_ss = S_ROW_TYPES_SS;
	RowType r = RT_STANDARD;

	while (r < RT_NUM_VALUES)
		{
			if (strcmp (*type_ss, value_s) == 0)
				{
					*rt_p = r;
				}
			else
				{
					++ type_ss;
					++ r;
				}
		}


	return false;
}




/*
 * base_row.c
 *
 *  Created on: 29 Jun 2022
 *      Author: billy
 */

#include "base_row.h"

#include "memory_allocations.h"
#include "streams.h"
#include "mongodb_tool.h"
#include "plot.h"


BaseRow *AllocateBlankRow (bson_oid_t *id_p, const uint32 study_index, Plot *parent_plot_p)
{
	return AllocateBaseRow (id_p, study_index, parent_plot_p, RT_BLANK);
}


BaseRow *AllocateDiscardRow (bson_oid_t *id_p, const uint32 study_index, Plot *parent_plot_p)
{
	return AllocateBaseRow (id_p, study_index, parent_plot_p, RT_DISCARD);
}


BaseRow *AllocateBaseRow (bson_oid_t *id_p, const uint32 study_index, Plot *parent_plot_p, RowType rt)
{
	BaseRow *row_p = (BaseRow *) AllocMemory (sizeof (BaseRow));

	if (row_p)
		{
			if (InitBaseRow (row_p, id_p, study_index, parent_plot_p, rt))
				{
					return row_p;
				}

			FreeMemory (row_p);
		}

	return NULL;
}


bool InitBaseRow (BaseRow *row_p, bson_oid_t *id_p, const uint32 study_index, Plot *parent_plot_p, RowType rt)
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

	return true;
}


void ClearBaseRow (BaseRow *row_p)
{
	FreeBSONOid (row_p -> br_id_p);
}


void FreeBaseRow (BaseRow *row_p)
{
	ClearBaseRow (row_p);
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



bool AddBaseRowToJSON (const BaseRow *row_p, json_t *row_json_p, const ViewFormat format)
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
											success_flag = true;

										}		/* if (AddNamedCompoundIdToJSON (row_json_p, row_p -> ro_plot_p -> pl_id_p, RO_PLOT_ID_S)) */
									else
										{
											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, row_json_p, "Failed to add study id for row " UINT32_FMT " in study \"%s\"",
																				 row_p -> br_by_study_index, row_p -> ro_plot_p -> pl_parent_p -> st_name_s);
										}

								}		/* if (AddNamedCompoundIdToJSON (row_json_p, row_p -> ro_plot_p -> pl_id_p, RO_PLOT_ID_S)) */
							else
								{
									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, row_json_p, "Failed to add plot id for for row " UINT32_FMT " in study \"%s\"",
																		 row_p -> br_by_study_index, row_p -> ro_plot_p -> pl_parent_p -> st_name_s);
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

			if (success_flag)
				{
					switch (row_p -> br_type)
						{
							case RT_DISCARD:
								{
									if (!SetJSONBoolean (row_json_p, RO_DISCARD_S, true))
										{
											success_flag = false;
											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, row_json_p, "Failed to add \"%s\": true", RO_DISCARD_S);
										}
								}
								break;


							case RT_BLANK:
								{
									if (!SetJSONBoolean (row_json_p, RO_BLANK_S, true))
										{
											success_flag = false;
											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, row_json_p, "Failed to add \"%s\": true", RO_BLANK_S);
										}
								}
								break;

							case RT_NORMAL:
								{
									if (!AddNormalRowToJSON (row_p, row_json_p, format, data_p))
										{
											success_flag = false;
											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, row_json_p, "Failed to add row with id " UINT32_FMT, row_p -> ro_by_study_index);
										}
								}

								break;

							default:
								break;

						}		/* switch (row_p -> ro_type) */

					if (success_flag)
						{
							return row_json_p;
						}

				}		/* if (success_flag) */

			json_decref (row_json_p);


		}		/* if (SetJSONInteger (row_json_p, RO_STUDY_INDEX_S, row_p -> ro_by_study_index)) */
	else
		{
			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, row_json_p, "Failed to add \"%s\": " UINT32_FMT, BR_STUDY_INDEX_S, row_p -> ro_by_study_index);
		}

	return success_flag;
}


bool PopulateBaseRowFromJSON (BaseRow *row_p, const json_t *row_json_p)
{
	bool success_flag = false;

	return success_flag;
}


/*
 * base_row.h
 *
 *  Created on: 29 Jun 2022
 *      Author: billy
 */

#ifndef SERVICES_FIELD_TRIALS_INCLUDE_BASE_ROW_H_
#define SERVICES_FIELD_TRIALS_INCLUDE_BASE_ROW_H_


#include "bson/bson.h"

#include "dfw_field_trial_service_library.h"
#include "plot.h"

/*
 * Forward declarations
 */
struct Study;


typedef enum
{
	/**
	 * A normal plot which is part of an experiment.
	 */
	RT_NORMAL,

	/**
	 * A physical plot where no measurements are taken, just there as a physical spacer
	 * which will appear in the plot-based view.
	 */
	RT_DISCARD,

	/**
	 * Used to keep the stylised view when laying out the plots and you have non-regular gaps.
	 * Grassroots won't draw these plots and will use them for gaps calculations and draw
	 * gaps instead
	 */
	RT_BLANK,

	/**
	 * The number of different values that a RoType can take.
	 */
	RT_NUM_VALUES
} RowType;


typedef struct
{
	RowType br_type;

	bson_oid_t *br_id_p;

	const struct Study *br_study_p;

	struct Plot *br_plot_p;

	/**
	 * The unique index for this row within
	 * its parent study.
	 */
	uint32 br_by_study_index;

} BaseRow;


#ifndef DOXYGEN_SHOULD_SKIP_THIS

#ifdef ALLOCATE_BASE_ROW_TAGS
	#define BASE_ROW_PREFIX DFW_FIELD_TRIAL_SERVICE_API
	#define BASE_ROW_VAL(x)	= x
	#define BASE_ROW_CONCAT_VAL(x,y)	= x y
#else
	#define BASE_ROW_PREFIX extern
	#define BASE_ROW_VAL(x)
	#define BASE_ROW_CONCAT_VAL(x,y)
#endif

#endif 		/* #ifndef DOXYGEN_SHOULD_SKIP_THIS */




BASE_ROW_PREFIX const char *BR_ID_S BASE_ROW_VAL ("id");

BASE_ROW_PREFIX const char *BR_STUDY_INDEX_S BASE_ROW_VAL ("study_index");

BASE_ROW_PREFIX const char *BR_PLOT_ID_S BASE_ROW_VAL ("plot_id");

BASE_ROW_PREFIX const char *BR_STUDY_ID_S BASE_ROW_VAL ("study_id");


#ifdef __cplusplus
extern "C"
{
#endif



DFW_FIELD_TRIAL_SERVICE_LOCAL BaseRow *AllocateBlankRow (bson_oid_t *id_p, const uint32 study_index, Plot *parent_plot_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL BaseRow *AllocateDiscardRow (bson_oid_t *id_p, const uint32 study_index, Plot *parent_plot_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL BaseRow *AllocateBaseRow (bson_oid_t *id_p, const uint32 study_index, Plot *parent_plot_p, RowType rt);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool InitBaseRow (BaseRow *row_p, bson_oid_t *id_p, const uint32 study_index, Plot *parent_plot_p, RowType rt);


DFW_FIELD_TRIAL_SERVICE_LOCAL void ClearBaseRow (BaseRow *row_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL void FreeBaseRow (BaseRow *row_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL void SetRowType (BaseRow *row_p, RowType rt);


DFW_FIELD_TRIAL_SERVICE_LOCAL RowType GetRowType (const BaseRow *row_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool AddBaseRowToJSON (const BaseRow *row_p, json_t *row_json_p, const ViewFormat format);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool PopulateBaseRowFromJSON (BaseRow *row_p, const json_t *row_json_p);




#ifdef __cplusplus
}
#endif

#endif /* SERVICES_FIELD_TRIALS_INCLUDE_BASE_ROW_H_ */

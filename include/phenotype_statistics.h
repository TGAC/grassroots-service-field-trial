/*
 * measured_varaiable_statistics.h
 *
 *  Created on: 27 May 2022
 *      Author: billy
 */

#ifndef SERVICES_FIELD_TRIALS_INCLUDE_PHENOTYPE_STATISTICS_H_
#define SERVICES_FIELD_TRIALS_INCLUDE_PHENOTYPE_STATISTICS_H_


#include "statistics.h"
#include "measured_variable.h"
#include "dfw_field_trial_service_library.h"


/**
 * A datatype for having the statistics of a
 * given Measure Variable for a Study
 */
typedef struct
{
	/**
	 * The base list node.
	 */
	ListItem psn_node;

	/**
	 * The variable name of the MeasuredVariable that
	 * the Statistics are for.
	 * @see GetMeasuredVariableName()
	 */
	char *psn_measured_variable_name_s;

	/**
	 * The Study's Statistics for this MeasuredVariable.
	 */
	Statistics *psn_stats_p;

} PhenotypeStatisticsNode;



#ifdef __cplusplus
extern "C"
{
#endif


DFW_FIELD_TRIAL_SERVICE_LOCAL PhenotypeStatisticsNode *AllocatePhenotypeStatisticsNode (const char *measured_variable_name_s, const Statistics *src_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL void FreePhenotypeStatisticsNode (ListItem *psn_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool AddPhenotypeStatisticsNodeAsJSON (const PhenotypeStatisticsNode *psn_p, json_t *parent_p, const ViewFormat format, const FieldTrialServiceData *service_data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool AddPhenotypeStatisticsNodeFromJSON (LinkedList *nodes_p, const json_t *phenotype_p, const FieldTrialServiceData *service_data_p);

#ifdef __cplusplus
}
#endif



#endif /* SERVICES_FIELD_TRIALS_INCLUDE_PHENOTYPE_STATISTICS_H_ */

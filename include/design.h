/*
 * design.h
 *
 *  Created on: 6 Jul 2019
 *      Author: billy
 */

#ifndef SERVICES_FIELD_TRIALS_INCLUDE_DESIGN_H_
#define SERVICES_FIELD_TRIALS_INCLUDE_DESIGN_H_


typedef struct BlockingFactors
{
	char *bf_block_s;

	char *bf_whole_plot_s;

	char *bf_sub_plot_s;

	uint32 bf_row;

	uint32 bf_column;

} BlockingFactors;


typedef struct TreatmentFactors
{
	char *tf_nitrogen_exposure_s;

	char *tf_accession_s;

	char *tf_spatial_control_s;

} TreatmentFactors;



typedef struct TrialDesign
{
	BlockingFactors *td_blocking_factors_p;

	TreatmentFactors *td_treatment_factors_p;

} TrialDesign;


#endif /* SERVICES_FIELD_TRIALS_INCLUDE_DESIGN_H_ */

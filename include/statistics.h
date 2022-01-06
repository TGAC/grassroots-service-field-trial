/*
** Copyright 2014-2018 The Earlham Institute
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/
/*
 * statistics.h
 *
 *  Created on: 4 Jan 2022
 *      Author: billy
 */

#ifndef SERVICES_FIELD_TRIALS_INCLUDE_STATISTICS_H_
#define SERVICES_FIELD_TRIALS_INCLUDE_STATISTICS_H_

#include "typedefs.h"

/*
	Stats.afplot
	Stats.aplot
	Stats.bigcol
	Stats.bigrow
	Stats.block
	Stats.colblock
	Stats.esplots
	Stats.esspl
	Stats.ewplots
	Stats.lineplot
	Stats.MainPlot
	Stats.MPlotNo
	Stats.nest
	Stats.Plot
	Stats.plotblock
	Stats.PlotNo
	Stats.ps1
	Stats.ps2
	Stats.rep
	Stats.rowblock
	Stats.Sblock
	Stats.SplitPlot
	Stats.SplitSplitPlot
	Stats.Srow
	Stats.Stats Column
	Stats.Stats Replicate
	Stats.Stats Row
	Stats.subplot
	Stats.SubPlotNo
	Stats.supercol
	Stats.trial
	Stats.wholeplot
	Stats.Part of Statistical design
*/

typedef struct PlotStatistics
{
	int32 *ps_afplot_p;

	int32 *ps_aplot_p;

	int32 *ps_bigcol_p;

	int32 *ps_bigrow_p;

	int32 *ps_block_p;

	int32 *ps_colblock_p;

	int32 *ps_esplots_p;

	int32 *ps_esspl_p;

	int32 *ps_ewplots_p;

	int32 *ps_lineplot_p;

	int32 *ps_mainplot_p;

	int32 *ps_mplotno_p;

	int32 *ps_next_p;

	int32 *ps_plot_p;

	int32 *ps_plotblock_p;

	int32 *ps_plotno_p;

	int32 *ps_ps1_p;

	int32 *ps_ps2_p;

	int32 *ps_rep_p;

	int32 *ps_rowblock_p;

	int32 *ps_sblock_p;

	int32 *ps_splitplot_p;

	int32 *ps_splitsplitplot_p;

	int32 *ps_srow_p;

	int32 *ps_stats_column_p;

	int32 *ps_stats_replicate_p;

	int32 *ps_stats_row_p;

	int32 *ps_subplot_p;

	int32 *ps_subplotno_p;

	int32 *ps_supercol_p;

	int32 *ps_trial_p;

	int32 *ps_wholeplot_p;

	int32 *ps_part_of_statistical_design_p;

} PlotStatistics;

#endif /* SERVICES_FIELD_TRIALS_INCLUDE_STATISTICS_H_ */

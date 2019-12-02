/*
 * json_processor.c
 *
 *  Created on: 2 Dec 2019
 *      Author: billy
 */

#include "json_processor.h"
#include "plot.h"



void FreeJSONProcessor (struct JSONProcessor *processor_p)
{
	if (processor_p -> jp_free_fn)
		{
			processor_p -> jp_free_fn (processor_p);
		}
	else
		{
			FreeMemory (processor_p);
		}
}


json_t *ProcessPlotJSON (struct JSONProcessor *processor_p, Plot *plot_p, ViewFormat format, const DFWFieldTrialServiceData *service_data_p)
{
	json_t *plot_json_p = NULL;

	if ((processor_p) && (processor_p -> jp_process_plot_json_fn))
		{
			plot_json_p = processor_p -> jp_process_plot_json_fn (processor_p, plot_p, format, service_data_p);
		}
	else
		{
			plot_json_p = GetPlotAsJSON (plot_p, format, service_data_p);
		}

	return plot_json_p;
}

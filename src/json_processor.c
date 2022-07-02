/*
 * json_processor.c
 *
 *  Created on: 2 Dec 2019
 *      Author: billy
 */

#include "json_processor.h"
#include "plot.h"
#include "base_row.h"
#include "material.h"



void InitialiseJSONProcessor (struct JSONProcessor *processor_p,
															json_t *(*process_plot_json_fn) (struct JSONProcessor *processor_p, struct Plot *plot_p, ViewFormat format, const FieldTrialServiceData *service_data_p),
															json_t *(*process_row_json_fn) (struct JSONProcessor *processor_p, struct BaseRow *row_p, ViewFormat format, const FieldTrialServiceData *service_data_p),
															void (*free_fn) (struct JSONProcessor *processor_p))
{
	processor_p -> jp_process_row_json_fn = process_row_json_fn;
	processor_p -> jp_process_plot_json_fn = process_plot_json_fn;
	processor_p -> jp_free_fn = free_fn;
}


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


json_t *ProcessPlotJSON (struct JSONProcessor *processor_p, Plot *plot_p, ViewFormat format, const FieldTrialServiceData *service_data_p)
{
	json_t *plot_json_p = NULL;

	if ((processor_p) && (processor_p -> jp_process_plot_json_fn))
		{
			plot_json_p = processor_p -> jp_process_plot_json_fn (processor_p, plot_p, format, service_data_p);
		}
	else
		{
			plot_json_p = GetPlotAsJSON (plot_p, format, processor_p, service_data_p);
		}

	return plot_json_p;
}


json_t *ProcessRowJSON (struct JSONProcessor *processor_p, BaseRow *row_p, ViewFormat format, const FieldTrialServiceData *service_data_p)
{
	json_t *row_json_p = NULL;

	if ((processor_p) && (processor_p -> jp_process_row_json_fn))
		{
			row_json_p = processor_p -> jp_process_row_json_fn (processor_p, row_p, format, service_data_p);
		}
	else
		{
			row_json_p = GetBaseRowAsJSON (row_p, format, processor_p, service_data_p);
		}

	return row_json_p;
}



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
 * plot_jobs.c
 *
 *  Created on: 1 Oct 2018
 *      Author: billy
 */


#include "plot_jobs.h"
#include "plot.h"

static NamedParameterType S_PLOT_SOWING_DATE = { "PL Sowing Year", PT_TIME };
static NamedParameterType S_PLOT_HARVEST_DATE = { "PL Harvest Year", PT_TIME };
static NamedParameterType S_PLOT_WIDTH = { "PL Width", PT_UNSIGNED_REAL };
static NamedParameterType S_PLOT_HEIGHT = { "PL Height", PT_UNSIGNED_REAL };


static NamedParameterType S_PLOT_TRIAL_DESIGN = { "PL Trial Design", PT_STRING };
static NamedParameterType S_PLOT_GROWING_CONDITION = { "PL Growing Condition", PT_STRING };
static NamedParameterType S_PLOT_TREATMENT = { "PL Treatment", PT_STRING };

static NamedParameterType S_ADD_PLOT = { "Add Experimental Area", PT_BOOLEAN };
static NamedParameterType S_GET_ALL_PLOTS = { "Get all Experimental Areas", PT_BOOLEAN };

static NamedParameterType S_EXPERIMENTAL_AREAS_LIST = { "Field Trials", PT_STRING };


bool AddPlotParams (ServiceData *data_p, ParameterSet *param_set_p)
{
	bool success_flag = false;
	Parameter *param_p = NULL;
	SharedType def;
	ParameterGroup *group_p = CreateAndAddParameterGroupToParameterSet ("Experimental Area", NULL, false, data_p, param_set_p);

	def.st_ulong_value = 0;

	if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_PLOT_SOWING_DATE.npt_type, S_PLOT_SOWING_DATE.npt_name_s, "Sowing date", "The date when the seeds were sown", def, PL_BASIC)) != NULL)
		{
			if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_PLOT_HARVEST_DATE.npt_type, S_PLOT_HARVEST_DATE.npt_name_s, "Harvest date", "TThe date when the seeds were harvested", def, PL_BASIC)) != NULL)
				{
					if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_PLOT_WIDTH.npt_type, S_PLOT_WIDTH.npt_name_s, "Width", "The soil of the Experimental Area", def, PL_BASIC)) != NULL)
						{
							def.st_ulong_value = 2017;

							if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_EXPERIMENTAL_AREA_SOWING_YEAR.npt_type, S_EXPERIMENTAL_AREA_SOWING_YEAR.npt_name_s, "Year", "The sowing year of the Experimental Area", def, PL_BASIC)) != NULL)
								{
									if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_EXPERIMENTAL_AREA_HARVEST_YEAR.npt_type, S_EXPERIMENTAL_AREA_HARVEST_YEAR.npt_name_s, "Year", "The harvest year of the Experimental Area", def, PL_BASIC)) != NULL)
										{
											if (SetUpFieldTrialsListParameter ((DFWFieldTrialServiceData *) data_p, param_set_p, group_p))
												{
													if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_ADD_EXPERIMENTAL_AREA.npt_type, S_ADD_EXPERIMENTAL_AREA.npt_name_s, "Add", "Add a new Experimental Area", def, PL_BASIC)) != NULL)
														{
															if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_GET_ALL_EXPERIMENTAL_AREAS.npt_type, S_GET_ALL_EXPERIMENTAL_AREAS.npt_name_s, "List", "Get all of the existing Experimental Areas", def, PL_BASIC)) != NULL)
																{
																	success_flag = true;
																}
														}

												}
										}
								}
						}
				}
		}

	return success_flag;
}


bool RunForPlotParams (DFWFieldTrialServiceData *data_p, ParameterSet *param_set_p, ServiceJob *job_p)
{
	bool job_done_flag = false;

	return job_done_flag;
}

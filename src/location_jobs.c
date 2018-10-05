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
 * location_jobs.c
 *
 *  Created on: 5 Oct 2018
 *      Author: billy
 */



#include "location_jobs.h"

/*
 * Experimental Area parameters
 */
static NamedParameterType S_LOCATION_NAME = { "LO Name", PT_STRING };
static NamedParameterType S_LOCATION_STREET = { "LO Street", PT_STRING };
static NamedParameterType S_LOCATION_TOWN = { "LO Town", PT_STRING };
static NamedParameterType S_LOCATION_COUNTY = { "LO County", PT_STRING };
static NamedParameterType S_LOCATION_COUNTRY = { "LO Country", PT_STRING };
static NamedParameterType S_LOCATION_POSTCODE = { "LO Postcode", PT_STRING };
static NamedParameterType S_LOCATION_USE_GPS = { "LO Use GPS", PT_BOOLEAN };
static NamedParameterType S_LOCATION_LATITUDE = { "LO Latitude", PT_SIGNED_REAL };
static NamedParameterType S_LOCATION_LONGITUDE = { "LO Longitude", PT_SIGNED_REAL };
static NamedParameterType S_LOCATION_ALTITUDE = { "LO Altitude", PT_SIGNED_REAL };


bool AddLocationParams (ServiceData *data_p, ParameterSet *param_set_p)
{
	bool success_flag = false;
	Parameter *param_p = NULL;
	SharedType def;
	ParameterGroup *group_p = CreateAndAddParameterGroupToParameterSet ("Location", NULL, false, data_p, param_set_p);

	def.st_string_value_s = NULL;

	if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_LOCATION_NAME.npt_type, S_LOCATION_NAME.npt_name_s, "Name", "The building name or number", def, PL_BASIC)) != NULL)
		{
			if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_LOCATION_STREET.npt_type, S_LOCATION_STREET.npt_name_s, "Street", "The street", def, PL_BASIC)) != NULL)
				{
					if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_LOCATION_TOWN.npt_type, S_LOCATION_TOWN.npt_name_s, "Town", "The town, city or village", def, PL_BASIC)) != NULL)
						{
							if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_LOCATION_COUNTY.npt_type, S_LOCATION_COUNTY.npt_name_s, "County", "The county or state", def, PL_BASIC)) != NULL)
								{
									if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_LOCATION_COUNTRY.npt_type, S_LOCATION_COUNTRY.npt_name_s, "Country", "The country", def, PL_BASIC)) != NULL)
										{
											if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_LOCATION_POSTCODE.npt_type, S_LOCATION_POSTCODE.npt_name_s, "Postal code", "The postcode", def, PL_BASIC)) != NULL)
												{
													def.st_boolean_value = true;

													if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_LOCATION_USE_GPS.npt_type, S_LOCATION_USE_GPS.npt_name_s, "Use given GPS", "Use the given GPS values, uncheck this to look up the GPS values using the location instead", def, PL_BASIC)) != NULL)
														{
															def.st_data_value = 0.0;

															if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_LOCATION_LATITUDE.npt_type, S_LOCATION_LATITUDE.npt_name_s, "Latitude", "The latitude of the Experimental Area", def, PL_BASIC)) != NULL)
																{
																	const char *precision_s = "6";

																	if (AddParameterKeyValuePair (param_p, PA_DOUBLE_PRECISION_S, precision_s))
																		{
																			if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_LOCATION_LONGITUDE.npt_type, S_LOCATION_LONGITUDE.npt_name_s, "Longitude", "The longitude of the Experimental Area", def, PL_BASIC)) != NULL)
																				{
																					if (AddParameterKeyValuePair (param_p, PA_DOUBLE_PRECISION_S, precision_s))
																						{
																							if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_LOCATION_ALTITUDE.npt_type, S_LOCATION_ALTITUDE.npt_name_s, "Altitude", "The altitude of the Experimental Area", def, PL_BASIC)) != NULL)
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
								}
						}
				}
		}

	return false;
}


bool RunForLocationParams (DFWFieldTrialServiceData *data_p, ParameterSet *param_set_p, ServiceJob *job_p)
{
	return false;
}

Address *GetAddressFromLocationString (const char *location_s)
{
	return NULL;
}

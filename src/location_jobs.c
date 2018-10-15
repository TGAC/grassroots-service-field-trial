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
#include "location.h"
#include "geocoder_util.h"
#include "string_utils.h"


static const char *S_DEFAULT_COORD_PRECISION_S = "6";

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


static NamedParameterType S_ADD_LOCATION = { "Add Location", PT_BOOLEAN };
static NamedParameterType S_GET_ALL_LOCATIONS = { "Get all Locations", PT_BOOLEAN };


/*
 * static declarations
 */
static bool AddLocation (ServiceJob *job_p, ParameterSet *param_set_p, DFWFieldTrialServiceData *data_p);



bool AddLocationParams (ServiceData *data_p, ParameterSet *param_set_p)
{
	bool success_flag = false;
	Parameter *param_p = NULL;
	SharedType def;
	ParameterGroup *group_p = CreateAndAddParameterGroupToParameterSet ("Location", NULL, false, data_p, param_set_p);

	def.st_string_value_s = NULL;

	if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_LOCATION_NAME.npt_type, S_LOCATION_NAME.npt_name_s, "Name", "The building name or number", def, PL_ALL)) != NULL)
		{
			if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_LOCATION_STREET.npt_type, S_LOCATION_STREET.npt_name_s, "Street", "The street", def, PL_ALL)) != NULL)
				{
					if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_LOCATION_TOWN.npt_type, S_LOCATION_TOWN.npt_name_s, "Town", "The town, city or village", def, PL_ALL)) != NULL)
						{
							if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_LOCATION_COUNTY.npt_type, S_LOCATION_COUNTY.npt_name_s, "County", "The county or state", def, PL_ALL)) != NULL)
								{
									def.st_string_value_s = (char *) "GB";

									if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_LOCATION_COUNTRY.npt_type, S_LOCATION_COUNTRY.npt_name_s, "Country", "The country", def, PL_ALL)) != NULL)
										{
											def.st_string_value_s = NULL;

											if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_LOCATION_POSTCODE.npt_type, S_LOCATION_POSTCODE.npt_name_s, "Postal code", "The postcode", def, PL_ALL)) != NULL)
												{
													def.st_boolean_value = true;

													if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_LOCATION_USE_GPS.npt_type, S_LOCATION_USE_GPS.npt_name_s, "Use given GPS", "Use the given GPS values, uncheck this to look up the GPS values using the location instead", def, PL_ALL)) != NULL)
														{
															def.st_data_value = 0.0;

															if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_LOCATION_LATITUDE.npt_type, S_LOCATION_LATITUDE.npt_name_s, "Latitude", "The latitude of the location", def, PL_ALL)) != NULL)
																{
																	const char *precision_s = S_DEFAULT_COORD_PRECISION_S;

																	if (AddParameterKeyValuePair (param_p, PA_DOUBLE_PRECISION_S, precision_s))
																		{
																			if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_LOCATION_LONGITUDE.npt_type, S_LOCATION_LONGITUDE.npt_name_s, "Longitude", "The longitude of the location", def, PL_ALL)) != NULL)
																				{
																					if (AddParameterKeyValuePair (param_p, PA_DOUBLE_PRECISION_S, precision_s))
																						{
																							if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_LOCATION_ALTITUDE.npt_type, S_LOCATION_ALTITUDE.npt_name_s, "Altitude", "The altitude of the location", def, PL_ALL)) != NULL)
																								{
																									if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_ADD_LOCATION.npt_type, S_ADD_LOCATION.npt_name_s, "Add", "Add a new Location", def, PL_ALL)) != NULL)
																										{
																											if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_GET_ALL_LOCATIONS.npt_type, S_GET_ALL_LOCATIONS.npt_name_s, "List", "Get all of the existing locations", def, PL_ALL)) != NULL)
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
				}
		}

	return success_flag;
}


bool RunForLocationParams (DFWFieldTrialServiceData *data_p, ParameterSet *param_set_p, ServiceJob *job_p)
{
	bool job_done_flag = false;
	SharedType value;
	InitSharedType (&value);

	if (GetParameterValueFromParameterSet (param_set_p, S_ADD_LOCATION.npt_name_s, &value, true))
		{
			if (value.st_boolean_value)
				{
					bool success_flag = AddLocation (job_p, param_set_p, data_p);

					job_done_flag = true;
				}		/* if (value.st_boolean_value) */

		}		/* if (GetParameterValueFromParameterSet (param_set_p, S_ADD_LOCATION.npt_name_s, &value, true)) */

	return job_done_flag;
}


Address *GetAddressFromLocationString (const char *location_s)
{
	return NULL;
}



static bool AddLocation (ServiceJob *job_p, ParameterSet *param_set_p, DFWFieldTrialServiceData *data_p)
{
	bool success_flag = false;
	SharedType name_value;
	InitSharedType (&name_value);

	if (GetParameterValueFromParameterSet (param_set_p, S_LOCATION_NAME.npt_name_s, &name_value, true))
		{
			SharedType street_value;
			InitSharedType (&street_value);

			if (GetParameterValueFromParameterSet (param_set_p, S_LOCATION_STREET.npt_name_s, &street_value, true))
				{
					SharedType town_value;
					InitSharedType (&town_value);

					if (GetParameterValueFromParameterSet (param_set_p, S_LOCATION_TOWN.npt_name_s, &town_value, true))
						{
							SharedType county_value;
							InitSharedType (&county_value);

							if (GetParameterValueFromParameterSet (param_set_p, S_LOCATION_COUNTY.npt_name_s, &county_value, true))
								{
									SharedType country_value;
									InitSharedType (&country_value);

									if (GetParameterValueFromParameterSet (param_set_p, S_LOCATION_COUNTRY.npt_name_s, &country_value, true))
										{
											SharedType postcode_value;
											InitSharedType (&postcode_value);

											if (GetParameterValueFromParameterSet (param_set_p, S_LOCATION_POSTCODE.npt_name_s, &postcode_value, true))
												{
													SharedType use_gps_value;
													InitSharedType (&use_gps_value);

													if (GetParameterValueFromParameterSet (param_set_p, S_LOCATION_USE_GPS.npt_name_s, &use_gps_value, true))
														{
															const char *country_code_s = NULL;
															const char *gps_s = NULL;

															Address *address_p = AllocateAddress (name_value.st_string_value_s, street_value.st_string_value_s, town_value.st_string_value_s, county_value.st_string_value_s,
																																		country_value.st_string_value_s, postcode_value.st_string_value_s, country_code_s, gps_s);

															if (address_p)
																{
																	const uint32 order = 0;
																	ExperimentalArea *area_p = NULL;
																	bson_oid_t *id_p = NULL;

																	if (use_gps_value.st_boolean_value)
																		{
																			SharedType latitude_value;
																			InitSharedType (&latitude_value);

																			if (GetParameterValueFromParameterSet (param_set_p, S_LOCATION_LATITUDE.npt_name_s, &latitude_value, true))
																				{
																					SharedType longitude_value;
																					InitSharedType (&longitude_value);

																					if (GetParameterValueFromParameterSet (param_set_p, S_LOCATION_LONGITUDE.npt_name_s, &longitude_value, true))
																						{
																							double64 *elevation_p = NULL;
																							SharedType elevation_value;
																							InitSharedType (&elevation_value);

																							if (GetParameterValueFromParameterSet (param_set_p, S_LOCATION_ALTITUDE.npt_name_s, &elevation_value, true))
																								{
																									elevation_p = elevation_value.st_data_value;
																								}


																							if (SetAddressCentreCoordinate (address_p, latitude_value.st_data_value, longitude_value.st_data_value, elevation_p))
																								{
																									success_flag = true;
																								}
																						}

																				}
																		}
																	else
																		{
																			success_flag = DetermineGPSLocationForAddress (address_p, NULL);
																		}

																	if (success_flag)
																		{
																			Location *location_p = AllocateLocation (address_p, order, id_p);

																			if (location_p)
																				{
																					success_flag = SaveLocation (location_p, data_p);
																				}
																			else
																				{
																					FreeAddress (address_p);
																					success_flag = false;
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

	SetServiceJobStatus (job_p, success_flag ? OS_SUCCEEDED : OS_FAILED);

	return success_flag;
}


bool SetUpLocationsListParameter (const DFWFieldTrialServiceData *data_p, Parameter *param_p)
{
	bool success_flag = false;

	if (SetMongoToolCollection (data_p -> dftsd_mongo_p, data_p -> dftsd_collection_ss [DFTD_LOCATION]))
		{
			bson_t *query_p = NULL;
			bson_t *opts_p =  NULL; // BCON_NEW ( "sort", "{", "name", BCON_INT32 (1), "}");
			json_t *results_p = GetAllMongoResultsAsJSON (data_p -> dftsd_mongo_p, query_p, opts_p);

			if (results_p)
				{
					if (json_is_array (results_p))
						{
							const size_t num_results = json_array_size (results_p);

							if (num_results > 0)
								{
									size_t i = 0;
									json_t *entry_p = json_array_get (results_p, i);
									Location *location_p = GetLocationFromJSON (entry_p, data_p);

									if (location_p)
										{
											char *name_s = GetLocationAsString (location_p);

											if (name_s)
												{
													SharedType def;
													char *id_s = GetBSONOidAsString (location_p -> lo_id_p);

													if (id_s)
														{
															def.st_string_value_s = id_s;

															if (SetParameterValueFromSharedType (param_p, &def, false))
																{
																	if (SetParameterValueFromSharedType (param_p, &def, true))
																		{
																			success_flag = CreateAndAddParameterOptionToParameter (param_p, def, name_s);
																		}
																}

															FreeCopiedString (id_s);
														}

													FreeCopiedString (name_s);
												}

											FreeLocation (location_p);
										}		/* if (location_p) */

									if (success_flag)
										{
											for (++ i; i < num_results; ++ i)
												{
													entry_p = json_array_get (results_p, i);
													location_p = GetLocationFromJSON (entry_p, data_p);

													if (location_p)
														{
															char *name_s = GetLocationAsString (location_p);

															if (name_s)
																{
																	SharedType def;
																	char *id_s = GetBSONOidAsString (location_p -> lo_id_p);

																	if (id_s)
																		{
																			def.st_string_value_s = id_s;

																			if (param_p)
																				{
																					success_flag = CreateAndAddParameterOptionToParameter (param_p, def, name_s);
																				}

																			FreeCopiedString (id_s);
																		}

																	FreeCopiedString (name_s);
																}		/* if (name_s) */

															FreeLocation (location_p);
														}		/* if (location_p) */

												}		/* for (++ i; i < num_results; ++ i) */

											if (!success_flag)
												{
													FreeParameter (param_p);
													param_p = NULL;
												}

										}		/* if (param_p) */

								}		/* if (num_results > 0) */
							else
								{
									/* nothing to add */
									success_flag = true;
								}

						}		/* if (json_is_array (results_p)) */

				}		/* if (results_p) */

		}		/* if (SetMongoToolCollection (data_p -> dftsd_mongo_p, data_p -> dftsd_collection_ss [DFTD_LOCATION])) */

	return success_flag;
}


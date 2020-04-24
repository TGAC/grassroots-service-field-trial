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


#define ALLOCATE_LOCATION_JOB_CONSTANTS (1)
#include "/location_jobs.h"
#include "geocoder_util.h"
#include "string_utils.h"
#include "/dfw_util.h"

#include "boolean_parameter.h"
#include "double_parameter.h"


static const char *DEFAULT_COORD_PRECISION_S = "6";

static const char * const S_UNSET_LOCATION_S = "Any Location";

static const char * const S_EMPTY_LIST_OPTION_S = "<empty>";


/*
 * Experimental Area parameters
 */
/*
static NamedParameterType LOCATION_NAME = { "LO Name", PT_STRING };
static NamedParameterType LOCATION_STREET = { "LO Street", PT_STRING };
static NamedParameterType LOCATION_TOWN = { "LO Town", PT_STRING };
static NamedParameterType LOCATION_COUNTY = { "LO County", PT_STRING };
static NamedParameterType LOCATION_COUNTRY = { "LO Country", PT_STRING };
static NamedParameterType LOCATION_POSTCODE = { "LO Postcode", PT_STRING };
static NamedParameterType LOCATION_USE_GPS = { "LO Use GPS", PT_BOOLEAN };
static NamedParameterType LOCATION_LATITUDE = { "LO Latitude", PT_SIGNED_REAL };
static NamedParameterType LOCATION_LONGITUDE = { "LO Longitude", PT_SIGNED_REAL };
static NamedParameterType LOCATION_ALTITUDE = { "LO Altitude", PT_SIGNED_REAL };
*/

static NamedParameterType S_ADD_LOCATION = { "Add Location", PT_BOOLEAN };
static NamedParameterType S_GET_ALL_LOCATIONS_PAGE_SIZE = { "Page size", PT_UNSIGNED_INT };
static NamedParameterType S_GET_ALL_LOCATIONS_PAGE_NUMBER = { "Page number", PT_UNSIGNED_INT };


/*
 * static declarations
 */
static bool AddLocation (ServiceJob *job_p, ParameterSet *param_set_p, FieldTrialServiceData *data_p);



bool AddSubmissionLocationParams (ServiceData *data_p, ParameterSet *param_set_p, Resource *resource_p)
{
	bool success_flag = false;
	Parameter *param_p = NULL;
	ParameterGroup *group_p = CreateAndAddParameterGroupToParameterSet ("Location", false, data_p, param_set_p);
	FieldTrialServiceData *dfw_data_p = (FieldTrialServiceData *) data_p;
	const char * const def_country_s = "GB";
	const bool def_use_coords_flag = true;
	char *id_s = NULL;
	const char *name_s = NULL;
	const char *street_s = NULL;
	const char *town_s = NULL;
	const char *county_s = NULL;
	const char *country_s = def_country_s;
	const char *post_code_s = NULL;
	const double64 *latitude_p = NULL;
	const double64 *longitude_p = NULL;
	const double64 *altitude_p = NULL;
	const bool *use_coords_p = &def_use_coords_flag;
	Location *active_location_p = GetLocationFromResource (resource_p, LOCATION_ID, dfw_data_p);
	bool defaults_flag = false;


	if (active_location_p && (active_location_p -> lo_address_p))
		{
			id_s = GetBSONOidAsString (active_location_p -> lo_id_p);

			if (id_s)
				{
					const Address *address_p = active_location_p -> lo_address_p;

					name_s = address_p -> ad_name_s;
					street_s = address_p -> ad_street_s;
					town_s = address_p -> ad_town_s;
					county_s = address_p -> ad_county_s;
					country_s = address_p -> ad_country_code_s;
					post_code_s = address_p -> ad_postcode_s;

					if (address_p -> ad_gps_centre_p)
						{
							latitude_p = & (address_p -> ad_gps_centre_p -> co_x);
							longitude_p = & (address_p -> ad_gps_centre_p -> co_y);
							use_coords_p = &def_use_coords_flag;
						}

					altitude_p = address_p -> ad_elevation_p;

					defaults_flag = true;
				}
		}
	else
		{
			defaults_flag = true;
		}

	if (defaults_flag)
		{
			if ((param_p = EasyCreateAndAddStringParameterToParameterSet (data_p, param_set_p, group_p, LOCATION_ID.npt_type, LOCATION_ID.npt_name_s, "Load Location", "Edit an existing location", id_s, PL_ALL)) != NULL)
				{
					if (SetUpLocationsListParameter (dfw_data_p, (StringParameter *) param_p, active_location_p, S_EMPTY_LIST_OPTION_S))
						{
							/*
							 * We want to update all of the values in the form
							 * when a user selects a study from the list so
							 * we need to make the parameter automatically
							 * refresh the values. So we set the
							 * pa_refresh_service_flag to true.
							 */
							param_p -> pa_refresh_service_flag = true;


							if ((param_p = EasyCreateAndAddStringParameterToParameterSet (data_p, param_set_p, group_p, LOCATION_NAME.npt_type, LOCATION_NAME.npt_name_s, "Name", "The building name or number", name_s, PL_ALL)) != NULL)
								{
									param_p -> pa_required_flag = true;

									if ((param_p = EasyCreateAndAddStringParameterToParameterSet (data_p, param_set_p, group_p, LOCATION_STREET.npt_type, LOCATION_STREET.npt_name_s, "Street", "The street", street_s, PL_ALL)) != NULL)
										{
											if ((param_p = EasyCreateAndAddStringParameterToParameterSet (data_p, param_set_p, group_p, LOCATION_TOWN.npt_type, LOCATION_TOWN.npt_name_s, "Town", "The town, city or village", town_s, PL_ALL)) != NULL)
												{
													if ((param_p = EasyCreateAndAddStringParameterToParameterSet (data_p, param_set_p, group_p, LOCATION_COUNTY.npt_type, LOCATION_COUNTY.npt_name_s, "County", "The county or state", county_s, PL_ALL)) != NULL)
														{

															if ((param_p = EasyCreateAndAddStringParameterToParameterSet (data_p, param_set_p, group_p, LOCATION_COUNTRY.npt_type, LOCATION_COUNTRY.npt_name_s, "Country", "The country", country_s, PL_ALL)) != NULL)
																{
																	if ((param_p = EasyCreateAndAddStringParameterToParameterSet (data_p, param_set_p, group_p, LOCATION_POSTCODE.npt_type, LOCATION_POSTCODE.npt_name_s, "Postal code", "The postcode", post_code_s, PL_ALL)) != NULL)
																		{
																			if ((param_p = EasyCreateAndAddBooleanParameterToParameterSet (data_p, param_set_p, group_p, LOCATION_USE_GPS.npt_name_s, "Supply your own GPS coordinates", "Use your own GPS values, uncheck this to look up the GPS values using the location instead", use_coords_p, PL_ALL)) != NULL)
																				{
																					if ((param_p = EasyCreateAndAddDoubleParameterToParameterSet (data_p, param_set_p, group_p, LOCATION_LATITUDE.npt_type, LOCATION_LATITUDE.npt_name_s, "Latitude", "The latitude of the location", latitude_p, PL_ALL)) != NULL)
																						{
																							const char *precision_s = DEFAULT_COORD_PRECISION_S;

																							if (AddParameterKeyStringValuePair (param_p, PA_DOUBLE_PRECISION_S, precision_s))
																								{
																									if ((param_p = EasyCreateAndAddDoubleParameterToParameterSet (data_p, param_set_p, group_p, LOCATION_LONGITUDE.npt_type, LOCATION_LONGITUDE.npt_name_s, "Longitude", "The longitude of the location", longitude_p, PL_ALL)) != NULL)
																										{
																											if (AddParameterKeyStringValuePair (param_p, PA_DOUBLE_PRECISION_S, precision_s))
																												{
																													if ((param_p = EasyCreateAndAddDoubleParameterToParameterSet (data_p, param_set_p, group_p, LOCATION_ALTITUDE.npt_type, LOCATION_ALTITUDE.npt_name_s, "Altitude", "The altitude of the location", altitude_p, PL_ALL)) != NULL)
																														{
																															success_flag = true;
																														}
																													else
																														{
																															PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", LOCATION_ALTITUDE.npt_name_s);
																														}
																												}
																											else
																												{
																													PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "AddParameterKeyValuePair failed for \"%s\": \"%s\"", PA_DOUBLE_PRECISION_S, precision_s);
																												}
																										}
																									else
																										{
																											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", LOCATION_LATITUDE.npt_name_s);
																										}
																								}
																							else
																								{
																									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "AddParameterKeyValuePair failed for \"%s\": \"%s\"", PA_DOUBLE_PRECISION_S, precision_s);
																								}
																						}
																					else
																						{
																							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", LOCATION_LATITUDE.npt_name_s);
																						}
																				}
																			else
																				{
																					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", LOCATION_USE_GPS.npt_name_s);
																				}
																		}
																	else
																		{
																			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", LOCATION_POSTCODE.npt_name_s);
																		}
																}
															else
																{
																	PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", LOCATION_COUNTRY.npt_name_s);
																}
														}
													else
														{
															PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", LOCATION_COUNTY.npt_name_s);
														}
												}
											else
												{
													PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", LOCATION_TOWN.npt_name_s);
												}
										}
									else
										{
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", LOCATION_STREET.npt_name_s);
										}
								}
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", LOCATION_NAME.npt_name_s);
								}

						}

				}		/* if ((param_p = EasyCreateAndAddStringParameterToParameterSet (data_p, param_set_p, group_p, LOCATION_ID.npt_type, LOCATION_ID.npt_name_s, "Load Location", "Edit an existing location", id_s, PL_ADVANCED)) != NULL) */





		}		/* if (defaults_flag) */


	return success_flag;
}


bool RunForSubmissionLocationParams (FieldTrialServiceData *data_p, ParameterSet *param_set_p, ServiceJob *job_p)
{
	bool success_flag = AddLocation (job_p, param_set_p, data_p);

	return success_flag;
}


bool GetSubmissionLocationParameterTypeForNamedParameter (const char *param_name_s, ParameterType *pt_p)
{
	bool success_flag = true;

	if (strcmp (param_name_s, LOCATION_ID.npt_name_s) == 0)
		{
			*pt_p = LOCATION_ID.npt_type;
		}
	else if (strcmp (param_name_s, LOCATION_NAME.npt_name_s) == 0)
		{
			*pt_p = LOCATION_NAME.npt_type;
		}
	else if (strcmp (param_name_s, LOCATION_STREET.npt_name_s) == 0)
		{
			*pt_p = LOCATION_STREET.npt_type;
		}
	else if (strcmp (param_name_s, LOCATION_TOWN.npt_name_s) == 0)
		{
			*pt_p = LOCATION_TOWN.npt_type;
		}
	else if (strcmp (param_name_s, LOCATION_COUNTY.npt_name_s) == 0)
		{
			*pt_p = LOCATION_COUNTY.npt_type;
		}
	else if (strcmp (param_name_s, LOCATION_COUNTRY.npt_name_s) == 0)
		{
			*pt_p = LOCATION_COUNTRY.npt_type;
		}
	else if (strcmp (param_name_s, LOCATION_POSTCODE.npt_name_s) == 0)
		{
			*pt_p = LOCATION_POSTCODE.npt_type;
		}
	else if (strcmp (param_name_s, LOCATION_USE_GPS.npt_name_s) == 0)
		{
			*pt_p = LOCATION_USE_GPS.npt_type;
		}
	else if (strcmp (param_name_s, LOCATION_LATITUDE.npt_name_s) == 0)
		{
			*pt_p = LOCATION_LATITUDE.npt_type;
		}
	else if (strcmp (param_name_s, LOCATION_LONGITUDE.npt_name_s) == 0)
		{
			*pt_p = LOCATION_LONGITUDE.npt_type;
		}
	else if (strcmp (param_name_s, LOCATION_ALTITUDE.npt_name_s) == 0)
		{
			*pt_p = LOCATION_ALTITUDE.npt_type;
		}
	else if (strcmp (param_name_s, S_ADD_LOCATION.npt_name_s) == 0)
		{
			*pt_p = S_ADD_LOCATION.npt_type;
		}
	else if (strcmp (param_name_s, LOCATION_GET_ALL_LOCATIONS.npt_name_s) == 0)
		{
			*pt_p = LOCATION_GET_ALL_LOCATIONS.npt_type;
		}
	else
		{
			success_flag = false;
		}

	return success_flag;
}




bool AddSearchLocationParams (ServiceData *data_p, ParameterSet *param_set_p)
{
	bool success_flag = false;
	Parameter *param_p = NULL;
	ParameterGroup *group_p = CreateAndAddParameterGroupToParameterSet ("Location", false, data_p, param_set_p);
	bool list_flag = false;

	if ((param_p = EasyCreateAndAddBooleanParameterToParameterSet (data_p, param_set_p, group_p, LOCATION_GET_ALL_LOCATIONS.npt_name_s, "List", "Get all of the existing locations", &list_flag, PL_ADVANCED)) != NULL)
		{
			if ((param_p = EasyCreateAndAddStringParameterToParameterSet (data_p, param_set_p, group_p, LOCATION_ID.npt_type, LOCATION_ID.npt_name_s, "Id", "The id of the Location", NULL, PL_ADVANCED)) != NULL)
				{
					success_flag = true;
				}
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", LOCATION_ID.npt_name_s);
				}
		}
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", LOCATION_GET_ALL_LOCATIONS.npt_name_s);
		}

	return success_flag;
}


bool GetSearchLocationParameterTypeForNamedParameter (const char *param_name_s, ParameterType *pt_p)
{
	bool success_flag = true;

	if (strcmp (param_name_s, LOCATION_GET_ALL_LOCATIONS.npt_name_s) == 0)
		{
			*pt_p = LOCATION_GET_ALL_LOCATIONS.npt_type;
		}
	else if (strcmp (param_name_s, LOCATION_ID.npt_name_s) == 0)
		{
			*pt_p = LOCATION_ID.npt_type;
		}
	else
		{
			success_flag = false;
		}

	return success_flag;
}


bool RunForSearchLocationParams (FieldTrialServiceData *data_p, ParameterSet *param_set_p, ServiceJob *job_p)
{
	bool job_done_flag = false;
	const bool *get_all_flag_p = NULL;

	if (GetCurrentBooleanParameterValueFromParameterSet (param_set_p, LOCATION_GET_ALL_LOCATIONS.npt_name_s, &get_all_flag_p))
		{
			if ((get_all_flag_p != NULL) && (*get_all_flag_p == true))
				{
					bson_t *opts_p =  BCON_NEW ( "sort", "{", "name", BCON_INT32 (1), "}");
					json_t *db_results_p = GetAllLocationsAsJSON (data_p, opts_p);

					if (db_results_p)
						{
							size_t num_results = json_array_size (db_results_p);
							size_t i;
							size_t num_added = 0;
							OperationStatus status = OS_FAILED;

							for (i = 0; i < num_results; ++ i)
								{
									json_t *db_result_p = json_array_get (db_results_p, i);
									Location *location_p = GetLocationFromJSON (db_result_p, data_p);

									if (location_p)
										{
											json_t *job_result_p = GetResourceAsJSONByParts (PROTOCOL_INLINE_S, NULL, location_p -> lo_address_p -> ad_name_s, db_result_p);

											if (job_result_p)
												{
													if (AddResultToServiceJob (job_p, job_result_p))
														{
															++ num_added;
														}		/* if (AddResultToServiceJob (job_p, job_result_p)) */
													else
														{
															PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, job_result_p, "Failed to add result " SIZET_FMT " to ServiceJob", i);

															json_decref (job_result_p);
														}
												}		/* if (job_result_p) */
											else
												{
													PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, db_result_p, "Failed to create result " SIZET_FMT " to ServiceJob", i);
												}

											FreeLocation (location_p);
										}		/* if (location_p) */
									else
										{
											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, db_result_p, "Failed to create location " SIZET_FMT, i);
										}

								}		/* for (i = 0; i < num_results; ++ i) */

							if (num_added == num_results)
								{
									status = OS_SUCCEEDED;
								}
							else if (num_added > 0)
								{
									status = OS_PARTIALLY_SUCCEEDED;
								}

							SetServiceJobStatus (job_p, status);

							json_decref (db_results_p);
						}		/* if (db_results_p) */

					if (opts_p)
						{
							bson_destroy (opts_p);
						}

					job_done_flag = true;
				}		/* if (value.st_boolean_value) */

		}		/* if (GetParameterValueFromParameterSet (param_set_p, S_ADD_LOCATION.npt_name_s, &value, true)) */

	if (!job_done_flag)
		{
			const char *id_s = NULL;

			if (GetCurrentStringParameterValueFromParameterSet (param_set_p, LOCATION_ID.npt_name_s, &id_s))
				{
					OperationStatus status = OS_FAILED;

					if (!IsStringEmpty (id_s))
						{
							Location *location_p = GetLocationByIdString (id_s, VF_CLIENT_FULL, data_p);

							if (location_p)
								{
									json_t *location_json_p = GetLocationAsJSON (location_p);

									if (location_json_p)
										{
											json_t *job_result_p = GetResourceAsJSONByParts (PROTOCOL_INLINE_S, NULL, location_p -> lo_address_p -> ad_name_s, location_json_p);

											if (job_result_p)
												{
													if (AddResultToServiceJob (job_p, job_result_p))
														{
															status = OS_SUCCEEDED;
														}
													else
														{
															PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, job_result_p, "Failed to add result to ServiceJob");

															json_decref (job_result_p);
														}
												}		/* if (job_result_p) */
											else
												{
													PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, location_json_p, "Failed to create result for Location \"%s\"", location_p -> lo_address_p -> ad_name_s);
												}
										}
									else
										{
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get Location \"%s\" as JSON", location_p -> lo_address_p -> ad_name_s);
										}

									FreeLocation (location_p);
								}
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to find location \"%s\"", id_s);
								}

							job_done_flag = true;
						}
					else
						{
							status = OS_FAILED_TO_START;
						}

					SetServiceJobStatus (job_p, status);
				}
		}

	return job_done_flag;
}



const char *GetUnsetLocationValue (void)
{
	return S_UNSET_LOCATION_S;
}



Address *GetAddressFromLocationString (const char *location_s)
{
	return NULL;
}



static bool AddLocation (ServiceJob *job_p, ParameterSet *param_set_p, FieldTrialServiceData *data_p)
{
	OperationStatus status = OS_FAILED;
	bool success_flag = false;
	const char *name_s = NULL;
	const char *id_s = NULL;
	bson_oid_t *id_p = NULL;

	/*
	 * Get the existing location id if specified
	 */
	GetCurrentStringParameterValueFromParameterSet (param_set_p, LOCATION_ID.npt_name_s, &id_s);

	if (id_s)
		{
			if (strcmp (S_EMPTY_LIST_OPTION_S, id_s) != 0)
				{
					id_p = GetBSONOidFromString (id_s);

					if (!id_p)
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to load location \"%s\" for editing", id_s);
							return false;
						}
				}
		}		/* if (id_value.st_string_value_s) */


	if (GetCurrentStringParameterValueFromParameterSet (param_set_p, LOCATION_NAME.npt_name_s, &name_s))
		{
			Address *address_p = NULL;
			const char *street_s = NULL;
			const char *town_s = NULL;
			const char *county_s = NULL;
			const char *country_s = NULL;
			const char *postcode_s = NULL;
			const bool *use_gps_flag_p = NULL;
			const char *country_code_s = NULL;
			const char *gps_s = NULL;

			GetCurrentStringParameterValueFromParameterSet (param_set_p, LOCATION_STREET.npt_name_s, &street_s);
			GetCurrentStringParameterValueFromParameterSet (param_set_p, LOCATION_TOWN.npt_name_s, &town_s);
			GetCurrentStringParameterValueFromParameterSet (param_set_p, LOCATION_COUNTY.npt_name_s, &county_s);
			GetCurrentStringParameterValueFromParameterSet (param_set_p, LOCATION_COUNTRY.npt_name_s, &country_s);
			GetCurrentStringParameterValueFromParameterSet (param_set_p, LOCATION_POSTCODE.npt_name_s, &postcode_s);
			GetCurrentBooleanParameterValueFromParameterSet (param_set_p, LOCATION_USE_GPS.npt_name_s, &use_gps_flag_p);

			address_p = AllocateAddress (name_s, street_s, town_s, county_s,
																	 country_s, postcode_s, country_code_s, gps_s);

			if (address_p)
				{
					GrassrootsServer *grassroots_p = GetGrassrootsServerFromService (job_p -> sj_service_p);

					if ((use_gps_flag_p != NULL) && (*use_gps_flag_p == true))
						{
							const double64 *latitude_p = NULL;

							if ((GetCurrentDoubleParameterValueFromParameterSet (param_set_p, LOCATION_LATITUDE.npt_name_s, &latitude_p)) && (latitude_p != NULL))
								{
									const double64 *longitude_p = NULL;

									if ((GetCurrentDoubleParameterValueFromParameterSet (param_set_p, LOCATION_LONGITUDE.npt_name_s, &longitude_p)) && (longitude_p != NULL))
										{
											const double64 *elevation_p = NULL;

											GetCurrentDoubleParameterValueFromParameterSet (param_set_p, LOCATION_ALTITUDE.npt_name_s, &elevation_p);

											if (latitude_p && longitude_p)
												{
													if (SetAddressCentreCoordinate (address_p, *latitude_p, *longitude_p, elevation_p))
														{
															success_flag = true;
														}
													else
														{
															PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to set GPS coordinate to [ %lf, %lf ] for %s", *latitude_p, *longitude_p, address_p -> ad_name_s);
															AddGeneralErrorMessageToServiceJob (job_p, "Failed to save GPS coordinate");
														}
												}
										}
									else
										{
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "No Longitude for %s", address_p -> ad_name_s);
											AddParameterErrorMessageToServiceJob (job_p, LOCATION_LONGITUDE.npt_name_s, LOCATION_LONGITUDE.npt_type, "Value required when using your own specified GPS Coordinates");
										}


								}		/* if (GetParameterValueFromParameterSet (param_set_p, LOCATION_LATITUDE.npt_name_s, &latitude_value, true)) */
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "No Latitude for %s", address_p -> ad_name_s);
									AddParameterErrorMessageToServiceJob (job_p, LOCATION_LATITUDE.npt_name_s, LOCATION_LATITUDE.npt_type, "Value required when using your own specified GPS Coordinates");
								}

						}		/* if (use_gps_value.st_boolean_value) */
					else
						{
							if (DetermineGPSLocationForAddress (address_p, NULL, grassroots_p))
								{
									success_flag = true;
								}
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "DetermineGPSLocationForAddress faialed for %s", address_p -> ad_name_s);
									AddGeneralErrorMessageToServiceJob (job_p, "Unable to determine GPS coordinate");
								}
						}


					if (success_flag)
						{
							const uint32 order = 0;
							Location *location_p = AllocateLocation (address_p, order, id_p);

							if (location_p)
								{
									status = SaveLocation (location_p, job_p, data_p);
								}
							else
								{
									success_flag = false;
								}

						}

					FreeAddress (address_p);
				}		/* if (address_p) */

		}		/* if (GetCurrentParameterValueFromParameterSet (param_set_p, LOCATION_NAME.npt_name_s, &name_value)) */


	return ((status == OS_SUCCEEDED) || (status == OS_PARTIALLY_SUCCEEDED));
}



bool SetUpLocationsListParameter (const FieldTrialServiceData *data_p, StringParameter *param_p, const Location *active_location_p, const char *extra_option_s)
{
	bool success_flag = false;
	bson_t *opts_p =  BCON_NEW ( "sort", "{", "name", BCON_INT32 (1), "}");
	json_t *results_p = GetAllLocationsAsJSON (data_p, opts_p);
	bool value_set_flag = false;

	if (results_p)
		{
			if (json_is_array (results_p))
				{
					const size_t num_results = json_array_size (results_p);

					success_flag = true;

					if (num_results > 0)
						{
							/*
							 * If there's an empty option, add it
							 */
							if (extra_option_s)
								{
									success_flag = CreateAndAddStringParameterOption (param_p, extra_option_s, extra_option_s);
								}

							if (success_flag)
								{
									size_t i = 0;
									const char *param_value_s = GetStringParameterCurrentValue (param_p);

									while ((i < num_results) && success_flag)
										{
											json_t *entry_p = json_array_get (results_p, i);
											Location *location_p = GetLocationFromJSON (entry_p, data_p);

											if (location_p)
												{
													char *name_s = GetLocationAsString (location_p);

													if (name_s)
														{
															char *id_s = GetBSONOidAsString (location_p -> lo_id_p);

															if (id_s)
																{
																	if (param_value_s && (strcmp (param_value_s, id_s) == 0))
																		{
																			value_set_flag = true;
																		}

																	if (!CreateAndAddStringParameterOption (param_p, id_s, name_s))
																		{
																			success_flag = false;
																			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add param option \"%s\": \"%s\"", id_s, name_s);
																		}

																	FreeCopiedString (id_s);
																}
															else
																{
																	success_flag = false;
																	PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, entry_p, "Failed to get Location BSON oid");
																}

															FreeCopiedString (name_s);
														}
													else
														{
															success_flag = false;
															PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, entry_p, "Failed to get Location name");
														}

													FreeLocation (location_p);
												}		/* if (location_p) */

											if (success_flag)
												{
													++ i;
												}

										}		/* while ((i < num_results) && success_flag) */

									/*
									 * If the parameter's value isn't on the list, reset it
									 */
									if (!value_set_flag)
										{
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "param value \"%s\" not on list of existing studies", param_value_s);
										}

								}		/* if (num_results > 0) */
							else
								{
									/* nothing to add */
									success_flag = true;
								}

						}		/* if (success_flag) */


				}		/* if (json_is_array (results_p)) */

			json_decref (results_p);
		}		/* if (results_p) */

	if (opts_p)
		{
			bson_destroy (opts_p);
		}

	return success_flag;
}


json_t *GetAllLocationsAsJSON (const FieldTrialServiceData *data_p, bson_t *opts_p)
{
	json_t *results_p = NULL;

	if (SetMongoToolCollection (data_p -> dftsd_mongo_p, data_p -> dftsd_collection_ss [DFTD_LOCATION]))
		{
			bson_t *query_p = NULL;

			results_p = GetAllMongoResultsAsJSON (data_p -> dftsd_mongo_p, query_p, opts_p);
		}		/* if (SetMongoToolCollection (data_p -> dftsd_mongo_p, data_p -> dftsd_collection_ss [DFTD_LOCATION])) */

	return results_p;
}


bool AddLocationToServiceJob (ServiceJob *job_p, Location *location_p, const ViewFormat format, FieldTrialServiceData *data_p)
{
	bool success_flag = false;
	json_t *location_json_p = GetLocationAsJSON (location_p);

	if (location_json_p)
		{
			char *title_s = NULL;

			if (location_p -> lo_address_p)
				{
					if (location_p -> lo_address_p -> ad_name_s)
						{
							title_s = EasyCopyToNewString (location_p -> lo_address_p -> ad_name_s);
						}
					else
						{
							title_s = GetAddressAsString (location_p -> lo_address_p);
						}
				}

			if (title_s)
				{
					if (AddContext (location_json_p))
						{
							json_t *dest_record_p = GetResourceAsJSONByParts (PROTOCOL_INLINE_S, NULL, title_s, location_json_p);

							if (dest_record_p)
								{
									AddImage (dest_record_p, DFTD_LOCATION, data_p);

									if (AddResultToServiceJob (job_p, dest_record_p))
										{
											success_flag = true;
										}
									else
										{
											json_decref (dest_record_p);
										}

								}		/* if (dest_record_p) */

						}		/* if (AddContext (treatment_json_p)) */

					FreeCopiedString (title_s);
				}		/* if (title_s) */

		}		/* if (location_json_p) */

	return success_flag;
}


Location *GetLocationFromResource (Resource *resource_p, const NamedParameterType location_param_type, FieldTrialServiceData *dfw_data_p)
{
	Location *location_p = NULL;

	/*
	 * Have we been set some parameter values to refresh from?
	 */
	if (resource_p && (resource_p -> re_data_p))
		{
			const json_t *param_set_json_p = json_object_get (resource_p -> re_data_p, PARAM_SET_KEY_S);

			if (param_set_json_p)
				{
					json_t *params_json_p = json_object_get (param_set_json_p, PARAM_SET_PARAMS_S);

					if (params_json_p)
						{
							const char *id_s =  NULL;
							const size_t num_entries = json_array_size (params_json_p);
							size_t i;

							for (i = 0; i < num_entries; ++ i)
								{
									const json_t *param_json_p = json_array_get (params_json_p, i);
									const char *name_s = GetJSONString (param_json_p, PARAM_NAME_S);

									if (name_s)
										{
											if (strcmp (name_s, location_param_type.npt_name_s) == 0)
												{
													id_s = GetJSONString (param_json_p, PARAM_CURRENT_VALUE_S);

													if (!id_s)
														{
															PrintJSONToErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, param_json_p, "Failed to get \"%s\" from \"%s\"", PARAM_CURRENT_VALUE_S, location_param_type.npt_name_s);
														}

													/* force exit from loop */
													i = num_entries;
												}
										}		/* if (name_s) */

								}		/* if (params_json_p) */

							/*
							 * Do we have an existing study id?
							 */
							if (id_s)
								{
									location_p = GetLocationByIdString (id_s, VF_CLIENT_FULL , dfw_data_p);

									if (!location_p)
										{
											PrintJSONToErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, params_json_p, "Failed to load Location with id \"%s\"", id_s);
										}

								}		/* if (study_id_s) */

						}
					else
						{
							PrintJSONToErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, param_set_json_p, "Failed to get params with key \"%s\"", PARAM_SET_PARAMS_S);
						}
				}
			else
				{
					PrintJSONToErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, resource_p -> re_data_p, "Failed to get param set with key \"%s\"", PARAM_SET_KEY_S);
				}

		}		/* if (resource_p && (resource_p -> re_data_p)) */

	return location_p;
}



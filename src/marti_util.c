/*
 * marti_util.c
 *
 *  Created on: 3 Oct 2024
 *      Author: billy
 */

#include "marti_util.h"

#include "marti_service_data.h"
#include "marti_search_service.h"
#include "marti_entry.h"

#include "double_parameter.h"
#include "time_parameter.h"


static bool AddMartiSearchParametersByValues (ParameterSet *param_set_p, ParameterGroup *param_group_p, const double64 *latitude_p, const double64 *longitude_p, const struct tm *date_p, ServiceData *data_p);

static OperationStatus SearchMarti (const double64 *latitude_p, const double64 *longitude_p, const struct tm *date_p, const FieldTrialServiceData *ft_service_data_p);


OperationStatus AddMartiResults (Study *study_p, json_t *study_json_p, const ViewFormat format, const FieldTrialServiceData *data_p)
{
	OperationStatus status = OS_IDLE;

	if (study_p -> st_location_p)
		{
			if (study_p -> st_location_p -> lo_address_p)
				{
					Coordinate *c_p = study_p -> st_location_p -> lo_address_p -> ad_gps_centre_p;

					if (c_p)
						{
							const struct tm *date_p = NULL;

							status = SearchMarti (& (c_p -> co_x), & (c_p -> co_y), date_p, data_p);

						}
				}
		}

	return status;
}


static OperationStatus SearchMarti (const double64 *latitude_p, const double64 *longitude_p, const struct tm *date_p, const FieldTrialServiceData *ft_service_data_p)
{
	OperationStatus status = OS_FAILED;

	if (ft_service_data_p -> dftsd_grassroots_marti_search_url_s)
		{
			ParameterSet *param_set_p = AllocateParameterSet (NULL, NULL);

			if (param_set_p)
				{
					const ServiceData *service_data_p = & (ft_service_data_p -> dftsd_base_data);

					if (AddCommonMartiSearchParametersByValues (param_set_p, NULL, latitude_p, longitude_p, date_p, service_data_p))
						{
							const char *service_name_s = "MARTi search service"; //GetMartiSearchServiceName (NULL);
							const char *service_uri_s = ft_service_data_p -> dftsd_grassroots_marti_search_url_s;
							GrassrootsServer *grassroots_p = GetGrassrootsServerFromService (service_data_p -> sd_service_p);
							ProvidersStateTable *providers_p = NULL;

							json_t *res_p = MakeRemotePairedServiceCall (service_name_s, param_set_p, service_uri_s, providers_p, grassroots_p);

							if (res_p)
								{
									PrintJSONToLog (STM_LEVEL_INFO, __FILE__, __LINE__, res_p, "Searching \"%lf, %lf\"", *latitude_p, *longitude_p);


									json_decref (res_p);
								}		/* if (res_p) */

						}		/* if (AddCommonMartiSearchParameters (param_set_p, NULL, NULL, ServiceData *data_p)) */

					FreeParameterSet (param_set_p);
				}		/* if (param_set_p) */


		}		/* if (ft_service_data_p -> dftsd_grassroots_marti_search_url_s) */
	else
		{
			status = OS_IDLE;
		}
	return status;
}


static bool AddMartiSearchParametersByValues (ParameterSet *param_set_p, ParameterGroup *param_group_p, const double64 *latitude_p, const double64 *longitude_p, const struct tm *date_p, ServiceData *data_p)
{
	bool success_flag = false;
	Parameter *param_p = NULL;

	if ((param_p = EasyCreateAndAddDoubleParameterToParameterSet (data_p, param_set_p, param_group_p, MA_LATITUDE.npt_type, MA_LATITUDE.npt_name_s, "Latitude", "The latitude of this location", latitude_p, PL_ALL)) != NULL)
		{
			const char *precision_s = "8";

			if (AddParameterKeyStringValuePair (param_p, PA_DOUBLE_PRECISION_S, precision_s))
				{
					param_p -> pa_required_flag = true;

					if ((param_p = EasyCreateAndAddDoubleParameterToParameterSet (data_p, param_set_p, param_group_p, MA_LONGITUDE.npt_type, MA_LONGITUDE.npt_name_s, "Longitude", "The longitude of this location", longitude_p, PL_ALL)) != NULL)
						{
							if (AddParameterKeyStringValuePair (param_p, PA_DOUBLE_PRECISION_S, precision_s))
								{
									param_p -> pa_required_flag = true;

									if ((param_p = EasyCreateAndAddTimeParameterToParameterSet (data_p, param_set_p, param_group_p, MA_START_DATE.npt_name_s, "Start Date", "The starting date of when this sample was taken", date_p, PL_ALL)) != NULL)
										{
											param_p -> pa_required_flag = true;

											success_flag = true;
										}
									else
										{
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", MA_START_DATE.npt_name_s);
										}

								}		/* if (AddParameterKeyStringValuePair (param_p, PA_DOUBLE_PRECISION_S, precision_s)) */
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to set precision of %s for %s parameter", precision_s, MA_LONGITUDE.npt_name_s);
								}

						}
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", MA_LONGITUDE.npt_name_s);
						}

				}		/* if (AddParameterKeyStringValuePair (param_p, PA_DOUBLE_PRECISION_S, precision_s)) */
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to set precision of %s for %s parameter", precision_s, MA_LATITUDE.npt_name_s);
				}

		}
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", MA_LATITUDE.npt_name_s);
		}

	return success_flag;
}


static void ProcessMartiResults (const json_t *results_json_p, json_t *study_json_p, const char * const marti_service_name_s, const FieldTrialServiceData *service_data_p)
{
	json_t *services_json_p = json_object_get (results_json_p, SERVICE_RESULTS_S);

	if (json_is_array (services_json_p))
		{
			size_t i;
			json_t *job_p;

			json_array_foreach (services_json_p, i, job_p)
				{
					const char *service_name_s = GetJSONString (job_p, JOB_SERVICE_S);

					if (service_name_s)
						{
							if (strcmp (service_name_s, marti_service_name_s) == 0)
								{
									int32 status = OS_IDLE;

									if (GetJSONInteger (job_p, SERVICES_STATUS_S, &status))
										{
											if (((OperationStatus) status) == OS_SUCCEEDED)
												{
													json_t *marti_links_p = json_array ();

													if (marti_links_p)
														{
															json_t *results_p = json_object_get (job_p, SERVICE_RESULTS_S);

															if (results_p)
																{
																	json_t *result_p;
																	size_t j;
																	const bool has_slash_flag = DoesStringEndWith (service_data_p -> dftsd_marti_api_url_s, "/");

																	json_array_foreach (results_p, j, result_p)
																		{
																			const char *sample_name_s = GetJSONString (result_p, ME_NAME_S);
																			const char *marti_id_s = GetJSONString (result_p, ME_MARTI_ID_S);
																			char *url_s = NULL;

																			if (has_slash_flag)
																				{
																					url_s = ConcatenateStrings (service_data_p -> dftsd_marti_api_url_s, marti_id_s);
																				}
																			else
																				{
																					url_s = ConcatenateVarargsStrings (service_data_p -> dftsd_marti_api_url_s, "/", marti_id_s, NULL);
																				}

																			if (url_s)
																				{
																					json_t *marti_p = json_object ();

																					if (marti_p)
																						{
																							bool success_flag = false;

																							if (SetJSONString (marti_p, JOB_NAME_S, sample_name_s))
																								{
																									if (SetJSONString (marti_p, JOB_URL_S, url_s))
																										{
																											if (json_array_append_new (marti_links_p, marti_p) == 0)
																												{
																													success_flag = true;
																												}

																										}

																								}

																							if (!success_flag)
																								{
																									json_decref (marti_p);
																								}
																						}
																				}
																		}

																}		/* if (results_p) */


															if (json_object_set_new (results_json_p, "marti_samples", marti_links_p) == 0)
																{

																}
															else
																{
																	json_decref (marti_links_p);
																}

														}		/* if (marti_links_p) */


												}		/* if (((OperationStatus) status) == OS_SUCCEEDED) */

										}		/* if (GetJSONInteger (job_p, SERVICES_STATUS_S, &status)) */

								}		/* if (strcmp (service_name_s, marti_service_name_s) == 0) */

						}		/* if (service_name_s) */

				}		/* json_array_foreach (services_json_p, i, job_p) */

		}		/* if (json_is_array (services_json_p)) */

}



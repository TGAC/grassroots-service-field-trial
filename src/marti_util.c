/*
 * marti_util.c
 *
 *  Created on: 3 Oct 2024
 *      Author: billy
 */

#include "marti_util.h"

#include "marti_service_data.h"
#include "marti_search_service.h"

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




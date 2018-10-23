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
 * gene_bank_jobs.c
 *
 *  Created on: 23 Oct 2018
 *      Author: billy
 */


#include "gene_bank_jobs.h"
#include "gene_bank.h"
#include "streams.h"


static NamedParameterType S_GENE_BANK_NAME = { "GB Name", PT_STRING };
static NamedParameterType S_GENE_BANK_URL = { "GB Url", PT_STRING};
static NamedParameterType S_GENE_BANK_API_URL = { "GB API Url", PT_STRING };
static NamedParameterType S_ADD_GENE_BANK = { "GB Add Gene Bank", PT_BOOLEAN };


static bool AddGeneBank (ServiceJob *job_p, ParameterSet *param_set_p, DFWFieldTrialServiceData *data_p);


/*
 * API definitions
 */

bool AddGeneBankParams (ServiceData *data_p, ParameterSet *param_set_p)
{
	bool success_flag = false;
	ParameterGroup *group_p = CreateAndAddParameterGroupToParameterSet ("Gene Bank", NULL, false, data_p, param_set_p);

	if (group_p)
		{
			Parameter *param_p = NULL;
			SharedType def;

			def.st_string_value_s = NULL;

			if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_GENE_BANK_NAME.npt_type, S_GENE_BANK_NAME.npt_name_s, "Name", "The name of the Gene Bank", def, PL_ALL)) != NULL)
				{
					if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_GENE_BANK_URL.npt_type, S_GENE_BANK_URL.npt_name_s, "URL", "The web page with information about the Gene Bank", def, PL_ALL)) != NULL)
						{
							if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_GENE_BANK_API_URL.npt_type, S_GENE_BANK_API_URL.npt_name_s, "API URL", "The API URL for the Gene Bank", def, PL_ALL)) != NULL)
								{
									def.st_boolean_value = false;

									if ((param_p = EasyCreateAndAddParameterToParameterSet (data_p, param_set_p, group_p, S_ADD_GENE_BANK.npt_type, S_ADD_GENE_BANK.npt_name_s, "Add", "Add Gene Bank", def, PL_ALL)) != NULL)
										{
											success_flag = true;
										}
									else
										{
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add param \%s\"", S_ADD_GENE_BANK.npt_name_s);
										}

								}
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add param \%s\"", S_GENE_BANK_API_URL.npt_name_s);
								}
						}
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add param \%s\"", S_GENE_BANK_URL.npt_name_s);
						}

				}
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add param \%s\"", S_GENE_BANK_NAME.npt_name_s);
				}

		}		/* if (group_p) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to create Gene Bank Parameter Group");
		}

	return success_flag;
}


bool RunForGeneBankParams (DFWFieldTrialServiceData *data_p, ParameterSet *param_set_p, ServiceJob *job_p)
{
	bool job_done_flag = false;
	SharedType value;
	InitSharedType (&value);

	if (GetParameterValueFromParameterSet (param_set_p, S_ADD_GENE_BANK.npt_name_s, &value, true))
		{
			if (value.st_boolean_value)
				{
					bool success_flag = AddGeneBank (job_p, param_set_p, data_p);

					job_done_flag = true;
				}		/* if (value.st_boolean_value) */

		}		/* if (GetParameterValueFromParameterSet (param_set_p, S_ADD_LOCATION.npt_name_s, &value, true)) */

	return job_done_flag;
}



bool SetUpGenBanksListParameter (const DFWFieldTrialServiceData *data_p, Parameter *param_p)
{
	bool success_flag = false;

	if (SetMongoToolCollection (data_p -> dftsd_mongo_p, data_p -> dftsd_collection_ss [DFTD_GENE_BANK]))
		{
			bson_t *query_p = NULL;
			bson_t *opts_p = BCON_NEW ( "sort", "{", CONTEXT_PREFIX_SCHEMA_ORG_S "name", BCON_INT32 (1), "}");
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
									GeneBank *gene_bank_p = GetGeneBankFromJSON (entry_p);

									if (gene_bank_p)
										{
											SharedType def;
											char id_s [MONGO_OID_STRING_BUFFER_SIZE];

											bson_oid_to_string (gene_bank_p -> gb_id_p, id_s);

											def.st_string_value_s = id_s;

											if (SetParameterValueFromSharedType (param_p, &def, false))
												{
													if (SetParameterValueFromSharedType (param_p, &def, true))
														{
															success_flag = CreateAndAddParameterOptionToParameter (param_p, def, gene_bank_p -> gb_name_s);
														}
												}

											FreeGeneBank (gene_bank_p);
										}		/* if (gene_bank_p) */

									if (success_flag)
										{
											for (++ i; i < num_results; ++ i)
												{
													entry_p = json_array_get (results_p, i);
													gene_bank_p = GetGeneBankFromJSON (entry_p);

													if (gene_bank_p)
														{
															SharedType def;
															char id_s [MONGO_OID_STRING_BUFFER_SIZE];

															bson_oid_to_string (gene_bank_p -> gb_id_p, id_s);

															def.st_string_value_s = id_s;

															success_flag = CreateAndAddParameterOptionToParameter (param_p, def, gene_bank_p -> gb_name_s);

															FreeGeneBank (gene_bank_p);
														}		/* if (gene_bank_p) */

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

					json_decref (results_p);
				}		/* if (results_p) */

			if (opts_p)
				{
					bson_destroy (opts_p);
				}

		}		/* if (SetMongoToolCollection (data_p -> dftsd_mongo_p, data_p -> dftsd_collection_ss [DFTD_LOCATION])) */

	return success_flag;
}



static bool AddGeneBank (ServiceJob *job_p, ParameterSet *param_set_p, DFWFieldTrialServiceData *data_p)
{
	bool success_flag = false;
	SharedType name_value;
	InitSharedType (&name_value);

	if (GetParameterValueFromParameterSet (param_set_p, S_GENE_BANK_NAME.npt_name_s, &name_value, true))
		{
			SharedType url_value;
			InitSharedType (&url_value);

			if (GetParameterValueFromParameterSet (param_set_p, S_GENE_BANK_URL.npt_name_s, &url_value, true))
				{
					SharedType api_url_value;
					InitSharedType (&api_url_value);

					if (GetParameterValueFromParameterSet (param_set_p, S_GENE_BANK_API_URL.npt_name_s, &api_url_value, true))
						{
							GeneBank *gene_bank_p = AllocateGeneBank (NULL, name_value.st_string_value_s, url_value.st_string_value_s, api_url_value.st_string_value_s);

							if (gene_bank_p)
								{
									if (SaveGeneBank (gene_bank_p, data_p))
										{
											success_flag = true;
										}
									else
										{
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "SaveGeneBank failed for \"%s\", \"%s\", \"%s\"", name_value.st_string_value_s, url_value.st_string_value_s, api_url_value.st_string_value_s);
										}

									FreeGeneBank (gene_bank_p);
								}		/* if (gene_bank_p) */
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "AllocateGeneBank failed for \"%s\", \"%s\", \"%s\"", name_value.st_string_value_s, url_value.st_string_value_s, api_url_value.st_string_value_s);
								}

						}		/* if (GetParameterValueFromParameterSet (param_set_p, S_GENE_BANK_API_URL.npt_name_s, &api_url_value, true)) */
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get \"%s\" parameter value", S_GENE_BANK_API_URL.npt_name_s);
						}

				}		/* if (GetParameterValueFromParameterSet (param_set_p, S_GENE_BANK_URL.npt_name_s, &url_value, true)) */
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get \"%s\" parameter value", S_GENE_BANK_URL.npt_name_s);
				}

		}		/* if (GetParameterValueFromParameterSet (param_set_p, S_GENE_BANK_NAME.npt_name_s, &name_value, true)) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get \"%s\" parameter value", S_GENE_BANK_NAME.npt_name_s);
		}

	SetServiceJobStatus (job_p, success_flag ? OS_SUCCEEDED : OS_FAILED);

	return success_flag;
}


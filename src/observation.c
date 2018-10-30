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
 * observation.c
 *
 *  Created on: 30 Oct 2018
 *      Author: billy
 */

#include <string.h>

#define ALLOCATE_OBSERVATION_TAGS (1)
#include "observation.h"

#include "time_util.h"
#include "memory_allocations.h"
#include "string_utils.h"
#include "dfw_util.h"


static const char *S_OBSERVATION_NATURES_SS [ON_NUM_PHENOTYPE_NATURES] = { "Row", "Experimental Area" };


static bool AddObservationNatureToJSON (const ObservationNature phenotype_nature, json_t *doc_p);

static bool GetObservationNatureFromJSON (ObservationNature *phenotype_nature_p, const json_t *doc_p);

static bool CreateInstrumentFromObservationJSON (const json_t *observation_json_p, Instrument **instrument_pp, const DFWFieldTrialServiceData *data_p);

static Phenotype *CreatePhenotypeFromObservationJSON (const json_t *observation_json_p, const DFWFieldTrialServiceData *data_p);


/*
 * API definitions
 */


Observation *AllocateObservation (bson_oid_t *id_p, const struct tm *date_p, Phenotype *phenotype_p, const char *value_s,
																	const char *growth_stage_s, const bool corrected_value_flag, const char *method_s, Instrument *instrument_p, const ObservationNature nature)
{
	bool success_flag = true;
	struct tm *copied_date_p = NULL;

	if (date_p)
		{
			copied_date_p = DuplicateTime (date_p);

			if (!copied_date_p)
				{
					success_flag = false;
				}
		}

	if (success_flag)
		{
			char *copied_value_s = EasyCopyToNewString (value_s);

			if (copied_value_s)
				{
					char *copied_growth_stage_s = NULL;

					if ((IsStringEmpty (growth_stage_s)) || ((copied_growth_stage_s = EasyCopyToNewString (growth_stage_s)) != NULL))
						{
							char *copied_method_s = NULL;

							if ((IsStringEmpty (method_s)) || ((copied_method_s = EasyCopyToNewString (method_s)) != NULL))
								{
									Observation *observation_p = (Observation *) AllocMemory (sizeof (Observation));

									if (observation_p)
										{
											observation_p -> ob_id_p = id_p;
											observation_p -> ob_phenotype_p = phenotype_p;
											observation_p -> ob_measured_value_s = copied_value_s;
											observation_p -> ob_date_p = copied_date_p;
											observation_p -> ob_instrument_p = instrument_p;
											observation_p -> ob_growth_stage_s = copied_growth_stage_s;
											observation_p -> ob_corrected_flag = corrected_value_flag;
											observation_p -> ob_method_s = copied_method_s;
											observation_p -> ob_type = nature;

											return observation_p;
										}		/* if (observation_p) */


									if (copied_method_s)
										{
											FreeCopiedString (copied_method_s);
										}

								}		/* if ((IsStringEmpty (method_s)) || ((copied_method_s = EasyCopyToNewString (method_s)) != NULL)) */

							if (copied_growth_stage_s)
								{
									FreeCopiedString (copied_growth_stage_s);
								}

						}		/* if ((IsStringEmpty (growth_stage_s)) || ((copied_growth_stage_s = EasyCopyToNewString (growth_stage_s)) != NULL)) */

					FreeCopiedString (copied_value_s);
				}		/* if (copied_value_s) */

			if (copied_date_p)
				{
					FreeTime (copied_date_p);
				}

		}		/* if (success_flag) */


	return NULL;
}


void FreeObservation (Observation *observation_p)
{
	if (observation_p -> ob_id_p)
		{
			FreeBSONOid (observation_p -> ob_id_p);
		}

	if (observation_p -> ob_phenotype_p)
		{
			FreePhenotype (observation_p -> ob_phenotype_p);
		}

	if (observation_p -> ob_growth_stage_s)
		{
			FreeCopiedString (observation_p -> ob_growth_stage_s);
		}

	if (observation_p -> ob_measured_value_s)
		{
			FreeCopiedString (observation_p -> ob_measured_value_s);
		}

	if (observation_p -> ob_date_p)
		{
			FreeTime (observation_p -> ob_date_p);
		}

	if (observation_p -> ob_method_s)
		{
			FreeCopiedString (observation_p -> ob_method_s);
		}


	/*
	 * Still need to deallocate the instrument
	 *
	 * observation_p -> ob_instrument_p
	 */

	FreeMemory (observation_p);
}


ObservationNode *AllocateObservationNode (Observation *observation_p)
{
	ObservationNode *ob_node_p = (ObservationNode *) AllocMemory (sizeof (ObservationNode));

	if (ob_node_p)
		{
			InitListItem (& (ob_node_p -> on_node));

			ob_node_p -> on_observation_p = observation_p;
		}

	return ob_node_p;
}


void FreeObservationNode (ListItem *node_p)
{
	ObservationNode *ob_node_p = (ObservationNode *) node_p;

	if (ob_node_p -> on_observation_p)
		{
			FreeObservation (ob_node_p -> on_observation_p);
		}

	FreeMemory (ob_node_p);
}


json_t *GetObservationAsJSON (const Observation *observation_p, const bool expand_fields_flag)
{
	json_t *observation_json_p = json_object ();

	if (observation_json_p)
		{
			if (AddValidDateToJSON (observation_p -> ob_date_p, observation_json_p, OB_DATE_S))
				{
					if ((IsStringEmpty (observation_p -> ob_measured_value_s)) || (SetJSONString (observation_json_p, OB_VALUE_S, observation_p -> ob_measured_value_s)))
						{
							if ((IsStringEmpty (observation_p -> ob_growth_stage_s)) || (SetJSONString (observation_json_p, OB_GROWTH_STAGE_S, observation_p -> ob_growth_stage_s)))
								{
									if ((IsStringEmpty (observation_p -> ob_method_s)) || (SetJSONString (observation_json_p, OB_METHOD_S, observation_p -> ob_method_s)))
										{
											if (AddCompoundIdToJSON (observation_json_p, observation_p -> ob_id_p))
												{
													if (SetJSONBoolean (observation_json_p, OB_CORRECTED_S, observation_p -> ob_corrected_flag))
														{
															bool done_objects_flag = false;

															if (expand_fields_flag)
																{
																	bool done_instrument_flag = false;

																	if (observation_p -> ob_instrument_p)
																		{
																			json_t *instrument_json_p = GetInstrumentAsJSON (observation_p -> ob_instrument_p);

																			if (instrument_json_p)
																				{
																					if (json_object_set_new (observation_json_p, OB_INSTRUMENT_S, instrument_json_p) == 0)
																						{
																							done_instrument_flag = true;
																						}		/* if (json_object_set_new (observation_json_p, OB_INSTRUMENT_S, instrument_json_p) == 0) */
																					else
																						{
																							json_decref (instrument_json_p);
																						}
																				}
																		}
																	else
																		{
																			done_instrument_flag = true;
																		}

																	if (done_instrument_flag)
																		{
																			json_t *phenotype_json_p = GetPhenotypeAsJSON (observation_p -> ob_phenotype_p);

																			if (phenotype_json_p)
																				{
																					if (json_object_set_new (observation_json_p, OB_PHENOTYPE_S, phenotype_json_p) == 0)
																						{
																							done_objects_flag = true;
																						}		/* if (json_object_set_new (observation_json_p, OB_PHENOTYPE_S, phenotype_json_p) == 0) */
																					else
																						{
																							json_decref (phenotype_json_p);
																						}
																				}
																		}

																}		/* if (expand_fields_flag) */
															else
																{
																	if ((! (observation_p -> ob_instrument_p)) || (AddNamedCompoundIdToJSON (observation_json_p, observation_p -> ob_instrument_p -> in_id_p, OB_INSTRUMENT_ID_S)))
																		{
																			if ((! (observation_p -> ob_phenotype_p)) || (AddNamedCompoundIdToJSON (observation_json_p, observation_p -> ob_phenotype_p -> ph_id_p, OB_PHENOTYPE_ID_S)))
																				{
																					done_objects_flag = true;
																				}
																		}
																}

															if (done_objects_flag)
																{
																	if (AddObservationNatureToJSON (observation_p -> ob_type, observation_json_p))
																		{
																			return observation_json_p;
																		}
																	else
																		{
																			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, observation_json_p, "Failed to add \"%s\": %d to JSON", OB_NATURE_S, observation_p -> ob_type);
																		}
																}

														}		/* if (SetJSONBoolean (observation_json_p, OB_CORRECTED_S, observation_p -> ob_corrected_flag)) */
													else
														{
															PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, observation_json_p, "Failed to set \"%s\": \"%d\" to JSON", OB_CORRECTED_S, observation_p -> ob_corrected_flag);
														}


												}		/* if (AddCompoundIdToJSON (observation_json_p, observation_p -> ob_id_p)) */
											else
												{
													char id_s [MONGO_OID_STRING_BUFFER_SIZE];

													bson_oid_to_string (observation_p -> ob_id_p, id_s);
													PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, observation_json_p, "Failed to set \"%s\": \"%s\" to JSON", MONGO_ID_S, id_s);
												}

										}		/* if ((IsStringEmpty (observation_p -> ob_method_s)) || SetJSONString (observation_json_p, OB_METHOD_S, observation_p -> ob_method_s)) */
									else
										{
											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, observation_json_p, "Failed to set \"%s\": \"%s\" to JSON", OB_METHOD_S, observation_p -> ob_method_s);
										}

								}		/* if ((IsStringEmpty (observation_p -> ob_growth_stage_s)) || SetJSONString (observation_json_p, OB_GROWTH_STAGE_S, observation_p -> ob_growth_stage_s)) */
							else
								{
									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, observation_json_p, "Failed to set \"%s\": \"%s\" to JSON", OB_GROWTH_STAGE_S, observation_p -> ob_growth_stage_s);
								}

						}		/* if (SetJSONString (observation_json_p, OB_VALUE_S, observation_p -> ob_measured_value_s)) */
					else
						{
							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, observation_json_p, "Failed to add \"%s\": \"%s\" to JSON", OB_VALUE_S, observation_p -> ob_measured_value_s);
						}

				}		/* if (AddValidDateToJSON (observation_p -> ob_date_p, observation_json_p, OB_DATE_S)) */
			else
				{
					char *date_s = GetTimeAsString (observation_p -> ob_date_p, false);

					if (date_s)
						{
							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, observation_json_p, "Failed to add \"%s\": \"%s\" to JSON", OB_DATE_S, date_s);
							FreeCopiedString (date_s);
						}
					else
						{
							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, observation_json_p, "Failed to add \"%s\": unknown date to JSON", OB_DATE_S);
						}
				}

			json_decref (observation_json_p);
		}		/* if (observation_json_p) */

	return NULL;
}


Observation *GetObservationFromJSON (const json_t *observation_json_p, const DFWFieldTrialServiceData *data_p)
{
	struct tm *date_p = NULL;

	if (CreateValidDateFromJSON (observation_json_p, OB_DATE_S, &date_p))
		{
			bson_oid_t *id_p = GetNewUnitialisedBSONOid ();

			if (id_p)
				{
					if (GetMongoIdFromJSON (observation_json_p, id_p))
						{
							Instrument *instrument_p = NULL;

							if (CreateInstrumentFromObservationJSON (observation_json_p, &instrument_p, data_p))
								{
									Phenotype *phenotype_p = CreatePhenotypeFromObservationJSON (observation_json_p, data_p);

									if (phenotype_p)
										{
											ObservationNature nature = ON_ROW;
											Observation *observation_p = NULL;
											bool corrected_flag;
											const char *growth_stage_s = GetJSONString (observation_json_p, OB_GROWTH_STAGE_S);
											const char *method_s = GetJSONString (observation_json_p, OB_METHOD_S);
											const char *value_s = GetJSONString (observation_json_p, OB_VALUE_S);

											GetObservationNatureFromJSON (&nature, observation_json_p);
											GetJSONBoolean (observation_json_p, OB_CORRECTED_S, &corrected_flag);

											observation_p = AllocateObservation (id_p, date_p, phenotype_p, value_s, growth_stage_s, corrected_flag, method_s, instrument_p, nature);

											if (observation_p)
												{
													return observation_p;
												}
											else
												{
													PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, observation_json_p, "Failed to allocate Observation");
												}


										}		/* if (phenotype_p) */

								}		/* if (CreateInstrumentFromObservationJSON (observation_json_p, &instrument_p, data_p)) */
							else
								{
									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, observation_json_p, "Failed to create instrument from \"%s\"", OB_DATE_S);
								}

						}		/* if (GetMongoIdFromJSON (observation_json_p, id_p)) */
					else
						{
							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, observation_json_p, "Failed to get id \"%s\"", MONGO_ID_S);
						}

					FreeBSONOid (id_p);
				}		/* if (id_p) */
			else
				{
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, observation_json_p, "Failed to allocate id");
				}

		}		/* if (CreateValidDateFromJSON (observation_json_p, OB_DATE_S, &date_p)) */
	else
		{
			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, observation_json_p, "Failed to create date from \"%s\"", OB_DATE_S);
		}

	return NULL;
}


bool SaveObservation (Observation *observation_p, const DFWFieldTrialServiceData *data_p)
{
	bson_t *selector_p = NULL;
	bool success_flag = PrepareSaveData (& (observation_p -> ob_id_p), &selector_p);

	if (success_flag)
		{
			json_t *observation_json_p = GetObservationAsJSON (observation_p, false);

			if (observation_json_p)
				{
					const char *collection_s = observation_p -> ob_corrected_flag ? data_p -> dftsd_collection_ss [DFTD_CORRECTED_OBSERVATION] : data_p -> dftsd_collection_ss [DFTD_RAW_OBSERVATION];
					success_flag = SaveMongoData (data_p -> dftsd_mongo_p, observation_json_p, collection_s, selector_p);

					json_decref (observation_json_p);
				}		/* if (observation_json_p) */

		}		/* if (observation_p -> ob_id_p) */

	return success_flag;
}


bool SetObservationValue (Observation *observation_p, const char *value_s)
{
	bool success_flag = false;

	if (value_s)
		{
			char *copied_value_s = EasyCopyToNewString (value_s);

			if (copied_value_s)
				{
					if (observation_p -> ob_measured_value_s)
						{
							FreeCopiedString (observation_p -> ob_measured_value_s);
						}

					observation_p -> ob_measured_value_s = copied_value_s;
					success_flag = true;
				}
		}
	else
		{
			if (observation_p -> ob_measured_value_s)
				{
					FreeCopiedString (observation_p -> ob_measured_value_s);
					observation_p -> ob_measured_value_s = NULL;
				}

			success_flag = true;
		}

	return success_flag;
}



/*
 * static definitions
 */

static bool CreateInstrumentFromObservationJSON (const json_t *observation_json_p, Instrument **instrument_pp, const DFWFieldTrialServiceData *data_p)
{
	bool success_flag = false;
	Instrument *instrument_p = NULL;
	const json_t *val_p = json_object_get (observation_json_p, OB_INSTRUMENT_S);

	if (val_p)
		{
			instrument_p = GetInstrumentFromJSON (val_p);

			if (instrument_p)
				{
					*instrument_pp = instrument_p;
					success_flag = true;
				}
		}
	else
		{
			bson_oid_t *instrument_id_p = GetNewUnitialisedBSONOid ();

			if (instrument_id_p)
				{
					if (GetNamedIdFromJSON (observation_json_p, OB_INSTRUMENT_ID_S, instrument_id_p))
						{
							instrument_p = GetInstrumentById (instrument_id_p, data_p);

							if (instrument_p)
								{
									*instrument_pp = instrument_p;
									success_flag = true;
								}

						}
					else
						{
							/* no instrument in json */
							success_flag = true;
						}

					FreeBSONOid (instrument_id_p);
				}
		}


	return success_flag;
}


static bool AddObservationNatureToJSON (const ObservationNature nature, json_t *doc_p)
{
	bool success_flag = false;

	if (nature < ON_NUM_PHENOTYPE_NATURES)
		{
			success_flag = SetJSONString (doc_p, OB_NATURE_S, * (S_OBSERVATION_NATURES_SS + nature));
		}

	return success_flag;
}


static bool GetObservationNatureFromJSON (ObservationNature *nature_p, const json_t *doc_p)
{
	bool success_flag = false;
	const char *value_s = GetJSONString (doc_p, OB_NATURE_S);

	if (value_s)
		{
			ObservationNature i = ON_ROW;

			while (i < ON_NUM_PHENOTYPE_NATURES)
				{
					if (strcmp (value_s, * (S_OBSERVATION_NATURES_SS + i)) == 0)
						{
							*nature_p = i;
							return true;
						}
					else
						{
							++ i;
						}

				}
		}

	return success_flag;
}



static Phenotype *CreatePhenotypeFromObservationJSON (const json_t *observation_json_p, const DFWFieldTrialServiceData *data_p)
{
	Phenotype *phenotype_p = NULL;
	const json_t *val_p = json_object_get (observation_json_p, OB_PHENOTYPE_S);

	if (val_p)
		{
			phenotype_p = GetPhenotypeFromJSON (val_p, data_p);

			if (!phenotype_p)
				{
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, observation_json_p, "Failed to get phenotype from json");
				}
		}
	else
		{
			bson_oid_t *phenotype_id_p = GetNewUnitialisedBSONOid ();

			if (phenotype_id_p)
				{
					if (GetNamedIdFromJSON (observation_json_p, OB_PHENOTYPE_ID_S, phenotype_id_p))
						{
							phenotype_p = GetPhenotypeById (phenotype_id_p, data_p);

							if (!phenotype_p)
								{
									char id_s [MONGO_OID_STRING_BUFFER_SIZE];

									bson_oid_to_string (phenotype_id_p, id_s);
									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, observation_json_p, "Failed to get phenotype from id \"%s\"", id_s);
								}

						}		/* if (GetNamedIdFromJSON (observation_json_p, OB_PHENOTYPE_ID_S, phenotype_id_p)) */
					else
						{
							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, observation_json_p, "Failed to get id from \"%s\"", OB_PHENOTYPE_ID_S);
						}

					FreeBSONOid (phenotype_id_p);
				}		/* if (phenotype_id_p) */
			else
				{
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, observation_json_p, "Failed to allocate Phenotype's BSONOid");
				}
		}

	return phenotype_p;
}

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
 * instrument.c
 *
 *  Created on: 17 Oct 2018
 *      Author: billy
 */

#define ALLOCATE_INSTRUMENT_TAGS (1)
#include "instrument.h"
#include "memory_allocations.h"
#include "string_utils.h"



Instrument *AllocateInstrument (bson_oid_t *id_p, const char *name_s, const char *model_s)
{
	char *copied_name_s = EasyCopyToNewString (name_s);

	if (copied_name_s)
		{
			char *copied_model_s = EasyCopyToNewString (model_s);

			if (copied_model_s)
				{
					Instrument *instrument_p = (Instrument *) AllocMemory (sizeof (Instrument));

					if (instrument_p)
						{
							instrument_p -> in_id_p = id_p;
							instrument_p -> in_model_s = copied_model_s;
							instrument_p -> in_name_s = copied_name_s;

							return instrument_p;
						}


					FreeCopiedString (copied_model_s);
				}

			FreeCopiedString (copied_name_s);
		}

	return NULL;
}


void FreeInstrument (Instrument *instrument_p)
{
	FreeCopiedString (instrument_p -> in_model_s);
	FreeCopiedString (instrument_p -> in_name_s);

	if (instrument_p -> in_id_p)
		{
			FreeBSONOid (instrument_p -> in_id_p);
		}

	FreeMemory (instrument_p);
}


json_t *GetInstrumentAsJSON (const Instrument *instrument_p)
{
	json_t *instrument_json_p = json_object ();

	if (instrument_json_p)
		{
			if (SetJSONString (instrument_json_p, IN_NAME_S, instrument_p -> in_name_s))
				{
					if (SetJSONString (instrument_json_p, IN_MODEL_S, instrument_p -> in_model_s))
						{
							if (AddCompoundIdToJSON (instrument_json_p, instrument_p -> in_id_p))
								{
									return instrument_json_p;
								}
						}
				}

			json_decref (instrument_json_p);
		}

	return NULL;
}


Instrument *GetInstrumentFromJSON (const json_t *instrument_json_p)
{
	const char *name_s = GetJSONString (instrument_json_p, IN_NAME_S);

	if (name_s)
		{
			const char *model_s = GetJSONString (instrument_json_p, IN_MODEL_S);

			if (model_s)
				{
					bson_oid_t *id_p = GetNewUnitialisedBSONOid ();

					if (id_p)
						{
							if (GetMongoIdFromJSON (instrument_json_p, id_p))
								{
									Instrument *instrument_p = AllocateInstrument (id_p, name_s, model_s);

									if (instrument_p)
										{
											return instrument_p;
										}
								}

							FreeBSONOid (id_p);
						}
				}

		}

	return NULL;
}


bool SaveInstrument (Instrument *instrument_p, const DFWFieldTrialServiceData *data_p)
{
	bool success_flag = false;
	bool insert_flag = false;

	if (! (instrument_p -> in_id_p))
		{
			instrument_p -> in_id_p  = GetNewBSONOid ();

			if (instrument_p -> in_id_p)
				{
					insert_flag = true;
				}
		}

	if (instrument_p -> in_id_p)
		{
			json_t *instrument_json_p = GetInstrumentAsJSON (instrument_p);

			if (instrument_json_p)
				{
					success_flag = SaveMongoData (data_p -> dftsd_mongo_p, instrument_json_p, data_p -> dftsd_collection_ss [DFTD_INSTRUMENT], insert_flag);

					json_decref (instrument_json_p);
				}		/* if (instrument_json_p) */

		}		/* if (instrument_p -> in_id_p) */

	return success_flag;
}
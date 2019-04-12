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
 * crop.c
 *
 *  Created on: 12 Apr 2019
 *      Author: billy
 */


#include "crop.h"

#include "memory_allocations.h"


Crop *AllocateCrop (bson_oid_t *id_p, const char *name_s, const char *argovoc_preferred_term_s, const char *agrovoc_uri_s, const char **synonyms_ss)
{

}



void FreeCrop (Crop *crop_p)
{
	if (crop_p -> cr_id_p)
		{
			FreeBSONOid (crop_p -> cr_id_p)
		}

	if (crop_p -> cr_argovoc_preferred_term_s)
		{
			FreeCopiedString (crop_p -> cr_argovoc_preferred_term_s);
		}

	if (crop_p -> cr_agrovoc_uri_s)
		{
			FreeCopiedString (crop_p -> cr_agrovoc_uri_s);
		}

	if (crop_p -> cr_argovoc_preferred_term_s)
		{
			FreeCopiedString (crop_p -> cr_argovoc_preferred_term_s);
		}


	FreeCopiedString (crop_p -> cr_name_s);

	if (crop_p -> cr_synonyms_ss)
		{
			char **synonym_ss = crop_p -> cr_synonyms_ss;


			while (*synonym_ss)
				{
					FreeCopiedString (**synonym_ss);
					++ *synonym_ss;
				}

			FreeMemory (crop_p -> cr_synonyms_ss);
		}

	FreeMemory (crop_p);
}


json_t *GetCropAsJSON (Crop *crop_p, const ViewFormat format, const DFWFieldTrialServiceData *data_p)
{

}


Crop *GetCropFromJSON (const json_t *crop__json_p, Study *parent_area_p, const DFWFieldTrialServiceData *data_p)
{

}


bool SaveCrop (Crop *crop_p, const DFWFieldTrialServiceData *data_p)
{

}

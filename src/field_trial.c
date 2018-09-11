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
 * field_trial.c
 *
 *  Created on: 11 Sep 2018
 *      Author: billy
 */


#include "memory_allocations.h"
#include "field_trial.h"
#include "dfw_field_trial_service_data.h"


static bool SaveFieldTrial (FieldTrial *trial_p);

static bool LoadFieldTrialByName (const char *name_s);



FieldTrial *AllocateFieldTrial (const char *name_s, const char *team_s)
{
	char *copied_name_s = EasyCopyToNewString (name_s);

	if (copied_name_s)
		{
			char *copied_team_s = EasyCopyToNewString (team_s);

			if (copied_team_s)
				{
					FieldTrial *trial_p = (FieldTrial *) AllocMemory (sizeof (FieldTrial));

					if (trial_p)
						{
							trial_p -> ft_name_s = copied_name_s;
							trial_p -> ft_team_s = copied_team_s;

							trial_p -> ft_id = DFW_UNSET_ID;

							return trial_p;
						}

					FreeCopiedString (copied_team_s);
				}		/* if (copied_team_s) */

			FreeCopiedString (copied_name_s);
		}		/* if (copied_name_s) */

	return NULL;
}




void FreeFieldTrial (FieldTrial *trial_p)
{
	FreeCopiedString (trial_p -> ft_name_s);
	FreeCopiedString (trial_p -> ft_team_s);

	FreeMemory (trial_p);
}



static bool SaveFieldTrial (DFWFieldTrialServiceData *data_p, FieldTrial *trial_p)
{
	bool success_flag = false;

	switch (data_p -> dftsd_backend)
		{
			case DB_MONGO_DB:
				break;
		}

	return success_flag;
}

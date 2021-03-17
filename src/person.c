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
 * person.c
 *
 *  Created on: 7 Jan 2019
 *      Author: billy
 */

#include "person.h"
#include "memory_allocations.h"
#include "dfw_util.h"


Person *AllocatePerson (const char *name_s, const char *email_s)
{
	char *copied_name_s = NULL;

	if (CloneValidString (name_s, &copied_name_s))
		{
			char *copied_email_s = NULL;

			if (CloneValidString (email_s, &copied_email_s))
				{
					Person *person_p = (Person *) AllocMemory (sizeof (Person));

					if (person_p)
						{
							person_p -> pe_name_s = copied_name_s;
							person_p -> pe_email_s = copied_email_s;

							return person_p;
						}

					if (copied_email_s)
						{
							FreeCopiedString (copied_email_s);
						}
				}


			if (copied_name_s)
				{
					FreeCopiedString (copied_name_s);
				}
		}

	return NULL;
}


void FreePerson (Person *person_p)
{
	if (person_p -> pe_name_s)
		{
			FreeCopiedString (person_p -> pe_name_s);
		}

	if (person_p -> pe_email_s)
		{
			FreeCopiedString (person_p -> pe_email_s);
		}

	FreeMemory (person_p);

}



bool AddPersonToCompoundJSON (const Person *person_p, json_t *parent_json_p, const char * const key_s,  const ViewFormat format, const FieldTrialServiceData *data_p)
{
	bool success_flag = false;
	json_t *person_json_p = GetPersonAsJSON (person_p, format, data_p);

	if (person_json_p)
		{
			if (json_object_set_new (parent_json_p, key_s, person_json_p) == 0)
				{
					success_flag = true;
				}
			else
				{
					json_decref (parent_json_p);
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, parent_json_p, "Failed to add person to json \"%s\" \"%s\"", person_p -> pe_name_s, person_p -> pe_email_s);
				}
		}
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get person as json \"%s\" \"%s\"", person_p -> pe_name_s, person_p -> pe_email_s);
		}

	return success_flag;
}


Person *GetPersonFromCompoundJSON (const json_t *parent_p, const char * const key_s, const ViewFormat format, const FieldTrialServiceData *data_p)
{
	Person *person_p = NULL;
	const json_t *child_p = json_object_get (parent_p, key_s);

	if (child_p)
		{
			person_p = GetPersonFromJSON (child_p, format, data_p);
		}

	return person_p;
}



json_t *GetPersonAsJSON (const Person * const person_p, const ViewFormat format, const FieldTrialServiceData *data_p)
{
	json_t *person_json_p = json_object ();

	if (person_json_p)
		{
			if (SetNonTrivialString (person_json_p, PE_NAME_S, person_p -> pe_name_s))
				{
					if (SetNonTrivialString (person_json_p, PE_EMAIL_S, person_p -> pe_email_s))
						{
							if (SetJSONString (person_json_p, INDEXING_TYPE_S, CONTEXT_PREFIX_SCHEMA_ORG_S "Person"))
								{
									return person_json_p;
								}
						}
				}

			json_decref (person_json_p);
		}

	return NULL;
}




Person *GetPersonFromJSON (const json_t *json_p, const ViewFormat format, const FieldTrialServiceData *data_p)
{
	const char *name_s = GetJSONString (json_p, PE_NAME_S);
	const char *email_s = GetJSONString (json_p, PE_EMAIL_S);

	Person *person_p = AllocatePerson (name_s, email_s);

	return person_p;
}


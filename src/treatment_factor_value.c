/*
** Copyright 2014-2020 The Earlham Institute
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
 * treatment_factor_value.c
 *
 *  Created on: 25 Jan 2021
 *      Author: billy
 */


#include "treatment_factor_value.h"
#include "memory_allocations.h"
#include "string_utils.h"

#include "study.h"
#include "study_jobs.h"


static const char * const S_TFV_LABEL_S = "label";


TreatmentFactorValue *AllocateTreatmentFactorValue (TreatmentFactor *treatment_factor_p, const char *label_s)
{
	char *copied_label_s = EasyCopyToNewString (label_s);

	if (copied_label_s)
		{
			TreatmentFactorValue *value_p = (TreatmentFactorValue *) AllocMemory (sizeof (TreatmentFactorValue));

			if (value_p)
				{
					value_p -> tfv_factor_p = treatment_factor_p;
					value_p -> tfv_label_s = copied_label_s;

					return value_p;
				}

			FreeCopiedString (copied_label_s);
		}		/* if (copied_label_s) */

	return NULL;
}



void FreeTreatmentFactorValue (TreatmentFactorValue *treatment_factor_value_p)
{
	FreeCopiedString (treatment_factor_value_p -> tfv_label_s);
	FreeMemory (treatment_factor_value_p);
}


TreatmentFactorValueNode *AllocateTreatmentFactorValueNode (TreatmentFactorValue *treatment_factor_value_p)
{
	TreatmentFactorValueNode *node_p = (TreatmentFactorValueNode *) AllocMemory (sizeof (TreatmentFactorValueNode));

	if (node_p)
		{
			InitListItem (& (node_p -> tfvn_node));
			node_p -> tfvn_value_p = treatment_factor_value_p;
		}

	return node_p;

}


void FreeTreatmentFactorValueNode (ListItem *node_p)
{
	TreatmentFactorValueNode *tfv_node_p = (TreatmentFactorValueNode *) node_p;

	FreeTreatmentFactorValue (tfv_node_p -> tfvn_value_p);
	FreeMemory (tfv_node_p);
}


const char *GetTreatmentFactorLabelValue (const TreatmentFactorValue *treatment_factor_value_p)
{
	return GetTreatmentFactorValue (treatment_factor_value_p -> tfv_factor_p, treatment_factor_value_p -> tfv_label_s);
}


bool AreTreatmentFactorValuesMatching (const TreatmentFactorValue *tfv_0_p, const TreatmentFactorValue *tfv_1_p)
{
	bool match_flag = false;
	const bson_oid_t *id_0_p = GetTreatmentIdForTreatmentFactorValue (tfv_0_p);

	if (id_0_p)
		{
			const bson_oid_t *id_1_p = GetTreatmentIdForTreatmentFactorValue (tfv_1_p);

			if (id_1_p)
				{
					if (bson_oid_equal (id_0_p, id_1_p))
						{
							match_flag = true;
						}
				}
		}

	return match_flag;
}


const bson_oid_t *GetTreatmentIdForTreatmentFactorValue (const TreatmentFactorValue *tfv_p)
{
	const TreatmentFactor *tf_p = tfv_p -> tfv_factor_p;

	if (tf_p)
		{
			const Treatment *treatment_p = tf_p -> tf_treatment_p;

			if (treatment_p)
				{
					return treatment_p -> tr_id_p;
				}
		}

	return NULL;
}


TreatmentFactorValue *GetTreatmentFactorValueFromJSON (const json_t *tf_value_json_p, const Study *study_p, const FieldTrialServiceData *data_p)
{
	TreatmentFactorValue *tfv_p = NULL;
	const char *treatment_url_s = GetJSONString (tf_value_json_p, SCHEMA_TERM_URL_S);

	if (treatment_url_s)
		{
			TreatmentFactor *factor_p = GetTreatmentFactorForStudyByUrl (study_p, treatment_url_s, data_p);

			if (factor_p)
				{
					const char *label_s = GetJSONString (tf_value_json_p, S_TFV_LABEL_S);

					if (label_s)
						{
							tfv_p = AllocateTreatmentFactorValue (factor_p, label_s);
						}

				}

		}

	return tfv_p;
}


json_t *GetTreatmentFactorValueAsJSON (const TreatmentFactorValue *tf_value_p, const Study *study_p)
{
	json_t *tfv_json_p = json_object ();

	if (tfv_json_p)
		{
			const char *treatment_url_s = GetTreatmentFactorUrl (tf_value_p -> tfv_factor_p);
			const char *label_s = tf_value_p -> tfv_label_s;

			if (SetJSONString (tfv_json_p, SCHEMA_TERM_URL_S, treatment_url_s))
				{
					if (SetJSONString (tfv_json_p, S_TFV_LABEL_S, label_s))
						{
							return tfv_json_p;
						}

				}

			json_decref (tfv_json_p);
		}		/* if (tfv_json_p) */

	return NULL;
}



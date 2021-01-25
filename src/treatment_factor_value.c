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

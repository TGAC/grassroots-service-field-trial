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
 * germplasm.h
 *
 *  Created on: 23 Jul 2018
 *      Author: billy
 */

#ifndef SERVICES_DFW_FIELD_TRIAL_SERVICE_INCLUDE_GERMPLASM_H_
#define SERVICES_DFW_FIELD_TRIAL_SERVICE_INCLUDE_GERMPLASM_H_


#include "dfw_field_trial_service_library.h"
#include "dfw_field_trial_service_data.h"
#include "typedefs.h"

#include "jansson.h"


typedef struct GeneBank
{
	bson_oid_t *gb_id_p;

	char *gb_name_s;

	char *gb_url_s;

	char *gb_api_url_s;

} GeneBank;


#ifndef DOXYGEN_SHOULD_SKIP_THIS

#ifdef ALLOCATE_GENE_BANK_TAGS
	#define GENE_BANK_PREFIX DFW_FIELD_TRIAL_SERVICE_LOCAL
	#define GENE_BANK_VAL(x)	= x
	#define GENE_BANK_CONCAT_VAL(x,y)	= x y
#else
	#define GENE_BANK_PREFIX extern
	#define GENE_BANK_VAL(x)
	#define GENE_BANK_CONCAT_VAL(x,y)
#endif

#endif 		/* #ifndef DOXYGEN_SHOULD_SKIP_THIS */




/* Location */



GENE_BANK_PREFIX const char *GB_NAME_S GENE_BANK_CONCAT_VAL (CONTEXT_PREFIX_SCHEMA_ORG_S, "name");

GENE_BANK_PREFIX const char *GB_URL_S GENE_BANK_CONCAT_VAL (CONTEXT_PREFIX_SCHEMA_ORG_S, "url");

GENE_BANK_PREFIX const char *GB_API_URL_S GENE_BANK_VAL ("api_url");





#ifdef __cplusplus
extern "C"
{
#endif


DFW_FIELD_TRIAL_SERVICE_LOCAL GeneBank *AllocateGeneBank (bson_oid_t *id_p, const char *name_s, const char *url_s, const char *api_url_s);


DFW_FIELD_TRIAL_SERVICE_LOCAL void FreeGeneBank (GeneBank *germplasm_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL json_t *GetGeneBankAsJSON (const GeneBank *gene_bank_p, const char * const api_query_s);

DFW_FIELD_TRIAL_SERVICE_LOCAL GeneBank *GetGeneBankFromJSON (const json_t *germplasm_json_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL bool SaveGeneBank (GeneBank *gene_bank_p, FieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL GeneBank *GetGeneBankById (const bson_oid_t *id_p, const ViewFormat format, const FieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL GeneBank *GetGeneBankByIdString (const char *gene_bank_id_s, const ViewFormat format, const FieldTrialServiceData *data_p);


DFW_FIELD_TRIAL_SERVICE_LOCAL GeneBank *GetGeneBankByName (const char *name_s, const FieldTrialServiceData *data_p);



#ifdef __cplusplus
}
#endif

#endif /* SERVICES_DFW_FIELD_TRIAL_SERVICE_INCLUDE_GERMPLASM_H_ */

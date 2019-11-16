/*
 * highlighter.h
 *
 *  Created on: 8 Nov 2019
 *      Author: billy
 */

#ifndef SERVICES_FIELD_TRIALS_INCLUDE_HIGHLIGHTER_H_
#define SERVICES_FIELD_TRIALS_INCLUDE_HIGHLIGHTER_H_


#include "dfw_field_trial_service_data.h"
#include "dfw_field_trial_service_library.h"

#include "jansson.h"


typedef struct SearchHighlighter
{
	char *sh_key_s;
	char *sh_value_s;
	bool (*dh_fn) (json_t *data_p);
} SearchHighlighter;


#ifdef __cplusplus
extern "C"
{
#endif






#ifdef __cplusplus
}
#endif




#endif /* SERVICES_FIELD_TRIALS_INCLUDE_HIGHLIGHTER_H_ */

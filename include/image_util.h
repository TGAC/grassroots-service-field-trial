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
 * image_util.h
 *
 *  Created on: 30 Mar 2020
 *      Author: billy
 */

#ifndef SERVICES_FIELD_TRIALS_INCLUDE_IMAGE_UTIL_H_
#define SERVICES_FIELD_TRIALS_INCLUDE_IMAGE_UTIL_H_

#include <time.h>

#include "dfw_field_trial_service_library.h"
#include "coordinate.h"


typedef struct ImageMetadata
{
	Coordinate *im_coord_p;

	struct tm *im_date_p;

	uint32 im_width;

	uint32 im_height;

} ImageMetadata;



#ifdef __cplusplus
extern "C"
{
#endif


DFW_FIELD_TRIAL_SERVICE_LOCAL ImageMetadata *GetImageMetadataForImageFile (const char *path_s);


DFW_FIELD_TRIAL_SERVICE_LOCAL ImageMetadata *AllocateImageMetadata (Coordinate *coord_p, struct tm *time_p, uint32 width, uint32 height);

DFW_FIELD_TRIAL_SERVICE_LOCAL void FreeImageMetadata (ImageMetadata *metadata_p);


#ifdef __cplusplus
}
#endif


#endif /* SERVICES_FIELD_TRIALS_INCLUDE_IMAGE_UTIL_H_ */

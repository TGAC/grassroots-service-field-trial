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
 * germplasm.c
 *
 *  Created on: 23 Jul 2018
 *      Author: billy
 */

#include "germplasm.h"

#include "memory_allocations.h"


static void FreeStringArray (char **array_ss, uint32 size);


Germplasm *AllocateGermplasm ()
{

}


void FreeGermplasm (Germplasm *germplasm_p)
{
	if (germplasm_p -> ge_sources_ss)
		{
			FreeStringArray (germplasm_p -> ge_sources_ss, germplasm_p -> ge_num_crosses);
		}

	if (germplasm_p -> ge_accessions_ss)
		{
			FreeStringArray (germplasm_p -> ge_accessions_ss, germplasm_p -> ge_num_crosses);
		}

	FreeMemory (germplasm_p);
}


json_t *GetGermplasmAsJSON (const Germplasm *germplasm_p)
{

}


static void FreeStringArray (char **array_ss, uint32 size)
{
	while (size > 0)
		{
			FreeCopiedString (*array_ss);

			-- size;
			++ array_ss;
		}

	FreeCopiedString (*array_ss);
	FreeMemory (array_ss);
}

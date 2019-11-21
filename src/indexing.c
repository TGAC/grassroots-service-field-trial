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
 * indexing.c
 *
 *  Created on: 23 Aug 2019
 *      Author: billy
 */

#include "indexing.h"
#include "study_jobs.h"
#include "location_jobs.h"
#include "field_trial_jobs.h"
#include "treatment_jobs.h"


bool IndexData (ServiceJob *job_p, const json_t *data_to_index_p)
{
	bool success_flag = false;
	GrassrootsServer *grassroots_p = GetGrassrootsServerFromService (job_p -> sj_service_p);
	LuceneTool *lucene_p = AllocateLuceneTool (grassroots_p, job_p -> sj_id);

	if (lucene_p)
		{
			if (IndexLucene (lucene_p, data_to_index_p, true))
				{
					success_flag = true;
				}

			FreeLuceneTool (lucene_p);
		}		/* if (lucene_p) */

	return success_flag;
}


OperationStatus ReindexAllData (ServiceJob *job_p, const DFWFieldTrialServiceData *service_data_p)
{
	OperationStatus status = OS_FAILED_TO_START;
	GrassrootsServer *grassroots_p = GetGrassrootsServerFromService (job_p -> sj_service_p);
	LuceneTool *lucene_p = AllocateLuceneTool (grassroots_p, job_p -> sj_id);

	if (lucene_p)
		{
			/* clear the index initially ...*/
			bool update_flag = false;
			uint32 num_succeeded = 0;
			const uint32 num_reindexes = 4;

			status = OS_SUCCEEDED;


			if (ReindexStudies (job_p, lucene_p, update_flag, service_data_p))
				{
					++ num_succeeded;
				}

			/* ... then update it from here */
			update_flag = true;

			if (ReindexTrials (job_p, lucene_p, update_flag, service_data_p))
				{
					++ num_succeeded;
				}

			if (ReindexLocations (job_p, lucene_p, update_flag, service_data_p))
				{
					++ num_succeeded;
				}

			if (ReindexTreatments (job_p, lucene_p, update_flag, service_data_p))
				{
					++ num_succeeded;
				}


			if (num_succeeded == num_reindexes)
				{
					status = OS_SUCCEEDED;
				}
			else if (num_succeeded > 0)
				{
					status = OS_PARTIALLY_SUCCEEDED;
				}
			else
				{
					status = OS_FAILED;
				}

			FreeLuceneTool (lucene_p);
		}		/* if (lucene_p) */

	SetServiceJobStatus (job_p, status);
}


bool ReindexStudies (ServiceJob *job_p, LuceneTool *lucene_p, bool update_flag, const DFWFieldTrialServiceData *service_data_p)
{
	bool success_flag = false;
	json_t *studies_p = GetAllStudiesAsJSONInViewFormat (service_data_p, VF_CLIENT_FULL);

	if (studies_p)
		{
			if (SetLuceneToolName (lucene_p, "index_studies"))
				{
					success_flag = IndexLucene (lucene_p, studies_p, update_flag);
				}

			json_decref (studies_p);
		}

	return success_flag;
}


bool ReindexLocations (ServiceJob *job_p, LuceneTool *lucene_p, bool update_flag, const DFWFieldTrialServiceData *service_data_p)
{
	bool success_flag = false;
	json_t *locations_p = GetAllLocationsAsJSON (service_data_p, NULL);

	if (locations_p)
		{
			if (SetLuceneToolName (lucene_p, "index_locations"))
				{
					success_flag = IndexLucene (lucene_p, locations_p, update_flag);
				}

			json_decref (locations_p);
		}

	return success_flag;
}


bool ReindexTrials (ServiceJob *job_p, LuceneTool *lucene_p, bool update_flag, const DFWFieldTrialServiceData *service_data_p)
{
	bool success_flag = false;
	json_t *trials_p = GetAllFieldTrialsAsJSON (service_data_p, NULL);

	if (trials_p)
		{
			if (SetLuceneToolName (lucene_p, "index_trials"))
				{
					success_flag = IndexLucene (lucene_p, trials_p, update_flag);
				}

			json_decref (trials_p);
		}

	return success_flag;
}



bool ReindexTreatments (ServiceJob *job_p, LuceneTool *lucene_p, bool update_flag, const DFWFieldTrialServiceData *service_data_p)
{
	bool success_flag = false;
	json_t *trials_p = GetAllTreatmentsAsJSON (service_data_p, NULL);

	if (trials_p)
		{
			if (SetLuceneToolName (lucene_p, "index_treatments"))
				{
					success_flag = IndexLucene (lucene_p, trials_p, update_flag);
				}

			json_decref (trials_p);
		}

	return success_flag;
}

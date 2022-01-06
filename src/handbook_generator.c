
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
 * handbook_generator.c
 *
 *  Created on: 4 Jan 2022
 *      Author: billy
 */

#include <stdio.h>

#include "string_utils.h"
#include "byte_buffer.h"

#include "programme.h"
#include "handbook_generator.h"

static bool InsertLatexTabularRow (FILE *out_f, const char * const key_s, const char * const value_s);
static bool InsertPersonAsLatexTabularRow (FILE *out_f, const char * const key_s, const Person * const person_p);
static bool InsertLatexTabularRowAsHyperlink (FILE *out_f, const char * const key_s, const char * const value_s);
static bool InsertLatexTabularRowAsUint (FILE *out_f, const char * const key_s, const uint32 * const value_p);
static bool EscapeLatexCharacters (ByteBuffer *buffer_p, const char *input_s);



OperationStatus GenerateStudyAsPDF (const Study *study_p, FieldTrialServiceData *data_p)
{
	OperationStatus status = OS_FAILED;
	char *id_s = GetBSONOidAsString (study_p -> st_id_p);

	if (id_s)
		{
			char *study_filename_s = ConcatenateStrings (id_s, ".tex");

			if (study_filename_s)
				{
					const char *output_dir_s = "/home/billy/Desktop";
					char *full_filename_s = MakeFilename (output_dir_s, study_filename_s);

					if (full_filename_s)
						{
							FILE *study_tex_f = fopen (full_filename_s, "w");

							if (study_tex_f)
								{
									const FieldTrial *trial_p = study_p -> st_parent_p;
									const Programme *programme_p = trial_p ? trial_p -> ft_parent_p : NULL;

									/* start the doc and use a sans serif font */
									fputs ("\\documentclass[a4paper,12pt]{article}\n", study_tex_f);

									fputs ("\\usepackage{tabularx}\n", study_tex_f);
									fputs ("\\usepackage{hyperref}\n", study_tex_f);
									fputs ("\\usepackage[margin=0.6in]{geometry}\n", study_tex_f);

									fputs ("\\urlstyle{same}\n\\hypersetup{\n\tcolorlinks=true,\n\tlinkcolor=blue,\n\tfilecolor=blue,\n\turlcolor=blue,\n", study_tex_f);

									fprintf (study_tex_f, "\tpdftitle={%s}\n}\n", study_p -> st_name_s);


									fprintf (study_tex_f, "\\title {%s}\n", study_p -> st_name_s);

									fputs ("\\begin{document}\n\\sffamily\n", study_tex_f);

									fputs ("\\maketitle\n", study_tex_f);

									if (programme_p)
										{
											fputs ("\\section* {Programme}\n", study_tex_f);

											fputs ("\\begin{tabularx}{1\\textwidth}{l X}\n", study_tex_f);

											InsertLatexTabularRow (study_tex_f, "Name", programme_p -> pr_name_s);
											InsertLatexTabularRow (study_tex_f, "Objective", programme_p -> pr_objective_s);

											InsertPersonAsLatexTabularRow (study_tex_f, "Principal Investigator", programme_p -> pr_pi_p);

											InsertLatexTabularRowAsHyperlink (study_tex_f, "Web Address", programme_p -> pr_documentation_url_s);

									    fputs ("\\end{tabularx}\n", study_tex_f);
										}		/* if (trial_p) */


									if (trial_p)
										{
											fputs ("\\section* {Field Trial}\n", study_tex_f);

											fputs ("\\begin{tabularx}{1\\textwidth}{l X}\n", study_tex_f);

									    InsertLatexTabularRow (study_tex_f, "Name", trial_p -> ft_name_s);
											InsertLatexTabularRow (study_tex_f, "Team", trial_p -> ft_team_s);

									    fputs ("\\end{tabularx}\n", study_tex_f);
										}		/* if (trial_p) */


									fputs ("\\section* {Study}\n", study_tex_f);

									fputs ("\\begin{tabularx}{1\\textwidth}{l X}\n", study_tex_f);

									InsertLatexTabularRow (study_tex_f, "Name", study_p -> st_name_s);
									InsertLatexTabularRow (study_tex_f, "Description", study_p -> st_description_s);

									InsertLatexTabularRowAsUint (study_tex_f, "Sowing Year", study_p -> st_predicted_sowing_year_p);
									InsertLatexTabularRowAsUint (study_tex_f, "Harvest Year", study_p -> st_predicted_harvest_year_p);


									InsertLatexTabularRowAsHyperlink (study_tex_f, "Web Address", study_p -> st_data_url_s);

									InsertLatexTabularRow (study_tex_f, "Design", study_p -> st_design_s);
									InsertLatexTabularRow (study_tex_f, "Growing Conditions", study_p -> st_growing_conditions_s);

									InsertPersonAsLatexTabularRow (study_tex_f, "Contact", study_p -> st_contact_p);
									InsertPersonAsLatexTabularRow (study_tex_f, "Curator", study_p -> st_curator_p);

									InsertLatexTabularRow (study_tex_f, "Phenotype Gathering Notes", study_p -> st_phenotype_gathering_notes_s);
									InsertLatexTabularRow (study_tex_f, "Physical Samples Collected", study_p -> st_physical_samples_collected_s);

									InsertLatexTabularRow (study_tex_f, "Changes to Plan", study_p -> st_plan_changes_s);
									InsertLatexTabularRow (study_tex_f, "Data not stored in Grassroots", study_p -> st_data_not_included_s);


									InsertLatexTabularRow (study_tex_f, "Weather", study_p -> st_weather_link_s);


							    fputs ("\\end{tabularx}\n", study_tex_f);

									fputs ("\\end{document}\n", study_tex_f);
									fclose (study_tex_f);
								}		/* if (study_tex_f) */

						}		/* if (full_filename_s) */


					FreeCopiedString (study_filename_s);
				}		/* if (study_filename_s) */

			FreeCopiedString (id_s);
		}		/* if (id_s) */


	return status;
}


static bool InsertLatexTabularRowAsUint (FILE *out_f, const char * const key_s, const uint32 * const value_p)
{
	bool success_flag = true;

  if (value_p)
  	{
			success_flag = (fprintf (out_f, "\\textbf{%s}: & " UINT32_FMT " \\\\\n", key_s, *value_p) > 0);
  	}

  return success_flag;
}


static bool InsertLatexTabularRow (FILE *out_f, const char * const key_s, const char * const value_s)
{
	bool success_flag = true;

  if (value_s)
  	{
			success_flag = (fprintf (out_f, "\\textbf{%s}: & %s \\\\\n", key_s, value_s) > 0);
  	}

  return success_flag;
}


static bool InsertLatexTabularRowAsHyperlink (FILE *out_f, const char * const key_s, const char * const value_s)
{
	bool success_flag = true;

  if (value_s)
  	{
			success_flag = (fprintf (out_f, "\\textbf{%s}: & \\url{%s} \\\\\n", key_s, value_s) > 0);
  	}

  return success_flag;
}



static bool InsertPersonAsLatexTabularRow (FILE *out_f, const char * const key_s, const Person * const person_p)
{
	bool success_flag = false;

  if (person_p)
  	{
			success_flag = (fprintf (out_f, "\\textbf{%s}: & %s (\\href{mailto:%s}{%s}) \\\\\n", key_s, person_p -> pe_name_s, person_p -> pe_email_s, person_p -> pe_email_s) > 0);
  	}
  else
  	{
  		success_flag = true;
  	}

  return success_flag;
}


/*
 *

    & % $ # _ { } ~ ^ \

 */

static bool EscapeLatexCharacters (ByteBuffer *buffer_p, const char *input_s)
{
	const char * const chars_to_escape_p = "&%%$#_{}~^\\";
	bool loop_flag = (*input_s != '\0');
	bool success_flag = true;

	while (loop_flag && success_flag)
		{
			if (strchr (chars_to_escape_p, *input_s))
				{
					if (AppendToByteBuffer (buffer_p, input_s, 1))
						{
							++ input_s;

							loop_flag = (*input_s != '\0');
						}
				}
		}


	return success_flag;
}

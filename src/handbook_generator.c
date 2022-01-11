
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
#include "treatment_factor.h"

typedef enum
{
	CS_NORMAL,
	CS_FIRST_WORD_ONLY,
	CS_ALL_WORDS,
	CS_NUM_MODES
} CapitalizeState;


static bool InsertLatexTabularRow (FILE *out_f, const char * const key_s, const char * const value_s, const CapitalizeState capitalize, ByteBuffer *buffer_p);
static bool InsertPersonAsLatexTabularRow (FILE *out_f, const char * const key_s, const Person * const person_p);
static bool InsertLatexTabularRowAsHyperlink (FILE *out_f, const char * const key_s, const char * const value_s);
static bool InsertLatexTabularRowAsUint (FILE *out_f, const char * const key_s, const uint32 * const value_p);
static bool InsertLatexTabularRowAsDouble (FILE *out_f, const char * const key_s, const double64 * const value_p);
static bool EscapeLatexCharacters (ByteBuffer *buffer_p, const char *input_s, const CapitalizeState capitalize);

static bool PrintStudy (FILE *study_tex_f, const Study * const study_p, ByteBuffer *buffer_p, FieldTrialServiceData *data_p);

static bool PrintProgramme (FILE *study_tex_f, const Programme * const programme_p, ByteBuffer *buffer_p, FieldTrialServiceData *data_p);

static bool PrintTrial (FILE *study_tex_f, const FieldTrial * const trial_p, ByteBuffer *buffer_p, FieldTrialServiceData *data_p);

static bool PrintLocation (FILE *study_tex_f, const Location * const location_p, ByteBuffer *buffer_p, FieldTrialServiceData *data_p);


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
									ByteBuffer *buffer_p = AllocateByteBuffer (4096);

									if (buffer_p)
										{

											const FieldTrial *trial_p = study_p -> st_parent_p;
											const Programme *programme_p = trial_p ? trial_p -> ft_parent_p : NULL;
											const Location *location_p = study_p -> st_location_p;

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
													PrintProgramme (study_tex_f, programme_p, buffer_p, data_p);
												}		/* if (programme_p) */


											if (trial_p)
												{
													PrintTrial (study_tex_f, trial_p, buffer_p, data_p);
												}		/* if (trial_p) */

											if (location_p)
												{
													PrintLocation (study_tex_f, location_p, buffer_p, data_p);
												}

											PrintStudy (study_tex_f, study_p, buffer_p, data_p);

											fputs ("\\end{document}\n", study_tex_f);

											FreeByteBuffer (buffer_p);
										}		/* if (buffer_p); */


									fclose (study_tex_f);
								}		/* if (study_tex_f) */

						}		/* if (full_filename_s) */


					FreeCopiedString (study_filename_s);
				}		/* if (study_filename_s) */

			FreeCopiedString (id_s);
		}		/* if (id_s) */


	return status;
}


static bool PrintStudy (FILE *study_tex_f, const Study * const study_p, ByteBuffer *buffer_p, FieldTrialServiceData *data_p)
{
	bool success_flag = true;
	TreatmentFactorNode *tf_node_p = (TreatmentFactorNode *) (study_p -> st_treatments_p -> ll_head_p);

	fputs ("\\section* {Study}\n", study_tex_f);

	fputs ("\\begin{tabularx}{1\\textwidth}{l X}\n", study_tex_f);

	InsertLatexTabularRow (study_tex_f, "Name", study_p -> st_name_s, CS_FIRST_WORD_ONLY, buffer_p);
	InsertLatexTabularRow (study_tex_f, "Description", study_p -> st_description_s, CS_FIRST_WORD_ONLY, buffer_p);

	InsertLatexTabularRowAsUint (study_tex_f, "Sowing Year", study_p -> st_predicted_sowing_year_p);
	InsertLatexTabularRowAsUint (study_tex_f, "Harvest Year", study_p -> st_predicted_harvest_year_p);

	InsertLatexTabularRowAsHyperlink (study_tex_f, "Web Address", study_p -> st_data_url_s);

	if (study_p -> st_current_crop_p)
		{
			InsertLatexTabularRow (study_tex_f, "Current Crop", study_p -> st_current_crop_p -> cr_name_s, CS_ALL_WORDS, buffer_p);
		}

	if (study_p -> st_current_crop_p)
		{
			InsertLatexTabularRow (study_tex_f, "Previous Crop", study_p -> st_previous_crop_p -> cr_name_s, CS_ALL_WORDS, buffer_p);
		}


	InsertLatexTabularRow (study_tex_f, "Design", study_p -> st_design_s, CS_FIRST_WORD_ONLY, buffer_p);
	InsertLatexTabularRow (study_tex_f, "Growing Conditions", study_p -> st_growing_conditions_s, CS_FIRST_WORD_ONLY, buffer_p);

	InsertPersonAsLatexTabularRow (study_tex_f, "Contact", study_p -> st_contact_p);
	InsertPersonAsLatexTabularRow (study_tex_f, "Curator", study_p -> st_curator_p);

	InsertLatexTabularRow (study_tex_f, "Phenotype Gathering Notes", study_p -> st_phenotype_gathering_notes_s, CS_FIRST_WORD_ONLY, buffer_p);
	InsertLatexTabularRow (study_tex_f, "Physical Samples Collected", study_p -> st_physical_samples_collected_s, CS_FIRST_WORD_ONLY, buffer_p);

	InsertLatexTabularRow (study_tex_f, "Changes to Plan", study_p -> st_plan_changes_s, CS_FIRST_WORD_ONLY, buffer_p);
	InsertLatexTabularRow (study_tex_f, "Data not stored in Grassroots", study_p -> st_data_not_included_s, CS_FIRST_WORD_ONLY, buffer_p);


	InsertLatexTabularRow (study_tex_f, "Weather", study_p -> st_weather_link_s, CS_FIRST_WORD_ONLY, buffer_p);

  fputs ("\\end{tabularx}\n", study_tex_f);
	/*
	 * Do we have any treatment factors?
	 */
	if (study_p -> st_treatments_p -> ll_size > 0)
		{
			ByteBuffer *names_buffer_p = AllocateByteBuffer (256);

			if (names_buffer_p)
				{
					fputs ("\\subsection* {Treatment Factors}\n", study_tex_f);

					while (tf_node_p)
						{
							TreatmentFactor *tf_p = tf_node_p -> tfn_p;
							const char *tf_name_s = GetTreatmentFactorName (tf_p);

							//InsertLatexTabularRow (study_tex_f, "Name", tf_name_s, buffer_p);

							if (EscapeLatexCharacters (names_buffer_p, tf_name_s, CS_ALL_WORDS))
								{
									tf_name_s = GetByteBufferData (names_buffer_p);
									fprintf (study_tex_f, "\\subsubsection* {%s}\n", tf_name_s);

								  ResetByteBuffer (names_buffer_p);

									fputs ("\\begin{tabularx}{1\\textwidth}{l X}\n", study_tex_f);

									KeyValuePairNode *kvp_node_p = (KeyValuePairNode *) (tf_p -> tf_values_p -> ll_head_p);

									while (kvp_node_p)
										{
											KeyValuePair *pair_p = kvp_node_p -> kvpn_pair_p;

											InsertLatexTabularRow (study_tex_f, pair_p -> kvp_key_s, pair_p -> kvp_value_s, CS_FIRST_WORD_ONLY, buffer_p);
											kvp_node_p = (KeyValuePairNode *) (kvp_node_p -> kvpn_node.ln_next_p);
										}

								  fputs ("\\end{tabularx}\n", study_tex_f);

								}

							tf_node_p = (TreatmentFactorNode *) (tf_node_p -> tfn_node.ln_next_p);
						}		/* while (tf_node_p) */

					FreeByteBuffer (names_buffer_p);
				}		/* if (names_buffer_p) */

		}




  return success_flag;
}


static bool PrintProgramme (FILE *study_tex_f, const Programme * const programme_p, ByteBuffer *buffer_p, FieldTrialServiceData *data_p)
{
	bool success_flag = true;

	fputs ("\\section* {Programme}\n", study_tex_f);

	fputs ("\\begin{tabularx}{1\\textwidth}{l X}\n", study_tex_f);

	InsertLatexTabularRow (study_tex_f, "Name", programme_p -> pr_name_s, CS_FIRST_WORD_ONLY, buffer_p);
	InsertLatexTabularRow (study_tex_f, "Objective", programme_p -> pr_objective_s, CS_FIRST_WORD_ONLY, buffer_p);

	InsertPersonAsLatexTabularRow (study_tex_f, "Principal Investigator", programme_p -> pr_pi_p);

	InsertLatexTabularRowAsHyperlink (study_tex_f, "Web Address", programme_p -> pr_documentation_url_s);

	if (programme_p -> pr_crop_p)
		{
			InsertLatexTabularRow (study_tex_f, "Crop", programme_p -> pr_crop_p -> cr_name_s, CS_ALL_WORDS, buffer_p);
		}

  fputs ("\\end{tabularx}\n", study_tex_f);

  return success_flag;
}


static bool PrintTrial (FILE *study_tex_f, const FieldTrial * const trial_p, ByteBuffer *buffer_p, FieldTrialServiceData *data_p)
{
	bool success_flag = true;

	fputs ("\\section* {Field Trial}\n", study_tex_f);

	fputs ("\\begin{tabularx}{1\\textwidth}{l X}\n", study_tex_f);

  InsertLatexTabularRow (study_tex_f, "Name", trial_p -> ft_name_s, CS_FIRST_WORD_ONLY, buffer_p);
	InsertLatexTabularRow (study_tex_f, "Team", trial_p -> ft_team_s, CS_FIRST_WORD_ONLY, buffer_p);

  fputs ("\\end{tabularx}\n", study_tex_f);


  return success_flag;
}

static bool PrintLocation (FILE *study_tex_f, const Location * const location_p, ByteBuffer *buffer_p, FieldTrialServiceData *data_p)
{
	bool success_flag = true;

	fputs ("\\section* {Location}\n", study_tex_f);

	fputs ("\\begin{tabularx}{1\\textwidth}{l X}\n", study_tex_f);

	if (location_p -> lo_address_p)
		{
			const Coordinate *centre_p = location_p -> lo_address_p -> ad_gps_centre_p;
			char *address_s = GetAddressAsString (location_p -> lo_address_p);

			if (address_s)
				{
			    InsertLatexTabularRow (study_tex_f, "Address", address_s, CS_NORMAL, buffer_p);

			    FreeCopiedString (address_s);
				}

			if (centre_p)
				{
					InsertLatexTabularRowAsDouble (study_tex_f, "Latitude", & (centre_p -> co_x));
					InsertLatexTabularRowAsDouble (study_tex_f, "Longitude", & (centre_p -> co_y));
					InsertLatexTabularRowAsDouble (study_tex_f, "Elevation", centre_p -> co_elevation_p);
				}
		}

	InsertLatexTabularRow (study_tex_f, "Soil", location_p -> lo_soil_s, CS_FIRST_WORD_ONLY, buffer_p);

	InsertLatexTabularRowAsDouble (study_tex_f, "Minimum pH", location_p -> lo_min_ph_p);
	InsertLatexTabularRowAsDouble (study_tex_f, "Maximum pH", location_p -> lo_max_ph_p);

  fputs ("\\end{tabularx}\n", study_tex_f);


  return success_flag;
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


static bool InsertLatexTabularRowAsDouble (FILE *out_f, const char * const key_s, const double64 * const value_p)
{
	bool success_flag = true;

  if (value_p)
  	{
			success_flag = (fprintf (out_f, "\\textbf{%s}: & " DOUBLE64_FMT " \\\\\n", key_s, *value_p) > 0);
  	}

  return success_flag;
}


static bool InsertLatexTabularRow (FILE *out_f, const char * const key_s, const char * const value_s, const CapitalizeState capitalize, ByteBuffer *buffer_p)
{
	bool success_flag = true;

  if (value_s)
  	{
  		if (EscapeLatexCharacters (buffer_p, value_s, capitalize))
  			{
  				success_flag = (fprintf (out_f, "\\textbf{%s}: & %s \\\\\n", key_s, GetByteBufferData (buffer_p)) > 0);
  			}
  		else
  			{
  				success_flag = false;
  			}

  		ResetByteBuffer (buffer_p);
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

static bool EscapeLatexCharacters (ByteBuffer *buffer_p, const char *input_s, const CapitalizeState capitalize)
{
	const char * const chars_to_escape_p = "&%%$#_{}~^\\";
	bool loop_flag = (*input_s != '\0');
	bool success_flag = true;
	bool first_char_flag = true;
	bool space_flag = false;

	while (loop_flag && success_flag)
		{
			if (strchr (chars_to_escape_p, *input_s))
				{
					const char esc = '\\';

					if (!AppendToByteBuffer (buffer_p, &esc, 1))
						{
							success_flag = false;
						}

				}

			if (success_flag)
				{
					char c = *input_s;

					switch (capitalize)
						{
							case CS_FIRST_WORD_ONLY:
								{
									if (first_char_flag)
										{
											c = toupper (c);
											first_char_flag = false;
										}
								}
								break;

							case CS_ALL_WORDS:
								{
									if (isspace (*input_s))
										{
											space_flag = true;
										}
									else if (first_char_flag)
										{
											c = toupper (c);
											first_char_flag = false;
										}
									else if (space_flag)
										{
											c = toupper (c);
											space_flag = false;
										}
								}
								break;

							case CS_NORMAL:
							default:
								break;
						}

					if (AppendToByteBuffer (buffer_p, &c, 1))
						{
							++ input_s;

							loop_flag = (*input_s != '\0');
						}
				}
			else
				{
					success_flag = false;
				}
		}


	return success_flag;
}

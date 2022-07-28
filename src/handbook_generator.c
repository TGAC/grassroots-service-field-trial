
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
#include "study_jobs.h"
#include "measured_variable.h"
#include "phenotype_statistics.h"
#include "measured_variable_jobs.h"


typedef enum
{
	CS_NORMAL,
	CS_FIRST_WORD_ONLY,
	CS_ALL_WORDS,
	CS_NUM_MODES
} CapitalizeState;


static bool InsertLatexTabularRow (FILE *out_f, const char * const key_s, const char * const value_s, const CapitalizeState capitalize, ByteBuffer *buffer_p);
static bool InsertPersonAsLatexTabularRow (FILE *out_f, const char * const key_s, const Person * const person_p);
static bool InsertLatexTabularRowAsHyperlink (FILE *out_f, const char * const key_s, const char * const value_s, ByteBuffer *buffer_p);
static bool InsertLatexTabularRowAsUint (FILE *out_f, const char * const key_s, const uint32 * const value_p);
static bool InsertLatexTabularRowAsSizeT (FILE *out_f, const char * const key_s, const size_t * const value_p);

static bool InsertLatexTabularRowAsDouble (FILE *out_f, const char * const key_s, const double64 * const value_p);
static bool EscapeLatexCharacters (ByteBuffer *buffer_p, const char *input_s, const CapitalizeState capitalize);

static bool PrintStudy (FILE *study_tex_f, const Study * const study_p, ByteBuffer *buffer_p, FieldTrialServiceData *data_p);

static bool PrintProgramme (FILE *study_tex_f, const Programme * const programme_p, ByteBuffer *buffer_p, FieldTrialServiceData *data_p);

static bool PrintTrial (FILE *study_tex_f, const FieldTrial * const trial_p, ByteBuffer *buffer_p, FieldTrialServiceData *data_p);

static bool PrintLocation (FILE *study_tex_f, const Location * const location_p, ByteBuffer *buffer_p, FieldTrialServiceData *data_p);

static bool PrintJSONChildValue (FILE *study_tex_f, const json_t *parent_p, const char *key_s, const char *printed_key_s, CapitalizeState capitalize, ByteBuffer *buffer_p);

static char *DownloadToFile (const char *url_s, const char *download_directory_s, const char *filename_s);

static bool RunLatex (const char *pdf_latex_command_s, const char *output_path_s, const char *filename_s);



char *GetStudyHandbookFilename (const Study *study_p, const FieldTrialServiceData *data_p)
{
	char *full_filename_s = NULL;

	if (data_p -> dftsd_assets_path_s)
		{
			char *study_filename_s = ConcatenateStrings (study_p -> st_name_s, ".tex");

			if (study_filename_s)
				{
					full_filename_s = MakeFilename (data_p -> dftsd_assets_path_s, study_filename_s);

					if (!full_filename_s)
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get full filename for \"%s\" & \"%s\"", data_p -> dftsd_assets_path_s, study_filename_s);
						}

					FreeCopiedString (study_filename_s);
				}		/* if (study_filename_s) */
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get latex filename for \"%s\"", study_p -> st_name_s);
				}

		}		/** if (data_p -> dftsd_assets_path_s) */
	else
		{
			PrintLog (STM_LEVEL_INFO, __FILE__, __LINE__, "No path specified for field trials assets so cannot generate handbook");
		}



	return full_filename_s;
}


OperationStatus GenerateStudyAsPDF (const Study *study_p, FieldTrialServiceData *data_p)
{
	OperationStatus status = OS_FAILED;
	char *full_filename_s = GetStudyHandbookFilename (study_p, data_p);

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
							const char *download_path_s = data_p -> dftsd_assets_path_s;

							/* start the doc and use a sans serif font */
							fputs ("\\documentclass[a4paper,12pt]{article}\n", study_tex_f);

							fputs ("\\usepackage{tabularx}\n", study_tex_f);
							fputs ("\\usepackage{hyperref}\n", study_tex_f);
							fputs ("\\usepackage{bookmark}\n", study_tex_f);
							fputs ("\\usepackage[margin=0.6in]{geometry}\n", study_tex_f);

							fputs ("\\usepackage{sectsty}\n", study_tex_f);


							fputs ("\\urlstyle{same}\n\\hypersetup{\n\tcolorlinks=true,\n\tlinkcolor=blue,\n\tfilecolor=blue,\n\turlcolor=blue,\n", study_tex_f);
							fprintf (study_tex_f, "\tpdftitle={%s}\n}\n", study_p -> st_name_s);

							fputs ("\\usepackage{float}\n", study_tex_f);
							fputs ("\\usepackage{graphicx}\n", study_tex_f);
							fprintf (study_tex_f, "\\graphicspath{ {%s} }\n", download_path_s);

							fputs ("\n\\usepackage{calc}\n", study_tex_f);
							fputs ("\\newlength{\\imgwidth}\n\n", study_tex_f);

							fputs ("\\newcommand\\scalegraphics[1]{\%\n", study_tex_f);
							fputs ("\t\\settowidth{\\imgwidth}{\\includegraphics{#1}}\%\n", study_tex_f);
							fputs ("\t\\setlength{\\imgwidth}{\\minof{\\imgwidth}{\\textwidth}}\%\n", study_tex_f);
							fputs ("\t\\includegraphics[width=\\imgwidth,keepaspectratio]{#1}\%\n}\n", study_tex_f);



							if (programme_p)
								{
									if (programme_p -> pr_logo_url_s)
										{
											char *id_s = GetBSONOidAsString (programme_p -> pr_id_p);

											if (id_s)
												{
													char *image_s = DownloadToFile (programme_p -> pr_logo_url_s, download_path_s, id_s);

													if (image_s)
														{
															char *last_dot_p = strrchr (image_s, '.');

															/*
															 * includegraphics doesn;t use the extension so
															 * remove it if it's there
															 */
															if (last_dot_p)
																{
																	*last_dot_p = '\0';
																}

															fputs ("\\usepackage{titling}\n", study_tex_f);

															fputs ("\\pretitle{%\n\\begin{center}\n\\LARGE\n}\n", study_tex_f);

															fputs ("\\posttitle{\%\n\\begin{figure}[H]\n\\centering\n", study_tex_f);

															fprintf (study_tex_f, "\\scalegraphics{%s}\n", image_s);

															fputs ("\\end{figure}\n\\end{center}\n}\n", study_tex_f);

															FreeCopiedString (image_s);
														}


													FreeBSONOidString (id_s);
												}

										}

								}		/* if (programme_p) */

							fprintf (study_tex_f, "\\title {%s}\n", study_p -> st_name_s);

							fputs ("\\begin{document}\n\\sffamily\n\\allsectionsfont{\\sffamily}\n", study_tex_f);

							fputs ("\\maketitle\n", study_tex_f);

							if (data_p -> dftsd_view_study_url_s)
								{
									char *id_s = GetBSONOidAsString (study_p -> st_id_p);

									if (id_s)
										{
											fprintf (study_tex_f, "To view this study online, go to \\url{%s%s}\n", data_p -> dftsd_view_study_url_s, id_s);

											FreeBSONOidString (id_s);
										}
								}

							if (programme_p)
								{
									if (!PrintProgramme (study_tex_f, programme_p, buffer_p, data_p))
										{
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add programme to \"%s\"", full_filename_s);
										}
								}		/* if (programme_p) */


							if (trial_p)
								{
									if (!PrintTrial (study_tex_f, trial_p, buffer_p, data_p))
										{
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add trial to \"%s\"", full_filename_s);
										}

								}		/* if (trial_p) */


							if (!PrintStudy (study_tex_f, study_p, buffer_p, data_p))
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add study to \"%s\"", full_filename_s);
								}


							if (location_p)
								{
									if (!PrintLocation (study_tex_f, study_p -> st_location_p, buffer_p, data_p))
										{
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add location to \"%s\"", full_filename_s);
										}
								}

							fputs ("\\end{document}\n", study_tex_f);

							FreeByteBuffer (buffer_p);
						}		/* if (buffer_p); */
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate byte buffer \"%s\"", full_filename_s);
						}

					fclose (study_tex_f);

					RunLatex (data_p -> dftsd_latex_commmand_s, data_p -> dftsd_assets_path_s, full_filename_s);


				}		/* if (study_tex_f) */
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get open file \"%s\"", full_filename_s);
				}

		}		/* if (full_filename_s) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get full filename for \"%s\" & \"%s\"", data_p -> dftsd_assets_path_s, study_p -> st_name_s);
		}

	return status;
}



static bool RunLatex (const char *pdf_latex_command_s, const char *output_path_s, const char *latex_filename_s)
{
	bool success_flag = false;
	/*
		 pdflatex -interaction=nonstopmode -output-directory=/home/billy/Applications/apache/htdocs/grassroots/frictionless/ ~/Applications/apache/htdocs/grassroots/frictionless/5f8eebc202700f64852ae919.tex
		 latexmk -c -cd ~/Applications/apache/htdocs/grassroots/frictionless/5f8eebc202700f64852ae919.tex
	 */
	char *command_s = ConcatenateVarargsStrings (pdf_latex_command_s, " -interaction=nonstopmode -output-directory=", output_path_s, " \"", latex_filename_s, "\"", NULL);

	if (command_s)
		{
			int res = system (command_s);

			if (res == 0)
				{
					success_flag = true;
				}
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "\"%s\" failed with res %d", command_s, res);
				}

			FreeCopiedString (command_s);
		}
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to generate latex command for \"%s\", \"%s\" and \"%s\"", pdf_latex_command_s, output_path_s, latex_filename_s);
		}

	return success_flag;
}



static char *DownloadToFile (const char *url_s, const char *download_directory_s, const char *prefix_s)
{
	char *result_s = NULL;
	char *full_output_filename_s = NULL;

	if (EnsureDirectoryExists (download_directory_s))
		{
			const char *local_url_s = strrchr (url_s, '/');

			if (local_url_s)
				{
					++ local_url_s;

					if (*local_url_s == '\0')
						{
							local_url_s = NULL;
						}
				}

			if (prefix_s)
				{
					if (local_url_s)
						{
							char *local_filename_s = ConcatenateStrings (prefix_s, local_url_s);

							if (local_filename_s)
								{
									full_output_filename_s = MakeFilename (download_directory_s, local_filename_s);

									FreeCopiedString (local_filename_s);
								}
						}
				}
			else
				{
					if (local_url_s)
						{
							full_output_filename_s = MakeFilename (download_directory_s, local_url_s);
						}

				}

			if (full_output_filename_s)
				{
					CurlTool *curl_tool_p = AllocateCurlTool (CM_MEMORY);

					if (curl_tool_p)
						{
							if (SetUriForCurlTool (curl_tool_p, url_s))
								{
									CURLcode res = curl_easy_setopt (curl_tool_p -> ct_curl_p, CURLOPT_BUFFERSIZE, CURL_MAX_READ_SIZE);

									res = RunCurlTool (curl_tool_p);

									if (res == CURLE_OK)
										{
											FILE *out_f = fopen (full_output_filename_s, "w");

											if (out_f)
												{
													size_t length = GetCurlToolDataSize (curl_tool_p);
													const char *data_p = GetCurlToolData (curl_tool_p);

													size_t s = fwrite (data_p, sizeof (char), length, out_f);

													if (s == length)
														{
															int r = fclose (out_f);

															if (r == 0)
																{
																	result_s = full_output_filename_s;
																}
														}


												}


										}
								}

							FreeCurlTool (curl_tool_p);
						}

					if (!result_s)
						{
							FreeCopiedString (full_output_filename_s);
						}
				}		/* if (full_output_filename_s) */

		}


	return result_s;
}





static bool PrintStudy (FILE *study_tex_f, const Study * const study_p, ByteBuffer *buffer_p, FieldTrialServiceData *data_p)
{
	bool success_flag = true;
	TreatmentFactorNode *tf_node_p = (TreatmentFactorNode *) (study_p -> st_treatments_p -> ll_head_p);
	size_t num_plots = (size_t) GetNumberOfPlotsInStudy (study_p, data_p);

	fputs ("\\section* {Study}\n", study_tex_f);

	fputs ("\\begin{tabularx}{1\\textwidth}{l X}\n", study_tex_f);

	InsertLatexTabularRow (study_tex_f, "Name", study_p -> st_name_s, CS_FIRST_WORD_ONLY, buffer_p);
	InsertLatexTabularRow (study_tex_f, "Description", study_p -> st_description_s, CS_FIRST_WORD_ONLY, buffer_p);

	InsertLatexTabularRowAsUint (study_tex_f, "Sowing Year", study_p -> st_predicted_sowing_year_p);
	InsertLatexTabularRowAsUint (study_tex_f, "Harvest Year", study_p -> st_predicted_harvest_year_p);

	InsertLatexTabularRowAsHyperlink (study_tex_f, "Web Address", study_p -> st_data_url_s, buffer_p);

	if (study_p -> st_current_crop_p)
		{
			InsertLatexTabularRow (study_tex_f, "Current Crop", study_p -> st_current_crop_p -> cr_name_s, CS_ALL_WORDS, buffer_p);
		}

	if (study_p -> st_previous_crop_p)
		{
			InsertLatexTabularRow (study_tex_f, "Previous Crop", study_p -> st_previous_crop_p -> cr_name_s, CS_ALL_WORDS, buffer_p);
		}

	InsertLatexTabularRow (study_tex_f, "Design", study_p -> st_design_s, CS_FIRST_WORD_ONLY, buffer_p);
	InsertLatexTabularRow (study_tex_f, "Growing Conditions", study_p -> st_growing_conditions_s, CS_FIRST_WORD_ONLY, buffer_p);

	InsertPersonAsLatexTabularRow (study_tex_f, "Contact", study_p -> st_contact_p);
	InsertPersonAsLatexTabularRow (study_tex_f, "Curator", study_p -> st_curator_p);

	InsertLatexTabularRow (study_tex_f, "Phenotype Gathering Notes", study_p -> st_phenotype_gathering_notes_s, CS_FIRST_WORD_ONLY, buffer_p);
	InsertLatexTabularRow (study_tex_f, "Physical Samples Collected", study_p -> st_physical_samples_collected_s, CS_FIRST_WORD_ONLY, buffer_p);

	InsertLatexTabularRow (study_tex_f, "Changes to Plan", study_p -> st_aspect_s, CS_FIRST_WORD_ONLY, buffer_p);
	InsertLatexTabularRow (study_tex_f, "Data not stored in Grassroots", study_p -> st_data_not_included_s, CS_FIRST_WORD_ONLY, buffer_p);

	InsertLatexTabularRow (study_tex_f, "Image Collection Notes", study_p -> st_image_collection_notes_s, CS_FIRST_WORD_ONLY, buffer_p);


	InsertLatexTabularRow (study_tex_f, "Aspect", study_p -> st_aspect_s, CS_FIRST_WORD_ONLY, buffer_p);
	InsertLatexTabularRow (study_tex_f, "Slope", study_p -> st_slope_s, CS_FIRST_WORD_ONLY, buffer_p);


	InsertLatexTabularRowAsHyperlink (study_tex_f, "Weather", study_p -> st_weather_link_s, buffer_p);

	fputs ("\\end{tabularx}\n", study_tex_f);

	/* Plots */
	fputs ("\\subsection* {Layout}\n", study_tex_f);

	fputs ("\\begin{tabularx}{1\\textwidth}{l X}\n", study_tex_f);



	InsertLatexTabularRowAsSizeT (study_tex_f, "Number of Plots", &num_plots);

	InsertLatexTabularRowAsUint (study_tex_f, "Number of Rows", study_p -> st_num_rows_p);
	InsertLatexTabularRowAsUint (study_tex_f, "Number of Columns", study_p -> st_num_columns_p);
	InsertLatexTabularRowAsUint (study_tex_f, "Number of Replicates", study_p -> st_num_replicates_p);

	InsertLatexTabularRowAsDouble (study_tex_f, "Default Plot Width", study_p -> st_default_plot_width_p);
	InsertLatexTabularRowAsDouble (study_tex_f, "Default Plot Length", study_p -> st_default_plot_length_p);

	InsertLatexTabularRowAsUint (study_tex_f, "Plot Rows per Block", study_p -> st_plots_rows_per_block_p);
	InsertLatexTabularRowAsUint (study_tex_f, "Plot Columns per Block", study_p -> st_plots_columns_per_block_p);

	InsertLatexTabularRowAsDouble (study_tex_f, "Plot Horizontal Gap", study_p -> st_plot_horizontal_gap_p);
	InsertLatexTabularRowAsDouble (study_tex_f, "Plot Vertical Gap", study_p -> st_plot_vertical_gap_p);

	InsertLatexTabularRowAsDouble (study_tex_f, "Plot Block Horizontal Gap", study_p -> st_plot_block_horizontal_gap_p);
	InsertLatexTabularRowAsDouble (study_tex_f, "Plot Block Vertical Gap", study_p -> st_plot_block_vertical_gap_p);


	fputs ("\\end{tabularx}\n", study_tex_f);



	if (study_p -> st_photo_url_s)
		{
			char *id_s = GetBSONOidAsString (study_p -> st_id_p);

			if (id_s)
				{
					char *image_s = DownloadToFile (study_p -> st_photo_url_s, data_p -> dftsd_assets_path_s, id_s);

					if (image_s)
						{
							char *last_dot_p = strrchr (image_s, '.');

							/*
							 * includegraphics doesn't use the extension so
							 * remove it if it's there
							 */
							if (last_dot_p)
								{
									*last_dot_p = '\0';
								}

							fputs ("\\begin{center}\n\\begin{figure}[H]\n", study_tex_f);
							fprintf (study_tex_f, "\\scalegraphics{%s}\n", image_s);
							fputs ("\\end{figure}\n\\end{center}\n\n", study_tex_f);
						}

					FreeBSONOidString (id_s);
				}

		}


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


	if (study_p -> st_phenotypes_p -> ll_size > 0)
		{
			ByteBuffer *phenotypes_buffer_p = AllocateByteBuffer (256);

			if (phenotypes_buffer_p)
				{
					PhenotypeStatisticsNode *node_p = (PhenotypeStatisticsNode *) (study_p -> st_phenotypes_p -> ll_head_p);
					/*
					 {
					    "trait": {
					      "so:sameAs": "CO_321:0000005",
					      "so:name": "Aboveground biomass at maturity",
					      "so:description": "All above-ground biomass at maturity.",
					      "abbreviation": "BM"
					    },
					    "measurement": {
					      "so:sameAs": "CO_321:0001027",
					      "so:name": "BM Computation",
					      "so:description": "Cut all aboveground biomass in a predetermined area (A). Avoid border effects by sampling away from edges of plot. Biomass as other yield components can be calculated or measured individually (Bell and Fischer, 1994; Reynolds et al., 2001; Pask et al., 2012), decide which method suit better for your objectives."
					    },
					    "unit": {
					      "so:sameAs": "CO_321:0000432",
					      "so:name": "t/ha"
					    },
					    "variable": {
					      "so:sameAs": "CO_321:0001036",
					      "so:name": "BM_Calc_tha"
					    },
					    "@type": "Grassroots:MeasuredVariable",
					    "type_description": "Measured Variable"
					  },
					 */
					fputs ("\\subsection* {Measured Variables}\n", study_tex_f);

					while (node_p)
						{
							MEM_FLAG variable_mem = MF_ALREADY_FREED;
							MeasuredVariable *variable_p = GetMeasuredVariableByVariableName (node_p -> psn_measured_variable_name_s, &variable_mem, data_p);

							if (variable_p)
								{
									if (EscapeLatexCharacters (phenotypes_buffer_p, node_p -> psn_measured_variable_name_s, CS_NORMAL))
										{
											fprintf (study_tex_f, "\\subsubsection* {%s}\n", GetByteBufferData (phenotypes_buffer_p));
											ResetByteBuffer(phenotypes_buffer_p);
										}

									fputs ("\\begin{tabularx}{1\\textwidth}{l X}\n", study_tex_f);

									if (variable_p -> mv_trait_term_p)
										{
											InsertLatexTabularRow (study_tex_f, "Trait Name", variable_p -> mv_trait_term_p -> st_name_s, CS_NORMAL, buffer_p);
											InsertLatexTabularRow (study_tex_f, "Trait Description", variable_p -> mv_trait_term_p -> st_description_s, CS_FIRST_WORD_ONLY, buffer_p);
											InsertLatexTabularRow (study_tex_f, "Trait Abbreviation", variable_p -> mv_trait_term_p -> st_abbreviation_s, CS_NORMAL, buffer_p);
										}


									if (variable_p -> mv_measurement_term_p)
										{
											InsertLatexTabularRow (study_tex_f, "Measurement Name", variable_p -> mv_measurement_term_p -> st_name_s, CS_NORMAL, buffer_p);
											InsertLatexTabularRow (study_tex_f, "Measurement Description", variable_p -> mv_measurement_term_p -> st_description_s, CS_FIRST_WORD_ONLY, buffer_p);
										}

									if (variable_p -> mv_unit_term_p)
										{
											InsertLatexTabularRow (study_tex_f, "Unit Name", variable_p -> mv_unit_term_p -> st_name_s, CS_NORMAL, buffer_p);
										}

									fputs ("\\end{tabularx}\n", study_tex_f);


									if ((variable_mem == MF_DEEP_COPY) || (variable_mem == MF_SHALLOW_COPY))
										{
											FreeMeasuredVariable (variable_p);
										}

								}		/* if (variable_p) */

							if (node_p -> psn_stats_p)
								{
									const Statistics *stats_p = node_p -> psn_stats_p;
									fprintf (study_tex_f, "\\subsubsection* {Statistics}\n");

									fputs ("\\begin{tabularx}{1\\textwidth}{l X}\n", study_tex_f);

									InsertLatexTabularRowAsSizeT (study_tex_f, "Number of samples", & (stats_p -> st_population_size));
									InsertLatexTabularRowAsDouble (study_tex_f, "Minimum", & (stats_p -> st_min));
									InsertLatexTabularRowAsDouble (study_tex_f, "Arithmetic Mean", & (stats_p -> st_mean));
									InsertLatexTabularRowAsDouble (study_tex_f, "Maximum", & (stats_p -> st_max));
									InsertLatexTabularRowAsDouble (study_tex_f, "Standard Deviation", & (stats_p -> st_std_dev));
									InsertLatexTabularRowAsDouble (study_tex_f, "Variance", & (stats_p -> st_variance));
									InsertLatexTabularRowAsDouble (study_tex_f, "Sum", & (stats_p -> st_sum));

									fputs ("\\end{tabularx}\n", study_tex_f);
								}		/* if (node_p -> psn_stats_p) */

							node_p = (PhenotypeStatisticsNode *) (node_p -> psn_node.ln_next_p);
						}		/* while (node_p) */

					FreeByteBuffer (phenotypes_buffer_p);
				}		/* if (phenotypes_buffer_p) */

		}		/* if (study_p -> st_phenotypes_p -> ll_size > 0) */

	return success_flag;
}


static bool PrintJSONChildValue (FILE *study_tex_f, const json_t *parent_p, const char *key_s, const char *printed_key_s, CapitalizeState capitalize, ByteBuffer *buffer_p)
{
	bool success_flag = true;
	const char *value_s = GetJSONString (parent_p, key_s);

	if (value_s)
		{
			InsertLatexTabularRow (study_tex_f, printed_key_s, value_s, capitalize, buffer_p);
		}

	return success_flag;
}


static bool PrintProgramme (FILE *study_tex_f, const Programme * const programme_p, ByteBuffer *buffer_p, FieldTrialServiceData *data_p)
{
	bool success_flag = true;

	fputs ("\\section* {Programme}\n", study_tex_f);

	fputs ("\\begin{tabularx}{1\\textwidth}{l X}\n", study_tex_f);


	if (programme_p -> pr_logo_url_s)
		{

		}

	InsertLatexTabularRow (study_tex_f, "Name", programme_p -> pr_name_s, CS_FIRST_WORD_ONLY, buffer_p);
	InsertLatexTabularRow (study_tex_f, "Abbreviation", programme_p -> pr_abbreviation_s, CS_NORMAL, buffer_p);
	InsertLatexTabularRow (study_tex_f, "Objective", programme_p -> pr_objective_s, CS_FIRST_WORD_ONLY, buffer_p);

	InsertPersonAsLatexTabularRow (study_tex_f, "Principal Investigator", programme_p -> pr_pi_p);

	InsertLatexTabularRowAsHyperlink (study_tex_f, "Web Address", programme_p -> pr_documentation_url_s, buffer_p);

	if (programme_p -> pr_crop_p)
		{
			InsertLatexTabularRow (study_tex_f, "Crop", programme_p -> pr_crop_p -> cr_name_s, CS_ALL_WORDS, buffer_p);
		}

	fputs ("\\end{tabularx}\n", study_tex_f);

	return success_flag;
}


static bool PrintTrial (FILE *study_tex_f, const FieldTrial * const trial_p, ByteBuffer *buffer_p, FieldTrialServiceData *data_p)
{
	if (fputs ("\\section* {Field Trial}\n", study_tex_f) > 0)
		{
			if (fputs ("\\begin{tabularx}{1\\textwidth}{l X}\n", study_tex_f) > 0)
				{
					if (InsertLatexTabularRow (study_tex_f, "Name", trial_p -> ft_name_s, CS_FIRST_WORD_ONLY, buffer_p))
						{
							if (InsertLatexTabularRow (study_tex_f, "Team", trial_p -> ft_team_s, CS_FIRST_WORD_ONLY, buffer_p))
								{
									if (fputs ("\\end{tabularx}\n", study_tex_f) > 0)
										{
											return true;
										}
								}
						}
				}
		}

	return false;
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



static bool InsertLatexTabularRowAsSizeT (FILE *out_f, const char * const key_s, const size_t * const value_p)
{
	bool success_flag = true;

	if (value_p)
		{
			success_flag = (fprintf (out_f, "\\textbf{%s}: & " SIZET_FMT " \\\\\n", key_s, *value_p) > 0);
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


static bool InsertLatexTabularRowAsHyperlink (FILE *out_f, const char * const key_s, const char * const value_s, ByteBuffer *buffer_p)
{
	bool success_flag = true;

	if (value_s)
		{
			if (EscapeLatexCharacters (buffer_p, value_s, CS_NORMAL))
				{
					success_flag = (fprintf (out_f, "\\textbf{%s}: & \\url{%s} \\\\\n", key_s, value_s) > 0);
				}
			else
				{
					success_flag = false;
				}

			ResetByteBuffer (buffer_p);
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

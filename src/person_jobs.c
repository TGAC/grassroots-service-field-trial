#include "person_jobs.h"


static bool CopyAndAddStringValue (const char * const src_s, char **dest_ss);


bool AddPersonParameters (ParameterSet *params_p, ParameterGroup *group_p, LinkedList *existing_people_p, FieldTrialServiceData *data_p)
{
	bool success_flag = false;
	ParameterGroup *group_p = CreateAndAddParameterGroupToParameterSet ("Treatment Factors", true, & (data_p -> dftsd_base_data), params_p);

	if (group_p)
		{
			Parameter *name_param_p = NULL;
			Parameter *values_param_p = NULL;
			const char * const tf_name_display_name_s = "Treatment name";
			const char * const tf_name_description_s = "The name of the treatment";
			size_t num_people = 0;
			json_t *tf_json_p = NULL;

			if (existing_people_p)
				{
					num_people = existing_people_p -> ll_size;
				}

			if (num_people > 1)
				{
					const char **names_ss = NULL;
					const char **emails_ss = NULL;
					const char **roles_ss = NULL;
					const char **affiliations_ss = NULL;
					const char **orcids_ss = NULL;
				
					if (PopulateValues (existing_people_p, &names_ss, &emails_ss, &roles_ss, &affiliations_ss, &orcids_ss))
						{
							
						}		/* if (PopulateValues (existing_people_p, &names_ss, &emails_ss, &roles_ss, &affiliations_ss, &orcids_ss)) */

				}		/* if (num_treatments > 1) */
			else
				{
					if ((param_p = EasyCreateAndAddStringParameterToParameterSet (data_p, param_set_p, pi_group_p, PERSON_NAME.npt_type, PERSON_NAME.npt_name_s, "Person Name", "The name of the Person", pi.pe_name_s, PL_ALL)) != NULL)
						{
							param_p -> pa_required_flag = true;

							if ((param_p = EasyCreateAndAddStringParameterToParameterSet (data_p, param_set_p, pi_group_p, PERSON_EMAIL.npt_type, PERSON_EMAIL.npt_name_s, "Person Email", "The email address of the Person's lead", pi.pe_email_s, PL_ALL)) != NULL)
								{
									param_p -> pa_required_flag = true;

									if ((param_p = EasyCreateAndAddStringParameterToParameterSet (data_p, param_set_p, pi_group_p, PERSON_ROLE.npt_type, PERSON_ROLE.npt_name_s, "Person Role", "The role of the Person's lead", pi.pe_role_s, PL_ALL)) != NULL)
										{
											if ((param_p = EasyCreateAndAddStringParameterToParameterSet (data_p, param_set_p, pi_group_p, PERSON_AFFILATION.npt_type, PERSON_AFFILATION.npt_name_s, "Person Affiliation", "The affiliation of the Person's lead", pi.pe_affiliation_s, PL_ALL)) != NULL)
												{
													if ((param_p = EasyCreateAndAddStringParameterToParameterSet (data_p, param_set_p, pi_group_p, PERSON_ORCID.npt_type, PERSON_ORCID.npt_name_s, "Person OrCID", "The OrCID of the Person's lead", pi.pe_orcid_s, PL_ALL)) != NULL)
														{			
															success_flag = true; 
														}
													else
														{
															PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", PERSON_ORCID.npt_name_s);
														}
												}
											else
												{
													PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", PERSON_AFFILATION.npt_name_s);
												}
										}
									else
										{
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", PERSON_ROLE.npt_name_s);
										}

								}
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", PERSON_EMAIL.npt_name_s);
								}
						}
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add %s parameter", PERSON_NAME.npt_name_s);
						}
				}
		}

	return success_flag;
}


static bool PopulateValues (LinkedList *existing_people_p)
{
	const uint32 num_entries = existing_people_p -> ll_size;

	if (num_entries > 0)
		{
			char **existing_names_ss = (char **) AllocMemoryArray (num_entries, sizeof (char *));

			if (existing_names_ss)
				{
					char **existing_emails_ss = (char **) AllocMemoryArray (num_entries, sizeof (char *));

					if (existing_emails_ss)
							{
								char **existing_roles_ss = (char  **) AllocMemoryArray (num_entries, sizeof (char *));

								if (existing_roles_ss)
									{
										char **existing_affiliations_ss = (char **) AllocMemoryArray (num_entries, sizeof (char *));

										if (existing_affiliations_ss)
											{
												char **existing_orcids_ss = (char **) AllocMemoryArray (num_entries, sizeof (char *));

												if (existing_orcids_ss)
													{
														PersonNode *person_node_p = (PersonNode *) (existing_people_p -> ll_head_p);
														char **existing_name_ss = existing_names_ss;
														char **existing_email_ss = existing_emails_ss;
														char **existing_role_ss = existing_roles_ss;
														char **existing_affiliation_ss = existing_affiliations_ss;
														char **existing_orcid_ss = existing_orcids_ss;
													
														success_flag = true;

														while (person_node_p && success_flag)
															{
																Person *person_p = person_node_p -> pn_person_p;
															
																if (CopyAndAddStringValue (person_p -> pe_name_s, existing_name_ss))
																	{
																		if (CopyAndAddStringValue (person_p -> pe_email_s, existing_email_ss))
																			{
																				if (CopyAndAddStringValue (person_p -> pe_role_s, existing_role_ss))
																					{
																						if (CopyAndAddStringValue (person_p -> pe_affiliation_s, existing_affiliation_ss))
																							{
																								if (CopyAndAddStringValue (person_p -> pe_orcid_s, existing_orcid_ss))
																									{
																										person_node_p = (PersonNode *) (person_node_p -> pn_node.ln_next_p);

																										++ existing_name_ss;
																										++ existing_email_ss;
																										++ existing_role_ss;
																										++ existing_affiliation_ss;
																										++ existing_orcid_ss;
																									}
																								else
																									{
																										PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "CopyAndAddStringValue () failed for orcid \"%s\"", person_p -> pe_orcid_s);
																										success_flag = false;
																									}

																							}
																						else
																							{
																								PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "CopyAndAddStringValue () failed for affiliation \"%s\"", person_p -> pe_affiliation_s);
																								success_flag = false;
																							}
																							
																					}
																				else
																					{
																						PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "CopyAndAddStringValue () failed for role \"%s\"", person_p -> pe_role_s);
																						success_flag = false;
																					}

																			}
																		else
																			{
																				PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "CopyAndAddStringValue () failed for email \"%s\"", person_p -> pe_email_s);
																				success_flag = false;
																			}
																	}
																else
																	{
																		PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "CopyAndAddStringValue () failed for name \"%s\"", person_p -> pe_name_s);
																		success_flag = false;
																	}


															}		/* while (obs_node_p && loop_flag) */

														if (success_flag)
															{
																*existing_names_sss = existing_names_ss;
																*existing_emails_sss = existing_emails_ss;
																*existing_roles_sss = existing_roles_ss;
																*existing_affiliations_sss = existing_affiliations_ss;
																*existing_orcids_sss = existing_orcids_ss;

																*num_entries_p = num_entries;
																return true;
															}		/* if (success_flag) */

														FreeMemory (existing_orcids_ss);
													}		/* if (existing_phenotype_corrected_p) */
												else
													{
														PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate existing_phenotype_corrected_p");
													}

												FreeMemory (existing_affiliations_ss);
											}		/* if (existing_phenotype_end_dates_p) */
										else
											{
												PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate existing_phenotype_end_dates_p");
											}

										FreeMemory (existing_roles_ss);
									}		/* if (existing_phenotype_start_dates_p) */
								else
									{
										PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate existing_phenotype_start_dates_p");
									}

								FreeMemory (existing_emails_ss);
							}		/* if (existing_phenotype_values_p) */
						else
							{
								PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate existing_emails_ss");
							}

					FreeMemory (existing_mv_names_ss);
				}		/* if (existing_mv_names_p) */
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate existing_mv_names_p");
				}


		}
}


static bool CopyAndAddStringValue (const char * const src_s, char **dest_ss)
{
	bool success_flag = false;
	
	if (src_s)
		{
			char *copied_value_s = EasyCopyToNewString (src_s);

			if (copied_value_s)
				{
					*dest_ss = copied_value_s;
				}
		}
	else
		{
			*dest_ss = NULL;
			success_flag = true;
		}
		
	return success_flag;
}
/*******************************************************************************
FILE : parser.c

LAST MODIFIED : 4 December 2003

DESCRIPTION :
A module for supporting command parsing.
???DB.  What about allocate and deallocate
???DB.  Make the set functions all look for "?" instead of NULL parser_state ?
???DB.  Move variables into own module ?
???DB.  Help for set_char_flag ?
???DB.  Move fuzzy_string_compare to compare.c ?
???DB.  Move extract_token into mystring.h ?
==============================================================================*/
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is cmgui.
 *
 * The Initial Developer of the Original Code is
 * Auckland Uniservices Ltd, Auckland, New Zealand.
 * Portions created by the Initial Developer are Copyright (C) 2005
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include "command/parser.h"
#if 1
#include "configure/cmgui_configure.h"
#endif /* defined (1) */
#include "general/debug.h"
#include "general/mystring.h"
#include "general/object.h"
#include "general/indexed_list_private.h"
#include "general/message.h"
#include "user_interface/user_interface.h"

/* size of blocks allocated onto option table - to reduce number of reallocs */
#define OPTION_TABLE_ALLOCATE_SIZE 10

/*
Module types
------------
*/

struct Option_table
/*******************************************************************************
LAST MODIFIED : 23 December 1999

DESCRIPTION :
==============================================================================*/
{
	struct Modifier_entry *entry;
	char *help;
	int allocated_entries,number_of_entries,valid;
	/* store suboption_tables added to table for destroying with option_table */
	int number_of_suboption_tables;
	struct Option_table **suboption_tables;
}; /* struct Option_table */

enum Variable_operation_type
{
	ADD_VARIABLE_OPERATION,
	DIVIDE_VARIABLE_OPERATION,
	MULTIPLY_VARIABLE_OPERATION,
	SET_VARIABLE_OPERATION,
	SUBTRACT_VARIABLE_OPERATION
}; /* enum Variable_operation_type */

/* SAB.  First implementation of "assign variable" to conform to "new interpreter".
   These are string variables, are case sensitive and have global scope.
	The use of a variable is indicated by a $ symbol in any token. */
struct Assign_variable
{
	char *name;
	char *value;

	int access_count;
}; /* struct Assign_variable */

/*
Module variables
----------------
*/
/*???DB.  Initial go at variables of form %<f|i|z|l><#>% , where the %'s are
	required, f(loat) i(nteger) z(ero extended integer) l(ogical) are the variable
	type and the # is an integer.  Can easily be extended to general names and
	arbitrary numbers of variables */
#define MAX_VARIABLES 100
static float variable_float[MAX_VARIABLES];
static int exclusive_option=0,multiple_options=0,usage_indentation_level=0,
	usage_newline;

DECLARE_LIST_TYPES(Assign_variable);
FULL_DECLARE_INDEXED_LIST_TYPE(Assign_variable);

PROTOTYPE_OBJECT_FUNCTIONS(Assign_variable);
PROTOTYPE_LIST_FUNCTIONS(Assign_variable);
PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(Assign_variable,name,const char *);

static struct LIST(Assign_variable) *assign_variable_list = NULL;

/*
Module functions
----------------
*/
static struct Assign_variable *CREATE(Assign_variable)(const char *name)
/*******************************************************************************
LAST MODIFIED : 10 March 2000

DESCRIPTION :
==============================================================================*/
{
	struct Assign_variable *variable;

	if (name)
	{
		if (ALLOCATE(variable, struct Assign_variable, 1)
			&& ALLOCATE(variable->name, char, strlen(name)+1))
		{
			strcpy(variable->name, name);
			variable->value = (char *)NULL;
			variable->access_count = 0;
			/* Add into the global list */
			if (!assign_variable_list)
			{
				assign_variable_list = CREATE_LIST(Assign_variable)();
			}

			if (!(ADD_OBJECT_TO_LIST(Assign_variable)(variable, assign_variable_list)))
			{
				display_message(ERROR_MESSAGE,
					"CREATE(Assign_variable).  Unable to add variable to global list");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"CREATE(Assign_variable).  Unable to allocate memory for assign_variable structure");
			DEALLOCATE(variable);
			variable = (struct Assign_variable *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(Assign_variable).  Invalid arguments");
		variable = (struct Assign_variable *)NULL;
	}


	return(variable);
}

int DESTROY(Assign_variable)(struct Assign_variable **variable_address)
/*******************************************************************************
LAST MODIFIED : 10 March 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code = 0;
	struct Assign_variable *variable;

	if (variable_address && (variable = *variable_address))
	{
		if (variable->access_count <= 1)
		{
			if (variable->access_count == 1)
			{
				if (assign_variable_list)
				{
					/* Check that it is only the global list and then remove */
					if (IS_OBJECT_IN_LIST(Assign_variable)(variable,
						assign_variable_list))
					{
						REMOVE_OBJECT_FROM_LIST(Assign_variable)(variable,
							assign_variable_list);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"DESTROY(Assign_variable).  Destroy called when access count == 1 and the variable isn't in the global list.");
						*variable_address = (struct Assign_variable *)NULL;
						return_code = 0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"DESTROY(Assign_variable).  Destroy called when access count == 1 and there isn't a global list.");
					*variable_address = (struct Assign_variable *)NULL;
					return_code = 0;
				}
			}
			else
			{
				return_code = 1;
			}
			if (return_code)
			{
				if (variable->name)
				{
					DEALLOCATE(variable->name);
				}
				if (variable->value)
				{
					DEALLOCATE(variable->value);
				}
				DEALLOCATE(variable);
				*variable_address = (struct Assign_variable *)NULL;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"DESTROY(Assign_variable).  Destroy called when access count > 1.");
			*variable_address = (struct Assign_variable *)NULL;
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Assign_variable).  Invalid arguments.");
		return_code = 0;
	}

	return (return_code);
}

static int Assign_variable_set_value(struct Assign_variable *variable, const char *value)
/*******************************************************************************
LAST MODIFIED : 10 March 2000

DESCRIPTION :
==============================================================================*/
{
	char *new_value;
	int return_code;

	if (variable && value)
	{
		if (REALLOCATE(new_value, variable->value, char, strlen(value) + 1))
		{
			variable->value = new_value;
			strcpy(variable->value, value);
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Assign_variable_set_value.  Unable to allocate memory for assign_variable value");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Assign_variable_set_value.  Invalid arguments");
		return_code = 0;
	}

	return(return_code);
} /* Assign_variable_set_value */

DECLARE_OBJECT_FUNCTIONS(Assign_variable)
DECLARE_INDEXED_LIST_MODULE_FUNCTIONS(Assign_variable,name,const char *,strcmp)
DECLARE_INDEXED_LIST_FUNCTIONS(Assign_variable)
DECLARE_FIND_BY_IDENTIFIER_IN_INDEXED_LIST_FUNCTION(Assign_variable,name,const char *,
	strcmp)

static int execute_variable_command_operation(struct Parse_state *state,
	void *operation_type_void,void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 21 June 1999

DESCRIPTION :
Executes a VARIABLE operation command.
==============================================================================*/
{
	const char *current_token;
	enum Variable_operation_type *operation_type;
	float value;
	int number,return_code;

	ENTER(execute_variable_command_operation);
	USE_PARAMETER(dummy_user_data);
	if (state)
	{
		current_token=state->current_token;
		if (current_token != NULL)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				operation_type=(enum Variable_operation_type *)operation_type_void;
				if (operation_type != NULL)
				{
					/* get number */
					if (1==sscanf(current_token," %i",&number))
					{
						shift_Parse_state(state,1);
						current_token=state->current_token;
						if (current_token != NULL)
						{
							/* get value */
							if (1==sscanf(current_token," %f",&value))
							{
								return_code=0;
								switch (*operation_type)
								{
									case ADD_VARIABLE_OPERATION:
									{
										variable_float[number] += value;
									} break;
									case DIVIDE_VARIABLE_OPERATION:
									{
										variable_float[number] /= value;
									} break;
									case MULTIPLY_VARIABLE_OPERATION:
									{
										variable_float[number] *= value;
									} break;
									case SET_VARIABLE_OPERATION:
									{
										variable_float[number]=value;
									} break;
									case SUBTRACT_VARIABLE_OPERATION:
									{
										variable_float[number] -= value;
									} break;
									default:
									{
										display_message(ERROR_MESSAGE,
						"execute_variable_command_operation.  Unknown variable operation");
										return_code=0;
									} break;
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,"Invalid variable value: %s",
									current_token);
								display_parse_state_location(state);
								return_code=0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,"Missing variable value");
							display_parse_state_location(state);
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"Invalid variable number: %s",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"execute_variable_command_show.  Missing operation_type");
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," VARIABLE_NUMBER SET_VALUE");
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing variable number");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_variable_command_operation.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_variable_command_operation */

static int execute_variable_command_show(struct Parse_state *state,
	void *dummy_to_be_modified,void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 21 June 1999

DESCRIPTION :
Executes a VARIABLE SHOW command.
==============================================================================*/
{
	const char *current_token;
	int number,return_code;

	ENTER(execute_variable_command_show);
	USE_PARAMETER(dummy_to_be_modified);
	USE_PARAMETER(dummy_user_data);
	if (state)
	{
		current_token=state->current_token;
		if (current_token != NULL)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				/* get number */
				if (1==sscanf(current_token," %i",&number))
				{
					display_message(INFORMATION_MESSAGE,"Variable %i = %g\n",number,
						variable_float[number]);
					return_code=1;
				}
				else
				{
					display_message(ERROR_MESSAGE,"Invalid variable number: %s",
						current_token);
					display_parse_state_location(state);
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," VARIABLE_NUMBER");
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing variable number");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_variable_command_show.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_variable_command_show */

/*
Global functions
----------------
*/
int process_option(struct Parse_state *state,
	struct Modifier_entry *modifier_table)
/*******************************************************************************
LAST MODIFIED : 10 September 2002

DESCRIPTION :
If the <state->current_token> is "?", then the options in the <modifier_table>
and the values expected for each will be written to the command window and 1
returned.  Otherwise, the <modifier_table> is searched for entries whose option
field matchs <state->current_token>.  If no matchs are found, then if the
terminating entry in the <modifier_table> has a modifier function it is called,
otherwise an error message is written and 0 returned.  If one match is found,
then the modifier function of the entry is called and its return value returned.
If more than one match is found then the possible matchs are written to the
command window and 0 is returned.  Note that <process_option> is a modifier
function.
Now allows a single option to be matched exactly even if longer tokens start
with the same text.
==============================================================================*/
{
	const char *current_token;
	char *error_message, **token;
	int append_error, exact_match_count, first, i, number_of_sub_entries,
		partial_match_count, return_code;
	struct Modifier_entry *entry, *matching_entry, *sub_entry;

	ENTER(process_option);
	exclusive_option++;
	if (state && (entry = modifier_table))
	{
		current_token = state->current_token;
		if (current_token != NULL)
		{
			if (strcmp(PARSER_HELP_STRING, current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING, current_token))
			{
				matching_entry = (struct Modifier_entry *)NULL;
				error_message = (char *)NULL;
				append_error = 0;
				/* count the number of entries that are exact and partial matches
				   matching_entry stores first exact or partial match */
				exact_match_count = 0;
				partial_match_count = 0;
				while ((entry->option) || ((entry->user_data) && !(entry->modifier)))
				{
					if (entry->option)
					{
						if (fuzzy_string_compare(current_token, entry->option))
						{
							if (fuzzy_string_compare_same_length(current_token,
								entry->option))
							{
								exact_match_count++;
								if (1 == exact_match_count)
								{
									matching_entry = entry;
								}
							}
							else
							{
								partial_match_count++;
								if (!matching_entry)
								{
									matching_entry = entry;
								}
							}
						}
					}
					else
					{
						/* assume that the user_data is another option table */
						sub_entry = (struct Modifier_entry *)(entry->user_data);
						while (sub_entry->option)
						{
							if (fuzzy_string_compare(current_token, sub_entry->option))
							{
								if (fuzzy_string_compare_same_length(current_token,
									sub_entry->option))
								{
									exact_match_count++;
									if (1 == exact_match_count)
									{
										matching_entry = sub_entry;
									}
								}
								else
								{
									partial_match_count++;
									if (!matching_entry)
									{
										matching_entry = sub_entry;
									}
								}
							}
							sub_entry++;
						}
					}
					entry++;
				}
				if (matching_entry)
				{
					if ((1 == exact_match_count) ||
						((0 == exact_match_count) && (1 == partial_match_count)))
					{
						if (shift_Parse_state(state, 1))
						{
							return_code = (matching_entry->modifier)(state,
								matching_entry->to_be_modified, matching_entry->user_data);
						}
						else
						{
							display_message(ERROR_MESSAGE,"process_option.  Error parsing");
							return_code = 0;
						}
					}
					else
					{
						/* report on ambiguous entries */
						if (1 < exact_match_count)
						{
							append_string(&error_message, "Repeated option table entry <",
								&append_error);
							append_string(&error_message, matching_entry->option,
								&append_error);
							append_string(&error_message, ">", &append_error);
							return_code = 0;
						}
						else
						{
							append_string(&error_message, "Ambiguous option <",
								&append_error);
							append_string(&error_message, current_token,
								&append_error);
							append_string(&error_message, "> could be <",
								&append_error);
							append_string(&error_message, matching_entry->option,
								&append_error);
							entry = modifier_table;
							while ((entry->option) || ((entry->user_data) &&
								!(entry->modifier)))
							{
								if (entry->option)
								{
									if (fuzzy_string_compare(current_token, entry->option))
									{
										if (entry != matching_entry)
										{
											append_string(&error_message, "> or <", &append_error);
											append_string(&error_message, entry->option,
												&append_error);
										}
									}
								}
								else
								{
									/* assume that the user_data is another option table */
									sub_entry = (struct Modifier_entry *)(entry->user_data);
									while (sub_entry->option)
									{
										if (fuzzy_string_compare(current_token, sub_entry->option))
										{
											if (sub_entry != matching_entry)
											{
												append_string(&error_message, "> or <", &append_error);
												append_string(&error_message, sub_entry->option,
													&append_error);
											}
										}
										sub_entry++;
									}
								}
								entry++;
							}
							append_string(&error_message, ">", &append_error);
						}
						return_code = 0;
					}
				}
				else
				{
					/* use the default modifier function if it exists */
					if (entry->modifier)
					{
						return_code = (entry->modifier)(state,entry->to_be_modified,
							entry->user_data);
					}
					else
					{
						append_string(&error_message, "Unknown option <", &append_error);
						append_string(&error_message, current_token, &append_error);
						append_string(&error_message, ">", &append_error);
						return_code = 0;
					}
				}
				if (error_message)
				{
					display_message(ERROR_MESSAGE, error_message);
					DEALLOCATE(error_message);
					display_parse_state_location(state);
				}
			}
			else
			{
				/* write help */
				if (0==usage_indentation_level)
				{
					display_message(INFORMATION_MESSAGE,"Usage :");
					token=state->tokens;
					for (i=state->current_index;i>0;i--)
					{
						display_message(INFORMATION_MESSAGE," %s",*token);
						token++;
					}
					display_message(INFORMATION_MESSAGE," %s",*token);
				}
				if (!((multiple_options>0)&&(exclusive_option>1)))
				{
					display_message(INFORMATION_MESSAGE,"\n");
				}
				usage_indentation_level += 2;
				if (strcmp(PARSER_HELP_STRING,current_token)||(multiple_options>0))
				{
					/* recursive help */
					first=1;
					while ((entry->option)||((entry->user_data)&&!(entry->modifier)))
					{
						if (entry->option)
						{
							if (entry->modifier)
							{
								if (multiple_options>0)
								{
									if (exclusive_option>1)
									{
										if (first)
										{
											display_message(INFORMATION_MESSAGE,"\n%*s(%s",
												usage_indentation_level," ",entry->option);
										}
										else
										{
											display_message(INFORMATION_MESSAGE,"|%s",entry->option);
										}
									}
									else
									{
										display_message(INFORMATION_MESSAGE,"%*s<%s",
											usage_indentation_level," ",entry->option);
									}
								}
								else
								{
									display_message(INFORMATION_MESSAGE,"%*s%s",
										usage_indentation_level," ",entry->option);
								}
								usage_newline=1;
								(entry->modifier)(state,entry->to_be_modified,entry->user_data);
								if (multiple_options>0)
								{
									if (exclusive_option<=1)
									{
										display_message(INFORMATION_MESSAGE,">");
										if (usage_newline)
										{
											display_message(INFORMATION_MESSAGE,"\n");
											usage_newline=0;
										}
									}
								}
								else
								{
									if (usage_newline)
									{
										display_message(INFORMATION_MESSAGE,"\n");
										usage_newline=0;
									}
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"process_option.  Missing modifier: %s",entry->option);
								display_parse_state_location(state);
							}
						}
						else
						{
							/* assume that the user_data is another option table */
							sub_entry=(struct Modifier_entry *)(entry->user_data);
							display_message(INFORMATION_MESSAGE,"%*s",
								usage_indentation_level," ");
							if (0<multiple_options)
							{
								display_message(INFORMATION_MESSAGE,"<");
							}
							number_of_sub_entries=0;
							while (sub_entry->option)
							{
								number_of_sub_entries++;
								if (sub_entry->modifier)
								{
									if (1<number_of_sub_entries)
									{
										display_message(INFORMATION_MESSAGE,"|");
									}
									display_message(INFORMATION_MESSAGE,"%s",sub_entry->option);
									(sub_entry->modifier)(state,sub_entry->to_be_modified,
										sub_entry->user_data);
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"process_option.  Missing modifier: %s",sub_entry->option);
									display_parse_state_location(state);
								}
								sub_entry++;
							}
							if (0<multiple_options)
							{
								display_message(INFORMATION_MESSAGE,">");
							}
							display_message(INFORMATION_MESSAGE,"\n");
							usage_newline=0;
						}
						first=0;
						entry++;
					}
					/* write help for default modifier it it exists */
					if (entry->modifier)
					{
						if (multiple_options>0)
						{
							if (exclusive_option>1)
							{
								if (first)
								{
									display_message(INFORMATION_MESSAGE,"\n%*s(",
										usage_indentation_level," ");
								}
								else
								{
									display_message(INFORMATION_MESSAGE,"|");
								}
							}
							else
							{
								display_message(INFORMATION_MESSAGE,"%*s<",
									usage_indentation_level," ");
							}
						}
						else
						{
							display_message(INFORMATION_MESSAGE,"%*s",usage_indentation_level,
								" ");
						}
						usage_newline=1;
						(entry->modifier)(state,entry->to_be_modified,entry->user_data);
						if (multiple_options>0)
						{
							if (exclusive_option<=1)
							{
								display_message(INFORMATION_MESSAGE,">");
								if (usage_newline)
								{
									display_message(INFORMATION_MESSAGE,"\n");
									usage_newline=0;
								}
							}
							else
							{
								display_message(INFORMATION_MESSAGE,")");
							}
						}
						else
						{
							if (usage_newline)
							{
								display_message(INFORMATION_MESSAGE,"\n");
								usage_newline=0;
							}
						}
					}
				}
				else
				{
					/* one level of help */
						/*???DB.  Have added  multiple_options>0  to then so won't come
							here, but haven't stripped out yet */
					while ((entry->option)||((entry->user_data)&&!(entry->modifier)))
					{
						if (entry->option)
						{
							if (multiple_options>0)
							{
								display_message(INFORMATION_MESSAGE,"%*s<%s>\n",
									usage_indentation_level," ",entry->option);
							}
							else
							{
								display_message(INFORMATION_MESSAGE,"%*s%s\n",
									usage_indentation_level," ",entry->option);
							}
						}
						else
						{
							/* assume that the user_data is another option table */
							sub_entry=(struct Modifier_entry *)(entry->user_data);
							if (sub_entry->option)
							{
								if (multiple_options>0)
								{
									display_message(INFORMATION_MESSAGE,"%*s<%s",
										usage_indentation_level," ",sub_entry->option);
								}
								else
								{
									display_message(INFORMATION_MESSAGE,"%*s(%s",
										usage_indentation_level," ",sub_entry->option);
								}
								sub_entry++;
								while (sub_entry->option)
								{
									display_message(INFORMATION_MESSAGE,"|%s",sub_entry->option);
									sub_entry++;
								}
								if (multiple_options>0)
								{
									display_message(INFORMATION_MESSAGE,">");
								}
								else
								{
									display_message(INFORMATION_MESSAGE,")");
								}
								display_message(INFORMATION_MESSAGE,"\n");
								usage_newline=0;
							}
						}
						entry++;
					}
					/* write help for default modifier if it exists */
					if (entry->modifier)
					{
						if (multiple_options>0)
						{
							display_message(INFORMATION_MESSAGE,"%*s<",
								usage_indentation_level," ");
						}
						else
						{
							display_message(INFORMATION_MESSAGE,"%*s",usage_indentation_level,
								" ");
						}
						(entry->modifier)(state,entry->to_be_modified,entry->user_data);
						if (multiple_options>0)
						{
							display_message(INFORMATION_MESSAGE,">");
						}
						display_message(INFORMATION_MESSAGE,"\n");
					}
				}
				usage_indentation_level -= 2;
				/* so that process_option is only called once in parsing loop */
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing token");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"process_option.  Invalid argument(s)");
		return_code=0;
	}
	exclusive_option--;
	LEAVE;

	return (return_code);
} /* process_option */

int process_multiple_options(struct Parse_state *state,
	struct Modifier_entry *modifier_table)
/*******************************************************************************
LAST MODIFIED : 4 October 1996

DESCRIPTION :
==============================================================================*/
{
	int local_exclusive_option,return_code;

	ENTER(process_multiple_options);
	if (state&&modifier_table)
	{
		multiple_options++;
		local_exclusive_option=exclusive_option;
		exclusive_option=0;
		return_code=1;
		while ((state->current_token)&&(return_code=process_option(state,
			modifier_table)));
		multiple_options--;
		exclusive_option=local_exclusive_option;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"process_multiple_options.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* process_multiple_options */

struct Option_table *CREATE(Option_table)(void)
/*******************************************************************************
LAST MODIFIED : 23 December 1999

DESCRIPTION :
Creates an Option_table for text parsing.
==============================================================================*/
{
	struct Option_table *option_table;

	ENTER(CREATE(Option_table));
	if (ALLOCATE(option_table,struct Option_table,1))
	{
		option_table->allocated_entries = 0;
		option_table->number_of_entries = 0;
		option_table->entry = (struct Modifier_entry *)NULL;
		option_table->help = (char *)NULL;
		/* flag indicating all options successfully added */
		option_table->valid = 1;
		/* store suboption_tables added to table for destroying with option_table */
		option_table->number_of_suboption_tables = 0;
		option_table->suboption_tables = (struct Option_table **)NULL;
	}
	else
	{
		display_message(ERROR_MESSAGE,"CREATE(Option_table).  Not enough memory");
		if (option_table)
		{
			DEALLOCATE(option_table);
		}
	}
	LEAVE;

	return (option_table);
} /* CREATE(Option_table) */

int DESTROY(Option_table)(struct Option_table **option_table_address)
/*******************************************************************************
LAST MODIFIED : 23 December 1999

DESCRIPTION :
==============================================================================*/
{
	int i,return_code;
	struct Option_table *option_table;

	ENTER(DESTROY(Option_table));
	if (option_table_address)
	{
		return_code=1;
		option_table = *option_table_address;
		if (option_table != NULL)
		{
			/* clean up suboption_tables added to option_table */
			if (option_table->suboption_tables)
			{
				for (i=0;i<option_table->number_of_suboption_tables;i++)
				{
					DESTROY(Option_table)(&(option_table->suboption_tables[i]));
				}
				DEALLOCATE(option_table->suboption_tables);
			}
			if (option_table->help)
			{
				DEALLOCATE(option_table->help);
			}
			if (option_table->entry)
			{
				DEALLOCATE(option_table->entry);
			}
			DEALLOCATE(*option_table_address);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"DESTROY(Option_table).  Invalid argument");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Option_table) */

static int Option_table_add_entry_private(struct Option_table *option_table,
	const char *token,void *to_be_modified,void *user_data,modifier_function modifier)
/*******************************************************************************
LAST MODIFIED : 23 December 1999

DESCRIPTION :
Adds the given entry to the option table, enlarging the table as needed.
If fails, marks the option_table as invalid.
==============================================================================*/
{
	int i, return_code;
	struct Modifier_entry *temp_entry;

	ENTER(Option_table_add_entry_private);
	if (option_table)
	{
		return_code=1;
		if (token)
		{
			i=0;
			while (return_code && (i<option_table->number_of_entries))
			{
				if (option_table->entry[i].option
					&& (!strcmp(token, option_table->entry[i].option)))
				{
					display_message(ERROR_MESSAGE,
						"Option_table_add_entry_private.  Token '%s' already in option table",
						token);
					return_code=0;
				}
				i++;
			}
		}
		if (option_table->number_of_entries == option_table->allocated_entries)
		{
			if (REALLOCATE(temp_entry,option_table->entry,struct Modifier_entry,
				option_table->allocated_entries+OPTION_TABLE_ALLOCATE_SIZE))
			{
				option_table->entry = temp_entry;
				option_table->allocated_entries += OPTION_TABLE_ALLOCATE_SIZE;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Option_table_add_entry_private.  Not enough memory");
				return_code=0;
				option_table->valid=0;
			}
		}
		if (return_code)
		{
			temp_entry = &(option_table->entry[option_table->number_of_entries]);
			temp_entry->option=(char *)token;
			temp_entry->to_be_modified=to_be_modified;
			temp_entry->user_data=user_data;
			temp_entry->modifier=modifier;
			option_table->number_of_entries++;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Option_table_add_entry_private.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Option_table_add_entry_private */

int Option_table_add_help(struct Option_table *option_table,
	const char *help_string)
/*******************************************************************************
LAST MODIFIED : 2 May 2007

DESCRIPTION :
Adds the given help to the option table.
==============================================================================*/
{
	int error, return_code;

	ENTER(Option_table_add_help);
	if (option_table)
	{
		error = 0;
		append_string(&option_table->help,
			help_string, &error);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Option_table_add_help.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Option_table_add_help */

int Option_table_add_entry(struct Option_table *option_table,const char *token,
	void *to_be_modified,void *user_data,modifier_function modifier)
/*******************************************************************************
LAST MODIFIED : 23 December 1999

DESCRIPTION :
Adds the given <token> etc. to the option table, enlarging the table as needed.
If fails, marks the option_table as invalid.
==============================================================================*/
{
	int return_code;

	ENTER(Option_table_add_entry);
	if (option_table)
	{
		if (!(return_code=Option_table_add_entry_private(option_table,token,
			to_be_modified,user_data,modifier)))
		{
			display_message(ERROR_MESSAGE,
				"Option_table_add_entry.  Could not add option");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Option_table_add_entry.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Option_table_add_entry */

int Option_table_add_suboption_table(struct Option_table *option_table,
	struct Option_table *suboption_table)
/*******************************************************************************
LAST MODIFIED : 8 November 2000

DESCRIPTION :
Checks that <suboption_table> is valid, and if so, adds it to <option_table>.
On calling this function, <suboption_table> is owned by <option_table> and the
latter is responsible for destroying it. It will be destroyed immediately if it
is invalid or cannot be added to list of entries.
Mechanism currently used to handle enumerated options, though it does not insist
that only one valid enumerator is entered.
If fails, marks the option_table as invalid.
Note must not make any further changes to suboption_table after it is made part
of option_table!
==============================================================================*/
{
	int return_code;
	struct Option_table **temp_suboption_tables;

	ENTER(Option_table_add_suboption_table);
	if (option_table&&suboption_table)
	{
		/* add blank entry needed for process_option */
		Option_table_add_entry_private(suboption_table,(char *)NULL,(void *)NULL,
			(void *)NULL,(modifier_function)NULL);
		if (suboption_table->valid)
		{
			if (REALLOCATE(temp_suboption_tables,option_table->suboption_tables,
				struct Option_table *,option_table->number_of_suboption_tables+1))
			{
				option_table->suboption_tables=temp_suboption_tables;
				option_table->suboption_tables[option_table->number_of_suboption_tables]
					=suboption_table;
				option_table->number_of_suboption_tables++;
				if (!(return_code=Option_table_add_entry_private(option_table,
					(char *)NULL,(void *)NULL,suboption_table->entry,
					(modifier_function)NULL)))
				{
					display_message(ERROR_MESSAGE,
						"Option_table_add_entry.  Could not add option");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Option_table_add_suboption_table.  Not enough memory");
				return_code=0;
				option_table->valid=0;
				DESTROY(Option_table)(&suboption_table);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Option_table_parse.  Invalid suboption_table");
			option_table->valid=0;
			DESTROY(Option_table)(&suboption_table);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Option_table_add_suboption_table.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Option_table_add_suboption_table */

int Option_table_add_switch(struct Option_table *option_table,
	const char *on_string,const char *off_string,int *value_address)
/*******************************************************************************
LAST MODIFIED : 12 May 2000

DESCRIPTION :
Adds a newly created suboption table containing 2 items:
an <on_string> token that invokes set_int_switch with <value_address>;
an <off_string> token that invokes unset_int_switch with <value_address>;
The <on_string> and <off_string> should be static, eg. passed in quotes.
==============================================================================*/
{
	int return_code;
	struct Option_table *suboption_table;

	ENTER(Option_table_add_switch);
	if (option_table&&on_string&&off_string&&value_address)
	{
		if (option_table->valid)
		{
			suboption_table=CREATE(Option_table)();
			if (suboption_table != NULL)
			{
				Option_table_add_entry(suboption_table,on_string,
					value_address,(void *)on_string,set_int_switch);
				Option_table_add_entry(suboption_table,off_string,
					value_address,(void *)off_string,unset_int_switch);
				return_code=
					Option_table_add_suboption_table(option_table,suboption_table);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Option_table_add_switch.  Not enough memory");
				return_code=0;
				option_table->valid=0;
			}
		}
		else
		{
			/* report no further errors */
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Option_table_add_switch.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Option_table_add_switch */

int Option_table_is_valid(struct Option_table *option_table)
/*******************************************************************************
LAST MODIFIED : 4 November 2005

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Option_table_is_valid);
	if (option_table)
	{
		return_code = option_table->valid;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Option_table_is_valid.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Option_table_is_valid */

int Option_table_set_invalid(struct Option_table *option_table)
/*******************************************************************************
LAST MODIFIED : 4 November 2005

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Option_table_set_invalid);
	if (option_table)
	{
		option_table->valid = 0;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Option_table_set_invalid.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Option_table_set_invalid */

int set_enumerator_string(struct Parse_state *state,
	void *enumerator_string_address_void,void *enumerator_string_value_void)
/*******************************************************************************
LAST MODIFIED : 21 December 1999

DESCRIPTION :
A modifier function for setting an enumerated type variable to a specified
value.
==============================================================================*/
{
	const char *current_token;
	char **enumerator_string_address,*enumerator_string_value;
	int return_code;

	ENTER(set_enumerator_string);
	if (state&&(enumerator_string_address=(char **)enumerator_string_address_void)
		&&(enumerator_string_value=(char *)enumerator_string_value_void))
	{
		return_code=1;
		if (!(NULL != (current_token=state->current_token))||
			(strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token)))
		{
			*enumerator_string_address = enumerator_string_value;
		}
		else
		{
			if (*enumerator_string_address == enumerator_string_value)
			{
				display_message(INFORMATION_MESSAGE,"[%s]",enumerator_string_value);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_enumerator_string.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_enumerator_string */

int Option_table_add_enumerator(struct Option_table *option_table,
	int number_of_valid_strings,const char **valid_strings,
	const char **enumerator_string_address)
/*******************************************************************************
LAST MODIFIED : 20 December 1999

DESCRIPTION :
Adds a newly created suboption table for all the valid_strings for the
enumerator. The <valid_strings> array should contain <number_of_valid_strings>
pointers to static strings, one per enumerator option. Responsibility for
deallocating this array is left to the calling function. The static string value
of the enumerator is maintained in <enumerator_string_address> and it is up to
the calling function to convert back to an enumerated value.
Note that if any error occurs, the option_table is marked as being invalid and
no further errors will be reported on subsequent calls.
==============================================================================*/
{
	int i,return_code;
	struct Option_table *suboption_table;

	ENTER(Option_table_add_enumerator);
	if (option_table&&(0<number_of_valid_strings)&&valid_strings&&
		enumerator_string_address)
	{
		if (option_table->valid)
		{
			suboption_table=CREATE(Option_table)();
			if (suboption_table != NULL)
			{
				for (i=0;i<number_of_valid_strings;i++)
				{
					Option_table_add_entry(suboption_table,valid_strings[i],
						enumerator_string_address,(void *)valid_strings[i],set_enumerator_string);
				}
				return_code=
					Option_table_add_suboption_table(option_table,suboption_table);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Option_table_add_enumerator.  Not enough memory");
				return_code=0;
				option_table->valid=0;
			}
		}
		else
		{
			/* report no further errors */
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Option_table_add_enumerator.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Option_table_add_enumerator */

int Option_table_parse(struct Option_table *option_table,
	struct Parse_state *state)
/*******************************************************************************
LAST MODIFIED : 23 December 1999

DESCRIPTION :
Parses the options in the <option_table>, giving only one option a chance to be
entered.
==============================================================================*/
{
	int return_code;

	ENTER(Option_table_parse);
	if (option_table&&state)
	{
		/* add blank entry needed for process_option */
		Option_table_add_entry_private(option_table,(char *)NULL,(void *)NULL,
			(void *)NULL,(modifier_function)NULL);
		if (option_table->valid)
		{
			return_code=process_option(state,option_table->entry);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Option_table_parse.  Invalid option table");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Option_table_parse.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Option_table_parse */

int Option_table_multi_parse(struct Option_table *option_table,
	struct Parse_state *state)
/*******************************************************************************
LAST MODIFIED : 2 May 2007

DESCRIPTION :
Parses the options in the <option_table>, giving all options a chance to be
entered.
==============================================================================*/
{
	int return_code;

	ENTER(Option_table_multi_parse);
	if (option_table&&state)
	{
		/* Write out the help */
		if (option_table->help && state && state->current_token &&
			(!strcmp(PARSER_HELP_STRING,state->current_token)||
			!strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
		{
			char *space_offset;
			int index = 0, output, length = strlen(option_table->help), local_indent,
				in_text_indent;
			local_indent = usage_indentation_level + 4;
			in_text_indent = 0;
			while (index < length)
			{
				if (index + 60 > length)
				{
					output = length - index;
				}
				else
				{
					space_offset = strchr(
							 option_table->help + index + 50, ' ');
					if (space_offset != NULL)
					{
						space_offset++;
						while (*space_offset == ' ')
						{
							space_offset++;
						}
						output = space_offset - option_table->help - index;
					}
					else
					{
						output = length - index;
					}
				}
				display_message(INFORMATION_MESSAGE,"\n%*s*%*s%.*s",
					local_indent, " ", in_text_indent, " ",
					output, option_table->help + index);
				index += output;
				in_text_indent = 2;
			}
		}
		/* add blank entry needed for process_option */
		Option_table_add_entry_private(option_table,(char *)NULL,(void *)NULL,
			(void *)NULL,(modifier_function)NULL);
		if (option_table->valid)
		{
			return_code=process_multiple_options(state,option_table->entry);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Option_table_multi_parse.  Invalid option table");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Option_table_multi_parse.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Option_table_multi_parse */

static int extract_token(char **source_address,char **token_address)
/*******************************************************************************
LAST MODIFIED : 11 September 2002

DESCRIPTION :
On successful return, <*token_address> will point to a newly-allocated string
containing the first token in the string at <*source_address>. <source_address>
is then updated to point to the next character after the last one used in
creating the token.
The function skips any leading whitespace and stops at the first token delimiter
(whitespace/=/,/;), comment character (!/#) or end of string. Tokens containing
any of the above special characters may be produced by enclosing them in single
or double quotes - but not a mixture of them.
To allow quote marks to be put in the final string, the function interprets
\\, \" and \' as \, " and ', respectively.
Note that the quote mark if used must mark exactly the beginning and end of the
string; the string is not permitted to end with a NULL character or with a
non-delimiting character after the end-quote.
To minimise memory allocation, the function uses the string at <*source_address>
as working space in which the token is constructed from the source string.
==============================================================================*/
{
	char character,quote_mark,*token,*destination,*source;
	int return_code,token_length;

	ENTER(extract_token);
	if (source_address && *source_address && token_address)
	{
		return_code=1;
		destination = source = *source_address;
		/* pass over leading white space and other delimiters */
		while ((*source) && (isspace(*source) ||
			('=' == *source) || (',' == *source) || (';' == *source)))
		{
			source++;
		}
		if (*source)
		{
			if (('\''== *source) || ('\"'== *source))
			{
				quote_mark= *source;
				/* read token until final quote_mark or end of string, ignoring quote
					 marks after a backslash/escape character */
				source++;
				while ((quote_mark != *source) && ('\0' != *source))
				{
					/* replace \\, \" and \' by \, " and ' in token */
					if (('\\' == *source) && (('\\' == *(source+1)) ||
						('\"' == *(source+1)) || ('\'' == *(source+1))))
					{
						source++;
					}
					*destination = *source;
					destination++;
					source++;
				}
				if (quote_mark == *source)
				{
					source++;
					/* ensure there is a valid delimiter after end quote */
					if (('\0' != *source) && !isspace(*source) &&
						('=' != *source) && (',' != *source) &&
						(';' != *source) && ('#' != *source))
					{
						/* string missing delimiter after quote; report error */
						display_message(ERROR_MESSAGE,
							"Token missing delimiter after final quote (%c)",quote_mark);
						return_code=0;
					}
				}
				else
				{
					/* string ended without final quote; report error */
					display_message(ERROR_MESSAGE,
						"Token missing final quote (%c)",quote_mark);
					return_code=0;
				}
			}
			else
			{
				while (('\0' != (character = *source)) && (!isspace(character)) &&
					('=' != character) && (',' != character) &&
					(';' != character) && ('#' != character))
				{
					*destination = *source;
					destination++;
					source++;
				}
			}
		}
		if (return_code)
		{
			if (0<(token_length= destination - *source_address))
			{
				if (ALLOCATE(token,char,token_length+1))
				{
					strncpy(token,*source_address,token_length);
					token[token_length]='\0';
					*token_address = token;
					*source_address = source;
				}
				else
				{
					display_message(ERROR_MESSAGE,"extract_token.  Not enough memory");
					return_code=0;
				}
			}
			else
			{
				/* source string empty or only contained whitespace/delimiters */
				*token_address = (char *)NULL;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"extract_token.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* extract_token */

struct Parse_state *create_Parse_state(const char *command_string)
/*******************************************************************************
LAST MODIFIED : 24 November 1998

DESCRIPTION :
Creates a Parse_state structure which contains
- a trimmed copy of the <command_string>
- the <command_string> split into tokens
NB
1 ! and # indicate that the rest of the command string is a comment (not split
	into tokens;
2 tokens containing normal token separator commands may be entered by surround-
  ing them with single '' or double "" quotes - useful for entering text. Paired
	quotes in such strings are read as a quote mark in the final token;
3 Variables are converted into values;
==============================================================================*/
{
	char *next_token,*token_source,*working_string,**tokens,**temp_tokens;
	int allocated_tokens,i,number_of_tokens,return_code,still_tokenising;
	struct Parse_state *state;

	ENTER(create_Parse_state);
	if (command_string)
	{
		if (ALLOCATE(state,struct Parse_state,1))
		{
			/*???RC trim_string not used as trailing whitespace may be in a quote */
			if (ALLOCATE(working_string,char,strlen(command_string)+1))
			{
				/* Replace the %z1% variables and $variables in the working_string */
				strcpy(working_string,command_string);
#if ! defined (USE_PERL_INTERPRETER)
				parse_variable(&working_string);
#endif /* ! defined (USE_PERL_INTERPRETER) */
				if (ALLOCATE(state->command_string,char,strlen(working_string)+1))
				{
					strcpy(state->command_string,working_string);
					token_source=working_string;
					tokens=(char **)NULL;
					allocated_tokens=0;
					number_of_tokens=0;
					return_code=1;
					still_tokenising=1;
					while (still_tokenising)
					{
						if (extract_token(&token_source,&next_token))
						{
							if (next_token)
							{
								if (number_of_tokens == allocated_tokens)
								{
									if (REALLOCATE(temp_tokens,tokens,char *,allocated_tokens+10))
									{
										tokens=temp_tokens;
										allocated_tokens += 10;
									}
									else
									{
										return_code=0;
									}
								}
								if (return_code)
								{
									tokens[number_of_tokens]=next_token;
									number_of_tokens++;
								}
								else
								{
									DEALLOCATE(next_token);
									still_tokenising=0;
								}
							}
							else
							{
								/* successful end of tokenising */
								still_tokenising=0;
							}
						}
						else
						{
							/* tokenising failed */
							return_code=still_tokenising=0;
						}
					}
					if (return_code)
					{
						state->tokens=tokens;
						state->number_of_tokens=number_of_tokens;
						state->current_index=0;
						if (tokens)
						{
							state->current_token=tokens[0];
						}
						else
						{
							state->current_token=(char *)NULL;
						}
					}
					else
					{
						/* clean up any memory allocated for tokens */
						if (tokens)
						{
							for (i=0;i<number_of_tokens;i++)
							{
								DEALLOCATE(tokens[i]);
							}
							DEALLOCATE(tokens);
						}
						DEALLOCATE(state->command_string);
					}
				}
				else
				{
					return_code=0;
				}
			}
			else
			{
					return_code=0;
			}
			DEALLOCATE(working_string);
			if (!return_code)
			{
				display_message(ERROR_MESSAGE,
					"create_Parse_state.  Error filling parse state");
				DEALLOCATE(state);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_Parse_state.  Insufficient memory for parse state");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_Parse_state.  Missing command string");
		state=(struct Parse_state *)NULL;
	}
	LEAVE;

	return (state);
} /* create_Parse_state */

struct Parse_state *create_Parse_state_from_tokens(
	int number_of_tokens, const char **tokens)
/*******************************************************************************
LAST MODIFIED : 31 July 2002

DESCRIPTION :
Creates a Parse_state structure which contains all <number_of_tokens> <tokens>.
Does not perform any parsing.
==============================================================================*/
{
	int i, return_code;
	struct Parse_state *state;

	ENTER(create_Parse_state_from_tokens);
	if ((0 < number_of_tokens) && tokens)
	{
		if (ALLOCATE(state, struct Parse_state, 1))
		{
			state->tokens = (char **)NULL;
			state->number_of_tokens = 0;
			state->current_index = 0;
			state->current_token = (char *)NULL;
			state->command_string = (char *)NULL;
			return_code = 1;
			if (ALLOCATE(state->tokens, char *, number_of_tokens))
			{
				for (i = 0; i < number_of_tokens; i++)
				{
					state->tokens[i] = (char *)NULL;
				}
				for (i = 0; i < number_of_tokens; i++)
				{
					if (!(tokens[i] &&
						(state->tokens[i] = duplicate_string(tokens[i]))))
					{
						return_code = 0;
					}
				}
			}
			else
			{
				return_code = 0;
			}
			if (return_code)
			{
				state->number_of_tokens = number_of_tokens;
				state->current_token = tokens[0];
			}
			else
			{
				destroy_Parse_state(&state);
				state = (struct Parse_state *)NULL;
				display_message(ERROR_MESSAGE,
					"create_Parse_state_from_tokens.  Error filling parse state");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_Parse_state_from_tokens.  Insufficient memory for parse state");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_Parse_state_from_tokens.  Missing tokens");
		state = (struct Parse_state *)NULL;
	}
	LEAVE;

	return (state);
} /* create_Parse_state_from_tokens */

int destroy_Parse_state(struct Parse_state **state_address)
/*******************************************************************************
LAST MODIFIED : 12 June 1996

DESCRIPTION :
==============================================================================*/
{
	char **token;
	int number_of_tokens,return_code;
	struct Parse_state *state;

	ENTER(destroy_Parse_state);
	if (state_address)
	{
		state = *state_address;
		if (state != NULL)
		{
			if ((number_of_tokens=state->number_of_tokens)>0)
			{
				token=state->tokens;
				while (number_of_tokens>0)
				{
					DEALLOCATE(*token);
					token++;
					number_of_tokens--;
				}
				DEALLOCATE(state->tokens);
			}
			DEALLOCATE(state->command_string);
			DEALLOCATE(*state_address);
			return_code=1;
		}
		else
		{
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"destroy_Parse_state.  Invalid argument");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* destroy_Parse_state */

int Parse_state_help_mode(struct Parse_state *state)
/*******************************************************************************
LAST MODIFIED : 12 May 2000

DESCRIPTION :
Returns 1 if the current_token in <state> is either of
PARSER_HELP_STRING or PARSER_RECURSIVE_HELP_STRING.
==============================================================================*/
{
	const char *current_token;
	int return_code;

	ENTER(Parse_state_help_mode);
	if (state)
	{
		current_token=state->current_token;
		if (current_token != NULL)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				/* non-help token */
				return_code=0;
			}
			else
			{
				/* help token */
				return_code=1;
			}
		}
		else
		{
			/* no token */
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Parse_state_help_mode.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Parse_state_help_mode */

int shift_Parse_state(struct Parse_state *state,int shift)
/*******************************************************************************
LAST MODIFIED : 25 March 2004

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(shift_Parse_state);
	if (state)
	{
		state->current_index += shift;
		if ((state->current_index < 0) || (state->current_index <= state->number_of_tokens))
		{
			if (state->current_index==state->number_of_tokens)
			{
				state->current_token=(char *)NULL;
			}
			else
			{
				state->current_token=state->tokens[state->current_index];
			}
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"shift_Parse_state.  Cannot shift beyond end of token list");
			state->current_token=(char *)NULL;
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"shift_Parse_state.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* shift_Parse_state */

int display_parse_state_location(struct Parse_state *state)
/*******************************************************************************
LAST MODIFIED : 27 June 1996

DESCRIPTION :
Shows the current location in the parse <state>.
==============================================================================*/
{
	char **token;
	int i,return_code;

	ENTER(display_parse_state_location);
	if (state)
	{
		if (state->current_token)
		{
			i=state->current_index;
		}
		else
		{
			i=state->number_of_tokens;
		}
		token=state->tokens;
		while (i>0)
		{
			display_message(INFORMATION_MESSAGE,"%s ",*token);
			token++;
			i--;
		}
		display_message(INFORMATION_MESSAGE,"*\n");
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"display_parse_state_location.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* display_parse_state_location */

int Parse_state_append_to_command_string(struct Parse_state *state,
	char *addition)
/*******************************************************************************
LAST MODIFIED : 29 October 1999

DESCRIPTION :
Appends the <addition> string to the end of the current command_string stored in
the <state>.  Useful for changing the kept history echoed to the command window.
==============================================================================*/
{
	char *new_command_string;
	int return_code;

	ENTER(Parse_state_append_to_command_string);
	if (state && addition)
	{
		if (REALLOCATE(new_command_string, state->command_string,
			char, strlen(state->command_string) + strlen(addition) + 1))
		{
			strcat(new_command_string, addition);
			state->command_string = new_command_string;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Parse_state_append_to_command_string.  "
				"Unable to reallocate command string");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Parse_state_append_to_command_string.  Invalid arguments");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Parse_state_append_to_command_string */

int parse_variable(char **token)
/*******************************************************************************
LAST MODIFIED : 10 March 2000

DESCRIPTION :
Replaces occurrences of %<f/i/z/l><nnn>% with the value of that variable.  May
be called recursively.  Reallocates <*token>, so there is no problem with
over-writing.
Also replaces $LFX with the string representing LFX in the Assign_variable list
if it exists.
==============================================================================*/
{
	char *begin,*end,*index,*new_token,temp_string[100],*var_name;
	int length,temp_int,number,return_code,temp_string_offset;
	struct Assign_variable *variable;

	ENTER(parse_variable);
	/* check argument */
	if (token&&(*token))
	{
		return_code=1;
		/* try to find a % */
		begin=strchr(*token,'%');
		if (begin != NULL)
		{
			/* look for another % */
			end=strchr(begin+1,'%');
			if (end != NULL)
			{
				length=end-begin+1;
				/* find the variable number (starting after the format) */
				if (1==sscanf(begin+2,"%i",&number))
				{
					if ((number>=0)&&(number<MAX_VARIABLES))
					{
						switch (begin[1])
						{
							case 'f':
							{
								sprintf(temp_string,"%f",variable_float[number]);
								temp_string_offset=0;
							} break;
							case 'i':
							{
								/* write it as an integer */
								temp_int=(int)variable_float[number];
								sprintf(temp_string,"%i",temp_int);
								temp_string_offset=0;
							} break;
							case 'z':
							{
								/* leading zeros and the width of the existing field */
								temp_int=(int)variable_float[number];
								sprintf(temp_string,"%0*i",length,temp_int);
								temp_string_offset=strlen(temp_string)-length;
							} break;
							case 'l':
							{
								/* write it as a logical - check for <0.1 so that close to
									zero=false */
								if (fabs(variable_float[number])<0.1)
								{
									strcpy(temp_string,"off");
								}
								else
								{
									strcpy(temp_string,"on");
								}
								temp_string_offset=0;
							} break;
							default:
							{
								display_message(ERROR_MESSAGE,
									"Variable format not known: %c : %s",begin[1],*token);
								return_code=0;
							} break;
						}
						if (return_code)
						{
							if (ALLOCATE(new_token,char,(begin-(*token))+
								strlen(temp_string+temp_string_offset)+strlen(end+1)+1))
							{
								strncpy(new_token,*token,begin-(*token));
								new_token[begin-(*token)]='\0';
								strcat(new_token,temp_string+temp_string_offset);
								strcat(new_token,end+1);
								DEALLOCATE(*token);
								*token=new_token;
								/* see if there are any more to parse */
								return_code=parse_variable(token);
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"parse_variable.  Could not allocate new token");
								return_code=0;
							}
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Variable number out of range: %d : %s",number,*token);
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,"Variable number not found: %s",*token);
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"Two percentage signs required: %s",
					*token);
				return_code=0;
			}
		}
		/* Separately parse for $assign_variables */
		/* try to find a $ */
		while (return_code && (begin=strchr(*token,'$')))
		{
			if ((begin == *token)
				|| ((begin > *token + 1) && (!strncmp(begin - 1, " ", 1)))
				|| ((begin > *token + 2) && (!strncmp(begin - 2, "//", 2))))
			{
				begin++;
				/* look for the end of this token */
				if (!(end=strpbrk(begin,"/ ")))
				{
					end = begin + strlen(begin);
				}
				if ((*end == 0) || (!(strncmp(end, " ", 1))) || (!(strncmp(end, "//", 2))))
				{
					if (ALLOCATE(var_name, char, end - begin))
					{
						strncpy(var_name, begin, end - begin);
						var_name[end - begin] = 0;
						variable = FIND_BY_IDENTIFIER_IN_LIST(Assign_variable, name)
							(var_name, assign_variable_list);
						if (variable != NULL)
						{
							if (ALLOCATE(new_token, char, strlen(variable->value) +
								strlen(*token) - (end - begin)))
							{
								index = new_token;
								if ((begin > *token + 2) && (!strncmp(begin - 2, " ", 1)))
								{
									strncpy(index, *token, begin - *token - 1);
									index += begin - *token - 1;
								}
								else if ((begin > *token + 3) && (!strncmp(begin - 3, "//", 2)))
								{
									strncpy(index, *token, begin - *token - 3);
									index += begin - *token - 3;
								}
								if (strlen(variable->value) > 0)
								{
									strncpy(index, variable->value, strlen(variable->value));
									index += strlen(variable->value);
								}
								if (!(strncmp(end, " ", 1)))
								{
									strcpy(index, end);
									index += strlen(end);
								}
								else if (!(strncmp(end, "//", 2)))
								{
									strcpy(index, end + 2);
									index += strlen(end + 2);
								}
								*index = 0;
#if defined (DEBUG_CODE)
								display_message(INFORMATION_MESSAGE,
									"parse_variable.\n\tOld token %s\n\tVariable value %s\n\tNew token %s\n", *token, variable->value, new_token);
#endif /* defined (DEBUG_CODE) */
								DEALLOCATE(*token);
								*token = new_token;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"parse_variable.  Variable \"%s\" not found.", var_name);
							return_code=0;
						}
						DEALLOCATE(var_name);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"parse_variable.  Unable to allocate variable name string");
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"parse_variable.  Concatenation token operator \"//\" required between variable and plain text.");
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"parse_variable.  Concatenation token operator \"//\" required between variable and plain text.");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"parse_variable.  Missing token");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* parse_variable */

int execute_variable_command(struct Parse_state *state,
	void *dummy_to_be_modified,void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 21 June 1999

DESCRIPTION :
Executes a VARIABLE command.
==============================================================================*/
{
	static enum Variable_operation_type add=ADD_VARIABLE_OPERATION,
		divide=DIVIDE_VARIABLE_OPERATION,multiply=MULTIPLY_VARIABLE_OPERATION,
		set=SET_VARIABLE_OPERATION,subtract=SUBTRACT_VARIABLE_OPERATION;
	int return_code;
	static struct Modifier_entry option_table[]=
	{
		{"add",&add,(void *)NULL,execute_variable_command_operation},
		{"divide",&divide,(void *)NULL,execute_variable_command_operation},
		{"multiply",&multiply,(void *)NULL,execute_variable_command_operation},
		{"set",&set,(void *)NULL,execute_variable_command_operation},
		{"show",NULL,(void *)NULL,execute_variable_command_show},
		{"subtract",&subtract,(void *)NULL,execute_variable_command_operation},
		{NULL,NULL,NULL,NULL}
	};

	ENTER(execute_variable_command);
	USE_PARAMETER(dummy_to_be_modified);
	USE_PARAMETER(dummy_user_data);
	if (state)
	{
		if (state->current_token)
		{
			return_code=process_option(state,option_table);
		}
		else
		{
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_variable_command.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_variable_command */

int execute_assign_variable(struct Parse_state *state,
	void *dummy_to_be_modified,void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 10 March 2000

DESCRIPTION :
Executes an ASSIGN VARIABLE command.  Does a very small subset of the intended
use of this command.
==============================================================================*/
{
	const char *current_token;
	const char *end;
	char *var_name, *env_string;
	const char *begin, *begin2;
	int return_code;
	struct Assign_variable *variable;

	ENTER(execute_assign_variable);
	USE_PARAMETER(dummy_to_be_modified);
	USE_PARAMETER(dummy_user_data);
	if (state)
	{
		current_token=state->current_token;
		if (current_token != NULL)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				return_code = 1;
				if (!(assign_variable_list) ||
					!(variable = FIND_BY_IDENTIFIER_IN_LIST(Assign_variable, name)
					(current_token, assign_variable_list)))
				{
					if (!(variable = CREATE(Assign_variable)(current_token)))
					{
						display_message(ERROR_MESSAGE,
							"execute_assign_variable.  Unable to find or create variable %s",
							current_token);
						return_code = 0;
					}
				}
				if (return_code && variable)
				{
					shift_Parse_state(state,1);
					current_token=state->current_token;
					if (current_token != NULL)
					{
						if (strcmp(PARSER_HELP_STRING,current_token)&&
							strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
						{
							/* Implement special case getenv command */
							if (!strncmp(current_token, "getenv", 6))
							{
								begin=strchr(current_token,'(');
								if (begin != NULL)
								{
									begin2=strchr(begin,'"');
									if (begin2 != NULL)
									{
										begin = begin2;
										if (!(end = strchr(begin + 1,'"')))
										{
											display_message(ERROR_MESSAGE,
												"execute_assign_variable.  Closing \" missing.",
												state->current_token);
											return_code = 0;
										}
									}
									else
									{
										if (!(end = strchr(begin + 1,')')))
										{
											display_message(ERROR_MESSAGE,
												"execute_assign_variable.  Closing ) missing.",
												state->current_token);
											return_code = 0;
										}
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"execute_assign_variable.  Bracket missing after funciton getenv",
										state->current_token);
									return_code = 0;
								}
								if (return_code)
								{
									if (ALLOCATE(var_name, char, end - begin))
									{
										strncpy(var_name, begin + 1, end - begin - 1);
										var_name[end - begin - 1] = 0;
										env_string = getenv(var_name);
										if (env_string != NULL)
										{
											return_code =
												Assign_variable_set_value(variable,
													env_string);
										}
										else
										{
											display_message(ERROR_MESSAGE,
												"execute_assign_variable.  Environment variable %s not found",
												var_name);
											return_code = 0;
										}
										DEALLOCATE(var_name);
									}
								}
							}
							else
							{
								return_code =
									Assign_variable_set_value(variable, current_token);
							}
						}
						else
						{
							display_message(INFORMATION_MESSAGE,
								"\n           value");
							return_code = 1;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"execute_assign_variable.  Specify new value",
							state->current_token);
						return_code = 0;
					}
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE,
					"\n         VARIABLE_NAME value");
				return_code = 1;
			}
		}
		else
		{
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_assign_variable.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_assign_variable */

int destroy_assign_variable_list(void)
/*******************************************************************************
LAST MODIFIED : 10 March 2000

DESCRIPTION :
Clean up the global assign_variable_list.
==============================================================================*/
{
	int return_code;

	ENTER(destroy_assign_variable_list);

	if (assign_variable_list)
	{
		return_code = DESTROY_LIST(Assign_variable)(&assign_variable_list);
		assign_variable_list = (struct LIST(Assign_variable) *)NULL;
	}
	else
	{
		return_code = 1;
	}

	LEAVE;

	return (return_code);
} /* destroy_assign_variable_list */

int set_name(struct Parse_state *state,void *name_address_void,
	void *prefix_space)
/*******************************************************************************
LAST MODIFIED : 27 May 1997

DESCRIPTION :
Allocates memory for a name, then copies the passed string into it.
==============================================================================*/
{
	const char *current_token;
	char **name_address;
	int return_code;

	ENTER(set_name);
	if (state)
	{
		current_token=state->current_token;
		if (current_token != NULL)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				name_address=(char **)name_address_void;
				if (name_address != NULL)
				{
					if (*name_address)
					{
						DEALLOCATE(*name_address);
					}
					if (ALLOCATE(*name_address,char,strlen(current_token)+1))
					{
						strcpy(*name_address,current_token);
						return_code=shift_Parse_state(state,1);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"set_name.  Could not allocate memory for name");
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,"set_name.  Missing name_address");
					return_code=0;
				}
			}
			else
			{
				if (prefix_space)
				{
					display_message(INFORMATION_MESSAGE," NAME");
				}
				else
				{
					display_message(INFORMATION_MESSAGE,"NAME");
				}
				if ((name_address=(char **)name_address_void)&&(*name_address))
				{
					display_message(INFORMATION_MESSAGE,"[%s]",*name_address);
				}
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing name");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_name.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_name */

int set_names(struct Parse_state *state,void *names_void,
	void *number_of_names_address_void)
/*******************************************************************************
LAST MODIFIED : 10 May 2000

DESCRIPTION :
Modifier function for reading number_of_names (>0) string names from
<state>. User data consists of a pointer to an integer containing the
number_of_names, while <names_void> should point to a large enough space to
store the number_of_names pointers. The names in this array must either be NULL
or pointing to allocated strings.
==============================================================================*/
{
	const char *current_token;
	char **names;
	int i,number_of_names,return_code;

	ENTER(set_names);
	if (state&&(names=(char **)names_void)&&number_of_names_address_void&&
		(0<(number_of_names=*((int *)number_of_names_address_void))))
	{
		current_token=state->current_token;
		if (current_token != NULL)
		{
			return_code=1;
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				for (i=0;return_code&&(i<number_of_names);i++)
				{
					current_token=state->current_token;
					if (current_token != NULL)
					{
						if (names[i])
						{
							DEALLOCATE(names[i]);
						}
						if (NULL != (names[i]=duplicate_string(current_token)))
						{
							return_code=shift_Parse_state(state,1);
						}
						else
						{
							display_message(ERROR_MESSAGE,"set_names.  Not enough memory");
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"Missing name");
						display_parse_state_location(state);
						return_code=0;
					}
				}
			}
			else
			{
				/* write help text */
				for (i=0;i<number_of_names;i++)
				{
					display_message(INFORMATION_MESSAGE," NAME");
				}
				for (i=0;i<number_of_names;i++)
				{
					if (0==i)
					{
						display_message(INFORMATION_MESSAGE,"[",names[0]);
					}
					else
					{
						display_message(INFORMATION_MESSAGE," ",names[i]);
					}
					if (names[i])
					{
						display_message(INFORMATION_MESSAGE,"%s",names[i]);
					}
					else
					{
						display_message(INFORMATION_MESSAGE,"\"\"");
					}
				}
				display_message(INFORMATION_MESSAGE,"]");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing %d names",number_of_names);
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_names.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_names */

static int set_names_from_list(struct Parse_state *state, void *data_void,
	void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 7 July 2004

DESCRIPTION :
Modifier function for reading string names from <state> until one of the
tokens is not in <data->tokens->string>.  While a valid token is encountered
then the index for that token is set in <data->valid_tokens->index> (these should
all be initialised to zero).
==============================================================================*/
{
	const char *current_token;
	int counter, i, return_code, valid_token;
	struct Set_names_from_list_data *data;

	ENTER(set_names_from_list);
	USE_PARAMETER(dummy_void);
	if (state && (data=(struct Set_names_from_list_data *)data_void))
	{
		return_code = 1;
		valid_token = 1;
		counter = 0;
		while (return_code && (NULL != (current_token=state->current_token)) && valid_token)
		{
			return_code=1;
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				valid_token = 0;
				for (i = 0 ; !valid_token && (i < data->number_of_tokens) ; i++)
				{
					if (fuzzy_string_compare_same_length(current_token,
						data->tokens[i].string))
					{
						if (data->tokens[i].index == 0)
						{
							counter++;
							data->tokens[i].index = counter;
							valid_token = 1;
							return_code = shift_Parse_state(state,1);
						}
						else
						{
							display_message(ERROR_MESSAGE,"set_names_from_list.  "
								"Token \"%s\" repeated in list", data->tokens[i].string);
							display_parse_state_location(state);
							return_code = 0;
						}
					}
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," ");
				for (i = 0 ; i < data->number_of_tokens ; i++)
				{
					if (i > 0)
					{
						display_message(INFORMATION_MESSAGE,"|");
					}
					display_message(INFORMATION_MESSAGE,data->tokens[i].string);
				}
				valid_token = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_names_from_list.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_names_from_list */

int set_string(struct Parse_state *state,void *string_address_void,
	void *string_description_void)
/*******************************************************************************
LAST MODIFIED : 1 August 2002

DESCRIPTION :
Parses a string from the parse <state> into <*string_address>. Outputs the
<string_description> text in help mode.
==============================================================================*/
{
	const char *current_token;
	char **string_address;
	int return_code;

	ENTER(set_string);
	if (state && string_description_void)
	{
		current_token = state->current_token;
		if (current_token != NULL)
		{
			if (strcmp(PARSER_HELP_STRING, current_token) &&
				strcmp(PARSER_RECURSIVE_HELP_STRING, current_token))
			{
				string_address = (char **)string_address_void;
				if (string_address != NULL)
				{
					if (*string_address)
					{
						DEALLOCATE(*string_address);
					}
					if (NULL != (*string_address = duplicate_string(current_token)))
					{
						return_code = shift_Parse_state(state,1);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"set_string.  Could not allocate memory for string");
						return_code = 0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,"set_string.  Missing string_address");
					return_code = 0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE, (const char *)string_description_void);
				if ((string_address = (char **)string_address_void) &&
					(*string_address))
				{
					display_message(INFORMATION_MESSAGE, "[%s]", *string_address);
				}
				return_code = 1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE, "Missing string");
			display_parse_state_location(state);
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "set_string.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* set_string */

/***************************************************************************//**
 * Modifier function for extracting a string which may not be reallocated.
 * When used as a default, token-less entry, prevents tokens from being silently
 * ignored.
 *
 * @param state  Current parse state.
 * @param multiple_strings_address_void  Void pointer giving address of string.
 * @param string_description_void  Void pointer to string to write as help.
 * @return  1 on success, 0 on failure.
 */
int set_string_no_realloc(struct Parse_state *state,void *string_address_void,
	void *string_description_void)
{
	const char *current_token;
	char **string_address;
	int return_code;

	ENTER(set_string_no_realloc);
	if (state && (string_address = (char **)string_address_void) &&
		string_description_void)
	{
		current_token = state->current_token;
		if (current_token != NULL)
		{
			if (strcmp(PARSER_HELP_STRING, current_token) &&
				strcmp(PARSER_RECURSIVE_HELP_STRING, current_token))
			{
				if (*string_address)
				{
					display_message(ERROR_MESSAGE, "Already read %s as '%s'",
						(const char *)string_description_void, *string_address);
					display_parse_state_location(state);
					return_code = 0;
				}
				else if (NULL != (*string_address = duplicate_string(current_token)))
				{
					return_code = shift_Parse_state(state,1);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"set_string_no_realloc.  Could not allocate memory for string");
					return_code = 0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE, (const char *)string_description_void);
				return_code = 1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE, "Missing string");
			display_parse_state_location(state);
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "set_string_no_realloc.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* set_string_no_realloc */

int set_int(struct Parse_state *state,void *value_address_void,
	void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 21 June 1999

DESCRIPTION :
A modifier function for setting a int.
==============================================================================*/
{
	const char *current_token;
	int return_code,value,*value_address;

	ENTER(set_int);
	USE_PARAMETER(dummy_user_data);
	if (state)
	{
		current_token=state->current_token;
		if (current_token != NULL)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				value_address=(int *)value_address_void;
				if (value_address != NULL)
				{
					if (1==sscanf(current_token," %d ",&value))
					{
						*value_address=value;
						return_code=shift_Parse_state(state,1);
					}
					else
					{
						display_message(ERROR_MESSAGE,"Invalid integer: %s",current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,"set_int.  Missing value_address");
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," #");
				value_address=(int *)value_address_void;
				if (value_address != NULL)
				{
					display_message(INFORMATION_MESSAGE,"[%d]",*value_address);
				}
				display_message(INFORMATION_MESSAGE,"{integer}");
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing integer");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_int.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_int */

int set_int_with_description(struct Parse_state *state,void *value_address_void,
	void *description_string_void)
/*******************************************************************************
LAST MODIFIED : 1 August 2002

DESCRIPTION :
A modifier function for setting a int.
In help mode writes the <description_string>.
==============================================================================*/
{
	const char *current_token;
	int return_code, value, *value_address;

	ENTER(set_int_with_description);
	if (state && description_string_void)
	{
		current_token = state->current_token;
		if (current_token != NULL)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				value_address=(int *)value_address_void;
				if (value_address != NULL)
				{
					if (1 == sscanf(current_token, " %d ", &value))
					{
						*value_address = value;
						return_code = shift_Parse_state(state, 1);
					}
					else
					{
						display_message(ERROR_MESSAGE, "Invalid integer: %s",
							current_token);
						display_parse_state_location(state);
						return_code = 0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"set_int_with_description.  Missing value_address");
					return_code = 0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE, (char *)description_string_void);
				return_code = 1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE, "Missing integer");
			display_parse_state_location(state);
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_int_with_description.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* set_int_with_description */

int set_int_optional(struct Parse_state *state,void *value_address_void,
	void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 21 June 1999

DESCRIPTION :
If the next token is an integer then the int is set to that value otherwise the
int is set to 1.
==============================================================================*/
{
	const char *current_token;
	int return_code,value,*value_address;

	ENTER(set_int_optional);
	USE_PARAMETER(dummy_user_data);
	if (state)
	{
		value_address=(int *)value_address_void;
		if (value_address != NULL)
		{
			current_token=state->current_token;
			if (current_token != NULL)
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					if (1==sscanf(current_token," %d ",&value))
					{
						*value_address=value;
						return_code=shift_Parse_state(state,1);
					}
					else
					{
						*value_address=1;
						return_code=1;
					}
				}
				else
				{
					display_message(INFORMATION_MESSAGE," <#>[1]{integer}");
					return_code=1;
				}
			}
			else
			{
				*value_address=1;
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"set_int_optional.  Missing value_address");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_int_optional.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_int_optional */

int set_int_non_negative(struct Parse_state *state,void *value_address_void,
	void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 21 June 1999

DESCRIPTION :
A modifier function for setting a int to a non-negative value.
==============================================================================*/
{
	const char *current_token;
	int return_code,value,*value_address;

	ENTER(set_int_non_negative);
	USE_PARAMETER(dummy_user_data);
	if (state)
	{
		current_token=state->current_token;
		if (current_token != NULL)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				value_address=(int *)value_address_void;
				if (value_address != NULL)
				{
					if (1==sscanf(current_token," %d ",&value))
					{
						/* make sure that the value value is non-negative */
						if (value>=0)
						{
							*value_address=value;
							return_code=shift_Parse_state(state,1);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Value must be a non-negative integer: %s\n",current_token);
							display_parse_state_location(state);
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"Invalid non-negative integer: %s",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"set_int_non_negative.  Missing value_address");
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," #");
				value_address=(int *)value_address_void;
				if (value_address != NULL)
				{
					display_message(INFORMATION_MESSAGE,"[%d]",*value_address);
				}
				display_message(INFORMATION_MESSAGE,"{>=0,integer}");
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing non_negative integer");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_int_non_negative.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_int_non_negative */

int set_int_positive(struct Parse_state *state,void *value_address_void,
	void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 21 June 1999

DESCRIPTION :
A modifier function for setting a int to a positive value.
==============================================================================*/
{
	const char *current_token;
	int return_code,value,*value_address;

	ENTER(set_int_positive);
	USE_PARAMETER(dummy_user_data);
	if (state)
	{
		current_token=state->current_token;
		if (current_token != NULL)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				value_address=(int *)value_address_void;
				if (value_address != NULL)
				{
					if (1==sscanf(current_token," %d ",&value))
					{
						/* make sure that the value value is positive */
						if (value>0)
						{
							*value_address=value;
							return_code=shift_Parse_state(state,1);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Value must be a positive integer: %s\n",current_token);
							display_parse_state_location(state);
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"Invalid positive integer: %s",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"set_int_positive.  Missing value_address");
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," #");
				value_address=(int *)value_address_void;
				if (value_address != NULL)
				{
					display_message(INFORMATION_MESSAGE,"[%d]",*value_address);
				}
				display_message(INFORMATION_MESSAGE,"{>0,integer}");
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing positive integer");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_int_positive.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_int_positive */

int set_int_and_char_flag(struct Parse_state *state,void *value_address_void,
	void *flag_address_void)
/*******************************************************************************
LAST MODIFIED : 22 December 2000

DESCRIPTION :
A modifier function for setting an int, and a char flag in the user data to
indicate that the int has been set.
==============================================================================*/
{
	const char *current_token;
	char *flag_address;
	int value, *value_address;
	int return_code;

	ENTER(set_int_and_char_flag);
	if (state && (NULL != (value_address = (int *)value_address_void)) &&
		(flag_address = (char *)flag_address_void))
	{
		current_token = state->current_token;
		if (current_token != NULL)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if (1 == sscanf(current_token, " %d ", &value))
				{
					*value_address = value;
					*flag_address = 1;
					return_code = shift_Parse_state(state, 1);
				}
				else
				{
					display_message(ERROR_MESSAGE, "Invalid int: %s", current_token);
					display_parse_state_location(state);
					return_code = 0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE, " #");
				if (*flag_address)
				{
					display_message(INFORMATION_MESSAGE, "[%d]", *value_address);
				}
				else
				{
					display_message(INFORMATION_MESSAGE, "[NOT SET]");
				}
				return_code = 1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE, "Missing int");
			display_parse_state_location(state);
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_int_and_char_flag.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* set_int_and_char_flag */

int set_int_vector(struct Parse_state *state,void *values_address_void,
	void *number_of_components_address_void)
/*******************************************************************************
LAST MODIFIED : 27 March 2000

DESCRIPTION :
Modifier function for reading number_of_components (>0) ints from <state>.
User data consists of a pointer to an integer containing number_of_components,
while <values_address_void> should point to a large enough space to store the
number_of_components ints.
Now prints current contents of the vector with help.
==============================================================================*/
{
	const char *current_token;
	int value,*values_address;
	int comp_no,number_of_components,return_code;

	ENTER(set_int_vector);
	if (state && number_of_components_address_void)
	{
		values_address=(int *)values_address_void;
		number_of_components= *((int *)number_of_components_address_void);
		current_token=state->current_token;
		if (current_token != NULL)
		{
			return_code=1;
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if (values_address && (0 < number_of_components))
				{
					for (comp_no=0;return_code&&(comp_no<number_of_components);comp_no++)
					{
						current_token=state->current_token;
						if (current_token != NULL)
						{
							if (1==sscanf(current_token," %d ",&value))
							{
								values_address[comp_no]=value;
								return_code=shift_Parse_state(state,1);
							}
							else
							{
								display_message(ERROR_MESSAGE,"Invalid int: %s",current_token);
								display_parse_state_location(state);
								return_code=0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,"Missing int vector component(s)");
							display_parse_state_location(state);
							return_code=0;
						}
					}
				}
			}
			else
			{
				/* write help text */
				if (values_address && (0 < number_of_components))
				{
					for (comp_no=0;comp_no<number_of_components;comp_no++)
					{
						display_message(INFORMATION_MESSAGE," #");
					}
					display_message(INFORMATION_MESSAGE,"[%d",values_address[0]);
					for (comp_no=1;comp_no<number_of_components;comp_no++)
					{
						display_message(INFORMATION_MESSAGE," %d",values_address[comp_no]);
					}
					display_message(INFORMATION_MESSAGE,"]");
				}
				else
				{
					display_message(INFORMATION_MESSAGE," VALUES");
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Missing %d component int vector",number_of_components);
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_int_vector.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_int_vector */

int set_float(struct Parse_state *state,void *value_address_void,
	void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 21 June 1999

DESCRIPTION :
A modifier function for setting a float.
==============================================================================*/
{
	const char *current_token;
	float value,*value_address;
	int return_code;

	ENTER(set_float);
	USE_PARAMETER(dummy_user_data);
	if (state)
	{
		current_token=state->current_token;
		if (current_token != NULL)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				value_address=(float *)value_address_void;
				if (value_address != NULL)
				{
					if (1==sscanf(current_token," %f ",&value))
					{
						*value_address=value;
						return_code=shift_Parse_state(state,1);
					}
					else
					{
						display_message(ERROR_MESSAGE,"Invalid float: %s",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,"set_float.  Missing value_address");
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," #");
				value_address=(float *)value_address_void;
				if (value_address != NULL)
				{
					display_message(INFORMATION_MESSAGE,"[%g]",*value_address);
				}
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing float");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_float.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_float */

int set_float_and_char_flag(struct Parse_state *state,void *value_address_void,
	void *flag_address_void)
/*******************************************************************************
LAST MODIFIED : 9 September 1999

DESCRIPTION :
A modifier function for setting a float, and a char flag in the user data to
indicate that the float has been set.
==============================================================================*/
{
	const char *current_token;
	char *flag_address;
	float value,*value_address;
	int return_code;

	ENTER(set_float_and_char_flag);
	if (state)
	{
		current_token=state->current_token;
		if (current_token != NULL)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				value_address=(float *)value_address_void;
				flag_address=(char *)flag_address_void;
				if ((value_address != NULL) && (flag_address != NULL))
				{
					if (1==sscanf(current_token," %f ",&value))
					{
						*value_address=value;
						*flag_address=1;
						return_code=shift_Parse_state(state,1);
					}
					else
					{
						display_message(ERROR_MESSAGE,"Invalid float: %s",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"set_float_and_char_flag.  Missing value_address");
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," #");
				value_address=(float *)value_address_void;
				if (value_address != NULL)
				{
					display_message(INFORMATION_MESSAGE,"[%g]",*value_address);
				}
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing float");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_float_and_char_flag.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_float_and_char_flag */

int set_float_positive(struct Parse_state *state,void *value_address_void,
	void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 21 June 1999

DESCRIPTION :
A modifier function for setting a float to a positive value.
==============================================================================*/
{
	float value,*value_address;
	int return_code;

	ENTER(set_float_positive);
	USE_PARAMETER(dummy_user_data);
	if (state)
	{
		if (state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,state->current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token))
			{
				value_address=(float *)value_address_void;
				if (value_address != NULL)
				{
					if (1==sscanf(state->current_token," %f ",&value))
					{
						/* make sure that the value value is positive */
						if (value>0)
						{
							*value_address=value;
							return_code=shift_Parse_state(state,1);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Value must be a positive float: %s\n",state->current_token);
							display_parse_state_location(state);
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"Invalid positive float: %s",
							state->current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"set_float_positive.  Missing value_address");
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," #");
				value_address=(float *)value_address_void;
				if (value_address != NULL)
				{
					display_message(INFORMATION_MESSAGE,"[%g]",*value_address);
				}
				display_message(INFORMATION_MESSAGE,"{>0}");
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing positive float");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_float_positive.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_float_positive */

int set_FE_value_positive(struct Parse_state *state,void *value_address_void,
	void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 21 June 1999

DESCRIPTION :
A modifier function for setting a FE_value to a positive value.
==============================================================================*/
{
	FE_value value,*value_address;
	int return_code;

	ENTER(set_FE_value_positive);
	USE_PARAMETER(dummy_user_data);
	if (state)
	{
		if (state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,state->current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token))
			{
				value_address=(FE_value *)value_address_void;
				if (value_address != NULL)
				{
					if (1==sscanf(state->current_token," "FE_VALUE_INPUT_STRING" ",&value))
					{
						/* make sure that the value value is positive */
						if (value>0)
						{
							*value_address=value;
							return_code=shift_Parse_state(state,1);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Value must be a positive FE_value: %s\n",state->current_token);
							display_parse_state_location(state);
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"Invalid positive FE_vlaue: %s",
							state->current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"set_FE_value_positive.  Missing value_address");
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," #");
				value_address=(FE_value *)value_address_void;
				if (value_address != NULL)
				{
					display_message(INFORMATION_MESSAGE,"[%g]",*value_address);
				}
				display_message(INFORMATION_MESSAGE,"{>0}");
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing positive FE_value");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_FE_value_positive.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_FE_value_positive */

int set_float_non_negative(struct Parse_state *state,void *value_address_void,
	void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 21 June 1999

DESCRIPTION :
A modifier function for setting a float to a non_negative value.
==============================================================================*/
{
	const char *current_token;
	float value,*value_address;
	int return_code;

	ENTER(set_float_non_negative);
	USE_PARAMETER(dummy_user_data);
	if (state)
	{
		current_token=state->current_token;
		if (current_token != NULL)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				value_address=(float *)value_address_void;
				if (value_address != NULL)
				{
					if (1==sscanf(current_token," %f ",&value))
					{
						/* make sure that the value value is non-negative */
						if (value>=0)
						{
							*value_address=value;
							return_code=shift_Parse_state(state,1);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Value must be a non_negative float: %s\n",current_token);
							display_parse_state_location(state);
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"Invalid non-negative float: %s",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"set_float_non_negative.  Missing value_address");
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," #");
				value_address=(float *)value_address_void;
				if (value_address != NULL)
				{
					display_message(INFORMATION_MESSAGE,"[%g]",*value_address);
				}
				display_message(INFORMATION_MESSAGE,"{>=0}");
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing non-negative float");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_float_non_negative.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_float_non_negative */

int set_float_0_to_1_inclusive(struct Parse_state *state,
	void *value_address_void,void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 21 June 1999

DESCRIPTION :
A modifier function for setting a float to a value in [0,1].
==============================================================================*/
{
	const char *current_token;
	float value,*value_address;
	int return_code;

	ENTER(set_float_0_to_1_inclusive);
	USE_PARAMETER(dummy_user_data);
	if (state)
	{
		current_token=state->current_token;
		if (current_token != NULL)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				value_address=(float *)value_address_void;
				if (value_address != NULL)
				{
					if (1==sscanf(current_token," %f ",&value))
					{
						/* make sure that the value value is non-negative */
						if (value>=0)
						{
							*value_address=value;
							return_code=shift_Parse_state(state,1);
						}
						else
						{
							display_message(ERROR_MESSAGE,"Value must be a 0<=float<=1: %s\n",
								current_token);
							display_parse_state_location(state);
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"Invalid 0<=float<=1: %s",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"set_float_0_to_1_inclusive.  Missing value_address");
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," #");
				value_address=(float *)value_address_void;
				if (value_address != NULL)
				{
					display_message(INFORMATION_MESSAGE,"[%g]",*value_address);
				}
				display_message(INFORMATION_MESSAGE,"{>=0,<=1}");
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing 0<=float<=1");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_float_0_to_1_inclusive.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_float_0_to_1_inclusive */

int set_double(struct Parse_state *state,void *value_address_void,
	void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 21 June 1999

DESCRIPTION :
A modifier function for setting a double.
==============================================================================*/
{
	const char *current_token;
	double value,*value_address;
	int return_code;

	ENTER(set_double);
	USE_PARAMETER(dummy_user_data);
	if (state)
	{
		current_token=state->current_token;
		if (current_token != NULL)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				value_address=(double *)value_address_void;
				if (value_address != NULL)
				{
					if (1==sscanf(current_token," %lf ",&value))
					{
						*value_address=value;
						return_code=shift_Parse_state(state,1);
					}
					else
					{
						display_message(ERROR_MESSAGE,"Invalid double: %s",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,"set_double.  Missing value_address");
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," #");
				value_address=(double *)value_address_void;
				if (value_address != NULL)
				{
					display_message(INFORMATION_MESSAGE,"[%g]",*value_address);
				}
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing double");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_double.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_double */

int set_double_and_char_flag(struct Parse_state *state,void *value_address_void,
	void *flag_address_void)
/*******************************************************************************
LAST MODIFIED : 19 October 1998

DESCRIPTION :
A modifier function for setting a double, and a char flag in the user data to
indicate that the double has been set.
???SAB  The user_data could be used to supply many more helpful things such as
	limits on the double or a string used in the help.
==============================================================================*/
{
	const char *current_token;
	char *flag_address;
	double value,*value_address;
	int return_code;

	ENTER(set_double_and_char_flag);
	if (state)
	{
		current_token=state->current_token;
		if (current_token != NULL)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				value_address=(double *)value_address_void;
				flag_address=(char *)flag_address_void;
				if ((value_address != NULL) && (flag_address != NULL))
				{
					if (1==sscanf(current_token," %lf ",&value))
					{
						*value_address=value;
						*flag_address=1;
						return_code=shift_Parse_state(state,1);
					}
					else
					{
						display_message(ERROR_MESSAGE,"Invalid double: %s",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"set_double_and_char_flag.  Missing value_address");
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," #");
				value_address=(double *)value_address_void;
				if (value_address != NULL)
				{
					display_message(INFORMATION_MESSAGE,"[%g]",*value_address);
				}
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing double");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_double_and_char_flag.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_double_and_char_flag */

int set_double_non_negative(struct Parse_state *state,void *value_address_void,
	void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 21 June 1999

DESCRIPTION :
A modifier function for setting a double to a non_negative value.
==============================================================================*/
{
	const char *current_token;
	double value,*value_address;
	int return_code;

	ENTER(set_double_non_negative);
	USE_PARAMETER(dummy_user_data);
	if (state)
	{
		current_token=state->current_token;
		if (current_token != NULL)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				value_address = (double *)value_address_void;
				if (value_address != NULL)
				{
					if (1==sscanf(current_token," %lf ",&value))
					{
						/* make sure that the value value is non-negative */
						if (value>=0)
						{
							*value_address=value;
							return_code=shift_Parse_state(state,1);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Value must be a non_negative double: %s\n",current_token);
							display_parse_state_location(state);
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"Invalid non-negative double: %s",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"set_double_non_negative.  Missing value_address");
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," #");
				value_address=(double *)value_address_void;
				if (value_address != NULL)
				{
					display_message(INFORMATION_MESSAGE,"[%lg]",*value_address);
				}
				display_message(INFORMATION_MESSAGE,"{>=0}");
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing non-negative double");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_double_non_negative.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
}

int set_special_float3(struct Parse_state *state,void *values_address_void,
	void *separation_char_address_void)
/*******************************************************************************
LAST MODIFIED : 9 July 1998

DESCRIPTION :
Modifier function for setting a float[3] from a token with 1 to 3 characters
separated by the character at <separation_char_address> which may be either an
asterisk or a comma. If '*' is used missing components take the values of the
last number entered, eg '3' -> 3,3,3, while  '2.4*7.6' becomes 2.4,7.6,7.6.
This functionality is useful for setting the size of glyphs. If the separation
character is ',', values of unspecified components are left untouched, useful
for setting glyph offsets which default to zero or some other number.
Missing a number by putting two separators together works as expected, eg:
'1.2**3.0' returns 1.2,1.2,3.0, '*2' gives 0.0,2.0,2.0 while ',,3' changes the
third component of the float only to 3.
???RC The comma case does not work since ',' is a delimiter for the parser.
==============================================================================*/
{
	const char *current_token;
	char separator;
	float value,*values;
	int i,return_code;

	ENTER(set_special_float3);
	if (state&&(values=(float *)values_address_void)&&
		(separator=*((char *)separation_char_address_void))&&
		(('*'==separator)||(','==separator)))
	{
		current_token=state->current_token;
		if (current_token != NULL)
		{
			return_code=1;
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				value=(float)0.0;
				for (i=0;i<3;i++)
				{
					if (current_token&&(*current_token != separator))
					{
						value=(float)atof(current_token);
						values[i]=value;
						current_token=strchr(current_token,separator);
					}
					else
					{
						if ('*' == separator)
						{
							values[i]=value;
						}
					}
					if (current_token)
					{
						current_token++;
						if ('\0' == *current_token)
						{
							current_token=(char *)NULL;
						}
					}
				}
				return_code=shift_Parse_state(state,1);
			}
			else
			{
				display_message(INFORMATION_MESSAGE,
					" #%c#%c#[%g%c%g%c%g]{float[%cfloat[%cfloat]]}",separator,separator,
					values[0],separator,values[1],separator,values[2],
					separator,separator);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing vector");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_float_vector.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_special_float3 */

int set_float_vector(struct Parse_state *state,void *values_address_void,
	void *number_of_components_address_void)
/*******************************************************************************
LAST MODIFIED : 20 November 1998

DESCRIPTION :
Modifier function for reading number_of_components (>0) floats from <state>.
User data consists of a pointer to an integer containing number_of_components,
while <values_address_void> should point to a large enough space to store the
number_of_components floats.
Now prints current contents of the vector with help.
==============================================================================*/
{
	const char *current_token;
	float value,*values_address;
	int comp_no,number_of_components,return_code;

	ENTER(set_float_vector);
	if (state)
	{
		if ((values_address=(float *)values_address_void)&&
			number_of_components_address_void&&(0<(number_of_components=
			*((int *)number_of_components_address_void))))
		{
			current_token=state->current_token;
			if (current_token != NULL)
			{
				return_code=1;
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					for (comp_no=0;return_code&&(comp_no<number_of_components);comp_no++)
					{
						current_token=state->current_token;
						if (current_token != NULL)
						{
							if (1==sscanf(current_token," %f ",&value))
							{
								values_address[comp_no]=value;
								return_code=shift_Parse_state(state,1);
							}
							else
							{
								display_message(ERROR_MESSAGE,"Invalid float: %s",
									current_token);
								display_parse_state_location(state);
								return_code=0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Missing float vector component(s)");
							display_parse_state_location(state);
							return_code=0;
						}
					}
				}
				else
				{
					/* write help text */
					for (comp_no=0;comp_no<number_of_components;comp_no++)
					{
						display_message(INFORMATION_MESSAGE," #");
					}
					display_message(INFORMATION_MESSAGE,"[%g",values_address[0]);
					for (comp_no=1;comp_no<number_of_components;comp_no++)
					{
						display_message(INFORMATION_MESSAGE," %g",values_address[comp_no]);
					}
					display_message(INFORMATION_MESSAGE,"]");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Missing %d component float vector",number_of_components);
				display_parse_state_location(state);
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"set_float_vector.  Invalid argument(s)");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_float_vector.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_float_vector */

int set_reversed_float_vector(struct Parse_state *state,void *values_address_void,
	void *number_of_components_address_void)
{
	int return_code = 0;
	float *values_address;
	int comp_no,number_of_components;

	if (set_float_vector(state, values_address_void, number_of_components_address_void))
	{
		if ((values_address=(float *)values_address_void)&&
			number_of_components_address_void&&(0<(number_of_components=
				*((int *)number_of_components_address_void))))
		{
			for (comp_no=0;(comp_no<number_of_components);comp_no++)
			{
				// want to avoid values of -0.0
				if (values_address[comp_no] != 0.0f)
				{
					values_address[comp_no] = -values_address[comp_no];
				}
			}
		}
		return_code = 1;
	}

	return (return_code);
}

int set_FE_value(struct Parse_state *state,void *value_address_void,
	void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 21 June 1999

DESCRIPTION :
A modifier function for setting a float.
==============================================================================*/
{
	const char *current_token;
	FE_value value,*value_address;
	int return_code;

	ENTER(set_FE_value);
	USE_PARAMETER(dummy_user_data);
	if (state)
	{
		current_token=state->current_token;
		if (current_token != NULL)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				value_address=(FE_value *)value_address_void;
				if (value_address != NULL)
				{
					if (1==sscanf(current_token,FE_VALUE_INPUT_STRING,&value))
					{
						*value_address=value;
						return_code=shift_Parse_state(state,1);
					}
					else
					{
						display_message(ERROR_MESSAGE,"Invalid fe value: %s",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,"set_FE_value.  Missing value_address");
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," #");
				value_address=(FE_value *)value_address_void;
				if (value_address != NULL)
				{
					display_message(INFORMATION_MESSAGE,"[%g]",*value_address);
				}
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing FE_value");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_FE_value.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_FE_value */

int set_FE_value_array(struct Parse_state *state, void *values_void,
	void *number_of_components_address_void)
/*******************************************************************************
LAST MODIFIED : 6 November 2001

DESCRIPTION :
Modifier function for reading number_of_components (>0) FE_values from <state>.
User data consists of a pointer to an integer containing number_of_components,
while <values_void> should point to a large enough space to store the
number_of_components FE_values.
<number_of_components> can be zero and <values> can be NULL as long as only
help mode is entered.
Now prints current contents of the vector with help.
==============================================================================*/
{
	const char *current_token;
	FE_value value, *values;
	int i, number_of_components, return_code = 0;

	ENTER(set_FE_value_array);
	if (state && number_of_components_address_void)
	{
		values = (FE_value *)values_void;
		number_of_components = *((int *)number_of_components_address_void);
		current_token = state->current_token;
		if (current_token != NULL)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if (values && (0 < number_of_components))
				{
					return_code = 1;
					for (i = 0; return_code && (i < number_of_components); i++)
					{
						current_token = state->current_token;
						if (current_token != NULL)
						{
							if (1 == sscanf(current_token, " "FE_VALUE_INPUT_STRING" ", &value))
							{
								values[i] = value;
								return_code = shift_Parse_state(state, 1);
							}
							else
							{
								display_message(ERROR_MESSAGE, "Invalid FE_value: %s",
									current_token);
								display_parse_state_location(state);
								return_code = 0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Missing FE_value vector component(s)");
							display_parse_state_location(state);
							return_code = 0;
						}
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"set_FE_value_array.  Invalid array or number_of_components");
					return_code = 0;
				}
			}
			else
			{
				if (values && (0 < number_of_components))
				{
					/* write help text */
					for (i = 0; i < number_of_components; i++)
					{
						display_message(INFORMATION_MESSAGE," #");
					}
					display_message(INFORMATION_MESSAGE, "[%g", values[0]);
					for (i = 1; i < number_of_components; i++)
					{
						display_message(INFORMATION_MESSAGE, " %g", values[i]);
					}
					display_message(INFORMATION_MESSAGE, "]");
				}
				else
				{
					display_message(INFORMATION_MESSAGE," VALUES");
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE, "Missing values");
			display_parse_state_location(state);
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "set_FE_value_array.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* set_FE_value_array */

int set_double_vector(struct Parse_state *state,void *values_address_void,
	void *number_of_components_address_void)
/*******************************************************************************
LAST MODIFIED : 20 November 1998

DESCRIPTION :
Modifier function for reading number_of_components (>0) floats from <state>.
User data consists of a pointer to an integer containing number_of_components,
while <values_address_void> should point to a large enough space to store the
number_of_components floats.
Now prints current contents of the vector with help.
==============================================================================*/
{
	const char *current_token;
	double value,*values_address;
	int comp_no,number_of_components,return_code;

	ENTER(set_double_vector);
	if (state && number_of_components_address_void)
	{
		values_address=(double *)values_address_void;
		number_of_components=	*((int *)number_of_components_address_void);
		current_token=state->current_token;
		if (current_token != NULL)
		{
			return_code=1;
			if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if (values_address && (0 < number_of_components))
				{
					for (comp_no=0;return_code&&(comp_no<number_of_components);comp_no++)
					{
						current_token=state->current_token;
						if (current_token != NULL)
						{
							if (1==sscanf(current_token," %lf ",&value))
							{
								values_address[comp_no]=value;
								return_code=shift_Parse_state(state,1);
							}
							else
							{
								display_message(ERROR_MESSAGE,"Invalid double: %s",
									current_token);
								display_parse_state_location(state);
								return_code=0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
							  "Missing double vector component(s)");
							display_parse_state_location(state);
							return_code=0;
						}
					}
				}
			}
			else
			{
				/* write help text */
				if (values_address && (0 < number_of_components))
				{
					for (comp_no=0;comp_no<number_of_components;comp_no++)
					{
						display_message(INFORMATION_MESSAGE," #");
					}
					display_message(INFORMATION_MESSAGE,"[%g",values_address[0]);
					for (comp_no=1;comp_no<number_of_components;comp_no++)
					{
						display_message(INFORMATION_MESSAGE," %g",values_address[comp_no]);
					}
					display_message(INFORMATION_MESSAGE,"]");
				}
				else
				{
					display_message(INFORMATION_MESSAGE," VALUES");
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Missing %d component double vector",number_of_components);
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_double_vector.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_double_vector */

static int set_variable_length_double_vector(struct Parse_state *state,
	void *values_address_void, void *number_of_components_address_void)
/*******************************************************************************
LAST MODIFIED : 18 February 2005

DESCRIPTION :
Modifier function for reading doubles from <state>.  This function keeps
consuming tokens until one cannot be parsed as a double and sets this
<number_of_components> into <number_of_components_address>.
User data consists of a pointer to an integer containing number_of_components,
while <values_address_void> should point to a large enough space to store the
number_of_components floats.
==============================================================================*/
{
	const char *current_token;
	double value,**values_address;
	int allocated_length = 0,comp_no,length_read,return_code,valid_token;
#define VARIABLE_LENGTH_VECTOR_ALLOCATION (10)

	ENTER(set_variable_length_double_vector);
	if (state)
	{
		if ((values_address=(double **)values_address_void)&&
			number_of_components_address_void)
		{
			current_token=state->current_token;
			if (current_token != NULL)
			{
				return_code=1;
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					comp_no = 0;
					valid_token = 1;
					if (REALLOCATE(*values_address, *values_address, double,
							VARIABLE_LENGTH_VECTOR_ALLOCATION))
					{
						allocated_length = VARIABLE_LENGTH_VECTOR_ALLOCATION;
					}
					else
					{
						display_message(ERROR_MESSAGE, "set_variable_length_double_vector.  "
							"Unable to allocate memory.");
						return_code=0;
					}
					while (valid_token && return_code)
					{
						if (comp_no >= allocated_length)
						{
							REALLOCATE(*values_address, *values_address, double,
								VARIABLE_LENGTH_VECTOR_ALLOCATION + allocated_length);
							allocated_length += VARIABLE_LENGTH_VECTOR_ALLOCATION;
						}
						current_token=state->current_token;
						if (current_token != NULL)
						{
							if (1==sscanf(current_token," %lf%n ",&value,&length_read))
							{
								if (length_read == (int)strlen(current_token))
								{
									(*values_address)[comp_no]=value;
									comp_no++;
									return_code=shift_Parse_state(state,1);
								}
								else
								{
									valid_token = 0;
								}
							}
							else
							{
								valid_token = 0;
							}
						}
						else
						{
							valid_token = 0;
						}
					}
					if (comp_no > 0)
					{
						*((int *)number_of_components_address_void) = comp_no;
						if (allocated_length != comp_no)
						{
							REALLOCATE(*values_address, *values_address, double,
								comp_no);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"No valid double vector component(s) found.");
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					/* write help text */
					display_message(INFORMATION_MESSAGE," #..#");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE, "Missing double vector");
				display_parse_state_location(state);
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"set_variable_length_double_vector.  "
				"Invalid argument(s)");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_variable_length_double_vector.  "
			"Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_variable_length_double_vector */

int set_double_vector_with_help(struct Parse_state *state,
	void *vector_void,void *set_vector_with_help_data_void)
/*******************************************************************************
LAST MODIFIED : 20 November 1998

DESCRIPTION :
Modifier function to parse a variable number of double values with appropriate
help text. Number of components in vectoy, help text and set flag are contained
in the struct Set_vector_with_help_data passed as the last argument. The 'set'
flag is set if values are entered into the vector.
The current values of the vector are not printed with the help text since they
may not be initialised (calling function could put them in the help text).
==============================================================================*/
{
	const char *current_token;
	double *vector;
	int return_code,i;
	struct Set_vector_with_help_data *set_vector_data;

	ENTER(set_double_vector_with_help);
	if (state&&(vector=(double *)vector_void)&&(set_vector_data=
		(struct Set_vector_with_help_data *)set_vector_with_help_data_void)&&
		(0<set_vector_data->num_values)&&set_vector_data->help_text)
	{
		current_token=state->current_token;
		if (current_token != NULL)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				set_vector_data->set=1;
				return_code=1;
				for (i=set_vector_data->num_values;return_code&&(0<i);i--)
				{
					current_token=state->current_token;
					if (current_token != NULL)
					{
						*vector=atof(current_token);
						vector++;
						return_code=shift_Parse_state(state,1);
					}
					else
					{
						display_message(ERROR_MESSAGE,"Missing vector component(s)");
						return_code=0;
					}
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE,set_vector_data->help_text);
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing%s",set_vector_data->help_text);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_double_vector_with_help.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_double_vector_with_help */

int set_char_flag(struct Parse_state *state,void *value_address_void,
	void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 21 June 1999

DESCRIPTION :
A modifier function for setting a character flag to 1.
==============================================================================*/
{
	char *value_address;
	int return_code;

	ENTER(set_char_flag);
	USE_PARAMETER(dummy_user_data);
	if (state)
	{
		value_address=(char *)value_address_void;
		if (value_address != NULL)
		{
			*value_address=1;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,"set_char_flag.  Missing value_address");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_char_flag.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_char_flag */

int unset_char_flag(struct Parse_state *state,void *value_address_void,
	void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 21 June 1999

DESCRIPTION :
A modifier function for setting a character flag to 0.
==============================================================================*/
{
	char *value_address;
	int return_code;

	ENTER(unset_char_flag);
	USE_PARAMETER(dummy_user_data);
	if (state)
	{
		value_address=(char *)value_address_void;
		if (value_address != NULL)
		{
			*value_address=0;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,"unset_char_flag.  Missing value_address");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"unset_char_flag.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* unset_char_flag */

int set_int_switch(struct Parse_state *state,void *value_address_void,
	void *token_void)
/*******************************************************************************
LAST MODIFIED : 12 May 2000

DESCRIPTION :
A modifier function for setting an integer switch to 1.
If the value is currently set, this is indicated in the help, with the <token>
if supplied, otherwise the word CURRENT.
If the option's <token> is supplied and its value is currently set, it
==============================================================================*/
{
	char *token;
	int *value_address;
	int return_code;

	ENTER(set_int_switch);
	value_address=(int *)value_address_void;
	if (state && (value_address != NULL))
	{
		if (!Parse_state_help_mode(state))
		{
			*value_address=1;
			return_code=1;
		}
		else
		{
			/* indicate if switch currently set */
			if (*value_address)
			{
				token = (char *)token_void;
				if (token != NULL)
				{
					display_message(INFORMATION_MESSAGE,"[%s]",token);
				}
				else
				{
					display_message(INFORMATION_MESSAGE,"[CURRENT]");
				}
			}
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_int_switch.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_int_switch */

int unset_int_switch(struct Parse_state *state,void *value_address_void,
	void *token_void)
/*******************************************************************************
LAST MODIFIED : 12 May 2000

DESCRIPTION :
A modifier function for setting an integer switch to 0.
If the value is currently unset, this is indicated in the help, with the <token>
if supplied, otherwise the word CURRENT.
If the option's <token> is supplied and its value is currently set, it
==============================================================================*/
{
	char *token;
	int *value_address;
	int return_code;

	ENTER(unset_int_switch);
	value_address=(int *)value_address_void;
	if (state && (value_address != NULL))
	{
		if (!Parse_state_help_mode(state))
		{
			*value_address=0;
			return_code=1;
		}
		else
		{
			/* indicate if switch currently unset */
			if (!(*value_address))
			{
				token = (char *)token_void;
				if (token != NULL)
				{
					display_message(INFORMATION_MESSAGE,"[%s]",token);
				}
				else
				{
					display_message(INFORMATION_MESSAGE,"[CURRENT]");
				}
			}
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"unset_int_switch.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* unset_int_switch */

int set_file_name(struct Parse_state *state,void *name_address_void,
	void *directory_name_address_void)
/*******************************************************************************
LAST MODIFIED : 26 September 1996

DESCRIPTION :
Allows the user to specify "special" directories, eg examples.  Allocates the
memory for the file name string.
==============================================================================*/
{
	const char *current_token;
	char *directory_name,**name_address;
	int file_name_length,return_code;

	ENTER(set_file_name);
	if (state)
	{
		current_token = state->current_token;
		if (current_token != NULL)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				name_address=(char **)name_address_void;
				if (name_address != NULL)
				{
					if (*name_address)
					{
						DEALLOCATE(*name_address);
					}
					if (directory_name_address_void)
					{
						directory_name= *((char **)directory_name_address_void);
					}
					else
					{
						directory_name=(char *)NULL;
					}
					file_name_length=strlen(current_token)+1;
					if (directory_name)
					{
						file_name_length += strlen(directory_name);
					}
					if (ALLOCATE(*name_address,char,file_name_length+1))
					{
						(*name_address)[0]='\0';
						if (directory_name)
						{
							strcat(*name_address,directory_name);
						}
						strcat(*name_address,current_token);
						return_code=shift_Parse_state(state,1);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"set_file_name.  Could not allocate memory for name");
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"set_file_name.  Missing name_address");
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," FILE_NAME");
				if ((name_address=(char **)name_address_void)&&(*name_address))
				{
					display_message(INFORMATION_MESSAGE,"[%s]",*name_address);
				}
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing file name");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_file_name.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_file_name */

int set_integer_range(struct Parse_state *state,
	void *integer_range_address_void,void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 21 June 1999

DESCRIPTION :
integer_range= *((int **)integer_range_address_void) is an array.
integer_range[0] is the number of pairs of ints in the rest of integer_range, so
that the size of integer_range is 1+2*integer_range[0].  integer_range is
ordered integer_range[2*i+1]<=integer_range[2*i+2]<integer_range[2*i+3] for
i>=0.  The integers in integer_range are those j for which
integer_range[2*i+1]<=j<=integer_range[2*i+2] for some i>=0.

This routine updates the integer_range based on the current token which can be
of two forms - # or #..#
==============================================================================*/
{
	const char *current_token;
	int first,i,*integer_range,**integer_range_address,j,last,
		number_of_sub_ranges,return_code;

	ENTER(set_integer_range);
	USE_PARAMETER(dummy_user_data);
	if (state)
	{
		current_token=state->current_token;
		if (current_token != NULL)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				return_code=0;
				if (2==sscanf(current_token,"%d..%d",&first,&last))
				{
					if (first<=last)
					{
						return_code=1;
					}
				}
				else
				{
					if (1==sscanf(current_token,"%d",&first))
					{
						last=first;
						return_code=1;
					}
				}
				if (return_code)
				{
					integer_range_address=(int **)integer_range_address_void;
					if (integer_range_address != NULL)
					{
						if ((integer_range= *integer_range_address)&&
							(0<(number_of_sub_ranges=integer_range[0])))
						{
							i=0;
							while ((i<number_of_sub_ranges)&&(first>=integer_range[2*i+1]))
							{
								i++;
							}
							if (i<number_of_sub_ranges)
							{
								if (last>=integer_range[2*i+1]-1)
								{
									integer_range[2*i+1]=first;
									if (last>integer_range[2*i+2])
									{
										integer_range[2*i+2]=last;
									}
									i= -1;
								}
							}
							if (i>0)
							{
								if (first<=integer_range[2*i]+1)
								{
									integer_range[2*i]=last;
									if (first<integer_range[2*i-1])
									{
										integer_range[2*i-1]=first;
									}
									i= -1;
								}
							}
							if (i>=0)
							{
								number_of_sub_ranges++;
								if (REALLOCATE(integer_range,integer_range,int,
									2*number_of_sub_ranges+1))
								{
									*integer_range_address=integer_range;
									integer_range[0]=number_of_sub_ranges;
									for (j=number_of_sub_ranges-1;j>i;j--)
									{
										integer_range[2*j+2]=integer_range[2*j];
										integer_range[2*j+1]=integer_range[2*j-1];
									}
									integer_range[2*i+2]=last;
									integer_range[2*i+1]=first;
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"set_integer_range.  Could not reallocate integer_range");
									return_code=0;
								}
							}
						}
						else
						{
							if (REALLOCATE(integer_range,integer_range,int,3))
							{
								integer_range[0]=1;
								integer_range[1]=first;
								integer_range[2]=last;
								*integer_range_address=integer_range;
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"set_integer_range.  Could not reallocate integer_range");
								return_code=0;
							}
						}
						if (return_code)
						{
							return_code=shift_Parse_state(state,1);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"set_integer_range.  Missing integer_range_address");
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,"Invalid integer range");
					display_parse_state_location(state);
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," #|#..#");
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing integer range");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_integer_range.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_integer_range */

int set_enum(struct Parse_state *state,void *set_value_address_void,
	void *enum_value_address_void)
/*******************************************************************************
LAST MODIFIED : 19 November 1998

DESCRIPTION :
A modifier function for setting an enumerated type variable to a specified
value.
NB.  *enum_value_address_void is put in *set_value_address_void
???DB.  Unwieldy.  Can it be done better ?
==============================================================================*/
{
	int *enum_value_address,return_code,*set_value_address;

	ENTER(set_enum);
	USE_PARAMETER(state);
	if ((set_value_address=(int *)set_value_address_void)&&
		(enum_value_address=(int *)enum_value_address_void))
	{
		*set_value_address=*enum_value_address;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_enum.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_enum */

int Option_table_add_char_flag_entry(struct Option_table *option_table,
	const char *token, char *flag)
/*******************************************************************************
LAST MODIFIED : 8 October 2003

DESCRIPTION :
Adds the given <token> to the <option_table>.  If the <token> is specified
then the <flag> will be set.
==============================================================================*/
{
	int return_code;

	ENTER(Option_table_add_char_flag_entry);
	if (option_table && token && flag)
	{
		return_code = Option_table_add_entry(option_table, token, (void *)flag, NULL,
			set_char_flag);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Option_table_add_char_flag_entry.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Option_table_add_char_flag_entry */

int Option_table_add_unset_char_flag_entry(struct Option_table *option_table,
	const char *token, char *flag)
{
	int return_code;

	ENTER(Option_table_add_unset_char_flag_entry);
	if (option_table && token && flag)
	{
		return_code = Option_table_add_entry(option_table, token, (void *)flag, NULL,
			unset_char_flag);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Option_table_add_unset_char_flag_entry.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Option_table_add_unset_char_flag_entry */

int Option_table_add_int_positive_entry(struct Option_table *option_table,
	const char *token, int *value)
/*******************************************************************************
LAST MODIFIED : 4 December 2003

DESCRIPTION :
Adds the given <token> to the <option_table>.  If the <token> is specified then
the token following is assigned to <value>.
==============================================================================*/
{
	int return_code;

	ENTER(Option_table_add_int_positive_entry);
	if (option_table && token && value)
	{
		return_code = Option_table_add_entry(option_table, token, (void *)value, NULL,
			set_int_positive);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Option_table_add_int_positive_entry.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Option_table_add_int_positive_entry */

int Option_table_add_int_non_negative_entry(struct Option_table *option_table,
	const char *token, int *value)
/*******************************************************************************
LAST MODIFIED : 4 December 2003

DESCRIPTION :
Adds the given <token> to the <option_table>.  If the <token> is specified then
the token following is assigned to <value>.
==============================================================================*/
{
	int return_code;

	ENTER(Option_table_add_int_non_negative_entry);
	if (option_table && token && value)
	{
		return_code = Option_table_add_entry(option_table, token, (void *)value, NULL,
			set_int_non_negative);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Option_table_add_int_non_negative_entry.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Option_table_add_int_non_negative_entry */

int Option_table_add_int_vector_entry(struct Option_table *option_table,
	const char *token, int *vector, int *number_of_components)
/*******************************************************************************
LAST MODIFIED : 4 December 2003

DESCRIPTION :
Adds the given <token> to the <option_table>.  The <vector> is filled in with the
<number_of_components>.
<number_of_components> can be zero and <values> can be NULL as long as only
help mode is entered.
==============================================================================*/
{
	int return_code;

	ENTER(Option_table_add_int_vector_entry);
	if (option_table && token && number_of_components)
	{
		return_code = Option_table_add_entry(option_table, token, vector,
			(void *)number_of_components, set_int_vector);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Option_table_add_int_vector_entry.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Option_table_add_int_vector_entry */

int Option_table_add_float_entry(struct Option_table *option_table,
	const char *token, float *value)
/*******************************************************************************
LAST MODIFIED : 28 June 2006

DESCRIPTION :
Adds the given <token> to the <option_table>.  If the <token> is specified then
the token following is assigned to <value>.
==============================================================================*/
{
	int return_code;

	ENTER(Option_table_add_float_entry);
	if (option_table && token && value)
	{
		return_code = Option_table_add_entry(option_table, token, (void *)value, NULL,
			set_float);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Option_table_add_float_entry.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Option_table_add_float_entry */

int Option_table_add_float_vector_entry(struct Option_table *option_table,
	const char *token, float *vector, int *number_of_components)
/*******************************************************************************
LAST MODIFIED : 16 July 2007

DESCRIPTION :
Adds the given <token> to the <option_table>.  The <vector> is filled in with the
<number_of_components>.
<number_of_components> can be zero and <values> can be NULL as long as only
help mode is entered.
==============================================================================*/
{
	int return_code;

	ENTER(Option_table_add_float_vector_entry);
	if (option_table && token && number_of_components)
	{
		return_code = Option_table_add_entry(option_table, token, vector,
			(void *)number_of_components, set_float_vector);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Option_table_add_float_vector_entry.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Option_table_add_float_vector_entry */

int Option_table_add_FE_value_vector_entry(struct Option_table *option_table,
	const char *token, FE_value *vector, int *number_of_components)
/*******************************************************************************
LAST MODIFIED : 4 December 2003

DESCRIPTION :
Adds the given <token> to the <option_table>.  The <vector> is filled in with the
<number_of_components>.
<number_of_components> can be zero and <values> can be NULL as long as only
help mode is entered.
==============================================================================*/
{
	int return_code;

	ENTER(Option_table_add_FE_value_vector_entry);
	if (option_table && token && number_of_components)
	{
		return_code = Option_table_add_entry(option_table, token, vector,
			(void *)number_of_components, set_FE_value_array);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Option_table_add_FE_value_vector_entry.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Option_table_add_FE_value_vector_entry */

int Option_table_add_double_entry(struct Option_table *option_table,
	const char *token, double *value)
/*******************************************************************************
LAST MODIFIED : 8 October 2003

DESCRIPTION :
Adds the given <token> to the <option_table>.  If the <token> is specified then
the token following is assigned to <value>.
==============================================================================*/
{
	int return_code;

	ENTER(Option_table_add_double_entry);
	if (option_table && token && value)
	{
		return_code = Option_table_add_entry(option_table, token, value,
			NULL, set_double);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Option_table_add_double_entry.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Option_table_add_double_entry */

int Option_table_add_non_negative_double_entry(struct Option_table *option_table,
	const char *token, double *value)
{
	int return_code;

	ENTER(Option_table_add_non_negative_double_entry);
	if (option_table && token && value)
	{
		return_code = Option_table_add_entry(option_table, token, value,
			NULL, set_double_non_negative);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Option_table_add_non_negative_double_entry.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
}

int Option_table_add_double_vector_entry(struct Option_table *option_table,
	const char *token, double *vector, int *number_of_components)
/*******************************************************************************
LAST MODIFIED : 8 October 2003

DESCRIPTION :
Adds the given <token> to the <option_table>.  The <vector> is filled in with the
<number_of_components>.
==============================================================================*/
{
	int return_code;

	ENTER(Option_table_add_double_vector_entry);
	if (option_table && token && number_of_components)
	{
		return_code = Option_table_add_entry(option_table, token, vector,
			(void *)number_of_components, set_double_vector);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Option_table_add_double_vector_entry.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Option_table_add_double_vector_entry */

int Option_table_add_variable_length_double_vector_entry(
	struct Option_table *option_table, const char *token, int *number_of_components,
	double **vector)
/*******************************************************************************
LAST MODIFIED : 18 February 2005

DESCRIPTION :
Adds the given <token> to the <option_table>.  The <vector> is filled in with the
<number_of_components>.
==============================================================================*/
{
	int return_code;

	ENTER(Option_table_add_variable_length_double_vector_entry);
	if (option_table && token && vector && number_of_components)
	{
		return_code = Option_table_add_entry(option_table, token, vector,
			(void *)number_of_components, set_variable_length_double_vector);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Option_table_add_variable_length_double_vector_entry.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Option_table_add_variable_length_double_vector_entry */

int Option_table_add_double_vector_with_help_entry(
	struct Option_table *option_table, const char *token, double *vector,
	struct Set_vector_with_help_data *data)
/*******************************************************************************
LAST MODIFIED : 8 October 2003

DESCRIPTION :
Adds the given <token> to the <option_table>.  The <vector> is filled in with the
number of values specified in the <data>.
==============================================================================*/
{
	int return_code;

	ENTER(Option_table_add_double_vector_with_help_entry);
	if (option_table && token && vector && data)
	{
		return_code = Option_table_add_entry(option_table, token, (void *)vector,
			(void *)data, set_double_vector_with_help);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Option_table_add_double_vector_with_help_entry.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Option_table_add_double_vector_with_help_entry */

int Option_table_add_name_entry(struct Option_table *option_table,
	const char *token, char **name)
/*******************************************************************************
LAST MODIFIED : 25 March 2004

DESCRIPTION :
Adds the given <token> to the <option_table>.  If the <token> is specified then
the token following is assigned to <value>.  If <token> is NULL then this becomes
a default option.
==============================================================================*/
{
	int return_code;

	ENTER(Option_table_add_name_entry);
	if (option_table && name)
	{
		return_code = Option_table_add_entry(option_table, token, name,
			NULL, set_name);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Option_table_add_name_entry.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Option_table_add_name_entry */

int Option_table_add_set_names_from_list_entry(struct Option_table *option_table,
   const char *token, struct Set_names_from_list_data *data)
/*******************************************************************************
LAST MODIFIED : 7 July 2004

DESCRIPTION :
Adds the given <token> to the <option_table>.  The <data> contains an array
of size <data->number_of_tokens> tokens.  Each of these tokens points to a
string <data->tokens[i].string>.  Input will be read from the parse state until
a token not in the list of strings.  As each string is encountered the
corresponding <data->tokens[i].index> is set.  When set these start from one,
and are initialised to zero in this routine.  The function checks that the tokens
are not repeated.
==============================================================================*/
{
	int i, return_code;

	ENTER(Option_table_add_names_from_list_entry);
	if (option_table && token && data && data->number_of_tokens)
	{
		for (i = 0 ; i < data->number_of_tokens ; i++)
		{
			data->tokens[i].index = 0;
		}
		return_code = Option_table_add_entry(option_table, token,
			(void *)data, NULL, set_names_from_list);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Option_table_add_names_from_list_entry.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Option_table_add_names_from_list_entry */

static int set_nothing(struct Parse_state *state,void *dummy_to_be_modified,
	void *expected_parameters_void)
/*******************************************************************************
LAST MODIFIED : 21 September 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code = 0;
	int* expected_parameters;

	ENTER(set_nothing);
	USE_PARAMETER(dummy_to_be_modified);
	expected_parameters = (int *)expected_parameters_void;
	if (expected_parameters && *expected_parameters)
	{
		return_code=shift_Parse_state(state, *expected_parameters);
	}
	LEAVE;

	return (return_code);
} /* set_nothing */

int Option_table_add_ignore_token_entry(struct Option_table *option_table,
	const char *token, int *expected_parameters)
/*******************************************************************************
LAST MODIFIED : 21 September 2006

DESCRIPTION :
Specifies that the given <token> will be ignored when parsing the option_table.
The specified number of <expected_parameters> will also be ignored following
the <token>.
==============================================================================*/
{
	int return_code;

	ENTER(Option_table_add_ignore_token_entry);
	if (option_table && token)
	{
		return_code = Option_table_add_entry(option_table, token,
			NULL, (void *)expected_parameters, set_nothing);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Option_table_add_ignore_token_entry.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Option_table_add_ignore_token_entry */

int Option_table_add_special_float3_entry(struct Option_table *option_table,
	const char *token, float *values, const char *separation_char_string)
{
	int return_code;

	ENTER(Option_table_add_special_float3_entry);
	if (option_table && token)
	{
		return_code = Option_table_add_entry(option_table, token,
			values, (void *)separation_char_string, set_special_float3);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Option_table_add_special_float3_entry.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Option_table_add_special_float3_entry */


static int set_nothing_and_shift(struct Parse_state *state,void *dummy_to_be_modified,
	void *dummy_user_data_void)
/*******************************************************************************
LAST MODIFIED : 21 September 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(set_nothing);
	USE_PARAMETER(dummy_to_be_modified);
	USE_PARAMETER(dummy_user_data_void);
	return_code=shift_Parse_state(state, 1);
	LEAVE;

	return (return_code);
} /* set_nothing */

int Option_table_ignore_all_unmatched_entries(struct Option_table *option_table)
/*******************************************************************************
LAST MODIFIED : 21 September 2006

DESCRIPTION :
Adds a dummy option to the Option_table that will consume and ignore all tokens
that do not match other options.  This option must be added last.
==============================================================================*/
{
	int return_code;

	ENTER(Option_table_ignore_all_unmatched_entries);
	if (option_table)
	{
		/* Advance past this NULL entry */
		return_code = Option_table_add_entry(option_table, NULL,
			NULL, NULL, set_nothing_and_shift);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Option_table_ignore_all_unmatched_entries.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Option_table_ignore_all_unmatched_entries */

int Option_table_add_string_entry(struct Option_table *option_table,
	const char *token, char **string_address, const char *string_description)
{
	int return_code;

	ENTER(Option_table_add_string_entry);
	if (option_table && token && string_address && string_description)
	{
		return_code = Option_table_add_entry(option_table, token,
			(void *)string_address, (void *)string_description, set_string);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Option_table_add_string_entry.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Option_table_add_string_entry */

int Option_table_add_default_string_entry(struct Option_table *option_table,
	char **string_address, const char *string_description)
{
	int return_code;

	ENTER(Option_table_add_default_string_entry);
	if (option_table && string_address && string_description)
	{
		if (*string_address)
		{
			display_message(ERROR_MESSAGE,
				"Option_table_add_default_string_entry.  String must initially be NULL");
			Option_table_set_invalid(option_table);
			return_code = 0;
		}
		else
		{
			return_code = Option_table_add_entry(option_table, /*token*/(const char *)NULL,
				(void *)string_address, (void *)string_description, set_string_no_realloc);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Option_table_add_default_string_entry.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Option_table_add_default_string_entry */

/***************************************************************************//**
 * Modifier function for extracting one or more strings, separated by and
 * ampersand &.
 *
 * @param state  Current parse state.
 * @param multiple_strings_address_void  Address of Multiple_strings structure.
 * @param strings_description_void  void pointer to string to write as help.
 * @return  1 on success, 0 on failure.
 */
int set_multiple_strings(struct Parse_state *state,void *multiple_strings_address_void,
	void *strings_description_void)
{
	const char *separator = "&";
	const char *current_token;
	char **new_strings;
	int finished, last_separator, return_code;
	struct Multiple_strings *multiple_strings;

	ENTER(set_multiple_strings);
	if (state && (multiple_strings = (struct Multiple_strings *)multiple_strings_address_void) &&
		((0 == multiple_strings->number_of_strings) ||
			(0 < multiple_strings->number_of_strings && multiple_strings->strings)) &&
		strings_description_void)
	{
		last_separator = 1;
		return_code = 1;
		finished = 0;
		while (!finished && (NULL != (current_token = state->current_token)) && return_code)
		{
			if ((0 == strcmp(PARSER_HELP_STRING, current_token)) ||
				(0 == strcmp(PARSER_RECURSIVE_HELP_STRING, current_token)))
			{
				display_message(INFORMATION_MESSAGE, " %s", (char *)strings_description_void);
				finished = 1;
			}
			else if (0 == strcmp(current_token, separator))
			{
				if (last_separator)
				{
					display_message(ERROR_MESSAGE, "Missing string");
					display_parse_state_location(state);
					return_code = 0;
				}
				else
				{
					return_code = shift_Parse_state(state, 1);
				}
				last_separator = 1;
			}
			else if (!last_separator)
			{
				finished = 1;
			}
			else
			{
				if (REALLOCATE(new_strings, multiple_strings->strings, char *, multiple_strings->number_of_strings + 1))
				{
					multiple_strings->strings = new_strings;
					new_strings[multiple_strings->number_of_strings] = duplicate_string(current_token);
					if (new_strings[multiple_strings->number_of_strings] != NULL)
					{
						multiple_strings->number_of_strings++;
						return_code = shift_Parse_state(state, 1);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"set_multiple_strings.  Could not allocate memory for string");
						return_code = 0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"set_multiple_strings.  Could not reallocate string array");
					return_code = 0;
				}
				last_separator = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "set_multiple_strings.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* set_multiple_strings */

int Option_table_add_multiple_strings_entry(struct Option_table *option_table,
	const char *token, struct Multiple_strings *multiple_strings_address,
	const char *strings_description)
{
	int return_code;

	ENTER(Option_table_add_multiple_strings_entry);
	if (option_table && token && multiple_strings_address && strings_description)
	{
		return_code = Option_table_add_entry(option_table, token,
			(void *)multiple_strings_address, (void *)strings_description, set_multiple_strings);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Option_table_add_multiple_strings_entry.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Option_table_add_multiple_strings_entry */

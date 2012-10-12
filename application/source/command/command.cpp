/*******************************************************************************
FILE : command.c

LAST MODIFIED : 15 July 2002

DESCRIPTION :
Functions associated with commands.
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
#include <string.h>
#include <stdio.h>
#include "command/command.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "general/message.h"
#include "user_interface/user_interface.h"

/*
Module types
------------
*/

struct Execute_command
/*******************************************************************************
LAST MODIFIED : 5 June 1996

DESCRIPTION :
==============================================================================*/
{
	Execute_command_function *function;
	void *data;
}; /* struct Execute_command */

/*
Global functions
----------------
*/

int read_iod_file_via_selection_box(char *file_name,void *execute_command_void)
/*******************************************************************************
LAST MODIFIED : 15 July 2002

DESCRIPTION:
Submits a command to open an iod file.
==============================================================================*/
{
	char *command_string,*file_extension;
	int length,return_code;
	struct Execute_command *execute_command;

	ENTER(read_iod_file_via_selection_box);
	/* check arguments */
	if (file_name &&
		(execute_command=(struct Execute_command *)execute_command_void))
	{
		/* remove the file extension */
		if (NULL != (file_extension=strrchr(file_name,'.')))
		{
			length=file_extension-file_name;
		}
		else
		{
			length=strlen(file_name);
		}
		if (ALLOCATE(command_string,char,length+14))
		{
			strcpy(command_string,"FEM read ");
			strncpy(command_string+9,file_name,length);
			strcpy(command_string+(9+length),";iod");
			return_code =
				Execute_command_execute_string(execute_command, command_string);
			DEALLOCATE(command_string);
		}
		else
		{
			display_message(ERROR_MESSAGE,"read_iod_file_via_selection_box.  "
				"Could not allocate memory for command string");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"read_iod_file_via_selection_box.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* read_iod_file_via_selection_box */

struct Execute_command *CREATE(Execute_command)(void)
/*******************************************************************************
LAST MODIFIED : 15 July 2002

DESCRIPTION :
Creates a blank execute command. Must call Execute_command_set_command_function
to set the function it calls, and the user data to be passed with it.
==============================================================================*/
{
	struct Execute_command *execute_command;

	ENTER(CREATE(Execute_command));
	if (ALLOCATE(execute_command,struct Execute_command, 1))
	{
		execute_command->function = (Execute_command_function *)NULL;
		execute_command->data = (void *)NULL;
	}
	else
	{
		display_message(ERROR_MESSAGE,"CREATE(Execute_command).  "
			"Unable to allocate Execute_command structure");
		execute_command = (struct Execute_command *)NULL;
	}
	LEAVE;

	return (execute_command);
} /* CREATE(Execute_command) */

int DESTROY(Execute_command)(struct Execute_command **execute_command_address)
/*******************************************************************************
LAST MODIFIED : 5 October 2001

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(DESTROY(Execute_command));
	return_code=0;
	if (execute_command_address&&(*execute_command_address))
	{
		DEALLOCATE(*execute_command_address);
		*execute_command_address=(struct Execute_command *)NULL;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Execute_command).  Missing execute_command");
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Execute_command) */

int Execute_command_set_command_function(
	struct Execute_command *execute_command,
	Execute_command_function *execute_command_function,
	void *command_function_data)
/*******************************************************************************
LAST MODIFIED : 15 July 2002

DESCRIPTION :
Sets the function called by <execute_command>, and the user data to be passed
with it.
==============================================================================*/
{
	int return_code;

	ENTER(Execute_command_set_command_function);
	if (execute_command && execute_command_function)
	{
		execute_command->function = execute_command_function;
		execute_command->data = command_function_data;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Execute_command_set_command_function.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Execute_command_set_command_function */

int Execute_command_execute_string(struct Execute_command *execute_command,
	const char *command_string)
/*******************************************************************************
LAST MODIFIED : 15 July 2002

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Execute_command_execute_string);
	return_code = 0;
	if (execute_command)
	{
		if (command_string)
		{
			if (execute_command->function)
			{
				return_code =
					(*(execute_command->function))((char *)command_string, execute_command->data);
			}
			else
			{
				display_message(ERROR_MESSAGE, "Execute_command_execute_string.  "
					"Missing function for executing '%s'",command_string);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Execute_command_execute_string.  Missing command string");
		}
	}
	else
	{
		if (command_string)
		{
			display_message(ERROR_MESSAGE, "Execute_command_execute_string.  "
				"Missing Execute command for '%s'",command_string);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Execute_command_execute_string.  Missing Execute command");
		}
	}
	LEAVE;

	return (return_code);
} /* Execute_command_execute_string */

int execute_comfile(char *file_name,struct IO_stream_package *io_stream_package,
	struct Execute_command *execute_command)
/******************************************************************************
LAST MODIFIED : 3 September 2004

DESCRIPTION :
Opens, executes and then closes a com file.  No window is created.
=============================================================================*/
{
	char *command_string;
	int return_code;
	struct IO_stream *comfile;

	ENTER(execute_comfile);
	if (file_name)
	{
		if (execute_command)
		{
			if ((comfile=CREATE(IO_stream)(io_stream_package)) &&
				IO_stream_open_for_read(comfile, file_name))
			{
				IO_stream_scan(comfile," ");
				while (!IO_stream_end_of_stream(comfile)&&
					(IO_stream_read_string(comfile,"[^\n]",&command_string)))
				{
					Execute_command_execute_string(execute_command, command_string);
					DEALLOCATE(command_string);
					IO_stream_scan(comfile," ");
				}
				IO_stream_close(comfile);
				DESTROY(IO_stream)(&comfile);
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,"Could not open: %s",file_name);
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"execute_comfile.  "
				"Invalid execute command");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_comfile.  Missing file name");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_comfile */


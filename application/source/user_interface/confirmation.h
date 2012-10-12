/*******************************************************************************
FILE : confirmation.h

LAST MODIFIED : 7 July 1999

DESCRIPTION :
Routines for waiting for user input.
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
#if !defined (CONFIRMATION_H)
#define CONFIRMATION_H

#if 1
#include "configure/cmgui_configure.h"
#endif /* defined (1) */

#include "user_interface/user_interface.h"

/*
Global types
------------
*/

/*
Global functions
----------------
*/
int confirmation_warning_ok_cancel(const char *title,const char *prompt,
	struct User_interface *user_interface
#if  defined (WX_USER_INTERFACE) 
	 , struct Execute_command *execute_command
#endif /* defined (WX_USER_INTERFACE) */
																	 );
/*******************************************************************************
LAST MODIFIED : 11 June 1999

DESCRIPTION :
This routine supplies a dialog window which requires a response
before anything else will continue and returns 1 if the OK button
is clicked and 0 if the cancel button is clicked.
==============================================================================*/

int confirmation_error_ok(char *title,char *prompt,
	struct User_interface *user_interface
#if  defined (WX_USER_INTERFACE) 
	 , struct Execute_command *execute_command
#endif /* defined (WX_USER_INTERFACE) */
													);
/*******************************************************************************
LAST MODIFIED : 11 June 1999

DESCRIPTION :
This routine supplies a error dialog window which requires a response before
anything else will continue and returns as the OK button is clicked.  No other
options are supplied.
==============================================================================*/

int confirmation_information_ok(char *title,char *prompt,
	struct User_interface *user_interface
#if  defined (WX_USER_INTERFACE) 
	 , struct Execute_command *execute_command
#endif /* defined (WX_USER_INTERFACE) */
																);
/*******************************************************************************
LAST MODIFIED : 11 June 1999

DESCRIPTION :
This routine supplies a information dialog window which requires a response
before anything else will continue and returns as the OK button is clicked.  No
other options are supplied.
==============================================================================*/

int confirmation_warning_ok(const char *title,const char *prompt,
	struct User_interface *user_interface
#if  defined (WX_USER_INTERFACE) 
	 , struct Execute_command *execute_command
#endif /* defined (WX_USER_INTERFACE) */
														);
/*******************************************************************************
LAST MODIFIED : 11 June 1999

DESCRIPTION :
This routine supplies a warning dialog window which requires a response before
anything else will continue and returns as the OK button is clicked.  No other
options are supplied.
==============================================================================*/

int confirmation_question_yes_no(char *title,char *prompt,
	struct User_interface *user_interface
#if  defined (WX_USER_INTERFACE) 
	 , struct Execute_command *execute_command
#endif /* defined (WX_USER_INTERFACE) */
																 );
/*******************************************************************************
LAST MODIFIED : 11 June 1999

DESCRIPTION :
This routine supplies a dialog window which requires a response
before anything else will continue.  It returns one if the Yes button
is clicked and No if it isn't.
==============================================================================*/

char *confirmation_get_read_filename(const char *extension,
	struct User_interface *user_interface
#if  defined (WX_USER_INTERFACE) 
	 , struct Execute_command *execute_command
#endif /* defined (WX_USER_INTERFACE) */
																		 );
/*******************************************************************************
LAST MODIFIED : 11 June 1999

DESCRIPTION :
This routine supplies a file selection dialog window
==============================================================================*/

char *confirmation_get_write_filename(const char *extension,
	struct User_interface *user_interface
#if  defined (WX_USER_INTERFACE) 
	 , struct Execute_command *execute_command
#endif /* defined (WX_USER_INTERFACE) */
																			);
/*******************************************************************************
LAST MODIFIED : 11 June 1999

DESCRIPTION :
This routine supplies a file selection dialog window
==============================================================================*/

char *confirmation_change_current_working_directory(
	struct User_interface *user_interface
#if  defined (WX_USER_INTERFACE) 
	 , struct Execute_command *execute_command
#endif /* defined (WX_USER_INTERFACE) */
	);
/*******************************************************************************
LAST MODIFIED : 7 July 1999

DESCRIPTION :
This routine supplies a file selection dialog window for changing the current
working directory.  The new directory will be created if necessary.
==============================================================================*/

#endif /* !defined (CONFIRMATION_H) */

/*******************************************************************************
FILE : graphics_font.h

LAST MODIFIED : 17 November 2005

DESCRIPTION :
This provides a Cmgui interface to the OpenGL contexts of many types.
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
#if !defined (GRAPHICS_FONT_H)
#define GRAPHICS_FONT_H

#include "general/callback.h"
#include "general/manager.h"
#include "general/object.h"

/*
Global types
------------
*/

struct Graphics_font_package;

struct Graphics_font;

DECLARE_LIST_TYPES(Graphics_font);

DECLARE_MANAGER_TYPES(Graphics_font);

/*
Global functions
----------------
*/

struct Graphics_font_package *CREATE(Graphics_font_package)(void);
/*******************************************************************************
LAST MODIFIED : 11 April 2007

DESCRIPTION :
==============================================================================*/


int DESTROY(Graphics_font_package)(struct Graphics_font_package **package_address);
/*******************************************************************************
LAST MODIFIED : 11 April 2007

DESCRIPTION :
==============================================================================*/



struct MANAGER(Graphics_font)
	 *Graphics_font_package_get_font_manager(
		 struct Graphics_font_package *font_package);
/*******************************************************************************
LAST MODIFIED : 17 May 2007

DESCRIPTION :
Extracts the font_manager from the graphics_font_package. Note that
the rest of the program should use this sparingly - it is really only here to
allow interfacing to the choose_object widgets.
==============================================================================*/

int Graphics_font_package_define_font(
	struct Graphics_font_package *font_package,
	const char *font_name, const char *font_string);
/*******************************************************************************
LAST MODIFIED : 11 April 2007

DESCRIPTION :
Defines font named <font_name> in the <font_package> using the
user interface dependent <font_string>.
==============================================================================*/

struct Graphics_font *Graphics_font_package_get_font(
	struct Graphics_font_package *font_package, const char *font_name);
/*******************************************************************************
LAST MODIFIED : 11 April 2007

DESCRIPTION :
Finds a Graphics_font with name <font_name> in the <font_package>.
If it doesn't exist then a font is created using the <font_name>
as the user interface dependent font string.
==============================================================================*/

PROTOTYPE_OBJECT_FUNCTIONS(Graphics_font);

PROTOTYPE_GET_OBJECT_NAME_FUNCTION(Graphics_font);

PROTOTYPE_LIST_FUNCTIONS(Graphics_font);
PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(Graphics_font,name,const char *);

PROTOTYPE_MANAGER_COPY_FUNCTIONS(Graphics_font,name,const char *);
PROTOTYPE_MANAGER_FUNCTIONS(Graphics_font);
PROTOTYPE_MANAGER_IDENTIFIER_FUNCTIONS(Graphics_font,name,const char *);

int DESTROY(Graphics_font)(struct Graphics_font **font_address);
/*******************************************************************************
LAST MODIFIED : 17 November 2005

DESCRIPTION :
==============================================================================*/


int Graphics_font_compile(struct Graphics_font *font,
	struct Graphics_buffer *buffer);
/*******************************************************************************
LAST MODIFIED : 17 November 2005

DESCRIPTION :
Compiles the specified <font> so it can be used by the graphics.  The
<buffer> is required as the Win32 API requires a window context.
==============================================================================*/

int Graphics_font_rendergl_text(struct Graphics_font *font, char *text,
	float x, float y, float z);
/*******************************************************************************
LAST MODIFIED : 17 November 2005

DESCRIPTION :
==============================================================================*/
#endif /* !defined (GRAPHICS_FONT_H) */


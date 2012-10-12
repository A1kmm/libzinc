/*******************************************************************************
FILE : element_point_tool.h

LAST MODIFIED : 5 July 2002

DESCRIPTION :
Interactive tool for selecting element/grid points with mouse and other devices.
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
#if !defined (ELEMENT_POINT_TOOL_H)
#define ELEMENT_POINT_TOOL_H

#include "graphics/material.h"
#include "interaction/interactive_tool.h"
#include "selection/element_point_ranges_selection.h"
#include "time/time_keeper.h"
/*
Global types
------------
*/

struct Element_point_tool;
/*******************************************************************************
LAST MODIFIED : 16 May 2000

DESCRIPTION :
The contents of this structure are private.
==============================================================================*/

/*
Global functions
----------------
*/

struct Element_point_tool *CREATE(Element_point_tool)(
	struct MANAGER(Interactive_tool) *interactive_tool_manager,
	struct Cmiss_region *region,
	struct Element_point_ranges_selection *element_point_ranges_selection,
	struct Graphical_material *rubber_band_material,
	struct User_interface *user_interface,
	struct Time_keeper *time_keeper);
/*******************************************************************************
LAST MODIFIED : 5 July 2002

DESCRIPTION :
Creates an Element_point_tool with Interactive_tool in
<interactive_tool_manager>. Selects element/grid points in
<element_point_ranges_selection> in response to interactive_events.
==============================================================================*/

int DESTROY(Element_point_tool)(
	struct Element_point_tool **element_point_tool_address);
/*******************************************************************************
LAST MODIFIED : 16 May 2000

DESCRIPTION :
Frees and deaccesses objects in the <element_point_tool> and deallocates the
structure itself.
==============================================================================*/

int Element_point_tool_pop_up_dialog(
																		 struct Element_point_tool *element_point_tool,struct Graphics_window *graphics_window);
/*******************************************************************************
LAST MODIFIED : 5 July 2002

DESCRIPTION :
Pops up a dialog for editing settings of the Element_point_tool.
==============================================================================*/

int Element_point_tool_pop_down_dialog(
	struct Element_point_tool *element_point_tool);
/*******************************************************************************
LAST MODIFIED : 5 July 2002

DESCRIPTION :
Hides the dialog for editing settings of the Element_point_tool.
==============================================================================*/

struct Computed_field *Element_point_tool_get_command_field(
	struct Element_point_tool *element_point_tool);
/*******************************************************************************
LAST MODIFIED : 30 September 2003

DESCRIPTION :
Returns the command_field to be executed when the element is clicked on in the 
<element_point_tool>.
==============================================================================*/

int Element_point_tool_set_command_field(
	struct Element_point_tool *element_point_tool,
	struct Computed_field *command_field);
/*******************************************************************************
LAST MODIFIED : 30 September 2003

DESCRIPTION :
Sets the command_field to be executed when the element is clicked on in the 
<element_point_tool>.
==============================================================================*/

struct Interactive_tool *Element_point_tool_get_interactive_tool(
  struct Element_point_tool *element_point_tool);
/*******************************************************************************
LAST MODIFIED : 29 March 2007

DESCRIPTION :
Returns the generic interactive_tool the represents the <element_point_tool>.
==============================================================================*/

int Element_point_tool_set_execute_command(struct Element_point_tool *element_point_tool, 
	struct Execute_command *execute_command);
#endif /* !defined (ELEMENT_POINT_TOOL_H) */

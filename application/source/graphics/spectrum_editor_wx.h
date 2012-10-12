/*******************************************************************************
FILE : spectrum_editor_wx.h

LAST MODIFIED : 23 August 2007

DESCRIPTION :
Provides the widgets to manipulate graphical element group settings.
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
#if !defined (SPECTRUM_EDITOR_H)
#define SPECTRUM_EDITOR_H

#include "user_interface/user_interface.h"
#include "graphics/spectrum_editor_dialog_wx.h"
/*
Global Types
------------
*/
struct Spectrum_editor;


/*
Global Functions
----------------
*/

struct Spectrum_editor *CREATE(Spectrum_editor)(
	 struct Spectrum_editor_dialog **spectrum_editor_dialog_address,
	 struct Spectrum *spectrum,
	 struct Graphics_font *font,
	 struct Graphics_buffer_app_package *graphics_buffer_package,
	 struct User_interface *user_interface,
	 struct Cmiss_graphics_module *graphics_module,
	 struct MANAGER(Scene) *scene_manager,
	 struct Cmiss_region *spectrum_region);
/*******************************************************************************
LAST MODIFIED : 4 May 2004

DESCRIPTION :
Creates a spectrum_editor widget.
==============================================================================*/

int spectrum_editor_wx_set_spectrum(
	struct Spectrum_editor *spectrum_editor,
	struct Spectrum *spectrum);
/*******************************************************************************
LAST MODIFIED : 23 August 2007

DESCRIPTION :
Set the <spectrum> for the <spectrum_editor_dialog>.
==============================================================================*/

int spectrum_editor_wx_add_item_to_spectrum_editor_check_list(struct Spectrum *spectrum, void *spectrum_editor_void);
/*******************************************************************************
LAST MODIFIED : 28 August 2007

DESCRIPTION :
Add spectrum item to the spectrum_editor.
==============================================================================*/

void spectrum_editor_wx_bring_up_editor(struct Spectrum_editor *spectrum_editor);
/*******************************************************************************
LAST MODIFIED : 10 Jan 2008

DESCRIPTION :
bring the spectrum editor to the front.
==============================================================================*/


int DESTROY(Spectrum_editor)(struct Spectrum_editor **spectrum_editor_address);
/*******************************************************************************
LAST MODIFIED : 12 August 2002

DESCRIPTION :
Destroys the <*spectrum_editor_address> and sets
<*spectrum_editor_address> to NULL.
==============================================================================*/

#endif /* !defined (SPECTRUM_EDITOR_H) */

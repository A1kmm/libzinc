/*******************************************************************************
FILE : render_stl.h

LAST MODIFIED : 3 July 2008

DESCRIPTION :
Renders gtObjects to STL stereolithography file.
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
 * Portions created by the Initial Developer are Copyright (C) 2008
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
#if !defined (RENDERSTL_H)
#define RENDERSTL_H

struct Cmiss_scene;
#define Scene Cmiss_scene // GRC temporary
struct Scene_object;

/*
Global functions
----------------
*/

/**************************************************************************//**
 * Renders the visible objects to an STL file.
 * 
 * @param file_name The name of the file to write to.
 * @param scene The scene to output
 * @param scene_object A scene object to output; if not specified use scene.
 * @return 1 on success, 0 on failure
 */
int export_to_stl(char *file_name, struct Scene *scene);

#endif /* !defined (RENDERSTL_H) */

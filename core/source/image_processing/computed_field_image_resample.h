/*******************************************************************************
FILE : computed_field_image_resample.h

LAST MODIFIED : 7 March 2007

DESCRIPTION :
Field that changes the native resolution of a computed field.
Image processing fields use the native resolution to determine their image size.
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
#if !defined (COMPUTED_FIELD_IMAGE_RESAMPLE_H)
#define COMPUTED_FIELD_IMAGE_RESAMPLE_H

/***************************************************************************//**
 * Create a field which resamples the image in source field, and overrides the
 * sizes that will be used for subsequent image processing operations.
 * The texture_coordinate field could also be overridden.  The minimums and
 * maximums are not implemented at all, which would allow a windowing, and could
 * also be overridden here.
 * 
 * @param field_module  Region field module which will own new field.
 * @return  Newly created field.
 */
Computed_field *Cmiss_field_module_create_image_resample(
	struct Cmiss_field_module *field_module,
	struct Computed_field *source_field, int dimension, int *sizes);

#endif /* !defined (COMPUTED_FIELD_IMAGE_RESAMPLE_H) */

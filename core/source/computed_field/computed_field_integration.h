/*******************************************************************************
FILE : computed_field_integration.h

LAST MODIFIED : 26 October 2000

DESCRIPTION :
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
#if !defined (COMPUTED_FIELD_INTEGRATION_H)
#define COMPUTED_FIELD_INTEGRATION_H

int Computed_field_is_type_integration(struct Computed_field *field);
/*******************************************************************************
LAST MODIFIED : 26 October 2000

DESCRIPTION :
==============================================================================*/

/*****************************************************************************//**
 * Creates a field that computes an integration.
 * The seed element is set to the number given and the mapping calculated.
 * Sets the number of components to be the same as the <integrand> field.
 * The <integrand> is the value that is integrated over each element and the
 * <coordinate_field> is used to define the arc length differential for each
 * element. Currently only two gauss points are supported, a linear integration.
 * If <magnitude_coordinates> is false then the resulting field has the same
 * number of components as the <coordinate_field> and each component is the
 * integration with respect to each of the components, if <magnitude_components>
 * is true then the field will have a single component and the magnitude of the
 * <coordinate_field> derivatives are used to calculate arc lengths at each
 * gauss point.
 *
 * @param field_module  Region field module which will own new field.
 * @return Newly created field
 */
struct Computed_field *Computed_field_create_integration(
	struct Cmiss_field_module *field_module, Cmiss_mesh_id mesh,
	Cmiss_element_id seed_element, Computed_field *integrand,
	int magnitude_coordinates, Computed_field *coordinate_field);

/***************************************************************************//**
 * If the field is of type COMPUTED_FIELD_INTEGRATION, the arguments including
 * seed element used for the mapping are returned.
 * @param mesh_address  Pointer to mesh handle which must be null. On successful
 * return this will be an accessed handle to mesh.
 */
int Computed_field_get_type_integration(struct Computed_field *field,
	Cmiss_mesh_id *mesh_address, struct FE_element **seed_element,
	struct Computed_field **integrand, int *magnitude_coordinates,
	struct Computed_field **coordinate_field);

#endif /* !defined (COMPUTED_FIELD_INTEGRATION_H) */

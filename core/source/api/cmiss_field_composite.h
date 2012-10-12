/*******************************************************************************
FILE : cmiss_field_composite.h

LAST MODIFIED : 13 May 2008

DESCRIPTION :
The public interface to the Cmiss_fields that perform arithmetic operations.
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
 * Shane Blackett (shane at blackett.co.nz)
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
#ifndef __CMISS_FIELD_COMPOSITE_H__
#define __CMISS_FIELD_COMPOSITE_H__

#include "types/cmiss_field_id.h"
#include "types/cmiss_field_module_id.h"

#include "cmiss_shared_object.h"

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************//**
 * Creates a field with the single source field.  This field is useful
 * as a placeholder candidate for replacement with more complicated operations
 * later on.
 * Internally this a composite field.
 *
 * @param field_module  Region field module which will own new field.
 * @param source_field The field the values are copied from.
 * @return Newly created field
 */
ZINC_API Cmiss_field_id Cmiss_field_module_create_identity(Cmiss_field_module_id field_module,
	Cmiss_field_id source_field);

/*****************************************************************************//**
 * Creates a field with the single source field and extracts a single component
 * specified by the component_number.
 * Internally this a composite field.
 *
 * @param field_module  Region field module which will own new field.
 * @param source_field The field the component value is copied from.
 * @param component_index  The index for the component.  The first values index
 * is 0 and the last is (number_of_field_components - 1).
 * @return Newly created field
 */
ZINC_API Cmiss_field_id Cmiss_field_module_create_component(Cmiss_field_module_id field_module,
	Cmiss_field_id source_field, int component_index);

/*****************************************************************************//**
 * Creates a field which concatenates the components of all source fields, in
 * order, into a single vector.
 *
 * @param field_module  Region field module which will own new field.
 * @param number_of_source_fields  The number of source fields in the array.
 * @param source_fields  The array of fields to be concatenating together.
 * @return Newly created field
 */
ZINC_API Cmiss_field_id Cmiss_field_module_create_concatenate(Cmiss_field_module_id field_module,
	int number_of_source_fields, Cmiss_field_id *source_fields);

#ifdef __cplusplus
}
#endif

#endif /* __CMISS_FIELD_COMPOSITE_H__ */

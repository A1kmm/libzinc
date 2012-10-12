/***************************************************************************//**
 * FILE : cmiss_field_subobject_group.h
 *
 * Implements region sub object groups, e.g. node group, element group.
 * These group fields evaluate to 1 (true) at domain locations in the group, and
 * 0 elsewhere.
 */
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
 * Portions created by the Initial Developer are Copyright (C) 2010
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

#if !defined (CMISS_FIELD_SUBOBJECT_GROUP_H)
#define CMISS_FIELD_SUBOBJECT_GROUP_H

#include "types/cmiss_c_inline.h"
#include "types/cmiss_field_id.h"
#include "types/cmiss_field_module_id.h"
#include "types/cmiss_element_id.h"
#include "types/cmiss_node_id.h"
#include "types/cmiss_field_subobject_group_id.h"

#include "cmiss_shared_object.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************//**
 * Creates a node group field which packages a Cmiss_nodeset_group i.e. a subset
 * of nodes from a master nodeset. As a field it evaluates to 1 for nodes in
 * the nodeset group and 0 elsewhere, i.e. it is the predicate for the sub-
 * domain, and this Boolean value can be combined in logical operations with
 * other fields.
 *
 * @param field_module  Region field module which will own new field.
 * @param mesh  Handle to a nodeset the node group is to be compatible with. If
 * it is not a master nodeset, the master is obtained from it.
 * Nodeset must be from the same region as field_module.
 * @return  Newly created field, or NULL if failed.
 */
ZINC_API Cmiss_field_id Cmiss_field_module_create_node_group(
	Cmiss_field_module_id field_module, Cmiss_nodeset_id nodeset);

/***************************************************************************//**
 * If field can be cast to a Cmiss_field_node_group_id do so
 * and return the field.  Otherwise return NULL.
 * Caller is responsible for destroying the new reference.
 *
 * @param field  handle to the field to cast
 * @return  handle of the cast field, or NULL
*/
ZINC_API Cmiss_field_node_group_id Cmiss_field_cast_node_group(Cmiss_field_id field);

/***************************************************************************//**
 * Cast node group field back to its base field and return the field.  Otherwise
 * return NULL.
 *
 * @param group  handle to the node group field to cast
 * @return  handle of the field, or NULL
 */
CMISS_C_INLINE Cmiss_field_id Cmiss_field_node_group_base_cast(Cmiss_field_node_group_id group)
{
	return (Cmiss_field_id)(group);
}

/***************************************************************************//**
 * Destroy the reference to the node group.
 *
 * @param group_address  address to the handle to the node group field
 * @return  Status CMISS_OK if successfully destroy the node group,
 * any other value on failure.
 */
ZINC_API int Cmiss_field_node_group_destroy(Cmiss_field_node_group_id *node_group_address);

/***************************************************************************//**
 * Get a handle to the 'dual' nodeset group of this node group, which provides
 * most of the methods for working with it.
 *
 * @param node_group  Handle to node group field.
 * @return  Handle to nodeset group. Caller is responsible for destroying this.
 */
ZINC_API Cmiss_nodeset_group_id Cmiss_field_node_group_get_nodeset(
	Cmiss_field_node_group_id node_group);

/***************************************************************************//**
 * Creates an element group field which packages a Cmiss_mesh_group i.e. a
 * subset of elements from a master mesh. As a field it evaluates to 1 for
 * elements in the mesh group and 0 elsewhere, i.e. it is the predicate for the
 * sub-domain, and this Boolean value can be combined in logical operations with
 * other fields.
 *
 * @param field_module  Region field module which will own new field.
 * @param mesh  Handle to a finite element mesh the element group is to be
 * compatible with. If it is not a master mesh, the master is obtained from it.
 * Mesh must be from the same region as field_module.
 * @return  Newly created field, or NULL if failed.
 */
ZINC_API Cmiss_field_id Cmiss_field_module_create_element_group(
	Cmiss_field_module_id field_module, Cmiss_mesh_id mesh);

/***************************************************************************//**
 * If field can be cast to a Cmiss_field_element_group_id do so
 * and return the field.  Otherwise return NULL.
 * Caller is responsible for destroying the new reference.
 *
 * @param field  handle to the field to cast
 * @return  handle of the cast field, or NULL
*/
ZINC_API Cmiss_field_element_group_id Cmiss_field_cast_element_group(Cmiss_field_id field);

/***************************************************************************//**
 * Cast element group field back to its base field and return the field.  Otherwise
 * return NULL.
 *
 * @param group  handle to the element group field to cast
 * @return  handle of the field, or NULL
 */
CMISS_C_INLINE Cmiss_field_id Cmiss_field_element_group_base_cast(Cmiss_field_element_group_id group)
{
	return (Cmiss_field_id)(group);
}

/***************************************************************************//**
 * Destroy the reference to the element group.
 *
 * @param element_group_address  address to the handle to the element group field
 * @return  Status CMISS_OK if successfully destroy the element group,
 * any other value on failure.
 */
ZINC_API int Cmiss_field_element_group_destroy(Cmiss_field_element_group_id *element_group_address);

/***************************************************************************//**
 * Get a handle to the 'dual' mesh group of this element group, which provides
 * most of the methods for working with it.
 *
 * @param element_group  Handle to element group field.
 * @return  Handle to mesh group. Caller is responsible for destroying this.
 */
ZINC_API Cmiss_mesh_group_id Cmiss_field_element_group_get_mesh(
	Cmiss_field_element_group_id element_group);

#ifdef __cplusplus
}
#endif

#endif /* !defined (CMISS_FIELD_SUBOBJECT_GROUP_H) */

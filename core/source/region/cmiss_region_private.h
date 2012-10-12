/*******************************************************************************
FILE : cmiss_region_private.h

LAST MODIFIED : 1 October 2002

DESCRIPTION :
Private interface for attaching any object type to Cmiss_region objects.
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
#if !defined (CMISS_REGION_PRIVATE_H)
#define CMISS_REGION_PRIVATE_H

#include "general/any_object.h"
#include "region/cmiss_region.h"

/*
Global functions
----------------
*/

/***************************************************************************//**
 * Private function for adding field to region. Ensures the new field has a
 * unique cache_index.
 */
int Cmiss_region_add_field_private(Cmiss_region_id region, Cmiss_field_id field);

/***************************************************************************//**
 * Private function for clearing field value caches for field in all caches
 * listed in region.
 */
void Cmiss_region_clear_field_value_caches(Cmiss_region_id region, Cmiss_field_id field);

/***************************************************************************//**
 * Deaccesses fields from region and all child regions recursively.
 * Temporary until circular references sorted out - certain fields access
 * regions. Call ONLY before deaccessing root_region in Cmiss_context.
 */
void Cmiss_region_detach_fields_hierarchical(struct Cmiss_region *region);

int Cmiss_region_private_attach_any_object(struct Cmiss_region *region,
	struct Any_object *any_object);
/*******************************************************************************
LAST MODIFIED : 1 October 2002

DESCRIPTION :
Adds <any_object> to the list of objects attached to <region>.
This function is only externally visible to context objects.
==============================================================================*/

int Cmiss_region_private_detach_any_object(struct Cmiss_region *region,
	struct Any_object *any_object);
/*******************************************************************************
LAST MODIFIED : 1 October 2002

DESCRIPTION :
Removes <any_object> from the list of objects attached to <region>.
Note that only in the case that <any_object> is the exact Any_object stored in
<region> may it be cleaned up. In any other case the <any_object> passed in
must be cleaned up by the calling function.
This function is only externally visible to context objects.
==============================================================================*/

struct LIST(Any_object) *
Cmiss_region_private_get_any_object_list(struct Cmiss_region *region);
/*******************************************************************************
LAST MODIFIED : 1 October 2002

DESCRIPTION :
Returns the list of objects, abstractly stored as struct Any_object from
<region>. It is important that this list not be modified directly.
This function is only externally visible to context objects.
==============================================================================*/

#endif /* !defined (CMISS_REGION_PRIVATE_H) */

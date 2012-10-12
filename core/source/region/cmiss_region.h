/***************************************************************************//**
 * FILE : cmiss_region.h
 *
 * Definition of Cmiss_region, container of fields for representing model data,
 * and child regions for building hierarchical models.
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
#if !defined (CMISS_REGION_H)
#define CMISS_REGION_H

#include "api/cmiss_field_group.h"
#include "api/cmiss_region.h"
#include "computed_field/computed_field.h"
#include "general/callback.h"
#include "general/object.h"


/*
Global constants
----------------
*/

/* separator character for Cmiss_region in path strings */
#define CMISS_REGION_PATH_SEPARATOR_CHAR '/'
#define CMISS_REGION_PATH_SEPARATOR_STRING "/"

/*
Global types
------------
*/

struct Cmiss_region;
/*******************************************************************************
LAST MODIFIED : 30 September 2002

DESCRIPTION :
Object responsible for building directed acyclic graph hierarchies in Cmiss.
Implements hierarchies by contains a list of Cmiss_region_child objects, each
with their own name as seen by this parent.
Implements advanced hierarchical functionality through context objects stored
within it. Type and role of context objects are not known to the Cmiss_region.
==============================================================================*/

struct Cmiss_region_changes
/*******************************************************************************
LAST MODIFIED : 11 March 2010

DESCRIPTION :
Data broadcast with callbacks from <Cmiss_region> describing the changes.
==============================================================================*/
{
	/* true if the name of this region has changed */
	int name_changed;
	/* true if children added, removed or reordered in Cmiss_region */
	int children_changed;
	/* if a single child has been added (and none removed) it is indicated here */
	struct Cmiss_region *child_added;
	/* if a single child has been removed (and none added) it is indicated here */
	struct Cmiss_region *child_removed;
}; /* struct Cmiss_region_changes */

DECLARE_CMISS_CALLBACK_TYPES(Cmiss_region_change, \
	struct Cmiss_region *, struct Cmiss_region_changes *, void);

/*
Global functions
----------------
*/

PROTOTYPE_OBJECT_FUNCTIONS(Cmiss_region);

/***************************************************************************//**
 * Creates a Cmiss_region, able to have its own fields. This is an internal
 * function and should not be exposed to the API.
 *
 * @return  Accessed reference to the newly created region, or NULL if none.
 */
struct Cmiss_region *Cmiss_region_create_internal(void);

/***************************************************************************//**
 * Remove all nodes, elements, data and finite element fields from this region.
 *
 * @param region  The region to clear the fields from. Must not be a group.
 * @return  1 on success, 0 if no region supplied.
 */
int Cmiss_region_clear_finite_elements(struct Cmiss_region *region);

/***************************************************************************//**
 * Returns FE_region for this Cmiss_region.
 */
struct FE_region *Cmiss_region_get_FE_region(struct Cmiss_region *region);

/***************************************************************************//**
 * Returns the field manager for this region.
 */
struct MANAGER(Computed_field) *Cmiss_region_get_Computed_field_manager(
	struct Cmiss_region *region);

/***************************************************************************//**
 * Returns the size a field cache array needs to be to fit the assigned field
 * cache indexes.
 */
int Cmiss_region_get_field_cache_size(Cmiss_region_id region);

/***************************************************************************//**
 * Adds cache to the list of caches for this region. Region needs this list to
 * add new value caches for any fields created while the cache exists.
 */
void Cmiss_region_add_field_cache(Cmiss_region_id region, Cmiss_field_cache_id cache);

/***************************************************************************//**
 * Removes cache from the list of caches for this region.
 */
void Cmiss_region_remove_field_cache(Cmiss_region_id region,
	Cmiss_field_cache_id cache);

int Cmiss_region_begin_change(struct Cmiss_region *region);
/*******************************************************************************
LAST MODIFIED : 10 December 2002

DESCRIPTION :
Increments the change counter in <region>. No update callbacks will occur until
change counter is restored to zero by calls to Cmiss_region_end_change.
???RC Make recursive/hierarchical?
==============================================================================*/

int Cmiss_region_end_change(struct Cmiss_region *region);
/*******************************************************************************
LAST MODIFIED : 10 December 2002

DESCRIPTION :
Decrements the change counter in <region>. No update callbacks occur until the
change counter is restored to zero by this function.
???RC Make recursive/hierarchical?
==============================================================================*/

int Cmiss_region_add_callback(struct Cmiss_region *region,
	CMISS_CALLBACK_FUNCTION(Cmiss_region_change) *function, void *user_data);
/*******************************************************************************
LAST MODIFIED : 2 December 2002

DESCRIPTION :
Adds a callback to <region> so that when it changes <function> is called with
<user_data>. <function> has 3 arguments, a struct Cmiss_region *, a
struct Cmiss_region_changes * and the void *user_data.
==============================================================================*/

int Cmiss_region_remove_callback(struct Cmiss_region *region,
	CMISS_CALLBACK_FUNCTION(Cmiss_region_change) *function, void *user_data);
/*******************************************************************************
LAST MODIFIED : 2 December 2002

DESCRIPTION :
Removes the callback calling <function> with <user_data> from <region>.
==============================================================================*/

/***************************************************************************//**
 * Returns the name of the region.
 *
 * @param region  The region whose name is requested.
 * @return  On success: allocated string containing region name.
 */
char *Cmiss_region_get_name(struct Cmiss_region *region);

/***************************************************************************//**
 * Sets the name of the region.
 * A valid region name must start with an alphanumeric character, contain only
 * alphanumeric characters, spaces ' ', dots '.', colons ':' or underscores '_',
 * and may not finish with a space.
 * Fails if the new name is already in use by another region in the same parent.
 *
 * @param region  The region to be named.
 * @param name  The new name for the region.
 * @return  1 on success, 0 on failure.
 */
int Cmiss_region_set_name(struct Cmiss_region *region, const char *name);

/***************************************************************************//**
 * Allocates and returns the path to the root_region ("/").
 *
 * @return  Allocated string "/".
 */
char *Cmiss_region_get_root_region_path(void);

/***************************************************************************//**
 * Returns the full path name from the root region to this region. Path name
 * always begins and ends with the CMISS_REGION_PATH_SEPARATOR_CHAR '/'.
 *
 * @param region  The region whose path is requested.
 * @return  On success: allocated string containing full region path.
 */
char *Cmiss_region_get_path(struct Cmiss_region *region);

/***************************************************************************//**
 * Returns the relative path name to this region from other_region. Path name
 * always begins and ends with the CMISS_REGION_PATH_SEPARATOR_CHAR '/'.
 *
 * @param region  The region whose path is requested.
 * @param other_region  The region the path is relative to.
 * @return  On success: allocated string containing relative region path; on
 * failure: NULL, including case when region is not within other_region.
 */
char *Cmiss_region_get_relative_path(struct Cmiss_region *region,
	struct Cmiss_region *other_region);

/***************************************************************************//**
 * Returns a reference to the parent region of this region.
 *
 * @param region  The child region.
 * @return  Accessed reference to parent region, or NULL if none.
 */
struct Cmiss_region *Cmiss_region_get_parent(struct Cmiss_region *region);

/***************************************************************************//**
 * Returns a non-accessed pointer to parent region of this region, if any.
 * Internal only.
 *
 * @param region  The child region.
 * @return  Non-accessed reference to parent region, or NULL if none.
 */
struct Cmiss_region *Cmiss_region_get_parent_internal(struct Cmiss_region *region);

/***************************************************************************//**
 * Returns a reference to the first child region of this region.
 *
 * @param region  The region whose first child is requested.
 * @return  Accessed reference to first child region, or NULL if none.
 */
struct Cmiss_region *Cmiss_region_get_first_child(struct Cmiss_region *region);

/***************************************************************************//**
 * Returns a reference to this region's next sibling region.
 *
 * @param region  The region whose next sibling is requested.
 * @return  Accessed reference to next sibling region, or NULL if none.
 */
struct Cmiss_region *Cmiss_region_get_next_sibling(struct Cmiss_region *region);

/***************************************************************************//**
 * Returns a reference to this region's previous sibling region.
 *
 * @param region  The region whose previous sibling is requested.
 * @return  Accessed reference to previous sibling region, or NULL if none.
 */
struct Cmiss_region *Cmiss_region_get_previous_sibling(struct Cmiss_region *region);

/***************************************************************************//**
 * Replaces the region reference with a reference to its next sibling.
 * Convenient for iterating through a child list, equivalent to:
 * {
 *   struct Cmiss_region *temp = Cmiss_region_get_next_sibling(*region_address);
 *   Cmiss_region_destroy(region_address);
 *   *region_address = temp;
 * }
 *
 * @param region_address  The address of the region reference to replace.
 */
void Cmiss_region_reaccess_next_sibling(struct Cmiss_region **region_address);

/***************************************************************************//**
 * Adds new_child to the end of the list of child regions of this region.
 * If the new_child is already in the region tree, it is first removed.
 * Fails if new_child contains this region.
 * Fails if new_child is unnamed or the name is already used by another child of
 * this region.
 *
 * @param region  The intended parent region of new_child.
 * @param new_child  The child to add.
 * @return  1 on success, 0 on failure.
 */
int Cmiss_region_append_child(struct Cmiss_region *region,
	struct Cmiss_region *new_child);

/***************************************************************************//**
 * Inserts new_child before the existing ref_child in the list of child regions
 * of this region. If ref_child is NULL new_child is added at the end of the
 * list. If the new_child is already in the region tree, it is first removed.
 * Fails if new_child contains this region.
 * Fails if new_child is unnamed or the name is already used by another child of
 * this region.
 *
 * @param region  The intended parent region of new_child.
 * @param new_child  The child to append.
 * @return  1 on success, 0 on failure.
 */
int Cmiss_region_insert_child_before(struct Cmiss_region *region,
	struct Cmiss_region *new_child, struct Cmiss_region *ref_child);

/***************************************************************************//**
 * Removes old_child from the list of child regions of this region.
 * Fails if old_child is not a child of this region.
 *
 * @param region  The current parent region of old_child.
 * @param old_child  The child to remove.
 * @return  1 on success, 0 on failure.
 */
int Cmiss_region_remove_child(struct Cmiss_region *region,
	struct Cmiss_region *old_child);

/***************************************************************************//**
 * Returns true if region is or contains the subregion.
 *
 * @param region  The region being tested as container.
 * @param subregion  The region being tested for containment.
 * @return  1 if this region is or contains subregion, 0 if not.
 */
int Cmiss_region_contains_subregion(struct Cmiss_region *region,
	struct Cmiss_region *subregion);

/***************************************************************************//**
 * Returns a reference to the child region with supplied name, if any.
 *
 * @param region  The region to search.
 * @param name  The name of the child.
 * @return  Accessed reference to the named child, or NULL if no match.
 */
struct Cmiss_region *Cmiss_region_find_child_by_name(
	struct Cmiss_region *region, const char *name);

/***************************************************************************//**
 * Returns a reference to the subregion at the path relative to this region.
 * The format of the path string is CHILD_NAME/CHILD_NAME/...
 * i.e. forward slash characters '/' are used as parent/child name separators.
 * Single leading and trailing separator characters are ignored.
 * Hence, both name="" and name="/" find the region itself.
 *
 * @param region  The region to search.
 * @param path  The directory-style path to the subregion.
 * @return  Accessed reference to subregion, or NULL no match.
 */
struct Cmiss_region *Cmiss_region_find_subregion_at_path(
	struct Cmiss_region *region, const char *path);

/*******************************************************************************
 * Internal only. External API is Cmiss_field_module_find_field_by_name.
 * @return  Accessed handle to field of given name, or NULL if none.
 */
Cmiss_field_id Cmiss_region_find_field_by_name(Cmiss_region_id region,
	const char *field_name);

/***************************************************************************//**
 * Deprecated legacy version of Cmiss_region_find_subregion_at_path returning
 * non-ACCESSed region as final argument.
 *
 * @param region  The region to search.
 * @param path  The directory-style path to the subregion.
 * @param subregion_address  Address to put region at path. Set to NULL if no
 * region is identified.
 * @return  1 if region found, 0 otherwise.
 */
int Cmiss_region_get_region_from_path_deprecated(struct Cmiss_region *region,
	const char *path, struct Cmiss_region **subregion_address);

/***************************************************************************//**
 * Returns a reference to the root region of this region.
 *
 * @param region  The region.
 * @return  Accessed reference to root region, or NULL if none.
 */
struct Cmiss_region *Cmiss_region_get_root(struct Cmiss_region *region);

/***************************************************************************//**
 * Separates a region/path/name into the region plus region-path and remainder
 * string containing text from the first unrecognized child region name.
 *
 * Examples:
 * "fibres" or "/fibres" -> root_region, "" and "fibres" if fibres was not a
 *     child region of the root region
 * "heart/fibres" or "/heart/fibres" -> heart region, "heart" and "fibres" if
 *     heart region has no child region called fibres
 * "body/heart" -> heart region, "body/heart" and NULL name if root region
 *     contains body region contains heart region
 * "heart/bob/fred/" -> region heart, "heart" and "bob/fred" if region heart
 *     has no child region called bob
 *
 * @param root_region the starting region for path
 * @path string the input path
 * @param region_address on success, points to region partially matched by path
 * @param region_path_address on success, returns allocated string path to the
 *   returned region, stripped of leading and trailing region path separators
 * @param remainder_address on success, returns pointer to allocated remainder
 *   of path stripped of leading and trailing region path separators, or NULL
 *   if all of path was resolved
 * @return 1 on success, 0 on failure
 */
int Cmiss_region_get_partial_region_path(struct Cmiss_region *root_region,
	const char *path, struct Cmiss_region **region_address,
	char **region_path_address,	char **remainder_address);

int Cmiss_region_list(struct Cmiss_region *region,
	int indent, int indent_increment);
/*******************************************************************************
LAST MODIFIED : 5 March 2003

DESCRIPTION :
Lists the Cmiss_region hierarchy starting from <region>. Contents are listed
indented from the left margin by <indent> spaces; this is incremented by
<indent_increment> for each child level.
==============================================================================*/

/***************************************************************************//**
 * Call to confirm compatibility of fields and other object definitions in
 * source region tree versus those in global region. Call this before calling
 * Cmiss_region_merge.
 * @param target_region  Target / global region to merge into.
 * @param source_region  Source region to check compatibility of fields for.
 * @return  1 if compatible, 0 if not.
 */
int Cmiss_region_can_merge(Cmiss_region_id target_region, Cmiss_region_id source_region);

/***************************************************************************//**
 * Merge fields and other objects from source region tree into global region.
 * Source must be destroyed afterwards as some of its objects may be transfered
 * to global region.
 * @param target_region  Target / global region to merge into.
 * @param source_region  Source region to merge from.
 * @return  1 on success, 0 on failure.
 */
int Cmiss_region_merge(Cmiss_region_id target_region, Cmiss_region_id source_region);

#endif /* !defined (CMISS_REGION_H) */

/*****************************************************************************//**
 * FILE : computed_field_group.cpp
 * 
 * Implements a cmiss field which is an group for another field, commonly from a
 * different region to make it available locally.
 *
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
#include <stdlib.h>
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_group.h"
#include "computed_field/computed_field_group_base.hpp"
#include "computed_field/computed_field_subobject_group_private.hpp"
#include "computed_field/computed_field_private.hpp"
#include "zinc/fieldgroup.h"
#include "zinc/fieldsubobjectgroup.h"
#include "zinc/rendition.h"
#if defined (USE_OPENCASCADE)
#include "graphics/rendition.h"
#include "zinc/field_cad.h"
#endif /* defined (USE_OPENCASCADE) */
#include "finite_element/finite_element_region.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "region/cmiss_region.h"
#include "general/message.h"
#include "mesh/cmiss_node_private.hpp"
#include "mesh/cmiss_element_private.hpp"
#if defined (USE_OPENCASCADE)
#include "cad/field_location.hpp"
#endif /* defined (USE_OPENCASCADE) */
#include <list>
#include <map>

/*
Module types
------------
*/

struct Cmiss_field_hierarchical_group_change_detail : public Cmiss_field_group_base_change_detail
{
public:
	Cmiss_field_hierarchical_group_change_detail() :
		local_change(CMISS_FIELD_GROUP_NO_CHANGE),
		non_local_change(CMISS_FIELD_GROUP_NO_CHANGE)
	{
	}

	void clear()
	{
		local_change = CMISS_FIELD_GROUP_NO_CHANGE;
		non_local_change = CMISS_FIELD_GROUP_NO_CHANGE;
	}

	/** note: if returns CLEAR or REMOVE, must check sub-group is empty */
	Cmiss_field_group_change_type getChange() const
	{
		return merge(local_change, non_local_change);
	}

	Cmiss_field_group_change_type getLocalChange() const
	{
		return local_change;
	}

	Cmiss_field_group_change_type getNonLocalChange() const
	{
		return non_local_change;
	}

	/** Inform group has been cleared, but wasn't before */
	void changeClear()
	{
		local_change = CMISS_FIELD_GROUP_CLEAR;
		non_local_change = CMISS_FIELD_GROUP_CLEAR;
	}

	/** Inform local group has been added, but wasn't before */
	void changeAddLocal()
	{
		local_change = CMISS_FIELD_GROUP_ADD;
	}

	/** Inform local group has been cleared, but wasn't before */
	void changeClearLocal()
	{
		local_change = CMISS_FIELD_GROUP_CLEAR;
	}

	/** Inform local group has been replaced, but wasn't before */
	void changeReplaceLocal()
	{
		local_change = CMISS_FIELD_GROUP_REPLACE;
	}

	/** Inform local group has been cleared, but wasn't before */
	void changeClearNonLocal()
	{
		non_local_change = CMISS_FIELD_GROUP_CLEAR;
	}

	void changeMergeLocal(const Cmiss_field_group_base_change_detail *in_change_detail)
	{
		local_change = merge(local_change, in_change_detail->getChange());
	}

	void changeMergeNonLocal(const Cmiss_field_group_base_change_detail *in_change_detail)
	{
		non_local_change = merge(non_local_change, in_change_detail->getChange());
	}

private:
	Cmiss_field_group_change_type local_change;
	Cmiss_field_group_change_type non_local_change;

	/** Warning: Assumes group is not empty. Caller must check empty -> clear */
	static Cmiss_field_group_change_type merge(
		Cmiss_field_group_change_type change1,
		Cmiss_field_group_change_type change2)
	{
		Cmiss_field_group_change_type result = change1;
		if ((change2 != CMISS_FIELD_GROUP_NO_CHANGE) && (change2 != change1))
		{
			if (change2 == CMISS_FIELD_GROUP_CLEAR)
			{
				if (change1 == CMISS_FIELD_GROUP_NO_CHANGE)
				{
					result = CMISS_FIELD_GROUP_REMOVE;
				}
				else if (change1 == CMISS_FIELD_GROUP_ADD)
				{
					result = CMISS_FIELD_GROUP_REPLACE;
				}
			}
			else if (change1 == CMISS_FIELD_GROUP_NO_CHANGE)
			{
				result = change2;
			}
			else
			{
				result = CMISS_FIELD_GROUP_REPLACE;
			}
		}
		return result;
	}
};

class Computed_field_group_package : public Computed_field_type_package
{
public:
	Cmiss_region *root_region;

	Computed_field_group_package(Cmiss_region *root_region)
	  : root_region(root_region)
	{
		ACCESS(Cmiss_region)(root_region);
	}
	
	~Computed_field_group_package()
	{
		DEACCESS(Cmiss_region)(&root_region);
	}
};

typedef std::map<Cmiss_region_id, Cmiss_field_group_id> Region_field_map;
typedef std::map<Cmiss_region_id, Cmiss_field_group_id>::iterator Region_field_map_iterator;
typedef std::map<Cmiss_region_id, Cmiss_field_group_id>::const_iterator Region_field_map_const_iterator;

namespace {

char computed_field_group_type_string[] = "group";

class Computed_field_group : public Computed_field_group_base
{
private:
	Cmiss_field_hierarchical_group_change_detail change_detail;
	Cmiss_region *region;
	int contains_all;
	Computed_field *local_node_group, *local_data_group, *local_element_group[MAXIMUM_ELEMENT_XI_DIMENSIONS];

public:
	std::map<Computed_field *, Computed_field *> domain_selection_group;
	Region_field_map subregion_group_map;

	Computed_field_group(Cmiss_region *region)
		: Computed_field_group_base()
		, region(region)
		, contains_all(0)
		, local_node_group(NULL)
		, local_data_group(NULL)
		, subregion_group_map()
	{		//ACCESS(Cmiss_region)(region);
		for (int i = 0; i < MAXIMUM_ELEMENT_XI_DIMENSIONS; i++)
			local_element_group[i] = NULL;
	}

	~Computed_field_group()
	{
		if (local_node_group)
			Cmiss_field_destroy(&local_node_group);
		if (local_data_group)
			Cmiss_field_destroy(&local_data_group);
		for (int i = 0; i < MAXIMUM_ELEMENT_XI_DIMENSIONS; i++)
		{
			if (local_element_group[i])
				Cmiss_field_destroy(&local_element_group[i]);
		}
		for (Region_field_map_iterator iter = subregion_group_map.begin();
			iter != subregion_group_map.end(); ++iter)
		{
			Cmiss_field_group_id subregion_group_field = iter->second;
				Cmiss_field_group_destroy(&subregion_group_field);
		}
		std::map<Computed_field *, Computed_field *>::iterator it = domain_selection_group.begin();
		for (;it != domain_selection_group.end(); it++)
		{
			//Cmiss_field_destroy(&(it->first)); don't destroy this it is not a reference just a key
			Cmiss_field_destroy(&(it->second));
		}
	}

	/** @return allocated name for node group for master_nodeset */
	char *get_standard_node_group_name(Cmiss_nodeset_id master_nodeset);

	Cmiss_field_node_group_id create_node_group(Cmiss_nodeset_id nodeset);

	Cmiss_field_node_group_id get_node_group(Cmiss_nodeset_id nodeset);

	/** @return allocated name for element group for master_mesh */
	char *get_standard_element_group_name(Cmiss_mesh_id master_mesh);

	Cmiss_field_element_group_id create_element_group(Cmiss_mesh_id mesh);

	Cmiss_field_element_group_id get_element_group(Cmiss_mesh_id mesh);

	Cmiss_field_id get_subobject_group_for_domain(Cmiss_field_id domain);

#if defined (USE_OPENCASCADE)
	Cmiss_field_id create_cad_primitive_group(Cmiss_field_cad_topology_id cad_topology_domain);

	int clear_region_tree_cad_primitive();
#endif /*defined (USE_OPENCASCADE) */

	Cmiss_field_group_id getSubRegionGroup(Cmiss_region_id subregion);

	Cmiss_field_group_id createSubRegionGroup(Cmiss_region_id subregion);

	Cmiss_field_group_id getFirstNonEmptyGroup();

	int clear_region_tree_node(int use_data);

	int clear_region_tree_element();

	int for_each_group_hiearchical(Cmiss_field_group_iterator_function function, void *user_data);

	int remove_empty_subgroups();

	virtual Cmiss_field_change_detail *extract_change_detail()
	{
		if (change_detail.getChange() == CMISS_FIELD_GROUP_NO_CHANGE)
			return NULL;
		Cmiss_field_hierarchical_group_change_detail *prior_change_detail =
			new Cmiss_field_hierarchical_group_change_detail();
		*prior_change_detail = change_detail;
#ifdef DEBUG_CODE
		{
			Cmiss_region *region = Computed_field_get_region(field);
			char *path = Cmiss_region_get_path(region);
			display_message(INFORMATION_MESSAGE, "Group %s%s change local %d non-local %d\n", path, field->name,
				prior_change_detail->getLocalChange(), prior_change_detail->getNonLocalChange());
			DEALLOCATE(path);
		}
#endif // DEBUG_CODE
		change_detail.clear();
		return prior_change_detail;
	}

	virtual int check_dependency();

	virtual void propagate_hierarchical_field_changes(MANAGER_MESSAGE(Computed_field) *message);

	int isEmptyLocal() const;

	virtual int isEmpty() const
	{
		return (isEmptyLocal() && isEmptyNonLocal());
	}

	virtual int clear();

	int clearLocal();

	int addLocalRegion();

	int containsLocalRegion();

	int addRegion(struct Cmiss_region *child_region);

	int removeRegion(struct Cmiss_region *region);

	int containsRegion(struct Cmiss_region *region);

private:

	Computed_field_core* copy()
	{
		Computed_field_group *core = new Computed_field_group(region);
		core->contains_all = this->contains_all;
		return (core);
	};

	const char* get_type_string()
	{
		return (computed_field_group_type_string);
	}

	void remove_child_group(struct Cmiss_region *child_region);

	int compare(Computed_field_core* other_field);

	int evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache);

	int list();

	Cmiss_field_id get_element_group_field_private(int dimension)
	{
		if ((dimension > 0) && (dimension <= MAXIMUM_ELEMENT_XI_DIMENSIONS))
			return local_element_group[dimension - 1];
		return 0;
	}

	int getSubgroupLocal();

	int add_region_tree(struct Cmiss_region *region_tree);

	int remove_region(struct Cmiss_region *child_region);

	int remove_region_tree(struct Cmiss_region *child_region);

	int contain_region_tree(struct Cmiss_region *child_region);

	inline int isSubGroupEmpty(Computed_field_core *source_core) const
	{
		Computed_field_group_base *group_base = dynamic_cast<Computed_field_group_base *>(source_core);
		if (group_base)
		{
			return group_base->isEmpty();
		}
		display_message(ERROR_MESSAGE,
			"Computed_field_group::isSubGroupEmpty.  Subgroup not derived from Computed_field_group_base");
		return 0;
	}

	int isEmptyNonLocal() const;

	int check_subobject_group_dependency(Computed_field_core *source_core);

};

inline Computed_field *Computed_field_cast(
	Cmiss_field_group *group_field)
{
	return (reinterpret_cast<Computed_field*>(group_field));
}

inline Computed_field_group *Computed_field_group_core_cast(
	Cmiss_field_group *group_field)
{
	return (static_cast<Computed_field_group*>(
		reinterpret_cast<Computed_field*>(group_field)->core));
}

/***************************************************************************//**
 * Compare the type specific data.
 */
int Computed_field_group::compare(Computed_field_core *other_core)
{
	int return_code;

	ENTER(Computed_field_group::compare);
	if (field && dynamic_cast<Computed_field_group*>(other_core))
	{
		return_code = 1;
	}
	else
	{
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_group::compare */

/***************************************************************************//**
 * Evaluates to 1 if domain location is in group, otherwise 0.
 */
int Computed_field_group::evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache)
{
	RealFieldValueCache &valueCache = RealFieldValueCache::cast(inValueCache);
	valueCache.values[0] = 0.0;
	if (contains_all)
	{
		valueCache.values[0] = 1;
	}
#if defined (USE_OPENCASCADE)
	else if (dynamic_cast<Field_cad_geometry_location*>(location))
	{
		printf("=== Cad geometry field location\n");
	}
#endif /* defined (USE_OPENCASCADE) */
	else
	{
		Field_element_xi_location *element_xi_location;
		if (dynamic_cast<Field_node_location*>(cache.getLocation()))
		{
			if (local_node_group)
			{
				RealFieldValueCache *sourceCache = RealFieldValueCache::cast(local_node_group->evaluate(cache));
				if (sourceCache)
				{
					valueCache.values[0] = sourceCache->values[0];
				}
			}
			if (local_data_group && (0.0 == valueCache.values[0]))
			{
				RealFieldValueCache *sourceCache = RealFieldValueCache::cast(local_data_group->evaluate(cache));
				if (sourceCache)
				{
					valueCache.values[0] = sourceCache->values[0];
				}
			}
		}
		else if (0 != (element_xi_location = dynamic_cast<Field_element_xi_location*>(cache.getLocation())))
		{
			int dimension = element_xi_location->get_dimension();
			Cmiss_field_id subobject_group_field = get_element_group_field_private(dimension);
			if (subobject_group_field)
			{
				RealFieldValueCache *sourceCache = RealFieldValueCache::cast(subobject_group_field->evaluate(cache));
				if (sourceCache)
				{
					valueCache.values[0] = sourceCache->values[0];
				}
			}
		}
	}
	return 1;
}

int Computed_field_group::isEmptyLocal() const
{
	if (contains_all)
	{
		return 0;
	}
	if (local_node_group && !isSubGroupEmpty(local_node_group->core))
		return 0;
	if (local_data_group && !isSubGroupEmpty(local_data_group->core))
		return 0;
	for (int i = 0; i < MAXIMUM_ELEMENT_XI_DIMENSIONS; i++)
	{
		if (local_element_group[i] && !isSubGroupEmpty(local_element_group[i]->core))
			return 0;
	}
	std::map<Computed_field *, Computed_field *>::const_iterator it = domain_selection_group.begin();
	while (it != domain_selection_group.end())
	{
		Computed_field *subobject_group_field = it->second;
		if (!isSubGroupEmpty(subobject_group_field->core))
			return 0;
		++it;
	}
	return 1;
}

int Computed_field_group::isEmptyNonLocal() const
{
	for (Region_field_map_const_iterator iter = subregion_group_map.begin();
		iter != subregion_group_map.end(); iter++)
	{
		Cmiss_field_group_id subregion_group_field = iter->second;
		if (!isSubGroupEmpty(Cmiss_field_group_base_cast(subregion_group_field)->core))
			return 0;
	}
	return 1;
}

int Computed_field_group::clearLocal()
{
	int return_code = 1;
	Cmiss_field_module_id field_module = Cmiss_region_get_field_module(region);
	Cmiss_field_module_begin_change(field_module);
	contains_all = 0;
	if (local_node_group)
	{
		Computed_field_group_base *group_base = dynamic_cast<Computed_field_group_base *>(local_node_group->core);
		return_code = group_base->clear();
		Cmiss_field_destroy(&local_node_group);
	}
	if (local_data_group)
	{
		Computed_field_group_base *group_base = dynamic_cast<Computed_field_group_base *>(local_data_group->core);
		return_code = group_base->clear();
		Cmiss_field_destroy(&local_data_group);
	}
	for (int i = 0; i < MAXIMUM_ELEMENT_XI_DIMENSIONS; i++)
	{
		if (local_element_group[i])
		{
			Computed_field_group_base *group_base = dynamic_cast<Computed_field_group_base *>(local_element_group[i]->core);
			return_code = group_base->clear();
			Cmiss_field_destroy(&local_element_group[i]);
		}
	}
	change_detail.changeClearLocal();
	Computed_field_changed(this->field);
	Cmiss_field_module_end_change(field_module);
	Cmiss_field_module_destroy(&field_module);
	return return_code;
};

int Computed_field_group::clear()
{
	int return_code = 0;
	Cmiss_field_module_id field_module = Cmiss_region_get_field_module(region);
	Cmiss_field_module_begin_change(field_module);
	for (Region_field_map_iterator iter = subregion_group_map.begin();
		iter != subregion_group_map.end(); iter++)
	{
		Computed_field_group *group_core = Computed_field_group_core_cast(iter->second);
		group_core->clear();
	}
	return_code = clearLocal();
	Computed_field_changed(this->field);
	Cmiss_field_module_end_change(field_module);
	Cmiss_field_module_destroy(&field_module);
	return return_code;
};

int Computed_field_group::check_subobject_group_dependency(Computed_field_core *source_core)
{
	Computed_field_subobject_group *subobject_group = dynamic_cast<Computed_field_subobject_group *>(source_core);
	/* check_dependency method is not sufficient to determine a subobject group has changed or not for a group */
	if (subobject_group->check_dependency_for_group_special())
	{
		Computed_field_dependency_change_private(field);
		const Cmiss_field_subobject_group_change_detail *subobject_group_change_detail =
			dynamic_cast<const Cmiss_field_subobject_group_change_detail *>(source_core->get_change_detail());
		if (subobject_group_change_detail)
		{
			if ((subobject_group_change_detail->getChange() == CMISS_FIELD_GROUP_CLEAR) &&
				isEmptyLocal())
			{
				change_detail.changeClearLocal();
			}
			else
			{
				change_detail.changeMergeLocal(subobject_group_change_detail);
			}
		}
	}
	return 1;
}

int Computed_field_group::check_dependency()
{
	if (field)
	{
		if (local_node_group)
			check_subobject_group_dependency(local_node_group->core);
		if (local_data_group)
			check_subobject_group_dependency(local_data_group->core);
		for (int i = 0; i < MAXIMUM_ELEMENT_XI_DIMENSIONS; i++)
		{
			if (local_element_group[i])
			{
				check_subobject_group_dependency(local_element_group[i]->core);
			}
		}
		std::map<Computed_field *, Computed_field *>::const_iterator it = domain_selection_group.begin();
		while (it != domain_selection_group.end())
		{
			Computed_field *subobject_group_field = it->second;
			check_subobject_group_dependency(subobject_group_field->core);
			++it;
		}
		return (field->manager_change_status & MANAGER_CHANGE_RESULT(Computed_field));
	}
	return 0;
}

void Computed_field_group::propagate_hierarchical_field_changes(
	MANAGER_MESSAGE(Computed_field) *message)
{
	if (message)
	{
		for (Region_field_map_iterator iter = subregion_group_map.begin();
			iter != subregion_group_map.end(); iter++)
		{
			Cmiss_field_group_id subregion_group = iter->second;
			// future optimisation: check subfield is from changed region
			const Cmiss_field_change_detail *source_change_detail = NULL;
			int change = Computed_field_manager_message_get_object_change_and_detail(
				message, Cmiss_field_group_base_cast(subregion_group), &source_change_detail);
			if (change != MANAGER_CHANGE_NONE(Computed_field))
			{
				if (source_change_detail)
				{
					const Cmiss_field_group_base_change_detail *subregion_group_change_detail =
						dynamic_cast<const Cmiss_field_group_base_change_detail *>(source_change_detail);
					if (subregion_group_change_detail)
					{
						Cmiss_field_group_change_type subregion_group_change =
							subregion_group_change_detail->getChange();
						if (subregion_group_change != CMISS_FIELD_GROUP_NO_CHANGE)
						{
							if (((subregion_group_change == CMISS_FIELD_GROUP_CLEAR) ||
								(subregion_group_change == CMISS_FIELD_GROUP_REMOVE)) &&
								isEmptyNonLocal())
							{
								change_detail.changeClearNonLocal();
							}
							else
							{
								change_detail.changeMergeNonLocal(subregion_group_change_detail);
							}
							Computed_field_dependency_changed(field);
						}
					}
					else
					{
						display_message(WARNING_MESSAGE, "Sub-region group changes could not be propagated.");
					}
				}
				// we have found only possible subgroup for sub-region:
				break;
			}
		}
	}
}

/***************************************************************************//**
 * Writes type-specific details of the field to the console. 
 */
int Computed_field_group::list()
{
	int return_code;
	
	ENTER(List_Computed_field_group);
	if (field)
	{
		display_message(INFORMATION_MESSAGE, "    Region : ");
		if (region)
		{
			char *path = Cmiss_region_get_path(region);
			display_message(INFORMATION_MESSAGE, "%s", path);
			DEALLOCATE(path);
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_group.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_group */

int Computed_field_group::removeRegion(Cmiss_region_id region)
{
	int return_code = 0;
	Cmiss_field_module_id field_module = Cmiss_region_get_field_module(region);
	Cmiss_field_module_begin_change(field_module);
	Cmiss_field_group_id subgroup = getSubRegionGroup(region);
	if (subgroup)
	{
		Computed_field_group *group_core = Computed_field_group_core_cast(subgroup);
		if (group_core->contains_all)
		{
			group_core->contains_all = 0;
			group_core->change_detail.changeClearLocal();
		}
		Computed_field_changed(Cmiss_field_group_base_cast(subgroup));
		Cmiss_field_group_destroy(&subgroup);
	}
	Computed_field_changed(this->field);
	Cmiss_field_module_end_change(field_module);
	Cmiss_field_module_destroy(&field_module);
	return return_code;
}

int Computed_field_group::containsRegion(Cmiss_region_id region)
{
	int return_code = 0;
	Cmiss_field_group_id subgroup = getSubRegionGroup(region);
	if (subgroup)
	{
		Computed_field_group *group_core = Computed_field_group_core_cast(subgroup);
		return_code = group_core->containsLocalRegion();
		Cmiss_field_group_destroy(&subgroup);
	}
	return return_code;
}

Cmiss_field_group_id Computed_field_group::getSubRegionGroup(Cmiss_region_id subregion)
{
	Cmiss_field_group_id subregion_group = NULL;
	if (region == subregion)
	{
		subregion_group = Cmiss_field_cast_group(this->getField());
	}
	else
	{
		Region_field_map_iterator iter = subregion_group_map.find(subregion);
		if (iter != subregion_group_map.end())
		{
			subregion_group = iter->second;
			ACCESS(Computed_field)(Cmiss_field_group_base_cast(subregion_group));
		}
		if (!subregion_group && !subregion_group_map.empty())
		{
			for (iter = subregion_group_map.begin(); iter != subregion_group_map.end(); iter++)
			{
				Cmiss_field_group_id temp = iter->second;
				Computed_field_group *group_core = Computed_field_group_core_cast(temp);
				subregion_group = group_core->getSubRegionGroup(subregion);
				if (subregion_group)
					break;
			}
		}
	}
	return subregion_group;
}

Cmiss_field_group_id Computed_field_group::createSubRegionGroup(Cmiss_region_id subregion)
{
	Cmiss_field_group_id subregion_group = NULL;
	if (Cmiss_region_contains_subregion(region, subregion) && region != subregion)
	{
		Cmiss_region_id parent_region = Cmiss_region_get_parent_internal(subregion);
		if (parent_region != region)
		{
			Cmiss_field_group_id temp = getSubRegionGroup(subregion);
			if (!temp)
			{
				/* this will construct the hierarchy tree */
				temp = getSubRegionGroup(parent_region);
				if (!temp)
					temp = createSubRegionGroup(parent_region);
				if (temp)
				{
					Computed_field_group *group_core = Computed_field_group_core_cast(temp);
					subregion_group = group_core->createSubRegionGroup(subregion);
				}
			}
			if (temp)
				Cmiss_field_group_destroy(&temp);
		}
		else // (parent_region == region)
		{
			Region_field_map_iterator pos = subregion_group_map.find(subregion);
			if (pos == subregion_group_map.end())
			{
				Cmiss_field_module_id field_module =
					Cmiss_region_get_field_module(subregion);
				Cmiss_field_id generic_field =
					Cmiss_field_module_find_field_by_name(field_module, this->getField()->name);
				if (generic_field)
				{
					subregion_group = Cmiss_field_cast_group(generic_field);
					// Not calling Cmiss_field_set_attribute_integer(subregion_group, CMISS_FIELD_ATTRIBUTE_IS_MANAGED, 0);
					Cmiss_field_destroy(&generic_field);
				}
				if (!subregion_group)
				{
					Cmiss_field_module_set_field_name(field_module, this->getField()->name);
					subregion_group = reinterpret_cast<Cmiss_field_group_id>(Cmiss_field_module_create_group(field_module));
				}
				Cmiss_field_module_destroy(&field_module);
				ACCESS(Computed_field)(Cmiss_field_group_base_cast(subregion_group));
				subregion_group_map.insert(std::make_pair(subregion, subregion_group));
			}
			// else already exists: fail
		}
	}
	return subregion_group;
}

Cmiss_field_group_id Computed_field_group::getFirstNonEmptyGroup()
{
	if (!isEmptyLocal())
	{
		return Cmiss_field_cast_group(this->getField());
	}
	if (!subregion_group_map.empty())
	{
		Cmiss_field_group_id subregion_group = 0;
		Region_field_map_iterator iter;
		for (Region_field_map_iterator iter = subregion_group_map.begin();
			iter != subregion_group_map.end(); iter++)
		{
			Cmiss_field_group_id temp = iter->second;
			Computed_field_group *group_core = Computed_field_group_core_cast(temp);
			subregion_group = group_core->getFirstNonEmptyGroup();
			if (subregion_group)
				return subregion_group;
		}
	}
	return 0;
}

char *Computed_field_group::get_standard_node_group_name(Cmiss_nodeset_id master_nodeset)
{
	char *name = Cmiss_field_get_name(this->getField());
	int error = 0;
	append_string(&name, ".", &error);
	char *nodeset_name = Cmiss_nodeset_get_name(master_nodeset);
	append_string(&name, nodeset_name, &error);
	DEALLOCATE(nodeset_name);
	return name;
}

Cmiss_field_node_group_id Computed_field_group::create_node_group(Cmiss_nodeset_id nodeset)
{
	if (contains_all)
		return 0;
	if (Cmiss_nodeset_get_master_region_internal(nodeset) != region)
		return 0;
	Cmiss_field_node_group_id node_group = get_node_group(nodeset);
	if (node_group)
	{
		// can't create if already exists
		Cmiss_field_node_group_destroy(&node_group);
	}
	else
	{
		Cmiss_nodeset_id master_nodeset = Cmiss_nodeset_get_master(nodeset);
		Cmiss_field_module_id field_module = Cmiss_region_get_field_module(region);
		Cmiss_field_module_begin_change(field_module);
		Cmiss_field_id node_group_field = Cmiss_field_module_create_node_group(field_module, master_nodeset);
		node_group = Cmiss_field_cast_node_group(node_group_field);
		char *name = get_standard_node_group_name(master_nodeset);
		Cmiss_field_set_name(node_group_field, name);
		DEALLOCATE(name);
		int use_data = Cmiss_nodeset_is_data_internal(master_nodeset);
		if (use_data)
		{
			local_data_group = Cmiss_field_access(node_group_field);
		}
		else
		{
			local_node_group = Cmiss_field_access(node_group_field);
		}
		Cmiss_field_destroy(&node_group_field);
		Cmiss_field_module_end_change(field_module);
		Cmiss_field_module_destroy(&field_module);
		Cmiss_nodeset_destroy(&master_nodeset);
	}
	return (node_group);
}

Cmiss_field_node_group_id Computed_field_group::get_node_group(Cmiss_nodeset_id nodeset)
{
	if (contains_all)
		return 0;
	if (Cmiss_nodeset_get_master_region_internal(nodeset) != region)
		return 0;
	Cmiss_field_node_group_id node_group = NULL;
	int use_data = Cmiss_nodeset_is_data_internal(nodeset);
	if (!use_data && local_node_group)
	{
		node_group = Cmiss_field_cast_node_group(local_node_group);
	}
	else if (use_data && local_data_group)
	{
		node_group = Cmiss_field_cast_node_group(local_data_group);
	}
	if (!node_group)
	{
		// find by name & check it is for same master nodeset (must avoid group regions)
		Cmiss_nodeset_id master_nodeset = Cmiss_nodeset_get_master(nodeset);
		Cmiss_field_module_id field_module = Cmiss_region_get_field_module(region);
		char *name = get_standard_node_group_name(master_nodeset);
		Cmiss_field_id node_group_field = Cmiss_field_module_find_field_by_name(field_module, name);
		DEALLOCATE(name);
		node_group = Cmiss_field_cast_node_group(node_group_field);
		if (node_group)
		{
			if (Cmiss_nodeset_match(master_nodeset,
				Computed_field_node_group_core_cast(node_group)->getMasterNodeset()))
			{
				if (use_data)
				{
					local_data_group = Cmiss_field_access(node_group_field);
				}
				else
				{
					local_node_group = Cmiss_field_access(node_group_field);
				}
			}
			else
			{
				// wrong nodeset
				Cmiss_field_node_group_destroy(&node_group);
			}
		}
		Cmiss_field_destroy(&node_group_field);
		Cmiss_field_module_destroy(&field_module);
		Cmiss_nodeset_destroy(&master_nodeset);
	}
	return (node_group);
}

char *Computed_field_group::get_standard_element_group_name(Cmiss_mesh_id master_mesh)
{
	char *name = Cmiss_field_get_name(this->getField());
	int error = 0;
	append_string(&name, ".", &error);
	char *mesh_name = Cmiss_mesh_get_name(master_mesh);
	append_string(&name, mesh_name, &error);
	DEALLOCATE(mesh_name);
	return name;
}

Cmiss_field_element_group_id Computed_field_group::create_element_group(Cmiss_mesh_id mesh)
{
	if (contains_all)
		return 0;
	if (Cmiss_mesh_get_master_region_internal(mesh) != region)
		return 0;
	Cmiss_field_element_group_id element_group = get_element_group(mesh);
	if (element_group)
	{
		// can't create if already exists
		Cmiss_field_element_group_destroy(&element_group);
	}
	else
	{
		Cmiss_mesh_id master_mesh = Cmiss_mesh_get_master(mesh);
		Cmiss_field_module_id field_module = Cmiss_region_get_field_module(region);
		Cmiss_field_module_begin_change(field_module);
		Cmiss_field_id element_group_field = Cmiss_field_module_create_element_group(field_module, master_mesh);
		element_group = Cmiss_field_cast_element_group(element_group_field);
		char *name = get_standard_element_group_name(master_mesh);
		Cmiss_field_set_name(element_group_field, name);
		DEALLOCATE(name);
		int dimension = Cmiss_mesh_get_dimension(mesh);
		local_element_group[dimension - 1] = Cmiss_field_access(element_group_field);
		Cmiss_field_destroy(&element_group_field);
		Cmiss_field_module_end_change(field_module);
		Cmiss_field_module_destroy(&field_module);
		Cmiss_mesh_destroy(&master_mesh);
	}
	return (element_group);
}

Cmiss_field_element_group_id Computed_field_group::get_element_group(Cmiss_mesh_id mesh)
{
	if (contains_all)
		return 0;
	if (Cmiss_mesh_get_master_region_internal(mesh) != region)
		return 0;
	Cmiss_field_element_group_id element_group = NULL;
	int dimension = Cmiss_mesh_get_dimension(mesh);
	if (local_element_group[dimension - 1])
	{
		element_group = Cmiss_field_cast_element_group(local_element_group[dimension - 1]);
	}
	else
	{
		// find by name & check it is for same master mesh (must avoid group regions)
		Cmiss_mesh_id master_mesh = Cmiss_mesh_get_master(mesh);
		Cmiss_field_module_id field_module = Cmiss_region_get_field_module(region);
		char *name = get_standard_element_group_name(master_mesh);
		Cmiss_field_id element_group_field = Cmiss_field_module_find_field_by_name(field_module, name);
		DEALLOCATE(name);
		element_group = Cmiss_field_cast_element_group(element_group_field);
		if (element_group)
		{
			if (Cmiss_mesh_match(master_mesh,
				Computed_field_element_group_core_cast(element_group)->getMasterMesh()))
			{
				local_element_group[dimension - 1] = Cmiss_field_access(element_group_field);
			}
			else
			{
				// wrong mesh
				Cmiss_field_element_group_destroy(&element_group);
			}
		}
		Cmiss_field_destroy(&element_group_field);
		Cmiss_field_module_destroy(&field_module);
		Cmiss_mesh_destroy(&master_mesh);
	}
	return (element_group);
}

Cmiss_field_id Computed_field_group::get_subobject_group_for_domain(Cmiss_field_id domain)
{
	Computed_field *field = NULL;
	std::map<Computed_field *, Computed_field *>::const_iterator it;
	it = domain_selection_group.find(domain);
	if (it != domain_selection_group.end())
	{
		field = it->second;
		Cmiss_field_access(field);
	}

	return field;
}

#if defined (USE_OPENCASCADE)
Cmiss_field_id Computed_field_group::create_cad_primitive_group(Cmiss_field_cad_topology_id cad_topology_domain)
{
	Computed_field *field = NULL;
	if (cad_topology_domain)
	{
		const char *base_name = "cad_primitive_selection";
		const char *domain_field_name = Cmiss_field_get_name(reinterpret_cast<Cmiss_field_id>(cad_topology_domain));
		char *field_name = NULL;
		int error = 0;
		ALLOCATE(field_name, char, strlen(base_name)+strlen(domain_field_name)+2);
		field_name[0] = '\0';
		append_string(&field_name, base_name, &error);
		append_string(&field_name, "_", &error);
		append_string(&field_name, domain_field_name, &error);

		Cmiss_field_module_id field_module =
			Cmiss_region_get_field_module(region);
		Cmiss_field_module_set_field_name(field_module, field_name);
		field = Cmiss_field_module_create_cad_primitive_group_template(field_module);
		Computed_field *cad_topology_key = reinterpret_cast<Cmiss_field_id>(cad_topology_domain);
		domain_selection_group.insert(std::pair<Computed_field *, Computed_field *>(cad_topology_key, field));

		Cmiss_field_module_destroy(&field_module);
		Cmiss_field_access(field);
		DEALLOCATE(field_name);
	}
	else
	{
		display_message(ERROR_MESSAGE, "Computed_field_group::create_cad_primitive_group.  Invalid arguments\n");
	}

	return (field);
}

int Computed_field_group::clear_region_tree_cad_primitive()
{
	Region_field_map_iterator pos;
	int return_code = 1;
	Cmiss_field_group_id group_field = NULL;
	std::map<Computed_field *, Computed_field *>::iterator it = domain_selection_group.begin();
	while (it != domain_selection_group.end())
	{
		Cmiss_field_cad_primitive_group_template_id cad_primitive_group =
			Cmiss_field_cast_cad_primitive_group_template(it->second);
		return_code = Cmiss_field_cad_primitive_group_template_clear(cad_primitive_group);
		Computed_field_changed(this->field);
		//Cmiss_field_id cad_primitive_group_field = reinterpret_cast<Computed_field*>(cad_primitive_group);
		Cmiss_field_cad_primitive_group_template_destroy(&cad_primitive_group);
		Cmiss_field_destroy(&it->second);
		domain_selection_group.erase(it++);
	}
	if (!subregion_group_map.empty())
	{
		for (pos = subregion_group_map.begin(); pos != subregion_group_map.end(); pos++)
		{
			group_field = pos->second;
			Cmiss_field_group_clear_region_tree_cad_primitive(group_field);
		}
	}

	return (return_code);
}

#endif /* defined (USE_OPENCASCADE) */


int Computed_field_group::addRegion(struct Cmiss_region *child_region)
{
	int return_code = 0;
	if (Cmiss_region_contains_subregion(region, child_region))
	{
		Cmiss_field_module_id field_module = Cmiss_region_get_field_module(child_region);
		Cmiss_field_module_begin_change(field_module);
		Cmiss_field_group_id subregion_group = getSubRegionGroup(child_region);
		if (!subregion_group)
			subregion_group = createSubRegionGroup(child_region);
		Computed_field_group *group_core =
			Computed_field_group_core_cast(subregion_group);
		group_core->addLocalRegion();
		Cmiss_field_group_destroy(&subregion_group);
		Computed_field_changed(this->field);
		Cmiss_field_module_end_change(field_module);
		Cmiss_field_module_destroy(&field_module);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_group::addRegion.  Sub region is not a child region"
			"or part of the parent region");
		return_code = 0;
	}

	return (return_code);
}

void Computed_field_group::remove_child_group(struct Cmiss_region *child_region)
{
	if (Cmiss_region_contains_subregion(region, child_region))
	{
		Region_field_map_iterator pos = subregion_group_map.find(child_region);
		if (pos != subregion_group_map.end())
		{
			Cmiss_region_id region = pos->first;
			if (region)
			{
				Cmiss_field_group_id temp = pos->second;
				subregion_group_map.erase(child_region);
				Cmiss_field_group_destroy(&temp);
			}
		}
	}
}

int Computed_field_group::remove_empty_subgroups()
{
	/* remove empty subobject groups */
	if (local_node_group)
	{
		Computed_field_group_base *group_base = static_cast<Computed_field_group_base *>(local_node_group->core);
		if (group_base->isEmpty())
		{
			Cmiss_field_destroy(&local_node_group);
		}
	}
	if (local_data_group)
	{
		Computed_field_group_base *group_base = static_cast<Computed_field_group_base *>(local_data_group->core);
		if (group_base->isEmpty())
		{
			Cmiss_field_destroy(&local_data_group);
		}
	}
	for (int i = 0; i < MAXIMUM_ELEMENT_XI_DIMENSIONS; i++)
	{
		if (local_element_group[i])
		{
			Computed_field_group_base *group_base = static_cast<Computed_field_group_base *>(local_element_group[i]->core);
			if (group_base->isEmpty())
			{
				Cmiss_field_destroy(&local_element_group[i]);
			}
		}
	}
	/* remove empty subregion group */
	for (Region_field_map_iterator iter = subregion_group_map.begin();
		iter != subregion_group_map.end();)
	{
		Cmiss_field_group_id subregion_group_field = iter->second;
		Computed_field_group *group_core = Computed_field_group_core_cast(subregion_group_field);
		group_core->remove_empty_subgroups();
		if (group_core->isEmpty())
		{
			subregion_group_map.erase(iter++);
			Cmiss_field_group_destroy(&subregion_group_field);
		}
		else
		{
			++iter;
		}
	}
	return 1;
}

int Computed_field_group::clear_region_tree_node(int use_data)
{
	Region_field_map_iterator pos;
	int return_code = 1;
	Cmiss_field_group_id group_field = NULL;
	if (!use_data && local_node_group)
	{
		Cmiss_field_node_group_id node_group = Cmiss_field_cast_node_group(local_node_group);
		Cmiss_nodeset_group_id nodeset_group = Cmiss_field_node_group_get_nodeset(node_group);
		return_code = Cmiss_nodeset_group_remove_all_nodes(nodeset_group);
		Cmiss_nodeset_group_destroy(&nodeset_group);
		check_subobject_group_dependency(local_node_group->core);
		Computed_field_changed(this->field);
		Cmiss_field_node_group_destroy(&node_group);
	}
	if (use_data && local_data_group)
	{
		Cmiss_field_node_group_id data_group = Cmiss_field_cast_node_group(local_data_group);
		Cmiss_nodeset_group_id nodeset_group = Cmiss_field_node_group_get_nodeset(data_group);
		return_code = Cmiss_nodeset_group_remove_all_nodes(nodeset_group);
		Cmiss_nodeset_group_destroy(&nodeset_group);
		check_subobject_group_dependency(local_data_group->core);
		Computed_field_changed(this->field);
		Cmiss_field_node_group_destroy(&data_group);
	}
	if (!subregion_group_map.empty())
	{
		for (pos = subregion_group_map.begin(); pos != subregion_group_map.end(); pos++)
		{
			group_field = pos->second;
			if (!use_data)
				Cmiss_field_group_clear_region_tree_node(group_field);
			else
				Cmiss_field_group_clear_region_tree_data(group_field);
		}
	}
	return (return_code);
}

int Computed_field_group::addLocalRegion()
{
	Cmiss_field_module_id field_module = Cmiss_region_get_field_module(region);
	Cmiss_field_module_begin_change(field_module);
	if (isEmpty())
		change_detail.changeAddLocal();
	else
		change_detail.changeReplaceLocal();
	clearLocal();
	contains_all = 1;
	Computed_field_changed(this->field);
	Cmiss_field_module_end_change(field_module);
	Cmiss_field_module_destroy(&field_module);
	return 1;
}

int Computed_field_group::containsLocalRegion()
{
	return contains_all;
}

int Computed_field_group::clear_region_tree_element()
{
	Region_field_map_iterator pos;
	int return_code = 1;
	for (int i = 0; i < MAXIMUM_ELEMENT_XI_DIMENSIONS; i++)
	{
		if (local_element_group[i])
		{
			Cmiss_field_element_group_id element_group =
				Cmiss_field_cast_element_group(local_element_group[i]);
			return_code = Computed_field_element_group_core_cast(element_group)->clear();
			check_subobject_group_dependency(local_element_group[i]->core);
			Computed_field_changed(this->field);
			Cmiss_field_element_group_destroy(&element_group);
		}
	}
	if (!subregion_group_map.empty())
	{
		for (pos = subregion_group_map.begin(); pos != subregion_group_map.end(); pos++)
		{
			Cmiss_field_group_clear_region_tree_element(pos->second);
		}
	}
	return (return_code);
}

int Computed_field_group::for_each_group_hiearchical(
	Cmiss_field_group_iterator_function function, void *user_data)
{
	int return_code = 0;
	if (field)
	{
		// ensure group is accessed while user function is called so not destroyed
		Cmiss_field_id access_field = Cmiss_field_access(field);
		return_code = function(reinterpret_cast<Cmiss_field_group_id>(field), user_data);
		Cmiss_field_destroy(&access_field);
		if (return_code)
		{
			for (Region_field_map_iterator child_group_iter = subregion_group_map.begin();
				child_group_iter !=subregion_group_map.end(); ++child_group_iter)
			{
				Computed_field_group *child_group_core = Computed_field_group_core_cast(child_group_iter->second);
				if ((!child_group_core) ||
					(!child_group_core->for_each_group_hiearchical(function, user_data)))
				{
					return_code = 0;
					break;
				}
			}
		}
	}
	return return_code;
}

} //namespace

Cmiss_field_group *Cmiss_field_cast_group(Cmiss_field_id field)
{

	if (field && dynamic_cast<Computed_field_group*>(field->core))
	{
		Cmiss_field_access(field);
		return (reinterpret_cast<Cmiss_field_group_id>(field));
	}
	else
	{
		return (NULL);
	}
}

Cmiss_field_node_group_id Cmiss_field_group_create_node_group(Cmiss_field_group_id group, Cmiss_nodeset_id nodeset)
{
	Cmiss_field_node_group_id field = NULL;
	if (group && nodeset)
	{
		Computed_field_group *group_core =
			Computed_field_group_core_cast(group);
		if (group_core)
		{
			field = group_core->create_node_group(nodeset);
		}
	}

	return field;
}

Cmiss_field_node_group_id Cmiss_field_group_get_node_group(Cmiss_field_group_id group, Cmiss_nodeset_id nodeset)
{
	Cmiss_field_node_group_id field = NULL;
	if (group && nodeset)
	{
		Computed_field_group *group_core =
			Computed_field_group_core_cast(group);
		if (group_core)
		{
			field = group_core->get_node_group(nodeset);
		}
	}

	return field;
}

Cmiss_field_element_group_id Cmiss_field_group_create_element_group(Cmiss_field_group_id group,
	Cmiss_mesh_id mesh)
{
	Cmiss_field_element_group_id field = NULL;
	if (group && mesh)
	{
		Computed_field_group *group_core =
			Computed_field_group_core_cast(group);
		if (group_core)
		{
			field = group_core->create_element_group(mesh);
		}
	}

	return field;
}

Cmiss_field_element_group_id Cmiss_field_group_get_element_group(Cmiss_field_group_id group,
	Cmiss_mesh_id mesh)
{
	Cmiss_field_element_group_id field = NULL;
	if (group && mesh)
	{
		Computed_field_group *group_core =
			Computed_field_group_core_cast(group);
		if (group_core)
		{
			field = group_core->get_element_group(mesh);
		}
	}

	return field;
}

#if defined (USE_OPENCASCADE)
Cmiss_field_cad_primitive_group_template_id Cmiss_field_group_create_cad_primitive_group(Cmiss_field_group_id group, Cmiss_field_cad_topology_id cad_topology_domain)
{
	Cmiss_field_cad_primitive_group_template_id cad_primitive_group = NULL;
	if (group)
	{
		Computed_field_group *group_core =
			Computed_field_group_core_cast(group);
		if (group_core)
		{
			Cmiss_field_id field = group_core->create_cad_primitive_group(cad_topology_domain);
			if (field != NULL)
			{
				cad_primitive_group = Cmiss_field_cast_cad_primitive_group_template(field);
				Cmiss_field_destroy(&field);
			}
		}
	}

	return cad_primitive_group;
}

Cmiss_field_cad_primitive_group_template_id Cmiss_field_group_get_cad_primitive_group(Cmiss_field_group_id group, Cmiss_field_cad_topology_id cad_topology_domain)
{
	Cmiss_field_cad_primitive_group_template_id cad_primitive_group = NULL;
	if (group)
	{
		Computed_field_group *group_core =
			Computed_field_group_core_cast(group);
		if (group_core)
		{
			Cmiss_field_id field = group_core->get_subobject_group_for_domain(reinterpret_cast< Computed_field * >(cad_topology_domain));//cad_primitive_group();
			if (field != NULL)
			{
				cad_primitive_group = Cmiss_field_cast_cad_primitive_group_template(field);
				Cmiss_field_destroy(&field);
			}
		}
	}

	return cad_primitive_group;
}

int Cmiss_field_group_clear_region_tree_cad_primitive(Cmiss_field_group_id group)
{
	int return_code = 0;
	if (group)
	{
		Computed_field_group *group_core =
			Computed_field_group_core_cast(group);
		if (group_core)
		{
			return_code = group_core->clear_region_tree_cad_primitive();
		}
	}
	return return_code;
}


#endif /* defined (USE_OPENCASCADE) */

Cmiss_field_group_id Cmiss_field_group_get_subregion_group(
	Cmiss_field_group_id group, Cmiss_region_id subregion)
{
	Cmiss_field_group_id subgroup = NULL;
	if (group)
	{
		Computed_field_group *group_core =
			Computed_field_group_core_cast(group);
		if (group_core)
		{
			subgroup = group_core->getSubRegionGroup(subregion);
		}
	}
	return subgroup;
}

Cmiss_field_group_id Cmiss_field_group_create_subregion_group(
	Cmiss_field_group_id group, Cmiss_region_id subregion)
{
	Cmiss_field_group_id subgroup = NULL;
	if (group)
	{
		Computed_field_group *group_core =
			Computed_field_group_core_cast(group);
		if (group_core)
		{
			subgroup = group_core->createSubRegionGroup(subregion);
		}
	}
	return subgroup;
}

int Cmiss_field_group_clear_region_tree_node(Cmiss_field_group_id group)
{
	int return_code = 0;
	if (group)
	{
		Computed_field_group *group_core =
			Computed_field_group_core_cast(group);
		if (group_core)
		{
			return_code = group_core->clear_region_tree_node(/*use_data*/0);
		}
	}
	return return_code;
}

int Cmiss_field_group_clear_region_tree_data(Cmiss_field_group_id group)
{
	int return_code = 0;
	if (group)
	{
		Computed_field_group *group_core =
			Computed_field_group_core_cast(group);
		if (group_core)
		{
			return_code = group_core->clear_region_tree_node(/*use_data*/1);
		}
	}
	return return_code;
}

int Cmiss_field_group_remove_empty_subgroups(Cmiss_field_group_id group)
{
	if (group)
	{
		Computed_field_group *group_core =
			Computed_field_group_core_cast(group);
		if (group_core)
		{
			return group_core->remove_empty_subgroups();
		}
	}
	return 0;
}

int Cmiss_field_group_clear_region_tree_element(Cmiss_field_group_id group)
{
	int return_code = 0;
	if (group)
	{
		Computed_field_group *group_core =
			Computed_field_group_core_cast(group);
		if (group_core)
		{
			return_code = group_core->clear_region_tree_element();
		}
	}
	return return_code;
}

Cmiss_field_id Cmiss_field_group_get_subobject_group_for_domain(Cmiss_field_group_id group, Cmiss_field_id domain)
{
	Computed_field *field = NULL;

	if (group)
	{
		Computed_field_group *group_core =
			Computed_field_group_core_cast(group);
		if (group_core)
		{
			field = group_core->get_subobject_group_for_domain(domain);
		}
	}

	return field;
}

int Cmiss_field_group_for_each_group_hierarchical(Cmiss_field_group_id group,
	Cmiss_field_group_iterator_function function, void *user_data)
{
	int return_code = 0;
	if (group && function)
	{
		Computed_field_group *group_core = Computed_field_group_core_cast(group);
		if (group_core)
		{
			return group_core->for_each_group_hiearchical(function, user_data);
		}
	}
	return return_code;
}

int Cmiss_field_group_destroy(Cmiss_field_group_id *group_address)
{
	return Cmiss_field_destroy(reinterpret_cast<Cmiss_field_id *>(group_address));
}

Computed_field *Cmiss_field_module_create_group(Cmiss_field_module_id field_module)
{
	Computed_field *field;

	ENTER(Computed_field_create_group);
	field = (Computed_field *)NULL;
	if (field_module)
	{
		Cmiss_region_id region = Cmiss_field_module_get_region(field_module);
		field = Computed_field_create_generic(field_module,
			/*check_source_field_regions*/false, 1,
			/*number_of_source_fields*/0, NULL,
			/*number_of_source_values*/0, NULL,
			new Computed_field_group(region));
		Cmiss_region_destroy(&region);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_field_module_create_group.  Invalid argument(s)");
	}
	LEAVE;

	return (field);
} /* Cmiss_field_module_create_group */

int Cmiss_field_group_clear(Cmiss_field_group_id group)
{
	if (group)
	{
		Computed_field_group *group_core =
			Computed_field_group_core_cast(group);
		if (group_core)
		{
			return group_core->clear();
		}
	}
	return 0;
}

int Cmiss_field_group_clear_local(Cmiss_field_group_id group)
{
	if (group)
	{
		Computed_field_group *group_core =
			Computed_field_group_core_cast(group);
		if (group_core)
		{
			return group_core->clearLocal();
		}
	}
	return 0;
}

int Cmiss_field_group_is_empty(Cmiss_field_group_id group)
{
	if (group)
	{
		Computed_field_group *group_core =
			Computed_field_group_core_cast(group);
		if (group_core)
		{
			return group_core->isEmpty();
		}
	}
	return 0;
}

int Cmiss_field_group_is_empty_local(Cmiss_field_group_id group)
{
	if (group)
	{
		Computed_field_group *group_core =
			Computed_field_group_core_cast(group);
		if (group_core)
		{
			return group_core->isEmptyLocal();
		}
	}
	return 0;
}

int Cmiss_field_group_add_local_region(Cmiss_field_group_id group)
{
	if (group)
	{
		Computed_field_group *group_core =
			Computed_field_group_core_cast(group);
		if (group_core)
		{
			return group_core->addLocalRegion();
		}
	}
	return 0;
}

int Cmiss_field_group_contains_local_region(Cmiss_field_group_id group)
{
	if (group)
	{
		Computed_field_group *group_core =
			Computed_field_group_core_cast(group);
		if (group_core)
		{
			return group_core->containsLocalRegion();
		}
	}
	return 0;
}

int Cmiss_field_group_contains_region(Cmiss_field_group_id group, Cmiss_region_id region)
{
	if (group && region)
	{
		Computed_field_group *group_core =
			Computed_field_group_core_cast(group);
		if (group_core)
		{
			return group_core->containsRegion(region);
		}
	}
	return 0;
}

int Cmiss_field_group_remove_region(Cmiss_field_group_id group, Cmiss_region_id region)
{
	if (group && region)
	{
		Computed_field_group *group_core =
			Computed_field_group_core_cast(group);
		if (group_core)
		{
			return group_core->removeRegion(region);
		}
	}
	return 0;
}

int Cmiss_field_group_add_region(Cmiss_field_group_id group, Cmiss_region_id region)
{
	if (group && region)
	{
		Computed_field_group *group_core =
			Computed_field_group_core_cast(group);
		if (group_core)
		{
			return group_core->addRegion(region);
		}
	}
	return 0;
}

Cmiss_field_group_id Cmiss_field_group_get_first_non_empty_group(
	Cmiss_field_group_id group)
{
	if (group)
	{
		Computed_field_group *group_core =
			Computed_field_group_core_cast(group);
		if (group_core)
		{
			return group_core->getFirstNonEmptyGroup();
		}
	}
	return 0;
}

int Cmiss_field_is_type_group(Cmiss_field_id field, void *dummy_void)
{
	int return_code;
	USE_PARAMETER(dummy_void);
	if (field)
	{
		if (dynamic_cast<Computed_field_group*>(field->core))
		{
			return_code = 1;
		}
		else
		{
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_field_is_type_group.  Missing field");
		return_code = 0;
	}

	return (return_code);
} /* Cmiss_field_is_type_group */

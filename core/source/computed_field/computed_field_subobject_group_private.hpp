/***************************************************************************//**
 * FILE : computed_field_subobject_group_private.hpp
 *
 * Implements region sub object groups, e.g. node group, element group.
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

#if !defined (COMPUTED_FIELD_SUBOBJECT_GROUP_HPP)
#define COMPUTED_FIELD_SUBOBJECT_GROUP_HPP
#include <stdlib.h>
#include "api/cmiss_field_subobject_group.h"
#include "computed_field/computed_field.h"
#include "finite_element/finite_element.h"
#include "finite_element/finite_element_region.h"
#include "computed_field/computed_field_group_base.hpp"
#include "computed_field/computed_field_private.hpp"
#include "general/cmiss_set.hpp"
#include "general/debug.h"
#include "region/cmiss_region.h"
#include "general/message.h"
#include "mesh/cmiss_element_private.hpp"
#include "mesh/cmiss_node_private.hpp"
#include <map>
#include <iterator>

/***************************************************************************//**
 * Change details for simple object groups where a single change status is
 * sufficient.
 */
struct Cmiss_field_subobject_group_change_detail : public Cmiss_field_group_base_change_detail
{
private:
	Cmiss_field_group_change_type change;

public:
	Cmiss_field_subobject_group_change_detail() :
		change(CMISS_FIELD_GROUP_NO_CHANGE)
	{
	}

	void clear()
	{
		change = CMISS_FIELD_GROUP_NO_CHANGE;
	}

	Cmiss_field_group_change_type getChange() const
	{
		return change;
	}

	Cmiss_field_group_change_type getLocalChange() const
	{
		return change;
	}

	/** Inform object(s) have been added */
	void changeAdd()
	{
		switch (change)
		{
		case CMISS_FIELD_GROUP_NO_CHANGE:
			change = CMISS_FIELD_GROUP_ADD;
			break;
		case CMISS_FIELD_GROUP_CLEAR:
		case CMISS_FIELD_GROUP_REMOVE:
			change = CMISS_FIELD_GROUP_REPLACE;
			break;
		default:
			// do nothing
			break;
		}
	}

	/** Inform object(s) have been removed (clear handled separately) */
	void changeRemove()
	{
		switch (change)
		{
		case CMISS_FIELD_GROUP_NO_CHANGE:
			change = CMISS_FIELD_GROUP_REMOVE;
			break;
		case CMISS_FIELD_GROUP_ADD:
			change = CMISS_FIELD_GROUP_REPLACE;
			break;
		default:
			// do nothing
			break;
		}
	}

	/** Inform group has been cleared, but wasn't before */
	void changeClear()
	{
		change = CMISS_FIELD_GROUP_CLEAR;
	}
};

class Computed_field_subobject_group : public Computed_field_group_base
{
public:

	Computed_field_subobject_group() :
		Computed_field_group_base()
	{
	}

	const char* get_type_string()
	{
		return ("sub_group_object");
	}

	virtual int isIdentifierInList(int identifier) = 0;

	int check_dependency_for_group_special()
	{
		int dependency_changed = 0;
		if (field->manager_change_status & MANAGER_CHANGE_RESULT(Computed_field))
		{
			dependency_changed = 1;
		}
		else if (field->manager_change_status & MANAGER_CHANGE_ADD(Computed_field))
		{
			const Cmiss_field_subobject_group_change_detail *change_detail =
				dynamic_cast<const Cmiss_field_subobject_group_change_detail *>(get_change_detail());
			const Cmiss_field_group_change_type change = change_detail->getChange();
			if ((change == CMISS_FIELD_GROUP_ADD) || (change == CMISS_FIELD_GROUP_REPLACE))
			{
				dependency_changed = 1;
			}
		}
		return dependency_changed;
	}

};

	template <typename T>
	class Computed_field_sub_group_object : public Computed_field_subobject_group
	{
	public:

		Computed_field_sub_group_object() :
			Computed_field_subobject_group(),
			object_map()
		{
			object_pos = object_map.begin();
		}

		~Computed_field_sub_group_object()
		{
		}

		inline int add_object(int identifier, T object)
		{
			if (object_map.insert(std::make_pair(identifier,object)).second)
			{
				change_detail.changeAdd();
				update();
				return 1;
			}
			return 0;
		};

		inline int remove_object(int identifier)
		{
			if (object_map.find(identifier) != object_map.end())
			{
				object_map.erase(identifier);
				if (0 == object_map.size())
				{
					change_detail.changeClear();
				}
				else
				{
					change_detail.changeRemove();
				}
				update();
				return 1;
			}
			return 0;
		};

		inline T get_object(int identifier)
		{
			T return_object = NULL;
			if (object_map.find(identifier) != object_map.end())
				return_object = object_map.find(identifier)->second;

			return return_object;
		}

		virtual int clear()
		{
			if (object_map.size())
			{
				object_map.clear();
				change_detail.changeClear();
				update();
			}
			return 1;
		};

		int get_object_selected(int identifier,T object)
		{
			USE_PARAMETER(object);
			int return_code = 0;
			if (object_map.find(identifier) != object_map.end() &&
				object_map.find(identifier)->second == object)
			{
				return_code = 1;
			}
			else
			{
				return_code = 0;
			}

			return (return_code);
		};

		virtual int isEmpty() const
		{
			return object_map.empty();
		}

		virtual int isIdentifierInList(int identifier)
		{
			return (!(object_map.empty()) && (object_map.find(identifier) != object_map.end()));
		}

		T getFirstObject()
		{
			T return_object = NULL;
			object_pos = object_map.begin();
			if (object_pos != object_map.end())
			{
				return_object = object_pos->second;
			}
			return return_object;
		}

		T getNextObject()
		{
			T return_object = NULL;
			if (object_pos != object_map.end())
			{
				object_pos++;
				if (object_pos != object_map.end())
				{
					return_object = object_pos->second;
				}
			}
			return return_object;
		}

		virtual Cmiss_field_change_detail *extract_change_detail()
		{
			if (change_detail.getChange() == CMISS_FIELD_GROUP_NO_CHANGE)
				return NULL;
			Cmiss_field_subobject_group_change_detail *prior_change_detail =
				new Cmiss_field_subobject_group_change_detail();
			*prior_change_detail = change_detail;
			change_detail.clear();
			return prior_change_detail;
		}

		virtual const Cmiss_field_change_detail *get_change_detail() const
		{
			return &change_detail;
		}

	private:

		std::map<int, T> object_map;
		Cmiss_field_subobject_group_change_detail change_detail;
		typename std::map<int, T>::iterator object_pos;

		Computed_field_core* copy()
		{
			Computed_field_sub_group_object *core = new Computed_field_sub_group_object();
			core->object_map = this->object_map;
			return (core);
		};

		int compare(Computed_field_core* other_field)
		{
			int return_code;

			ENTER(Computed_field_sub_group_object::compare);
			if (field && dynamic_cast<Computed_field_sub_group_object<T>*>(other_field))
			{
				return_code = 1;
			}
			else
			{
				return_code = 0;
			}
			LEAVE;

			return (return_code);
		}

		int list()
		{
			return 1;
		};

		inline void update()
		{
			Computed_field_changed(field);
		}

	};

	class Computed_field_element_group : public Computed_field_subobject_group
	{
	private:

		Cmiss_mesh_id master_mesh;
		const int dimension;
		struct LIST(FE_element) *object_list;
		Cmiss_field_subobject_group_change_detail change_detail;

	public:

		Computed_field_element_group(Cmiss_mesh_id mesh) :
			Computed_field_subobject_group(),
			// don't want element_groups based on group region FE_region so get master:
			master_mesh(Cmiss_mesh_get_master(mesh)),
			dimension(Cmiss_mesh_get_dimension(master_mesh)),
			object_list(Cmiss_mesh_create_element_list_internal(master_mesh))
		{
			FE_region *fe_region = Cmiss_mesh_get_FE_region_internal(master_mesh);
			FE_region_add_callback(fe_region, Computed_field_element_group::fe_region_change, (void *)this);
		}

		~Computed_field_element_group()
		{
			FE_region *fe_region = Cmiss_mesh_get_FE_region_internal(master_mesh);
			FE_region_remove_callback(fe_region, Computed_field_element_group::fe_region_change, (void *)this);
			DESTROY(LIST(FE_element))(&object_list);
			Cmiss_mesh_destroy(&master_mesh);
		}

		int getDimension()
		{
			return dimension;
		}

		Cmiss_mesh_id getMasterMesh()
		{
			return master_mesh;
		}

		inline int addObject(FE_element *object)
		{
			if (isElementCompatible(object) &&
				(!IS_OBJECT_IN_LIST(FE_element)(object, object_list)) &&
				ADD_OBJECT_TO_LIST(FE_element)(object, object_list))
			{
				change_detail.changeAdd();
				update();
				return 1;
			}
			return 0;
		};

		inline int removeObject(FE_element *object)
		{
			if (IS_OBJECT_IN_LIST(FE_element)(object, object_list))
			{
				REMOVE_OBJECT_FROM_LIST(FE_element)(object, object_list);
				if (0 == NUMBER_IN_LIST(FE_element)(object_list))
				{
					change_detail.changeClear();
				}
				else
				{
					change_detail.changeRemove();
				}
				update();
				return 1;
			}
			return 0;
		};

		/** remove all elements for which conditional_field is true */
		int removeElementsConditional(Cmiss_field_id conditional_field);

		virtual int clear()
		{
			if (NUMBER_IN_LIST(FE_element)(object_list))
			{
				REMOVE_ALL_OBJECTS_FROM_LIST(FE_element)(object_list);
				change_detail.changeClear();
				update();
			}
			return 1;
		};

		int containsObject(FE_element *object)
		{
			int return_code = 0;
			if (IS_OBJECT_IN_LIST(FE_element)(object, object_list))
			{
				return_code = 1;
			}
			else
			{
				return_code = 0;
			}

			return (return_code);
		};

		Cmiss_element_iterator_id createIterator()
		{
			return CREATE_LIST_ITERATOR(FE_element)(object_list);
		}

		/** @return  non-accessed element with that identifier, or 0 if none */
		inline Cmiss_element_id findElementByIdentifier(int identifier)
		{
			struct CM_element_information cm;
			cm.type = ((dimension == 3) ? CM_ELEMENT : ((dimension == 2) ? CM_FACE : CM_LINE));
			cm.number = identifier;
			return FIND_BY_IDENTIFIER_IN_LIST(FE_element,identifier)(&cm, object_list);
		}

		int getSize()
		{
			return NUMBER_IN_LIST(FE_element)(object_list);
		}

		virtual int isEmpty() const
		{
			if (NUMBER_IN_LIST(FE_element)(object_list))
				return 0;
			else
				return 1;
		}

		virtual int isIdentifierInList(int identifier)
		{
			return (0 != findElementByIdentifier(identifier));
		}

		virtual Cmiss_field_change_detail *extract_change_detail()
		{
			if (change_detail.getChange() == CMISS_FIELD_GROUP_NO_CHANGE)
				return NULL;
			Cmiss_field_subobject_group_change_detail *prior_change_detail =
				new Cmiss_field_subobject_group_change_detail();
			*prior_change_detail = change_detail;
			change_detail.clear();
			return prior_change_detail;
		}

		virtual const Cmiss_field_change_detail *get_change_detail() const
		{
			return &change_detail;
		}

		void write_btree_statistics() const
		{
			FE_element_list_write_btree_statistics(object_list);
		}

		/** ensure parent element's faces are in element group */
		int addElementFaces(Cmiss_element_id parent);

		/** ensure parent element's faces are not in element group */
		int removeElementFaces(Cmiss_element_id parent);

	private:

		Computed_field_core* copy()
		{
			return new Computed_field_element_group(master_mesh);
		};

		int compare(Computed_field_core* other_field)
		{
			int return_code;

			ENTER(Computed_field_stl_object_group::compare);
			if (field && dynamic_cast<Computed_field_element_group*>(other_field))
			{
				return_code = 1;
			}
			else
			{
				return_code = 0;
			}
			LEAVE;

			return (return_code);
		}

		int evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache)
		{
			Field_element_xi_location *element_xi_location = dynamic_cast<Field_element_xi_location*>(cache.getLocation());
			if (element_xi_location)
			{
				RealFieldValueCache &valueCache = RealFieldValueCache::cast(inValueCache);
				Cmiss_element_id element = element_xi_location->get_element();
				if (Cmiss_element_get_dimension(element) == dimension)
				{
					valueCache.values[0] = containsObject(element);
				}
				else
				{
					valueCache.values[0] = 0;
				}
				return 1;
			}
			return 0;
		};

		int list()
		{
			return 1;
		};

		inline void update()
		{
			Computed_field_changed(field);
		}

		/** remove objects from the group if they are in the supplied list */
		int removeObjectsNotInFeRegion(struct FE_region *fe_region)
		{
			const int old_size = NUMBER_IN_LIST(FE_element)(object_list);
			int return_code = REMOVE_OBJECTS_FROM_LIST_THAT(FE_element)(
				FE_element_is_not_in_FE_region, (void *)fe_region, object_list);
			const int new_size = NUMBER_IN_LIST(FE_element)(object_list);
			if (new_size != old_size)
			{
				if (new_size == 0)
					change_detail.changeClear();
				else
					change_detail.changeRemove();
				update();
			}
			return return_code;
		}

		/***************************************************************************//**
		 * Callback from <fe_region> with its <changes>. Remove any destroyed elements.
		 */
		static void fe_region_change(struct FE_region *fe_region,
			struct FE_region_changes *changes, void *element_group_void)
		{
			Computed_field_element_group *element_group =
				reinterpret_cast<Computed_field_element_group *>(element_group_void);
			if (fe_region && changes && element_group)
			{
				int fe_element_change_summary;
				if (CHANGE_LOG_GET_CHANGE_SUMMARY(FE_element)(
					changes->fe_element_changes[element_group->dimension - 1], &fe_element_change_summary))
				{
					/* remove elements removed from this fe_region */
					if (fe_element_change_summary & CHANGE_LOG_OBJECT_REMOVED(FE_element))
					{
						element_group->removeObjectsNotInFeRegion(fe_region);
					}
				}
			}
		}

		bool isElementCompatible(Cmiss_element_id element)
		{
			if (get_FE_element_dimension(element) != dimension)
				return false;
			FE_region *fe_region = Cmiss_mesh_get_FE_region_internal(master_mesh);
			FE_region *element_fe_region = FE_element_get_FE_region(element);
			return (element_fe_region == fe_region);
		}

		bool isParentElementCompatible(Cmiss_element_id element)
		{
			if (get_FE_element_dimension(element) != dimension + 1)
				return false;
			FE_region *fe_region = Cmiss_mesh_get_FE_region_internal(master_mesh);
			FE_region *element_fe_region = FE_element_get_FE_region(element);
			return (element_fe_region == fe_region);
		}

	};

	class Computed_field_node_group : public Computed_field_subobject_group
	{
	private:

		Cmiss_nodeset_id master_nodeset;
		struct LIST(FE_node) *object_list;
		Cmiss_field_subobject_group_change_detail change_detail;

	public:

		Computed_field_node_group(Cmiss_nodeset_id nodeset) :
			Computed_field_subobject_group(),
			// don't want node_groups based on group region FE_region so get master:
			master_nodeset(Cmiss_nodeset_get_master(nodeset)),
			object_list(Cmiss_nodeset_create_node_list_internal(master_nodeset))
		{
			FE_region *fe_region = Cmiss_nodeset_get_FE_region_internal(master_nodeset);
			FE_region_add_callback(fe_region, Computed_field_node_group::fe_region_change, (void *)this);
		}

		~Computed_field_node_group()
		{
			FE_region *fe_region = Cmiss_nodeset_get_FE_region_internal(master_nodeset);
			FE_region_remove_callback(fe_region, Computed_field_node_group::fe_region_change, (void *)this);
			DESTROY(LIST(FE_node))(&object_list);
			Cmiss_nodeset_destroy(&master_nodeset);
		}

		Cmiss_nodeset_id getMasterNodeset()
		{
			return master_nodeset;
		}

		inline int addObject(FE_node *object)
		{
			if (isNodeCompatible(object) &&
				(!IS_OBJECT_IN_LIST(FE_node)(object, object_list)) &&
				ADD_OBJECT_TO_LIST(FE_node)(object, object_list))
			{
				change_detail.changeAdd();
				update();
				return 1;
			}
			return 0;
		};

		inline int removeObject(FE_node *object)
		{
			if (IS_OBJECT_IN_LIST(FE_node)(object, object_list))
			{
				REMOVE_OBJECT_FROM_LIST(FE_node)(object, object_list);
				if (0 == NUMBER_IN_LIST(FE_node)(object_list))
				{
					change_detail.changeClear();
				}
				else
				{
					change_detail.changeRemove();
				}
				update();
				return 1;
			}
			return 0;
		};

		/** remove all nodes for which conditional_field is true */
		int removeNodesConditional(Cmiss_field_id conditional_field);

		virtual int clear()
		{
			if (NUMBER_IN_LIST(FE_node)(object_list))
			{
				REMOVE_ALL_OBJECTS_FROM_LIST(FE_node)(object_list);
				change_detail.changeClear();
				update();
			}
			return 1;
		};

		int containsObject(FE_node *object)
		{
			return IS_OBJECT_IN_LIST(FE_node)(object, object_list);
		};

		Cmiss_node_iterator_id createIterator()
		{
			return CREATE_LIST_ITERATOR(FE_node)(object_list);
		}

		/** @return  non-accessed node with that identifier, or 0 if none */
		inline Cmiss_node_id findNodeByIdentifier(int identifier)
		{
			return FIND_BY_IDENTIFIER_IN_LIST(FE_node,cm_node_identifier)(identifier, object_list);
		}

		int getSize()
		{
			return NUMBER_IN_LIST(FE_node)(object_list);
		}

		virtual int isEmpty() const
		{
			if (NUMBER_IN_LIST(FE_node)(object_list))
				return 0;
			else
				return 1;
		}

		virtual int isIdentifierInList(int identifier)
		{
			return (0 != findNodeByIdentifier(identifier));
		}

		virtual Cmiss_field_change_detail *extract_change_detail()
		{
			if (change_detail.getChange() == CMISS_FIELD_GROUP_NO_CHANGE)
				return NULL;
			Cmiss_field_subobject_group_change_detail *prior_change_detail =
				new Cmiss_field_subobject_group_change_detail();
			*prior_change_detail = change_detail;
			change_detail.clear();
			return prior_change_detail;
		}

		virtual const Cmiss_field_change_detail *get_change_detail() const
		{
			return &change_detail;
		}

		void write_btree_statistics() const
		{
			FE_node_list_write_btree_statistics(object_list);
		}

		/** ensure element's nodes are in node group */
		int addElementNodes(Cmiss_element_id element);

		/** ensure element's nodes are not in node group */
		int removeElementNodes(Cmiss_element_id element);

	private:

		Computed_field_core* copy()
		{
			return new Computed_field_node_group(master_nodeset);
		};

		int compare(Computed_field_core* other_field)
		{
			int return_code;

			ENTER(Computed_field_stl_object_group::compare);
			if (field && dynamic_cast<Computed_field_node_group*>(other_field))
			{
				return_code = 1;
			}
			else
			{
				return_code = 0;
			}
			LEAVE;

			return (return_code);
		}

		int evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache)
		{
			Field_node_location *node_location = dynamic_cast<Field_node_location*>(cache.getLocation());
			if (node_location)
			{
				RealFieldValueCache &valueCache = RealFieldValueCache::cast(inValueCache);
				Cmiss_node_id node = node_location->get_node();
				valueCache.values[0] = containsObject(node);
				return 1;
			}
			return 0;
		};

		int list()
		{
			return 1;
		};

		inline void update()
		{
			Computed_field_changed(field);
		}

		/** remove objects from the group if they are in the supplied list */
		int removeObjectsNotInFeRegion(struct FE_region *fe_region)
		{
			const int old_size = NUMBER_IN_LIST(FE_node)(object_list);
			int return_code = REMOVE_OBJECTS_FROM_LIST_THAT(FE_node)(
				FE_node_is_not_in_FE_region, (void *)fe_region, object_list);
			const int new_size = NUMBER_IN_LIST(FE_node)(object_list);
			if (new_size != old_size)
			{
				if (new_size == 0)
					change_detail.changeClear();
				else
					change_detail.changeRemove();
				update();
			}
			return return_code;
		}

		/***************************************************************************//**
		 * Callback from <fe_region> with its <changes>. Remove any destroyed nodes.
		 */
		static void fe_region_change(struct FE_region *fe_region,
			struct FE_region_changes *changes, void *node_group_void)
		{
			Computed_field_node_group *node_group =
				reinterpret_cast<Computed_field_node_group *>(node_group_void);
			if (fe_region && changes && node_group)
			{
				int fe_node_change_summary;
				if (CHANGE_LOG_GET_CHANGE_SUMMARY(FE_node)(changes->fe_node_changes, &fe_node_change_summary))
				{
					/* remove nodes removed from this fe_region */
					if (fe_node_change_summary & CHANGE_LOG_OBJECT_REMOVED(FE_node))
					{
						node_group->removeObjectsNotInFeRegion(fe_region);
					}
				}
			}
		}

		bool isNodeCompatible(Cmiss_node_id node)
		{
			FE_region *fe_region = Cmiss_nodeset_get_FE_region_internal(master_nodeset);
			FE_region *node_fe_region = FE_node_get_FE_region(node);
			if (FE_region_is_data_FE_region(fe_region))
				node_fe_region = FE_region_get_data_FE_region(node_fe_region);
			return (node_fe_region == fe_region);
		}

		bool isParentElementCompatible(Cmiss_element_id element)
		{
			FE_region *fe_region = Cmiss_nodeset_get_FE_region_internal(master_nodeset);
			FE_region *element_fe_region = FE_element_get_FE_region(element);
			return (element_fe_region == fe_region);
		}

	};

template <typename ObjectType, typename FieldType>
Computed_field_sub_group_object<ObjectType> *Computed_field_sub_group_object_core_cast(
	FieldType object_group_field)
 {
	return (static_cast<Computed_field_sub_group_object<ObjectType>*>(
		reinterpret_cast<Computed_field*>(object_group_field)->core));
 }

inline Computed_field_element_group *Computed_field_element_group_core_cast(
	Cmiss_field_element_group_id object_group_field)
{
	return (static_cast<Computed_field_element_group *>(
		reinterpret_cast<Computed_field*>(object_group_field)->core));
}

inline Computed_field_node_group *Computed_field_node_group_core_cast(
	Cmiss_field_node_group_id object_group_field)
{
	return (static_cast<Computed_field_node_group *>(
		reinterpret_cast<Computed_field*>(object_group_field)->core));
}

#endif /* COMPUTED_FIELD_SUBOBJECT_GROUP_HPP */


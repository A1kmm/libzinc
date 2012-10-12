/*******************************************************************************
FILE : computed_field_finite_element.cpp

LAST MODIFIED : 24 August 2006

DESCRIPTION :
Implements a number of basic component wise operations on computed fields.
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
#include <math.h>
#include "api/cmiss_field_module.h"
#include "api/cmiss_field_finite_element.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_coordinate.h"
#include "computed_field/computed_field_find_xi.h"
#include "computed_field/computed_field_private.hpp"
#include "computed_field/computed_field_set.h"
#include "finite_element/finite_element.h"
#include "finite_element/finite_element_discretization.h"
#include "finite_element/finite_element_region.h"
#include "finite_element/finite_element_region_private.h"
#include "finite_element/finite_element_time.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "general/message.h"
#include "computed_field/computed_field_finite_element.h"
#include "computed_field/field_module.hpp"
#include "general/enumerator_conversion.hpp"
#include "mesh/cmiss_element_private.hpp"

#if defined (DEBUG_CODE)
/* SAB This field is useful for debugging when things don't clean up properly
	but has to be used carefully, especially as operations such as caching
	accesses the node or element being considered so you get effects like
	the first point evaluated in an element having a count one less than
	all the others */
#define COMPUTED_FIELD_ACCESS_COUNT
#endif /* defined (DEBUG_CODE) */

/*
Module types
------------
*/

struct Computed_field_finite_element_package : public Computed_field_type_package
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
}; /* Computed_field_finite_element_package */

namespace {

/***************************************************************************//**
 * Establishes the FE_element_field values necessary for evaluating field in
 * element at time, inherited from optional top_level_element. Uses existing
 * values in cache if nothing changed.
 * @param calculate_derivatives  Controls whether basis functions for
 * derivatives are also evaluated.
 * @param differential_order  Optional order to differentiate monomials by.
 * @param differential_xi_indices  Which xi indices to differentiate.
 */
int calculate_FE_element_field_values_for_element(
	LIST(FE_element_field_values) *field_values_cache,
	FE_element_field_values* &fe_element_field_values,
	FE_field *fe_field, int calculate_derivatives, struct FE_element *element,
	FE_value time, struct FE_element *top_level_element, int differential_order = 0,
	int *differential_xi_indices = 0)
{
	int return_code = 1;
	if (field_values_cache && fe_field && element)
	{
		/* ensure we have FE_element_field_values for element, with
			 derivatives_calculated if requested */
		if ((!fe_element_field_values) ||
			(!FE_element_field_values_are_for_element_and_time(
				fe_element_field_values, element, time, top_level_element)) ||
			(calculate_derivatives &&
				(!FE_element_field_values_have_derivatives_calculated(fe_element_field_values))))
		{
			int need_update = 0;
			int need_to_add_to_list = 0;
			if (!(fe_element_field_values = FIND_BY_IDENTIFIER_IN_LIST(
				FE_element_field_values, element)(element, field_values_cache)))
			{
				need_update = 1;
				fe_element_field_values = CREATE(FE_element_field_values)();
				if (fe_element_field_values)
				{
					need_to_add_to_list = 1;
				}
				else
				{
					return_code = 0;
				}
			}
			else
			{
				if ((!FE_element_field_values_are_for_element_and_time(
						 fe_element_field_values,element,time,top_level_element))||
					(calculate_derivatives&&
						(!FE_element_field_values_have_derivatives_calculated(fe_element_field_values))))
				{
					need_update = 1;
					clear_FE_element_field_values(fe_element_field_values);
				}
			}
			if (return_code && need_update)
			{
				/* note that FE_element_field_values accesses the element */
				if (calculate_FE_element_field_values(element,fe_field,
						time,calculate_derivatives,fe_element_field_values,
						top_level_element))
				{

					for (int i = 0 ; i < differential_order ; i++)
					{
						FE_element_field_values_differentiate(fe_element_field_values,
							differential_xi_indices[i]);
					}

					if (need_to_add_to_list)
					{
						/* Set a cache size limit */
						if (1000 < NUMBER_IN_LIST(FE_element_field_values)(field_values_cache))
						{
							REMOVE_ALL_OBJECTS_FROM_LIST(FE_element_field_values)
								(field_values_cache);
						}
						return_code = ADD_OBJECT_TO_LIST(FE_element_field_values)(
							fe_element_field_values, field_values_cache);
					}
				}
				else
				{
					/* clear element to indicate that values are clear */
					clear_FE_element_field_values(fe_element_field_values);
					return_code=0;
				}
			}
		}
	}
	else
	{
		return_code = 0;
	}
	return (return_code);
}

class FiniteElementRealFieldValueCache : public RealFieldValueCache
{
public:
	FE_element_field_values* fe_element_field_values; // cache for a single element at one time

	/* Keep a cache of FE_element_field_values as calculation is expensive */
	LIST(FE_element_field_values) *field_values_cache;

	FiniteElementRealFieldValueCache(int componentCount) :
		RealFieldValueCache(componentCount),
		fe_element_field_values(0),
		field_values_cache(CREATE(LIST(FE_element_field_values))())
	{
	}

	virtual ~FiniteElementRealFieldValueCache()
	{
		DESTROY(LIST(FE_element_field_values))(&field_values_cache);
	}

	virtual void clear()
	{
		REMOVE_ALL_OBJECTS_FROM_LIST(FE_element_field_values)(field_values_cache);
		// Following was a pointer to an object just destroyed, so must clear
		fe_element_field_values = (FE_element_field_values *)NULL;
		RealFieldValueCache::clear();
	}

	static FiniteElementRealFieldValueCache* cast(FieldValueCache* valueCache)
   {
		return FIELD_VALUE_CACHE_CAST<FiniteElementRealFieldValueCache*>(valueCache);
   }

	static FiniteElementRealFieldValueCache& cast(FieldValueCache& valueCache)
   {
		return FIELD_VALUE_CACHE_CAST<FiniteElementRealFieldValueCache&>(valueCache);
   }

};

// FE_element_field_values are needed to evaluate indexed string FE_fields on elements
class FiniteElementStringFieldValueCache : public StringFieldValueCache
{
public:
	FE_element_field_values* fe_element_field_values; // cache for a single element at one time

	/* Keep a cache of FE_element_field_values as calculation is expensive */
	LIST(FE_element_field_values) *field_values_cache;

	FiniteElementStringFieldValueCache() :
		StringFieldValueCache(),
		fe_element_field_values(0),
		field_values_cache(CREATE(LIST(FE_element_field_values))())
	{
	}

	virtual ~FiniteElementStringFieldValueCache()
	{
		DESTROY(LIST(FE_element_field_values))(&field_values_cache);
	}

	virtual void clear()
	{
		REMOVE_ALL_OBJECTS_FROM_LIST(FE_element_field_values)(field_values_cache);
		// Following was a pointer to an object just destroyed, so must clear
		fe_element_field_values = (FE_element_field_values *)NULL;
		StringFieldValueCache::clear();
	}

	static FiniteElementStringFieldValueCache* cast(FieldValueCache* valueCache)
   {
		return FIELD_VALUE_CACHE_CAST<FiniteElementStringFieldValueCache*>(valueCache);
   }

	static FiniteElementStringFieldValueCache& cast(FieldValueCache& valueCache)
   {
		return FIELD_VALUE_CACHE_CAST<FiniteElementStringFieldValueCache&>(valueCache);
   }

};

const char computed_field_finite_element_type_string[] = "finite_element";

class Computed_field_finite_element : public Computed_field_core
{
public:
	FE_field* fe_field;

public:
	Computed_field_finite_element(FE_field *fe_field) :
		Computed_field_core(),
		fe_field(ACCESS(FE_field)(fe_field))
	{
		FE_field_add_wrapper(fe_field);
	};

	virtual ~Computed_field_finite_element();

private:
	Computed_field_core *copy();

	const char *get_type_string()
	{
		return(computed_field_finite_element_type_string);
	}

	int compare(Computed_field_core* other_field);

	virtual FieldValueCache *createValueCache(Cmiss_field_cache& /*parentCache*/)
	{
		enum Value_type value_type = get_FE_field_value_type(fe_field);
		switch (value_type)
		{
			case ELEMENT_XI_VALUE:
				return new MeshLocationFieldValueCache();
			case STRING_VALUE:
			case URL_VALUE:
				return new FiniteElementStringFieldValueCache();
			default:
				break;
		}
		// Note it will be more efficient in some cases to allow finite element field value caches
		// in related Cmiss_field_cache objects to have a common FE_element_field_values list,
		// however they must not be shared with time lookup fields as only a single time is cached
		// and performance will be poor. Leaving implementation to a later date.
		return new FiniteElementRealFieldValueCache(field->number_of_components);
	}

	int evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache);

	int list();

	char* get_command_string();

	virtual bool is_defined_at_location(Cmiss_field_cache& cache);

	int has_multiple_times();

	int has_numerical_components();

	int not_in_use();

	enum FieldAssignmentResult assign(Cmiss_field_cache& cache, RealFieldValueCache& valueCache);

	virtual enum FieldAssignmentResult assign(Cmiss_field_cache& /*cache*/, MeshLocationFieldValueCache& /*valueCache*/);

	virtual enum FieldAssignmentResult assign(Cmiss_field_cache& /*cache*/, StringFieldValueCache& /*valueCache*/);

	virtual void propagate_coordinate_system()
	{
		set_FE_field_coordinate_system(fe_field, &(field->coordinate_system));
	}

	int get_native_discretization_in_element(
		struct FE_element *element,int *number_in_xi);

	virtual bool is_non_linear() const
	{
		return FE_field_uses_non_linear_basis(fe_field) == 1;
	}

	virtual int set_name(const char *name)
	{
		return FE_region_set_FE_field_name(FE_field_get_FE_region(fe_field), fe_field, name);
	};

	virtual int get_attribute_integer(enum Cmiss_field_attribute attribute) const
	{
		if (attribute == CMISS_FIELD_ATTRIBUTE_IS_COORDINATE)
			return (get_FE_field_CM_field_type(fe_field) == CM_COORDINATE_FIELD);
		return 0;
	}

	virtual int set_attribute_integer(enum Cmiss_field_attribute attribute, int value)
	{
		// Note that CM_field_type is an enum with 3 states
		// so can't be COORDINATE and ANATOMICAL at the same time.
		if (attribute == CMISS_FIELD_ATTRIBUTE_IS_COORDINATE)
		{
			CM_field_type cm_field_type = get_FE_field_CM_field_type(fe_field);
			if (value)
			{
				if (cm_field_type != CM_COORDINATE_FIELD)
					set_FE_field_CM_field_type(fe_field, CM_COORDINATE_FIELD);
			}
			else
			{
				if (cm_field_type == CM_COORDINATE_FIELD)
					set_FE_field_CM_field_type(fe_field, CM_GENERAL_FIELD);
			}
			return 1;
		}
		return 0;
	}

	virtual Cmiss_field_value_type get_value_type() const
	{
		enum Value_type fe_value_type = get_FE_field_value_type(fe_field);
		Cmiss_field_value_type value_type = CMISS_FIELD_VALUE_TYPE_INVALID;
		switch (fe_value_type)
		{
			case ELEMENT_XI_VALUE:
				value_type = CMISS_FIELD_VALUE_TYPE_MESH_LOCATION;
				break;
			case STRING_VALUE:
			case URL_VALUE:
				value_type = CMISS_FIELD_VALUE_TYPE_STRING;
				break;
			case DOUBLE_VALUE:
			case FE_VALUE_VALUE:
			case FLT_VALUE:
			case INT_VALUE:
			case SHORT_VALUE:
				value_type = CMISS_FIELD_VALUE_TYPE_REAL;
				break;
			default:
				break;
		}
		return value_type;
	}

};

Computed_field_finite_element::~Computed_field_finite_element()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Clear the type specific data used by this type.
==============================================================================*/
{

	ENTER(Computed_field_finite_element::~Computed_field_finite_element);
	if (field)
	{
		if (fe_field)
		{
			/* The following logic only removes the FE_field when it is not in
			 * use, which should be ensured in normal use by
			 * MANAGED_OBJECT_NOT_IN_USE(Computed_field).
			 * There are complications due to the merge process used when reading
			 * fields from file which appears to leave some FE_fields temporarily
			 * not in their owning FE_region when this is called.
			 * Also gfx define field commands create and destroy temporary
			 * finite_element field wrappers & we don't want to clean up the
			 * FE_field until the last wrapper is destroyed.
			 */
			int number_of_remaining_wrappers = FE_field_remove_wrapper(fe_field);
			if (0 == number_of_remaining_wrappers)
			{
				struct FE_region *fe_region = FE_field_get_FE_region(fe_field);
				if (fe_region && FE_region_contains_FE_field(fe_region, fe_field) &&
					(!FE_region_is_FE_field_in_use(fe_region, fe_field)))
				{
					if (!FE_region_remove_FE_field(fe_region, fe_field))
					{
						display_message(ERROR_MESSAGE,
							"Computed_field_finite_element::~Computed_field_finite_element.  "
							"Destroying computed field before FE_field.");
					}
				}
			}
			DEACCESS(FE_field)(&(fe_field));
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_finite_element::~Computed_field_finite_element.  "
			"Invalid arguments.");
	}
	LEAVE;

} /* Computed_field_finite_element::~Computed_field_finite_element */

Computed_field_core* Computed_field_finite_element::copy()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Copy the type specific data used by this type.
==============================================================================*/
{
	Computed_field_finite_element* core =
		new Computed_field_finite_element(fe_field);

	return (core);
} /* Computed_field_finite_element::copy */

int Computed_field_finite_element::compare(Computed_field_core *other_core)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Compare the type specific data
==============================================================================*/
{
	Computed_field_finite_element* other;
	int return_code;

	ENTER(Computed_field_finite_element::compare);
	if (field && (other = dynamic_cast<Computed_field_finite_element*>(other_core)))
	{
		return_code = (fe_field == other->fe_field);
	}
	else
	{
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_finite_element::compare */

bool Computed_field_finite_element::is_defined_at_location(Cmiss_field_cache& cache)
{
	Field_element_xi_location *element_xi_location;
	Field_node_location *node_location;
	if (0 != (element_xi_location = dynamic_cast<Field_element_xi_location*>(cache.getLocation())))
	{
		return (0 != FE_field_is_defined_in_element(fe_field, element_xi_location->get_element()));
	}
	else if (0 != (node_location = dynamic_cast<Field_node_location*>(cache.getLocation())))
	{
		return (0 != FE_field_is_defined_at_node(fe_field, node_location->get_node()));
	}
	return false;
}

int Computed_field_finite_element::has_multiple_times()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Check the fe_field
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_finite_element::has_multiple_times);
	if (field)
	{
		return_code=FE_field_has_multiple_times(fe_field);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_finite_element::has_multiple_times.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_finite_element::has_multiple_times */

int Computed_field_finite_element::has_numerical_components()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_finite_element::has_numerical_components);
	if (field)
	{
		return_code=Value_type_is_numeric_simple(
			get_FE_field_value_type(fe_field));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_finite_element::has_numerical_components.  Missing field");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_finite_element::has_numerical_components */

int Computed_field_finite_element::not_in_use()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
The FE_field must also not be in use.
==============================================================================*/
{
	int return_code;
	struct FE_region *fe_region;

	ENTER(Computed_field_finite_element::not_in_use);
	if (field)
	{
		/* check the fe_field can be destroyed */
		fe_region = FE_field_get_FE_region(fe_field);
		if (fe_region)
		{
			/* ask owning FE_region if fe_field is used in nodes and elements */
			if (FE_region_is_FE_field_in_use(fe_region, fe_field))
			{
				return_code = 0;
			}
			else
			{
				return_code = 1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_finite_element::not_in_use.  Missing FE_region");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_finite_element::not_in_use.  Missing field");
		return_code = 0;
	}

	return (return_code);
} /* Computed_field_finite_element::not_in_use */

int Computed_field_finite_element::evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache)
{
	int return_code = 0;
	enum Value_type value_type = get_FE_field_value_type(fe_field);
	switch (value_type)
	{
		case ELEMENT_XI_VALUE:
		{
			Field_node_location *node_location;
			if (0 != (node_location = dynamic_cast<Field_node_location*>(cache.getLocation())))
			{
				MeshLocationFieldValueCache& meshLocationValueCache = MeshLocationFieldValueCache::cast(inValueCache);
				// can only have 1 component; can only be evaluated at node so assume node location
				Cmiss_element_id element = 0;
				FE_value xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
				return_code = get_FE_nodal_element_xi_value(node_location->get_node(), fe_field, /*component_number*/0,
					/*version_number*/0, FE_NODAL_VALUE, &element, xi) && element;
				if (return_code)
				{
					meshLocationValueCache.setMeshLocation(element, xi);
				}
			}
		} break;
		case STRING_VALUE:
		case URL_VALUE:
		{
			FiniteElementStringFieldValueCache& feStringValueCache = FiniteElementStringFieldValueCache::cast(inValueCache);
			if (feStringValueCache.stringValue)
			{
				DEALLOCATE(feStringValueCache.stringValue);
			}
			Field_element_xi_location* element_xi_location;
			Field_node_location *node_location;
			if (0 != (node_location = dynamic_cast<Field_node_location*>(cache.getLocation())))
			{
				// can only have 1 component
				return_code = get_FE_nodal_value_as_string(node_location->get_node(), fe_field,
					/*component_number*/0, /*version_number*/0, /*nodal_value_type*/FE_NODAL_VALUE,
					/*ignored*/cache.getTime(), &(feStringValueCache.stringValue));
			}
			else if (0 != (element_xi_location = dynamic_cast<Field_element_xi_location*>(cache.getLocation())))
			{
				Cmiss_element_id element = element_xi_location->get_element();
				Cmiss_element_id top_level_element = element_xi_location->get_top_level_element();
				FE_value time = element_xi_location->get_time();
				const FE_value* xi = element_xi_location->get_xi();

				return_code = calculate_FE_element_field_values_for_element(
					feStringValueCache.field_values_cache, feStringValueCache.fe_element_field_values,
					fe_field, /*number_of_derivatives*/0, element, time, top_level_element);
				if (return_code)
				{
					return_code = calculate_FE_element_field_as_string(-1,
						feStringValueCache.fe_element_field_values, xi, &(feStringValueCache.stringValue));
				}
			}
		} break;
		default:
		{
			FiniteElementRealFieldValueCache& feValueCache = FiniteElementRealFieldValueCache::cast(inValueCache);
			Field_element_xi_location* element_xi_location;
			Field_node_location *node_location;
			if (0 != (element_xi_location = dynamic_cast<Field_element_xi_location*>(cache.getLocation())))
			{
				Cmiss_element_id element = element_xi_location->get_element();
				Cmiss_element_id top_level_element = element_xi_location->get_top_level_element();
				FE_value time = element_xi_location->get_time();
				const FE_value* xi = element_xi_location->get_xi();
				int number_of_derivatives = cache.getRequestedDerivatives();

				return_code = calculate_FE_element_field_values_for_element(
					feValueCache.field_values_cache, feValueCache.fe_element_field_values,
					fe_field, (0 < number_of_derivatives), element, time, top_level_element);
				if (return_code)
				{
					/* component number -1 = calculate all components */
					switch (value_type)
					{
						case FE_VALUE_VALUE:
						case SHORT_VALUE:
						{
							if (number_of_derivatives)
							{
								return_code=calculate_FE_element_field(-1,
									feValueCache.fe_element_field_values,xi,feValueCache.values,
									feValueCache.derivatives);
								feValueCache.derivatives_valid = (0<number_of_derivatives);
							}
							else
							{
								return_code=calculate_FE_element_field(-1,
									feValueCache.fe_element_field_values,xi,feValueCache.values,
									(FE_value *)NULL);
							}
						} break;
						case INT_VALUE:
						{
							/* no derivatives for this value_type */
							feValueCache.derivatives_valid=0;
							if (number_of_derivatives)
							{
								display_message(ERROR_MESSAGE,
									"Computed_field_finite_element::evaluate.  "
									"Derivatives not defined for integer fields");
								return_code=0;
							}
							else
							{
								int *int_values;
								if (ALLOCATE(int_values,int,field->number_of_components))
								{
									return_code=calculate_FE_element_field_int_values(-1,
										feValueCache.fe_element_field_values,xi,int_values);
									for (int i=0;i<field->number_of_components;i++)
									{
										feValueCache.values[i]=(FE_value)int_values[i];
									}
									DEALLOCATE(int_values);
								}
								else
								{
									return_code=0;
								}
							}
						} break;
						default:
						{
							display_message(ERROR_MESSAGE,
								"Computed_field_finite_element::evaluate.  "
								"Unsupported value type %s in finite_element field",
								Value_type_string(value_type));
							return_code=0;
						} break;
					}
				}
			}
			else if (0 != (node_location = dynamic_cast<Field_node_location*>(cache.getLocation())))
			{
				double double_value;
				float float_value;
				int int_value;
				short short_value;

				FE_node *node = node_location->get_node();
				FE_value time = node_location->get_time();

				/* not very efficient - should cache FE_node_field or similar */
				return_code = 1;
				for (int i=0;(i<field->number_of_components)&&return_code;i++)
				{
					switch (value_type)
					{
						case DOUBLE_VALUE:
						{
							return_code=get_FE_nodal_double_value(node,
								fe_field,/*component_number*/i, /*version_number*/0,
								/*nodal_value_type*/FE_NODAL_VALUE,time,&double_value);
							feValueCache.values[i] = (FE_value)double_value;
						} break;
						case FE_VALUE_VALUE:
						{
							return_code=get_FE_nodal_FE_value_value(node,
								fe_field,/*component_number*/i, /*version_number*/0,
								/*nodal_value_type*/FE_NODAL_VALUE,time,&(feValueCache.values[i]));
						} break;
						case FLT_VALUE:
						{
							return_code=get_FE_nodal_float_value(node,fe_field,/*component_number*/i,
								/*version_number*/0,/*nodal_value_type*/FE_NODAL_VALUE,
								time,&float_value);
							feValueCache.values[i] = (FE_value)float_value;
						} break;
						case INT_VALUE:
						{
							return_code=get_FE_nodal_int_value(node,fe_field,/*component_number*/i,
								/*version_number*/0,/*nodal_value_type*/FE_NODAL_VALUE,
								time,&int_value);
							feValueCache.values[i] = (FE_value)int_value;
						} break;
						case SHORT_VALUE:
						{
							return_code=get_FE_nodal_short_value(node,fe_field,/*component_number*/i,
								/*version_number*/0,/*nodal_value_type*/FE_NODAL_VALUE,
								time,&short_value);
							feValueCache.values[i] = (FE_value)short_value;
						} break;
						default:
						{
							display_message(ERROR_MESSAGE,
								"Computed_field_finite_element::evaluate.  "
								"Unsupported value type %s in finite_element field",
								Value_type_string(value_type));
							return_code=0;
						} break;
					}
					/* No derivatives at node (at least at the moment!) */
					feValueCache.derivatives_valid = 0;
				}
			}
			else
			{
				return_code = 0;
			}
		} break;
	}
	return return_code;
}

enum FieldAssignmentResult Computed_field_finite_element::assign(Cmiss_field_cache& cache, RealFieldValueCache& valueCache)
{
	if (cache.assignInCacheOnly())
	{
		return FIELD_ASSIGNMENT_RESULT_ALL_VALUES_SET;
	}
	FieldAssignmentResult result = FIELD_ASSIGNMENT_RESULT_ALL_VALUES_SET;
	enum Value_type value_type = get_FE_field_value_type(fe_field);
	Field_element_xi_location *element_xi_location;
	Field_node_location *node_location;
	element_xi_location = dynamic_cast<Field_element_xi_location*>(cache.getLocation());
	if (element_xi_location)
	{
		enum Value_type value_type;
		FE_value *grid_values;
		int element_dimension, grid_map_number_in_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS],
			indices[MAXIMUM_ELEMENT_XI_DIMENSIONS], *grid_int_values, offset;
		struct FE_element_shape *element_shape;

		FE_element* element = element_xi_location->get_element();
		const FE_value* xi = element_xi_location->get_xi();

		element_dimension = get_FE_element_dimension(element);
		if (FE_element_field_is_grid_based(element,fe_field))
		{
			int return_code=1;
			for (int k = 0 ; (k < field->number_of_components) && return_code ; k++)
			{
				/* ignore non-grid-based components */
				if (get_FE_element_field_component_grid_map_number_in_xi(element,
						fe_field, /*component_number*/k, grid_map_number_in_xi))
				{
					if (get_FE_element_shape(element, &element_shape) &&
						FE_element_shape_get_indices_for_xi_location_in_cell_corners(
							element_shape, grid_map_number_in_xi, xi, indices))
					{
						offset = indices[element_dimension - 1];
						for (int i = element_dimension - 2 ; i >= 0 ; i--)
						{
							offset = offset * (grid_map_number_in_xi[i] + 1) + indices[i];
						}
						value_type=get_FE_field_value_type(fe_field);
						switch (value_type)
						{
							case FE_VALUE_VALUE:
							{
								if (get_FE_element_field_component_grid_FE_value_values(element,
										fe_field, k, &grid_values))
								{
									grid_values[offset] = valueCache.values[k];
									if (!set_FE_element_field_component_grid_FE_value_values(
											 element, fe_field, k, grid_values))
									{
										display_message(ERROR_MESSAGE,
											"Computed_field_finite_element::assign.  "
											"Unable to set finite element grid FE_value values");
										return_code=0;
									}
									DEALLOCATE(grid_values);
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"Computed_field_finite_element::assign.  "
										"Unable to get old grid FE_value values");
									return_code=0;
								}
							} break;
							case INT_VALUE:
							{
								result = FIELD_ASSIGNMENT_RESULT_PARTIAL_VALUES_SET;
								if (get_FE_element_field_component_grid_int_values(element,
										fe_field, k, &grid_int_values))
								{
									grid_int_values[offset] = (int)valueCache.values[k];
									if (!set_FE_element_field_component_grid_int_values(
											 element, fe_field, k, grid_int_values))
									{
										display_message(ERROR_MESSAGE,
											"Computed_field_finite_element::assign.  "
											"Unable to set finite element grid int values");
										return_code=0;
									}
									DEALLOCATE(grid_int_values);
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"Computed_field_finite_element::assign.  "
										"Unable to get old grid int values");
									return_code=0;
								}
							} break;
							default:
							{
								return_code=0;
							} break;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Computed_field_finite_element::assign.  "
							"Element locations do not coincide with grid");
						return_code = 0;
					}
				}
			}
			if (!return_code)
			{
				result = FIELD_ASSIGNMENT_RESULT_FAIL;
			}
		}
		else if (FE_element_field_is_standard_node_based(element, fe_field))
		{
			result = FIELD_ASSIGNMENT_RESULT_FAIL;
		}
	}
	else if (0 != (node_location = dynamic_cast<Field_node_location*>(cache.getLocation())))
	{
		FE_node *node = node_location->get_node();
		FE_value time = node_location->get_time();
		for (int i=0;i<field->number_of_components;i++)
		{
			/* set values all versions; to set values for selected version only,
				use COMPUTED_FIELD_NODE_VALUE instead */
			int k=get_FE_node_field_component_number_of_versions(node, fe_field, i);
			for (int j=0; j<k; j++)
			{
				int return_code = 0;
				switch (value_type)
				{
					case DOUBLE_VALUE:
					{
						double double_value=(double)valueCache.values[i];
						return_code=set_FE_nodal_double_value(node,fe_field,/*component_number*/i,
							j,/*nodal_value_type*/FE_NODAL_VALUE,time,double_value);
					} break;
					case FE_VALUE_VALUE:
					{
						return_code=set_FE_nodal_FE_value_value(node,fe_field,/*component_number*/i,
							j,/*nodal_value_type*/FE_NODAL_VALUE,time,valueCache.values[i]);
					} break;
					case FLT_VALUE:
					{
						float float_value=(float)valueCache.values[i];
						return_code=set_FE_nodal_float_value(node,fe_field,/*component_number*/i,
							j,/*nodal_value_type*/FE_NODAL_VALUE,time,float_value);
					} break;
					case INT_VALUE:
					{
						result = FIELD_ASSIGNMENT_RESULT_PARTIAL_VALUES_SET;
						int int_value=(int)floor(valueCache.values[i]+0.5);
						return_code=set_FE_nodal_int_value(node,fe_field,/*component_number*/i,
							j,/*nodal_value_type*/FE_NODAL_VALUE,time,int_value);
					} break;
					case SHORT_VALUE:
					{
						result = FIELD_ASSIGNMENT_RESULT_PARTIAL_VALUES_SET;
						short short_value=(short)floor(valueCache.values[i]+0.5);
						return_code=set_FE_nodal_short_value(node,fe_field,/*component_number*/i,
							j,/*nodal_value_type*/FE_NODAL_VALUE,time,short_value);
					} break;
					default:
					{
					} break;
				}
				if (!return_code)
					return FIELD_ASSIGNMENT_RESULT_FAIL;
			}
		}
	}
	else
	{
		result = FIELD_ASSIGNMENT_RESULT_FAIL;
	}
	if (result != FIELD_ASSIGNMENT_RESULT_FAIL)
	{
		// clear this and dependent field caches due to DOFs changing (wasteful if data points changed):
		field->clearCaches();
		valueCache.derivatives_valid = 0;
	}
	return result;
}

enum FieldAssignmentResult Computed_field_finite_element::assign(Cmiss_field_cache& cache, MeshLocationFieldValueCache& valueCache)
{
	Field_node_location *node_location = dynamic_cast<Field_node_location*>(cache.getLocation());
	if (node_location &&
		(get_FE_field_value_type(fe_field) == ELEMENT_XI_VALUE) &&
		(get_FE_field_FE_field_type(fe_field) == GENERAL_FE_FIELD))
	{
		if (cache.assignInCacheOnly() ||
			set_FE_nodal_element_xi_value(node_location->get_node(), fe_field,
				/*component_number*/0, /*version*/0, FE_NODAL_VALUE, valueCache.element, valueCache.xi))
		{
			return FIELD_ASSIGNMENT_RESULT_ALL_VALUES_SET;
		}
	}
	return FIELD_ASSIGNMENT_RESULT_FAIL;
};

enum FieldAssignmentResult Computed_field_finite_element::assign(Cmiss_field_cache& cache, StringFieldValueCache& valueCache)
{
	Field_node_location *node_location = dynamic_cast<Field_node_location*>(cache.getLocation());
	if (node_location &&
		(get_FE_field_value_type(fe_field) == STRING_VALUE) &&
		(get_FE_field_FE_field_type(fe_field) == GENERAL_FE_FIELD))
	{
		if (cache.assignInCacheOnly() ||
			set_FE_nodal_string_value(node_location->get_node(),
				fe_field, /*component_number*/0, /*version*/0,
				FE_NODAL_VALUE, const_cast<char *>(valueCache.stringValue)))
		{
			return FIELD_ASSIGNMENT_RESULT_ALL_VALUES_SET;
		}
	}
	return FIELD_ASSIGNMENT_RESULT_FAIL;
}

int Computed_field_finite_element::get_native_discretization_in_element(
	struct FE_element *element,int *number_in_xi)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
If the <field> is grid-based in <element>, returns in
<number_in_xi> the numbers of finite difference cells in each xi-direction of
<element>. Note that this number is one less than the number of grid points in
each direction. <number_in_xi> should be allocated with at least as much space
as the number of dimensions in <element>, but is assumed to have no more than
MAXIMUM_ELEMENT_XI_DIMENSIONS so that
int number_in_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS] can be passed to this function.
Returns 0 with no errors if the field is not grid-based.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_native_discretization_in_element);
	if (field&&element&&number_in_xi&&
		(MAXIMUM_ELEMENT_XI_DIMENSIONS>=get_FE_element_dimension(element)))
	{
		if (FE_element_field_is_grid_based(element,fe_field))
		{
			/* use only first component */
			return_code=get_FE_element_field_component_grid_map_number_in_xi(element,
				fe_field, /*component_number*/0, number_in_xi);
		}
		else
		{
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_native_discretization_in_element.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_native_discretization_in_element */

int Computed_field_finite_element::list()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	char *field_name;
	int return_code;

	ENTER(List_Computed_field_finite_element);
	if (field)
	{
		return_code=GET_NAME(FE_field)(fe_field,&field_name);
		if (return_code)
		{
			display_message(INFORMATION_MESSAGE,"    fe_field : %s\n",field_name);
			DEALLOCATE(field_name);
		}
		display_message(INFORMATION_MESSAGE,"    CM field type : %s\n",
			ENUMERATOR_STRING(CM_field_type)(get_FE_field_CM_field_type(fe_field)));
		Value_type value_type = get_FE_field_value_type(fe_field);
		display_message(INFORMATION_MESSAGE,"    Value type : %s\n",
			Value_type_string(value_type));
		if (ELEMENT_XI_VALUE == value_type)
		{
			int element_xi_mesh_dimension = FE_field_get_element_xi_mesh_dimension(fe_field);
			if (element_xi_mesh_dimension)
			{
				display_message(INFORMATION_MESSAGE,"    mesh dimension : %d\n", element_xi_mesh_dimension);
			}
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_finite_element.  Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_finite_element */

char *Computed_field_finite_element::get_command_string()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *component_name, temp_string[40];
	int error, i, number_of_components;

	ENTER(Computed_field_finite_element::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_finite_element_type_string, &error);
		number_of_components = get_FE_field_number_of_components(fe_field);
		sprintf(temp_string, " number_of_components %d ", number_of_components);
		append_string(&command_string, temp_string, &error);
		append_string(&command_string, ENUMERATOR_STRING(CM_field_type)(
			get_FE_field_CM_field_type(fe_field)), &error);
		append_string(&command_string, " ", &error);
		append_string(&command_string,
			Value_type_string(get_FE_field_value_type(fe_field)), &error);
		append_string(&command_string, " component_names", &error);
		for (i = 0; i < number_of_components; i++)
		{
			component_name = get_FE_field_component_name(fe_field, i);
			if (component_name)
			{
				make_valid_token(&component_name);
				append_string(&command_string, " ", &error);
				append_string(&command_string, component_name, &error);
				DEALLOCATE(component_name);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_finite_element::get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_finite_element::get_command_string */

} //namespace

inline Computed_field *Computed_field_cast(
	Cmiss_field_finite_element *finite_element_field)
{
	return (reinterpret_cast<Computed_field*>(finite_element_field));
}

inline Computed_field_finite_element *Computed_field_finite_element_core_cast(
	Cmiss_field_finite_element *finite_element_field)
{
	return (static_cast<Computed_field_finite_element*>(
		reinterpret_cast<Computed_field*>(finite_element_field)->core));
}

struct Computed_field *Computed_field_create_finite_element_internal(
	struct Cmiss_field_module *field_module, struct FE_field *fe_field)
{
	char **component_names;
	int i, number_of_components, return_code;
	struct Computed_field *field = NULL;

	ENTER(Computed_field_create_finite_element_internal);
	if (field_module && fe_field)
	{
		return_code = 1;
		Cmiss_region *region = Cmiss_field_module_get_region_internal(field_module);
		FE_region *fe_region = Cmiss_region_get_FE_region(region);
		if (FE_field_get_FE_region(fe_field) != fe_region)
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_create_finite_element_internal.  Region mismatch");
			return_code = 0;
		}
		number_of_components = get_FE_field_number_of_components(fe_field);
		/* 1. make dynamic allocations for any new type-specific data */
		if (ALLOCATE(component_names, char *, number_of_components))
		{
			for (i = 0 ; i < number_of_components; i++)
			{
				if (!(component_names[i]=get_FE_field_component_name(fe_field,i)))
				{
					return_code = 0;
				}
			}
		}
		else
		{
			return_code = 0;
		}
		if (return_code)
		{
			field = Computed_field_create_generic(field_module,
				/*check_source_field_regions*/true, number_of_components,
				/*number_of_source_fields*/0, NULL,
				/*number_of_source_values*/0, NULL,
				new Computed_field_finite_element(fe_field));
			if (field)
			{
				// following should be a function call
				field->component_names = component_names;
			}
			else
			{
				return_code = 0;
			}
		}
		if ((!return_code) && (component_names))
		{
			for (i = 0 ; i < number_of_components ; i++)
			{
				DEALLOCATE(component_names[i]);
			}
			DEALLOCATE(component_names);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_create_finite_element_internal.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (field);
}

Cmiss_field_id Cmiss_field_module_create_finite_element_internal(
	Cmiss_field_module_id field_module, enum Value_type value_type, int number_of_components)
{
	Cmiss_field_id field = 0;
	// cache changes to ensure FE_field not automatically wrapped already
	Cmiss_field_module_begin_change(field_module);
	FE_region *fe_region = Cmiss_region_get_FE_region(Cmiss_field_module_get_region_internal(field_module));
	// ensure FE_field and Computed_field have same name
	char *field_name = Cmiss_field_module_get_field_name(field_module);
	bool no_default_name = (0 == field_name);
	if (no_default_name)
	{
		field_name = Cmiss_field_module_get_unique_field_name(field_module);
		Cmiss_field_module_set_field_name(field_module, field_name);
	}
	FE_field *fe_field = FE_region_get_FE_field_with_general_properties(
		fe_region, field_name, value_type, number_of_components);
	if (fe_field)
	{
		Coordinate_system coordinate_system = Cmiss_field_module_get_coordinate_system(field_module);
		set_FE_field_coordinate_system(fe_field, &coordinate_system);
		field = Computed_field_create_generic(field_module,
			/*check_source_field_regions*/false, number_of_components,
			/*number_of_source_fields*/0, NULL,
			/*number_of_source_values*/0, NULL,
			new Computed_field_finite_element(fe_field));
	}
	DEALLOCATE(field_name);
	if (no_default_name)
	{
		Cmiss_field_module_set_field_name(field_module, /*field_name*/0);
	}
	Cmiss_field_module_end_change(field_module);
	return (field);
}

Cmiss_field_id Cmiss_field_module_create_finite_element(
	Cmiss_field_module_id field_module, int number_of_components)
{
	Computed_field *field = NULL;
	if (field_module && (0 < number_of_components))
	{
		field = Cmiss_field_module_create_finite_element_internal(
			field_module, FE_VALUE_VALUE, number_of_components);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_field_module_create_finite_element.  Invalid argument(s)");
	}
	return (field);
}

Cmiss_field_finite_element_id Cmiss_field_cast_finite_element(Cmiss_field_id field)
{
	if (field)
	{
		Computed_field_finite_element* core =
			dynamic_cast<Computed_field_finite_element*>(field->core);
		if (core &&
			(get_FE_field_FE_field_type(core->fe_field) == GENERAL_FE_FIELD) &&
			(get_FE_field_value_type(core->fe_field) == FE_VALUE_VALUE))
		{
			Cmiss_field_access(field);
			return (reinterpret_cast<Cmiss_field_finite_element_id>(field));
		}
	}
	return 0;
}

int Cmiss_field_finite_element_destroy(
	Cmiss_field_finite_element_id *finite_element_field_address)
{
	return Cmiss_field_destroy(reinterpret_cast<Cmiss_field_id *>(finite_element_field_address));
}

Cmiss_field_id Cmiss_field_module_create_stored_mesh_location(
	Cmiss_field_module_id field_module, Cmiss_mesh_id mesh)
{
	Computed_field *field = NULL;
	if (field_module && mesh && (Cmiss_mesh_get_master_region_internal(mesh) ==
		Cmiss_field_module_get_master_region_internal(field_module)))
	{
		field = Cmiss_field_module_create_finite_element_internal(
			field_module, ELEMENT_XI_VALUE, /*number_of_components*/1);
		struct FE_field *fe_field = 0;
		Computed_field_get_type_finite_element(field, &fe_field);
		FE_field_set_element_xi_mesh_dimension(fe_field, Cmiss_mesh_get_dimension(mesh));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_field_module_create_finite_element.  Invalid argument(s)");
	}
	return (field);
}

Cmiss_field_stored_mesh_location_id Cmiss_field_cast_stored_mesh_location(Cmiss_field_id field)
{
	if (field)
	{
		Computed_field_finite_element* core =
			dynamic_cast<Computed_field_finite_element*>(field->core);
		if (core &&
			(get_FE_field_FE_field_type(core->fe_field) == GENERAL_FE_FIELD) &&
			(get_FE_field_value_type(core->fe_field) == ELEMENT_XI_VALUE))
		{
			Cmiss_field_access(field);
			return (reinterpret_cast<Cmiss_field_stored_mesh_location_id>(field));
		}
	}
	return 0;
}

int Cmiss_field_stored_mesh_location_destroy(
	Cmiss_field_stored_mesh_location_id *stored_mesh_location_field_address)
{
	return Cmiss_field_destroy(
		reinterpret_cast<Cmiss_field_id *>(stored_mesh_location_field_address));
}

Cmiss_field_id Cmiss_field_module_create_stored_string(
	Cmiss_field_module_id field_module)
{
	return Cmiss_field_module_create_finite_element_internal(
		field_module, STRING_VALUE, /*number_of_components*/1);
}

Cmiss_field_stored_string_id Cmiss_field_cast_stored_string(Cmiss_field_id field)
{
	if (field)
	{
		Computed_field_finite_element* core =
			dynamic_cast<Computed_field_finite_element*>(field->core);
		if (core &&
			(get_FE_field_FE_field_type(core->fe_field) == GENERAL_FE_FIELD) &&
			(get_FE_field_value_type(core->fe_field) == STRING_VALUE))
		{
			Cmiss_field_access(field);
			return (reinterpret_cast<Cmiss_field_stored_string_id>(field));
		}
	}
	return 0;
}

int Cmiss_field_stored_string_destroy(
	Cmiss_field_stored_string_id *stored_string_field_address)
{
	return Cmiss_field_destroy(
		reinterpret_cast<Cmiss_field_id *>(stored_string_field_address));
}

int Computed_field_is_type_finite_element(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

DESCRIPTION :
Returns true if <field> has the appropriate static type string.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_type_finite_element);
	if (field)
	{
		if (dynamic_cast<Computed_field_finite_element*>(field->core))
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
			"Computed_field_is_type_finite_element.  Missing field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_is_type_finite_element */

int Computed_field_get_type_finite_element(struct Computed_field *field,
	struct FE_field **fe_field)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
If the field is of type COMPUTED_FIELD_FINITE_ELEMENT, the FE_field being
"wrapped" by it is returned - otherwise an error is reported.
==============================================================================*/
{
	Computed_field_finite_element* core;
	int return_code;

	ENTER(Computed_field_get_type_finite_element);
	if (field&&(core=dynamic_cast<Computed_field_finite_element*>(field->core)))
	{
		*fe_field=core->fe_field;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_finite_element.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_finite_element */

namespace {

const char computed_field_cmiss_number_type_string[] = "cmiss_number";

class Computed_field_cmiss_number : public Computed_field_core
{
public:
	Computed_field_cmiss_number() : Computed_field_core()
	{
	};

private:
	Computed_field_core *copy()
	{
		return new Computed_field_cmiss_number();
	}

	const char *get_type_string()
	{
		return(computed_field_cmiss_number_type_string);
	}

	int compare(Computed_field_core* other_field)
	{
		if (dynamic_cast<Computed_field_cmiss_number*>(other_field))
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}

	int evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache);

	int list();

	char* get_command_string();
};

int Computed_field_cmiss_number::evaluate(Cmiss_field_cache& cache,
	FieldValueCache& inValueCache)
{
	int return_code = 1;
	RealFieldValueCache& valueCache = RealFieldValueCache::cast(inValueCache);
	Field_element_xi_location *element_xi_location;
	Field_node_location *node_location;
	if (0 != (element_xi_location = dynamic_cast<Field_element_xi_location*>(cache.getLocation())))
	{
		FE_element* element = element_xi_location->get_element();
		CM_element_information cm;
		get_FE_element_identifier(element, &cm);
		valueCache.values[0] = (FE_value)cm.number;
		/* derivatives are always zero for this type, hence always calculated */
		int element_dimension = get_FE_element_dimension(element);
		for (int i = 0; i < element_dimension; i++)
		{
			valueCache.derivatives[i] = 0.0;
		}
		valueCache.derivatives_valid = 1;
	}
	else if (0 != (node_location = dynamic_cast<Field_node_location*>(cache.getLocation())))
	{
		FE_node *node = node_location->get_node();
		valueCache.values[0] = (FE_value)get_FE_node_identifier(node);
		valueCache.derivatives_valid = 0;
	}
	else
	{
		// Location type unknown or not implemented
		return_code = 0;
	}
	return (return_code);
}

int Computed_field_cmiss_number::list()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_cmiss_number);
	if (field)
	{
		/* no extra parameters */
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_cmiss_number.  Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_cmiss_number */

char *Computed_field_cmiss_number::get_command_string()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string;

	ENTER(Computed_field_cmiss_number::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		/* no command options */
		command_string = duplicate_string(computed_field_cmiss_number_type_string);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_cmiss_number::get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_cmiss_number::get_command_string */

} //namespace

int Computed_field_is_type_cmiss_number(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

DESCRIPTION :
Returns true if <field> has the appropriate static type string.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_type_cmiss_number);
	if (field)
	{
		if (dynamic_cast<Computed_field_cmiss_number*>(field->core))
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
			"Computed_field_is_type_cmiss_number.  Missing field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_is_type_cmiss_number */

struct Computed_field *Computed_field_create_cmiss_number(
	struct Cmiss_field_module *field_module)
{
	Computed_field *field = Computed_field_create_generic(field_module,
		/*check_source_field_regions*/true,
		/*number_of_components*/1,
		/*number_of_source_fields*/0, NULL,
		/*number_of_source_values*/0, NULL,
		new Computed_field_cmiss_number());

	return (field);
}

#if defined (COMPUTED_FIELD_ACCESS_COUNT)
namespace {

const char computed_field_access_count_type_string[] = "access_count";

class Computed_field_access_count : public Computed_field_core
{
public:
	Computed_field_access_count() : Computed_field_core()
	{
	};

private:
	Computed_field_core *copy()
	{
		return new Computed_field_access_count();
	}

	const char *get_type_string()
	{
		return(computed_field_access_count_type_string);
	}

	int compare(Computed_field_core* other_field)
	{
		if (dynamic_cast<Computed_field_access_count*>(other_field))
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}

	int evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache);

	int list();

	char* get_command_string();
};

int Computed_field_access_count::evaluate(Cmiss_field_cache& cache,
	FieldValueCache& inValueCache)
{
	RealFieldValueCache& valueCache = RealFieldValueCache::cast(inValueCache);
	Field_element_xi_location *element_xi_location;
	Field_node_location *node_location;
	if (0 != (element_xi_location = dynamic_cast<Field_element_xi_location*>(cache.getLocation())))
	{
		FE_element* element = element_xi_location->get_element();
		valueCache.values[0] = (FE_value)FE_element_get_access_count(element);
	}
	else if (0 != (node_location = dynamic_cast<Field_node_location*>(cache.getLocation())))
	{
		FE_node *node = node_location->get_node();
		valueCache.values[0] = (FE_value)FE_node_get_access_count(node);
	}
	else
	{
		valueCache.values[0] = 0;
	}
	valueCache.derivatives_valid = 0;
	return 1;
}

int Computed_field_access_count::list()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_access_count);
	if (field)
	{
		/* no extra parameters */
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_access_count.  Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_access_count */

char *Computed_field_access_count::get_command_string()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string;

	ENTER(Computed_field_access_count::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		/* no command options */
		command_string = duplicate_string(computed_field_access_count_type_string);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_access_count::get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_access_count::get_command_string */

} //namespace

/*****************************************************************************//**
 * Creates a field which returns the element or node access count as its value.
 *
 * @experimental
 * @param field_module  Region field module which will own new field.
 * @return Newly created field
 */
struct Computed_field *Computed_field_create_access_count(
	struct Cmiss_field_module *field_module)
{
	Computed_field *field = Computed_field_create_generic(field_module,
		/*check_source_field_regions*/true,
		/*number_of_components*/1,
		/*number_of_source_fields*/0, NULL,
		/*number_of_source_values*/0, NULL,
		new Computed_field_access_count());

	return (field);
}

#endif /* defined (COMPUTED_FIELD_ACCESS_COUNT) */

namespace {

const char computed_field_node_value_type_string[] = "node_value";

class Computed_field_node_value : public Computed_field_core
{
public:
	Cmiss_field_id finite_element_field;
	struct FE_field *fe_field;
	enum FE_nodal_value_type nodal_value_type;
	int version_number;

	Computed_field_node_value(Cmiss_field_id finite_element_field,
			enum FE_nodal_value_type nodal_value_type, int version_number) :
		Computed_field_core(),
		finite_element_field(Cmiss_field_access(finite_element_field)),
		fe_field(0),
		nodal_value_type(nodal_value_type),
		version_number(version_number)
	{
		Computed_field_get_type_finite_element(finite_element_field, &fe_field);
		ACCESS(FE_field)(fe_field);
	};

	virtual ~Computed_field_node_value();

	virtual void inherit_source_field_attributes()
	{
		if (field)
		{
			Computed_field_set_coordinate_system(field,
				get_FE_field_coordinate_system(fe_field));
		}
	}

private:
	Computed_field_core *copy();

	const char *get_type_string()
	{
		return(computed_field_node_value_type_string);
	}

	int compare(Computed_field_core* other_field);

	int evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache);

	int list();

	char* get_command_string();

	virtual bool is_defined_at_location(Cmiss_field_cache& cache);

	int has_numerical_components();

	enum FieldAssignmentResult assign(Cmiss_field_cache& cache, RealFieldValueCache& valueCache);

	int has_multiple_times();
};

Computed_field_node_value::~Computed_field_node_value()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Clear the type specific data used by this type.
==============================================================================*/
{

	ENTER(Computed_field_node_value::~Computed_field_node_value);
	if (field)
	{
		DEACCESS(FE_field)(&(fe_field));
		Cmiss_field_destroy(&finite_element_field);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_node_value::~Computed_field_node_value.  "
			"Invalid arguments.");
	}
	LEAVE;

} /* Computed_field_node_value::~Computed_field_node_value */

Computed_field_core* Computed_field_node_value::copy()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Copy the type specific data used by this type.
==============================================================================*/
{
	Computed_field_node_value* core =
		new Computed_field_node_value(finite_element_field, nodal_value_type, version_number);

	return (core);
} /* Computed_field_node_value::copy */

int Computed_field_node_value::compare(Computed_field_core *other_core)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Compare the type specific data
==============================================================================*/
{
	Computed_field_node_value* other;
	int return_code;

	ENTER(Computed_field_node_value::compare);
	if (field && (other = dynamic_cast<Computed_field_node_value*>(other_core)))
	{
		return_code = ((fe_field == other->fe_field)
			&& (nodal_value_type == other->nodal_value_type)
			&& (version_number == other->version_number));
	}
	else
	{
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_node_value::compare */

bool Computed_field_node_value::is_defined_at_location(Cmiss_field_cache& cache)
{
	Field_node_location *node_location;
	if (0 != (node_location = dynamic_cast<Field_node_location*>(cache.getLocation())))
	{
		FE_node *node = node_location->get_node();
		if (FE_field_is_defined_at_node(fe_field, node))
		{
			/* must ensure at least one component of version_number,
				nodal_value_type defined at node */
			for (int i = 0; i < field->number_of_components; ++i)
			{
				if (FE_nodal_value_version_exists(node, fe_field, /*component_number*/i,
					version_number, nodal_value_type))
				{
					return true;
				}
			}
		}
	}
	return false;
}

int Computed_field_node_value::has_numerical_components()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_node_value::has_numerical_components);
	if (field)
	{
		return_code=Value_type_is_numeric_simple(
			get_FE_field_value_type(fe_field));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_node_value::has_numerical_components.  Missing field");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_node_value::has_numerical_components */

int Computed_field_node_value::evaluate(Cmiss_field_cache& cache,
	FieldValueCache& inValueCache)
{
	int return_code = 1;
	RealFieldValueCache& valueCache = RealFieldValueCache::cast(inValueCache);
	Field_node_location *node_location = dynamic_cast<Field_node_location*>(cache.getLocation());
	if (node_location)
	{
		FE_node *node = node_location->get_node();
		FE_value time = node_location->get_time();
		double double_value;
		float float_value;
		int i, int_value, return_code = 1;
		short short_value;

		enum Value_type value_type = get_FE_field_value_type(fe_field);
		for (i=0;(i<field->number_of_components)&&return_code;i++)
		{
			if (FE_nodal_value_version_exists(node,fe_field,/*component_number*/i,
					version_number,nodal_value_type))
			{
				switch (value_type)
				{
					case DOUBLE_VALUE:
					{
						return_code=get_FE_nodal_double_value(node,
							fe_field,/*component_number*/i, version_number,
							nodal_value_type,time,&double_value);
						valueCache.values[i] = (FE_value)double_value;
					} break;
					case FE_VALUE_VALUE:
					{
						return_code=get_FE_nodal_FE_value_value(node,
							fe_field,/*component_number*/i, version_number,
							nodal_value_type,time,&(valueCache.values[i]));
					} break;
					case FLT_VALUE:
					{
						return_code=get_FE_nodal_float_value(node,
							fe_field,/*component_number*/i, version_number,
							nodal_value_type,time,&float_value);
						valueCache.values[i] = (FE_value)float_value;
					} break;
					case INT_VALUE:
					{
						return_code=get_FE_nodal_int_value(node,fe_field,/*component_number*/i,
							version_number,nodal_value_type,time,&int_value);
						valueCache.values[i] = (FE_value)int_value;
					} break;
					case SHORT_VALUE:
					{
						return_code=get_FE_nodal_short_value(node,fe_field,/*component_number*/i,
							version_number,nodal_value_type,time,&short_value);
						valueCache.values[i] = (FE_value)short_value;
					} break;
					default:
					{
						display_message(ERROR_MESSAGE,
							"Computed_field_node_value::evaluate.  "
							"Unsupported value type %s in node_value field",
							Value_type_string(value_type));
						return_code=0;
					} break;
				}
			}
			else
			{
				/* use 0 for all undefined components */
				valueCache.values[i]=0.0;
			}
		}
	}
	else
	{
		// Only valid for Field_node_location type
		return_code = 0;
	}
	return (return_code);
}

enum FieldAssignmentResult Computed_field_node_value::assign(Cmiss_field_cache& cache, RealFieldValueCache& valueCache)
{
	Field_node_location *node_location = dynamic_cast<Field_node_location*>(cache.getLocation());
	if (node_location)
	{
		if (cache.assignInCacheOnly())
		{
			return FIELD_ASSIGNMENT_RESULT_ALL_VALUES_SET;
		}
		// clear finite element field cache due to DOFs changing (wasteful if data points changed):
		finite_element_field->clearCaches();
		FieldAssignmentResult result = FIELD_ASSIGNMENT_RESULT_ALL_VALUES_SET;
		FE_node *node = node_location->get_node();
		FE_value time = node_location->get_time();
		enum Value_type value_type = get_FE_field_value_type(fe_field);
		for (int i=0; (i<field->number_of_components); i++)
		{
			/* only set nodal value/versions that exist */
			if (FE_nodal_value_version_exists(node,fe_field,/*component_number*/i,
					version_number,nodal_value_type))
			{
				int return_code = 0;
				switch (value_type)
				{
					case DOUBLE_VALUE:
					{
						double double_value = (double)valueCache.values[i];
						return_code=set_FE_nodal_double_value(node,fe_field,/*component_number*/i,
							version_number,nodal_value_type,time,double_value);
					} break;
					case FE_VALUE_VALUE:
					{
						return_code=set_FE_nodal_FE_value_value(node,fe_field,/*component_number*/i,
							version_number,nodal_value_type,time,valueCache.values[i]);
					} break;
					case FLT_VALUE:
					{
						float float_value=(float)valueCache.values[i];
						return_code=set_FE_nodal_float_value(node,fe_field,/*component_number*/i,
							version_number,nodal_value_type,time,float_value);
					} break;
					case INT_VALUE:
					{
						result = FIELD_ASSIGNMENT_RESULT_PARTIAL_VALUES_SET;
						int int_value=(int)floor(valueCache.values[i]+0.5);
						return_code=set_FE_nodal_int_value(node,fe_field,/*component_number*/i,
							version_number,nodal_value_type,time,int_value);
					} break;
					default:
					{
					} break;
				}
				if (!return_code)
					return FIELD_ASSIGNMENT_RESULT_FAIL;
			}
		}
		return result;
	}
	return FIELD_ASSIGNMENT_RESULT_FAIL;
}

int Computed_field_node_value::list()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	char *field_name;
	int return_code;

	ENTER(List_Computed_field_node_value);
	if (field)
	{
		return_code=GET_NAME(FE_field)(fe_field,&field_name);
		if (return_code)
		{
			display_message(INFORMATION_MESSAGE,"    fe_field : %s\n",field_name);
			display_message(INFORMATION_MESSAGE,"    nodal value type : %s\n",
				ENUMERATOR_STRING(FE_nodal_value_type)(nodal_value_type));
			display_message(INFORMATION_MESSAGE,"    version : %d\n",
				version_number+1);
			DEALLOCATE(field_name);
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_node_value.  Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_node_value */

char *Computed_field_node_value::get_command_string()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name, temp_string[40];
	int error;

	ENTER(Computed_field_node_value::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_node_value_type_string, &error);
		append_string(&command_string, " fe_field ", &error);
		if (GET_NAME(FE_field)(fe_field, &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
		append_string(&command_string, " ", &error);
		append_string(&command_string,
			ENUMERATOR_STRING(FE_nodal_value_type)(nodal_value_type), &error);
		sprintf(temp_string, " version %d", version_number + 1);
		append_string(&command_string, temp_string, &error);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_node_value::get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_node_value::get_command_string */

int Computed_field_node_value::has_multiple_times()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Check the fe_field
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_node_value::has_multiple_times);
	if (field)
	{
		return_code=FE_field_has_multiple_times(fe_field);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_node_value::has_multiple_times.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_node_value::has_multiple_times */

} //namespace

/*****************************************************************************//**
 * Creates a field returning the values for the given <nodal_value_type> and
 * <version_number> of <fe_field> at a node.
 * Makes the number of components the same as in the <fe_field>.
 * Field automatically takes the coordinate system of the source fe_field.
 *
 * @param field_module  Region field module which will own new field.
 * @param fe_field  FE_field whose nodal parameters will be extracted
 * @param nodal_value_type  Parameter value type to extract
 * @param version_number  Version of parameter value to extract.
 * @return Newly created field
 */

struct Computed_field *Computed_field_create_node_value(
	struct Cmiss_field_module *field_module,
	Cmiss_field_id finite_element_field, enum FE_nodal_value_type nodal_value_type,
	int version_number)
{
	char **component_names;
	int i, number_of_components, return_code;
	struct Computed_field *field = NULL;

	ENTER(Computed_field_create_node_value);
	struct FE_field *fe_field = 0;
	if (finite_element_field && finite_element_field->isNumerical() &&
		Computed_field_get_type_finite_element(finite_element_field, &fe_field) && fe_field)
	{
		return_code=1;
		number_of_components = get_FE_field_number_of_components(fe_field);
		/* 1. make dynamic allocations for any new type-specific data */
		if (ALLOCATE(component_names, char *, number_of_components))
		{
			for (i = 0 ; i < number_of_components; i++)
			{
				if (!(component_names[i]=get_FE_field_component_name(fe_field,i)))
				{
					return_code = 0;
				}
			}
		}
		else
		{
			return_code = 0;
		}
		if (return_code)
		{
			field = Computed_field_create_generic(field_module,
				/*check_source_field_regions*/true, number_of_components,
				/*number_of_source_fields*/1, &finite_element_field,
				/*number_of_source_values*/0, NULL,
				new Computed_field_node_value(
					finite_element_field, nodal_value_type, version_number));
			if (field)
			{
				// following should be a function call
				field->component_names = component_names;
			}
			else
			{
				return_code = 0;
			}
		}
		if ((!return_code) && (component_names))
		{
			for (i = 0 ; i < number_of_components ; i++)
			{
				DEALLOCATE(component_names[i]);
			}
			DEALLOCATE(component_names);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_create_node_value.  Invalid argument(s)");
	}
	LEAVE;

	return (field);
} /* Computed_field_create_node_value */

int Computed_field_get_type_node_value(struct Computed_field *field,
	Cmiss_field_id *finite_element_field_address, enum FE_nodal_value_type *nodal_value_type,
	int *version_number)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
If the field is of type COMPUTED_FIELD_NODE_VALUE, the FE_field being
"wrapped" by it is returned - otherwise an error is reported.
==============================================================================*/
{
	Computed_field_node_value* core;
	int return_code;

	ENTER(Computed_field_get_type_node_value);
	if (field&&(core = dynamic_cast<Computed_field_node_value*>(field->core)))
	{
		*finite_element_field_address=core->finite_element_field;
		*nodal_value_type=core->nodal_value_type;
		*version_number=core->version_number;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_node_value.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_node_value */

namespace {

const char computed_field_embedded_type_string[] = "embedded";

class Computed_field_embedded : public Computed_field_core
{
public:
	Computed_field_embedded() :
		Computed_field_core()
	{
	};

	virtual void inherit_source_field_attributes()
	{
		if (field)
		{
			Computed_field_set_coordinate_system_from_sources(field);
		}
	}

private:
	Computed_field_core *copy();

	const char *get_type_string()
	{
		return(computed_field_embedded_type_string);
	}

	int compare(Computed_field_core* other_field);

	virtual FieldValueCache *createValueCache(Cmiss_field_cache& parentCache)
	{
		RealFieldValueCache *valueCache = new RealFieldValueCache(field->number_of_components);
		valueCache->createExtraCache(parentCache, Computed_field_get_region(field));
		return valueCache;
	}

	int evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache);

	int list();

	char* get_command_string();

	virtual bool is_defined_at_location(Cmiss_field_cache& cache);

	int has_numerical_components();

};

Computed_field_core* Computed_field_embedded::copy()
{
	return new Computed_field_embedded();
}

int Computed_field_embedded::compare(Computed_field_core *other_core)
{
	return (field && (0 != dynamic_cast<Computed_field_embedded*>(other_core)));
}

bool Computed_field_embedded::is_defined_at_location(Cmiss_field_cache& cache)
{
	return (0 != field->evaluate(cache));
}

int Computed_field_embedded::has_numerical_components()
{
	return (field && Computed_field_has_numerical_components(
		field->source_fields[0],(void *)NULL));
}

int Computed_field_embedded::evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache)
{
	// assumes mesh-location valued fields all create MeshLocationFieldValueCache
	MeshLocationFieldValueCache *meshLocationValueCache = MeshLocationFieldValueCache::cast(getSourceField(1)->evaluate(cache));
	if (meshLocationValueCache)
	{
		RealFieldValueCache& valueCache = RealFieldValueCache::cast(inValueCache);
		Cmiss_field_cache& extraCache = *valueCache.getExtraCache();
		extraCache.setMeshLocation(meshLocationValueCache->element, meshLocationValueCache->xi);
		extraCache.setTime(cache.getTime());
		RealFieldValueCache *sourceValueCache = RealFieldValueCache::cast(getSourceField(0)->evaluate(extraCache));
		if (sourceValueCache)
		{
			valueCache.copyValues(*sourceValueCache);
			return 1;
		}
	}
	return 0;
}

int Computed_field_embedded::list()
{
	int return_code;
	if (field)
	{
		display_message(INFORMATION_MESSAGE, "    embedded location field : %s\n", field->source_fields[1]->name);
		display_message(INFORMATION_MESSAGE, "    source field : %s\n", field->source_fields[0]->name);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_embedded.  Invalid arguments.");
		return_code = 0;
	}
	return (return_code);
}

/** Returns allocated command string for reproducing field. Includes type. */
char *Computed_field_embedded::get_command_string()
{
	char *command_string, *field_name;
	int error;

	ENTER(Computed_field_embedded::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_embedded_type_string, &error);
		append_string(&command_string, " element_xi ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[1], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
		append_string(&command_string, " field ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[0], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_embedded::get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
}

} //namespace

Cmiss_field_id Cmiss_field_module_create_embedded(
	Cmiss_field_module_id field_module, Cmiss_field_id source_field,
	Cmiss_field_id embedded_location_field)
{
	struct Computed_field *field = 0;
	if (field_module && embedded_location_field && source_field &&
		(CMISS_FIELD_VALUE_TYPE_MESH_LOCATION ==
			Cmiss_field_get_value_type(embedded_location_field)) &&
		Computed_field_has_numerical_components(source_field, NULL))
	{
		Cmiss_field_id source_fields[2];
		source_fields[0] = source_field;
		source_fields[1] = embedded_location_field;
		field = Computed_field_create_generic(field_module,
			/*check_source_field_regions*/true,
			source_field->number_of_components,
			/*number_of_source_fields*/2, source_fields,
			/*number_of_source_values*/0, NULL,
			new Computed_field_embedded());
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_create_embedded.  Invalid argument(s)");
	}
	return (field);
}

/** If the field is of type COMPUTED_FIELD_EMBEDDED, returns the fields it uses. */
int Computed_field_get_type_embedded(struct Computed_field *field,
	struct Computed_field **source_field_address,
	struct Computed_field **embedded_location_field_address)
{
	int return_code = 0;
	if (field && (0 != dynamic_cast<Computed_field_embedded*>(field->core)))
	{
		*source_field_address = field->source_fields[0];
		*embedded_location_field_address = field->source_fields[1];
		return_code = 1;
	}
	return (return_code);
}

namespace {

const char computed_field_find_mesh_location_type_string[] = "find_mesh_location";

class Computed_field_find_mesh_location : public Computed_field_core
{
private:
	Cmiss_mesh_id mesh;
	enum Cmiss_field_find_mesh_location_search_mode search_mode;

public:

	Computed_field_find_mesh_location(Cmiss_mesh_id mesh) :
		Computed_field_core(),
		mesh(Cmiss_mesh_access(mesh)),
		search_mode(CMISS_FIELD_FIND_MESH_LOCATION_SEARCH_MODE_FIND_EXACT)
	{
	};

	virtual ~Computed_field_find_mesh_location();

	Cmiss_field_id get_source_field()
	{
		return field->source_fields[0];
	}

	Cmiss_field_id get_mesh_field()
	{
		return field->source_fields[1];
	}

	Cmiss_mesh_id get_mesh()
	{
		return mesh;
	}

	enum Cmiss_field_find_mesh_location_search_mode get_search_mode() const
	{
		return search_mode;
	}

	int set_search_mode(enum Cmiss_field_find_mesh_location_search_mode search_mode_in)
	{
		if (search_mode_in != search_mode)
		{
			search_mode = search_mode_in;
			Computed_field_changed(field);
		}
		return 1;
	}

private:
	Computed_field_core *copy();

	const char *get_type_string()
	{
		return (computed_field_find_mesh_location_type_string);
	}

	int compare(Computed_field_core* other_field);

	virtual FieldValueCache *createValueCache(Cmiss_field_cache& parentCache)
	{
		MeshLocationFieldValueCache *valueCache = new MeshLocationFieldValueCache();
		valueCache->createExtraCache(parentCache, Computed_field_get_region(field));
		return valueCache;
	}

	int evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache);

	int list();

	char* get_command_string();

	virtual bool is_defined_at_location(Cmiss_field_cache& cache);

	int has_numerical_components()
	{
		return 0;
	}

	virtual Cmiss_field_value_type get_value_type() const
	{
		return CMISS_FIELD_VALUE_TYPE_MESH_LOCATION;
	}

};

Computed_field_find_mesh_location::~Computed_field_find_mesh_location()
{
	Cmiss_mesh_destroy(&mesh);
}

Computed_field_core* Computed_field_find_mesh_location::copy()
{
	return new Computed_field_find_mesh_location(mesh);
}

int Computed_field_find_mesh_location::compare(Computed_field_core *other_core)
{
	Computed_field_find_mesh_location* other;
	int return_code = 0;
	if (field && (other = dynamic_cast<Computed_field_find_mesh_location*>(other_core)))
	{
		return_code = (mesh == other->mesh);
	}
	return (return_code);
}

bool Computed_field_find_mesh_location::is_defined_at_location(Cmiss_field_cache& cache)
{
	return (0 != field->evaluate(cache));
}

int Computed_field_find_mesh_location::evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache)
{
	int return_code = 0;
	RealFieldValueCache *sourceValueCache = RealFieldValueCache::cast(get_source_field()->evaluateNoDerivatives(cache));
	if (sourceValueCache)
	{
		MeshLocationFieldValueCache& meshLocationValueCache = MeshLocationFieldValueCache::cast(inValueCache);
		if (meshLocationValueCache.element)
		{
			Cmiss_element_destroy(&meshLocationValueCache.element);
		}
		Cmiss_field_cache& extraCache = *meshLocationValueCache.getExtraCache();
		extraCache.setTime(cache.getTime());
		if (Computed_field_find_element_xi(get_mesh_field(), &extraCache,
			sourceValueCache->values, sourceValueCache->componentCount, &meshLocationValueCache.element,
			meshLocationValueCache.xi, mesh, /*propagate_field*/0,
			/*find_nearest*/(search_mode != CMISS_FIELD_FIND_MESH_LOCATION_SEARCH_MODE_FIND_EXACT))
			&& meshLocationValueCache.element)
		{
			Cmiss_element_access(meshLocationValueCache.element);
			return_code = 1;
		}
	}
	return (return_code);
}

int Computed_field_find_mesh_location::list()
{
	int return_code = 0;
	if (field)
	{
		display_message(INFORMATION_MESSAGE, "    search mode : ");
		if (search_mode == CMISS_FIELD_FIND_MESH_LOCATION_SEARCH_MODE_FIND_NEAREST)
		{
			display_message(INFORMATION_MESSAGE, " find_nearest\n");
		}
		else
		{
			display_message(INFORMATION_MESSAGE, " find_exact\n");
		}
		display_message(INFORMATION_MESSAGE, "    mesh : ");
		char *mesh_name = Cmiss_mesh_get_name(mesh);
		display_message(INFORMATION_MESSAGE, "%s\n", mesh_name);
		DEALLOCATE(mesh_name);
		display_message(INFORMATION_MESSAGE,
			"    mesh field : %s\n", get_mesh_field()->name);
		display_message(INFORMATION_MESSAGE,
			"    source field : %s\n", get_source_field()->name);
		return_code = 1;
	}
	return (return_code);
}

/** Returns allocated command string for reproducing field. Includes type */
char *Computed_field_find_mesh_location::get_command_string()
{
	char *command_string = 0;
	int error = 0;
	if (field)
	{
		append_string(&command_string, computed_field_find_mesh_location_type_string, &error);

		if (search_mode == CMISS_FIELD_FIND_MESH_LOCATION_SEARCH_MODE_FIND_NEAREST)
		{
			append_string(&command_string, " find_nearest", &error);
		}
		else
		{
			append_string(&command_string, " find_exact", &error);
		}

		append_string(&command_string, " mesh ", &error);
		char *mesh_name = Cmiss_mesh_get_name(mesh);
		append_string(&command_string, mesh_name, &error);
		DEALLOCATE(mesh_name);

		char *mesh_field_name = Cmiss_field_get_name(get_mesh_field());
		make_valid_token(&mesh_field_name);
		append_string(&command_string, " mesh_field ", &error);
		append_string(&command_string, mesh_field_name, &error);
		DEALLOCATE(mesh_field_name);

		char *source_field_name = Cmiss_field_get_name(get_source_field());
		make_valid_token(&source_field_name);
		append_string(&command_string, " source_field ", &error);
		append_string(&command_string, source_field_name, &error);
		DEALLOCATE(source_field_name);
	}
	return (command_string);
}

} // namespace

Cmiss_field_id Cmiss_field_module_create_find_mesh_location(
	Cmiss_field_module_id field_module, Cmiss_field_id source_field,
	Cmiss_field_id mesh_field, Cmiss_mesh_id mesh)
{
	struct Computed_field *field = NULL;
	int number_of_source_field_components = Computed_field_get_number_of_components(source_field);
	int number_of_mesh_field_components = Computed_field_get_number_of_components(mesh_field);
	if (field_module && source_field && mesh_field && mesh &&
		(number_of_source_field_components == number_of_mesh_field_components) &&
		Computed_field_has_numerical_components(source_field, NULL) &&
		Computed_field_has_numerical_components(mesh_field, NULL) &&
		(number_of_mesh_field_components >= Cmiss_mesh_get_dimension(mesh)) &&
		(Cmiss_field_module_get_master_region_internal(field_module) ==
			Cmiss_mesh_get_master_region_internal(mesh)))
	{
		Cmiss_field_id source_fields[2];
		source_fields[0] = source_field;
		source_fields[1] = mesh_field;
		field = Computed_field_create_generic(field_module,
			/*check_source_field_regions*/true,
			/*number_of_components*/1,
			/*number_of_source_fields*/2, source_fields,
			/*number_of_source_values*/0, NULL,
			new Computed_field_find_mesh_location(mesh));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_field_module_create_find_mesh_location.  Invalid argument(s)");
	}
	return (field);
}

struct Cmiss_field_find_mesh_location : private Computed_field
{
	inline Computed_field_find_mesh_location *get_core()
	{
		return static_cast<Computed_field_find_mesh_location*>(core);
	}
};

Cmiss_field_find_mesh_location_id Cmiss_field_cast_find_mesh_location(
	Cmiss_field_id field)
{
	if (field)
	{
		if (dynamic_cast<Computed_field_find_mesh_location*>(field->core))
		{
			Cmiss_field_access(field);
			return (reinterpret_cast<Cmiss_field_find_mesh_location_id>(field));
		}
	}
	return 0;
}

int Cmiss_field_find_mesh_location_destroy(
	Cmiss_field_find_mesh_location_id *find_mesh_location_field_address)
{
	return Cmiss_field_destroy(reinterpret_cast<Cmiss_field_id *>(find_mesh_location_field_address));
}

Cmiss_mesh_id Cmiss_field_find_mesh_location_get_mesh(
	Cmiss_field_find_mesh_location_id find_mesh_location_field)
{
	Cmiss_mesh_id mesh = 0;
	if (find_mesh_location_field)
	{
		mesh = find_mesh_location_field->get_core()->get_mesh();
		Cmiss_mesh_access(mesh);
	}
	return mesh;
}

class Cmiss_field_find_mesh_location_search_mode_conversion
{
public:
	static const char *to_string(enum Cmiss_field_find_mesh_location_search_mode mode)
	{
		const char *enum_string = 0;
		switch (mode)
		{
			case CMISS_FIELD_FIND_MESH_LOCATION_SEARCH_MODE_FIND_EXACT:
				enum_string = "FIND_EXACT";
				break;
			case CMISS_FIELD_FIND_MESH_LOCATION_SEARCH_MODE_FIND_NEAREST:
				enum_string = "FIND_NEAREST";
				break;
			default:
				break;
		}
		return enum_string;
	}
};

enum Cmiss_field_find_mesh_location_search_mode
	Cmiss_field_find_mesh_location_search_mode_enum_from_string(const char *string)
{
	return string_to_enum<enum Cmiss_field_find_mesh_location_search_mode,
		Cmiss_field_find_mesh_location_search_mode_conversion>(string);
}

char *Cmiss_field_find_mesh_location_search_mode_enum_to_string(
	enum Cmiss_field_find_mesh_location_search_mode mode)
{
	const char *mode_string = Cmiss_field_find_mesh_location_search_mode_conversion::to_string(mode);
	return (mode_string ? duplicate_string(mode_string) : 0);
}

enum Cmiss_field_find_mesh_location_search_mode
	Cmiss_field_find_mesh_location_get_search_mode(
		Cmiss_field_find_mesh_location_id find_mesh_location_field)
{
	Cmiss_field_find_mesh_location_search_mode search_mode =
		CMISS_FIELD_FIND_MESH_LOCATION_SEARCH_MODE_FIND_EXACT;
	if (find_mesh_location_field)
	{
		search_mode = find_mesh_location_field->get_core()->get_search_mode();
	}
	return search_mode;
}

int Cmiss_field_find_mesh_location_set_search_mode(
	Cmiss_field_find_mesh_location_id find_mesh_location_field,
	enum Cmiss_field_find_mesh_location_search_mode search_mode)
{
	int return_code = 0;
	if (find_mesh_location_field)
	{
		return_code = find_mesh_location_field->get_core()->set_search_mode(search_mode);
	}
	return return_code;
}

namespace {

const char computed_field_xi_coordinates_type_string[] = "xi_coordinates";

class Computed_field_xi_coordinates : public Computed_field_core
{
public:
	Computed_field_xi_coordinates() : Computed_field_core()
	{
	};

private:
	Computed_field_core *copy()
	{
		return new Computed_field_xi_coordinates();
	}

	const char *get_type_string()
	{
		return(computed_field_xi_coordinates_type_string);
	}

	int compare(Computed_field_core* other_field)
	{
		if (dynamic_cast<Computed_field_xi_coordinates*>(other_field))
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}

	int evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache);

	int list();

	char* get_command_string();

	virtual bool is_defined_at_location(Cmiss_field_cache& cache);
};

bool Computed_field_xi_coordinates::is_defined_at_location(Cmiss_field_cache& cache)
{
	return (0 != dynamic_cast<Field_element_xi_location*>(cache.getLocation()));
}

int Computed_field_xi_coordinates::evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache)
{
	Field_element_xi_location *element_xi_location = dynamic_cast<Field_element_xi_location*>(cache.getLocation());
	if (element_xi_location)
	{
		RealFieldValueCache& valueCache = RealFieldValueCache::cast(inValueCache);
		FE_element* element = element_xi_location->get_element();
		const FE_value* xi = element_xi_location->get_xi();

		/* returns the values in xi, up to the element_dimension and padded
			with zeroes */
		int element_dimension = get_FE_element_dimension(element);
		FE_value *temp = valueCache.derivatives;
		for (int i=0;i<field->number_of_components;i++)
		{
			if (i<element_dimension)
			{
				valueCache.values[i]=xi[i];
			}
			else
			{
				valueCache.values[i]=0.0;
			}
			for (int j=0;j<element_dimension;j++)
			{
				if (i==j)
				{
					*temp = 1.0;
				}
				else
				{
					*temp = 0.0;
				}
				temp++;
			}
		}
		/* derivatives are always calculated since they are merely part of
			the identity matrix */
		valueCache.derivatives_valid=1;
		return 1;
	}
	return 0;
}

int Computed_field_xi_coordinates::list()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_xi_coordinates);
	if (field)
	{
		/* no extra parameters */
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_xi_coordinates.  Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_xi_coordinates */

char *Computed_field_xi_coordinates::get_command_string()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string;

	ENTER(Computed_field_xi_coordinates::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		/* no command options */
		command_string =
			duplicate_string(computed_field_xi_coordinates_type_string);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_xi_coordinates::get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_xi_coordinates::get_command_string */

} // namespace

int Computed_field_is_type_xi_coordinates(struct Computed_field *field,
	void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Returns true if <field> has the appropriate static type string.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_type_xi_coordinates);
	USE_PARAMETER(dummy_void);
	if (field)
	{
		if (dynamic_cast<Computed_field_xi_coordinates*>(field->core))
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
			"Computed_field_is_type_xi_coordinates.  Missing field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_is_type_xi_coordinates */

struct Computed_field *Computed_field_create_xi_coordinates(
	struct Cmiss_field_module *field_module)
{
	Computed_field *field = Computed_field_create_generic(field_module,
		/*check_source_field_regions*/true,
		/*number_of_components*/3,
		/*number_of_source_fields*/0, NULL,
		/*number_of_source_values*/0, NULL,
		new Computed_field_xi_coordinates());

	return (field);
}

namespace {

const char computed_field_basis_derivative_type_string[] = "basis_derivative";

class Computed_field_basis_derivative : public Computed_field_core
{
public:
	FE_field* fe_field;
	int order;
	int *xi_indices;

	Computed_field_basis_derivative(
		FE_field *fe_field, int order, int *xi_indices_in) :
		Computed_field_core(), fe_field(ACCESS(FE_field)(fe_field)),
		order(order)
	{
		int i;

		xi_indices = new int[order];
		for (i = 0 ; i < order ; i++)
		{
			xi_indices[i] = xi_indices_in[i];
		}
	};

	virtual ~Computed_field_basis_derivative();

private:
	Computed_field_core *copy();

	const char *get_type_string()
	{
		return(computed_field_basis_derivative_type_string);
	}

	int compare(Computed_field_core* other_field);

	virtual FieldValueCache *createValueCache(Cmiss_field_cache& /*parentCache*/)
	{
		return new FiniteElementRealFieldValueCache(field->number_of_components);
	}

	int evaluate(Cmiss_field_cache& cache, FieldValueCache& valueCache);

	int list();

	char* get_command_string();

	virtual bool is_defined_at_location(Cmiss_field_cache& cache);

	int has_multiple_times();

	int has_numerical_components();
};

Computed_field_basis_derivative::~Computed_field_basis_derivative()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Clear the type specific data used by this type.
==============================================================================*/
{

	ENTER(Computed_field_basis_derivative::~Computed_field_basis_derivative);
	if (field)
	{
		delete [] xi_indices;
		if (fe_field)
		{
			DEACCESS(FE_field)(&(fe_field));
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_basis_derivative::~Computed_field_basis_derivative.  "
			"Invalid arguments.");
	}
	LEAVE;

} /* Computed_field_basis_derivative::~Computed_field_basis_derivative */

Computed_field_core* Computed_field_basis_derivative::copy()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Copy the type specific data used by this type.
==============================================================================*/
{
	Computed_field_basis_derivative* core =
		new Computed_field_basis_derivative(fe_field, order, xi_indices);

	return (core);
} /* Computed_field_basis_derivative::copy */

int Computed_field_basis_derivative::compare(Computed_field_core *other_core)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Compare the type specific data
==============================================================================*/
{
	Computed_field_basis_derivative* other;
	int return_code;

	ENTER(Computed_field_basis_derivative::compare);
	if (field && (other = dynamic_cast<Computed_field_basis_derivative*>(other_core)))
	{
		return_code = (fe_field == other->fe_field);
	}
	else
	{
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_basis_derivative::compare */

bool Computed_field_basis_derivative::is_defined_at_location(Cmiss_field_cache& cache)
{
	Field_element_xi_location *element_xi_location;
	if (0 != (element_xi_location = dynamic_cast<Field_element_xi_location*>(cache.getLocation())))
	{
		return (0 != FE_field_is_defined_in_element(fe_field, element_xi_location->get_element()));
	}
	return false;
}

int Computed_field_basis_derivative::has_multiple_times()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Check the fe_field
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_basis_derivative::has_multiple_times);
	if (field)
	{
		return_code=FE_field_has_multiple_times(fe_field);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_basis_derivative::has_multiple_times.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_basis_derivative::has_multiple_times */

int Computed_field_basis_derivative::has_numerical_components()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_basis_derivative::has_numerical_components);
	if (field)
	{
		return_code=Value_type_is_numeric_simple(
			get_FE_field_value_type(fe_field));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_basis_derivative::has_numerical_components.  Missing field");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_basis_derivative::has_numerical_components */

int Computed_field_basis_derivative::evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache)
{
	FiniteElementRealFieldValueCache& feValueCache = FiniteElementRealFieldValueCache::cast(inValueCache);
	Field_element_xi_location* element_xi_location;
	if (0 != (element_xi_location = dynamic_cast<Field_element_xi_location*>(cache.getLocation())))
	{
		FE_element* element = element_xi_location->get_element();
		FE_element* top_level_element = element_xi_location->get_top_level_element();
		FE_value time = element_xi_location->get_time();
		const FE_value* xi = element_xi_location->get_xi();
		int number_of_derivatives = cache.getRequestedDerivatives();

		if (calculate_FE_element_field_values_for_element(
			feValueCache.field_values_cache, feValueCache.fe_element_field_values,
			fe_field, /*derivatives_required*/1, element, time, top_level_element, order, xi_indices))
		{
			int return_code = 1;
			/* component number -1 = calculate all components */
			enum Value_type value_type=get_FE_field_value_type(fe_field);
			switch (value_type)
			{
				case FE_VALUE_VALUE:
				case SHORT_VALUE:
				{
					if (number_of_derivatives)
					{
						return_code=calculate_FE_element_field(-1,
							feValueCache.fe_element_field_values,xi,feValueCache.values,
							feValueCache.derivatives);
						feValueCache.derivatives_valid = (0<number_of_derivatives);
					}
					else
					{
						return_code=calculate_FE_element_field(-1,
							feValueCache.fe_element_field_values,xi,feValueCache.values,
							(FE_value *)NULL);
					}
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"Computed_field_basis_derivative::evaluate.  "
						"Unsupported value type %s in basis_derivative field",
						Value_type_string(value_type));
					return_code=0;
				} break;
			}
			return return_code;
		}
	}
	return 0;
}

int Computed_field_basis_derivative::list()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	char *field_name;
	int return_code;
	int i;

	ENTER(List_Computed_field_basis_derivative);
	if (field)
	{
		return_code=GET_NAME(FE_field)(fe_field,&field_name);
		if (return_code)
		{
			display_message(INFORMATION_MESSAGE,"    fe_field : %s\n",field_name);
			display_message(INFORMATION_MESSAGE,"    order : %d\n",order);
			display_message(INFORMATION_MESSAGE,"    xi_indices : ");
			for (i=0;i<order;i++)
			{
				display_message(INFORMATION_MESSAGE," %d",xi_indices[i]+1);
			}
			display_message(INFORMATION_MESSAGE,"\n");

			DEALLOCATE(field_name);
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_basis_derivative.  Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_basis_derivative */

char *Computed_field_basis_derivative::get_command_string()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name, temp_string[40];
	int error, i;

	ENTER(Computed_field_basis_derivative::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{

		error = 0;
		append_string(&command_string,
			computed_field_basis_derivative_type_string, &error);

		append_string(&command_string, " fe_field ", &error);
		if (GET_NAME(FE_field)(fe_field, &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}

		append_string(&command_string, " order", &error);
		sprintf(temp_string, " %d", order);
		append_string(&command_string, temp_string, &error);

		append_string(&command_string, " xi_indices", &error);
		for (i = 0; i < order; i++)
		{
			sprintf(temp_string, " %d", xi_indices[i]+1);
			append_string(&command_string, temp_string, &error);
		}

	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_basis_derivative::get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_basis_derivative::get_command_string */

} //namespace

int Computed_field_is_type_basis_derivative(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

DESCRIPTION :
Returns true if <field> has the appropriate static type string.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_type_basis_derivative);
	if (field)
	{
		if (dynamic_cast<Computed_field_basis_derivative*>(field->core))
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
			"Computed_field_is_type_basis_derivative.  Missing field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_is_type_basis_derivative */

/*****************************************************************************//**
 * Creates a field giving arbitrary order derivatives of a finite element field.
 * Modifies the calculated monomial coefficients by differentiating them wrt
 * to the xi directions in accordance with the vector of <xi_indices> which is
 * length <order>.
 *
 * @param field_module  Region field module which will own new field.
 * @param fe_field  FE_field to return derivatives w.r.t. xi for.
 * @param order  The order of the derivatives.
 * @param xi_indices  Array of length order giving the xi indices the derivative
 * is calculated with respect to.
 * @return Newly created field
 */
Cmiss_field_id Cmiss_field_module_create_basis_derivative(
	Cmiss_field_module_id field_module, Cmiss_field_id finite_element_field,
	int order, int *xi_indices)
{
	Cmiss_field_id field = 0;
	struct FE_field *fe_field = 0;
	if (finite_element_field && finite_element_field->isNumerical() &&
		Computed_field_get_type_finite_element(finite_element_field, &fe_field) && fe_field &&
		(order > 0) && xi_indices)
	{
		int return_code=1;
		int number_of_components = get_FE_field_number_of_components(fe_field);
		char **component_names = 0;
		if (ALLOCATE(component_names, char *, number_of_components))
		{
			for (int i = 0 ; i < number_of_components; i++)
			{
				if (!(component_names[i]=get_FE_field_component_name(fe_field,i)))
				{
					return_code = 0;
				}
			}
		}
		else
		{
			return_code = 0;
		}
		if (return_code)
		{
			field = Computed_field_create_generic(field_module,
				/*check_source_field_regions*/true, number_of_components,
				/*number_of_source_fields*/1, &finite_element_field,
				/*number_of_source_values*/0, NULL,
				new Computed_field_basis_derivative(
					fe_field, order, xi_indices));
			if (field)
			{
				// following should be a function call
				field->component_names = component_names;
			}
			else
			{
				return_code = 0;
			}
		}
		if ((!return_code) && (component_names))
		{
			for (int i = 0 ; i < number_of_components ; i++)
			{
				DEALLOCATE(component_names[i]);
			}
			DEALLOCATE(component_names);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_field_module_create_basis_derivative.  Invalid argument(s)");
	}
	return (field);
}

/***************************************************************************//**
 * Ensures there is an up-to-date computed field wrapper for <fe_field>.
 * @param fe_field  Field to wrap.
 * @param field_change_data_void FE_field_to_Computed_field_change_data.
 */
int FE_field_to_Computed_field_change(struct FE_field *fe_field,
	int change, void *region_void)
{
	int return_code = 1;
	Cmiss_region_id region = reinterpret_cast<Cmiss_region_id>(region_void);
	if (fe_field && region)
	{
		if (change &
			(CHANGE_LOG_OBJECT_IDENTIFIER_CHANGED(FE_field) |
			 CHANGE_LOG_OBJECT_NOT_IDENTIFIER_CHANGED(FE_field) |
			 CHANGE_LOG_OBJECT_ADDED(FE_field)))
		{
			char *field_name = NULL;
			GET_NAME(FE_field)(fe_field, &field_name);
			int update_wrapper = (change != CHANGE_LOG_OBJECT_IDENTIFIER_CHANGED(FE_field));
			Cmiss_field_id existing_wrapper = FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
				field_name, Cmiss_region_get_Computed_field_manager(region));
			if (existing_wrapper && !Computed_field_wraps_fe_field(existing_wrapper, (void *)fe_field))
			{
				existing_wrapper = FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
					Computed_field_wraps_fe_field, (void *)fe_field,
					Cmiss_region_get_Computed_field_manager(region));
				update_wrapper = 1;
			}
			if (update_wrapper)
			{
				Cmiss_field_module *field_module = Cmiss_field_module_create(region);
				if (existing_wrapper)
				{
					Cmiss_field_module_set_replace_field(field_module, existing_wrapper);
				}
				else
				{
					Cmiss_field_module_set_field_name(field_module, field_name);
					struct Coordinate_system *coordinate_system = get_FE_field_coordinate_system(fe_field);
					if (coordinate_system)
					{
						Cmiss_field_module_set_coordinate_system(field_module, *coordinate_system);
					}
				}
				Cmiss_field_id field = Computed_field_create_finite_element_internal(field_module, fe_field);
				Cmiss_field_set_attribute_integer(field, CMISS_FIELD_ATTRIBUTE_IS_MANAGED, 1);
				Cmiss_field_destroy(&field);
				Cmiss_field_module_destroy(&field_module);
				char *new_field_name = NULL;
				GET_NAME(FE_field)(fe_field, &new_field_name);
				if (strcmp(new_field_name, field_name))
				{
					display_message(WARNING_MESSAGE, "Renamed finite element field %s to %s as another field is already using that name.",
						field_name, new_field_name);
				}
				DEALLOCATE(new_field_name);
			}
			DEALLOCATE(field_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_field_to_Computed_field_change.  Invalid argument(s)");
		return_code = 0;
	}
	return (return_code);
}

void Cmiss_region_FE_region_change(struct FE_region *fe_region,
	struct FE_region_changes *changes, void *cmiss_region_void)
{
	ENTER(Cmiss_region_FE_region_change);
	Cmiss_region *cmiss_region = reinterpret_cast<Cmiss_region *>(cmiss_region_void);
	if (fe_region && changes && cmiss_region)
	{
		int field_change_summary;
		CHANGE_LOG_GET_CHANGE_SUMMARY(FE_field)(changes->fe_field_changes,
			&field_change_summary);
		int check_field_wrappers = field_change_summary & (
			CHANGE_LOG_OBJECT_ADDED(FE_field) |
			CHANGE_LOG_OBJECT_IDENTIFIER_CHANGED(FE_field) |
			CHANGE_LOG_OBJECT_NOT_IDENTIFIER_CHANGED(FE_field));
		int add_cmiss_number_field = FE_region_need_add_cmiss_number_field(fe_region);
		int add_xi_field = FE_region_need_add_xi_field(fe_region);
		if (check_field_wrappers || add_cmiss_number_field || add_xi_field)
		{
			Cmiss_field_module_id field_module = Cmiss_region_get_field_module(cmiss_region);
			Cmiss_field_module_begin_change(field_module);

			if (check_field_wrappers)
			{
				CHANGE_LOG_FOR_EACH_OBJECT(FE_field)(changes->fe_field_changes,
					FE_field_to_Computed_field_change, (void *)cmiss_region);
			}
			if (add_cmiss_number_field)
			{
				const char *cmiss_number_field_name = "cmiss_number";
				Cmiss_field_id field = Cmiss_field_module_find_field_by_name(field_module, cmiss_number_field_name);
				if (!field)
				{
					field = Computed_field_create_cmiss_number(field_module);
					Cmiss_field_set_name(field, cmiss_number_field_name);
					Cmiss_field_set_attribute_integer(field, CMISS_FIELD_ATTRIBUTE_IS_MANAGED, 1);
				}
				Cmiss_field_destroy(&field);
			}
			if (add_xi_field)
			{
				const char *xi_field_name = "xi";
				Cmiss_field_id field = Cmiss_field_module_find_field_by_name(field_module, xi_field_name);
				if (!field)
				{
					field = Computed_field_create_xi_coordinates(field_module);
					Cmiss_field_set_name(field, xi_field_name);
					Cmiss_field_set_attribute_integer(field, CMISS_FIELD_ATTRIBUTE_IS_MANAGED, 1);
				}
				Cmiss_field_destroy(&field);
			}
			Cmiss_field_module_end_change(field_module);
			Cmiss_field_module_destroy(&field_module);
		}
		if (field_change_summary & CHANGE_LOG_OBJECT_REMOVED(FE_field))
		{
			/* Currently we do nothing as the computed field wrapper is destroyed
				before the FE_field is removed from the manager.  This is not necessary
				and this response could be to delete the wrapper. */
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_region_FE_region_change.  Invalid argument(s)");
	}
	LEAVE;
} /* Cmiss_region_FE_region_change */

int Computed_field_contains_changed_FE_field(
	struct Computed_field *field, void *fe_field_change_log_void)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Returns true if <field> directly contains an FE_field and it is listed as
changed, added or removed in <fe_field_change_log>.
<fe_field_change_log_void> must point at a struct CHANGE_LOG<FE_field>.
==============================================================================*/
{
	int fe_field_change;
	enum FE_nodal_value_type nodal_value_type;
	int return_code, version_number;
	struct CHANGE_LOG(FE_field) *fe_field_change_log;
	struct FE_field *fe_field;

	ENTER(Computed_field_contains_changed_FE_field);
	if (field && (fe_field_change_log =
		(struct CHANGE_LOG(FE_field) *)fe_field_change_log_void))
	{
		if (dynamic_cast<Computed_field_finite_element*>(field->core))
		{
			return_code = Computed_field_get_type_finite_element(field, &fe_field);
		}
		else if (dynamic_cast<Computed_field_node_value*>(field->core))
		{
			Cmiss_field_id finite_element_field = 0;
			return_code = Computed_field_get_type_node_value(field, &finite_element_field,
					&nodal_value_type, &version_number) &&
				Computed_field_get_type_finite_element(field, &fe_field);
		}
		else
		{
			return_code = 0;
		}
		if (return_code)
		{
			return_code = CHANGE_LOG_QUERY(FE_field)(fe_field_change_log, fe_field,
				&fe_field_change) &&
				(fe_field_change != CHANGE_LOG_OBJECT_UNCHANGED(FE_field));
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_contains_changed_FE_field.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_contains_changed_FE_field */

int Computed_field_add_source_FE_field_to_list(
	struct Computed_field *field, void *fe_field_list_void)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
If <field> has a source FE_field, ensures it is in <fe_field_list>.
==============================================================================*/
{
	enum FE_nodal_value_type nodal_value_type;
	int return_code, version_number;
	struct FE_field *fe_field;
	struct LIST(FE_field) *fe_field_list;

	ENTER(Computed_field_add_source_FE_field_to_list);
	if (field && (fe_field_list = (struct LIST(FE_field) *)fe_field_list_void))
	{
		fe_field = (struct FE_field *)NULL;
		if (dynamic_cast<Computed_field_finite_element*>(field->core))
		{
			return_code = Computed_field_get_type_finite_element(field, &fe_field);
		}
		else if (dynamic_cast<Computed_field_node_value*>(field->core))
		{
			Cmiss_field_id finite_element_field = 0;
			return_code = Computed_field_get_type_node_value(field, &finite_element_field,
					&nodal_value_type, &version_number) &&
				Computed_field_get_type_finite_element(field, &fe_field);
		}
		else
		{
			return_code = 1;
		}
		if (return_code && fe_field)
		{
			if (!IS_OBJECT_IN_LIST(FE_field)(fe_field, fe_field_list))
			{
				return_code = ADD_OBJECT_TO_LIST(FE_field)(fe_field, fe_field_list);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_add_source_FE_field_to_list.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_add_source_FE_field_to_list */

struct LIST(FE_field)
	*Computed_field_get_defining_FE_field_list(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Returns the list of FE_fields that <field> depends on.
==============================================================================*/
{
	struct LIST(FE_field) *fe_field_list;

	ENTER(Computed_field_get_defining_FE_field_list);
	fe_field_list = (struct LIST(FE_field) *)NULL;
	if (field)
	{
		fe_field_list = CREATE(LIST(FE_field))();
		if (fe_field_list)
		{
			if (!Computed_field_for_each_ancestor(field,
				Computed_field_add_source_FE_field_to_list, (void *)fe_field_list))
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_get_defining_FE_field_list.  Failed");
				DESTROY(LIST(FE_field))(&fe_field_list);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_defining_FE_field_list.  Invalid argument(s)");
	}
	LEAVE;

	return (fe_field_list);
} /* Computed_field_get_defining_FE_field_list */

struct LIST(FE_field)
	*Computed_field_array_get_defining_FE_field_list(
		int number_of_fields, struct Computed_field **field_array)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Returns the compiled list of FE_fields that are required by any of
the <number_of_fields> fields in <field_array>.
==============================================================================*/
{
	int i;
	struct LIST(FE_field) *additional_fe_field_list, *fe_field_list;

	ENTER(Computed_field_get_defining_FE_field_list);
	fe_field_list = (struct LIST(FE_field) *)NULL;
	if ((0 < number_of_fields) && field_array)
	{
		fe_field_list = Computed_field_get_defining_FE_field_list(field_array[0]);
		for (i = 1 ; i < number_of_fields ; i++)
		{
			additional_fe_field_list = Computed_field_get_defining_FE_field_list(field_array[i]);
			FOR_EACH_OBJECT_IN_LIST(FE_field)(ensure_FE_field_is_in_list,
				(void *)fe_field_list, additional_fe_field_list);
			DESTROY(LIST(FE_field))(&additional_fe_field_list);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_array_get_defining_FE_field_list.  Invalid argument(s)");
	}
	LEAVE;

	return (fe_field_list);
} /* Computed_field_array_get_defining_FE_field_list */

int Computed_field_is_type_finite_element_iterator(
	struct Computed_field *field, void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 16 March 2007

DESCRIPTION :
Iterator/conditional function returning true if <field> is read only and a
wrapper for an FE_field.
==============================================================================*/
{
	int return_code;

	USE_PARAMETER(dummy_void);
	ENTER(Computed_field_is_type_finite_element_iterator);
	if (field)
	{
		return_code = Computed_field_is_type_finite_element(field);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_type_finite_element_iterator.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_is_type_finite_element_iterator */

int Computed_field_wraps_fe_field(
	struct Computed_field *field, void *fe_field_void)
{
	int return_code = 0;

	ENTER(Computed_field_wraps_fe_field);
	struct FE_field *fe_field = (struct FE_field *)fe_field_void;
	if (field && fe_field)
	{
		Computed_field_finite_element* core =
			dynamic_cast<Computed_field_finite_element*>(field->core);
		if (core)
		{
			return_code = (fe_field == core->fe_field);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_wraps_fe_field.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
}

int Computed_field_has_coordinate_fe_field(struct Computed_field *field,
	void *dummy)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Iterator/conditional function returning true if <field> is a wrapper for a
coordinate type fe_field.
==============================================================================*/
{
	Computed_field_finite_element* core;
	int return_code;

	ENTER(Computed_field_has_coordinate_fe_field);
	USE_PARAMETER(dummy);
	if (field)
	{
		return_code = 0;
		core = dynamic_cast<Computed_field_finite_element*>(field->core);
		if (core)
		{
			return_code = FE_field_is_coordinate_field(core->fe_field, NULL);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_has_coordinate_fe_field.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_has_coordinate_fe_field */

int Computed_field_has_element_xi_fe_field(struct Computed_field *field,
	void *dummy)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Iterator/conditional function returning true if <field> is a wrapper for an
element_xi type fe_field.
==============================================================================*/
{
	Computed_field_finite_element* core;
	int return_code;

	ENTER(Computed_field_has_element_xi_fe_field);
	USE_PARAMETER(dummy);
	if (field)
	{
		return_code = 0;
		core = dynamic_cast<Computed_field_finite_element*>(field->core);
		if (core)
		{
			enum Value_type field_value_type = get_FE_field_value_type(core->fe_field);
			return_code = (field_value_type == ELEMENT_XI_VALUE);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_has_element_xi_fe_field.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_has_element_xi_fe_field */

int Computed_field_is_scalar_integer(struct Computed_field *field,
	void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Returns true if <field> is a 1 integer component FINITE_ELEMENT wrapper.
==============================================================================*/
{
	Computed_field_finite_element* core;
	int return_code;

	ENTER(Computed_field_is_scalar_integer);
	USE_PARAMETER(dummy_void);
	if (field)
	{
		if((1==field->number_of_components)&&
			(core = dynamic_cast<Computed_field_finite_element*>(field->core)))
		{
			return_code = (INT_VALUE==get_FE_field_value_type(core->fe_field));
		}
		else
		{
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_scalar_integer.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_is_scalar_integer */

int Computed_field_is_scalar_integer_grid_in_element(
	struct Computed_field *field,void *element_void)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Returns true if <field> is a 1 integer component FINITE_ELEMENT wrapper which
is defined in <element> AND is grid-based.
Used for choosing field suitable for identifying grid points.
==============================================================================*/
{
	Computed_field_finite_element* core;
	int return_code;
	struct FE_element *element;

	ENTER(Computed_field_is_scalar_integer_grid_in_element);
	if (field&&(element=(struct FE_element *)element_void))
	{
		if ((1==field->number_of_components)&&
			(core=dynamic_cast<Computed_field_finite_element*>(field->core)))
		{
			return_code = (INT_VALUE==get_FE_field_value_type(core->fe_field))&&
				Computed_field_is_defined_in_element(field,element)&&
				FE_element_field_is_grid_based(element,core->fe_field);
		}
		else
		{
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_scalar_integer_grid_in_element.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_is_scalar_integer_grid_in_element */

struct FE_time_sequence *Computed_field_get_FE_node_field_FE_time_sequence(
	 struct Computed_field *field, struct FE_node *node)
/*******************************************************************************
LAST MODIFIED : 22 Feb 2008

DESCRIPTION :
Returns the <fe_time_sequence> corresponding to the <node> and <field>.  If the
<node> and <field> have no time dependence then the function will return NULL.
==============================================================================*/
{
	 FE_time_sequence *time_sequence;
	 FE_field *fe_field;
	 struct LIST(FE_field) *fe_field_list;

	 ENTER(Computed_field_get_FE_node_field_FE_time_sequence);
	 time_sequence = (FE_time_sequence *)NULL;
	 if (field)
	 {
			fe_field_list = Computed_field_get_defining_FE_field_list(field);
			if (fe_field_list)
			{
				 if (NUMBER_IN_LIST(FE_field)(fe_field_list) == 1)
				 {
						fe_field = FIRST_OBJECT_IN_LIST_THAT(FE_field)(
							 (LIST_CONDITIONAL_FUNCTION(FE_field) *)NULL, (void *)NULL,
							 fe_field_list);
						time_sequence = get_FE_node_field_FE_time_sequence(node,
							 fe_field);
				 }
				 else
				 {
						display_message(ERROR_MESSAGE,
							 "Computed_field_get_FE_node_field_FE_time_sequence. None or"
							 "more than one FE element field is used to define this"
							 "computed field, this function expects only one finite element"
							 "field at the corresponding node otherwise it may contain more than"
							 "one time sequence./n");
				 }
				 DESTROY(LIST(FE_field))(&fe_field_list);
			}
			else
			{
						display_message(ERROR_MESSAGE,
							 "Computed_field_get_FE_node_field_FE_time_sequence. Cannot get the"
							 "FE field list /n");
			}
	}
	else
	{
		 display_message(ERROR_MESSAGE,
				"Computed_field_get_FE_node_field_FE_time_sequence.  Invalid argument(s)");
		 time_sequence = (FE_time_sequence *)NULL;
	}
	LEAVE;

	return (time_sequence);
} /* Computed_field_get_FE_node_field_FE_time_sequence */

Cmiss_field_id Cmiss_field_module_create_node_value(
	Cmiss_field_module_id field_module, Cmiss_field_id field,
	enum Cmiss_nodal_value_type type, int version)
{
	if (field_module && field && (Computed_field_is_type_finite_element(field)) && (version > 0))
	{
		enum FE_nodal_value_type fe_nodal_value_type = FE_NODAL_UNKNOWN;
		switch (type)
		{
			case CMISS_NODAL_VALUE_TYPE_INVALID:
				fe_nodal_value_type = FE_NODAL_UNKNOWN;
				break;
			case CMISS_NODAL_VALUE:
				fe_nodal_value_type = FE_NODAL_VALUE;
				break;
			case CMISS_NODAL_D_DS1:
				fe_nodal_value_type = FE_NODAL_D_DS1;
				break;
			case CMISS_NODAL_D_DS2:
				fe_nodal_value_type = FE_NODAL_D_DS2;
				break;
			case CMISS_NODAL_D_DS3:
				fe_nodal_value_type = FE_NODAL_D_DS3;
				break;
			case CMISS_NODAL_D2_DS1DS2:
				fe_nodal_value_type = FE_NODAL_D2_DS1DS2;
				break;
			case CMISS_NODAL_D2_DS1DS3:
				fe_nodal_value_type = FE_NODAL_D2_DS1DS3;
				break;
			case CMISS_NODAL_D2_DS2DS3:
				fe_nodal_value_type = FE_NODAL_D2_DS2DS3;
				break;
			case CMISS_NODAL_D3_DS1DS2DS3:
				fe_nodal_value_type = FE_NODAL_D3_DS1DS2DS3;
				break;
		}
		if (FE_NODAL_UNKNOWN == fe_nodal_value_type)
		{
			return NULL;
		}
		return Computed_field_create_node_value(field_module, field, fe_nodal_value_type, version-1);
	}
	else
	{
		return NULL;
	}
}

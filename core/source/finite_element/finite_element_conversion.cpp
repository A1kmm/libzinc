/*******************************************************************************
FILE : finite_element_conversion.cpp

LAST MODIFIED : 5 April 2006

DESCRIPTION :
Functions for converting one finite_element representation to another.
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
/* for IGES */
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "zinc/fieldmodule.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_finite_element.h"
#include "computed_field/computed_field_wrappers.h"
#include "finite_element/finite_element.h"
#include "finite_element/finite_element_helper.h"
#include "finite_element/finite_element_region.h"
#include "finite_element/finite_element_conversion.h"
#include "general/debug.h"
#include "general/octree.h"
#include "general/enumerator_private.hpp"
#include "region/cmiss_region.h"
#include "general/message.h"
#include "general/mystring.h"

/*
Module types
------------
*/

struct Convert_finite_elements_data
{
	Cmiss_field_module_id source_field_module;
	Cmiss_field_cache_id source_field_cache;
	enum Convert_finite_elements_mode mode;
	Element_refinement refinement;
	FE_value tolerance;
	struct Octree *octree;
	struct LIST(Octree_object) *nearby_nodes;
	struct FE_node *template_node;
	struct FE_element *template_element;
	int number_of_fields;
	struct Computed_field **field_array;
	struct FE_field **destination_fe_fields;
	int maximum_number_of_components;
	FE_value *temporary_values;
	struct FE_region *destination_fe_region;
	int mode_dimension;
	int subelement_count;
	FE_value delta_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];

	int node_number; /* Do not to search from beginning for a valid node number each time */
	int element_number; /* Do not to search from beginning for a valid element number each time */

	Convert_finite_elements_data(Cmiss_region_id source_region,
			Convert_finite_elements_mode mode,
			Element_refinement refinement, FE_value tolerance) :
		source_field_module(Cmiss_region_get_field_module(source_region)),
		source_field_cache(Cmiss_field_module_create_cache(source_field_module)),
		mode(mode),
		refinement(refinement),
		tolerance(tolerance),
		octree(CREATE(Octree)()),
		nearby_nodes(CREATE(LIST(Octree_object))()),
		template_node((struct FE_node *)NULL),
		template_element((struct FE_element *)NULL),
		destination_fe_fields(NULL),
		temporary_values((FE_value *)NULL),
		mode_dimension(0),
		node_number(1),
		element_number(1)
	{
		if (CONVERT_TO_FINITE_ELEMENTS_HERMITE_2D_PRODUCT == mode)
		{
			mode_dimension = 2;
		}
		else if ((CONVERT_TO_FINITE_ELEMENTS_TRILINEAR == mode) ||
			(CONVERT_TO_FINITE_ELEMENTS_TRIQUADRATIC == mode))
		{
			mode_dimension = 3;
		}
		subelement_count = 1;
		for (int i = 0; i < mode_dimension; i++)
		{
			subelement_count *= refinement.count[i];
			delta_xi[i] = 1.0 / (FE_value)(refinement.count[i]);
		}
	}

	~Convert_finite_elements_data()
	{
		if (template_element)
		{
			DESTROY(FE_element)(&template_element);
		}
		if (template_node)
		{
			DESTROY(FE_node)(&template_node);
		}
		if (temporary_values)
		{
			DEALLOCATE(temporary_values);
		}
		DESTROY(LIST(Octree_object))(&nearby_nodes);
		DESTROY(Octree)(&octree);
		if (destination_fe_fields)
		{
			for (int i = 0; i < number_of_fields; i++)
			{
				DEACCESS(FE_field)(&destination_fe_fields[i]);
			}
			DEALLOCATE(destination_fe_fields);
		}
		Cmiss_field_cache_destroy(&source_field_cache);
		Cmiss_field_module_destroy(&source_field_module);
	}

	FE_node *getNearestNode(FE_value *coordinates)
	{
		Octree_add_objects_near_coordinate_to_list(octree,
			/*dimension*/3, coordinates, tolerance, nearby_nodes);
		if (0 == NUMBER_IN_LIST(Octree_object)(nearby_nodes))
		{
			return NULL;
		}
		struct Octree_object *nearest_octree_object =
			Octree_object_list_get_nearest(nearby_nodes, coordinates);
		REMOVE_ALL_OBJECTS_FROM_LIST(Octree_object)(nearby_nodes);
		return (struct FE_node *)Octree_object_get_user_data(nearest_octree_object);
	}

	int addNode(FE_value *coordinates, FE_node *node)
	{
		Octree_object *octree_node = CREATE(Octree_object)(/*dimension*/3, coordinates);
		Octree_object_set_user_data(octree_node, (void *)node);
		return Octree_add_object(octree, octree_node);
	}

	int convert_subelement(struct FE_element *element, int subelement_number);
}; /* struct Convert_finite_elements_data */

/**************************************************************************//**
 * Converts the specified subelement refinement of element with the current
 * mode and adds it to data->destination_fe_field.
 */
int Convert_finite_elements_data::convert_subelement(struct FE_element *element,
	int subelement_number)
{
	const int HERMITE_2D_NUMBER_OF_NODES = 4;
	const int TRILINEAR_NUMBER_OF_NODES = 8;
	const int TRIQUADRATIC_NUMBER_OF_NODES = 27;
	const int MAX_NUMBER_OF_NODES = 27;
	FE_value base_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS], *values, *derivatives,
		*nodal_values, source_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	int i, j, k, number_of_components, number_of_values, return_code = 1;
	struct CM_element_information identifier;
	struct FE_element *new_element;
	struct FE_node *nodes[MAX_NUMBER_OF_NODES];

	int inner_subelement_count = 1;
	for (int d = 0; d < mode_dimension; d++)
	{
		int e = (subelement_number / inner_subelement_count) % refinement.count[d];
		base_xi[d] = (FE_value)e / (FE_value)refinement.count[d];
		inner_subelement_count *= refinement.count[d];
	}

	switch (mode)
	{
		case CONVERT_TO_FINITE_ELEMENTS_HERMITE_2D_PRODUCT:
		{
			FE_value destination_xi[HERMITE_2D_NUMBER_OF_NODES][2] =
				{{0.0, 0.0}, {1.0, 0.0}, {0.0, 1.0}, {1.0, 1.0}};
			for (i = 0 ; return_code && (i < HERMITE_2D_NUMBER_OF_NODES) ; i++)
			{
				node_number = FE_region_get_next_FE_node_identifier(
					destination_fe_region, node_number);
				if ((nodes[i] = CREATE(FE_node)(node_number, (struct FE_region *)NULL,
						template_node)))
				{
					if (!FE_region_merge_FE_node(destination_fe_region, nodes[i]))
					{
						display_message(ERROR_MESSAGE,
							"FE_element_convert_element.  "
							"Could not merge node into region");
						return_code = 0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,"FE_element_convert_element.  "
						"Unable to create node.");
					return_code=0;
				}
			}
			for (j = 0 ; j < number_of_fields ; j++)
			{
				Computed_field *cfield = field_array[j];
				number_of_components = Computed_field_get_number_of_components(cfield);
				number_of_values = 4 * number_of_components;
				values = temporary_values;
				derivatives = temporary_values + number_of_components;
				nodal_values = temporary_values + number_of_values;
				for (i = 0 ; return_code && (i < HERMITE_2D_NUMBER_OF_NODES) ; i++)
				{
					for (int d = 0; d < mode_dimension; d++)
					{
						source_xi[d] = base_xi[d] + destination_xi[i][d]*delta_xi[d];
					}
					if (Cmiss_field_cache_set_mesh_location(source_field_cache, element, mode_dimension, source_xi) &&
						Cmiss_field_evaluate_real_with_derivatives(cfield, source_field_cache, number_of_components, values,
							mode_dimension, derivatives))
					{
						/* Reorder the separate lists of values and derivatives into
							a single mixed list */
						for (k = 0 ; k < number_of_components ; k++)
						{
							nodal_values[4 * k] = values[k];
							nodal_values[4 * k + 1] = delta_xi[0]*derivatives[2 * k];
							nodal_values[4 * k + 2] = delta_xi[1]*derivatives[2 * k + 1];
							// cannot determine cross derivative from first derivatives, so use zero:
							nodal_values[4 * k + 3] = 0.0; // nodal_values[4 * k + 1] * nodal_values[4 * k + 2];
						}
						set_FE_nodal_field_FE_value_values(destination_fe_fields[j],
							nodes[i], nodal_values, &number_of_values);
					}
					else
					{
						display_message(ERROR_MESSAGE,"FE_element_convert_element.  "
							"Field not defined.");
						return_code = 0;
					}
				}
			}
			identifier.type = CM_ELEMENT;
			identifier.number =  FE_region_get_next_FE_element_identifier(
				destination_fe_region, mode_dimension, element_number);
			element_number = identifier.number;
			if ((new_element = CREATE(FE_element)(&identifier, (struct FE_element_shape *)NULL,
					(struct FE_region *)NULL, template_element)))
			{
				/* The FE_element_define_tensor_product_basis function does not merge the
					nodes used to make it simple to add and a field to and existing node so
					we have to add the nodes for every field. */
				for (j = 0 ; return_code && (j < number_of_fields) ; j++)
				{
					for (i = 0 ; return_code && (i < HERMITE_2D_NUMBER_OF_NODES) ; i++)
					{
						if (!set_FE_element_node(new_element, j * HERMITE_2D_NUMBER_OF_NODES + i,
							nodes[i]))
						{
							display_message(ERROR_MESSAGE,"FE_element_convert_element.  "
								"Unable to set element node.");
							return_code = 0;
						}
					}
				}
				if (return_code)
				{
					if (!FE_region_merge_FE_element(destination_fe_region,
						new_element))
					{
						display_message(ERROR_MESSAGE,
							"FE_element_convert_element.  "
							"Could not merge node into region");
						return_code = 0;
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"FE_element_convert_element.  "
					"Unable to create element.");
				return_code=0;
			}
		} break;
		case CONVERT_TO_FINITE_ELEMENTS_TRILINEAR:
		case CONVERT_TO_FINITE_ELEMENTS_TRIQUADRATIC:
		{
			FE_value_triple destination_xi_trilinear[TRILINEAR_NUMBER_OF_NODES] =
			{
				{0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {1.0, 1.0, 0.0},
				{0.0, 0.0, 1.0}, {1.0, 0.0, 1.0}, {0.0, 1.0, 1.0}, {1.0, 1.0, 1.0}
			};
			FE_value_triple destination_xi_triquadratic[TRIQUADRATIC_NUMBER_OF_NODES] =
			{
				{0.0, 0.0, 0.0}, {0.5, 0.0, 0.0}, {1.0, 0.0, 0.0},
				{0.0, 0.5, 0.0}, {0.5, 0.5, 0.0}, {1.0, 0.5, 0.0},
				{0.0, 1.0, 0.0}, {0.5, 1.0, 0.0}, {1.0, 1.0, 0.0},

				{0.0, 0.0, 0.5}, {0.5, 0.0, 0.5}, {1.0, 0.0, 0.5},
				{0.0, 0.5, 0.5}, {0.5, 0.5, 0.5}, {1.0, 0.5, 0.5},
				{0.0, 1.0, 0.5}, {0.5, 1.0, 0.5}, {1.0, 1.0, 0.5},

				{0.0, 0.0, 1.0}, {0.5, 0.0, 1.0}, {1.0, 0.0, 1.0},
				{0.0, 0.5, 1.0}, {0.5, 0.5, 1.0}, {1.0, 0.5, 1.0},
				{0.0, 1.0, 1.0}, {0.5, 1.0, 1.0}, {1.0, 1.0, 1.0},
			};
			FE_value_triple *destination_xi = (mode == CONVERT_TO_FINITE_ELEMENTS_TRILINEAR) ?
				destination_xi_trilinear : destination_xi_triquadratic;
			const int number_of_local_nodes =
				(mode == CONVERT_TO_FINITE_ELEMENTS_TRILINEAR) ?
				TRILINEAR_NUMBER_OF_NODES : TRIQUADRATIC_NUMBER_OF_NODES;
			for (j = 0 ; j < number_of_fields ; j++)
			{
				Computed_field *cfield = field_array[j];
				number_of_components = Computed_field_get_number_of_components(cfield);
				number_of_values = number_of_components;
				values = temporary_values;
				for (i = 0 ; return_code && (i < number_of_local_nodes) ; i++)
				{
					for (int d = 0; d < mode_dimension; d++)
					{
						source_xi[d] = base_xi[d] + destination_xi[i][d]*delta_xi[d];
					}
					if (Cmiss_field_cache_set_mesh_location(source_field_cache, element, mode_dimension, source_xi) &&
						Cmiss_field_evaluate_real(cfield, source_field_cache, number_of_components, values))
					{
						if (j==0)
						{
							// assuming first field is coordinate field, find node with same field value
							struct FE_node *node = getNearestNode(values);
							if (node)
							{
								nodes[i] = node;
							}
							else
							{
								node_number = FE_region_get_next_FE_node_identifier(
									destination_fe_region, node_number);
								node = CREATE(FE_node)(node_number, (struct FE_region *)NULL,
									template_node);
								if (node)
								{
									set_FE_nodal_field_FE_value_values(destination_fe_fields[j],
										node, values, &number_of_values);
									if (FE_region_merge_FE_node(destination_fe_region, node))
									{
										nodes[i] = node;
										addNode(values, node);
									}
									else
									{
										display_message(ERROR_MESSAGE,
											"FE_element_convert_element.  "
											"Could not merge node into region");
										return_code = 0;
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,"FE_element_convert_element.  "
										"Unable to create node.");
									return_code=0;
								}
							}
						}
						else
						{
							set_FE_nodal_field_FE_value_values(destination_fe_fields[j],
								nodes[i], values, &number_of_values);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"FE_element_convert_element.  "
							"Field not defined.");
						return_code = 0;
					}
				}
			}
			identifier.type = CM_ELEMENT;
			identifier.number =  FE_region_get_next_FE_element_identifier(
				destination_fe_region, mode_dimension, element_number);
			element_number = identifier.number;
			if ((new_element = CREATE(FE_element)(&identifier, (struct FE_element_shape *)NULL,
					(struct FE_region *)NULL, template_element)))
			{
				/* The FE_element_define_tensor_product_basis function does not merge the
					nodes used to make it simple to add and a field to and existing node so
					we have to add the nodes for every field. */
				j = 0;
				//for (j = 0 ; return_code && (j < number_of_fields) ; j++)
				{
					for (i = 0 ; return_code && (i < number_of_local_nodes) ; i++)
					{
						if (!set_FE_element_node(new_element, j * number_of_local_nodes + i,
							nodes[i]))
						{
							display_message(ERROR_MESSAGE,"FE_element_convert_element.  "
								"Unable to set element node.");
							return_code = 0;
						}
					}
				}
				if (return_code)
				{
					if (!FE_region_merge_FE_element(destination_fe_region,
						new_element))
					{
						display_message(ERROR_MESSAGE,
							"FE_element_convert_element.  "
							"Could not merge node into region");
						return_code = 0;
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"FE_element_convert_element.  "
					"Unable to create element.");
				return_code=0;
			}
		} break;
		default:
		{
			return_code = 0;
		}
	}
	return return_code;
}

/*
Module functions
----------------
*/

/**************************************************************************//**
 * Converts all subelements of element as specified by data->mode and adds
 * them to data->destination_fe_field.
 */
static int FE_element_convert_element(struct FE_element *element,
	void *data_void)
{
	int dimension, return_code;
	struct Convert_finite_elements_data *data;

	ENTER(FE_element_convert_element);
	if (element && (data = (struct Convert_finite_elements_data *)data_void))
	{
		return_code = 1;
		dimension = get_FE_element_dimension(element);
		if (dimension == data->mode_dimension)
		{
			for (int i = 0; (i < data->subelement_count) && return_code; i++)
			{
				return_code = data->convert_subelement(element, i);
			}
		}
	}
	else
	{
		return_code = 0;
	}
	LEAVE;
	
	return (return_code);
} /* FE_element_convert_element */

/*
Global functions
----------------
*/

PROTOTYPE_ENUMERATOR_STRING_FUNCTION(Convert_finite_elements_mode)
{
	const char *enumerator_string;

	ENTER(ENUMERATOR_STRING(Convert_finite_elements_mode));
	switch (enumerator_value)
	{
		case CONVERT_TO_FINITE_ELEMENTS_HERMITE_2D_PRODUCT:
		{

			enumerator_string = "convert_hermite_2D_product_elements";
		} break;
		case CONVERT_TO_FINITE_ELEMENTS_TRILINEAR:
		{

			enumerator_string = "convert_trilinear";
		} break;
		case CONVERT_TO_FINITE_ELEMENTS_TRIQUADRATIC:
		{
			enumerator_string = "convert_triquadratic";
		} break;
		default:
		{
			enumerator_string = (const char *)NULL;
		} break;
	}
	LEAVE;

	return (enumerator_string);
} /* ENUMERATOR_STRING(Convert_finite_elements_mode) */

DEFINE_DEFAULT_ENUMERATOR_FUNCTIONS(Convert_finite_elements_mode)

int finite_element_conversion(struct Cmiss_region *source_region,
	struct Cmiss_region *destination_region,
	enum Convert_finite_elements_mode mode, int number_of_fields, 
	struct Computed_field **field_array,
	struct Element_refinement refinement, FE_value tolerance)
{
	enum FE_nodal_value_type hermite_2d_nodal_value_types[] = {
		FE_NODAL_D_DS1,  FE_NODAL_D_DS2, FE_NODAL_D2_DS1DS2};
	int i, return_code;

	ENTER(finite_element_conversion);
	return_code=0;
	if (source_region && destination_region &&
		(Cmiss_region_get_Computed_field_manager(destination_region) !=
			Cmiss_region_get_Computed_field_manager(source_region))
		&& (0 < number_of_fields) && (field_array) && (*field_array) && (tolerance >= 0.0))
	{
		struct FE_region *destination_fe_region = Cmiss_region_get_FE_region(destination_region);
		struct FE_region *source_fe_region = Cmiss_region_get_FE_region(source_region);

		struct Convert_finite_elements_data data(source_region, mode, refinement, tolerance);
		data.maximum_number_of_components = 0; /* Initialised by iterator */
		data.destination_fe_region = destination_fe_region;
		data.field_array = field_array;

		// Create new FE_fields in destination region matching input computed fields

		data.number_of_fields = number_of_fields;
		ALLOCATE(data.destination_fe_fields, struct FE_field *, number_of_fields);
		for (int fi = 0; fi < number_of_fields; fi++)
		{
			Computed_field *field = field_array[fi];
			char *name = Cmiss_field_get_name(field);
			int number_of_components = Computed_field_get_number_of_components(field);
			if (number_of_components > data.maximum_number_of_components)
			{
				data.maximum_number_of_components = number_of_components;
			}
			char **component_names = new char*[number_of_components];
			for (int j = 0; j < number_of_components; j++)
			{
				component_names[j] = Computed_field_get_component_name(field, j);
			}
			Coordinate_system *coordinate_system = Computed_field_get_coordinate_system(field);
			enum CM_field_type cm_field_type = CM_GENERAL_FIELD;
			struct FE_field *fe_field = NULL;
			if (Computed_field_is_type_finite_element(field) &&
				Computed_field_get_type_finite_element(field, &fe_field) && (fe_field))
			{
				cm_field_type = get_FE_field_CM_field_type(fe_field);
			}
			else if (fi == 0)
			{
				cm_field_type = CM_COORDINATE_FIELD;
			}
			else if (coordinate_system->type == FIBRE)
			{
				cm_field_type = CM_ANATOMICAL_FIELD;
			}
			data.destination_fe_fields[fi] = FE_region_get_FE_field_with_properties(
				destination_fe_region, name, GENERAL_FE_FIELD,
				/*indexer*/NULL, /*number_of_indexed_values*/0,
				cm_field_type, coordinate_system,
				/*value_type*/FE_VALUE_VALUE, number_of_components, component_names,
				/*number_of_times*/0, /*time_value_type*/FE_VALUE_VALUE,
				/*external*/NULL);
			ACCESS(FE_field)(data.destination_fe_fields[fi]);
			for (int j = 0; j < number_of_components; j++)
			{
				DEALLOCATE(component_names[j]);
			}
			delete[] component_names;
			DEALLOCATE(name);
		}

		/* Set up data */
		switch (mode)
		{
			case CONVERT_TO_FINITE_ELEMENTS_HERMITE_2D_PRODUCT:
			{
				if (ALLOCATE(data.temporary_values, FE_value,
						/*dofs*/2 * 4 * data.maximum_number_of_components))
				{
					return_code = 1;
				}
				else
				{
					display_message(ERROR_MESSAGE,"finite_element_conversion.  "
						"Unable to allocate temporary values.");
					return_code=0;
				}
				if (return_code)
				{
					/* Make template node defining all the fields in the field list */
					if ((data.template_node = CREATE(FE_node)(/*node_number*/0, destination_fe_region,
							/*template_node*/(struct FE_node *)NULL)))
					{
						for (i = 0 ; return_code && (i < data.number_of_fields) ; i++)
						{
							return_code = define_FE_field_at_node_simple(data.template_node,
								data.destination_fe_fields[i], /*number_of_derivatives*/3, hermite_2d_nodal_value_types);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"finite_element_conversion.  "
							"Unable to make hermite template node.");
						return_code=0;
					}
				}
				if (return_code)
				{
					/* Make template element defining all the fields in the field list */
					if ((data.template_element = create_FE_element_with_line_shape(/*element_number*/1,
						destination_fe_region, /*dimension*/2)))
					{
						for (i = 0 ; return_code && (i < data.number_of_fields) ; i++)
						{
							return_code = FE_element_define_tensor_product_basis(data.template_element,
								/*dimension*/2, CUBIC_HERMITE, data.destination_fe_fields[i]);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"finite_element_conversion.  "
							"Unable to make hermite template node.");
						return_code=0;
					}
				}
			} break;
			case CONVERT_TO_FINITE_ELEMENTS_TRILINEAR:
			case CONVERT_TO_FINITE_ELEMENTS_TRIQUADRATIC:
			{
				Computed_field *coordinate_field = field_array[0];
				if (coordinate_field && (3 == Computed_field_get_number_of_components(coordinate_field)))
				{
					// OK
				}
				else
				{
					display_message(ERROR_MESSAGE,"finite_element_conversion.  "
						"Invalid first/coordinate field.");
					return_code=0;
				}

				if (ALLOCATE(data.temporary_values, FE_value,
						/*dofs*/2 * data.maximum_number_of_components))
				{
					return_code = 1;
				}
				else
				{
					display_message(ERROR_MESSAGE,"finite_element_conversion.  "
						"Unable to allocate temporary values.");
					return_code=0;
				}
				if (return_code)
				{
					/* Make template node defining all the fields in the field list */
					if ((data.template_node = CREATE(FE_node)(/*node_number*/0, destination_fe_region,
							/*template_node*/(struct FE_node *)NULL)))
					{
						for (i = 0 ; return_code && (i < data.number_of_fields) ; i++)
						{
							return_code = define_FE_field_at_node_simple(data.template_node,
								data.destination_fe_fields[i], /*number_of_derivatives*/0, NULL);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"finite_element_conversion.  "
							"Unable to make hermite template node.");
						return_code=0;
					}
				}
				if (return_code)
				{
					enum FE_basis_type fe_basis_type = (mode == CONVERT_TO_FINITE_ELEMENTS_TRILINEAR) ?
						LINEAR_LAGRANGE : QUADRATIC_LAGRANGE;
					const int number_of_nodes =
						(mode == CONVERT_TO_FINITE_ELEMENTS_TRILINEAR) ? 8 : 27;
					/* Make template element defining all the fields in the field list */
					if ((data.template_element = create_FE_element_with_line_shape(/*element_number*/1,
						destination_fe_region, /*dimension*/3)) &&
						set_FE_element_number_of_nodes(data.template_element, number_of_nodes))
					{
						for (i = 0 ; return_code && (i < data.number_of_fields) ; i++)
						{
							return_code = FE_element_define_field_simple(data.template_element,
								data.destination_fe_fields[i], fe_basis_type);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"finite_element_conversion.  "
							"Unable to make template element.");
						return_code=0;
					}
				}
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,"finite_element_conversion.  "
					"Invalid or unimplemented conversion mode.");
				return_code=0;
			} break;
		}

		FE_region_begin_change(destination_fe_region);

		if (return_code)
		{
			return_code = FE_region_for_each_FE_element_of_dimension(source_fe_region,
				data.mode_dimension, FE_element_convert_element, (void *)&data);
		}

		FE_region_end_change(destination_fe_region);
	}
	else
	{
		display_message(ERROR_MESSAGE,"finite_element_conversion.  "
			"Invalid arguments.");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* finite_element_conversion */

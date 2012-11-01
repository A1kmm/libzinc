/*******************************************************************************
FILE : cmiss_graphic.cpp

LAST MODIFIED : 22 October 2008

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
#include <string>

#include "api/cmiss_zinc_configure.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "api/cmiss_element.h"
#include "api/cmiss_graphic.h"
#include "api/cmiss_graphics_font.h"
#include "api/cmiss_graphics_filter.h"
#include "api/cmiss_field_subobject_group.h"
#include "api/cmiss_node.h"
#include "general/debug.h"
#include "general/enumerator_private.hpp"
#include "general/indexed_list_private.h"
#include "general/compare.h"
#include "general/multi_range.h"
#include "general/mystring.h"
#include "general/object.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_finite_element.h"
#include "computed_field/computed_field_group.h"
#include "computed_field/computed_field_set.h"
#include "computed_field/computed_field_wrappers.h"
#include "finite_element/finite_element.h"
#include "finite_element/finite_element_discretization.h"
#include "finite_element/finite_element_region.h"
#include "finite_element/finite_element_to_graphics_object.h"
#include "finite_element/finite_element_to_iso_lines.h"
#include "finite_element/finite_element_to_iso_surfaces.h"
#include "finite_element/finite_element_to_streamlines.h"
#include "graphics/auxiliary_graphics_types.h"
#include "graphics/rendition.h"
#include "graphics/font.h"
#include "graphics/graphics_object.h"
#include "graphics/scene.h"
#include "graphics/graphic.h"
#include "general/message.h"
#include "general/enumerator_conversion.hpp"
#include "graphics/graphics_coordinate_system.hpp"
#include "graphics/render_gl.h"
#include "graphics/tessellation.hpp"
#include "computed_field/computed_field_subobject_group_private.hpp"
#if defined(USE_OPENCASCADE)
#	include "cad/computed_field_cad_geometry.h"
#	include "cad/computed_field_cad_topology.h"
#	include "cad/cad_geometry_to_graphics_object.h"
#endif /* defined(USE_OPENCASCADE) */

FULL_DECLARE_INDEXED_LIST_TYPE(Cmiss_graphic);

DECLARE_INDEXED_LIST_MODULE_FUNCTIONS(Cmiss_graphic,position,int, \
	compare_int);

struct Cmiss_graphic_select_graphics_data
{
	struct FE_region *fe_region;
	struct Cmiss_graphic *graphic;
};

enum Cmiss_graphic_change
{
	CMISS_GRAPHIC_CHANGE_NONE = 0,
	CMISS_GRAPHIC_CHANGE_REDRAW = 1,          /**< minor change requiring redraw, e.g. visibility flag toggled */
	CMISS_GRAPHIC_CHANGE_RECOMPILE = 2,       /**< graphics display list may need to be recompiled */
	CMISS_GRAPHIC_CHANGE_SELECTION = 3,       /**< change to selected objects */
	CMISS_GRAPHIC_CHANGE_PARTIAL_REBUILD = 4, /**< partial rebuild of graphics object */
	CMISS_GRAPHIC_CHANGE_FULL_REBUILD = 5,    /**< graphics object needs full rebuild */
};

/***************************************************************************//**
 * Call whenever attributes of the graphic have changed to ensure the graphics
 * object is invalidated (if needed) or that the minimum rebuild and redraw is
 * performed.
 */
static int Cmiss_graphic_changed(struct Cmiss_graphic *graphic,
	enum Cmiss_graphic_change change)
{
	int return_code = 1;
	if (graphic)
	{
		switch (change)
		{
		case CMISS_GRAPHIC_CHANGE_REDRAW:
			break;
		case CMISS_GRAPHIC_CHANGE_RECOMPILE:
		case CMISS_GRAPHIC_CHANGE_SELECTION:
			graphic->selected_graphics_changed = 1;
			break;
		case CMISS_GRAPHIC_CHANGE_PARTIAL_REBUILD:
			// partial removal of graphics should have been done by caller
			graphic->graphics_changed = 1;
			break;
		case CMISS_GRAPHIC_CHANGE_FULL_REBUILD:
			graphic->graphics_changed = 1;
			if (graphic->graphics_object)
			{
				// Following cannot handle change of GT_object type for isosurface, streamline
				//GT_object_remove_primitives_at_time(
				//	graphic->graphics_object, /*time*/0.0,
				//	(GT_object_primitive_object_name_conditional_function *)NULL,
				//	(void *)NULL);
				DEACCESS(GT_object)(&(graphic->graphics_object));
			}
			break;
		default:
			return_code = 0;
			break;
		}
		if (return_code)
		{
			Cmiss_rendition_graphic_changed_private(graphic->rendition, graphic);
		}
	}
	else
	{
		return_code = 0;
	}
	return return_code;
}

PROTOTYPE_ENUMERATOR_STRING_FUNCTION(Cmiss_graphic_type)
{
	const char *enumerator_string;

	ENTER(ENUMERATOR_STRING(Cmiss_graphic_type));
	switch (enumerator_value)
	{
		case CMISS_GRAPHIC_NODE_POINTS:
		{
			enumerator_string = "node_points";
		} break;
		case CMISS_GRAPHIC_DATA_POINTS:
		{
			enumerator_string = "data_points";
		} break;
		case CMISS_GRAPHIC_LINES:
		{
			enumerator_string = "lines";
		} break;
		case CMISS_GRAPHIC_CYLINDERS:
		{
			enumerator_string = "cylinders";
		} break;
		case CMISS_GRAPHIC_SURFACES:
		{
			enumerator_string = "surfaces";
		} break;
		case CMISS_GRAPHIC_ISO_SURFACES:
		{
			enumerator_string = "iso_surfaces";
		} break;
		case CMISS_GRAPHIC_ELEMENT_POINTS:
		{
			enumerator_string = "element_points";
		} break;
		case CMISS_GRAPHIC_STREAMLINES:
		{
			enumerator_string = "streamlines";
		} break;
		case CMISS_GRAPHIC_POINT:
		{
			enumerator_string = "point";
		} break;
		default:
		{
			enumerator_string = (const char *)NULL;
		} break;
	}
	LEAVE;

	return (enumerator_string);
} /* ENUMERATOR_STRING(Cmiss_graphic_type) */

DEFINE_DEFAULT_ENUMERATOR_FUNCTIONS(Cmiss_graphic_type)

PROTOTYPE_ENUMERATOR_STRING_FUNCTION(Graphic_glyph_scaling_mode)
{
	const char *enumerator_string;

	ENTER(ENUMERATOR_STRING(Graphic_glyph_scaling_mode));
	switch (enumerator_value)
	{
		case GRAPHIC_GLYPH_SCALING_CONSTANT:
		{
			enumerator_string = "constant";
		} break;
		case GRAPHIC_GLYPH_SCALING_SCALAR:
		{
			enumerator_string = "scalar";
		} break;
		case GRAPHIC_GLYPH_SCALING_VECTOR:
		{
			enumerator_string = "vector";
		} break;
		case GRAPHIC_GLYPH_SCALING_AXES:
		{
			enumerator_string = "axes";
		} break;
		case GRAPHIC_GLYPH_SCALING_GENERAL:
		{
			enumerator_string = "general";
		} break;
		default:
		{
			enumerator_string = (const char *)NULL;
		} break;
	}
	LEAVE;

	return (enumerator_string);
} /* ENUMERATOR_STRING(Glyph_scaling_mode) */

DEFINE_DEFAULT_ENUMERATOR_FUNCTIONS(Graphic_glyph_scaling_mode)

int Cmiss_graphic_type_uses_attribute(enum Cmiss_graphic_type graphic_type,
	enum Cmiss_graphic_attribute attribute)
{
	int return_code = 0;

	switch (attribute)
	{
		case CMISS_GRAPHIC_ATTRIBUTE_DISCRETIZATION:
		case CMISS_GRAPHIC_ATTRIBUTE_XI_DISCRETIZATION_MODE:
		{
			return_code = (graphic_type == CMISS_GRAPHIC_ELEMENT_POINTS) ||
				(graphic_type == CMISS_GRAPHIC_STREAMLINES);
		} break;
		case CMISS_GRAPHIC_ATTRIBUTE_GLYPH:
		case CMISS_GRAPHIC_ATTRIBUTE_LABEL_FIELD:
		{
			return_code =
				(graphic_type == CMISS_GRAPHIC_NODE_POINTS) ||
				(graphic_type == CMISS_GRAPHIC_DATA_POINTS) ||
				(graphic_type == CMISS_GRAPHIC_ELEMENT_POINTS) ||
				(graphic_type == CMISS_GRAPHIC_POINT);
		} break;
		case CMISS_GRAPHIC_ATTRIBUTE_NATIVE_DISCRETIZATION_FIELD:
		case CMISS_GRAPHIC_ATTRIBUTE_TESSELLATION:
		{
			return_code = (graphic_type != CMISS_GRAPHIC_DATA_POINTS) &&
				(graphic_type != CMISS_GRAPHIC_NODE_POINTS) &&
				(graphic_type != CMISS_GRAPHIC_POINT);
		} break;
		case CMISS_GRAPHIC_ATTRIBUTE_RENDER_TYPE:
		{
			return_code =
				(graphic_type == CMISS_GRAPHIC_SURFACES) ||
				(graphic_type == CMISS_GRAPHIC_ISO_SURFACES) ||
				(graphic_type == CMISS_GRAPHIC_CYLINDERS);
		} break;
		case CMISS_GRAPHIC_ATTRIBUTE_TEXTURE_COORDINATE_FIELD:
		{
			return_code =
				(graphic_type == CMISS_GRAPHIC_SURFACES) ||
				(graphic_type == CMISS_GRAPHIC_ISO_SURFACES) ||
				(graphic_type == CMISS_GRAPHIC_LINES) ||
				(graphic_type == CMISS_GRAPHIC_CYLINDERS);
		} break;
		case CMISS_GRAPHIC_ATTRIBUTE_EXTERIOR_FLAG:
		case CMISS_GRAPHIC_ATTRIBUTE_FACE:
		{
			return_code =
				(graphic_type == CMISS_GRAPHIC_SURFACES) ||
				(graphic_type == CMISS_GRAPHIC_ISO_SURFACES) ||
				(graphic_type == CMISS_GRAPHIC_LINES) ||
				(graphic_type == CMISS_GRAPHIC_ELEMENT_POINTS) ||
				(graphic_type == CMISS_GRAPHIC_CYLINDERS);
		} break;
		case CMISS_GRAPHIC_ATTRIBUTE_LINE_WIDTH:
		{
			return_code =
				(graphic_type == CMISS_GRAPHIC_ISO_SURFACES) ||
				(graphic_type == CMISS_GRAPHIC_LINES);

		} break;
		case CMISS_GRAPHIC_ATTRIBUTE_USE_ELEMENT_TYPE:
		{
			return_code =
				(graphic_type == CMISS_GRAPHIC_ISO_SURFACES) ||
				(graphic_type == CMISS_GRAPHIC_ELEMENT_POINTS);
		} break;
		default:
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_graphic_type_uses_attribute.  Unrecognised attribute %d", attribute);
		} break;
	}
	return return_code;
}

struct Cmiss_graphic *CREATE(Cmiss_graphic)(
	enum Cmiss_graphic_type graphic_type)
/*******************************************************************************
LAST MODIFIED : 14 March 2003

DESCRIPTION :
Allocates memory for a Cmiss_graphic and initialises its members.
==============================================================================*/
{
	struct Cmiss_graphic *graphic;

	ENTER(CREATE(Cmiss_graphic));

	if ((CMISS_GRAPHIC_NODE_POINTS==graphic_type)||
		(CMISS_GRAPHIC_DATA_POINTS==graphic_type)||
		(CMISS_GRAPHIC_LINES==graphic_type)||
		(CMISS_GRAPHIC_CYLINDERS==graphic_type)||
		(CMISS_GRAPHIC_SURFACES==graphic_type)||
		(CMISS_GRAPHIC_ISO_SURFACES==graphic_type)||
		(CMISS_GRAPHIC_ELEMENT_POINTS==graphic_type)||
		(CMISS_GRAPHIC_STREAMLINES==graphic_type)||
		(CMISS_GRAPHIC_POINT==graphic_type))
	{
		if (ALLOCATE(graphic,struct Cmiss_graphic,1))
		{
			graphic->position=0;
			graphic->rendition = NULL;
			graphic->name = (char *)NULL;

			/* geometry settings defaults */
			/* for all graphic types */
			graphic->graphic_type=graphic_type;
			graphic->coordinate_field=(struct Computed_field *)NULL;
			/* For surfaces only at the moment */
			graphic->texture_coordinate_field=(struct Computed_field *)NULL;
			/* for 1-D and 2-D elements only */
			graphic->exterior=0;
			graphic->face=-1; /* any face */
			/* for cylinders only */
			graphic->constant_radius=0.0;
			graphic->radius_scale_factor=1.0;
			graphic->radius_scalar_field=(struct Computed_field *)NULL;
			/* for iso_surfaces only */
			graphic->iso_scalar_field=(struct Computed_field *)NULL;
			graphic->number_of_iso_values=0;
			graphic->iso_values=(double *)NULL;
			graphic->first_iso_value=0.0;
			graphic->last_iso_value=0.0;
			graphic->decimation_threshold = 0.0;
			/* for node_points, data_points and element_points only */
			graphic->glyph=(struct GT_object *)NULL;
			graphic->glyph_scaling_mode = GRAPHIC_GLYPH_SCALING_GENERAL;
			graphic->glyph_offset[0]=0.0;
			graphic->glyph_offset[1]=0.0;
			graphic->glyph_offset[2]=0.0;
			graphic->glyph_scale_factors[0]=1.0;
			graphic->glyph_scale_factors[1]=1.0;
			graphic->glyph_scale_factors[2]=1.0;
			graphic->glyph_size[0]=1.0;
			graphic->glyph_size[1]=1.0;
			graphic->glyph_size[2]=1.0;
			graphic->orientation_scale_field=(struct Computed_field *)NULL;
			graphic->variable_scale_field=(struct Computed_field *)NULL;
			graphic->label_field=(struct Computed_field *)NULL;
			graphic->label_density_field=(struct Computed_field *)NULL;
			graphic->subgroup_field=(struct Computed_field *)NULL;
			graphic->select_mode=GRAPHICS_SELECT_ON;
			/* for element_points and iso_surfaces */
			graphic->use_element_type=USE_ELEMENTS;
			/* for element_points only */
			graphic->xi_discretization_mode=XI_DISCRETIZATION_CELL_CENTRES;
			graphic->xi_point_density_field = (struct Computed_field *)NULL;
			graphic->native_discretization_field=(struct FE_field *)NULL;
			graphic->tessellation=NULL;
			/* default to 1*1*1 discretization for fastest possible display.
				 Important since model may have a *lot* of elements */
			graphic->discretization.number_in_xi1=1;
			graphic->discretization.number_in_xi2=1;
			graphic->discretization.number_in_xi3=1;
			graphic->circle_discretization=6;
			/* for volumes only */
			graphic->volume_texture=(struct VT_volume_texture *)NULL;
			graphic->displacement_map_field=(struct Computed_field *)NULL;
			graphic->displacement_map_xi_direction = 12;
			/* for settings starting in a particular element */
			graphic->seed_element=(struct FE_element *)NULL;
			/* for settings requiring an exact xi location */
			graphic->seed_xi[0]=0.5;
			graphic->seed_xi[1]=0.5;
			graphic->seed_xi[2]=0.5;
			/* for streamlines only */
			graphic->streamline_type=STREAM_LINE;
			graphic->stream_vector_field=(struct Computed_field *)NULL;
			graphic->reverse_track=0;
			graphic->streamline_length=1.0;
			graphic->streamline_width=1.0;
			graphic->seed_nodeset = (Cmiss_nodeset_id)0;
			graphic->seed_node_mesh_location_field = (struct Computed_field *)NULL;
			graphic->overlay_flag = 0;
			graphic->overlay_order = 1;
			graphic->coordinate_system = CMISS_GRAPHICS_COORDINATE_SYSTEM_LOCAL;
			/* appearance settings defaults */
			/* for all graphic types */
			graphic->visibility_flag = true;
			graphic->material=(struct Graphical_material *)NULL;
			graphic->secondary_material=(struct Graphical_material *)NULL;
			graphic->selected_material=(struct Graphical_material *)NULL;
			graphic->data_field=(struct Computed_field *)NULL;
			graphic->spectrum=(struct Spectrum *)NULL;
			graphic->customised_graphics_object =(struct GT_object *)NULL;
			graphic->autorange_spectrum_flag = 0;
			/* for glyphsets */
			graphic->font = NULL;
			/* for cylinders, surfaces and volumes */
			graphic->render_type = CMISS_GRAPHICS_RENDER_TYPE_SHADED;
			/* for streamlines only */
			graphic->streamline_data_type=STREAM_NO_DATA;
			/* for lines */
			graphic->line_width = 0;

			/* rendering information defaults */
			graphic->graphics_object = (struct GT_object *)NULL;
			graphic->graphics_changed = 1;
			graphic->selected_graphics_changed = 0;
			graphic->time_dependent = 0;

			graphic->access_count=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"CREATE(Cmiss_graphic).  Insufficient memory");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(Cmiss_graphic).  Invalid graphic type");
		graphic=(struct Cmiss_graphic *)NULL;
	}
	LEAVE;

	return (graphic);
} /* CREATE(Cmiss_graphic) */

int DESTROY(Cmiss_graphic)(
	struct Cmiss_graphic **cmiss_graphic_address)
{
	int return_code;
	struct Cmiss_graphic *graphic;

	ENTER(DESTROY(Cmiss_graphic));
	if (cmiss_graphic_address && (graphic= *cmiss_graphic_address))
	{
		if (graphic->name)
		{
			DEALLOCATE(graphic->name);
		}
		if (graphic->customised_graphics_object)
		{
			DEACCESS(GT_object)(&(graphic->customised_graphics_object));
		}
		if (graphic->graphics_object)
		{
			DEACCESS(GT_object)(&(graphic->graphics_object));
		}
		if (graphic->coordinate_field)
		{
			DEACCESS(Computed_field)(&(graphic->coordinate_field));
		}
		if (graphic->texture_coordinate_field)
		{
			DEACCESS(Computed_field)(&(graphic->texture_coordinate_field));
		}
		if (graphic->radius_scalar_field)
		{
			DEACCESS(Computed_field)(&(graphic->radius_scalar_field));
		}
		if (graphic->iso_scalar_field)
		{
				DEACCESS(Computed_field)(&(graphic->iso_scalar_field));
		}
		if (graphic->iso_values)
		{
			DEALLOCATE(graphic->iso_values);
		}
		if (graphic->glyph)
		{
				GT_object_remove_callback(graphic->glyph, Cmiss_graphic_glyph_change,
					(void *)graphic);
				DEACCESS(GT_object)(&(graphic->glyph));
		}
		if (graphic->orientation_scale_field)
		{
			DEACCESS(Computed_field)(&(graphic->orientation_scale_field));
		}
		if (graphic->variable_scale_field)
		{
			DEACCESS(Computed_field)(&(graphic->variable_scale_field));
		}
		if (graphic->label_field)
		{
			DEACCESS(Computed_field)(&(graphic->label_field));
		}
		if (graphic->label_density_field)
		{
			DEACCESS(Computed_field)(&(graphic->label_density_field));
		}
		if (graphic->subgroup_field)
		{
			DEACCESS(Computed_field)(&(graphic->subgroup_field));
		}
		if (graphic->volume_texture)
		{
			DEACCESS(VT_volume_texture)(&(graphic->volume_texture));
		}
		if (graphic->displacement_map_field)
		{
			DEACCESS(Computed_field)(&(graphic->displacement_map_field));
		}
		if (graphic->xi_point_density_field)
		{
			DEACCESS(Computed_field)(&(graphic->xi_point_density_field));
		}
		if (graphic->native_discretization_field)
		{
			DEACCESS(FE_field)(&(graphic->native_discretization_field));
		}
		if (graphic->tessellation)
		{
			DEACCESS(Cmiss_tessellation)(&(graphic->tessellation));
		}
		if (graphic->stream_vector_field)
		{
			DEACCESS(Computed_field)(&(graphic->stream_vector_field));
		}
		/* appearance graphic */
		if (graphic->material)
		{
			DEACCESS(Graphical_material)(&(graphic->material));
		}
		if (graphic->secondary_material)
		{
			DEACCESS(Graphical_material)(&(graphic->secondary_material));
		}
		if (graphic->selected_material)
		{
			DEACCESS(Graphical_material)(&(graphic->selected_material));
		}
		if (graphic->data_field)
		{
			DEACCESS(Computed_field)(&(graphic->data_field));
		}
		if (graphic->spectrum)
		{
			DEACCESS(Spectrum)(&(graphic->spectrum));
		}
		if (graphic->font)
		{
			DEACCESS(Cmiss_graphics_font)(&(graphic->font));
		}
		if (graphic->seed_element)
		{
			DEACCESS(FE_element)(&(graphic->seed_element));
		}
		if (graphic->seed_nodeset)
		{
			Cmiss_nodeset_destroy(&graphic->seed_nodeset);
		}
		if (graphic->seed_node_mesh_location_field)
		{
			DEACCESS(Computed_field)(&(graphic->seed_node_mesh_location_field));
		}
		DEALLOCATE(*cmiss_graphic_address);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"DESTROY(Cmiss_graphic_address).  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
}

/***************************************************************************//**
 * Returns the dimension of the <graphic>, which varies for some graphic types.
 * @param graphic Cmiss graphic
 * @param fe_region  Used for iso_surfaces and element_points with USE_ELEMENT
 * type. Gives the highest dimension for which elements exist. If omitted uses 3.
 * @return the dimension of the graphic
 */
int Cmiss_graphic_get_dimension(struct Cmiss_graphic *graphic, struct FE_region *fe_region)
{
	int dimension;

	ENTER(cmiss_graphic_get_dimension);
	if (graphic)
	{
		switch (graphic->graphic_type)
		{
			case CMISS_GRAPHIC_NODE_POINTS:
			case CMISS_GRAPHIC_DATA_POINTS:
			case CMISS_GRAPHIC_POINT:
			{
				dimension=0;
			} break;
			case CMISS_GRAPHIC_LINES:
			case CMISS_GRAPHIC_CYLINDERS:
			{
				dimension=1;
			} break;
			case CMISS_GRAPHIC_SURFACES:
			{
				dimension=2;
			} break;
			case CMISS_GRAPHIC_STREAMLINES:
			{
				// GRC: should be controllable, e.g. via use_element_type
				dimension = fe_region ? FE_region_get_highest_dimension(fe_region) : 3;
				if (0 == dimension)
					dimension = 3;
			} break;
			case CMISS_GRAPHIC_ELEMENT_POINTS:
			case CMISS_GRAPHIC_ISO_SURFACES:
			{
				dimension=Use_element_type_dimension(graphic->use_element_type, fe_region);
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"Cmiss_graphic_get_dimension.  Unknown graphic type");
				dimension=-1;
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_get_dimension.  Invalid argument(s)");
		dimension=0;
	}
	LEAVE;

	return (dimension);
} /* Cmiss_graphic_get_dimension */

struct Cmiss_element_conditional_field_data
{
	Cmiss_field_cache_id field_cache;
	Cmiss_field_id conditional_field;
};

/** @return true if conditional field evaluates to true in element */
int Cmiss_element_conditional_field_is_true(Cmiss_element_id element,
	void *conditional_field_data_void)
{
	Cmiss_element_conditional_field_data *data =
		reinterpret_cast<Cmiss_element_conditional_field_data*>(conditional_field_data_void);
	if (element && data)
	{
		Cmiss_field_cache_set_element(data->field_cache, element);
		return Cmiss_field_evaluate_boolean(data->conditional_field, data->field_cache);
	}
	return 0;
}

/***************************************************************************//**
 * Converts a finite element into a graphics object with the supplied graphic.
 * @param element  The Cmiss_element.
 * @param graphic_to_object_data  Data for converting finite element to graphics.
 * @return return 1 if the element would contribute any graphics generated from the Cmiss_graphic
 */
static int FE_element_to_graphics_object(struct FE_element *element,
	Cmiss_graphic_to_graphics_object_data *graphic_to_object_data)
{
	FE_value base_size[3], offset[3], initial_xi[3], scale_factors[3];
	GLfloat time;
	int element_dimension = 1, element_graphics_name,
		element_selected, i, number_in_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS],
		number_of_xi_points, return_code,
		*top_level_xi_point_numbers,
		use_element_dimension, *use_number_in_xi;
	struct CM_element_information cm;
	struct Element_point_ranges *element_point_ranges;
	struct Element_point_ranges_identifier element_point_ranges_identifier;
	struct FE_element *top_level_element,*use_element;
	struct FE_field *native_discretization_field;
	struct Cmiss_graphic *graphic;
	struct GT_glyph_set *glyph_set;
	struct GT_polyline *polyline;
	struct GT_surface *surface;
	struct Multi_range *ranges;
	FE_value_triple *xi_points = NULL;

	ENTER(FE_element_to_graphics_object);
	if (element && graphic_to_object_data &&
		(NULL != (graphic = graphic_to_object_data->graphic)) &&
		graphic->graphics_object)
	{
		element_dimension = get_FE_element_dimension(element);
		return_code = 1;
		get_FE_element_identifier(element, &cm);
		element_graphics_name = cm.number;
		/* proceed only if graphic uses this element */
		int draw_element = 1;
		Cmiss_element_conditional_field_data conditional_field_data = { graphic_to_object_data->field_cache, graphic->subgroup_field };
		if (draw_element)
		{
			int dimension = Cmiss_graphic_get_dimension(graphic, graphic_to_object_data->fe_region);
			draw_element = FE_element_meets_topological_criteria(element, dimension,
				graphic->exterior, graphic->face,
				graphic->subgroup_field ? Cmiss_element_conditional_field_is_true : 0,
				graphic->subgroup_field ? (void *)&conditional_field_data : 0);
		}
		if (draw_element)
		{
			// FE_element_meets_topological_criteria may have set element in cache, so must set afterwards
			Cmiss_field_cache_set_element(graphic_to_object_data->field_cache, element);
			if (graphic->subgroup_field && (graphic_to_object_data->iteration_mesh == graphic_to_object_data->master_mesh))
			{
				draw_element = Cmiss_field_evaluate_boolean(graphic->subgroup_field, graphic_to_object_data->field_cache);
			}
		}
		int name_selected = 0;
		if (draw_element)
		{
			if ((GRAPHICS_DRAW_SELECTED == graphic->select_mode) ||
				(GRAPHICS_DRAW_UNSELECTED == graphic->select_mode))
			{
				if (graphic_to_object_data->selection_group_field)
				{
					name_selected = Cmiss_field_evaluate_boolean(graphic_to_object_data->selection_group_field, graphic_to_object_data->field_cache);
				}
				draw_element = ((name_selected && (GRAPHICS_DRAW_SELECTED == graphic->select_mode)) ||
					((!name_selected) && (GRAPHICS_DRAW_SELECTED != graphic->select_mode)));
			}
		}
		if (draw_element)
		{
			/* determine discretization of element for graphic */
			// copy top_level_number_in_xi since scaled by native_discretization in
			// get_FE_element_discretization
			int top_level_number_in_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
			for (int dim = 0; dim < MAXIMUM_ELEMENT_XI_DIMENSIONS; dim++)
			{
				top_level_number_in_xi[dim] = graphic_to_object_data->top_level_number_in_xi[dim];
			}
			top_level_element = (struct FE_element *)NULL;
			native_discretization_field = graphic->native_discretization_field;

			if (get_FE_element_discretization(element,
				graphic->subgroup_field ? Cmiss_element_conditional_field_is_true : 0,
				graphic->subgroup_field ? (void *)&conditional_field_data : 0,
				graphic->face, native_discretization_field, top_level_number_in_xi,
				&top_level_element, number_in_xi))
			{
				/* g_element renditions use only one time = 0.0. Must take care. */
				time = 0.0;
				switch (graphic->graphic_type)
				{
					case CMISS_GRAPHIC_LINES:
					{
						if (graphic_to_object_data->existing_graphics)
						{
							/* So far ignore these */
						}
						if (draw_element)
						{
							return_code = FE_element_add_line_to_vertex_array(
								element, graphic_to_object_data->field_cache,
								GT_object_get_vertex_set(graphic->graphics_object),
								graphic_to_object_data->rc_coordinate_field,
								graphic->data_field,
								graphic_to_object_data->number_of_data_values,
								graphic_to_object_data->data_copy_buffer,
								graphic->texture_coordinate_field,
								number_in_xi[0], top_level_element,
								graphic_to_object_data->time);
						}
					} break;
					case CMISS_GRAPHIC_CYLINDERS:
					{
						if (graphic_to_object_data->existing_graphics)
						{
							surface = GT_OBJECT_EXTRACT_FIRST_PRIMITIVES_AT_TIME(GT_surface)
								(graphic_to_object_data->existing_graphics, time,
									element_graphics_name);
						}
						else
						{
							surface = (struct GT_surface *)NULL;
						}
						if (draw_element)
						{
							if (surface ||
								(surface = create_cylinder_from_FE_element(element,
									graphic_to_object_data->field_cache,
									graphic_to_object_data->master_mesh,
									graphic_to_object_data->rc_coordinate_field,
									graphic->data_field, graphic->constant_radius,
									graphic->radius_scale_factor, graphic->radius_scalar_field,
									number_in_xi[0],
									graphic->circle_discretization,
									graphic->texture_coordinate_field,
									top_level_element, graphic->render_type,
									graphic_to_object_data->time)))
							{
								if (!GT_OBJECT_ADD(GT_surface)(
									graphic->graphics_object, time, surface))
								{
									DESTROY(GT_surface)(&surface);
									return_code = 0;
								}
							}
							else
							{
								return_code = 0;
							}
						}
						else
						{
							if (surface)
							{
								DESTROY(GT_surface)(&surface);
							}
						}
					} break;
					case CMISS_GRAPHIC_SURFACES:
					{
						if (graphic_to_object_data->existing_graphics)
						{
							surface = GT_OBJECT_EXTRACT_FIRST_PRIMITIVES_AT_TIME(GT_surface)
								(graphic_to_object_data->existing_graphics, time,
									element_graphics_name);
						}
						else
						{
							surface = (struct GT_surface *)NULL;
						}
						if (draw_element)
						{
							if (surface ||
								(surface = create_GT_surface_from_FE_element(
									element, graphic_to_object_data->field_cache,
									graphic_to_object_data->master_mesh,
									graphic_to_object_data->rc_coordinate_field,
									graphic->texture_coordinate_field, graphic->data_field,
									number_in_xi[0], number_in_xi[1],
									/*reverse_normals*/0, top_level_element,graphic->render_type,
									graphic_to_object_data->time)))
							{
								if (!GT_OBJECT_ADD(GT_surface)(
									graphic->graphics_object, time, surface))
								{
									DESTROY(GT_surface)(&surface);
									return_code = 0;
								}
							}
							else
							{
								return_code = 0;
							}
						}
						else
						{
							if (surface)
							{
								DESTROY(GT_surface)(&surface);
							}
						}
					} break;
					case CMISS_GRAPHIC_ISO_SURFACES:
					{
						switch (GT_object_get_type(graphic->graphics_object))
						{
							case g_SURFACE:
							{
								if (3 == element_dimension)
								{
									if (graphic_to_object_data->existing_graphics)
									{
										surface = GT_OBJECT_EXTRACT_FIRST_PRIMITIVES_AT_TIME(GT_surface)
											(graphic_to_object_data->existing_graphics, time,
												element_graphics_name);
									}
									else
									{
										surface = (struct GT_surface *)NULL;
									}
									if (draw_element)
									{
										if (NULL != surface)
										{
											if (!GT_OBJECT_ADD(GT_surface)(
												graphic->graphics_object, time, surface))
											{
												DESTROY(GT_surface)(&surface);
												return_code = 0;
											}
										}
										else
										{
											return_code = create_iso_surfaces_from_FE_element_new(element,
												graphic_to_object_data->field_cache,
												graphic_to_object_data->master_mesh,
												graphic_to_object_data->time, number_in_xi,
												graphic_to_object_data->iso_surface_specification,
												graphic->graphics_object,
												graphic->render_type);
										}
									}
									else
									{
										if (surface)
										{
											DESTROY(GT_surface)(&surface);
										}
									}
								}
							} break;
							case g_POLYLINE:
							{
								if (2 == element_dimension)
								{
									if (graphic_to_object_data->existing_graphics)
									{
										polyline =
											GT_OBJECT_EXTRACT_FIRST_PRIMITIVES_AT_TIME(GT_polyline)
											(graphic_to_object_data->existing_graphics, time,
												element_graphics_name);
									}
									else
									{
										polyline = (struct GT_polyline *)NULL;
									}
									if (draw_element)
									{
										if (polyline)
										{
											if (!GT_OBJECT_ADD(GT_polyline)(
												graphic->graphics_object, time, polyline))
											{
												DESTROY(GT_polyline)(&polyline);
												return_code = 0;
											}
										}
										else
										{
											if (graphic->iso_values)
											{
												for (i = 0 ; i < graphic->number_of_iso_values ; i++)
												{
													return_code = create_iso_lines_from_FE_element(element,
														graphic_to_object_data->field_cache,
														graphic_to_object_data->rc_coordinate_field,
														graphic->iso_scalar_field, graphic->iso_values[i],
														graphic->data_field, number_in_xi[0], number_in_xi[1],
														top_level_element, graphic->graphics_object,
														graphic->line_width);
												}
											}
											else
											{
												double iso_value_range;
												if (graphic->number_of_iso_values > 1)
												{
													iso_value_range =
														(graphic->last_iso_value - graphic->first_iso_value)
														/ (double)(graphic->number_of_iso_values - 1);
												}
												else
												{
													iso_value_range = 0;
												}
												for (i = 0 ; i < graphic->number_of_iso_values ; i++)
												{
													double iso_value =
														graphic->first_iso_value +
														(double)i * iso_value_range;
													return_code = create_iso_lines_from_FE_element(element,
														graphic_to_object_data->field_cache,
														graphic_to_object_data->rc_coordinate_field,
														graphic->iso_scalar_field, iso_value,
														graphic->data_field, number_in_xi[0], number_in_xi[1],
														top_level_element, graphic->graphics_object,
														graphic->line_width);
												}
											}
										}
									}
									else
									{
										if (polyline)
										{
											DESTROY(GT_polyline)(&polyline);
										}
									}
								}
							} break;
							default:
							{
								display_message(ERROR_MESSAGE,"FE_element_to_graphics_object.  "
									"Invalid graphic type for iso_scalar");
								return_code = 0;
							} break;
						}
					} break;
					case CMISS_GRAPHIC_ELEMENT_POINTS:
					{
						Cmiss_field_cache_set_time(graphic_to_object_data->field_cache, graphic_to_object_data->time);
						glyph_set = (struct GT_glyph_set *)NULL;
						if (graphic_to_object_data->existing_graphics)
						{
							glyph_set =
								GT_OBJECT_EXTRACT_FIRST_PRIMITIVES_AT_TIME(GT_glyph_set)(
									graphic_to_object_data->existing_graphics, time,
									element_graphics_name);
						}
						if (draw_element)
						{
							if (!glyph_set)
							{
								for (i = 0; i < 3; i++)
								{
									element_point_ranges_identifier.exact_xi[i] =
										graphic->seed_xi[i];
								}
								if (FE_element_get_xi_points(element,
									graphic->xi_discretization_mode, number_in_xi,
									element_point_ranges_identifier.exact_xi,
									graphic_to_object_data->field_cache,
									graphic_to_object_data->rc_coordinate_field,
									graphic->xi_point_density_field,
									&number_of_xi_points, &xi_points))
								{
									get_FE_element_identifier(element, &cm);
									element_graphics_name = cm.number;
									top_level_xi_point_numbers = (int *)NULL;
									if (XI_DISCRETIZATION_CELL_CORNERS ==
										graphic->xi_discretization_mode)
									{
										FE_element_convert_xi_points_cell_corners_to_top_level(
											element, top_level_element, top_level_number_in_xi,
											number_of_xi_points, xi_points, &top_level_xi_point_numbers);
									}
									if (top_level_xi_point_numbers)
									{
										/* xi_points have been converted to top-level */
										use_element = top_level_element;
										use_number_in_xi = top_level_number_in_xi;
									}
									else
									{
										use_element = element;
										use_number_in_xi = number_in_xi;
									}
									ranges = (struct Multi_range *)NULL;
									element_point_ranges_identifier.element = use_element;
									element_point_ranges_identifier.top_level_element=
										top_level_element;
									element_point_ranges_identifier.xi_discretization_mode =
										graphic->xi_discretization_mode;
									use_element_dimension = get_FE_element_dimension(use_element);
									for (i = 0; i < use_element_dimension; i++)
									{
										element_point_ranges_identifier.number_in_xi[i] =
											use_number_in_xi[i];
									}
									if (NULL != (element_point_ranges = FIND_BY_IDENTIFIER_IN_LIST(
										Element_point_ranges, identifier)(
											&element_point_ranges_identifier,
											graphic_to_object_data->selected_element_point_ranges_list)))
									{
										ranges = Element_point_ranges_get_ranges(element_point_ranges);
									}
									element_selected = 0;
									if (graphic_to_object_data->selection_group_field)
									{
										element_selected = Cmiss_field_evaluate_boolean(graphic_to_object_data->selection_group_field, graphic_to_object_data->field_cache);
									}
									base_size[0] = (FE_value)(graphic->glyph_size[0]);
									base_size[1] = (FE_value)(graphic->glyph_size[1]);
									base_size[2] = (FE_value)(graphic->glyph_size[2]);
									offset[0] = (FE_value)(graphic->glyph_offset[0]);
									offset[1] = (FE_value)(graphic->glyph_offset[1]);
									offset[2] = (FE_value)(graphic->glyph_offset[2]);
									scale_factors[0] = (FE_value)(graphic->glyph_scale_factors[0]);
									scale_factors[1] = (FE_value)(graphic->glyph_scale_factors[1]);
									scale_factors[2] = (FE_value)(graphic->glyph_scale_factors[2]);
									/* NOT an error if no glyph_set produced == empty selection */
									if ((0 < number_of_xi_points) &&
										NULL != (glyph_set = create_GT_glyph_set_from_FE_element(
											graphic_to_object_data->field_cache,
											use_element, top_level_element,
											graphic_to_object_data->rc_coordinate_field,
											number_of_xi_points, xi_points,
											graphic->glyph, base_size, offset, scale_factors,
											graphic_to_object_data->wrapper_orientation_scale_field,
											graphic->variable_scale_field, graphic->data_field,
											graphic->font, graphic->label_field, graphic->select_mode,
											element_selected, ranges, top_level_xi_point_numbers)))
									{
										/* set auxiliary_object_name for glyph_set to
											 element_graphics_name so we can edit */
										GT_glyph_set_set_auxiliary_integer_identifier(glyph_set,
											element_graphics_name);
									}
									if (top_level_xi_point_numbers)
									{
										DEALLOCATE(top_level_xi_point_numbers);
									}
									DEALLOCATE(xi_points);
								}
								else
								{
									return_code = 0;
								}
							}
							if (glyph_set)
							{
								if (!GT_OBJECT_ADD(GT_glyph_set)(
									graphic->graphics_object,time,glyph_set))
								{
									DESTROY(GT_glyph_set)(&glyph_set);
									return_code = 0;
								}
							}
						}
						else
						{
							if (glyph_set)
							{
								DESTROY(GT_glyph_set)(&glyph_set);
							}
						}
					} break;
					case CMISS_GRAPHIC_STREAMLINES:
					{
						/* use local copy of seed_xi since tracking function updates it */
						initial_xi[0] = graphic->seed_xi[0];
						initial_xi[1] = graphic->seed_xi[1];
						initial_xi[2] = graphic->seed_xi[2];
						for (i = 0; i < 3; i++)
						{
							element_point_ranges_identifier.exact_xi[i] =
								graphic->seed_xi[i];
						}
						if (FE_element_get_xi_points(element,
							graphic->xi_discretization_mode, number_in_xi,
							element_point_ranges_identifier.exact_xi,
							graphic_to_object_data->field_cache,
							graphic_to_object_data->rc_coordinate_field,
							graphic->xi_point_density_field,
							&number_of_xi_points, &xi_points))
						{
							if (STREAM_LINE == graphic->streamline_type)
							{
								for (i = 0; i < number_of_xi_points; i++)
								{
									initial_xi[0] = xi_points[i][0];
									initial_xi[1] = xi_points[i][1];
									initial_xi[2] = xi_points[i][2];
									if (NULL != (polyline = create_GT_polyline_streamline_FE_element(
											element, initial_xi, graphic_to_object_data->field_cache,
											graphic_to_object_data->rc_coordinate_field,
											graphic_to_object_data->wrapper_stream_vector_field,
											graphic->reverse_track, graphic->streamline_length,
											graphic->streamline_data_type, graphic->data_field,
											graphic_to_object_data->fe_region)))
									{
										if (!GT_OBJECT_ADD(GT_polyline)(graphic->graphics_object,
											time, polyline))
										{
											DESTROY(GT_polyline)(&polyline);
										}
									}
								}
							}
							else if ((graphic->streamline_type == STREAM_RIBBON)
								|| (graphic->streamline_type == STREAM_EXTRUDED_RECTANGLE)
								|| (graphic->streamline_type == STREAM_EXTRUDED_ELLIPSE)
								|| (graphic->streamline_type == STREAM_EXTRUDED_CIRCLE))
							{
								for (i = 0; i < number_of_xi_points; i++)
								{
									initial_xi[0] = xi_points[i][0];
									initial_xi[1] = xi_points[i][1];
									initial_xi[2] = xi_points[i][2];
									if (NULL != (surface = create_GT_surface_streamribbon_FE_element(
											element, initial_xi, graphic_to_object_data->field_cache,
											graphic_to_object_data->rc_coordinate_field,
											graphic_to_object_data->wrapper_stream_vector_field,
											graphic->reverse_track, graphic->streamline_length,
											graphic->streamline_width, graphic->streamline_type,
											graphic->streamline_data_type, graphic->data_field,
											graphic_to_object_data->fe_region)))
									{
										if (!GT_OBJECT_ADD(GT_surface)(graphic->graphics_object,
											time, surface))
										{
											DESTROY(GT_surface)(&surface);
										}
									}
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"FE_element_to_graphics_object.  Unknown streamline type");
								return_code = 0;
							}
						}
						else
						{
							return_code = 0;
						}
						if (xi_points)
							DEALLOCATE(xi_points);
					} break;
					default:
					{
						display_message(ERROR_MESSAGE,"FE_element_to_graphics_object.  "
							"Unknown element graphic type");
						return_code = 0;
					} break;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"FE_element_to_graphics_object.  Could not get discretization");
				return_code = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"FE_element_to_graphics_object.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_element_to_graphics_object */

/***************************************************************************//**
 * Creates a streamline seeded from the location given by the
 * seed_node_mesh_location_field at the node.
 * @param node  The node to seed streamline from.
 * @param graphic_to_object_data  All other data including graphic.
 * @return  1 if successfully added streamline
 */
static int Cmiss_node_to_streamline(struct FE_node *node,
	struct Cmiss_graphic_to_graphics_object_data *graphic_to_object_data)
{
	int return_code = 1;

	ENTER(node_to_streamline);
	struct Cmiss_graphic *graphic = 0;
	if (node && graphic_to_object_data &&
		(NULL != (graphic = graphic_to_object_data->graphic)) &&
		graphic->graphics_object)
	{
		Cmiss_field_cache_set_node(graphic_to_object_data->field_cache, node);
		FE_value xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
		Cmiss_element_id element = Cmiss_field_evaluate_mesh_location(
			graphic->seed_node_mesh_location_field, graphic_to_object_data->field_cache,
			MAXIMUM_ELEMENT_XI_DIMENSIONS, xi);
		if (element)
		{
			if (STREAM_LINE==graphic->streamline_type)
			{
				struct GT_polyline *polyline;
				if (NULL != (polyline=create_GT_polyline_streamline_FE_element(element,
						xi, graphic_to_object_data->field_cache,
						graphic_to_object_data->rc_coordinate_field,
						graphic_to_object_data->wrapper_stream_vector_field,
						graphic->reverse_track, graphic->streamline_length,
						graphic->streamline_data_type, graphic->data_field,
						graphic_to_object_data->fe_region)))
				{
					if (!(return_code=GT_OBJECT_ADD(GT_polyline)(
								graphic->graphics_object,
								/*graphics_object_time*/0,polyline)))
					{
						DESTROY(GT_polyline)(&polyline);
					}
				}
				else
				{
					return_code=0;
				}
			}
			else if ((graphic->streamline_type == STREAM_RIBBON)||
				(graphic->streamline_type == STREAM_EXTRUDED_RECTANGLE)||
				(graphic->streamline_type == STREAM_EXTRUDED_ELLIPSE)||
				(graphic->streamline_type == STREAM_EXTRUDED_CIRCLE))
			{
				struct GT_surface *surface;
				if (NULL != (surface=create_GT_surface_streamribbon_FE_element(element,
							xi, graphic_to_object_data->field_cache,
							graphic_to_object_data->rc_coordinate_field,
							graphic_to_object_data->wrapper_stream_vector_field,
							graphic->reverse_track, graphic->streamline_length,
							graphic->streamline_width, graphic->streamline_type,
							graphic->streamline_data_type, graphic->data_field,
							graphic_to_object_data->fe_region)))
				{
					if (!(return_code=GT_OBJECT_ADD(GT_surface)(
								graphic->graphics_object,
								/*graphics_object_time*/0,surface)))
					{
						DESTROY(GT_surface)(&surface);
					}
				}
				else
				{
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Cmiss_node_to_streamline.  Unknown streamline type");
				return_code=0;
			}
			Cmiss_element_destroy(&element);
		}
		else
		{
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_node_to_streamline.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* node_to_streamline */

int Cmiss_graphic_add_to_list(struct Cmiss_graphic *graphic,
	int position,struct LIST(Cmiss_graphic) *list_of_graphic)
{
	int last_position,return_code;
	struct Cmiss_graphic *graphic_in_way;

	ENTER(Cmiss_graphic_add_to_list);
	if (graphic&&list_of_graphic&&
		!IS_OBJECT_IN_LIST(Cmiss_graphic)(graphic,list_of_graphic))
	{
		return_code=1;
		last_position=NUMBER_IN_LIST(Cmiss_graphic)(list_of_graphic);
		if ((1>position)||(position>last_position))
		{
			/* add to end of list */
			position=last_position+1;
		}
		ACCESS(Cmiss_graphic)(graphic);
		while (return_code&&graphic)
		{
			graphic->position=position;
			/* is there already a graphic with that position? */
			if (NULL != (graphic_in_way=FIND_BY_IDENTIFIER_IN_LIST(Cmiss_graphic,
						position)(position,list_of_graphic)))
			{
				/* remove the old graphic to make way for the new */
				ACCESS(Cmiss_graphic)(graphic_in_way);
				REMOVE_OBJECT_FROM_LIST(Cmiss_graphic)(
					graphic_in_way,list_of_graphic);
			}
			if (ADD_OBJECT_TO_LIST(Cmiss_graphic)(graphic,list_of_graphic))
			{
				DEACCESS(Cmiss_graphic)(&graphic);
				/* the old, in-the-way graphic now become the new graphic */
				graphic=graphic_in_way;
				position++;
			}
			else
			{
				DEACCESS(Cmiss_graphic)(&graphic);
				if (graphic_in_way)
				{
					DEACCESS(Cmiss_graphic)(&graphic_in_way);
				}
				display_message(ERROR_MESSAGE,"Cmiss_graphic_add_to_list.  "
					"Could not add graphic - graphic lost");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_add_to_list.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
}

int Cmiss_graphic_remove_from_list(struct Cmiss_graphic *graphic,
	struct LIST(Cmiss_graphic) *list_of_graphic)
{
	int return_code,next_position;

	ENTER(Cmiss_graphic_remove_from_list);
	if (graphic&&list_of_graphic)
	{
		if (IS_OBJECT_IN_LIST(Cmiss_graphic)(graphic,list_of_graphic))
		{
			next_position=graphic->position+1;
			return_code=REMOVE_OBJECT_FROM_LIST(Cmiss_graphic)(
				graphic,list_of_graphic);
			/* decrement position of all remaining graphic */
			while (return_code&&(graphic=FIND_BY_IDENTIFIER_IN_LIST(
				Cmiss_graphic,position)(next_position,list_of_graphic)))
			{
				ACCESS(Cmiss_graphic)(graphic);
				REMOVE_OBJECT_FROM_LIST(Cmiss_graphic)(graphic,list_of_graphic);
				(graphic->position)--;
				if (ADD_OBJECT_TO_LIST(Cmiss_graphic)(graphic,list_of_graphic))
				{
					next_position++;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Cmiss_graphic_remove_from_list.  "
						"Could not readjust positions - graphic lost");
					return_code=0;
				}
				DEACCESS(Cmiss_graphic)(&graphic);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_graphic_remove_from_list.  Graphic not in list");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_remove_from_list.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_remove_from_list */

int Cmiss_graphic_modify_in_list(struct Cmiss_graphic *graphic,
	struct Cmiss_graphic *new_graphic,
	struct LIST(Cmiss_graphic) *list_of_graphic)
{
	int return_code,old_position;

	ENTER(Cmiss_graphic_modify_in_list);
	if (graphic&&new_graphic&&list_of_graphic)
	{
		if (IS_OBJECT_IN_LIST(Cmiss_graphic)(graphic,list_of_graphic))
		{
			/* save the current position */
			old_position=graphic->position;
			return_code=Cmiss_graphic_copy_without_graphics_object(graphic,new_graphic);
			graphic->position=old_position;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_graphic_modify_in_list.  graphic not in list");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_modify_in_list.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_modify_in_list */

DECLARE_OBJECT_FUNCTIONS(Cmiss_graphic);
DECLARE_INDEXED_LIST_FUNCTIONS(Cmiss_graphic);
DECLARE_FIND_BY_IDENTIFIER_IN_INDEXED_LIST_FUNCTION(Cmiss_graphic, \
	position,int,compare_int);

#if defined (USE_OPENCASCADE)

int Cmiss_graphic_selects_cad_primitives(struct Cmiss_graphic *graphic)
{
	int return_code;

	if (graphic)
	{
		return_code=(GRAPHICS_NO_SELECT != graphic->select_mode)&&(
			(CMISS_GRAPHIC_LINES==graphic->graphic_type)||
			(CMISS_GRAPHIC_SURFACES==graphic->graphic_type));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_selects_elements.  Invalid argument(s)");
		return_code=0;
	}

	return (return_code);
} /* Cmiss_graphic_selects_cad_primitives */

#endif /*defined (USE_OPENCASCADE) */

int Cmiss_graphic_selects_elements(struct Cmiss_graphic *graphic)
{
	int return_code;

	ENTER(Cmiss_graphic_selects_elements);
	if (graphic)
	{
		return_code=(GRAPHICS_NO_SELECT != graphic->select_mode)&&(
			(CMISS_GRAPHIC_LINES==graphic->graphic_type)||
			(CMISS_GRAPHIC_CYLINDERS==graphic->graphic_type)||
			(CMISS_GRAPHIC_SURFACES==graphic->graphic_type)||
			(CMISS_GRAPHIC_ELEMENT_POINTS==graphic->graphic_type)||
			(CMISS_GRAPHIC_ISO_SURFACES==graphic->graphic_type));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_selects_elements.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_selects_elements */

enum Cmiss_graphics_coordinate_system Cmiss_graphic_get_coordinate_system(
	struct Cmiss_graphic *graphic)
{
	enum Cmiss_graphics_coordinate_system coordinate_system;

	ENTER(Cmiss_graphic_get_coordinate_system);
	if (graphic)
	{
		coordinate_system=graphic->coordinate_system;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_get_coordinate_system.  Invalid argument(s)");
		coordinate_system = CMISS_GRAPHICS_COORDINATE_SYSTEM_INVALID;
	}
	LEAVE;

	return (coordinate_system);
}

int Cmiss_graphic_set_coordinate_system(
	struct Cmiss_graphic *graphic, enum Cmiss_graphics_coordinate_system coordinate_system)
{
	int return_code = 1;
	ENTER(Cmiss_graphic_set_coordinate_system);
	if (graphic)
	{
		if (coordinate_system != graphic->coordinate_system)
		{
			graphic->coordinate_system=coordinate_system;
			if (Cmiss_graphics_coordinate_system_is_window_relative(coordinate_system))
			{
				graphic->overlay_flag = 1;
				graphic->overlay_order = 1;
			}
			else
			{
				graphic->overlay_flag = 0;
				graphic->overlay_order = 0;
			}
			Cmiss_graphic_changed(graphic, CMISS_GRAPHIC_CHANGE_REDRAW);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_set_coordinate_system.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

enum Cmiss_graphic_type Cmiss_graphic_get_graphic_type(
	struct Cmiss_graphic *graphic)
{
	enum Cmiss_graphic_type graphic_type;

	ENTER(Cmiss_graphic_get_graphic_type);
	if (graphic)
	{
		graphic_type=graphic->graphic_type;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_get_graphic_type.  Invalid argument(s)");
		graphic_type = CMISS_GRAPHIC_LINES;
	}
	LEAVE;

	return (graphic_type);
}

int Cmiss_graphic_is_graphic_type(struct Cmiss_graphic *graphic,
	enum Cmiss_graphic_type graphic_type)
{
	int return_code = 0;

	ENTER(Cmiss_graphic_is_graphic_type);
	if (graphic)
	{
		if (graphic->graphic_type==graphic_type)
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
			"Cmiss_graphic_is_graphic_type.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}
int Cmiss_graphic_set_glyph_type(Cmiss_graphic_id graphic, Cmiss_graphic_glyph_type glyph_type)
{
	int return_code = 0;

	if (graphic && glyph_type != CMISS_GRAPHIC_GLYPH_TYPE_INVALID)
	{
		if ((graphic->graphic_type == CMISS_GRAPHIC_NODE_POINTS) ||
			(graphic->graphic_type == CMISS_GRAPHIC_DATA_POINTS) ||
			(graphic->graphic_type == CMISS_GRAPHIC_ELEMENT_POINTS) ||
			(graphic->graphic_type == CMISS_GRAPHIC_POINT))
		{
			GT_object *glyph;
			Graphic_glyph_scaling_mode glyph_scaling_mode;
			Triple glyph_centre,glyph_scale_factors,glyph_size;
			Computed_field *orientation_scale_field, *variable_scale_field; ;
			Cmiss_graphic_get_glyph_parameters(graphic,
				&glyph, &glyph_scaling_mode ,glyph_centre, glyph_size,
				&orientation_scale_field, glyph_scale_factors,
				&variable_scale_field);
			if (glyph_type == CMISS_GRAPHIC_GLYPH_AXES)
			{
				glyph = Cmiss_rendition_get_glyph_from_manager(graphic->rendition, "axes_solid");
			}
			return_code = Cmiss_graphic_set_glyph_parameters(graphic,glyph,
				glyph_scaling_mode, glyph_centre, glyph_size,
				orientation_scale_field, glyph_scale_factors,
				variable_scale_field);
		}
	}

	return return_code;
}

int Cmiss_graphic_get_visibility_flag(struct Cmiss_graphic *graphic)
{
	int return_code;

	ENTER(Cmiss_graphic_get_visibility_flag);
	if (graphic)
	{
		return_code = graphic->visibility_flag;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_get_visibility_flag.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

int Cmiss_graphic_set_visibility_flag(struct Cmiss_graphic *graphic,
	int visibility_flag)
{
	int return_code;

	ENTER(Cmiss_graphic_set_visibility_flag);
	if (graphic)
	{
		return_code = 1;
		bool bool_visibility_flag = visibility_flag == 1;
		if (graphic->visibility_flag != bool_visibility_flag)
		{
			graphic->visibility_flag = bool_visibility_flag;
			Cmiss_graphic_changed(graphic, CMISS_GRAPHIC_CHANGE_REDRAW);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_set_visibility_flag.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
}

int Cmiss_graphic_and_rendition_visibility_flags_set(struct Cmiss_graphic *graphic)
{
	int return_code;

	ENTER(Cmiss_graphic_and_rendition_visibility_flags_set);
	if (graphic)
	{
		if (graphic->visibility_flag && Cmiss_rendition_is_visible_hierarchical(graphic->rendition))
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
			"Cmiss_graphic_and_rendition_visibility_flags_set.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

int Cmiss_graphic_is_from_region_hierarchical(struct Cmiss_graphic *graphic, struct Cmiss_region *region)
{
	int return_code = 0;

	ENTER(Cmiss_graphic_is_from_region_hierarchical);
	if (graphic && region)
	{
		struct Cmiss_region *rendition_region = Cmiss_rendition_get_region(graphic->rendition);
		if ((rendition_region == region) ||
			(Cmiss_region_contains_subregion(region, rendition_region)))
		{
			return_code = 1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_is_from_region_hierarchical.  Invalid argument(s)");
	}

	return (return_code);
}

int Cmiss_graphic_get_glyph_parameters(
	struct Cmiss_graphic *graphic,
	struct GT_object **glyph, enum Graphic_glyph_scaling_mode *glyph_scaling_mode,
	Triple glyph_offset, Triple glyph_size,
	struct Computed_field **orientation_scale_field, Triple glyph_scale_factors,
	struct Computed_field **variable_scale_field)
{
	int return_code;

	ENTER(Cmiss_graphic_get_glyph_parameters);
	if (graphic && glyph && glyph_scaling_mode && glyph_offset && glyph_size &&
		((CMISS_GRAPHIC_NODE_POINTS==graphic->graphic_type) ||
			(CMISS_GRAPHIC_DATA_POINTS==graphic->graphic_type) ||
			(CMISS_GRAPHIC_ELEMENT_POINTS==graphic->graphic_type) ||
			(CMISS_GRAPHIC_POINT==graphic->graphic_type)) &&
		orientation_scale_field && glyph_scale_factors && variable_scale_field)
	{
		*glyph = graphic->glyph;
		*glyph_scaling_mode = graphic->glyph_scaling_mode;
		glyph_offset[0] = graphic->glyph_offset[0];
		glyph_offset[1] = graphic->glyph_offset[1];
		glyph_offset[2] = graphic->glyph_offset[2];
		glyph_size[0] = graphic->glyph_size[0];
		glyph_size[1] = graphic->glyph_size[1];
		glyph_size[2] = graphic->glyph_size[2];
		*orientation_scale_field = graphic->orientation_scale_field;
		glyph_scale_factors[0] = graphic->glyph_scale_factors[0];
		glyph_scale_factors[1] = graphic->glyph_scale_factors[1];
		glyph_scale_factors[2] = graphic->glyph_scale_factors[2];
		*variable_scale_field = graphic->variable_scale_field;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_get_glyph_parameters.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_get_glyph_parameters */

struct Computed_field *Cmiss_graphic_get_coordinate_field(
	struct Cmiss_graphic *graphic)
{
	struct Computed_field *coordinate_field;

	ENTER(Cmiss_grpahic_get_coordinate_field);
	if (graphic)
	{
		coordinate_field=graphic->coordinate_field;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_get_coordinate_field.  Invalid argument(s)");
		coordinate_field=(struct Computed_field *)NULL;
	}
	LEAVE;

	return (coordinate_field);
}

int Cmiss_graphic_set_coordinate_field(
	struct Cmiss_graphic *graphic,struct Computed_field *coordinate_field)
{
	int return_code = 1;

	ENTER(Cmiss_graphic_set_coordinate_field);
	if (graphic&&((!coordinate_field)||
		(3>=Computed_field_get_number_of_components(coordinate_field))))
	{
		if (coordinate_field != graphic->coordinate_field)
		{
			REACCESS(Computed_field)(&(graphic->coordinate_field),coordinate_field);
			Cmiss_graphic_changed(graphic, CMISS_GRAPHIC_CHANGE_FULL_REBUILD);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_set_coordinate_field.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
}

int Cmiss_graphic_set_face(struct Cmiss_graphic *graphic, int face)
{
	int return_code;

	ENTER(Cmiss_graphic_set_face);
	if (graphic&&(
		Cmiss_graphic_type_uses_dimension(graphic->graphic_type,1)||
		Cmiss_graphic_type_uses_dimension(graphic->graphic_type,2)))
	{
		return_code=1;
		/* want -1 to represent none/all faces */
		if ((0>face)||(5<face))
		{
			face = -1;
		}
		graphic->face = face;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_set_face.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_set_face */

int Cmiss_graphic_set_exterior(struct Cmiss_graphic *graphic,
	int exterior)
{
	int return_code;

	ENTER(Cmiss_graphic_set_exterior);
	if (graphic&&(
		Cmiss_graphic_type_uses_dimension(graphic->graphic_type,1)||
		Cmiss_graphic_type_uses_dimension(graphic->graphic_type,2)))
	{
		return_code=1;
		/* ensure flags are 0 or 1 to simplify comparison with other graphic */
		if (exterior)
		{
			graphic->exterior = 1;
		}
		else
		{
			graphic->exterior = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_set_exterior.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
}

int Cmiss_graphic_update_selected(struct Cmiss_graphic *graphic, void *dummy_void)
{
	USE_PARAMETER(dummy_void);
	if (graphic)
	{
		switch (graphic->select_mode)
		{
		case GRAPHICS_SELECT_ON:
			Cmiss_graphic_changed(graphic, CMISS_GRAPHIC_CHANGE_SELECTION);
			break;
		case GRAPHICS_NO_SELECT:
			/* nothing to do as no names put out with graphic */
			break;
		case GRAPHICS_DRAW_SELECTED:
		case GRAPHICS_DRAW_UNSELECTED:
			Cmiss_graphic_changed(graphic, CMISS_GRAPHIC_CHANGE_FULL_REBUILD);
			break;
		default:
			display_message(ERROR_MESSAGE,
				"Cmiss_graphic_update_selected.  Unknown select_mode");
			break;
		}
		return 1;
	}
	return 0;
}

/** replace materials and spectrums in existing graphics object */
int Cmiss_graphic_update_non_trivial_GT_objects(struct Cmiss_graphic *graphic)
{
	int return_code = 0;

	ENTER(Cmiss_graphic_update_non_trivial_GT_objects);
	if (graphic && graphic->graphics_object)
	{
		set_GT_object_default_material(graphic->graphics_object,
			graphic->material);
		set_GT_object_secondary_material(graphic->graphics_object,
			graphic->secondary_material);
		set_GT_object_selected_material(graphic->graphics_object,
			graphic->selected_material);
		if (graphic->data_field && graphic->spectrum)
		{
			set_GT_object_Spectrum(graphic->graphics_object,
				(void *)graphic->spectrum);
		}
		return_code = 1;
	}
	LEAVE;

	return return_code;
}

int Cmiss_graphic_set_material(struct Cmiss_graphic *graphic,
	struct Graphical_material *material)
{
	int return_code = 1;

	ENTER(Cmiss_graphic_set_material);
	if (graphic&&material)
	{
		if (material != graphic->material)
		{
			REACCESS(Graphical_material)(&(graphic->material),material);
			Cmiss_graphic_update_non_trivial_GT_objects(graphic);
			Cmiss_graphic_changed(graphic, CMISS_GRAPHIC_CHANGE_RECOMPILE);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_set_material.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_set_material */

int Cmiss_graphic_set_label_field(
	struct Cmiss_graphic *graphic,struct Computed_field *label_field,
	struct Cmiss_graphics_font *font)
{
	int return_code;

	ENTER(Cmiss_graphic_set_label_field);
	if (graphic&&((!label_field)||
		((CMISS_GRAPHIC_NODE_POINTS==graphic->graphic_type)||
			(CMISS_GRAPHIC_DATA_POINTS==graphic->graphic_type)||
			(CMISS_GRAPHIC_ELEMENT_POINTS==graphic->graphic_type)||
			(CMISS_GRAPHIC_POINT==graphic->graphic_type))) &&
		(!label_field || font))
	{
		REACCESS(Computed_field)(&(graphic->label_field), label_field);
		REACCESS(Cmiss_graphics_font)(&(graphic->font), font);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_set_label_field.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
}

int Cmiss_graphic_set_selected_material(
	struct Cmiss_graphic *graphic,
	struct Graphical_material *selected_material)
{
	int return_code = 1;

	ENTER(Cmiss_graphic_set_selected_material);
	if (graphic&&selected_material)
	{
		if (selected_material != graphic->selected_material)
		{
			REACCESS(Graphical_material)(&(graphic->selected_material),
				selected_material);
			Cmiss_graphic_update_non_trivial_GT_objects(graphic);
			Cmiss_graphic_changed(graphic, CMISS_GRAPHIC_CHANGE_RECOMPILE);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_set_selected_material.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
}

char *Cmiss_graphic_get_name(Cmiss_graphic_id graphic)
{
	char *name = NULL;
	if (graphic && graphic->name)
	{
		name = duplicate_string(graphic->name);
	}

	return name;
}

char *Cmiss_graphic_get_name_internal(struct Cmiss_graphic *graphic)
{
	char *name = 0;
	if (graphic)
	{
		if (graphic->name)
		{
			name = duplicate_string(graphic->name);
		}
		else
		{
			char temp[30];
			sprintf(temp, "%d", graphic->position);
			name = duplicate_string(temp);
		}
	}
	return name;
}

int Cmiss_graphic_set_name(struct Cmiss_graphic *graphic, const char *name)
{
	int return_code;

	ENTER(Cmiss_graphic_set_name);
	if (graphic&&name)
	{
		if (graphic->name)
		{
			DEALLOCATE(graphic->name);
		}
		if (ALLOCATE(graphic->name, char, strlen(name) + 1))
		{
			strcpy((char *)graphic->name, name);
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_set_name.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_set_name */

char *Cmiss_graphic_get_summary_string(struct Cmiss_graphic *graphic)
{
	if (!graphic)
		return 0;
	char *graphic_string = 0;
	int error = 0;
	char temp_string[100];
	if (graphic->name)
	{
		sprintf(temp_string, "%s. ", graphic->name);
	}
	else
	{
		sprintf(temp_string, "%i. ", graphic->position);
	}
	append_string(&graphic_string,temp_string,&error);
	append_string(&graphic_string,
		ENUMERATOR_STRING(Cmiss_graphic_type)(graphic->graphic_type),
		&error);
	if (graphic->subgroup_field)
	{
		char *name = Cmiss_field_get_name(graphic->subgroup_field);
		append_string(&graphic_string, " subgroup ", &error);
		append_string(&graphic_string, name, &error);
		DEALLOCATE(name);
	}
	return graphic_string;
}

char *Cmiss_graphic_string(struct Cmiss_graphic *graphic,
	enum Cmiss_graphic_string_details graphic_detail)
{
	char *graphic_string = NULL,temp_string[100],*name;
	int dimension,error,i;

	ENTER(Cmiss_graphic_string);
	graphic_string=(char *)NULL;
	if (graphic&&(
		(GRAPHIC_STRING_GEOMETRY==graphic_detail)||
		(GRAPHIC_STRING_COMPLETE==graphic_detail)||
		(GRAPHIC_STRING_COMPLETE_PLUS==graphic_detail)))
	{
		error=0;
		if (GRAPHIC_STRING_COMPLETE_PLUS==graphic_detail)
		{
			if (graphic->name)
			{
				sprintf(temp_string,"%i. (%s) ",graphic->position, graphic->name);
			}
			else
			{
				sprintf(temp_string,"%i. ",graphic->position);
			}
			append_string(&graphic_string,temp_string,&error);
		}

		/* show geometry graphic */
		/* for all graphic types */
		/* write graphic type = "points", "lines" etc. */
		append_string(&graphic_string,
			ENUMERATOR_STRING(Cmiss_graphic_type)(graphic->graphic_type),
			&error);
		if (graphic->name)
		{
			sprintf(temp_string," as %s", graphic->name);
			append_string(&graphic_string,temp_string,&error);
		}
		if (graphic->subgroup_field)
		{
			if (GET_NAME(Computed_field)(graphic->subgroup_field,&name))
			{
				/* put quotes around name if it contains special characters */
				make_valid_token(&name);
				append_string(&graphic_string," subgroup ",&error);
				append_string(&graphic_string,name,&error);
				DEALLOCATE(name);
			}
		}
		if (graphic->coordinate_field)
		{
			append_string(&graphic_string," coordinate ",&error);
			name=(char *)NULL;
			if (GET_NAME(Computed_field)(graphic->coordinate_field,&name))
			{
				/* put quotes around name if it contains special characters */
				make_valid_token(&name);
				append_string(&graphic_string,name,&error);
				DEALLOCATE(name);
			}
			else
			{
				append_string(&graphic_string,"NONE",&error);
			}
		}

		/* for 1-D and 2-D elements only */
		dimension=Cmiss_graphic_get_dimension(graphic, (struct FE_region *)NULL);
		if ((1==dimension)||(2==dimension))
		{
			if (graphic->exterior)
			{
				append_string(&graphic_string," exterior",&error);
			}
			if (-1 != graphic->face)
			{
				append_string(&graphic_string," face",&error);
				switch (graphic->face)
				{
					case 0:
					{
						append_string(&graphic_string," xi1_0",&error);
					} break;
					case 1:
					{
						append_string(&graphic_string," xi1_1",&error);
					} break;
					case 2:
					{
						append_string(&graphic_string," xi2_0",&error);
					} break;
					case 3:
					{
						append_string(&graphic_string," xi2_1",&error);
					} break;
					case 4:
					{
						append_string(&graphic_string," xi3_0",&error);
					} break;
					case 5:
					{
						append_string(&graphic_string," xi3_1",&error);
					} break;
					default:
					{
						display_message(ERROR_MESSAGE,
							"Cmiss_graphic_string.  Invalid face number");
						DEALLOCATE(graphic_string);
						error=1;
					} break;
				}
			}
		}

		if (Cmiss_graphic_type_uses_attribute(graphic->graphic_type,
			CMISS_GRAPHIC_ATTRIBUTE_DISCRETIZATION))
		{
			sprintf(temp_string, " discretization \"%d*%d*%d\"",
				graphic->discretization.number_in_xi1,
				graphic->discretization.number_in_xi2,
				graphic->discretization.number_in_xi3);
			append_string(&graphic_string,temp_string,&error);
		}

		if (Cmiss_graphic_type_uses_attribute(graphic->graphic_type,
			CMISS_GRAPHIC_ATTRIBUTE_TESSELLATION))
		{
			append_string(&graphic_string, " tessellation ", &error);
			if (graphic->tessellation)
			{
				name = Cmiss_tessellation_get_name(graphic->tessellation);
				/* put quotes around name if it contains special characters */
				make_valid_token(&name);
				append_string(&graphic_string, name, &error);
				DEALLOCATE(name);
			}
			else
			{
				append_string(&graphic_string,"NONE",&error);
			}
		}

		append_string(&graphic_string," ",&error);
		append_string(&graphic_string,
			ENUMERATOR_STRING(Cmiss_graphics_coordinate_system)(graphic->coordinate_system),&error);
		if (Cmiss_graphic_type_uses_attribute(graphic->graphic_type, CMISS_GRAPHIC_ATTRIBUTE_LINE_WIDTH))
		{
			if (0 != graphic->line_width)
			{
				sprintf(temp_string, " line_width %d", graphic->line_width);
				append_string(&graphic_string,temp_string,&error);
			}
		}

		if (CMISS_GRAPHIC_CYLINDERS==graphic->graphic_type)
		{
			sprintf(temp_string," circle_discretization %d",
				graphic->circle_discretization);
			append_string(&graphic_string,temp_string,&error);
			if (0.0 != graphic->constant_radius)
			{
				sprintf(temp_string," constant_radius %g",graphic->constant_radius);
				append_string(&graphic_string,temp_string,&error);
			}
			if (graphic->radius_scalar_field)
			{
				if (GET_NAME(Computed_field)(graphic->radius_scalar_field,&name))
				{
					/* put quotes around name if it contains special characters */
					make_valid_token(&name);
					append_string(&graphic_string," radius_scalar ",&error);
					append_string(&graphic_string,name,&error);
					DEALLOCATE(name);
					if (1.0 != graphic->radius_scale_factor)
					{
						sprintf(temp_string," scale_factor %g",
							graphic->radius_scale_factor);
						append_string(&graphic_string,temp_string,&error);
					}
				}
				else
				{
					DEALLOCATE(graphic_string);
					error=1;
				}
			}
		}
		/* overlay is temporarily disabled, instead the functionality is replaced
			 by coordinate_system
		if (CMISS_GRAPHIC_STATIC==graphic->graphic_type)
		{
			if (graphic->overlay_flag == 0 )
			{
				append_string(&graphic_string, " no_overlay ",&error);
			}
			else
			{
				sprintf(temp_string, " overlay %d", graphic->overlay_order);
				append_string(&graphic_string,temp_string,&error);
			}
		}
		*/

		/* for iso_surfaces only */
		if (CMISS_GRAPHIC_ISO_SURFACES==graphic->graphic_type)
		{
			if (graphic->iso_scalar_field)
			{
				if (GET_NAME(Computed_field)(graphic->iso_scalar_field,&name))
				{
					/* put quotes around name if it contains special characters */
					make_valid_token(&name);
					append_string(&graphic_string," iso_scalar ",&error);
					append_string(&graphic_string,name,&error);
					DEALLOCATE(name);
				}
				else
				{
					DEALLOCATE(graphic_string);
					error=1;
				}
			}
			if (graphic->iso_values)
			{
				sprintf(temp_string," iso_values");
				append_string(&graphic_string,temp_string,&error);
				for (i = 0 ; i < graphic->number_of_iso_values ; i++)
				{
					sprintf(temp_string, " %g", graphic->iso_values[i]);
					append_string(&graphic_string,temp_string,&error);
				}
			}
			else
			{
				sprintf(temp_string," range_number_of_iso_values %d",
					graphic->number_of_iso_values);
				append_string(&graphic_string,temp_string,&error);
				sprintf(temp_string," first_iso_value %g",
					graphic->first_iso_value);
				append_string(&graphic_string,temp_string,&error);
				sprintf(temp_string," last_iso_value %g",
					graphic->last_iso_value);
				append_string(&graphic_string,temp_string,&error);
			}
			if (graphic->decimation_threshold > 0.0)
			{
				sprintf(temp_string," decimation_threshold %g",
					graphic->decimation_threshold);
				append_string(&graphic_string,temp_string,&error);
			}
		}
		/* for node_points, data_points and element_points only */
		if ((CMISS_GRAPHIC_NODE_POINTS==graphic->graphic_type)||
			(CMISS_GRAPHIC_DATA_POINTS==graphic->graphic_type)||
			(CMISS_GRAPHIC_ELEMENT_POINTS==graphic->graphic_type)||
			(CMISS_GRAPHIC_POINT==graphic->graphic_type))
		{
			if (graphic->glyph)
			{
				append_string(&graphic_string," glyph ",&error);
				if (GET_NAME(GT_object)(graphic->glyph, &name))
				{
					append_string(&graphic_string,name,&error);
					DEALLOCATE(name);
				}
				append_string(&graphic_string," ",&error);
				append_string(&graphic_string,
					ENUMERATOR_STRING(Graphic_glyph_scaling_mode)(graphic->glyph_scaling_mode),
					&error);
				sprintf(temp_string," size \"%g*%g*%g\"",graphic->glyph_size[0],
					graphic->glyph_size[1],graphic->glyph_size[2]);
				append_string(&graphic_string,temp_string,&error);
				// legacy command uses negative offset as glyph centre
				Triple glyph_centre;
				for (int comp_no=0;(comp_no<3);comp_no++)
				{
					glyph_centre[comp_no] = graphic->glyph_offset[comp_no];
					// want to avoid values of -0.0
					if (glyph_centre[comp_no] != 0.0f)
					{
						glyph_centre[comp_no] = -glyph_centre[comp_no];
					}
				}
				sprintf(temp_string," centre %g,%g,%g",
					glyph_centre[0], glyph_centre[1], glyph_centre[2]);

				append_string(&graphic_string,temp_string,&error);
				if (graphic->font)
				{
					append_string(&graphic_string," font ",&error);
					if (GET_NAME(Cmiss_graphics_font)(graphic->font, &name))
					{
						append_string(&graphic_string,name,&error);
						DEALLOCATE(name);
					}
				}
				if (graphic->label_field)
				{
					if (GET_NAME(Computed_field)(graphic->label_field,&name))
					{
						/* put quotes around name if it contains special characters */
						make_valid_token(&name);
						append_string(&graphic_string," label ",&error);
						append_string(&graphic_string,name,&error);
						DEALLOCATE(name);
					}
				}
				if (graphic->label_density_field)
				{
					if (GET_NAME(Computed_field)(graphic->label_density_field,&name))
					{
						/* put quotes around name if it contains special characters */
						make_valid_token(&name);
						append_string(&graphic_string," ldensity ",&error);
						append_string(&graphic_string,name,&error);
						DEALLOCATE(name);
					}
				}
				if (graphic->orientation_scale_field)
				{
					if (GET_NAME(Computed_field)(graphic->orientation_scale_field,&name))
					{
						/* put quotes around name if it contains special characters */
						make_valid_token(&name);
						append_string(&graphic_string," orientation ",&error);
						append_string(&graphic_string,name,&error);
						DEALLOCATE(name);
					}
					else
					{
						DEALLOCATE(graphic_string);
						error=1;
					}
				}
				if (graphic->variable_scale_field)
				{
					if (GET_NAME(Computed_field)(graphic->variable_scale_field,&name))
					{
						/* put quotes around name if it contains special characters */
						make_valid_token(&name);
						append_string(&graphic_string," variable_scale ",&error);
						append_string(&graphic_string,name,&error);
						DEALLOCATE(name);
					}
					else
					{
						DEALLOCATE(graphic_string);
						error=1;
					}
				}
				if (graphic->orientation_scale_field || graphic->variable_scale_field)
				{
					sprintf(temp_string," scale_factors \"%g*%g*%g\"",
						graphic->glyph_scale_factors[0],
						graphic->glyph_scale_factors[1],
						graphic->glyph_scale_factors[2]);
					append_string(&graphic_string,temp_string,&error);
				}
			}
			else
			{
				append_string(&graphic_string," glyph none",&error);
			}
		}
		/* for element_points and iso_surfaces */
		if ((CMISS_GRAPHIC_ELEMENT_POINTS==graphic->graphic_type)||
			(CMISS_GRAPHIC_ISO_SURFACES==graphic->graphic_type))
		{
			sprintf(temp_string, " %s",
				ENUMERATOR_STRING(Use_element_type)(graphic->use_element_type));
			append_string(&graphic_string,temp_string,&error);
		}
		/* for element_points only */
		if ((CMISS_GRAPHIC_ELEMENT_POINTS==graphic->graphic_type) ||
			(CMISS_GRAPHIC_STREAMLINES==graphic->graphic_type))
		{
			append_string(&graphic_string," ",&error);
			append_string(&graphic_string, ENUMERATOR_STRING(Xi_discretization_mode)(
				graphic->xi_discretization_mode), &error);
			if (XI_DISCRETIZATION_EXACT_XI != graphic->xi_discretization_mode)
			{
				if ((XI_DISCRETIZATION_CELL_DENSITY ==
					graphic->xi_discretization_mode) ||
					(XI_DISCRETIZATION_CELL_POISSON == graphic->xi_discretization_mode))
				{
					append_string(&graphic_string, " density ", &error);
					if (graphic->xi_point_density_field)
					{
						if (GET_NAME(Computed_field)(graphic->xi_point_density_field,&name))
						{
							/* put quotes around name if it contains special characters */
							make_valid_token(&name);
							append_string(&graphic_string, name, &error);
							DEALLOCATE(name);
						}
						else
						{
							DEALLOCATE(graphic_string);
							error = 1;
						}
					}
					else
					{
						append_string(&graphic_string,"NONE",&error);
					}
				}
			}
		}

		if (Cmiss_graphic_type_uses_attribute(graphic->graphic_type,
			CMISS_GRAPHIC_ATTRIBUTE_NATIVE_DISCRETIZATION_FIELD))
		{
			if (graphic->native_discretization_field)
			{
				append_string(&graphic_string," native_discretization ", &error);
				if (GET_NAME(FE_field)(graphic->native_discretization_field,&name))
				{
					/* put quotes around name if it contains special characters */
					make_valid_token(&name);
					append_string(&graphic_string,name,&error);
					DEALLOCATE(name);
				}
				else
				{
					DEALLOCATE(graphic_string);
					error=1;
				}
			}
		}

		/* for graphic starting in a particular element */
		if (CMISS_GRAPHIC_STREAMLINES==graphic->graphic_type)
		{
			if (graphic->seed_element)
			{
				sprintf(temp_string, " seed_element %d",
					FE_element_get_cm_number(graphic->seed_element));
				append_string(&graphic_string, temp_string, &error);
			}
		}

		/* for graphic requiring an exact xi location */
		if (((CMISS_GRAPHIC_ELEMENT_POINTS==graphic->graphic_type) ||
			(CMISS_GRAPHIC_STREAMLINES==graphic->graphic_type))&&
			(XI_DISCRETIZATION_EXACT_XI == graphic->xi_discretization_mode))
		{
			sprintf(temp_string," xi %g,%g,%g",
				graphic->seed_xi[0],graphic->seed_xi[1],graphic->seed_xi[2]);
			append_string(&graphic_string,temp_string,&error);
		}

		/* for streamlines only */
		if (CMISS_GRAPHIC_STREAMLINES==graphic->graphic_type)
		{
			append_string(&graphic_string," ",&error);
			append_string(&graphic_string,
				ENUMERATOR_STRING(Streamline_type)(graphic->streamline_type),&error);
			if (graphic->stream_vector_field)
			{
				if (GET_NAME(Computed_field)(graphic->stream_vector_field,&name))
				{
					/* put quotes around name if it contains special characters */
					make_valid_token(&name);
					append_string(&graphic_string," vector ",&error);
					append_string(&graphic_string,name,&error);
					DEALLOCATE(name);
				}
				else
				{
					DEALLOCATE(graphic_string);
					error=1;
				}
			}
			if (graphic->reverse_track)
			{
				append_string(&graphic_string," reverse_track ",&error);
			}
			sprintf(temp_string," length %g width %g ",
				graphic->streamline_length,graphic->streamline_width);
			append_string(&graphic_string,temp_string,&error);
			append_string(&graphic_string,
				ENUMERATOR_STRING(Streamline_data_type)(graphic->streamline_data_type),&error);
			if (graphic->seed_nodeset)
			{
				append_string(&graphic_string, " seed_nodeset ", &error);
				char *nodeset_name = Cmiss_nodeset_get_name(graphic->seed_nodeset);
				make_valid_token(&nodeset_name);
				append_string(&graphic_string, nodeset_name, &error);
				DEALLOCATE(nodeset_name);
			}
			if (graphic->seed_node_mesh_location_field)
			{
				if (GET_NAME(Computed_field)(graphic->seed_node_mesh_location_field,&name))
				{
					/* put quotes around name if it contains special characters */
					make_valid_token(&name);
					append_string(&graphic_string," seed_node_mesh_location_field ",&error);
					append_string(&graphic_string,name,&error);
					DEALLOCATE(name);
				}
				else
				{
					DEALLOCATE(graphic_string);
					error=1;
				}
			}
		}
		append_string(&graphic_string," ",&error);
		append_string(&graphic_string,
			ENUMERATOR_STRING(Graphics_select_mode)(graphic->select_mode),&error);

		if ((GRAPHIC_STRING_COMPLETE==graphic_detail)||
			(GRAPHIC_STRING_COMPLETE_PLUS==graphic_detail))
		{
			/* show appearance graphic */
			/* for all graphic types */
			if (!graphic->visibility_flag)
			{
				append_string(&graphic_string," invisible",&error);
			}
			if (graphic->material&&
				GET_NAME(Graphical_material)(graphic->material,&name))
			{
				/* put quotes around name if it contains special characters */
				make_valid_token(&name);
				append_string(&graphic_string," material ",&error);
				append_string(&graphic_string,name,&error);
				DEALLOCATE(name);
			}
			if (graphic->secondary_material&&
				GET_NAME(Graphical_material)(graphic->secondary_material,&name))
			{
				/* put quotes around name if it contains special characters */
				make_valid_token(&name);
				append_string(&graphic_string," secondary_material ",&error);
				append_string(&graphic_string,name,&error);
				DEALLOCATE(name);
			}
			if (graphic->texture_coordinate_field)
			{
				if (GET_NAME(Computed_field)(graphic->texture_coordinate_field,&name))
				{
					/* put quotes around name if it contains special characters */
					make_valid_token(&name);
					append_string(&graphic_string," texture_coordinates ",&error);
					append_string(&graphic_string,name,&error);
					DEALLOCATE(name);
				}
				else
				{
					DEALLOCATE(graphic_string);
					error=1;
				}
			}
			if (graphic->data_field)
			{
				if (GET_NAME(Computed_field)(graphic->data_field,&name))
				{
					/* put quotes around name if it contains special characters */
					make_valid_token(&name);
					append_string(&graphic_string," data ",&error);
					append_string(&graphic_string,name,&error);
					DEALLOCATE(name);
				}
				else
				{
					DEALLOCATE(graphic_string);
					error=1;
				}
				if (graphic->spectrum&&
					GET_NAME(Spectrum)(graphic->spectrum,&name))
				{
					/* put quotes around name if it contains special characters */
					make_valid_token(&name);
					append_string(&graphic_string," spectrum ",&error);
					append_string(&graphic_string,name,&error);
					DEALLOCATE(name);
				}
			}
			if (graphic->selected_material&&
				GET_NAME(Graphical_material)(graphic->selected_material,&name))
			{
				/* put quotes around name if it contains special characters */
				make_valid_token(&name);
				append_string(&graphic_string," selected_material ",&error);
				append_string(&graphic_string,name,&error);
				DEALLOCATE(name);
			}
			/* for surfaces and volumes */
			if ((CMISS_GRAPHIC_CYLINDERS==graphic->graphic_type)
				|| (CMISS_GRAPHIC_SURFACES==graphic->graphic_type)
				|| (CMISS_GRAPHIC_ISO_SURFACES==graphic->graphic_type))
			{
				append_string(&graphic_string," ",&error);
				append_string(&graphic_string,
					ENUMERATOR_STRING(Cmiss_graphics_render_type)(graphic->render_type),&error);
			}
		}
		if (error)
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_graphic_string.  Error creating string");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_string.  Invalid argument(s)");
	}
	LEAVE;

	return graphic_string;
} /* Cmiss_graphic_string */

int Cmiss_graphic_to_point_object_at_time(
	struct Cmiss_graphic *graphic, Cmiss_field_cache_id field_cache, FE_value time,
	GLfloat graphics_object_primitive_time)
{
	FE_value base_size[3], offset[3], scale_factors[3];
	int return_code = 1;
	struct GT_glyph_set *glyph_set;
	char **labels = NULL;
	ENTER(Cmiss_graphic_to_point_object_at_time);
	if (graphic && field_cache)
	{
		Cmiss_field_cache_set_time(field_cache, time);
		if (!graphic->customised_graphics_object)
		{
			if (graphic->label_field)
			{
				ALLOCATE(labels, char *, 1);
				*labels = Cmiss_field_evaluate_string(graphic->label_field, field_cache);
			}
			GT_object_remove_primitives_at_time(
				graphic->graphics_object, graphics_object_primitive_time,
				(GT_object_primitive_object_name_conditional_function *)NULL,
				(void *)NULL);
			base_size[0] = (FE_value)(graphic->glyph_size[0]);
			base_size[1] = (FE_value)(graphic->glyph_size[1]);
			base_size[2] = (FE_value)(graphic->glyph_size[2]);
			offset[0] = (FE_value)(graphic->glyph_offset[0]);
			offset[1] = (FE_value)(graphic->glyph_offset[1]);
			offset[2] = (FE_value)(graphic->glyph_offset[2]);
			scale_factors[0] = (FE_value)(graphic->glyph_scale_factors[0]);
			scale_factors[1] = (FE_value)(graphic->glyph_scale_factors[1]);
			scale_factors[2] = (FE_value)(graphic->glyph_scale_factors[2]);

			Triple *point_list, *axis1_list, *axis2_list, *axis3_list,
				*scale_list;
			ALLOCATE(point_list, Triple, 1);
			ALLOCATE(axis1_list, Triple, 1);
			ALLOCATE(axis2_list, Triple, 1);
			ALLOCATE(axis3_list, Triple, 1);
			ALLOCATE(scale_list, Triple, 1);
			glyph_set = CREATE(GT_glyph_set)(1,
				point_list, axis1_list, axis2_list, axis3_list, scale_list, graphic->glyph, graphic->font,
				labels, /*n_data_components*/0, /*data*/(GLfloat *)NULL,
				/*label_bounds_dimension*/0, /*label_bounds_components*/0, /*label_bounds*/(ZnReal *)NULL,
				/*label_density_list*/(Triple *)NULL, /*object_name*/0, /*names*/(int *)NULL);
			/* NOT an error if no glyph_set produced == empty group */
			(*point_list)[0] = (GLfloat)offset[0];
			(*point_list)[1] = (GLfloat)offset[1];
			(*point_list)[2] = (GLfloat)offset[2];
			(*axis1_list)[0] = (GLfloat)base_size[0];
			(*axis1_list)[1] = 0.0;
			(*axis1_list)[2] = 0.0;
			(*axis2_list)[0] = 0.0;
			(*axis2_list)[1] = (GLfloat)base_size[1];
			(*axis2_list)[2] = 0.0;
			(*axis3_list)[0] = 0.0;
			(*axis3_list)[1] = 0.0;
			(*axis3_list)[2] = (GLfloat)base_size[2];
			(*scale_list)[0] = (GLfloat)scale_factors[0];
			(*scale_list)[1] = (GLfloat)scale_factors[1];
			(*scale_list)[2] = (GLfloat)scale_factors[2];
			if (glyph_set)
			{
				if (!GT_OBJECT_ADD(GT_glyph_set)(graphic->graphics_object,
					graphics_object_primitive_time,glyph_set))
				{
					DESTROY(GT_glyph_set)(&glyph_set);
					return_code=0;
				}
			}
		}
		else
		{
			if (graphic->graphics_object != graphic->customised_graphics_object)
			{
				REACCESS(GT_object)(&graphic->graphics_object,
					graphic->customised_graphics_object);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_to_point_object_at_time.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

#if defined (USE_OPENCASCADE)
static int Cad_shape_to_graphics_object(struct Computed_field *field,
	struct Cmiss_graphic_to_graphics_object_data *graphic_to_object_data)
{
	int return_code = 0;
	GLfloat time = 0.0;
	struct Cmiss_graphic *graphic = graphic_to_object_data->graphic;
	Cmiss_field_cad_topology_id cad_topology = Cmiss_field_cast_cad_topology(field);

	if (cad_topology)
	{
		switch (graphic->graphic_type)
		{
			case CMISS_GRAPHIC_SURFACES:
			{
				//printf( "Building cad geometry surfaces\n" );
				int surface_count = Cmiss_field_cad_topology_get_surface_count(cad_topology);
				if (surface_count > 0)
				{
					return_code = 1;
				}
				for (int i = 0; i < surface_count && return_code; i++)
				{
					Cmiss_cad_surface_identifier identifier = i;
					struct GT_surface *surface = create_surface_from_cad_shape(cad_topology, graphic_to_object_data->field_cache, graphic_to_object_data->rc_coordinate_field, graphic->data_field, graphic->render_type, identifier);
					if (surface && GT_OBJECT_ADD(GT_surface)(graphic->graphics_object, time, surface))
					{
						//printf( "Surface added to graphics object\n" );
						return_code = 1;
					}
					else
					{
						return_code = 0;
					}
				}
				break;
			}
			case CMISS_GRAPHIC_LINES:
			{
				//struct GT_object *graphics_object = settings->graphics_object;
				/*
				GT_polyline_vertex_buffers *lines =
					CREATE(GT_polyline_vertex_buffers)(
					g_PLAIN, settings->line_width);
				*/
				GT_polyline_vertex_buffers *lines = create_curves_from_cad_shape(cad_topology, graphic_to_object_data->field_cache, graphic_to_object_data->rc_coordinate_field, graphic->data_field, graphic->graphics_object);
				if (lines && GT_OBJECT_ADD(GT_polyline_vertex_buffers)(
					graphic->graphics_object, lines))
				{
					//printf("Adding lines for cad shape\n");
					return_code = 1;
				}
				else
				{
					//DESTROY(GT_polyline_vertex_buffers)(&lines);
					return_code = 0;
				}
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,"Cad_geometry_to_graphics_object.  "
					"Can't handle this type of graphic");
				return_code = 0;
			}
		}
		Cmiss_field_destroy((Cmiss_field_id*)&cad_topology);
	}

	return return_code;
}
#endif /* (USE_OPENCASCADE) */

/***************************************************************************//**
 * @return  1 if all compulsory attributes or fields of graphic are set,
 * 0 otherwise.
 */
static int Cmiss_graphic_has_all_compulsory_attributes(struct Cmiss_graphic *graphic)
{
	int return_code = 1;
	if (graphic)
	{
		if (((CMISS_GRAPHIC_ELEMENT_POINTS == graphic->graphic_type) ||
			(CMISS_GRAPHIC_STREAMLINES == graphic->graphic_type)) &&
			((XI_DISCRETIZATION_CELL_DENSITY == graphic->xi_discretization_mode) ||
				(XI_DISCRETIZATION_CELL_POISSON == graphic->xi_discretization_mode)) &&
			(!graphic->xi_point_density_field))
		{
			return_code = 0;
		}
	}
	else
	{
		return_code = 0;
	}
	return return_code;
}

#if defined (USE_OPENCASCADE)
SubObjectGroupHighlightFunctor *create_highlight_functor_cad_primitive(
	struct Computed_field *group_field, Cmiss_field_cad_topology_id cad_topology_domain)
{
	SubObjectGroupHighlightFunctor *highlight_functor = NULL;
	if (group_field)
	{
		Cmiss_field_group_id sub_group = Cmiss_field_cast_group(group_field);

		//Cmiss_field_id cad_primitive_group_field = Cmiss_field_group_get_cad_primitive_group(sub_group, cad_topology_domain);
		Cmiss_field_id cad_primitive_subgroup_field = Cmiss_field_group_get_subobject_group_for_domain(sub_group,
			reinterpret_cast<Cmiss_field_id>(cad_topology_domain));
		Cmiss_field_cad_primitive_group_template_id cad_primitive_group = NULL;
		if (cad_primitive_subgroup_field)
		{
			cad_primitive_group =
				Cmiss_field_cast_cad_primitive_group_template(cad_primitive_subgroup_field);
			Cmiss_field_destroy(&cad_primitive_subgroup_field);
			if (cad_primitive_group)
			{
				Computed_field_sub_group_object<Cmiss_cad_identifier_id> *group_core =
					Computed_field_sub_group_object_core_cast<Cmiss_cad_identifier_id,
					Cmiss_field_cad_primitive_group_template_id>(cad_primitive_group);
				highlight_functor =
					new SubObjectGroupHighlightFunctor(group_core,
					&Computed_field_subobject_group::isIdentifierInList);
				Cmiss_field_id temporary_handle =
					reinterpret_cast<Computed_field *>(cad_primitive_group);
				Cmiss_field_destroy(&temporary_handle);
			}
		}
		if (sub_group)
		{
			Cmiss_field_group_destroy(&sub_group);
		}
	}

	return (highlight_functor);
}
#endif /* defined (USE_OPENCASCADE) */

SubObjectGroupHighlightFunctor *create_highlight_functor_element(
	struct Computed_field *group_field, Cmiss_mesh_id mesh)
{
  SubObjectGroupHighlightFunctor *highlight_functor = NULL;
  if (group_field)
  {
	Cmiss_field_group_id sub_group = Cmiss_field_cast_group(group_field);
	  if (Cmiss_field_group_contains_local_region(sub_group))
	  {
		highlight_functor =	new SubObjectGroupHighlightFunctor(NULL, NULL);
		highlight_functor->setContainsAll(1);
	  }
	  else
	  {
		Cmiss_field_element_group_id element_group = Cmiss_field_group_get_element_group(sub_group, mesh);
			if (element_group)
			{
				Computed_field_element_group *group_core =
					Computed_field_element_group_core_cast(element_group);
				highlight_functor =
					new SubObjectGroupHighlightFunctor(group_core,
					&Computed_field_subobject_group::isIdentifierInList);
				Cmiss_field_element_group_destroy(&element_group);
		}
		}
	if (sub_group)
	{
	  Cmiss_field_group_destroy(&sub_group);
	}
  }

  return (highlight_functor);
}

SubObjectGroupHighlightFunctor *create_highlight_functor_nodeset(
	struct Computed_field *group_field, Cmiss_nodeset_id nodeset)
{
  SubObjectGroupHighlightFunctor *highlight_functor = NULL;
	if (group_field)
	{
	  Cmiss_field_group_id sub_group = Cmiss_field_cast_group(group_field);
	  if (Cmiss_field_group_contains_local_region(sub_group))
	  {
		highlight_functor = new SubObjectGroupHighlightFunctor(NULL, NULL);
		highlight_functor->setContainsAll(1);
	  }
	  else
	  {
		Cmiss_field_node_group_id node_group = Cmiss_field_group_get_node_group(sub_group, nodeset);
		if (node_group)
		{
			Computed_field_node_group *group_core =
				Computed_field_node_group_core_cast(node_group);
			highlight_functor =	new SubObjectGroupHighlightFunctor(group_core,
				&Computed_field_subobject_group::isIdentifierInList);
			Cmiss_field_node_group_destroy(&node_group);
		}
	  }
	if (sub_group)
		{
		Cmiss_field_group_destroy(&sub_group);
		}
	}

	return (highlight_functor);
}

int Cmiss_graphic_remove_renderer_highlight_functor(struct Cmiss_graphic *graphic,
	Render_graphics *renderer)
{
	if (graphic && renderer)
	{
		renderer->set_highlight_functor(NULL);
		return 1;
	}
	return 0;
}

int Cmiss_graphic_set_renderer_highlight_functor(struct Cmiss_graphic *graphic, Render_graphics *renderer)
{
	int return_code = 0;

		if (graphic && renderer && graphic->rendition)
		{
			Cmiss_field_id group_field =
				Cmiss_rendition_get_selection_group_private_for_highlighting(graphic->rendition);
			Cmiss_field_module_id field_module = NULL;
			if (group_field &&
				(NULL != (field_module = Cmiss_field_get_field_module(group_field))))
			{
				if ((GRAPHICS_SELECT_ON == graphic->select_mode) ||
					(GRAPHICS_DRAW_SELECTED == graphic->select_mode))
				{
					switch (graphic->graphic_type)
					{
						case CMISS_GRAPHIC_DATA_POINTS:
						{
							Cmiss_nodeset_id nodeset = Cmiss_field_module_find_nodeset_by_name(
								field_module, "cmiss_data");
							SubObjectGroupHighlightFunctor *functor = create_highlight_functor_nodeset(
								group_field, nodeset);
							if (!(renderer->set_highlight_functor(functor)) && functor)
							{
									delete functor;
							}
							Cmiss_nodeset_destroy(&nodeset);
						} break;
						case CMISS_GRAPHIC_NODE_POINTS:
						{
							Cmiss_nodeset_id nodeset = Cmiss_field_module_find_nodeset_by_name(
								field_module, "cmiss_nodes");
							SubObjectGroupHighlightFunctor *functor = create_highlight_functor_nodeset(
								group_field, nodeset);
							if (!(renderer->set_highlight_functor(functor)) && functor)
							{
									delete functor;
							}
							Cmiss_nodeset_destroy(&nodeset);
						} break;
						case CMISS_GRAPHIC_CYLINDERS:
						case CMISS_GRAPHIC_LINES:
						case CMISS_GRAPHIC_ISO_SURFACES:
						case CMISS_GRAPHIC_ELEMENT_POINTS:
						{
							FE_region *fe_region = Cmiss_region_get_FE_region(Cmiss_field_module_get_region_internal(field_module));
							int dimension = Cmiss_graphic_get_dimension(graphic, fe_region);
							Cmiss_mesh_id temp_mesh = Cmiss_field_module_find_mesh_by_dimension(field_module, dimension);
							SubObjectGroupHighlightFunctor *functor =
								create_highlight_functor_element(group_field, temp_mesh);
							if (!(renderer->set_highlight_functor(functor)) && functor)
							{
									delete functor;
							}
							Cmiss_mesh_destroy(&temp_mesh);
						} break;
						case CMISS_GRAPHIC_SURFACES:
#if defined(USE_OPENCASCADE)
						{
							// test here for domain of object coordinate_field
							// if it is a cad_geometry do something about it
							struct LIST(Computed_field) *domain_field_list = CREATE_LIST(Computed_field)();
							int return_code = Computed_field_get_domain( graphic->coordinate_field, domain_field_list );
							if ( return_code )
							{
								// so test for topology domain
								struct Computed_field *cad_topology_field = FIRST_OBJECT_IN_LIST_THAT(Computed_field)
									( Cmiss_field_is_type_cad_topology, (void *)NULL, domain_field_list );
								if ( cad_topology_field )
								{
									Cmiss_field_cad_topology_id cad_topology_domain =
										Cmiss_field_cast_cad_topology(cad_topology_field);
									SubObjectGroupHighlightFunctor *functor = create_highlight_functor_cad_primitive(
										group_field, cad_topology_domain);
									if (!(renderer->set_highlight_functor(functor)) && functor)
									{
										if (functor)
											delete functor;
									}
									DESTROY_LIST(Computed_field)(&domain_field_list);
									break;
								}
							}
							if ( domain_field_list )
								DESTROY_LIST(Computed_field)(&domain_field_list);
						}
#else
						{
							Cmiss_mesh_id temp_mesh = Cmiss_field_module_find_mesh_by_dimension(
								field_module, /*dimension*/2);
							SubObjectGroupHighlightFunctor *functor = create_highlight_functor_element(
								group_field, temp_mesh);
							if (!(renderer->set_highlight_functor(functor)) && functor)
							{
									delete functor;
							}
							Cmiss_mesh_destroy(&temp_mesh);
						} break;
#endif /* defined(USE_OPENCASCADE) */
						case CMISS_GRAPHIC_POINT:
						case CMISS_GRAPHIC_STREAMLINES:
						{
							/* no element to select by since go through several */
						} break;
						default:
						{
							display_message(ERROR_MESSAGE,
								"cmiss_graphic_to_graphics_object.  "
								"Unknown graphic type");
						} break;
					}
				}
				Cmiss_field_module_destroy(&field_module);
			}
			return_code = 1;
		}

	return return_code;
}

int Cmiss_graphic_get_iteration_domain(Cmiss_graphic_id graphic,
	Cmiss_graphic_to_graphics_object_data *graphic_to_object_data)
{
	if (!graphic || !graphic_to_object_data)
		return 0;
	graphic_to_object_data->master_mesh = 0;
	graphic_to_object_data->iteration_mesh = 0;
	int dimension = Cmiss_graphic_get_dimension(graphic, graphic_to_object_data->fe_region);
	if (dimension > 0)
	{
		graphic_to_object_data->master_mesh =
			Cmiss_field_module_find_mesh_by_dimension(graphic_to_object_data->field_module, dimension);
		if (graphic->subgroup_field)
		{
			Cmiss_field_group_id group = Cmiss_field_cast_group(graphic->subgroup_field);
			if (group)
			{
				Cmiss_field_element_group_id element_group = Cmiss_field_group_get_element_group(group, graphic_to_object_data->master_mesh);
				if (element_group)
				{
					graphic_to_object_data->iteration_mesh =
						Cmiss_mesh_group_base_cast(Cmiss_field_element_group_get_mesh(element_group));
					Cmiss_field_element_group_destroy(&element_group);
				}
				Cmiss_field_group_destroy(&group);
			}
			else
			{
				Cmiss_field_element_group_id element_group = Cmiss_field_cast_element_group(graphic->subgroup_field);
				if (element_group)
				{
					// check group is for same master mesh
					graphic_to_object_data->iteration_mesh = Cmiss_mesh_group_base_cast(Cmiss_field_element_group_get_mesh(element_group));
					Cmiss_mesh_id temp_master_mesh = Cmiss_mesh_get_master(graphic_to_object_data->iteration_mesh);
					if (!Cmiss_mesh_match(graphic_to_object_data->master_mesh, temp_master_mesh))
					{
						Cmiss_mesh_destroy(&graphic_to_object_data->iteration_mesh);
					}
					Cmiss_mesh_destroy(&temp_master_mesh);
					Cmiss_field_element_group_destroy(&element_group);
				}
				else
				{
					graphic_to_object_data->iteration_mesh = Cmiss_mesh_access(graphic_to_object_data->master_mesh);
				}
			}
		}
		else
		{
			graphic_to_object_data->iteration_mesh = Cmiss_mesh_access(graphic_to_object_data->master_mesh);
		}
	}
	return (0 != graphic_to_object_data->iteration_mesh);
}

static char *Cmiss_graphic_get_graphics_object_name(Cmiss_graphic *graphic, const char *name_prefix)
{
	if (!graphic || !name_prefix)
		return 0;
	int error = 0;
	char *graphics_object_name = 0;
	append_string(&graphics_object_name, name_prefix, &error);
	if (graphic->subgroup_field)
	{
		char *subgroup_name = Cmiss_field_get_name(graphic->subgroup_field);
		append_string(&graphics_object_name, subgroup_name, &error);
		append_string(&graphics_object_name, "/", &error);
		DEALLOCATE(subgroup_name);
	}
	append_string(&graphics_object_name, ".", &error);
	char temp[20];
	sprintf(temp, "%d", graphic->position);
	append_string(&graphics_object_name, temp, &error);
	if (graphic->name)
	{
		append_string(&graphics_object_name, "_", &error);
		append_string(&graphics_object_name, graphic->name, &error);
	}
	return graphics_object_name;
}

static int Cmiss_mesh_to_graphics(Cmiss_mesh_id mesh, Cmiss_graphic_to_graphics_object_data *graphic_to_object_data)
{
	int return_code = 1;
	Cmiss_element_iterator_id iterator = Cmiss_mesh_create_element_iterator(mesh);
	Cmiss_element_id element = 0;
	while (0 != (element = Cmiss_element_iterator_next_non_access(iterator)))
	{
		if (!FE_element_to_graphics_object(element, graphic_to_object_data))
		{
			return_code = 0;
			break;
		}
	}
	Cmiss_element_iterator_destroy(&iterator);
	return return_code;
}

int Cmiss_graphic_to_graphics_object(
	struct Cmiss_graphic *graphic,void *graphic_to_object_data_void)
{
	char *existing_name, *graphic_string;
	FE_value base_size[3], offset[3], scale_factors[3];
	GLfloat time;
	enum GT_object_type graphics_object_type;
	int return_code;
	struct FE_region *fe_region;

	ENTER(Cmiss_graphic_to_graphics_object);
	struct Cmiss_graphic_to_graphics_object_data *graphic_to_object_data =
		reinterpret_cast<struct Cmiss_graphic_to_graphics_object_data *>(graphic_to_object_data_void);
	if (graphic && graphic_to_object_data &&
		(((CMISS_GRAPHIC_DATA_POINTS == graphic->graphic_type) &&
			(fe_region = graphic_to_object_data->data_fe_region)) ||
			(fe_region = graphic_to_object_data->fe_region)))
	{
		int dimension = Cmiss_graphic_get_dimension(graphic, graphic_to_object_data->fe_region);
		/* all primitives added at time 0.0 */
		time = 0.0;
		return_code = 1;
		/* build only if visible... */
		Cmiss_graphics_filter_id filter = Cmiss_scene_get_filter(graphic_to_object_data->scene);
		if (filter)
		{
			/* build only if changed... and complete */
			if (Cmiss_graphics_filter_evaluate_graphic(filter, graphic) &&
				  Cmiss_graphic_has_all_compulsory_attributes(graphic))
			{
				if (graphic->graphics_changed)
				{
					Computed_field *coordinate_field = graphic->coordinate_field;
					if (coordinate_field ||
						(graphic->graphic_type == CMISS_GRAPHIC_POINT))
					{
						/* RC coordinate_field to pass to FE_element_to_graphics_object */
						graphic_to_object_data->rc_coordinate_field = (Cmiss_field_id)0;
						graphic_to_object_data->wrapper_orientation_scale_field = (Cmiss_field_id)0;
						graphic_to_object_data->wrapper_stream_vector_field = (Cmiss_field_id)0;
						if (coordinate_field)
						{
							graphic_to_object_data->rc_coordinate_field =
								Computed_field_begin_wrap_coordinate_field(coordinate_field);
							if (!graphic_to_object_data->rc_coordinate_field)
							{
								display_message(ERROR_MESSAGE,
									"cmiss_graphic_to_graphics_object.  Could not get rc_coordinate_field wrapper");
								return_code = 0;
							}
						}
						if (return_code && graphic->orientation_scale_field)
						{
							graphic_to_object_data->wrapper_orientation_scale_field =
								Computed_field_begin_wrap_orientation_scale_field(
									graphic->orientation_scale_field, graphic_to_object_data->rc_coordinate_field);
							if (!graphic_to_object_data->wrapper_orientation_scale_field)
							{
								display_message(ERROR_MESSAGE,
									"cmiss_graphic_to_graphics_object.  Could not get orientation_scale_field wrapper");
								return_code = 0;
							}
						}
						if (return_code && graphic->stream_vector_field)
						{
							graphic_to_object_data->wrapper_stream_vector_field =
								Computed_field_begin_wrap_orientation_scale_field(
									graphic->stream_vector_field, graphic_to_object_data->rc_coordinate_field);
							if (!graphic_to_object_data->wrapper_stream_vector_field)
							{
								display_message(ERROR_MESSAGE,
									"cmiss_graphic_to_graphics_object.  Could not get stream_vector_field wrapper");
								return_code = 0;
							}
						}
						if (return_code)
						{
#if defined (DEBUG_CODE)
							/*???debug*/
							if ((graphic_string = Cmiss_graphic_string(graphic,
								GRAPHIC_STRING_COMPLETE_PLUS)) != NULL)
							{
								printf("> building %s\n", graphic_string);
								DEALLOCATE(graphic_string);
							}
#endif /* defined (DEBUG_CODE) */
							Cmiss_graphic_get_top_level_number_in_xi(graphic,
								MAXIMUM_ELEMENT_XI_DIMENSIONS, graphic_to_object_data->top_level_number_in_xi);
							graphic_to_object_data->existing_graphics =
								(struct GT_object *)NULL;
							/* work out the name the graphics object is to have */
							char *graphics_object_name = Cmiss_graphic_get_graphics_object_name(graphic, graphic_to_object_data->name_prefix);
							if (graphics_object_name)
							{
								if (graphic->graphics_object)
								{
									/* replace the graphics object name */
									GT_object_set_name(graphic->graphics_object,
										graphics_object_name);
									if (GT_object_has_primitives_at_time(graphic->graphics_object,
										time))
									{
#if defined (DEBUG_CODE)
										/*???debug*/printf("  EDIT EXISTING GRAPHICS!\n");
#endif /* defined (DEBUG_CODE) */
										GET_NAME(GT_object)(graphic->graphics_object, &existing_name);
										graphic_to_object_data->existing_graphics =
											CREATE(GT_object)(existing_name,
												GT_object_get_type(graphic->graphics_object),
												get_GT_object_default_material(graphic->graphics_object));
										DEALLOCATE(existing_name);
										GT_object_transfer_primitives_at_time(
											graphic_to_object_data->existing_graphics,
											graphic->graphics_object, time);
									}
								}
								else
								{
									switch (graphic->graphic_type)
									{
										case CMISS_GRAPHIC_LINES:
										{
											graphics_object_type = g_POLYLINE_VERTEX_BUFFERS;
										} break;
										case CMISS_GRAPHIC_CYLINDERS:
										case CMISS_GRAPHIC_SURFACES:
										{
											graphics_object_type = g_SURFACE;
										} break;
										case CMISS_GRAPHIC_ISO_SURFACES:
										{
											switch (dimension)
											{
												case 3:
												{
													//graphics_object_type = g_VOLTEX; // for old isosurfaces
													graphics_object_type = g_SURFACE; // for new isosurfaces
												} break;
												case 2:
												{
													graphics_object_type = g_POLYLINE;
												} break;
												case 1:
												{
													display_message(ERROR_MESSAGE,
														"Cmiss_graphic_to_graphics_object.  "
														"iso_scalars of 1-D elements is not supported");
													return_code = 0;
												} break;
												default:
												{
													display_message(ERROR_MESSAGE,
														"Cmiss_graphic_to_graphics_object.  "
														"Invalid dimension for iso_scalars");
													return_code = 0;
												} break;
											}
										} break;
										case CMISS_GRAPHIC_NODE_POINTS:
										case CMISS_GRAPHIC_DATA_POINTS:
										case CMISS_GRAPHIC_ELEMENT_POINTS:
										case CMISS_GRAPHIC_POINT:
										{
											graphics_object_type = g_GLYPH_SET;
										} break;
										case CMISS_GRAPHIC_STREAMLINES:
										{
											if (STREAM_LINE == graphic->streamline_type)
											{
												graphics_object_type = g_POLYLINE;
											}
											else
											{
												graphics_object_type = g_SURFACE;
											}
										} break;
										default:
										{
											display_message(ERROR_MESSAGE,
												"Cmiss_graphic_to_graphics_object.  "
												"Unknown graphic type");
											return_code = 0;
										} break;
									}
									if (return_code)
									{
										graphic->graphics_object = CREATE(GT_object)(
											graphics_object_name, graphics_object_type,
											graphic->material);
										GT_object_set_select_mode(graphic->graphics_object,
											graphic->select_mode);
										if (graphic->secondary_material)
										{
											set_GT_object_secondary_material(graphic->graphics_object,
												graphic->secondary_material);
										}
										if (graphic->selected_material)
										{
											set_GT_object_selected_material(graphic->graphics_object,
												graphic->selected_material);
										}
										ACCESS(GT_object)(graphic->graphics_object);
									}
								}
								DEALLOCATE(graphics_object_name);
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"Cmiss_graphic_to_graphics_object.  "
									"Unable to make graphics object name");
								return_code = 0;
							}
							if (graphic->data_field)
							{
								graphic_to_object_data->number_of_data_values =
									Computed_field_get_number_of_components(graphic->data_field);
								ALLOCATE(graphic_to_object_data->data_copy_buffer,
									FE_value, graphic_to_object_data->number_of_data_values);
							}
							if (graphic->graphics_object)
							{
								graphic->selected_graphics_changed=1;
								/* need graphic for FE_element_to_graphics_object routine */
								graphic_to_object_data->graphic=graphic;
								Cmiss_graphic_get_iteration_domain(graphic, graphic_to_object_data);
								switch (graphic->graphic_type)
								{
									case CMISS_GRAPHIC_NODE_POINTS:
									case CMISS_GRAPHIC_DATA_POINTS:
									{
										/* currently all nodes put together in a single GT_glyph_set,
										so rebuild all even if editing a single node or element */
										GT_object_remove_primitives_at_time(
											graphic->graphics_object, time,
											(GT_object_primitive_object_name_conditional_function *)NULL,
											(void *)NULL);
										base_size[0] = (FE_value)(graphic->glyph_size[0]);
										base_size[1] = (FE_value)(graphic->glyph_size[1]);
										base_size[2] = (FE_value)(graphic->glyph_size[2]);
										offset[0] = (FE_value)(graphic->glyph_offset[0]);
										offset[1] = (FE_value)(graphic->glyph_offset[1]);
										offset[2] = (FE_value)(graphic->glyph_offset[2]);
										scale_factors[0] = (FE_value)(graphic->glyph_scale_factors[0]);
										scale_factors[1] = (FE_value)(graphic->glyph_scale_factors[1]);
										scale_factors[2] = (FE_value)(graphic->glyph_scale_factors[2]);
										Cmiss_nodeset_id master_nodeset = Cmiss_field_module_find_nodeset_by_name(graphic_to_object_data->field_module,
											(graphic->graphic_type == CMISS_GRAPHIC_NODE_POINTS) ? "cmiss_nodes" : "cmiss_data");
										Cmiss_nodeset_id iteration_nodeset = 0;
										if (graphic->subgroup_field)
										{
											Cmiss_field_group_id group = Cmiss_field_cast_group(graphic->subgroup_field);
											if (group)
											{
												Cmiss_field_node_group_id node_group = Cmiss_field_group_get_node_group(group, master_nodeset);
												if (node_group)
												{
													iteration_nodeset =
														Cmiss_nodeset_group_base_cast(Cmiss_field_node_group_get_nodeset(node_group));
													Cmiss_field_node_group_destroy(&node_group);
												}
												Cmiss_field_group_destroy(&group);
											}
											else
											{
												Cmiss_field_node_group_id node_group = Cmiss_field_cast_node_group(graphic->subgroup_field);
												if (node_group)
												{
													// check group is for same master nodeset
													iteration_nodeset = Cmiss_nodeset_group_base_cast(Cmiss_field_node_group_get_nodeset(node_group));
													Cmiss_nodeset_id temp_master_nodeset = Cmiss_nodeset_get_master(iteration_nodeset);
													if (!Cmiss_nodeset_match(master_nodeset, temp_master_nodeset))
													{
														Cmiss_nodeset_destroy(&iteration_nodeset);
													}
													Cmiss_nodeset_destroy(&temp_master_nodeset);
													Cmiss_field_node_group_destroy(&node_group);
												}
												else
												{
													iteration_nodeset = Cmiss_nodeset_access(master_nodeset);
												}
											}
										}
										else
										{
											iteration_nodeset = Cmiss_nodeset_access(master_nodeset);
										}
										if (iteration_nodeset)
										{
											GT_glyph_set *glyph_set = create_GT_glyph_set_from_nodeset(
												iteration_nodeset, graphic_to_object_data->field_cache,
												graphic_to_object_data->rc_coordinate_field,
												graphic->glyph, base_size, offset, scale_factors,
												graphic_to_object_data->time,
												graphic_to_object_data->wrapper_orientation_scale_field,
												graphic->variable_scale_field, graphic->data_field,
												graphic->font, graphic->label_field,
												graphic->label_density_field,
												(iteration_nodeset == master_nodeset) ? graphic->subgroup_field : 0,
												graphic->select_mode,graphic_to_object_data->selection_group_field);
											/* NOT an error if no glyph_set produced == empty group */
											if (glyph_set)
											{
												if (!GT_OBJECT_ADD(GT_glyph_set)(graphic->graphics_object,
													time,glyph_set))
												{
													DESTROY(GT_glyph_set)(&glyph_set);
													return_code=0;
												}
											}
											Cmiss_nodeset_destroy(&iteration_nodeset);
										}
										Cmiss_nodeset_destroy(&master_nodeset);
									} break;
									case CMISS_GRAPHIC_POINT:
									{
										Cmiss_graphic_to_point_object_at_time(
											graphic, graphic_to_object_data->field_cache,
											graphic_to_object_data->time, /*graphics_object_primitive_time*/time);
									} break;
									case CMISS_GRAPHIC_LINES:
									{
#if defined(USE_OPENCASCADE)
										// test here for domain of rc_coordinate_field
										// if it is a cad_geometry do something about it
										struct LIST(Computed_field) *domain_field_list = CREATE_LIST(Computed_field)();
										int return_code = Computed_field_get_domain( graphic_to_object_data->rc_coordinate_field, domain_field_list );
										if ( return_code )
										{
											// so test for topology domain
											struct Computed_field *cad_topology_field = FIRST_OBJECT_IN_LIST_THAT(Computed_field)
												( Cmiss_field_is_type_cad_topology, (void *)NULL, domain_field_list );
											if ( cad_topology_field )
											{
												// if topology domain then draw item at location
												return_code = Cad_shape_to_graphics_object( cad_topology_field, graphic_to_object_data );
												DESTROY_LIST(Computed_field)(&domain_field_list);
												break;
											}
										}
										if ( domain_field_list )
											DESTROY_LIST(Computed_field)(&domain_field_list);
#endif /* defined(USE_OPENCASCADE) */
										GT_polyline_vertex_buffers *lines =
											CREATE(GT_polyline_vertex_buffers)(
												g_PLAIN, graphic->line_width);
										if (GT_OBJECT_ADD(GT_polyline_vertex_buffers)(
											graphic->graphics_object, lines))
										{
											if (graphic_to_object_data->iteration_mesh)
											{
												return_code = Cmiss_mesh_to_graphics(graphic_to_object_data->iteration_mesh, graphic_to_object_data);
											}
										}
										else
										{
											//DESTROY(GT_polyline_vertex_buffers)(&lines);
											return_code = 0;
										}
									} break;
									case CMISS_GRAPHIC_SURFACES:
									{
										bool cad_surfaces = false;
#if defined(USE_OPENCASCADE)
										{
											// test here for domain of rc_coordinate_field
											// if it is a cad_geometry do something about it
											//if ( is_cad_geometry( settings_to_object_data->rc_coordinate_field->get_domain() ) )
											struct LIST(Computed_field) *domain_field_list = CREATE_LIST(Computed_field)();
											int return_code = Computed_field_get_domain( graphic_to_object_data->rc_coordinate_field, domain_field_list );
											if ( return_code )
											{
												//printf( "got domain of rc_coordinate_field (%d)\n", NUMBER_IN_LIST(Computed_field)(domain_field_list) );
												// so test for topology domain
												struct Computed_field *cad_topology_field = FIRST_OBJECT_IN_LIST_THAT(Computed_field)
													( Cmiss_field_is_type_cad_topology, (void *)NULL, domain_field_list );
												if ( cad_topology_field )
												{
													cad_surfaces = true;
													//printf( "hurrah, we have a cad topology domain.\n" );
													// if topology domain then draw item at location
													return_code = Cad_shape_to_graphics_object( cad_topology_field, graphic_to_object_data );
													DESTROY_LIST(Computed_field)(&domain_field_list);
													break;
												}
											}
											if ( domain_field_list )
												DESTROY_LIST(Computed_field)(&domain_field_list);
										}
#endif /* defined(USE_OPENCASCADE) */
										if (!cad_surfaces)
										{
											if (graphic_to_object_data->iteration_mesh)
											{
												return_code = Cmiss_mesh_to_graphics(graphic_to_object_data->iteration_mesh, graphic_to_object_data);
											}
										}
									} break;
									case CMISS_GRAPHIC_CYLINDERS:
									case CMISS_GRAPHIC_ELEMENT_POINTS:
									{
										if (graphic_to_object_data->iteration_mesh)
										{
											return_code = Cmiss_mesh_to_graphics(graphic_to_object_data->iteration_mesh, graphic_to_object_data);
										}
									} break;
									case CMISS_GRAPHIC_ISO_SURFACES:
									{
										Cmiss_field_cache_set_time(graphic_to_object_data->field_cache, graphic_to_object_data->time);
										if (0 < graphic->number_of_iso_values)
										{
											if (g_SURFACE == GT_object_get_type(graphic->graphics_object))
											{
												graphic_to_object_data->iso_surface_specification =
													Iso_surface_specification_create(
														graphic->number_of_iso_values, graphic->iso_values,
														graphic->first_iso_value, graphic->last_iso_value,
														graphic_to_object_data->rc_coordinate_field,
														graphic->data_field,
														graphic->iso_scalar_field,
														graphic->texture_coordinate_field);
											}
											if (graphic_to_object_data->iteration_mesh)
											{
												return_code = Cmiss_mesh_to_graphics(graphic_to_object_data->iteration_mesh, graphic_to_object_data);
											}
											if (g_SURFACE == GT_object_get_type(graphic->graphics_object))
											{
												Iso_surface_specification_destroy(&graphic_to_object_data->iso_surface_specification);
												/* Decimate */
												if (graphic->decimation_threshold > 0.0)
												{
													GT_object_decimate_GT_surface(graphic->graphics_object,
														graphic->decimation_threshold);
												}
											}
											/* If the isosurface is a volume we can decimate and
											then normalise, otherwise if it is a polyline
											representing a isolines, skip over. */
											if (g_VOLTEX == GT_object_get_type(graphic->graphics_object))
											{
												/* Decimate */
												if (graphic->decimation_threshold > 0.0)
												{
													GT_object_decimate_GT_voltex(graphic->graphics_object,
														graphic->decimation_threshold);
												}
												/* Normalise normals now that the entire mesh has been calculated */
												GT_object_normalise_GT_voltex_normals(graphic->graphics_object);
											}
										}
									} break;
									case CMISS_GRAPHIC_STREAMLINES:
									{
										Cmiss_field_cache_set_time(graphic_to_object_data->field_cache, graphic_to_object_data->time);
										/* must always regenerate ALL streamlines since they can cross
										into other elements */
										if (graphic_to_object_data->existing_graphics)
										{
											DESTROY(GT_object)(
												&(graphic_to_object_data->existing_graphics));
										}
										if (graphic->seed_element)
										{
											return_code = FE_element_to_graphics_object(
												graphic->seed_element, graphic_to_object_data);
										}
										else if (graphic->seed_nodeset &&
											graphic->seed_node_mesh_location_field)
										{
											Cmiss_node_iterator_id iterator = Cmiss_nodeset_create_node_iterator(graphic->seed_nodeset);
											Cmiss_node_id node = 0;
											while (0 != (node = Cmiss_node_iterator_next_non_access(iterator)))
											{
												if (!Cmiss_node_to_streamline(node, graphic_to_object_data))
												{
													return_code = 0;
													break;
												}
											}
											Cmiss_node_iterator_destroy(&iterator);
										}
										else
										{
											if (graphic_to_object_data->iteration_mesh)
											{
												return_code = Cmiss_mesh_to_graphics(graphic_to_object_data->iteration_mesh, graphic_to_object_data);
											}
										}
									} break;
									default:
									{
										return_code = 0;
									} break;
								} /* end of switch */
								Cmiss_mesh_destroy(&graphic_to_object_data->iteration_mesh);
								Cmiss_mesh_destroy(&graphic_to_object_data->master_mesh);
								if (return_code)
								{
									/* set the spectrum in the graphics object - if required */
									if ((graphic->data_field)||
										((CMISS_GRAPHIC_STREAMLINES == graphic->graphic_type) &&
											(STREAM_NO_DATA != graphic->streamline_data_type)))
									{
										set_GT_object_Spectrum(graphic->graphics_object,
											(void *)(graphic->spectrum));
									}
									/* mark display list as needing updating */
									graphic->graphics_changed = 0;
									GT_object_changed(graphic->graphics_object);
								}
								else
								{
									graphic_string = Cmiss_graphic_string(graphic,
										GRAPHIC_STRING_COMPLETE_PLUS);
									display_message(ERROR_MESSAGE,
										"cmiss_graphic_to_graphics_object.  "
										"Could not build '%s'",graphic_string);
									DEALLOCATE(graphic_string);
									/* set return_code to 1, so rest of graphic can be built */
									return_code = 1;
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"cmiss_graphic_to_graphics_object.  "
									"Could not create graphics object");
								return_code = 0;
							}
							if (graphic_to_object_data->existing_graphics)
							{
								DESTROY(GT_object)(&(graphic_to_object_data->existing_graphics));
							}
							if (graphic->data_field)
							{
								graphic_to_object_data->number_of_data_values = 0;
								DEALLOCATE(graphic_to_object_data->data_copy_buffer);
							}
						}
						if (graphic->stream_vector_field)
						{
							Computed_field_end_wrap(&(graphic_to_object_data->wrapper_stream_vector_field));
						}
						if (graphic->orientation_scale_field)
						{
							Computed_field_end_wrap(&(graphic_to_object_data->wrapper_orientation_scale_field));
						}
						if (graphic_to_object_data->rc_coordinate_field)
						{
							Computed_field_end_wrap(&(graphic_to_object_data->rc_coordinate_field));
						}
					}
				}
				if (graphic->selected_graphics_changed)
				{
					if (graphic->graphics_object)
						GT_object_changed(graphic->graphics_object);
					graphic->selected_graphics_changed = 0;
				}
			}
			Cmiss_graphics_filter_destroy(&filter);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmiss_graphic_to_graphics_object.  Invalid argument(s)");
		return_code = 0;
	}

	LEAVE;

	return (return_code);
}

int Cmiss_graphic_compile_visible_graphic(
	struct Cmiss_graphic *graphic, void *renderer_void)
{
	int return_code = 1;
	Render_graphics *renderer;

	ENTER(Cmiss_graphic_compile_visible_graphic);
	if (graphic && (renderer = static_cast<Render_graphics *>(renderer_void)))
	{
		return_code = 1;
		if (graphic->graphics_object)
		{
			Cmiss_graphics_filter_id filter = Cmiss_scene_get_filter(renderer->get_Scene());
			if (filter)
			{
				if (Cmiss_graphics_filter_evaluate_graphic(filter, graphic))
				{
					Cmiss_graphic_set_renderer_highlight_functor(graphic, renderer);
					return_code = renderer->Graphics_object_compile(graphic->graphics_object);
					Cmiss_graphic_remove_renderer_highlight_functor(graphic, renderer);
				}
				Cmiss_graphics_filter_destroy(&filter);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_compile_visible_graphic.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_compile_visible_graphic */

int Cmiss_graphic_glyph_change(
	struct GT_object *glyph,void *graphic_void)
{
	int return_code;
	Cmiss_graphic *graphic =NULL;

	ENTER(Cmiss_graphic_glyph_change);
	graphic = (Cmiss_graphic *)graphic_void;
	if (glyph && graphic)
	{
		graphic->graphics_changed = 1;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_glyph_change.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_glyph_change */

int Cmiss_graphic_execute_visible_graphic(
	struct Cmiss_graphic *graphic, void *renderer_void)
{
	int return_code = 1;
	Render_graphics *renderer;

	ENTER(Cmiss_graphic_execute_visible_graphic);
	if (graphic && (renderer = static_cast<Render_graphics *>
			(renderer_void)))
	{
		return_code = 1;
		if (graphic->graphics_object)
		{
			Cmiss_graphics_filter_id filter = Cmiss_scene_get_filter(renderer->get_Scene());
			if (filter)
			{
				if (Cmiss_graphics_filter_evaluate_graphic(filter, graphic))
				{
					if (renderer->rendering_layer(graphic->overlay_flag))
					{
						if (renderer->begin_coordinate_system(graphic->coordinate_system))
						{
#if defined (OPENGL_API)
							/* use position in list as name for GL picking */
							glLoadName((GLuint)graphic->position);
#endif /* defined (OPENGL_API) */
							return_code = renderer->Graphics_object_execute(graphic->graphics_object);
							renderer->end_coordinate_system(graphic->coordinate_system);
						}
					}
				}
				Cmiss_graphics_filter_destroy(&filter);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_execute_visible_graphic.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_execute_visible_graphic */

static int Cmiss_graphic_Computed_field_or_ancestor_satisfies_condition(
	struct Cmiss_graphic *graphic,
	LIST_CONDITIONAL_FUNCTION(Computed_field) *conditional_function,
	void *user_data)
{
	int return_code;

	ENTER(Cmiss_graphic_Computed_field_or_ancestor_satisfies_condition);
	if (graphic && conditional_function)
	{
		return_code = 0;
		/* compare geometry graphic */
		/* for all graphic types */
		if ((graphic->coordinate_field &&
			Computed_field_or_ancestor_satisfies_condition(
				graphic->coordinate_field, conditional_function, user_data)) ||
			(graphic->subgroup_field &&
				(Computed_field_or_ancestor_satisfies_condition(
					graphic->subgroup_field, conditional_function, user_data))))
		{
			return_code = 1;
		}
		/* currently for surfaces only */
		else if (graphic->texture_coordinate_field &&
			Computed_field_or_ancestor_satisfies_condition(
				graphic->texture_coordinate_field, conditional_function, user_data))
		{
			return_code = 1;
		}
		/* for cylinders only */
		else if ((CMISS_GRAPHIC_CYLINDERS == graphic->graphic_type) &&
			graphic->radius_scalar_field &&
			Computed_field_or_ancestor_satisfies_condition(
				graphic->radius_scalar_field, conditional_function, user_data))
		{
			return_code = 1;
		}
		/* for iso_surfaces only */
		else if ((CMISS_GRAPHIC_ISO_SURFACES == graphic->graphic_type) &&
			(graphic->iso_scalar_field &&
			Computed_field_or_ancestor_satisfies_condition(
				graphic->iso_scalar_field, conditional_function, user_data)))
		{
			return_code = 1;
		}
		/* for node_points, data_points and element_points only */
		else if (((CMISS_GRAPHIC_NODE_POINTS == graphic->graphic_type) ||
			(CMISS_GRAPHIC_DATA_POINTS == graphic->graphic_type) ||
			(CMISS_GRAPHIC_ELEMENT_POINTS == graphic->graphic_type)) &&
			((graphic->orientation_scale_field &&
				(Computed_field_or_ancestor_satisfies_condition(
					graphic->orientation_scale_field, conditional_function, user_data)))||
			(graphic->variable_scale_field &&
				(Computed_field_or_ancestor_satisfies_condition(
					graphic->variable_scale_field, conditional_function, user_data))) ||
			(graphic->label_field &&
				(Computed_field_or_ancestor_satisfies_condition(
					graphic->label_field, conditional_function, user_data))) ||
			(graphic->label_density_field &&
				(Computed_field_or_ancestor_satisfies_condition(
					graphic->label_density_field, conditional_function, user_data)))))
		{
			return_code = 1;
		}
		/* for element_points with a density field only */
		else if (((CMISS_GRAPHIC_ELEMENT_POINTS == graphic->graphic_type) ||
			(CMISS_GRAPHIC_STREAMLINES == graphic->graphic_type))
			&& ((XI_DISCRETIZATION_CELL_DENSITY ==
				graphic->xi_discretization_mode) ||
				(XI_DISCRETIZATION_CELL_POISSON ==
					graphic->xi_discretization_mode)) &&
			Computed_field_or_ancestor_satisfies_condition(
				graphic->xi_point_density_field, conditional_function, user_data))
		{
			return_code = 1;
		}
		/* for streamlines only */
		else if ((CMISS_GRAPHIC_STREAMLINES == graphic->graphic_type) &&
			graphic->stream_vector_field &&
			Computed_field_or_ancestor_satisfies_condition(
				graphic->stream_vector_field, conditional_function, user_data))
		{
			return_code = 1;
		}
		else if ((CMISS_GRAPHIC_POINT == graphic->graphic_type) &&
			graphic->label_field &&
			Computed_field_or_ancestor_satisfies_condition(
				graphic->label_field, conditional_function, user_data))
		{
			return_code = 1;
		}
		/* appearance graphic for all graphic types */
		else if (graphic->data_field &&
			Computed_field_or_ancestor_satisfies_condition(
				graphic->data_field, conditional_function, user_data))
		{
			return_code = 1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_Computed_field_or_ancestor_satisfies_condition.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_Computed_field_or_ancestor_satisfies_condition */

static int Cmiss_graphic_uses_changed_FE_field(
	struct Cmiss_graphic *graphic,
	struct CHANGE_LOG(FE_field) *fe_field_change_log)
{
	int fe_field_change;
	int return_code;

	ENTER(Cmiss_graphic_uses_changed_FE_field);
	if (graphic && ((CMISS_GRAPHIC_POINT==graphic->graphic_type) ||
			graphic->coordinate_field) && fe_field_change_log)
	{
		if (((CMISS_GRAPHIC_ELEMENT_POINTS == graphic->graphic_type) ||
			(CMISS_GRAPHIC_STREAMLINES == graphic->graphic_type))&&
			graphic->native_discretization_field &&
			CHANGE_LOG_QUERY(FE_field)(fe_field_change_log,
				graphic->native_discretization_field, &fe_field_change) &&
			(CHANGE_LOG_OBJECT_UNCHANGED(FE_field) != fe_field_change))
		{
			return_code = 1;
		}
		else if (CMISS_GRAPHIC_POINT!=graphic->graphic_type)
		{
			return_code =
				Cmiss_graphic_Computed_field_or_ancestor_satisfies_condition(
					graphic, Computed_field_contains_changed_FE_field,
					(void *)fe_field_change_log);
		}
		else
		{
			return_code = 1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_uses_changed_FE_field.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_uses_changed_FE_field */

int Cmiss_graphic_Computed_field_change(
	struct Cmiss_graphic *graphic, void *change_data_void)
{
	int return_code = 1;
	struct Cmiss_graphic_Computed_field_change_data *change_data;

	ENTER(Cmiss_graphic_Computed_field_change);
	if (graphic && (change_data =
		(struct Cmiss_graphic_Computed_field_change_data *)change_data_void))
	{
		if (change_data->changed_field_list && Cmiss_graphic_Computed_field_or_ancestor_satisfies_condition(
			graphic, Computed_field_is_in_list, (void *)change_data->changed_field_list))
		{
			Cmiss_graphic_changed(graphic, CMISS_GRAPHIC_CHANGE_FULL_REBUILD);
		}
		if (change_data->selection_changed && graphic->graphics_object &&
			(CMISS_GRAPHIC_POINT != graphic->graphic_type) &&
			(CMISS_GRAPHIC_STREAMLINES != graphic->graphic_type))
		{
			Cmiss_graphic_update_selected(graphic, (void *)NULL);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_Computed_field_change.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_Computed_field_change */

int Cmiss_graphic_get_visible_graphics_object_range(
	struct Cmiss_graphic *graphic,void *graphic_range_void)
{
	int return_code = 1;
	struct Cmiss_graphic_range *graphic_range =
		(struct Cmiss_graphic_range *)graphic_range_void;

	ENTER(Cmiss_graphic_get_visible_graphics_object_range);

	if (graphic && graphic_range && graphic_range->graphics_object_range)
	{
		if (graphic->graphics_object &&
			(graphic->coordinate_system == graphic_range->coordinate_system))
		{
			Cmiss_graphics_filter_id filter = Cmiss_scene_get_filter(
				graphic_range->graphics_object_range->scene);
			if (filter)
			{
				if (Cmiss_graphics_filter_evaluate_graphic(filter, graphic))
				{
					return_code=get_graphics_object_range(graphic->graphics_object,
						(void *)graphic_range->graphics_object_range);
				}
				Cmiss_graphics_filter_destroy(&filter);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_get_visible_graphics_object_range.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_get_visible_graphics_object_range */

struct GT_object *Cmiss_graphic_get_graphics_object(
	struct Cmiss_graphic *graphic)
{
	struct GT_object *graphics_object

	ENTER(Cmiss_graphic_get_graphics_object);
	if (graphic)
	{
		graphics_object=graphic->graphics_object;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_get_graphics_object.  Invalid argument(s)");
		graphics_object=(struct GT_object *)NULL;
	}
	LEAVE;

	return (graphics_object);
} /* Cmiss_graphic_get_graphics_object */

int Cmiss_graphic_type_uses_dimension(
	enum Cmiss_graphic_type graphic_type, int dimension)
{
	int return_code;

	ENTER(Cmiss_graphic_type_uses_dimension);
	switch (graphic_type)
	{
		case CMISS_GRAPHIC_NODE_POINTS:
		case CMISS_GRAPHIC_DATA_POINTS:
		case CMISS_GRAPHIC_POINT:
		{
			return_code = ((-1 == dimension) || (0 == dimension));
		} break;
		case CMISS_GRAPHIC_LINES:
		case CMISS_GRAPHIC_CYLINDERS:
		{
			return_code = ((-1 == dimension) || (1 == dimension));
		} break;
		case CMISS_GRAPHIC_SURFACES:
		{
			return_code = ((-1 == dimension) || (2 == dimension));
		} break;
		case CMISS_GRAPHIC_STREAMLINES:
		{
			return_code = ((-1 == dimension) || (2 == dimension) ||
				(3 == dimension));
		} break;
		case CMISS_GRAPHIC_ELEMENT_POINTS:
		case CMISS_GRAPHIC_ISO_SURFACES:
		{
			return_code = ((-1 == dimension) ||
				(1 == dimension) || (2 == dimension) || (3 == dimension));
		} break;
		default:
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_graphic_type_uses_dimension.  Unknown graphic type");
			return_code = 0;
		} break;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_type_uses_dimension */

enum Graphics_select_mode Cmiss_graphic_get_select_mode(
	struct Cmiss_graphic *graphic)
{
	enum Graphics_select_mode select_mode;

	ENTER(Cmiss_graphic_get_select_mode);
	if (graphic)
	{
		select_mode = graphic->select_mode;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_get_select_mode.  Invalid argument(s)");
		select_mode = GRAPHICS_NO_SELECT;
	}
	LEAVE;

	return (select_mode);
} /* Cmiss_graphic_get_select_mode */


int Cmiss_graphic_set_select_mode(struct Cmiss_graphic *graphic,
	enum Graphics_select_mode select_mode)
{
	int return_code;

	ENTER(Cmiss_graphic_set_select_mode);
	if (graphic)
	{
		graphic->select_mode = select_mode;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_set_select_mode.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_set_select_mode */

struct Graphical_material *Cmiss_graphic_get_material(
	struct Cmiss_graphic *graphic)
{
	struct Graphical_material *material;

	ENTER(Cmiss_graphic_get_material);
	if (graphic)
	{
		material=graphic->material;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_get_material.  Invalid argument(s)");
		material=(struct Graphical_material *)NULL;
	}
	LEAVE;

	return (material);
} /* Cmiss_graphic_get_material */

struct Graphical_material *Cmiss_graphic_get_selected_material(
	struct Cmiss_graphic *graphic)
{
	struct Graphical_material *selected_material;

	ENTER(Cmiss_graphic_get_selected_material);
	if (graphic)
	{
		selected_material=graphic->selected_material;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_get_selected_material.  Invalid argument(s)");
		selected_material=(struct Graphical_material *)NULL;
	}
	LEAVE;

	return (selected_material);
} /* Cmiss_graphic_get_selected_material */


int Cmiss_graphic_set_data_spectrum_parameters(
	struct Cmiss_graphic *graphic,struct Computed_field *data_field,
	struct Spectrum *spectrum)
{
	int return_code;

	ENTER(Cmiss_graphic_set_data_spectrum_parameters);
	if (graphic&&((!data_field)||spectrum)&&
		(CMISS_GRAPHIC_STREAMLINES != graphic->graphic_type))
	{
		return_code=1;
		REACCESS(Computed_field)(&(graphic->data_field),data_field);
		if (!data_field)
		{
			/* don't want graphic accessing spectrum when not using it: */
			spectrum=(struct Spectrum *)NULL;
		}
		REACCESS(Spectrum)(&(graphic->spectrum),spectrum);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_set_data_spectrum_parameters.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_set_data_spectrum_parameters */

int Cmiss_graphic_set_data_spectrum_parameters_streamlines(
	struct Cmiss_graphic *graphic,
	enum Streamline_data_type streamline_data_type,
	struct Computed_field *data_field,struct Spectrum *spectrum)
{
	int return_code;

	ENTER(Cmiss_graphic_set_data_spectrum_parameters_streamlines);
	if (graphic&&((STREAM_FIELD_SCALAR!=streamline_data_type)||data_field)&&
		((STREAM_NO_DATA==streamline_data_type)||spectrum)&&
		(CMISS_GRAPHIC_STREAMLINES==graphic->graphic_type))
	{
		return_code=1;
		graphic->streamline_data_type=streamline_data_type;
		if (STREAM_FIELD_SCALAR!=streamline_data_type)
		{
			/* don't want graphic accessing data_field when not using it: */
			data_field=(struct Computed_field *)NULL;
		}
		REACCESS(Computed_field)(&(graphic->data_field),data_field);
		if (STREAM_NO_DATA==streamline_data_type)
		{
			/* don't want graphic accessing spectrum when not using it: */
			spectrum=(struct Spectrum *)NULL;
		}
		REACCESS(Spectrum)(&(graphic->spectrum),spectrum);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_set_data_spectrum_parameters_streamlines.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_set_data_spectrum_parameters_streamlines */

int Cmiss_graphic_set_render_type(
	struct Cmiss_graphic *graphic, enum Cmiss_graphics_render_type render_type)
{
	int return_code;

	ENTER(Cmiss_graphic_set_render_type);
	if (graphic)
	{
		return_code = 1;
		if (graphic->render_type != render_type)
		{
			graphic->render_type = render_type;
			Cmiss_graphic_changed(graphic, CMISS_GRAPHIC_CHANGE_FULL_REBUILD);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_set_render_type.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);

} /* Cmiss_graphic_set_render_type */

int Cmiss_graphic_set_glyph_parameters(
	struct Cmiss_graphic *graphic,
	struct GT_object *glyph, enum Graphic_glyph_scaling_mode glyph_scaling_mode,
	Triple glyph_offset, Triple glyph_size,
	struct Computed_field *orientation_scale_field, Triple glyph_scale_factors,
	struct Computed_field *variable_scale_field)
{
	int return_code;

	ENTER(Cmiss_graphic_set_glyph_parameters);
	if (graphic && ((glyph_offset && glyph_size &&
		((CMISS_GRAPHIC_NODE_POINTS==graphic->graphic_type)||
			(CMISS_GRAPHIC_DATA_POINTS==graphic->graphic_type)||
			(CMISS_GRAPHIC_ELEMENT_POINTS==graphic->graphic_type) ||
			(CMISS_GRAPHIC_POINT==graphic->graphic_type))&&
		((!orientation_scale_field) || Computed_field_is_orientation_scale_capable(
			orientation_scale_field,(void *)NULL)) && glyph_scale_factors &&
		((!variable_scale_field) || Computed_field_has_up_to_3_numerical_components(
			variable_scale_field,(void *)NULL))) || !glyph))
	{
		if (graphic->glyph)
		{
			GT_object_remove_callback(graphic->glyph,
				Cmiss_graphic_glyph_change, (void *)graphic);
		}
		REACCESS(GT_object)(&(graphic->glyph),glyph);
		if (glyph)
		{
			GT_object_add_callback(graphic->glyph, Cmiss_graphic_glyph_change,
					(void *)graphic);
		}
		graphic->glyph_scaling_mode = glyph_scaling_mode;
		graphic->glyph_offset[0] = glyph_offset[0];
		graphic->glyph_offset[1] = glyph_offset[1];
		graphic->glyph_offset[2] = glyph_offset[2];
		graphic->glyph_size[0] = glyph_size[0];
		graphic->glyph_size[1] = glyph_size[1];
		graphic->glyph_size[2] = glyph_size[2];
		REACCESS(Computed_field)(&(graphic->orientation_scale_field),
			orientation_scale_field);
		graphic->glyph_scale_factors[0]=glyph_scale_factors[0];
		graphic->glyph_scale_factors[1]=glyph_scale_factors[1];
		graphic->glyph_scale_factors[2]=glyph_scale_factors[2];
		REACCESS(Computed_field)(&(graphic->variable_scale_field),
			variable_scale_field);

		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_set_glyph_parameters.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_set_glyph_parameters */

int Cmiss_graphic_get_iso_surface_parameters(
	struct Cmiss_graphic *graphic,struct Computed_field **iso_scalar_field,
	int *number_of_iso_values, double **iso_values,
	double *first_iso_value, double *last_iso_value,
	double *decimation_threshold)
{
	int i, return_code;

	ENTER(Cmiss_graphic_get_iso_surface_parameters);
	if (graphic&&iso_scalar_field&&number_of_iso_values&&iso_values&&
		decimation_threshold&&
		(CMISS_GRAPHIC_ISO_SURFACES==graphic->graphic_type))
	{
		*iso_scalar_field=graphic->iso_scalar_field;
		*decimation_threshold=graphic->decimation_threshold;
		*number_of_iso_values = graphic->number_of_iso_values;
		*first_iso_value = graphic->first_iso_value;
		*last_iso_value = graphic->last_iso_value;
		if (0 < graphic->number_of_iso_values)
		{
			if (graphic->iso_values)
			{
				if (ALLOCATE(*iso_values, double, graphic->number_of_iso_values))
				{
					for (i = 0 ; i < graphic->number_of_iso_values ; i++)
					{
						(*iso_values)[i] = graphic->iso_values[i];
					}
					return_code = 1;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Cmiss_graphic_get_iso_surface_parameters.  "
						"Could not allocate memory.");
					return_code=0;
				}
			}
			else
			{
				*iso_values = (double *)NULL;
				return_code = 1;
			}
		}
		else
		{
			*number_of_iso_values=graphic->number_of_iso_values;
			*iso_values=(double *)NULL;
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_get_iso_surface_parameters.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_get_iso_surface_parameters */

int Cmiss_graphic_set_iso_surface_parameters(
	struct Cmiss_graphic *graphic,struct Computed_field *iso_scalar_field,
	int number_of_iso_values, double *iso_values,
	double first_iso_value, double last_iso_value,
	double decimation_threshold)
{
	int i, return_code;

	ENTER(Cmiss_graphic_set_iso_surface_parameters);
	if (graphic&& (!iso_scalar_field || (iso_scalar_field &&
		(1==Computed_field_get_number_of_components(iso_scalar_field))))&&
		(CMISS_GRAPHIC_ISO_SURFACES==graphic->graphic_type))
	{
		return_code=1;
		REACCESS(Computed_field)(&(graphic->iso_scalar_field),iso_scalar_field);
		graphic->decimation_threshold = decimation_threshold;
		if (0 < number_of_iso_values)
		{
			if (iso_values)
			{
				double *temp_values;
				if (REALLOCATE(temp_values, graphic->iso_values, double,
						number_of_iso_values))
				{
					graphic->iso_values = temp_values;
					graphic->number_of_iso_values = number_of_iso_values;
					for (i = 0 ; i < number_of_iso_values ; i++)
					{
						graphic->iso_values[i] = iso_values[i];
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"cmiss_graphic_set_iso_surface_parameters.  "
						"Could not allocate memory.");
					return_code=0;
				}
			}
			else
			{
				if (graphic->iso_values)
				{
					DEALLOCATE(graphic->iso_values);
				}
				graphic->number_of_iso_values = number_of_iso_values;
				graphic->first_iso_value = first_iso_value;
				graphic->last_iso_value = last_iso_value;
			}
		}
		else
		{
			if (graphic->iso_values)
			{
				DEALLOCATE(graphic->iso_values);
				graphic->iso_values = (double *)NULL;
			}
			graphic->number_of_iso_values = 0;
			graphic->first_iso_value = 0;
			graphic->last_iso_value = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_set_iso_surface_parameters.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_set_iso_surface_parameters */

int Cmiss_graphic_set_radius_parameters(
	struct Cmiss_graphic *graphic,GLfloat constant_radius,
	GLfloat radius_scale_factor,struct Computed_field *radius_scalar_field)
{
	int return_code;

	ENTER(Cmiss_graphic_set_radius_parameters);
	if (graphic&&(CMISS_GRAPHIC_CYLINDERS==graphic->graphic_type)&&
		((!radius_scalar_field)||
			(1==Computed_field_get_number_of_components(radius_scalar_field))))
	{
		return_code=1;
		graphic->constant_radius=constant_radius;
		graphic->radius_scale_factor=radius_scale_factor;
		REACCESS(Computed_field)(&(graphic->radius_scalar_field),
			radius_scalar_field);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_set_radius_parameters.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
}

int Cmiss_graphic_get_streamline_parameters(
	struct Cmiss_graphic *graphic,enum Streamline_type *streamline_type,
	struct Computed_field **stream_vector_field,int *reverse_track,
	GLfloat *streamline_length,GLfloat *streamline_width)
{
	int return_code;

	ENTER(Cmiss_graphic_get_streamline_parameters);
	if (graphic&&streamline_type&&stream_vector_field&&reverse_track&&
		streamline_length&&streamline_width&&
		(CMISS_GRAPHIC_STREAMLINES==graphic->graphic_type))
	{
		*streamline_type=graphic->streamline_type;
		*stream_vector_field=graphic->stream_vector_field;
		*reverse_track=graphic->reverse_track;
		*streamline_length=graphic->streamline_length;
		*streamline_width=graphic->streamline_width;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_get_streamline_parameters.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_get_streamline_parameters */

int Cmiss_graphic_set_streamline_parameters(
	struct Cmiss_graphic *graphic,enum Streamline_type streamline_type,
	struct Computed_field *stream_vector_field,int reverse_track,
	GLfloat streamline_length,GLfloat streamline_width)
{
	int return_code;

	ENTER(Cmiss_graphic_set_streamline_parameters);
	if (graphic && (CMISS_GRAPHIC_STREAMLINES==graphic->graphic_type))
	{
		graphic->streamline_type=streamline_type;
		REACCESS(Computed_field)(&(graphic->stream_vector_field),
			stream_vector_field);
		graphic->reverse_track=reverse_track;
		graphic->streamline_length=streamline_length;
		graphic->streamline_width=streamline_width;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_set_streamline_parameters.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_set_streamline_parameters */

int Cmiss_graphic_set_use_element_type(
	struct Cmiss_graphic *graphic,enum Use_element_type use_element_type)
{
	int return_code;

	ENTER(Cmiss_graphic_set_use_element_type);
	if (graphic&&((CMISS_GRAPHIC_ELEMENT_POINTS==graphic->graphic_type)||
		(CMISS_GRAPHIC_ISO_SURFACES==graphic->graphic_type)))
	{
		graphic->use_element_type=use_element_type;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_set_use_element_type.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_set_use_element_type */

int Cmiss_graphic_get_xi_discretization(
	struct Cmiss_graphic *graphic,
	enum Xi_discretization_mode *xi_discretization_mode,
	struct Computed_field **xi_point_density_field)
{
	int return_code;

	ENTER(Cmiss_graphic_get_xi_discretization);
	if (graphic &&
		((CMISS_GRAPHIC_ELEMENT_POINTS == graphic->graphic_type) ||
			(CMISS_GRAPHIC_STREAMLINES == graphic->graphic_type)))
	{
		if (xi_discretization_mode)
		{
			*xi_discretization_mode = graphic->xi_discretization_mode;
		}
		if (xi_point_density_field)
		{
			*xi_point_density_field = graphic->xi_point_density_field;
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_get_xi_discretization.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_get_xi_discretization */

int Cmiss_graphic_set_xi_discretization(
	struct Cmiss_graphic *graphic,
	enum Xi_discretization_mode xi_discretization_mode,
	struct Computed_field *xi_point_density_field)
{
	int uses_density_field, return_code;

	ENTER(Cmiss_graphic_set_xi_discretization);
	uses_density_field =
		(XI_DISCRETIZATION_CELL_DENSITY == xi_discretization_mode) ||
		(XI_DISCRETIZATION_CELL_POISSON == xi_discretization_mode);

	if (graphic && ((!uses_density_field && !xi_point_density_field) ||
		(uses_density_field && (!xi_point_density_field ||
			Computed_field_is_scalar(xi_point_density_field, (void *)NULL)))))
	{
		graphic->xi_discretization_mode = xi_discretization_mode;
		REACCESS(Computed_field)(&(graphic->xi_point_density_field),
			xi_point_density_field);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_set_xi_discretization.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_set_xi_discretization */

int Cmiss_graphic_set_discretization(
  struct Cmiss_graphic *graphic, struct Element_discretization *discretization)
{
	int return_code;

	ENTER(Cmiss_graphic_set_discretization);
	if (graphic && discretization && Cmiss_graphic_type_uses_attribute(
		graphic->graphic_type, CMISS_GRAPHIC_ATTRIBUTE_DISCRETIZATION))
	{
		if ((graphic->discretization.number_in_xi1 !=
				discretization->number_in_xi1) ||
			(graphic->discretization.number_in_xi2 !=
				discretization->number_in_xi2) ||
			(graphic->discretization.number_in_xi3 !=
				discretization->number_in_xi3))
		{
			graphic->discretization.number_in_xi1=discretization->number_in_xi1;
			graphic->discretization.number_in_xi2=discretization->number_in_xi2;
			graphic->discretization.number_in_xi3=discretization->number_in_xi3;
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_set_discretization.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_set_discretization */

int Cmiss_graphic_copy_without_graphics_object(
	struct Cmiss_graphic *destination, struct Cmiss_graphic *source)
{
	int return_code;

	ENTER(Cmiss_graphic_copy_without_graphics_object);
	if (destination && source && (destination != source))
	{
		return_code = 1;
		destination->position = source->position;

		if (destination->name)
		{
			DEALLOCATE(destination->name);
		}
		if (source->name && ALLOCATE(destination->name, char,
			strlen(source->name) + 1))
		{
			strcpy((char *)destination->name, source->name);
		}

		/* copy geometry graphic */
		/* for all graphic types */
		destination->graphic_type=source->graphic_type;
		destination->coordinate_system=source->coordinate_system;
		REACCESS(Computed_field)(&(destination->coordinate_field),
			source->coordinate_field);
		destination->select_mode=source->select_mode;
		/* for surfaces only at the moment */
		REACCESS(Computed_field)(&(destination->texture_coordinate_field),
			source->texture_coordinate_field);
		/* for 1-D and 2-D elements only */
		destination->exterior=source->exterior;
		destination->face=source->face;
		/* overlay_flag */
		destination->overlay_flag = source->overlay_flag;
		destination->overlay_order = source->overlay_order;
		/* for cylinders only */
		if (CMISS_GRAPHIC_CYLINDERS==source->graphic_type)
		{
			Cmiss_graphic_set_radius_parameters(destination,
				source->constant_radius,source->radius_scale_factor,
				source->radius_scalar_field);
		}
		else
		{
			if (destination->radius_scalar_field)
			{
				DEACCESS(Computed_field)(&destination->radius_scalar_field);
			}
		}
		/* for iso_surfaces only */
		if (CMISS_GRAPHIC_ISO_SURFACES==source->graphic_type)
		{
			Cmiss_graphic_set_iso_surface_parameters(destination,
				source->iso_scalar_field,source->number_of_iso_values,
				source->iso_values,
				source->first_iso_value, source->last_iso_value,
				source->decimation_threshold);
		}
		else
		{
			if (destination->iso_scalar_field)
			{
				DEACCESS(Computed_field)(&destination->iso_scalar_field);
			}
		}
		/* for node_points, data_points and element_points only */
		if ((CMISS_GRAPHIC_NODE_POINTS==source->graphic_type)||
			(CMISS_GRAPHIC_DATA_POINTS==source->graphic_type)||
			(CMISS_GRAPHIC_ELEMENT_POINTS==source->graphic_type)||
			(CMISS_GRAPHIC_POINT==source->graphic_type))
		{
			Cmiss_graphic_set_glyph_parameters(destination,
				source->glyph, source->glyph_scaling_mode,
				source->glyph_offset, source->glyph_size,
				source->orientation_scale_field, source->glyph_scale_factors,
				source->variable_scale_field);
		}
		else
		{
			if (destination->glyph)
			{
				GT_object_remove_callback(destination->glyph,
					Cmiss_graphic_glyph_change, (void *)destination);
				DEACCESS(GT_object)(&(destination->glyph));
			}
			if (destination->orientation_scale_field)
			{
				DEACCESS(Computed_field)(&destination->orientation_scale_field);
			}
			if (destination->variable_scale_field)
			{
				DEACCESS(Computed_field)(&destination->variable_scale_field);
			}
		}

		if (CMISS_GRAPHIC_POINT==source->graphic_type)
		{
			destination->overlay_flag = source->overlay_flag;
			destination->overlay_order = source->overlay_order;
		}

		REACCESS(Computed_field)(&(destination->label_field),source->label_field);
		REACCESS(Computed_field)(&(destination->subgroup_field),source->subgroup_field);
		/* for element_points and iso_surfaces */
		destination->use_element_type=source->use_element_type;
		/* for element_points only */
		destination->xi_discretization_mode=source->xi_discretization_mode;
		REACCESS(Computed_field)(&(destination->xi_point_density_field),
			source->xi_point_density_field);
		destination->discretization.number_in_xi1=
			source->discretization.number_in_xi1;
		destination->discretization.number_in_xi2=
			source->discretization.number_in_xi2;
		destination->discretization.number_in_xi3=
			source->discretization.number_in_xi3;
		destination->circle_discretization=source->circle_discretization;
		REACCESS(FE_field)(&(destination->native_discretization_field),
			source->native_discretization_field);
		REACCESS(Cmiss_tessellation)(&(destination->tessellation),
			source->tessellation);
		REACCESS(Computed_field)(&(destination->label_density_field),source->label_density_field);
		/* for volumes only */
		REACCESS(VT_volume_texture)(&(destination->volume_texture),
			source->volume_texture);
		REACCESS(Computed_field)(&(destination->displacement_map_field),
			source->displacement_map_field);
		/* for graphic starting in a particular element */
		REACCESS(FE_element)(&(destination->seed_element),
			source->seed_element);
		/* for graphic requiring an exact xi location */
		destination->seed_xi[0]=source->seed_xi[0];
		destination->seed_xi[1]=source->seed_xi[1];
		destination->seed_xi[2]=source->seed_xi[2];
		/* for streamlines only */
		destination->streamline_type=source->streamline_type;
		REACCESS(Computed_field)(&(destination->stream_vector_field),
			source->stream_vector_field);
		destination->reverse_track=source->reverse_track;
		destination->streamline_length=source->streamline_length;
		destination->streamline_width=source->streamline_width;
		if (destination->seed_nodeset)
		{
			Cmiss_nodeset_destroy(&destination->seed_nodeset);
		}
		if (source->seed_nodeset)
		{
			destination->seed_nodeset = Cmiss_nodeset_access(source->seed_nodeset);
		}
		REACCESS(Computed_field)(&(destination->seed_node_mesh_location_field),
			source->seed_node_mesh_location_field);

		/* copy appearance graphic */
		/* for all graphic types */
		destination->visibility_flag = source->visibility_flag;
		destination->line_width = source->line_width;
		REACCESS(Graphical_material)(&(destination->material),source->material);
		REACCESS(Graphical_material)(&(destination->secondary_material),
			source->secondary_material);
		Cmiss_graphic_set_render_type(destination,source->render_type);
		if (CMISS_GRAPHIC_STREAMLINES==source->graphic_type)
		{
			Cmiss_graphic_set_data_spectrum_parameters_streamlines(destination,
				source->streamline_data_type,source->data_field,source->spectrum);
		}
		else
		{
			Cmiss_graphic_set_data_spectrum_parameters(destination,
				source->data_field,source->spectrum);
		}
		REACCESS(Graphical_material)(&(destination->selected_material),
			source->selected_material);
		destination->autorange_spectrum_flag = source->autorange_spectrum_flag;
		REACCESS(Cmiss_graphics_font)(&(destination->font), source->font);

		/* ensure destination graphics object is cleared */
		REACCESS(GT_object)(&(destination->graphics_object),
			(struct GT_object *)NULL);
		destination->graphics_changed = 1;
		destination->selected_graphics_changed = 1;

		if (!return_code)
		{
			display_message(ERROR_MESSAGE,"Cmiss_graphic_copy_without_graphics_object.  "
				"Error copying graphic");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_graphic_copy_without_graphics_object.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* cmiss_graphic_copy_without_graphics_object */

int Cmiss_graphic_has_name(struct Cmiss_graphic *graphic,
	void *name_void)
{
	char *name, temp_name[30];
	int return_code;

	ENTER(Cmiss_graphic_has_name);
	if (graphic && (name=(char *)name_void))
	{
		return_code = 0;
		if (graphic->name)
		{
			return_code=!strcmp(name,graphic->name);
		}
		if (!return_code)
		{
			/* Compare with number if the graphic
			 has no name or the name didn't match */
			sprintf(temp_name, "%d", graphic->position);
			return_code=!strcmp(name,temp_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmiss_graphic_has_name.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_has_name */

static int FE_element_as_graphics_name_is_removed_or_modified(
	int graphics_name, void *data_void)
{
	int return_code;
	struct CM_element_information cm;
	struct FE_element *element;
	struct Cmiss_graphic_FE_region_change_data *data;

	ENTER(FE_element_as_graphics_name_is_removed_or_modified);
	return_code = 0;
	if (NULL != (data = (struct Cmiss_graphic_FE_region_change_data *)data_void))
	{
		cm.number = graphics_name;
		if (data->element_type == 1)
		{
			cm.type = CM_LINE;
		}
		else if (data->element_type == 2)
		{
			cm.type = CM_FACE;
		}
		else
		{
			cm.type = CM_ELEMENT;
		}
		if (NULL != (element = FE_region_get_FE_element_from_identifier_deprecated(data->fe_region,
					&cm)))
		{
			return_code = FE_element_or_parent_changed(element,
				data->fe_element_changes, data->fe_node_changes);
		}
		else
		{
			/* must have been removed or never in FE_region */
			return_code = 1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_as_graphics_name_is_removed_or_modified.  "
			"Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* FE_element_as_graphics_name_is_removed_or_modified */

int Cmiss_graphic_FE_region_change(
	struct Cmiss_graphic *graphic, void *data_void)
{
	int fe_field_related_object_change, return_code;
	struct Cmiss_graphic_FE_region_change_data *data;

	ENTER(Cmiss_graphic_FE_region_change);
	if (graphic &&
		(data = (struct Cmiss_graphic_FE_region_change_data *)data_void))
	{
		if (graphic->graphics_object)
		{
			switch (graphic->graphic_type)
			{
				case CMISS_GRAPHIC_DATA_POINTS:
				{
					/* handled by Cmiss_graphic_data_FE_region_change */
				} break;
				case CMISS_GRAPHIC_NODE_POINTS:
				{
					/* must always rebuild if identifiers changed */
					if ((data->fe_node_change_summary &
						CHANGE_LOG_OBJECT_IDENTIFIER_CHANGED(FE_node)) ||
						(Cmiss_graphic_uses_changed_FE_field(graphic,
							data->fe_field_changes) && (
								(data->fe_field_change_summary & (
									CHANGE_LOG_OBJECT_IDENTIFIER_CHANGED(FE_field) |
									CHANGE_LOG_OBJECT_NOT_IDENTIFIER_CHANGED(FE_field))) ||
								((data->fe_field_change_summary &
									CHANGE_LOG_RELATED_OBJECT_CHANGED(FE_field)) &&
									(0 < data->number_of_fe_node_changes)))))
					{
						/* currently node points are always rebuilt from scratch */
						Cmiss_graphic_changed(graphic, CMISS_GRAPHIC_CHANGE_FULL_REBUILD);
					}
				} break;
				default:
				{
					fe_field_related_object_change =
						CHANGE_LOG_OBJECT_UNCHANGED(FE_field);
					/* must always rebuild if identifiers changed */
					bool element_identifier_change = false;
					int number_of_element_changes_all_dimensions = 0;
					for (int dim = 0; dim < MAXIMUM_ELEMENT_XI_DIMENSIONS; dim++)
					{
						if (data->fe_element_change_summary[dim] &
							CHANGE_LOG_OBJECT_IDENTIFIER_CHANGED(FE_element))
						{
							element_identifier_change = true;
						}
						number_of_element_changes_all_dimensions +=
							data->number_of_fe_element_changes[dim];
					}
					if (element_identifier_change ||
						(Cmiss_graphic_uses_changed_FE_field(graphic,
							data->fe_field_changes) && (
								(data->fe_field_change_summary & (
									CHANGE_LOG_OBJECT_IDENTIFIER_CHANGED(FE_field) |
									CHANGE_LOG_OBJECT_NOT_IDENTIFIER_CHANGED(FE_field))) ||
								(fe_field_related_object_change = (
									(data->fe_field_change_summary &
										CHANGE_LOG_RELATED_OBJECT_CHANGED(FE_field)) && (
									(0 < data->number_of_fe_node_changes) ||
									(0 < number_of_element_changes_all_dimensions)))))))
					{
						if (fe_field_related_object_change && (
							((data->number_of_fe_node_changes*2) <
								FE_region_get_number_of_FE_nodes(data->fe_region)) &&
							((number_of_element_changes_all_dimensions*4) <
								FE_region_get_number_of_FE_elements_all_dimensions(data->fe_region))))
						{
							data->element_type = Cmiss_graphic_get_dimension(graphic, data->fe_region);
							/* partial rebuild for few node/element field changes */
							GT_object_remove_primitives_at_time(graphic->graphics_object,
								(GLfloat)data->time, FE_element_as_graphics_name_is_removed_or_modified,
								data_void);
							Cmiss_graphic_changed(graphic, CMISS_GRAPHIC_CHANGE_PARTIAL_REBUILD);
						}
						else
						{
							/* full rebuild for changed identifiers, FE_field definition
								 changes or many node/element field changes */
							Cmiss_graphic_changed(graphic, CMISS_GRAPHIC_CHANGE_FULL_REBUILD);
						}
					}
				} break;
			}
		}
		else
		{
			/* Graphics have definitely changed as they have not been built yet */
			Cmiss_graphic_changed(graphic, CMISS_GRAPHIC_CHANGE_REDRAW);
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_FE_region_change.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_FE_region_change */

int Cmiss_graphic_data_FE_region_change(
	struct Cmiss_graphic *graphic, void *data_void)
{
	int return_code;
	struct Cmiss_graphic_FE_region_change_data *data;

	ENTER(Cmiss_graphic_data_FE_region_change);
	if (graphic &&
		(data = (struct Cmiss_graphic_FE_region_change_data *)data_void))
	{
		if (graphic->graphics_object)
		{
			switch (graphic->graphic_type)
			{
				case CMISS_GRAPHIC_DATA_POINTS:
				case CMISS_GRAPHIC_ELEMENT_POINTS:
				case CMISS_GRAPHIC_NODE_POINTS:
				case CMISS_GRAPHIC_POINT:
				{
					// must ensure changes to fields on host elements/nodes force
					// data_points to be rebuilt if using embedded fields referencing them:
					if (((0 < data->number_of_fe_node_changes) ||
						(data->fe_field_change_summary & (
							CHANGE_LOG_OBJECT_NOT_IDENTIFIER_CHANGED(FE_field) |
							CHANGE_LOG_RELATED_OBJECT_CHANGED(FE_field)))) &&
						Cmiss_graphic_uses_changed_FE_field(graphic,
							data->fe_field_changes))
					{
						Cmiss_graphic_changed(graphic, CMISS_GRAPHIC_CHANGE_FULL_REBUILD);
					}
				} break;
				default:
				{
					/* do nothing */
				} break;
			}
		}
		else
		{
			/* Graphics have definitely changed as they have not been built yet */
			Cmiss_graphic_changed(graphic, CMISS_GRAPHIC_CHANGE_REDRAW);
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_data_FE_region_change.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_data_FE_region_change */

namespace {

/***************************************************************************//**
 * Makes a new graphic of the supplied graphic_type, optionally a copy of an
 * existing_graphic.
 *
 * @param rendition  Source of graphics defaults if creating a new graphic.
 * @param graphic_type  The type of the new graphic.
 * @param existing_graphic  An existing graphic to copy settings from if of
 * same graphic_type.
 * @return  1 on success, 0 on failure.
 */
Cmiss_graphic* get_graphic_for_gfx_modify(Cmiss_rendition *rendition,
	Cmiss_graphic_type graphic_type, Cmiss_graphic *existing_graphic)
{
	Cmiss_graphic *graphic = CREATE(Cmiss_graphic)(graphic_type);
	if (existing_graphic && (graphic_type == existing_graphic->graphic_type))
	{
		Cmiss_graphic_copy_without_graphics_object(graphic, existing_graphic);
		// GRC not sure why this was done:
		// Cmiss_graphic_set_rendition_private(graphic, existing_graphic->rendition);
	}
	else
	{
		Cmiss_rendition_set_graphic_defaults(rendition, graphic);
		/* Set up the coordinate_field */
		// GRC move following to Cmiss_rendition_set_graphic_defaults ?
		// GRC can improve as logic is probably already in rendition
		if (!graphic->coordinate_field)
		{
			struct Cmiss_region *region = Cmiss_rendition_get_region(rendition);
			struct FE_region *fe_region =	Cmiss_region_get_FE_region(region);
			struct FE_region *data_region = FE_region_get_data_FE_region(fe_region);
			struct FE_field *fe_field;
			if (FE_region_get_default_coordinate_FE_field(fe_region, &fe_field) ||
				FE_region_get_default_coordinate_FE_field(data_region, &fe_field))
			{
				struct Computed_field *coordinate_field = FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
					Computed_field_wraps_fe_field,
					(void *)fe_field, Cmiss_region_get_Computed_field_manager(region));
				if (coordinate_field)
				{
					Cmiss_graphic_set_coordinate_field(graphic, coordinate_field);
				}
			}
		}
	}
	return (graphic);
}

} // namespace anonymous

int Cmiss_graphic_same_geometry(struct Cmiss_graphic *graphic,
	void *second_graphic_void)
{
	int dimension,i,return_code;
	struct Cmiss_graphic *second_graphic;

	ENTER(Cmiss_graphic_same_geometry);
	if (graphic
		&&(second_graphic=(struct Cmiss_graphic *)second_graphic_void))
	{
		return_code=1;

		/* compare geometry graphic */
		/* for all graphic types */
		if (return_code)
		{
			/* note: different if names are different */
			return_code=
				(graphic->graphic_type==second_graphic->graphic_type)&&
				(graphic->coordinate_field==second_graphic->coordinate_field)&&
				(graphic->subgroup_field==second_graphic->subgroup_field)&&
				((((char *)NULL==graphic->name)&&((char *)NULL==second_graphic->name))
					||((graphic->name)&&(second_graphic->name)&&
						(0==strcmp(graphic->name,second_graphic->name))))&&
				(graphic->select_mode==second_graphic->select_mode);
		}
		/* for 1-D and 2-D elements only */
		if (return_code)
		{
			dimension=Cmiss_graphic_get_dimension(graphic, (struct FE_region *)NULL);
			if ((1==dimension)||(2==dimension))
			{
				return_code=(graphic->exterior == second_graphic->exterior)&&
					(graphic->face == second_graphic->face);
			}
		}
		/* for cylinders only */
		if (return_code&&(CMISS_GRAPHIC_CYLINDERS==graphic->graphic_type))
		{
			return_code=(graphic->constant_radius==second_graphic->constant_radius)
				&&(graphic->radius_scalar_field==second_graphic->radius_scalar_field)
				&&(graphic->radius_scale_factor==second_graphic->radius_scale_factor)
				&&(graphic->circle_discretization==second_graphic->circle_discretization);
		}
		/* for iso_surfaces only */
		if (return_code&&
			(CMISS_GRAPHIC_ISO_SURFACES==graphic->graphic_type))
		{
			return_code=(graphic->number_of_iso_values==
				second_graphic->number_of_iso_values)&&
				(graphic->decimation_threshold==second_graphic->decimation_threshold)&&
				(graphic->iso_scalar_field==second_graphic->iso_scalar_field);
			if (return_code)
			{
				if (graphic->iso_values)
				{
					if (second_graphic->iso_values)
					{
						i = 0;
						while (return_code && (i < graphic->number_of_iso_values))
						{
							if (graphic->iso_values[i] != second_graphic->iso_values[i])
							{
								return_code = 0;
							}
							i++;
						}
					}
					else
					{
						return_code = 0;
					}
				}
				else
				{
					if (second_graphic->iso_values)
					{
						return_code = 0;
					}
					else
					{
						return_code =
							(graphic->first_iso_value == second_graphic->first_iso_value)
							&& (graphic->last_iso_value == second_graphic->last_iso_value);
					}
				}
			}
		}
		/* for node_points, data_points and element_points only */
		if (return_code&&
			((CMISS_GRAPHIC_NODE_POINTS==graphic->graphic_type)||
				(CMISS_GRAPHIC_DATA_POINTS==graphic->graphic_type)||
				(CMISS_GRAPHIC_ELEMENT_POINTS==graphic->graphic_type)||
				(CMISS_GRAPHIC_POINT==graphic->graphic_type) ))
		{
			return_code=
				(graphic->glyph==second_graphic->glyph)&&
				(graphic->glyph_scaling_mode==second_graphic->glyph_scaling_mode)&&
				(graphic->glyph_size[0]==second_graphic->glyph_size[0])&&
				(graphic->glyph_size[1]==second_graphic->glyph_size[1])&&
				(graphic->glyph_size[2]==second_graphic->glyph_size[2])&&
				(graphic->glyph_scale_factors[0]==
					second_graphic->glyph_scale_factors[0])&&
				(graphic->glyph_scale_factors[1]==
					second_graphic->glyph_scale_factors[1])&&
				(graphic->glyph_scale_factors[2]==
					second_graphic->glyph_scale_factors[2])&&
				(graphic->glyph_offset[0]==second_graphic->glyph_offset[0])&&
				(graphic->glyph_offset[1]==second_graphic->glyph_offset[1])&&
				(graphic->glyph_offset[2]==second_graphic->glyph_offset[2])&&
				(graphic->orientation_scale_field==
					second_graphic->orientation_scale_field)&&
				(graphic->variable_scale_field==
					second_graphic->variable_scale_field)&&
				(graphic->label_field==second_graphic->label_field)&&
				(graphic->label_density_field==second_graphic->label_density_field)&&
				(graphic->font==second_graphic->font);
		}
		/* for element_points and iso_surfaces */
		if (return_code&&
			((CMISS_GRAPHIC_ELEMENT_POINTS==graphic->graphic_type)||
				(CMISS_GRAPHIC_ISO_SURFACES==graphic->graphic_type)))
		{
			return_code=
				(graphic->use_element_type==second_graphic->use_element_type);
		}

		if (return_code && Cmiss_graphic_type_uses_attribute(
			graphic->graphic_type, CMISS_GRAPHIC_ATTRIBUTE_TESSELLATION))
		{
			return_code = (graphic->tessellation == second_graphic->tessellation);
		}

		if (return_code && Cmiss_graphic_type_uses_attribute(
			graphic->graphic_type, CMISS_GRAPHIC_ATTRIBUTE_DISCRETIZATION))
		{
			return_code =
				(graphic->discretization.number_in_xi1 ==
					second_graphic->discretization.number_in_xi1) &&
				(graphic->discretization.number_in_xi2 ==
					second_graphic->discretization.number_in_xi2) &&
				(graphic->discretization.number_in_xi3 ==
					second_graphic->discretization.number_in_xi3);
		}

		if (return_code && Cmiss_graphic_type_uses_attribute(
			graphic->graphic_type, CMISS_GRAPHIC_ATTRIBUTE_NATIVE_DISCRETIZATION_FIELD))
		{
			return_code =
				(graphic->native_discretization_field ==
					second_graphic->native_discretization_field);
		}

		/* for element_points only */
		if (return_code&&
			((CMISS_GRAPHIC_ELEMENT_POINTS==graphic->graphic_type) ||
				(CMISS_GRAPHIC_STREAMLINES==graphic->graphic_type)))
		{
			return_code=
				(graphic->xi_discretization_mode==
					second_graphic->xi_discretization_mode)&&
				(graphic->xi_point_density_field==
					second_graphic->xi_point_density_field);
		}
		/* for graphic starting in a particular element */
		if (return_code&&(CMISS_GRAPHIC_STREAMLINES==graphic->graphic_type))
		{
			return_code=
				(graphic->seed_element==second_graphic->seed_element);
		}
		/* for graphic requiring an exact xi location */
		if (return_code&&(
			(CMISS_GRAPHIC_ELEMENT_POINTS==graphic->graphic_type)||
			(CMISS_GRAPHIC_STREAMLINES==graphic->graphic_type)))
		{
			return_code=
				(graphic->seed_xi[0]==second_graphic->seed_xi[0])&&
				(graphic->seed_xi[1]==second_graphic->seed_xi[1])&&
				(graphic->seed_xi[2]==second_graphic->seed_xi[2]);
		}
		/* for streamlines only */
		if (return_code&&(CMISS_GRAPHIC_STREAMLINES==graphic->graphic_type))
		{
			return_code=
				(graphic->streamline_type==second_graphic->streamline_type)&&
				(graphic->stream_vector_field==second_graphic->stream_vector_field)&&
				(graphic->reverse_track==second_graphic->reverse_track)&&
				(graphic->streamline_length==second_graphic->streamline_length)&&
				(graphic->streamline_width==second_graphic->streamline_width)&&
				(((graphic->seed_nodeset==0) && (second_graphic->seed_nodeset==0)) ||
					((graphic->seed_nodeset) && (second_graphic->seed_nodeset) &&
						Cmiss_nodeset_match(graphic->seed_nodeset, second_graphic->seed_nodeset)))&&
				(graphic->seed_node_mesh_location_field==second_graphic->seed_node_mesh_location_field);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_same_geometry.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_same_geometry */

int Cmiss_graphic_same_name(struct Cmiss_graphic *graphic,
	void *name_void)
{
	int return_code = 0;
	char *name;

	if (graphic && graphic->name && (NULL != (name =(char *)name_void)))
	{
		return_code = (0==strcmp(graphic->name, name));
	}

	return (return_code);
} /* Cmiss_graphic_same_name_or_geometry */

int Cmiss_graphic_same_name_or_geometry(struct Cmiss_graphic *graphic,
	void *second_graphic_void)
{
	int return_code;
	struct Cmiss_graphic *second_graphic;

	ENTER(Cmiss_graphic_same_name_or_geometry);
	if (graphic
		&&(second_graphic=(struct Cmiss_graphic *)second_graphic_void))
	{
		if (graphic->name && second_graphic->name)
		{
			return_code = (0==strcmp(graphic->name,second_graphic->name));
		}
		else
		{
			return_code = Cmiss_graphic_same_geometry(graphic,
				second_graphic_void);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_same_name_or_geometry.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_same_name_or_geometry */

int Cmiss_graphic_list_contents(struct Cmiss_graphic *graphic,
	void *list_data_void)
{
	int return_code;
	char *graphic_string,line[40];
	struct Cmiss_graphic_list_data *list_data;

	ENTER(Cmiss_graphic_list_contents);
	if (graphic&&
		NULL != (list_data=(struct Cmiss_graphic_list_data *)list_data_void))
	{
		if (NULL != (graphic_string=Cmiss_graphic_string(graphic,
					list_data->graphic_string_detail)))
		{
			if (list_data->line_prefix)
			{
				display_message(INFORMATION_MESSAGE,list_data->line_prefix);
			}
			display_message(INFORMATION_MESSAGE,graphic_string);
			if (list_data->line_suffix)
			{
				display_message(INFORMATION_MESSAGE,list_data->line_suffix);
			}
			/*???RC temp */
			if ((GRAPHIC_STRING_COMPLETE_PLUS==list_data->graphic_string_detail)&&
				(graphic->access_count != 1))
			{
				sprintf(line," (access count = %i)",graphic->access_count);
				display_message(INFORMATION_MESSAGE,line);
			}
			display_message(INFORMATION_MESSAGE,"\n");
			DEALLOCATE(graphic_string);
			return_code=1;
		}
		else
		{
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_list_contents.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

			return (return_code);
} /* Cmiss_graphic_list_contents */

int Cmiss_graphic_get_position_in_list(
	struct Cmiss_graphic *graphic,
	struct LIST(Cmiss_graphic) *list_of_graphic)
{
	int position;

	ENTER(Cmiss_graphic_get_position_in_list);
	if (graphic&&list_of_graphic)
	{
		if (IS_OBJECT_IN_LIST(Cmiss_graphic)(graphic,list_of_graphic))
		{
			position=graphic->position;
		}
		else
		{
			position=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_get_position_in_list.  Invalid argument(s)");
		position=0;
	}
	LEAVE;

	return (position);
} /* Cmiss_graphic_get_position_in_list */

int Cmiss_graphic_match(struct Cmiss_graphic *graphic1,
	struct Cmiss_graphic *graphic2)
{
	int return_code;

	ENTER(Cmiss_graphic_match);
	if (graphic1 && graphic2)
	{
		return_code=
			Cmiss_graphic_same_geometry(graphic1, (void *)graphic2) &&
			(graphic1->visibility_flag == graphic2->visibility_flag) &&
			(graphic1->material == graphic2->material) &&
			(graphic1->secondary_material == graphic2->secondary_material) &&
			(graphic1->line_width == graphic2->line_width) &&
			(graphic1->selected_material == graphic2->selected_material) &&
			(graphic1->data_field == graphic2->data_field) &&
			(graphic1->spectrum == graphic2->spectrum) &&
			(graphic1->font == graphic2->font) &&
			(graphic1->render_type == graphic2->render_type) &&
			(graphic1->texture_coordinate_field ==
				graphic2->texture_coordinate_field) &&
			((CMISS_GRAPHIC_STREAMLINES != graphic1->graphic_type) ||
				(graphic1->streamline_data_type ==
					graphic2->streamline_data_type));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_match.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_match */

int Cmiss_graphic_copy_and_put_in_list(
	struct Cmiss_graphic *graphic,void *list_of_graphic_void)
{
	int return_code;
	struct Cmiss_graphic *copy_graphic;
	struct LIST(Cmiss_graphic) *list_of_graphic;

	ENTER(Cmiss_graphic_copy_and_put_in_list);
	if (graphic&&NULL != (list_of_graphic=
		(struct LIST(Cmiss_graphic) *)list_of_graphic_void))
	{
		/* create new graphic to take the copy */
		if (NULL != (copy_graphic=CREATE(Cmiss_graphic)(graphic->graphic_type)))
		{
			/* copy and insert in list */
			if (!(return_code=Cmiss_graphic_copy_without_graphics_object(
				copy_graphic,graphic)&&
				ADD_OBJECT_TO_LIST(Cmiss_graphic)(copy_graphic,
					list_of_graphic)))
			{
				display_message(ERROR_MESSAGE,
					"Cmiss_graphic_copy_and_put_in_list.  "
					"Could not put copy in list");
			}
			DEACCESS(Cmiss_graphic)(&copy_graphic);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_graphic_copy_and_put_in_list.  Could not create copy");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_copy_and_put_in_list.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_copy_and_put_in_list */

int Cmiss_graphic_type_matches(struct Cmiss_graphic *graphic,
	void *graphic_type_void)
{
	int return_code;

	ENTER(Cmiss_graphic_type_matches);
	if (graphic)
	{
		return_code=((void *)graphic->graphic_type == graphic_type_void);
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_graphic_type_matches.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_type_matches */

/***************************************************************************//**
 * Cmiss_graphic list conditional function returning 1 iff the two
 * graphic have the same geometry and the same nontrivial appearance
 * characteristics. Trivial appearance characteristics are the material,
 * visibility and spectrum.
 */
int Cmiss_graphic_same_non_trivial(struct Cmiss_graphic *graphic,
	void *second_graphic_void)
{
	int return_code;
	struct Cmiss_graphic *second_graphic;

	ENTER(Cmiss_graphic_same_non_trivial);
	if (graphic
		&&(second_graphic=(struct Cmiss_graphic *)second_graphic_void))
	{
		return_code=
			Cmiss_graphic_same_geometry(graphic,second_graphic_void)&&
			(graphic->data_field==second_graphic->data_field)&&
			(graphic->render_type==second_graphic->render_type)&&
			(graphic->line_width==second_graphic->line_width)&&
			(graphic->texture_coordinate_field==second_graphic->texture_coordinate_field)&&
			((CMISS_GRAPHIC_STREAMLINES != graphic->graphic_type)||
				(graphic->streamline_data_type==
					second_graphic->streamline_data_type));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_same_non_trivial.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_same_non_trivial */


/***************************************************************************//**
 * Same as Cmiss_graphic_same_non_trivial except <graphic> must also have
 * a graphics_object. Used for getting graphics objects from previous graphic
 * that are the same except for trivial differences such as the material and
 * spectrum which can be changed in the graphics object to match the new graphic .
 */
int Cmiss_graphic_same_non_trivial_with_graphics_object(
	struct Cmiss_graphic *graphic,void *second_graphic_void)
{
	int return_code;

	ENTER(Cmiss_graphic_same_non_trivial_with_graphics_object);

	if (graphic)
	{
		return_code=graphic->graphics_object&&
			Cmiss_graphic_same_non_trivial(graphic,second_graphic_void);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_same_non_trivial_with_graphics_object.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_same_non_trivial_with_graphics_object */

/***************************************************************************//**
 * If <graphic> does not already have a graphics object, this function attempts
 * to find graphic in <list_of_graphic> which differ only trivially in material,
 * spectrum etc. AND have a graphics object. If such a graphic is found, the
 * graphics_object is moved from the matching graphic and put in <graphic>, while
 * any trivial differences are fixed up in the graphics_obejct.
 */
int Cmiss_graphic_extract_graphics_object_from_list(
	struct Cmiss_graphic *graphic,void *list_of_graphic_void)
{
	int return_code;
	struct Cmiss_graphic *matching_graphic;
	struct LIST(Cmiss_graphic) *list_of_graphic;

	ENTER(Cmiss_graphic_extract_graphics_object_from_list);
	if (graphic&&(list_of_graphic=
		(struct LIST(Cmiss_graphic) *)list_of_graphic_void))
	{
		return_code = 1;
		if (!(graphic->graphics_object))
		{
			if (NULL != (matching_graphic = FIRST_OBJECT_IN_LIST_THAT(Cmiss_graphic)(
				Cmiss_graphic_same_non_trivial_with_graphics_object,
				(void *)graphic,list_of_graphic)))
			{
				/* make sure graphics_changed and selected_graphics_changed flags
					 are brought across */
				graphic->graphics_object = matching_graphic->graphics_object;
				/* make sure graphic and graphics object have same material and
					 spectrum */
				Cmiss_graphic_update_non_trivial_GT_objects(graphic);
				graphic->graphics_changed = matching_graphic->graphics_changed;
				graphic->selected_graphics_changed =
					matching_graphic->selected_graphics_changed;
// 				graphic->overlay_flag = matching_graphic->overlay_flag;
// 				graphic->overlay_order = matching_graphic->overlay_order;
				/* reset graphics_object and flags in matching_graphic */
				matching_graphic->graphics_object = (struct GT_object *)NULL;
				Cmiss_graphic_changed(matching_graphic, CMISS_GRAPHIC_CHANGE_FULL_REBUILD);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_extract_graphics_object_from_list.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_extract_graphics_object_from_list */

int Cmiss_graphic_get_data_spectrum_parameters(
	struct Cmiss_graphic *graphic,
	struct Computed_field **data_field,struct Spectrum **spectrum)
{
	int return_code;

	ENTER(Cmiss_graphic_get_data_spectrum_parameters);
	if (graphic&&data_field&&spectrum&&
		(CMISS_GRAPHIC_STREAMLINES != graphic->graphic_type))
	{
		*data_field=graphic->data_field;
		*spectrum=graphic->spectrum;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_get_data_spectrum_parameters.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_get_data_spectrum_parameters */

int Cmiss_graphic_get_radius_parameters(
	struct Cmiss_graphic *graphic,GLfloat *constant_radius,
	GLfloat *radius_scale_factor,struct Computed_field **radius_scalar_field)
{
	int return_code;

	ENTER(Cmiss_graphic_get_radius_parameters);
	if (graphic&&constant_radius&&radius_scale_factor&&radius_scalar_field&&
		(CMISS_GRAPHIC_CYLINDERS==graphic->graphic_type))
	{
		return_code=1;
		*constant_radius=graphic->constant_radius;
		*radius_scale_factor=graphic->radius_scale_factor;
		*radius_scalar_field=graphic->radius_scalar_field;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_get_radius_parameters.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_get_radius_parameters */

int Cmiss_graphic_get_label_field(struct Cmiss_graphic *graphic,
	struct Computed_field **label_field, struct Cmiss_graphics_font **font)
{
	int return_code;

	ENTER(Cmiss_graphic_get_label_field);
	if (graphic&&
		((CMISS_GRAPHIC_NODE_POINTS==graphic->graphic_type)||
			(CMISS_GRAPHIC_DATA_POINTS==graphic->graphic_type)||
			(CMISS_GRAPHIC_ELEMENT_POINTS==graphic->graphic_type)||
			(CMISS_GRAPHIC_POINT==graphic->graphic_type)))
	{
		*label_field = graphic->label_field;
		*font = graphic->font;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_get_label_field.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_get_label_field */

int Cmiss_graphic_get_subgroup_field(struct Cmiss_graphic *graphic,
	struct Computed_field **subgroup_field)
{
	int return_code;

	ENTER(Cmiss_graphic_get_subgroup_field);
	if (graphic)
	{
		*subgroup_field = graphic->subgroup_field;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_get_subgroup_field.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

int Cmiss_graphic_set_subgroup_field(
	struct Cmiss_graphic *graphic, struct Computed_field *subgroup_field)
{
	int return_code;

	ENTER(Cmiss_graphic_set_subgroup_field);
	if (graphic)
	{
		REACCESS(Computed_field)(&(graphic->subgroup_field), subgroup_field);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_set_subgroup_field.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
}

enum Use_element_type Cmiss_graphic_get_use_element_type(
	struct Cmiss_graphic *graphic)
{
	enum Use_element_type use_element_type;

	ENTER(Cmiss_graphic_get_use_element_type);
	if (graphic&&((CMISS_GRAPHIC_ELEMENT_POINTS==graphic->graphic_type)||
		(CMISS_GRAPHIC_ISO_SURFACES==graphic->graphic_type)))
	{
		use_element_type=graphic->use_element_type;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_get_use_element_type.  Invalid argument(s)");
		use_element_type = USE_ELEMENTS;
	}
	LEAVE;

	return (use_element_type);
} /* Cmiss_graphic_get_use_element_type */

struct Cmiss_tessellation *Cmiss_graphic_get_tessellation(
	Cmiss_graphic_id graphic)
{
	struct Cmiss_tessellation *tessellation = NULL;
	if (graphic)
	{
		tessellation=graphic->tessellation;
		if (tessellation)
		{
			ACCESS(Cmiss_tessellation)(tessellation);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_get_tessellation.  Invalid argument(s)");
	}
	return (tessellation);
}

int Cmiss_graphic_set_tessellation(
	Cmiss_graphic_id graphic, struct Cmiss_tessellation *tessellation)
{
	int return_code = 1;
	if (graphic && Cmiss_graphic_type_uses_attribute(graphic->graphic_type,
		CMISS_GRAPHIC_ATTRIBUTE_TESSELLATION))
	{
		if (tessellation != graphic->tessellation)
		{
			REACCESS(Cmiss_tessellation)(&(graphic->tessellation), tessellation);
			Cmiss_graphic_changed(graphic, CMISS_GRAPHIC_CHANGE_FULL_REBUILD);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_set_tessellation.  Invalid argument(s)");
		return_code = 0;
	}
	return (return_code);
}

int Cmiss_graphic_get_discretization(struct Cmiss_graphic *graphic,
	struct Element_discretization *discretization)
{
	int return_code;

	ENTER(Cmiss_graphic_get_discretization);
	if (graphic&&discretization)
	{
		discretization->number_in_xi1=graphic->discretization.number_in_xi1;
		discretization->number_in_xi2=graphic->discretization.number_in_xi2;
		discretization->number_in_xi3=graphic->discretization.number_in_xi3;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_get_discretization.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_get_discretization */

int Cmiss_graphic_get_top_level_number_in_xi(struct Cmiss_graphic *graphic,
	int max_dimensions, int *top_level_number_in_xi)
{
	int return_code = 1;
	if (graphic && (0 < max_dimensions) && top_level_number_in_xi)
	{
		int dim;
		for (dim = 0; dim < max_dimensions; dim++)
		{
			top_level_number_in_xi[dim] = 1;
		}
		if (graphic->tessellation)
		{
			Cmiss_tessellation_get_minimum_divisions(graphic->tessellation,
				max_dimensions, top_level_number_in_xi);
			if (graphic->coordinate_field)
			{
				// refine if coordinate field is non-linear
				// first check if its coordinate system is non-linear (cheaper)
				Coordinate_system_type type = get_coordinate_system_type(
					Computed_field_get_coordinate_system(graphic->coordinate_field));
				if (Coordinate_system_type_is_non_linear(type) ||
					Computed_field_is_non_linear(graphic->coordinate_field))
				{
					int *refinement_factors = new int[max_dimensions];
					if (Cmiss_tessellation_get_refinement_factors(graphic->tessellation,
						max_dimensions, refinement_factors))
					{
						for (dim = 0; dim < max_dimensions; dim++)
						{
							top_level_number_in_xi[dim] *= refinement_factors[dim];
						}
					}
					delete [] refinement_factors;
				}
			}
		}
		if (Cmiss_graphic_type_uses_attribute(graphic->graphic_type, CMISS_GRAPHIC_ATTRIBUTE_DISCRETIZATION))
		{
			top_level_number_in_xi[0] *= graphic->discretization.number_in_xi1;
			if (max_dimensions > 1)
				top_level_number_in_xi[1] *= graphic->discretization.number_in_xi2;
			if (max_dimensions > 2)
				top_level_number_in_xi[2] *= graphic->discretization.number_in_xi3;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_get_top_level_number_in_xi.  Invalid argument(s)");
		return_code = 0;
	}
	return return_code;
}

struct FE_field *Cmiss_graphic_get_native_discretization_field(
	struct Cmiss_graphic *graphic)
{
	struct FE_field *native_discretization_field;

	ENTER(Cmiss_graphic_get_native_discretization_field);
	if (graphic)
	{
		native_discretization_field=graphic->native_discretization_field;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_get_native_discretization_field.  "
			"Invalid argument(s)");
		native_discretization_field=(struct FE_field *)NULL;
	}
	LEAVE;

	return (native_discretization_field);
} /* Cmiss_graphic_get_native_discretization_field */

int Cmiss_graphic_set_native_discretization_field(
	struct Cmiss_graphic *graphic, struct FE_field *native_discretization_field)
{
	int return_code;

	ENTER(Cmiss_graphic_set_native_discretization_field);
	if (graphic && Cmiss_graphic_type_uses_attribute(graphic->graphic_type,
		CMISS_GRAPHIC_ATTRIBUTE_NATIVE_DISCRETIZATION_FIELD))
	{
		return_code=1;
		REACCESS(FE_field)(&(graphic->native_discretization_field),
			native_discretization_field);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_set_native_discretization_field.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
}

int Cmiss_graphic_get_circle_discretization(struct Cmiss_graphic *graphic)
{
	int circle_discretization;

	ENTER(Cmiss_graphic_get_discretization);
	if (graphic)
	{
	 circle_discretization = graphic->circle_discretization;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_get_circle_discretization.  Invalid argument(s)");
		circle_discretization = 0;
	}
	LEAVE;

	return (circle_discretization);
} /* Cmiss_graphic_get_discretization */

int Cmiss_graphic_set_circle_discretization(
	struct Cmiss_graphic *graphic,int circle_discretization)
{
	int return_code;

	ENTER(Cmiss_graphic_set_circle_discretization);
	if (graphic)
	{
		if (graphic->circle_discretization != circle_discretization
			&& (CMISS_GRAPHIC_CYLINDERS==graphic->graphic_type))
		{
			graphic->circle_discretization = circle_discretization;
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_set_circle_discretization.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
}

struct FE_element *Cmiss_graphic_get_seed_element(
	struct Cmiss_graphic *graphic)
{
	struct FE_element *seed_element;

	ENTER(Cmiss_graphic_get_seed_element);
	if (graphic&&(CMISS_GRAPHIC_STREAMLINES==graphic->graphic_type))
	{
		seed_element=graphic->seed_element;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_get_seed_element.  Invalid argument(s)");
		seed_element=(struct FE_element *)NULL;
	}
	LEAVE;

	return (seed_element);
} /* Cmiss_graphic_get_seed_element */

int Cmiss_graphic_set_seed_element(struct Cmiss_graphic *graphic,
	struct FE_element *seed_element)
/*******************************************************************************
LAST MODIFIED : 19 March 1999

DESCRIPTION :
For graphic starting in a particular element.
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_graphic_set_seed_element);
	if (graphic&&(CMISS_GRAPHIC_STREAMLINES==graphic->graphic_type))
	{
		REACCESS(FE_element)(&graphic->seed_element,seed_element);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_set_seed_element.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_set_seed_element */

int Cmiss_graphic_get_seed_xi(struct Cmiss_graphic *graphic,
	Triple seed_xi)
{
	int return_code;

	ENTER(Cmiss_graphic_get_seed_xi);
	if (graphic&&seed_xi&&(
		(CMISS_GRAPHIC_ELEMENT_POINTS==graphic->graphic_type)||
		(CMISS_GRAPHIC_STREAMLINES==graphic->graphic_type)))
	{
		seed_xi[0]=graphic->seed_xi[0];
		seed_xi[1]=graphic->seed_xi[1];
		seed_xi[2]=graphic->seed_xi[2];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_get_seed_xi.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_get_seed_xi */

int Cmiss_graphic_set_seed_xi(struct Cmiss_graphic *graphic,
	Triple seed_xi)
{
	int return_code;

	ENTER(Cmiss_graphic_set_seed_xi);
	if (graphic&&seed_xi&&(
		(CMISS_GRAPHIC_ELEMENT_POINTS==graphic->graphic_type)||
		(CMISS_GRAPHIC_STREAMLINES==graphic->graphic_type)))
	{
		graphic->seed_xi[0]=seed_xi[0];
		graphic->seed_xi[1]=seed_xi[1];
		graphic->seed_xi[2]=seed_xi[2];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_set_seed_xi.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_set_seed_xi */

int Cmiss_graphic_get_line_width(struct Cmiss_graphic *graphic)
{
	int line_width;

	ENTER(Cmiss_graphic_get_line_width);
	if (graphic)
	{
		line_width=graphic->line_width;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_get_line_width.  Invalid argument(s)");
		line_width = 0;
	}
	LEAVE;

	return (line_width);
} /* Cmiss_graphic_get_line_width */

int Cmiss_graphic_set_line_width(struct Cmiss_graphic *graphic, int line_width)
{
	int return_code;

	ENTER(Cmiss_graphic_set_line_width);
	if (graphic)
	{
	  return_code = 1;
	  graphic->line_width = line_width;
	}
	else
	{
	  display_message(ERROR_MESSAGE,
		"Cmiss_graphic_set_line_width.  Invalid argument(s)");
	  return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_set_line_width */

int Cmiss_graphic_get_data_spectrum_parameters_streamlines(
	struct Cmiss_graphic *graphic,
	enum Streamline_data_type *streamline_data_type,
	struct Computed_field **data_field,struct Spectrum **spectrum)
{
	int return_code;

	ENTER(Cmiss_graphic_get_data_spectrum_parameters_streamlines);
	if (graphic&&streamline_data_type&&data_field&&spectrum&&
		(CMISS_GRAPHIC_STREAMLINES==graphic->graphic_type))
	{
		*streamline_data_type=graphic->streamline_data_type;
		*data_field=graphic->data_field;
		*spectrum=graphic->spectrum;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_get_data_spectrum_parameters_streamlines.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_get_data_spectrum_parameters_streamlines */

struct Computed_field *Cmiss_graphic_get_texture_coordinate_field(
	struct Cmiss_graphic *graphic)
{
	struct Computed_field *texture_coordinate_field;

	ENTER(Cmiss_graphic_get_texture_coordinate_field);
	if (graphic)
	{
		texture_coordinate_field=graphic->texture_coordinate_field;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_get_texture_coordinate_field.  Invalid argument(s)");
		texture_coordinate_field=(struct Computed_field *)NULL;
	}
	LEAVE;

	return (texture_coordinate_field);
} /* Cmiss_graphic_get_texture_coordinate_field */

int Cmiss_graphic_set_texture_coordinate_field(
   struct Cmiss_graphic *graphic, struct Computed_field *texture_coordinate_field)
{
	int return_code = 1;

	ENTER(Cmiss_graphic_set_texture_coordinate_field);
	if (graphic)
	{
		if (texture_coordinate_field != graphic->texture_coordinate_field)
		{
			REACCESS(Computed_field)(&graphic->texture_coordinate_field, texture_coordinate_field);
			Cmiss_graphic_changed(graphic, CMISS_GRAPHIC_CHANGE_FULL_REBUILD);
		}
	}
	else
	{
		return_code = 0;
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_set_texture_coordinate_field.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_set_texture_coordinate_field */

enum Cmiss_graphics_render_type Cmiss_graphic_get_render_type(
	struct Cmiss_graphic *graphic)
{
	enum Cmiss_graphics_render_type render_type;

	ENTER(Cmiss_graphic_get_render_type);
	if (graphic)
	{
		render_type=graphic->render_type;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_get_render_type.  Invalid argument(s)");
		render_type = CMISS_GRAPHICS_RENDER_TYPE_SHADED;
	}
	LEAVE;

	return (render_type);
} /* Cmiss_graphic_get_render_type */

int Cmiss_graphic_get_exterior(struct Cmiss_graphic *graphic)
{
	int return_code;

	ENTER(Cmiss_graphic_get_exterior);
	if (graphic&&(
		Cmiss_graphic_type_uses_dimension(graphic->graphic_type,1)||
		Cmiss_graphic_type_uses_dimension(graphic->graphic_type,2)))
	{
		return_code=graphic->exterior;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_get_exterior.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_get_exterior */

int Cmiss_graphic_get_face(struct Cmiss_graphic *graphic,int *face)
{
	int return_code;

	ENTER(Cmiss_graphic_get_face);
	if (graphic&&face&&(
		Cmiss_graphic_type_uses_dimension(graphic->graphic_type,1)||
		Cmiss_graphic_type_uses_dimension(graphic->graphic_type,2)))
	{
		return_code=(0 <= graphic->face);
		*face=graphic->face;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_get_face.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_get_face */

int Cmiss_graphic_time_change(
	struct Cmiss_graphic *graphic,void *dummy_void)
{
	int return_code;

	ENTER(Cmiss_graphic_time_change);
	USE_PARAMETER(dummy_void);
	if (graphic)
	{
		return_code = 1;
		if (graphic->glyph && (1 < GT_object_get_number_of_times(graphic->glyph)))
		{
			GT_object_changed(graphic->glyph);
		}
		if (graphic->time_dependent)
		{
			Cmiss_graphic_changed(graphic, CMISS_GRAPHIC_CHANGE_FULL_REBUILD);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_time_change.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_time_change */

int Cmiss_graphic_update_time_behaviour(
	struct Cmiss_graphic *graphic, void *update_time_behaviour_void)
{
	int return_code, time_dependent;
	struct Cmiss_graphic_update_time_behaviour_data *data;

	ENTER(Cmiss_graphic_update_time_behaviour);
	if (graphic && (data =
		(struct Cmiss_graphic_update_time_behaviour_data *)
		update_time_behaviour_void))
	{
		return_code = 1;
		time_dependent = 0;
		if (graphic->glyph && (1 < GT_object_get_number_of_times(graphic->glyph)))
		{
			time_dependent = 1;
		}
		if (graphic->coordinate_field)
		{
			if (Computed_field_has_multiple_times(graphic->coordinate_field))
			{
				time_dependent = 1;
			}
		}
		else
		{
			if (data->default_coordinate_depends_on_time)
			{
				time_dependent = 1;
			}
		}
		if (graphic->texture_coordinate_field && Computed_field_has_multiple_times(
			graphic->texture_coordinate_field))
		{
			time_dependent = 1;
		}
		if (graphic->radius_scalar_field && Computed_field_has_multiple_times(
			graphic->radius_scalar_field))
		{
			time_dependent = 1;
		}
		if (graphic->iso_scalar_field && Computed_field_has_multiple_times(
			graphic->iso_scalar_field))
		{
			time_dependent = 1;
		}
		if (graphic->orientation_scale_field &&
			Computed_field_has_multiple_times(graphic->orientation_scale_field))
		{
			time_dependent = 1;
		}
		if (graphic->variable_scale_field &&
			Computed_field_has_multiple_times(graphic->variable_scale_field))
		{
			time_dependent = 1;
		}
		if (graphic->label_field &&
			Computed_field_has_multiple_times(graphic->label_field))
		{
			time_dependent = 1;
		}
		if (graphic->label_density_field &&
			Computed_field_has_multiple_times(graphic->label_density_field))
		{
			time_dependent = 1;
		}
		if (graphic->subgroup_field &&
			Computed_field_has_multiple_times(graphic->subgroup_field))
		{
			time_dependent = 1;
		}
		if (graphic->variable_scale_field &&
			Computed_field_has_multiple_times(graphic->variable_scale_field))
		{
			time_dependent = 1;
		}
		if (graphic->displacement_map_field &&
			Computed_field_has_multiple_times(graphic->displacement_map_field))
		{
			time_dependent = 1;
		}
		if (graphic->stream_vector_field &&
			Computed_field_has_multiple_times(graphic->stream_vector_field))
		{
			time_dependent = 1;
		}
		if (graphic->data_field &&
			Computed_field_has_multiple_times(graphic->data_field))
		{
			time_dependent = 1;
		}
		/* Or any field that is pointed to has multiple times...... */

		graphic->time_dependent = time_dependent;
		if (time_dependent)
		{
			data->time_dependent = time_dependent;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_update_time_behaviour.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_update_time_behaviour */

int Cmiss_graphics_material_change(
	struct Cmiss_graphic *graphic, void *material_change_data_void)
{
	int return_code;
	Cmiss_graphics_material_change_data *material_change_data =
		(Cmiss_graphics_material_change_data *)material_change_data_void;
	if (graphic && material_change_data)
	{
		return_code = 1;
		bool material_change = false;
		if (graphic->material)
		{
			int change_flags = MANAGER_MESSAGE_GET_OBJECT_CHANGE(Graphical_material)(
				material_change_data->manager_message, graphic->material);
			material_change = (change_flags & MANAGER_CHANGE_RESULT(Graphical_material)) != 0;
		}
		if (!material_change && graphic->secondary_material)
		{
			int change_flags = MANAGER_MESSAGE_GET_OBJECT_CHANGE(Graphical_material)(
				material_change_data->manager_message, graphic->secondary_material);
			material_change = (change_flags & MANAGER_CHANGE_RESULT(Graphical_material)) != 0;
		}
		if (!material_change && graphic->selected_material)
		{
			int change_flags = MANAGER_MESSAGE_GET_OBJECT_CHANGE(Graphical_material)(
				material_change_data->manager_message, graphic->selected_material);
			material_change = (change_flags & MANAGER_CHANGE_RESULT(Graphical_material)) != 0;
		}
		if (material_change)
		{
			if (graphic->graphics_object)
			{
				GT_object_Graphical_material_change(graphic->graphics_object,
					(struct LIST(Graphical_material) *)NULL);
			}
			/* need a way to tell either graphic is used in any scene or not */
			material_change_data->graphics_changed = 1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphics_material_change.  Invalid argument(s)");
		return_code = 0;
	}
	return return_code;
}

int Cmiss_graphic_spectrum_change(
	struct Cmiss_graphic *graphic, void *spectrum_change_data_void)
{
	int return_code;
	Cmiss_graphic_spectrum_change_data *spectrum_change_data =
		(Cmiss_graphic_spectrum_change_data *)spectrum_change_data_void;
	if (graphic && spectrum_change_data)
	{
		return_code = 1;
		if (graphic->spectrum)
		{
			int change_flags = MANAGER_MESSAGE_GET_OBJECT_CHANGE(Spectrum)(
				spectrum_change_data->manager_message, graphic->spectrum);
			if (change_flags & MANAGER_CHANGE_RESULT(Spectrum))
			{
				if (graphic->graphics_object)
				{
					GT_object_Spectrum_change(graphic->graphics_object,
						(struct LIST(Spectrum) *)NULL);
				}
				/* need a way to tell either graphic is used in any scene or not */
				spectrum_change_data->graphics_changed = 1;
			}
		}
		/* The material gets it's own notification of the change,
			it should propagate that to the Cmiss_graphic */
		struct Spectrum *colour_lookup;
		if (graphic->material && (colour_lookup =
				Graphical_material_get_colour_lookup_spectrum(graphic->material)))
		{
			int change_flags = MANAGER_MESSAGE_GET_OBJECT_CHANGE(Spectrum)(
				spectrum_change_data->manager_message, colour_lookup);
			if (change_flags & MANAGER_CHANGE_RESULT(Spectrum))
			{
				if (graphic->graphics_object)
				{
					GT_object_Graphical_material_change(graphic->graphics_object,
						(struct LIST(Graphical_material) *)NULL);
				}
				/* need a way to tell either graphic is used in any scene or not */
				spectrum_change_data->graphics_changed = 1;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_spectrum_change.  Invalid argument(s)");
		return_code = 0;
	}
	return return_code;
}

int Cmiss_graphic_tessellation_change(struct Cmiss_graphic *graphic,
	void *tessellation_manager_message_void)
{
	int return_code;
	struct MANAGER_MESSAGE(Cmiss_tessellation) *manager_message =
		(struct MANAGER_MESSAGE(Cmiss_tessellation) *)tessellation_manager_message_void;
	if (graphic && manager_message)
	{
		return_code = 1;
		if (graphic->tessellation)
		{
			int change_flags = MANAGER_MESSAGE_GET_OBJECT_CHANGE(Cmiss_tessellation)(
				manager_message, graphic->tessellation);
			// GRC: following could be smarter, e.g. by checking if actual discretization is changing
			if (change_flags & MANAGER_CHANGE_RESULT(Cmiss_tessellation))
			{
				Cmiss_graphic_changed(graphic, CMISS_GRAPHIC_CHANGE_FULL_REBUILD);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_tessellation_change.  Invalid argument(s)");
		return_code = 0;
	}
	return return_code;
}

int Cmiss_graphic_font_change(struct Cmiss_graphic *graphic,
	void *font_manager_message_void)
{
	int return_code;
	struct MANAGER_MESSAGE(Cmiss_graphics_font) *manager_message =
		(struct MANAGER_MESSAGE(Cmiss_graphics_font) *)font_manager_message_void;
	if (graphic && manager_message)
	{
		return_code = 1;
		if (graphic->font)
		{
			if (((graphic->graphic_type == CMISS_GRAPHIC_NODE_POINTS) ||
				(graphic->graphic_type == CMISS_GRAPHIC_DATA_POINTS) ||
				(graphic->graphic_type == CMISS_GRAPHIC_ELEMENT_POINTS) ||
				(graphic->graphic_type == CMISS_GRAPHIC_POINT)) && graphic->label_field)
			{
				int change_flags = MANAGER_MESSAGE_GET_OBJECT_CHANGE(Cmiss_graphics_font)(
					manager_message, graphic->font);
				// GRC: following could be smarter, e.g. by checking if actual discretization is changing
				if (change_flags & MANAGER_CHANGE_RESULT(Cmiss_graphics_font))
				{
					Cmiss_graphic_changed(graphic, CMISS_GRAPHIC_CHANGE_FULL_REBUILD);
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_tessellation_change.  Invalid argument(s)");
		return_code = 0;
	}
	return return_code;
}

int Cmiss_graphic_set_customised_graphics_object(
	struct Cmiss_graphic *graphic, struct GT_object *graphics_object)
{
	int return_code = 0;

	ENTER(Cmiss_graphic_set_customised_graphics_object);
	if (graphic)
	{
		switch (graphic->graphic_type)
		{
			case CMISS_GRAPHIC_POINT:
			{
				if (graphic->customised_graphics_object != graphics_object)
				{
					REACCESS(GT_object)(&(graphic->customised_graphics_object), graphics_object);
					return_code = 1;
				}
			} break;
			default :
			{
				display_message(INFORMATION_MESSAGE,
					"Cmiss_graphic_customised_graphics_object.  Cannot set customised"
					"graphics_object for this cmiss graphic type");
			}break;
		}
	}
	else
	{
		display_message(INFORMATION_MESSAGE,
			"Cmiss_graphic_customised_graphics_object.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
}

int Cmiss_graphic_detach_fields(struct Cmiss_graphic *graphic, void *dummy_void)
{
	int return_code = 1;
	USE_PARAMETER(dummy_void);

	if (graphic)
	{
		if (graphic->coordinate_field)
		{
			DEACCESS(Computed_field)(&(graphic->coordinate_field));
		}
		if (graphic->texture_coordinate_field)
		{
			DEACCESS(Computed_field)(&(graphic->texture_coordinate_field));
		}
		if (graphic->radius_scalar_field)
		{
			DEACCESS(Computed_field)(&(graphic->radius_scalar_field));
		}
		if (graphic->iso_scalar_field)
		{
			DEACCESS(Computed_field)(&(graphic->iso_scalar_field));
		}
		if (graphic->orientation_scale_field)
		{
			DEACCESS(Computed_field)(&(graphic->orientation_scale_field));
		}
		if (graphic->variable_scale_field)
		{
			DEACCESS(Computed_field)(&(graphic->variable_scale_field));
		}
		if (graphic->label_field)
		{
			DEACCESS(Computed_field)(&(graphic->label_field));
		}
		if (graphic->label_density_field)
		{
			DEACCESS(Computed_field)(&(graphic->label_density_field));
		}
		if (graphic->subgroup_field)
		{
			DEACCESS(Computed_field)(&(graphic->subgroup_field));
		}
		if (graphic->displacement_map_field)
		{
			DEACCESS(Computed_field)(&(graphic->displacement_map_field));
		}
		if (graphic->xi_point_density_field)
		{
			DEACCESS(Computed_field)(&(graphic->xi_point_density_field));
		}
		if (graphic->native_discretization_field)
		{
			DEACCESS(FE_field)(&(graphic->native_discretization_field));
		}
		if (graphic->stream_vector_field)
		{
			DEACCESS(Computed_field)(&(graphic->stream_vector_field));
		}
		if (graphic->data_field)
		{
			DEACCESS(Computed_field)(&(graphic->data_field));
		}
		if (graphic->seed_node_mesh_location_field)
		{
			DEACCESS(Computed_field)(&(graphic->seed_node_mesh_location_field));
		}
	}
	else
	{
		display_message(INFORMATION_MESSAGE,
			"Cmiss_graphic_detach_fields.  Invalid argument(s)");
		return_code = 0;
	}

	return return_code;
}

int Cmiss_graphic_selected_element_points_change(
	struct Cmiss_graphic *graphic,void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 28 February 2000

DESCRIPTION :
Tells <graphic> that if the graphics resulting from it depend on the currently
selected element points, then they should be updated.
Must call Cmiss_graphic_to_graphics_object afterwards to complete.
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_graphic_selected_element_points_change);
	USE_PARAMETER(dummy_void);
	if (graphic)
	{
		return_code=1;
		if (graphic->graphics_object&&
			(CMISS_GRAPHIC_ELEMENT_POINTS==graphic->graphic_type))
		{
			Cmiss_graphic_update_selected(graphic, (void *)NULL);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_selected_element_points_change.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_selected_element_points_change */

struct Cmiss_rendition *Cmiss_graphic_get_rendition_private(struct Cmiss_graphic *graphic)
{
	if (graphic)
		return graphic->rendition;
	return NULL;
}

int Cmiss_graphic_set_rendition_private(struct Cmiss_graphic *graphic,
	struct Cmiss_rendition *rendition)
{
	if (graphic && ((NULL == rendition) || (NULL == graphic->rendition)))
	{
		graphic->rendition = rendition;
		return 1;
	}
	else
	{
		display_message(INFORMATION_MESSAGE,
			"Cmiss_graphic_set_rendition_private.  Invalid argument(s)");
	}
	return 0;
}

int Cmiss_graphic_set_rendition_for_list_private(struct Cmiss_graphic *graphic, void *rendition_void)
{
	Cmiss_rendition *rendition = (Cmiss_rendition *)rendition_void;
	int return_code = 0;
	if (graphic && rendition)
	{
		if (graphic->rendition == rendition)
		{
			return_code = 1;
		}
		else
		{
			return_code = Cmiss_graphic_set_rendition_private(graphic, NULL);
			return_code = Cmiss_graphic_set_rendition_private(graphic, rendition);
		}
	}
	else
	{
		display_message(INFORMATION_MESSAGE,
			"Cmiss_graphic_set_rendition_for_list_private.  Invalid argument(s)");
	}

	return return_code;
}

Cmiss_graphic_id Cmiss_graphic_access(Cmiss_graphic_id graphic)
{
	return (ACCESS(Cmiss_graphic)(graphic));
}

int Cmiss_graphic_destroy(Cmiss_graphic_id *graphic)
{
	return DEACCESS(Cmiss_graphic)(graphic);
}

class Cmiss_graphic_type_conversion
{
public:
	static const char *to_string(enum Cmiss_graphic_type type)
	{
		const char *enum_string = 0;
		switch (type)
		{
			case CMISS_GRAPHIC_NODE_POINTS:
				enum_string = "NODE_POINTS";
				break;
			case CMISS_GRAPHIC_DATA_POINTS:
				enum_string = "DATA_POINTS";
				break;
			case CMISS_GRAPHIC_LINES:
				enum_string = "LINES";
				break;
			case CMISS_GRAPHIC_CYLINDERS:
				enum_string = "CYLINDERS";
				break;
			case CMISS_GRAPHIC_SURFACES:
				enum_string = "SURFACES";
				break;
			case CMISS_GRAPHIC_ISO_SURFACES:
				enum_string = "ISO_SURFACES";
				break;
			case CMISS_GRAPHIC_ELEMENT_POINTS:
				enum_string = "ELEMENT_POINTS";
				break;
			case CMISS_GRAPHIC_STREAMLINES:
				enum_string = "STREAMLINES";
				break;
			case CMISS_GRAPHIC_POINT:
				enum_string = "POINT";
				break;
		default:
			break;
		}
		return enum_string;
	}
};

enum Cmiss_graphic_type Cmiss_graphic_type_enum_from_string(const char *string)
{
	return string_to_enum<enum Cmiss_graphic_type, Cmiss_graphic_type_conversion>(string);
}

char *Cmiss_graphic_type_enum_to_string(enum Cmiss_graphic_type type)
{
	const char *type_string = Cmiss_graphic_type_conversion::to_string(type);
	return (type_string ? duplicate_string(type_string) : 0);
}

class Cmiss_graphics_render_type_conversion
{
public:
	static const char *to_string(enum Cmiss_graphics_render_type type)
	{
		const char *enum_string = 0;
		switch (type)
		{
		case CMISS_GRAPHICS_RENDER_TYPE_SHADED:
			enum_string = "SHADED";
			break;
		case CMISS_GRAPHICS_RENDER_TYPE_WIREFRAME:
			enum_string = "WIREFRAME";
			break;
		default:
			break;
		}
		return enum_string;
	}
};

enum Cmiss_graphics_render_type Cmiss_graphics_render_type_enum_from_string(
	const char *string)
{
	return string_to_enum<enum Cmiss_graphics_render_type,
		Cmiss_graphics_render_type_conversion>(string);
}

char *Cmiss_graphics_render_type_enum_to_string(
	enum Cmiss_graphics_render_type type)
{
	const char *type_string = Cmiss_graphics_render_type_conversion::to_string(type);
	return (type_string ? duplicate_string(type_string) : 0);
}

int Cmiss_graphic_define(Cmiss_graphic_id graphic, const char *command_string)
{
	int return_code = 0;
	Cmiss_rendition_id owning_rendition = NULL;

	if (graphic && command_string && (NULL != (owning_rendition = graphic->rendition)))
	{
		struct Modify_rendition_data modify_rendition_data;
		modify_rendition_data.delete_flag = 0;
		modify_rendition_data.position = -1;
		modify_rendition_data.graphic = Cmiss_graphic_access(graphic);
		modify_rendition_data.modify_this_graphic = 1;
		modify_rendition_data.group = 0;
		struct Rendition_command_data rendition_command_data;
		Cmiss_rendition_fill_rendition_command_data(owning_rendition, &rendition_command_data);
		Cmiss_graphic_changed(graphic, CMISS_GRAPHIC_CHANGE_FULL_REBUILD);
		if (modify_rendition_data.graphic)
		{
			Cmiss_graphic_destroy(&(modify_rendition_data.graphic));
		}
		if (rendition_command_data.default_font)
		{
			DEACCESS(Cmiss_graphics_font)(&rendition_command_data.default_font);
		}
		if (rendition_command_data.default_spectrum)
		{
			DEACCESS(Spectrum)(&rendition_command_data.default_spectrum);
		}
		if (rendition_command_data.root_region)
		{
			Cmiss_region_destroy(&(rendition_command_data.root_region));
		}
	}

	return return_code;
}

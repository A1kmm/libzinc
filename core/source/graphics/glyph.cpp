/*******************************************************************************
FILE : glyph.c

LAST MODIFIED : 19 September 2005

DESCRIPTION :
Glyphs are GT_objects which contain simple geometric shapes such as
cylinders, arrows and axes which are (or should) fit into a unit (1x1x1) cube,
with the major axes of the glyph aligned with the x, y and z axes. 
The logical centre of each glyph should be at (0,0,0). This should be
interpreted as follows:
- if the glyph is symmetrical along any axis, its coordinates in that
direction should vary from -0.5 to +0.5;
- if the glyph involves any sort of arrow that is unsymmetric in its direction
(ie. is single-headed), (0,0,0) should be at the base of the arrow.
- axes should therefore be centred at (0,0,0) and extend to 1 in each axis
direction. Axis titles "x", "y" and "z" may be outside the unit cube.

Glyphs are referenced by GT_glyph_set objects. Glyphs themselves should not
reference graphical materials or spectrums.
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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "general/debug.h"
#include "general/mystring.h"
#include "graphics/glyph.h"
#include "graphics/graphics_object.h"
#include "graphics/defined_graphics_objects.h"
#include "general/message.h"
#include "graphics/spectrum.h"
#include "graphics/graphics_object.hpp"
#include "graphics/render_gl.h"

/*
Module functions
----------------
*/

static int construct_tube(int number_of_segments_around,ZnReal x1,ZnReal r1,
	ZnReal x2,ZnReal r2,ZnReal cy,ZnReal cz,int primary_axis,Triple *vertex_list,
	Triple *normal_list)
/*******************************************************************************
LAST MODIFIED : 17 July 1998

DESCRIPTION :
Adds vertices and normals for a tube/cone/anulus/disc stretching from position
x1 with radius r1 to position x2 with radius r2. The axis of the cylinder is
parallel with the x axis and its centre is at cy,cz. If <primary_axis> is 1, the
above is the case, if its value is 2, x->y, y->z and z->x, and a further
permutation if <primary_axis> is 3. Values other than 1, 2 or 3 are taken as 1.
The vertices and normals are added to create a single quadrilateral strip
suitable for using with GT_surfaces of type g_SH_DISCONTINUOUS_STRIP.
==============================================================================*/
{
	ZnReal longitudinal_normal,normal_angle,radial_normal,theta,y,z;
	int j,ix,iy,iz,return_code;
	Triple *normal,*vertex;

	ENTER(construct_tube);
	if ((2<number_of_segments_around)&&((x1 != x2)||(r1 != r2))&&
		vertex_list&&normal_list)
	{
		return_code=1;
		switch (primary_axis)
		{
			case 2:
			{
				ix=1;
				iy=2;
				iz=0;
			} break;
			case 3:
			{
				ix=2;
				iy=0;
				iz=1;
			} break;
			default:
			{
				ix=0;
				iy=1;
				iz=2;
			} break;
		}
		vertex=vertex_list;
		normal=normal_list;
		/* get radial and longitudinal components of surface normals */
		normal_angle=atan2(r2-r1,x2-x1);
		radial_normal=cos(normal_angle);
		longitudinal_normal=-sin(normal_angle);
		for (j=0;j <= number_of_segments_around;j++)
		{
			theta=2.0*PI*(ZnReal)j/(ZnReal)number_of_segments_around;
			y = sin(theta);
			z = cos(theta);
			(*vertex)[ix] = x1;
			(*vertex)[iy] = cy+r1*y;
			(*vertex)[iz] = cz+r1*z;
			vertex++;
			(*vertex)[ix] = x2;
			(*vertex)[iy] = cy+r2*y;
			(*vertex)[iz] = cz+r2*z;
			vertex++;
			y *= radial_normal;
			z *= radial_normal;
			(*normal)[ix] = longitudinal_normal;
			(*normal)[iy] = y;
			(*normal)[iz] = z;
			normal++;
			(*normal)[ix] = longitudinal_normal;
			(*normal)[iy] = y;
			(*normal)[iz] = z;
			normal++;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"construct_tube.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* construct_tube */

static int tick_mark_get_grid_spacing(FE_value *minor_grid_size,
	int *minor_grids_per_major,FE_value scale, FE_value min_minor_grid,
	FE_value min_major_grid)
/*******************************************************************************
LAST MODIFIED : 19 September 2005

DESCRIPTION :
Returns the spacing of minor and major (major include labels in this case) grids
that cross the drawing area. From an initial, desired <minor_grid_spacing>, this
is enlarged by multiplying by 2,2.5,2 (to give 2x,5x,10x,20x the original value)
until the grid is spaced apart by at least <min_minor_grid_pixels> on the
screen. FE_Value values are multiplied by <scale> to give pixel values.
A suitable number of minor_grids_per_major is then calculated, ensuring that
major grid lines are spaced apart by at least <min_major_grid_pixels>.
Copied from the control curve editor.
==============================================================================*/
{
	int return_code,j;

	ENTER(tick_mark_get_grid_spacing);
	if (*minor_grid_size&&(0< *minor_grid_size)&&minor_grids_per_major&&
		(0.0!=scale))
	{
		j=1;
		while (fabs(scale*(*minor_grid_size)) < min_minor_grid)
		{
			j=(j+1)%3;
			if (j)
			{
				*minor_grid_size *= 2.0;
			}
			else
			{
				*minor_grid_size *= 2.5;
			}
		}
		*minor_grids_per_major=1;
		if (0==j)
		{
			j=1;
			if (fabs(scale*(*minor_grids_per_major)*(*minor_grid_size))<
				min_major_grid)
			{
				*minor_grids_per_major *= 2;
			}
		}
		while (fabs(scale*(*minor_grids_per_major)*(*minor_grid_size))<
			min_major_grid)
		{
			j=(j+1)%2;
			if (j)
			{
				*minor_grids_per_major *= 5;
			}
			else
			{
				*minor_grids_per_major *= 2;
			}
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"tick_mark_get_grid_spacing.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* tick_mark_get_grid_spacing */

static int draw_glyph_axes_general(Triple *coordinate_scaling, 
	int label_bounds_dimension, int label_bounds_components, ZnReal *label_bounds,
	Triple *label_density,
	int primary_axis_number, int label_bounds_component,
	ZnReal major_cross_min, ZnReal major_cross_max,
	ZnReal minor_cross_min, ZnReal minor_cross_max, FE_value minor_grid_size,
	int minor_grids_per_major, FE_value min_minor_grid, FE_value min_major_grid,
	struct Graphical_material *material, struct Graphical_material *secondary_material,
	struct Cmiss_graphics_font *font, Render_graphics *renderer)
/*******************************************************************************
LAST MODIFIED : 22 November 2005

DESCRIPTION :
Renders the label_bounds as lines and labels.  
==============================================================================*/
{
	const char *name = "axes_ticks_temporary";
	char **label_strings;
	FE_value grid_scale;
	int first, j, label_bounds_offset, last, number_of_major_lines, 
		number_of_minor_lines, number_of_labels, number_of_ticks, return_code;
	ZnReal axis_position, fabs_length, length, log_scale;
	struct GT_object *graphics_object;
	struct GT_pointset *label_set;
	struct GT_polyline *polyline;
	Triple *label_string_locations, *label_vertex, *major_linepoints, *major_vertex, 
		*minor_linepoints, *minor_vertex;

	ENTER(draw_glyph_axes_general);
	if ((label_bounds_dimension > 0) && (label_bounds_components > 0) && label_bounds)
	{
		return_code=1;

		switch(primary_axis_number)
		{
			case 0:
			default:
			{
				label_bounds_offset = label_bounds_components;
			} break;
			case 1:
			{
				label_bounds_offset = 2 * label_bounds_components;
			} break;
			case 2:
			{
				label_bounds_offset = 4 * label_bounds_components;
			} break;
		}

		length = label_bounds[label_bounds_offset + label_bounds_component] -
			label_bounds[label_bounds_component];
		fabs_length = fabs(length);

		if (fabs_length > 1e-7)
		{
			minor_grid_size *= fabs_length;
			log_scale = ceil(-0.5 + log10(minor_grid_size * 2.0));
			minor_grid_size = pow(10, log_scale) / 2.0;
			if (label_density)
			{
				grid_scale = 0.01 * (*label_density)[0] / fabs_length;				
			}
			else
			{
				grid_scale = (*coordinate_scaling)[0] / fabs_length;
			}
			
			tick_mark_get_grid_spacing(&minor_grid_size,
				&minor_grids_per_major, grid_scale, min_minor_grid, min_major_grid);
			
			first=(int)ceil(label_bounds[label_bounds_component] / minor_grid_size);
			last=(int)floor(label_bounds[label_bounds_offset + label_bounds_component]
				/ minor_grid_size);
		}
		else
		{
			length = 1e-7f;
			if (label_density)
			{
				grid_scale = 0.01 * (*label_density)[0];				
			}
			else
			{
				grid_scale = 0.01 * (*coordinate_scaling)[0];
			}

			minor_grids_per_major = 5;
  			tick_mark_get_grid_spacing(&minor_grid_size,
				&minor_grids_per_major, grid_scale, min_minor_grid, min_major_grid);

			first = (int)ceil(label_bounds[label_bounds_component] / minor_grid_size);
			last = first;
		}								
		if (first > last)
		{
			j = last;
			last = first;
			first = j;
		}
		number_of_ticks = last - first + 1;

		number_of_minor_lines = 0;
		number_of_major_lines = 0;
		number_of_labels = 0;
		/* This is too much memory but saves us working it out in advance */
		if (ALLOCATE(major_linepoints, Triple, 2 * number_of_ticks) && 
			ALLOCATE(minor_linepoints, Triple, 2 * number_of_ticks) && 
			ALLOCATE(label_string_locations, Triple, number_of_ticks) &&
			ALLOCATE(label_strings, char *, number_of_ticks))
		{
			minor_vertex = minor_linepoints;
			major_vertex = major_linepoints;
			label_vertex = label_string_locations;
			for (j=first;j<=last;j++)
			{
				if (number_of_ticks > 1)
				{
					axis_position = ((ZnReal)j * minor_grid_size - label_bounds[label_bounds_component]) / length;
				}
				else
				{
					axis_position = 0.0;
				}
				if (0==(j % minor_grids_per_major))
				{
					/* Major */
					switch (primary_axis_number)
					{
						case 0:
						default:
						{
							(*major_vertex)[0] = axis_position;
							(*major_vertex)[1] = major_cross_min;
							(*major_vertex)[2] = 0.0;
							major_vertex++;
							(*major_vertex)[0] = axis_position;
							(*major_vertex)[1] = major_cross_max;
							(*major_vertex)[2] = 0.0;
							major_vertex++;
							(*label_vertex)[0] = axis_position;
							(*label_vertex)[1] = 0.0;
							(*label_vertex)[2] = 0.0;
							label_vertex++;
						} break;
						case 1:
						{
							(*major_vertex)[0] = major_cross_min;
							(*major_vertex)[1] = axis_position;
							(*major_vertex)[2] = 0.0;
							major_vertex++;
							(*major_vertex)[0] = major_cross_max;
							(*major_vertex)[1] = axis_position;
							(*major_vertex)[2] = 0.0;
							major_vertex++;
							(*label_vertex)[0] = 0.0;
							(*label_vertex)[1] = axis_position;
							(*label_vertex)[2] = 0.0;
							label_vertex++;
						} break;
						case 2:
						{
							(*major_vertex)[0] = major_cross_min;
							(*major_vertex)[1] = 0.0;
							(*major_vertex)[2] = axis_position;
							major_vertex++;
							(*major_vertex)[0] = major_cross_max;
							(*major_vertex)[1] = 0.0;
							(*major_vertex)[2] = axis_position;
							major_vertex++;
							(*label_vertex)[0] = 0.0;
							(*label_vertex)[1] = 0.0;
							(*label_vertex)[2] = axis_position;
							label_vertex++;
						} break;
					}
					number_of_major_lines++;

					if (ALLOCATE(label_strings[number_of_labels], char, 50))
					{
						sprintf(label_strings[number_of_labels], "%1g",
							(ZnReal)j * minor_grid_size);
					}
					number_of_labels++;
				}
				else
				{
					/* Minor */
					switch (primary_axis_number)
					{
						case 0:
						default:
						{
							(*minor_vertex)[0] = axis_position;
							(*minor_vertex)[1] = minor_cross_min;
							(*minor_vertex)[2] = 0.0;
							minor_vertex++;
							(*minor_vertex)[0] = axis_position;
							(*minor_vertex)[1] = minor_cross_max;
							(*minor_vertex)[2] = 0.0;
							minor_vertex++;
						} break;
						case 1:
						{
							(*minor_vertex)[0] = minor_cross_min;
							(*minor_vertex)[1] = axis_position;
							(*minor_vertex)[2] = 0.0;
							minor_vertex++;
							(*minor_vertex)[0] = minor_cross_max;
							(*minor_vertex)[1] = axis_position;
							(*minor_vertex)[2] = 0.0;
							minor_vertex++;
						} break;
						case 2:
						{
							(*minor_vertex)[0] = minor_cross_min;
							(*minor_vertex)[1] = 0.0;
							(*minor_vertex)[2] = axis_position;
							minor_vertex++;
							(*minor_vertex)[0] = minor_cross_max;
							(*minor_vertex)[1] = 0.0;
							(*minor_vertex)[2] = axis_position;
							minor_vertex++;
						} break;
					}
					number_of_minor_lines++;
				}
			}
			polyline=CREATE(GT_polyline)(g_PLAIN_DISCONTINUOUS,/*line_width=default*/0,
				number_of_major_lines, major_linepoints,/*normalpoints*/(Triple *)NULL,g_NO_DATA,(GLfloat *)NULL);
			if (polyline)
			{
				graphics_object=CREATE(GT_object)(name,g_POLYLINE,
					/*use the default material, must be before graphics which specify a material*/
					(struct Graphical_material *)NULL);
				if (graphics_object)
				{
					if (GT_OBJECT_ADD(GT_polyline)(graphics_object,/*time*/0.0,polyline))
					{
						renderer->Graphics_object_render_immediate(graphics_object);
						DESTROY(GT_object)(&graphics_object);
					}
					else
					{
						DESTROY(GT_polyline)(&polyline);
						return_code = 0;
					}
				}
			}
			else
			{
				DEALLOCATE(major_linepoints);
			}
			label_set = CREATE(GT_pointset)(number_of_labels, label_string_locations,
				label_strings, g_NO_MARKER, /*marker_size*/0, /*n_data_components*/0,
				(GLfloat *)NULL, /*names*/(int *)NULL, font);
			if (label_set)
			{
				graphics_object=CREATE(GT_object)(name, g_POINTSET, material);
				if (graphics_object)
				{
					if (GT_OBJECT_ADD(GT_pointset)(graphics_object,/*time*/0.0,label_set))
					{
						renderer->Graphics_object_render_immediate(graphics_object);
						DESTROY(GT_object)(&graphics_object);
					}
					else
					{
						DESTROY(GT_pointset)(&label_set);
						return_code = 0;
					}
				}
			}
			else
			{
				DEALLOCATE(label_string_locations);
				for (j = 0 ; j < number_of_labels ; j++)
				{
					DEALLOCATE(label_strings[j]);
				}
				DEALLOCATE(label_strings);
			}
			polyline=CREATE(GT_polyline)(g_PLAIN_DISCONTINUOUS,/*line_width=default*/0,
				number_of_minor_lines, minor_linepoints,/*normalpoints*/(Triple *)NULL,g_NO_DATA,(GLfloat *)NULL);
			if (polyline)
			{
				graphics_object=CREATE(GT_object)(name,g_POLYLINE, secondary_material);
				if (graphics_object)
				{
					if (GT_OBJECT_ADD(GT_polyline)(graphics_object,/*time*/0.0,polyline))
					{
						renderer->Graphics_object_render_immediate(graphics_object);
						DESTROY(GT_object)(&graphics_object);
					}
					else
					{
						DESTROY(GT_polyline)(&polyline);
						return_code = 0;
					}
				}
			}
			else
			{
				DEALLOCATE(minor_linepoints);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"draw_glyph_axes_general.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* draw_glyph_axes_general */

static int draw_glyph_axes_ticks(Triple *coordinate_scaling, 
	int label_bounds_dimension, int label_bounds_components, ZnReal *label_bounds,
	Triple *label_density,
	struct Graphical_material *material, struct Graphical_material *secondary_material,
	struct Cmiss_graphics_font *font, Render_graphics *renderer)
/*******************************************************************************
LAST MODIFIED : 22 November 2005

DESCRIPTION :
Renders the label_bounds as a grid in the first two axis directions.  
An assumption is made that the label values have some alignment with the axis 
so only the first component of the label_bounds is drawn.
==============================================================================*/
{
	int return_code;

	ENTER(draw_glyph_axes_ticks);
	if ((label_bounds_dimension > 0) && (label_bounds_components > 0)
		&& label_bounds)
	{
		return_code = draw_glyph_axes_general(coordinate_scaling, 
			label_bounds_dimension, label_bounds_components, label_bounds,
			label_density,
			/*primary_axis_number*/0, /*label_bounds_component*/0,
			/*major_cross_min*/-0.05f, /*major_cross_max*/0.05f,
			/*minor_cross_min*/-0.01f, /*minor_cross_max*/0.01f,
			/*minor_grid_size*/0.01, /*minor_grids_per_major*/5,
			/*min_minor_grid*/0.01, /*min_major_grid*/0.1,
			material, secondary_material, font, renderer);
	}
	else
	{
		display_message(ERROR_MESSAGE,"draw_glyph_axes_ticks.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* draw_glyph_axes_ticks */

static int draw_glyph_grid_lines(Triple *coordinate_scaling, 
	int label_bounds_dimension, int label_bounds_components, ZnReal *label_bounds,
	Triple *label_density,
	struct Graphical_material *material, struct Graphical_material *secondary_material,
	struct Cmiss_graphics_font *font, Render_graphics *renderer)
/*******************************************************************************
LAST MODIFIED : 22 November 2005

DESCRIPTION :
Renders the label_bounds as a grid in the first two axis directions.  
It is assumed that the label values have some alignment with the axes so only
the first component of the label_bounds is drawn along the first axis and the 
second component on the second axis.
==============================================================================*/
{
	int return_code;

	ENTER(draw_glyph_axes_ticks);
	if ((label_bounds_dimension > 0) && (label_bounds_components > 0)
		&& label_bounds)
	{
		return_code = draw_glyph_axes_general(coordinate_scaling, 
			label_bounds_dimension, label_bounds_components, label_bounds,
			label_density,
			/*primary_axis_number*/0, /*label_bounds_component*/0,
			/*major_cross_min*/-0.01f, /*major_cross_max*/1.01f,
			/*minor_cross_min*/-0.01f, /*minor_cross_max*/1.01f,
			/*minor_grid_size*/0.01, /*minor_grids_per_major*/2,
			/*min_minor_grid*/0.1, /*min_major_grid*/0.2,
			material, secondary_material, font, renderer);
		return_code = draw_glyph_axes_general(coordinate_scaling, 
			label_bounds_dimension, label_bounds_components, label_bounds,
			label_density,
			/*primary_axis_number*/1, /*label_bounds_component*/1,
			/*major_cross_min*/-0.01f, /*major_cross_max*/1.01f,
			/*minor_cross_min*/-0.01f, /*minor_cross_max*/1.01f,
			/*minor_grid_size*/0.01, /*minor_grids_per_major*/2,
			/*min_minor_grid*/0.1, /*min_major_grid*/0.2,
			material, secondary_material, font, renderer);
	}
	else
	{
		display_message(ERROR_MESSAGE,"draw_glyph_axes_ticks.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* draw_glyph_axes_ticks */

/*
Global functions
----------------
*/

struct GT_object *make_glyph_arrow_line(const char *name,ZnReal head_length,
	ZnReal half_head_width)
/*******************************************************************************
LAST MODIFIED : 3 August 1999

DESCRIPTION :
Creates a graphics object named <name> consisting of a line from <0,0,0> to
<1,0,0> with 4 arrow head ticks <head_length> long and <half_head_width> out
from the shaft.
==============================================================================*/
{
	int j;
	struct GT_object *glyph;
	struct GT_polyline *polyline;
	Triple *points,*vertex;

	ENTER(make_glyph_arrow_line);
	if (name)
	{
		polyline=(struct GT_polyline *)NULL;
		if (ALLOCATE(points,Triple,10))
		{
			vertex=points;
			/* most coordinates are 0.0, so clear them all to that */
			for (j=0;j<10;j++)
			{
				(*vertex)[0]=0.0;
				(*vertex)[1]=0.0;
				(*vertex)[2]=0.0;
				vertex++;
			}
			/* x-axis */
			points[ 1][0]=1.0;
			points[ 2][0]=1.0;
			points[ 3][0]=1.0-head_length;
			points[ 3][1]=half_head_width;
			points[ 4][0]=1.0;
			points[ 5][0]=1.0-head_length;
			points[ 5][2]=half_head_width;
			points[ 6][0]=1.0;
			points[ 7][0]=1.0-head_length;
			points[ 7][1]=-half_head_width;
			points[ 8][0]=1.0;
			points[ 9][0]=1.0-head_length;
			points[ 9][2]=-half_head_width;
			if (!(polyline=CREATE(GT_polyline)(g_PLAIN_DISCONTINUOUS,/*line_width=default*/0,
				5,points,/*normalpoints*/(Triple *)NULL,g_NO_DATA,(GLfloat *)NULL)))
			{
				DEALLOCATE(points);
			}
		}
		if (polyline)
		{
			glyph=CREATE(GT_object)(name,g_POLYLINE,(struct Graphical_material *)NULL);
			if (glyph)
			{
				if (!GT_OBJECT_ADD(GT_polyline)(glyph,/*time*/0.0,polyline))
				{
					DESTROY(GT_object)(&glyph);
				}
			}
			if (!glyph)
			{
				DESTROY(GT_polyline)(&polyline);
			}
		}
		else
		{
			glyph=(struct GT_object *)NULL;
		}
		if (!glyph)
		{
			display_message(ERROR_MESSAGE,
				"make_glyph_arrow_line.  Error creating glyph");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"make_glyph_arrow_line.  Invalid argument(s)");
		glyph=(struct GT_object *)NULL;
	}
	LEAVE;

	return (glyph);
} /* make_glyph_arrow_line */

struct GT_object *make_glyph_arrow_solid(const char *name, int primary_axis,
	int number_of_segments_around,ZnReal shaft_length,ZnReal shaft_radius,
	ZnReal cone_radius)
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Creates a graphics object named <name> resembling an arrow made from a cone on
a cylinder. The base of the arrow is at (0,0,0) while its head lies at (1,0,0).
The radius of the cone is <cone_radius>. The cylinder is <shaft_length> long
with its radius given by <shaft_radius>. The ends of the arrow and the cone
are both closed.  Primary axis is either 1,2 or 3 and indicates the direction
the arrow points in.
==============================================================================*/
{
	ZnReal r1 = 0.0, r2 = 0.0, x1 = 0.0, x2 = 0.0;
	int i;
	struct GT_object *glyph;
	struct GT_surface *surface;
	Triple *points = NULL, *normalpoints = NULL;

	ENTER(make_glyph_arrow_solid);
	if (name&&(2<number_of_segments_around)&&(0<shaft_radius)&&(1>shaft_radius)&&
		(0<shaft_length)&&(1>shaft_length))
	{
		glyph=CREATE(GT_object)(name,g_SURFACE,	(struct Graphical_material *)NULL);
		if (glyph)
		{
			for (i=0;(i<4)&&glyph;i++)
			{
				if (ALLOCATE(points,Triple,2*(number_of_segments_around+1))&&
					ALLOCATE(normalpoints,Triple,2*(number_of_segments_around+1)))
				{
					switch (i)
					{
						case 0:
						{
							/* base of shaft */
							x1=0.0;
							r1=0.0;
							x2=0.0;
							r2=shaft_radius;
						} break;
						case 1:
						{
							/* shaft */
							x1=0.0;
							r1=shaft_radius;
							x2=shaft_length;
							r2=shaft_radius;
						} break;
						case 2:
						{
							/* base of head */
							x1=shaft_length;
							r1=shaft_radius;
							x2=shaft_length;
							r2=cone_radius;
						} break;
						case 3:
						{
							/* head */
							x1=shaft_length;
							r1=cone_radius;
							x2=1.0;
							r2=0.0;
						} break;
					}
					if (!construct_tube(number_of_segments_around,x1,r1,x2,r2,0.0,0.0,
						primary_axis,points,normalpoints))
					{
						DEALLOCATE(points);
						DEALLOCATE(normalpoints);
					}
				}
				if (points&&(surface=CREATE(GT_surface)(g_SHADED_TEXMAP,
					CMISS_GRAPHICS_RENDER_TYPE_SHADED,g_QUADRILATERAL,2,number_of_segments_around+1,
					points,normalpoints,/*tangentpoints*/(Triple *)NULL,
					/*texturepoints*/(Triple *)NULL,g_NO_DATA,(GLfloat *)NULL)))
				{
					if (!GT_OBJECT_ADD(GT_surface)(glyph,/*time*/0.0,surface))
					{
						DESTROY(GT_surface)(&surface);
						DESTROY(GT_object)(&glyph);
					}
				}
				else
				{
					DEALLOCATE(points);
					DEALLOCATE(normalpoints);
					DESTROY(GT_object)(&glyph);
				}
			}
		}
		if (!glyph)
		{
			display_message(ERROR_MESSAGE,
				"make_glyph_arrow_solid.  Error creating glyph");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"make_glyph_arrow_solid.  Invalid argument(s)");
		glyph=(struct GT_object *)NULL;
	}
	LEAVE;

	return (glyph);
} /* make_glyph_arrow_solid */

struct GT_object *make_glyph_axes(const char *name, int make_solid, ZnReal head_length,
	ZnReal half_head_width,const char **labels, ZnReal label_offset,
	struct Cmiss_graphics_font *font)
/*******************************************************************************
LAST MODIFIED : 18 November 2005

DESCRIPTION :
Creates a graphics object named <name> consisting of three axis arrows heading
from <0,0,0> to 1 in each of their directions. The arrows are made up of lines,
with a 4-way arrow head so it looks normal from the other two axes. If <labels>
is specified then it is assumed to point to an array of 3 strings which will
be used to label each arrow and are attached to it so
that the two objects are displayed and destroyed together. The labels are
located on the respective axes <label_offset> past 1.0.
The length and width of the arrow heads are specified by the final parameters.
==============================================================================*/
{
	char *glyph_name,**text;
	int j;
	struct Colour colour;
	struct Graphical_material *material;
	struct GT_object *glyph = NULL,*arrow2,*arrow3,*labels_object,*last_object;
	struct GT_pointset *pointset;
	struct GT_polyline *polyline;
	Triple *points,*vertex;

	ENTER(make_glyph_axes);
	if (name)
	{
		last_object = (struct GT_object *)NULL;
		if (make_solid)
		{
			if (ALLOCATE(glyph_name, char, strlen(name) + 8))
			{
				glyph = make_glyph_arrow_solid(name, /*primary_axis*/1,
					/*number_of_segments_around*/12, /*shaft_length*/2.f/3.f,
					/*shaft_radius*/1.f/20.f, /*cone_radius*/1.f/8.f);
				material = CREATE(Graphical_material)("red");
				colour.red = 1;
				colour.green = 0;
				colour.blue = 0;
				Graphical_material_set_diffuse(material, &colour);
				set_GT_object_default_material(glyph, material);
				last_object = glyph;

				sprintf(glyph_name, "%s_arrow2", name);
				arrow2 = make_glyph_arrow_solid(glyph_name, /*primary_axis*/2,
					/*number_of_segments_around*/12, /*shaft_length*/2.f/3.f,
					/*shaft_radius*/1.f/20.f, /*cone_radius*/1.f/8.f);
				material = CREATE(Graphical_material)("green");
				colour.red = 0;
				colour.green = 1;
				colour.blue = 0;
				Graphical_material_set_diffuse(material, &colour);
				set_GT_object_default_material(arrow2, material);
				GT_object_set_next_object(last_object, arrow2);
				last_object = arrow2;

				sprintf(glyph_name, "%s_arrow3", name);
				arrow3 = make_glyph_arrow_solid(glyph_name, /*primary_axis*/3,
					/*number_of_segments_around*/12, /*shaft_length*/2.f/3.f,
					/*shaft_radius*/1.f/20.f, /*cone_radius*/1.f/8.f);
				material = CREATE(Graphical_material)("blue");
				colour.red = 0;
				colour.green = 0;
				colour.blue = 1;
				Graphical_material_set_diffuse(material, &colour);
				set_GT_object_default_material(arrow3, material);
				GT_object_set_next_object(last_object, arrow3);
				last_object = arrow3;
				
				DEALLOCATE(glyph_name);
			}
		}
		else
		{
			polyline=(struct GT_polyline *)NULL;
			if (ALLOCATE(points,Triple,30))
			{
				vertex=points;
				/* most coordinates are 0.0, so clear them all to that */
				for (j=0;j<30;j++)
				{
					(*vertex)[0]=0.0;
					(*vertex)[1]=0.0;
					(*vertex)[2]=0.0;
					vertex++;
				}
				/* x-axis */
				points[ 1][0]=1.0;
				points[ 2][0]=1.0;
				points[ 3][0]=1.0-head_length;
				points[ 3][1]=half_head_width;
				points[ 4][0]=1.0;
				points[ 5][0]=1.0-head_length;
				points[ 5][2]=half_head_width;
				points[ 6][0]=1.0;
				points[ 7][0]=1.0-head_length;
				points[ 7][1]=-half_head_width;
				points[ 8][0]=1.0;
				points[ 9][0]=1.0-head_length;
				points[ 9][2]=-half_head_width;
				/* y-axis */
				points[11][1]=1.0;
				points[12][1]=1.0;
				points[13][1]=1.0-head_length;
				points[13][2]=half_head_width;
				points[14][1]=1.0;
				points[15][1]=1.0-head_length;
				points[15][0]=half_head_width;
				points[16][1]=1.0;
				points[17][1]=1.0-head_length;
				points[17][2]=-half_head_width;
				points[18][1]=1.0;
				points[19][1]=1.0-head_length;
				points[19][0]=-half_head_width;
				/* z-axis */
				points[21][2]=1.0;
				points[22][2]=1.0;
				points[23][2]=1.0-head_length;
				points[23][0]=half_head_width;
				points[24][2]=1.0;
				points[25][2]=1.0-head_length;
				points[25][1]=half_head_width;
				points[26][2]=1.0;
				points[27][2]=1.0-head_length;
				points[27][0]=-half_head_width;
				points[28][2]=1.0;
				points[29][2]=1.0-head_length;
				points[29][1]=-half_head_width;
				polyline=CREATE(GT_polyline)(g_PLAIN_DISCONTINUOUS,/*line_width=default*/0,
					15,points,/*normalpoints*/(Triple *)NULL,g_NO_DATA,(GLfloat *)NULL);
				if (polyline)
				{
					glyph=CREATE(GT_object)(name,g_POLYLINE, (struct Graphical_material *)NULL);
					if (glyph)
					{
						GT_OBJECT_ADD(GT_polyline)(glyph,/*time*/0.0,polyline);
						last_object = glyph;
					}
				}
				else
				{
					DEALLOCATE(points);
				}
			}
		}
		if (glyph && labels)
		{
			pointset=(struct GT_pointset *)NULL;
			if (ALLOCATE(points,Triple,3)&&
				ALLOCATE(text,char *,3)&&
				ALLOCATE(text[0],char,strlen(labels[0]) + 1)&&
				ALLOCATE(text[1],char,strlen(labels[1]) + 1)&&
				ALLOCATE(text[2],char,strlen(labels[2]) + 1)&&
				ALLOCATE(glyph_name,char,strlen(name)+8))
			{
				sprintf(glyph_name,"%s_labels",name);
				points[0][0]=1.0+label_offset;
				points[0][1]=0.0;
				points[0][2]=0.0;
				strcpy(text[0],labels[0]);
				points[1][0]=0.0;
				points[1][1]=1.0+label_offset;
				points[1][2]=0.0;
				strcpy(text[1],labels[1]);
				points[2][0]=0.0;
				points[2][1]=0.0;
				points[2][2]=1.0+label_offset;
				strcpy(text[2],labels[2]);
				pointset=CREATE(GT_pointset)(3,points,text,g_NO_MARKER,0.0,
					g_NO_DATA,(GLfloat *)NULL,(int *)NULL, font);
				if (pointset)
				{
					labels_object=CREATE(GT_object)(glyph_name,g_POINTSET,
						(struct Graphical_material *)NULL);
					if (labels_object)
					{
						GT_OBJECT_ADD(GT_pointset)(labels_object,/*time*/0.0,pointset);
						GT_object_set_next_object(last_object, labels_object);
						last_object = labels_object;
					}
				}
				else
				{
					DEALLOCATE(text[0]);
					DEALLOCATE(text[1]);
					DEALLOCATE(text[2]);
					DEALLOCATE(text);
					DEALLOCATE(points);
				}
				DEALLOCATE(glyph_name);
			}
		}
		if (!glyph)
		{
			display_message(ERROR_MESSAGE,"make_glyph_axes.  Error creating glyph");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"make_glyph_axes.  Invalid argument(s)");
		glyph=(struct GT_object *)NULL;
	}
	LEAVE;

	return (glyph);
} /* make_glyph_axes */

struct GT_object *make_glyph_cone(const char *name,int number_of_segments_around)
/*******************************************************************************
LAST MODIFIED : 19 October 1998

DESCRIPTION :
Creates a graphics object named <name> resembling a cone with the given
<number_of_segments_around>. The base of the cone is at <0,0,0> while its head
lies at <1,0,0>. The radius of the cone is 0.5 at its base.
==============================================================================*/
{
	struct GT_object *glyph;
	struct GT_surface *surface;
	Triple *points,*normalpoints;

	ENTER(make_glyph_cone);
	if (name&&(2<number_of_segments_around))
	{
		surface=(struct GT_surface *)NULL;
		if (ALLOCATE(points,Triple,2*(number_of_segments_around+1))&&
			ALLOCATE(normalpoints,Triple,2*(number_of_segments_around+1)))
		{
			construct_tube(number_of_segments_around, 0.0, 0.5, 1.0, 0.0, 0.0, 0.0, 1,
				points,normalpoints);
			if (!(surface=CREATE(GT_surface)(g_SHADED_TEXMAP,CMISS_GRAPHICS_RENDER_TYPE_SHADED,
				g_QUADRILATERAL,2,number_of_segments_around+1,points,normalpoints,
				/*tangentpoints*/(Triple *)NULL,/*texturepoints*/(Triple *)NULL,
				g_NO_DATA,(GLfloat *)NULL)))
			{
				DEALLOCATE(points);
				DEALLOCATE(normalpoints);
			}
		}
		if (surface)
		{
			glyph=CREATE(GT_object)(name,g_SURFACE,	(struct Graphical_material *)NULL);
			if (glyph)
			{
				if (!GT_OBJECT_ADD(GT_surface)(glyph,/*time*/0.0,surface))
				{
					DESTROY(GT_object)(&glyph);
					DESTROY(GT_surface)(&surface);
				}
			}
		}
		else
		{
			glyph=(struct GT_object *)NULL;
		}
		if (!glyph)
		{
			display_message(ERROR_MESSAGE,"make_glyph_cone.  Error creating glyph");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"make_glyph_cone.  Invalid argument(s)");
		glyph=(struct GT_object *)NULL;
	}
	LEAVE;

	return (glyph);
} /* make_glyph_cone */

struct GT_object *make_glyph_cone_solid(const char *name,int number_of_segments_around)
/*******************************************************************************
LAST MODIFIED : 20 January 2004

DESCRIPTION :
Creates a graphics object named <name> resembling a cone with the given
<number_of_segments_around>. The base of the cone is at <0,0,0> while its head
lies at <1,0,0>. The radius of the cone is 0.5 at its base.  This cone has a
solid base.
==============================================================================*/
{
	struct GT_object *glyph;
	struct GT_surface *surface;
	Triple *points,*normalpoints;

	ENTER(make_glyph_cone_solid);
	if (name&&(2<number_of_segments_around))
	{
		glyph=CREATE(GT_object)(name,g_SURFACE,	(struct Graphical_material *)NULL);
		if (glyph)
		{
			surface=(struct GT_surface *)NULL;
			if (ALLOCATE(points,Triple,2*(number_of_segments_around+1))&&
				ALLOCATE(normalpoints,Triple,2*(number_of_segments_around+1)))
			{
				construct_tube(number_of_segments_around, 0.0, 0.5, 1.0, 0.0, 0.0, 0.0, 1,
					points,normalpoints);
				if (!(surface=CREATE(GT_surface)(g_SHADED_TEXMAP,CMISS_GRAPHICS_RENDER_TYPE_SHADED,
							g_QUADRILATERAL,2,number_of_segments_around+1,points,normalpoints,
							/*tangentpoints*/(Triple *)NULL,/*texturepoints*/(Triple *)NULL,
							g_NO_DATA,(GLfloat *)NULL)))
				{
					DEALLOCATE(points);
					DEALLOCATE(normalpoints);
				}
			}
			if (surface)
			{
				if (!GT_OBJECT_ADD(GT_surface)(glyph,/*time*/0.0,surface))
				{
					DESTROY(GT_object)(&glyph);
					DESTROY(GT_surface)(&surface);
				}
			
			}
			else
			{
				glyph=(struct GT_object *)NULL;
			}
			if (ALLOCATE(points,Triple,2*(number_of_segments_around+1))&&
				ALLOCATE(normalpoints,Triple,2*(number_of_segments_around+1)))
			{
				construct_tube(number_of_segments_around, 0.0, 0.0, 0.0, 0.5, 0.0, 0.0, 1,
					points,normalpoints);
				if (!(surface=CREATE(GT_surface)(g_SHADED_TEXMAP,CMISS_GRAPHICS_RENDER_TYPE_SHADED,
							g_QUADRILATERAL,2,number_of_segments_around+1,points,normalpoints,
							/*tangentpoints*/(Triple *)NULL,/*texturepoints*/(Triple *)NULL,
							g_NO_DATA,(GLfloat *)NULL)))
				{
					DEALLOCATE(points);
					DEALLOCATE(normalpoints);
				}
			}
			if (surface)
			{
				if (!GT_OBJECT_ADD(GT_surface)(glyph,/*time*/0.0,surface))
				{
					DESTROY(GT_object)(&glyph);
					DESTROY(GT_surface)(&surface);
				}
			
			}
			else
			{
				DESTROY(GT_object)(&glyph);
			}
		}
		if (!glyph)
		{
			display_message(ERROR_MESSAGE,"make_glyph_cone_solid.  Error creating glyph");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"make_glyph_cone_solid.  Invalid argument(s)");
		glyph=(struct GT_object *)NULL;
	}
	LEAVE;

	return (glyph);
} /* make_glyph_cone_solid */

struct GT_object *make_glyph_cross(const char *name)
/*******************************************************************************
LAST MODIFIED : 16 July 1999

DESCRIPTION :
Creates a graphics object named <name> consisting of a 3 lines:
from <-0.5,0,0> to <+0.5,0,0>
from <0,-0.5,0> to <0,+0.5,0>
from <0,0,-0.5> to <0,0,+0.5>
==============================================================================*/
{
	struct GT_object *glyph;
	struct GT_polyline *polyline;
	Triple *points;

	ENTER(make_glyph_cross);
	if (name)
	{
		polyline=(struct GT_polyline *)NULL;
		if (ALLOCATE(points,Triple,6))
		{
			/* x-line */
			points[0][0]=-0.5;
			points[0][1]=0.0;
			points[0][2]=0.0;
			points[1][0]=+0.5;
			points[1][1]=0.0;
			points[1][2]=0.0;
			/* y-line */
			points[2][0]=0.0;
			points[2][1]=-0.5;
			points[2][2]=0.0;
			points[3][0]=0.0;
			points[3][1]=+0.5;
			points[3][2]=0.0;
			/* z-line */
			points[4][0]=0.0;
			points[4][1]=0.0;
			points[4][2]=-0.5;
			points[5][0]=0.0;
			points[5][1]=0.0;
			points[5][2]=+0.5;
			if (!(polyline=CREATE(GT_polyline)(g_PLAIN_DISCONTINUOUS,/*line_width=default*/0,
				3,points,/*normalpoints*/(Triple *)NULL,g_NO_DATA,(GLfloat *)NULL)))
			{
				DEALLOCATE(points);
			}
		}
		if (polyline)
		{
			glyph=CREATE(GT_object)(name,g_POLYLINE, (struct Graphical_material *)NULL);
			if (glyph)
			{
				if (!GT_OBJECT_ADD(GT_polyline)(glyph,/*time*/0.0,polyline))
				{
					DESTROY(GT_object)(&glyph);
				}
			}
			if (!glyph)
			{
				DESTROY(GT_polyline)(&polyline);
			}
		}
		else
		{
			glyph=(struct GT_object *)NULL;
		}
		if (!glyph)
		{
			display_message(ERROR_MESSAGE,"make_glyph_cross.  Error creating glyph");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"make_glyph_cross.  Invalid argument(s)");
		glyph=(struct GT_object *)NULL;
	}
	LEAVE;

	return (glyph);
} /* make_glyph_cross */

struct GT_object *make_glyph_cube_solid(const char *name)
/*******************************************************************************
LAST MODIFIED : 20 November 2000

DESCRIPTION :
Creates a graphics object named <name> consisting of a unit-sized GT_surface
cube centred at <0,0,0>.
==============================================================================*/
{
	ZnReal factor;
	int a, b, c, i;
	struct GT_object *glyph;
	struct GT_surface *surface;
	Triple *point, *points, *normalpoint, *normalpoints;

	ENTER(make_glyph_cube_solid);
	if (name)
	{
		surface=(struct GT_surface *)NULL;
		if (ALLOCATE(points,Triple,24)&&
			ALLOCATE(normalpoints,Triple,24))
		{
			point = points;
			normalpoint = normalpoints;
			/* all coordinates are +0.5 or -0.5, so clear them all to former */
			for (i = 0; i < 6; i++)
			{
				a = i / 2;
				if ((2*a) == i)
				{
					factor = -1.0;
				}
				else
				{
					factor = 1.0;
				}
				b = (a + 1) % 3;
				c = (a + 2) % 3;
				/* vertices */
				(*point)[a] = 0.5*factor;
				(*point)[b] = 0.5*factor;
				(*point)[c] = 0.5;
				point++;
				(*point)[a] = 0.5*factor;
				(*point)[b] = -0.5*factor;
				(*point)[c] = 0.5;
				point++;
				(*point)[a] = 0.5*factor;
				(*point)[b] = -0.5*factor;
				(*point)[c] = -0.5;
				point++;
				(*point)[a] = 0.5*factor;
				(*point)[b] = 0.5*factor;
				(*point)[c] = -0.5;
				point++;
				/* normals */
				(*normalpoint)[a] = factor;
				(*normalpoint)[b] = 0.0;
				(*normalpoint)[c] = 0.0;
				normalpoint++;
				(*normalpoint)[a] = factor;
				(*normalpoint)[b] = 0.0;
				(*normalpoint)[c] = 0.0;
				normalpoint++;
				(*normalpoint)[a] = factor;
				(*normalpoint)[b] = 0.0;
				(*normalpoint)[c] = 0.0;
				normalpoint++;
				(*normalpoint)[a] = factor;
				(*normalpoint)[b] = 0.0;
				(*normalpoint)[c] = 0.0;
				normalpoint++;
			}
			if (!(surface=CREATE(GT_surface)(g_SH_DISCONTINUOUS,CMISS_GRAPHICS_RENDER_TYPE_SHADED,
				g_QUADRILATERAL,6,4,points,normalpoints,/*tangentpoints*/(Triple *)NULL,
				/*texturepoints*/(Triple *)NULL,g_NO_DATA,(GLfloat *)NULL)))
			{
				DEALLOCATE(points);
				DEALLOCATE(normalpoints);
			}
		}
		if (surface)
		{
			glyph=CREATE(GT_object)(name,g_SURFACE,	(struct Graphical_material *)NULL);
			if (glyph)
			{
				if (!GT_OBJECT_ADD(GT_surface)(glyph,/*time*/0.0,surface))
				{
					DESTROY(GT_object)(&glyph);
					DESTROY(GT_surface)(&surface);
				}
			}
		}
		else
		{
			glyph=(struct GT_object *)NULL;
		}
		if (!glyph)
		{
			display_message(ERROR_MESSAGE,
				"make_glyph_cube_solid.  Error creating glyph");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"make_glyph_cube_solid.  Invalid argument(s)");
		glyph=(struct GT_object *)NULL;
	}
	LEAVE;

	return (glyph);
} /* make_glyph_cube_solid */

struct GT_object *make_glyph_cube_wireframe(const char *name)
/*******************************************************************************
LAST MODIFIED : 20 November 2000

DESCRIPTION :
Creates a graphics object named <name> consisting of lines marking a unit-sized
wireframe cube centred at <0,0,0>.
==============================================================================*/
{
	int a, b, c, i;
	struct GT_object *glyph;
	struct GT_polyline *polyline;
	Triple *points, *vertex;

	ENTER(make_glyph_cube_wireframe);
	if (name)
	{
		polyline = (struct GT_polyline *)NULL;
		if (ALLOCATE(points, Triple, 24))
		{
			vertex=points;
			/* all coordinates are +0.5 or -0.5, so clear them all to former */
			for (a = 0; a < 3; a++)
			{
				b = (a + 1) % 3;
				c = (a + 2) % 3;
				for (i = 0; i < 8; i++)
				{
					if (0 == (i % 2))
					{
						(*vertex)[a] = -0.5;
					}
					else
					{
						(*vertex)[a] = 0.5;
					}
					if (2 > (i % 4))
					{
						(*vertex)[b] = -0.5;
					}
					else
					{
						(*vertex)[b] = 0.5;
					}
					if (4 > i)
					{
						(*vertex)[c] = -0.5;
					}
					else
					{
						(*vertex)[c] = 0.5;
					}
					vertex++;
				}
			}
			if (!(polyline=CREATE(GT_polyline)(g_PLAIN_DISCONTINUOUS,/*line_width=default*/0,
				12,points,/*normalpoints*/(Triple *)NULL,g_NO_DATA,(GLfloat *)NULL)))
			{
				DEALLOCATE(points);
			}
		}
		if (polyline)
		{
			glyph=CREATE(GT_object)(name,g_POLYLINE, (struct Graphical_material *)NULL);
			if (glyph)
			{
				if (!GT_OBJECT_ADD(GT_polyline)(glyph,/*time*/0.0,polyline))
				{
					DESTROY(GT_object)(&glyph);
				}
			}
			if (!glyph)
			{
				DESTROY(GT_polyline)(&polyline);
			}
		}
		else
		{
			glyph=(struct GT_object *)NULL;
		}
		if (!glyph)
		{
			display_message(ERROR_MESSAGE,
				"make_glyph_cube_wireframe.  Error creating glyph");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"make_glyph_cube_wireframe.  Invalid argument(s)");
		glyph=(struct GT_object *)NULL;
	}
	LEAVE;

	return (glyph);
} /* make_glyph_cube_wireframe */

struct GT_object *make_glyph_cylinder(const char *name,int number_of_segments_around)
/*******************************************************************************
LAST MODIFIED : 14 July 1999

DESCRIPTION :
Creates a graphics object named <name> resembling a cylinder with the given
<number_of_segments_around>. The cylinder is centred at (0.5,0,0) and its axis
lies in the direction <1,0,0>. It fits into the unit cube spanning from
(0,-0.5,-0.5) to (0,+0.5,+0.5).
==============================================================================*/
{
	struct GT_object *glyph;
	struct GT_surface *surface;
	Triple *points,*normalpoints;

	ENTER(make_glyph_cylinder);
	if (name&&(2<number_of_segments_around))
	{
		surface=(struct GT_surface *)NULL;
		if (ALLOCATE(points,Triple,2*(number_of_segments_around+1))&&
			ALLOCATE(normalpoints,Triple,2*(number_of_segments_around+1)))
		{
			construct_tube(number_of_segments_around,0.0,0.5,1.0,0.5,0.0,0.0,1,
				points,normalpoints);
			if (!(surface=CREATE(GT_surface)(g_SHADED_TEXMAP,CMISS_GRAPHICS_RENDER_TYPE_SHADED,
				g_QUADRILATERAL,2,number_of_segments_around+1,points,normalpoints,
				/*tangentpoints*/(Triple *)NULL,/*texturepoints*/(Triple *)NULL,
				g_NO_DATA,(GLfloat *)NULL)))
			{
				DEALLOCATE(points);
				DEALLOCATE(normalpoints);
			}
		}
		if (surface)
		{
			glyph=CREATE(GT_object)(name,g_SURFACE,	(struct Graphical_material *)NULL);
			if (glyph)
			{
				if (!GT_OBJECT_ADD(GT_surface)(glyph,/*time*/0.0,surface))
				{
					DESTROY(GT_object)(&glyph);
					DESTROY(GT_surface)(&surface);
				}
			}
		}
		else
		{
			glyph=(struct GT_object *)NULL;
		}
		if (!glyph)
		{
			display_message(ERROR_MESSAGE,
				"make_glyph_cylinder.  Error creating glyph");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"make_glyph_cylinder.  Invalid argument(s)");
		glyph=(struct GT_object *)NULL;
	}
	LEAVE;

	return (glyph);
} /* make_glyph_cylinder */

struct GT_object *make_glyph_cylinder_solid(const char *name,int number_of_segments_around)
/*******************************************************************************
LAST MODIFIED : 20 January 2004

DESCRIPTION :
Creates a graphics object named <name> resembling a cylinder with the given
<number_of_segments_around>. The cylinder is centred at (0.5,0,0) and its axis
lies in the direction <1,0,0>. It fits into the unit cube spanning from
(0,-0.5,-0.5) to (0,+0.5,+0.5).  This cylinder has its ends covered over.
==============================================================================*/
{
	struct GT_object *glyph;
	struct GT_surface *surface;
	Triple *points,*normalpoints;

	ENTER(make_glyph_cylinder);
	if (name&&(2<number_of_segments_around))
	{
		glyph=CREATE(GT_object)(name,g_SURFACE, (struct Graphical_material *)NULL);
		if (glyph)
		{
			surface=(struct GT_surface *)NULL;
			if (ALLOCATE(points,Triple,2*(number_of_segments_around+1))&&
				ALLOCATE(normalpoints,Triple,2*(number_of_segments_around+1)))
			{
				construct_tube(number_of_segments_around,0.0,0.5,1.0,0.5,0.0,0.0,1,
					points,normalpoints);
				if (!(surface=CREATE(GT_surface)(g_SHADED_TEXMAP,CMISS_GRAPHICS_RENDER_TYPE_SHADED,
							g_QUADRILATERAL,2,number_of_segments_around+1,points,normalpoints,
							/*tangentpoints*/(Triple *)NULL,/*texturepoints*/(Triple *)NULL,
							g_NO_DATA,(GLfloat *)NULL)))
				{
					DEALLOCATE(points);
					DEALLOCATE(normalpoints);
				}
			}
			if (surface)
			{
				if (!GT_OBJECT_ADD(GT_surface)(glyph,/*time*/0.0,surface))
				{
					DESTROY(GT_object)(&glyph);
					DESTROY(GT_surface)(&surface);
				}
			}
			else
			{
				DESTROY(GT_object)(&glyph);
			}
			/* Cover over the ends */
			if (ALLOCATE(points,Triple,2*(number_of_segments_around+1))&&
				ALLOCATE(normalpoints,Triple,2*(number_of_segments_around+1)))
			{
				construct_tube(number_of_segments_around,0.0,0.0,0.0,0.5,0.0,0.0,1,
					points,normalpoints);
				if (!(surface=CREATE(GT_surface)(g_SHADED_TEXMAP,CMISS_GRAPHICS_RENDER_TYPE_SHADED,
							g_QUADRILATERAL,2,number_of_segments_around+1,points,normalpoints,
							/*tangentpoints*/(Triple *)NULL,/*texturepoints*/(Triple *)NULL,
							g_NO_DATA,(GLfloat *)NULL)))
				{
					DEALLOCATE(points);
					DEALLOCATE(normalpoints);
				}
			}
			if (surface)
			{
				if (!GT_OBJECT_ADD(GT_surface)(glyph,/*time*/0.0,surface))
				{
					DESTROY(GT_object)(&glyph);
					DESTROY(GT_surface)(&surface);
				}
			}
			else
			{
				DESTROY(GT_object)(&glyph);
			}
			if (ALLOCATE(points,Triple,2*(number_of_segments_around+1))&&
				ALLOCATE(normalpoints,Triple,2*(number_of_segments_around+1)))
			{
				construct_tube(number_of_segments_around,1.0,0.0,1.0,0.5,0.0,0.0,1,
					points,normalpoints);
				if (!(surface=CREATE(GT_surface)(g_SHADED_TEXMAP,CMISS_GRAPHICS_RENDER_TYPE_SHADED,
							g_QUADRILATERAL,2,number_of_segments_around+1,points,normalpoints,
							/*tangentpoints*/(Triple *)NULL,/*texturepoints*/(Triple *)NULL,
							g_NO_DATA,(GLfloat *)NULL)))
				{
					DEALLOCATE(points);
					DEALLOCATE(normalpoints);
				}
			}
			if (surface)
			{
				if (!GT_OBJECT_ADD(GT_surface)(glyph,/*time*/0.0,surface))
				{
					DESTROY(GT_object)(&glyph);
					DESTROY(GT_surface)(&surface);
				}
			}
			else
			{
				DESTROY(GT_object)(&glyph);
			}
		}
		if (!glyph)
		{
			display_message(ERROR_MESSAGE,
				"make_glyph_cylinder.  Error creating glyph");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"make_glyph_cylinder.  Invalid argument(s)");
		glyph=(struct GT_object *)NULL;
	}
	LEAVE;

	return (glyph);
} /* make_glyph_cylinder */

struct GT_object *make_glyph_line(const char *name)
/*******************************************************************************
LAST MODIFIED : 14 July 1999

DESCRIPTION :
Creates a graphics object named <name> consisting of a line from <0,0,0> to
<1,0,0>.
==============================================================================*/
{
	struct GT_object *glyph;
	struct GT_polyline *polyline;
	Triple *points;

	ENTER(make_glyph_line);
	if (name)
	{
		polyline=(struct GT_polyline *)NULL;
		if (ALLOCATE(points,Triple,2))
		{
			points[0][0]=0.0;
			points[0][1]=0.0;
			points[0][2]=0.0;
			points[1][0]=1.0;
			points[1][1]=0.0;
			points[1][2]=0.0;
			if (!(polyline=CREATE(GT_polyline)(g_PLAIN_DISCONTINUOUS,/*line_width=default*/0,
				1,points,/*normalpoints*/(Triple *)NULL,g_NO_DATA,(GLfloat *)NULL)))
			{
				DEALLOCATE(points);
			}
		}
		if (polyline)
		{
			glyph=CREATE(GT_object)(name,g_POLYLINE, (struct Graphical_material *)NULL);
			if (glyph)
			{
				if (!GT_OBJECT_ADD(GT_polyline)(glyph,/*time*/0.0,polyline))
				{
					DESTROY(GT_object)(&glyph);
				}
			}
			if (!glyph)
			{
				DESTROY(GT_polyline)(&polyline);
			}
		}
		else
		{
			glyph=(struct GT_object *)NULL;
		}
		if (!glyph)
		{
			display_message(ERROR_MESSAGE,"make_glyph_line.  Error creating glyph");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"make_glyph_line.  Invalid argument(s)");
		glyph=(struct GT_object *)NULL;
	}
	LEAVE;

	return (glyph);
} /* make_glyph_line */

struct GT_object *make_glyph_mirror(const char *name, struct GT_object *mirror_glyph)
/*******************************************************************************
LAST MODIFIED : 16 November 2000

DESCRIPTION :
Makes a glyph with the given <name> that automatically mirrors the given
<mirror_glyph>.
==============================================================================*/
{
	struct GT_object *glyph;

	ENTER(make_glyph_mirror);
	if (name && mirror_glyph)
	{
		/*???temporary. Use dummy GT_object until we have struct Glyph */
		glyph = CREATE(GT_object)(name, g_SURFACE, (struct Graphical_material *)NULL);
		if (glyph)
		{
			GT_object_set_glyph_mirror_mode(glyph, 1);
			GT_object_set_next_object(glyph, mirror_glyph);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "make_glyph_mirror.  Invalid argument(s)");
		glyph = (struct GT_object *)NULL;
	}
	LEAVE;

	return (glyph);
} /* make_glyph_mirror */

struct GT_object *make_glyph_point(const char *name,gtMarkerType marker_type,
	ZnReal marker_size)
/*******************************************************************************
LAST MODIFIED : 1 December 1998

DESCRIPTION :
Creates a graphics object named <name> consisting of a single point at <0,0,0>.
The point will be drawn with the given <marker_type> and <marker_size>.
==============================================================================*/
{
	struct GT_object *glyph;
	struct GT_pointset *pointset;
	Triple *points;

	ENTER(make_glyph_point);
	if (name)
	{
		pointset=(struct GT_pointset *)NULL;
		if (ALLOCATE(points,Triple,1))
		{
			(*points)[0]=0.0;
			(*points)[1]=0.0;
			(*points)[2]=0.0;
			if (!(pointset=CREATE(GT_pointset)(1,points,(char **)NULL,marker_type,
				marker_size,g_NO_DATA,(GLfloat *)NULL,(int *)NULL, (struct Cmiss_graphics_font *)NULL)))
			{
				DEALLOCATE(points);
			}
		}
		if (pointset)
		{
			glyph=CREATE(GT_object)(name,g_POINTSET, (struct Graphical_material *)NULL);
			if (glyph)
			{
				if (!GT_OBJECT_ADD(GT_pointset)(glyph,/*time*/0.0,pointset))
				{
					DESTROY(GT_object)(&glyph);
				}
			}
			if (!glyph)
			{
				DESTROY(GT_pointset)(&pointset);
			}
		}
		else
		{
			glyph=(struct GT_object *)NULL;
		}
		if (!glyph)
		{
			display_message(ERROR_MESSAGE,"make_glyph_point.  Error creating glyph");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"make_glyph_point.  Invalid argument(s)");
		glyph=(struct GT_object *)NULL;
	}
	LEAVE;

	return (glyph);
} /* make_glyph_point */

struct GT_object *make_glyph_sheet(const char *name, int define_texturepoints)
/*******************************************************************************
LAST MODIFIED : 5 May 1999

DESCRIPTION :
Creates a graphics object named <name> resembling a square sheet spanning from
coordinate <-0.5,-0.5,0> to <0.5,0.5,0>.
If define_texturepoints is true then texture coordinates will also be defined
ranging from <0.0,0.0,0.0> to <1.0,1.0,0.0>.
==============================================================================*/
{
	struct GT_object *glyph;
	struct GT_surface *surface;
	Triple *point,*points,*normalpoints, *texturepoints;

	ENTER(make_glyph_sheet);
	if (name)
	{
		surface=(struct GT_surface *)NULL;
		texturepoints = (Triple *)NULL;
		if (ALLOCATE(points,Triple,4)&&
			ALLOCATE(normalpoints,Triple,4)&&
			(!define_texturepoints || ALLOCATE(texturepoints,Triple,4)))
		{
			point = points;
			/* vertices */
			(*point)[0] = -0.5;
			(*point)[1] = -0.5;
			(*point)[2] = 0.0;
			point++;
			(*point)[0] = 0.5;
			(*point)[1] = -0.5;
			(*point)[2] = 0.0;
			point++;
			(*point)[0] = 0.5;
			(*point)[1] = 0.5;
			(*point)[2] = 0.0;
			point++;
			(*point)[0] = -0.5;
			(*point)[1] = 0.5;
			(*point)[2] = 0.0;
			/* normals */
			point = normalpoints;
			(*point)[0] = 0.0;
			(*point)[1] = 0.0;
			(*point)[2] = 1.0;
			point++;
			(*point)[0] = 0.0;
			(*point)[1] = 0.0;
			(*point)[2] = 1.0;
			point++;
			(*point)[0] = 0.0;
			(*point)[1] = 0.0;
			(*point)[2] = 1.0;
			point++;
			(*point)[0] = 0.0;
			(*point)[1] = 0.0;
			(*point)[2] = 1.0;
			point++;
			/* texture coordinates */
			if (define_texturepoints)
			{
				point = texturepoints;
				(*point)[0] = 0.0;
				(*point)[1] = 0.0;
				(*point)[2] = 0.0;
				point++;
				(*point)[0] = 1.0;
				(*point)[1] = 0.0;
				(*point)[2] = 0.0;
				point++;
				(*point)[0] = 1.0;
				(*point)[1] = 1.0;
				(*point)[2] = 0.0;
				point++;
				(*point)[0] = 0.0;
				(*point)[1] = 1.0;
				(*point)[2] = 0.0;
				point++;
			}
			if (!(surface=CREATE(GT_surface)(g_SH_DISCONTINUOUS_TEXMAP,CMISS_GRAPHICS_RENDER_TYPE_SHADED,
				g_QUADRILATERAL,1,4,points,normalpoints,/*tangentpoints*/(Triple *)NULL,
				texturepoints,g_NO_DATA,(GLfloat *)NULL)))
			{
				DEALLOCATE(points);
				DEALLOCATE(normalpoints);
			}
		}
		if (surface)
		{
			glyph=CREATE(GT_object)(name,g_SURFACE,	(struct Graphical_material *)NULL);
			if (glyph)
			{
				if (!GT_OBJECT_ADD(GT_surface)(glyph,/*time*/0.0,surface))
				{
					DESTROY(GT_object)(&glyph);
					DESTROY(GT_surface)(&surface);
				}
			}
		}
		else
		{
			glyph=(struct GT_object *)NULL;
		}
		if (!glyph)
		{
			display_message(ERROR_MESSAGE,
				"make_glyph_sheet.  Error creating glyph");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"make_glyph_sheet.  Invalid argument(s)");
		glyph=(struct GT_object *)NULL;
	}
	LEAVE;

	return (glyph);
} /* make_glyph_sheet */

struct GT_object *make_glyph_empty(const char *name)
/*******************************************************************************
LAST MODIFIED : 19 October 1998

DESCRIPTION :
Creates a graphics object named <name> with no default glyph.
==============================================================================*/
{
	struct GT_object *glyph;

	ENTER(make_glyph_point);
	if (name)
	{
			glyph=CREATE(GT_object)(name,g_POINTSET, (struct Graphical_material *)NULL);
	}
	else
	{
		display_message(ERROR_MESSAGE,"make_glyph_point.  Invalid argument(s)");
		glyph=(struct GT_object *)NULL;
	}
	LEAVE;

	return (glyph);
} /* make_glyph_point */


struct GT_object *make_glyph_sphere(const char *name,int number_of_segments_around,
	int number_of_segments_down)
/*******************************************************************************
LAST MODIFIED : 19 October 1998

DESCRIPTION :
Creates a graphics object named <name> resembling a sphere with the given
<number_of_segments_around> and <number_of_segments_down> from pole to pole.
The sphere is centred at (0,0,0) and its poles are on the (1,0,0) line. It fits
into the unit cube spanning from -0.5 to +0.5 across all axes. Parameter
<number_of_segments_around> should normally be an even number at least 6 and
twice <number_of_segments_down> look remotely spherical.
==============================================================================*/
{
	ZnReal longitudinal_normal,phi,radial_normal,theta,x,y,z;
	int i,j;
	struct GT_object *glyph;
	struct GT_surface *surface;
	Triple *normal,*normalpoints,*points,*vertex;

	ENTER(make_glyph_sphere);
	if (name&&(2<number_of_segments_around)&&(1<number_of_segments_down))
	{
		surface=(struct GT_surface *)NULL;
		if (ALLOCATE(points,Triple,
			(number_of_segments_down+1)*(number_of_segments_around+1))&&
			ALLOCATE(normalpoints,Triple,(number_of_segments_down+1)*
				(number_of_segments_around+1)))
		{
			/*vertex=points;
				normal=points+(number_of_segments_down+1)*(number_of_segments_around+1);*/
			for (i=0;i <= number_of_segments_down;i++)
			{
				phi=PI*(ZnReal)i/(ZnReal)number_of_segments_down;
				x=-0.5*cos(phi);
				radial_normal=sin(phi);
				longitudinal_normal=2*x;
				/*printf("x=%g l=%g r=%g\n",x,longitudinal_normal,radial_normal);*/
				vertex=points+i;
				normal=normalpoints+i;
				for (j=0;j <= number_of_segments_around;j++)
				{
					theta=2.0*PI*(ZnReal)j/(ZnReal)number_of_segments_around;
					y = radial_normal*sin(theta);
					z = radial_normal*cos(theta);
					(*vertex)[0] = x;
					(*vertex)[1] = 0.5*y;
					(*vertex)[2] = 0.5*z;
					vertex += (number_of_segments_down+1);
					(*normal)[0] = longitudinal_normal;
					(*normal)[1] = y;
					(*normal)[2] = z;
					normal += (number_of_segments_down+1);
				}
			}
			if (!(surface=CREATE(GT_surface)(g_SHADED_TEXMAP,CMISS_GRAPHICS_RENDER_TYPE_SHADED,
				g_QUADRILATERAL,number_of_segments_down+1,number_of_segments_around+1,
				points,normalpoints,/*tangentpoints*/(Triple *)NULL,
				/*texturepoints*/(Triple *)NULL,g_NO_DATA,(GLfloat *)NULL)))
			{
				DEALLOCATE(points);
				DEALLOCATE(normalpoints);
			}
		}
		if (surface)
		{
			glyph=CREATE(GT_object)(name,g_SURFACE,	(struct Graphical_material *)NULL);
			if (glyph)
			{
				if (!GT_OBJECT_ADD(GT_surface)(glyph,/*time*/0.0,surface))
				{
					DESTROY(GT_object)(&glyph);
					DESTROY(GT_surface)(&surface);
				}
			}
		}
		else
		{
			glyph=(struct GT_object *)NULL;
		}
		if (!glyph)
		{
			display_message(ERROR_MESSAGE,
				"make_glyph_sphere.  Error creating glyph");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"make_glyph_sphere.  Invalid argument(s)");
		glyph=(struct GT_object *)NULL;
	}
	LEAVE;

	return (glyph);
} /* make_glyph_sphere */

struct MANAGER(GT_object) *make_standard_glyphs(struct Cmiss_graphics_font *font)
/*******************************************************************************
LAST MODIFIED : 18 November 2005

DESCRIPTION :
Creates a list of standard glyphs for the cmgui applications.
==============================================================================*/
{
	const char *labels_xyz[] = {"x","y","z"}, *labels_fsn[] = {"f","s","n"},
		 *labels_123[] = {"1","2","3"};
	struct GT_object *glyph, *mirror_glyph;
	struct MANAGER(GT_object) *glyph_manager;

	ENTER(make_glyph_sphere);
	glyph_manager = CREATE(MANAGER(GT_object))();
	if (glyph_manager)
	{
		/* add standard glyphs */
		glyph=make_glyph_arrow_line("arrow_line",0.25,0.125);
		if (glyph)
		{
			ADD_OBJECT_TO_MANAGER(GT_object)(glyph,glyph_manager);
		}
		mirror_glyph = glyph;
		glyph=make_glyph_mirror("mirror_arrow_line",mirror_glyph);
		if (glyph)
		{
			ADD_OBJECT_TO_MANAGER(GT_object)(glyph,glyph_manager);
		}
		glyph=make_glyph_arrow_solid("arrow_solid",/*primary_axis*/1,
			12,2.f/3.f,1.f/6.f,/*cone_radius*/0.5f);
		if (glyph)
		{
			ADD_OBJECT_TO_MANAGER(GT_object)(glyph,glyph_manager);
		}
		mirror_glyph = glyph;
		glyph=make_glyph_mirror("mirror_arrow_solid",mirror_glyph);
		if (glyph)
		{
			ADD_OBJECT_TO_MANAGER(GT_object)(glyph,glyph_manager);
		}
		glyph=make_glyph_axes("axes_xyz",/*make_solid*/0,0.1f,0.025f,labels_xyz, 0.1f,font);
		if (glyph)
		{
			ADD_OBJECT_TO_MANAGER(GT_object)(glyph,glyph_manager);
		}
		glyph=make_glyph_axes("axes_fsn",/*make_solid*/0,0.1f,0.025f,labels_fsn, 0.1f,font);
		if (glyph)
		{
			ADD_OBJECT_TO_MANAGER(GT_object)(glyph,glyph_manager);
		}
		glyph=make_glyph_axes("axes_123",/*make_solid*/0,0.1f,0.025f,labels_123, 0.1f,font);
		if (glyph)
		{
			ADD_OBJECT_TO_MANAGER(GT_object)(glyph,glyph_manager);
		}
		glyph=make_glyph_axes("axes",/*make_solid*/0,0.1f,0.025f,(const char **)NULL, 0.1f,font);
		if (glyph)
		{
			ADD_OBJECT_TO_MANAGER(GT_object)(glyph,glyph_manager);
		}
		glyph=make_glyph_axes("axes_solid",/*make_solid*/1,0.1f,0.025f,(const char **)NULL, 0.1f,font);
		if (glyph)
		{
			ADD_OBJECT_TO_MANAGER(GT_object)(glyph,glyph_manager);
		}
		glyph=make_glyph_axes("axes_solid_xyz",/*make_solid*/1,0.1f,0.025f, labels_xyz, 0.1f,font);
		if (glyph)
		{
			ADD_OBJECT_TO_MANAGER(GT_object)(glyph,glyph_manager);
		}
		glyph=make_glyph_cone("cone",12);
		if (glyph)
		{
			ADD_OBJECT_TO_MANAGER(GT_object)(glyph,glyph_manager);
		}
		mirror_glyph = glyph;
		glyph=make_glyph_mirror("mirror_cone",mirror_glyph);
		if (glyph)
		{
			ADD_OBJECT_TO_MANAGER(GT_object)(glyph,glyph_manager);
		}
		glyph=make_glyph_cone_solid("cone_solid",12);
		if (glyph)
		{
			ADD_OBJECT_TO_MANAGER(GT_object)(glyph,glyph_manager);
		}
		glyph=make_glyph_cross("cross");
		if (glyph)
		{
			ADD_OBJECT_TO_MANAGER(GT_object)(glyph,glyph_manager);
		}
		glyph=make_glyph_cube_solid("cube_solid");
		if (glyph)
		{
			ADD_OBJECT_TO_MANAGER(GT_object)(glyph,glyph_manager);
		}
		glyph=make_glyph_cube_wireframe("cube_wireframe");
		if (glyph)
		{
			ADD_OBJECT_TO_MANAGER(GT_object)(glyph,glyph_manager);
		}
		glyph=make_glyph_cylinder("cylinder6",6);
		if (glyph)
		{
			ADD_OBJECT_TO_MANAGER(GT_object)(glyph,glyph_manager);
		}
		glyph=make_glyph_cylinder("cylinder",12);
		if (glyph)
		{
			ADD_OBJECT_TO_MANAGER(GT_object)(glyph,glyph_manager);
		}
		glyph=make_glyph_cylinder_solid("cylinder_solid",12);
		if (glyph)
		{
			ADD_OBJECT_TO_MANAGER(GT_object)(glyph,glyph_manager);
		}
		glyph=make_glyph_cylinder("cylinder_hires",48);
		if (glyph)
		{
			ADD_OBJECT_TO_MANAGER(GT_object)(glyph,glyph_manager);
		}
		glyph=make_glyph_cylinder_solid("cylinder_solid_hires",48);
		if (glyph)
		{
			ADD_OBJECT_TO_MANAGER(GT_object)(glyph,glyph_manager);
		}
		glyph=make_glyph_sphere("diamond",4,2);
		if (glyph)
		{
			ADD_OBJECT_TO_MANAGER(GT_object)(glyph,glyph_manager);
		}
		glyph=make_glyph_axes("grid_lines",
			/*make_solid*/0, /*head_length*/0.0, /*half_head_width*/0.0,
			/*labels*/(const char **)NULL, /*label_offset*/0.1f, font);
		if (glyph)
		{
			Graphics_object_set_glyph_labels_function(glyph, draw_glyph_grid_lines);
			ADD_OBJECT_TO_MANAGER(GT_object)(glyph,glyph_manager);
		}
		glyph=make_glyph_line("line");
		if (glyph)
		{
			ADD_OBJECT_TO_MANAGER(GT_object)(glyph,glyph_manager);
		}
		mirror_glyph = glyph;
		glyph=make_glyph_mirror("mirror_line",mirror_glyph);
		if (glyph)
		{
			ADD_OBJECT_TO_MANAGER(GT_object)(glyph,glyph_manager);
		}
		glyph=make_glyph_empty("empty");
		if (glyph)
		{
			ADD_OBJECT_TO_MANAGER(GT_object)(glyph,glyph_manager);
		}
		glyph=make_glyph_line("line_ticks");
		if (glyph)
		{
			Graphics_object_set_glyph_labels_function(glyph, draw_glyph_axes_ticks);
			ADD_OBJECT_TO_MANAGER(GT_object)(glyph,glyph_manager);
		}
		glyph=make_glyph_point("point",g_POINT_MARKER,0);
		if (glyph)
		{
			ADD_OBJECT_TO_MANAGER(GT_object)(glyph,glyph_manager);
		}
		glyph=make_glyph_sheet("sheet", /*define_texturepoints*/0);
		if (glyph)
		{
			ADD_OBJECT_TO_MANAGER(GT_object)(glyph,glyph_manager);
		}
		glyph=make_glyph_sphere("sphere",12,6);
		if (glyph)
		{
			ADD_OBJECT_TO_MANAGER(GT_object)(glyph,glyph_manager);
		}
		glyph=make_glyph_sphere("sphere_hires",48,24);
		if (glyph)
		{
			ADD_OBJECT_TO_MANAGER(GT_object)(glyph,glyph_manager);
		}
		glyph=make_glyph_sheet("textured_sheet", /*define_texturepoints*/1);
		if (glyph)
		{
			ADD_OBJECT_TO_MANAGER(GT_object)(glyph,glyph_manager);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"make_standard_glyphs.  Could not create glyph list");
	}
	LEAVE;

	return (glyph_manager);
} /* make_standard_glyphs */

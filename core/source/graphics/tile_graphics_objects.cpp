/*******************************************************************************
FILE : tile_graphics_objects.c

LAST MODIFIED : 29 November 2007

DESCRIPTION :
Tiling routines that split graphics objects based on texture coordinates
and Texture_tiling boundaries.
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
 * Portions created by the Initial Developer are Copyright (C) 2007
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
#include <stdio.h>
#include <math.h>
#include "general/debug.h"
#include "graphics/graphics_object.h"
#include "general/message.h"
#include "graphics/tile_graphics_objects.h"
#include "graphics/graphics_object_private.hpp"

const GLfloat small_tolerance = 1e-5f;

static struct GT_surface *tile_create_GT_surface(struct GT_surface *original_surface,
	int size, int tile_number, int polygon_size)
{
	int return_code;
	struct GT_surface *current_surface;

	if (ALLOCATE(current_surface, struct GT_surface, 1))
	{
		return_code = 1;
		current_surface->n_pts1 = 0;
		current_surface->n_pts2 = polygon_size;
		current_surface->object_name = original_surface->object_name;
		current_surface->n_data_components = original_surface->n_data_components;
		current_surface->surface_type = g_SH_DISCONTINUOUS_TEXMAP;
		current_surface->render_type = CMISS_GRAPHICS_RENDER_TYPE_SHADED;
		current_surface->ptrnext = (struct GT_surface *)NULL;
		switch (polygon_size)
		{
			case 3:
			{
				current_surface->polygon = g_TRIANGLE;
			} break;
			case 4:
			{
				current_surface->polygon = g_QUADRILATERAL;
			} break;
		}
		current_surface->tile_number = tile_number;
		if (!ALLOCATE(current_surface->pointlist, Triple,
				size))
		{
			return_code = 0;
		}
		if (original_surface->normallist)
		{
			if (!ALLOCATE(current_surface->normallist, Triple,
					size))
			{
				return_code = 0;
			}
		}
		else
		{
			current_surface->normallist = (Triple *)NULL;
		}
		if (original_surface->tangentlist)
		{
			if (!ALLOCATE(current_surface->tangentlist, Triple,
					size))
			{
				return_code = 0;
			}
		}
		else
		{
			current_surface->tangentlist = (Triple *)NULL;
		}
		if (!ALLOCATE(current_surface->texturelist, Triple,
				size))
		{
			return_code = 0;
		}
		if (original_surface->data)
		{
			if (!ALLOCATE(current_surface->data, GLfloat,
					original_surface->n_data_components * size))
			{
				return_code = 0;
			}
		}
		else
		{
			current_surface->data = 0;
		}
		if (return_code)
		{
			current_surface->allocated_size = size;
		}
		else
		{
			current_surface = 0;
		}
	}
	else
	{
		current_surface = (struct GT_surface *)NULL;
	}
	return(current_surface);
}

static int tile_reallocate_GT_surface(struct GT_surface *surface,
	int size)
{
	int return_code;

	return_code = 1;
	if (!REALLOCATE(surface->pointlist, surface->pointlist, Triple,
			size))
	{
		return_code = 0;
	}
	if (surface->normallist)
	{
		if (!REALLOCATE(surface->normallist, surface->normallist, Triple,
				size))
		{
			return_code = 0;
		}
	}
	if (surface->tangentlist)
	{
		if (!REALLOCATE(surface->tangentlist, surface->tangentlist, Triple,
				size))
		{
			return_code = 0;
		}
	}
	if (!REALLOCATE(surface->texturelist, surface->texturelist, Triple,
			size))
	{
		return_code = 0;
	}
	if (surface->data)
	{
		if (!REALLOCATE(surface->data, surface->data, GLfloat,
				surface->n_data_components * size))
		{
			return_code = 0;
		}
	}
	if (return_code)
	{
		surface->allocated_size = size;
	}
	else
	{
		surface->allocated_size = 0;
	}
	return(return_code);
}

static int tile_interpolate_triangle(struct GT_surface *new_surface,
	int index_new, struct GT_surface *surface, int index,
	int vertexstart, int vertexi, int vertexj,
	GLfloat xi0, GLfloat xii, GLfloat xij, int stepindex)
{
	int k2, return_code;
	Triple *texturetri, *newtexturetri, *pointtri, *newpointtri,
		*normaltri, *newnormaltri, *tangenttri, *newtangenttri;
	GLfloat *datatri, *newdatatri;

	return_code = 1;
	if (index_new + 2 >= new_surface->allocated_size)
	{
		return_code = tile_reallocate_GT_surface(
			new_surface, 2 * new_surface->allocated_size);
	}

	if (return_code)
	{
		texturetri = surface->texturelist;
		newtexturetri = new_surface->texturelist;

		pointtri = surface->pointlist;
		newpointtri = new_surface->pointlist;

		normaltri = surface->normallist;
		newnormaltri = new_surface->normallist;

		tangenttri = surface->tangentlist;
		newtangenttri = new_surface->tangentlist;

		datatri = surface->data;
		newdatatri = new_surface->data;

		for (k2 = 0 ; k2 < 3 ; k2++)
		{
			if (0 == stepindex)
			{
				newtexturetri[index_new][k2] = 
					(1.0 - xi0) * texturetri[index+vertexstart][k2]
					+ xi0 * texturetri[index+vertexi][k2];
			}
			else
			{
				newtexturetri[index_new][k2] = 
					(1.0 - xi0) * texturetri[index+vertexstart][k2]
					+ xi0 * texturetri[index+vertexj][k2];
			}
			newtexturetri[index_new+1][k2] =
				(1.0 - xii) * texturetri[index+vertexstart][k2]
				+ xii * texturetri[index+vertexi][k2];
			newtexturetri[index_new+2][k2] =
				(1.0 - xij) * texturetri[index+vertexstart][k2]
				+ xij * texturetri[index+vertexj][k2];

			if (0 == stepindex)
			{
				newpointtri[index_new][k2] = 
					(1.0 - xi0) * pointtri[index+vertexstart][k2]
					+ xi0 * pointtri[index+vertexi][k2];
			}
			else
			{
				newpointtri[index_new][k2] = 
					(1.0 - xi0) * pointtri[index+vertexstart][k2]
					+ xi0 * pointtri[index+vertexj][k2];
			}
			newpointtri[index_new+1][k2] =
				(1.0 - xii) * pointtri[index+vertexstart][k2]
				+ xii * pointtri[index+vertexi][k2];
			newpointtri[index_new+2][k2] =
				(1.0 - xij) * pointtri[index+vertexstart][k2]
				+ xij * pointtri[index+vertexj][k2];

			if (normaltri)
			{
				if (0 == stepindex)
				{
					newnormaltri[index_new][k2] = 
						(1.0 - xi0) * normaltri[index+vertexstart][k2]
						+ xi0 * normaltri[index+vertexi][k2];
				}
				else
				{
					newnormaltri[index_new][k2] = 
						(1.0 - xi0) * normaltri[index+vertexstart][k2]
						+ xi0 * normaltri[index+vertexj][k2];
				}
				newnormaltri[index_new+1][k2] =
					(1.0 - xii) * normaltri[index+vertexstart][k2]
					+ xii * normaltri[index+vertexi][k2];
				newnormaltri[index_new+2][k2] =
					(1.0 - xij) * normaltri[index+vertexstart][k2]
					+ xij * normaltri[index+vertexj][k2];
			}

			if (tangenttri)
			{
				if (0 == stepindex)
				{
					newtangenttri[index_new][k2] = 
						(1.0 - xi0) * tangenttri[index+vertexstart][k2]
						+ xi0 * tangenttri[index+vertexi][k2];
				}
				else
				{
					newtangenttri[index_new][k2] = 
						(1.0 - xi0) * tangenttri[index+vertexstart][k2]
						+ xi0 * tangenttri[index+vertexj][k2];
				}
				newtangenttri[index_new+1][k2] =
					(1.0 - xii) * tangenttri[index+vertexstart][k2]
					+ xii * tangenttri[index+vertexi][k2];
				newtangenttri[index_new+2][k2] =
					(1.0 - xij) * tangenttri[index+vertexstart][k2]
					+ xij * tangenttri[index+vertexj][k2];
			}
		}
		if (datatri)
		{
			int new_data_index = index_new * surface->n_data_components;
			int data_index = index * surface->n_data_components;
			for (k2 = 0 ; k2 < surface->n_data_components ; k2++)
			{
				if (0 == stepindex)
				{
					newdatatri[new_data_index+k2] = 
						(1.0 - xi0) * datatri[data_index+
							vertexstart * surface->n_data_components+k2]
						+ xi0 * datatri[data_index+
							vertexi * surface->n_data_components+k2];
				}
				else
				{
					newdatatri[new_data_index+k2] = 
						(1.0 - xi0) * datatri[data_index+
							vertexstart* surface->n_data_components+k2]
						+ xi0 * datatri[data_index+
							vertexj* surface->n_data_components+k2];
				}
				newdatatri[new_data_index+surface->n_data_components+k2] = 
					(1.0 - xii) * datatri[data_index+
						vertexstart* surface->n_data_components+k2]
					+ xii * datatri[data_index+
						vertexi* surface->n_data_components+k2];
				newdatatri[new_data_index+2*surface->n_data_components+k2] = 
					(1.0 - xij) * datatri[data_index+
						vertexstart* surface->n_data_components+k2]
					+ xij * datatri[data_index+
						vertexj* surface->n_data_components+k2];
			}
		}
	}
	return(return_code);
}

static int tile_interpolate_quad(struct GT_surface *new_surface,
	int index_new, struct GT_surface *surface, int index,
	int vertexstart, int vertexi, int vertexj, int vertexk,
	GLfloat xi0, GLfloat xi)
{
	int k2, return_code;
	Triple *texturetri, *newtexturetri, *pointtri, *newpointtri,
		*normaltri, *newnormaltri, *tangenttri, *newtangenttri;
	GLfloat *datatri, *newdatatri;

	return_code = 1;
	if (index_new + 2 >= new_surface->allocated_size)
	{
		return_code = tile_reallocate_GT_surface(
			new_surface, 2 * new_surface->allocated_size);
	}

	if (return_code)
	{
		texturetri = surface->texturelist;
		newtexturetri = new_surface->texturelist;

		pointtri = surface->pointlist;
		newpointtri = new_surface->pointlist;

		normaltri = surface->normallist;
		newnormaltri = new_surface->normallist;

		tangenttri = surface->tangentlist;
		newtangenttri = new_surface->tangentlist;

		datatri = surface->data;
		newdatatri = new_surface->data;

		for (k2 = 0 ; k2 < 3 ; k2++)
		{
			newtexturetri[index_new][k2] = 
				(1.0 - xi0) * texturetri[index+vertexstart][k2]
				+ xi0 * texturetri[index+vertexi][k2];
			newtexturetri[index_new+1][k2] =
				(1.0 - xi) * texturetri[index+vertexstart][k2]
				+ xi * texturetri[index+vertexi][k2];
			newtexturetri[index_new+3][k2] =
				(1.0 - xi0) * texturetri[index+vertexj][k2]
				+ xi0 * texturetri[index+vertexk][k2];
			newtexturetri[index_new+2][k2] =
				(1.0 - xi) * texturetri[index+vertexj][k2]
				+ xi * texturetri[index+vertexk][k2];

			newpointtri[index_new][k2] = 
				(1.0 - xi0) * pointtri[index+vertexstart][k2]
				+ xi0 * pointtri[index+vertexi][k2];
			newpointtri[index_new+1][k2] =
				(1.0 - xi) * pointtri[index+vertexstart][k2]
				+ xi * pointtri[index+vertexi][k2];
			newpointtri[index_new+3][k2] =
				(1.0 - xi0) * pointtri[index+vertexj][k2]
				+ xi0 * pointtri[index+vertexk][k2];
			newpointtri[index_new+2][k2] =
				(1.0 - xi) * pointtri[index+vertexj][k2]
				+ xi * pointtri[index+vertexk][k2];

			if (normaltri)
			{
				newnormaltri[index_new][k2] = 
					(1.0 - xi0) * normaltri[index+vertexstart][k2]
					+ xi0 * normaltri[index+vertexi][k2];
				newnormaltri[index_new+1][k2] =
					(1.0 - xi) * normaltri[index+vertexstart][k2]
					+ xi * normaltri[index+vertexi][k2];
				newnormaltri[index_new+3][k2] =
					(1.0 - xi0) * normaltri[index+vertexj][k2]
					+ xi0 * normaltri[index+vertexk][k2];
				newnormaltri[index_new+2][k2] =
					(1.0 - xi) * normaltri[index+vertexj][k2]
					+ xi * normaltri[index+vertexk][k2];
			}

			if (tangenttri)
			{
				newtangenttri[index_new][k2] = 
					(1.0 - xi0) * tangenttri[index+vertexstart][k2]
					+ xi0 * tangenttri[index+vertexi][k2];
				newtangenttri[index_new+1][k2] =
					(1.0 - xi) * tangenttri[index+vertexstart][k2]
					+ xi * tangenttri[index+vertexi][k2];
				newtangenttri[index_new+3][k2] =
					(1.0 - xi0) * tangenttri[index+vertexj][k2]
					+ xi0 * tangenttri[index+vertexk][k2];
				newtangenttri[index_new+2][k2] =
					(1.0 - xi) * tangenttri[index+vertexj][k2]
					+ xi * tangenttri[index+vertexk][k2];
			}
		}
		if (datatri)
		{
			int new_data_index = index_new * surface->n_data_components;
			int data_index = index * surface->n_data_components;
			for (k2 = 0 ; k2 < surface->n_data_components ; k2++)
			{
				newdatatri[new_data_index+k2] = 
					(1.0 - xi0) * datatri[data_index+
						vertexstart * surface->n_data_components+k2]
					+ xi0 * datatri[data_index+
						vertexi * surface->n_data_components+k2];
				newdatatri[new_data_index+surface->n_data_components+k2] = 
					(1.0 - xi) * datatri[data_index+
						vertexstart * surface->n_data_components+k2]
					+ xi * datatri[data_index+
						vertexi * surface->n_data_components+k2];
				newdatatri[new_data_index+3*surface->n_data_components+k2] = 
					(1.0 - xi0) * datatri[data_index+
						vertexj * surface->n_data_components+k2]
					+ xi0 * datatri[data_index+
						vertexk * surface->n_data_components+k2];
				newdatatri[new_data_index+2*surface->n_data_components+k2] = 
					(1.0 - xi) * datatri[data_index+
						vertexj * surface->n_data_components+k2]
					+ xi * datatri[data_index+
						vertexk * surface->n_data_components+k2];
			}
		}
	}
	return(return_code);
}

static int tile_copy_quad_strip_to_dc(struct GT_surface *new_surface,
	int index, struct GT_surface *surface, int i, int j,
	struct Texture_tiling *texture_tiling)
{
	int npts1 = surface->n_pts1;
	GLfloat overlap_range, texture_offset, texture_scaling;
	Triple *texturepoints = surface->texturelist;
	int k, texture_tile;

	if (index + 3 >= new_surface->allocated_size)
	{
		tile_reallocate_GT_surface(new_surface, 2 * new_surface->allocated_size);
	}

	for (k = 0 ; k < 3 ; k++)
	{
		new_surface->pointlist[index][k] = 
			surface->pointlist[i+npts1*j][k];
		new_surface->pointlist[index+1][k] = 
			surface->pointlist[i+1+npts1*j][k];
		new_surface->pointlist[index+3][k] = 
			surface->pointlist[i+npts1*(j+1)][k];
		new_surface->pointlist[index+2][k] = 
			surface->pointlist[i+1+npts1*(j+1)][k];
	}
	if (surface->normallist)
	{
		for (k = 0 ; k < 3 ; k++)
		{
			new_surface->normallist[index][k] = 
				surface->normallist[i+npts1*j][k];
			new_surface->normallist[index+1][k] = 
				surface->normallist[i+1+npts1*j][k];
			new_surface->normallist[index+3][k] = 
				surface->normallist[i+npts1*(j+1)][k];
		new_surface->normallist[index+2][k] = 
				surface->normallist[i+1+npts1*(j+1)][k];
		}
	}
	for (k = 0 ; k < 3 ; k++)
	{
		if (k < 0)
		{
			overlap_range = 2.0 * (GLfloat)texture_tiling->overlap * 
				texture_tiling->tile_coordinate_range[k] /
				(GLfloat)texture_tiling->tile_size[k];
			texture_tile = (int)floor((texturepoints[i+npts1*j][k]) /
				(texture_tiling->tile_coordinate_range[k] - overlap_range));
			/* Need to handle the boundaries specially */
			texture_offset = texture_tile *
				(texture_tiling->tile_coordinate_range[k]- overlap_range);
			texture_scaling = 
				texture_tiling->coordinate_scaling[k];
		}
		else
		{
			texture_offset = 0.0;
			texture_scaling = 1.0;
		}
		new_surface->texturelist[index][k] = 
			(texturepoints[i+npts1*j][k] - texture_offset)
			* texture_scaling;
		new_surface->texturelist[index+1][k] = 
			(texturepoints[i+1+npts1*j][k] - texture_offset)
			* texture_scaling;
		new_surface->texturelist[index+3][k] = 
			(texturepoints[i+npts1*(j+1)][k] - texture_offset)
			* texture_scaling;
		new_surface->texturelist[index+2][k] = 
			(texturepoints[i+1+npts1*(j+1)][k] - texture_offset)
			* texture_scaling;
	}
	if (surface->tangentlist)
	{
		for (k = 0 ; k < 3 ; k++)
		{
			new_surface->tangentlist[index][k] = 
				surface->tangentlist[i+npts1*j][k];
			new_surface->tangentlist[index+1][k] = 
				surface->tangentlist[i+1+npts1*j][k];
			new_surface->tangentlist[index+3][k] = 
				surface->tangentlist[i+npts1*(j+1)][k];
			new_surface->tangentlist[index+2][k] = 
				surface->tangentlist[i+1+npts1*(j+1)][k];
		}
	}
	if (surface->data)
	{
		index *= surface->n_data_components;
		for (k = 0 ; k < surface->n_data_components ; k++)
		{
			new_surface->data[index+k] = 
				surface->data[(i+npts1*j)*surface->n_data_components+k];
			new_surface->data[index+surface->n_data_components+k] = 
				surface->data[(i+1+npts1*j)*surface->n_data_components+k];
			new_surface->data[index+3*surface->n_data_components+k] = 
				surface->data[(i+npts1*(j+1))*surface->n_data_components+k];
			new_surface->data[index+2*surface->n_data_components+k] = 
				surface->data[(i+1+npts1*(j+1))*surface->n_data_components+k];
		}
	}
	return(1);
}

static int tile_copy_vertices_to_triangle(
	struct GT_surface *new_surface, int new_index,
	struct GT_surface *surface, int index1, int index2, int index3)
{
	Triple *texturepoints = surface->texturelist;
	Triple *points = surface->pointlist;
	int k;

	if (new_index + 2 >= new_surface->allocated_size)
	{
		tile_reallocate_GT_surface(new_surface, 2 * new_surface->allocated_size);
	}

	for (k = 0 ; k < 3 ; k++)
	{
		new_surface->texturelist[new_index][k] = texturepoints[index1][k];
		new_surface->texturelist[new_index+1][k] = texturepoints[index2][k];
		new_surface->texturelist[new_index+2][k] = texturepoints[index3][k];

		new_surface->pointlist[new_index][k] = points[index1][k];
		new_surface->pointlist[new_index+1][k] = points[index2][k];
		new_surface->pointlist[new_index+2][k] = points[index3][k];
		if (surface->normallist)
		{
			new_surface->normallist[new_index][k] =
				surface->normallist[index1][k];
			new_surface->normallist[new_index+1][k] = 
				surface->normallist[index2][k];
			new_surface->normallist[new_index+2][k] =
				surface->normallist[index3][k];
		}
		if (surface->tangentlist)
		{
			new_surface->tangentlist[new_index][k] =
				surface->tangentlist[index1][k];
			new_surface->tangentlist[new_index+1][k] = 
				surface->tangentlist[index2][k];
			new_surface->tangentlist[new_index+2][k] =
				surface->tangentlist[index3][k];
		}
	}
	if (surface->data)
	{
		new_index *= surface->n_data_components;
 		index1 *= surface->n_data_components;
 		index2 *= surface->n_data_components;
 		index3 *= surface->n_data_components;
		for (k = 0 ; k < surface->n_data_components ; k++)
		{
			new_surface->data[new_index+k] = 
				surface->data[index1+k];
			new_surface->data[new_index+surface->n_data_components+k] = 
				surface->data[index2+k];
			new_surface->data[new_index+2*surface->n_data_components+k] = 
				surface->data[index3+k];
		}
	}
	return(1);
}

static int tile_copy_polygon(struct GT_surface *new_surface,
	int index_new, struct GT_surface *surface, int index)
{
	int i, k2;
	Triple *texturetri, *newtexturetri, *pointtri, *newpointtri,
		*normaltri, *newnormaltri, *tangenttri, *newtangenttri;
	GLfloat *datatri, *newdatatri;

	int local_index, local_index_new;

	int return_code = 1;
	if (index_new + new_surface->n_pts2 >= new_surface->allocated_size)
	{
		return_code = tile_reallocate_GT_surface(new_surface,
			2 * new_surface->allocated_size);
	}

	if (return_code)
	{
		texturetri = surface->texturelist;
		newtexturetri = new_surface->texturelist;

		pointtri = surface->pointlist;
		newpointtri = new_surface->pointlist;

		normaltri = surface->normallist;
		newnormaltri = new_surface->normallist;

		tangenttri = surface->tangentlist;
		newtangenttri = new_surface->tangentlist;

		datatri = surface->data;
		newdatatri = new_surface->data;

		if (new_surface->n_pts2 == surface->n_pts2)
		{
			for (k2 = 0 ; k2 < 3 ; k2++)
			{
				local_index = index;
				local_index_new = index_new;

				for (i = 0 ; i < surface->n_pts2 ; i++)
				{
					newtexturetri[local_index_new][k2] = texturetri[local_index][k2];

					newpointtri[local_index_new][k2] = 
						pointtri[local_index][k2];

					if (normaltri)
					{
						newnormaltri[local_index_new][k2] = 
							normaltri[local_index][k2];
					}

					if (tangenttri)
					{
						newtangenttri[local_index_new][k2] = 
							tangenttri[local_index][k2];
					}

					local_index++;
					local_index_new++;
				}
			}
			if (datatri)
			{
				int new_data_index = index_new * surface->n_data_components;
				int data_index = index * surface->n_data_components;
				for (i = 0 ; i < surface->n_pts2 ; i++)
				{
					for (k2 = 0 ; k2 < surface->n_data_components ; k2++)
					{
						newdatatri[new_data_index] = 
							datatri[data_index];
						new_data_index++;
						data_index++;
					}
				}
			}
		}
		else
		{
			return_code = 0;
		}
	}
	return (return_code);
}

static struct GT_surface *tile_create_or_get_tile_bin(
	struct GT_surface **tiles, int bin_number, int initial_points,
	int polygon_size, struct GT_surface *template_surface)
{
	struct GT_surface *return_surface;

	if (tiles[bin_number])
 	{
		return_surface = tiles[bin_number];
	}
	else
	{
		return_surface = tile_create_GT_surface(template_surface,
					initial_points, bin_number, polygon_size);
		if (return_surface)
		{
			tiles[bin_number] = return_surface;
		}
	}
	
	return (return_surface);
}

static int scale_texture_coordinates(struct GT_surface *surface,
	struct Texture_tiling *texture_tiling)
{
	GLfloat overlap_range, texture_average, texture_offset, texture_scaling;
	int i, j, k, texture_tile;
	Triple *texturepoints = surface->texturelist;

	for (i = 0 ; i < surface->n_pts1 ; i++)
	{
		for (k = 0 ; k < texture_tiling->dimension ; k++)
		{
			if (texture_tiling->texture_tiles[k] > 1)
			{
			/* These polygons may come from different repeats of the
				same tile so we have to work out the offset each time */
			texture_average = 0.0;
			for (j = 0 ; j < surface->n_pts2 ; j++)
			{
				texture_average += texturepoints[j][k];
			}
			texture_average /= (GLfloat)surface->n_pts2;
			overlap_range = 2.0 * (GLfloat)texture_tiling->overlap * 
				texture_tiling->tile_coordinate_range[k] /
				(GLfloat)texture_tiling->tile_size[k];
			texture_tile = (int)floor((texture_average) /
				(texture_tiling->tile_coordinate_range[k] - overlap_range));

			texture_offset =
				texture_tile *
				(texture_tiling->tile_coordinate_range[k] - overlap_range)
				- 0.5 * overlap_range;
			texture_scaling = 
				texture_tiling->coordinate_scaling[k];

			for (j = 0 ; j < surface->n_pts2 ; j++)
			{
				texturepoints[j][k] = (texturepoints[j][k] - texture_offset)
					* texture_scaling;
				GLfloat texture_min = 0.0;		
				if (texturepoints[j][k] < texture_min + small_tolerance)
				{
					texturepoints[j][k] = texture_min;
				}
				GLfloat texture_max = texture_scaling * (texture_tiling->tile_coordinate_range[k]);
				if (texturepoints[j][k] > texture_max - small_tolerance)
				{
					texturepoints[j][k] = texture_max - small_tolerance;
				}
			}
			}
		}
		texturepoints += surface->n_pts2;
	}

	return (1);
}

static int tile_select_tile_bin(Triple texture_point,
	struct Texture_tiling *texture_tiling, int with_modulus,
	Triple overlap_range)
{
	int tile, tilex, tiley, tilez;
	switch (texture_tiling->dimension)
	{
		case 1:
		{
			if (texture_tiling->texture_tiles[0] > 1)
			{
				tilex = (int)floor((texture_point[0]) / 
					(texture_tiling->tile_coordinate_range[0] - overlap_range[0]));
			}
			else
			{
				tilex = 0;
			}
			if (with_modulus)
			{
				if (tilex < 0)
				{
					tilex = ((-tilex) % texture_tiling->texture_tiles[0]);
					if (tilex != 0)
					{
						tilex = texture_tiling->texture_tiles[0] - tilex;
					}			
				}
				else
				{
					tilex = tilex % texture_tiling->texture_tiles[0];
				}
			}
			tile = tilex;
		} break;
		case 2:
		{
			if (texture_tiling->texture_tiles[0] > 1)
			{
				tilex = (int)floor((texture_point[0]) / 
					(texture_tiling->tile_coordinate_range[0] - overlap_range[0]));
			}
			else
			{
				tilex = 0;
			}
			if (texture_tiling->texture_tiles[1] > 1)
			{
				tiley = (int)floor((texture_point[1]) / 
					(texture_tiling->tile_coordinate_range[1] - overlap_range[1]));
			}
			else
			{
				tiley = 0;
			}
			if (with_modulus)
			{
				if (tilex < 0)
				{
					tilex = ((-tilex) % texture_tiling->texture_tiles[0]);
					if (tilex != 0)
					{
						tilex = texture_tiling->texture_tiles[0] - tilex;
					}			
				}
				else
				{
					tilex = tilex % texture_tiling->texture_tiles[0];
				}
				if (tiley < 0)
				{
					tiley = ((-tiley) % texture_tiling->texture_tiles[1]);
					if (tiley != 0)
					{
						tiley = texture_tiling->texture_tiles[1] - tiley;
					}			
				}
				else
				{
					tiley = tiley % texture_tiling->texture_tiles[1];
				}
			}
			tile = tilex +
				texture_tiling->texture_tiles[0] * tiley;
		} break;
		case 3:
		{
			if (texture_tiling->texture_tiles[0] > 1)
			{
				tilex = (int)floor((texture_point[0]) / 
					(texture_tiling->tile_coordinate_range[0] - overlap_range[0]));
			}
			else
			{
				tilex = 0;
			}
			if (texture_tiling->texture_tiles[1] > 1)
			{
				tiley = (int)floor((texture_point[1]) / 
					(texture_tiling->tile_coordinate_range[1] - overlap_range[1]));
			}
			else
			{
				tiley = 0;
			}
			if (texture_tiling->texture_tiles[2] > 1)
			{
				tilez = (int)floor((texture_point[2]) / 
					(texture_tiling->tile_coordinate_range[2] - overlap_range[2]));
			}
			else
			{
				tilez = 0;
			}
			if (with_modulus)
			{
				if (tilex < 0)
				{
					tilex = ((-tilex) % texture_tiling->texture_tiles[0]);
					if (tilex != 0)
					{
						tilex = texture_tiling->texture_tiles[0] - tilex;
					}			
				}
				else
				{
					tilex = tilex % texture_tiling->texture_tiles[0];
				}
				if (tiley < 0)
				{
					tiley = ((-tiley) % texture_tiling->texture_tiles[1]);
					if (tiley != 0)
					{
						tiley = texture_tiling->texture_tiles[1] - tiley;
					}			
				}
				else
				{
					tiley = tiley % texture_tiling->texture_tiles[1];
				}
				if (tilez < 0)
				{
					tilez = ((-tilez) % texture_tiling->texture_tiles[2]);
					if (tilez != 0)
					{
						tilez = texture_tiling->texture_tiles[2] - tilez;
					}			
				}
				else
				{
					tilez = tilez % texture_tiling->texture_tiles[2];
				}
			}
			tile = tilex +
				texture_tiling->texture_tiles[0] * 
				(tiley + 
					texture_tiling->texture_tiles[1] * tilez);
		} break;
		default:
		{
			tile = 0;
		}
	}
	return (tile);
}

/***************************************************************************//**
 * Split a GT_surface <surface> of quads based on its texture coordinates and 
 * <texture_tiling> boundaries.  Returns a surface or linked list of surfaces
 * that have equivalent geometry separated into separate surfaces for separate
 * tiles.
 * This routine is not general, it only works if the quadrilateril sides are 
 * exactly aligned with the texture coordinates.  If this is not the case it is
 * expected that the quad is split into two triangles and this version of the tile
 * algorithm used.
 */
static int tile_and_bin_GT_surface_quads(struct GT_surface *surface, 
	struct Texture_tiling *texture_tiling, struct GT_surface **quad_tiles,
	GLfloat *overlap_range, int size, int number_of_tiles)
{
	GLfloat nextcut, xi0, xi;
	int finished, index, k, l, index_new, return_code, 
		vertex1, vertexstart, vertexi, vertexj, othervertex;
	struct GT_surface *current_surface, *new_surface;
	Triple texture_centre, *texturetri;
	GLfloat vertexk[4];

	ENTER(tile_GT_surface);

	if (surface && texture_tiling && quad_tiles)
	{
		return_code = 1;
		current_surface = surface;
		for (k = 0 ; return_code && (k < texture_tiling->dimension) ; k++)
		{
#if defined (DEBUG_CODE)
			printf("direction %d\n", k);
#endif /* defined (DEBUG_CODE) */
			if (texture_tiling->texture_tiles[k] > 1)
			{
				new_surface = tile_create_GT_surface(current_surface,
					size, 0, /*polygon_size*/4);
									
				for (l = 0 ; return_code && (l < current_surface->n_pts1) ; l++)
				{
					index = 4 * l;
					texturetri = current_surface->texturelist;
	
					/* We can work from the three vertices as we know the fourth is 
					 * aligned in texture coordinates.
					 */
					vertexk[0] = (texturetri[index][k]) / 
						(texture_tiling->tile_coordinate_range[k] - overlap_range[k]);
					vertexk[1] = (texturetri[index+1][k]) / 
						(texture_tiling->tile_coordinate_range[k] - overlap_range[k]);
					vertexk[2] = (texturetri[index+2][k]) / 
						(texture_tiling->tile_coordinate_range[k] - overlap_range[k]);
					vertexk[3] = (texturetri[index+3][k]) / 
						(texture_tiling->tile_coordinate_range[k] - overlap_range[k]);
#if defined (DEBUG_CODE)
					printf(" tricoordinates %g %g %g\n",
						texturetri[index][k], texturetri[index+1][k], texturetri[index+2][k]);
					printf(" vertexk %g %g %g\n",
						vertexk[0], vertexk[1], vertexk[2]);
#endif /* defined (DEBUG_CODE) */
					if ((vertexk[1] >= vertexk[0]) && (vertexk[2] >= vertexk[0]))
					{
						vertexstart = 0;
						vertexi = 1;
						vertexj = 3;
						othervertex = 2;
					}
					else
					{
						if ((vertexk[2] >= vertexk[1]) && (vertexk[0] >= vertexk[1]))
						{
							vertexstart = 1;
							vertexi = 2;
							vertexj = 0;
							othervertex = 3;
						}
						else
						{
							vertexstart = 3;
							vertexi = 0;
							vertexj = 2;
							othervertex = 1;
						}
					}
	
#if defined (DEBUG_CODE)
					printf(" vertexstart %d %g %g %g\n",
						vertexstart, vertexk[vertexstart],
						vertexk[vertexi], vertexk[vertexj]);
#endif /* defined (DEBUG_CODE) */
	
					nextcut = floor(vertexk[vertexstart] + 1.0 + small_tolerance);
					if (vertexk[vertexj] > vertexk[vertexi] + small_tolerance)
					{
						/* Then we assume vertexi and vertexstart have the same value and so
						 * rather than flipping we rotate so vertexstart is now what was vertexi
						 * to maintain the same winding order.
						 */
						int tempvertexj = vertexstart;
						vertexstart = vertexi;
						vertexi = othervertex;
						othervertex = vertexj;
						vertexj = tempvertexj;
					}
					if (vertexk[vertexi] > vertexk[vertexstart] + small_tolerance)
					{
						/* What was the previous xi */
						xi0 = 0.0;
						finished = 0;
						
						while (!finished)
						{
							xi = (nextcut - vertexk[vertexstart]) /
								(vertexk[vertexi] - vertexk[vertexstart]);
							if (xi > 1.0 - small_tolerance)
							{
								xi = 1.0;
							}
		
							index_new = 4 * new_surface->n_pts1;
							if (tile_interpolate_quad(new_surface,
									index_new, current_surface, index,
									vertexstart, vertexi, vertexj, othervertex,
									xi0, xi))
							{
								new_surface->n_pts1++;
							}
							else
							{
								return_code = 0;
								finished = 1;
							}
		
							xi0 = xi;
							if (nextcut < vertexk[vertexi])
							{
								nextcut++;
								
								if (nextcut > vertexk[vertexi] - small_tolerance)
								{
									nextcut = vertexk[vertexi];
								}
							}
							else
							{
								finished = 1;
							}
						}
					}
					else
					{
						if (tile_copy_polygon(new_surface,
							4 * new_surface->n_pts1,
							current_surface, index))
						{
							new_surface->n_pts1++;
						}
					}
				}
				if (current_surface != surface)
				{
					/* Destroy this if it is a local temporary, 
						otherwise leave it up to the calling routine who
						created it. */
					DESTROY(GT_surface)(&current_surface);
				}
				current_surface = new_surface;
			}
		}
							
		/* Put the quads in their appropriate bin */
		for (l = 0 ; return_code && (l < current_surface->n_pts1) ; l++)
		{
			index = 4 * l;
			/* Use the centre of a triangle to determine its bin */
								
			texturetri = current_surface->texturelist;
			texture_centre[0] = (texturetri[index][0]
				+ texturetri[index+1][0] + 
				texturetri[index+3][0]) / 3.0;
			texture_centre[1] = (texturetri[index][1]
				+ texturetri[index+1][1] + 
				texturetri[index+3][1]) / 3.0;
			texture_centre[2] = (texturetri[index][2]
				+ texturetri[index+1][2] + 
				texturetri[index+3][2]) / 3.0;
			vertex1 = tile_select_tile_bin(texture_centre,
				texture_tiling, /*with_modulus*/1, overlap_range);
			if ((vertex1 >= 0) && (vertex1 < number_of_tiles))
			{
				new_surface = tile_create_or_get_tile_bin(
					quad_tiles, vertex1, size,
					/*polygon_size*/4, current_surface);
				if (new_surface)
				{
					if (tile_copy_polygon(new_surface,
							4 * new_surface->n_pts1,
							current_surface, index))
					{
						new_surface->n_pts1++;
					}
					else
					{
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
				display_message(ERROR_MESSAGE, "tile_GT_surface.  "
					"Invalid triangle tile %d", vertex1);
			}
		}
		if (surface != current_surface)
		{
			DESTROY(GT_surface)(&current_surface);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"tile_GT_surface. Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* tile_and_bin_GT_surface_quads */

/***************************************************************************//**
 * Split a GT_surface <surface> of triangles based on its texture coordinates and 
 * <texture_tiling> boundaries.  Returns a surface or linked list of surfaces
 * that have equivalent geometry separated into separate surfaces for separate
 * tiles.
 * 
 */
static int tile_and_bin_GT_surface_triangles(struct GT_surface *surface, 
	struct Texture_tiling *texture_tiling,
	struct GT_surface **triangle_tiles,
	GLfloat *overlap_range, int size, int number_of_tiles)
{
	GLfloat nextcuti, nextcutj, xi0, xii, xij;
	int finished, index, k, l, index_new, return_code, stepindex, 
		vertex1, vertexstart, vertexi, vertexj;
	struct GT_surface *current_surface, *new_surface;
	Triple texture_centre, *texturetri, vertexk;

	ENTER(tile_GT_surface);

	if (surface && texture_tiling && triangle_tiles)
	{
		return_code = 1;
		current_surface = surface;
		for (k = 0 ; return_code && (k < texture_tiling->dimension) ; k++)
		{
#if defined (DEBUG_CODE)
			printf("direction %d\n", k);
#endif /* defined (DEBUG_CODE) */
			if (texture_tiling->texture_tiles[k] > 1)
			{
				new_surface = tile_create_GT_surface(current_surface,
					size, 0, /*polygon_size*/3);
									
				for (l = 0 ; return_code && (l < current_surface->n_pts1) ; l++)
				{
					index = 3 * l;
					texturetri = current_surface->texturelist;
	
					vertexk[0] = (texturetri[index][k]) / 
						(texture_tiling->tile_coordinate_range[k] - overlap_range[k]);
					vertexk[1] = (texturetri[index+1][k]) / 
						(texture_tiling->tile_coordinate_range[k] - overlap_range[k]);
					vertexk[2] = (texturetri[index+2][k]) / 
						(texture_tiling->tile_coordinate_range[k] - overlap_range[k]);
#if defined (DEBUG_CODE)
					printf(" tricoordinates %g %g %g\n",
						texturetri[index][k], texturetri[index+1][k], texturetri[index+2][k]);
					printf(" vertexk %g %g %g\n",
						vertexk[0], vertexk[1], vertexk[2]);
#endif /* defined (DEBUG_CODE) */
					if ((vertexk[1] >= vertexk[0]) && (vertexk[2] >= vertexk[0]))
					{
						vertexstart = 0;
						vertexi = 1;
						vertexj = 2;
					}
					else
					{
						if ((vertexk[2] >= vertexk[1]) && (vertexk[0] >= vertexk[1]))
						{
							vertexstart = 1;
							vertexi = 2;
							vertexj = 0;
						}
						else
						{
							vertexstart = 2;
							vertexi = 0;
							vertexj = 1;
						}
					}
	
#if defined (DEBUG_CODE)
					printf(" vertexstart %d %g %g %g\n",
						vertexstart, vertexk[vertexstart],
						vertexk[vertexi], vertexk[vertexj]);
#endif /* defined (DEBUG_CODE) */
	
					nextcuti = floor(vertexk[vertexstart] + 1.0 + small_tolerance);
					nextcutj = nextcuti;					
					/* Which side did we last step on */
					stepindex = -1;
					if (nextcuti > vertexk[vertexi])
					{
						nextcuti = vertexk[vertexi];
						/* Don't step the other side */
						stepindex = 1;
					}
					if (nextcutj > vertexk[vertexj])
					{
						nextcutj = vertexk[vertexj];
						/* Don't step the other side */
						stepindex = 0;
					}
					/* What was the previous xi on the step side */
					xi0 = 0.0;
	
					finished = 0;
					while (!finished)
					{
						if (fabs(vertexk[vertexi] - vertexk[vertexstart]) > small_tolerance)
						{
							xii = (nextcuti - vertexk[vertexstart]) /
								(vertexk[vertexi] - vertexk[vertexstart]);
							if (xii > 1.0 - small_tolerance)
							{
								xii = 1.0;
							}
						}
						else
						{
							xii = 1.0;
						}
						if (fabs(vertexk[vertexj] - vertexk[vertexstart]) > small_tolerance)
						{
							xij = (nextcutj - vertexk[vertexstart]) /
								(vertexk[vertexj] - vertexk[vertexstart]);
							if (xij > 1.0 - small_tolerance)
							{
								xij = 1.0;
							}
						}
						else
						{
							xij = 1.0;
						}
	
#if defined (DEBUG_CODE)
						GLfloat lastcut;
						if (stepindex == 0)
						{
							lastcut = vertexk[vertexstart] +
								xi0 * (vertexk[vertexi] - vertexk[vertexstart]);
						}
						else
						{
							lastcut = vertexk[vertexstart] +
								xi0 * (vertexk[vertexj] - vertexk[vertexstart]);
						}
						printf(" triangle %d (%g %g %g) %g %g %g\n",
							stepindex, lastcut,
							nextcuti, nextcutj,
							vertexk[vertexi], vertexk[vertexj], xi0);
#endif /* defined (DEBUG_CODE) */
	
						index_new = 3 * new_surface->n_pts1;
						if (tile_interpolate_triangle(new_surface,
								index_new, current_surface, index,
								vertexstart, vertexi, vertexj,
								xi0, xii, xij, stepindex))
						{
							new_surface->n_pts1++;
						}
						else
						{
							return_code = 0;
							finished = 1;
						}
	
						/* Test != as initially -1 and either step 
							would be OK */
						if (((stepindex != 0) && (nextcuti < vertexk[vertexi]) && (nextcuti <= nextcutj)) ||
							((stepindex != 1) && (nextcutj < vertexk[vertexj]) && (nextcutj <= nextcuti)))
						{
							if ((stepindex != 0) && (nextcuti < vertexk[vertexi]) && (nextcuti <= nextcutj))
							{
								nextcuti++;
								stepindex = 0;
								xi0 = xii;
								if (nextcuti > vertexk[vertexi] - small_tolerance)
								{
									nextcuti = vertexk[vertexi];
								}
							}
							else
							{
								nextcutj++;
								stepindex = 1;
								xi0 = xij;
								if (nextcutj > vertexk[vertexj] - small_tolerance)
								{
									nextcutj = vertexk[vertexj];
								}
							}
						}
						else
						{
							finished = 1;
							if ((nextcuti != vertexk[vertexi])
								|| (nextcutj != vertexk[vertexj]))
							{
#if defined (DEBUG_CODE)
								printf("  not finished %g %g  %g %g\n",
									nextcuti, nextcutj,
									vertexk[vertexi], vertexk[vertexj]);
#endif /* defined (DEBUG_CODE) */
								/* Add the remainder to the old list */
								if (nextcuti < vertexk[vertexi])
								{
									stepindex = 0;
									xi0 = xii;
									nextcuti = vertexk[vertexi];
									xii = 1.0;
								}
								else
								{
									stepindex = 1;
									xi0 = xij;
									nextcutj = vertexk[vertexj];
									xij = 1.0;
								}
								/* Put this in the queue for the current surface */
								index_new = 3 * current_surface->n_pts1;
								if (tile_interpolate_triangle(current_surface,
										index_new, current_surface, index,
										vertexstart, vertexi, vertexj,
										xi0, xii, xij, stepindex))
								{
									current_surface->n_pts1++;
								}
								else
								{
									finished = 1;
									return_code = 0;
								}
							}
#if defined (DEBUG_CODE)
							else
							{
								printf("  finished %g %g  %g %g\n",
									nextcuti, nextcutj,
									vertexk[vertexi], vertexk[vertexj]);
							}
#endif /* defined (DEBUG_CODE) */
						}
					}
				}
				if (current_surface != surface)
				{
					/* Destroy this if it is a local temporary, 
						otherwise leave it up to the calling routine who
						created it. */
					DESTROY(GT_surface)(&current_surface);
				}
				current_surface = new_surface;
			}
		}
							
		/* Put the triangles in their appropriate bin */
		for (l = 0 ; return_code && (l < current_surface->n_pts1) ; l++)
		{
			index = 3 * l;
			/* Use the centre of a triangle to determine its bin */
								
			texturetri = current_surface->texturelist;
			texture_centre[0] = (texturetri[index][0]
				+ texturetri[index+1][0] + 
				texturetri[index+2][0]) / 3.0;
			texture_centre[1] = (texturetri[index][1]
				+ texturetri[index+1][1] + 
				texturetri[index+2][1]) / 3.0;
			texture_centre[2] = (texturetri[index][2]
				+ texturetri[index+1][2] + 
				texturetri[index+2][2]) / 3.0;
			vertex1 = tile_select_tile_bin(texture_centre,
				texture_tiling, /*with_modulus*/1, overlap_range);
			if ((vertex1 >= 0) && (vertex1 < number_of_tiles))
			{
				new_surface = tile_create_or_get_tile_bin(
					triangle_tiles, vertex1, size,
					/*polygon_size*/3, current_surface);
				if (new_surface)
				{
					if (tile_copy_polygon(new_surface,
							3 * new_surface->n_pts1,
							current_surface, index))
					{
						new_surface->n_pts1++;
					}
					else
					{
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
				display_message(ERROR_MESSAGE, "tile_GT_surface.  "
					"Invalid triangle tile %d", vertex1);
			}
		}
		if (surface != current_surface)
		{
			DESTROY(GT_surface)(&current_surface);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"tile_GT_surface. Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* tile_and_bin_GT_surface */

#if defined (DEBUG_CODE)
static int write_GT_surface(struct GT_surface *surface)
{
	struct GT_surface *current_surface = surface;
	
	while (current_surface)
	{
		int i, j, k;
		printf("GT_surface_type %d\n", current_surface->surface_type);
		printf("Render_type %d\n", current_surface->render_type);
		printf("gtPolygonType %d\n", current_surface->polygon);
		printf("n_data_components %d\n", current_surface->n_data_components);
		printf("n_pts1 %d\n", current_surface->n_pts1);
		printf("n_pts2 %d\n", current_surface->n_pts2);
		Triple *point = current_surface->pointlist;
		Triple *normal = current_surface->normallist;
		Triple *tangent = current_surface->tangentlist;
		Triple *texture = current_surface->texturelist;
		GLfloat *data = current_surface->data;
		for (i = 0 ; i < current_surface->n_pts1 ; i++)
		{
			for (j = 0 ; j < current_surface->n_pts2 ; j++)
			{
				if (point)
				{
					printf("point %f %f %f\n", (*point)[0], (*point)[1], (*point)[2]);
					point++;
				}
				if (normal)
				{
					printf("normal %f %f %f\n", (*normal)[0], (*normal)[1], (*normal)[2]);
					normal++;
				}
				if (tangent)
				{
					printf("tangent %f %f %f\n", (*tangent)[0], (*tangent)[1], (*tangent)[2]);
					tangent++;
				}
				if (texture)
				{
					printf("texture %f %f %f\n", (*texture)[0], (*texture)[1], (*texture)[2]);
					texture++;
				}
				if (data)
				{
					printf("data");
					for (k = 0 ; k < current_surface->n_data_components ; k++)
					{
						printf(" %f", *data);
						data++;
					}
					printf("\n");
				}
			}
		}
		/* store integer object_name eg. element number from which this object came */
		printf("object_name %d\n", current_surface->object_name);
		printf("tile_number %d\n", current_surface->tile_number);
		printf("allocated_size %d\n", current_surface->allocated_size);

		printf("ptrnext %p\n\n", current_surface->ptrnext);
		current_surface = current_surface->ptrnext;
	}

	return (1);
}
#endif // defined (DEBUG_CODE)

static int tile_test_aligned_texture_coordinates(Triple vertex1, Triple vertex2)
{
	int return_code;
	
	int coord_0_same = (fabs(vertex1[0] - vertex2[0]) < small_tolerance);
	int coord_1_same = (fabs(vertex1[1] - vertex2[1]) < small_tolerance);
	int coord_2_same = (fabs(vertex1[2] - vertex2[2]) < small_tolerance);
	return_code = (coord_0_same + coord_1_same + coord_2_same >= 2);

	return (return_code);
}

struct GT_surface *tile_GT_surface(struct GT_surface *surface, 
	struct Texture_tiling *texture_tiling)
/*******************************************************************************
LAST MODIFIED : 30 November 2007

DESCRIPTION :
Split a GT_surface <surface> based on its texture coordinates and 
<texture_tiling> boundaries.  Returns a surface or linked list of surfaces
that have equivalent geometry separated into separate surfaces for separate
tiles.
==============================================================================*/
{
	int i, index, index1, index2, index3, index4, j, k, kmax,
		initial_points, npts1, npts2,
		number_of_tiles, return_code, vertex1, vertex2, vertex3, vertex4;
	struct GT_surface *current_surface,  *return_surface,
		**surface_tiles, **triangle_tiles;
	Triple overlap_range, *texturepoints;

	ENTER(tile_GT_surface);

	if (surface && texture_tiling)
	{
		return_code = 1;
		npts1 = surface->n_pts1;
		npts2 = surface->n_pts2;
		number_of_tiles = texture_tiling->texture_tiles[0];
		overlap_range[0] = 2.0 * (GLfloat)texture_tiling->overlap * 
			texture_tiling->tile_coordinate_range[0] /
			(GLfloat)texture_tiling->tile_size[0];
		if (texture_tiling->dimension > 1)
		{
			number_of_tiles *= texture_tiling->texture_tiles[1];
			overlap_range[1] = 2.0 * (GLfloat)texture_tiling->overlap * 
				texture_tiling->tile_coordinate_range[1] /
				(GLfloat)texture_tiling->tile_size[1];
		}
		if (texture_tiling->dimension > 2)
		{
			number_of_tiles *= texture_tiling->texture_tiles[2];
			overlap_range[2] = 2.0 * (GLfloat)texture_tiling->overlap * 
				texture_tiling->tile_coordinate_range[2] /
				(GLfloat)texture_tiling->tile_size[2];
		}
		ALLOCATE(surface_tiles, struct GT_surface *, number_of_tiles);
		for (i = 0 ; i < number_of_tiles ; i++)
		{
			surface_tiles[i] = (struct GT_surface *)NULL;
		}
		ALLOCATE(triangle_tiles, struct GT_surface *, number_of_tiles);
		for (i = 0 ; i < number_of_tiles ; i++)
		{
			triangle_tiles[i] = (struct GT_surface *)NULL;
		}
		texturepoints = surface->texturelist;
		initial_points = 100;
		switch (surface->polygon)
		{
			case g_QUADRILATERAL:
			{
				for (i=0;return_code && (i<npts1-1);i++)
				{
					for (j=0;return_code && (j<npts2-1);j++)
					{
						index1 = i+npts1*j;
						index2 = i+1+npts1*j;
						index3 = i+npts1*(j+1);
						index4 = i+1+npts1*(j+1);

						/* Determine which tile each vertex belongs too */
						vertex1 = tile_select_tile_bin(texturepoints[index1],
							texture_tiling, /*with_modulus*/0, overlap_range);
						vertex2 = tile_select_tile_bin(texturepoints[index2],
							texture_tiling, /*with_modulus*/0, overlap_range);
						vertex3 = tile_select_tile_bin(texturepoints[index3],
							texture_tiling, /*with_modulus*/0, overlap_range);
						vertex4 = tile_select_tile_bin(texturepoints[index4],
							texture_tiling, /*with_modulus*/0, overlap_range);
						if ((vertex1 == vertex2) &&
							(vertex1 == vertex3) &&
							(vertex1 == vertex4))
						{
							vertex1 = tile_select_tile_bin(texturepoints[index1],
							texture_tiling, /*with_modulus*/1, overlap_range);
							current_surface = tile_create_or_get_tile_bin(
								surface_tiles, vertex1, initial_points,
								/*polygon_size*/4, surface);
							if (current_surface)
							{
								index = 4 * current_surface->n_pts1;
								return_code = tile_copy_quad_strip_to_dc(
									current_surface, index, surface, i, j,
									texture_tiling);
								current_surface->n_pts1++;
							}
							else
							{
								return_code = 0;
							}
						}
						else if (tile_test_aligned_texture_coordinates(texturepoints[index1], texturepoints[index2])
							&& tile_test_aligned_texture_coordinates(texturepoints[index1], texturepoints[index3])
							&& tile_test_aligned_texture_coordinates(texturepoints[index2], texturepoints[index4]))
						{
							/* Make a surface to store our triangles as we split them */
							current_surface = tile_create_GT_surface(surface,
								initial_points, /*tile_number*/0, /*polygon_size*/4);
							return_code = tile_copy_quad_strip_to_dc(
								current_surface, /*index*/0, surface, i, j,
								texture_tiling);
							current_surface->n_pts1 = 1;
							return_code = tile_and_bin_GT_surface_quads(current_surface,
								texture_tiling, surface_tiles,
								overlap_range, initial_points, number_of_tiles);
							DESTROY(GT_surface)(&current_surface);
						}
						else
						{
							/* Make a surface to store our triangles as we split them */
							current_surface = tile_create_GT_surface(surface,
								initial_points, /*tile_number*/0, /*polygon_size*/3);
							/* Make two triangles out of the quad */
							tile_copy_vertices_to_triangle(
								current_surface, /*vertex_index*/0, surface,
								index1, index2, index3);
							tile_copy_vertices_to_triangle(
								current_surface, /*vertex_index*/3, surface,
								index2, index4, index3);
							current_surface->n_pts1 = 2;
							return_code = tile_and_bin_GT_surface_triangles(current_surface,
								texture_tiling, triangle_tiles,
								overlap_range, initial_points, number_of_tiles);
							DESTROY(GT_surface)(&current_surface);
						}						
					}
				}
			} break;
			case g_TRIANGLE:
			{
				index1 = 0;
				index2 = npts1;				
				for (i=npts1-1;return_code && (i>0);i--)
				{
					for (j=i;return_code && (j>0);j--)
					{
						index3 = index1+1;
						if (j > 1)
						{
							kmax = 2;
						}
						else
						{
							kmax = 1;
						}
						for (k = 0 ; k < kmax ; k++)
						{
							/* Determine which tile each vertex belongs too */
							vertex1 = tile_select_tile_bin(texturepoints[index1],
								texture_tiling, /*with_modulus*/0, overlap_range);
							vertex2 = tile_select_tile_bin(texturepoints[index2],
								texture_tiling, /*with_modulus*/0, overlap_range);
							vertex3 = tile_select_tile_bin(texturepoints[index3],
								texture_tiling, /*with_modulus*/0, overlap_range);
							if ((vertex1 == vertex2) &&
								(vertex1 == vertex3))
							{
								vertex1 = tile_select_tile_bin(texturepoints[index1],
									texture_tiling, /*with_modulus*/1, overlap_range);
								current_surface = tile_create_or_get_tile_bin(
									triangle_tiles, vertex1, initial_points,
									/*polygon_size*/3, surface);
								if (current_surface)
								{
									index = 3 * current_surface->n_pts1;
									tile_copy_vertices_to_triangle(
										current_surface, index, surface,
										index1, index2, index3);
									current_surface->n_pts1++;
								}
								else
								{
									return_code = 0;
								}
							}
							else
							{
								/* Make a surface to store our triangles as we split them */
								current_surface = tile_create_GT_surface(surface,
									initial_points, 3, /*polygon_size*/3);
								tile_copy_vertices_to_triangle(
									current_surface, /*vertex_index*/0, surface,
									index1, index2, index3);
								current_surface->n_pts1 = 1;
								return_code = tile_and_bin_GT_surface_triangles(current_surface,
									texture_tiling, triangle_tiles,
									overlap_range, initial_points, number_of_tiles);
								DESTROY(GT_surface)(&current_surface);
							}

							if (k == 0)
							{
								index1++;
								index3 = index2+1;
							}
						}
						index2++;
					}
					index1++;
				}
			} break;
			default:
			{
				/* Surface type not handled */
				return_code = 0;
			}
		}
		current_surface = (struct GT_surface *)NULL;
		return_surface = (struct GT_surface *)NULL;
		for (i = 0 ; return_code && (i < number_of_tiles) ; i++)
		{
			if (surface_tiles[i])
			{
				if (current_surface)
				{
					current_surface->ptrnext = surface_tiles[i];
				}
				else
				{
					return_surface = surface_tiles[i];
				}
				current_surface = surface_tiles[i];
				index = current_surface->n_pts1 * current_surface->n_pts2;
				scale_texture_coordinates(current_surface,
					texture_tiling);
				if (index != current_surface->allocated_size)
				{
					tile_reallocate_GT_surface(current_surface,
						index);
				}
#if defined (DEBUG_CODE)
				/* Writing every surface is helpful when using valgrind so that it traps
				 * earlier.  i.e. now rather than in the opengl render.
				 */
				write_GT_surface(current_surface);				
#endif // defined (DEBUG_CODE)
			}
			if (triangle_tiles[i])
			{
				if (current_surface)
				{
					current_surface->ptrnext = triangle_tiles[i];
				}
				else
				{
					return_surface = triangle_tiles[i];
				}
				current_surface = triangle_tiles[i];
				index = current_surface->n_pts1 * current_surface->n_pts2;
				scale_texture_coordinates(current_surface,
					texture_tiling);
				if (index != current_surface->allocated_size)
				{
					tile_reallocate_GT_surface(current_surface,
						index);
				}
#if defined (DEBUG_CODE)
				write_GT_surface(current_surface);				
#endif // defined (DEBUG_CODE)
			}
		}
		DEALLOCATE(surface_tiles);
		DEALLOCATE(triangle_tiles);
		if (!return_code)
		{
			return_surface=(struct GT_surface *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"tile_GT_surface. Invalid argument(s)");
		return_surface=(struct GT_surface *)NULL;
	}
	LEAVE;

	return (return_surface);
} /* tile_GT_surface */


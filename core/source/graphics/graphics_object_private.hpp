/*******************************************************************************
FILE : graphics_object_private.h

LAST MODIFIED : 18 June 2004

DESCRIPTION :
Graphical object data structures.

HISTORY :
Used to be gtypes.h
7 June 1994
	Merged GTTEXT into GTPOINT and GTPOINTSET and added a marker type and a marker
	size.
4 February 1996
	Added time dependence for gtObject.
16 February 1996
	Added graphical finite element structure (code yet to be done).
24 February 1996
	Separated out user defined objects and put in userdef_object.h
4 June 1996
	Replaced gtObjectListItem by struct LIST(gtObject)
5 June 1996
	Changed gtObject to GT_object
11 January 1997
	Added pointers to the nodes in a GTPOINTSET.  This is a temporary measure to
	allow the graphical_node_editor to work (will be replaced by the graphical
	FE_node)
30 June 1997
	Added macros/functions for safer access to graphics objects. Should convert
	rest of code to use them, so that members of graphics_objects are private.
13 October 1998
	SAB Added a callback mechanism so that through GT_object_changed interested
	objects can automatically be notified of changes (i.e. the Scene_object and
	consequently the Scene).
18 June 2004
   SAB Spilt GT_object out into graphics_object_private.h so that we control
   which files can see the guts.
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
#if !defined (GRAPHICS_OBJECT_PRIVATE_H)
#define GRAPHICS_OBJECT_PRIVATE_H


#include "zinc/zincconfigure.h"

#include "general/geometry.h"
#include "general/list.h"
#include "graphics/auxiliary_graphics_types.h"
#include "graphics/graphics_library.h"
#include "graphics/graphics_object.h"
#include "graphics/material.h"
#include "graphics/selected_graphic.h"
#include "graphics/spectrum.h"
#include "graphics/graphics_object.hpp"
#include "graphics/graphics_object_highlight.hpp"
/*
Global types
------------
*/

struct GT_glyph_set
/*******************************************************************************
LAST MODIFIED : 7 June 2001

DESCRIPTION :
Graphics primitive for positioning and scaling a glyph - a graphics object with
no materials that merely defines geometry. Glyphs should be designed to be
centred at 0,0,0 and oriented with the 3 primary axes 1, 2 and 3. The display
list of the glyph is called after setting up a model transformation matrix that
shifts the centre and axes 1, 2 and 3 of the glyph to vectors specified in the
<point_list>, <axis1_list>, <axis2_list> and <axis3_list>, respectively. Both
scaling and rotation - not necessarily maintaining orthogonality - are supplied
by the axis vectors. An additional scaling is supplied by the <size> member
which has one component per axis. Certain glyphs, eg. arrows and cones, are able
to take advantage of negative sizes to point inwards in a given axis, eg. to
show compressive strains as -><-, and tensile strains as <-->. Data may also be
stored with this object so that the entire glyph may be coloured by a spectrum.
Hence, glyphs should not have any data themselves. An optional label may also be
drawn beside each glyph. The optional <names> array allows individual items in
the glyph_set to be identified in picking for node position/vector editing.
==============================================================================*/
{
	int number_of_points;
	Triple *axis1_list, *axis2_list, *axis3_list, *label_density_list, *point_list, *scale_list;
	char **labels;
	struct GT_object *glyph;
	int n_data_components;
	GLfloat *data;
	int label_bounds_dimension, label_bounds_components;
	ZnReal *label_bounds;
	struct Cmiss_graphics_font *font;
	/* store integer object_name eg. element number from which this object came */
	int object_name;
	/* have auxiliary_object_name for marking glyph_set for editing purposes; this
		 is necessary since for some element_point types we put the
		 top_level_element in the object_name; the graphics name of the actual
		 face/line/element in use should go here: */
	int auxiliary_object_name;
	/* names recorded per point in the set, eg. node or grid point numbers */
	int *names;
	struct GT_glyph_set *ptrnext;
}; /* struct GT_glyph_set */

struct GT_nurbs
/*******************************************************************************
LAST MODIFIED : 25 February 2000

DESCRIPTION :
==============================================================================*/
{
	int nurbsprop;

	/* surface description */
	int sorder;
	int torder;
	int sknotcnt;
	int tknotcnt;
	int maxs;
	int maxt;
	ZnReal *sknots;
	ZnReal *tknots;
	ZnReal *controlpts;
	/* normals */
	ZnReal *normal_control_points;
	/* texture coordinates */
	ZnReal *texture_control_points;

	/* nurb trim curve */
	int corder;
	int cknotcnt;
	int ccount;
	ZnReal *cknots;
	ZnReal *trimarray;

	/* piecewise linear trim curve */
	int pwlcnt;
	ZnReal *pwlarray;

	/* store integer object_name eg. element number from which this object came */
	int object_name;

	struct GT_nurbs *ptrnext;
}; /* struct GT_nurbs */

struct GT_point
/*******************************************************************************
LAST MODIFIED : 22 January 2003

DESCRIPTION :
==============================================================================*/
{
	Triple *position;
	char *text;
	gtMarkerType marker_type;
	ZnReal marker_size;
	int n_data_components;
	GLfloat *data;
	struct Cmiss_graphics_font *font;

	/* store integer object_name eg. node number from which this object came */
	int object_name;

	struct GT_point *ptrnext;
}; /* struct GT_point */

struct GT_pointset
/*******************************************************************************
LAST MODIFIED : 22 January 2003

DESCRIPTION :
???RC.  integer names added for OpenGL picking.
==============================================================================*/
{
	int n_pts;
	Triple *pointlist;
	char **text;
	gtMarkerType marker_type;
	ZnReal marker_size;
	int n_data_components;
	GLfloat *data;
	struct Cmiss_graphics_font *font;

	/* store integer object_name eg. element number from which this object came */
	int object_name;

	/* names are usually node numbers (they were accessed nodes, but this
		 prevented the nodes from being destroyed) */
	int *names;
	struct GT_pointset *ptrnext;
}; /* struct GT_pointset */

struct GT_polyline
/*******************************************************************************
LAST MODIFIED : 16 April 1999

DESCRIPTION :
==============================================================================*/
{
	enum GT_polyline_type polyline_type;
	int n_pts;
	/* If non zero then this specifies to use a non default pixel line width */
	int line_width;
	int n_data_components;
	Triple *pointlist;
	Triple *normallist;
	GLfloat *data;
	/* store integer object_name eg. element number from which this object came */
	int object_name;
	struct GT_polyline *ptrnext;
}; /* struct GT_polyline */

/***************************************************************************//**
 * Provides the rendition information for the lines stored in the
 * vertex_array.
 */
struct GT_polyline_vertex_buffers
{
	enum GT_polyline_type polyline_type;
	/** If non zero then this specifies to use a non default pixel line width */
	int line_width;
}; /* struct GT_polyline_vertex_buffers */

struct GT_surface
/*******************************************************************************
LAST MODIFIED : 16 April 1999

DESCRIPTION :
==============================================================================*/
{
	enum GT_surface_type surface_type;
	enum Cmiss_graphics_render_type render_type;
	gtPolygonType polygon;
	int n_data_components;
	int n_pts1;
	int n_pts2;
	Triple *pointlist;
	Triple *normallist;
	Triple *tangentlist;
	Triple *texturelist;
	GLfloat *data;
	/* store integer object_name eg. element number from which this object came */
	int object_name;
	int tile_number;
	int allocated_size;
	struct GT_surface *ptrnext;
}; /* struct GT_surface */

struct GT_userdef
/*******************************************************************************
LAST MODIFIED : 22 January 2003

DESCRIPTION :
User defined graphics object primitive type. Contains three parameters:
- void pointer to data for user defined graphics;
- pointer to function used to destroy user data when the GT_userdef is
	destroyed.
- pointer to function for rendering the user defined graphic passing data as
	a parameter.
???RC May wish to add time to render function.
???RC This structure could/should be made private.
==============================================================================*/
{
	void *data;
	int (*destroy_function)(void **);
	int (*render_function)(void *);

	/* store integer object_name eg. node number from which this object came */
	int object_name;

	/* for compatibility with the other GT_objects: */
	struct GT_userdef *ptrnext;
}; /* struct GT_userdef */

struct GT_voltex
/*******************************************************************************
LAST MODIFIED : 9 November 2005

DESCRIPTION :
==============================================================================*/
{
	int number_of_vertices;
	struct VT_iso_vertex **vertex_list;

	int number_of_triangles;
	struct VT_iso_triangle **triangle_list;

	/* octree of vertex locations, used to accelerate stitching,
		NULL is not being used.  The user_data in this octree points
		to struct VT_iso_vertex objects. */
	struct Octree *vertex_octree;

	int n_data_components;
	int n_texture_coordinates;

	/* store integer object_name eg. element number from which this object came */
	int object_name;

	struct GT_voltex *ptrnext;

	/* voltex type */
	enum GT_voltex_type voltex_type;
}; /* struct GT_voltex */

union GT_primitive_list
/*******************************************************************************
LAST MODIFIED : 17 March 2003

DESCRIPTION :
Storage for a single linked list of primitives of any type. The linked list
maintains a pointer to the first and last primitives so that the list can be
traversed from first to last, and additional primitives added at the end.
==============================================================================*/
{
	struct {
		struct GT_glyph_set *first, *last;
	} gt_glyph_set;
	struct {
		struct GT_nurbs *first, *last;
	} gt_nurbs;
	struct {
		struct GT_point *first, *last;
	} gt_point;
	struct {
		struct GT_pointset *first, *last;
	} gt_pointset;
	struct {
		struct GT_polyline *first, *last;
	} gt_polyline;
	/* Only one object allowed for this type */
	struct GT_polyline_vertex_buffers *gt_polyline_vertex_buffers;
	struct {
		struct GT_surface *first, *last;
	} gt_surface;
	struct {
		struct GT_userdef *first, *last;
	} gt_userdef;
	struct {
		struct GT_voltex *first, *last;
	} gt_voltex;
}; /* union GT_primitive_list */

struct GT_object
/*******************************************************************************
LAST MODIFIED : 17 March 2003

DESCRIPTION :
Graphical object data structure.
==============================================================================*/
{
	const char *name;
	enum GT_object_type object_type;
	/* for linked object */
	struct GT_object *nextobject;
	/* for inheritance */
	struct GT_object *parentobject;
	/* for selected primitives and subobjects */
	enum Graphics_select_mode select_mode;
	/* default attributes */
		/*???DB.  Default is a bit of a misnomer.  Here it means the unhighlighted
			colour */
	/* either colour or material */
	gtAttributeType default_att;
	int default_colourindex;
	struct Graphical_material *default_material, *secondary_material, 
		*selected_material;
	struct Graphics_object_callback_data *update_callback_list;
	/* spectrum */
	struct Spectrum *spectrum;
	/* the object can vary with time.  It is specified at <number_of_times>
		<times>.  The <times> are in increasing order.  The object varies linearly
		between <times> and is constant before the first and after the last */
	int number_of_times;
	ZnReal *times;

	Graphics_vertex_array *vertex_array;

	/* If the graphics object was compiled with respect to a texture
		tiling then this pointer is set to that tiling. */
	struct Texture_tiling *texture_tiling;

	union GT_primitive_list *primitive_lists;
	struct MANAGER(GT_object) *manager;
	int manager_change_status;
#if defined (OPENGL_API)
	GLuint display_list;

	GLuint vertex_array_object;

	GLuint position_vertex_buffer_object;
	GLuint position_values_per_vertex;
	GLuint colour_vertex_buffer_object;
	GLuint colour_values_per_vertex;
	GLuint normal_vertex_buffer_object;
	/* The normal pointer doesn't have a size option, must be 3. */
	GLuint texture_coordinate0_vertex_buffer_object;
	GLuint texture_coordinate0_values_per_vertex;
	
	/* For multipass rendering we use some more vertex_buffers
	 * a framebuffer and a texture. */
	unsigned int multipass_width;
	unsigned int multipass_height;
	GLuint multipass_vertex_buffer_object;
	GLuint multipass_frame_buffer_object;
	GLuint multipass_frame_buffer_texture;
#endif /* defined (OPENGL_API) */
	/* enumeration indicates whether the graphics display list is up to date */
	enum Graphics_compile_status compile_status;

	/* Custom per compile code for graphics_objects used as glyphs. */
	Graphics_object_glyph_labels_function glyph_labels_function;

	/*???temporary*/
	int glyph_mirror_mode;
	int access_count;
};

struct GT_object_compile_context
/*******************************************************************************
LAST MODIFIED : 19 November 2007

DESCRIPTION :
Data used to control the compilation fo the GT_object.
==============================================================================*/
{ 
	ZnReal time;
	struct Graphics_buffer *graphics_buffer;
	int draw_selected;

#if defined (OPENGL_API)
	/* Execute this display list to shift to the ndc coordinate system */
	GLuint ndc_display_list;
	/* Execute this display list to return to the standard coordinate system */
	GLuint end_ndc_display_list;
#endif /* defined (OPENGL_API) */

	/* If <texture_tiling> is not NULL, then the primary texture was too 
		large for the current hardware and has been tiled into several textures.
		The information about the tile boundaries is in the <texture_tiling> 
		structure and should be used to compile any graphics objects. */
	struct Texture_tiling *texture_tiling;
};

class Render_graphics_compile_members;

int Graphics_object_compile_members(GT_object *graphics_object_list,
	Render_graphics_compile_members *renderer);

#endif /* ! defined (GRAPHICS_OBJECT_PRIVATE_H) */

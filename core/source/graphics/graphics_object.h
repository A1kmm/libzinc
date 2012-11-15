/*******************************************************************************
FILE : graphics_object.h

LAST MODIFIED : 30 April 2003

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
#if !defined (GRAPHICS_OBJECT_H)
#define GRAPHICS_OBJECT_H


#include "zinc/zincconfigure.h"

#include "zinc/graphic.h"
#include "general/geometry.h"
#include "general/list.h"
#include "general/manager.h"
#include "graphics/auxiliary_graphics_types.h"
#include "graphics/material.h"
#include "graphics/selected_graphic.h"
#include "graphics/spectrum.h"
#include "graphics/volume_texture.h"
#if defined (USE_OPENCASCADE)
#include "zinc/fieldcad.h"
#endif /* defined (USE_OPENCASCADE) */

/*
Global types
------------
*/

struct Graphical_material;
struct Cmiss_graphics_font;
struct Cmiss_scene;

typedef enum
/*******************************************************************************
LAST MODIFIED : 16 February 1996

DESCRIPTION :
==============================================================================*/
{
	g_SIMPLE,
	g_LINKED
} gtObjectLinkType;

enum GT_object_type
/*******************************************************************************
LAST MODIFIED : 13 August 1998

DESCRIPTION :
Must ensure all the types defined here are handled by function
get_GT_object_type_string.
Have members BEFORE_FIRST and AFTEGraphics_object_range_structR_LAST to enable iterating through the list
without knowing which order the types are in.
==============================================================================*/
{
	g_OBJECT_TYPE_INVALID,
	g_OBJECT_TYPE_BEFORE_FIRST,
	g_GLYPH_SET,
	g_NURBS,
	g_POINT,
	g_POINTSET,
	g_POLYLINE,
	g_SURFACE,
	g_USERDEF,
	g_VOLTEX,
	g_POLYLINE_VERTEX_BUFFERS,
	g_OBJECT_TYPE_AFTER_LAST
};

enum GT_visibility_type
/*******************************************************************************
LAST MODIFIED : 22 September 1997

DESCRIPTION :
Flag for visibility of GT_objects when drawn on windows.
==============================================================================*/
{
	g_INVISIBLE,
	g_VISIBLE
}; /* enum GT_visibility_type */

typedef enum
/*******************************************************************************
LAST MODIFIED : 26 June 1998

DESCRIPTION :
==============================================================================*/
{
	g_GENERAL_POLYGON,
	g_QUADRILATERAL,
	g_TRIANGLE
} gtPolygonType;

enum GT_surface_type
/*******************************************************************************
LAST MODIFIED : 13 August 1998

DESCRIPTION :
Must ensure all the types defined here are handled by functions:
  get_GT_surface_type_string      (in: graphics_object.c)
  get_GT_surface_type_from_string (in: graphics_object.c)
  makegtobj                       (in: makegtobj.c)
  file_read_graphics_objects      (in: import_graphics_object.c)
Have members BEFORE_FIRST and AFTER_LAST to enable iterating through the list
without knowing which order the types are in.
==============================================================================*/
{
	g_SURFACE_TYPE_INVALID,
	g_SURFACE_TYPE_BEFORE_FIRST,
	g_SHADED, /* old 0 */
	g_SH_DISCONTINUOUS, /* old 3 */
	g_SHADED_TEXMAP, /* old 6 */
	g_SH_DISCONTINUOUS_TEXMAP, /* old 7 */
	g_SH_DISCONTINUOUS_STRIP,
	g_SH_DISCONTINUOUS_STRIP_TEXMAP,
	g_SURFACE_TYPE_AFTER_LAST
};

enum GT_voltex_type
/*******************************************************************************
LAST MODIFIED : 3 May 2000

DESCRIPTION :
==============================================================================*/
{
	g_VOLTEX_TYPE_INVALID,
	g_VOLTEX_TYPE_BEFORE_FIRST,
	g_VOLTEX_SHADED_TEXMAP,
	g_VOLTEX_WIREFRAME_SHADED_TEXMAP,
	g_VOLTEX_TYPE_AFTER_LAST
};

enum GT_polyline_type
/*******************************************************************************
LAST MODIFIED : 13 August 1998

DESCRIPTION :
Must ensure all the types defined here are handled by functions:
  get_GT_polyline_type_string      (in: graphics_object.c)
  get_GT_polyline_type_from_string (in: graphics_object.c)
  makegtobj                        (in: makegtobj.c)
  file_read_graphics_objects       (in: import_graphics_object.c)
Have members BEFORE_FIRST and AFTER_LAST to enable iterating through the list
without knowing which order the types are in.
==============================================================================*/
{
	g_POLYLINE_TYPE_INVALID,
	g_POLYLINE_TYPE_BEFORE_FIRST,
	g_PLAIN, /* old 0 */
	g_NORMAL, /* old 1 */
	g_PLAIN_DISCONTINUOUS, /* old 2 */
	g_NORMAL_DISCONTINUOUS, /* old 3 */
	g_POLYLINE_TYPE_AFTER_LAST
}; /* enum GT_polyline_type */

typedef enum
/*******************************************************************************
LAST MODIFIED : 16 February 1996

DESCRIPTION :
==============================================================================*/
{
	g_RGBCOLOUR,
	g_MATERIAL
} gtAttributeType;

typedef enum
/*******************************************************************************
LAST MODIFIED : 16 February 1996

DESCRIPTION :
==============================================================================*/
{
	g_NOT_CREATED,
	g_CREATED
} gtStatusType;

typedef enum
/*******************************************************************************
LAST MODIFIED : 6 July 1998

DESCRIPTION :
If the gtTransformType of a graphics object is g_NOT_ID it will use its
transformation matrix, otherwise no matrix manipulations are output with it.
Note that if the gtInheritanceType of the graphics object is g_CHILD, the
transformation step is bypassed anyway.
==============================================================================*/
{
	g_ID,
	g_NOT_ID
} gtTransformType;

typedef enum
/*******************************************************************************
LAST MODIFIED : 16 February 1996

DESCRIPTION :
==============================================================================*/
{
	g_PARENT,
	g_CHILD
} gtInheritanceType;

typedef enum
/*******************************************************************************
LAST MODIFIED : 16 February 1996

DESCRIPTION :
==============================================================================*/
{
	g_NO_DATA,
	g_SCALAR,
	g_TWO_COMPONENTS,
	g_VECTOR,
	g_VECTOR4
} ZnRealType;

typedef enum
/*******************************************************************************
LAST MODIFIED : 23 February 1998

DESCRIPTION :
==============================================================================*/
{
	g_NO_MARKER,
	g_POINT_MARKER,
	g_PLUS_MARKER,
	g_DERIVATIVE_MARKER
} gtMarkerType;

struct GT_glyph_set;
struct GT_nurbs;
struct GT_point;
struct GT_pointset;
struct GT_polyline;
struct GT_polyline_vertex_buffers;
struct GT_surface;
struct GT_userdef;
struct GT_voltex;

struct GT_object;

typedef int(*Graphics_object_callback)(struct GT_object *graphics_object,
	void *user_data);

struct Graphics_object_callback_data
{
	Graphics_object_callback callback;
	void *callback_user_data;
	struct Graphics_object_callback_data *next;
}; /* struct Graphics_object_callback_data */

typedef struct GT_object gtObject;

DECLARE_LIST_TYPES(GT_object);

DECLARE_MANAGER_TYPES(GT_object);

struct Graphics_object_range_struct
/*******************************************************************************
LAST MODIFIED : 6 August 1997

DESCRIPTION :
Structure for storing range of time in one or several graphics objects.
Set first=1 before calling range routines. Only if first==0 afterwards is the
range valid.
==============================================================================*/
{
	int first;
	struct Cmiss_scene *scene;
	Triple maximum,minimum;
}; /* Graphics_object_range_struct */

struct Graphics_object_data_range_struct
/*******************************************************************************
LAST MODIFIED : 29 October 1997

DESCRIPTION :
Structure for storing range of data in one or several graphics objects.
Set first=1 before calling data range routines. Only if first==0 afterwards is
the data range valid.
???RC likely to change when multiple data values stored in graphics_objects
for combining several spectrums.
==============================================================================*/
{
	int first;
	ZnReal minimum,maximum;
}; /* Graphics_object_data_range_struct */

struct Graphics_object_time_range_struct
/*******************************************************************************
LAST MODIFIED : 6 August 1997

DESCRIPTION :
Structure for storing range of time in one or several graphics objects.
Set first=1 before calling time range routines. Only if first==0 afterwards is
the time range valid.
==============================================================================*/
{
	int first;
	ZnReal minimum,maximum;
}; /* Graphics_object_time_range_struct */

/*
Global variables
----------------
*/
/*???DB.  I'm not sure that this should be here */
extern ZnReal global_line_width,global_point_size;

/*
Global functions
----------------
*/
const char *GT_object_get_name(struct GT_object *gt_object);

enum GT_object_type GT_object_get_type(struct GT_object *gt_object);
/*******************************************************************************
LAST MODIFIED : 17 June 2004

DESCRIPTION :
Returns the object type from the gt_object.
==============================================================================*/

struct GT_object *GT_object_get_next_object(struct GT_object *gt_object);
/*******************************************************************************
LAST MODIFIED : 17 June 2004

DESCRIPTION :
Returns the next object from the gt_object.
==============================================================================*/

int GT_object_set_next_object(struct GT_object *gt_object,
	struct GT_object *next_object);
/*******************************************************************************
LAST MODIFIED : 17 June 2004

DESCRIPTION :
Sets the next object for the gt_object.
==============================================================================*/

int GT_object_compare_name(struct GT_object *gt_object,
	char *name);
/*******************************************************************************
LAST MODIFIED : 17 June 2004

DESCRIPTION :
Returns true if the name of the <gt_object> matches the string <name> exactly.
==============================================================================*/

const char *get_GT_object_type_string(enum GT_object_type object_type);
/*******************************************************************************
LAST MODIFIED : 13 August 1998

DESCRIPTION :
Returns a string describing the object type, suitable for writing to file
(and reinterpreting it later).
==============================================================================*/

int get_GT_object_type_from_string(char *type_string,
	enum GT_object_type *object_type);
/*******************************************************************************
LAST MODIFIED : 13 August 1998

DESCRIPTION :
Returns the object type from the string produced by function
get_GT_object_type_string. For compatibility, also supports converting old
enumerator numbers (as text) into the new enumerator values, with a warning.
==============================================================================*/

const char *get_GT_polyline_type_string(enum GT_polyline_type polyline_type);
/*******************************************************************************
LAST MODIFIED : 13 August 1998

DESCRIPTION :
Returns a string describing the polyline type, suitable for writing to file
(and reinterpreting it later).
==============================================================================*/

int get_GT_polyline_type_from_string(char *type_string,
	enum GT_polyline_type *polyline_type);
/*******************************************************************************
LAST MODIFIED : 13 August 1998

DESCRIPTION :
Returns the polyline type from the string produced by function
get_GT_polyline_type_string. For compatibility, also supports converting old
enumerator numbers (as text) into the new enumerator values, with a warning.
==============================================================================*/

const char *get_GT_surface_type_string(enum GT_surface_type surface_type);
/*******************************************************************************
LAST MODIFIED : 13 August 1998

DESCRIPTION :
Returns a string describing the surface type, suitable for writing to file
(and reinterpreting it later).
==============================================================================*/

int get_GT_surface_type_from_string(char *type_string,
	enum GT_surface_type *surface_type);
/*******************************************************************************
LAST MODIFIED : 13 August 1998

DESCRIPTION :
Returns the surface type from the string produced by function
get_GT_surface_type_string. For compatibility, also supports converting old
enumerator numbers (as text) into the new enumerator values, with a warning.
==============================================================================*/

struct GT_glyph_set *morph_GT_glyph_set(ZnReal proportion,
	struct GT_glyph_set *initial,struct GT_glyph_set *final);
/*******************************************************************************
LAST MODIFIED : 7 July 1998

DESCRIPTION :
Creates a new GT_glyph_set which is the interpolation of two GT_glyph_sets.
The two glyph_sets must have the same glyph and data_type.
==============================================================================*/

struct GT_pointset *morph_GT_pointset(ZnReal proportion,
	struct GT_pointset *initial,struct GT_pointset *final);
/*******************************************************************************
LAST MODIFIED : 19 June 1998

DESCRIPTION :
Creates a new GT_pointset which is the interpolation of two GT_pointsets.
==============================================================================*/

struct GT_polyline *morph_GT_polyline(ZnReal proportion,
	struct GT_polyline *initial,struct GT_polyline *final);
/*******************************************************************************
LAST MODIFIED : 6 February 1996

DESCRIPTION :
Creates a new GT_polyline which is the interpolation of two GT_polylines.
==============================================================================*/

struct GT_surface *morph_GT_surface(ZnReal proportion,
	struct GT_surface *initial,struct GT_surface *final);
/*******************************************************************************
LAST MODIFIED : 19 June 1998

DESCRIPTION :
Creates a new GT_surface which is the interpolation of two GT_surfaces.
==============================================================================*/

gtObject *morph_gtObject(char *name,ZnReal proportion,gtObject *initial,
	gtObject *final);
/*******************************************************************************
LAST MODIFIED : 27 December 1995

DESCRIPTION :
Creates a new gtObject which is the interpolation of two gtObjects.
==============================================================================*/

struct GT_object *transform_GT_object(struct GT_object *object,
	ZnReal *transformation);
/*******************************************************************************
LAST MODIFIED : 8 July 1999

DESCRIPTION :
Creates a new GT_object which is the transformation of <object>.
Only surfaces are implemented at the moment.
Normals are not updated (wavefront export doesn't use normals anyway).
==============================================================================*/

Triple *surfalloc(int,int,int);
/*******************************************************************************
LAST MODIFIED : 21 March 1993

DESCRIPTION :
==============================================================================*/

Triple *linealloc(int,int);
/*******************************************************************************
LAST MODIFIED : 21 March 1993

DESCRIPTION :
==============================================================================*/

PROTOTYPE_OBJECT_FUNCTIONS(GT_object);
PROTOTYPE_GET_OBJECT_NAME_FUNCTION(GT_object);

PROTOTYPE_LIST_FUNCTIONS(GT_object);
PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(GT_object,name,const char *);

PROTOTYPE_MANAGER_COPY_FUNCTIONS(GT_object,name,const char *);
PROTOTYPE_MANAGER_FUNCTIONS(GT_object);
PROTOTYPE_MANAGER_IDENTIFIER_FUNCTIONS(GT_object,name,const char *);

struct GT_glyph_set *CREATE(GT_glyph_set)(int number_of_points,
	Triple *point_list, Triple *axis1_list, Triple *axis2_list,
	Triple *axis3_list, Triple *scale_list, struct GT_object *glyph,
	struct Cmiss_graphics_font *font, char **labels, int n_data_components, GLfloat *data,
	int label_bounds_dimension, int label_bounds_components, ZnReal *label_bounds,
	Triple *label_density_list,	int object_name, int *names);
/*******************************************************************************
Allocates memory and assigns fields for a GT_glyph_set. The glyph set shows
the object <glyph> at the specified <number_of_points> with positions given in
<point_list>, and principal axes in <axis1_list>, <axis2_list> and <axis3_list>.
The magnitude of these axes control scaling of the glyph at each point, while
their orientations - which need not be orthogonal - effect rotations and skew.
There magnitudes also multiplied by the <scale_list> values, 1 value per axis,
which permit certain glyphs to reverse direction with negative values.
The optional <labels> parameter is an array of strings to be written beside each
glyph, while the optional <data> of number <n_data_components> per glyph allows
colouring of the glyphs by a spectrum.
The optional <label_density_list> controls the number of labels in each direction
for glyphs that output more than one label per glyph such as axes or graph grids.
The glyph_set will be marked as coming from the <object_name>, and integer
identifier, while the optional <names> contains an integer identifier per point.
Note: All arrays passed to this routine are owned by the new GT_glyph_set
and are deallocated by its DESTROY function.
==============================================================================*/

int DESTROY(GT_glyph_set)(struct GT_glyph_set **glyph_set_address);
/*******************************************************************************
LAST MODIFIED : 16 November 2000

DESCRIPTION :
Frees the frees the memory for <**glyph_set_address> and sets
<*glyph_set_address> to NULL.
==============================================================================*/

int GT_glyph_set_set_integer_identifier(struct GT_glyph_set *glyph_set,
	int identifier);
/*******************************************************************************
LAST MODIFIED : 18 June 2004

DESCRIPTION :
Sets the integer identifier used by the graphics to distinguish this object.
==============================================================================*/

int GT_glyph_set_set_auxiliary_integer_identifier(struct GT_glyph_set *glyph_set,
	int identifier);
/*******************************************************************************
LAST MODIFIED : 18 June 2004

DESCRIPTION :
Sets the extra integer identifier used by the graphics to distinguish this object.
==============================================================================*/

struct GT_nurbs *CREATE(GT_nurbs)(void);
/*******************************************************************************
LAST MODIFIED : 10 March 1999

DESCRIPTION :
Creates a default GT_nurbs object.
==============================================================================*/

int DESTROY(GT_nurbs)(struct GT_nurbs **nurbs);
/*******************************************************************************
LAST MODIFIED : 19 June 1998

DESCRIPTION :
Frees the memory for <**nurbs> and its fields and sets <*nurbs> to NULL.
==============================================================================*/

int GT_nurbs_set_integer_identifier(struct GT_nurbs *nurbs,
	int identifier);
/*******************************************************************************
LAST MODIFIED : 18 June 2004

DESCRIPTION :
Sets the integer identifier used by the graphics to distinguish this object.
==============================================================================*/

int GT_nurbs_set_surface(struct GT_nurbs *nurbs,
	int sorder, int torder, int sknotcount, int tknotcount,
	ZnReal *sknots, ZnReal *tknots,
	int scontrolcount, int tcontrolcount, ZnReal *control_points);
/*******************************************************************************
LAST MODIFIED : 10 March 1999

DESCRIPTION :
Sets the surface in a GT_nurbs structure.
There must be sknotcount values in sknots,
tknotcount values in tknots and scontrolcount * tcontrolcount values
in the controlpoints, with the s direction varying more quickly.
The arrays are assigned directly to the object and not copied.
==============================================================================*/

int GT_nurbs_set_nurb_trim_curve(struct GT_nurbs *nurbs,
	int order, int knotcount, ZnReal *knots,
	int control_count, ZnReal *control_points);
/*******************************************************************************
LAST MODIFIED : 10 March 1999

DESCRIPTION :
Sets a Nurb curve used to trim the Nurbs surface in a GT_nurbs structure.
The arrays are assigned directly to the object and not copied.
==============================================================================*/

int GT_nurbs_set_piecewise_linear_trim_curve(struct GT_nurbs *nurbs,
	int number_of_points, ZnReal *points);
/*******************************************************************************
LAST MODIFIED : 10 March 1999

DESCRIPTION :
Sets a piecewise linear curve used to trim the Nurbs surface in a GT_nurbs structure.
The array is assigned directly to the object and not copied.
==============================================================================*/

int GT_nurbs_set_normal_control_points(struct GT_nurbs *nurbs,
	ZnReal *normal_control_points);
/*******************************************************************************
LAST MODIFIED : 10 March 1999

DESCRIPTION :
Sets the control point array for the normals.  The array is assigned directly and
the number of points is assumed to be the same as the geometry control
points specified in set surface.  Each normal is assumed to have three components.
==============================================================================*/

int GT_nurbs_set_texture_control_points(struct GT_nurbs *nurbs,
	ZnReal *texture_control_points);
/*******************************************************************************
LAST MODIFIED : 10 March 1999

DESCRIPTION :
Sets the control point array for the texture.  The array is assigned directly and
the number of points is assumed to be the same as the geometry control
points specified in set surface.  Each point is assumed to have three
texture coordinates.
==============================================================================*/

struct GT_point *CREATE(GT_point)(Triple *position,char *text,
	gtMarkerType marker_type,ZnReal marker_size,int n_data_components,
	int object_name, GLfloat *data, struct Cmiss_graphics_font *font);
/*******************************************************************************
LAST MODIFIED : 18 November 2005

DESCRIPTION :
Allocates memory and assigns fields for a GT_point.  When the <marker_type> is
g_DERIVATIVE_MARKER, there should be 4 points in <pointlist> - first point is
for the <node>, next point is the end point for the xi1 derivative axis, etc.
If the end point is the same as the node point it is assumed that there isn't a
derivative in that xi direction.
==============================================================================*/

int DESTROY(GT_point)(struct GT_point **point);
/*******************************************************************************
LAST MODIFIED : 19 June 1998

DESCRIPTION :
Frees the frees the memory for <**point> and sets <*point> to NULL.
==============================================================================*/

int GT_point_set_integer_identifier(struct GT_point *point,
	int identifier);
/*******************************************************************************
LAST MODIFIED : 18 June 2004

DESCRIPTION :
Sets the integer identifier used by the graphics to distinguish this object.
==============================================================================*/

struct GT_pointset *CREATE(GT_pointset)(int n_pts,Triple *pointlist,char **text,
	gtMarkerType marker_type,ZnReal marker_size,int n_data_components,GLfloat *data,
	int *names, struct Cmiss_graphics_font *font);
/*******************************************************************************
LAST MODIFIED : 18 November 2005

DESCRIPTION :
Allocates memory and assigns fields for a GT_pointset.  When the <marker_type>
is g_DERIVATIVE_MARKER, there should be 4*<n_pts> points in <pointlist> - in
each group of four points the first for the nodes, the next is the end points
for the xi1 derivative axis, etc.  If the end point is the same as the node
point it is assumed that there isn't a derivative in that xi direction.
==============================================================================*/

int DESTROY(GT_pointset)(struct GT_pointset **pointset);
/*******************************************************************************
LAST MODIFIED : 19 June 1998

DESCRIPTION :
Frees the frees the memory for <**pointset> and sets <*pointset> to NULL.
==============================================================================*/

int GT_pointset_set_integer_identifier(struct GT_pointset *point_set,
	int identifier);
/*******************************************************************************
LAST MODIFIED : 18 June 2004

DESCRIPTION :
Sets the integer identifier used by the graphics to distinguish this object.
==============================================================================*/

int GT_pointset_get_point_list(struct GT_pointset *pointset, int *number_of_points,
	Triple **positions);
/*******************************************************************************
LAST MODIFIED : 18 June 2004

DESCRIPTION :
SAB Added to allow GT_pointset to be hidden but should be replaced.
Gets the <number_of_points> and the <positions> of those points into the
<pointset> object.  The <positions> pointer is copied directly from the internal
storage.
==============================================================================*/

int GT_pointset_set_point_list(struct GT_pointset *pointset, int number_of_points,
	Triple *positions);
/*******************************************************************************
LAST MODIFIED : 18 June 2004

DESCRIPTION :
SAB Added to allow GT_pointset to be hidden but should be replaced.
Sets the <number_of_points> and the <positions> of those points into the
<pointset> object.  The <positions> pointer is copied directly overwriting the
current storage and the internal data, text and names arrays are messed up.
==============================================================================*/

struct GT_polyline *CREATE(GT_polyline)(enum GT_polyline_type polyline_type,
	int line_width, int n_pts,Triple *pointlist,Triple *normallist,
	int n_data_components,GLfloat *data);
/*******************************************************************************
LAST MODIFIED : 22 April 2004

DESCRIPTION :
Allocates memory and assigns fields for a graphics polyline.
==============================================================================*/

int DESTROY(GT_polyline)(struct GT_polyline **polyline);
/*******************************************************************************
LAST MODIFIED : 19 June 1998

DESCRIPTION :
Frees the memory for <**polyline> and its fields and sets <*polyline> to NULL.
==============================================================================*/

/***************************************************************************//**
 * Creates the shared rendition information for a GT_polyline_vertex_buffers.
 */
struct GT_polyline_vertex_buffers *CREATE(GT_polyline_vertex_buffers)(
	enum GT_polyline_type polyline_type, int line_width);

int GT_polyline_set_integer_identifier(struct GT_polyline *polyline,
	int identifier);
/*******************************************************************************
LAST MODIFIED : 18 June 2004

DESCRIPTION :
Sets the integer identifier used by the graphics to distinguish this object.
==============================================================================*/

struct GT_surface *CREATE(GT_surface)(enum GT_surface_type surface_type,
	enum Cmiss_graphics_render_type render_type, gtPolygonType polytype,
	int n_pts1,int n_pts2,Triple *pointlist,
	Triple *normallist, Triple *tangentlist, Triple *texturelist,
	int n_data_components,GLfloat *data);
/*******************************************************************************
LAST MODIFIED : 28 November 2003

DESCRIPTION :
Allocates memory and assigns fields for a graphics surface.
==============================================================================*/

int DESTROY(GT_surface)(struct GT_surface **surface);
/*******************************************************************************
LAST MODIFIED : 19 June 1998

DESCRIPTION :
Frees the memory for <**surface> and sets <*surface> to NULL.
==============================================================================*/

int GT_surface_set_integer_identifier(struct GT_surface *surface,
	int identifier);
/*******************************************************************************
LAST MODIFIED : 18 June 2004

DESCRIPTION :
Sets the integer identifier used by the graphics to distinguish this object.
==============================================================================*/

struct GT_userdef *CREATE(GT_userdef)(void *data,
	int (*destroy_function)(void **),int (*render_function)(void *));
/*******************************************************************************
LAST MODIFIED : 19 June 1998

DESCRIPTION :
Allocates memory and assigns fields for a user-defined primitive.
Any data required for rendering the primitive should be passed in the
void *data parameter; a destroy_function should be given if dynamically
allocated data is passed. The render function is called with the data as a
parameter to render the user-defined primitive.
==============================================================================*/

int DESTROY(GT_userdef)(struct GT_userdef **userdef);
/*******************************************************************************
LAST MODIFIED : 19 June 1998

DESCRIPTION :
Frees the memory for <**userdef> and its fields and sets <*userdef> to NULL.
==============================================================================*/

struct GT_voltex *CREATE(GT_voltex)(
	int number_of_vertices,  struct VT_iso_vertex **vertex_list,
	int number_of_triangles, struct VT_iso_triangle **triangle_list,
	int n_data_components, int n_texture_coordinates,
	enum GT_voltex_type voltex_type);
/*******************************************************************************
LAST MODIFIED : 17 February 2006

DESCRIPTION :
Allocates memory and assigns fields for a graphics volume texture.
==============================================================================*/

int DESTROY(GT_voltex)(struct GT_voltex **voltex);
/*******************************************************************************
LAST MODIFIED : 19 June 1998

DESCRIPTION :
Frees the memory for <**voltex> and sets <*voltex> to NULL.
???DB.  Free memory for fields ?
==============================================================================*/

int GT_voltex_set_integer_identifier(struct GT_voltex *voltex,
	int identifier);
/*******************************************************************************
LAST MODIFIED : 18 June 2004

DESCRIPTION :
Sets the integer identifier used by the graphics to distinguish this object.
==============================================================================*/

int GT_voltex_get_number_of_triangles(struct GT_voltex *voltex);
/*******************************************************************************
LAST MODIFIED : 18 June 2004

DESCRIPTION :
Returns the number of polygons used in the GT_voltex.
==============================================================================*/

struct VT_iso_triangle **GT_voltex_get_triangle_list(struct GT_voltex *voltex);
/*******************************************************************************
LAST MODIFIED : 10 November 2005

DESCRIPTION :
Returns the internal pointer to the triangles used in the GT_voltex.
==============================================================================*/

int GT_voltex_get_number_of_vertices(struct GT_voltex *voltex);
/*******************************************************************************
LAST MODIFIED : 18 June 2004

DESCRIPTION :
Returns the number of vertices used in the GT_voltex.
==============================================================================*/

struct VT_iso_vertex **GT_voltex_get_vertex_list(struct GT_voltex *voltex);
/*******************************************************************************
LAST MODIFIED : 10 November 2005

DESCRIPTION :
Returns the internal pointer to the list of vertices used in the GT_voltex.
==============================================================================*/

int GT_voltex_set_integer_identifier(struct GT_voltex *voltex,
	int identifier);
/*******************************************************************************
LAST MODIFIED : 18 June 2004

DESCRIPTION :
Sets the integer identifier used by the graphics to distinguish this object.
==============================================================================*/

struct GT_object *CREATE(GT_object)(const char *name,enum GT_object_type object_type,
	struct Graphical_material *default_material);
/*******************************************************************************
LAST MODIFIED : 18 November 2005

DESCRIPTION :
Allocates memory and assigns fields for a graphics object.
==============================================================================*/

int DESTROY(GT_object)(struct GT_object **object_ptr);
/*******************************************************************************
LAST MODIFIED : 19 June 1998

DESCRIPTION :
Frees the memory for the fields of <**object>, frees the memory for <**object>
and sets <*object> to NULL.
==============================================================================*/

int GT_object_changed(struct GT_object *graphics_object);
/*******************************************************************************
LAST MODIFIED : 12 March 2002

DESCRIPTION :
External modules that change a GT_object should call this routine so that
objects interested in this GT_object will be notified that is has changed.
==============================================================================*/

int GT_object_Graphical_material_change(struct GT_object *graphics_object,
	struct LIST(Graphical_material) *changed_material_list);
/*******************************************************************************
LAST MODIFIED : 12 March 2002

DESCRIPTION :
Tells the <graphics_object> that the materials in the <changed_material_list>
have changed. If any of these materials are used in any graphics object,
changes the compile_status to CHILD_GRAPHICS_NOT_COMPILED and
informs clients of the need to recompile and redraw. Note that if a spectrum is
in use the more expensive GRAPHICS_NOT_COMPILED status is necessarily set.
Note: Passing a NULL <changed_material_list> indicates the equivalent of a
change to any material in use in the linked graphics objects.
==============================================================================*/

int GT_object_Spectrum_change(struct GT_object *graphics_object,
	struct LIST(Spectrum) *changed_spectrum_list);
/*******************************************************************************
LAST MODIFIED : 12 March 2002

DESCRIPTION :
Tells the <graphics_object> that the spectrums in the <changed_spectrum_list>
have changed. If any of these spectrums are used in any graphics object,
changes the compile_status to GRAPHICS_NOT_COMPILED and
informs clients of the need to recompile and redraw.
Note: Passing a NULL <changed_spectrum_list> indicates the equivalent of a
change to any spectrum in use in the linked graphics objects.
==============================================================================*/

int GT_object_add_callback(struct GT_object *graphics_object,
	Graphics_object_callback callback, void *user_data);
/*******************************************************************************
LAST MODIFIED : 13 October 1998

DESCRIPTION :
Adds a callback routine which is called whenever a GT_object is aware of
changes.  As the GT_object is not private, this relies on modules that change a
GT_object calling GT_object_changed.
==============================================================================*/

int GT_object_remove_callback(struct GT_object *graphics_object,
	Graphics_object_callback callback, void *user_data);
/*******************************************************************************
LAST MODIFIED : 13 October 1998

DESCRIPTION :
Removes a callback which was added previously
==============================================================================*/

int GT_object_has_time(struct GT_object *graphics_object,ZnReal time);
/*******************************************************************************
LAST MODIFIED : 26 June 1997

DESCRIPTION :
Returns 1 if the time parameter is used by the graphics_object.
==============================================================================*/

int GT_object_has_primitives_at_time(struct GT_object *graphics_object,
	ZnReal time);
/*******************************************************************************
LAST MODIFIED : 17 March 2003

DESCRIPTION :
Returns true if <graphics_object> has primitives stored exactly at <time>.
==============================================================================*/

/***************************************************************************//**
 * Returns the vertex buffer set if the graphics_object has one.
 */
struct Graphics_vertex_array *GT_object_get_vertex_set(struct GT_object *graphics_object);

int GT_object_get_number_of_times(struct GT_object *graphics_object);
/*******************************************************************************
LAST MODIFIED : 18 June 1998

DESCRIPTION :
Returns the number of times/primitive lists in the graphics_object.
==============================================================================*/

ZnReal GT_object_get_time(struct GT_object *graphics_object,int time_no);
/*******************************************************************************
LAST MODIFIED : 18 June 1998

DESCRIPTION :
Returns the time at <time_no> from the graphics_object.
Note that time numbers range from 1 to number_of_times.
==============================================================================*/

ZnReal GT_object_get_nearest_time(struct GT_object *graphics_object,ZnReal time);
/*******************************************************************************
LAST MODIFIED : 7 August 1997

DESCRIPTION :
Returns the nearest time to <time> in <graphics_object> at which graphics
primitives are called.
NOTE: presently finds the nearest time that is *lower* than <time>. When all
routines updated to use this, may be changed to get actual nearest time.
==============================================================================*/

int get_graphics_object_range(struct GT_object *graphics_object,
	void *graphics_object_range_void);
/*******************************************************************************
LAST MODIFIED : 8 August 1997

DESCRIPTION :
Returns the range of the coordinates in <graphics_object> or 0 if object empty
or error occurs. First should be set to 1 outside routine. Several calls to
this routine for differing graphics objects (without settings first in between)
will produce the range of all the graphics objects.
???RC only does some object types.
==============================================================================*/

int get_graphics_object_data_range(struct GT_object *graphics_object,
	void *graphics_object_data_range_void);
/*******************************************************************************
LAST MODIFIED : 29 October 1997

DESCRIPTION :
Returns the range of the data values stored in the graphics object.
Returned range generally used to set or enlarge spectrum ranges.
???RC likely to change when multiple data values stored in graphics_objects
for combining several spectrums.
==============================================================================*/

int get_graphics_object_time_range(struct GT_object *graphics_object,
	void *graphics_object_time_range_void);
/*******************************************************************************
LAST MODIFIED : 8 August 1997

DESCRIPTION :
Enlarges the minimum and maximum time range by that of the graphics_object.
==============================================================================*/

#if ! defined (SHORT_NAMES)
#define GT_OBJECT_ADD_(primitive_type) GT_object_add_ ## primitive_type
#else
#define GT_OBJECT_ADD_(primitive_type) goa_ ## primitive_type
#endif
#define GT_OBJECT_ADD(primitive_type) GT_OBJECT_ADD_(primitive_type)

#define PROTOTYPE_GT_OBJECT_ADD_FUNCTION(primitive_type) \
int GT_OBJECT_ADD(primitive_type)( \
	struct GT_object *graphics_object, \
	ZnReal time,struct primitive_type *primitive) \
/***************************************************************************** \
LAST MODIFIED : 17 March 2003 \
\
DESCRIPTION : \
Adds <primitive> to <graphics_object> at <time>, creating the new time if it \
does not already exist. If the <primitive> is NULL an empty time is added if \
there is not already one. <primitive> is a NULL-terminated linked-list. \
============================================================================*/ \

PROTOTYPE_GT_OBJECT_ADD_FUNCTION(GT_glyph_set);
PROTOTYPE_GT_OBJECT_ADD_FUNCTION(GT_nurbs);
PROTOTYPE_GT_OBJECT_ADD_FUNCTION(GT_point);
PROTOTYPE_GT_OBJECT_ADD_FUNCTION(GT_pointset);
PROTOTYPE_GT_OBJECT_ADD_FUNCTION(GT_polyline);
PROTOTYPE_GT_OBJECT_ADD_FUNCTION(GT_surface);
PROTOTYPE_GT_OBJECT_ADD_FUNCTION(GT_userdef);
PROTOTYPE_GT_OBJECT_ADD_FUNCTION(GT_voltex);

/*************************************************************************//**
 * Adds <primitive> to <graphics_object>.  There can be only one time for this
 * type
 */
int GT_OBJECT_ADD(GT_polyline_vertex_buffers)(
	struct GT_object *graphics_object, struct GT_polyline_vertex_buffers *primitive);

#if ! defined (SHORT_NAMES)
#define GT_OBJECT_GET_(primitive_type) GT_object_get_ ## primitive_type
#else
#define GT_OBJECT_GET_(primitive_type) gog_ ## primitive_type
#endif
#define GT_OBJECT_GET(primitive_type) GT_OBJECT_GET_(primitive_type)

#define PROTOTYPE_GT_OBJECT_GET_FUNCTION(primitive_type) \
struct primitive_type *GT_OBJECT_GET(primitive_type)( \
	struct GT_object *graphics_object,ZnReal time) \
/***************************************************************************** \
LAST MODIFIED : 19 June 1997 \
\
DESCRIPTION : \
Returns pointer to the primitive at the given time in graphics_object. \
???RC only used in spectrum_editor.c and should be replaced.
============================================================================*/

PROTOTYPE_GT_OBJECT_GET_FUNCTION(GT_pointset);
PROTOTYPE_GT_OBJECT_GET_FUNCTION(GT_polyline);
PROTOTYPE_GT_OBJECT_GET_FUNCTION(GT_surface);

typedef int (GT_object_primitive_object_name_conditional_function) \
	(int object_name, void *user_data);

int GT_object_remove_primitives_at_time(
	struct GT_object *graphics_object, ZnReal time,
	GT_object_primitive_object_name_conditional_function *conditional_function,
	void *user_data);
/*******************************************************************************
LAST MODIFIED : 5 February 2003

DESCRIPTION :
Removes primitives at <time> from <graphics_object>.
The optional <conditional_function> allows a subset of the primitives to
be removed. This function is called with the object_name integer associated
with each primitive plus the void *<user_data> supplied here. A true result
from the conditional_function causes the primitive to be removed.
==============================================================================*/

int GT_object_transfer_primitives_at_time(struct GT_object *destination,
	struct GT_object *source, ZnReal time);
/*******************************************************************************
LAST MODIFIED : 18 March 2003

DESCRIPTION :
Transfers the primitives stored at exactly <time> in <source> to <time> in
<destination>. Should already have called GT_object_has_primitives_at_time
with <source> to verify it has primitives at that time.
Primitives are added after any in <destination> at <time>.
==============================================================================*/

#if ! defined (SHORT_NAMES)
#define GT_OBJECT_EXTRACT_FIRST_PRIMITIVES_AT_TIME_(primitive_type) \
	GT_object_extract_first_primitives_at_time_ ## primitive_type
#else
#define GT_OBJECT_EXTRACT_FIRST_PRIMITIVES_AT_TIME_(primitive_type) \
	goefpt_ ## primitive_type
#endif
#define GT_OBJECT_EXTRACT_FIRST_PRIMITIVES_AT_TIME(primitive_type) \
	GT_OBJECT_EXTRACT_FIRST_PRIMITIVES_AT_TIME_(primitive_type)

#define PROTOTYPE_GT_OBJECT_EXTRACT_FIRST_PRIMITIVES_AT_TIME_FUNCTION( \
	primitive_type) \
struct primitive_type *GT_OBJECT_EXTRACT_FIRST_PRIMITIVES_AT_TIME( \
	primitive_type)(struct GT_object *graphics_object, \
	ZnReal time, int object_name) \
/***************************************************************************** \
LAST MODIFIED : 18 March 2003 \
\
DESCRIPTION : \
Returns the first primitives in <graphics_object> at <time> that have the \
given <object_name>, or NULL if there are no primitives or none with the name. \
The extracted primitives are returned in a linked-list. \
If the primitive type has an auxiliary_object_name, it is matched, not the \
object_name. \
============================================================================*/ \

PROTOTYPE_GT_OBJECT_EXTRACT_FIRST_PRIMITIVES_AT_TIME_FUNCTION(GT_glyph_set);
PROTOTYPE_GT_OBJECT_EXTRACT_FIRST_PRIMITIVES_AT_TIME_FUNCTION(GT_polyline);
PROTOTYPE_GT_OBJECT_EXTRACT_FIRST_PRIMITIVES_AT_TIME_FUNCTION(GT_surface);
PROTOTYPE_GT_OBJECT_EXTRACT_FIRST_PRIMITIVES_AT_TIME_FUNCTION(GT_voltex);

int GT_object_merge_GT_voltex(struct GT_object *graphics_object,
	struct GT_voltex *voltex);
/*******************************************************************************
LAST MODIFIED : 26 October 2005

DESCRIPTION :
If <graphics_object> does not already contain a GT_voltex then the <voltex> is
added in the normal way.  If a GT_voltex is already contained in the
<graphics_object> then the new <voltex> is merged into the existing one and
any co-located vertices are merged, stitching the two voltexes together.
==============================================================================*/

int GT_object_decimate_GT_voltex(struct GT_object *graphics_object,
	double threshold_distance);
/*******************************************************************************
LAST MODIFIED : 11 November 2005

DESCRIPTION :
==============================================================================*/

int GT_object_normalise_GT_voltex_normals(struct GT_object *graphics_object);
/*******************************************************************************
LAST MODIFIED : 28 October 2005

DESCRIPTION :
If a GT_voltex is contained in the <graphics_object> then normals are
normalised for each of the VT_iso_vertices using the surrounding triangles.
==============================================================================*/

/***************************************************************************//**
 * Performs polygon reduction by converting surface into a GT_voltex and using
 * its decimation function, then converting back to a GT_surface which will
 * replace all previous GT_surface objects in graphics_object.
 *
 * @param graphics_object  Graphics object of type g_SURFACE. Note that only
 * a few surface types are supported in the conversion to voltex; see
 * GT_voltex_create_from_GT_surface().
 * @param threshold_distance  Parameter controlling decimation; see
 * GT_voltex_decimate_triangles().
 * @return  1 on success, 0 on failure. On success the previous surface list in
 * graphics_object is replaced by a single discontinuous triangle surface which
 * cannot be selectively edited by graphics name / element number.
 */
int GT_object_decimate_GT_surface(struct GT_object *graphics_object,
	double threshold_distance);

enum Graphics_select_mode GT_object_get_select_mode(
	struct GT_object *graphics_object);
/*******************************************************************************
LAST MODIFIED : 7 July 2000

DESCRIPTION :
Gets the default_select_mode of a GT_object.
==============================================================================*/

int GT_object_set_select_mode(struct GT_object *graphics_object,
	enum Graphics_select_mode select_mode);
/*******************************************************************************
LAST MODIFIED : 7 July 2000

DESCRIPTION :
Sets the select_mode of the <graphics_object>.
==============================================================================*/

int GT_object_get_glyph_mirror_mode(struct GT_object *graphics_object);
/*******************************************************************************
LAST MODIFIED : 16 November 2000

DESCRIPTION :
Gets the glyph_mirror_mode of a GT_object -- true or false.
???RC temporary until we have a separate struct Glyph.
==============================================================================*/

int GT_object_set_glyph_mirror_mode(struct GT_object *graphics_object,
	int glyph_mirror_mode);
/*******************************************************************************
LAST MODIFIED : 16 November 2000

DESCRIPTION :
Sets the glyph_mirror_mode of the <graphics_object> to true or false.
???RC temporary until we have a separate struct Glyph.
==============================================================================*/

struct Graphical_material *get_GT_object_default_material(
	struct GT_object *graphics_object);
/*******************************************************************************
LAST MODIFIED : 4 June 1999

DESCRIPTION :
Gets the default_material of a GT_object.
==============================================================================*/

int set_GT_object_default_material(struct GT_object *graphics_object,
	struct Graphical_material *material);
/*******************************************************************************
LAST MODIFIED : 11 June 1998

DESCRIPTION :
Sets the default_material of a GT_object.
==============================================================================*/

struct Graphical_material *get_GT_object_secondary_material(
	struct GT_object *graphics_object);
/*******************************************************************************
LAST MODIFIED : 30 September 2005

DESCRIPTION :
Gets the secondary_material of a GT_object.
==============================================================================*/

int set_GT_object_secondary_material(struct GT_object *graphics_object,
	struct Graphical_material *material);
/*******************************************************************************
LAST MODIFIED : 30 September 2005

DESCRIPTION :
Sets the secondary_material of a GT_object.
==============================================================================*/

int GT_object_set_name(struct GT_object *graphics_object, const char *name);
/*******************************************************************************
LAST MODIFIED : 30 April 2003

DESCRIPTION :
Changes the name of <graphics_object> to a copy of <name>.
==============================================================================*/

struct Graphical_material *get_GT_object_selected_material(
	struct GT_object *graphics_object);
/*******************************************************************************
LAST MODIFIED : 18 February 2000

DESCRIPTION :
Gets the selected_material of a GT_object.
==============================================================================*/

int set_GT_object_selected_material(struct GT_object *graphics_object,
	struct Graphical_material *material);
/*******************************************************************************
LAST MODIFIED : 18 February 2000

DESCRIPTION :
Sets the selected_material of a GT_object.
==============================================================================*/

int set_GT_object_Spectrum(struct GT_object *graphics_object,
	void *spectrum_void);
/*******************************************************************************
LAST MODIFIED : 20 October 1997

DESCRIPTION :
Sets the spectrum of a GT_object.
==============================================================================*/

struct Spectrum *get_GT_object_spectrum(struct GT_object *graphics_object);
/*******************************************************************************
LAST MODIFIED : 4 June 1999

DESCRIPTION :
Gets the spectrum of a GT_object.
==============================================================================*/

int GT_object_list_contents(struct GT_object *graphics_object,void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 5 January 1998

DESCRIPTION :
Writes out information contained in <graphics_object> including its name and
type.
==============================================================================*/

int expand_spectrum_range_with_graphics_object(
	struct GT_object *graphics_object,void *spectrum_void);
/*******************************************************************************
LAST MODIFIED : 24 November 1997

DESCRIPTION :
Ensures the <spectrum> maximum and minimum is at least large enough to include
the range of data values in <graphics_object>.
==============================================================================*/

int resolve_glyph_axes(Triple point, Triple axis1, Triple axis2,
	Triple axis3, Triple scale, int mirror, int reverse, Triple final_point,
	Triple final_axis1, Triple final_axis2, Triple final_axis3);
/*******************************************************************************
LAST MODIFIED : 16 November 2000

DESCRIPTION :
Multiplies the three axes by their <scale> to give the final axes, reversing
<final_axis3> if necessary to produce a right handed coordinate system.
If <mirror> is true, then the axes are pointed in the opposite direction.
If <reverse> is true, then the point is shifted to the end of each axis if the
scale is negative for that axis.
==============================================================================*/

struct GT_object_compile_context *CREATE(GT_object_compile_context)(
	ZnReal time, struct Graphics_buffer *graphics_buffer
#if defined (OPENGL_API)
	, unsigned int ndc_display_list, unsigned int end_ndc_display_list
#endif /* defined (OPENGL_API) */
	);
/*******************************************************************************
LAST MODIFIED : 12 October 2005

DESCRIPTION :
Creates a GT_object_compile_context structure.
==============================================================================*/

int DESTROY(GT_object_compile_context)(struct GT_object_compile_context **context);
/*******************************************************************************
LAST MODIFIED : 12 October 2005

DESCRIPTION :
Frees the memory for <**context> and sets <*context> to NULL.
==============================================================================*/

#endif /* !defined (GRAPHICS_OBJECT_H) */

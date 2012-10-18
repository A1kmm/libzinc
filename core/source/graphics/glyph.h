/*******************************************************************************
FILE : glyph.h

LAST MODIFIED : 10 June 2004

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
#if !defined (GLYPH_H)
#define GLYPH_H
#include "graphics/graphics_object.h"

struct GT_object *make_glyph_arrow_line(const char *name,ZnReal head_length,
	ZnReal half_head_width);
/*******************************************************************************
LAST MODIFIED : 3 August 1999

DESCRIPTION :
Creates a graphics object named <name> consisting of a line from <0,0,0> to
<1,0,0> with 4 arrow head ticks <head_length> long and <half_head_width> out
from the shaft.
==============================================================================*/

struct GT_object *make_glyph_arrow_solid(const char *name, int primary_axis,
	int number_of_segments_around,ZnReal shaft_length,ZnReal shaft_radius,
	ZnReal cone_radius);
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

struct GT_object *make_glyph_axes(const char *name, int make_solid, ZnReal head_length,
	ZnReal half_head_width,const char **labels, ZnReal label_offset,
	struct Cmiss_graphics_font *font);
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

struct GT_object *make_glyph_cone(const char *name,int number_of_segments_around);
/*******************************************************************************
LAST MODIFIED : 17 July 1998

DESCRIPTION :
Creates a graphics object named <name> resembling a cone with the given
<number_of_segments_around>. The base of the cone is at (0,0,0) while its head
lies at (1,0,0). The radius of the cone is 0.5 at its base.
==============================================================================*/

struct GT_object *make_glyph_cross(const char *name);
/*******************************************************************************
LAST MODIFIED : 16 July 1999

DESCRIPTION :
Creates a graphics object named <name> consisting of a 3 lines:
from <-0.5,0,0> to <+0.5,0,0>
from <0,-0.5,0> to <0,+0.5,0>
from <0,0,-0.5> to <0,0,+0.5>
==============================================================================*/

struct GT_object *make_glyph_cube_solid(const char *name);
/*******************************************************************************
LAST MODIFIED : 20 November 2000

DESCRIPTION :
Creates a graphics object named <name> consisting of a unit-sized GT_surface
cube centred at <0,0,0>.
==============================================================================*/

struct GT_object *make_glyph_cube_wireframe(const char *name);
/*******************************************************************************
LAST MODIFIED : 20 November 2000

DESCRIPTION :
Creates a graphics object named <name> consisting of lines marking a unit-sized
wireframe cube centred at <0,0,0>.
==============================================================================*/

struct GT_object *make_glyph_cylinder(const char *name,int number_of_segments_around);
/*******************************************************************************
LAST MODIFIED : 16 July 1998

DESCRIPTION :
Creates a graphics object named <name> resembling a cylinder with the given
<number_of_segments_around>. The cylinder is centred at (0,0,0) and its axis
lies in the direction (1,0,0). It fits into the unit cube spanning from -0.5 to
+0.5 across all axes.
==============================================================================*/

struct GT_object *make_glyph_line(const char *name);
/*******************************************************************************
LAST MODIFIED : 16 September 1998

DESCRIPTION :
Creates a graphics object named <name> consisting of a line from <-0.5,0,0> to
<0.5,0,0>.
==============================================================================*/

struct GT_object *make_glyph_mirror(const char *name, struct GT_object *mirror_glyph);
/*******************************************************************************
LAST MODIFIED : 16 November 2000

DESCRIPTION :
Makes a glyph with the given <name> that automatically mirrors the given
<mirror_glyph>.
==============================================================================*/

struct GT_object *make_glyph_point(const char *name,gtMarkerType marker_type,
	ZnReal marker_size);
/*******************************************************************************
LAST MODIFIED : 1 December 1998

DESCRIPTION :
Creates a graphics object named <name> consisting of a single point at <0,0,0>.
The point will be drawn with the given <marker_type> and <marker_size>.
==============================================================================*/

struct GT_object *make_glyph_sheet(const char *name, int define_texturepoints);
/*******************************************************************************
LAST MODIFIED : 16 July 1998

DESCRIPTION :
Creates a graphics object named <name> resembling a square sheet spanning from
coordinate (-0.5,-0.5,0) to (0.5,0.5,0).
If define_texturepoints is true then texture coordinates will also be defined
ranging from <0.0,0.0,0.0> to <1.0,1.0,0.0>.
==============================================================================*/

struct GT_object *make_glyph_sphere(const char *name,int number_of_segments_around,
	int number_of_segments_down);
/*******************************************************************************
LAST MODIFIED : 23 July 1998

DESCRIPTION :
Creates a graphics object named <name> resembling a sphere with the given
<number_of_segments_around> and <number_of_segments_down> from pole to pole.
The sphere is centred at (0,0,0) and its poles are on the (1,0,0) line. It fits
into the unit cube spanning from -0.5 to +0.5 across all axes. Parameter
<number_of_segments_around> should normally be an even number at least 6 and
twice <number_of_segments_down> look remotely spherical.
==============================================================================*/

struct MANAGER(GT_object) *make_standard_glyphs(struct Cmiss_graphics_font *font);
/*******************************************************************************
LAST MODIFIED : 18 November 2005

DESCRIPTION :
Creates a list of standard glyphs for the cmgui applications.
==============================================================================*/

#endif /* !defined (GLYPH_H) */

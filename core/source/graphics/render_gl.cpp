/*******************************************************************************
FILE : rendergl.cpp

DESCRIPTION :
GL rendering calls - API specific.
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
#include <stdio.h>
#include <math.h>

#include "zinc/zincconfigure.h"

#include "general/debug.h"
#include "graphics/auxiliary_graphics_types.h"
#include "graphics/graphics_library.h"
#include "graphics/font.h"
#include "graphics/graphics_object.h"
#include "graphics/light_model.h"
#include "graphics/mcubes.h"
#include "graphics/scene.h"
#include "graphics/spectrum.h"
#include "graphics/tile_graphics_objects.h"
#include "general/message.h"
#include "graphics/graphics_coordinate_system.hpp"
#include "graphics/graphics_object_private.hpp"
#include "graphics/material.hpp"
#include "graphics/scene.hpp"
#include "graphics/render_gl.h"
#include "graphics/rendition.hpp"
#include "graphics/texture.hpp"

/***************************************************************************//**
																			 * Specifies the rendering type for this graphics_object.  The render function
																			 * could be split for different types or virtual methods or function callbacks
																			 * added but much of the code would be repeated with the first strategy and
																			 * there are a number of different points where the behaviour changes which would
																			 * need over-riding for the second.  So the current plan is to keep a single
																			 * routine and switch on this type.
																			 */
enum Graphics_object_rendering_type
{
	GRAPHICS_OBJECT_RENDERING_TYPE_GLBEGINEND,
	GRAPHICS_OBJECT_RENDERING_TYPE_CLIENT_VERTEX_ARRAYS,
	GRAPHICS_OBJECT_RENDERING_TYPE_VERTEX_BUFFER_OBJECT
};

/***************************************************************************//**
																			 * Convert graphical object into API object.
																			 */
static int render_GT_object_opengl_immediate(gtObject *object,
	int draw_selected, Render_graphics_opengl *renderer,
	Graphics_object_rendering_type type);

static int Graphics_object_compile_opengl_display_list(GT_object *graphics_object,
	Callback_base< GT_object * > *execute_function,
	Render_graphics_opengl *renderer);

static int Graphics_object_execute_opengl_display_list(GT_object *graphics_object,
	Render_graphics_opengl *renderer);

/***************************************************************************//**
																			 * Call the renderer to compile all the member objects which this GT_object depends
																			 * on.
																			 */
static int Graphics_object_compile_members_opengl(GT_object *graphics_object_list,
	Render_graphics_opengl *renderer);

/***************************************************************************//**
																			 * Rebuilds the display list for each uncreated graphics object in the
																			 * <graphics_object>, a simple linked list.
																			 */
static int Graphics_object_render_opengl(GT_object *graphics_object,
	Render_graphics_opengl *renderer, Graphics_object_rendering_type type);

/***************************************************************************//**
																			 * Compile the vertex buffer object for rendering the specified primitive.
																			 */
static int Graphics_object_compile_opengl_vertex_buffer_object(GT_object *object,
	Render_graphics_opengl *renderer);

/****************** Render_graphics_opengl method implementations **************/

int Render_graphics_opengl::Graphics_object_compile(GT_object *graphics_object)
{
	return Graphics_object_compile_members_opengl(graphics_object, this);
}

int Render_graphics_opengl::Material_compile(Graphical_material *material)
{
	return Material_compile_members_opengl(material, this);
}

/***************************************************************************//**
																			 * An implementation of a render class that uses immediate mode glBegin/glEnd.
																			 */
class Render_graphics_opengl_glbeginend : public Render_graphics_opengl
{
public:
	Render_graphics_opengl_glbeginend(Graphics_buffer *graphics_buffer) :
	  Render_graphics_opengl(graphics_buffer)
	  {
	  }

	  /***************************************************************************//**
																				   * Execute the Scene.
																				   */
	  int Scene_execute(Scene *scene)
	  {
		  set_Scene(scene);
		  return Scene_render_opengl(scene, this);
	  }

	  int Graphics_object_execute(GT_object *graphics_object)
	  {
		  return Graphics_object_render_opengl(graphics_object, this,
			  GRAPHICS_OBJECT_RENDERING_TYPE_GLBEGINEND);
	  }

	  int Graphics_object_render_immediate(GT_object *graphics_object)
	  {
		  return Graphics_object_render_opengl(graphics_object, this,
			  GRAPHICS_OBJECT_RENDERING_TYPE_GLBEGINEND);
	  }

	  int Cmiss_rendition_execute(Cmiss_rendition *cmiss_rendition)
	  {
		  return execute_Cmiss_rendition(cmiss_rendition, this);
	  }

	  int Cmiss_rendition_execute_members(Cmiss_rendition *cmiss_rendition)
	  {
		  return Cmiss_rendition_render_opengl(cmiss_rendition, this);
	  }

	  int Cmiss_rendition_execute_child_rendition(Cmiss_rendition *cmiss_rendition)
	  {
		  return Cmiss_rendition_render_child_rendition(cmiss_rendition, this);
	  }

	  int Material_execute(Graphical_material *material)
	  {
		  return Material_render_opengl(material, this);
	  }

	  int Texture_compile(Texture *texture)
	  {
		  return Texture_compile_opengl_texture_object(texture, this);
	  }

	  int Texture_execute(Texture *texture)
	  {
		  return Texture_execute_opengl_texture_object(texture, this);
	  }

	  int Light_execute(Light *light)
	  {
		  return execute_Light(light, NULL);
	  }

	  int Light_model_execute(Light_model *light_model)
	  {
		  return Light_model_render_opengl(light_model, this);
	  }

	  virtual int begin_coordinate_system(enum Cmiss_graphics_coordinate_system coordinate_system)
	  {
		  int return_code = 1;
		  switch (coordinate_system)
		  {
		  case CMISS_GRAPHICS_COORDINATE_SYSTEM_LOCAL:
			  {
				  /* Do nothing */
			  } break;
		  case CMISS_GRAPHICS_COORDINATE_SYSTEM_WORLD:
			  {
				  glMatrixMode(GL_MODELVIEW);
				  glPushMatrix();
				  glLoadMatrixd(world_view_matrix);
			  } break;
		  case CMISS_GRAPHICS_COORDINATE_SYSTEM_NORMALISED_WINDOW_FILL:
		  case CMISS_GRAPHICS_COORDINATE_SYSTEM_NORMALISED_WINDOW_FIT_CENTRE:
		  case CMISS_GRAPHICS_COORDINATE_SYSTEM_NORMALISED_WINDOW_FIT_LEFT:
		  case CMISS_GRAPHICS_COORDINATE_SYSTEM_NORMALISED_WINDOW_FIT_RIGHT:
		  case CMISS_GRAPHICS_COORDINATE_SYSTEM_NORMALISED_WINDOW_FIT_TOP:
		  case CMISS_GRAPHICS_COORDINATE_SYSTEM_NORMALISED_WINDOW_FIT_BOTTOM:
		  case CMISS_GRAPHICS_COORDINATE_SYSTEM_WINDOW_PIXEL_BOTTOM_LEFT:
		  case CMISS_GRAPHICS_COORDINATE_SYSTEM_WINDOW_PIXEL_TOP_LEFT:
			  {
				  if (picking)
				  {
					  /* To pick correctly need additional transformation to interaction volume.
					  * skip window-relative graphics for now. */
					  return_code = 0;
				  }
				  else
				  {
					  /* Push the current model matrix and reset the model matrix to identity */
					  glMatrixMode(GL_PROJECTION);
					  glPushMatrix();
					  glLoadIdentity();
					  double left, right, bottom, top;
					  if (Cmiss_graphics_coordinate_system_get_viewport(coordinate_system,
						  viewport_width, viewport_height, &left, &right, &bottom, &top))
					  {
						  /* near = 1.0 and far = 3.0 gives -1 to be the near clipping plane
						  and +1 to be the far clipping plane */
						  glOrtho(left, right, bottom, top, /*nearVal*/1.0, /*farVal*/3.0);
					  }
					  else
					  {
						  return_code = 0;
					  }
					  glMatrixMode(GL_MODELVIEW);
					  glPushMatrix();
					  glLoadIdentity();
					  gluLookAt(/*eye*/0.0,0.0,2.0, /*lookat*/0.0,0.0,0.0,
						  /*up*/0.0,1.0,0.0);
				  }
			  } break;
		  default:
			  {
				  display_message(ERROR_MESSAGE,"begin_coordinate_system.  Invalid graphics coordinate system.");
				  return_code = 0;
			  } break;
		  }
		  return return_code;
	  }

	  virtual void end_coordinate_system(enum Cmiss_graphics_coordinate_system coordinate_system)
	  {
		  switch (coordinate_system)
		  {
		  case CMISS_GRAPHICS_COORDINATE_SYSTEM_LOCAL:
			  {
				  /* Do nothing */
			  } break;
		  case CMISS_GRAPHICS_COORDINATE_SYSTEM_WORLD:
			  {
				  glMatrixMode(GL_MODELVIEW);
				  glPopMatrix();
			  } break;
		  case CMISS_GRAPHICS_COORDINATE_SYSTEM_NORMALISED_WINDOW_FILL:
		  case CMISS_GRAPHICS_COORDINATE_SYSTEM_NORMALISED_WINDOW_FIT_CENTRE:
		  case CMISS_GRAPHICS_COORDINATE_SYSTEM_NORMALISED_WINDOW_FIT_LEFT:
		  case CMISS_GRAPHICS_COORDINATE_SYSTEM_NORMALISED_WINDOW_FIT_RIGHT:
		  case CMISS_GRAPHICS_COORDINATE_SYSTEM_NORMALISED_WINDOW_FIT_TOP:
		  case CMISS_GRAPHICS_COORDINATE_SYSTEM_NORMALISED_WINDOW_FIT_BOTTOM:
		  case CMISS_GRAPHICS_COORDINATE_SYSTEM_WINDOW_PIXEL_BOTTOM_LEFT:
		  case CMISS_GRAPHICS_COORDINATE_SYSTEM_WINDOW_PIXEL_TOP_LEFT:
			  {
				  /* Pop the model matrix stack */
				  glMatrixMode(GL_PROJECTION);
				  glPopMatrix();
				  glMatrixMode(GL_MODELVIEW);
				  glPopMatrix();
			  } break;
		  default:
			  {
			  } break;
		  }
	  }

}; /* class Render_graphics_opengl_glbeginend */

Render_graphics_opengl *Render_graphics_opengl_create_glbeginend_renderer(
	Graphics_buffer *graphics_buffer)
{
	return new Render_graphics_opengl_glbeginend(graphics_buffer);
}

/***************************************************************************//**
																			 * An implementation of a render class that uses client vertex arrays.
																			 */
class Render_graphics_opengl_client_vertex_arrays : public Render_graphics_opengl_glbeginend
{
public:
	Render_graphics_opengl_client_vertex_arrays(Graphics_buffer *graphics_buffer) :
	  Render_graphics_opengl_glbeginend(graphics_buffer)
	  {
	  }

	  /***************************************************************************//**
																				   * Execute the Graphics_object.
																				   */
	  int Graphics_object_execute(GT_object *graphics_object)
	  {
		  return Graphics_object_render_opengl(graphics_object, this,
			  GRAPHICS_OBJECT_RENDERING_TYPE_CLIENT_VERTEX_ARRAYS);
	  }

	  /***************************************************************************//**
																				   * Execute the Graphics_object.
																				   */
	  int Graphics_object_render_immediate(GT_object *graphics_object)
	  {
		  return Graphics_object_render_opengl(graphics_object, this,
			  GRAPHICS_OBJECT_RENDERING_TYPE_CLIENT_VERTEX_ARRAYS);
	  }

}; /* class Render_graphics_opengl_client_vertex_arrays */


/***************************************************************************//**
																			 * An implementation of a render class that uses client vertex arrays.
																			 */
class Render_graphics_opengl_vertex_buffer_object : public Render_graphics_opengl_glbeginend
{
public:
	Render_graphics_opengl_vertex_buffer_object(Graphics_buffer *graphics_buffer) :
	  Render_graphics_opengl_glbeginend(graphics_buffer)
	  {
	  }

	  /***************************************************************************//**
																				   * Compile the Graphics_object.
																				   */
	  int Graphics_object_compile(GT_object *graphics_object)
	  {
		  return Render_graphics_opengl_glbeginend::Graphics_object_compile(
			  graphics_object)
			  && Graphics_object_compile_opengl_vertex_buffer_object(
			  graphics_object, this);
	  }

	  /***************************************************************************//**
																				   * Execute the Graphics_object.
																				   */
	  int Graphics_object_execute(GT_object *graphics_object)
	  {
		  return Graphics_object_render_opengl(graphics_object, this,
			  GRAPHICS_OBJECT_RENDERING_TYPE_VERTEX_BUFFER_OBJECT);
	  }

	  /***************************************************************************//**
																				   * Execute the Graphics_object.
																				   */
	  int Graphics_object_render_immediate(GT_object *graphics_object)
	  {
		  return Graphics_object_compile_opengl_vertex_buffer_object(
			  graphics_object, this)
			  && Graphics_object_render_opengl(graphics_object, this,
			  GRAPHICS_OBJECT_RENDERING_TYPE_VERTEX_BUFFER_OBJECT);
	  }

}; /* class Render_graphics_opengl_vertex_buffer_object */

Render_graphics_opengl *Render_graphics_opengl_create_client_vertex_arrays_renderer(
	Graphics_buffer *graphics_buffer)
{
	return new Render_graphics_opengl_client_vertex_arrays(graphics_buffer);
}

class Render_graphics_opengl_vertex_buffer_objects : public Render_graphics_opengl_glbeginend
{
public:
	Render_graphics_opengl_vertex_buffer_objects(Graphics_buffer *graphics_buffer) :
	  Render_graphics_opengl_glbeginend(graphics_buffer)
	  {
	  }

	  int Graphics_object_compile(GT_object *graphics_object)
	  {
		  return Render_graphics_opengl_glbeginend::Graphics_object_compile(graphics_object) &&
			  Graphics_object_compile_opengl_vertex_buffer_object(graphics_object, this);
	  }

}; /* class Render_graphics_opengl_glbeginend */

Render_graphics_opengl *Render_graphics_opengl_create_vertex_buffer_object_renderer(
	Graphics_buffer *graphics_buffer)
{
	return new Render_graphics_opengl_vertex_buffer_object(graphics_buffer);
}

/***************************************************************************//**
																			 * An implementation of a render class that wraps another opengl renderer in
																			 * compile and then execute stages.
																			 */
template <class Render_immediate> class Render_graphics_opengl_display_list
	: public Render_immediate
{

public:
	Render_graphics_opengl_display_list(Graphics_buffer *graphics_buffer) :
	  Render_immediate(graphics_buffer)
	  {
	  }

	  ~Render_graphics_opengl_display_list()
	  {
	  }

	  int Cmiss_rendition_execute_members_parent(Cmiss_rendition *cmiss_rendition)
	  {
		  return Render_immediate::Cmiss_rendition_execute_members(cmiss_rendition);
	  }

	  int Cmiss_rendition_execute_members(Cmiss_rendition *cmiss_rendition)
	  {
		  return Cmiss_rendition_render_opengl(cmiss_rendition, this);
	  }

	  int Cmiss_rendition_execute_child_rendition(Cmiss_rendition *cmiss_rendition)
	  {
		  return Cmiss_rendition_render_child_rendition(cmiss_rendition, this);
	  }

	  int Graphics_object_execute_parent(GT_object *graphics_object)
	  {
		  return Render_immediate::Graphics_object_execute(graphics_object);
	  }

	  int Graphics_object_compile(GT_object *graphics_object)
	  {
		  int return_code;

		  return_code = Render_immediate::Graphics_object_compile(graphics_object);
		  if (return_code)
		  {
			  Callback_member_callback< GT_object*, Render_graphics_opengl_display_list,
				  int (Render_graphics_opengl_display_list::*)(GT_object*) >
				  execute_method(static_cast<Render_graphics_opengl_display_list*>(this),
				  &Render_graphics_opengl_display_list::Graphics_object_execute_parent);
			  return_code = ::Graphics_object_compile_opengl_display_list(graphics_object,
				  &execute_method, this);
		  }
		  return (return_code);
	  }

	  int Graphics_object_execute(GT_object *graphics_object)
	  {
		  return ::Graphics_object_execute_opengl_display_list(graphics_object, this);
	  }

	  int Material_execute_parent(Graphical_material *material)
	  {
		  return Render_immediate::Material_execute(material);
	  }

	  int Material_compile(Graphical_material *material)
	  {
		  int return_code;

		  return_code = Render_immediate::Material_compile(material);
		  if (return_code)
		  {
			  Callback_member_callback< Graphical_material*, Render_graphics_opengl_display_list,
				  int (Render_graphics_opengl_display_list::*)(Graphical_material*) >
				  execute_method(static_cast<Render_graphics_opengl_display_list*>(this),
				  &Render_graphics_opengl_display_list::Material_execute_parent);
			  return_code = Material_compile_opengl_display_list(material,
				  &execute_method, this);
		  }
		  return (return_code);
	  }

	  int Material_execute(Graphical_material *material)
	  {
		  return Material_execute_opengl_display_list(material, this);
	  }

	  int Texture_execute_parent(Texture *texture)
	  {
		  return Render_immediate::Texture_execute(texture);
	  }

	  int Texture_compile(Texture *texture)
	  {
		  int return_code;

		  return_code = Render_immediate::Texture_compile(texture);
		  if (return_code)
		  {
			  Callback_member_callback< Texture*, Render_graphics_opengl_display_list,
				  int (Render_graphics_opengl_display_list::*)(Texture*) >
				  execute_method(static_cast<Render_graphics_opengl_display_list*>(this),
				  &Render_graphics_opengl_display_list::Texture_execute_parent);
			  return_code = Texture_compile_opengl_display_list(texture,
				  &execute_method, this);
		  }
		  return (return_code);
	  }

	  int Texture_execute(Texture *texture)
	  {
		  return Texture_execute_opengl_display_list(texture, this);
	  }

}; /* class Render_graphics_opengl_display_list */

Render_graphics_opengl *Render_graphics_opengl_create_glbeginend_display_list_renderer(
	Graphics_buffer *graphics_buffer)
{
	return new Render_graphics_opengl_display_list
		<Render_graphics_opengl_glbeginend>(graphics_buffer);
}

Render_graphics_opengl *Render_graphics_opengl_create_client_vertex_arrays_display_list_renderer(
	Graphics_buffer *graphics_buffer)
{
	return new Render_graphics_opengl_display_list
		<Render_graphics_opengl_client_vertex_arrays>(graphics_buffer);
}

Render_graphics_opengl *Render_graphics_opengl_create_vertex_buffer_object_display_list_renderer(
	Graphics_buffer *graphics_buffer)
{
	return new Render_graphics_opengl_display_list
		<Render_graphics_opengl_vertex_buffer_object>(graphics_buffer);
}

/******************  **********************/

static int draw_glyphsetGL(int number_of_points,Triple *point_list, Triple *axis1_list,
	Triple *axis2_list, Triple *axis3_list, Triple *scale_list,
struct GT_object *glyph, char **labels,
	int number_of_data_components, GLfloat *data, int *names,
	int label_bounds_dimension, int label_bounds_components, ZnReal *label_bounds,
	Triple *label_density_list,
struct Graphical_material *material, struct Graphical_material *secondary_material,
struct Spectrum *spectrum, struct Cmiss_graphics_font *font,
	//int draw_selected, int some_selected,struct Multi_range *selected_name_ranges,
	int draw_selected, SubObjectGroupHighlightFunctor *highlight_functor,
	Render_graphics_opengl *renderer, int object_name, int *lighting_off)
	/*******************************************************************************
	LAST MODIFIED : 22 November 2005

	DESCRIPTION :
	Draws graphics object <glyph> at <number_of_points> points given by the
	positions in <point_list> and oriented and scaled by <axis1_list>, <axis2_list>
	and <axis3_list>, each axis additionally scaled by its value in <scale_list>.
	If the glyph is part of a linked list through its nextobject
	member, these attached glyphs are also executed.
	Writes the <labels> array strings, if supplied, beside each glyph point.
	If <names> are supplied these identify each point/glyph for OpenGL picking.
	If <draw_selected> is set, then only those <names> in <selected_name_ranges>
	are drawn, otherwise only those names not there are drawn.
	If <some_selected> is true, <selected_name_ranges> indicates the points that
	are selected, or all points if <selected_name_ranges> is NULL.
	@param no_lighting  Flag giving current state of whether lighting is off.
	Enable or disable lighting and update as appropriate for point/line or surface
	glyphs to minimise state changes.
	==============================================================================*/
{
	char **label;
	ZnReal *label_bound;
	GLfloat transformation[16], x, y, z;
	Graphics_object_glyph_labels_function glyph_labels_function;
	GLfloat *datum = 0;
	int draw_all, i, j, *name = NULL, name_selected = 0, label_bounds_per_glyph = 0,
		return_code;
	struct GT_object *temp_glyph = NULL;
	struct Spectrum_render_data *render_data = NULL;
	Triple *axis1, *axis2, *axis3, *label_density, *point, *scale, temp_axis1, temp_axis2,
		temp_axis3, temp_point;

	ENTER(draw_glyphsetGL);
	if ((0 == number_of_points) || ((0 < number_of_points) && ((glyph && point_list &&
		axis1_list && axis2_list && axis3_list && scale_list) || (!glyph && labels))))
	{
		/*if ((0==number_of_points) ||
		(draw_selected&&((!names) || (!some_selected)))||
		((!draw_selected)&&(some_selected && (!selected_name_ranges))))*/
		if ((0==number_of_points) ||
			(draw_selected&&((!names) || (!highlight_functor))))
		{
			/* nothing to draw */
			return_code=1;
		}
		else
		{
#if defined (OPENGL_API)
			if ((!data)||(render_data=spectrum_start_renderGL
				(spectrum,material,number_of_data_components)))
			{
				draw_all = (!names) ||
					((!draw_selected)&&(!highlight_functor));
				point = point_list;
				axis1 = axis1_list;
				axis2 = axis2_list;
				axis3 = axis3_list;
				scale = scale_list;
				/* if there is data to plot, start the spectrum rendering */
				if (data)
				{
					datum=data;
				}
				if (label_density_list)
				{
					label_density = label_density_list;
				}
				else
				{
					label_density = (Triple *)NULL;
				}
				name = names;
				if (name)
				{
					/* make space for picking name on name stack */
					glPushName(0);
				}
				label=labels;
				label_bound = label_bounds;
				if (label_bounds)
				{
					label_bounds_per_glyph = 1 << label_bounds_dimension;
				}
				/* try to draw points and lines faster */
				if (!glyph)
				{
					if (!*lighting_off)
					{
						/* disable lighting so rendered in flat diffuse colour */
						glDisable(GL_LIGHTING);
						*lighting_off = 1;
					}
					if ((object_name > 0) && highlight_functor)
					{
						name_selected=highlight_functor->call(object_name);
					}
					for (i=0;i<number_of_points;i++)
					{
						if ((object_name < 1) && highlight_functor)
						{
							name_selected = highlight_functor->call(*name);
						}
						if (draw_all||(name_selected&&draw_selected)||((!name_selected)&&(!draw_selected)))
						{
							/* set the spectrum for this datum, if any */
							if (data)
							{
								spectrum_renderGL_value(spectrum,material,render_data,datum);
							}
							x = (*point)[0];
							y = (*point)[1];
							z = (*point)[2];
							if (names)
							{
								glLoadName((GLuint)(*name));
							}
							if (labels && *label)
							{
								Cmiss_graphics_font_rendergl_text(font, *label, x, y, z);
							}
						}
						point++;
						if (data)
						{
							datum += number_of_data_components;
						}
						if (names)
						{
							name++;
						}
						if (labels)
						{
							label++;
						}
					}
				}
				else if (0 == strcmp(glyph->name, "point"))
				{
					if (!*lighting_off)
					{
						/* disable lighting so rendered in flat diffuse colour */
						glDisable(GL_LIGHTING);
						*lighting_off = 1;
					}
					if (names||labels||highlight_functor)
					{
						/* cannot put glLoadName between glBegin and glEnd */
						if ((object_name > 0) && highlight_functor)
						{
							name_selected=highlight_functor->call(object_name);
						}
						for (i=0;i<number_of_points;i++)
						{
							if ((object_name < 1) && highlight_functor)
							{
								name_selected = highlight_functor->call(*name);
							}
							if (draw_all||(name_selected&&draw_selected)||((!name_selected)&&(!draw_selected)))
							{
								/* set the spectrum for this datum, if any */
								if (data)
								{
									spectrum_renderGL_value(spectrum,material,render_data,datum);
								}
								x = (*point)[0];
								y = (*point)[1];
								z = (*point)[2];
								if (names)
								{
									glLoadName((GLuint)(*name));
								}
								glBegin(GL_POINTS);
								glVertex3f(x, y, z);
								glEnd();
								if (labels && *label)
								{
									Cmiss_graphics_font_rendergl_text(font, *label, x, y, z);
								}
							}
							/* advance pointers */
							point++;
							if (data)
							{
								datum += number_of_data_components;
							}
							if (names)
							{
								name++;
							}
							if (labels)
							{
								label++;
							}
						}
					}
					else
					{
						/* more efficient if all points put out between glBegin and glEnd */
						glBegin(GL_POINTS);
						for (i=0;i<number_of_points;i++)
						{
							/* set the spectrum for this datum, if any */
							if (data)
							{
								spectrum_renderGL_value(spectrum,material,render_data,datum);
								datum+=number_of_data_components;
							}
							x = (*point)[0];
							y = (*point)[1];
							z = (*point)[2];
							point++;
							glVertex3f(x,y,z);
						}
						glEnd();
					}
				}
				else if ((0 == strcmp(glyph->name, "line")) ||
					(0 == strcmp(glyph->name, "mirror_line")))
				{
					GLfloat f0, f1;
					int mirror_mode = GT_object_get_glyph_mirror_mode(glyph);
					if (!*lighting_off)
					{
						/* disable lighting so rendered in flat diffuse colour */
						glDisable(GL_LIGHTING);
						*lighting_off = 1;
					}
					if (names||labels||highlight_functor)
					{
						if ((object_name > 0) && highlight_functor)
						{
							name_selected=highlight_functor->call(object_name);
						}
						/* cannot put glLoadName between glBegin and glEnd */
						for (i=0;i<number_of_points;i++)
						{
							if ((object_name < 1) && highlight_functor)
							{
								name_selected = highlight_functor->call(*name);
							}
							if (draw_all||(name_selected&&draw_selected)||((!name_selected)&&(!draw_selected)))
							{
								/* set the spectrum for this datum, if any */
								if (data)
								{
									spectrum_renderGL_value(spectrum,material,render_data,datum);
								}
								if (names)
								{
									glLoadName((GLuint)(*name));
								}
								x = (*point)[0];
								y = (*point)[1];
								z = (*point)[2];
								if (mirror_mode)
								{
									f0 = (*scale)[0];
									x -= f0*(*axis1)[0];
									y -= f0*(*axis1)[1];
									z -= f0*(*axis1)[2];
								}
								if (labels && *label)
								{
									Cmiss_graphics_font_rendergl_text(font, *label, x, y, z);
								}
								glBegin(GL_LINES);
								glVertex3f(x,y,z);
								f1 = (*scale)[0];
								x = (*point)[0] + f1*(*axis1)[0];
								y = (*point)[1] + f1*(*axis1)[1];
								z = (*point)[2] + f1*(*axis1)[2];
								glVertex3f(x,y,z);
								glEnd();
							}
							/* advance pointers */
							point++;
							axis1++;
							scale++;
							if (data)
							{
								datum+=number_of_data_components;
							}
							if (names)
							{
								name++;
							}
							if (labels)
							{
								label++;
							}
						}
					}
					else
					{
						/* more efficient if all lines put out between glBegin and glEnd */
						glBegin(GL_LINES);
						for (i=0;i<number_of_points;i++)
						{
							/* set the spectrum for this datum, if any */
							if (data)
							{
								spectrum_renderGL_value(spectrum,material,render_data,datum);
								datum+=number_of_data_components;
							}
							x = (*point)[0];
							y = (*point)[1];
							z = (*point)[2];
							if (mirror_mode)
							{
								f0 = (*scale)[0];
								x -= f0*(*axis1)[0];
								y -= f0*(*axis1)[1];
								z -= f0*(*axis1)[2];
							}
							glVertex3f(x,y,z);
							f1 = (*scale)[0];
							x = (*point)[0] + f1*(*axis1)[0];
							y = (*point)[1] + f1*(*axis1)[1];
							z = (*point)[2] + f1*(*axis1)[2];
							glVertex3f(x,y,z);
							point++;
							axis1++;
							scale++;
						}
						glEnd();
					}
				}
				else if (0==strcmp(glyph->name,"cross"))
				{
					if (!*lighting_off)
					{
						/* disable lighting so rendered in flat diffuse colour */
						glDisable(GL_LIGHTING);
						*lighting_off = 1;
					}
					if (names||labels||highlight_functor)
					{
						if ((object_name > 0) && highlight_functor)
						{
							name_selected=highlight_functor->call(object_name);
						}
						/* cannot put glLoadName between glBegin and glEnd */
						for (i=0;i<number_of_points;i++)
						{
							if ((object_name < 1) && highlight_functor)
							{
								name_selected = highlight_functor->call(*name);
							}
							if (draw_all||(name_selected&&draw_selected)||((!name_selected)&&(!draw_selected)))
							{
								/* set the spectrum for this datum, if any */
								if (data)
								{
									spectrum_renderGL_value(spectrum,material,render_data,datum);
								}
								if (names)
								{
									glLoadName((GLuint)(*name));
								}
								resolve_glyph_axes(*point, *axis1, *axis2, *axis3, *scale,
									/*mirror*/0, /*reverse*/0,
									temp_point, temp_axis1, temp_axis2, temp_axis3);

								x = temp_point[0];
								y = temp_point[1];
								z = temp_point[2];
								if (labels && *label)
								{
									Cmiss_graphics_font_rendergl_text(font, *label, x, y, z);
								}
								glBegin(GL_LINES);
								/* x-line */
								x = temp_point[0] - 0.5*temp_axis1[0];
								y = temp_point[1] - 0.5*temp_axis1[1];
								z = temp_point[2] - 0.5*temp_axis1[2];
								glVertex3f(x,y,z);
								x += temp_axis1[0];
								y += temp_axis1[1];
								z += temp_axis1[2];
								glVertex3f(x,y,z);
								/* y-line */
								x = temp_point[0] - 0.5*temp_axis2[0];
								y = temp_point[1] - 0.5*temp_axis2[1];
								z = temp_point[2] - 0.5*temp_axis2[2];
								glVertex3f(x,y,z);
								x += temp_axis2[0];
								y += temp_axis2[1];
								z += temp_axis2[2];
								glVertex3f(x,y,z);
								/* z-line */
								x = temp_point[0] - 0.5*temp_axis3[0];
								y = temp_point[1] - 0.5*temp_axis3[1];
								z = temp_point[2] - 0.5*temp_axis3[2];
								glVertex3f(x,y,z);
								x += temp_axis3[0];
								y += temp_axis3[1];
								z += temp_axis3[2];
								glVertex3f(x,y,z);
								glEnd();
							}
							/* advance pointers */
							point++;
							axis1++;
							axis2++;
							axis3++;
							scale++;
							if (data)
							{
								datum += number_of_data_components;
							}
							if (names)
							{
								name++;
							}
							if (labels)
							{
								label++;
							}
						}
					}
					else
					{
						/* more efficient if all lines put out between glBegin and glEnd */
						glBegin(GL_LINES);
						for (i=0;i<number_of_points;i++)
						{
							/* set the spectrum for this datum, if any */
							if (data)
							{
								spectrum_renderGL_value(spectrum,material,render_data,datum);
								datum += number_of_data_components;
							}
							resolve_glyph_axes(*point, *axis1, *axis2, *axis3, *scale,
								/*mirror*/0, /*reverse*/0,
								temp_point, temp_axis1, temp_axis2, temp_axis3);
							/* x-line */
							x = temp_point[0] - 0.5*temp_axis1[0];
							y = temp_point[1] - 0.5*temp_axis1[1];
							z = temp_point[2] - 0.5*temp_axis1[2];
							glVertex3f(x,y,z);
							x += temp_axis1[0];
							y += temp_axis1[1];
							z += temp_axis1[2];
							glVertex3f(x,y,z);
							/* y-line */
							x = temp_point[0] - 0.5*temp_axis2[0];
							y = temp_point[1] - 0.5*temp_axis2[1];
							z = temp_point[2] - 0.5*temp_axis2[2];
							glVertex3f(x,y,z);
							x += temp_axis2[0];
							y += temp_axis2[1];
							z += temp_axis2[2];
							glVertex3f(x,y,z);
							/* z-line */
							x = temp_point[0] - 0.5*temp_axis3[0];
							y = temp_point[1] - 0.5*temp_axis3[1];
							z = temp_point[2] - 0.5*temp_axis3[2];
							glVertex3f(x,y,z);
							x += temp_axis3[0];
							y += temp_axis3[1];
							z += temp_axis3[2];
							glVertex3f(x,y,z);
							point++;
							axis1++;
							axis2++;
							axis3++;
							scale++;
						}
						glEnd();
					}
				}
				else
				{
					if (*lighting_off)
					{
						/* enable lighting for general glyphs */
						glEnable(GL_LIGHTING);
						*lighting_off = 0;
					}
					int mirror_mode = GT_object_get_glyph_mirror_mode(glyph);
					const int number_of_glyphs = (mirror_mode) ? 2 : 1;
					/* must push and pop the modelview matrix */
					glMatrixMode(GL_MODELVIEW);
					if ((object_name > 0) && highlight_functor)
					{
						name_selected=highlight_functor->call(object_name);
					}
					for (i = 0; i < number_of_points; i++)
					{
						if ((object_name < 1) && highlight_functor)
						{
							name_selected = highlight_functor->call(*name);
						}
						if (draw_all||(name_selected&&draw_selected)||((!name_selected)&&(!draw_selected)))
						{
							if (names)
							{
								glLoadName((GLuint)(*name));
							}
							/* set the spectrum for this datum, if any */
							if (data)
							{
								spectrum_renderGL_value(spectrum,material,render_data,datum);
							}
							for (j = 0; j < number_of_glyphs; j++)
							{
								/* store the current modelview matrix */
								glPushMatrix();
								resolve_glyph_axes(*point, *axis1, *axis2, *axis3, *scale,
									/*mirror*/j, /*reverse*/mirror_mode,
									temp_point, temp_axis1, temp_axis2, temp_axis3);

								/* make transformation matrix for manipulating glyph */
								transformation[ 0] = temp_axis1[0];
								transformation[ 1] = temp_axis1[1];
								transformation[ 2] = temp_axis1[2];
								transformation[ 3] = 0.0;
								transformation[ 4] = temp_axis2[0];
								transformation[ 5] = temp_axis2[1];
								transformation[ 6] = temp_axis2[2];
								transformation[ 7] = 0.0;
								transformation[ 8] = temp_axis3[0];
								transformation[ 9] = temp_axis3[1];
								transformation[10] = temp_axis3[2];
								transformation[11] = 0.0;
								transformation[12] = temp_point[0];
								transformation[13] = temp_point[1];
								transformation[14] = temp_point[2];
								transformation[15] = 1.0;
								glMultMatrixf(transformation);
								if (mirror_mode)
								{
									/* ignore first glyph since just a wrapper for the second */
									temp_glyph = GT_object_get_next_object(glyph);
								}
								else
								{
									temp_glyph = glyph;
								}
								renderer->Graphics_object_execute(temp_glyph);
								glyph_labels_function = Graphics_object_get_glyph_labels_function(glyph);
								if (glyph_labels_function)
								{
									return_code = (*glyph_labels_function)(scale,
										label_bounds_dimension, label_bounds_components, label_bound,
										label_density, material, secondary_material, font, renderer);
								}
								/* restore the original modelview matrix */
								glPopMatrix();
							}
						}
						/* advance pointers */
						point++;
						axis1++;
						axis2++;
						axis3++;
						scale++;
						if (data)
						{
							datum += number_of_data_components;
						}
						if (label_density_list)
						{
							label_density++;
						}
						if (names)
						{
							name++;
						}
						if (label_bounds)
						{
							label_bound += label_bounds_components * label_bounds_per_glyph;
						}
					}
					/* output label at each point, if supplied */
					if ((label=labels) && !label_bounds)
					{
						/* Default is to draw the label value at the origin */
						name=names;
						if (!*lighting_off)
						{
							/* disable lighting so rendered in flat diffuse colour */
							glDisable(GL_LIGHTING);
							*lighting_off = 1;
						}
						point=point_list;
						datum=data;
						if ((object_name > 0) && highlight_functor)
						{
							name_selected=highlight_functor->call(object_name);
						}
						for (i=0;i<number_of_points;i++)
						{
							if ((object_name < 1) && highlight_functor)
							{
								name_selected = highlight_functor->call(*name);
							}
							if ((*label) && (draw_all	|| ((name_selected) && draw_selected)
								|| ((!name_selected)&&(!draw_selected))))
							{
								if (names)
								{
									glLoadName((GLuint)(*name));
								}
								x=(*point)[0];
								y=(*point)[1];
								z=(*point)[2];
								/* set the spectrum for this datum, if any */
								if (data)
								{
									spectrum_renderGL_value(spectrum,material,render_data,datum);
								}
								Cmiss_graphics_font_rendergl_text(font, *label, x, y, z);
							}
							/* advance pointers */
							point++;
							if (data)
							{
								datum += number_of_data_components;
							}
							if (names)
							{
								name++;
							}
							label++;
						}
					}
				}
				if (names)
				{
					/* free space for point number on picking name stack */
					glPopName();
				}
				if (data)
				{
					spectrum_end_renderGL(spectrum,render_data);
				}
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"draw_glyphsetGL.  Unable to start data rendering");
				return_code=0;
			}
#endif /* defined (OPENGL_API) */
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"draw_glyphsetGL. Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* draw_glyphsetGL */

static int draw_pointsetGL(int n_pts,Triple *point_list,char **text,
	gtMarkerType marker_type,GLfloat marker_size,int *names,
	int number_of_data_components, GLfloat *data,
struct Graphical_material *material, struct Spectrum *spectrum,
struct Cmiss_graphics_font *font)
	/*******************************************************************************
	LAST MODIFIED : 18 November 2005

	DESCRIPTION :
	???RC.  21/12/97 The optional names are supplied to allow identification of the
	object with OpenGL picking. (used to use a accessed pointer to an FE_node which
	did not allow other objects to be identified, nor nodes to be destroyed). Since
	can't change picking names between glBegin and glEnd, must have a separate
	begin/end bracket for each point. This may reduce rendering performance a
	little, but on the upside, spectrum handling and text output are done
	simultaneously.
	==============================================================================*/
{
	char **text_string;
	GLfloat half_marker_size,x,x1,x2,x3,y,y1,y2,y3,z,z1,z2,z3;
	GLfloat *datum = NULL;
	int i,*name,offset,return_code;
	struct Spectrum_render_data *render_data = NULL;
	Triple *point;

	ENTER(draw_pointsetGL);
	/* default return code */
	return_code=0;
	/* checking arguments */
	if (point_list&&(0<n_pts))
	{
#if defined (OPENGL_API)
		if ((!data)||(render_data=spectrum_start_renderGL
			(spectrum,material,number_of_data_components)))
		{
			point=point_list;
			/* if there is data to plot, start the spectrum rendering */
			if (data)
			{
				datum = data;
			}
			/* draw markers */
			half_marker_size=marker_size/2;
			if ((g_POINT_MARKER==marker_type)||
				((g_PLUS_MARKER==marker_type)&&(0>=half_marker_size)))
			{
				glBegin(GL_POINTS);
				for (i=0;i<n_pts;i++)
				{
					/* set the spectrum for this datum, if any */
					if (data)
					{
						spectrum_renderGL_value(spectrum,material,render_data,datum);
						datum += number_of_data_components;
					}
					x=(*point)[0];
					y=(*point)[1];
					z=(*point)[2];
					point++;
					glVertex3f(x,y,z);
				}
				glEnd();
			}
			else
			{
				switch (marker_type)
				{
				case g_PLUS_MARKER:
					{
						glBegin(GL_LINES);
						for (i=0;i<n_pts;i++)
						{
							/* set the spectrum for this datum, if any */
							if (data)
							{
								spectrum_renderGL_value(spectrum,material,render_data,datum);
								datum += number_of_data_components;
							}
							x=(*point)[0];
							y=(*point)[1];
							z=(*point)[2];
							point++;
							/* first line */
							glVertex3f(x-half_marker_size,y,z);
							glVertex3f(x+half_marker_size,y,z);
							/* second line */
							glVertex3f(x,y-half_marker_size,z);
							glVertex3f(x,y+half_marker_size,z);
							/* third line */
							glVertex3f(x,y,z-half_marker_size);
							glVertex3f(x,y,z+half_marker_size);
						}
						glEnd();
					} break;
				case g_DERIVATIVE_MARKER:
					{
						glBegin(GL_LINES);
						for (i=0;i<n_pts;i++)
						{
							/* set the spectrum for this datum, if any */
							if (data)
							{
								spectrum_renderGL_value(spectrum,material,render_data,datum);
								datum += number_of_data_components;
							}
							x=(*point)[0];
							y=(*point)[1];
							z=(*point)[2];
							point++;
							x1=(*point)[0];
							y1=(*point)[1];
							z1=(*point)[2];
							point++;
							if ((x1!=x)||(y1!=y)||(z1!=z))
							{
								glVertex3f(x,y,z);
								glVertex3f(x1,y1,z1);
							}
							x2=(*point)[0];
							y2=(*point)[1];
							z2=(*point)[2];
							point++;
							if ((x2!=x)||(y2!=y)||(z2!=z))
							{
								glVertex3f(x,y,z);
								glVertex3f(x2,y2,z2);
							}
							x3=(*point)[0];
							y3=(*point)[1];
							z3=(*point)[2];
							point++;
							if ((x3!=x)||(y3!=y)||(z3!=z))
							{
								glVertex3f(x,y,z);
								glVertex3f(x3,y3,z3);
							}
						}
						glEnd();
					} break;
				default:
					{
						/* Do nothing, satisfy warnings */
					} break;
				}
			}
			/* output text at each point, if supplied */
			text_string = text;
			if (text_string)
			{
				point=point_list;
				datum=data;
				if (g_DERIVATIVE_MARKER==marker_type)
				{
					offset=4;
				}
				else
				{
					offset=1;
				}
				for (i=0;i<n_pts;i++)
				{
					x=(*point)[0];
					y=(*point)[1];
					z=(*point)[2];
					point += offset;
					/* set the spectrum for this datum, if any */
					if (data)
					{
						spectrum_renderGL_value(spectrum,material,render_data,datum);
						datum += number_of_data_components;
					}
					if (*text_string)
					{
						Cmiss_graphics_font_rendergl_text(font, *text_string, x, y, z);
					}
					text_string++;
				}
			}
			/* picking */
			name = names;
			if (name)
			{
				point=point_list;
				datum=data;
				/* make a first name to load over with each name */
				glPushName(0);
				switch (marker_type)
				{
				case g_POINT_MARKER:
				case g_PLUS_MARKER:
					{
						for (i=0;i<n_pts;i++)
						{
							x=(*point)[0];
							y=(*point)[1];
							z=(*point)[2];
							point++;
							/* set the spectrum for this datum, if any */
							if (data)
							{
								spectrum_renderGL_value(spectrum,material,render_data,datum);
								datum += number_of_data_components;
							}
							glLoadName((GLuint)(*name));
							glBegin(GL_POINTS);
							glVertex3f(x,y,z);
							glEnd();
							name++;
						}
					} break;
				case g_DERIVATIVE_MARKER:
					{
						for (i=0;i<n_pts;i++)
						{
							x=(*point)[0];
							y=(*point)[1];
							z=(*point)[2];
							point++;
							x1=(*point)[0];
							y1=(*point)[1];
							z1=(*point)[2];
							point++;
							x2=(*point)[0];
							y2=(*point)[1];
							z2=(*point)[2];
							point++;
							x3=(*point)[0];
							y3=(*point)[1];
							z3=(*point)[2];
							point++;
							/* set the spectrum for this datum, if any */
							if (data)
							{
								spectrum_renderGL_value(spectrum,material,render_data,datum);
								datum += number_of_data_components;
							}
							/* output names */
							glLoadName((GLuint)(*name));
							glBegin(GL_POINTS);
							glVertex3f(x,y,z);
							glEnd();
							if ((x1!=x)||(y1!=y)||(z1!=z))
							{
								glPushName(1);
								glBegin(GL_POINTS);
								glVertex3f(x1,y1,z1);
								glEnd();
								glPopName();
							}
							if ((x2!=x)||(y2!=y)||(z2!=z))
							{
								glPushName(2);
								glBegin(GL_POINTS);
								glVertex3f(x2,y2,z2);
								glEnd();
								glPopName();
							}
							if ((x3!=x)||(y3!=y)||(z3!=z))
							{
								glPushName(3);
								glBegin(GL_POINTS);
								glVertex3f(x3,y3,z3);
								glEnd();
								glPopName();
							}
						}
						name++;
					} break;
				default:
					{
						/* Do nothing, satisfy warnings */
					} break;
				}
				glPopName();
			}
			if (data)
			{
				spectrum_end_renderGL(spectrum,render_data);
			}
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,"draw_pointsetGL.  Missing spectrum");
			return_code=0;
		}
#endif /* defined (OPENGL_API) */
	}
	else
	{
		if (0<n_pts)
		{
			display_message(ERROR_MESSAGE,"drawpointsetGL. Invalid argument(s)");
			return_code=0;
		}
		else
		{
			return_code=1;
		}
	}
	LEAVE;

	return (return_code);
} /* draw_pointsetGL */

static int draw_polylineGL(Triple *point_list, Triple *normal_list, int n_pts,
	int number_of_data_components, GLfloat *data,
struct Graphical_material *material,struct Spectrum *spectrum)
	/*******************************************************************************
	LAST MODIFIED : 9 June 1999

	DESCRIPTION :
	==============================================================================*/
{
	int return_code;
#if defined (OPENGL_API)
	GLfloat *datum = NULL;
	int i;
	Triple *normal = NULL, *point = NULL;
#endif /* defined (OPENGL_API) */
	struct Spectrum_render_data *render_data = NULL;

	ENTER(drawpolylineGL);
#if defined (DEBUG_CODE)
	printf("draw_polylineGL %d\n",n_pts);
#endif /* defined (DEBUG_CODE) */

	/* checking arguments */
	if (point_list&&(0<n_pts)&&((!data)||(render_data=
		spectrum_start_renderGL(spectrum,material,number_of_data_components))))
	{
#if defined (OPENGL_API)
		point=point_list;
		if (normal_list)
		{
			normal = normal_list;
		}
		if (data)
		{
			datum=data;
		}
		glBegin(GL_LINE_STRIP);
		for (i=n_pts;i>0;i--)
		{
			if (data)
			{
				spectrum_renderGL_value(spectrum,material,render_data,datum);
				datum += number_of_data_components;
			}
			if (normal_list)
			{
				glNormal3fv(*normal);
				normal++;
			}
			glVertex3fv(*point);
			point++;
		}
		glEnd();
		if (data)
		{
			spectrum_end_renderGL(spectrum,render_data);
		}
		return_code=1;
	}
	else
	{
		if (0<n_pts)
		{
			display_message(ERROR_MESSAGE,"draw_polylineGL.  Invalid argument(s)");
			return_code=0;
		}
		else
		{
			return_code=1;
		}
#endif /* defined (OPENGL_API) */
	}
	LEAVE;

	return (return_code);
} /* draw_polylineGL */

static int draw_dc_polylineGL(Triple *point_list,Triple *normal_list, int n_pts,
	int number_of_data_components, GLfloat *data,
struct Graphical_material *material,struct Spectrum *spectrum)
	/*******************************************************************************
	LAST MODIFIED : 9 June 1999

	DESCRIPTION :
	==============================================================================*/
{
	int return_code;
#if defined (OPENGL_API)
	int i;
	Triple *point = NULL, *normal = NULL;
	GLfloat *datum = NULL;
#endif /* defined (OPENGL_API) */
	struct Spectrum_render_data *render_data = NULL;

	ENTER(draw_dc_polylineGL);

	/* checking arguments */
	if (point_list&&(0<n_pts)&&((!data)||(render_data=
		spectrum_start_renderGL(spectrum,material,number_of_data_components))))
	{
#if defined (OPENGL_API)
		point=point_list;
		if (data)
		{
			datum=data;
		}
		if (normal_list)
		{
			normal = normal_list;
		}
		glBegin(GL_LINES);
		for (i=n_pts;i>0;i--)
		{
			if (data)
			{
				spectrum_renderGL_value(spectrum,material,render_data,datum);
				datum += number_of_data_components;
			}
			if (normal_list)
			{
				glNormal3fv(*normal);
				normal++;
			}
			glVertex3fv(*point);
			point++;
			if (data)
			{
				spectrum_renderGL_value(spectrum,material,render_data,datum);
				datum += number_of_data_components;
			}
			if (normal_list)
			{
				glNormal3fv(*normal);
				normal++;
			}
			glVertex3fv(*point);
			point++;
		}
		glEnd();
#endif /* defined (OPENGL_API) */
		if (data)
		{
			spectrum_end_renderGL(spectrum,render_data);
		}
		return_code=1;
	}
	else
	{
		if (0<n_pts)
		{
			display_message(ERROR_MESSAGE,"draw_dc_polylineGL.  Invalid argument(s)");
			return_code=0;
		}
		else
		{
			return_code=1;
		}
	}
	LEAVE;

	return (return_code);
} /* draw_dc_polylineGL */

/**
* Outputs quadrilateral surfaces as a series of OpenGL quad strips, and
* triangle surfaces as a series of triangle strips.
*/
static int draw_surfaceGL(Triple *surfpts, Triple *normalpoints, Triple *tangentpoints,
	Triple *texturepoints, int npts1, int npts2, gtPolygonType polygon_type,
	int number_of_data_components, GLfloat *data,
struct Graphical_material *material, struct Spectrum *spectrum,
struct Spectrum_render_data *render_data)
{
	int return_code;

	ENTER(draw_surfaceGL);
#if ! defined GL_VERSION_1_3
	USE_PARAMETER(tangentpoints);
#endif /* ! defined GL_VERSION_1_3 */
	/* checking arguments */
	if (surfpts && (1<npts1) &&(1<npts2) &&
		((NULL == data) || (NULL != render_data)))
	{
#if defined (OPENGL_API)
#if defined GL_VERSION_1_3
		if (tangentpoints)
		{
			if (!Graphics_library_check_extension(GL_VERSION_1_3))
			{
				/* Ignore these tangentpoints */
				tangentpoints = (Triple *)NULL;
			}
		}
#endif /* defined GL_VERSION_1_3 */
		switch (polygon_type)
		{
		case g_QUADRILATERAL:
			{
				/* For standard OpenGL polygon winding with normals out of the page,
				* expect points in sequence:
				*   7-8-9
				*   | | |
				*   4-5-6
				*   | | |
				*   1-2-3
				* The first quad strip is the left column from bottom to top:
				*   1,2,4,5,7,8
				*/
				const int number_of_strips = npts1 - 1;
				const int points_per_strip = 2*npts2;
				int i, index, j;
				for (i = 0; i < number_of_strips; i++)
				{
					index = i;
					glBegin(GL_QUAD_STRIP);
					for (j = 0; j < points_per_strip; j++)
					{
						if (normalpoints)
						{
							glNormal3fv(normalpoints[index]);
						}
#if defined GL_VERSION_1_3
						if (tangentpoints)
						{
							glMultiTexCoord3fv(GL_TEXTURE1_ARB, tangentpoints[index]);
						}
#endif /* defined GL_VERSION_1_3 */
						if (texturepoints)
						{
							glTexCoord3fv(texturepoints[index]);
						}
						if (data)
						{
							spectrum_renderGL_value(spectrum, material, render_data,
								data + index*number_of_data_components);
						}
						glVertex3fv(surfpts[index]);
						if (j & 1)
						{
							index += number_of_strips;
						}
						else
						{
							index++;
						}
					}
					glEnd();
				}
			} break;
		case g_TRIANGLE:
			{
				/* For standard OpenGL polygon winding with normals out of the page,
				* expect points in sequence:
				*   6
				*   |\
				*   4-5
				*   |\|\
				*   1-2-3
				* The first triangle strip is the left column from bottom to top:
				*   1,2,4,5,6
				*/
				const int number_of_strips = npts1 - 1;
				int i, index, j, points_per_strip;
				for (i = 0; i < number_of_strips; i++)
				{
					index = i;
					points_per_strip = (npts1 - i)*2 - 1;
					glBegin(GL_TRIANGLE_STRIP);
					for (j = 0; j < points_per_strip; j++)
					{
						if (normalpoints)
						{
							glNormal3fv(normalpoints[index]);
						}
#if defined GL_VERSION_1_3
						if (tangentpoints)
						{
							glMultiTexCoord3fv(GL_TEXTURE1_ARB, tangentpoints[index]);
						}
#endif /* defined GL_VERSION_1_3 */
						if (texturepoints)
						{
							glTexCoord3fv(texturepoints[index]);
						}
						if (data)
						{
							spectrum_renderGL_value(spectrum, material, render_data,
								data + index*number_of_data_components);
						}
						glVertex3fv(surfpts[index]);
						if (j & 1)
						{
							index += (number_of_strips - (j >> 1));
						}
						else
						{
							index++;
						}
					}
					glEnd();
				}
			} break;
		default:
			{
				/* Do nothing, satisfy warnings */
			} break;
		}
#endif /* defined (OPENGL_API) */
		return_code=1;
	}
	else
	{
		if ((1<npts1)&&(1<npts2))
		{
			display_message(ERROR_MESSAGE,
				"draw_surfaceGL.  Invalid argument(s)");
			return_code=0;
		}
		else
		{
			return_code=1;
		}
	}
	LEAVE;

	return (return_code);
} /* draw_surfaceGL */

static int draw_dc_surfaceGL(Triple *surfpts, Triple *normal_points,
	Triple *tangent_points, Triple *texture_points,
	int npolys,int npp,gtPolygonType polygon_type,int strip,
#if defined (OPENGL_API)
	GLenum& gl_surface_mode,
#endif /* defined (OPENGL_API) */
	int number_of_data_components,GLfloat *data,
struct Graphical_material *material,struct Spectrum *spectrum,
struct Spectrum_render_data *render_data)
	/*******************************************************************************
	LAST MODIFIED : 9 June 1999

	DESCRIPTION :
	If the <polygon_type> is g_TRIANGLE or g_QUADRILATERAL, the discontinuous
	sets of vertices are drawn as separate triangles or quadrilaterals - or as
	strips of the respective types if the <strip> flag is set. Otherwise a single
	polygon is drawn for each of the <npolys>.
	==============================================================================*/
{
#if defined (OPENGL_API)
	GLenum mode;
#endif /* defined (OPENGL_API) */
	GLfloat *data_item = NULL;
	int i,j,return_code = 0;
	Triple *normal_point = NULL, *point = NULL, *texture_point = NULL;

	ENTER(draw_data_dc_surfaceGL);
	USE_PARAMETER(tangent_points);
	if (surfpts&&(0<npolys)&&(2<npp) && ((NULL == data) || (NULL != render_data)))
	{
		if (data)
		{
			data_item=data;
		}
		point=surfpts;
		if (normal_points)
		{
			normal_point = normal_points;
		}
		if (texture_points)
		{
			texture_point = texture_points;
		}
#if defined (OPENGL_API)
		switch (polygon_type)
		{
		case g_QUADRILATERAL:
			{
				if (strip)
				{
					mode=GL_QUAD_STRIP;
				}
				else
				{
					mode=GL_QUADS;
				}
			} break;
		case g_TRIANGLE:
			{
				if (strip)
				{
					mode=GL_TRIANGLE_STRIP;
				}
				else
				{
					mode=GL_TRIANGLES;
				}
			} break;
		default:
			{
				mode=GL_POLYGON;
			}
		}
		for (i=0;i<npolys;i++)
		{
			if (mode != gl_surface_mode)
			{
				if (gl_surface_mode != GL_POINTS)
				{
					glEnd();
				}
				gl_surface_mode = mode;
				glBegin(gl_surface_mode);
			}
			for (j=0;j<npp;j++)
			{
				if (data)
				{
					spectrum_renderGL_value(spectrum,material,render_data,data_item);
					data_item += number_of_data_components;
				}
				if (normal_points)
				{
					glNormal3fv(*normal_point);
					normal_point++;
				}
				if (texture_points)
				{
					glTexCoord3fv(*texture_point);
					texture_point++;
				}
				glVertex3fv(*point);
				point++;


			}
		}
#endif /* defined (OPENGL_API) */
		return_code=1;
	}
	else
	{
		if (0<npolys)
		{
			display_message(ERROR_MESSAGE,
				"draw_dc_surfaceGL.  Invalid argument(s)");
			return_code=0;
		}
		else
		{
			return_code=1;
		}
	}
	LEAVE;

	return (return_code);
} /* draw_dc_surfaceGL */

static int draw_nurbsGL(struct GT_nurbs *nurbptr)
	/*******************************************************************************
	LAST MODIFIED : 9 June 1999

	DESCRIPTION :
	==============================================================================*/
{
	int return_code;
#if defined (OPENGL_API)
	GLfloat *cknots_temp = NULL, *control_temp = NULL, *normal_temp = NULL,
		*pwl_temp = NULL, *sknots_temp = NULL, *tknots_temp = NULL,	*texture_temp = NULL,
		*trimarray_temp = NULL;
	GLUnurbsObj *the_nurb;
	int i;
#endif /* defined (OPENGL_API) */

	ENTER(draw_nurbsGL);
	/* default return code */
	return_code=0;
	/* checking arguments */
	if (nurbptr)
	{
#if defined (OPENGL_API)
		/* allocate local temporary arrays */
		ALLOCATE(sknots_temp,GLfloat,nurbptr->sknotcnt);
		ALLOCATE(tknots_temp,GLfloat,nurbptr->tknotcnt);
		ALLOCATE(control_temp,GLfloat,4*(nurbptr->maxs)*(nurbptr->maxt));
		/* check memory allocation */
		if (sknots_temp&&tknots_temp&&control_temp)
		{
			/* move sknots into temp GLfloat array */
			for (i=0;i<nurbptr->sknotcnt;i++)
			{
				sknots_temp[i]=(GLfloat)nurbptr->sknots[i];
			}
			/* move tknots into a temp GLfloat array */
			for (i=0;i<nurbptr->tknotcnt;i++)
			{
				tknots_temp[i]=(GLfloat)nurbptr->tknots[i];
			}
			if (nurbptr->normal_control_points)
			{
				/* move normals into a temp GLfloat array */
				ALLOCATE(normal_temp,GLfloat,4*(nurbptr->maxs)*(nurbptr->maxt));
				for (i=0;i<(4*nurbptr->maxt*nurbptr->maxs);i++)
				{
					normal_temp[i]=(GLfloat)nurbptr->normal_control_points[i];
				}
			}
			if (nurbptr->texture_control_points)
			{
				/* move textures into a temp GLfloat array */
				ALLOCATE(texture_temp,GLfloat,4*(nurbptr->maxs)*(nurbptr->maxt));
				for (i=0;i<(4*nurbptr->maxt*nurbptr->maxs);i++)
				{
					texture_temp[i]=(GLfloat)nurbptr->texture_control_points[i];
				}
			}
			if (nurbptr->cknotcnt>0)
			{
				/* move cknots into a temp GLfloat array */
				ALLOCATE(cknots_temp,GLfloat,nurbptr->cknotcnt);
				for (i=0;i<nurbptr->cknotcnt;i++)
				{
					cknots_temp[i]=(GLfloat)nurbptr->cknots[i];
				}
			}
			/* move controlpts into a temp GLfloat array */
			for (i=0;i<(4*nurbptr->maxt*nurbptr->maxs);i++)
			{
				control_temp[i]=(GLfloat)nurbptr->controlpts[i];
			}
			if (nurbptr->ccount>0)
			{
				/* move trimarray into a temp GLfloat array */
				ALLOCATE(trimarray_temp,GLfloat,3*nurbptr->ccount);
				for (i=0;i<(3*nurbptr->ccount);i++)
				{
					trimarray_temp[i]=(GLfloat)nurbptr->trimarray[i];
				}
			}
			if (nurbptr->pwlcnt>0)
			{
				/* move pwlarray into a temp GLfloat array */
				ALLOCATE(pwl_temp,GLfloat,3*nurbptr->pwlcnt);
				for (i=0;i<(3*nurbptr->pwlcnt);i++)
				{
					pwl_temp[i]=(GLfloat)nurbptr->pwlarray[i];
				}
			}
			/* store the evaluator attributes */
			glPushAttrib(GL_EVAL_BIT);
			if(!nurbptr->normal_control_points)
			{
				/* automatically calculate surface normals */
				glEnable(GL_AUTO_NORMAL);
			}
			the_nurb=gluNewNurbsRenderer();
			/* set the maximum sampled pixel size */
			/* start the nurbs surface */
			gluBeginSurface(the_nurb);
			/* draw the nurbs surface */
			gluNurbsSurface(the_nurb,nurbptr->sknotcnt,sknots_temp,nurbptr->tknotcnt,
				tknots_temp,4,4*(nurbptr->maxs),control_temp,nurbptr->sorder,
				nurbptr->torder,GL_MAP2_VERTEX_4);
			if(nurbptr->normal_control_points)
			{
				gluNurbsSurface(the_nurb,nurbptr->sknotcnt,sknots_temp,nurbptr->tknotcnt,
					tknots_temp,4,4*(nurbptr->maxs),normal_temp,nurbptr->sorder,
					nurbptr->torder,GL_MAP2_NORMAL);
			}
			if(nurbptr->texture_control_points)
			{
				gluNurbsSurface(the_nurb,nurbptr->sknotcnt,sknots_temp,nurbptr->tknotcnt,
					tknots_temp,4,4*(nurbptr->maxs),texture_temp,nurbptr->sorder,
					nurbptr->torder,GL_MAP2_TEXTURE_COORD_3);
			}
			if (nurbptr->cknotcnt>0)
			{
				/* trim the nurbs surface with a nurbs curve */
				gluBeginTrim(the_nurb);
				gluNurbsCurve(the_nurb,nurbptr->cknotcnt,cknots_temp,3,trimarray_temp,
					nurbptr->corder,GLU_MAP1_TRIM_3);
				gluEndTrim(the_nurb);
			}
			if (nurbptr->pwlcnt)
			{
				/* trim the nurbs surface with a pwl curve */
				gluBeginTrim(the_nurb);
				gluPwlCurve(the_nurb,nurbptr->pwlcnt,pwl_temp,3,GLU_MAP1_TRIM_3);
				gluEndTrim(the_nurb);
			}
			/* end the nurbs surface */
			gluEndSurface(the_nurb);
			/* restore the evaluator attributes */
			glPopAttrib();
		}
		else
		{
			display_message(ERROR_MESSAGE,"drawnurbsGL.	Memory allocation failed");
			return_code=0;
		}
		/* deallocate local temp arrays */
		DEALLOCATE(control_temp);
		if (nurbptr->ccount>0)
		{
			DEALLOCATE(trimarray_temp);
		}
		if (nurbptr->normal_control_points)
		{
			DEALLOCATE(normal_temp);
		}
		if (nurbptr->texture_control_points)
		{
			DEALLOCATE(texture_temp);
		}
		if (nurbptr->cknotcnt>0)
		{
			DEALLOCATE(cknots_temp);
		}
		DEALLOCATE(sknots_temp);
		DEALLOCATE(tknots_temp);
		if(nurbptr->pwlcnt>0)
		{
			DEALLOCATE(pwl_temp);
		}
#endif /* defined (OPENGL_API) */
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"drawnurbsGL.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* draw_nurbsGL */

static int draw_voltexGL(int number_of_vertices, struct VT_iso_vertex **vertex_list,
	int number_of_triangles, struct VT_iso_triangle **triangle_list,
	int number_of_data_components, int number_of_texture_coordinates,
struct Graphical_material *default_material, struct Spectrum *spectrum)
	/*******************************************************************************
	LAST MODIFIED : 11 November 2005

	DESCRIPTION :
	Numbers in <iso_poly_material_index> are indices into the materials in the
	<per_vertex_materials>. A zero value denotes use of the default material;
	and index of 1 means the first material in the per_vertex_materials. Not
	supplying the <iso_poly_material_index> gives the default material to all
	vertices.
	Use of <iso_poly_environment_map_index> and <per_vertex_environment_maps> is
	exactly the same as for materials. Note environment map materials are used in
	preference to normal materials.
	==============================================================================*/
{
	int i, j, return_code;
	struct Spectrum_render_data *render_data = NULL;
	struct VT_iso_triangle *triangle;
	struct VT_iso_vertex *vertex;

	ENTER(draw_voltexGL);
	/* default return code */
	return_code = 0;
	/* checking arguments */
	if (triangle_list && vertex_list && (0 < number_of_vertices) && (0 < number_of_triangles))
	{
#if defined (OPENGL_API)
		if ((!number_of_data_components) ||
			(render_data=spectrum_start_renderGL(spectrum,default_material,
			number_of_data_components)))
		{
			glBegin(GL_TRIANGLES);
			for (i = 0; i < number_of_triangles; i++)
			{
				triangle = triangle_list[i];
				GLfloat values[3];
				GLfloat *data = 0;
				if (number_of_data_components)
				{
					data = new GLfloat[number_of_data_components];
				}
				for (j = 0 ; j < 3 ; j++)
				{
					vertex = triangle->vertices[j];
					if (number_of_data_components)
					{
						CAST_TO_OTHER(data, vertex->data, GLfloat, number_of_data_components);
						spectrum_renderGL_value(spectrum,default_material,render_data,data);
					}
					if (number_of_texture_coordinates)
					{
						CAST_TO_OTHER(values, vertex->texture_coordinates, GLfloat, 3);
						glTexCoord3fv(values);
					}
					CAST_TO_OTHER(values, vertex->normal, GLfloat, 3);
					glNormal3fv(values);
					CAST_TO_OTHER(values, vertex->coordinates, GLfloat, 3);
					glVertex3fv(values);
				}
				if (data)
					delete[] data;
			}
			glEnd();
			if (number_of_data_components)
			{
				spectrum_end_renderGL(spectrum,render_data);
			}
			return_code=1;
		}
		else
#endif /* defined (OPENGL_API) */
		{
			display_message(ERROR_MESSAGE,"draw_voltexGL.  Unable to render data.");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"draw_voltexGL.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* draw_voltexGL */

/** Routine that uses the objects material and spectrum to convert
* an array of data to corresponding colour data.
*/
static int Graphics_object_create_colour_buffer_from_data(GT_object *object,
	GLfloat **colour_buffer, unsigned int *colour_values_per_vertex,
	unsigned int *colour_vertex_count)
{
	GLfloat *data_buffer = NULL;
	int return_code;
	unsigned int data_values_per_vertex, data_vertex_count;

	if (object->vertex_array->get_float_vertex_buffer(
		GRAPHICS_VERTEX_ARRAY_ATTRIBUTE_TYPE_DATA,
		&data_buffer, &data_values_per_vertex, &data_vertex_count))
	{
		Spectrum *spectrum = get_GT_object_spectrum(object);
		/* Ignoring selected state here so we don't need to refer to primitive */
		Graphical_material *material = get_GT_object_default_material(object);

		if (ALLOCATE(*colour_buffer, GLfloat, 4 * data_vertex_count))
		{
			GLfloat *data_vertex;
			GLfloat *colour_vertex;
			unsigned int i;

			if (!Spectrum_get_opaque_colour_flag(spectrum))
			{
				Colour diffuse_colour;
				Graphical_material_get_diffuse(material, &diffuse_colour);
				MATERIAL_PRECISION prec;
				Graphical_material_get_alpha(material, &prec);
				GLfloat alpha = prec;

				colour_vertex = *colour_buffer;
				for (i = 0 ; i < data_vertex_count ; i++)
				{
					colour_vertex[0] = diffuse_colour.red;
					colour_vertex[1] = diffuse_colour.green;
					colour_vertex[2] = diffuse_colour.blue;
					colour_vertex[3] = alpha;
					colour_vertex += 4;
				}
			}
			colour_vertex = *colour_buffer;
			data_vertex = data_buffer;
			FE_value *feData = new FE_value[data_values_per_vertex];
			ZnReal colData[4];
			for (i = 0 ; i < data_vertex_count ; i++)
			{
				CAST_TO_FE_VALUE(feData,data_vertex,(int)data_values_per_vertex);
				CAST_TO_OTHER(colData,colour_vertex, ZnReal, 4);
				Spectrum_value_to_rgba(spectrum, data_values_per_vertex,
					feData, colData);
				colour_vertex += 4;
				data_vertex += data_values_per_vertex;
			}
			Spectrum_end_value_to_rgba(spectrum);

			*colour_vertex_count = data_vertex_count;
			*colour_values_per_vertex = 4;
			delete[] feData;
			return_code = 1;
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

	return (return_code);
}

static int Graphics_object_enable_opengl_client_vertex_arrays(GT_object *object,
	Render_graphics_opengl *renderer,
	GLfloat **vertex_buffer, GLfloat **colour_buffer, GLfloat **normal_buffer,
	GLfloat **texture_coordinate0_buffer)
{
	int return_code;

	USE_PARAMETER(renderer);
	if (object && object->vertex_array)
	{
		return_code = 1;
		switch (GT_object_get_type(object))
		{
		case g_POLYLINE_VERTEX_BUFFERS:
			{
				if (object->secondary_material)
				{
					display_message(WARNING_MESSAGE,"Graphics_object_enable_opengl_client_vertex_arrays.  "
						"Multipass rendering not implemented with client vertex arrays.");
				}
				unsigned int position_values_per_vertex, position_vertex_count;
				object->vertex_array->get_float_vertex_buffer(
					GRAPHICS_VERTEX_ARRAY_ATTRIBUTE_TYPE_POSITION,
					vertex_buffer, &position_values_per_vertex, &position_vertex_count);

				glEnableClientState(GL_VERTEX_ARRAY);
				glVertexPointer(position_values_per_vertex, GL_FLOAT,
					/*Packed vertices*/0, /*Client vertex array*/*vertex_buffer);

				unsigned int colour_values_per_vertex, colour_vertex_count;
				*colour_buffer = (GLfloat *)NULL;
				if (Graphics_object_create_colour_buffer_from_data(object,
					colour_buffer,	&colour_values_per_vertex, &colour_vertex_count))
				{
					if (colour_vertex_count == position_vertex_count)
					{
						glEnableClientState(GL_COLOR_ARRAY);
						glColorPointer(colour_values_per_vertex, GL_FLOAT,
							/*Packed vertices*/0, /*Client vertex array*/*colour_buffer);
					}
					else
					{
						DEALLOCATE(*colour_buffer);
					}
				}

				*normal_buffer = NULL;
				unsigned int normal_values_per_vertex, normal_vertex_count;
				if (object->vertex_array->get_float_vertex_buffer(
					GRAPHICS_VERTEX_ARRAY_ATTRIBUTE_TYPE_NORMAL,
					normal_buffer, &normal_values_per_vertex, &normal_vertex_count)
					&& (3 == normal_values_per_vertex))
				{
					glEnableClientState(GL_NORMAL_ARRAY);
					glNormalPointer(GL_FLOAT, /*Packed vertices*/0,
						/*Client vertex array*/*normal_buffer);
				}

				*texture_coordinate0_buffer = NULL;
				unsigned int texture_coordinate0_values_per_vertex,
					texture_coordinate0_vertex_count;
				if (object->vertex_array->get_float_vertex_buffer(
					GRAPHICS_VERTEX_ARRAY_ATTRIBUTE_TYPE_TEXTURE_COORDINATE_ZERO,
					texture_coordinate0_buffer, &texture_coordinate0_values_per_vertex,
					&texture_coordinate0_vertex_count)
					&& (texture_coordinate0_vertex_count == position_vertex_count))
				{
					glEnableClientState(GL_TEXTURE_COORD_ARRAY);
					glTexCoordPointer(texture_coordinate0_values_per_vertex,
						GL_FLOAT, /*Packed vertices*/0,
						/*Client vertex array*/*texture_coordinate0_buffer);
				}
			} break;
		default:
			{
				/* Do nothing, satisfy warnings */
			} break;
		}
	}
	else
	{
		/* Nothing to do */
		return_code = 1;
	}
	return (return_code);
} /* Graphics_object_enable_opengl_client_vertex_arrays */

static int Graphics_object_disable_opengl_client_vertex_arrays(GT_object *object,
	Render_graphics_opengl *renderer,
	GLfloat *vertex_buffer, GLfloat *colour_buffer, GLfloat *normal_buffer,
	GLfloat *texture_coordinate0_buffer)
{
	int return_code;

	USE_PARAMETER(renderer);
	if (object && object->vertex_array)
	{
		return_code = 1;
		switch (GT_object_get_type(object))
		{
		case g_POLYLINE_VERTEX_BUFFERS:
			{
				if (vertex_buffer)
				{
					glDisableClientState(GL_VERTEX_ARRAY);
				}
				if (colour_buffer)
				{
					glDisableClientState(GL_COLOR_ARRAY);
					DEALLOCATE(colour_buffer);
				}
				if (normal_buffer)
				{
					glDisableClientState(GL_NORMAL_ARRAY);
				}
				if (texture_coordinate0_buffer)
				{
					glDisableClientState(GL_TEXTURE_COORD_ARRAY);
				}
			} break;
		default:
			{
				/* Do nothing, satisfy warnings */
			} break;
		}
	}
	else
	{
		/* Nothing to do */
		return_code = 1;
	}
	return (return_code);
} /* Graphics_object_disable_opengl_client_vertex_arrays */

/* Require glDrawBuffers from OpenGL 2.0 */
#if defined (GL_VERSION_2_0)
/***************************************************************************//**
																			 * Uses the secondary material (which is assumed to write GLfloat colour values) to
																			 * calculate a vertex buffer which will be used as the primitive vertex positions
																			 * when finally actually rendering geometry.
																			 */
static int Graphics_object_generate_vertex_positions_from_secondary_material(GT_object *object,
	Render_graphics_opengl *renderer, GLfloat *position_vertex_buffer,
	unsigned int position_values_per_vertex, unsigned int position_vertex_count)
{
	int return_code;

	/* We will be overriding this so we will need to store the original value */
	GLuint object_position_vertex_buffer_object;
	/* Sizes of our first pass render */
	GLuint tex_width = 0, tex_height = 0;

	renderer->Material_compile(object->secondary_material);

	return_code = 1;
	switch (GT_object_get_type(object))
	{
	case g_POLYLINE_VERTEX_BUFFERS:
		{
			/* Add up the number of vertices that we are calculating,
			* we need a pixel in our view buffer for each vertex. */
			/* All lines must be the same length so that the rendered space is rectangular. */
			unsigned int line_count =
				object->vertex_array->get_number_of_vertices(
				GRAPHICS_VERTEX_ARRAY_ATTRIBUTE_TYPE_ELEMENT_INDEX_START);
			unsigned int line_index, line_length;

			object->vertex_array->get_unsigned_integer_attribute(
				GRAPHICS_VERTEX_ARRAY_ATTRIBUTE_TYPE_ELEMENT_INDEX_COUNT,
				/*line_index*/0, 1, &line_length);
			for (line_index = 1 ; line_index < line_count ; line_index++)
			{
				unsigned int index_count;
				object->vertex_array->get_unsigned_integer_attribute(
					GRAPHICS_VERTEX_ARRAY_ATTRIBUTE_TYPE_ELEMENT_INDEX_COUNT,
					line_index, 1, &index_count);
				if (index_count != line_length)
				{
					return_code = 0;
				}
			}

			tex_width = line_length;
			tex_height = line_count;
		} break;
	default:
		{
			/* Do nothing */
		} break;
	}

	if (return_code)
	{
		if (!object->multipass_vertex_buffer_object)
		{
			glGenBuffers(1, &object->multipass_vertex_buffer_object);
		}
		if (!object->multipass_frame_buffer_object)
		{
			glGenFramebuffersEXT(1,&object->multipass_frame_buffer_object);
		}
		if ((object->multipass_width != tex_width)
			|| (object->multipass_height != tex_height))
		{
			GLfloat *temp_position_array, *temp_pointer;
			glBindBuffer(GL_ARRAY_BUFFER, object->multipass_vertex_buffer_object);
			/* Initialise the size to hold the original vertex data and the
			* pass 1 frame buffer positions.
			*/
			glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*
				(position_values_per_vertex + 3)*position_vertex_count,
				NULL, GL_STATIC_DRAW);

			/* Calculate and store the tex1 texture coordinates which are
			* the positions that each vertex must have in the frame buffer. */
			ALLOCATE(temp_position_array, GLfloat,3*position_vertex_count);
			temp_pointer = temp_position_array;
			unsigned int i, j;
			for (j = 0 ; j < tex_height ; j++)
			{
				for (i = 0 ; i < tex_width ; i++)
				{
					//We are rendering a line so the x coordinate covers the
					//full range of the image coordinates.
					*temp_pointer = (GLfloat)i * ((GLfloat)tex_width/(GLfloat)(tex_width - 1));
					temp_pointer++;
					*temp_pointer = (GLfloat)j + 0.5;
					temp_pointer++;
					*temp_pointer = 0.0;
					temp_pointer++;
				}
			}

			/* Set pass 1 frame buffer positions. */
			glBufferSubData(GL_ARRAY_BUFFER,
				/*offset*/sizeof(GLfloat)*position_values_per_vertex*position_vertex_count,
				/*size*/sizeof(GLfloat)*3*position_vertex_count,
				temp_position_array);
			DEALLOCATE(temp_position_array);

			if (object->multipass_frame_buffer_texture)
			{
				glDeleteTextures(1, &object->multipass_frame_buffer_texture);
			}
			object->multipass_frame_buffer_texture = Texture_create_float_texture(
				tex_width, tex_height,
				/*initial_data*/NULL,/*alpha*/1, /*fallback_to_shorts*/1);

			object->multipass_width = tex_width;
			object->multipass_height = tex_height;
		}

		glBindBuffer(GL_ARRAY_BUFFER, object->multipass_vertex_buffer_object);
		glClientActiveTexture(GL_TEXTURE1);
		glTexCoordPointer(/*size*/3, GL_FLOAT, /*Packed vertices*/0,
			/*offset*/(void*)(sizeof(GLfloat)*position_values_per_vertex*position_vertex_count));
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glClientActiveTexture(GL_TEXTURE0);

		/* Load the position vertex buffer into the first half of the array
		* as it will be bound with the standard vertex buffer object
		* rendering code as the position vertex buffer.
		*/
#if !defined (GL_PIXEL_PACK_BUFFER_EXT) && defined (GL_PIXEL_PACK_BUFFER_ARB)
#define GL_PIXEL_PACK_BUFFER_EXT GL_PIXEL_PACK_BUFFER_ARB
#endif

		glBufferSubData(GL_ARRAY_BUFFER,
			/*offset*/0,
			/*size*/sizeof(GLfloat)*position_values_per_vertex*position_vertex_count,
			position_vertex_buffer);
		glBindBuffer(GL_PIXEL_PACK_BUFFER_EXT, object->position_vertex_buffer_object);
		glBufferData(GL_PIXEL_PACK_BUFFER_EXT, sizeof(GLfloat)*
			4*tex_width*tex_height,
			/*position_vertex_buffer*/NULL, GL_STATIC_DRAW);
		object_position_vertex_buffer_object = object->position_vertex_buffer_object;
		object->position_vertex_buffer_object = object->multipass_vertex_buffer_object;
		object->position_values_per_vertex = position_values_per_vertex;

		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, object->multipass_frame_buffer_object);

		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D,
			object->multipass_frame_buffer_texture, 0);

#if defined (NEW_CODE)
		/* Could generate more of attributes by adding new framebuffer attachments and copying
		* these to other buffers.  Currently no way to determine which attributes
		* should be set by the secondary material and used in the final render. */
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_TEXTURE_2D, fbo_tex_normals, 0);
		GLenum dbuffers[] =
		{GL_COLOR_ATTACHMENT0_EXT, GL_COLOR_ATTACHMENT1_EXT};
		glDrawBuffers(2, dbuffers);
#endif /* defined (NEW_CODE) */

		GLenum dbuffers[] =
		{GL_COLOR_ATTACHMENT0_EXT};
		glDrawBuffers(1, dbuffers);

		glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);
		glDisable(GL_CULL_FACE);
		glDisable(GL_ALPHA_TEST);
		glDisable(GL_STENCIL_TEST);
		glDisable(GL_BLEND);

		// Should not be necessary as we should be writing every pixel,
		// but better than random values when debugging if we aren't.
		glClearColor(1, 0.5, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0.0,tex_width,0.0,tex_height,-1.0,1.0);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glViewport(0,0,tex_width,tex_height);

		//Not available on my implementation, maybe we should be setting this
		//if available to ensure the results aren't clamped.
		//glClampColorARB(GL_CLAMP_VERTEX_COLOR_ARB, GL_FALSE);

		renderer->Material_execute(object->secondary_material);

		//Render my vertices here, the coordinates must fill in
		//the render buffer so we expect that the vertex position
		//from this stage will be read from the GL_TEXTURE1 multi
		//tex coordinates, so this is what the secondary material
		//must do.  We could override the vertex position but it
		//seems more useful to make this available to the program
		//if necessary.
		switch (GT_object_get_type(object))
		{
		case g_POLYLINE_VERTEX_BUFFERS:
			{
				/* Render lines with the original positions set as the vertex
				* position array, enabling all the OpenGL attributes defined on the
				* original geometry to be used when calculating the final coordinates. */
				render_GT_object_opengl_immediate(object, /*draw_selected*/0,
					renderer, GRAPHICS_OBJECT_RENDERING_TYPE_VERTEX_BUFFER_OBJECT);
				render_GT_object_opengl_immediate(object, /*draw_selected*/1,
					renderer, GRAPHICS_OBJECT_RENDERING_TYPE_VERTEX_BUFFER_OBJECT);
			} break;
			// A simpler (and therefore faster) case would be not to render with
			// the original geometry but use a single quad here instead.  In this
			// case the secondary material would need to calculate all the vertex
			// values required just from these values supplied here and not the original geometry.
#if defined (NEW_CODE)
		case ALL_VALUES_CALCULATED:
			{
				glBegin(GL_QUADS);
				glColor3f(1,1,1);
				glMultiTexCoord4f( GL_TEXTURE0, 0.0, 0.0, 0.0, 1.0);
				glVertex2f(0, 0);
				glMultiTexCoord4f( GL_TEXTURE0, 1.0, 0.0, 0.0, 1.0);
				glVertex2f(tex_width,0);
				glMultiTexCoord4f( GL_TEXTURE0, 1.0, 1.0, 0.0, 1.0);
				glVertex2f(tex_width, tex_height);
				glMultiTexCoord4f( GL_TEXTURE0, 0.0, 1.0, 0.0, 1.0);
				glVertex2f(0, tex_height);
				glEnd();
			} break;
#endif //defined (NEW_CODE)
		default:
			{
				/* Do nothing */
			} break;
		}

		glClientActiveTexture(GL_TEXTURE1);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glClientActiveTexture(GL_TEXTURE0);

		/*Set the vertex buffer object back to the final position buffer */
		object->position_vertex_buffer_object = object_position_vertex_buffer_object;
		object->position_values_per_vertex = 4;

		glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);
		glBindBuffer(GL_PIXEL_PACK_BUFFER_EXT, object->position_vertex_buffer_object);
		glReadPixels(0, 0, tex_width, tex_height, GL_RGBA, GL_FLOAT, 0);

#if defined (NEW_CODE)
		/* If we were copying other attributes */
		glReadBuffer(GL_COLOR_ATTACHMENT1_EXT);
		glBindBuffer(GL_PIXEL_PACK_BUFFER_EXT, vbo_normals_handle);
		glReadPixels(0, 0, tex_width, tex_height, GL_RGBA, GL_FLOAT, 0);
#endif /* defined (NEW_CODE) */

		glReadBuffer(GL_NONE);
		glBindBuffer(GL_PIXEL_PACK_BUFFER_EXT, 0);

#if defined (DEBUG_CODE)
		// Read the buffer out to memory so we can see the vertex values.
		{
			//GLfloat debugreadbuffer[4 * tex_width * tex_height];
			GLfloat *debugreadbuffer;
			ALLOCATE(debugreadbuffer, GLfloat, 4 * tex_width * tex_height);
			glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);
			glReadPixels(0, 0, tex_width, tex_height, GL_RGBA, GL_FLOAT, debugreadbuffer);
			unsigned int i, j;
			for (i = 0 ; i < tex_height ; i++)
				for (j = 0 ; j < tex_width ; j+=100)
				{
					printf("(%d,%d) %f %f %f %f\n", i, j,
						debugreadbuffer[(i * tex_width + j) * 4 + 0],
						debugreadbuffer[(i * tex_width + j) * 4 + 1],
						debugreadbuffer[(i * tex_width + j) * 4 + 2],
						debugreadbuffer[(i * tex_width + j) * 4 + 3]);
				}
				glReadBuffer(GL_NONE);
				DEALLOCATE(debugreadbuffer);
		}
#endif /* defined (DEBUG_CODE) */

		//Set back to the normal screen
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);


		renderer->Material_execute((Graphical_material *)NULL);
	}

	return (return_code);
} /* Graphics_object_generate_vertex_positions_from_secondary_material */
#endif /* defined (GL_VERSION_2_0) */

/***************************************************************************//**
																			 * Compile Graphics_vertex_array data into vertex buffer objects.
																			 */
static int Graphics_object_compile_opengl_vertex_buffer_object(GT_object *object,
	Render_graphics_opengl *renderer)
{
	int return_code;

	ENTER(Graphics_object_compile_opengl_vertex_buffer_object);

	if (object)
	{
		return_code = 1;
		switch (GT_object_get_type(object))
		{
		case g_POLYLINE_VERTEX_BUFFERS:
			{
				GLfloat *position_vertex_buffer = NULL;
				unsigned int position_values_per_vertex, position_vertex_count;
				if (object->vertex_array->get_float_vertex_buffer(
					GRAPHICS_VERTEX_ARRAY_ATTRIBUTE_TYPE_POSITION,
					&position_vertex_buffer, &position_values_per_vertex,
					&position_vertex_count))
				{
					if (!object->position_vertex_buffer_object)
					{
						glGenBuffers(1, &object->position_vertex_buffer_object);
					}

					if (object->secondary_material)
					{
						/* Defer to lower in this function as we may want to use the contents of
						* some of the other buffers in our first pass calculation.
						*/
					}
					else
					{
						glBindBuffer(GL_ARRAY_BUFFER, object->position_vertex_buffer_object);
						glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*
							position_values_per_vertex*position_vertex_count,
							position_vertex_buffer, GL_STATIC_DRAW);
						object->position_values_per_vertex = position_values_per_vertex;
					}
				}
				else
				{
					if (object->position_vertex_buffer_object)
					{
						glDeleteBuffers(1, &object->position_vertex_buffer_object);
						object->position_vertex_buffer_object = 0;
					}
				}

				unsigned int colour_values_per_vertex, colour_vertex_count;
				GLfloat *colour_buffer = (GLfloat *)NULL;
				if (Graphics_object_create_colour_buffer_from_data(object,
					&colour_buffer,
					&colour_values_per_vertex, &colour_vertex_count)
					&& (colour_vertex_count == position_vertex_count))
				{
					if (!object->colour_vertex_buffer_object)
					{
						glGenBuffers(1, &object->colour_vertex_buffer_object);
					}
					glBindBuffer(GL_ARRAY_BUFFER, object->colour_vertex_buffer_object);
					glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*colour_values_per_vertex*colour_vertex_count,
						colour_buffer, GL_STATIC_DRAW);
					object->colour_values_per_vertex = colour_values_per_vertex;
					if (colour_buffer)
					{
						DEALLOCATE(colour_buffer);
					}
				}
				else
				{
					if (colour_buffer)
					{
						DEALLOCATE(colour_buffer);
					}
					if (object->colour_vertex_buffer_object)
					{
						glDeleteBuffers(1, &object->colour_vertex_buffer_object);
						object->colour_vertex_buffer_object = 0;
					}
				}

				GLfloat *normal_buffer = NULL;
				unsigned int normal_values_per_vertex, normal_vertex_count;
				if (object->vertex_array->get_float_vertex_buffer(
					GRAPHICS_VERTEX_ARRAY_ATTRIBUTE_TYPE_NORMAL,
					&normal_buffer, &normal_values_per_vertex, &normal_vertex_count)
					&& (3 == normal_values_per_vertex))
				{
					if (!object->normal_vertex_buffer_object)
					{
						glGenBuffers(1, &object->normal_vertex_buffer_object);
					}
					glBindBuffer(GL_ARRAY_BUFFER, object->normal_vertex_buffer_object);
					glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*normal_values_per_vertex*normal_vertex_count,
						normal_buffer, GL_STATIC_DRAW);
				}
				else
				{
					if (object->normal_vertex_buffer_object)
					{
						glDeleteBuffers(1, &object->normal_vertex_buffer_object);
						object->normal_vertex_buffer_object = 0;
					}
				}

				GLfloat *texture_coordinate0_buffer = NULL;
				unsigned int texture_coordinate0_values_per_vertex,
					texture_coordinate0_vertex_count;
				if (object->vertex_array->get_float_vertex_buffer(
					GRAPHICS_VERTEX_ARRAY_ATTRIBUTE_TYPE_TEXTURE_COORDINATE_ZERO,
					&texture_coordinate0_buffer, &texture_coordinate0_values_per_vertex,
					&texture_coordinate0_vertex_count)
					&& (texture_coordinate0_vertex_count == position_vertex_count))
				{
					if (!object->texture_coordinate0_vertex_buffer_object)
					{
						glGenBuffers(1, &object->texture_coordinate0_vertex_buffer_object);
					}
					glBindBuffer(GL_ARRAY_BUFFER, object->texture_coordinate0_vertex_buffer_object);
					glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*texture_coordinate0_values_per_vertex*texture_coordinate0_vertex_count,
						texture_coordinate0_buffer, GL_STATIC_DRAW);
					object->texture_coordinate0_values_per_vertex = texture_coordinate0_values_per_vertex;
				}
				else
				{
					if (object->texture_coordinate0_vertex_buffer_object)
					{
						glDeleteBuffers(1, &object->texture_coordinate0_vertex_buffer_object);
						object->texture_coordinate0_vertex_buffer_object = 0;
					}
				}

				if (position_vertex_buffer && object->secondary_material)
				{
#if defined (GL_VERSION_2_0) && defined (GL_EXT_framebuffer_object)
					if (Graphics_library_check_extension(GL_ARB_draw_buffers)
						/* Need to check extension to load multitexture functions */
						&& Graphics_library_check_extension(GL_VERSION_1_3)
						&& Graphics_library_check_extension(GL_EXT_framebuffer_object))
					{
						return_code = Graphics_object_generate_vertex_positions_from_secondary_material(
							object, renderer, position_vertex_buffer, position_values_per_vertex,
							position_vertex_count);
					}
					else
					{
						display_message(ERROR_MESSAGE,"Graphics_object_compile_opengl_vertex_buffer_object.  "
							"Multipass rendering not available with this OpenGL driver.");
					}
#else /* defined (GL_VERSION_2_0) && defined (GL_EXT_framebuffer_object) */
					display_message(ERROR_MESSAGE,"Graphics_object_compile_opengl_vertex_buffer_object.  "
						"Multipass rendering not compiled in this version.");
#endif /* defined (GL_VERSION_2_0) && defined (GL_EXT_framebuffer_object) */
				}
			} break;
		default:
			{
				/* Do nothing */
			} break;
		}
	}
	else
	{
		/* Nothing to do */
		return_code = 1;
	}
	return (return_code);
} /* Graphics_object_compile_opengl_vertex_buffer_object */

/***************************************************************************//**
																			 * Set the OpenGL state to use vertex buffer objects that have been previously
																			 * created for this object.
																			 */
static int Graphics_object_enable_opengl_vertex_buffer_object(GT_object *object,
	Render_graphics_opengl *renderer)
{
	int return_code;

	USE_PARAMETER(renderer);
	if (object && object->vertex_array)
	{
		return_code = 1;
		switch (GT_object_get_type(object))
		{
		case g_POLYLINE_VERTEX_BUFFERS:
			{
				if (object->position_vertex_buffer_object)
				{
					glBindBuffer(GL_ARRAY_BUFFER,
						object->position_vertex_buffer_object);
					glEnableClientState(GL_VERTEX_ARRAY);
					glVertexPointer(
						object->position_values_per_vertex,
						GL_FLOAT, /*Packed vertices*/0,
						/*No offset in vertex array*/(void *)0);
				}
				if (object->colour_vertex_buffer_object)
				{
					glBindBuffer(GL_ARRAY_BUFFER,
						object->colour_vertex_buffer_object);
					glEnableClientState(GL_COLOR_ARRAY);
					glColorPointer(
						object->colour_values_per_vertex,
						GL_FLOAT, /*Packed vertices*/0,
						/*No offset in vertex array*/(void *)0);
				}
				if (object->normal_vertex_buffer_object)
				{
					glBindBuffer(GL_ARRAY_BUFFER,
						object->normal_vertex_buffer_object);
					glEnableClientState(GL_NORMAL_ARRAY);
					glNormalPointer(
						GL_FLOAT, /*Packed vertices*/0,
						/*No offset in vertex array*/(void *)0);
				}
				if (object->texture_coordinate0_vertex_buffer_object)
				{
					glBindBuffer(GL_ARRAY_BUFFER,
						object->texture_coordinate0_vertex_buffer_object);
					glEnableClientState(GL_TEXTURE_COORD_ARRAY);
					glTexCoordPointer(
						object->texture_coordinate0_values_per_vertex,
						GL_FLOAT, /*Packed vertices*/0,
						/*No offset in vertex array*/(void *)0);
				}
			} break;
		default:
			{
				/* Do nothing */
			} break;
		}
	}
	else
	{
		/* Nothing to do */
		return_code = 1;
	}
	return (return_code);
} /* Graphics_object_enable_opengl_client_vertex_arrays */

/***************************************************************************//**
																			 * Reset the OpenGL state to not use vertex buffer objects.
																			 */
static int Graphics_object_disable_opengl_vertex_buffer_object(GT_object *object,
	Render_graphics_opengl *renderer)
{
	int return_code;

	USE_PARAMETER(renderer);
	if (object && object->vertex_array)
	{
		return_code = 1;
		switch (GT_object_get_type(object))
		{
		case g_POLYLINE_VERTEX_BUFFERS:
			{
				if (object->position_vertex_buffer_object)
				{
					glDisableClientState(GL_VERTEX_ARRAY);
				}
				if (object->colour_vertex_buffer_object)
				{
					glDisableClientState(GL_COLOR_ARRAY);
				}
				if (object->normal_vertex_buffer_object)
				{
					glDisableClientState(GL_NORMAL_ARRAY);
				}
				if (object->texture_coordinate0_vertex_buffer_object)
				{
					glDisableClientState(GL_TEXTURE_COORD_ARRAY);
				}
				/* Reset to normal client vertex arrays */
				glBindBuffer(GL_ARRAY_BUFFER, 0);
			} break;
		default:
			{
				/* Do nothing */
			} break;
		}
	}
	else
	{
		/* Nothing to do */
		return_code = 1;
	}
	return (return_code);
} /* Graphics_object_enable_opengl_client_vertex_arrays */

static int render_GT_object_opengl_immediate(gtObject *object,
	int draw_selected, Render_graphics_opengl *renderer,
	Graphics_object_rendering_type rendering_type)
{
	ZnReal proportion = 0.0,*times;
	int itime, name_selected, number_of_times, picking_names, return_code, strip;
#if defined (OPENGL_API)
	int lighting_off, line_width;
#endif /* defined (OPENGL_API) */
	struct Graphical_material *material, *secondary_material;
	struct GT_glyph_set *interpolate_glyph_set,*glyph_set,*glyph_set_2;
	struct GT_nurbs *nurbs;
	struct GT_point *point;
	struct GT_pointset *interpolate_point_set,*point_set,*point_set_2;
	struct GT_polyline *interpolate_line,*line = NULL,*line_2 = NULL;
	struct GT_surface *interpolate_surface,*surface = NULL,*surface_2 = NULL,*tile_surface,
		*tile_surface_2;
	struct GT_userdef *userdef;
	struct GT_voltex *voltex;
	struct Spectrum *spectrum;
	union GT_primitive_list *primitive_list1 = NULL, *primitive_list2 = NULL;

	ENTER(render_GT_object_opengl_immediate)
		if (object)
		{
			return_code = 1;
			spectrum=get_GT_object_spectrum(object);
			/* determine if picking names are to be output */
			picking_names = renderer->picking &&
				(GRAPHICS_NO_SELECT != GT_object_get_select_mode(object));
			/* determine which material to use */
			if (draw_selected)
			{
				material = get_GT_object_selected_material(object);
			}
			else
			{
				material = get_GT_object_default_material(object);
			}
			secondary_material = get_GT_object_secondary_material(object);
			number_of_times = GT_object_get_number_of_times(object);
			if (0 < number_of_times)
			{
				itime = number_of_times;
				if ((itime > 1) && (times = object->times))
				{
					itime--;
					times += itime;
					if (renderer->time >= *times)
					{
						proportion = 0;
					}
					else
					{
						while ((itime>0)&&(renderer->time < *times))
						{
							itime--;
							times--;
						}
						if (renderer->time< *times)
						{
							proportion=0;
						}
						else
						{
							proportion=times[1]-times[0];
							if (proportion>0)
							{
								proportion=(renderer->time-times[0])/proportion;
							}
							else
							{
								proportion=0;
							}
						}
					}
				}
				else
				{
					itime = 0;
					proportion = 0;
				}
				if (object->primitive_lists &&
					(primitive_list1 = object->primitive_lists + itime))
				{
					if (proportion > 0)
					{
						if (!(primitive_list2 = object->primitive_lists + itime + 1))
						{
							display_message(ERROR_MESSAGE,
								"render_GT_object_opengl_immediate.  Invalid primitive_list");
							return_code = 0;
						}
					}
					else
					{
						primitive_list2 = (union GT_primitive_list *)NULL;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"render_GT_object_opengl_immediate.  Invalid primitive_lists");
					return_code = 0;
				}
			}
			if ((0 < number_of_times) && return_code)
			{
				switch (GT_object_get_type(object))
				{
				case g_GLYPH_SET:
					{
						glyph_set = primitive_list1->gt_glyph_set.first;
						if (glyph_set)
						{
							lighting_off = 0;
#if defined (OPENGL_API)
							// push enable bit as lighting is switched off and on depending on
							// whether glyph is points or lines vs. surfaces, whether labels are drawn
							glPushAttrib(GL_ENABLE_BIT);
							/* store the transform attribute group to save current matrix mode
							and GL_NORMALIZE flag. */
							glPushAttrib(GL_TRANSFORM_BIT);
							/* Must enable GL_NORMALIZE so that normals are normalized after
							scaling and shear by the transformations in the glyph set -
							otherwise lighting will be wrong. Since this may reduce
							performance, only enable for glyph_sets. */
							glEnable(GL_NORMALIZE);
							if (picking_names)
							{
								glPushName(0);
							}
#endif /* defined (OPENGL_API) */
							if (proportion > 0)
							{
								glyph_set_2 = primitive_list2->gt_glyph_set.first;
								while (glyph_set&&glyph_set_2)
								{
									interpolate_glyph_set=morph_GT_glyph_set(proportion,
										glyph_set,glyph_set_2);
									if (interpolate_glyph_set)
									{
										if (picking_names)
										{
											/* put out name for picking - cast to GLuint */
											glLoadName((GLuint)interpolate_glyph_set->object_name);
										}
										/* work out if subobjects selected  */
										draw_glyphsetGL(interpolate_glyph_set->number_of_points,
											interpolate_glyph_set->point_list,
											interpolate_glyph_set->axis1_list,
											interpolate_glyph_set->axis2_list,
											interpolate_glyph_set->axis3_list,
											interpolate_glyph_set->scale_list,
											interpolate_glyph_set->glyph,
											interpolate_glyph_set->labels,
											interpolate_glyph_set->n_data_components,
											interpolate_glyph_set->data,
											interpolate_glyph_set->names,
											/*label_bounds_dimension*/0, /*label_bounds_components*/0, /*label_bounds*/(ZnReal *)NULL,
											/*label_density*/NULL,
											material, secondary_material, spectrum,
											interpolate_glyph_set->font,
											draw_selected, renderer->highlight_functor,
											renderer, glyph_set->object_name,
											&lighting_off);
										DESTROY(GT_glyph_set)(&interpolate_glyph_set);
									}
									glyph_set=glyph_set->ptrnext;
									glyph_set_2=glyph_set_2->ptrnext;
								}
							}
							else
							{
								while (glyph_set)
								{
									if (picking_names)
									{
										/* put out name for picking - cast to GLuint */
										glLoadName((GLuint)glyph_set->object_name);
									}
									/* work out if subobjects selected  */
									draw_glyphsetGL(glyph_set->number_of_points,
										glyph_set->point_list, glyph_set->axis1_list,
										glyph_set->axis2_list, glyph_set->axis3_list,
										glyph_set->scale_list, glyph_set->glyph,
										glyph_set->labels, glyph_set->n_data_components,
										glyph_set->data, glyph_set->names,
										glyph_set->label_bounds_dimension, glyph_set->label_bounds_components,
										glyph_set->label_bounds, glyph_set->label_density_list,
										material, secondary_material,
										spectrum, glyph_set->font,
										draw_selected, renderer->highlight_functor,
										renderer, glyph_set->object_name,
										&lighting_off);
									glyph_set=glyph_set->ptrnext;
								}
							}
#if defined (OPENGL_API)
							if (picking_names)
							{
								glPopName();
							}
							/* restore the transform attribute group */
							glPopAttrib();
							/* restore previous lighting state */
							glPopAttrib();
#endif /* defined (OPENGL_API) */
							return_code=1;
						}
						else
						{
							display_message(ERROR_MESSAGE,"render_GT_object_opengl_immediate.  Missing glyph_set");
							return_code=0;
						}
					} break;
				case g_POINT:
					{
						point = primitive_list1->gt_point.first;
						if (point)
						{
							draw_pointsetGL(1, point->position, &(point->text),
								point->marker_type,
								point->marker_size, /*names*/(int *)NULL,
								point->n_data_components, point->data,
								material,spectrum,point->font);
							return_code=1;
						}
						else
						{
							display_message(ERROR_MESSAGE,"render_GT_object_opengl_immediate.  Missing point");
							return_code=0;
						}
					} break;
				case g_POINTSET:
					{
						point_set = primitive_list1->gt_pointset.first;
						if (point_set)
						{
#if defined (OPENGL_API)
							/* disable lighting so rendered in flat diffuse colour */
							/*???RC glPushAttrib and glPopAttrib are *very* slow */
							glPushAttrib(GL_ENABLE_BIT);
							glDisable(GL_LIGHTING);
#endif /* defined (OPENGL_API) */
							if (proportion>0)
							{
								point_set_2 = primitive_list2->gt_pointset.first;
								while (point_set&&point_set_2)
								{
									interpolate_point_set=morph_GT_pointset(proportion,
										point_set,point_set_2);
									if (interpolate_point_set)
									{
										draw_pointsetGL(interpolate_point_set->n_pts,
											interpolate_point_set->pointlist,
											interpolate_point_set->text,
											interpolate_point_set->marker_type,
											interpolate_point_set->marker_size, point_set->names,
											interpolate_point_set->n_data_components,
											interpolate_point_set->data,
											material,spectrum,interpolate_point_set->font);
										DESTROY(GT_pointset)(&interpolate_point_set);
									}
									point_set=point_set->ptrnext;
									point_set_2=point_set_2->ptrnext;
								}
							}
							else
							{
								while (point_set)
								{
									draw_pointsetGL(point_set->n_pts,point_set->pointlist,
										point_set->text,point_set->marker_type,point_set->marker_size,
										point_set->names,point_set->n_data_components,point_set->data,
										material,spectrum,point_set->font);
									point_set=point_set->ptrnext;
								}
							}
#if defined (OPENGL_API)
							/* restore previous lighting state */
							glPopAttrib();
#endif /* defined (OPENGL_API) */
							return_code=1;
						}
						else
						{
							/*???debug*/printf("! render_GT_object_opengl_immediate.  Missing point");
							display_message(ERROR_MESSAGE,"render_GT_object_opengl_immediate.  Missing point");
							return_code=0;
						}
					} break;
				case g_VOLTEX:
					{
						voltex = primitive_list1->gt_voltex.first;
#if defined (OPENGL_API)
						/* save transformation attributes state */
						if (voltex)
						{
							if (voltex->voltex_type == g_VOLTEX_WIREFRAME_SHADED_TEXMAP)
							{
								glPushAttrib(GL_TRANSFORM_BIT | GL_POLYGON_BIT);
								glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
							}
							else
							{
								glPushAttrib(GL_TRANSFORM_BIT);
							}
							/*???RC Why do we need NORMALIZE on for voltex? */
							glEnable(GL_NORMALIZE);
							if (picking_names)
							{
								glPushName(0);
							}
#endif /* defined (OPENGL_API) */
							while (voltex)
							{
								/* work out if subobjects selected */
								name_selected=0;
								if ((name_selected&&draw_selected)||
									((!name_selected)&&(!draw_selected)))
								{
									if (picking_names)
									{
										/* put out name for picking - cast to GLuint */
										glLoadName(voltex->object_name);
									}
									draw_voltexGL(voltex->number_of_vertices, voltex->vertex_list,
										voltex->number_of_triangles, voltex->triangle_list,
										voltex->n_data_components, voltex->n_texture_coordinates,
										material,spectrum);
								}
								voltex=voltex->ptrnext;
							}
							return_code=1;
#if defined (OPENGL_API)
							if (picking_names)
							{
								glPopName();
							}
							/* restore previous coloring state */
							glPopAttrib();
#endif /* defined (OPENGL_API) */
						}
					} break;
				case g_POLYLINE:
					{
						/*???debug */
						/*printf("g_POLYLINE time=%g proportion=%g\n",time,proportion);*/
						line = primitive_list1->gt_polyline.first;
						if (line)
						{
							if (proportion>0)
							{
								line_2 = primitive_list2->gt_polyline.first;
							}
#if defined (OPENGL_API)
							if (0 != (lighting_off=((g_PLAIN == line->polyline_type)||
								(g_PLAIN_DISCONTINUOUS == line->polyline_type))))
							{
								/* disable lighting so rendered in flat diffuse colour */
								/*???RC glPushAttrib and glPopAttrib are *very* slow */
								glPushAttrib(GL_ENABLE_BIT);
								glDisable(GL_LIGHTING);
							}
							if (picking_names)
							{
								glPushName(0);
							}
#endif /* defined (OPENGL_API) */
							switch (line->polyline_type)
							{
							case g_PLAIN:
							case g_NORMAL:
								{
									if (proportion>0)
									{
										while (line&&line_2)
										{
											/* work out if subobjects selected */
											if (renderer->highlight_functor)
											{
												name_selected=(renderer->highlight_functor)->call(line->object_name);
											}
											else
											{
												name_selected = 0;
											}
											if ((name_selected&&draw_selected)||
												((!name_selected)&&(!draw_selected)))
											{
												interpolate_line=morph_GT_polyline(proportion,
													line,line_2);
												if (interpolate_line)
												{
													if (picking_names)
													{
														/* put out name for picking - cast to GLuint */
														glLoadName((GLuint)interpolate_line->object_name);
													}
													draw_polylineGL(interpolate_line->pointlist,
														interpolate_line->normallist, interpolate_line->n_pts,
														interpolate_line->n_data_components,
														interpolate_line->data, material,
														spectrum);
													DESTROY(GT_polyline)(&interpolate_line);
												}
											}
											line=line->ptrnext;
											line_2=line_2->ptrnext;
										}
									}
									else
									{
										line_width = 0;
										while (line)
										{
											/* work out if subobjects selected */
											if (renderer->highlight_functor)
											{
												name_selected=(renderer->highlight_functor)->call(line->object_name);
											}
											else
											{
												name_selected = 0;
											}
											if ((name_selected&&draw_selected)||
												((!name_selected)&&(!draw_selected)))
											{
												if (picking_names)
												{
													/* put out name for picking - cast to GLuint */
													glLoadName((GLuint)line->object_name);
												}
												if (line->line_width != line_width)
												{
													if (line->line_width)
													{
														glLineWidth(line->line_width);
													}
													else
													{
														glLineWidth(global_line_width);
													}
													line_width = line->line_width;
												}
												draw_polylineGL(line->pointlist,line->normallist,
													line->n_pts, line->n_data_components, line->data,
													material,spectrum);
											}
											line=line->ptrnext;
										}
										if (line_width)
										{
											glLineWidth(global_line_width);
										}
									}
									return_code=1;
								} break;
							case g_PLAIN_DISCONTINUOUS:
							case g_NORMAL_DISCONTINUOUS:
								{
									if (proportion>0)
									{
										while (line&&line_2)
										{
											/* work out if subobjects selected */
											if (renderer->highlight_functor)
											{
												name_selected=(renderer->highlight_functor)->call(line->object_name);
											}
											else
											{
												name_selected = 0;
											}
											if ((name_selected&&draw_selected)||
												((!name_selected)&&(!draw_selected)))
											{
												interpolate_line=morph_GT_polyline(proportion,line,
													line_2);
												if (interpolate_line)
												{
													if (picking_names)
													{
														/* put out name for picking - cast to GLuint */
														glLoadName((GLuint)interpolate_line->object_name);
													}
													draw_dc_polylineGL(interpolate_line->pointlist,
														interpolate_line->normallist, interpolate_line->n_pts,
														interpolate_line->n_data_components,
														interpolate_line->data,
														material,spectrum);
													DESTROY(GT_polyline)(&interpolate_line);
												}
											}
											line=line->ptrnext;
											line_2=line_2->ptrnext;
										}
									}
									else
									{
										while (line)
										{
											/* work out if subobjects selected */
											if (renderer->highlight_functor)
											{
												name_selected=(renderer->highlight_functor)->call(line->object_name);
											}
											else
											{
												name_selected = 0;
											}
											if ((name_selected&&draw_selected)||
												((!name_selected)&&(!draw_selected)))
											{
												if (picking_names)
												{
													/* put out name for picking - cast to GLuint */
													glLoadName((GLuint)line->object_name);
												}
												draw_dc_polylineGL(line->pointlist,line->normallist,
													line->n_pts,line->n_data_components,line->data,
													material,spectrum);
											}
											line=line->ptrnext;
										}
									}
									return_code=1;
								} break;
							default:
								{
									display_message(ERROR_MESSAGE,
										"render_GT_object_opengl_immediate.  Invalid line type");
									return_code=0;
								} break;
							}
#if defined (OPENGL_API)
							if (picking_names)
							{
								glPopName();
							}
							if (lighting_off)
							{
								/* restore previous lighting state */
								glPopAttrib();
							}
#endif /* defined (OPENGL_API) */
						}
						else
						{
							/*???debug*/printf("! render_GT_object_opengl_immediate.  Missing line");
							display_message(ERROR_MESSAGE,"render_GT_object_opengl_immediate.  Missing line");
							return_code=0;
						}
					} break;
				case g_POLYLINE_VERTEX_BUFFERS:
					{
						GT_polyline_vertex_buffers *line;
						line = primitive_list1->gt_polyline_vertex_buffers;
						if (line)
						{
							if (0 != (lighting_off=((g_PLAIN == line->polyline_type)||
								(g_PLAIN_DISCONTINUOUS == line->polyline_type))))
							{
								/* disable lighting so rendered in flat diffuse colour */
								/*???RC glPushAttrib and glPopAttrib are *very* slow */
								glPushAttrib(GL_ENABLE_BIT);
								glDisable(GL_LIGHTING);
							}
							if (picking_names)
							{
								glPushName(0);
							}
							return_code = 1;
							GLenum mode;
							switch (line->polyline_type)
							{
							case g_PLAIN:
							case g_NORMAL:
								{
									mode = GL_LINE_STRIP;
								} break;
							case g_PLAIN_DISCONTINUOUS:
							case g_NORMAL_DISCONTINUOUS:
								{
									mode = GL_LINES;
								} break;
							default:
								{
									display_message(ERROR_MESSAGE,
										"render_GT_object_opengl_immediate.  Invalid line type");
									return_code=0;
								} break;
							}
							if (return_code)
							{
								if (line->line_width != 0)
								{
									glLineWidth(line->line_width);
								}
								else
								{
									glLineWidth(global_line_width);
								}

								unsigned int line_index;
								unsigned int line_count =
									object->vertex_array->get_number_of_vertices(
									GRAPHICS_VERTEX_ARRAY_ATTRIBUTE_TYPE_ELEMENT_INDEX_START);

								GLfloat *position_buffer, *data_buffer, *normal_buffer,
									*texture_coordinate0_buffer;
								struct Spectrum_render_data *render_data = NULL;
								unsigned int position_values_per_vertex, position_vertex_count,
									data_values_per_vertex, data_vertex_count, normal_values_per_vertex,
									normal_vertex_count, texture_coordinate0_values_per_vertex,
									texture_coordinate0_vertex_count;

								position_buffer = (GLfloat *)NULL;
								data_buffer = (GLfloat *)NULL;
								normal_buffer = (GLfloat *)NULL;
								texture_coordinate0_buffer = (GLfloat *)NULL;

								switch (rendering_type)
								{
								case GRAPHICS_OBJECT_RENDERING_TYPE_GLBEGINEND:
									{
										object->vertex_array->get_float_vertex_buffer(
											GRAPHICS_VERTEX_ARRAY_ATTRIBUTE_TYPE_POSITION,
											&position_buffer, &position_values_per_vertex, &position_vertex_count);

										object->vertex_array->get_float_vertex_buffer(
											GRAPHICS_VERTEX_ARRAY_ATTRIBUTE_TYPE_DATA,
											&data_buffer, &data_values_per_vertex, &data_vertex_count);
										if (data_buffer)
										{
											render_data=spectrum_start_renderGL
												(spectrum,material,data_values_per_vertex);
										}

										object->vertex_array->get_float_vertex_buffer(
											GRAPHICS_VERTEX_ARRAY_ATTRIBUTE_TYPE_NORMAL,
											&normal_buffer, &normal_values_per_vertex, &normal_vertex_count);
										object->vertex_array->get_float_vertex_buffer(
											GRAPHICS_VERTEX_ARRAY_ATTRIBUTE_TYPE_TEXTURE_COORDINATE_ZERO,
											&texture_coordinate0_buffer, &texture_coordinate0_values_per_vertex,
											&texture_coordinate0_vertex_count);

										if (object->secondary_material)
										{
											display_message(WARNING_MESSAGE,"render_GT_object_opengl_immediate.  "
												"Multipass rendering not implemented with glbegin/glend rendering.");
										}
									} break;
								case GRAPHICS_OBJECT_RENDERING_TYPE_CLIENT_VERTEX_ARRAYS:
									{
										Graphics_object_enable_opengl_client_vertex_arrays(
											object, renderer,
											&position_buffer, &data_buffer, &normal_buffer,
											&texture_coordinate0_buffer);
									} break;
								case GRAPHICS_OBJECT_RENDERING_TYPE_VERTEX_BUFFER_OBJECT:
									{
										Graphics_object_enable_opengl_vertex_buffer_object(
											object, renderer);
									} break;
								}

								for (line_index = 0; line_index < line_count; line_index++)
								{
									int object_name;
									object->vertex_array->get_integer_attribute(
										GRAPHICS_VERTEX_ARRAY_ATTRIBUTE_TYPE_ID,
										line_index, 1, &object_name);

									/* work out if subobjects selected */
									if (renderer->highlight_functor)
									{
										name_selected=(renderer->highlight_functor)->call(object_name);
									}
									else
									{
										name_selected = 0;
									}
									if ((name_selected&&draw_selected)||
										((!name_selected)&&(!draw_selected)))
									{
										if (picking_names)
										{
											/* put out name for picking - cast to GLuint */
											glLoadName((GLuint)object_name);
										}
										unsigned int i, index_start, index_count;
										GLfloat *position_vertex = NULL, *data_vertex = NULL, *normal_vertex = NULL,
											*texture_coordinate0_vertex = NULL;

										object->vertex_array->get_unsigned_integer_attribute(
											GRAPHICS_VERTEX_ARRAY_ATTRIBUTE_TYPE_ELEMENT_INDEX_START,
											line_index, 1, &index_start);
										object->vertex_array->get_unsigned_integer_attribute(
											GRAPHICS_VERTEX_ARRAY_ATTRIBUTE_TYPE_ELEMENT_INDEX_COUNT,
											line_index, 1, &index_count);

										switch (rendering_type)
										{
										case GRAPHICS_OBJECT_RENDERING_TYPE_GLBEGINEND:
											{
												position_vertex = position_buffer +
													position_values_per_vertex * index_start;
												if (data_buffer)
												{
													data_vertex = data_buffer +
														data_values_per_vertex * index_start;
												}
												if (normal_buffer)
												{
													normal_vertex = normal_buffer +
														normal_values_per_vertex * index_start;
												}
												if (texture_coordinate0_buffer)
												{
													texture_coordinate0_vertex = texture_coordinate0_buffer +
														texture_coordinate0_values_per_vertex * index_start;
												}

												glBegin(mode);
												for (i=index_count;i>0;i--)
												{
													if (data_buffer)
													{
														spectrum_renderGL_value(spectrum,material,render_data,data_vertex);
														data_vertex += data_values_per_vertex;
													}
													if (normal_buffer)
													{
														glNormal3fv(normal_vertex);
														normal_vertex += normal_values_per_vertex;
													}
													if (texture_coordinate0_buffer)
													{
														glTexCoord3fv(texture_coordinate0_vertex);
														texture_coordinate0_vertex += texture_coordinate0_values_per_vertex;
													}
													glVertex3fv(position_vertex);
													position_vertex += position_values_per_vertex;
												}
												glEnd();
											} break;
										case GRAPHICS_OBJECT_RENDERING_TYPE_CLIENT_VERTEX_ARRAYS:
										case GRAPHICS_OBJECT_RENDERING_TYPE_VERTEX_BUFFER_OBJECT:
											{
												glDrawArrays(mode, index_start, index_count);
											} break;
										}
									}
								}
								switch (rendering_type)
								{
								case GRAPHICS_OBJECT_RENDERING_TYPE_GLBEGINEND:
									{
										if (data_buffer)
										{
											spectrum_end_renderGL(spectrum, render_data);
										}
									} break;
								case GRAPHICS_OBJECT_RENDERING_TYPE_CLIENT_VERTEX_ARRAYS:
									{
										Graphics_object_disable_opengl_client_vertex_arrays(
											object, renderer,
											position_buffer, data_buffer, normal_buffer,
											texture_coordinate0_buffer);
									} break;
								case GRAPHICS_OBJECT_RENDERING_TYPE_VERTEX_BUFFER_OBJECT:
									{
										Graphics_object_disable_opengl_vertex_buffer_object(
											object, renderer);
									}
								}
							}
							if (picking_names)
							{
								glPopName();
							}
							if (lighting_off)
							{
								/* restore previous lighting state */
								glPopAttrib();
							}
						}
						else
						{
							/*???debug*/printf("! render_GT_object_opengl_immediate.  Missing line");
							display_message(ERROR_MESSAGE,"render_GT_object_opengl_immediate.  Missing line");
							return_code=0;
						}
					} break;
				case g_SURFACE:
					{
						surface = primitive_list1->gt_surface.first;
						if (surface)
						{
#if defined (OPENGL_API)
							if (picking_names)
							{
								glPushName(0);
							}
							bool wireframe_flag = (surface->render_type == CMISS_GRAPHICS_RENDER_TYPE_WIREFRAME);
							if (wireframe_flag)
							{
								glPushAttrib(GL_POLYGON_BIT);
								glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
							}
							struct Spectrum_render_data *spectrum_render_data = NULL;
							if (surface->data)
							{
								spectrum_render_data = spectrum_start_renderGL(spectrum,material,surface->n_data_components);
							}
#endif /* defined (OPENGL_API) */
							if (proportion>0)
							{
								surface_2 = primitive_list2->gt_surface.first;
							}
							switch (surface->surface_type)
							{
							case g_SHADED:
							case g_SHADED_TEXMAP:
								{
									if (proportion>0)
									{
										while (surface&&surface_2)
										{
											/* work out if subobjects selected */
											if (renderer->highlight_functor)
											{
												name_selected=(renderer->highlight_functor)->call(surface->object_name);
											}
											else
											{
												name_selected = 0;
											}
											if ((name_selected&&draw_selected)||
												((!name_selected)&&(!draw_selected)))
											{
												interpolate_surface=morph_GT_surface(proportion,
													surface,surface_2);
												if (interpolate_surface)
												{
													if (picking_names)
													{
														/* put out name for picking - cast to GLuint */
														glLoadName((GLuint)interpolate_surface->object_name);
													}
													if (object->texture_tiling &&
														interpolate_surface->texturelist)
													{
														tile_surface = tile_GT_surface(interpolate_surface,
															object->texture_tiling);
														while(tile_surface)
														{
															/* Need to select the texture target */
															Texture_tiling_activate_tile(object->texture_tiling,
																tile_surface->tile_number);
#if defined (OPENGL_API)
															GLenum gl_surface_mode = GL_POINTS; // Not a valid surface primitive
#endif /* defined (OPENGL_API) */
															draw_dc_surfaceGL(tile_surface->pointlist,
																tile_surface->normallist,
																tile_surface->tangentlist,
																tile_surface->texturelist,
																tile_surface->n_pts1,
																tile_surface->n_pts2,
																tile_surface->polygon,  /*strip*/0,
#if defined (OPENGL_API)
																gl_surface_mode,
#endif /* defined (OPENGL_API) */
																tile_surface->n_data_components,
																tile_surface->data,
																material, spectrum, spectrum_render_data);
#if defined (OPENGL_API)
															if (gl_surface_mode != GL_POINTS)
															{
																glEnd();
															}
#endif /* defined (OPENGL_API) */
															tile_surface_2 = tile_surface;
															tile_surface = tile_surface->ptrnext;
															DESTROY(GT_surface)(&tile_surface_2);
														}
													}
													else
													{
														draw_surfaceGL(interpolate_surface->pointlist,
															interpolate_surface->normallist,
															interpolate_surface->tangentlist,
															interpolate_surface->texturelist,
															interpolate_surface->n_pts1,
															interpolate_surface->n_pts2,
															interpolate_surface->polygon,
															interpolate_surface->n_data_components,
															interpolate_surface->data,
															material, spectrum, spectrum_render_data);
													}
													DESTROY(GT_surface)(&interpolate_surface);
												}
											}
											surface=surface->ptrnext;
											surface_2=surface_2->ptrnext;
										}
									}
									else
									{
										while (surface)
										{
											/* work out if subobjects selected */
											if (renderer->highlight_functor)
											{
												name_selected=(renderer->highlight_functor)->call(surface->object_name);
											}
											else
											{
												name_selected = 0;
											}
											if ((name_selected&&draw_selected)||
												((!name_selected)&&(!draw_selected)))
											{
												if (picking_names)
												{
													/* put out name for picking - cast to GLuint */
													glLoadName((GLuint)surface->object_name);
												}
												if (object->texture_tiling &&
													surface->texturelist)
												{
													tile_surface = tile_GT_surface(surface,
														object->texture_tiling);
													while(tile_surface)
													{
														Texture_tiling_activate_tile(object->texture_tiling,
															tile_surface->tile_number);
#if defined (OPENGL_API)
														GLenum gl_surface_mode = GL_POINTS; // Not a valid surface primitive
#endif /* defined (OPENGL_API) */
														draw_dc_surfaceGL(tile_surface->pointlist,
															tile_surface->normallist,
															tile_surface->tangentlist,
															tile_surface->texturelist,
															tile_surface->n_pts1,
															tile_surface->n_pts2,
															tile_surface->polygon, /*strip*/0,
#if defined (OPENGL_API)
															gl_surface_mode,
#endif /* defined (OPENGL_API) */
															tile_surface->n_data_components,
															tile_surface->data,
															material, spectrum, spectrum_render_data);
#if defined (OPENGL_API)
														if (gl_surface_mode != GL_POINTS)
														{
															glEnd();
														}
#endif /* defined (OPENGL_API) */
														tile_surface_2 = tile_surface;
														tile_surface = tile_surface->ptrnext;
														DESTROY(GT_surface)(&tile_surface_2);
													}
												}
												else
												{
													draw_surfaceGL(surface->pointlist, surface->normallist,
														surface->tangentlist,
														surface->texturelist, surface->n_pts1,
														surface->n_pts2, surface->polygon,
														surface->n_data_components, surface->data,
														material, spectrum, spectrum_render_data);
												}
											}
											surface=surface->ptrnext;
										}
									}
									return_code=1;
								} break;
							case g_SH_DISCONTINUOUS:
							case g_SH_DISCONTINUOUS_STRIP:
							case g_SH_DISCONTINUOUS_TEXMAP:
							case g_SH_DISCONTINUOUS_STRIP_TEXMAP:
								{
#if defined (OPENGL_API)
									GLenum gl_surface_mode = GL_POINTS; // Not a valid surface primitive
#endif /* defined (OPENGL_API) */
									strip=((g_SH_DISCONTINUOUS_STRIP_TEXMAP==surface->surface_type)
										||(g_SH_DISCONTINUOUS_STRIP==surface->surface_type));
									if (proportion>0)
									{
										while (surface&&surface_2)
										{
											/* work out if subobjects selected */
											if (renderer->highlight_functor)
											{
												name_selected=(renderer->highlight_functor)->call(surface->object_name);
											}
											else
											{
												name_selected = 0;
											}
											if ((name_selected&&draw_selected)||
												((!name_selected)&&(!draw_selected)))
											{
												interpolate_surface=morph_GT_surface(proportion,
													surface,surface_2);
												if (interpolate_surface)
												{
													if (picking_names)
													{
#if defined (OPENGL_API)
														if (gl_surface_mode != GL_POINTS)
														{
															glEnd();
															gl_surface_mode = GL_POINTS;
														}
														/* put out name for picking - cast to GLuint */
														glLoadName((GLuint)interpolate_surface->object_name);
#endif /* defined (OPENGL_API) */
													}
													draw_dc_surfaceGL(interpolate_surface->pointlist,
														interpolate_surface->normallist,
														interpolate_surface->tangentlist,
														interpolate_surface->texturelist,
														interpolate_surface->n_pts1,
														interpolate_surface->n_pts2,
														interpolate_surface->polygon,strip,
#if defined (OPENGL_API)
														gl_surface_mode,
#endif /* defined (OPENGL_API) */
														interpolate_surface->n_data_components,
														interpolate_surface->data,
														material, spectrum, spectrum_render_data);
													DESTROY(GT_surface)(&interpolate_surface);
												}
											}
											surface=surface->ptrnext;
											surface_2=surface_2->ptrnext;
										}
									}
									else
									{
										while (surface)
										{
											/* work out if subobjects selected */
											if (renderer->highlight_functor)
											{
												name_selected=(renderer->highlight_functor)->call(surface->object_name);
											}
											else
											{
												name_selected = 0;
											}
											if ((name_selected&&draw_selected)||
												((!name_selected)&&(!draw_selected)))
											{
												if (picking_names)
												{
#if defined (OPENGL_API)
													if (gl_surface_mode != GL_POINTS)
													{
														glEnd();
														gl_surface_mode = GL_POINTS;
													}
													/* put out name for picking - cast to GLuint */
													glLoadName((GLuint)surface->object_name);
#endif /* defined (OPENGL_API) */
												}
												draw_dc_surfaceGL(surface->pointlist,surface->normallist,
													surface->tangentlist,

													surface->texturelist,surface->n_pts1,surface->n_pts2,
													surface->polygon,strip,
#if defined (OPENGL_API)
													gl_surface_mode,
#endif /* defined (OPENGL_API) */
													surface->n_data_components, surface->data,
													material, spectrum, spectrum_render_data);
											}
											surface=surface->ptrnext;
										}
									}
#if defined (OPENGL_API)
									if (gl_surface_mode != GL_POINTS)
									{
										glEnd();
									}
#endif /* defined (OPENGL_API) */
									return_code=1;
								} break;
							default:
								{
									display_message(ERROR_MESSAGE,
										"render_GT_object_opengl_immediate.  Invalid surface type");
									return_code=0;
								} break;
							}
#if defined (OPENGL_API)
							if (spectrum_render_data)
							{
								spectrum_end_renderGL(spectrum,spectrum_render_data);
							}
							if (wireframe_flag)
							{
								glPopAttrib();
							}
							if (picking_names)
							{
								glPopName();
							}
#endif /* defined (OPENGL_API) */
						}
						else
						{
							/*???debug*/printf("! render_GT_object_opengl_immediate.  Missing surface");
							display_message(ERROR_MESSAGE,"render_GT_object_opengl_immediate.  Missing surface");
							return_code=0;
						}
					} break;
				case g_NURBS:
					{
#if defined (OPENGL_API)
						/* store transformation attributes and GL_NORMALIZE */
						glPushAttrib(GL_TRANSFORM_BIT);
						glEnable(GL_NORMALIZE);
						if (picking_names)
						{
							glPushName(0);
						}
#endif /* defined (OPENGL_API) */
						nurbs = primitive_list1->gt_nurbs.first;
						if (nurbs)
						{
							return_code = 1;
							while(return_code && nurbs)
							{
								if (picking_names)
								{
									/* put out name for picking - cast to GLuint */
									glLoadName((GLuint)nurbs->object_name);
								}
								/* work out if subobjects selected */
								name_selected=0;
								if ((name_selected&&draw_selected)||
									((!name_selected)&&(!draw_selected)))
								{
									return_code = draw_nurbsGL(nurbs);
								}
								nurbs=nurbs->ptrnext;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,"render_GT_object_opengl_immediate.  Missing nurbs");
							return_code=0;
						}
#if defined (OPENGL_API)
						if (picking_names)
						{
							glPopName();
						}
						/* restore the transform attribute group */
						glPopAttrib();
#endif /* defined (OPENGL_API) */
					} break;
				case g_USERDEF:
					{
#if defined (OPENGL_API)
						/* save transformation attributes state */
						glPushAttrib(GL_TRANSFORM_BIT);
						glEnable(GL_NORMALIZE);
#endif /* defined (OPENGL_API) */
						userdef = primitive_list1->gt_userdef.first;
						if (userdef)
						{
							if (userdef->render_function)
							{
								(userdef->render_function)(userdef->data);
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"render_GT_object_opengl_immediate.  Missing render function user defined object");
								return_code=0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,"render_GT_object_opengl_immediate.  Missing userdef");
							return_code=0;
						}
#if defined (OPENGL_API)
						/* restore previous transformation attributes */
						glPopAttrib();
#endif /* defined (OPENGL_API) */
					} break;
				default:
					{
						display_message(ERROR_MESSAGE,"render_GT_object_opengl_immediate.  Invalid object type");
						return_code=0;
					} break;
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"render_GT_object_opengl_immediate.  Missing object");
			return_code=0;
		}
		LEAVE;

		return (return_code);
} /* render_GT_object_opengl_immediate */

static int Graphics_object_compile_members_opengl(GT_object *graphics_object_list,
	Render_graphics_opengl *renderer)
{
	int i, return_code;
	struct GT_object *graphics_object;

	ENTER(Graphics_object_compile_members);
	if (graphics_object_list)
	{
		return_code = 1;
		for (graphics_object=graphics_object_list;graphics_object != NULL;
			graphics_object=graphics_object->nextobject)
		{
			if (GRAPHICS_COMPILED != graphics_object->compile_status)
			{
				/* compile components of graphics objects first */
				if (graphics_object->default_material)
				{
					renderer->texture_tiling = NULL;
					renderer->allow_texture_tiling = 1;
					renderer->Material_compile(graphics_object->default_material);

					/* The geometry needs to be updated if these are different. */
					if (renderer->texture_tiling || graphics_object->texture_tiling)
					{
						REACCESS(Texture_tiling)(&graphics_object->texture_tiling,
							renderer->texture_tiling);
						graphics_object->compile_status = GRAPHICS_NOT_COMPILED;
					}

					renderer->allow_texture_tiling = 0;
					renderer->texture_tiling = NULL;
				}
				if (graphics_object->selected_material)
				{
					renderer->Material_compile(graphics_object->selected_material);
				}
				if (graphics_object->secondary_material)
				{
					renderer->Material_compile(graphics_object->secondary_material);
				}
				switch (graphics_object->object_type)
				{
				case g_GLYPH_SET:
					{
						struct GT_glyph_set *glyph_set;

						if (graphics_object->primitive_lists)
						{
							for (i = 0 ; i < graphics_object->number_of_times ; i++)
							{
								glyph_set = graphics_object->primitive_lists[i].gt_glyph_set.first;
								if (glyph_set)
								{
									if (glyph_set->glyph)
										renderer->Graphics_object_compile(glyph_set->glyph);
									if (glyph_set->font)
										Cmiss_graphics_font_compile(glyph_set->font, renderer->graphics_buffer);
								}
							}
						}
					} break;
				case g_POINT:
					{
						struct GT_point *point;

						if (graphics_object->primitive_lists)
						{
							for (i = 0 ; i < graphics_object->number_of_times ; i++)
							{
								point = graphics_object->primitive_lists[i].gt_point.first;
								if (point)
								{
									if (point->font)
									{
										Cmiss_graphics_font_compile(point->font, renderer->graphics_buffer);
									}
								}
							}
						}
					} break;
				case g_POINTSET:
					{
						struct GT_pointset *point_set;

						if (graphics_object->primitive_lists)
						{
							for (i = 0 ; i < graphics_object->number_of_times ; i++)
							{
								point_set = graphics_object->primitive_lists[i].gt_pointset.first;
								if (point_set)
								{
									if (point_set->font)
									{
										Cmiss_graphics_font_compile(point_set->font, renderer->graphics_buffer);
									}
								}
							}
						}
					} break;
				default:
					{
						/* Do nothing, satisfy warnings */
					} break;
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Graphics_object_compile_members.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Graphics_object_compile_members */

/***************************************************************************//**
																			 * Compile the display list for this object.
																			 */
static int Graphics_object_compile_opengl_display_list(GT_object *graphics_object_list,
	Callback_base< GT_object * > *execute_function,
	Render_graphics_opengl *renderer)
{
	int return_code;
	struct GT_object *graphics_object;

	ENTER(compile_GT_object);
	USE_PARAMETER(renderer);
	if (graphics_object_list)
	{
		return_code = 1;
		for (graphics_object=graphics_object_list;graphics_object != NULL;
			graphics_object=graphics_object->nextobject)
		{
			if (GRAPHICS_COMPILED != graphics_object->compile_status)
			{
				if (GRAPHICS_NOT_COMPILED == graphics_object->compile_status)
				{
					if (graphics_object->display_list ||
						(graphics_object->display_list=glGenLists(1)))
					{
						glNewList(graphics_object->display_list,GL_COMPILE);
						(*execute_function)(graphics_object);
						glEndList();
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Graphics_object_compile_opengl_display_list.  Unable to get display list");
						return_code = 0;
					}
				}
				if (return_code)
				{
					graphics_object->compile_status = GRAPHICS_COMPILED;
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Graphics_object_compile_opengl_display_list.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Graphics_object_compile_opengl_display_list */

/***************************************************************************//**
																			 * Compile the display list for this object.
																			 */
static int Graphics_object_execute_opengl_display_list(GT_object *graphics_object_list,
	Render_graphics_opengl *renderer)
{
	int return_code;
	struct GT_object *graphics_object;

	ENTER(compile_GT_object);
	USE_PARAMETER(renderer);
	if (graphics_object_list)
	{
		return_code = 1;
		for (graphics_object=graphics_object_list;graphics_object != NULL;
			graphics_object=graphics_object->nextobject)
		{
			if (GRAPHICS_COMPILED == graphics_object->compile_status)
			{
				glCallList(graphics_object->display_list);
			}
			else
			{
				display_message(ERROR_MESSAGE,"Graphics_object_execute_opengl_display_list.  Graphics object not compiled.");
				return_code = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Graphics_object_execute_opengl_display_list.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Graphics_object_execute_opengl_display_list */

static int Graphics_object_render_opengl(
struct GT_object *graphics_object,
	Render_graphics_opengl *renderer, Graphics_object_rendering_type rendering_type)
{
	int graphics_object_no,return_code;
	struct GT_object *graphics_object_item;

	ENTER(execute_GT_object);
	if (graphics_object)
	{
		return_code=1;
		if (renderer->picking && (NULL != graphics_object->nextobject))
		{
			glPushName(0);
		}
		graphics_object_no=0;
		for (graphics_object_item=graphics_object;graphics_object_item != NULL;
			graphics_object_item=graphics_object_item->nextobject)
		{
			if (renderer->picking && (0<graphics_object_no))
			{
				glLoadName((GLuint)graphics_object_no);
			}
			graphics_object_no++;

			if ((GRAPHICS_SELECT_ON == graphics_object_item->select_mode) ||
				(GRAPHICS_DRAW_SELECTED == graphics_object_item->select_mode))
			{
				if (graphics_object_item->selected_material)
				{
					if (renderer->highlight_functor)
					{
						renderer->Material_execute(
							graphics_object_item->selected_material);
						render_GT_object_opengl_immediate(graphics_object_item,
							/*draw_selected*/1, renderer, rendering_type);
						renderer->Material_execute((Graphical_material *)NULL);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,"compile_GT_object.  "
						"Graphics object %s has no selected material",
						graphics_object_item->name);
				}
			}
			// there is no highlight_functor when picking, but draw_selected graphics need to be pickable
			if ((GRAPHICS_DRAW_SELECTED != graphics_object_item->select_mode) || (!renderer->highlight_functor))
			{
				if (graphics_object_item->default_material)
				{
					renderer->Material_execute(graphics_object_item->default_material);
				}
				render_GT_object_opengl_immediate(graphics_object_item,
					/*draw_selected*/0, renderer, rendering_type);
				if (graphics_object_item->default_material)
				{
					renderer->Material_execute((Graphical_material *)NULL);
				}
			}
		}
		if (renderer->picking && (NULL != graphics_object->nextobject))
		{
			glPopName();
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_GT_object.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_GT_object */



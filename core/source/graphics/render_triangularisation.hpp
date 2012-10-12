/***************************************************************************//**
 * render_to_triangularisation.hpp
 * OpenGL rendering calls
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
 * Portions created by the Initial Developer are Copyright (C) 2005
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Shane Blackett <shane@blackett.co.nz>
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
#if !defined (RENDER_TRIANGULARISATION_HPP)
#define RENDER_TRIANGULARISATION_HPP

#include "graphics/graphics_object.h"
#include "graphics/graphics_coordinate_system.hpp"
#include "graphics/render.hpp"
#include "graphics/triangle_mesh.hpp"

/***************************************************************************//**
 * Common code for all OpenGL based implementations.
 */
class Render_graphics_triangularisation : public Render_graphics_compile_members
{
public:
	Graphics_buffer *graphics_buffer;	
	Triangle_mesh *trimesh;

public:
	Render_graphics_triangularisation(Graphics_buffer *graphics_buffer, GLfloat tolerance) :
		graphics_buffer(graphics_buffer), trimesh(new Triangle_mesh(tolerance))
	{
	}

	virtual ~Render_graphics_triangularisation();

	/***************************************************************************//**
	 * Execute the Graphics_object.
	 */
	virtual int Graphics_object_execute(GT_object *graphics_object)
	{
		USE_PARAMETER(graphics_object);
		return 1;
	}

	/***************************************************************************//**
	 * Compile the Graphics_object.
	 */
	virtual int Graphics_object_compile(GT_object *graphics_object)
	{
		USE_PARAMETER(graphics_object);
		return 1;
	}

	virtual int Cmiss_rendition_execute(Cmiss_rendition *rendition)
	{
		USE_PARAMETER(rendition);
		return 1;
	}

	virtual int Cmiss_rendition_execute_members(Cmiss_rendition *rendition)
	{
		USE_PARAMETER(rendition);
		return 1;
	}
	
	virtual int Cmiss_rendition_execute_child_rendition(Cmiss_rendition *rendition)
	{
		USE_PARAMETER(rendition);
		return 1;
	}

	/***************************************************************************//**
	 * Render the Graphics_object.  Typically as the graphics_object is temporary
	 * this method suggests to renderers that compile and then render that this object
	 * should instead render now.
	 */
	virtual int Graphics_object_render_immediate(GT_object *graphics_object)
	{
		USE_PARAMETER(graphics_object);
		return 1;
	}

	/***************************************************************************//**
	 * Compile the Material.
	 */
	virtual int Material_compile(Graphical_material *material)
	{
		USE_PARAMETER(material);
		return 1;
	}
	
	/***************************************************************************//**
	 * Execute the Material.
	 */
	virtual int Material_execute(Graphical_material *material)
	{
		USE_PARAMETER(material);
		return 1;
	}
	
	/***************************************************************************//**
	 * Execute the Texture.
	 */
	virtual int Texture_execute(Texture *texture)
	{
		USE_PARAMETER(texture);
		return 1;
	}
	
	/***************************************************************************//**
	 * Execute the Light.
	 */
	virtual int Light_execute(Light *light)
	{
		USE_PARAMETER(light);
		return 1;
	}
	
	/***************************************************************************//**
	 * Execute the Light.
	 */
	virtual int Light_model_execute(Light_model *light_model)
	{
		USE_PARAMETER(light_model);
		return 1;
	}

	virtual int Scene_execute(Scene *scene);

	/** Only draw graphics in world and local coordinates. Not fully implemented */
	virtual int begin_coordinate_system(enum Cmiss_graphics_coordinate_system coordinate_system)
	{
		return !Cmiss_graphics_coordinate_system_is_window_relative(coordinate_system);
	}

	/** Not fully implemented */
	virtual void end_coordinate_system(enum Cmiss_graphics_coordinate_system /*coordinate_system*/)
	{
	}

	Triangle_mesh *get_triangle_mesh();
	
}; /* class Render_graphics */

int render_scene_triangularisation(struct Scene *scene);

#endif /* !defined (RENDER_TRIANGULARISATION_HPP) */

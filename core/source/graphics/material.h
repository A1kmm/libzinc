/*******************************************************************************
FILE : material.h

LAST MODIFIED : 6 December 2004

DESCRIPTION :
The data structures used for representing graphical materials.
???RC Only OpenGL is supported now.
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
#if !defined (MATERIAL_H)
#define MATERIAL_H

#include "zinc/zincconfigure.h"
#include "zinc/field.h"
#include "zinc/fieldimage.h"
#include "zinc/graphicsmodule.h"
#include "general/list.h"
#include "general/manager.h"
#include "general/object.h"
#include "graphics/colour.h"
#include "graphics/spectrum.h"
#include "graphics/texture.h"
#include "graphics/auxiliary_graphics_types.h"

/*
Global constants
----------------
*/
#define Graphical_material Cmiss_graphics_material

/*
Global types
------------
*/
struct IO_stream;
/*******************************************************************************
LAST MODIFIED : 6 December 2004

DESCRIPTION :
Declared here to satisfy function prototype.
==============================================================================*/

enum Material_program_type
/*****************************************************************************//**
@date LAST MODIFIED : 4 July 2007

Enumerates the main different types of vertex/fragment program for materials
==============================================================================*/
{
	/* This type is for Material_programs which have arbitrary specified strings
	 * rather than the program being generated based on this type value.
	 */
	MATERIAL_PROGRAM_SPECIFIED_STRINGS = 0,
	/* This first one is a standard Gouraud Shaded material, included here so
		that it can be peeled in order independent transparency */
	MATERIAL_PROGRAM_GOURAUD_SHADING = 1,
	MATERIAL_PROGRAM_PER_PIXEL_LIGHTING = 2,
	MATERIAL_PROGRAM_PER_PIXEL_TEXTURING = 10,
	MATERIAL_PROGRAM_BUMP_MAPPING = 770,
	MATERIAL_PROGRAM_BUMP_MAPPING_TEXTURING = 778,

	/* These classes modify the above programs and so must be bit independent */
	MATERIAL_PROGRAM_CLASS_GOURAUD_SHADING = 1,
	MATERIAL_PROGRAM_CLASS_PER_PIXEL_LIGHTING = 2,
	/* Use these bits to indicate the presence of and dimension (1, 2 or 3) of a colour texture. */
	MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE_1 = (1<<2),
	MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE_2 = (1<<3),
	/* If either bit is set then we are using a texture */
	MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE = 12,
	/* If this bit is set then a colour texture will replace the lighting calculation for a colour,
		if it is not set then the texture will modulate the lighting calculation colour */
	MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE_DECAL = (1<<4),
	/* Specifies the output dimension of the texture and therefore how it is applied.
		OUTPUT1 = grayscale, OUTPUT2 = grayscale and alpha, OUTPUT1 & OUTPUT2 = rgb
		!OUPUT1 & !OUTPUT2 = rgba. */
	MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE_OUTPUT_1 = (1<<5),
	MATERIAL_PROGRAM_CLASS_COLOUR_TEXTURE_OUTPUT_2 = (1<<6),

	/* Use these bits to indicate the presence of and dimension (1, 2 or 3) of a second or bump map texture. */
	MATERIAL_PROGRAM_CLASS_SECOND_TEXTURE_1 = (1<<7),
	MATERIAL_PROGRAM_CLASS_SECOND_TEXTURE_2 = (1<<8),
	MATERIAL_PROGRAM_CLASS_SECOND_TEXTURE = 384,
   /* Specifies that the second texture is intended to be used as a bump map texture, modulating
		the per pixel value of the normal in the lighting calculation */
	MATERIAL_PROGRAM_CLASS_SECOND_TEXTURE_BUMPMAP = (1<<9),

	/* The colour value is used as the input, derived from the primary texture or the lighting.
	   Specify which input components are used directly from the input source,
		at most 3 of the input values can be used (a 3D texture lookup) */
	MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_1 = (1<<10),
	MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_2 = (1<<11),
	MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_3 = (1<<12),
	MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_4 = (1<<13),
	MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_INPUTS = (1<<10) + (1<<11) + (1<<12) + (1<<13),

	/* Specify the outputs in the dependent texture lookup, either replacing the colour, alpha or both. */
	/* These modes work by assuming that the dependent_texture inputs form
		the axes of a single 1D, 2D or 3D texture */
	MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_COLOUR = (1<<14),
	MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_ALPHA = (1<<15),
	/* This mode works by looking up each input component independently
		in a common 1D texture */
	MATERIAL_PROGRAM_CLASS_DEPENDENT_TEXTURE_1D_COMPONENT_LOOKUP = (1<<16),

	/* Assume that the texture contains an intensity followed by a 3 component
		normal vector.  This vector is used to light volume rendering by
		performing per pixel lighting using this normal. */
	MATERIAL_PROGRAM_CLASS_LIT_VOLUME_INTENSITY_NORMAL_TEXTURE = (1<<17),
	/* Calculate a normal by using a finite difference operator. */
	MATERIAL_PROGRAM_CLASS_LIT_VOLUME_FINITE_DIFFERENCE_NORMAL = (1<<18),
	/* Scale the alpha by the magnitude of the normal */
	MATERIAL_PROGRAM_CLASS_LIT_VOLUME_SCALE_ALPHA = (1<<19),

   /* Order independent transparency passes */
	MATERIAL_PROGRAM_CLASS_ORDER_INDEPENDENT_FIRST_LAYER = (1<<20),
	MATERIAL_PROGRAM_CLASS_ORDER_INDEPENDENT_PEEL_LAYER = (1<<21)
}; /* enum Material_program_type */

enum Material_program_shader_type
{
	MATERIAL_PROGRAM_SHADER_NONE,
	MATERIAL_PROGRAM_SHADER_ARB,
	MATERIAL_PROGRAM_SHADER_GLSL
};


/*****************************************************************************//**
@date LAST MODIFIED : 20 June 2008

Stores a display list which sets up the correct state for a particular
material state.  This allows vertex/fragment programs to be used per material
but shared between different materials with the same state.
==============================================================================*/
struct Material_program
{
	/*! Specifies the type of the Material Program
	 * These should be unique for each differing program as the materials
	 * will only generate one program for each value of type.
	 * As a special case, type == 0, specifies a predefined arbitrary string
	 * is used and so these should never be shared. */
	enum Material_program_type type;
#if defined (OPENGL_API)
#if defined GL_ARB_vertex_program && defined GL_ARB_fragment_program
	GLuint vertex_program;
	GLuint fragment_program;
	GLuint geometry_program;
#endif /* defined GL_ARB_vertex_program && defined GL_ARB_fragment_program */
	GLuint glsl_current_program;
	char *vertex_program_string;
	char *geometry_program_string;
	char *fragment_program_string;
	enum Material_program_shader_type shader_type;
	/*! Display list which enables the correct state for this program */
	GLuint display_list;
#endif /* defined (OPENGL_API) */

	/*! Flag indicating whether the program is compiled or not */
	int compiled;

	int access_count;
}; /* struct Material_program */

DECLARE_LIST_TYPES(Material_program);
PROTOTYPE_LIST_FUNCTIONS(Material_program);

enum Material_program_uniform_type
{
	MATERIAL_PROGRAM_UNIFORM_TYPE_UNDEFINED,
	MATERIAL_PROGRAM_UNIFORM_TYPE_FLOAT
};

/*****************************************************************************//**
Store a uniform parameter value used by a Material_program.
These values are not stored in the program as a particular material may
use the same program with different values for these parameters.
Currently only the FLOAT type is implemented.
The object currently always stores the values in a double 4 array for simplicity.
==============================================================================*/
struct Material_program_uniform
{
	char *name;
	unsigned int number_of_defined_values;
	enum Material_program_uniform_type type;
	double values[4];
	int access_count;
}; /* struct Material_program_uniform */

DECLARE_LIST_TYPES(Material_program_uniform);
PROTOTYPE_LIST_FUNCTIONS(Material_program_uniform);
PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(Material_program_uniform,name,const char *);

/***************************************************************************//**
 * A structure for storing related object for the texture of a material.
 * Each material consists four of this (multitexture support).
 */
struct Material_image_texture
{
	struct Texture *texture;
	struct MANAGER(Computed_field) *manager;
	Cmiss_field_image_id field;
	void *callback_id;
	struct Graphical_material *material;
};

struct Graphical_material
/*******************************************************************************
LAST MODIFIED : 23 January 2004

DESCRIPTION :
The properties of a material.
==============================================================================*/
{
	/* the name of the material */
	const char *name;
	/* the colour of the background light (so scattered that its incident
		direction is unknown, comes from all directions) reflected */
	struct Colour ambient;
	/* the colour of the directional light scattered in all directions */
	struct Colour diffuse;
	/* the colour of the light emitted */
	struct Colour emission;
	/* the colour of the directional light which is reflected in a preferred
		direction */
	struct Colour specular;
	/* the transparency */
	MATERIAL_PRECISION alpha;
	/* how sharp and bright the glinting is */
	MATERIAL_PRECISION shininess;
#if defined (OPENGL_API)
	GLuint display_list;

	GLuint brightness_texture_id;
#endif /* defined (OPENGL_API) */
	/* enumeration indicates whether the graphics display list is up to date */
	enum Graphics_compile_status compile_status;
	/* the texture for this material */
	struct Material_image_texture image_texture;
	/* second stage multitexture (i.e. normals for bump mapping) */
	struct Material_image_texture second_image_texture;
	/* third stage multitexture */
	struct Material_image_texture third_image_texture;
	/* fourth stage multitexture */
	struct Material_image_texture fourth_image_texture;
	/* second stage multitexture (i.e. normals for bump mapping) */
	/* spectrum used to render this material */
	struct Spectrum *spectrum;
	/* callback if the spectrum changes */
	void *spectrum_manager_callback_id;
	/* the shared information for Graphical Materials, allowing them to share
	   Material_programs */
	struct Material_package *package;
	/* The normal calculated from the volume texture needs to be
		scaled similarly to how it is scaled into coordinate space,
		we do not take account of rotations or any other distortions.
		Four components as that is what ProgramEnvParameter4fvARB wants. */
	ZnReal lit_volume_normal_scaling[4];
	/* the graphics state program that represents this material */
	struct Material_program *program;
	/* user defined uniforms used by the program */
	LIST(Material_program_uniform) *program_uniforms;
	 int access_count, per_pixel_lighting_flag, bump_mapping_flag;
	/* this flag is for external API uses. If a material is set to be persistent
		 then this material will not be removed from the manager after destroy.
	 */
	bool is_managed_flag;

	/* after clearing in create, following to be modified only by manager */
	struct MANAGER(Graphical_material) *manager;
	int manager_change_status;
}; /* struct Graphical_material */

DECLARE_LIST_TYPES(Graphical_material);

DECLARE_MANAGER_TYPES(Graphical_material);

struct Material_package
/*******************************************************************************
LAST MODIFIED : 20 November 2003

DESCRIPTION :
Provide an opaque container for shared material information.
==============================================================================*/
{
	struct MANAGER(Graphical_material) *material_manager;
	struct MANAGER(Spectrum) *spectrum_manager;
	struct Graphical_material *default_material;
	struct Graphical_material *default_selected_material;
	struct LIST(Material_program) *material_program_list;
	struct Cmiss_region *root_region;
	int access_count;
}; /* struct Material_package */

/*
Global functions
----------------
*/
struct Material_package *CREATE(Material_package)(
	struct Cmiss_region *root_region,
	struct MANAGER(Spectrum) *spectrum_manager);
/*******************************************************************************
LAST MODIFIED : 20 May 2005

DESCRIPTION :
Create a shared information container for Materials.
==============================================================================*/

int DESTROY(Material_package)(struct Material_package **material_package_address);
/*******************************************************************************
LAST MODIFIED : 20 November 2003

DESCRIPTION :
==============================================================================*/

PROTOTYPE_OBJECT_FUNCTIONS(Material_package);

int Material_package_manage_material(struct Material_package *material_package,
	struct Graphical_material *material);
/*******************************************************************************
LAST MODIFIED : 20 November 2003

DESCRIPTION :
This puts the <material> into the manager connected with the <material_package>
and allows the OpenGL states within the materials mangaed by the package to be
shared.
==============================================================================*/

struct Graphical_material *Material_package_get_default_material(
	struct Material_package *material_package);
/*******************************************************************************
LAST MODIFIED : 20 November 2003

DESCRIPTION :
Returns the default material object.
==============================================================================*/

struct Graphical_material *Material_package_get_default_selected_material(
	struct Material_package *material_package);
/*******************************************************************************
LAST MODIFIED : 20 November 2003

DESCRIPTION :
Returns the default_selected material object.
==============================================================================*/

struct MANAGER(Graphical_material) *Material_package_get_material_manager(
	struct Material_package *material_package);
/*******************************************************************************
LAST MODIFIED : 20 November 2003

DESCRIPTION :
Returns the material manager.
==============================================================================*/

struct Graphical_material *CREATE(Graphical_material)(const char *name);
/*******************************************************************************
LAST MODIFIED : 20 November 2003

DESCRIPTION :
==============================================================================*/

int DESTROY(Graphical_material)(struct Graphical_material **material_address);
/*******************************************************************************
LAST MODIFIED : 25 September 1995

DESCRIPTION :
Frees the memory for the material and sets <*material_address> to NULL.
==============================================================================*/

PROTOTYPE_OBJECT_FUNCTIONS(Graphical_material);
PROTOTYPE_GET_OBJECT_NAME_FUNCTION(Graphical_material);

PROTOTYPE_LIST_FUNCTIONS(Graphical_material);
PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(Graphical_material,name,const char *);

PROTOTYPE_MANAGER_COPY_FUNCTIONS(Graphical_material,name,const char *);
PROTOTYPE_MANAGER_FUNCTIONS(Graphical_material);
PROTOTYPE_MANAGER_IDENTIFIER_FUNCTIONS(Graphical_material,name,const char *);

const char *Graphical_material_name(struct Graphical_material *material);
/*******************************************************************************
LAST MODIFIED : 29 November 1997

DESCRIPTION :
While the GET_NAME macro returns a copy of the name of an object, this function
has been created for returning just a pointer to the material's name, or some
other string if the name is invalid, suitable for putting in printf statements.
Be careful with the returned value: esp. do not modify or DEALLOCATE it!
==============================================================================*/

int Graphical_material_get_ambient(struct Graphical_material *material,
	struct Colour *diffuse);
/*******************************************************************************
LAST MODIFIED : 28 November 1997

DESCRIPTION :
Returns the ambient colour of the material.
==============================================================================*/

int Graphical_material_set_ambient(struct Graphical_material *material,
	struct Colour *diffuse);
/*******************************************************************************
LAST MODIFIED : 28 November 1997

DESCRIPTION :
Sets the ambient colour of the material.
==============================================================================*/

int Graphical_material_get_diffuse(struct Graphical_material *material,
	struct Colour *diffuse);
/*******************************************************************************
LAST MODIFIED : 28 November 1997

DESCRIPTION :
Returns the diffuse colour of the material.
==============================================================================*/

int Graphical_material_set_diffuse(struct Graphical_material *material,
	struct Colour *diffuse);
/*******************************************************************************
LAST MODIFIED : 28 November 1997

DESCRIPTION :
Sets the diffuse colour of the material.
==============================================================================*/

int Graphical_material_get_emission(struct Graphical_material *material,
	struct Colour *emission);
/*******************************************************************************
LAST MODIFIED : 28 November 1997

DESCRIPTION :
Returns the emission colour of the material.
==============================================================================*/

int Graphical_material_set_emission(struct Graphical_material *material,
	struct Colour *emission);
/*******************************************************************************
LAST MODIFIED : 28 November 1997

DESCRIPTION :
Sets the emission colour of the material.
==============================================================================*/

int Graphical_material_get_specular(struct Graphical_material *material,
	struct Colour *emission);
/*******************************************************************************
LAST MODIFIED : 28 November 1997

DESCRIPTION :
Returns the specular colour of the material.
==============================================================================*/

int Graphical_material_set_specular(struct Graphical_material *material,
	struct Colour *emission);
/*******************************************************************************
LAST MODIFIED : 28 November 1997

DESCRIPTION :
Sets the specular colour of the material.
==============================================================================*/

int Graphical_material_get_alpha(struct Graphical_material *material,
	MATERIAL_PRECISION *alpha);
/*******************************************************************************
LAST MODIFIED : 28 November 1997

DESCRIPTION :
Returns the alpha value of the material.
==============================================================================*/

int Graphical_material_set_alpha(struct Graphical_material *material,
	MATERIAL_PRECISION alpha);
/*******************************************************************************
LAST MODIFIED : 28 November 1997

DESCRIPTION :
Sets the alpha value of the material.
==============================================================================*/

int Graphical_material_get_shininess(struct Graphical_material *material,
	MATERIAL_PRECISION *shininess);
/*******************************************************************************
LAST MODIFIED : 28 November 1997

DESCRIPTION :
Returns the shininess value of the material.
==============================================================================*/

int Graphical_material_set_shininess(struct Graphical_material *material,
	MATERIAL_PRECISION shininess);
/*******************************************************************************
LAST MODIFIED : 28 November 1997

DESCRIPTION :
Sets the shininess value of the material.
==============================================================================*/

int Graphical_material_set_colour_lookup_spectrum(struct Graphical_material *material,
	struct Spectrum *spectrum);
/*******************************************************************************
LAST MODIFIED : 6 October 2006

DESCRIPTION :
Sets the spectrum member of the material.
==============================================================================*/

struct Spectrum *Graphical_material_get_colour_lookup_spectrum(
	struct Graphical_material *material);
/*******************************************************************************
LAST MODIFIED : 6 October 2006

DESCRIPTION :
Returns the spectrum member of the material.
==============================================================================*/

struct Texture *Graphical_material_get_texture(
	struct Graphical_material *material);
/*******************************************************************************
LAST MODIFIED : 12 February 1998

DESCRIPTION :
Returns the texture member of the material.
==============================================================================*/

struct Texture *Graphical_material_get_second_texture(
	 struct Graphical_material *material);
/*******************************************************************************
LAST MODIFIED : 5 Dec 2007

DESCRIPTION :
Returns the second texture of the material.
==============================================================================*/

struct Texture *Graphical_material_get_third_texture(
	 struct Graphical_material *material);
/*******************************************************************************
LAST MODIFIED : 5 Dec 2007

DESCRIPTION :
Returns the third texture of the material.
==============================================================================*/


struct Texture *Graphical_material_get_fourth_texture(
	 struct Graphical_material *material);
/*******************************************************************************
LAST MODIFIED : 5 Dec 2007

DESCRIPTION :
Returns the fourth texture of the material.
==============================================================================*/


int Graphical_material_get_bump_mapping_flag(struct Graphical_material *material);
/*******************************************************************************
LAST MODIFIED : 5 December 2007

DESCRIPTION :
Returns the flag set for bump_mapping.
==============================================================================*/

int Graphical_material_get_per_pixel_lighting_flag(struct Graphical_material *material);
/*******************************************************************************
LAST MODIFIED : 5 December 2007

DESCRIPTION :
Returns the flag set for per_pixel_lighting.
==============================================================================*/

int list_Graphical_material(struct Graphical_material *material,void *dummy);
/*******************************************************************************
LAST MODIFIED : 22 September 1998

DESCRIPTION :
Writes the properties of the <material> to the command window.
==============================================================================*/

int list_Graphical_material_commands(struct Graphical_material *material,
	void *command_prefix_void);
/*******************************************************************************
LAST MODIFIED : 22 September 1998

DESCRIPTION :
Writes on the command window the commands needed to recreate the <material>.
The command is started with the string pointed to by <command_prefix>.
==============================================================================*/

int write_Graphical_material_commands_to_comfile(struct Graphical_material *material,
	 void *command_prefix_void);
/*******************************************************************************
LAST MODIFIED : 10 August 2007

DESCRIPTION :
Writes on the command window the command needed to recreate the <material>.
The command is started with the string pointed to by <command_prefix>.
==============================================================================*/

int file_read_Graphical_material_name(struct IO_stream *file,
	struct Graphical_material **material_address,
	struct MANAGER(Graphical_material) *graphical_material_manager);
/*******************************************************************************
LAST MODIFIED : 6 December 2004

DESCRIPTION :
Reads a material name from a <file>.  Searchs the list of all materials for one
with the specified name.  If one is not found a new one is created with the
specified name and the default properties.
==============================================================================*/

int set_material_program_type(struct Graphical_material *material_to_be_modified,
	 int bump_mapping_flag, int colour_lookup_red_flag, int colour_lookup_green_flag,
	 int colour_lookup_blue_flag,  int colour_lookup_alpha_flag,
	 int lit_volume_intensity_normal_texture_flag, int lit_volume_finite_difference_normal_flag,
	 int lit_volume_scale_alpha_flag, int return_code);
/******************************************************************************
LAST MODIFIED : 4 Dec 2007

DESCRIPTION : Set up the material program type for using the vertex
and fragment program. This and following functions are orginally
from the modify_graphical_material.
NOTE: I use the pointer to the material_package from the material.
==============================================================================*/

int material_copy_bump_mapping_and_per_pixel_lighting_flag(struct Graphical_material *material,
	 struct Graphical_material *material_to_be_modified);
/******************************************************************************
LAST MODIFIED : 5 Dec 2007

DESCRIPTION : This function will set the bump mapping and per
pixel_lighting_flag of the material_to_be_modified to be the same as
the one in material, it is used for setting up the GUI.
==============================================================================*/

int compile_Graphical_material_for_order_independent_transparency(struct Graphical_material *material,
	void *material_order_independent_data_void);
/*******************************************************************************
LAST MODIFIED : 2 May 2005

DESCRIPTION :
Recompile each of the <materials> which have already been compiled so that they
will work with order_independent_transparency.
==============================================================================*/

/***************************************************************************//**
 * Set a value to the uniform qualified variable used in an arbitrary shader.
 *
 * @param material  Graphical_material with the arbitrary shaders program.
 * @param uniform_name  Name of the uniform_qualifier variable.
 * @param value  Value to be set to the uniform varaible.
 * @return 1 on success, 0 on failure
 */
int Material_set_program_uniform_qualifier_variable_value(
	struct Graphical_material* material, const char *uniform_name, ZnReal value);

/***************************************************************************//**
 * Sets the Graphics_module object which will own this manager.
 * Private! Only to be called only from Graphics_module object.
 *
 * @param manager  Material manager.
 * @return  The owning Graphics_module object.
 */
int Material_manager_set_owner(struct MANAGER(Graphical_material) *manager,
	struct Cmiss_graphics_module *graphics_module);

int material_deaccess_material_program(struct Graphical_material *material_to_be_modified);

int Material_set_material_program_strings(struct Graphical_material *material_to_be_modified,
	char *vertex_program_string, char *fragment_program_string, char *geometry_program_string);

struct Material_program_uniform *CREATE(Material_program_uniform)(char *name);

struct Cmiss_graphics_module *manager_get_owner_Cmiss_graphics_material(manager_Cmiss_graphics_material *manager);

//struct Material_program_uniform *list_find_by_identifier_Material_program_uniformname(const char *name, list_Material_program_uniform *list);
#endif

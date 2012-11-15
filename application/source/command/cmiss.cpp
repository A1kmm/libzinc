/***************************************************************************//**
 * cmiss.cpp
 *
 * Functions for executing cmiss commands.
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

#if 1
#include "configure/cmgui_configure.h"
#endif /* defined (BUILD_WITH_CMAKE) */

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#if defined (WIN32_SYSTEM)
#  include <direct.h>
#else /* !defined (WIN32_SYSTEM) */
#  include <unistd.h>
#endif /* !defined (WIN32_SYSTEM) */
#include <math.h>
#include <time.h>
#include "zinc/context.h"
#include "zinc/element.h"
#include "zinc/fieldmodule.h"
#include "zinc/fieldsubobjectgroup.h"
#include "zinc/graphicsmodule.h"
#include "zinc/region.h"
#include "zinc/rendition.h"
#include "zinc/scene.h"
#include "zinc/sceneviewer.h"
#include "zinc/stream.h"
#include "comfile/comfile.h"
#if defined (WX_USER_INTERFACE)
#include "comfile/comfile_window_wx.h"
#endif /* defined (WX_USER_INTERFACE) */
#include "command/console.h"
#include "command/command_window.h"
#include "command/example_path.h"
#include "command/parser.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_alias.h"
#include "computed_field/computed_field_arithmetic_operators.h"
#include "computed_field/computed_field_compose.h"
#include "computed_field/computed_field_composite.h"
#include "computed_field/computed_field_conditional.h"
#include "computed_field/computed_field_coordinate.h"
#include "computed_field/computed_field_curve.h"
#include "computed_field/computed_field_deformation.h"
#include "computed_field/computed_field_derivatives.h"
#include "computed_field/computed_field_find_xi.h"
#include "computed_field/computed_field_find_xi_graphics.h"
#include "computed_field/computed_field_finite_element.h"
#include "computed_field/computed_field_fibres.h"
#include "computed_field/computed_field_format_output.h"
#include "computed_field/computed_field_function.h"
#include "computed_field/computed_field_group.h"
#include "computed_field/computed_field_image.h"
#include "computed_field/computed_field_integration.h"
#include "computed_field/computed_field_logical_operators.h"
#include "computed_field/computed_field_lookup.h"
#include "computed_field/computed_field_matrix_operators.hpp"
#include "computed_field/computed_field_nodeset_operators.hpp"
#include "computed_field/computed_field_subobject_group_internal.hpp"
#include "computed_field/computed_field_vector_operators.hpp"
#include "computed_field/computed_field_set.h"
#include "computed_field/computed_field_string_constant.h"
#include "computed_field/computed_field_time.h"
#include "computed_field/computed_field_trigonometry.h"
#include "computed_field/computed_field_update.h"
#include "computed_field/computed_field_scene_viewer_projection.h"
#include "computed_field/computed_field_wrappers.h"
#include "context/context.h"
#include "element/element_operations.h"
#include "element/element_point_tool.h"
#if defined (WX_USER_INTERFACE)
#include "element/element_point_viewer_wx.h"
#endif /* defined (WX_USER_INTERFACE) */
#include "element/element_tool.h"
#include "emoter/emoter_dialog.h"
#include "field_io/read_fieldml.h"
#include "finite_element/export_cm_files.h"
#if defined (USE_NETGEN)
#include "finite_element/generate_mesh_netgen.h"
#endif /* defined (USE_NETGEN) */
#include "finite_element/export_finite_element.h"
#include "finite_element/finite_element.h"
#include "finite_element/finite_element_conversion.h"
#include "finite_element/finite_element_region.h"
#include "finite_element/finite_element_to_graphics_object.h"
#include "finite_element/finite_element_to_iges.h"
#include "finite_element/finite_element_to_iso_lines.h"
#include "finite_element/finite_element_to_streamlines.h"
#include "finite_element/import_finite_element.h"
#include "finite_element/read_fieldml_01.h"
#include "finite_element/snake.h"
#include "finite_element/write_fieldml_01.h"
#include "general/debug.h"
#include "general/error_handler.h"
#include "general/image_utilities.h"
#include "general/io_stream.h"
#include "general/matrix_vector.h"
#include "general/multi_range.h"
#include "general/mystring.h"
#include "graphics/auxiliary_graphics_types.h"
#include "graphics/defined_graphics_objects.h"
#include "graphics/environment_map.h"
#include "graphics/glyph.h"
#include "graphics/graphics_object.h"
#include "graphics/graphics_window.h"
#include "graphics/iso_field_calculation.h"
#include "graphics/light.h"
#include "graphics/light_model.h"
#include "graphics/material.h"
#include "graphics/graphic.h"
#include "graphics/graphics_module.h"
#include "graphics/rendition.h"
#include "graphics/render_to_finite_elements.h"
#include "graphics/render_stl.h"
#include "graphics/render_vrml.h"
#include "graphics/render_wavefront.h"
#include "graphics/scene.h"
#include "finite_element/finite_element_helper.h"
#include "graphics/triangle_mesh.hpp"
#include "graphics/render_triangularisation.hpp"
#include "graphics/import_graphics_object.h"
#include "graphics/scene.hpp"
#include "graphics/graphics_filter.hpp"
#include "graphics/tessellation.hpp"
#if defined (WX_USER_INTERFACE)
#include "graphics/region_tree_viewer_wx.h"
#endif /* switch(USER_INTERFACE)*/
#include "graphics/spectrum.h"
#if defined (WX_USER_INTERFACE)
#include "graphics/spectrum_editor_wx.h"
#include "graphics/spectrum_editor_dialog_wx.h"
#endif /* defined (WX_USER_INTERFACE) */
#include "graphics/spectrum_settings.h"
#include "graphics/texture.h"
#include "graphics/transform_tool.h"
#include "graphics/userdef_objects.h"
#include "graphics/volume_texture.h"
#if defined (GTK_USER_INTERFACE)
#include "gtk/gtk_cmiss_scene_viewer.h"
#endif /* defined (GTK_USER_INTERFACE) */
#include "image_processing/computed_field_image_resample.h"
#if defined (USE_ITK)
#include "image_processing/computed_field_threshold_image_filter.h"
#include "image_processing/computed_field_binary_threshold_image_filter.h"
#include "image_processing/computed_field_canny_edge_detection_filter.h"
#include "image_processing/computed_field_mean_image_filter.h"
#include "image_processing/computed_field_sigmoid_image_filter.h"
#include "image_processing/computed_field_discrete_gaussian_image_filter.h"
#include "image_processing/computed_field_curvature_anisotropic_diffusion_image_filter.h"
#include "image_processing/computed_field_derivative_image_filter.h"
#include "image_processing/computed_field_rescale_intensity_image_filter.h"
#include "image_processing/computed_field_connected_threshold_image_filter.h"
#include "image_processing/computed_field_gradient_magnitude_recursive_gaussian_image_filter.h"
#include "image_processing/computed_field_histogram_image_filter.h"
#include "image_processing/computed_field_fast_marching_image_filter.h"
#include "image_processing/computed_field_binary_dilate_image_filter.h"
#include "image_processing/computed_field_binary_erode_image_filter.h"
#endif /* defined (USE_ITK) */
#if defined (SELECT_DESCRIPTORS)
#include "io_devices/io_device.h"
#endif /* !defined (SELECT_DESCRIPTORS) */
#if defined (WX_USER_INTERFACE)
#include "material/material_editor_wx.h"
#endif /* defined (SWITCH_USER_INTERFACE) */
#include "minimise/minimise.h"
#include "node/node_operations.h"
#include "node/node_tool.h"
#if defined (WX_USER_INTERFACE)
#include "node/node_viewer_wx.h"
#endif /* defined (WX_USER_INTERFACE) */
#include "region/cmiss_region.h"
#include "region/cmiss_region_app.h"
#include "selection/any_object_selection.h"
#include "three_d_drawing/graphics_buffer.h"
#include "graphics/font.h"
#include "time/time_keeper.h"
#include "user_interface/filedir.h"
#include "user_interface/confirmation.h"
#include "general/message.h"
#include "user_interface/user_interface.h"
#include "curve/curve.h"
#if defined (USE_PERL_INTERPRETER)
#include "perl_interpreter.h"
#endif /* defined (USE_PERL_INTERPRETER) */
#include "user_interface/fd_io.h"
#include "user_interface/idle.h"
#include "command/cmiss.h"
#include "mesh/cmiss_element_private.hpp"
#include "mesh/cmiss_node_private.hpp"
#if defined (USE_OPENCASCADE)
#include "cad/graphicimporter.h"
#include "cad/point.h"
#include "cad/curve.h"
#include "cad/surface.h"
#include "cad/geometricshape.h"
#include "graphics/graphics_object.hpp"
#include "cad/opencascadeimporter.h"
#include "cad/cad_tool.h"
#include "cad/computed_field_cad_topology.h"
#endif /* defined (USE_OPENCASCADE) */

// insert app headers here
#include "computed_field/computed_field_image_app.h"
#include "computed_field/computed_field_integration_app.h"
#include "computed_field/computed_field_alias_app.h"
#include "computed_field/computed_field_coordinate_app.h"
#include "graphics/rendition_app.h"
#include "image_processing/computed_field_sigmoid_image_filter_app.h"
#include "image_processing/computed_field_mean_image_filter_app.h"
#include "image_processing/computed_field_rescale_intensity_image_filter_app.h"
#include "image_processing/computed_field_derivative_image_filter_app.h"
#include "image_processing/computed_field_canny_edge_detection_filter_app.h"
#include "image_processing/computed_field_curvature_anisotropic_diffusion_image_filter_app.h"
#include "image_processing/computed_field_histogram_image_filter_app.h"
#include "image_processing/computed_field_discrete_gaussian_image_filter_app.h"
#include "image_processing/computed_field_connected_threshold_image_filter_app.h"
#include "image_processing/computed_field_gradient_magnitude_recursive_gaussian_image_filter_app.h"
#include "image_processing/computed_field_fast_marching_image_filter_app.h"
#include "image_processing/computed_field_binary_erode_image_filter_app.h"
#include "image_processing/computed_field_binary_dilate_image_filter_app.h"
#include "computed_field/computed_field_time_app.h"
#include "image_processing/computed_field_binary_threshold_image_filter_app.h"
#include "image_processing/computed_field_threshold_image_filter_app.h"
#include "image_processing/computed_field_image_resample_app.h"
#include "computed_field/computed_field_string_constant_app.h"
#include "computed_field/computed_field_deformation_app.h"
#include "computed_field/computed_field_finite_element_app.h"
#include "computed_field/computed_field_vector_operators_app.hpp"
#include "computed_field/computed_field_matrix_operators_app.hpp"
#include "computed_field/computed_field_nodeset_operators_app.hpp"
#include "computed_field/computed_field_lookup_app.h"
#include "computed_field/computed_field_logical_operators_app.h"
#include "computed_field/computed_field_function_app.h"
#include "computed_field/computed_field_fibres_app.h"
#include "computed_field/computed_field_derivatives_app.h"
#include "computed_field/computed_field_curve_app.h"
#include "computed_field/computed_field_conditional_app.h"
#include "computed_field/computed_field_composite_app.h"
#include "computed_field/computed_field_compose_app.h"
#include "computed_field/computed_field_format_output_app.h"
#include "computed_field/computed_field_trigonometry_app.h"
#include "computed_field/computed_field_arithmetic_operators_app.h"
#include "minimise/minimise_app.h"
#include "finite_element/export_finite_element_app.h"
#include "graphics/element_point_ranges_app.h"
#include "graphics/environment_map_app.h"
#include "graphics/rendition_app.h"
#include "finite_element/finite_element_region_app.h"
#include "graphics/scene_viewer_app.h"
#include "graphics/graphics_font_app.h"
#include "graphics/graphics_object_app.h"
#include "graphics/tessellation_app.hpp"
#include "graphics/tessellation_app.hpp"
#include "graphics/graphics_filter_app.hpp"
#include "computed_field/computed_field_app.h"
#include "curve/curve_app.h"
#include "general/enumerator_app.h"
#include "graphics/render_to_finite_elements_app.h"
#include "graphics/auxiliary_graphics_types_app.h"
#include "finite_element/finite_element_conversion_app.h"
#include "graphics/texture_app.h"
#include "graphics/colour_app.h"
#include "graphics/scene_app.h"
#include "graphics/spectrum_settings_app.h"
#include "graphics/light_model_app.h"
#include "graphics/light_app.h"
#include "graphics/material_app.h"
#include "graphics/spectrum_app.h"
#include "general/multi_range_app.h"
#include "computed_field/computed_field_set_app.h"
#include "context/context_app.h"
#include "three_d_drawing/graphics_buffer_app.h"
/*
Module types
------------
*/


struct Cmiss_command_data
/*******************************************************************************
LAST MODIFIED : 12 August 2002

DESCRIPTION :
==============================================================================*/
{
	int access_count;
	char *cm_examples_directory,*cm_parameters_file_name,*example_directory,
		*examples_directory,*example_comfile,
		*example_requirements,*help_directory,*help_url;
	struct Console *command_console;
#if defined (USE_CMGUI_COMMAND_WINDOW)
	struct Command_window *command_window;
#endif /* USE_CMGUI_COMMAND_WINDOW */
	struct Colour background_colour,foreground_colour;
	struct Execute_command *execute_command,*set_command;
	struct Element_point_tool *element_point_tool;
	struct Element_tool *element_tool;
#if defined (USE_OPENCASCADE)
	struct Cad_tool *cad_tool;
#endif /* defined (USE_OPENCASCADE) */
	struct Event_dispatcher *event_dispatcher;
	struct Node_tool *data_tool,*node_tool;
	struct Interactive_tool *transform_tool;
#if defined (USE_PERL_INTERPRETER)
	struct Interpreter *interpreter;
#endif /* defined (USE_PERL_INTERPRETER) */
#if defined (SELECT_DESCRIPTORS)
	struct LIST(Io_device) *device_list;
#endif /* defined (SELECT_DESCRIPTORS) */
	/* list of glyphs = simple graphics objects with only geometry */
	struct MANAGER(GT_object) *glyph_manager;
#if defined (WX_USER_INTERFACE)
	struct MANAGER(Comfile_window) *comfile_window_manager;
#endif /* defined (WX_USER_INTERFACE)*/
	struct Cmiss_region *root_region;
	struct Computed_field_package *computed_field_package;
	struct MANAGER(Environment_map) *environment_map_manager;
	struct MANAGER(FE_basis) *basis_manager;
	struct LIST(FE_element_shape) *element_shape_list;
	/* Always want the entry for graphics_buffer_package even if it will
		not be available on this implementation */
	struct Graphics_buffer_app_package *graphics_buffer_package;
	struct Cmiss_scene_viewer_app_package *scene_viewer_package;
#if defined (USE_CMGUI_GRAPHICS_WINDOW)
	struct MANAGER(Graphics_window) *graphics_window_manager;
#endif /* defined (USE_CMGUI_GRAPHICS_WINDOW) */
	struct MANAGER(Interactive_tool) *interactive_tool_manager;
	struct IO_stream_package *io_stream_package;
	struct MANAGER(Light) *light_manager;
	struct Light *default_light;
	struct MANAGER(Light_model) *light_model_manager;
	struct Light_model *default_light_model;
	struct Material_package *material_package;
	struct Cmiss_graphics_font *default_font;
	struct MANAGER(Curve) *curve_manager;
	struct MANAGER(Scene) *scene_manager;
	struct Scene *default_scene;
	struct MANAGER(Spectrum) *spectrum_manager;
	struct MANAGER(VT_volume_texture) *volume_texture_manager;
	/* global list of selected objects */
	struct Any_object_selection *any_object_selection;
	struct Element_point_ranges_selection *element_point_ranges_selection;
	struct Spectrum *default_spectrum;
	struct Streampoint *streampoint_list;
	struct Time_keeper *default_time_keeper;
	struct User_interface *user_interface;
	struct Emoter_dialog *emoter_slider_dialog;
#if defined (WX_USER_INTERFACE)
	struct Node_viewer *data_viewer,*node_viewer;
	struct Element_point_viewer *element_point_viewer;
#endif /* defined (WX_USER_INTERFACE) */
#if defined (WX_USER_INTERFACE)
	struct Material_editor *material_editor;
	struct Region_tree_viewer *region_tree_viewer;
	struct Spectrum_editor_dialog *spectrum_editor_dialog;
#endif /* defined (WX_USER_INTERFACE) */
	struct Cmiss_graphics_module *graphics_module;
}; /* struct Cmiss_command_data */

typedef struct
/*******************************************************************************
LAST MODIFIED : 12 December 1996+

DESCRIPTION :
==============================================================================*/
{
	char *examples_directory,*help_directory,*help_url,*startup_comfile;
} User_settings;

/*
Module functions
----------------
*/

#if defined (WX_USER_INTERFACE)
static int Graphics_window_update_Interactive_tool(struct Graphics_window *graphics_window,
	void *interactive_tool_void)
/*******************************************************************************
LAST MODIFIED : 27 March 2007

DESCRIPTION :
WX_USER_INTERFACE_ONLY, get the interactive_tool_manager and pass it
to change the interactive tool settings.
==============================================================================*/
{
	char *tool_name;
	struct MANAGER(Interactive_tool) *interactive_tool_manager;
	struct Interactive_tool *global_interactive_tool;
	struct Interactive_tool *wx_interactive_tool;
	global_interactive_tool = (struct Interactive_tool *)interactive_tool_void;
	GET_NAME(Interactive_tool)(global_interactive_tool,&tool_name);
	interactive_tool_manager = Graphics_window_get_interactive_tool_manager(graphics_window);
	if (NULL != (wx_interactive_tool=
		FIND_BY_IDENTIFIER_IN_MANAGER(Interactive_tool,name)(
		(char *)tool_name,interactive_tool_manager)))
	{
		Interactive_tool_copy(wx_interactive_tool,
			global_interactive_tool, (struct MANAGER(Interactive_tool) *)NULL);
	}
	DEALLOCATE(tool_name);
	return 1;
}
#endif /*(WX_USER_INTERFACE)*/

#if defined (WX_USER_INTERFACE) && defined (WIN32_SYSTEM)
char *CMISS_set_directory_and_filename_WIN32(char *file_name,
	struct Cmiss_command_data *command_data)
/*******************************************************************************
LAST MODIFIED : 27 March 2007

DESCRIPTION :
WX_USER_INTERFACE_ONLY, get the interactive_tool_manager and pass it
to change the interactive tool settings.
==============================================================================*/
{
	 char *drive_name = NULL;
	 char *first = NULL;
	 char *last = NULL;
	 char *temp_directory_name,*directory_name, *temp_name, *temp_string;
	 int lastlength, length;
	 first = strchr(file_name, '\\');
	 last = strrchr(file_name, '\\');
	 lastlength = last - file_name +1;
	 length = first - file_name +1;
	 if ((length>0))
	 {
			if (ALLOCATE(drive_name,char,length))
			{
				 strncpy(drive_name,file_name,length);
				 drive_name[length-1]='\0';
				 if (ALLOCATE(temp_string,char,length+8))
				 {
						strcpy(temp_string, "set dir ");
						strcat(temp_string, drive_name);
						temp_string[length+7]='\0';
						Execute_command_execute_string(command_data->execute_command,temp_string);
						DEALLOCATE(temp_string);
				 }
				 DEALLOCATE(drive_name);
			}
	 }
	 if (lastlength>length)
	 {
			if (ALLOCATE(temp_directory_name,char,lastlength+1))
			{
				 strncpy(temp_directory_name,file_name,lastlength);
				 temp_directory_name[lastlength]='\0';
				 if (ALLOCATE(directory_name,char,lastlength-length+2))
				 {
						directory_name = &temp_directory_name[length-1];
						directory_name[lastlength-length+1]='\0';
						if (ALLOCATE(temp_string,char,lastlength-length+10))
						{
							 strcpy(temp_string, "set dir ");
							 strcat(temp_string, directory_name);
							 temp_string[lastlength-length+9]='\0';
							 Execute_command_execute_string(command_data->execute_command,temp_string);
							 DEALLOCATE(temp_string);
						}
						DEALLOCATE(directory_name);
				 }
				 DEALLOCATE(temp_directory_name);
			}
	 }
	 if (lastlength>0)
	 {
			temp_name = &file_name[lastlength];
	 }
	 else
	 {
			temp_name = file_name;
	 }
	 return (temp_name);
}

#endif /*(WX_USER_INTERFACE)*/

static int set_command_prompt(const char *prompt, struct Cmiss_command_data *command_data)
/*******************************************************************************
LAST MODIFIED : 26 June 2002

DESCRIPTION :
Changes the command prompt provided to the user.
==============================================================================*/
{
	int return_code = 0;

	ENTER(set_command_prompt);
	if (prompt && command_data)
	{
#if defined (WIN32_USER_INTERFACE) || defined (GTK_USER_INTERFACE) || defined (WX_USER_INTERFACE)
		if (command_data->command_window)
		{
			return_code = Command_window_set_command_prompt(command_data->command_window,
				prompt);
		}
#endif /* defined (WIN32_USER_INTERFACE) || defined (GTK_USER_INTERFACE) */
		if (command_data->command_console)
		{
			return_code = Console_set_command_prompt(command_data->command_console,
				prompt);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_command_prompt.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* set_command_prompt */

static int gfx_change_identifier(struct Parse_state *state,
	void *dummy_to_be_modified, void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 17 January 2003

DESCRIPTION :
==============================================================================*/
{
	char data_flag, element_flag, face_flag, line_flag, node_flag, *region_path, *sort_by_field_name;
	FE_value time;
	int data_offset, element_offset, face_offset, line_offset, node_offset,
		return_code;
	struct Cmiss_command_data *command_data;
	struct Cmiss_region *region;
	struct Computed_field *sort_by_field;
	struct FE_region *data_fe_region, *fe_region;
	struct Option_table *option_table;

	ENTER(gfx_change_identifier);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (command_data = (struct Cmiss_command_data *)command_data_void))
	{
		region_path = Cmiss_region_get_root_region_path();
		data_flag = 0;
		data_offset = 0;
		element_flag = 0;
		element_offset = 0;
		face_flag = 0;
		face_offset = 0;
		line_flag = 0;
		line_offset = 0;
		node_flag = 0;
		node_offset = 0;
		sort_by_field_name = NULL;
		sort_by_field = (struct Computed_field *)NULL;
		if (command_data->default_time_keeper)
		{
			time = Time_keeper_get_time(command_data->default_time_keeper);
		}
		else
		{
			time = 0;
		}

		option_table = CREATE(Option_table)();
		/* data_offset */
		Option_table_add_entry(option_table, "data_offset", &data_offset,
			&data_flag, set_int_and_char_flag);
		/* element_offset */
		Option_table_add_entry(option_table, "element_offset", &element_offset,
			&element_flag, set_int_and_char_flag);
		/* face_offset */
		Option_table_add_entry(option_table, "face_offset", &face_offset,
			&face_flag, set_int_and_char_flag);
		/* group */
		Option_table_add_entry(option_table, "group", &region_path,
			command_data->root_region, set_Cmiss_region_path);
		/* line_offset */
		Option_table_add_entry(option_table, "line_offset", &line_offset,
			&line_flag, set_int_and_char_flag);
		/* node_offset */
		Option_table_add_entry(option_table, "node_offset", &node_offset,
			&node_flag, set_int_and_char_flag);
		/* sort_by */
		Option_table_add_string_entry(option_table, "sort_by", &sort_by_field_name,
			" FIELD_NAME");
		/* time */
		Option_table_add_entry(option_table, "time", &time, NULL, set_FE_value);

		if (0 != (return_code = Option_table_multi_parse(option_table, state)))
		{
			if (Cmiss_region_get_region_from_path_deprecated(command_data->root_region,
				region_path, &region) &&
				(fe_region = Cmiss_region_get_FE_region(region)))
			{
				if (sort_by_field_name)
				{
					Cmiss_field_module_id field_module = Cmiss_region_get_field_module(region);
					sort_by_field = Cmiss_field_module_find_field_by_name(field_module,
						sort_by_field_name);
					if (sort_by_field)
					{
						if (!Computed_field_has_numerical_components(sort_by_field, NULL))
						{
							Cmiss_field_destroy(&sort_by_field);
							return_code = 0;
							display_message(ERROR_MESSAGE,
								"gfx_change_identifier.  Sort by field does not have numerical components");
						}
					}
					else
					{
						return_code = 0;
						display_message(ERROR_MESSAGE,
							"gfx_change_identifier.  Sort by field cannot be found");
					}
					Cmiss_field_module_destroy(&field_module);
				}
				if (return_code)
				{
					FE_region_begin_change(fe_region);
					int highest_dimension = FE_region_get_highest_dimension(fe_region);
					if (element_flag)
					{
						if (highest_dimension > 0)
						{
							if (!FE_region_change_element_identifiers(fe_region,
								highest_dimension, element_offset, sort_by_field, time))
							{
								return_code = 0;
							}
						}
						else
						{
							display_message(WARNING_MESSAGE,
								"gfx change identifier:  No elements found in region %s",region_path);
						}
					}
					if (face_flag && (highest_dimension > 2))
					{
						if (!FE_region_change_element_identifiers(fe_region,
							/*dimension*/2,	face_offset, sort_by_field, time))
						{
							return_code = 0;
						}
					}
					if (line_flag && (highest_dimension > 1))
					{
						if (!FE_region_change_element_identifiers(fe_region,
							/*dimension*/1, line_offset, sort_by_field, time))
						{
							return_code = 0;
						}
					}
					if (node_flag)
					{
						if (!FE_region_change_node_identifiers(fe_region,
							node_offset, sort_by_field, time))
						{
							return_code = 0;
						}
					}
					if (data_flag)
					{
						if (NULL != (data_fe_region=FE_region_get_data_FE_region(fe_region)))
						{
							FE_region_begin_change(data_fe_region);
							if (!FE_region_change_node_identifiers(data_fe_region,
								data_offset, sort_by_field, time))
							{
								return_code = 0;
							}
							FE_region_end_change(data_fe_region);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"gfx_change_identifier.  Invalid data region");
							return_code = 0;
						}
					}
					FE_region_end_change(fe_region);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_change_identifier.  Invalid region");
				return_code = 0;
			}
		}
		DESTROY(Option_table)(&option_table);
		DEALLOCATE(region_path);
		if (sort_by_field)
		{
			DEACCESS(Computed_field)(&sort_by_field);
		}
		if (sort_by_field_name)
		{
			DEALLOCATE(sort_by_field_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_change_identifier.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_change_identifier */

/***************************************************************************//**
 * Dummy modifier function explaining migration from old gfx create axes
 * command.
 */
static int gfx_create_axes(struct Parse_state *state,
	void *dummy_to_be_modified, void *dummy_user_data_void)
{
	USE_PARAMETER(state);
	USE_PARAMETER(dummy_to_be_modified);
	USE_PARAMETER(dummy_user_data_void);
	display_message(WARNING_MESSAGE,
		"The 'gfx create axes' command has been removed. These are now drawn as\n"
		"'point' graphics in the scene editor using a predefined axes glyph. Command\n"
		"'gfx draw NAME' still works if the glyph name matches the axes object, but\n"
		"offset, scale and material must be re-set in the scene editor. Enter\n"
		"'gfx list g_element REGION_PATH commands' to list the commands needed to\n"
		"reproduce what is set in the scene editor. \n");
	return 1;
}

static int gfx_create_colour_bar(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 29 March 2001

DESCRIPTION :
Executes a GFX CREATE COLOUR_BAR command. Creates a colour bar graphics object
with tick marks and labels for showing the scale of a spectrum.
==============================================================================*/
{
	char *font_name, *graphics_object_name,*number_format;
	float bar_length,bar_radius,extend_length,tick_length;
	int number_of_components,return_code,tick_divisions;
	struct Cmiss_command_data *command_data;
	struct Graphical_material *label_material,*material;
	struct Cmiss_graphics_font *font;
	struct GT_object *graphics_object = NULL;
	struct Option_table *option_table;
	struct Spectrum *spectrum;
	Triple bar_axis,bar_centre,side_axis;

	ENTER(gfx_create_colour_bar);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (NULL != (command_data = (struct Cmiss_command_data *)command_data_void))
		{
			/* initialise defaults */
			graphics_object_name = duplicate_string("colour_bar");
			number_format = duplicate_string("%+.4e");
			/* must access it now, because we deaccess it later */
			label_material=
				ACCESS(Graphical_material)(Material_package_get_default_material(command_data->material_package));
			material=
				ACCESS(Graphical_material)(Material_package_get_default_material(command_data->material_package));
			spectrum=ACCESS(Spectrum)(command_data->default_spectrum);
			number_of_components=3;
			bar_centre[0]=-0.9;
			bar_centre[1]=0.0;
			bar_centre[2]=0.5;
			bar_axis[0]=0.0;
			bar_axis[1]=1.0;
			bar_axis[2]=0.0;
			side_axis[0]=1.0;
			side_axis[1]=0.0;
			side_axis[2]=0.0;
			bar_length=1.6;
			extend_length=0.06;
			bar_radius=0.06;
			tick_length=0.04;
			tick_divisions=10;
			font_name = (char *)NULL;

			option_table=CREATE(Option_table)();
			/* as */
			Option_table_add_entry(option_table,"as",&graphics_object_name,
				(void *)1,set_name);
			/* axis */
			Option_table_add_entry(option_table,"axis",bar_axis,
				&number_of_components,set_float_vector);
			/* centre */
			Option_table_add_entry(option_table,"centre",bar_centre,
				&number_of_components,set_float_vector);
			/* divisions */
			Option_table_add_entry(option_table,"divisions",&tick_divisions,
				NULL,set_int_non_negative);
			/* extend_length */
			Option_table_add_entry(option_table,"extend_length",&extend_length,
				NULL,set_float_non_negative);
			/* font */
			Option_table_add_name_entry(option_table, "font",
				&font_name);
			/* label_material */
			Option_table_add_set_Material_entry(option_table, "label_material", &label_material,
				command_data->material_package);
			/* length */
			Option_table_add_entry(option_table,"length",&bar_length,
				NULL,set_float_positive);
			/* number_format */
			Option_table_add_entry(option_table,"number_format",&number_format,
				(void *)1,set_name);
			/* material */
			Option_table_add_set_Material_entry(option_table, "material", &material,
				command_data->material_package);
			/* radius */
			Option_table_add_entry(option_table,"radius",&bar_radius,
				NULL,set_float_positive);
			/* spectrum */
			Option_table_add_entry(option_table,"spectrum",&spectrum,
				command_data->spectrum_manager,set_Spectrum);
			/* tick_direction */
			Option_table_add_entry(option_table,"tick_direction",side_axis,
				&number_of_components,set_float_vector);
			/* tick_length */
			Option_table_add_entry(option_table,"tick_length",&tick_length,
				NULL,set_float_non_negative);
			if (0 != (return_code = Option_table_multi_parse(option_table, state)))
			{
				if (100 < tick_divisions)
				{
					display_message(WARNING_MESSAGE,"Limited to 100 tick_divisions");
					tick_divisions=100;
				}
				if (!(font_name && (0 != (font = Cmiss_graphics_module_find_font_by_name(
					command_data->graphics_module, font_name)))))
				{
					font = ACCESS(Cmiss_graphics_font)(command_data->default_font);
				}
				/* try to find existing colour_bar for updating */
				graphics_object=FIND_BY_IDENTIFIER_IN_MANAGER(GT_object,name)(
					graphics_object_name,command_data->glyph_manager);
				if (create_Spectrum_colour_bar(&graphics_object,
						graphics_object_name,spectrum,/*component_number*/0,
						bar_centre,bar_axis,side_axis,
						bar_length,bar_radius,extend_length,tick_divisions,tick_length,
						number_format,material,label_material, font))
				{
					ACCESS(GT_object)(graphics_object);
					if (IS_MANAGED(GT_object)(graphics_object,
						command_data->glyph_manager) ||
						ADD_OBJECT_TO_MANAGER(GT_object)(graphics_object,
							command_data->glyph_manager))
					{
						return_code=1;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"gfx_create_colour_bar.  Could not add graphics object to list");
						return_code=0;
					}
					DEACCESS(GT_object)(&graphics_object);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"gfx_create_colour_bar.  Could not create colour bar");
					return_code=0;
				}
				DEACCESS(Cmiss_graphics_font)(&font);
			} /* parse error, help */
			DESTROY(Option_table)(&option_table);
			DEACCESS(Graphical_material)(&label_material);
			DEACCESS(Graphical_material)(&material);
			DEACCESS(Spectrum)(&spectrum);
			DEALLOCATE(graphics_object_name);
			DEALLOCATE(number_format);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_create_colour_bar.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_colour_bar.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_colour_bar */

#if defined (WX_USER_INTERFACE)
static int gfx_create_element_creator(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 17 January 2003

DESCRIPTION :
Executes a GFX CREATE ELEMENT_CREATOR command.
==============================================================================*/
{
	int return_code;
	USE_PARAMETER(dummy_to_be_modified);
	ENTER(gfx_create_element_creator);
	USE_PARAMETER(state);
	USE_PARAMETER(command_data_void);
	display_message(INFORMATION_MESSAGE,"\ncommand has been removed from the cmgui-wx.\n"
		"please use gfx modify window (NAME) node ? for further instruction for creating elements\n"
		"or directly create new elements using the node tool");
		return_code=0;
	LEAVE;
	return (return_code);
} /* gfx_create_element_creator */
#endif /* defined (WX_USER_INTERFACE)*/

struct Interpreter_command_element_selection_callback_data
{
	char *perl_action;
	struct Interpreter *interpreter;
}; /* struct Interpreter_command_element_selection_callback_data */

/***************************************************************************//**
 * Adds or removes elements to/from group.
 * @param manage_nodes  Set if nodes are added/removed with elements. Nodes are
 * only removed if not in use by any other elements in group.
 * @param manage_faces  Set if faces are added/removed with parent elements.
 * Faces are only removed if not in use by any other elements in group.
 */
static int process_modify_element_group(Cmiss_field_group_id group,
	Cmiss_region_id region, int dimension, char add_flag,
	Cmiss_field_id conditional_field, Cmiss_field_group_id from_group,
	Multi_range *element_ranges, char selected_flag, FE_value time,
	int manage_nodes, int manage_faces)
{
	if (!group || !region)
		return 0;
	int return_code = 1;
	Cmiss_field_module_id field_module = Cmiss_region_get_field_module(region);
	Cmiss_mesh_id master_mesh = Cmiss_field_module_find_mesh_by_dimension(field_module, dimension);
	char remove_flag = !add_flag;
	Cmiss_mesh_group_id selection_mesh_group = 0;
	if (selected_flag)
	{
		Cmiss_rendition *rendition = Cmiss_region_get_rendition_internal(region);
		Cmiss_field_group_id selection_group = Cmiss_rendition_get_selection_group(rendition);
		if (selection_group)
		{
			Cmiss_field_element_group_id selection_element_group =
				Cmiss_field_group_get_element_group(selection_group, master_mesh);
			if (selection_element_group)
			{
				selection_mesh_group = Cmiss_field_element_group_get_mesh(selection_element_group);
				Cmiss_field_element_group_destroy(&selection_element_group);
			}
			Cmiss_field_group_destroy(&selection_group);
		}
		Cmiss_rendition_destroy(&rendition);
	}
	Cmiss_mesh_group_id from_mesh_group = 0;
	if (from_group)
	{
		Cmiss_field_element_group_id from_element_group =
			Cmiss_field_group_get_element_group(from_group, master_mesh);
		if (from_element_group)
		{
			from_mesh_group = Cmiss_field_element_group_get_mesh(from_element_group);
			Cmiss_field_element_group_destroy(&from_element_group);
		}

	}
	int objects_processed = 0;
	int elements_not_processed = 0;
	if (((!selected_flag) || selection_mesh_group) && ((!from_group) || from_mesh_group))
	{
		Cmiss_field_module_begin_change(field_module);
		Cmiss_field_element_group_id modify_element_group = Cmiss_field_group_get_element_group(group, master_mesh);
		if (!modify_element_group)
			modify_element_group = Cmiss_field_group_create_element_group(group, master_mesh);
		Cmiss_mesh_group_id modify_mesh_group = Cmiss_field_element_group_get_mesh(modify_element_group);
		objects_processed += Cmiss_mesh_get_size(Cmiss_mesh_group_base_cast(modify_mesh_group));
		Cmiss_field_element_group_destroy(&modify_element_group);
		Cmiss_field_cache_id cache = Cmiss_field_module_create_cache(field_module);
		Cmiss_field_cache_set_time(cache, time);

		Cmiss_nodeset_group_id modify_nodeset_group = 0;
		Cmiss_field_node_group_id remove_node_group = 0;
		Cmiss_nodeset_group_id remove_nodeset_group = 0;
		if (manage_nodes)
		{
			Cmiss_nodeset_id master_nodeset = Cmiss_field_module_find_nodeset_by_name(field_module, "cmiss_nodes");
			Cmiss_field_node_group_id modify_node_group = Cmiss_field_group_get_node_group(group, master_nodeset);
			if ((!modify_node_group) && add_flag)
				modify_node_group = Cmiss_field_group_create_node_group(group, master_nodeset);
			if (modify_node_group)
			{
				modify_nodeset_group = Cmiss_field_node_group_get_nodeset(modify_node_group);
				objects_processed += Cmiss_nodeset_get_size(Cmiss_nodeset_group_base_cast(modify_nodeset_group));
				if (remove_flag)
				{
					Cmiss_field_id remove_node_group_field = Cmiss_field_module_create_node_group(field_module, master_nodeset);
					remove_node_group = Cmiss_field_cast_node_group(remove_node_group_field);
					remove_nodeset_group = Cmiss_field_node_group_get_nodeset(remove_node_group);
					Cmiss_field_destroy(&remove_node_group_field);
				}
				Cmiss_field_node_group_destroy(&modify_node_group);
			}
			Cmiss_nodeset_destroy(&master_nodeset);
		}
		Cmiss_nodeset_group_id working_nodeset_group = add_flag ? modify_nodeset_group : remove_nodeset_group;

		Cmiss_mesh_id master_face_mesh = 0;
		Cmiss_mesh_group_id modify_face_mesh_group = 0;
		Cmiss_field_element_group_id working_face_element_group = 0;
		Cmiss_mesh_group_id working_face_mesh_group = 0;
		if (manage_faces && (1 < dimension))
		{
			master_face_mesh = Cmiss_field_module_find_mesh_by_dimension(field_module, dimension - 1);
			Cmiss_field_element_group_id modify_face_element_group = Cmiss_field_group_get_element_group(group, master_face_mesh);
			if ((!modify_face_element_group) && add_flag)
				modify_face_element_group = Cmiss_field_group_create_element_group(group, master_face_mesh);
			if (modify_face_element_group)
			{
				modify_face_mesh_group = Cmiss_field_element_group_get_mesh(modify_face_element_group);
				objects_processed += Cmiss_mesh_get_size(Cmiss_mesh_group_base_cast(modify_face_mesh_group));
				Cmiss_field_id working_face_element_group_field = Cmiss_field_module_create_element_group(field_module, master_face_mesh);
				working_face_element_group = Cmiss_field_cast_element_group(working_face_element_group_field);
				working_face_mesh_group = Cmiss_field_element_group_get_mesh(working_face_element_group);
				Cmiss_field_destroy(&working_face_element_group_field);
				Cmiss_field_element_group_destroy(&modify_face_element_group);
			}
		}

		Cmiss_mesh_id iteration_mesh = master_mesh;
		Cmiss_mesh_id selection_mesh = Cmiss_mesh_group_base_cast(selection_mesh_group);
		Cmiss_mesh_id from_mesh = Cmiss_mesh_group_base_cast(from_mesh_group);
		if (selected_flag && selection_mesh && !Cmiss_mesh_match(selection_mesh, Cmiss_mesh_group_base_cast(modify_mesh_group)))
		{
			iteration_mesh = selection_mesh;
		}
		if (from_mesh && (!Cmiss_mesh_match(from_mesh, Cmiss_mesh_group_base_cast(modify_mesh_group))) &&
			(Cmiss_mesh_get_size(from_mesh) < Cmiss_mesh_get_size(iteration_mesh)))
		{
			iteration_mesh = from_mesh;
		}
		Cmiss_element_iterator_id iter = Cmiss_mesh_create_element_iterator(iteration_mesh);
		Cmiss_element_id element = 0;
		while (NULL != (element = Cmiss_element_iterator_next_non_access(iter)))
		{
			if (element_ranges && !Multi_range_is_value_in_range(element_ranges, Cmiss_element_get_identifier(element)))
				continue;
			if (selection_mesh && (selection_mesh != iteration_mesh) && !Cmiss_mesh_contains_element(selection_mesh, element))
				continue;
			if (from_mesh && (from_mesh != iteration_mesh) && !Cmiss_mesh_contains_element(from_mesh, element))
				continue;
			if (conditional_field)
			{
				Cmiss_field_cache_set_element(cache, element);
				if (!Cmiss_field_evaluate_boolean(conditional_field, cache))
					continue;
			}
			if (add_flag)
			{
				if (!Cmiss_mesh_contains_element(Cmiss_mesh_group_base_cast(modify_mesh_group), element))
				{
					if (!Cmiss_mesh_group_add_element(modify_mesh_group, element))
					{
						display_message(ERROR_MESSAGE,
							"gfx modify egroup:  Could not add element %d", Cmiss_element_get_identifier(element));
						return_code = 0;
						break;
					}
				}
			}
			else
			{
				if (Cmiss_mesh_contains_element(Cmiss_mesh_group_base_cast(modify_mesh_group), element))
				{
					if (!Cmiss_mesh_group_remove_element(modify_mesh_group, element))
					{
						display_message(ERROR_MESSAGE,
							"gfx modify egroup:  Could not remove element %d", Cmiss_element_get_identifier(element));
						return_code = 0;
						break;
					}
				}
			}
			if (working_nodeset_group)
			{
				Cmiss_nodeset_group_add_element_nodes(working_nodeset_group, element);
			}
			if (working_face_mesh_group)
			{
				Cmiss_mesh_group_add_element_faces(working_face_mesh_group, element);
			}
		}
		Cmiss_element_iterator_destroy(&iter);

		if (remove_flag && (remove_nodeset_group || working_face_mesh_group))
		{
			// don't include faces and nodes still used by elements remaining in modify_mesh_group
			// Note: ignores nodes for other dimensions
			iter = Cmiss_mesh_create_element_iterator(Cmiss_mesh_group_base_cast(modify_mesh_group));
				while (NULL != (element = Cmiss_element_iterator_next_non_access(iter)))
				{
				if (remove_nodeset_group)
				{
					Cmiss_nodeset_group_remove_element_nodes(remove_nodeset_group, element);
				}
				if (working_face_mesh_group)
				{
					Cmiss_mesh_group_remove_element_faces(working_face_mesh_group, element);
				}
				}
				Cmiss_element_iterator_destroy(&iter);
				if (remove_nodeset_group)
				{
				Cmiss_nodeset_group_remove_nodes_conditional(modify_nodeset_group,
					Cmiss_field_node_group_base_cast(remove_node_group));
				}
		}
		if (working_face_mesh_group)
		{
			Cmiss_mesh_group_id modify_line_mesh_group = 0;
			Cmiss_field_element_group_id remove_line_element_group = 0;
			Cmiss_mesh_group_id remove_line_mesh_group = 0;
			if (2 < dimension)
			{
				Cmiss_mesh_id master_line_mesh = Cmiss_field_module_find_mesh_by_dimension(field_module, dimension - 2);
				Cmiss_field_element_group_id modify_line_element_group = Cmiss_field_group_get_element_group(group, master_line_mesh);
				if (add_flag && !modify_line_element_group)
					modify_line_element_group = Cmiss_field_group_create_element_group(group, master_line_mesh);
				if (modify_line_element_group)
				{
					modify_line_mesh_group = Cmiss_field_element_group_get_mesh(modify_line_element_group);
					objects_processed += Cmiss_mesh_get_size(Cmiss_mesh_group_base_cast(modify_line_mesh_group));
					if (remove_flag)
					{
						Cmiss_field_id remove_line_element_group_field = Cmiss_field_module_create_element_group(field_module, master_line_mesh);
						remove_line_element_group = Cmiss_field_cast_element_group(remove_line_element_group_field);
						remove_line_mesh_group = Cmiss_field_element_group_get_mesh(remove_line_element_group);
						Cmiss_field_destroy(&remove_line_element_group_field);
					}
					Cmiss_field_element_group_destroy(&modify_line_element_group);
				}
				Cmiss_mesh_destroy(&master_line_mesh);
			}
			Cmiss_mesh_group_id working_line_mesh_group = add_flag ? modify_line_mesh_group : remove_line_mesh_group;
			iter = Cmiss_mesh_create_element_iterator(Cmiss_mesh_group_base_cast(working_face_mesh_group));
			while (NULL != (element = Cmiss_element_iterator_next_non_access(iter)))
			{
				if (add_flag)
				{
					Cmiss_mesh_group_add_element(modify_face_mesh_group, element);
				}
				else
				{
					Cmiss_mesh_group_remove_element(modify_face_mesh_group, element);
				}
				if (working_line_mesh_group)
				{
					Cmiss_mesh_group_add_element_faces(working_line_mesh_group, element);
				}
			}
			Cmiss_element_iterator_destroy(&iter);
			if (remove_line_element_group)
			{
				Cmiss_mesh_group_remove_elements_conditional(modify_line_mesh_group,
					Cmiss_field_element_group_base_cast(remove_line_element_group));
			}
			Cmiss_mesh_group_destroy(&remove_line_mesh_group);
			Cmiss_field_element_group_destroy(&remove_line_element_group);
			if (modify_line_mesh_group)
			{
				objects_processed -= Cmiss_mesh_get_size(Cmiss_mesh_group_base_cast(modify_line_mesh_group));
				Cmiss_mesh_group_destroy(&modify_line_mesh_group);
			}
		}

		Cmiss_mesh_group_destroy(&working_face_mesh_group);
		Cmiss_field_element_group_destroy(&working_face_element_group);
		if (modify_face_mesh_group)
		{
			objects_processed -= Cmiss_mesh_get_size(Cmiss_mesh_group_base_cast(modify_face_mesh_group));
			Cmiss_mesh_group_destroy(&modify_face_mesh_group);
		}
		Cmiss_mesh_destroy(&master_face_mesh);
		Cmiss_nodeset_group_destroy(&remove_nodeset_group);
		Cmiss_field_node_group_destroy(&remove_node_group);
		if (modify_nodeset_group)
		{
			objects_processed -= Cmiss_nodeset_get_size(Cmiss_nodeset_group_base_cast(modify_nodeset_group));
			Cmiss_nodeset_group_destroy(&modify_nodeset_group);
		}
		Cmiss_field_cache_destroy(&cache);
		objects_processed -= Cmiss_mesh_get_size(Cmiss_mesh_group_base_cast(modify_mesh_group));
		Cmiss_mesh_group_destroy(&modify_mesh_group);
		Cmiss_field_module_end_change(field_module);
	}
	if (0 < elements_not_processed)
	{
		display_message(WARNING_MESSAGE,
			"gfx modify egroup:  %d elements could not be removed", elements_not_processed);
	}
	else if (0 == objects_processed)
	{
		display_message(WARNING_MESSAGE, "gfx modify egroup:  group unchanged");
	}
	Cmiss_mesh_group_destroy(&from_mesh_group);
	Cmiss_mesh_group_destroy(&selection_mesh_group);
	Cmiss_mesh_destroy(&master_mesh);
	Cmiss_field_module_destroy(&field_module);

	return return_code;
}

/***************************************************************************//**
 * Executes a GFX CREATE EGROUP/NGROUP/DGROUP command.
 * <use_object_type> is an integer; 0=elements, 1=nodes, 2=data.
 */
static int gfx_create_group(struct Parse_state *state,
	void *use_object_type, void *root_region_void)
{
	int return_code = 0;
	Cmiss_region_id root_region = reinterpret_cast<Cmiss_region_id>(root_region_void);
	if (state && root_region)
	{
		int object_type = VOIDPTR2INT(use_object_type);
		char *group_name = 0;
		if (set_name(state, (void *)&group_name, (void *)1))
		{
			Cmiss_region_id region = Cmiss_region_access(root_region);
			Multi_range *add_ranges = CREATE(Multi_range)();
			char *from_group_name = 0;
			int manage_subobjects = 1;

			Option_table *option_table = CREATE(Option_table)();
			Option_table_add_entry(option_table, "add_ranges", add_ranges, NULL, set_Multi_range);
			Option_table_add_string_entry(option_table, "from", &from_group_name, " GROUP_NAME");
			Option_table_add_switch(option_table, "manage_subobjects", "no_manage_subobjects", &manage_subobjects);
			Option_table_add_set_Cmiss_region(option_table, "region", root_region, &region);
			return_code = Option_table_multi_parse(option_table, state);
			DESTROY(Option_table)(&option_table);

			Cmiss_field_group_id from_group = 0;
			if (return_code && from_group_name)
			{
				Cmiss_field *field =	FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
					from_group_name, Cmiss_region_get_Computed_field_manager(region));
				from_group = Cmiss_field_cast_group(field);
				if (!from_group)
				{
					display_message(ERROR_MESSAGE, "gfx create group: '%s' is not a group.", from_group_name);
					return_code = 0;
				}
			}
			if (return_code)
			{
				Cmiss_region_id child_region = Cmiss_region_find_child_by_name(region, group_name);
				if (child_region)
				{
					display_message(ERROR_MESSAGE, "Child region with name '%s' already exists.", group_name);
					Cmiss_region_destroy(&child_region);
					return_code = 0;
				}
				Cmiss_field_module_id field_module = Cmiss_region_get_field_module(region);
				Cmiss_field_id field = Cmiss_field_module_find_field_by_name(field_module, group_name);
				if (field)
				{
					display_message(ERROR_MESSAGE, "Group/field with name '%s' already exists.", group_name);
					Cmiss_field_destroy(&field);
					return_code = 0;
				}
				if (return_code)
				{
					Cmiss_field_module_begin_change(field_module);
					Cmiss_field_id group_field = Cmiss_field_module_create_group(field_module);
					return_code = Cmiss_field_set_name(group_field, group_name) &&
						Cmiss_field_set_attribute_integer(group_field, CMISS_FIELD_ATTRIBUTE_IS_MANAGED, 1);
					if (Multi_range_get_number_of_ranges(add_ranges) > 0)
					{
						Cmiss_field_group_id group = Cmiss_field_cast_group(group_field);
						switch (object_type)
						{
							case 0: // elements
							{
								int max_dimension = FE_region_get_highest_dimension(Cmiss_region_get_FE_region(region));
								double time = 0;
								return_code = process_modify_element_group(group, region, max_dimension,
									/*add_flag*/1, /*conditional_field*/0, from_group, add_ranges,
									/*selected_flag*/0, time, /*manage_nodes*/manage_subobjects, /*manage_faces*/manage_subobjects);
							} break;
							case 1: // nodes
							case 2: // data
							{
								Cmiss_nodeset_id master_nodeset = Cmiss_field_module_find_nodeset_by_name(field_module,
									(object_type == 1) ? "cmiss_nodes" : "cmiss_data");
								Cmiss_field_node_group_id node_group = Cmiss_field_group_create_node_group(group, master_nodeset);
								Cmiss_nodeset_group_id modify_nodeset_group = Cmiss_field_node_group_get_nodeset(node_group);
								Cmiss_node_iterator_id iter = Cmiss_nodeset_create_node_iterator(master_nodeset);
								Cmiss_node_id node = 0;
								while (NULL != (node = Cmiss_node_iterator_next_non_access(iter)))
								{
									if (Multi_range_is_value_in_range(add_ranges, Cmiss_node_get_identifier(node)))
									{
										if (!Cmiss_nodeset_group_add_node(modify_nodeset_group, node))
										{
											return_code = 0;
											break;
										}
									}
								}
								Cmiss_node_iterator_destroy(&iter);
								Cmiss_nodeset_group_destroy(&modify_nodeset_group);
								Cmiss_field_node_group_destroy(&node_group);
								Cmiss_nodeset_destroy(&master_nodeset);
							} break;
							default:
							{
								return_code = 0;
							} break;
						}
						Cmiss_field_group_destroy(&group);
					}
					Cmiss_field_destroy(&group_field);
					Cmiss_field_module_end_change(field_module);
				}
				Cmiss_field_module_destroy(&field_module);
			}
			if (from_group_name)
			{
				DEALLOCATE(from_group_name);
			}
			Cmiss_field_group_destroy(&from_group);
			Cmiss_region_destroy(&region);
			DESTROY(Multi_range)(&add_ranges);
		}
		if (group_name)
		{
			DEALLOCATE(group_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_create_group.  Invalid argument(s)");
		return_code = 0;
	}
	return return_code;
}

static int gfx_create_flow_particles(struct Parse_state *state,
	void *create_more,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 19 May 2003

DESCRIPTION :
Executes a GFX CREATE FLOW_PARTICLES command.
==============================================================================*/
{
	float time;
	FE_value xi[3];
	gtObject *graphics_object;
	struct GT_pointset *pointset;
	int current_number_of_particles, element_number, number_of_particles,
		return_code, vector_components;
	struct Cmiss_command_data *command_data;
	struct Cmiss_region *region;
	struct Element_to_particle_data element_to_particle_data;
	struct Computed_field *coordinate_field, *stream_vector_field;
	struct FE_region *fe_region;
	struct Graphical_material *material;
	struct Spectrum *spectrum;
	Triple *new_particle_positions, *old_particle_positions,
		*final_particle_positions;
	struct Option_table *option_table;

	ENTER(gfx_create_flow_particles);
	if (state && (command_data = (struct Cmiss_command_data *)command_data_void))
	{
		/* initialise defaults */
		char *graphics_object_name = duplicate_string("particles");
		char *region_path = Cmiss_region_get_root_region_path();
		char *coordinate_field_name = NULL;
		char *stream_vector_field_name = NULL;
		element_number=0;  /* Zero gives all elements in group */
		coordinate_field=(struct Computed_field *)NULL;
		stream_vector_field=(struct Computed_field *)NULL;
		vector_components=3;
		xi[0]=0.5;
		xi[1]=0.5;
		xi[2]=0.5;
		time=0;
		/* must access it now,because we deaccess it later */
		material=
			ACCESS(Graphical_material)(Material_package_get_default_material(command_data->material_package));
		spectrum=
			ACCESS(Spectrum)(command_data->default_spectrum);

		option_table=CREATE(Option_table)();
		/* as */
		Option_table_add_entry(option_table, "as", &graphics_object_name,
			(void *)1, set_name);
		/* coordinate */
		Option_table_add_string_entry(option_table,"coordinate",&coordinate_field_name,
			" FIELD_NAME");
		/* element */
		Option_table_add_entry(option_table, "element", &element_number,
			NULL, set_int_non_negative);
		/* from */
		Option_table_add_entry(option_table, "from", &region_path,
			command_data->root_region, set_Cmiss_region_path);
		/* initial_xi */
		Option_table_add_entry(option_table, "initial_xi", xi,
			&(vector_components), set_FE_value_array);
		/* material */
		Option_table_add_set_Material_entry(option_table, "material", &material,
			command_data->material_package);
		/* spectrum */
		Option_table_add_entry(option_table,"spectrum",&spectrum,
			command_data->spectrum_manager,set_Spectrum);
		/* time */
		Option_table_add_entry(option_table,"time",&time,NULL,set_float);
		/* vector */
		Option_table_add_string_entry(option_table,"vector",&stream_vector_field_name,
			" FIELD_NAME");
		return_code=Option_table_multi_parse(option_table,state);
		/* no errors, not asking for help */
		if (return_code)
		{
			if (!(Cmiss_region_get_region_from_path_deprecated(command_data->root_region,
				region_path, &region) &&
				(fe_region = Cmiss_region_get_FE_region(region))))
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_flow_particles.  Invalid region");
				return_code = 0;
			}
			else
			{
				if (coordinate_field_name)
				{
				Cmiss_field_module_id field_module = Cmiss_region_get_field_module(region);
				coordinate_field = Cmiss_field_module_find_field_by_name(field_module, coordinate_field_name);
				if (stream_vector_field_name)
				{
					stream_vector_field = Cmiss_field_module_find_field_by_name(field_module, stream_vector_field_name);
					if (!stream_vector_field)
					{
						display_message(ERROR_MESSAGE,
							"gfx_create_flow_particles:  stream vector field cannot be found");
						return_code = 0;
					}
					else
					{
						if (!Computed_field_is_stream_vector_capable(stream_vector_field, NULL))
						{
							display_message(ERROR_MESSAGE,
								"gfx_create_flow_particles: field specifiedis not a valid vector field");
							return_code = 0;
						}
					}
				}
				if (!coordinate_field)
				{
					display_message(ERROR_MESSAGE,
						"gfx_create_flow_particles:  coordinates field cannot be found");
					return_code = 0;
				}
				else
				{
					if (!Computed_field_has_up_to_3_numerical_components(coordinate_field, NULL))
					{
						display_message(ERROR_MESSAGE,
							"gfx_create_flow_particles: field specifiedis not a valid coordinates field");
						return_code = 0;
					}
				}
				Cmiss_field_module_destroy(&field_module);
				}
				else
				{
					display_message(WARNING_MESSAGE, "Must specify a coordinate field");
					return_code = 0;
				}
			}
		}
		if (return_code)
		{
			if (NULL != (graphics_object=FIND_BY_IDENTIFIER_IN_MANAGER(GT_object,name)(
				graphics_object_name,command_data->glyph_manager)))
			{
				if (create_more)
				{
					if (NULL != (pointset = GT_OBJECT_GET(GT_pointset)(graphics_object, time)))
					{
						GT_pointset_get_point_list(pointset,
							&current_number_of_particles, &old_particle_positions);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"gfx_create_flow_particles.  Missing pointlist for adding more");
						return_code = 0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"gfx_create_flow_particles.  Object already exists");
					return_code = 0;
				}
			}
			else
			{
				if (create_more)
				{
					display_message(ERROR_MESSAGE, "gfx_create_flow_particles.  "
						"Graphics object does not exist for adding more to");
					return_code=0;
				}
				else
				{
					if ((graphics_object=CREATE(GT_object)(graphics_object_name,
						g_POINTSET,material))&&
						ADD_OBJECT_TO_MANAGER(GT_object)(graphics_object,
							command_data->glyph_manager))
					{
						return_code=1;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"gfx_create_flow_particles.  Could not create graphics object");
						DESTROY(GT_object)(&graphics_object);
						return_code=0;
					}
				}
			}
			if (return_code)
			{
				number_of_particles = FE_region_get_number_of_FE_elements_all_dimensions(fe_region);
				if (create_more)
				{
					number_of_particles += current_number_of_particles;
					if (REALLOCATE(new_particle_positions,old_particle_positions,Triple,
						number_of_particles))
					{
						GT_pointset_set_point_list(pointset, number_of_particles,
							new_particle_positions);
						element_to_particle_data.pointlist = &new_particle_positions;
						element_to_particle_data.index = current_number_of_particles;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"gfx_create_flow_particles.  Unable to reallocate pointset");
						return_code = 0;
					}
				}
				else
				{
					if (ALLOCATE(new_particle_positions,Triple,number_of_particles) &&
						(pointset=CREATE(GT_pointset)(number_of_particles,
							new_particle_positions,(char **)NULL,	g_POINT_MARKER,
							1,g_NO_DATA,(GLfloat *) NULL,(int *)NULL,command_data->default_font)))
					{
						element_to_particle_data.pointlist= &new_particle_positions;
						element_to_particle_data.index=0;
					}
					else
					{
						DEALLOCATE(new_particle_positions);
						display_message(ERROR_MESSAGE,
							"gfx_create_flow_particles.  Unable to create pointset");
						return_code = 0;
					}
				}
				if (return_code)
				{
					Cmiss_field_module_id field_module = Cmiss_field_get_field_module(coordinate_field);
					element_to_particle_data.field_cache = Cmiss_field_module_create_cache(field_module);
					element_to_particle_data.coordinate_field=coordinate_field;
					element_to_particle_data.element_number=element_number;
					element_to_particle_data.stream_vector_field=stream_vector_field;
					element_to_particle_data.graphics_object=graphics_object;
					element_to_particle_data.xi[0]=xi[0];
					element_to_particle_data.xi[1]=xi[1];
					element_to_particle_data.xi[2]=xi[2];
					element_to_particle_data.number_of_particles=0;
					element_to_particle_data.list= &(command_data->streampoint_list);
					return_code = FE_region_for_each_FE_element(fe_region,
						element_to_particle, (void *)&element_to_particle_data);
					number_of_particles = element_to_particle_data.number_of_particles;
					if (create_more)
					{
						number_of_particles += current_number_of_particles;
					}
					if (return_code && number_of_particles &&
						REALLOCATE(final_particle_positions, new_particle_positions,
							Triple, number_of_particles))
					{
						GT_pointset_set_point_list(pointset, number_of_particles,
							final_particle_positions);
						if (create_more)
						{
							GT_object_changed(graphics_object);
						}
						else
						{
							if (GT_OBJECT_ADD(GT_pointset)(graphics_object,time,pointset))
							{
								return_code = set_GT_object_Spectrum(graphics_object,spectrum);
							}
							else
							{
								DESTROY(GT_pointset)(&pointset);
								display_message(ERROR_MESSAGE, "gfx_create_flow_particles.  "
									"Could not add pointset to graphics object");
								return_code = 0;
							}
						}
					}
					else
					{
						if (create_more)
						{
							DEALLOCATE(old_particle_positions);
						}
						else
						{
							DESTROY(GT_pointset)(&pointset);
						}
						if (return_code)
						{
							display_message(WARNING_MESSAGE,"No particles created");
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"gfx_create_flow_particles.  Error creating particles");
						}
					}
					Cmiss_field_cache_destroy(&element_to_particle_data.field_cache);
					Cmiss_field_module_destroy(&field_module);
				}
			}
		} /* parse error,help */
		DESTROY(Option_table)(&option_table);
		if (coordinate_field)
		{
			DEACCESS(Computed_field)(&coordinate_field);
		}
		if (stream_vector_field)
		{
			DEACCESS(Computed_field)(&stream_vector_field);
		}
		if (coordinate_field_name)
			DEALLOCATE(coordinate_field_name);
		if (stream_vector_field_name)
			DEALLOCATE(stream_vector_field);
		DEALLOCATE(region_path);
		DEACCESS(Spectrum)(&spectrum);
		DEACCESS(Graphical_material)(&material);
		DEALLOCATE(graphics_object_name);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_create_flow_particles.  Invalid argument(s)");
		return_code = 0;
	}

	LEAVE;

	return (return_code);
} /* gfx_create_flow_particles */

static int gfx_modify_flow_particles(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 18 January 2000

DESCRIPTION :
Executes a GFX MODIFY FLOW_PARTICLES command.
==============================================================================*/
{
	int return_code;
	FE_value stepsize,time;
	struct Cmiss_command_data *command_data;
	struct Computed_field *coordinate_field,*stream_vector_field;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_coordinate_field_data,
		set_stream_vector_field_data;

	ENTER(gfx_modify_flow_particles);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (NULL != (command_data = (struct Cmiss_command_data *)command_data_void))
		{
			coordinate_field=(struct Computed_field *)NULL;
			stream_vector_field=(struct Computed_field *)NULL;
			stepsize=1;
			/* If time of 0 is sent the previous points are updated at the previous
				time value */
			time=0;

			option_table=CREATE(Option_table)();
			/* coordinate */
			set_coordinate_field_data.computed_field_manager=
				Computed_field_package_get_computed_field_manager(
					command_data->computed_field_package);
			set_coordinate_field_data.conditional_function=
				Computed_field_has_up_to_3_numerical_components;
			set_coordinate_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"coordinate",&coordinate_field,
				&set_coordinate_field_data,set_Computed_field_conditional);
			/* stepsize */
			Option_table_add_entry(option_table,"stepsize",&stepsize,
				NULL,set_FE_value);
			/* time */
			Option_table_add_entry(option_table,"time",&time,NULL,set_float);
			/* vector */
			set_stream_vector_field_data.computed_field_manager=
				Computed_field_package_get_computed_field_manager(
					command_data->computed_field_package);
			set_stream_vector_field_data.conditional_function=
				Computed_field_is_stream_vector_capable;
			set_stream_vector_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"vector",&stream_vector_field,
				&set_stream_vector_field_data,set_Computed_field_conditional);
			return_code=Option_table_multi_parse(option_table,state);
			/* no errors,not asking for help */
			if (return_code)
			{
				Cmiss_field_module_id field_module = Cmiss_field_get_field_module(coordinate_field);
				Cmiss_field_cache_id field_cache = Cmiss_field_module_create_cache(field_module);
				return_code=update_flow_particle_list(
					command_data->streampoint_list,field_cache,coordinate_field,stream_vector_field,
					stepsize,time);
				Cmiss_field_cache_destroy(&field_cache);
				Cmiss_field_module_destroy(&field_module);
			}
			DESTROY(Option_table)(&option_table);
			if (coordinate_field)
			{
				DEACCESS(Computed_field)(&coordinate_field);
			}
			if (stream_vector_field)
			{
				DEACCESS(Computed_field)(&stream_vector_field);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_modify_flow_particles.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_modify_flow_particles.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_modify_flow_particles */

/***************************************************************************//**
 * Creates data points with embedded locations at Gauss points in a mesh.
 */
static int gfx_create_gauss_points(struct Parse_state *state,
	void *dummy_to_be_modified, void *root_region_void)
{
	ENTER(gfx_create_gauss_points);
	int return_code = 0;
	USE_PARAMETER(dummy_to_be_modified);
	Cmiss_region *root_region = reinterpret_cast<Cmiss_region *>(root_region_void);
	if (state && root_region)
	{
		int first_identifier = 1;
		char *gauss_location_field_name = 0;
		char *gauss_weight_field_name = 0;
		char *gauss_point_nodeset_name = 0;
		char *mesh_name = 0;
		int order = 4;
		Cmiss_region_id region = Cmiss_region_access(root_region);
		Option_table *option_table = CREATE(Option_table)();
		Option_table_add_help(option_table,
			"Creates points at Gauss point locations in the elements of the mesh. "
			"Nodes are created in the gauss_point_nodeset starting from first_identifier, "
			"and setting the element_xi gauss_location and real gauss_weight fields. "
			"Supports all main element shapes, with polynomial order up to 4. Order gives "
			"the number of Gauss points per element dimension for line/square/cube shapes.");
		Option_table_add_int_non_negative_entry(option_table, "first_identifier",
			&first_identifier);
		Option_table_add_string_entry(option_table, "gauss_location_field",
			&gauss_location_field_name, " FIELD_NAME");
		Option_table_add_string_entry(option_table, "gauss_point_nodeset", &gauss_point_nodeset_name,
			" NODE_GROUP_FIELD_NAME|[GROUP_NAME.]cmiss_nodes|cmiss_data|none");
		Option_table_add_string_entry(option_table, "gauss_weight_field",
			&gauss_weight_field_name, " FIELD_NAME");
		Option_table_add_string_entry(option_table, "mesh", &mesh_name,
			" ELEMENT_GROUP_FIELD_NAME|[GROUP_REGION_NAME.]cmiss_mesh_1d|cmiss_mesh_2d|cmiss_mesh_3d");
		Option_table_add_int_positive_entry(option_table, "order", &order);
		Option_table_add_set_Cmiss_region(option_table, "region", root_region, &region);
		return_code = Option_table_multi_parse(option_table, state);
		DESTROY(Option_table)(&option_table);
		if (return_code)
		{
			Cmiss_field_module_id field_module = Cmiss_region_get_field_module(region);
			Cmiss_mesh_id mesh = 0;
			if (mesh_name)
			{
				mesh = Cmiss_field_module_find_mesh_by_name(field_module, mesh_name);
				if (!mesh)
				{
					display_message(ERROR_MESSAGE, "gfx create gauss_points:  Unknown mesh %s", mesh_name);
					return_code = 0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE, "gfx create gauss_points:  Must specify mesh");
				return_code = 0;
			}
			Cmiss_nodeset_id gauss_points_nodeset = 0;
			if (gauss_point_nodeset_name)
			{
				gauss_points_nodeset = Cmiss_field_module_find_nodeset_by_name(field_module, gauss_point_nodeset_name);
				if (!gauss_points_nodeset)
				{
					gauss_points_nodeset = Cmiss_nodeset_group_base_cast(
						Cmiss_field_module_create_nodeset_group_from_name_internal(
							field_module, gauss_point_nodeset_name));
				}
				if (!gauss_points_nodeset)
				{
					display_message(ERROR_MESSAGE,
						"gfx create gauss_points:  Unable to find nodeset %s", gauss_point_nodeset_name);
					return_code = 0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx create gauss_points:  Must specify gauss_point_nodeset");
				return_code = 0;
			}
			Cmiss_field_stored_mesh_location_id gauss_location_field = 0;
			if (gauss_location_field_name)
			{
				Cmiss_field_id field = Cmiss_field_module_find_field_by_name(field_module, gauss_location_field_name);
				if (field)
				{
					gauss_location_field = Cmiss_field_cast_stored_mesh_location(field);
					Cmiss_field_destroy(&field);
					if (!gauss_location_field)
					{
						display_message(ERROR_MESSAGE, "gfx create gauss_points:  Gauss location field %s is not element_xi valued", gauss_location_field_name);
						return_code = 0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE, "gfx create gauss_points:  No such field %s in region", gauss_location_field_name);
					return_code = 0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE, "gfx create gauss_points:  Must specify gauss_location_field");
				return_code = 0;
			}
			Cmiss_field_finite_element_id gauss_weight_field = 0;
			if (gauss_weight_field_name)
			{
				Cmiss_field_id field = Cmiss_field_module_find_field_by_name(field_module, gauss_weight_field_name);
				if (field)
				{
					gauss_weight_field = Cmiss_field_cast_finite_element(field);
					Cmiss_field_destroy(&field);
					if (!gauss_weight_field)
					{
						display_message(ERROR_MESSAGE, "gfx create gauss_points:  Gauss weight field %s is not scalar real finite_element", gauss_weight_field_name);
						return_code = 0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE, "gfx create gauss_points:  No such field %s in region", gauss_weight_field_name);
					return_code = 0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE, "gfx create gauss_points:  Must specify gauss_weight_field");
				return_code = 0;
			}
			if (return_code)
			{
				return_code = Cmiss_mesh_create_gauss_points(mesh, order, gauss_points_nodeset,
					first_identifier, gauss_location_field, gauss_weight_field);
			}
			Cmiss_field_finite_element_destroy(&gauss_weight_field);
			Cmiss_field_stored_mesh_location_destroy(&gauss_location_field);
			Cmiss_nodeset_destroy(&gauss_points_nodeset);
			Cmiss_mesh_destroy(&mesh);
			Cmiss_field_module_destroy(&field_module);
		}
		if (gauss_location_field_name)
			DEALLOCATE(gauss_location_field_name);
		if (gauss_weight_field_name)
			DEALLOCATE(gauss_weight_field_name);
		if (gauss_point_nodeset_name)
			DEALLOCATE(gauss_point_nodeset_name);
		if (mesh_name)
			DEALLOCATE(mesh_name);
		Cmiss_region_destroy(&region);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_create_gauss_points.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;
	return (return_code);
}

#if defined (WX_USER_INTERFACE)
static int gfx_create_graphical_material_editor(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 14 June 1999

DESCRIPTION :
Executes a GFX CREATE GRAPHICAL_MATERIAL_EDITOR command.
If there is a material editor dialog in existence, then bring it to the front,
otherwise it creates a new one.  Assumes we will only ever want one material
editor at a time.  This implementation may be changed later.
==============================================================================*/
{
	const char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;

	ENTER(gfx_create_graphical_material_editor);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (NULL != (current_token = state->current_token))
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				display_message(ERROR_MESSAGE,"Unknown option: %s",current_token);
				display_parse_state_location(state);
				return_code=0;
			}
			else
			{
				return_code=1;
			}
		}
		else
		{
			if (NULL != (command_data = (struct Cmiss_command_data *)command_data_void))
			{
#if defined (WX_USER_INTERFACE)
				return_code=material_editor_bring_up_editor(
					&(command_data->material_editor),
					command_data->root_region,
					command_data->graphics_module,
					command_data->graphics_buffer_package, command_data->user_interface);
#endif /* defined (WX_USER_INTERFACE) */
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_graphical_material_editor.  Missing command_data");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_create_graphical_material_editor.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_graphical_material_editor */
#endif /* defined (WX_USER_INTERFACE)*/

static int gfx_create_light(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 14 June 1999

DESCRIPTION :
Executes a GFX CREATE LIGHT command.
==============================================================================*/
{
	const char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Light *light;
	struct Modify_light_data modify_light_data;

	ENTER(gfx_create_light);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (NULL != (current_token = state->current_token))
		{
			if (NULL != (command_data = (struct Cmiss_command_data *)command_data_void))
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					if (!FIND_BY_IDENTIFIER_IN_MANAGER(Light,name)(current_token,
						command_data->light_manager))
					{
						if (NULL != (light=CREATE(Light)(current_token)))
						{
							MANAGER_COPY_WITHOUT_IDENTIFIER(Light,name)(light,
								command_data->default_light);
							shift_Parse_state(state,1);
							if (state->current_token)
							{
								modify_light_data.default_light=command_data->default_light;
								modify_light_data.light_manager=command_data->light_manager;
								return_code=modify_Light(state,(void *)light,
									(void *)(&modify_light_data));
							}
							else
							{
								return_code=1;
							}
							ADD_OBJECT_TO_MANAGER(Light)(light,command_data->light_manager);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"gfx_create_light.  Could not create light");
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"Light already exists: %s",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					modify_light_data.default_light=command_data->default_light;
					modify_light_data.light_manager=command_data->light_manager;
					return_code=modify_Light(state,(void *)NULL,
						(void *)(&modify_light_data));
					return_code=1;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_light.  Missing command_data_void");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing light name");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_light.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_light */

static int gfx_create_light_model(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 14 June 1999

DESCRIPTION :
Executes a GFX CREATE LMODEL command.
==============================================================================*/
{
	const char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Light_model *light_model;
	struct Modify_light_model_data modify_light_model_data;

	ENTER(gfx_create_light_model);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (NULL != (current_token = state->current_token))
		{
			if (NULL != (command_data = (struct Cmiss_command_data *)command_data_void))
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					if (!FIND_BY_IDENTIFIER_IN_MANAGER(Light_model,name)(current_token,
						command_data->light_model_manager))
					{
						if (NULL != (light_model=CREATE(Light_model)(current_token)))
						{
							MANAGER_COPY_WITHOUT_IDENTIFIER(Light_model,name)(light_model,
								command_data->default_light_model);
							shift_Parse_state(state,1);
							if (state->current_token)
							{
								modify_light_model_data.default_light_model=
									command_data->default_light_model;
								modify_light_model_data.light_model_manager=
									command_data->light_model_manager;
								return_code=modify_Light_model(state,(void *)light_model,
									(void *)(&modify_light_model_data));
							}
							else
							{
								return_code=1;
							}
							ADD_OBJECT_TO_MANAGER(Light_model)(light_model,
								command_data->light_model_manager);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"gfx_create_light_model.  Could not create light model");
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"Light_model already exists: %s",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					modify_light_model_data.default_light_model=
						command_data->default_light_model;
					modify_light_model_data.light_model_manager=
						command_data->light_model_manager;
					return_code=modify_Light_model(state,(void *)NULL,
						(void *)(&modify_light_model_data));
					return_code=1;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_light_model.  Missing command_data_void");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing light model name");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_light_model.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_light_model */

#if defined (WX_USER_INTERFACE)
static int gfx_create_node_viewer(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 6 July 2001

DESCRIPTION :
Executes a GFX CREATE NODE_VIEWER command.
==============================================================================*/
{
	const char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;

	ENTER(gfx_create_node_viewer);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (NULL != (current_token = state->current_token))
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				display_message(ERROR_MESSAGE,"Unknown option: %s",current_token);
				display_parse_state_location(state);
				return_code=0;
			}
			else
			{
				return_code=1;
			}
		}
		else
		{
			if (NULL != (command_data = (struct Cmiss_command_data *)command_data_void))
			{
				if (command_data->node_viewer)
				{
					return_code=Node_viewer_bring_window_to_front(
						command_data->node_viewer);
				}
				else
				{
					if (NULL != (command_data->node_viewer = Node_viewer_create(
						&(command_data->node_viewer),
						"Node Viewer",
						command_data->root_region, /*use_data*/0,
						command_data->graphics_module,
						command_data->default_time_keeper)))
					{
						return_code=1;
					}
					else
					{
						return_code=0;
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_node_viewer.  Missing command_data");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_node_viewer.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_node_viewer */
#endif /* defined (WX_USER_INTERFACE) */

#if defined (WX_USER_INTERFACE)
static int gfx_create_data_viewer(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 6 July 2001

DESCRIPTION :
Executes a GFX CREATE DATA_VIEWER command.
==============================================================================*/
{
	const char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;

	ENTER(gfx_create_data_viewer);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (NULL != (current_token = state->current_token))
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				display_message(ERROR_MESSAGE,"Unknown option: %s",current_token);
				display_parse_state_location(state);
				return_code=0;
			}
			else
			{
				return_code=1;
			}
		}
		else
		{
			if (NULL != (command_data = (struct Cmiss_command_data *)command_data_void))
			{
				if (command_data->data_viewer)
				{
					return_code=Node_viewer_bring_window_to_front(
						command_data->data_viewer);
				}
				else
				{
					if (NULL != (command_data->node_viewer = Node_viewer_create(
						&(command_data->node_viewer),
						"Data Viewer",
						command_data->root_region, /*use_data*/1,
						command_data->graphics_module,
						command_data->default_time_keeper)))
					{
						return_code=1;
					}
					else
					{
						return_code=0;
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_data_viewer.  Missing command_data");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_data_viewer.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_data_viewer */
#endif /* defined (WX_USER_INTERFACE)*/

#if defined (WX_USER_INTERFACE)
static int gfx_create_element_point_viewer(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 31 May 2000

DESCRIPTION :
Executes a GFX CREATE ELEMENT_POINT_VIEWER command.
==============================================================================*/
{
	const char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Time_object *time_object;

	ENTER(gfx_create_element_point_viewer);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (NULL != (current_token = state->current_token))
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				display_message(ERROR_MESSAGE,"Unknown option: %s",current_token);
				display_parse_state_location(state);
				return_code=0;
			}
			else
			{
				return_code=1;
			}
		}
		else
		{
			if (NULL != (command_data = (struct Cmiss_command_data *)command_data_void))
			{
				if (command_data->element_point_viewer)
				{
					return_code=Element_point_viewer_bring_window_to_front(
						command_data->element_point_viewer);
				}
				else
				{
					if ((time_object = Time_object_create_regular(
								 /*update_frequency*/10.0, /*time_offset*/0.0))
						&&(Time_keeper_add_time_object(command_data->default_time_keeper,
								time_object)))
					{
						Time_object_set_name(time_object, "element_point_viewer_time");
						if (NULL != (command_data->element_point_viewer=CREATE(Element_point_viewer)(
									&(command_data->element_point_viewer),
									command_data->root_region,
									command_data->element_point_ranges_selection,
									command_data->computed_field_package,
									time_object,
									command_data->user_interface)))
						{
							return_code=1;
						}
						else
						{
							return_code=0;
						}
						DEACCESS(Time_object)(&time_object);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"gfx_create_element_point_viewer.  Unable to make time object.");
						return_code=0;
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_element_point_viewer.  Missing command_data");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_create_element_point_viewer.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_element_point_viewer */
#endif /* defined (WX_USER_INTERFACE) */

/***************************************************************************//**
 * Executes a GFX CREATE REGION command.
 */
static int gfx_create_region(struct Parse_state *state,
	void *dummy, void *root_region_void)
{
	int return_code = 0;

	ENTER(gfx_create_region);
	USE_PARAMETER(dummy);
	Cmiss_region *root_region = (struct Cmiss_region *)root_region_void;
	if (state && root_region)
	{
		char *region_path = NULL;
		int error_if_exists = 1;
		Option_table *option_table = CREATE(Option_table)();
		Option_table_add_help(option_table,
			"Create a region at the supplied path, with names in the path "
			"separated by slash '/' characters. Use the 'no_error_if_exists' "
			"option to avoid errors if region exists already. ");
		Option_table_add_switch(option_table,
			"error_if_exists", "no_error_if_exists", &error_if_exists);
		Option_table_add_default_string_entry(option_table, &region_path,
			"PATH_TO_REGION");
		return_code = Option_table_multi_parse(option_table, state);
		DESTROY(Option_table)(&option_table);
		if (return_code)
		{
			Cmiss_region *region = Cmiss_region_find_subregion_at_path(root_region, region_path);
			if (region)
			{
				if (error_if_exists)
				{
					display_message(ERROR_MESSAGE,
						"gfx create region.  Region already exists at path '%s'", region_path);
					return_code = 0;
				}
				Cmiss_region_destroy(&region);
			}
			else
			{
				region = Cmiss_region_create_subregion(root_region, region_path);
				if (region)
				{
					Cmiss_region_destroy(&region);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"gfx create region.  Invalid region path '%s'", region_path);
					return_code = 0;
				}
			}
		}
		DEALLOCATE(region_path);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_create_region.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

static int gfx_create_snake(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 3 May 2006

DESCRIPTION :
Executes a GFX CREATE SNAKE command.
==============================================================================*/
{
	char *source_region_path;
	float density_factor, stiffness;
	int i, number_of_elements, number_of_fitting_fields,
		previous_state_index, return_code;
	struct Cmiss_command_data *command_data;
	struct Cmiss_region *region, *source_region;
	struct Computed_field *coordinate_field, **fitting_fields,
		*weight_field;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_coordinate_field_data,
		set_fitting_field_data, set_weight_field_data;
	struct Set_Computed_field_array_data set_fitting_field_array_data;

	ENTER(gfx_create_snake);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (command_data = (struct Cmiss_command_data *)command_data_void))
	{
		region = Cmiss_region_access(command_data->root_region);
		Cmiss_field_group_id group = 0;
		source_region_path = (char *)NULL;
		number_of_fitting_fields = 1;
		coordinate_field = (struct Computed_field *)NULL;
		weight_field = (struct Computed_field *)NULL;
		density_factor = 0.0;
		number_of_elements = 1;
		stiffness = 0.0;

		if (strcmp(PARSER_HELP_STRING,state->current_token)&&
			strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token))
		{
			/* Skip this preprocessing if we are just getting the help */
			number_of_fitting_fields = 1;
			previous_state_index = state->current_index;

			option_table = CREATE(Option_table)();
			/* number_of_fitting_fields */
			Option_table_add_entry(option_table, "number_of_fitting_fields",
				&number_of_fitting_fields, NULL, set_int_positive);
			/* absorb everything else */
			Option_table_ignore_all_unmatched_entries(option_table);
			return_code = Option_table_multi_parse(option_table, state);
			DESTROY(Option_table)(&option_table);
			/* Return back to where we were */
			shift_Parse_state(state, previous_state_index - state->current_index);
		}

		if (number_of_fitting_fields)
		{
			ALLOCATE(fitting_fields, struct Computed_field *, number_of_fitting_fields);
			for (i = 0; i < number_of_fitting_fields; i++)
			{
				fitting_fields[i] = (struct Computed_field *)NULL;
			}
		}
		else
		{
			fitting_fields = (struct Computed_field **)NULL;
		}

		option_table = CREATE(Option_table)();
		/* coordinate */
		set_coordinate_field_data.conditional_function =
			Computed_field_has_numerical_components;
		set_coordinate_field_data.conditional_function_user_data = (void *)NULL;
		set_coordinate_field_data.computed_field_manager =
			Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package);
		Option_table_add_entry(option_table, "coordinate",
			&coordinate_field, (void *)&set_coordinate_field_data,
			set_Computed_field_conditional);
		/* density_factor */
		Option_table_add_entry(option_table, "density_factor",
			&density_factor, NULL, set_float_0_to_1_inclusive);
		/* source_group */
		Option_table_add_entry(option_table, "source_group", &source_region_path,
			command_data->root_region, set_Cmiss_region_path);
		/* destination_group */
		Option_table_add_region_or_group_entry(option_table, "destination_group",
			&region, &group);
		/* fitting_fields */
		set_fitting_field_data.conditional_function =
			Computed_field_has_numerical_components;
		set_fitting_field_data.conditional_function_user_data = (void *)NULL;
		set_fitting_field_data.computed_field_manager =
			Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package);
		set_fitting_field_array_data.number_of_fields = number_of_fitting_fields;
		set_fitting_field_array_data.conditional_data = &set_fitting_field_data;
		Option_table_add_entry(option_table, "fitting_fields", fitting_fields,
			&set_fitting_field_array_data, set_Computed_field_array);
		/* number_of_fitting_fields */
		Option_table_add_entry(option_table, "number_of_fitting_fields",
			&number_of_fitting_fields, NULL, set_int_positive);
		/* number_of_elements */
		Option_table_add_entry(option_table, "number_of_elements",
			&number_of_elements, NULL, set_int_positive);
		/* stiffness */
		Option_table_add_entry(option_table, "stiffness",
			&stiffness, NULL, set_float_non_negative);
		/* weight_field */
		set_weight_field_data.conditional_function =
			Computed_field_is_scalar;
		set_weight_field_data.conditional_function_user_data = (void *)NULL;
		set_weight_field_data.computed_field_manager =
			Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package);
		Option_table_add_entry(option_table, "weight_field",
			&weight_field, (void *)&set_weight_field_data,
			set_Computed_field_conditional);
		return_code = Option_table_multi_parse(option_table, state);
		DESTROY(Option_table)(&option_table);

		if (return_code)
		{
			if (!coordinate_field)
			{
				display_message(ERROR_MESSAGE, "gfx create snake.  "
					"Must specify a coordinate_field to define on elements in snake");
				return_code = 0;
			}
			if (!(source_region_path &&
				Cmiss_region_get_region_from_path_deprecated(command_data->root_region,
					source_region_path, &source_region)))
			{
				source_region = command_data->root_region;
			}
		}
		if (return_code)
		{
			Cmiss_rendition *rendition = Cmiss_graphics_module_get_rendition(command_data->graphics_module, source_region);
			Cmiss_field_group_id selection_group = NULL;
			if (rendition)
			{
				selection_group = Cmiss_rendition_get_selection_group(rendition);
				Cmiss_rendition_destroy(&rendition);
			}
			if (selection_group)
			{
				Cmiss_field_module_id field_module = Cmiss_region_get_field_module(region);
				Cmiss_field_module_begin_change(field_module);
				struct LIST(FE_node) *data_list = FE_node_list_from_region_and_selection_group(
					source_region, NULL,	Cmiss_field_group_base_cast(selection_group), NULL, 0, /*use_data*/1);
				FE_region *fe_region = Cmiss_region_get_FE_region(region);
				Cmiss_nodeset_group_id nodeset_group = 0;
				Cmiss_mesh_group_id mesh_group = 0;
				if (group)
				{
					Cmiss_nodeset_id master_nodeset = Cmiss_field_module_find_nodeset_by_name(field_module, "cmiss_nodes");
					Cmiss_field_node_group_id node_group = Cmiss_field_group_get_node_group(group, master_nodeset);
					if (!node_group)
						node_group = Cmiss_field_group_create_node_group(group, master_nodeset);
					nodeset_group = Cmiss_field_node_group_get_nodeset(node_group);
					Cmiss_field_node_group_destroy(&node_group);
					Cmiss_nodeset_destroy(&master_nodeset);

					Cmiss_mesh_id master_mesh = Cmiss_field_module_find_mesh_by_dimension(field_module, 1);
					Cmiss_field_element_group_id element_group = Cmiss_field_group_get_element_group(group, master_mesh);
					if (!element_group)
						element_group = Cmiss_field_group_create_element_group(group, master_mesh);
					mesh_group = Cmiss_field_element_group_get_mesh(element_group);
					Cmiss_field_element_group_destroy(&element_group);
					Cmiss_mesh_destroy(&master_mesh);
				}
				return_code = create_FE_element_snake_from_data_points(
					fe_region, coordinate_field, weight_field,
					number_of_fitting_fields, fitting_fields,
					data_list,
					number_of_elements,
					density_factor,
					stiffness, nodeset_group, mesh_group);
				Cmiss_mesh_group_destroy(&mesh_group);
				Cmiss_nodeset_group_destroy(&nodeset_group);
				Cmiss_field_group_destroy(&selection_group);
				DESTROY(LIST(FE_node))(&data_list);
				Cmiss_field_module_end_change(field_module);
				Cmiss_field_module_destroy(&field_module);
			}
			else
			{
				return_code = 0;
			}
		}
		if (coordinate_field)
		{
			DEACCESS(Computed_field)(&coordinate_field);
		}
		if (weight_field)
		{
			DEACCESS(Computed_field)(&weight_field);
		}
		if (fitting_fields)
		{
			for (i = 0; i < number_of_fitting_fields; i++)
			{
				if (fitting_fields[i])
				{
					DEACCESS(Computed_field)(&fitting_fields[i]);
				}
			}
			DEALLOCATE(fitting_fields);
		}
		if (source_region_path)
		{
			DEALLOCATE(source_region_path);
		}
		Cmiss_field_group_destroy(&group);
		Cmiss_region_destroy(&region);
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_snake.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_snake */

static int gfx_modify_Spectrum(struct Parse_state *state,void *spectrum_void,
	void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 30 April 1999

DESCRIPTION :
Modifier function that parses all the command line options for creating or
modifying a spectrum.
I would put this with the other gfx modify routines but then it can't be
static and referred to by gfx_create_Spectrum
==============================================================================*/
{
	char autorange, blue_to_red, blue_white_red, clear, lg_blue_to_red,
		lg_red_to_blue, overlay_colour, overwrite_colour, red_to_blue;
	const char *current_token;
	int process, range_set, return_code;
	float maximum, minimum;
	struct Cmiss_command_data *command_data;
	struct Modify_spectrum_data modify_spectrum_data;
	struct Option_table *option_table;
	struct Scene *autorange_scene;
	struct Spectrum *spectrum_to_be_modified,*spectrum_to_be_modified_copy;
	struct Spectrum_command_data spectrum_command_data;

	ENTER(gfx_modify_Spectrum);
	if (state)
	{
		if (NULL != (command_data = (struct Cmiss_command_data *)command_data_void))
		{
			if (NULL != (current_token = state->current_token))
			{
				process=0;
				if (NULL != (spectrum_to_be_modified=(struct Spectrum *)spectrum_void))
				{
					if (IS_MANAGED(Spectrum)(spectrum_to_be_modified,
						command_data->spectrum_manager))
					{
						spectrum_to_be_modified_copy=CREATE(Spectrum)(
							"spectrum_modify_temp");
						if (spectrum_to_be_modified_copy)
						{
							MANAGER_COPY_WITHOUT_IDENTIFIER(Spectrum,name)(
								spectrum_to_be_modified_copy,spectrum_to_be_modified);
							process=1;
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"gfx_modify_Spectrum.  Could not create spectrum copy.");
							return_code=0;
						}
					}
					else
					{
						spectrum_to_be_modified_copy=spectrum_to_be_modified;
						spectrum_to_be_modified=(struct Spectrum *)NULL;
						process=1;
					}
				}
				else
				{
					if (strcmp(PARSER_HELP_STRING,current_token)&&
						strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
					{
						if (NULL != (spectrum_to_be_modified=FIND_BY_IDENTIFIER_IN_MANAGER(
							Spectrum,name)(current_token,
							command_data->spectrum_manager)))
						{
							return_code = shift_Parse_state(state, 1);
							if (return_code)
							{
								spectrum_to_be_modified_copy=CREATE(Spectrum)(
									"spectrum_modify_temp");
								if (spectrum_to_be_modified_copy)
								{
									MANAGER_COPY_WITH_IDENTIFIER(Spectrum,name)(
										spectrum_to_be_modified_copy,spectrum_to_be_modified);
									process=1;
								}
								else
								{
									display_message(ERROR_MESSAGE,
									"gfx_modify_Spectrum.  Could not create spectrum copy");
									return_code=0;
								}
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,"Unknown spectrum : %s",
								current_token);
							display_parse_state_location(state);
							return_code=0;
						}
					}
					else
					{
						spectrum_to_be_modified=CREATE(Spectrum)("dummy");
						if (spectrum_to_be_modified)
						{
							option_table=CREATE(Option_table)();
							Option_table_add_entry(option_table,"SPECTRUM_NAME",
								(void *)spectrum_to_be_modified,command_data_void,
								gfx_modify_Spectrum);
							return_code=Option_table_parse(option_table,state);
							DESTROY(Option_table)(&option_table);
							DEACCESS(Spectrum)(&spectrum_to_be_modified);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"gfx_modify_Spectrum.  Could not create dummy spectrum");
							return_code=0;
						}
					}
				}
				if (process)
				{
					autorange = 0;
					autorange_scene = ACCESS(Scene)(command_data->default_scene);
					blue_to_red = 0;
					clear = 0;
					lg_blue_to_red = 0;
					lg_red_to_blue = 0;
					overlay_colour = 0;
					overwrite_colour = 0;
					red_to_blue = 0;
					blue_white_red = 0;
					modify_spectrum_data.position = 0;
					modify_spectrum_data.settings = (struct Spectrum_settings *)NULL;
					modify_spectrum_data.spectrum_minimum = get_Spectrum_minimum(
						spectrum_to_be_modified_copy);
					modify_spectrum_data.spectrum_maximum = get_Spectrum_maximum(
						spectrum_to_be_modified_copy);
					modify_spectrum_data.computed_field_manager
						= Computed_field_package_get_computed_field_manager(
							command_data->computed_field_package);
					spectrum_command_data.spectrum_manager
						= command_data->spectrum_manager;
					option_table=CREATE(Option_table)();
					Option_table_add_entry(option_table,"autorange",&autorange,NULL,
						set_char_flag);
					Option_table_add_entry(option_table,"blue_to_red",&blue_to_red,NULL,
						set_char_flag);
					Option_table_add_entry(option_table,"blue_white_red",&blue_white_red,NULL,
						set_char_flag);
					Option_table_add_entry(option_table,"clear",&clear,NULL,
						set_char_flag);
					Option_table_add_entry(option_table,"field",&modify_spectrum_data,
						&spectrum_command_data,gfx_modify_spectrum_settings_field);
					Option_table_add_entry(option_table,"linear",&modify_spectrum_data,
						&spectrum_command_data,gfx_modify_spectrum_settings_linear);
					Option_table_add_entry(option_table,"log",&modify_spectrum_data,
						&spectrum_command_data,gfx_modify_spectrum_settings_log);
					Option_table_add_entry(option_table,"lg_blue_to_red",&lg_blue_to_red,
						NULL,set_char_flag);
					Option_table_add_entry(option_table,"lg_red_to_blue",&lg_red_to_blue,
						NULL,set_char_flag);
					Option_table_add_entry(option_table,"maximum",&spectrum_to_be_modified_copy,
						NULL,set_Spectrum_maximum_command);
					Option_table_add_entry(option_table,"minimum",&spectrum_to_be_modified_copy,
						NULL,set_Spectrum_minimum_command);
					Option_table_add_entry(option_table,"overlay_colour",&overlay_colour,
						NULL,set_char_flag);
					Option_table_add_entry(option_table,"overwrite_colour",&overwrite_colour,
						NULL,set_char_flag);
					Option_table_add_entry(option_table,"scene_for_autorange",&autorange_scene,
						command_data->scene_manager,set_Scene);
					Option_table_add_entry(option_table,"red_to_blue",&red_to_blue,
						NULL,set_char_flag);
					if (0 != (return_code = Option_table_multi_parse(option_table, state)))
					{
						if (return_code)
						{
							if ( clear )
							{
								Spectrum_remove_all_settings(spectrum_to_be_modified_copy);
							}
							if (blue_to_red + blue_white_red +red_to_blue + lg_red_to_blue +
								lg_blue_to_red > 1 )
							{
								display_message(ERROR_MESSAGE,
									"gfx_modify_Spectrum.  Specify only one simple spectrum type\n "
									"   (blue_to_red, blue_white_red, red_to_blue, lg_red_to_blue, lg_blue_to_red)");
								return_code=0;
							}
							else if (red_to_blue)
							{
								Spectrum_set_simple_type(spectrum_to_be_modified_copy,
									RED_TO_BLUE_SPECTRUM);
							}
							else if (blue_to_red)
							{
								Spectrum_set_simple_type(spectrum_to_be_modified_copy,
									BLUE_TO_RED_SPECTRUM);
							}
							else if (blue_white_red)
							{
								Spectrum_set_simple_type(spectrum_to_be_modified_copy,
									BLUE_WHITE_RED_SPECTRUM);
							}
							else if (lg_red_to_blue)
							{
								Spectrum_set_simple_type(spectrum_to_be_modified_copy,
									LOG_RED_TO_BLUE_SPECTRUM);
							}
							else if (lg_blue_to_red)
							{
								Spectrum_set_simple_type(spectrum_to_be_modified_copy,
									LOG_BLUE_TO_RED_SPECTRUM);
							}
							if ( modify_spectrum_data.settings )
							{
								/* add new settings */
								return_code=Spectrum_add_settings(spectrum_to_be_modified_copy,
									modify_spectrum_data.settings,
									modify_spectrum_data.position);
							}
							if (overlay_colour && overwrite_colour)
							{
								display_message(ERROR_MESSAGE,
									"gfx_modify_Spectrum.  Specify only one colour mode, overwrite_colour or overlay_colour");
								return_code=0;
							}
							else if (overlay_colour)
							{
								Spectrum_set_opaque_colour_flag(spectrum_to_be_modified_copy,
									0);
							}
							else if (overwrite_colour)
							{
								Spectrum_set_opaque_colour_flag(spectrum_to_be_modified_copy,
									1);
							}
							if (autorange)
							{
								/* Could also do all scenes */
								range_set = 0;
								Scene_get_data_range_for_spectrum(autorange_scene,
									spectrum_to_be_modified
									/* Not spectrum_to_be_modified_copy as this ptr
										identifies the valid graphics objects */,
									&minimum, &maximum, &range_set);
								if ( range_set )
								{
									Spectrum_set_minimum_and_maximum(spectrum_to_be_modified_copy,
										minimum, maximum );
								}
							}
							if (spectrum_to_be_modified)
							{
								MANAGER_MODIFY_NOT_IDENTIFIER(Spectrum,name)(
									spectrum_to_be_modified,spectrum_to_be_modified_copy,
									command_data->spectrum_manager);
								DEACCESS(Spectrum)(&spectrum_to_be_modified_copy);
							}
							else
							{
								spectrum_to_be_modified=spectrum_to_be_modified_copy;
							}
						}
						else
						{
							DEACCESS(Spectrum)(&spectrum_to_be_modified_copy);
						}
					}
					if(option_table)
					{
						DESTROY(Option_table)(&option_table);
					}
					if ( modify_spectrum_data.settings )
					{
						DEACCESS(Spectrum_settings)(&(modify_spectrum_data.settings));
					}
					DEACCESS(Scene)(&autorange_scene);
				}
			}
			else
			{
				if (spectrum_void)
				{
					display_message(ERROR_MESSAGE,"Missing spectrum modifications");
				}
				else
				{
					display_message(ERROR_MESSAGE,"Missing spectrum name");
				}
				display_parse_state_location(state);
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_modify_Spectrum.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
				"gfx_modify_Spectrum.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_modify_Spectrum */

static int gfx_create_spectrum(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 14 June 1999

DESCRIPTION :
Executes a GFX CREATE SPECTRUM command.
==============================================================================*/
{
	const char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Spectrum *spectrum;

	ENTER(gfx_create_spectrum);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (NULL != (current_token = state->current_token))
		{
			if (NULL != (command_data = (struct Cmiss_command_data *)command_data_void))
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					if (!FIND_BY_IDENTIFIER_IN_MANAGER(Spectrum,name)(
						current_token,command_data->spectrum_manager))
					{
						spectrum=CREATE(Spectrum)(current_token);
						if (spectrum)
						{
							/*???DB.  Temporary */
							MANAGER_COPY_WITHOUT_IDENTIFIER(Spectrum,name)(spectrum,
								command_data->default_spectrum);
							shift_Parse_state(state,1);
							if (state->current_token)
							{
								return_code=gfx_modify_Spectrum(state,(void *)spectrum,
									command_data_void);
							}
							else
							{
								return_code=1;
							}
							if (return_code)
							{
								ADD_OBJECT_TO_MANAGER(Spectrum)(spectrum,
									command_data->spectrum_manager);
							}
							DEACCESS(Spectrum)(&spectrum);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"gfx_create_spectrum.  Error creating spectrum");
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"Spectrum already exists: %s",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					return_code=gfx_modify_Spectrum(state,(void *)NULL,command_data_void);
					return_code=1;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_spectrum.  Missing command_data_void");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing spectrum name");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_spectrum.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_spectrum */

static int set_Texture_image_from_field(struct Texture *texture,
	struct Computed_field *field,
	struct Computed_field *texture_coordinate_field,
	int propagate_field,
	struct Spectrum *spectrum,
	Cmiss_mesh_id search_mesh,
	enum Texture_storage_type storage,
	int image_width, int image_height, int image_depth,
	int number_of_bytes_per_component,
	struct Graphics_buffer_app_package *graphics_buffer_package,
	struct Graphical_material *fail_material)
/*******************************************************************************
LAST MODIFIED : 30 June 2006

DESCRIPTION :
Creates the image in the format given by sampling the <field> according to the
reverse mapping of the <texture_coordinate_field>.  The values returned by
field are converted to "colours" by applying the <spectrum>.
Currently limited to 1 byte per component.
@param search_mesh  The mesh to find locations with matching texture coordinates.
==============================================================================*/
{
	char *field_name;
	int bytes_per_pixel, number_of_components, return_code,
		source_dimension, *source_sizes, tex_number_of_components, use_pixel_location = 1;
	struct Computed_field *source_texture_coordinate_field = NULL;

	ENTER(set_Texture_image_from_field);
	if (texture && field && spectrum &&
		(4 >= (number_of_components =
			Texture_storage_type_get_number_of_components(storage))))
	{
		/* Setup sizes */
		if (Computed_field_get_native_resolution(
			field, &source_dimension, &source_sizes,
			&source_texture_coordinate_field))
		{
			if (!texture_coordinate_field)
			{
				texture_coordinate_field =
					source_texture_coordinate_field;
			}
			if (image_width == 0)
			{
				if (source_dimension > 0)
				{
					image_width = source_sizes[0];
				}
				else
				{
					image_width = 1;
				}
			}
			if (image_height == 0)
			{
				if (source_dimension > 1)
				{
					image_height = source_sizes[1];
				}
				else
				{
					image_height = 1;
				}
			}
			if (image_depth == 0)
			{
				if (source_dimension > 2)
				{
					image_depth = source_sizes[2];
				}
				else
				{
					image_depth = 1;
				}
			}
			DEALLOCATE(source_sizes);
		}

		if (texture_coordinate_field &&
			(3 >= (tex_number_of_components =
			Computed_field_get_number_of_components(texture_coordinate_field))))
		{
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"set_Texture_image_from_field.  Invalid texture_coordinate field.");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_Texture_image_from_field.  Invalid argument(s)");
		return_code = 0;
	}
	if (return_code)
	{
		if (number_of_bytes_per_component <= 0)
		{
			 number_of_bytes_per_component = 1;
		}
		/* allocate the texture image */
		use_pixel_location = (texture_coordinate_field == source_texture_coordinate_field);
		field_name = (char *)NULL;
		GET_NAME(Computed_field)(field, &field_name);
		if (Texture_allocate_image(texture, image_width, image_height,
			image_depth, storage, number_of_bytes_per_component, field_name))
		{
			bytes_per_pixel = number_of_components*number_of_bytes_per_component;
			Set_cmiss_field_value_to_texture(field, texture_coordinate_field,
				texture, spectrum,	fail_material, image_height, image_width, image_depth,
				bytes_per_pixel, number_of_bytes_per_component, use_pixel_location,
				storage,propagate_field, Graphics_buffer_package_get_core_package(graphics_buffer_package), search_mesh);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"set_Texture_image_from_field.  Could not allocate image in texture");
			return_code = 0;
		}
		DEALLOCATE(field_name);
	}
	LEAVE;

	return (return_code);
} /* set_Texture_image_from_field */

int set_element_dimension_or_all(struct Parse_state *state,
	void *value_address_void, void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 21 June 1999

DESCRIPTION :
Allows either "all" - a return value of zero - or an element dimension up to
MAXIMUM_ELEMENT_XI_DIMENSIONS to be set.
==============================================================================*/
{
	const char *current_token;
	int return_code = 0, value, *value_address;

	ENTER(set_element_dimension_or_all);
	USE_PARAMETER(dummy_user_data);
	if (state)
	{
		if (NULL != (current_token = state->current_token))
		{
			if (strcmp(PARSER_HELP_STRING, current_token) &&
				strcmp(PARSER_RECURSIVE_HELP_STRING, current_token))
			{
				if (NULL != (value_address = (int *)value_address_void))
				{
					if (fuzzy_string_compare_same_length(current_token, "ALL"))
					{
						*value_address = 0;
					}
					else if ((1 == sscanf(current_token, " %d ", &value)) &&
						(0 < value) && (value <= MAXIMUM_ELEMENT_XI_DIMENSIONS))
					{
						*value_address = value;
						return_code = shift_Parse_state(state, 1);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Invalid element dimension: %s\n", current_token);
						display_parse_state_location(state);
						return_code = 0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"set_element_dimension_or_all.  Missing value_address");
					return_code = 0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE, " #|ALL");
				if (NULL != (value_address = (int *)value_address_void))
				{
					if (0 == *value_address)
					{
						display_message(INFORMATION_MESSAGE, "[ALL]");
					}
					else
					{
						display_message(INFORMATION_MESSAGE, "[%d]", *value_address);
					}
				}
				return_code = 1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE, "Missing element dimension or ALL");
			display_parse_state_location(state);
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_element_dimension_or_all.  Missing state");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* set_element_dimension_or_all */

struct Texture_evaluate_image_data
{
	Cmiss_region_id region;
	Cmiss_field_group_id group;
	char *field_name, *texture_coordinates_field_name;
	int element_dimension; /* where 0 is any dimension */
	int propagate_field;
	struct Computed_field *field, *texture_coordinates_field;
	struct Graphical_material *fail_material;
	struct Spectrum *spectrum;
};

static int gfx_modify_Texture_evaluate_image(struct Parse_state *state,
	void *data_void, void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 May 2003

DESCRIPTION :
Modifies the properties of a texture.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Option_table *option_table;
	struct Texture_evaluate_image_data *data;

	ENTER(gfx_modify_Texture_evaluate_image);
	if (state && (command_data = (struct Cmiss_command_data *)command_data_void)
		&& (data = (struct Texture_evaluate_image_data *)data_void))
	{
		if (state->current_token)
		{
			option_table = CREATE(Option_table)();
			/* element_dimension */
			Option_table_add_entry(option_table, "element_dimension",
				&data->element_dimension, NULL, set_element_dimension_or_all);
			/* element_group */
			Option_table_add_region_or_group_entry(option_table, "element_group",
				&data->region, &data->group);
			/* fail_material */
			Option_table_add_set_Material_entry(option_table, "fail_material",
				&data->fail_material, command_data->material_package);
			/* field */
			Option_table_add_entry(option_table, "field", &data->field_name,
				(void *)1, set_name);
			/* propagate_field/no_propagate_field */
			Option_table_add_switch(option_table, "propagate_field",
				"no_propagate_field", &data->propagate_field);
			/* spectrum */
			Option_table_add_entry(option_table, "spectrum", &data->spectrum,
				command_data->spectrum_manager, set_Spectrum);
			/* texture_coordinates */
			Option_table_add_entry(option_table, "texture_coordinates",
				&data->texture_coordinates_field_name, (void *)1, set_name);

			return_code = Option_table_multi_parse(option_table, state);
			DESTROY(Option_table)(&option_table);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_modify_Texture_evaluate_image.  Missing evaluate image options");
			display_parse_state_location(state);
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_modify_Texture_evaluate_image.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_modify_Texture_evaluate_image */

struct Texture_image_data
{
	char *image_file_name;
	int crop_bottom_margin,crop_left_margin,crop_height,crop_width;
};

static int gfx_modify_Texture_image(struct Parse_state *state,
	void *data_void, void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 7 January 2002

DESCRIPTION :
Modifies the properties of a texture.
==============================================================================*/
{
	const char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Option_table *option_table;
	struct Texture_image_data *data;

	ENTER(gfx_modify_Texture_image);
	if (state && (data = (struct Texture_image_data *)data_void) &&
		(command_data = (struct Cmiss_command_data *)command_data_void))
	{
		return_code=1;
		if (NULL != (current_token = state->current_token))
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if (fuzzy_string_compare("crop",current_token))
				{
					if (!(shift_Parse_state(state,1)&&
						(current_token=state->current_token)&&
						(1==sscanf(current_token," %d",&(data->crop_left_margin)))&&
						shift_Parse_state(state,1)&&(current_token=state->current_token)&&
						(1==sscanf(current_token," %d",&(data->crop_bottom_margin)))&&
						shift_Parse_state(state,1)&&(current_token=state->current_token)&&
						(1==sscanf(current_token," %d",&(data->crop_width)))&&
						shift_Parse_state(state,1)&&(current_token=state->current_token)&&
						(1==sscanf(current_token," %d",&(data->crop_height)))&&
						shift_Parse_state(state,1)))
					{
						display_message(WARNING_MESSAGE,"Missing/invalid crop value(s)");
						display_parse_state_location(state);
						return_code=0;
					}
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE,
					" <crop LEFT_MARGIN#[0] BOTTOM_MARGIN#[0] WIDTH#[0] HEIGHT#[0]>");
			}
		}
		if (return_code)
		{
			if (NULL != (current_token = state->current_token))
			{
				option_table = CREATE(Option_table)();
				/* example */
				Option_table_add_entry(option_table, CMGUI_EXAMPLE_DIRECTORY_SYMBOL,
					&(data->image_file_name), &(command_data->example_directory),
					set_file_name);
				/* default */
				Option_table_add_entry(option_table, NULL, &(data->image_file_name),
					NULL, set_file_name);
				return_code = Option_table_parse(option_table, state);
				DESTROY(Option_table)(&option_table);
				//				if (data->image_file_name)
				//{
				// DEALLOCATE(data->image_file_name);
				//}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx modify texture image:  Missing image file name");
				display_parse_state_location(state);
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_modify_Texture_image.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_modify_Texture_image */

struct Texture_file_number_series_data
{
	int start, stop, increment;
};

static int gfx_modify_Texture_file_number_series(struct Parse_state *state,
	void *data_void, void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 8 February 2002

DESCRIPTION :
Modifies the properties of a texture.
==============================================================================*/
{
	const char *current_token;
	int range, return_code;
	struct Texture_file_number_series_data *data;

	ENTER(gfx_modify_Texture_file_number_series);
	USE_PARAMETER(dummy_user_data);
	if (state && (data = (struct Texture_file_number_series_data *)data_void))
	{
		return_code = 1;
		if (NULL != (current_token = state->current_token))
		{
			if (strcmp(PARSER_HELP_STRING, current_token) &&
				strcmp(PARSER_RECURSIVE_HELP_STRING, current_token))
			{
				if ((1 == sscanf(current_token, " %d", &(data->start))) &&
					shift_Parse_state(state, 1) &&
					(current_token = state->current_token) &&
					(1 == sscanf(current_token, " %d", &(data->stop))) &&
					shift_Parse_state(state, 1) &&
					(current_token = state->current_token) &&
					(1 == sscanf(current_token, " %d", &(data->increment))) &&
					shift_Parse_state(state, 1))
				{
					/* check range proceeds from start to stop with a whole number of
						 increments, and that increment is positive */
					if (!(((0 < data->increment) &&
						(0 <= (range = data->stop - data->start)) &&
						(0 == (range % data->increment))) ||
						((0 > data->increment) &&
							(0 <= (range = data->start - data->stop))
							&& (0 == (range % -data->increment)))))
					{
						display_message(ERROR_MESSAGE,
							"Invalid file number series");
						display_parse_state_location(state);
						return_code = 0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Missing 3-D image series START, STOP or INCREMENT");
					display_parse_state_location(state);
					return_code = 0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE, " START STOP INCREMENT");
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_modify_Texture_file_number_series.  Missing state");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_modify_Texture_file_number_series */

int gfx_modify_Texture(struct Parse_state *state,void *texture_void,
	void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 28 February 2003

DESCRIPTION :
Modifies the properties of a texture.
==============================================================================*/
{
	char *file_number_pattern, texture_tiling_enabled;
	const char *current_token, *combine_mode_string, *compression_mode_string, *filter_mode_string,
		*raw_image_storage_string, *resize_filter_mode_string, **valid_strings,
		*wrap_mode_string;
	double texture_distortion[3];
	enum Raw_image_storage raw_image_storage;
	enum Texture_combine_mode combine_mode;
	enum Texture_compression_mode compression_mode;
	enum Texture_filter_mode filter_mode;
	enum Texture_resize_filter_mode resize_filter_mode;
	enum Texture_storage_type specify_format;
	enum Texture_wrap_mode wrap_mode;
	double alpha, depth, distortion_centre_x, distortion_centre_y,
		distortion_factor_k1, height, width, mipmap_level_of_detail_bias;
	float mipmap_level_of_detail_bias_flt;
	int file_number, i, number_of_file_names, number_of_valid_strings, process,
		return_code, specify_depth, specify_height,
		specify_number_of_bytes_per_component, specify_width, texture_is_managed = 0;
	struct Cmgui_image *cmgui_image;
	struct Cmgui_image_information *cmgui_image_information;
	struct Cmiss_command_data *command_data;
	struct Colour colour;
	struct Option_table *option_table;
	struct Texture *texture, *texture_copy;
	struct Texture_image_data image_data;
	struct Texture_file_number_series_data file_number_series_data;
	/* do not make the following static as 'set' flag must start at 0 */
	struct Set_vector_with_help_data texture_distortion_data=
		{3," DISTORTION_CENTRE_X DISTORTION_CENTRE_Y DISTORTION_FACTOR_K1",0};
	struct MANAGER(Computed_field) *field_manager = NULL;
	struct Computed_field *image_field = NULL;
#if defined (SGI_MOVIE_FILE)
	struct Movie_graphics *movie, *old_movie;
	struct X3d_movie *x3d_movie;
#endif /* defined (SGI_MOVIE_FILE) */
	Cmiss_field_image_id image = NULL;

	ENTER(gfx_modify_Texture);
	cmgui_image_information = NULL;
	if (state)
	{
		if (NULL != (current_token = state->current_token))
		{
			if (NULL != (command_data = (struct Cmiss_command_data *)command_data_void))
			{
				process = 0;
				if (NULL != (texture = (struct Texture *)texture_void))
				{
					process = 1;
				}
				else
				{
					if (strcmp(PARSER_HELP_STRING,current_token)&&
						strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
					{
						struct Cmiss_region *region = NULL;
						char *region_path, *field_name;
						if (Cmiss_region_get_partial_region_path(command_data->root_region,
							current_token, &region, &region_path, &field_name))
						{
							if (field_name && (strlen(field_name) > 0) &&
								(strchr(field_name, CMISS_REGION_PATH_SEPARATOR_CHAR)	== NULL))
							{
								field_manager = Cmiss_region_get_Computed_field_manager(region);
								Computed_field *existing_field =
									FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
										field_name, field_manager);
								if (existing_field)
								{
									image = Cmiss_field_cast_image(existing_field);
									if (image)
									{
										texture = Cmiss_field_image_get_texture(image);
										texture_is_managed = 1;
										image_field = Cmiss_field_access(existing_field);
									}
									Cmiss_field_destroy(&existing_field);
								}
							}
							else
							{
								if (field_name)
								{
									display_message(ERROR_MESSAGE,
										"gfx_modify_Texture:  Invalid region path or texture field name '%s'", field_name);
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"gfx_modify_Texture:  Missing texture field name or name matches child region '%s'", current_token);
								}
								display_parse_state_location(state);
								return_code = 0;
							}
							if (region_path)
								DEALLOCATE(region_path);
							if (field_name)
								DEALLOCATE(field_name);
						}
						if (texture)
						{
							process = 1;
							return_code = shift_Parse_state(state, 1);
						}
						else
						{
							display_message(ERROR_MESSAGE, "Unknown texture : %s",
								current_token);
							display_parse_state_location(state);
							return_code = 0;
						}
					}
					else
					{
						if (NULL != (texture = CREATE(Texture)((char *)NULL)))
						{
							option_table = CREATE(Option_table)();
							Option_table_add_entry(option_table, "TEXTURE_NAME",
								(void *)texture, command_data_void, gfx_modify_Texture);
							return_code = Option_table_parse(option_table, state);
							/*???DB.  return_code will be 0 ? */
							DESTROY(Option_table)(&option_table);
							DESTROY(Texture)(&texture);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"gfx_modify_Texture.  Could not create dummy texture");
							return_code = 0;
						}
					}
				}
				if (process)
				{
#if defined (SGI_MOVIE_FILE)
					if (x3d_movie=Texture_get_movie(texture))
					{
						if (movie = FIRST_OBJECT_IN_MANAGER_THAT(Movie_graphics)(
							Movie_graphics_has_X3d_movie, (void *)x3d_movie,
							command_data->movie_graphics_manager))
						{
							ACCESS(Movie_graphics)(movie);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"gfx_modify_Texture.  Missing Movie_graphics for X3d_movie");
						}
					}
					else
					{
						movie = (struct Movie_graphics *)NULL;
					}
					old_movie = movie;
#endif /* defined (SGI_MOVIE_FILE) */
					Texture_get_combine_alpha(texture, &alpha);
					Texture_get_combine_colour(texture, &colour);
					Texture_get_physical_size(texture,
						&width, &height, &depth);
					Texture_get_distortion_info(texture,
						&distortion_centre_x,&distortion_centre_y,&distortion_factor_k1);
					Texture_get_mipmap_level_of_detail_bias(texture, &mipmap_level_of_detail_bias);
					texture_tiling_enabled = Texture_get_texture_tiling_enabled(texture);
					texture_distortion[0]=(double)distortion_centre_x;
					texture_distortion[1]=(double)distortion_centre_y;
					texture_distortion[2]=(double)distortion_factor_k1;

					specify_format=TEXTURE_RGB;
					specify_width=0;
					specify_height=0;
					specify_depth=0;
					specify_number_of_bytes_per_component=0;

					image_data.image_file_name=(char *)NULL;
					image_data.crop_left_margin=0;
					image_data.crop_bottom_margin=0;
					image_data.crop_width=0;
					image_data.crop_height=0;

					Texture_evaluate_image_data evaluate_data;
					evaluate_data.region = Cmiss_region_access(command_data->root_region);
					evaluate_data.group = (Cmiss_field_group_id)0;
					evaluate_data.element_dimension = 0; /* dimension == number of texture coordinates components */
					evaluate_data.propagate_field = 1;
					evaluate_data.field = (struct Computed_field *)NULL;
					evaluate_data.texture_coordinates_field =
						(struct Computed_field *)NULL;
					evaluate_data.field_name = (char *)NULL;
					evaluate_data.texture_coordinates_field_name = (char *)NULL;
					/* Try for the special transparent gray material first */
					if (!(evaluate_data.fail_material =
						FIND_BY_IDENTIFIER_IN_MANAGER(Graphical_material, name)(
						"transparent_gray50", Material_package_get_material_manager(
							command_data->material_package))))
					{
						/* Just use the default material */
						evaluate_data.fail_material = Material_package_get_default_material(
							command_data->material_package);
					}
					ACCESS(Graphical_material)(evaluate_data.fail_material);
					evaluate_data.spectrum = (struct Spectrum *)NULL;

					file_number_pattern = (char *)NULL;
					/* increment must be non-zero for following to be "set" */
					file_number_series_data.start = 0;
					file_number_series_data.stop = 0;
					file_number_series_data.increment = 0;

					option_table = CREATE(Option_table)();
					/* alpha */
					Option_table_add_entry(option_table, "alpha", &alpha,
					  NULL,set_float_0_to_1_inclusive);
					/* blend/decal/modulate */
					combine_mode_string = ENUMERATOR_STRING(Texture_combine_mode)(
						Texture_get_combine_mode(texture));
					valid_strings = ENUMERATOR_GET_VALID_STRINGS(Texture_combine_mode)(
						&number_of_valid_strings,
						(ENUMERATOR_CONDITIONAL_FUNCTION(Texture_combine_mode) *)NULL,
						(void *)NULL);
					Option_table_add_enumerator(option_table, number_of_valid_strings,
						valid_strings, &combine_mode_string);
					DEALLOCATE(valid_strings);
					/* clamp_wrap/repeat_wrap */
					wrap_mode_string = ENUMERATOR_STRING(Texture_wrap_mode)(
						Texture_get_wrap_mode(texture));
					valid_strings = ENUMERATOR_GET_VALID_STRINGS(Texture_wrap_mode)(
						&number_of_valid_strings,
						(ENUMERATOR_CONDITIONAL_FUNCTION(Texture_wrap_mode) *)NULL,
						(void *)NULL);
					Option_table_add_enumerator(option_table,number_of_valid_strings,
						valid_strings,&wrap_mode_string);
					DEALLOCATE(valid_strings);
					/* colour */
					Option_table_add_entry(option_table, "colour", &colour,
					  NULL,set_Colour);
					/* compressed_unspecified/uncompressed */
					compression_mode_string = ENUMERATOR_STRING(Texture_compression_mode)(
						Texture_get_compression_mode(texture));
					valid_strings = ENUMERATOR_GET_VALID_STRINGS(Texture_compression_mode)(
						&number_of_valid_strings,
						(ENUMERATOR_CONDITIONAL_FUNCTION(Texture_compression_mode) *)NULL,
						(void *)NULL);
					Option_table_add_enumerator(option_table, number_of_valid_strings,
						valid_strings, &compression_mode_string);
					DEALLOCATE(valid_strings);
					/* depth */
					Option_table_add_entry(option_table, "depth", &depth,
					  NULL, set_float_non_negative);
					/* distortion */
					Option_table_add_entry(option_table, "distortion",
						&texture_distortion,
					  &texture_distortion_data,set_double_vector_with_help);
					/* height */
					Option_table_add_entry(option_table, "height", &height,
					  NULL,set_float_non_negative);
					/* image */
					Option_table_add_entry(option_table, "image",
						&image_data, command_data, gfx_modify_Texture_image);
					/* linear_filter/nearest_filter */
					filter_mode_string = ENUMERATOR_STRING(Texture_filter_mode)(
						Texture_get_filter_mode(texture));
					valid_strings = ENUMERATOR_GET_VALID_STRINGS(Texture_filter_mode)(
						&number_of_valid_strings,
						(ENUMERATOR_CONDITIONAL_FUNCTION(Texture_filter_mode) *)NULL,
						(void *)NULL);
					Option_table_add_enumerator(option_table,number_of_valid_strings,
						valid_strings,&filter_mode_string);
					DEALLOCATE(valid_strings);
#if defined (SGI_MOVIE_FILE)
					/* movie */
					Option_table_add_entry(option_table, "movie", &movie,
					  command_data->movie_graphics_manager, set_Movie_graphics);
#endif /* defined (SGI_MOVIE_FILE) */
					/* mipmap_level_of_detail_bias */
					mipmap_level_of_detail_bias_flt = mipmap_level_of_detail_bias;
					Option_table_add_float_entry(option_table, "mipmap_level_of_detail_bias",
						&mipmap_level_of_detail_bias_flt);
					/* number_pattern */
					Option_table_add_entry(option_table, "number_pattern",
						&file_number_pattern, (void *)1, set_name);
					/* number_series */
					Option_table_add_entry(option_table, "number_series",
						&file_number_series_data, NULL,
						gfx_modify_Texture_file_number_series);
					/* raw image storage mode */
					raw_image_storage_string =
						ENUMERATOR_STRING(Raw_image_storage)(RAW_PLANAR_RGB);
					valid_strings = ENUMERATOR_GET_VALID_STRINGS(Raw_image_storage)(
						&number_of_valid_strings,
						(ENUMERATOR_CONDITIONAL_FUNCTION(Raw_image_storage) *)NULL,
						(void *)NULL);
					Option_table_add_enumerator(option_table, number_of_valid_strings,
						valid_strings, &raw_image_storage_string);
					DEALLOCATE(valid_strings);
					/* resize_linear_filter/resize_nearest_filter */
					resize_filter_mode_string =
						ENUMERATOR_STRING(Texture_resize_filter_mode)(
							Texture_get_resize_filter_mode(texture));
					valid_strings =
						ENUMERATOR_GET_VALID_STRINGS(Texture_resize_filter_mode)(
							&number_of_valid_strings, (ENUMERATOR_CONDITIONAL_FUNCTION(
								Texture_resize_filter_mode) *)NULL, (void *)NULL);
					Option_table_add_enumerator(option_table,number_of_valid_strings,
						valid_strings,&resize_filter_mode_string);
					DEALLOCATE(valid_strings);
					/* specify_depth */
					Option_table_add_entry(option_table, "specify_depth",&specify_depth,
					  NULL,set_int_non_negative);
					/* specify_format */
					Option_table_add_entry(option_table, "specify_format", &specify_format,
						NULL, set_Texture_storage);
					/* specify_height */
					Option_table_add_entry(option_table, "specify_height",&specify_height,
					  NULL,set_int_non_negative);
					/* specify_number_of_bytes_per_component */
					Option_table_add_entry(option_table,
						"specify_number_of_bytes_per_component",
						&specify_number_of_bytes_per_component,NULL,set_int_non_negative);
					/* specify_width */
					Option_table_add_entry(option_table, "specify_width",&specify_width,
					  NULL,set_int_non_negative);
					/* texture_tiling */
					Option_table_add_char_flag_entry(option_table,
						"texture_tiling", &texture_tiling_enabled);
					/* no_texture_tiling */
					Option_table_add_unset_char_flag_entry(option_table,
						"no_texture_tiling", &texture_tiling_enabled);
					/* width */
					Option_table_add_entry(option_table, "width", &width,
					  NULL,set_float_non_negative);
					/* evaluate_image */
					Option_table_add_entry(option_table, "evaluate_image",
					  &evaluate_data, command_data, gfx_modify_Texture_evaluate_image);
					return_code=Option_table_multi_parse(option_table, state);
					if (return_code)
					{
						if (evaluate_data.field_name || evaluate_data.group ||
							evaluate_data.spectrum || evaluate_data.texture_coordinates_field_name)
						{
							if (evaluate_data.field_name && evaluate_data.texture_coordinates_field_name &&
								evaluate_data.spectrum)
							{
								Cmiss_field_module_id field_module = Cmiss_region_get_field_module(evaluate_data.region);
								evaluate_data.field = Cmiss_field_module_find_field_by_name(field_module,
									evaluate_data.field_name);
								evaluate_data.texture_coordinates_field = Cmiss_field_module_find_field_by_name(field_module,
									evaluate_data.texture_coordinates_field_name);
								Cmiss_field_module_destroy(&field_module);
								if (!evaluate_data.field && !evaluate_data.texture_coordinates_field)
								{
									return_code = 0;
									display_message(ERROR_MESSAGE, "Specified field cannot be found");
								}
							}
							else
							{
								return_code = 0;
							}
							if (!return_code)
							{
								display_message(ERROR_MESSAGE,
									"To evaluate the texture image from a field you must specify\n"
									"a field, element_group (region and optional group), spectrum and texture_coordinates");
								return_code = 0;
							}
						}
					}
					if (return_code)
					{
						if (texture_is_managed)
						{
							MANAGER_BEGIN_CACHE(Computed_field)(field_manager);
							MANAGED_OBJECT_CHANGE(Computed_field)(image_field,
								MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(Computed_field));
						}
						/* must change filter modes etc. before reading new images since
							 some of them will apply immediately to the new images */
						Texture_set_combine_alpha(texture, alpha);
						Texture_set_combine_colour(texture, &colour);
						Texture_set_physical_size(texture, width,
							height, depth);
						Texture_set_texture_tiling_enabled(texture, texture_tiling_enabled);
						Texture_set_mipmap_level_of_detail_bias(texture, mipmap_level_of_detail_bias);

						STRING_TO_ENUMERATOR(Texture_combine_mode)(
							combine_mode_string, &combine_mode);
						Texture_set_combine_mode(texture, combine_mode);

						STRING_TO_ENUMERATOR(Texture_compression_mode)(
							compression_mode_string, &compression_mode);
						Texture_set_compression_mode(texture, compression_mode);

						STRING_TO_ENUMERATOR(Texture_filter_mode)(
							filter_mode_string, &filter_mode);
						Texture_set_filter_mode(texture, filter_mode);

						STRING_TO_ENUMERATOR(Texture_resize_filter_mode)(
							resize_filter_mode_string, &resize_filter_mode);
						Texture_set_resize_filter_mode(texture,
							resize_filter_mode);

						STRING_TO_ENUMERATOR(Texture_wrap_mode)(
							wrap_mode_string, &wrap_mode);
						Texture_set_wrap_mode(texture, wrap_mode);

						if (texture_distortion_data.set)
						{
							distortion_centre_x=(float)texture_distortion[0];
							distortion_centre_y=(float)texture_distortion[1];
							distortion_factor_k1=(float)texture_distortion[2];
							Texture_set_distortion_info(texture,
								distortion_centre_x,distortion_centre_y,distortion_factor_k1);
						}

						if (image_data.image_file_name)
						{
							cmgui_image_information = CREATE(Cmgui_image_information)();
							/* specify file name(s) */
							if (0 != file_number_series_data.increment)
							{
								if (strstr(image_data.image_file_name, file_number_pattern))
								{
									Cmgui_image_information_set_file_name_series(
										cmgui_image_information,
										/*file_name_template*/image_data.image_file_name,
										file_number_pattern,
										file_number_series_data.start,
										file_number_series_data.start,
										/*increment*/1);
								}
								else
								{
									display_message(ERROR_MESSAGE, "gfx modify texture:  "
										"File number pattern \"%s\" not found in file name \"%s\"",
										file_number_pattern, image_data.image_file_name);
									return_code = 0;
								}
							}
							else
							{
								Cmgui_image_information_add_file_name(cmgui_image_information,
									image_data.image_file_name);
							}
							/* specify width and height and raw_image_storage */
							Cmgui_image_information_set_width(cmgui_image_information,
								specify_width);
							Cmgui_image_information_set_height(cmgui_image_information,
								specify_height);
							Cmgui_image_information_set_io_stream_package(cmgui_image_information,
								command_data->io_stream_package);
							STRING_TO_ENUMERATOR(Raw_image_storage)(
								raw_image_storage_string, &raw_image_storage);
							Cmgui_image_information_set_raw_image_storage(
								cmgui_image_information, raw_image_storage);
							switch (specify_format)
							{
								case TEXTURE_LUMINANCE:
								{
									Cmgui_image_information_set_number_of_components(
										cmgui_image_information, 1);
								} break;
								case TEXTURE_LUMINANCE_ALPHA:
								{
									Cmgui_image_information_set_number_of_components(
										cmgui_image_information, 2);
								} break;
								case TEXTURE_RGB:
								case TEXTURE_BGR:
								{
									Cmgui_image_information_set_number_of_components(
										cmgui_image_information, 3);
								} break;
								case TEXTURE_RGBA:
								{
									Cmgui_image_information_set_number_of_components(
										cmgui_image_information, 4);
								} break;
								case TEXTURE_ABGR:
								{
									Cmgui_image_information_set_number_of_components(
										cmgui_image_information, 4);
								} break;
								default:
								{
									display_message(ERROR_MESSAGE,
										"gfx modify texture:  Invalid value for specify_format");
									return_code = 0;
								} break;
							}
							if (specify_number_of_bytes_per_component)
							{
								Cmgui_image_information_set_number_of_bytes_per_component(
									cmgui_image_information, specify_number_of_bytes_per_component);
							}
							if (return_code)
							{
								if (NULL != (cmgui_image = Cmgui_image_read(cmgui_image_information)))
								{
									char *property, *value;

									return_code = Texture_set_image(texture, cmgui_image,
										image_data.image_file_name, file_number_pattern,
										file_number_series_data.start,
										file_number_series_data.stop,
										file_number_series_data.increment,
										image_data.crop_left_margin, image_data.crop_bottom_margin,
										image_data.crop_width, image_data.crop_height);
									/* Delete any existing properties as we are modifying */
									Texture_clear_all_properties(texture);
									/* Calling get_proprety with wildcard ensures they
										will be available to the iterator, as well as
										any other properties */
									Cmgui_image_get_property(cmgui_image,"exif:*");
									Cmgui_image_reset_property_iterator(cmgui_image);
									while ((property = Cmgui_image_get_next_property(
										cmgui_image)) &&
										(value = Cmgui_image_get_property(cmgui_image,
										property)))
									{
										Texture_set_property(texture, property, value);
										DEALLOCATE(property);
										DEALLOCATE(value);
									}
									DESTROY(Cmgui_image)(&cmgui_image);
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"gfx modify texture:  Could not read image file");
									return_code = 0;
								}
								if (return_code && (0 != file_number_series_data.increment))
								{
									number_of_file_names = 1 + (file_number_series_data.stop -
										file_number_series_data.start) /
										file_number_series_data.increment;
									file_number = file_number_series_data.start +
										file_number_series_data.increment;
									for (i = 1 ; return_code && (i < number_of_file_names) ; i++)
									{
										Cmgui_image_information_set_file_name_series(
											cmgui_image_information,
											/*file_name_template*/image_data.image_file_name,
											file_number_pattern, /*start*/file_number,
											/*end*/file_number, /*increment*/1);
										if (NULL != (cmgui_image = Cmgui_image_read(cmgui_image_information)))
										{
											return_code = Texture_add_image(texture, cmgui_image,
												image_data.crop_left_margin, image_data.crop_bottom_margin,
												image_data.crop_width, image_data.crop_height);
											DESTROY(Cmgui_image)(&cmgui_image);
										}
										else
										{
											display_message(ERROR_MESSAGE,
												"gfx modify texture:  Could not read image file");
											return_code = 0;
										}
										file_number += file_number_series_data.increment;
									}
								}
								if (! return_code)
								{
									/* Set a NULL image into texture so that an incomplete set isn't displayed */
									Texture_allocate_image(texture, /*image_width*/1, /*image_height*/1,
										/*image_depth*/1, TEXTURE_RGB, /*number_of_bytes_per_component*/1,
										"INCOMPLETEDTEXTURE");
									display_message(ERROR_MESSAGE,  "gfx modify texture:  "
										"Unable to read images into texture, setting it to black.");
								}
							}
							DESTROY(Cmgui_image_information)(&cmgui_image_information);
						}
#if defined (SGI_MOVIE_FILE)
						if ( movie != old_movie )
						{
							/* Movie is outside manager copy so that is updates
								the correct texture based on movie events */
							Texture_set_movie(texture,
								Movie_graphics_get_X3d_movie(movie),
								command_data->graphics_buffer_package, "movie");
						}
#endif /* defined (SGI_MOVIE_FILE) */

						if (evaluate_data.field && evaluate_data.spectrum &&
							evaluate_data.texture_coordinates_field)
						{
							if (Computed_field_depends_on_texture(evaluate_data.field,
									texture))
							{
								texture_copy = CREATE(Texture)("temporary");
								Texture_copy_without_identifier(texture, texture_copy);
							}
							else
							{
								texture_copy = texture;
							}

							Cmiss_field_module_id field_module = Cmiss_region_get_field_module(evaluate_data.region);
							int element_dimension = evaluate_data.element_dimension;
							if (element_dimension == 0)
								element_dimension = Cmiss_field_get_number_of_components(evaluate_data.texture_coordinates_field);
							const int highest_dimension = FE_region_get_highest_dimension(Cmiss_region_get_FE_region(evaluate_data.region));
							if (element_dimension > highest_dimension)
								element_dimension = highest_dimension;
							Cmiss_mesh_id search_mesh = Cmiss_field_module_find_mesh_by_dimension(field_module, element_dimension);
							if (evaluate_data.group)
							{
								Cmiss_field_element_group_id element_group =
									Cmiss_field_group_get_element_group(evaluate_data.group, search_mesh);
								Cmiss_mesh_destroy(&search_mesh);
								if (element_group)
								{
									search_mesh = Cmiss_mesh_group_base_cast(Cmiss_field_element_group_get_mesh(element_group));
									Cmiss_field_element_group_destroy(&element_group);
								}
							}
							set_Texture_image_from_field(texture_copy,
								evaluate_data.field,
								evaluate_data.texture_coordinates_field,
								evaluate_data.propagate_field,
								evaluate_data.spectrum,
								search_mesh,
								specify_format, specify_width,
								specify_height, specify_depth,
								specify_number_of_bytes_per_component,
								command_data->graphics_buffer_package,
								evaluate_data.fail_material);

							if (texture_copy != texture)
							{
								Texture_copy_without_identifier(texture_copy, texture);
								DESTROY(Texture)(&texture_copy);
							}
							Cmiss_mesh_destroy(&search_mesh);
							Cmiss_field_module_destroy(&field_module);
						}
						if (texture_is_managed)
						{
							if (image && texture)
								Cmiss_field_image_set_texture(image, texture);
							MANAGER_END_CACHE(Computed_field)(field_manager);
						}
					}
					if (image_data.image_file_name)
					{
						DEALLOCATE(image_data.image_file_name);
					}
					DESTROY(Option_table)(&option_table);
#if defined (SGI_MOVIE_FILE)
					if (movie)
					{
						DEACCESS(Movie_graphics)(&movie);
					}
#endif /* defined (SGI_MOVIE_FILE) */
					if (evaluate_data.region)
						Cmiss_region_destroy(&evaluate_data.region);
					if (evaluate_data.group)
						Cmiss_field_group_destroy(&evaluate_data.group);
					if (evaluate_data.fail_material)
					{
						DEACCESS(Graphical_material)(&evaluate_data.fail_material);
					}
					if (evaluate_data.spectrum)
					{
						DEACCESS(Spectrum)(&evaluate_data.spectrum);
					}
					if (evaluate_data.field)
					{
						DEACCESS(Computed_field)(&evaluate_data.field);
					}
					if (evaluate_data.texture_coordinates_field)
					{
						DEACCESS(Computed_field)(&evaluate_data.texture_coordinates_field);
					}
					if (evaluate_data.field_name)
					{
						DEALLOCATE(evaluate_data.field_name);
					}
					if (evaluate_data.texture_coordinates_field_name)
					{
						DEALLOCATE(evaluate_data.texture_coordinates_field_name);
					}

					DEALLOCATE(file_number_pattern);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_modify_Texture.  Missing command_data_void");
				return_code=0;
			}
		}
		else
		{
			if (texture_void)
			{
				display_message(WARNING_MESSAGE,"Missing texture modifications");
			}
			else
			{
				display_message(WARNING_MESSAGE,"Missing texture name");
			}
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_modify_Texture.  Missing state");
		return_code=0;
	}
	if (image_field)
		Cmiss_field_destroy(&image_field);
	LEAVE;

	return (return_code);
} /* gfx_modify_Texture */

static int gfx_create_texture(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/******************************************************************************
LAST MODIFIED : 14 June 1999

DESCRIPTION :
Executes a GFX CREATE TEXTURE command.
==============================================================================*/
{
	const char *current_token;
	int return_code = 0;
	struct Cmiss_command_data *command_data;
	struct Texture *texture;

	ENTER(gfx_create_texture);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (NULL != (current_token = state->current_token))
		{
			if (NULL != (command_data = (struct Cmiss_command_data *)command_data_void))
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					struct Cmiss_region *region = NULL;
					char *region_path, *field_name;
					if (Cmiss_region_get_partial_region_path(command_data->root_region,
						current_token, &region, &region_path, &field_name))
					{
						Cmiss_field_module *field_module = Cmiss_region_get_field_module(region);
						if (field_name && (strlen(field_name) > 0) &&
							(strchr(field_name, CMISS_REGION_PATH_SEPARATOR_CHAR)	== NULL))
						{
							Computed_field *existing_field =
								FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
									field_name, Cmiss_region_get_Computed_field_manager(region));
							if (existing_field)
							{
								display_message(ERROR_MESSAGE,
									"gfx_create_texture: Field already exists");
							}
							else
							{
								texture=CREATE(Texture)(field_name);
								if (texture)
								{
									shift_Parse_state(state,1);
									if (state->current_token)
									{
										return_code=gfx_modify_Texture(state,(void *)texture,
											command_data_void);
									}
									else
									{
										return_code=1;
									}
									if (return_code)
									{
										Cmiss_field_id image_field =	Cmiss_field_module_create_image(
											field_module, NULL, NULL);
										Cmiss_field_set_name(image_field, field_name);
										Cmiss_field_set_attribute_integer(image_field, CMISS_FIELD_ATTRIBUTE_IS_MANAGED, 1);
										Cmiss_field_image_id image = Cmiss_field_cast_image(image_field);
										Cmiss_field_image_set_texture(image, texture);
										Cmiss_field_destroy(&image_field);
										image_field = reinterpret_cast<Cmiss_field_id>(image);
										Cmiss_field_destroy(&image_field);
									}
									else
									{
										DESTROY(Texture)(&texture);
									}
								}
							}
						}
						else
						{
							if (field_name)
							{
								display_message(ERROR_MESSAGE,
									"gfx_create_texture:  Invalid region path or texture field name '%s'", field_name);
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"gfx_create_texture:  Missing texture field name or name matches child region '%s'", current_token);
							}
							display_parse_state_location(state);
							return_code = 0;
						}
						Cmiss_field_module_destroy(&field_module);
						if (region_path)
							DEALLOCATE(region_path);
						if (field_name)
							DEALLOCATE(field_name);
					}
				}
				else
				{
					return_code=gfx_modify_Texture(state,(void *)NULL, command_data_void);
					return_code=1;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_texture.  Missing command_data_void");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing texture name");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_texture.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /*gfx_create_texture */

#if defined (WX_USER_INTERFACE)
static int gfx_create_time_editor(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 15 June 1999

DESCRIPTION :
Executes a GFX CREATE TIME_EDITOR command.
If there is a time editor dialog in existence, then bring it to the front,
otherwise it creates a new one.  Assumes we will only ever want one time
editor at a time.  This implementation may be changed later.
==============================================================================*/
{
	const char *current_token;
	int return_code = 0;
	struct Cmiss_command_data *command_data;

	ENTER(gfx_create_graphical_time_editor);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (NULL != (current_token = state->current_token))
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				display_message(ERROR_MESSAGE,"Unknown option: %s",current_token);
				display_parse_state_location(state);
				return_code=0;
			}
			else
			{
				return_code=1;
			}
		}
		else
		{
			if (NULL != (command_data = (struct Cmiss_command_data *)command_data_void))
			{
				if (command_data->graphics_window_manager)
				{
					 return_code = FOR_EACH_OBJECT_IN_MANAGER(Graphics_window)(
							Graphics_window_bring_up_time_editor_wx,(void *)NULL,
							command_data->graphics_window_manager);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_graphical_time_editor.  Missing command_data");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_create_graphical_time_editor.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_graphical_time_editor */
#endif /* defined (WX_USER_INTERFACE) */

#if defined (USE_CMGUI_GRAPHICS_WINDOW)
static int gfx_create_window(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
Executes a GFX CREATE WINDOW command.
==============================================================================*/
{
	char any_buffering_mode_flag, any_stereo_mode_flag, double_buffer_flag,
		*name,mono_buffer_flag,single_buffer_flag,stereo_buffer_flag;
	enum Graphics_window_buffering_mode buffer_mode;
	enum Graphics_window_stereo_mode stereo_mode;
	int minimum_colour_buffer_depth, minimum_depth_buffer_depth,
		minimum_accumulation_buffer_depth, return_code;
	struct Cmiss_command_data *command_data;
	struct Graphics_window *window;
	struct Option_table *buffer_option_table, *option_table, *stereo_option_table

	ENTER(gfx_create_window);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (NULL != (command_data = (struct Cmiss_command_data *)command_data_void))
		{
			return_code=1;
			/* set_defaults */
			name=Graphics_window_manager_get_new_name(
				command_data->graphics_window_manager);
			buffer_mode = GRAPHICS_WINDOW_DOUBLE_BUFFERING;
			stereo_mode = GRAPHICS_WINDOW_ANY_STEREO_MODE;
			minimum_depth_buffer_depth=8;
			minimum_accumulation_buffer_depth=8;
			minimum_colour_buffer_depth = 8;
			if (state->current_token)
			{
				/* change defaults */
				any_buffering_mode_flag=0;
				single_buffer_flag=0;
				double_buffer_flag=0;
				any_stereo_mode_flag=0;
				mono_buffer_flag=0;
				stereo_buffer_flag=0;

				option_table = CREATE(Option_table)();
				/* accumulation_buffer_depth */
				Option_table_add_entry(option_table, "accumulation_buffer_depth",
					&minimum_accumulation_buffer_depth, NULL, set_int_non_negative);
				/* any_buffer_mode/double_buffer/single_buffer */
				buffer_option_table=CREATE(Option_table)();
				Option_table_add_entry(buffer_option_table,"any_buffer_mode",
					&any_buffering_mode_flag,(void *)NULL,set_char_flag);
				Option_table_add_entry(buffer_option_table,"double_buffer",
					&double_buffer_flag,(void *)NULL,set_char_flag);
				Option_table_add_entry(buffer_option_table,"single_buffer",
					&single_buffer_flag,(void *)NULL,set_char_flag);
				Option_table_add_suboption_table(option_table, buffer_option_table);
				/* any_stereo_mode/mono_buffer/stereo_buffer */
				stereo_option_table=CREATE(Option_table)();
				Option_table_add_entry(stereo_option_table,"any_stereo_mode",
					&any_stereo_mode_flag,(void *)NULL,set_char_flag);
				Option_table_add_entry(stereo_option_table,"mono_buffer",
					&mono_buffer_flag,(void *)NULL,set_char_flag);
				Option_table_add_entry(stereo_option_table,"stereo_buffer",
					&stereo_buffer_flag,(void *)NULL,set_char_flag);
				Option_table_add_suboption_table(option_table, stereo_option_table);
				/* colour_buffer_depth */
				Option_table_add_entry(option_table, "colour_buffer_depth",
					&minimum_colour_buffer_depth, NULL, set_int_non_negative);
				/* depth_buffer_depth */
				Option_table_add_entry(option_table, "depth_buffer_depth",
					&minimum_depth_buffer_depth, NULL, set_int_non_negative);
				/* name */
				Option_table_add_entry(option_table,"name",&name,(void *)1,set_name);
				/* default */
				Option_table_add_entry(option_table,(char *)NULL,&name,(void *)NULL,
					set_name);
				return_code = Option_table_multi_parse(option_table,state);
				DESTROY(Option_table)(&option_table);
				if (return_code)
				{
					if (any_buffering_mode_flag + single_buffer_flag + double_buffer_flag > 1)
					{
						display_message(ERROR_MESSAGE,
							"Only one of any_buffer_mode/single_buffer/double_buffer");
						return_code=0;
					}
					if (any_stereo_mode_flag + mono_buffer_flag + stereo_buffer_flag > 1)
					{
						display_message(ERROR_MESSAGE,
							"Only one of any_stereo_mode/mono_buffer/stereo_buffer");
						return_code=0;
					}
				}
				if (return_code)
				{
					if (any_buffering_mode_flag)
					{
						buffer_mode = GRAPHICS_WINDOW_ANY_BUFFERING_MODE;
					}
					else if (single_buffer_flag)
					{
						buffer_mode = GRAPHICS_WINDOW_SINGLE_BUFFERING;
					}
					else if (double_buffer_flag)
					{
						buffer_mode = GRAPHICS_WINDOW_DOUBLE_BUFFERING;
					}
					if (any_stereo_mode_flag)
					{
						stereo_mode = GRAPHICS_WINDOW_ANY_STEREO_MODE;
					}
					else if (stereo_buffer_flag)
					{
						stereo_mode = GRAPHICS_WINDOW_STEREO;
					}
					else if (mono_buffer_flag)
					{
						stereo_mode = GRAPHICS_WINDOW_MONO;
					}
				}
			}
			if (!name)
			{
				display_message(ERROR_MESSAGE,"gfx_create_window.  Missing name");
				return_code=0;
			}
			if (return_code)
			{
				if (NULL != (window=FIND_BY_IDENTIFIER_IN_MANAGER(Graphics_window,name)(name,
					command_data->graphics_window_manager)))
				{
					display_message(WARNING_MESSAGE,
						"Graphics window '%s' already exists",name);
					return_code=0;
				}
				else
				{
				   if (command_data->user_interface)
					{
#if defined (WX_USER_INTERFACE)
						struct MANAGER(Interactive_tool) *interactive_tool_manager;
						struct Interactive_tool *transform_tool;
						interactive_tool_manager = CREATE(MANAGER(Interactive_tool))();
						transform_tool=create_Interactive_tool_transform(
							 command_data->user_interface);
						ADD_OBJECT_TO_MANAGER(Interactive_tool)(transform_tool,
							 interactive_tool_manager);
						Node_tool_set_execute_command(CREATE(Node_tool)(
								interactive_tool_manager,
								command_data->root_region, /*use_data*/0,
								Material_package_get_default_material(command_data->material_package),
								command_data->user_interface,
								command_data->default_time_keeper),
								command_data->execute_command);
						Node_tool_set_execute_command(CREATE(Node_tool)(
								interactive_tool_manager,
								command_data->root_region, /*use_data*/1,
								Material_package_get_default_material(command_data->material_package),
								command_data->user_interface,
								command_data->default_time_keeper),
								command_data->execute_command);
						Element_tool_set_execute_command(CREATE(Element_tool)(
								interactive_tool_manager,
								command_data->root_region,
								command_data->element_point_ranges_selection,
								Material_package_get_default_material(command_data->material_package),
								command_data->user_interface,
								command_data->default_time_keeper),
								command_data->execute_command);
#if defined (USE_OPENCASCADE)
						Cad_tool_set_execute_command(CREATE(Cad_tool)(
								interactive_tool_manager,
								command_data->root_region,
								command_data->element_point_ranges_selection,
								Material_package_get_default_material(command_data->material_package),
								command_data->user_interface,
								command_data->default_time_keeper),
								command_data->execute_command);
#endif /* defined (USE_OPENCASCADE) */
						Element_point_tool_set_execute_command(CREATE(Element_point_tool)(
								interactive_tool_manager,
								command_data->root_region,
								command_data->element_point_ranges_selection,
								Material_package_get_default_material(command_data->material_package),
								command_data->user_interface,
								command_data->default_time_keeper),
								command_data->execute_command);
						if (NULL != (window=CREATE(Graphics_window)(name,buffer_mode,stereo_mode,
							minimum_colour_buffer_depth, minimum_depth_buffer_depth,
							minimum_accumulation_buffer_depth,
							command_data->graphics_buffer_package,
							&(command_data->background_colour),
							command_data->light_manager,command_data->default_light,
							command_data->light_model_manager,command_data->default_light_model,
							command_data->scene_manager,command_data->default_scene,
							interactive_tool_manager,
							command_data->default_time_keeper,
							command_data->user_interface)))
						{
							if (!ADD_OBJECT_TO_MANAGER(Graphics_window)(window,
							   command_data->graphics_window_manager))
							{
							   DESTROY(Graphics_window)(&window);
								 DESTROY(MANAGER(Interactive_tool))(&interactive_tool_manager);
							   return_code=0;
							}
						}
						else
					   {
						  display_message(ERROR_MESSAGE,
							 "gfx_create_window.  Could not create graphics window");
						  return_code=0;
						}
#else
						window=CREATE(Graphics_window)(name,buffer_mode,stereo_mode,
							minimum_colour_buffer_depth, minimum_depth_buffer_depth,
							minimum_accumulation_buffer_depth,
							command_data->graphics_buffer_package,
							&(command_data->background_colour),
							command_data->light_manager,command_data->default_light,
							command_data->light_model_manager,command_data->default_light_model,
							command_data->scene_manager,command_data->default_scene,
							command_data->interactive_tool_manager,
							command_data->default_time_keeper,
							command_data->user_interface);
					  if (window)
						{
						   if (!ADD_OBJECT_TO_MANAGER(Graphics_window)(window,
							   command_data->graphics_window_manager))
							{
							   DESTROY(Graphics_window)(&window);
							   return_code=0;
							}
						}
						else
					   {
						  display_message(ERROR_MESSAGE,
							 "gfx_create_window.  Could not create graphics window");
						  return_code=0;
						}
#endif /*(WX_USER_INTERFACE)*/
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"gfx_create_window.  Cannot create a graphics window without a display.");
						return_code=0;
					}
				}
			}
			if (name)
			{
				DEALLOCATE(name);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_create_window.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_window.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_window */
#endif /* defined (GTK_USER_INTERFACE)  || defined (WIN32_USER_INTERFACE) || defined (WX_USER_INTERFACE) */

#if defined (SELECT_DESCRIPTORS)
static int execute_command_attach(struct Parse_state *state,
	void *prompt_void,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 11 May 2001

DESCRIPTION :
Executes an ATTACH command.
==============================================================================*/
{
	const char *current_token;
	char end_detection, *perl_action, start_detection;
	int return_code = 0;
	struct Io_device *device;
	static struct Option_table *option_table;
	struct Cmiss_command_data *command_data;

	ENTER(execute_command_attach);
	USE_PARAMETER(prompt_void);
	/* check argument */
	if (state && (command_data=(struct Cmiss_command_data *)command_data_void))
	{
		device = (struct Io_device *)NULL;
		if (NULL != (current_token = state->current_token))
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if (command_data->device_list)
				{
					if (NULL != (device=FIND_BY_IDENTIFIER_IN_LIST(Io_device, name)
						(current_token,command_data->device_list)))
					{
						return_code=shift_Parse_state(state,1);
					}
					else
					{
						if (NULL != (device = CREATE(Io_device)(current_token)))
						{
							if (ADD_OBJECT_TO_LIST(Io_device)(device, command_data->device_list))
							{
								return_code=shift_Parse_state(state,1);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"execute_command_attach.  Unable to create device struture.");
							return_code=0;
						}
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"execute_command_attach.  Missing device list");
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE,
					" DEVICE_NAME");
				return_code = 1;
				/* By not shifting the parse state the rest of the help should come out */
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_attach.  Missing device name");
			return_code=0;
		}
		if (return_code)
		{
			end_detection = 0;
			perl_action = (char *)NULL;
			start_detection = 0;

			option_table = CREATE(Option_table)();
			Option_table_add_entry(option_table,"end_detection", &end_detection,
				NULL, set_char_flag);
			Option_table_add_entry(option_table,"perl_action", &perl_action, (void *)1,
				set_name);
			Option_table_add_entry(option_table,"start_detection", &start_detection,
				NULL, set_char_flag);
			return_code = Option_table_multi_parse(option_table,state);
			DESTROY(Option_table)(&option_table);
			if (return_code)
			{
				if (start_detection && end_detection)
				{
					display_message(ERROR_MESSAGE,"execute_command_attach.  "
						"Specify only one of start_detection and end_detection.");
					return_code=0;
				}
			}
			if (return_code)
			{
				if (start_detection)
				{
					Io_device_start_detection(device, command_data->user_interface);
				}
				if (end_detection)
				{
					Io_device_end_detection(device);
				}
#if defined (USE_PERL_INTERPRETER)
				if (perl_action)
				{
					Io_device_set_perl_action(device, command_data->interpreter,
						perl_action);
				}
#endif /* defined (USE_PERL_INTERPRETER) */
			}
			if (perl_action)
			{
				DEALLOCATE(perl_action);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_attach.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_attach */

static int execute_command_detach(struct Parse_state *state,
	void *prompt_void,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 11 May 2001

DESCRIPTION :
Executes a DETACH command.
==============================================================================*/
{
	const char *current_token;
	int return_code;
	struct Io_device *device;
	struct Cmiss_command_data *command_data;

	ENTER(execute_command_detach);
	USE_PARAMETER(prompt_void);
	/* check argument */
	if (state && (command_data=(struct Cmiss_command_data *)command_data_void))
	{
		device = (struct Io_device *)NULL;
		if (NULL != (current_token = state->current_token))
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if (command_data->device_list)
				{
					if (NULL != (device=FIND_BY_IDENTIFIER_IN_LIST(Io_device, name)
						(current_token,command_data->device_list)))
					{
						if (REMOVE_OBJECT_FROM_LIST(Io_device)(device,
							command_data->device_list))
						{
							if (DESTROY(Io_device)(&device))
							{
								return_code = 1;
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"execute_command_detach.  Unable to destroy device %s.",
									current_token);
								return_code=0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"execute_command_detach.  Unable to remove device %s.",
								current_token);
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"execute_command_detach.  Io_device %s not found.", current_token);
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"execute_command_detach.  Missing device list");
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE,
					" DEVICE_NAME");
				return_code = 1;
				/* By not shifting the parse state the rest of the help should come out */
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_detach.  Missing device name");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_detach.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_detach */
#endif /* defined (SELECT_DESCRIPTORS) */

void export_object_name_parser(const char *path_name, const char **scene_name,
	const char **rendition_name, const char **graphic_name)
{
	const char *slash_pointer, *dot_pointer;
	int total_length, length;
	char *temp_name;
	if (path_name)
	{
		total_length = strlen(path_name);
		slash_pointer = strchr(path_name, '/');
		dot_pointer = strrchr(path_name, '.');
		if (dot_pointer)
		{
			if ((dot_pointer - path_name) < total_length)
			{
				*graphic_name = duplicate_string(dot_pointer + 1);
			}
			total_length = dot_pointer - path_name;
		}
		if (slash_pointer)
		{
			length = total_length - (slash_pointer + 1 - path_name);
			if (length > 1)
			{
				ALLOCATE(temp_name, char, length+1);
				strncpy(temp_name, slash_pointer + 1, length);
				temp_name[length] = '\0';
				*rendition_name = temp_name;
				total_length = slash_pointer - path_name;
			}
		}
		if (total_length > 1)
		{
			ALLOCATE(temp_name, char, total_length+1);
			strncpy(temp_name, path_name, total_length);
			temp_name[total_length] = '\0';
			*scene_name = temp_name;
		}
	}
}

static int gfx_convert_elements(struct Parse_state *state,
	void *dummy_to_be_modified, void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 5 April 2006

DESCRIPTION :
Executes a GFX CONVERT ELEMENTS command.
==============================================================================*/
{
	int i, number_of_fields, previous_state_index, return_code;
	struct Cmiss_command_data *command_data;
	struct Computed_field **fields;
	struct Option_table *option_table;
	char **component_names;

	ENTER(gfx_convert_elements);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (NULL != (command_data = (struct Cmiss_command_data *)command_data_void))
		{
			Cmiss_region *source_region = Cmiss_region_access(command_data->root_region);
			Cmiss_region *destination_region = NULL;
			fields = (struct Computed_field **)NULL;
			component_names = NULL;
			number_of_fields = 1;
			Convert_finite_elements_mode conversion_mode = CONVERT_TO_FINITE_ELEMENTS_MODE_UNSPECIFIED;
			double tolerance = 1.0E-6;
			Element_discretization element_refinement = { 1, 1, 1 };

			if ((state->current_token) &&
				strcmp(PARSER_HELP_STRING,state->current_token) &&
				strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token))
			{
				/* Skip this preprocessing if we are just getting the help */
				number_of_fields = 1;
				previous_state_index = state->current_index;

				option_table = CREATE(Option_table)();
				/* number_of_fields */
				Option_table_add_entry(option_table, "number_of_fields",
					&number_of_fields, NULL, set_int_positive);
				/* absorb everything else */
				Option_table_ignore_all_unmatched_entries(option_table);
				return_code = Option_table_multi_parse(option_table, state);
				DESTROY(Option_table)(&option_table);
				/* Return back to where we were */
				shift_Parse_state(state, previous_state_index - state->current_index);
			}

			if (number_of_fields)
			{
				ALLOCATE(fields, struct Computed_field *, number_of_fields);
				ALLOCATE(component_names, char *, number_of_fields);
				for (i = 0; i < number_of_fields; i++)
				{
					fields[i] = (struct Computed_field *)NULL;
					component_names[i] = NULL;
				}
			}

			option_table=CREATE(Option_table)();
			Option_table_add_help(option_table,
				"Convert element fields to specified bases on a new mesh in destination_region. "
				"Modes \'convert_trilinear\' and \'convert_triquadratic\' ONLY: "
				"converts element fields on 3-D elements to specified basis. "
				"The first field specified must be a 3-component coordinate field; "
				"nodes with values of this field within the specified "
				"tolerance are merged on the resulting mesh. "
				"Note: field value versions of non-coordinate fields are not handled"
				" - the first processed element's versions are assumed. "
				"Mode \'convert_hermite_2D_product_elements\' ONLY: converts element fields on 2-D elements "
				"into bicubic hermite basis WITHOUT merging nearby nodes.");
			Option_table_add_set_Cmiss_region(option_table, "destination_region",
				command_data->root_region, &destination_region);
			Option_table_add_entry(option_table,"fields",component_names,
				&number_of_fields, set_names);
			Option_table_add_entry(option_table, "number_of_fields",
				&number_of_fields, NULL, set_int_positive);
			OPTION_TABLE_ADD_ENUMERATOR(Convert_finite_elements_mode)(option_table,
				&conversion_mode);
			Option_table_add_entry(option_table, "refinement",
				(void *)&element_refinement, (void *)NULL, set_Element_discretization);
			Option_table_add_set_Cmiss_region(option_table, "source_region",
				command_data->root_region, &source_region);
			Option_table_add_non_negative_double_entry(option_table, "tolerance", &tolerance);
			return_code=Option_table_multi_parse(option_table,state);
			DESTROY(Option_table)(&option_table);

			if (return_code)
			{
				if (conversion_mode == CONVERT_TO_FINITE_ELEMENTS_MODE_UNSPECIFIED)
				{
					display_message(ERROR_MESSAGE,
						"gfx_convert elements.  Must specify a conversion mode");
					return_code = 0;
				}
				if ((NULL == component_names) || (NULL == component_names[0]))
				{
					display_message(ERROR_MESSAGE,
						"gfx_convert elements.  Must specify which fields to convert");
					return_code = 0;
				}
				if (NULL == destination_region)
				{
					display_message(ERROR_MESSAGE,
						"gfx_convert elements.  Must specify destination region");
					return_code = 0;
				}
				else if (Cmiss_region_get_Computed_field_manager(destination_region) ==
					Cmiss_region_get_Computed_field_manager(source_region))
				{
					display_message(ERROR_MESSAGE,
						"gfx_convert elements.  Destination and source regions must be different and not share fields");
					return_code = 0;
				}
			}

			if (return_code)
			{
				if (component_names && fields)
				{
					Cmiss_field_module_id field_module = Cmiss_region_get_field_module(source_region);
					for (i = 0; i < number_of_fields; i++)
					{
						if (component_names[i])
						{
							fields[i] = Cmiss_field_module_find_field_by_name(field_module, component_names[i]);
							if (fields[i])
							{
								if (!Computed_field_has_numerical_components(fields[i], NULL))
								{
									return_code = 0;
									display_message(ERROR_MESSAGE,
										"gfx_convert elements.  One of the fields specified does not have numerical components");
									break;
								}
							}
							else
							{
								return_code = 0;
								display_message(ERROR_MESSAGE,
									"gfx_convert elements.  One of the fields specified cannot be found");
								break;
							}
						}
					}
					Cmiss_field_module_destroy(&field_module);
				}
				else
				{
					return_code = 0;
				}
				if (return_code)
				{
					Element_refinement refinement;
					refinement.count[0] = element_refinement.number_in_xi1;
					refinement.count[1] = element_refinement.number_in_xi2;
					refinement.count[2] = element_refinement.number_in_xi3;
					return_code = finite_element_conversion(
						source_region, destination_region, conversion_mode,
						number_of_fields, fields, refinement, tolerance);
				}
			}
			if (fields)
			{
				for (i = 0; i < number_of_fields; i++)
				{
					if (fields[i])
					{
						DEACCESS(Computed_field)(&fields[i]);
					}
				}
				DEALLOCATE(fields);
			}
			if (component_names)
			{
				for (i = 0; i < number_of_fields; i++)
				{
					if (component_names[i])
					{
						DEALLOCATE(component_names[i]);
					}
				}
				DEALLOCATE(component_names);
			}
			Cmiss_region_destroy(&source_region);
			if (destination_region)
			{
				Cmiss_region_destroy(&destination_region);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_convert_elements.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_convert_elements.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_convert_elements */

/***************************************************************************//**
 * Executes a GFX CONVERT GRAPHICS command.
 * Converts graphics to finite elements.
 */
static int gfx_convert_graphics(struct Parse_state *state,
	void *dummy_to_be_modified, void *command_data_void)
{
	int return_code;

	USE_PARAMETER(dummy_to_be_modified);
	Cmiss_command_data *command_data = reinterpret_cast<Cmiss_command_data *>(command_data_void);
	if (state && command_data)
	{
		Cmiss_region_id region = Cmiss_region_access(command_data->root_region);
		Cmiss_field_group_id group = 0;
		struct Cmiss_region *input_region = NULL;
		char *coordinate_field_name = 0;
		Cmiss_field_id coordinate_field = 0;
		const char *scene_name = 0;
		const char *input_region_path = 0;
		const char *graphic_name = NULL;
		Cmiss_scene_id scene = 0;
		char *scene_path_name = 0;
		enum Render_to_finite_elements_mode render_mode = RENDER_TO_FINITE_ELEMENTS_LINEAR_PRODUCT;

		Option_table *option_table=CREATE(Option_table)();
		/* coordinate */
		Option_table_add_string_entry(option_table,"coordinate",&coordinate_field_name,
			" FIELD_NAME");
		/* render_to_finite_elements_mode */
		OPTION_TABLE_ADD_ENUMERATOR(Render_to_finite_elements_mode)(option_table,
			&render_mode);
		/* region */
		Option_table_add_region_or_group_entry(option_table, "region", &region, &group);
		/* scene */
		Option_table_add_string_entry(option_table, "scene",
			&scene_path_name, " SCENE_NAME[/REGION_PATH][.GRAPHIC_NAME]{default}");
		return_code=Option_table_multi_parse(option_table,state);
		DESTROY(Option_table)(&option_table);

		if (return_code)
		{
			if (scene_path_name)
			{
				export_object_name_parser(scene_path_name, &scene_name,
					&input_region_path, &graphic_name);
			}
			if (scene_name)
			{
				REACCESS(Scene)(&(scene),
					FIND_BY_IDENTIFIER_IN_MANAGER(Scene,name)(scene_name,
						command_data->scene_manager));
			}
			if (!scene)
			{
				display_message(ERROR_MESSAGE,
					"gfx_convert.  Must specify a scene.");
				return_code = 0;
			}
			if (coordinate_field_name &&
				(coordinate_field = FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
						coordinate_field_name, Cmiss_region_get_Computed_field_manager(region))))
			{
				if (Computed_field_has_3_components(coordinate_field, NULL))
				{
					Cmiss_field_access(coordinate_field);
				}
				else
				{
					coordinate_field = NULL;
					display_message(ERROR_MESSAGE,
						"gfx_convert_graphics.  "
						"Field specified is not a coordinate field.");
					return_code = 0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_convert_graphics.  "
					"Field %s does not exist.", coordinate_field_name);
			}
			input_region = Cmiss_region_find_subregion_at_path(command_data->root_region,
					input_region_path);
			if (input_region_path && !input_region)
			{
				display_message(ERROR_MESSAGE,
					"gfx_convert.  Invalid input_region");
				return_code = 0;
			}
			if (!coordinate_field)
			{
				display_message(ERROR_MESSAGE,
					"gfx_convert_graphics.  "
					"Must specify a coordinate field to define on the new nodes and elements.");
				return_code = 0;
			}
		}

		if (return_code)
		{
			render_to_finite_elements(scene, input_region, graphic_name, render_mode,
				region, group, coordinate_field);
		}
		if (scene)
		{
			DEACCESS(Scene)(&scene);
		}
		if (coordinate_field)
		{
			DEACCESS(Computed_field)(&coordinate_field);
		}
		if (coordinate_field_name)
		{
			DEALLOCATE(coordinate_field_name);
		}
		if (scene_name)
		{
			DEALLOCATE(scene_name);
		}
		if (input_region_path)
		{
			DEALLOCATE(input_region_path);
		}
		if (graphic_name)
		{
			DEALLOCATE(graphic_name);
		}
		if (input_region)
		{
			Cmiss_region_destroy(&input_region);
		}
		if (scene_path_name)
		{
			DEALLOCATE(scene_path_name);
		}
		Cmiss_field_group_destroy(&group);
		Cmiss_region_destroy(&region);
	}
	else
	{
		display_message(ERROR_MESSAGE, "gfx_convert_graphics.  Invalid argument(s)");
		return_code = 0;
	}
	return (return_code);
}

static int gfx_convert(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 5 April 2006

DESCRIPTION :
Executes a GFX CONVERT command.
==============================================================================*/
{
	int return_code;
	struct Option_table *option_table;

	ENTER(gfx_convert);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && command_data_void)
	{
		option_table = CREATE(Option_table)();
		Option_table_add_entry(option_table,"elements",NULL,
			command_data_void, gfx_convert_elements);
		Option_table_add_entry(option_table,"graphics",NULL,
			command_data_void, gfx_convert_graphics);
		return_code = Option_table_parse(option_table,state);
		DESTROY(Option_table)(&option_table);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_convert.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_convert */

static int execute_command_gfx_create(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 4 May 2004

DESCRIPTION :
Executes a GFX CREATE command.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Create_emoter_slider_data create_emoter_slider_data;
	struct Option_table *option_table;

	ENTER(execute_command_gfx_create);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (NULL != (command_data = (struct Cmiss_command_data *)command_data_void))
		{
			if (state->current_token)
			{
				option_table=CREATE(Option_table)();
				Option_table_add_entry(option_table,"axes",NULL,
					command_data_void,gfx_create_axes);
				Option_table_add_entry(option_table,"colour_bar",NULL,
					command_data_void,gfx_create_colour_bar);
#if defined (WX_USER_INTERFACE)
				Option_table_add_entry(option_table,"data_viewer",NULL,
					command_data_void,gfx_create_data_viewer);
#endif /* defined (WX_USER_INTERFACE) */
				Option_table_add_entry(option_table, "dgroup", /*use_object_type*/(void *)2,
					(void *)command_data->root_region, gfx_create_group);
				Option_table_add_entry(option_table, "egroup", /*use_object_type*/(void *)0,
					(void *)command_data->root_region, gfx_create_group);
#if defined (WX_USER_INTERFACE)
				Option_table_add_entry(option_table,"element_creator",NULL,
					command_data_void,gfx_create_element_creator);
#endif /* defined (WX_USER_INTERFACE) */
#if defined (WX_USER_INTERFACE)
				Option_table_add_entry(option_table,"element_point_viewer",NULL,
					command_data_void,gfx_create_element_point_viewer);
#endif /* defined (WX_USER_INTERFACE) */
				create_emoter_slider_data.execute_command=command_data->execute_command;
				create_emoter_slider_data.root_region=
					command_data->root_region;
				create_emoter_slider_data.basis_manager=
					command_data->basis_manager;
#if defined (USE_CMGUI_GRAPHICS_WINDOW)
				create_emoter_slider_data.graphics_window_manager=
					command_data->graphics_window_manager;
#endif /* defined (USE_CMGUI_GRAPHICS_WINDOW) */
				create_emoter_slider_data.graphics_buffer_package=
					command_data->graphics_buffer_package;
				create_emoter_slider_data.curve_manager=
					command_data->curve_manager;
				create_emoter_slider_data.scene_manager=command_data->scene_manager;
				create_emoter_slider_data.io_stream_package =
					command_data->io_stream_package;
				create_emoter_slider_data.viewer_scene=command_data->default_scene;
				create_emoter_slider_data.viewer_background_colour=
					command_data->background_colour;
				create_emoter_slider_data.viewer_light=command_data->default_light;
				create_emoter_slider_data.viewer_light_model=
					command_data->default_light_model;
				create_emoter_slider_data.emoter_dialog_address=
					&(command_data->emoter_slider_dialog);
				create_emoter_slider_data.user_interface=
					command_data->user_interface;
				Option_table_add_entry(option_table,"emoter",NULL,
					(void *)&create_emoter_slider_data,gfx_create_emoter);
				Option_table_add_entry(option_table, "flow_particles",
					/*create_more*/(void *)0, command_data_void, gfx_create_flow_particles);
#if defined (WX_USER_INTERFACE)
				Option_table_add_entry(option_table,"graphical_material_editor",NULL,
					command_data_void,gfx_create_graphical_material_editor);
#endif /* defined (WX_USER_INTERFACE) */
				Option_table_add_entry(option_table, "gauss_points", NULL,
					(void *)command_data->root_region, gfx_create_gauss_points);
				Option_table_add_entry(option_table,"light",NULL,
					command_data_void,gfx_create_light);
				Option_table_add_entry(option_table,"lmodel",NULL,
					command_data_void,gfx_create_light_model);
				Option_table_add_entry(option_table,"material",NULL,
					(void *)command_data->material_package,gfx_create_material);
				Option_table_add_entry(option_table, "more_flow_particles",
					/*create_more*/(void *)1, command_data_void, gfx_create_flow_particles);
				Option_table_add_entry(option_table, "ngroup", /*use_object_type*/(void *)1,
					(void *)command_data->root_region, gfx_create_group);
#if defined (WX_USER_INTERFACE)
				Option_table_add_entry(option_table,"node_viewer",NULL,
					command_data_void,gfx_create_node_viewer);
#endif /* defined (WX_USER_INTERFACE) */
				Option_table_add_entry(option_table, "region", NULL,
					(void *)command_data->root_region, gfx_create_region);
				Option_table_add_entry(option_table, "snake", NULL,
					command_data_void, gfx_create_snake);
				Option_table_add_entry(option_table,"spectrum",NULL,
					command_data_void,gfx_create_spectrum);
				Option_table_add_entry(option_table,"texture",NULL,
					command_data_void,gfx_create_texture);
#if defined (WX_USER_INTERFACE)
				Option_table_add_entry(option_table,"time_editor",NULL,
					command_data_void,gfx_create_time_editor);
#endif /* defined (WX_USER_INTERFACE) */
#if defined (USE_CMGUI_GRAPHICS_WINDOW)
				Option_table_add_entry(option_table,"window",NULL,
					command_data_void,gfx_create_window);
#endif /* defined (USE_CMGUI_GRAPHICS_WINDOW) */
				return_code=Option_table_parse(option_table,state);
				DESTROY(Option_table)(&option_table);
			}
			else
			{
				set_command_prompt("gfx create",command_data);
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_gfx_create.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_gfx_create.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_create */

/***************************************************************************//**
 * Executes a GFX DEFINE FACES command.
 */
static int gfx_define_faces(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
{
	int return_code;
	USE_PARAMETER(dummy_to_be_modified);
	Cmiss_command_data *command_data = reinterpret_cast<Cmiss_command_data *>(command_data_void);
	if (state && command_data)
	{
		Cmiss_region_id region = Cmiss_region_access(command_data->root_region);
		Cmiss_field_group_id group = 0;
		Option_table *option_table = CREATE(Option_table)();
		Option_table_add_region_or_group_entry(option_table, "egroup", &region, &group);
		return_code = Option_table_multi_parse(option_table, state);
		DESTROY(Option_table)(&option_table);
		if (return_code)
		{
			Cmiss_field_module_id field_module = Cmiss_region_get_field_module(region);
			Cmiss_field_module_begin_change(field_module);
			FE_region *fe_region = Cmiss_region_get_FE_region(region);
			for (int dimension = MAXIMUM_ELEMENT_XI_DIMENSIONS; (1 < dimension) && return_code; --dimension)
			{
				Cmiss_mesh_id mesh = Cmiss_field_module_find_mesh_by_dimension(field_module, dimension);
				Cmiss_mesh_group_id face_mesh_group = 0;
				if (group)
				{
					Cmiss_field_element_group_id element_group = Cmiss_field_group_get_element_group(group, mesh);
					Cmiss_mesh_destroy(&mesh);
					mesh = Cmiss_mesh_group_base_cast(Cmiss_field_element_group_get_mesh(element_group));
					Cmiss_field_element_group_destroy(&element_group);
				}
				if (mesh && (0 < Cmiss_mesh_get_size(mesh)))
				{
					if (group)
					{
						Cmiss_mesh_id face_master_mesh = Cmiss_field_module_find_mesh_by_dimension(field_module, dimension - 1);
						Cmiss_field_element_group_id face_element_group = Cmiss_field_group_get_element_group(group, face_master_mesh);
						if (!face_element_group)
							face_element_group = Cmiss_field_group_create_element_group(group, face_master_mesh);
						Cmiss_mesh_destroy(&face_master_mesh);
						face_mesh_group = Cmiss_field_element_group_get_mesh(face_element_group);
						Cmiss_field_element_group_destroy(&face_element_group);

					}
					FE_region_begin_define_faces(fe_region, dimension - 1);
					Cmiss_element_iterator_id iter = Cmiss_mesh_create_element_iterator(mesh);
					Cmiss_element_id element = 0;
					while ((0 != (element = Cmiss_element_iterator_next_non_access(iter))) && return_code)
					{
						if (!FE_region_merge_FE_element_and_faces_and_nodes(fe_region, element))
						{
							return_code = 0;
						}
						if (face_mesh_group)
						{
							int number_of_faces = 0;
							get_FE_element_number_of_faces(element, &number_of_faces);
							Cmiss_element_id face = 0;
							for (int face_number = 0; face_number < number_of_faces; ++face_number)
							{
								if (get_FE_element_face(element, face_number, &face) && face)
								{
									if (!Cmiss_mesh_contains_element(Cmiss_mesh_group_base_cast(face_mesh_group), face) &&
										!Cmiss_mesh_group_add_element(face_mesh_group, face))
									{
										return_code = 0;
										break;
									}
								}
							}
						}
					}
					Cmiss_element_iterator_destroy(&iter);
					FE_region_end_define_faces(fe_region);
				}
				Cmiss_mesh_group_destroy(&face_mesh_group);
				Cmiss_mesh_destroy(&mesh);
			}
			Cmiss_field_module_end_change(field_module);
			Cmiss_field_module_destroy(&field_module);
		}
		Cmiss_field_group_destroy(&group);
		Cmiss_region_destroy(&region);
	}
	else
	{
		display_message(ERROR_MESSAGE, "gfx_define_faces.  Invalid argument(s)");
		return_code = 0;
	}
	return (return_code);
}


static int execute_command_gfx_define(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Executes a GFX DEFINE command.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Curve_command_data curve_command_data;
	struct Option_table *option_table;

	ENTER(execute_command_gfx_define);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (NULL != (command_data = (struct Cmiss_command_data *)command_data_void))
		{
			if (state->current_token)
			{
				option_table = CREATE(Option_table)();
				/* curve */
				curve_command_data.curve_manager =
					command_data->curve_manager;
				curve_command_data.io_stream_package =
					command_data->io_stream_package;
				Option_table_add_entry(option_table, "curve", NULL,
					&curve_command_data, gfx_define_Curve);
				/* faces */
				Option_table_add_entry(option_table, "faces", NULL,
					command_data_void, gfx_define_faces);
				/* field */
				Option_table_add_entry(option_table, "field", command_data->root_region,
					command_data->computed_field_package, define_Computed_field);
				/* font */
				Option_table_add_entry(option_table, "font", NULL,
					command_data->graphics_module, gfx_define_font);
				/* scene */
				Define_scene_data define_scene_data;
				define_scene_data.light_manager=command_data->light_manager;
				define_scene_data.scene_manager=command_data->scene_manager;
				define_scene_data.root_region = command_data->root_region;
				define_scene_data.graphics_module = command_data->graphics_module;
				Option_table_add_entry(option_table, "scene", NULL,
					(void *)(&define_scene_data), define_Scene);
				/* graphics_filter */
				Option_table_add_entry(option_table, "graphics_filter", command_data->root_region,
					command_data->graphics_module, gfx_define_graphics_filter);
				/* tessellation */
				Option_table_add_entry(option_table, "tessellation", NULL,
					command_data->graphics_module, gfx_define_tessellation);
				return_code = Option_table_parse(option_table, state);
				DESTROY(Option_table)(&option_table);
			}
			else
			{
				set_command_prompt("gfx define",command_data);
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_gfx_define.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_gfx_define.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_define */

/***************************************************************************//**
 * Destroys a group.
 */
static int gfx_destroy_group(struct Parse_state *state,
	void *dummy_to_be_modified, void *root_region_void)
{
	int return_code = 1;
	USE_PARAMETER(dummy_to_be_modified);
	Cmiss_region_id root_region = reinterpret_cast<Cmiss_region_id>(root_region_void);
	if (state && root_region)
	{
		Cmiss_region_id region = Cmiss_region_access(root_region);
		Cmiss_field_group_id group = 0;
		const char *current_token = state->current_token;
		if (!set_Cmiss_region_or_group(state, &region, &group) ||
			Parse_state_help_mode(state))
		{
			// message already output
		}
		else if (!group)
		{
			display_message(ERROR_MESSAGE, "Not a group: %s", current_token);
			return_code = 0;
		}
		else
		{
			if (!MANAGED_OBJECT_NOT_IN_USE(Computed_field)(Cmiss_field_group_base_cast(group),
				Cmiss_region_get_Computed_field_manager(region)))
			{
				display_message(INFORMATION_MESSAGE, "Group %s marked for destruction when no longer in use.\n",
					current_token);
			}
			return_code = Cmiss_field_set_attribute_integer(
				Cmiss_field_group_base_cast(group), CMISS_FIELD_ATTRIBUTE_IS_MANAGED, 0);
		}
		Cmiss_field_group_destroy(&group);
		Cmiss_region_destroy(&region);
	}
	else
	{
		display_message(ERROR_MESSAGE, "gfx_destroy_group.  Invalid argument(s)");
		return_code = 0;
	}
	return return_code;
}

static int gfx_destroy_region(struct Parse_state *state,
	void *dummy_to_be_modified, void *root_region_void)
/*******************************************************************************
LAST MODIFIED : 27 March 2003

DESCRIPTION :
Executes a GFX DESTROY REGION command.
==============================================================================*/
{
	const char *current_token;
	int return_code = 1;
	struct Cmiss_region *root_region;

	ENTER(gfx_destroy_region);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (root_region = (struct Cmiss_region *)root_region_void))
	{
		if (NULL != (current_token = state->current_token))
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				struct Cmiss_region *region =
					Cmiss_region_find_subregion_at_path(root_region, current_token);
				if (region)
				{
					struct Cmiss_region *parent_region = Cmiss_region_get_parent(region);
					if (parent_region)
					{
						Cmiss_region_remove_child(parent_region, region);
						Cmiss_region_destroy(&parent_region);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"gfx destroy region:  The root region may not be removed");
						display_parse_state_location(state);
						return_code = 0;
					}
					Cmiss_region_destroy(&region);
				}
				else
				{
					display_message(ERROR_MESSAGE, "Unknown region: %s", current_token);
					display_parse_state_location(state);
					return_code = 0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE, " REGION_PATH");
				return_code = 1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE, "Missing region path");
			display_parse_state_location(state);
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "gfx_destroy_region.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_destroy_region */

/***************************************************************************//**
 * Executes a GFX DESTROY ELEMENTS command.
 */
static int gfx_destroy_elements(struct Parse_state *state,
	void *dimension_void, void *command_data_void)
{
	int return_code;
	int dimension = VOIDPTR2INT(dimension_void);
	Cmiss_command_data *command_data = reinterpret_cast<Cmiss_command_data *>(command_data_void);
	if (state && command_data && (0 < dimension) && (dimension <= 3))
	{
		/* initialise defaults */
		char all_flag = 0;
		Cmiss_region_id region = Cmiss_region_access(command_data->root_region);
		Cmiss_field_group_id group = 0;
		Cmiss_field_id conditional_field = 0;
		char *conditional_field_name = 0;
		char selected_flag = 0;
		Multi_range *element_ranges = CREATE(Multi_range)();
		FE_value time;
		if (command_data->default_time_keeper)
		{
			time = Time_keeper_get_time(command_data->default_time_keeper);
		}
		else
		{
			time = 0.0;
		}

		Option_table *option_table = CREATE(Option_table)();
		/* all */
		Option_table_add_char_flag_entry(option_table, "all", &all_flag);
		/* conditional_field */
		Option_table_add_string_entry(option_table,"conditional_field",
			&conditional_field_name, " FIELD_NAME");
		/* group */
		Option_table_add_region_or_group_entry(option_table, "group", &region, &group);
		/* selected */
		Option_table_add_char_flag_entry(option_table, "selected", &selected_flag);
		/* default option: element number ranges */
		Option_table_add_entry(option_table, (char *)NULL, (void *)element_ranges,
			NULL, set_Multi_range);
		return_code = Option_table_multi_parse(option_table, state);
		DESTROY(Option_table)(&option_table);

		if (return_code)
		{
			if ((0 == Multi_range_get_number_of_ranges(element_ranges)) && (!selected_flag) &&
				(0 == conditional_field_name) && (!all_flag))
			{
				display_message(ERROR_MESSAGE, "gfx destroy elements:  No elements specified.");
				return_code = 0;
			}
		}
		if (return_code && conditional_field_name)
		{
			Cmiss_field_module_id field_module = Cmiss_region_get_field_module(region);
			conditional_field = Cmiss_field_module_find_field_by_name(field_module, conditional_field_name);
			if (!conditional_field)
			{
				display_message(ERROR_MESSAGE,
					"gfx_destroy_elements:  conditional field cannot be found");
				return_code = 0;
			}
			Cmiss_field_module_destroy(&field_module);
		}
		if (return_code)
		{
			Cmiss_field_module_id field_module = Cmiss_region_get_field_module(region);
			Cmiss_field_group_id selection_group = NULL;
			if (selected_flag)
			{
				Cmiss_rendition *rendition = Cmiss_region_get_rendition_internal(region);
				if (rendition)
				{
					selection_group = Cmiss_rendition_get_selection_group(rendition);
					Cmiss_rendition_destroy(&rendition);
				}
			}
			int use_dimension = dimension;
			if (dimension == 3)
			{
				use_dimension = FE_region_get_highest_dimension(Cmiss_region_get_FE_region(region));
			}
			Cmiss_field_id use_conditional_field = 0;
			if (group && conditional_field)
			{
				use_conditional_field = Cmiss_field_module_create_or(field_module, conditional_field, Cmiss_field_group_base_cast(group));
			}
			else if (conditional_field)
			{
				use_conditional_field = Cmiss_field_access(conditional_field);
			}
			else if (group)
			{
				use_conditional_field = Cmiss_field_access(Cmiss_field_group_base_cast(group));
			}
			LIST(FE_element) *destroy_element_list = 0;
			if ((!selected_flag) || selection_group)
			{
				destroy_element_list = FE_element_list_from_region_and_selection_group(
					region, use_dimension, element_ranges, Cmiss_field_group_base_cast(selection_group), use_conditional_field,
					time);
				if (!destroy_element_list)
					return_code = 0;
			}
			if (destroy_element_list && (0 < NUMBER_IN_LIST(FE_element)(destroy_element_list)))
			{
				Cmiss_field_module_begin_change(field_module);
				return_code = FE_region_remove_FE_element_list(
					Cmiss_region_get_FE_region(region), destroy_element_list);
				int number_not_destroyed = 0;
				if (0 < (number_not_destroyed = NUMBER_IN_LIST(FE_element)(
					destroy_element_list)))
				{
					display_message(WARNING_MESSAGE,
						"%d of the selected element(s) could not be destroyed",
						number_not_destroyed);
				}
				if (selection_group)
				{
					Cmiss_field_group_remove_empty_subgroups(selection_group);
				}
				Cmiss_field_module_end_change(field_module);
			}
			else
			{
				if (dimension == 1)
				{
					display_message(INFORMATION_MESSAGE, "gfx destroy lines:  No lines specified\n");
				}
				else if (dimension == 2)
				{
					display_message(INFORMATION_MESSAGE, "gfx destroy faces:  No faces specified\n");
				}
				else
				{
					display_message(INFORMATION_MESSAGE, "gfx destroy elements:  No elements specified\n");
				}
			}
			if (destroy_element_list)
				DESTROY(LIST(FE_element))(&destroy_element_list);
			if (selection_group)
				Cmiss_field_group_destroy(&selection_group);
			if (use_conditional_field)
				Cmiss_field_destroy(&use_conditional_field);
			Cmiss_field_module_destroy(&field_module);
		}
		if (conditional_field)
		{
			DEACCESS(Computed_field)(&conditional_field);
		}
		if (conditional_field_name)
		{
			DEALLOCATE(conditional_field_name);
		}
		DESTROY(Multi_range)(&element_ranges);
		Cmiss_field_group_destroy(&group);
		Cmiss_region_destroy(&region);
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_destroy_elements.  Invalid argument(s)");
		return_code = 0;
	}
	return (return_code);
}

static int gfx_destroy_Computed_field(struct Parse_state *state,
	void *dummy_to_be_modified,void *root_region_void)
/*******************************************************************************
LAST MODIFIED : 24 July 2008

DESCRIPTION :
Executes a GFX DESTROY FIELD command.
==============================================================================*/
{
	const char *current_token;
	char *field_name, *region_path;
	struct Computed_field *field;
	int return_code = 0;
	struct Cmiss_region *region, *root_region;

	ENTER(gfx_destroy_Computed_field);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (root_region=(struct Cmiss_region *)root_region_void))
	{
		if (NULL != (current_token = state->current_token))
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if (Cmiss_region_get_partial_region_path(root_region,
					current_token, &region, &region_path, &field_name))
				{
					if (field_name &&
						(field = FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
							field_name, Cmiss_region_get_Computed_field_manager(region))))
					{
						ACCESS(Computed_field)(field);
						if (!MANAGED_OBJECT_NOT_IN_USE(Computed_field)(field,
							Cmiss_region_get_Computed_field_manager(region)))
						{
							display_message(INFORMATION_MESSAGE, "Field %s marked for destruction when no longer in use.\n",
								current_token);
						}
						return_code = Cmiss_field_set_attribute_integer(field, CMISS_FIELD_ATTRIBUTE_IS_MANAGED, 0);
						if (!return_code)
						{
							display_message(ERROR_MESSAGE, "gfx destroy field.  Failed");
						}
						DEACCESS(Computed_field)(&field);
					}
					else
					{
						display_message(ERROR_MESSAGE,"Field does not exist: %s",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
					DEALLOCATE(region_path);
					DEALLOCATE(field_name);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"gfx_destroy_Computed_field.  Failed to get region_path/field_name");
					return_code = 0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," [REGION_PATH/]FIELD_NAME");
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing region_path/field name");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_destroy_Computed_field.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_destroy_Computed_field */

static int gfx_destroy_graphics_object(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 20 November 1998

DESCRIPTION :
Executes a GFX DESTROY GRAPHICS_OBJECT command.
==============================================================================*/
{
	const char *current_token;
	gtObject *graphics_object;
	int return_code;
	struct Cmiss_command_data *command_data;

	ENTER(gfx_destroy_graphics_object);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (NULL != (command_data = (struct Cmiss_command_data *)command_data_void))
		{
			if (NULL != (current_token = state->current_token))
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					if (NULL != (graphics_object=FIND_BY_IDENTIFIER_IN_MANAGER(GT_object,name)(
						current_token,command_data->glyph_manager)))
					{
						REMOVE_OBJECT_FROM_MANAGER(GT_object)(graphics_object,
							command_data->glyph_manager);
						return_code=1;
					}
					else
					{
						display_message(ERROR_MESSAGE,"Graphics object does not exist: %s",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(INFORMATION_MESSAGE," GRAPHICS_OBJECT_NAME");
					return_code=1;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"Missing graphics object name");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_destroy_graphics_object.  Missing glyph_manager");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_destroy_graphics_object.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_destroy_graphics_object */

static int gfx_destroy_material(struct Parse_state *state,
	void *dummy_to_be_modified, void *graphical_material_manager_void)
/*******************************************************************************
LAST MODIFIED : 8 May 2002

DESCRIPTION :
Executes a GFX DESTROY MATERIAL command.
==============================================================================*/
{
	const char *current_token;
	struct Graphical_material *graphical_material;
	int return_code;
	struct MANAGER(Graphical_material) *graphical_material_manager;

	ENTER(gfx_destroy_material);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (graphical_material_manager =
		(struct MANAGER(Graphical_material) *)graphical_material_manager_void))
	{
		if (NULL != (current_token = state->current_token))
		{
			if (strcmp(PARSER_HELP_STRING, current_token) &&
				strcmp(PARSER_RECURSIVE_HELP_STRING, current_token))
			{
				if (NULL != (graphical_material =
					FIND_BY_IDENTIFIER_IN_MANAGER(Graphical_material, name)(
						current_token, graphical_material_manager)))
				{
					if (REMOVE_OBJECT_FROM_MANAGER(Graphical_material)(graphical_material,
						graphical_material_manager))
					{
						return_code = 1;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Could not remove material %s from manager", current_token);
						return_code = 0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Unknown material: %s", current_token);
					return_code = 0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE, " MATERIAL_NAME");
				return_code = 1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE, "Missing material name");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_destroy_material.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_destroy_material */

/***************************************************************************//**
 * Executes a GFX DESTROY NODES/DATA command.
 * If <use_data_flag> is set, work with cmiss_data, otherwise cmiss_nodes.
 */
static int gfx_destroy_nodes(struct Parse_state *state,
	void *use_data, void *command_data_void)
{
	int return_code;
	Cmiss_command_data *command_data = reinterpret_cast<Cmiss_command_data *>(command_data_void);
	if (state && command_data)
	{
		/* initialise defaults */
		char all_flag = 0;
		Cmiss_region_id region = Cmiss_region_access(command_data->root_region);
		Cmiss_field_group_id group = 0;
		Cmiss_field_id conditional_field = 0;
		char *conditional_field_name = 0;
		char selected_flag = 0;
		Multi_range *node_ranges = CREATE(Multi_range)();
		FE_value time;
		if (command_data->default_time_keeper)
		{
			time = Time_keeper_get_time(command_data->default_time_keeper);
		}
		else
		{
			time = 0.0;
		}

		Option_table *option_table = CREATE(Option_table)();
		/* all */
		Option_table_add_char_flag_entry(option_table, "all", &all_flag);
		/* conditional_field */
		Option_table_add_string_entry(option_table,"conditional_field",
			&conditional_field_name, " FIELD_NAME");
		/* group */
		Option_table_add_region_or_group_entry(option_table, "group", &region, &group);
		/* selected */
		Option_table_add_char_flag_entry(option_table, "selected", &selected_flag);
		/* default option: node number ranges */
		Option_table_add_entry(option_table, (char *)NULL, (void *)node_ranges,
			NULL, set_Multi_range);
		return_code = Option_table_multi_parse(option_table, state);
		DESTROY(Option_table)(&option_table);

		if (return_code)
		{
			if ((0 == Multi_range_get_number_of_ranges(node_ranges)) && (!selected_flag) &&
				(0 == conditional_field_name) && (!all_flag))
			{
				display_message(ERROR_MESSAGE, "gfx destroy nodes:  No nodes specified.");
				return_code = 0;
			}
		}
		if (return_code && conditional_field_name)
		{
			Cmiss_field_module_id field_module = Cmiss_region_get_field_module(region);
			conditional_field = Cmiss_field_module_find_field_by_name(field_module, conditional_field_name);
			if (!conditional_field)
			{
				display_message(ERROR_MESSAGE,
					"gfx destroy nodes:  conditional field cannot be found");
				return_code = 0;
			}
			Cmiss_field_module_destroy(&field_module);
		}
		if (return_code)
		{
			Cmiss_field_module_id field_module = Cmiss_region_get_field_module(region);
			Cmiss_field_group_id selection_group = NULL;
			if (selected_flag)
			{
				Cmiss_rendition *rendition = Cmiss_region_get_rendition_internal(region);
				if (rendition)
				{
					selection_group = Cmiss_rendition_get_selection_group(rendition);
					Cmiss_rendition_destroy(&rendition);
				}
			}
			Cmiss_field_id use_conditional_field = 0;
			if (group && conditional_field)
			{
				use_conditional_field = Cmiss_field_module_create_or(field_module, conditional_field, Cmiss_field_group_base_cast(group));
			}
			else if (conditional_field)
			{
				use_conditional_field = Cmiss_field_access(conditional_field);
			}
			else if (group)
			{
				use_conditional_field = Cmiss_field_access(Cmiss_field_group_base_cast(group));
			}
			LIST(FE_node) *destroy_node_list = 0;
			if ((!selected_flag) || selection_group)
			{
				destroy_node_list = FE_node_list_from_region_and_selection_group(
					region, node_ranges, Cmiss_field_group_base_cast(selection_group), use_conditional_field, time, (use_data != NULL));
				if (!destroy_node_list)
					return_code = 0;
			}
			if (destroy_node_list && (0 < NUMBER_IN_LIST(FE_node)(destroy_node_list)))
			{
				Cmiss_field_module_begin_change(field_module);
				FE_region *fe_region = Cmiss_region_get_FE_region(region);
				if (use_data)
					fe_region = FE_region_get_data_FE_region(fe_region);
				return_code = FE_region_remove_FE_node_list(fe_region, destroy_node_list);
				int number_not_destroyed = 0;
				if (0 < (number_not_destroyed = NUMBER_IN_LIST(FE_node)(
					destroy_node_list)))
				{
					display_message(WARNING_MESSAGE,
						"%d of the selected node(s) could not be destroyed",
						number_not_destroyed);
				}
				if (selection_group)
				{
					Cmiss_field_group_remove_empty_subgroups(selection_group);
				}
				Cmiss_field_module_end_change(field_module);
			}
			else
			{
				if (use_data)
				{
					display_message(WARNING_MESSAGE, "gfx destroy data:  No data specified");
				}
				else
				{
					display_message(WARNING_MESSAGE, "gfx destroy nodes:  No nodes specified");
				}
			}
			if (destroy_node_list)
				DESTROY(LIST(FE_node))(&destroy_node_list);
			if (selection_group)
				Cmiss_field_group_destroy(&selection_group);
			if (use_conditional_field)
				Cmiss_field_destroy(&use_conditional_field);
			Cmiss_field_module_destroy(&field_module);
		}
		if (conditional_field)
		{
			DEACCESS(Computed_field)(&conditional_field);
		}
		if (conditional_field_name)
		{
			DEALLOCATE(conditional_field_name);
		}
		DESTROY(Multi_range)(&node_ranges);
		Cmiss_field_group_destroy(&group);
		Cmiss_region_destroy(&region);
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_destroy_nodes.  Invalid argument(s)");
		return_code = 0;
	}
	return (return_code);
}

static int gfx_destroy_Scene(struct Parse_state *state,
	void *dummy_to_be_modified, void *scene_manager_void)
/*******************************************************************************
LAST MODIFIED : 5 December 2001

DESCRIPTION :
Executes a GFX DESTROY SCENE command.
==============================================================================*/
{
	const char *current_token;
	struct Scene *scene;
	int return_code;
	struct MANAGER(Scene) *scene_manager;

	ENTER(gfx_destroy_Scene);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (scene_manager = (struct MANAGER(Scene) *)scene_manager_void))
	{
		if (NULL != (current_token = state->current_token))
		{
			if (strcmp(PARSER_HELP_STRING, current_token) &&
				strcmp(PARSER_RECURSIVE_HELP_STRING, current_token))
			{
				if (NULL != (scene = FIND_BY_IDENTIFIER_IN_MANAGER(Scene, name)(
					current_token, scene_manager)))
				{
					if (REMOVE_OBJECT_FROM_MANAGER(Scene)(scene, scene_manager))
					{
						return_code = 1;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Could not remove scene %s from manager", current_token);
						return_code = 0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE, "Unknown scene: %s", current_token);
					return_code = 0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE, " SCENE_NAME");
				return_code = 1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE, "Missing scene name");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "gfx_destroy_Scene.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_destroy_Scene */

static int gfx_destroy_vtextures(struct Parse_state *state,
	void *dummy_to_be_modified,void *volume_texture_manager_void)
/*******************************************************************************
LAST MODIFIED : 7 October 1996

DESCRIPTION :
Executes a GFX DESTROY VTEXTURES command.
???DB.  Could merge with destroy_graphics_objects if graphics_objects used the
	new list structures.
==============================================================================*/
{
	const char *current_token;
	int return_code;
	struct MANAGER(VT_volume_texture) *volume_texture_manager;
	struct VT_volume_texture *volume_texture;

	ENTER(gfx_destroy_vtextures);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (NULL != (volume_texture_manager=
			(struct MANAGER(VT_volume_texture) *)volume_texture_manager_void))
		{
			if (NULL != (current_token = state->current_token))
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					if (NULL != (volume_texture=FIND_BY_IDENTIFIER_IN_MANAGER(VT_volume_texture,
						name)(current_token,volume_texture_manager)))
					{
						/* remove object from list (destroys automatically) */
						return_code=REMOVE_OBJECT_FROM_MANAGER(VT_volume_texture)(
							volume_texture,volume_texture_manager);
					}
					else
					{
						display_message(ERROR_MESSAGE,"Volume texture does not exist: %s",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(INFORMATION_MESSAGE," <NAME[all]>");
					return_code=1;
				}
			}
			else
			{
				/* destroy all objects in list */
				return_code=REMOVE_ALL_OBJECTS_FROM_MANAGER(VT_volume_texture)(
					volume_texture_manager);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_destroy_vtextures.  Missing volume texture manager");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_destroy_vtextures.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_destroy_vtextures */

static int execute_command_gfx_destroy(struct Parse_state *state,
	void *dummy_to_be_modified, void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 3 March 2003

DESCRIPTION :
Executes a GFX DESTROY command.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Option_table *option_table;

	ENTER(execute_command_gfx_destroy);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (NULL != (command_data = (struct Cmiss_command_data *)command_data_void))
		{
			if (state->current_token)
			{
				option_table = CREATE(Option_table)();
				/* curve */
				Option_table_add_entry(option_table, "curve", NULL,
					command_data->curve_manager, gfx_destroy_Curve);
				/* data */
				Option_table_add_entry(option_table, "data", /*use_data*/(void *)1,
					command_data_void, gfx_destroy_nodes);
				/* dgroup */
				Option_table_add_entry(option_table, "dgroup", NULL,
					command_data->root_region, gfx_destroy_group);
				/* egroup */
				Option_table_add_entry(option_table, "egroup", NULL,
					command_data->root_region, gfx_destroy_group);
				/* elements */
				Option_table_add_entry(option_table, "elements", /*dimension*/(void *)3,
					command_data_void, gfx_destroy_elements);
				/* faces */
				Option_table_add_entry(option_table, "faces", /*dimension*/(void *)2,
					command_data_void, gfx_destroy_elements);
				/* field */
				Option_table_add_entry(option_table, "field", NULL,
					(void *)command_data->root_region, gfx_destroy_Computed_field);
				/* graphics_object */
				Option_table_add_entry(option_table, "graphics_object", NULL,
					command_data_void, gfx_destroy_graphics_object);
				/* lines */
				Option_table_add_entry(option_table, "lines", /*dimension*/(void *)1,
					command_data_void, gfx_destroy_elements);
				/* material */
				Option_table_add_entry(option_table, "material", NULL,
					Material_package_get_material_manager(command_data->material_package), gfx_destroy_material);
				/* ngroup */
				Option_table_add_entry(option_table, "ngroup", NULL,
					command_data->root_region, gfx_destroy_group);
				/* nodes */
				Option_table_add_entry(option_table, "nodes", /*use_data*/(void *)0,
					command_data_void, gfx_destroy_nodes);
				/* region */
				Option_table_add_entry(option_table, "region", NULL,
					command_data->root_region, gfx_destroy_region);
				/* scene */
				Option_table_add_entry(option_table, "scene", NULL,
					command_data->scene_manager, gfx_destroy_Scene);
				/* spectrum */
				Option_table_add_entry(option_table, "spectrum", NULL,
					command_data->spectrum_manager, gfx_destroy_spectrum);
				/* tessellation */
				Option_table_add_entry(option_table, "tessellation", NULL,
					command_data->graphics_module, gfx_destroy_tessellation);
				/* vtextures */
				Option_table_add_entry(option_table, "vtextures", NULL,
					command_data->volume_texture_manager, gfx_destroy_vtextures);
				return_code = Option_table_parse(option_table, state);
				DESTROY(Option_table)(&option_table);
			}
			else
			{
				set_command_prompt("gfx destroy",command_data);
				return_code = 1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_gfx_destroy.  Invalid argument(s)");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_gfx_destroy.  Missing state");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_destroy */

static int execute_command_gfx_draw(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 3 March 2003

DESCRIPTION :
Executes a GFX DRAW command.
==============================================================================*/
{
	const char *graphic_name;
	struct GT_object *graphics_object;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Option_table *option_table;
	struct Scene *scene;
	char *region_path = NULL;

	ENTER(execute_command_gfx_draw);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (command_data=(struct Cmiss_command_data *)command_data_void))
	{
		/* initialize defaults */
		graphics_object = (struct GT_object *)NULL;
		scene = ACCESS(Scene)(command_data->default_scene);
		graphic_name = NULL;
		option_table=CREATE(Option_table)();
		/* as */
		Option_table_add_entry(option_table,"as",&graphic_name,
			(void *)1,set_name);
		/* graphics_object */
		Option_table_add_entry(option_table,"glyph",&graphics_object,
			command_data->glyph_manager,set_Graphics_object);
		/* group */
		Option_table_add_entry(option_table, "group", &region_path,
			command_data->root_region, set_Cmiss_region_path);
		/* scene */
		Option_table_add_entry(option_table,"scene",&scene,
			command_data->scene_manager,set_Scene);
		/* default when token omitted (graphics_object) */
		Option_table_add_entry(option_table,(char *)NULL,&graphics_object,
			command_data->glyph_manager,set_Graphics_object);
		return_code = Option_table_multi_parse(option_table,state);
		/* no errors, not asking for help */
		if (return_code)
		{
			if (graphics_object)
			{
				if (scene)
				{
					return_code = Scene_add_graphics_object(scene, graphics_object,
						graphic_name);
				}
			}
			else if (region_path)
			{
				display_message(ERROR_MESSAGE,
					"execute_command_gfx_draw.  gfx draw group command is no "
					"longer supported, cmgui will always draw graphics for regions");
			}
		} /* parse error,help */
		DESTROY(Option_table)(&option_table);
		if (scene)
		{
			DEACCESS(Scene)(&scene);
		}
		if (region_path)
		{
			DEALLOCATE(region_path);
		}
		if (graphics_object)
		{
			DEACCESS(GT_object)(&graphics_object);
		}
		if (graphic_name)
		{
			DEALLOCATE(graphic_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_gfx_draw.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_draw */

struct Apply_transformation_data
/*******************************************************************************
LAST MODIFIED : 3 March 2003

DESCRIPTION :
Data for applying transformation.  Required because the data list of nodes is
not in the same region as the region returned from FE_node_get_FE_region which
is the region where the fields are defined (the parent region in this case).
==============================================================================*/
{
	gtMatrix transformation;
	struct FE_region *fe_region;
}; /* struct Apply_transformation_data */

static int apply_transformation_to_node(struct FE_node *node,
	void *data_void)
/*******************************************************************************
LAST MODIFIED : 3 March 2003

DESCRIPTION :
Iterator that modifies the position of each node according to the
transformation in the transformation data.
Should enclose multiple calls in FE_region_begin_change/end_change wrappers.
==============================================================================*/
{
	FE_value x, x2, y, y2, z, z2, h2;
	int return_code;
	struct Apply_transformation_data *data;

	ENTER(apply_transformation_to_node);
	return_code = 0;
	if (node && (data = (struct Apply_transformation_data  *)data_void))
	{
		FE_field *coordinate_field = get_FE_node_default_coordinate_field(node);
		if (FE_node_get_position_cartesian(node, coordinate_field,
			&x, &y, &z, (FE_value *)NULL))
		{
			/* Get the new position */
			h2 = (data->transformation)[0][3] * x
				+ (data->transformation)[1][3] * y
				+ (data->transformation)[2][3] * z
				+ (data->transformation)[3][3];
			x2 = ((data->transformation)[0][0] * x
				+ (data->transformation)[1][0] * y
				+ (data->transformation)[2][0] * z
				+ (data->transformation)[3][0]) / h2;
			y2 = ((data->transformation)[0][1] * x
				+ (data->transformation)[1][1] * y
				+ (data->transformation)[2][1] * z
				+ (data->transformation)[3][1]) / h2;
			z2 = ((data->transformation)[0][2] * x
				+ (data->transformation)[1][2] * y
				+ (data->transformation)[2][2] * z
				+ (data->transformation)[3][2]) / h2;

			if (FE_node_set_position_cartesian(node,coordinate_field,x2,y2,z2))
			{
				return_code = 1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"apply_transformation_to_node.  Could not move node");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"apply_transformation_to_node.  Could not calculate coordinate field");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"apply_transformation_to_node.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* apply_transformation_to_node */

static int gfx_edit_graphics_object(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 3 March 2003

DESCRIPTION :
Executes a GFX EDIT GRAPHICS_OBJECT command.
==============================================================================*/
{
	char apply_flag, *region_path;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Cmiss_region *region;
	struct FE_region *fe_region;
	struct Option_table *option_table;

	ENTER(gfx_edit_graphics_object);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (command_data = (struct Cmiss_command_data *)command_data_void))
	{
		/* initialize defaults */
		apply_flag = 0;
		region_path = (char *)NULL;

		option_table = CREATE(Option_table)();
		/* apply_transformation */
		Option_table_add_entry(option_table, "apply_transformation",  &apply_flag,
			NULL, set_char_flag);
		/* name */
		Option_table_add_entry(option_table, "name", &region_path,
			(void *)1, set_name);
		/* default when token omitted (graphics_object_name) */
		Option_table_add_entry(option_table, (char *)NULL, &region_path,
			(void *)0, set_name);
		if (0 != (return_code = Option_table_multi_parse(option_table, state)))
		{
			if (region_path && Cmiss_region_get_region_from_path_deprecated(command_data->root_region,
					region_path, &region) && region)
			{
				if (apply_flag)
				{
					/* SAB Temporary place for this command cause I really need to use it,
						 not very general, doesn't work in prolate or rotate derivatives */
					struct Apply_transformation_data data;
					struct Cmiss_rendition *rendition = NULL;
					gtMatrix identity = {{1,0,0,0},{0,1,0,0},{0,0,1,0},{0,0,0,1}};
					rendition = Cmiss_graphics_module_get_rendition(command_data->graphics_module, region);
					if (rendition)
					{
						if (Cmiss_rendition_has_transformation(rendition)&&
								Cmiss_rendition_get_transformation(rendition, &(data.transformation)))
						{
							if (NULL != (fe_region = Cmiss_region_get_FE_region(region)))
							{
								/* nodes */
								data.fe_region = fe_region;
								FE_region_begin_change(fe_region);
								FE_region_for_each_FE_node(fe_region,
									apply_transformation_to_node, (void *)&data);
								FE_region_end_change(fe_region);
								/* data */
								data.fe_region = FE_region_get_data_FE_region(fe_region);
								FE_region_begin_change(data.fe_region);
								FE_region_for_each_FE_node(data.fe_region,
									apply_transformation_to_node, (void *)&data);
								FE_region_end_change(data.fe_region);
							}
							else
							{
								display_message(WARNING_MESSAGE, "Invalid region");
								return_code = 0;
							}
							Cmiss_rendition_set_transformation(rendition, &identity);
						}
						else
						{
							return_code = 1;
						}
					}
				}
				else
				{
					display_message(WARNING_MESSAGE,
						"gfx edit graphics_object:  Must specify 'apply_transformation'");
					return_code = 0;
				}
			}
			else
			{
				display_message(WARNING_MESSAGE,
					"Must specify region");
				return_code = 0;
			}
			if (region_path)
			{
				DEALLOCATE(region_path);
			}
		}
		DESTROY(Option_table)(&option_table);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_edit_graphics_object.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;
	return return_code;
}

#if defined (WX_USER_INTERFACE)
static int gfx_edit_scene(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 20 November 2000

DESCRIPTION :

Executes a GFX EDIT_SCENE command.  Brings up the Region_tree_viewer.
==============================================================================*/
{
	char close_flag;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Option_table *option_table;
	struct Scene *scene;
	ENTER(gfx_edit_scene);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (command_data = (struct Cmiss_command_data *)command_data_void))
	{
		if (command_data->region_tree_viewer)
		{
			scene = Region_tree_viewer_get_scene(command_data->region_tree_viewer);
		}
		else
		{
			scene = command_data->default_scene;
		}
		ACCESS(Scene)(scene);
		close_flag = 0;

		option_table = CREATE(Option_table)();
		/* scene (to edit) */
		Option_table_add_entry(option_table, "scene",&scene,
			command_data->scene_manager,set_Scene);
		/* close (editor) */
		Option_table_add_entry(option_table, "close", &close_flag,
			NULL, set_char_flag);
		if (0 != (return_code = Option_table_multi_parse(option_table, state)))
		{
			if (command_data->region_tree_viewer)
			{
				if (close_flag)
				{
					DESTROY(Region_tree_viewer)(&(command_data->region_tree_viewer));
				}
				else
				{
					if (scene != Region_tree_viewer_get_scene(command_data->region_tree_viewer))
					{
						return_code = Region_tree_viewer_set_scene(command_data->region_tree_viewer,
							scene);
					}
					Region_tree_viewer_bring_to_front(command_data->region_tree_viewer);
				}
			}
			else if (close_flag)
			{
				display_message(ERROR_MESSAGE,
					"gfx edit scene:  There is no scene editor to close");
				return_code = 0;
			}
			else
			{
				if ((!command_data->user_interface) ||
					(!CREATE(Region_tree_viewer)(	&(command_data->region_tree_viewer),
						command_data->graphics_module,
						command_data->scene_manager,
						scene,
						command_data->root_region,
						Material_package_get_material_manager(command_data->material_package),
						Material_package_get_default_material(command_data->material_package),
						command_data->default_font,
						command_data->glyph_manager,
						command_data->spectrum_manager,
						command_data->default_spectrum,
						command_data->volume_texture_manager,
						command_data->user_interface)))
				{
					display_message(ERROR_MESSAGE, "gfx_edit_scene.  "
						"Could not create scene editor");
					return_code = 0;
				}
			}
		} /* parse error, help */
		DESTROY(Option_table)(&option_table);
		if (scene)
		{
			DEACCESS(Scene)(&scene);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_edit_scene.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_edit_scene */
#endif /* defined (WX_USER_INTERFACE) */

#if defined (WX_USER_INTERFACE)
static int gfx_edit_spectrum(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 13 August 2002

DESCRIPTION :
Executes a GFX EDIT SPECTRUM command.
Invokes the graphical spectrum group editor.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Spectrum *spectrum;
	struct Option_table *option_table;

	ENTER(gfx_edit_spectrum);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (command_data = (struct Cmiss_command_data *)command_data_void))
	{
		/* initialize defaults */
		spectrum = (struct Spectrum *)NULL;
		option_table = CREATE(Option_table)();
		Option_table_add_entry(option_table, (char *)NULL, &spectrum,
			command_data->spectrum_manager, set_Spectrum);
		if (0 != (return_code = Option_table_multi_parse(option_table, state)))
		{
			Cmiss_region *spectrum_region = Cmiss_region_create_region(command_data->root_region);
			return_code = bring_up_spectrum_editor_dialog(
				&(command_data->spectrum_editor_dialog),
				command_data->spectrum_manager, spectrum,
				command_data->default_font,
				command_data->graphics_buffer_package, command_data->user_interface,
				command_data->graphics_module,
				command_data->scene_manager,
				spectrum_region);
			Cmiss_region_destroy(&spectrum_region);
		} /* parse error, help */
		DESTROY(Option_table)(&option_table);
		if (spectrum)
		{
			DEACCESS(Spectrum)(&spectrum);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_edit_spectrum.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_edit_spectrum */
#endif /* defined (WX_USER_INTERFACE) */

static int execute_command_gfx_edit(struct Parse_state *state,
	void *dummy_to_be_modified, void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 31 October 2001

DESCRIPTION :
Executes a GFX EDIT command.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Option_table *option_table;

	ENTER(execute_command_gfx_edit);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (command_data = (struct Cmiss_command_data *)command_data_void))
	{
		if (state->current_token)
		{
			option_table = CREATE(Option_table)();
			Option_table_add_entry(option_table, "graphics_object", NULL,
				command_data_void, gfx_edit_graphics_object);
#if defined (WX_USER_INTERFACE)
			Option_table_add_entry(option_table, "scene", NULL,
				command_data_void, gfx_edit_scene);
#endif /* if defined (WX_USER_INTERFACE) */
#if defined (WX_USER_INTERFACE)
			Option_table_add_entry(option_table, "spectrum", NULL,
				command_data_void, gfx_edit_spectrum);
#endif /* defined (WX_USER_INTERFACE) */
			return_code = Option_table_parse(option_table, state);
			DESTROY(Option_table)(&option_table);
		}
		else
		{
			set_command_prompt("gfx edit", command_data);
			return_code = 1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_gfx_edit.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_edit */

#if defined (WX_USER_INTERFACE)
static int execute_command_gfx_element_creator(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 3 March 2003

DESCRIPTION :
Executes a GFX ELEMENT_CREATOR command.
==============================================================================*/
{
	int return_code;
	ENTER(execute_command_gfx_element_creator);
	USE_PARAMETER(dummy_to_be_modified);
#if defined (WX_USER_INTERFACE)
	USE_PARAMETER(state);
	USE_PARAMETER(command_data_void);
	display_message(INFORMATION_MESSAGE,
		"\nElement creator has been moved to node tool in the graphics window in cmgui-wx.\n"
		"Please use gfx node_tool command instead.\n");
	return_code = 1;
#endif /*defined (WX_USER_INTERFACE) */
	return (return_code);
} /* execute_command_gfx_element_creator */
#endif /* defined (WX_USER_INTERFACE) */

#if defined (GTK_USER_INTERFACE) || defined (WIN32_USER_INTERFACE) || defined (CARBON_USER_INTERFACE) || defined (WX_USER_INTERFACE)
static int execute_command_gfx_element_point_tool(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 5 July 2002

DESCRIPTION :
Executes a GFX ELEMENT_POINT_TOOL command.
==============================================================================*/
{
	static const char *(dialog_strings[2]) = {"open_dialog", "close_dialog"};
	const char *dialog_string;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Computed_field *command_field;
	struct Element_point_tool *element_point_tool;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_command_field_data;

	ENTER(execute_command_gfx_element_point_tool);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (command_data = (struct Cmiss_command_data *)command_data_void))
	{
		/* initialize defaults */
		if (NULL != (element_point_tool=command_data->element_point_tool))
		{
			command_field = Element_point_tool_get_command_field(element_point_tool);
		}
		else
		{
			command_field = (struct Computed_field *)NULL;
		}
		if (command_field)
		{
			ACCESS(Computed_field)(command_field);
		}
		option_table = CREATE(Option_table)();
		/* open_dialog/close_dialog */
		dialog_string = (char *)NULL;
		Option_table_add_enumerator(option_table, /*number_of_valid_strings*/2,
			dialog_strings, &dialog_string);
		/* command_field */
		set_command_field_data.computed_field_manager=
			Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package);
		set_command_field_data.conditional_function =
			Computed_field_has_string_value_type;
		set_command_field_data.conditional_function_user_data = (void *)NULL;
		Option_table_add_entry(option_table, "command_field", &command_field,
			&set_command_field_data, set_Computed_field_conditional);
		if (0 != (return_code = Option_table_multi_parse(option_table, state)))
		{
			if (element_point_tool)
			{
				Element_point_tool_set_command_field(element_point_tool,command_field);
				if (dialog_string == dialog_strings[0])
				{
					Element_point_tool_pop_up_dialog(element_point_tool, (struct Graphics_window *)NULL);
				}
#if defined (WX_USER_INTERFACE)
			FOR_EACH_OBJECT_IN_MANAGER(Graphics_window)(
				 Graphics_window_update_Interactive_tool,
				 (void *)Element_point_tool_get_interactive_tool(element_point_tool),
				 command_data->graphics_window_manager);
#endif
			Cmiss_scene_viewer_package_update_Interactive_tool(
				command_data->scene_viewer_package,
				Element_point_tool_get_interactive_tool(element_point_tool));
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"execute_command_gfx_element_point_tool.  "
					"Missing element_point_tool");
				return_code = 0;
			}
		} /* parse error,help */
		DESTROY(Option_table)(&option_table);
		if (command_field)
		{
			DEACCESS(Computed_field)(&command_field);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_gfx_element_point_tool.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_element_point_tool */
#endif /* defined (GTK_USER_INTERFACE) || defined (WIN32_USER_INTERFACE) || defined (CARBON_USER_INTERFACE) */

#if defined (GTK_USER_INTERFACE) || defined (WIN32_USER_INTERFACE) || defined (CARBON_USER_INTERFACE) || defined (WX_USER_INTERFACE)
static int execute_command_gfx_element_tool(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 5 July 2002

DESCRIPTION :
Executes a GFX ELEMENT_TOOL command.
==============================================================================*/
{
	static const char *(dialog_strings[2]) = {"open_dialog", "close_dialog"};
	const char *dialog_string;
	int select_elements_enabled,select_faces_enabled,select_lines_enabled,
		return_code;
	struct Cmiss_command_data *command_data;
	struct Computed_field *command_field;
	struct Element_tool *element_tool;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_command_field_data;

	ENTER(execute_command_gfx_element_tool);
	USE_PARAMETER(dummy_to_be_modified);
	if (state&&(command_data=(struct Cmiss_command_data *)command_data_void))
	{
		/* initialize defaults */
		if (NULL != (element_tool = command_data->element_tool))
		{
			select_elements_enabled=
				Element_tool_get_select_elements_enabled(element_tool);
			select_faces_enabled=Element_tool_get_select_faces_enabled(element_tool);
			select_lines_enabled=Element_tool_get_select_lines_enabled(element_tool);
			command_field=Element_tool_get_command_field(element_tool);
		}
		else
		{
			select_elements_enabled=1;
			select_faces_enabled=1;
			select_lines_enabled=1;
			command_field = (struct Computed_field *)NULL;
		}
		if (command_field)
		{
			ACCESS(Computed_field)(command_field);
		}
		option_table=CREATE(Option_table)();
		/* open_dialog/close_dialog */
		dialog_string = (char *)NULL;
		Option_table_add_enumerator(option_table, /*number_of_valid_strings*/2,
			dialog_strings, &dialog_string);
		/* select_elements/no_select_elements */
		Option_table_add_switch(option_table,"select_elements","no_select_elements",
			&select_elements_enabled);
		/* select_faces/no_select_faces */
		Option_table_add_switch(option_table,"select_faces","no_select_faces",
			&select_faces_enabled);
		/* select_lines/no_select_lines */
		Option_table_add_switch(option_table,"select_lines","no_select_lines",
			&select_lines_enabled);
		/* command_field */
		set_command_field_data.computed_field_manager=
			Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package);
		set_command_field_data.conditional_function =
			Computed_field_has_string_value_type;
		set_command_field_data.conditional_function_user_data=(void *)NULL;
		Option_table_add_entry(option_table,"command_field",&command_field,
			&set_command_field_data,set_Computed_field_conditional);
		if (0 != (return_code = Option_table_multi_parse(option_table, state)))
		{
			if (element_tool)
			{
				Element_tool_set_select_elements_enabled(element_tool,
					select_elements_enabled);
				Element_tool_set_select_faces_enabled(element_tool,
					select_faces_enabled);
				Element_tool_set_select_lines_enabled(element_tool,
					select_lines_enabled);
				Element_tool_set_command_field(element_tool,command_field);
				if (dialog_string == dialog_strings[0])
				{
					Element_tool_pop_up_dialog(element_tool, (struct Graphics_window *)NULL);
				}
#if defined (WX_USER_INTERFACE)
			FOR_EACH_OBJECT_IN_MANAGER(Graphics_window)(
				 Graphics_window_update_Interactive_tool,
				 (void *)Element_tool_get_interactive_tool(element_tool),
				 command_data->graphics_window_manager);
#endif /*(WX_USER_INTERFACE)*/
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"execute_command_gfx_element_tool.  Missing element_tool");
				return_code=0;
			}
		} /* parse error,help */
		DESTROY(Option_table)(&option_table);
		if (command_field)
		{
			DEACCESS(Computed_field)(&command_field);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_gfx_element_tool.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_element_tool */
#endif /* defined (GTK_USER_INTERFACE) || defined (WIN32_USER_INTERFACE) || defined (CARBON_USER_INTERFACE) || defined (WX_USER_INTERFACE) */

static int gfx_export_alias(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
Executes a GFX EXPORT ALIAS command.
==============================================================================*/
{
#if defined (NEW_ALIAS)
	char destroy_when_saved,*default_filename="cmgui_wire",*file_name,
		*retrieve_filename,save_now,write_sdl;
	float frame_in,frame_out,view_frame;
#endif /* defined (NEW_ALIAS) */
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Scene *scene;
	static struct Modifier_entry option_table[]=
	{
#if defined (NEW_ALIAS)
		{"dont_save_now",NULL,NULL,unset_char_flag},
		{"frame_in",NULL,NULL,set_float},
		{"frame_out",NULL,NULL,set_float},
		{"keep",NULL,NULL,unset_char_flag},
		{"retrieve",NULL,NULL,set_file_name},
#endif /* defined (NEW_ALIAS) */
		{"scene",NULL,NULL,set_Scene},
#if defined (NEW_ALIAS)
		{"sdl",NULL,NULL,set_char_flag},
		{"viewframe",NULL,NULL,set_float},
		{NULL,NULL,NULL,set_file_name}
#else /* defined (NEW_ALIAS) */
		{NULL,NULL,NULL,NULL}
#endif /* defined (NEW_ALIAS) */
	};

	ENTER(gfx_export_alias);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if (NULL != (command_data = (struct Cmiss_command_data *)command_data_void))
		{
			/* initialize defaults */
#if defined (NEW_ALIAS)
			file_name=(char *)NULL;
			frame_in=0;
			frame_out=0;
			retrieve_filename=(char *)NULL;
			save_now=1;
			view_frame=0;
			write_sdl=0;
			destroy_when_saved=1;
#endif /* defined (NEW_ALIAS) */
			scene=ACCESS(Scene)(command_data->default_scene);
#if defined (NEW_ALIAS)
			(option_table[0]).to_be_modified= &save_now;
			(option_table[1]).to_be_modified= &frame_in;
			(option_table[2]).to_be_modified= &frame_out;
			(option_table[3]).to_be_modified= &destroy_when_saved;
			(option_table[4]).to_be_modified= &retrieve_filename;
			(option_table[5]).to_be_modified= &scene;
			(option_table[5]).user_data=command_data->scene_manager;
			(option_table[6]).to_be_modified= &write_sdl;
			(option_table[7]).to_be_modified= &view_frame;
			(option_table[8]).to_be_modified= &file_name;
#else /* defined (NEW_ALIAS) */
			(option_table[0]).to_be_modified= &scene;
			(option_table[0]).user_data=command_data->scene_manager;
#endif /* defined (NEW_ALIAS) */
			return_code=process_multiple_options(state,option_table);
			/* no errors, not asking for help */
			if (return_code)
			{
				if (scene)
				{
#if defined (NEW_ALIAS)
					if (!file_name)
					{
						file_name=default_filename;
					}
					if (write_sdl)
					{
						export_to_alias_sdl(scene,file_name,retrieve_filename,view_frame);
					}
					else
					{
						export_to_alias_frames(scene,file_name,frame_in,frame_out,save_now,
							destroy_when_saved);
					}
#else /* defined (NEW_ALIAS) */
					display_message(ERROR_MESSAGE,"gfx_export_alias.  The old gfx export alias is superseeded by gfx export wavefront");
					return_code=0;
#endif /* defined (NEW_ALIAS) */
				}
			} /* parse error,help */
			if (scene)
			{
				DEACCESS(Scene)(&scene);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"gfx_export_alias.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_export_alias.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_export_alias */

static int gfx_export_cm(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 22 March 2006

DESCRIPTION :
Executes a GFX EXPORT CM command.
==============================================================================*/
{
	char *ipbase_filename, *ipcoor_filename, *ipelem_filename, *ipnode_filename,
		*ipmap_filename, *region_path;
	FILE *ipbase_file, *ipcoor_file, *ipelem_file, *ipmap_file, *ipnode_file;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct FE_field *coordinate_field;
	struct Option_table *option_table;
	struct Set_FE_field_conditional_FE_region_data coordinate_field_data;

	ENTER(gfx_export_cm);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (command_data = (struct Cmiss_command_data *)command_data_void))
	{
		coordinate_field = (struct FE_field *)NULL;
		ipbase_filename = (char *)NULL;
		ipcoor_filename = (char *)NULL;
		ipelem_filename = (char *)NULL;
		ipmap_filename = (char *)NULL;
		ipnode_filename = (char *)NULL;
		region_path = (char *)NULL;

		option_table = CREATE(Option_table)();
		/* field */
		coordinate_field_data.conditional_function =
			(LIST_CONDITIONAL_FUNCTION(FE_field) *)NULL;
		coordinate_field_data.user_data = (void *)NULL;
		coordinate_field_data.fe_region =
			Cmiss_region_get_FE_region(command_data->root_region);
		Option_table_add_entry(option_table, "field",
			&coordinate_field,
			(void *)&coordinate_field_data,
			set_FE_field_conditional_FE_region);
		/* ipcoor_filename */
		Option_table_add_entry(option_table, "ipcoor_filename", &ipcoor_filename,
			NULL, set_name);
		/* ipbase_filename */
		Option_table_add_entry(option_table, "ipbase_filename", &ipbase_filename,
			NULL, set_name);
		/* ipmap_filename */
		Option_table_add_entry(option_table, "ipmap_filename", &ipmap_filename,
			NULL, set_name);
		/* ipnode_filename */
		Option_table_add_entry(option_table, "ipnode_filename", &ipnode_filename,
			NULL, set_name);
		/* ipelem_filename */
		Option_table_add_entry(option_table, "ipelem_filename", &ipelem_filename,
			NULL, set_name);
		/* region */
		Option_table_add_entry(option_table, "region", &region_path,
			command_data->root_region, set_Cmiss_region_path);

		if (0 != (return_code = Option_table_multi_parse(option_table, state)))
		{
			if (!region_path)
			{
				display_message(ERROR_MESSAGE, "You must specify a region to export.");
				return_code = 0;
			}
			if (!(ipcoor_filename && ipbase_filename && ipnode_filename && ipelem_filename))
			{
				display_message(ERROR_MESSAGE,
					"You must specify all of ipcoor_filename, ipbase_filename, ipnode_filename and ipelem_filename.");
				return_code = 0;
			}
			if (!coordinate_field)
			{
				display_message(ERROR_MESSAGE,
					"You must specify an FE_field as the coordinate field to export.");
				return_code = 0;
			}
			if (return_code)
			{
				if (!(ipcoor_file = fopen(ipcoor_filename, "w")))
				{
					display_message(ERROR_MESSAGE,
						"Unable to open ipcoor_filename %s.", ipcoor_filename);
					return_code = 0;
				}
				if (!(ipbase_file = fopen(ipbase_filename, "w")))
				{
					display_message(ERROR_MESSAGE,
						"Unable to open ipbase_filename %s.", ipbase_filename);
					return_code = 0;
				}
				if (!(ipnode_file = fopen(ipnode_filename, "w")))
				{
					display_message(ERROR_MESSAGE,
						"Unable to open ipnode_filename %s.", ipnode_filename);
					return_code = 0;
				}
				if (!(ipelem_file = fopen(ipelem_filename, "w")))
				{
					display_message(ERROR_MESSAGE,
						"Unable to open ipelem_filename %s.", ipelem_filename);
					return_code = 0;
				}
				if (ipmap_filename)
				{
					if (!(ipmap_file = fopen(ipmap_filename, "w")))
					{
						display_message(ERROR_MESSAGE,
							"Unable to open ipmap_filename %s.", ipmap_filename);
						return_code = 0;
					}
				}
				else
				{
					ipmap_file = (FILE *)NULL;
				}
			}
			if (return_code)
			{
				write_cm_files(ipcoor_file, ipbase_file,
					ipnode_file, ipelem_file, ipmap_file,
					command_data->root_region, region_path,
					coordinate_field);
				fclose(ipcoor_file);
				fclose(ipbase_file);
				fclose(ipnode_file);
				fclose(ipelem_file);
				if (ipmap_file)
				{
					fclose(ipmap_file);
				}
			}
		}
		DESTROY(Option_table)(&option_table);
		if (region_path)
		{
			DEALLOCATE(region_path);
		}
		if (ipbase_filename)
		{
			DEALLOCATE(ipbase_filename);
		}
		if (ipcoor_filename)
		{
			DEALLOCATE(ipcoor_filename);
		}
		if (ipelem_filename)
		{
			DEALLOCATE(ipelem_filename);
		}
		if (ipmap_filename)
		{
			DEALLOCATE(ipmap_filename);
		}
		if (ipnode_filename)
		{
			DEALLOCATE(ipnode_filename);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "gfx_export_cm.  Invalid argument(s)");
		return_code = 0;
	}

	LEAVE;

	return (return_code);
} /* gfx_export_cm */

static int gfx_export_iges(struct Parse_state *state,void *dummy_to_be_modified,
	void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 4 March 2003

DESCRIPTION :
Executes a GFX EXPORT IGES command.
==============================================================================*/
{
	char *file_name, *region_path;
	int return_code;
	struct Computed_field *coordinate_field;
	struct Cmiss_command_data *command_data = (struct Cmiss_command_data *)command_data_void;
	struct Cmiss_region *region;
	struct FE_region *fe_region;
	struct Option_table *option_table;

	ENTER(gfx_export_iges);
	USE_PARAMETER(dummy_to_be_modified);
	return_code = 0;
	if (state && (command_data != 0))
	{
		return_code=1;
		/* initialize defaults */
		region_path = Cmiss_region_get_root_region_path();
		coordinate_field = (struct Computed_field *)NULL;
		char *coordinate_field_name = 0;
		file_name = (char *)NULL;

		option_table = CREATE(Option_table)();
		/* coordinate_field */
		Option_table_add_entry(option_table,"coordinate_field",&coordinate_field_name,
			(void *)1,set_name);
		/* group */
		Option_table_add_entry(option_table, "group", &region_path,
			command_data->root_region, set_Cmiss_region_path);
		/* default option: file name */
		Option_table_add_default_string_entry(option_table, &file_name, "FILE_NAME");

		/* no errors, not asking for help */
		if (0 != (return_code = Option_table_multi_parse(option_table, state)))
		{
			if (Cmiss_region_get_region_from_path_deprecated(command_data->root_region,
				region_path, &region) &&
				(0 != (fe_region = Cmiss_region_get_FE_region(region))))
			{
				if (region && coordinate_field_name &&
					(0 != (coordinate_field = FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
							coordinate_field_name, Cmiss_region_get_Computed_field_manager(region)))))
				{
					if (Computed_field_has_3_components(coordinate_field, NULL))
					{
						Cmiss_field_access(coordinate_field);
					}
					else
					{
						coordinate_field = NULL;
						display_message(ERROR_MESSAGE,
							"gfx_export_iges.  "
							"Field specified is not a coordinate field.");
						return_code = 0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"gfx_export_iges.  "
						"Field %s does not exist.", coordinate_field_name);
						return_code = 0;
				}
				if (return_code)
				{
					if (!file_name)
					{
						file_name = confirmation_get_write_filename(".igs",
							 command_data->user_interface
#if defined (WX_USER_INTERFACE)
							 , command_data->execute_command
#endif /* defined (WX_USER_INTERFACE)*/
);
					}
					if (file_name)
					{
						return_code = check_suffix(&file_name, ".igs");
						if (return_code)
						{
							return_code = export_to_iges(file_name, fe_region, region_path,
								coordinate_field);
						}
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE, "gfx_export_iges.  Invalid region");
			}
		} /* parse error,help */
		if (coordinate_field)
		{
			DEACCESS(Computed_field)(&coordinate_field);
		}
		DESTROY(Option_table)(&option_table);
		DEALLOCATE(coordinate_field_name);
		DEALLOCATE(region_path);
		DEALLOCATE(file_name);
	}
	else
	{
		display_message(ERROR_MESSAGE, "gfx_export_iges.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_export_iges */

static int gfx_export_stl(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 4 July 2008

DESCRIPTION :
Executes a GFX EXPORT STL command.
==============================================================================*/
{
	char *file_name;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Option_table *option_table;
	struct Scene *scene;

	ENTER(gfx_export_stl);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (NULL != (command_data = (struct Cmiss_command_data *)command_data_void))
		{
			file_name = (char *)NULL;
			scene = ACCESS(Scene)(command_data->default_scene);
			option_table = CREATE(Option_table)();
			/* file */
			Option_table_add_entry(option_table, "file", &file_name,
				(void *)1, set_name);
			/* scene */
			Option_table_add_entry(option_table,"scene",&scene,
				command_data->scene_manager,set_Scene);
			if (0 != (return_code = Option_table_multi_parse(option_table, state)))
			{
				if (scene)
				{
					if (file_name)
					{
						return_code = export_to_stl(file_name, scene);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"gfx export stl.  Must specify file name");
						return_code = 0;
					}
				}
			}
			DESTROY(Option_table)(&option_table);
			if (scene)
				DEACCESS(Scene)(&scene);
			if (file_name)
			{
				DEALLOCATE(file_name);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_export_stl.  Invalid argument(s)");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_export_stl.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_export_stl */

static int gfx_export_vrml(struct Parse_state *state,void *dummy_to_be_modified,
	void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
Executes a GFX EXPORT VRML command.
==============================================================================*/
{
	char *file_name;
	int return_code;
	struct Option_table *option_table;
	struct Cmiss_command_data *command_data;
	struct Scene *scene;

	ENTER(gfx_export_vrml);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if (NULL != (command_data = (struct Cmiss_command_data *)command_data_void))
		{
			/* initialize defaults */
			file_name=(char *)NULL;
			scene=ACCESS(Scene)(command_data->default_scene);
			option_table = CREATE(Option_table)();
			Option_table_add_entry(option_table, "file", &file_name, (void *)1, set_name);
			Option_table_add_entry(option_table,"scene",&scene,
				command_data->scene_manager,set_Scene);
			return_code = Option_table_multi_parse(option_table, state);
			DESTROY(Option_table)(&option_table);
			/* no errors, not asking for help */
			if (return_code)
			{
				if (!file_name)
				{
					file_name=confirmation_get_write_filename(".wrl",
						 command_data->user_interface
#if defined (WX_USER_INTERFACE)
							 , command_data->execute_command
#endif /* defined (WX_USER_INTERFACE)*/
																										);
				}
				if (scene)
				{
					if (file_name)
					{
						if (0 != (return_code = check_suffix(&file_name, ".wrl")))
						{
							return_code=export_to_vrml(file_name,scene);
						}
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,"gfx_export_vrml.  Specified scene not found");
					return_code = 0;
				}
			} /* parse error,help */

			DEACCESS(Scene)(&scene);
			if (file_name)
				DEALLOCATE(file_name);
		}
		else
		{
			display_message(ERROR_MESSAGE,"gfx_export_vrml.  Invalid argument(s)");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_export_vrml.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_export_vrml */

static int gfx_export_wavefront(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
Executes a GFX EXPORT WAVEFRONT command.
==============================================================================*/
{
	const char *file_ext = ".obj";
	char *file_name,full_comments,*temp_filename;
	int frame_number, number_of_frames, return_code, version;
	struct Cmiss_command_data *command_data;
	struct Scene *scene = NULL;
	static struct Modifier_entry option_table[]=
	{
		{"file",NULL,(void *)1,set_name},
		{"frame_number",NULL,NULL,set_int_non_negative},
		{"full_comments",NULL,NULL,set_char_flag},
		{"number_of_frames",NULL,NULL,set_int_positive},
		{"scene",NULL,NULL,set_Scene},
		{"version",NULL,NULL,set_int_positive},
		{NULL,NULL,NULL,NULL}
	};
	ENTER(gfx_export_wavefront);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if (NULL != (command_data = (struct Cmiss_command_data *)command_data_void))
		{
			/* initialize defaults */
			file_name=(char *)NULL;
			frame_number = 0;
			full_comments=0;
			number_of_frames=100;
			scene=ACCESS(Scene)(command_data->default_scene);
			version=3;
			(option_table[0]).to_be_modified= &file_name;
			(option_table[1]).to_be_modified= &frame_number;
			(option_table[2]).to_be_modified= &full_comments;
			(option_table[3]).to_be_modified= &number_of_frames;
			(option_table[4]).to_be_modified= &scene;
			(option_table[4]).user_data=command_data->scene_manager;
			(option_table[5]).to_be_modified= &version;
			return_code=process_multiple_options(state,option_table);
			/* no errors, not asking for help */

			if (return_code)
			{
				if (!file_name)
				{
					if (GET_NAME(Scene)(scene,&file_name))
					{
						if (REALLOCATE(temp_filename,file_name,char,strlen(file_name)+5))
						{
							file_name=temp_filename;
							strcat(file_name,".obj");
						}
					}
					else
					{
						if (GET_NAME(Scene)(scene,&file_name))
						{
							if (REALLOCATE(temp_filename,file_name,char,strlen(file_name)+5))
							{
								file_name=temp_filename;
								strcat(file_name, file_ext);
							}
						}
					}
				}
				if (file_name)
				{
					return_code = check_suffix(&file_name, file_ext);
					if (!return_code)
					{
						strcat(file_name, file_ext);
					}
					return_code=export_to_wavefront(file_name,scene,full_comments);
				}
			} /* parse error,help */
			DEACCESS(Scene)(&scene);

			if (file_name)
				DEALLOCATE(file_name);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_export_wavefront.  Invalid argument(s)");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_export_wavefront.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_export_wavefront */

#if defined (USE_OPENCASCADE)
static struct Spectrum_settings *create_spectrum_component( Spectrum_settings_colour_mapping colour )
{
	int component = 1;
	struct Spectrum_settings *settings = CREATE(Spectrum_settings)();
	Spectrum_settings_set_type(settings, SPECTRUM_LINEAR);
	Spectrum_settings_set_colour_mapping(settings, colour);
	Spectrum_settings_set_extend_above_flag(settings, 1);
	Spectrum_settings_set_extend_below_flag(settings, 1);
	Spectrum_settings_set_reverse_flag(settings, 0);

	if ( colour == SPECTRUM_RED )
		component = 1;
	else if ( colour == SPECTRUM_GREEN )
		component = 2;
	else
		component = 3;

	Spectrum_settings_set_component_number( settings, component );

	return settings;
}

static int create_RGB_spectrum( struct Spectrum **spectrum, void *command_data_void )
{
	int return_code = 0, number_in_list = 0;
	struct LIST(Spectrum_settings) *spectrum_settings_list;
	struct Spectrum_settings *red_settings;
	struct Spectrum_settings *green_settings;
	struct Spectrum_settings *blue_settings;
	struct Cmiss_command_data *command_data = (struct Cmiss_command_data *)command_data_void;

	if ( command_data && ( (*spectrum) = CREATE(Spectrum)("RGB") ) )
	{
		spectrum_settings_list = get_Spectrum_settings_list( (*spectrum) );
		number_in_list = NUMBER_IN_LIST(Spectrum_settings)(spectrum_settings_list);
		if ( number_in_list > 0 )
		{
			REMOVE_ALL_OBJECTS_FROM_LIST(Spectrum_settings)(spectrum_settings_list);
		}
		red_settings = create_spectrum_component( SPECTRUM_RED );
		Spectrum_settings_add( red_settings, /* end of list = 0 */0,
			spectrum_settings_list );

		green_settings = create_spectrum_component( SPECTRUM_GREEN );
		Spectrum_settings_add( green_settings, /* end of list = 0 */0,
			spectrum_settings_list );

		blue_settings = create_spectrum_component( SPECTRUM_BLUE );
		Spectrum_settings_add( blue_settings, /* end of list = 0 */0,
			spectrum_settings_list );

		Spectrum_calculate_range( (*spectrum) );
		Spectrum_calculate_range( (*spectrum) );
		Spectrum_set_minimum_and_maximum( (*spectrum), 0, 1);
		Spectrum_set_opaque_colour_flag( (*spectrum), 0 );
		if (!ADD_OBJECT_TO_MANAGER(Spectrum)( (*spectrum),
				command_data->spectrum_manager))
		{
			DESTROY(Spectrum)(spectrum);
			//DEACCESS(Spectrum)(&(command_data->default_spectrum));
		}
		else
		{
			return_code = 1;
		}
	}
	return return_code;
}

static int execute_command_gfx_import(struct Parse_state *state,
	void *dummy_to_be_modified, void *command_data_void )
{
	int return_code;
	struct Cmiss_command_data *command_data;

	ENTER(execute_command_gfx_import);

	USE_PARAMETER(dummy_to_be_modified);
	if (state && (command_data = (struct Cmiss_command_data *)command_data_void))
	{
		struct Spectrum *rgb_spectrum = (struct Spectrum *)NULL;
		struct Graphical_material *material =
			ACCESS(Graphical_material)(Material_package_get_default_material(command_data->material_package));

		char *graphics_object_name = NULL, *region_path = NULL;
		char *file_name;
		struct Cmiss_region *top_region = NULL;
		file_name = duplicate_string( "" );
		float deflection = 0.1;
		struct Option_table *option_table = CREATE(Option_table)();

		rgb_spectrum = FIND_BY_IDENTIFIER_IN_MANAGER(Spectrum,name)( "RGB", command_data->spectrum_manager );
		if ( !rgb_spectrum )
		{
			if ( !create_RGB_spectrum( &rgb_spectrum, command_data_void ) )
			{
				printf( "Failed to create RGB spectrum\n" );
			}
		}
		else
		{
			ACCESS(Spectrum)(rgb_spectrum);
		}
		// as
		Option_table_add_entry(option_table, "as", &graphics_object_name,
			(void *)1, set_name );
		// file
		Option_table_add_entry(option_table, "file", &file_name,
			(void *)1, set_name);
		// region
		Option_table_add_entry(option_table,"region",
			&region_path, (void *)1, set_name);
		// spectrum
		Option_table_add_entry(option_table, "spectrum", &rgb_spectrum,
			command_data->spectrum_manager,set_Spectrum);
		// material
		Option_table_add_set_Material_entry(option_table, "material", &material,
			command_data->material_package);
		// deflection
		Option_table_add_entry(option_table, "deflection", &deflection, NULL, set_float );

		return_code = Option_table_multi_parse(option_table, state);
		if (file_name == NULL)
		{
			return_code = 0;
		}
		if (return_code)
		{
			clock_t occStart, occEnd;
			occStart = clock();
			printf("region path = '%s'\n", region_path);
			if (region_path)
			{
				top_region = Cmiss_region_find_subregion_at_path(
					command_data->root_region, region_path);
				if (NULL == top_region)
				{
					top_region = Cmiss_region_create_subregion(
						command_data->root_region, region_path);
					if (NULL == top_region)
					{
						display_message(ERROR_MESSAGE, "execute_command_gfx_import.  "
							"Unable to find or create region '%s'.", region_path);
						return_code = 0;
					}
				}
			}
			else
			{
				top_region = ACCESS(Cmiss_region)(command_data->root_region);
			}

			if (return_code)
			{
				if (Cmiss_region_import_cad_file(top_region, file_name))
				{
					occEnd = clock();
					//DEBUG_PRINT( "OCC load took %.2lf seconds\n", ( occEnd - occStart ) / double( CLOCKS_PER_SEC ) );
				}
			}
			DEACCESS(Cmiss_region)(&top_region);
		}
		DESTROY(Option_table)(&option_table);
		if (region_path)
		{
			DEALLOCATE(region_path);
		}
		if (rgb_spectrum)
		{
			DEACCESS(Spectrum)(&rgb_spectrum);
		}
		DEALLOCATE(graphics_object_name);
		DEACCESS(Graphical_material)(&material);
		DEALLOCATE(file_name);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_gfx_import.  Invalid argument(s)\n");
		return_code = 0;
	}
	LEAVE;
	return (return_code);
}

#endif /* USE_OPENCASCADE */

static int execute_command_gfx_export(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 4 March 2003

DESCRIPTION :
Executes a GFX EXPORT command.
==============================================================================*/
{
	int return_code;
	struct Option_table *option_table;

	ENTER(execute_command_gfx_export);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && command_data_void)
	{
		option_table = CREATE(Option_table)();
		Option_table_add_entry(option_table,"alias",NULL,
			command_data_void, gfx_export_alias);
		Option_table_add_entry(option_table,"cm",NULL,
			command_data_void, gfx_export_cm);
		Option_table_add_entry(option_table,"iges",NULL,
			command_data_void, gfx_export_iges);
		Option_table_add_entry(option_table,"stl",NULL,
			command_data_void, gfx_export_stl);
		Option_table_add_entry(option_table,"vrml",NULL,
			command_data_void, gfx_export_vrml);
		Option_table_add_entry(option_table,"wavefront",NULL,
			command_data_void, gfx_export_wavefront);
		return_code = Option_table_parse(option_table,state);
		DESTROY(Option_table)(&option_table);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_gfx_export.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_export */

void create_triangle_mesh(struct Cmiss_region *region, Triangle_mesh *trimesh)
{
	struct FE_region *fe_region = Cmiss_region_get_FE_region(region);
	// for efficiency cache changes until all finished
	FE_region_begin_change(fe_region);

	// create a 3-D coordinate field
	FE_field *coordinate_field = FE_field_create_coordinate_3d(fe_region, "coordinates");

	const Mesh_triangle_list  triangle_list = trimesh->get_triangle_list();
	Mesh_triangle_list_const_iterator triangle_iter;
	const Triangle_vertex_set vertex_set = trimesh->get_vertex_set();
	Triangle_vertex_set_const_iterator vertex_iter;
	// create template node with 3-D coordinate field parameters
	struct FE_node *template_node;
	int identifier = 1;
	struct FE_node *node;
	int number_of_values_confirmed;
	FE_value coordinates[3];
	double coord1, coord2, coord3;
	int initial_identifier = FE_region_get_last_FE_node_identifier(fe_region);
	int i = 0;
	for (vertex_iter = vertex_set.begin(); vertex_iter!=vertex_set.end(); ++vertex_iter)
	{
		identifier = initial_identifier+(*vertex_iter)->get_identifier();
		(*vertex_iter)->get_coordinates(&coord1, &coord2, &coord3);
		template_node = CREATE(FE_node)(/*cm_node_identifier*/identifier, fe_region, /*template_node*/NULL);
		define_FE_field_at_node_simple(template_node, coordinate_field, /*number_of_derivatives*/0, /*derivative_value_types*/NULL);

		// create a node from the template
		coordinates[0] = coord1;
		coordinates[1] = coord2;
		coordinates[2] = coord3;
		node = CREATE(FE_node)(identifier, /*fe_region*/NULL, template_node);
		ACCESS(FE_node)(node);
		set_FE_nodal_field_FE_value_values(coordinate_field, node, coordinates, &number_of_values_confirmed);
		FE_region_merge_FE_node(fe_region, node);
		DEACCESS(FE_node)(&node);

		DESTROY(FE_node)(&template_node);
		i++;
	}
	// establish mode which automates creation of shared faces
	FE_region_begin_define_faces(fe_region, /*all dimensions*/-1);

	struct CM_element_information element_identifier;
	FE_element *element;
	FE_element *template_element;
	const Triangle_vertex *vertex1, *vertex2, *vertex3;

	// create a triangle template element with linear simplex field
	for (triangle_iter = triangle_list.begin(); triangle_iter!=triangle_list.end(); ++triangle_iter)
	{
		(*triangle_iter)->get_vertexes(&vertex1, &vertex2, &vertex3);
		template_element = FE_element_create_with_simplex_shape(fe_region, /*dimension*/2);
		set_FE_element_number_of_nodes(template_element, 3);
		FE_element_define_field_simple(template_element, coordinate_field, LINEAR_SIMPLEX);

		// make an element based on the template & fill node list
		element_identifier.type = CM_ELEMENT;
		element_identifier.number = FE_region_get_next_FE_element_identifier(fe_region, /*dimension*/2, 1);
		element = CREATE(FE_element)(&element_identifier, (struct FE_element_shape *)NULL,
			(struct FE_region *)NULL, template_element);
		ACCESS(FE_element)(element);
		set_FE_element_node(element, 0, FE_region_get_FE_node_from_identifier(fe_region,initial_identifier+vertex1->get_identifier()));
		set_FE_element_node(element, 1, FE_region_get_FE_node_from_identifier(fe_region,initial_identifier+vertex2->get_identifier()));
		set_FE_element_node(element, 2, FE_region_get_FE_node_from_identifier(fe_region,initial_identifier+vertex3->get_identifier()));
		FE_region_merge_FE_element_and_faces_and_nodes(fe_region, element);

		DEACCESS(FE_element)(&element);

	DEACCESS(FE_element)(&template_element);
	}
	// must remember to end define faces mode
	FE_region_end_define_faces(fe_region);

	DEACCESS(FE_field)(&coordinate_field);

	FE_region_end_change(fe_region);
}

static int gfx_mesh_graphics_tetrahedral(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
{
	int return_code;
	struct Cmiss_command_data *command_data;
	char *region_path;

	ENTER(gfx_mesh_graphics_tetrahedral);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		region_path = Cmiss_region_get_root_region_path();
		if (NULL != (command_data=(struct Cmiss_command_data *)command_data_void))
		{

			double maxh=100000;
			double fineness=0.5;
			int secondorder=0;
			struct Scene *scene;
			char *meshsize_file = NULL;
			struct Option_table *option_table;
			scene = ACCESS(Scene)(command_data->default_scene);
			option_table = CREATE(Option_table)();
			char clear = 0;
			Option_table_add_entry(option_table, "region", &region_path,
				command_data->root_region, set_Cmiss_region_path);
			Option_table_add_entry(option_table, "scene", &scene,
				command_data->scene_manager, set_Scene);
			Option_table_add_entry(option_table,"clear_region",
				&clear,(void *)NULL,set_char_flag);
			Option_table_add_entry(option_table,"mesh_global_size",
				&maxh,(void *)NULL,set_double);
			Option_table_add_entry(option_table,"fineness",
				&fineness,(void *)NULL,set_double);
			Option_table_add_entry(option_table,"secondorder",
				&secondorder,(void *)NULL,set_int);
			Option_table_add_string_entry(option_table,"meshsize_file",
				&meshsize_file, " FILENAME");

			if ((return_code = Option_table_multi_parse(option_table, state)))
			{
#if defined (USE_NETGEN)
				Triangle_mesh *trimesh = NULL;
				if (scene)
				{
					float tolerance = 0.000001;
					double centre_x, centre_y, centre_z, size_x, size_y, size_z;
					build_Scene(scene);
					Scene_get_graphics_range(scene,
						&centre_x, &centre_y, &centre_z, &size_x, &size_y, &size_z);
					if (size_x !=0 && size_y!=0 && size_z!=0)
					{
						tolerance = tolerance * (float)sqrt(
							size_x*size_x + size_y*size_y + size_z*size_z);
					}
					Render_graphics_triangularisation renderer(NULL, tolerance);
					if (renderer.Scene_compile(scene))
					{
						return_code = renderer.Scene_execute(scene);
						trimesh = renderer.get_triangle_mesh();
						struct Cmiss_region *region = Cmiss_region_find_subregion_at_path(
							command_data->root_region, region_path);
						if (clear)
						{
							Cmiss_region_clear_finite_elements(region);
						}
						if (trimesh && region)
						{
							struct Generate_netgen_parameters *generate_netgen_para=NULL;
							generate_netgen_para=create_netgen_parameters();
							set_netgen_parameters_maxh(generate_netgen_para,maxh);
							set_netgen_parameters_fineness(generate_netgen_para,fineness);
							set_netgen_parameters_secondorder(generate_netgen_para,secondorder);
							set_netgen_parameters_trimesh(generate_netgen_para, trimesh);
							if (meshsize_file)
								set_netgen_parameters_meshsize_filename(generate_netgen_para, meshsize_file);
							generate_mesh_netgen(Cmiss_region_get_FE_region(region), generate_netgen_para);
							release_netgen_parameters(generate_netgen_para);

						}
						else
						{
							display_message(ERROR_MESSAGE, "gfx_mesh_graphics_tetrahedral."
								"Unknown region: %s", region_path);
						}
						Cmiss_region_destroy(&region);
					}
				}
#else
				USE_PARAMETER(scene);
				USE_PARAMETER(region_path);
				display_message(ERROR_MESSAGE,
					"gfx_mesh_graphics. Does not support tetrahedral mesh yet. To use this feature"
					" please compile cmgui with Netgen");
				return_code = 0;
#endif /* defined (USE_NETGEN) */
			}
			DEALLOCATE(region_path);
			DESTROY(Option_table)(&option_table);
			DEACCESS(Scene)(&scene);
			if (meshsize_file)
			{
				DEALLOCATE(meshsize_file);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_mesh_graphics_tetrahedral.  Invalid argument(s)");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_mesh_graphics_tetrahedral.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_mesh_graphics */

static int gfx_mesh_graphics_triangle(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
{
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Option_table *option_table;
	char *region_path;

	ENTER(gfx_mesh_graphics_triangle);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		region_path = Cmiss_region_get_root_region_path();
		if (NULL != (command_data=(struct Cmiss_command_data *)command_data_void))
		{
			struct Scene *scene;
			Triangle_mesh *trimesh = NULL;
			scene = ACCESS(Scene)(command_data->default_scene);
			option_table = CREATE(Option_table)();
			char clear = 0;
			Option_table_add_entry(option_table, "scene", &scene,
				command_data->scene_manager, set_Scene);
			Option_table_add_entry(option_table, "region", &region_path,
				command_data->root_region, set_Cmiss_region_path);
			Option_table_add_entry(option_table,"clear_region",
				&clear,(void *)NULL,set_char_flag);
			if ((return_code = Option_table_multi_parse(option_table, state)))
			{
				if (scene)
				{
					float tolerance = 0.000001;
					double centre_x, centre_y, centre_z, size_x, size_y, size_z;
					build_Scene(scene);
					Scene_get_graphics_range(scene,
						&centre_x, &centre_y, &centre_z, &size_x, &size_y, &size_z);
					if (size_x !=0 && size_y!=0 && size_z!=0)
					{
						tolerance = tolerance * (float)sqrt(
							size_x*size_x + size_y*size_y + size_z*size_z);
					}
					Render_graphics_triangularisation renderer(NULL, tolerance);
					if (renderer.Scene_compile(scene))
					{
						return_code = renderer.Scene_execute(scene);
						trimesh = renderer.get_triangle_mesh();
						struct Cmiss_region *region = Cmiss_region_find_subregion_at_path(
							command_data->root_region, region_path);
						if (clear)
						{
							Cmiss_region_clear_finite_elements(region);
						}
						if (trimesh && region)
						{
							create_triangle_mesh(region, trimesh);
						}
						else
						{
							display_message(ERROR_MESSAGE, "gfx_mesh_graphics_triangle. Unknown region: %s", region_path);
						}
						Cmiss_region_destroy(&region);
					}
				}
			}
			DEALLOCATE(region_path);
			DESTROY(Option_table)(&option_table);
			DEACCESS(Scene)(&scene);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_mesh_graphics_triangle.  Invalid argument(s)");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_mesh_graphics_triangle.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_mesh_graphics */


static int gfx_mesh_graphics(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
{
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Option_table *option_table;

	ENTER(gfx_mesh_graphics);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (command_data=(struct Cmiss_command_data *)command_data_void))
	{
		return_code = 1;

		option_table = CREATE(Option_table)();
		Option_table_add_entry(option_table,"tetrahedral", NULL,
			(void *)command_data, gfx_mesh_graphics_tetrahedral);
		Option_table_add_entry(option_table,"triangle", NULL,
			(void *)command_data, gfx_mesh_graphics_triangle);
		return_code = Option_table_parse(option_table, state);

		DESTROY(Option_table)(&option_table);
	}
	else
	{
		display_message(ERROR_MESSAGE,
				"gfx_mesh_graphics.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_mesh_graphics */

static int execute_command_gfx_mesh(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 4 March 2003

DESCRIPTION :
Executes a GFX EXPORT command.
==============================================================================*/
{
	int return_code;
	struct Option_table *option_table;

	ENTER(execute_command_gfx_export);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && command_data_void)
	{
		option_table = CREATE(Option_table)();
		Option_table_add_entry(option_table,"graphics",NULL,
			command_data_void, gfx_mesh_graphics);
		return_code = Option_table_parse(option_table,state);
		DESTROY(Option_table)(&option_table);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_gfx_export.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_export */

/***************************************************************************//**
 * Executes a GFX EVALUATE DGROUP/EGROUP/NGROUP command.
 */
int gfx_evaluate(struct Parse_state *state, void *dummy_to_be_modified,
	void *command_data_void)
{
	int return_code;

	ENTER(gfx_evaluate);
	USE_PARAMETER(dummy_to_be_modified);
	Cmiss_command_data *command_data = reinterpret_cast<Cmiss_command_data *>(command_data_void);
	if (state && command_data)
	{
		char *data_region_path = 0;
		char *element_region_path = 0;
		char *node_region_path = 0;
		char selected_flag = 0;
		char *source_field_name = 0;
		char *destination_field_name = 0;
		FE_value time = 0;

		Option_table *option_table = CREATE(Option_table)();
		Option_table_add_string_entry(option_table, "destination", &destination_field_name, " FIELD_NAME");
		Option_table_add_string_entry(option_table, "dgroup", &data_region_path, " REGION_PATH/GROUP_NAME");
		Option_table_add_string_entry(option_table, "egroup", &element_region_path, " REGION_PATH/GROUP_NAME");
		Option_table_add_string_entry(option_table, "ngroup", &node_region_path, " REGION_PATH/GROUP_NAME");
		Option_table_add_char_flag_entry(option_table, "selected", &selected_flag);
		Option_table_add_string_entry(option_table, "source", &source_field_name, " FIELD_NAME");

		if (0 != (return_code = Option_table_multi_parse(option_table, state)))
		{
			if (destination_field_name && source_field_name)
			{
				struct Element_point_ranges_selection *element_point_ranges_selection = 0;
				if (selected_flag)
				{
					element_point_ranges_selection =
						command_data->element_point_ranges_selection;
				}
				char *region_or_group_path = 0;
				if (data_region_path && (!element_region_path) && (!node_region_path))
				{
					region_or_group_path = data_region_path;
				}
				else if (element_region_path && (!data_region_path) &&
					(!node_region_path))
				{
					region_or_group_path = element_region_path;
				}
				else if (node_region_path && (!data_region_path) &&
					(!element_region_path))
				{
					region_or_group_path = node_region_path;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"gfx_evaluate.  Must specify one of dgroup/egroup/ngroup");
					return_code = 0;
				}
				if (region_or_group_path)
				{
					Cmiss_region_id region = Cmiss_region_access(command_data->root_region);
					Cmiss_field_group_id group = 0;
					if (region_or_group_path)
					{
						Parse_state *temp_parse_state = create_Parse_state(region_or_group_path);
						return_code = set_Cmiss_region_or_group(temp_parse_state, (void *)&region, (void *)&group);
						destroy_Parse_state(&temp_parse_state);
					}
					if (return_code)
					{
						Cmiss_field_module_id field_module = Cmiss_region_get_field_module(region);
						Cmiss_field_id destination_field = Cmiss_field_module_find_field_by_name(field_module,
							destination_field_name);
						Cmiss_field_id source_field = Cmiss_field_module_find_field_by_name(field_module,
							source_field_name);
						Cmiss_field_group_id selection_group = 0;
						if (selected_flag)
						{
							Cmiss_rendition_id rendition =
								Cmiss_graphics_module_get_rendition(command_data->graphics_module, region);
							if (rendition)
							{
								selection_group = Cmiss_rendition_get_selection_group(rendition);
								Cmiss_rendition_destroy(&rendition);
							}
						}
						if (node_region_path || data_region_path)
						{
							Cmiss_nodeset_id nodeset = Cmiss_field_module_find_nodeset_by_name(
								field_module, node_region_path ? "cmiss_nodes" : "cmiss_data");
							if (group)
							{
								Cmiss_field_node_group_id node_group = Cmiss_field_group_get_node_group(group, nodeset);
								Cmiss_nodeset_destroy(&nodeset);
								nodeset = Cmiss_nodeset_group_base_cast(Cmiss_field_node_group_get_nodeset(node_group));
								Cmiss_field_node_group_destroy(&node_group);
							}
							if (nodeset)
							{
								return_code = Cmiss_nodeset_assign_field_from_source(nodeset, destination_field, source_field,
									/*conditional_field*/Cmiss_field_group_base_cast(selection_group), time);
								Cmiss_nodeset_destroy(&nodeset);
							}
						}
						else if (element_region_path)
						{
							int highest_dimension = FE_region_get_highest_dimension(Cmiss_region_get_FE_region(region));
							Cmiss_mesh_id mesh = Cmiss_field_module_find_mesh_by_dimension(field_module, highest_dimension);
							if (group)
							{
								Cmiss_field_element_group_id element_group = Cmiss_field_group_get_element_group(group, mesh);
								Cmiss_mesh_destroy(&mesh);
								mesh = Cmiss_mesh_group_base_cast(Cmiss_field_element_group_get_mesh(element_group));
								Cmiss_field_element_group_destroy(&element_group);
							}
							if (mesh)
							{
								return_code = Cmiss_mesh_assign_grid_field_from_source(mesh, destination_field, source_field,
									/*conditional_field*/Cmiss_field_group_base_cast(selection_group),
									element_point_ranges_selection, time);
								Cmiss_mesh_destroy(&mesh);
							}
						}
						if (selection_group)
						{
							Cmiss_field_group_destroy(&selection_group);
						}
						Cmiss_field_destroy(&source_field);
						Cmiss_field_destroy(&destination_field);
						Cmiss_field_module_destroy(&field_module);
					}
					Cmiss_field_group_destroy(&group);
					Cmiss_region_destroy(&region);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_evaluate.  Must specify destination and source fields");
				return_code = 0;
			}
		}
		DESTROY(Option_table)(&option_table);
		if (data_region_path)
		{
			DEALLOCATE(data_region_path);
		}
		if (element_region_path)
		{
			DEALLOCATE(element_region_path);
		}
		if (node_region_path)
		{
			DEALLOCATE(node_region_path);
		}
		if (source_field_name)
		{
			DEALLOCATE(source_field_name);
		}
		if (destination_field_name)
		{
			DEALLOCATE(destination_field_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "gfx_evaluate.  Invalid argument(s)");
		return_code = 0;
	}

	LEAVE;

	return (return_code);
}

static int gfx_list_all_commands(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 23 July 2008

DESCRIPTION :
Executes a GFX LIST ALL_COMMANDS.
==============================================================================*/
{
	int return_code = 1;
	static const char	*command_prefix, *current_token;
	struct Cmiss_command_data *command_data;
	struct MANAGER(Graphical_material) *graphical_material_manager;

	ENTER(gfx_list_all_commands);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (NULL != (current_token = state->current_token))
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
			}
			else
			{
				display_message(INFORMATION_MESSAGE," [ALL]");
				return_code=1;
			}
		}
		else
		{
			if (NULL != (command_data = (struct Cmiss_command_data *)command_data_void))
			{
				cmiss_execute_command("gfx list field commands", command_data_void);
				/* Commands of spectrum */
				if (command_data->spectrum_manager)
				{
					FOR_EACH_OBJECT_IN_MANAGER(Spectrum)(
						for_each_spectrum_list_or_write_commands, (void *)"false", command_data->spectrum_manager);
				}
				/* Command of graphical_material */
				if (NULL != (graphical_material_manager =
					Material_package_get_material_manager(command_data->material_package)))
				{
					command_prefix="gfx create material ";
					return_code=FOR_EACH_OBJECT_IN_MANAGER(Graphical_material)(
						list_Graphical_material_commands,(void *)command_prefix,
						graphical_material_manager);
				}
				return_code =1;
				/* Command of graphics window */
#if defined (USE_CMGUI_GRAPHICS_WINDOW)
				return_code=FOR_EACH_OBJECT_IN_MANAGER(Graphics_window)(
					 list_Graphics_window_commands,(void *)NULL,
					 command_data->graphics_window_manager);
#endif /*defined (USE_CMGUI_GRAPHICS_WINDOW)*/
			}
		}
	}
	LEAVE;

	return (return_code);
}/* gfx_list_all_commands */

/***************************************************************************//**
 * List statistical information about packing in node/element btree structures.
 */
static int gfx_list_btree_statistics(struct Parse_state *state,
	void *dummy_to_be_modified, void *root_region_void)
{
	int return_code = 0;
	USE_PARAMETER(dummy_to_be_modified);
	Cmiss_region *root_region = (struct Cmiss_region *)root_region_void;
	if (state && root_region)
	{
		Cmiss_region *region = Cmiss_region_access(root_region);
		Option_table *option_table = CREATE(Option_table)();
		Option_table_add_help(option_table,
			"List statistics about btree structures storing a region's nodes and elements.");
		Option_table_add_set_Cmiss_region(option_table,
			"region", root_region, &region);
		return_code = Option_table_multi_parse(option_table, state);
		DESTROY(Option_table)(&option_table);
		if (return_code)
		{
			Cmiss_field_module_id field_module = Cmiss_region_get_field_module(region);
			FE_region *fe_region = Cmiss_region_get_FE_region(region);
			FE_region_list_btree_statistics(fe_region);
			Cmiss_field_iterator_id iterator = Cmiss_field_module_create_field_iterator(field_module);
			Cmiss_field_id field = 0;
			while (0 != (field = Cmiss_field_iterator_next(iterator)))
			{
				Cmiss_field_node_group_id node_group = Cmiss_field_cast_node_group(field);
				Cmiss_field_element_group_id element_group = Cmiss_field_cast_element_group(field);
				if (node_group || element_group)
				{
					char *name = Cmiss_field_get_name(field);
					display_message(INFORMATION_MESSAGE, "%s group: %s\n", node_group ? "Node" : "Element", name);
					if (node_group)
						Cmiss_field_node_group_list_btree_statistics(node_group);
					else
						Cmiss_field_element_group_list_btree_statistics(element_group);
					Cmiss_field_node_group_destroy(&node_group);
					Cmiss_field_element_group_destroy(&element_group);
					DEALLOCATE(name);
				}
				Cmiss_field_destroy(&field);
			}
			Cmiss_field_iterator_destroy(&iterator);
			Cmiss_field_module_destroy(&field_module);
		}
		Cmiss_region_destroy(&region);
	}
	return (return_code);
}

static int gfx_list_environment_map(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 29 June 1996

DESCRIPTION :
Executes a GFX LIST ENVIRONMENT_MAP.
==============================================================================*/
{
	const char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Environment_map *environment_map;

	ENTER(gfx_list_environment_map);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (NULL != (current_token = state->current_token))
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if (NULL != (command_data = (struct Cmiss_command_data *)command_data_void))
				{
					if (NULL != (environment_map=FIND_BY_IDENTIFIER_IN_MANAGER(Environment_map,
						name)(current_token,command_data->environment_map_manager)))
					{
						return_code=list_Environment_map(environment_map);
					}
					else
					{
						display_message(ERROR_MESSAGE,"Unknown environment map: %s",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"gfx_list_environment_map.  Missing command_data");
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," ENVIRONMENT_MAP_NAME");
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing environment map name");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_list_environment_map.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_list_environment_map */

static int gfx_list_Computed_field(struct Parse_state *state,
	void *dummy_to_be_modified,void *root_region_void)
/*******************************************************************************
LAST MODIFIED : 23 July 2008

DESCRIPTION :
Executes a GFX LIST FIELD.
==============================================================================*/
{
	static const char	*command_prefix="gfx define field ";
	char commands_flag, *command_prefix_plus_region_path;
	int path_length, return_code;
	struct Cmiss_region *root_region;
	struct Cmiss_region_path_and_name region_path_and_name;
	struct Computed_field *field;
	struct List_Computed_field_commands_data list_commands_data;
	struct LIST(Computed_field) *list_of_fields;
	struct Option_table *option_table;

	ENTER(gfx_list_Computed_field);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (root_region = (struct Cmiss_region *)root_region_void))
	{
		commands_flag=0;
		region_path_and_name.region = (struct Cmiss_region *)NULL;
		region_path_and_name.region_path = (char *)NULL;
		region_path_and_name.name = (char *)NULL;

		option_table=CREATE(Option_table)();
		/* commands */
		Option_table_add_entry(option_table, "commands", &commands_flag, NULL,
			set_char_flag);
		/* default option: region_path and/or field_name */
		Option_table_add_region_path_and_or_field_name_entry(
			option_table, (char *)NULL, &region_path_and_name, root_region);
		if (0 != (return_code = Option_table_multi_parse(option_table, state)))
		{
			field = (struct Computed_field *)NULL;
			if (region_path_and_name.name)
			{
				/* following accesses the field, if any */
				field = Cmiss_region_find_field_by_name(region_path_and_name.region,
					region_path_and_name.name);
				if (!field)
				{
					display_message(ERROR_MESSAGE,
						"gfx list field:  There is no field or child region called %s in region %s",
						region_path_and_name.name, region_path_and_name.region_path);
					return_code = 0;
				}
			}
			if (return_code)
			{
				path_length = 0;
				if (region_path_and_name.region_path)
				{
					path_length = strlen(region_path_and_name.region_path);
					if (path_length > 0)
					{
						path_length += strlen(CMISS_REGION_PATH_SEPARATOR_STRING);
					}
				}
				else
				{
					region_path_and_name.region = ACCESS(Cmiss_region)(root_region);
				}
				if (commands_flag)
				{
					if (ALLOCATE(command_prefix_plus_region_path,char,
						strlen(command_prefix)+1+path_length))
					{
						strcpy(command_prefix_plus_region_path,command_prefix);
						if (path_length > 0)
						{
							strcat(command_prefix_plus_region_path,region_path_and_name.region_path);
							strcat(command_prefix_plus_region_path,CMISS_REGION_PATH_SEPARATOR_STRING);
						}
					}
					if (field)
					{
						return_code=list_Computed_field_commands(field,
							(void *)command_prefix_plus_region_path);
					}
					else
					{
						if (NULL != (list_of_fields = CREATE(LIST(Computed_field))()))
						{
							list_commands_data.command_prefix = command_prefix_plus_region_path;
							list_commands_data.listed_fields = 0;
							list_commands_data.computed_field_list = list_of_fields;
							list_commands_data.computed_field_manager =
								Cmiss_region_get_Computed_field_manager(region_path_and_name.region);
							while (FOR_EACH_OBJECT_IN_MANAGER(Computed_field)(
								list_Computed_field_commands_if_managed_source_fields_in_list,
								(void *)&list_commands_data, Cmiss_region_get_Computed_field_manager(region_path_and_name.region)) &&
								(0 != list_commands_data.listed_fields))
							{
								list_commands_data.listed_fields = 0;
							}
							DESTROY(LIST(Computed_field))(&list_of_fields);
						}
						else
						{
							return_code=0;
						}
					}
					DEALLOCATE(command_prefix_plus_region_path);
				}
				else
				{
					if (field)
					{
						return_code = list_Computed_field(field,(void *)NULL);
					}
					else
					{
						return_code=FOR_EACH_OBJECT_IN_MANAGER(Computed_field)(
							list_Computed_field_name,(void *)NULL,
							Cmiss_region_get_Computed_field_manager(region_path_and_name.region));
					}
				}
				if (!return_code)
				{
					display_message(ERROR_MESSAGE,
						"gfx list field.  Failed to list fields");
				}
			}
			if (field)
			{
				DEACCESS(Computed_field)(&field);
			}
		}
		DESTROY(Option_table)(&option_table);
		if (region_path_and_name.region)
		{
			DEACCESS(Cmiss_region)(&region_path_and_name.region);
		}
		if (region_path_and_name.region_path)
		{
			DEALLOCATE(region_path_and_name.region_path);
		}
		if (region_path_and_name.name)
		{
			DEALLOCATE(region_path_and_name.name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_list_Computed_field.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_list_Computed_field */

static int gfx_list_FE_element(struct Parse_state *state,
	void *dimension_void,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 28 March 2003

DESCRIPTION :
Executes a GFX LIST ELEMENT.
==============================================================================*/
{
	int return_code;
	int dimension = VOIDPTR2INT(dimension_void);
	Cmiss_command_data *command_data = reinterpret_cast<Cmiss_command_data *>(command_data_void);
	if (state && command_data && (0 < dimension) && (dimension <= 3))
	{
		/* initialise defaults */
		char all_flag = 0;
		Cmiss_region_id region = Cmiss_region_access(command_data->root_region);
		char selected_flag = 0;
		char verbose_flag = 0;
		Multi_range *element_ranges = CREATE(Multi_range)();
		char *conditional_field_name = 0;

		Option_table *option_table = CREATE(Option_table)();
		/* all (redundant option) */
		Option_table_add_entry(option_table, "all", &all_flag, NULL, set_char_flag);
		/* region */
		Option_table_add_set_Cmiss_region(option_table, "region",
			command_data->root_region, &region);
		/* selected */
		Option_table_add_entry(option_table, "selected", &selected_flag,
			NULL, set_char_flag);
		/* conditional */
		Option_table_add_string_entry(option_table, "conditional", &conditional_field_name,
			" FIELD_NAME");
		/* verbose */
		Option_table_add_entry(option_table, "verbose", &verbose_flag,
			NULL, set_char_flag);
		/* default option: element number ranges */
		Option_table_add_entry(option_table, (char *)NULL, (void *)element_ranges,
			NULL, set_Multi_range);
		return_code = Option_table_multi_parse(option_table, state);
		DESTROY(Option_table)(&option_table);

		Cmiss_field_id conditional_field = 0;
		if (return_code && conditional_field_name)
		{
			Cmiss_field_module_id field_module = Cmiss_region_get_field_module(region);
			conditional_field = Cmiss_field_module_find_field_by_name(field_module, conditional_field_name);
			if (!conditional_field)
			{
				display_message(ERROR_MESSAGE,
					"gfx list elements:  conditional field cannot be found");
				return_code = 0;
			}
			Cmiss_field_module_destroy(&field_module);
		}
		if (return_code)
		{
			int use_dimension = dimension;
			if (dimension == 3)
			{
				use_dimension = FE_region_get_highest_dimension(Cmiss_region_get_FE_region(region));
			}
			Cmiss_field_module_id field_module = Cmiss_region_get_field_module(region);
			Cmiss_mesh_id master_mesh = Cmiss_field_module_find_mesh_by_dimension(field_module, use_dimension);
			Cmiss_mesh_group_id selection_mesh_group = 0;
			if (selected_flag)
			{
				Cmiss_rendition *rendition = Cmiss_region_get_rendition_internal(region);
				Cmiss_field_group_id selection_group = Cmiss_rendition_get_selection_group(rendition);
				if (selection_group)
				{
					Cmiss_field_element_group_id selection_element_group =
						Cmiss_field_group_get_element_group(selection_group, master_mesh);
					if (selection_element_group)
					{
						selection_mesh_group = Cmiss_field_element_group_get_mesh(selection_element_group);
						Cmiss_field_element_group_destroy(&selection_element_group);
					}
				}
				Cmiss_field_group_destroy(&selection_group);
				Cmiss_rendition_destroy(&rendition);
			}
			int number_of_elements_listed = 0;
			if ((!selected_flag) || selection_mesh_group)
			{
				Cmiss_field_cache_id cache = Cmiss_field_module_create_cache(field_module);
				Cmiss_mesh_id iteration_mesh = master_mesh;
				if (selected_flag)
				{
					iteration_mesh = Cmiss_mesh_group_base_cast(selection_mesh_group);
				}
				const bool use_element_ranges = Multi_range_get_number_of_ranges(element_ranges) > 0;
				if (Multi_range_get_total_number_in_ranges(element_ranges) == 1)
					verbose_flag = 1;
				Multi_range *output_element_ranges = CREATE(Multi_range)();
				Cmiss_element_iterator_id iter = Cmiss_mesh_create_element_iterator(iteration_mesh);
				Cmiss_element_id element = 0;
				while (NULL != (element = Cmiss_element_iterator_next_non_access(iter)))
				{
					if (use_element_ranges && !Multi_range_is_value_in_range(element_ranges, Cmiss_element_get_identifier(element)))
						continue;
					if (conditional_field)
					{
						Cmiss_field_cache_set_element(cache, element);
						if (!Cmiss_field_evaluate_boolean(conditional_field, cache))
							continue;
					}
					if (verbose_flag)
					{
						list_FE_element(element);
					}
					else
					{
						int element_identifier = Cmiss_element_get_identifier(element);
						Multi_range_add_range(output_element_ranges, element_identifier, element_identifier);
					}
					++number_of_elements_listed;
				}
				Cmiss_element_iterator_destroy(&iter);
				if ((!verbose_flag) && number_of_elements_listed)
				{
					if (dimension == 1)
					{
						display_message(INFORMATION_MESSAGE, "Lines:\n");
					}
					else if (dimension == 2)
					{
						display_message(INFORMATION_MESSAGE, "Faces:\n");
					}
					else
					{
						display_message(INFORMATION_MESSAGE, "Elements (dimension %d):\n", use_dimension);
					}
					return_code = Multi_range_display_ranges(output_element_ranges);
					display_message(INFORMATION_MESSAGE, "Total number = %d\n", number_of_elements_listed);
				}
				DESTROY(Multi_range)(&output_element_ranges);
				Cmiss_field_cache_destroy(&cache);
			}
			if (0 == number_of_elements_listed)
			{
				if (dimension == 1)
				{
					display_message(INFORMATION_MESSAGE, "gfx list lines:  No lines specified\n");
				}
				else if (dimension == 2)
				{
					display_message(INFORMATION_MESSAGE, "gfx list faces:  No faces specified\n");
				}
				else
				{
					display_message(INFORMATION_MESSAGE, "gfx list elements:  No elements specified\n");
				}
			}
			Cmiss_mesh_group_destroy(&selection_mesh_group);
			Cmiss_mesh_destroy(&master_mesh);
			Cmiss_field_module_destroy(&field_module);
		}
		DESTROY(Multi_range)(&element_ranges);
		Cmiss_field_destroy(&conditional_field);
		if (conditional_field_name)
			DEALLOCATE(conditional_field_name);
		Cmiss_region_destroy(&region);
	}
	else
	{
		display_message(ERROR_MESSAGE, "gfx_list_FE_element.  Missing state");
		return_code = 0;
	}
	return return_code;
}

static int gfx_list_FE_node(struct Parse_state *state,
	void *use_data,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 6 May 2003

DESCRIPTION :
Executes a GFX LIST NODES.
If <use_data> is set, use data_manager and data_selection, otherwise
use node_manager and node_selection.
==============================================================================*/
{
	int return_code;
	Cmiss_command_data *command_data = reinterpret_cast<Cmiss_command_data *>(command_data_void);
	if (state && command_data)
	{
		/* initialise defaults */
		char all_flag = 0;
		Cmiss_region_id region = Cmiss_region_access(command_data->root_region);
		char selected_flag = 0;
		char verbose_flag = 0;
		Multi_range *node_ranges = CREATE(Multi_range)();
		char *conditional_field_name = 0;

		Option_table *option_table = CREATE(Option_table)();
		/* all (redundant option) */
		Option_table_add_entry(option_table, "all", &all_flag, NULL, set_char_flag);
		/* region */
		Option_table_add_set_Cmiss_region(option_table, "region",
			command_data->root_region, &region);
		/* selected */
		Option_table_add_entry(option_table, "selected", &selected_flag,
			NULL, set_char_flag);
		/* conditional */
		Option_table_add_string_entry(option_table, "conditional", &conditional_field_name,
			" FIELD_NAME");
		/* verbose */
		Option_table_add_entry(option_table, "verbose", &verbose_flag,
			NULL, set_char_flag);
		/* default option: node number ranges */
		Option_table_add_entry(option_table, (char *)NULL, (void *)node_ranges,
			NULL, set_Multi_range);
		return_code = Option_table_multi_parse(option_table, state);
		DESTROY(Option_table)(&option_table);

		Cmiss_field_id conditional_field = 0;
		if (return_code && conditional_field_name)
		{
			Cmiss_field_module_id field_module = Cmiss_region_get_field_module(region);
			conditional_field = Cmiss_field_module_find_field_by_name(field_module, conditional_field_name);
			if (!conditional_field)
			{
				display_message(ERROR_MESSAGE,
					"gfx list nodes:  conditional field cannot be found");
				return_code = 0;
			}
			Cmiss_field_module_destroy(&field_module);
		}
		if (return_code)
		{
			Cmiss_field_module_id field_module = Cmiss_region_get_field_module(region);
			Cmiss_nodeset_id master_nodeset = Cmiss_field_module_find_nodeset_by_name(field_module,
				use_data ? "cmiss_data" : "cmiss_nodes");
			Cmiss_nodeset_group_id selection_nodeset_group = 0;
			if (selected_flag)
			{
				Cmiss_rendition *rendition = Cmiss_region_get_rendition_internal(region);
				Cmiss_field_group_id selection_group = Cmiss_rendition_get_selection_group(rendition);
				if (selection_group)
				{
					Cmiss_field_node_group_id selection_node_group =
						Cmiss_field_group_get_node_group(selection_group, master_nodeset);
					if (selection_node_group)
					{
						selection_nodeset_group = Cmiss_field_node_group_get_nodeset(selection_node_group);
						Cmiss_field_node_group_destroy(&selection_node_group);
					}
				}
				Cmiss_field_group_destroy(&selection_group);
				Cmiss_rendition_destroy(&rendition);
			}
			int number_of_nodes_listed = 0;
			if ((!selected_flag) || selection_nodeset_group)
			{
				Cmiss_field_cache_id cache = Cmiss_field_module_create_cache(field_module);
				Cmiss_nodeset_id iteration_nodeset = master_nodeset;
				if (selected_flag)
				{
					iteration_nodeset = Cmiss_nodeset_group_base_cast(selection_nodeset_group);
				}
				const bool use_node_ranges = Multi_range_get_number_of_ranges(node_ranges) > 0;
				if (Multi_range_get_total_number_in_ranges(node_ranges) == 1)
					verbose_flag = 1;
				Multi_range *output_node_ranges = CREATE(Multi_range)();
				Cmiss_node_iterator_id iter = Cmiss_nodeset_create_node_iterator(iteration_nodeset);
				Cmiss_node_id node = 0;
				while (NULL != (node = Cmiss_node_iterator_next_non_access(iter)))
				{
					if (use_node_ranges && !Multi_range_is_value_in_range(node_ranges, Cmiss_node_get_identifier(node)))
						continue;
					if (conditional_field)
					{
						Cmiss_field_cache_set_node(cache, node);
						if (!Cmiss_field_evaluate_boolean(conditional_field, cache))
							continue;
					}
					if (verbose_flag)
					{
						list_FE_node(node);
					}
					else
					{
						int node_identifier = Cmiss_node_get_identifier(node);
						Multi_range_add_range(output_node_ranges, node_identifier, node_identifier);
					}
					++number_of_nodes_listed;
				}
				Cmiss_node_iterator_destroy(&iter);
				if ((!verbose_flag) && number_of_nodes_listed)
				{
					display_message(INFORMATION_MESSAGE, use_data ? "Data:\n" : "Nodes:\n");
					return_code = Multi_range_display_ranges(output_node_ranges);
					display_message(INFORMATION_MESSAGE, "Total number = %d\n", number_of_nodes_listed);
				}
				DESTROY(Multi_range)(&output_node_ranges);
				Cmiss_field_cache_destroy(&cache);
			}
			if (0 == number_of_nodes_listed)
			{
				display_message(INFORMATION_MESSAGE,
					use_data ? "gfx list data:  No data specified\n" : "gfx list nodes:  No nodes specified\n");
			}
			Cmiss_nodeset_group_destroy(&selection_nodeset_group);
			Cmiss_nodeset_destroy(&master_nodeset);
			Cmiss_field_module_destroy(&field_module);
		}
		DESTROY(Multi_range)(&node_ranges);
		Cmiss_field_destroy(&conditional_field);
		if (conditional_field_name)
			DEALLOCATE(conditional_field_name);
		Cmiss_region_destroy(&region);
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_list_FE_node.  Missing state");
		return_code = 0;
	}
	return return_code;
}

static int gfx_list_graphical_material(struct Parse_state *state,
	void *dummy_to_be_modified,void *graphical_material_manager_void)
/*******************************************************************************
LAST MODIFIED : 22 September 1998

DESCRIPTION :
Executes a GFX LIST MATERIAL.
???RC Could be moved to material.c.
==============================================================================*/
{
	static const char	*command_prefix="gfx create material ";
	char commands_flag;
	int return_code;
	static struct Modifier_entry option_table[]=
	{
		{"commands",NULL,NULL,set_char_flag},
		{"name",NULL,NULL,set_Graphical_material},
		{NULL,NULL,NULL,set_Graphical_material}
	};
	struct Graphical_material *material;
	struct MANAGER(Graphical_material) *graphical_material_manager;

	ENTER(gfx_list_graphical_material);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (NULL != (graphical_material_manager=
			(struct MANAGER(Graphical_material) *)graphical_material_manager_void))
		{
			commands_flag=0;
			/* if no material specified, list all materials */
			material=(struct Graphical_material *)NULL;
			(option_table[0]).to_be_modified= &commands_flag;
			(option_table[1]).to_be_modified= &material;
			(option_table[1]).user_data= graphical_material_manager_void;
			(option_table[2]).to_be_modified= &material;
			(option_table[2]).user_data= graphical_material_manager_void;
			if (0 != (return_code = process_multiple_options(state,option_table)))
			{
				if (commands_flag)
				{
					if (material)
					{
						return_code=list_Graphical_material_commands(material,
							(void *)command_prefix);
					}
					else
					{
						return_code=FOR_EACH_OBJECT_IN_MANAGER(Graphical_material)(
							list_Graphical_material_commands,(void *)command_prefix,
							graphical_material_manager);
					}
				}
				else
				{
					if (material)
					{
						return_code=list_Graphical_material(material,(void *)NULL);
					}
					else
					{
						return_code=FOR_EACH_OBJECT_IN_MANAGER(Graphical_material)(
							list_Graphical_material,(void *)NULL,
							graphical_material_manager);
					}
				}
			}
			/* must deaccess material since accessed by set_Graphical_material */
			if (material)
			{
				DEACCESS(Graphical_material)(&material);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"gfx_list_graphical_material.  "
				"Missing graphical_material_manager_void");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_list_graphical_material.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_list_graphical_material */

/***************************************************************************//**
 * Executes a GFX LIST EGROUP/NGROUP/DGROUP command.
 */
static int gfx_list_group(struct Parse_state *state, void *dummy_to_be_modified,
	void *root_region_void)
{
	int return_code = 1;
	USE_PARAMETER(dummy_to_be_modified);
	Cmiss_region_id root_region = reinterpret_cast<Cmiss_region_id>(root_region_void);
	if (state && root_region)
	{
		Cmiss_region_id region = Cmiss_region_access(root_region);
		if ((!state->current_token) || (set_Cmiss_region(state, (void *)&region, (void *)root_region) &&
			!Parse_state_help_mode(state)))
		{
			Cmiss_field_module_id field_module = Cmiss_region_get_field_module(region);
			Cmiss_field_iterator_id field_iter = Cmiss_field_module_create_field_iterator(field_module);
			Cmiss_field_id field = 0;
			char *region_path = Cmiss_region_get_path(region);
			display_message(INFORMATION_MESSAGE, "Groups in region %s:\n", region_path);
			DEALLOCATE(region_path);
			while (0 != (field = Cmiss_field_iterator_next_non_access(field_iter)))
			{
				Cmiss_field_group_id group = Cmiss_field_cast_group(field);
				if (group)
				{
					char *group_name = Cmiss_field_get_name(field);
					display_message(INFORMATION_MESSAGE, "  %s\n", group_name);
					DEALLOCATE(group_name);
					Cmiss_field_group_destroy(&group);
				}
			}
			Cmiss_field_iterator_destroy(&field_iter);
			Cmiss_field_module_destroy(&field_module);
		}
		Cmiss_region_destroy(&region);
	}
	else
	{
		display_message(ERROR_MESSAGE, "gfx_list_group.  Invalid argument(s)");
		return_code = 0;
	}
	return return_code;
}

static int gfx_list_region(struct Parse_state *state,
	void *dummy_to_be_modified, void *root_region_void)
/*******************************************************************************
LAST MODIFIED : 27 March 2003

DESCRIPTION :
Executes a GFX LIST REGION command.
==============================================================================*/
{
	const char *current_token;
	int return_code;
	struct Cmiss_region *region, *root_region;

	ENTER(gfx_list_region);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (root_region = (struct Cmiss_region *)root_region_void))
	{
		return_code = 1;
		if ((current_token = state->current_token) &&
			((0 == strcmp(PARSER_HELP_STRING,current_token)) ||
				(0 == strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))))
		{
			/* help */
			display_message(INFORMATION_MESSAGE, " REGION_PATH");
		}
		else
		{
			region = (struct Cmiss_region *)NULL;
			if (current_token)
			{
				/* get region to be listed */
				if (Cmiss_region_get_region_from_path_deprecated(root_region, current_token,
					&region))
				{
					display_message(INFORMATION_MESSAGE, "Region %s:\n", current_token);
				}
				else
				{
					display_message(ERROR_MESSAGE, "Unknown region: %s", current_token);
					display_parse_state_location(state);
					return_code = 0;
				}
			}
			else
			{
				region = root_region;
				display_message(INFORMATION_MESSAGE,
					"Region " CMISS_REGION_PATH_SEPARATOR_STRING ":\n");
			}
			if (return_code)
			{
				return_code = Cmiss_region_list(region, 2, 2);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "gfx_list_region.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_list_region */

int Cmiss_region_list_rendition(Cmiss_region_id region, int commands_flag, int recursive_flag)
{
	if (!region)
		return 0;
	int return_code = 1;
	char *region_path = Cmiss_region_get_path(region);
	Cmiss_rendition_id rendition = Cmiss_region_get_rendition_internal(region);
	if (commands_flag)
	{
		int error = 0;
		char *command_prefix = duplicate_string("gfx modify g_element ");
		make_valid_token(&region_path);
		append_string(&command_prefix, region_path, &error);
		append_string(&command_prefix, " ", &error);
		return_code = Cmiss_rendition_list_commands(rendition, command_prefix, /*command_suffix*/";");
		DEALLOCATE(command_prefix);
	}
	else
	{
		display_message(INFORMATION_MESSAGE,
			"Contents of region %s rendition:\n", region_path);
		return_code = Cmiss_rendition_list_contents(rendition);
	}
	Cmiss_rendition_destroy(&rendition);
	DEALLOCATE(region_path);
	if (recursive_flag)
	{
		Cmiss_region_id child = Cmiss_region_get_first_child(region);
		while (child)
		{
			if (!Cmiss_region_list_rendition(child, commands_flag, recursive_flag))
			{
				Cmiss_region_destroy(&child);
				return_code = 0;
				break;
			}
			Cmiss_region_reaccess_next_sibling(&child);
		}
	}
	return return_code;
}

/***************************************************************************//**
 * Executes a GFX LIST G_ELEMENT.
 */
static int gfx_list_g_element(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
{
	int return_code;
	USE_PARAMETER(dummy_to_be_modified);
	Cmiss_command_data *command_data = reinterpret_cast<Cmiss_command_data *>(command_data_void);
	if (state && command_data)
	{
		int commands_flag = 1;
		int recursive_flag = 1;
		Cmiss_region_id region = Cmiss_region_access(command_data->root_region);
		Option_table *option_table = CREATE(Option_table)();
		/* commands|description */
		Option_table_add_switch(option_table, "commands", "description", &commands_flag);
		/* recursive|non_recursive */
		Option_table_add_switch(option_table, "recursive", "non_recursive", &recursive_flag);
		/* default option: region */
		Option_table_add_set_Cmiss_region(option_table, /*token*/(const char *)0,
			command_data->root_region, &region);
		return_code = Option_table_multi_parse(option_table, state);
		DESTROY(Option_table)(&option_table);
		if (return_code)
		{
			return_code = Cmiss_region_list_rendition(region, commands_flag, recursive_flag);
		}
		Cmiss_region_destroy(&region);
	}
	else
	{
		display_message(ERROR_MESSAGE, "gfx_list_g_element.  Invalid argument(s)");
		return_code = 0;
	}
	return (return_code);
}

static int gfx_list_graphics_object(struct Parse_state *state,
	void *dummy_to_be_modified,void *object_list_void)
/*******************************************************************************
LAST MODIFIED : 26 July 1998

DESCRIPTION :
Executes a GFX LIST GLYPH/GRAPHICS_OBJECT command.
==============================================================================*/
{
	const char *current_token;
	int return_code;
	struct GT_object *object;
	struct MANAGER(GT_object) *list;

	ENTER(gfx_list_graphics_object);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (NULL != (list=(struct MANAGER(GT_object) *)object_list_void))
		{
			if (NULL != (current_token = state->current_token))
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					if (NULL != (object=FIND_BY_IDENTIFIER_IN_MANAGER(GT_object,name)(
						current_token,list)))
					{
						return_code=GT_object_list_contents(object,(void *)NULL);
					}
					else
					{
						display_message(ERROR_MESSAGE,"Could not find object named '%s'",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(INFORMATION_MESSAGE," OBJECT_NAME[ALL]");
					return_code=1;
				}
			}
			else
			{
				/* list contents of all objects in list */
				return_code=FOR_EACH_OBJECT_IN_MANAGER(GT_object)(
					GT_object_list_contents,(void *)NULL,list);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_list_graphics_object.  Missing graphics object list");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_list_graphics_object.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_list_graphics_object */

static int gfx_list_grid_points(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 5 March 2003

DESCRIPTION :
Executes a GFX LIST GRID_POINTS.
If <used_data_flag> is set, use data_manager and data_selection, otherwise
use node_manager and node_selection.
==============================================================================*/
{
	char all_flag,ranges_flag,selected_flag;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Element_point_ranges_grid_to_multi_range_data grid_to_multi_range_data;
	struct FE_element_grid_to_multi_range_data element_grid_to_multi_range_data;
	struct FE_field *grid_field;
	struct FE_region *fe_region;
	struct Multi_range *grid_point_ranges,*multi_range;
	struct Option_table *option_table;
	struct Set_FE_field_conditional_FE_region_data set_grid_field_data;

	ENTER(gfx_list_grid_points);
	USE_PARAMETER(dummy_to_be_modified);
	if (state&&(command_data=(struct Cmiss_command_data *)command_data_void))
	{
		/* initialise defaults */
		all_flag=0;
		selected_flag=0;
		grid_point_ranges=CREATE(Multi_range)();
		fe_region = Cmiss_region_get_FE_region(command_data->root_region);

		if ((grid_field = FE_region_get_FE_field_from_name(fe_region,
			"grid_point_number")) &&
			FE_field_is_1_component_integer(grid_field,(void *)NULL))
		{
			ACCESS(FE_field)(grid_field);
		}
		else
		{
			grid_field=(struct FE_field *)NULL;
		}

		option_table=CREATE(Option_table)();
		/* all */
		Option_table_add_entry(option_table,"all",&all_flag,NULL,set_char_flag);
		/* grid_field */
		set_grid_field_data.conditional_function = FE_field_is_1_component_integer;
		set_grid_field_data.user_data = (void *)NULL;
		set_grid_field_data.fe_region = fe_region;
		Option_table_add_entry(option_table, "grid_field", &grid_field,
			(void *)&set_grid_field_data, set_FE_field_conditional_FE_region);
		/* selected */
		Option_table_add_entry(option_table,"selected",&selected_flag,
			NULL,set_char_flag);
		/* default option: grid point number ranges */
		Option_table_add_entry(option_table,(char *)NULL,(void *)grid_point_ranges,
			NULL,set_Multi_range);
		if (0 != (return_code = Option_table_multi_parse(option_table, state)))
		{
			if (grid_field)
			{
				if (NULL != (multi_range=CREATE(Multi_range)()))
				{
					ranges_flag=(0<Multi_range_get_number_of_ranges(grid_point_ranges));
					if (selected_flag)
					{
						/* fill multi_range with selected grid_point_number ranges */
						grid_to_multi_range_data.grid_fe_field=grid_field;
						grid_to_multi_range_data.multi_range=multi_range;
						grid_to_multi_range_data.all_points_native=1;
						return_code=FOR_EACH_OBJECT_IN_LIST(Element_point_ranges)(
							Element_point_ranges_grid_to_multi_range,
							(void *)&grid_to_multi_range_data,
							Element_point_ranges_selection_get_element_point_ranges_list(
								command_data->element_point_ranges_selection));
					}
					else if (ranges_flag||all_flag)
					{
						/* fill multi_range with all grid_point_number ranges */
						element_grid_to_multi_range_data.grid_fe_field=grid_field;
						element_grid_to_multi_range_data.multi_range=multi_range;
						return_code = FE_region_for_each_FE_element(fe_region,
							FE_element_grid_to_multi_range,
							(void *)&element_grid_to_multi_range_data);
					}
					if (return_code)
					{
						if (ranges_flag)
						{
							/* include in multi_range only values also in grid_point_ranges */
							Multi_range_intersect(multi_range,grid_point_ranges);
						}
						if (0<Multi_range_get_number_of_ranges(multi_range))
						{
							display_message(INFORMATION_MESSAGE,"Grid points:\n");
							return_code=Multi_range_display_ranges(multi_range);
						}
						else
						{
							display_message(WARNING_MESSAGE,
								"gfx list grid_points:  No grid points specified");
						}
					}
					DESTROY(Multi_range)(&multi_range);
				}
				else
				{
					return_code=0;
				}
				if (!return_code)
				{
					display_message(ERROR_MESSAGE,"gfx_list_grid_points.  Failed");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"To list grid_points, "
					"need integer grid_field (eg. grid_point_number)");
				return_code=0;
			}
		}
		DESTROY(Option_table)(&option_table);
		DESTROY(Multi_range)(&grid_point_ranges);
		if (grid_field)
		{
			DEACCESS(FE_field)(&grid_field);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_list_grid_points.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_list_grid_points */

static int gfx_list_light(struct Parse_state *state,
	void *dummy_to_be_modified,void *light_manager_void)
/*******************************************************************************
LAST MODIFIED : 2 September 1996

DESCRIPTION :
Executes a GFX LIST LIGHT.
==============================================================================*/
{
	const char *current_token;
	int return_code;
	struct Light *light;
	struct MANAGER(Light) *light_manager;

	ENTER(gfx_list_light);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (NULL != (light_manager=(struct MANAGER(Light) *)light_manager_void))
		{
			if (NULL != (current_token = state->current_token))
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					if (NULL != (light=FIND_BY_IDENTIFIER_IN_MANAGER(Light,name)(current_token,
						light_manager)))
					{
						return_code=list_Light(light,(void *)NULL);
					}
					else
					{
						display_message(ERROR_MESSAGE,"Unknown light: %s",current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(INFORMATION_MESSAGE," LIGHT_NAME");
					return_code=1;
				}
			}
			else
			{
				return_code=FOR_EACH_OBJECT_IN_MANAGER(Light)(list_Light,(void *)NULL,
					light_manager);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_list_light.  Missing light_manager_void");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_list_light.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_list_light */

static int gfx_list_light_model(struct Parse_state *state,
	void *dummy_to_be_modified,void *light_model_manager_void)
/*******************************************************************************
LAST MODIFIED : 3 September 1996

DESCRIPTION :
Executes a GFX LIST LMODEL.
==============================================================================*/
{
	const char *current_token;
	int return_code;
	struct Light_model *light_model;
	struct MANAGER(Light_model) *light_model_manager;

	ENTER(gfx_list_light_model);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (NULL != (light_model_manager=
			(struct MANAGER(Light_model) *)light_model_manager_void))
		{
			if (NULL != (current_token = state->current_token))
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					if (NULL != (light_model=FIND_BY_IDENTIFIER_IN_MANAGER(Light_model,name)(
						current_token,light_model_manager)))
					{
						return_code=list_Light_model(light_model,(void *)NULL);
					}
					else
					{
						display_message(ERROR_MESSAGE,"Unknown light model: %s",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(INFORMATION_MESSAGE," LIGHT_MODEL_NAME");
					return_code=1;
				}
			}
			else
			{
				FOR_EACH_OBJECT_IN_MANAGER(Light_model)(list_Light_model,(void *)NULL,
					light_model_manager);
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_list_light_model.  Missing light_model_manager_void");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_list_light_model.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_list_light_model */

/***************************************************************************//**
 * Executes a GFX LIST SCENE command.
 */
static int gfx_list_scene(struct Parse_state *state,
	void *dummy_to_be_modified, void *scene_manager_void)
{
	int return_code;

	ENTER(gfx_list_scene);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && scene_manager_void)
	{
		struct MANAGER(Cmiss_scene) *scene_manager =
			(struct MANAGER(Cmiss_scene) *)scene_manager_void;
		Cmiss_scene *scene = NULL;
		Option_table *option_table = CREATE(Option_table)();
		Option_table_add_entry(option_table, /*token*/(const char *)NULL,
			(void *)&scene, scene_manager_void, set_Scene);
		if (0 != (return_code = Option_table_multi_parse(option_table, state)))
		{
			if (scene)
			{
				return_code = list_Scene(scene, (void *)NULL);
			}
			else
			{
				return_code = FOR_EACH_OBJECT_IN_MANAGER(Scene)(list_Scene,
					(void *)NULL, scene_manager);
			}
		}
		if (scene)
		{
			Cmiss_scene_destroy(&scene);
		}
		DESTROY(Option_table)(&option_table);
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_list_scene.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_list_scene */

static int gfx_list_spectrum(struct Parse_state *state,
	void *dummy_to_be_modified,void *spectrum_manager_void)
/*******************************************************************************
LAST MODIFIED : 28 November 2000

DESCRIPTION :
Executes a GFX LIST SPECTRUM.
==============================================================================*/
{
	static const char	*command_prefix="gfx modify spectrum";
	char *commands_flag;
	int return_code;
	struct MANAGER(Spectrum) *spectrum_manager;
	struct Spectrum *spectrum;
	struct Option_table *option_table;

	ENTER(gfx_list_spectrum);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (spectrum_manager =
		(struct MANAGER(Spectrum) *)spectrum_manager_void))
	{
		commands_flag = 0;
		spectrum = (struct Spectrum *)NULL;

		option_table=CREATE(Option_table)();
		/* commands */
		Option_table_add_entry(option_table, "commands", &commands_flag,
			NULL, set_char_flag);
		/* default option: spectrum name */
		Option_table_add_entry(option_table, (char *)NULL, &spectrum,
			spectrum_manager_void, set_Spectrum);
		if (0 != (return_code = Option_table_multi_parse(option_table, state)))
		{
			if (commands_flag)
			{
				if (spectrum)
				{
					display_message(INFORMATION_MESSAGE,
						"Commands for reproducing spectrum:\n");
					return_code = Spectrum_list_commands(spectrum,
						command_prefix, (char *)NULL);
				}
				else
				{
					display_message(INFORMATION_MESSAGE," SPECTRUM_NAME\n");
					return_code = 1;
				}
			}
			else
			{
				if (spectrum)
				{
					return_code = Spectrum_list_contents(spectrum, (void *)NULL);
				}
				else
				{
					return_code = FOR_EACH_OBJECT_IN_MANAGER(Spectrum)(
						Spectrum_list_contents, (void *)NULL, spectrum_manager);
				}
			}
		}
		DESTROY(Option_table)(&option_table);
		if (spectrum)
		{
			DEACCESS(Spectrum)(&spectrum);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_list_spectrum.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_list_spectrum */

static int gfx_list_texture(struct Parse_state *state,
	void *dummy_to_be_modified,void *region_void)
/*******************************************************************************
LAST MODIFIED : 19 May 1999

DESCRIPTION :
Executes a GFX LIST TEXTURE.
???RC Could be moved to texture.c.
==============================================================================*/
{
	static const char	*command_prefix="gfx create texture ";
	char *region_name = NULL, *field_name = NULL;
	char commands_flag;
	int return_code = 0;
	struct Cmiss_region *region = NULL, *root_region = NULL;
	struct Option_table *option_table;

	ENTER(gfx_list_texture);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		root_region=(struct Cmiss_region *)region_void;
		if (root_region)
		{
			commands_flag=0;
			/* if no texture specified, list all textures */
			option_table=CREATE(Option_table)();
			Option_table_add_entry(option_table,"region",&region_name,
				(void *)1,set_name);
			Option_table_add_entry(option_table, "commands", &commands_flag,
				NULL, set_char_flag);
			Option_table_add_entry(option_table,"field",&field_name,
				(void *)1,set_name);
			return_code=Option_table_multi_parse(option_table,state);
			if (return_code)
			{
				if (region_name)
				{
					region = Cmiss_region_find_subregion_at_path(root_region,
						region_name);
					if (!region)
					{
						display_message(ERROR_MESSAGE, "No region named '%s'",region_name);
					}
				}
				else
				{
					region = ACCESS(Cmiss_region)(root_region);
					region_name = Cmiss_region_get_path(region);
				}
				if (region)
				{
					MANAGER(Computed_field) *field_manager =
						Cmiss_region_get_Computed_field_manager(region);
					Computed_field *existing_field = NULL;
					if (field_name)
					{
						existing_field =	FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
							(char *)field_name, field_manager);
						if (existing_field)
						{
							if (Computed_field_is_image_type(existing_field, NULL))
							{
								if (commands_flag)
								{
									return_code=list_image_field_commands(existing_field,(void *)command_prefix);
								}
								else
								{
									return_code=list_image_field(existing_field,NULL);
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,"gfx_list_texture.  "
									"Field specified does not contains a texture");
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,"gfx_list_texture.  "
								"Field specified does not exist in region.");
							return_code = 0;
						}
					}
					else
					{
						if (commands_flag)
						{
							return_code=FOR_EACH_OBJECT_IN_MANAGER(Computed_field)(
								list_image_field_commands,(void *)command_prefix,field_manager);
						}
						else
						{
							return_code=FOR_EACH_OBJECT_IN_MANAGER(Computed_field)(
									list_image_field,(void *)NULL,field_manager);
						}
					}
					DEACCESS(Cmiss_region)(&region);
				}
			}
			DESTROY(Option_table)(&option_table);
			if (region_name)
				DEALLOCATE(region_name);
			if (field_name)
				DEALLOCATE(field_name);
		}
		else
		{
			display_message(ERROR_MESSAGE,"gfx_list_texture.  "
				"Missing root_region");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_list_texture.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_list_texture */


static int gfx_list_transformation(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 14 March 2001

DESCRIPTION :
Executes a GFX LIST TRANSFORMATION.
==============================================================================*/
{
	//char *command_prefix,commands_flag,*scene_name,*scene_object_name;
	char commands_flag,*region_name = NULL;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Cmiss_region *region = NULL;
	static struct Modifier_entry option_table[]=
	{
		{"commands",NULL,NULL,set_char_flag},
		{"region",NULL,(void *)1,set_name},
		{NULL,NULL,NULL,NULL}
	};

	ENTER(gfx_list_transformation);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (NULL != (command_data = (struct Cmiss_command_data *)command_data_void))
		{
			/* initialise defaults */
			commands_flag=0;
			/* parse the command line */
			(option_table[0]).to_be_modified= &commands_flag;
			(option_table[1]).to_be_modified= &region_name;
			return_code=process_multiple_options(state,option_table);
			/* no errors, not asking for help */
			if (return_code)
			{
				if (region_name)
				{
					region = Cmiss_region_find_subregion_at_path(command_data->root_region,
						region_name);
					if (!region)
					{
						display_message(ERROR_MESSAGE, "No region named '%s'",region_name);
					}
				}
				else
				{
					region = ACCESS(Cmiss_region)(command_data->root_region);
					region_name = Cmiss_region_get_path(region);
				}
				if (region)
				{
					Cmiss_rendition *rendition = Cmiss_region_get_rendition_internal(region);
					if (rendition)
					{
						if (commands_flag)
						{
							/* quote scene name if it contains special characters */
							make_valid_token(&region_name);
							char *command_prefix;
							if (ALLOCATE(command_prefix, char, 40 + strlen(region_name)))
							{
								sprintf(command_prefix, "gfx set transformation name ");
								if (rendition)
								{
									return_code = list_Cmiss_rendition_transformation_commands(
										rendition,(void *)command_prefix);
									DEACCESS(Cmiss_rendition)(&rendition);
								}
								DEALLOCATE(command_prefix);
							}
							else
							{
								return_code=0;
							}
						}
						else
						{
							return_code = list_Cmiss_rendition_transformation(rendition);
						}
						Cmiss_rendition_destroy(&rendition);
					}
				}
				else
				{
					return_code=0;
				}
			} /* parse error, help */
			DEACCESS(Cmiss_region)(&region);
			if (region_name)
			{
				DEALLOCATE(region_name);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_list_transformation.  Missing command_data_void");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_list_transformation.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_list_transformation */

#if defined (SGI_MOVIE_FILE)
static int gfx_list_movie_graphics(struct Parse_state *state,
	void *dummy_to_be_modified,void *movie_graphics_manager_void)
/*******************************************************************************
LAST MODIFIED : 3 February 2000

DESCRIPTION :
Executes a GFX LIST MOVIE.
==============================================================================*/
{
	const char *current_token;
	int return_code;
	struct Movie_graphics *movie;
	struct MANAGER(Movie_graphics) *movie_graphics_manager;

	ENTER(gfx_list_movie_graphics);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (movie_graphics_manager=
			(struct MANAGER(Movie_graphics) *)movie_graphics_manager_void)
		{
			if (NULL != (current_token = state->current_token))
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					if (movie=FIND_BY_IDENTIFIER_IN_MANAGER(Movie_graphics,name)(
						current_token,movie_graphics_manager))
					{
						return_code=list_Movie_graphics(movie,(void *)NULL);
					}
					else
					{
						display_message(ERROR_MESSAGE,"Unknown volume movie: %s",
							current_token);
						return_code=0;
					}
				}
				else
				{
					display_message(INFORMATION_MESSAGE," MOVIE_NAME");
					return_code=1;
				}
			}
			else
			{
				return_code=FOR_EACH_OBJECT_IN_MANAGER(Movie_graphics)(
					list_Movie_graphics,(void *)NULL,movie_graphics_manager);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_list_movie_graphics.  Missing movie_graphics_manager_void");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_list_movie_graphics.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_list_movie_graphics */
#endif /* defined (SGI_MOVIE_FILE) */

#if defined (USE_CMGUI_GRAPHICS_WINDOW)
static int gfx_list_graphics_window(struct Parse_state *state,
	void *dummy_to_be_modified,void *graphics_window_manager_void)
/*******************************************************************************
LAST MODIFIED : 14 October 1998

DESCRIPTION :
Executes a GFX LIST WINDOW.
==============================================================================*/
{
	char commands_flag;
	int return_code;
	static struct Modifier_entry option_table[]=
	{
		{"commands",NULL,NULL,set_char_flag},
		{"name",NULL,NULL,set_Graphics_window},
		{NULL,NULL,NULL,set_Graphics_window}
	};
	struct Graphics_window *window;
	struct MANAGER(Graphics_window) *graphics_window_manager;

	ENTER(gfx_list_graphics_window);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (NULL != (graphics_window_manager=
			(struct MANAGER(Graphics_window) *)graphics_window_manager_void))
		{
			commands_flag=0;
			/* if no window specified, list all windows */
			window=(struct Graphics_window *)NULL;
			(option_table[0]).to_be_modified= &commands_flag;
			(option_table[1]).to_be_modified= &window;
			(option_table[1]).user_data= graphics_window_manager_void;
			(option_table[2]).to_be_modified= &window;
			(option_table[2]).user_data= graphics_window_manager_void;
			if (0 != (return_code = process_multiple_options(state,option_table)))
			{
				if (commands_flag)
				{
					if (window)
					{
						return_code=list_Graphics_window_commands(window,(void *)NULL);
					}
					else
					{
						return_code=FOR_EACH_OBJECT_IN_MANAGER(Graphics_window)(
							list_Graphics_window_commands,(void *)NULL,
							graphics_window_manager);
					}
				}
				else
				{
					if (window)
					{
						return_code=list_Graphics_window(window,(void *)NULL);
					}
					else
					{
						return_code=FOR_EACH_OBJECT_IN_MANAGER(Graphics_window)(
							list_Graphics_window,(void *)NULL,
							graphics_window_manager);
					}
				}
			}
			/* must deaccess window since accessed by set_Graphics_window */
			if (window)
			{
				DEACCESS(Graphics_window)(&window);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"gfx_list_graphics_window.  "
				"Missing graphics_window_manager_void");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_list_graphics_window.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_list_graphics_window */
#endif /* defined (USE_CMGUI_GRAPHICS_WINDOW) */

static int execute_command_gfx_list(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 28 November 2000

DESCRIPTION :
Executes a GFX LIST command.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Option_table *option_table;

	ENTER(execute_command_gfx_list);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (command_data=(struct Cmiss_command_data *)command_data_void))
	{
		if (state->current_token)
		{
			option_table = CREATE(Option_table)();
			/* all_commands */
			Option_table_add_entry(option_table, "all_commands", NULL,
				command_data_void, gfx_list_all_commands);
			/* btree_statistics */
			Option_table_add_entry(option_table, "btree_statistics", NULL,
				(void *)command_data->root_region, gfx_list_btree_statistics);
#if defined (USE_OPENCASCADE)
			/* cad */
			Option_table_add_entry(option_table, "cad", NULL,
				(void *)command_data->root_region, gfx_list_cad_entity);
#endif /* defined (USE_OPENCASCADE) */
			/* curve */
			Option_table_add_entry(option_table, "curve", NULL,
				command_data->curve_manager, gfx_list_Curve);
			/* data */
			Option_table_add_entry(option_table, "data", /*use_data*/(void *)1,
				command_data_void, gfx_list_FE_node);
			/* element */
			Option_table_add_entry(option_table, "elements", /*dimension=highest*/(void *)3,
				command_data_void, gfx_list_FE_element);
			/* environment_map */
			Option_table_add_entry(option_table, "environment_map", NULL,
				command_data_void, gfx_list_environment_map);
			/* faces */
			Option_table_add_entry(option_table, "faces", /*dimension*/(void *)2,
				command_data_void, gfx_list_FE_element);
			/* field */
			Option_table_add_entry(option_table, "field", NULL,
				(void *)command_data->root_region, gfx_list_Computed_field);
			/* g_element */
			Option_table_add_entry(option_table, "g_element", NULL,
				command_data_void, gfx_list_g_element);
			/* glyph */
			Option_table_add_entry(option_table, "glyph", NULL,
				command_data->glyph_manager, gfx_list_graphics_object);
			/* graphics_filter */
			Option_table_add_entry(option_table, "graphics_filter", NULL,
				(void *)command_data->graphics_module, gfx_list_graphics_filter);
			/* grid_points */
			Option_table_add_entry(option_table, "grid_points", NULL,
				command_data_void, gfx_list_grid_points);
			/* group */
			Option_table_add_entry(option_table, "group", (void *)0,
				command_data->root_region, gfx_list_group);
			/* light */
			Option_table_add_entry(option_table, "light", NULL,
				command_data->light_manager, gfx_list_light);
			/* lines */
			Option_table_add_entry(option_table, "lines", /*dimension*/(void *)1,
				command_data_void, gfx_list_FE_element);
			/* lmodel */
			Option_table_add_entry(option_table, "lmodel", NULL,
				command_data->light_model_manager, gfx_list_light_model);
			/* material */
			Option_table_add_entry(option_table, "material", NULL,
				Material_package_get_material_manager(command_data->material_package), gfx_list_graphical_material);
#if defined (SGI_MOVIE_FILE)
			/* movie */
			Option_table_add_entry(option_table, "movie", NULL,
				command_data->movie_graphics_manager, gfx_list_movie_graphics);
#endif /* defined (SGI_MOVIE_FILE) */
			/* nodes */
			Option_table_add_entry(option_table, "nodes", /*use_data*/(void *)0,
				command_data_void, gfx_list_FE_node);
			/* region */
			Option_table_add_entry(option_table, "region", NULL,
				command_data->root_region, gfx_list_region);
			/* scene */
			Option_table_add_entry(option_table, "scene", NULL,
				command_data->scene_manager, gfx_list_scene);
			/* spectrum */
			Option_table_add_entry(option_table, "spectrum", NULL,
				command_data->spectrum_manager, gfx_list_spectrum);
			/* tessellation */
			Option_table_add_entry(option_table, "tessellation", NULL,
				command_data->graphics_module, gfx_list_tessellation);
			/* texture */
			Option_table_add_entry(option_table, "texture", NULL,
					command_data->root_region, gfx_list_texture);
			/* transformation */
			Option_table_add_entry(option_table, "transformation", NULL,
				command_data_void, gfx_list_transformation);
#if defined (USE_CMGUI_GRAPHICS_WINDOW)
			/* graphics window */
			Option_table_add_entry(option_table, "window", NULL,
				command_data->graphics_window_manager, gfx_list_graphics_window);
#endif /* defined (USE_CMGUI_GRAPHICS_WINDOW) */
			return_code = Option_table_parse(option_table, state);
			DESTROY(Option_table)(&option_table);
		}
		else
		{
			set_command_prompt("gfx list", command_data);
			return_code = 1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_gfx_list.  Missing state");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_list */

/***************************************************************************//**
 * Modifies the membership of a group.  Only one of <add> or <remove> can
 * be specified at once.
 */
static int gfx_modify_element_group(struct Parse_state *state,
	void *dummy_to_be_modified, void *command_data_void)
{
	int return_code = 0;
	USE_PARAMETER(dummy_to_be_modified);
	Cmiss_command_data *command_data = reinterpret_cast<Cmiss_command_data *>(command_data_void);
	if (state && command_data)
	{
		Cmiss_region_id region = Cmiss_region_access(command_data->root_region);
		Cmiss_field_group_id group = 0;
		const char *current_token = state->current_token;
		if (!set_Cmiss_region_or_group(state, &region, &group))
		{
			// message already output
		}
		else if (!Parse_state_help_mode(state) && (!group))
		{
			display_message(ERROR_MESSAGE, "Not a group: %s", current_token);
			return_code = 0;
		}
		else
		{
			const char *elements_type_string = "elements";
			const char *faces_type_string = "faces";
			const char *lines_type_string = "lines";
			const char *element_type_strings[3] = { elements_type_string, faces_type_string, lines_type_string };
			/* initialise defaults */
			char add_flag = 0;
			char remove_flag = 0;
			Cmiss_field_id conditional_field = 0;
			const char *element_type_string = elements_type_string;
			char all_flag = 0;
			char selected_flag = 0;
			Multi_range *element_ranges = CREATE(Multi_range)();
			char *from_group_name = 0;
			double time = 0;
			if (command_data->default_time_keeper)
			{
				time = Time_keeper_get_time(command_data->default_time_keeper);
			}
			int manage_subobjects = 1; // add faces, lines and nodes with elements, remove if solely in use by removed elements

			Option_table *option_table = CREATE(Option_table)();
			/* add */
			Option_table_add_char_flag_entry(option_table, "add", &add_flag);
			/* all */
			Option_table_add_char_flag_entry(option_table, "all", &all_flag);
			/* conditional_field */
			struct Set_Computed_field_conditional_data set_conditional_field_data;
			set_conditional_field_data.computed_field_manager =
				Computed_field_package_get_computed_field_manager(command_data->computed_field_package);
			set_conditional_field_data.conditional_function = 0;
			set_conditional_field_data.conditional_function_user_data = 0;
			Option_table_add_Computed_field_conditional_entry(option_table, "conditional_field",
				&conditional_field, &set_conditional_field_data);
			/* elements|faces|lines */
			Option_table_add_enumerator(option_table, 3, element_type_strings, &element_type_string);
			/* group */
			Option_table_add_string_entry(option_table, "group", &from_group_name, " GROUP_NAME");
			/* manage_subobjects|no_manage_subobjects */
			Option_table_add_switch(option_table, "manage_subobjects", "no_manage_subobjects", &manage_subobjects);
			/* remove */
			Option_table_add_char_flag_entry(option_table, "remove", &remove_flag);
			/* selected */
			Option_table_add_char_flag_entry(option_table, "selected", &selected_flag);
			/* default option: element number ranges */
			Option_table_add_entry(option_table, (char *)NULL, (void *)element_ranges,
				NULL, set_Multi_range);
			return_code = Option_table_multi_parse(option_table, state);
			DESTROY(Option_table)(&option_table);

			Cmiss_field_group_id from_group = 0;
			if (return_code)
			{
				if (from_group_name)
				{
					Cmiss_field *field =	FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
						from_group_name, Cmiss_region_get_Computed_field_manager(region));
					from_group = Cmiss_field_cast_group(field);
					if (!from_group)
					{
						display_message(ERROR_MESSAGE, "gfx modify egroup: '%s' is not a group.", from_group_name);
						return_code = 0;
					}
				}
				if ((add_flag + remove_flag) != 1)
				{
					display_message(ERROR_MESSAGE, "gfx modify egroup:  Must specify operation 'add' or 'remove'.");
					return_code = 0;
				}
				if ((0 == Multi_range_get_number_of_ranges(element_ranges)) && (!selected_flag) && (0 == conditional_field) && (!all_flag) && (0 == from_group))
				{
					display_message(ERROR_MESSAGE, "gfx modify egroup:  No elements specified.");
					return_code = 0;
				}
			}
			if (return_code)
			{
				int dimension = 0;
				if (element_type_string == lines_type_string)
				{
					dimension = 1;
				}
				else if (element_type_string == faces_type_string)
				{
					dimension = 2;
				}
				else
				{
					dimension = FE_region_get_highest_dimension(Cmiss_region_get_FE_region(region));
				}
				// following code
				return_code = process_modify_element_group(group, region, dimension,
					add_flag, conditional_field, from_group,
					Multi_range_get_number_of_ranges(element_ranges) > 0 ? element_ranges : 0,
					selected_flag, time, /*manage_nodes*/manage_subobjects, /*manage_faces*/manage_subobjects);
			}
			if (from_group_name)
			{
				DEALLOCATE(from_group_name);
			}
			Cmiss_field_group_destroy(&from_group);
			DESTROY(Multi_range)(&element_ranges);
			Cmiss_field_destroy(&conditional_field);
		}
		Cmiss_field_group_destroy(&group);
		Cmiss_region_destroy(&region);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_modify_element_group.  Invalid argument(s)");
		return_code = 0;
	}
	return (return_code);
}

/***************************************************************************//**
 * Renames a field.
 */
static int gfx_modify_field_rename(struct Parse_state *state,
	void *field_void, void *region_void)
{
	int return_code = 1;
	ENTER(gfx_modify_field_rename);
	Cmiss_field* field = (Cmiss_field*)field_void;
	USE_PARAMETER(region_void);
	if (state)
	{
		if (!state->current_token || (
			strcmp(PARSER_HELP_STRING,state->current_token) &&
			strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
		{
			if (state->current_token)
			{
				return_code = Cmiss_field_set_name(field, state->current_token);
			}
			else
			{
				display_message(ERROR_MESSAGE,"Missing new field name");
				display_parse_state_location(state);
				return_code = 0;
			}
		}
		else
		{
			/* Help */
			display_message(INFORMATION_MESSAGE, " NEW_FIELD_NAME");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_modify_field_rename.  Invalid argument(s)");
	}
	LEAVE;
	return (return_code);
}

/***************************************************************************//**
 * Executes GFX MODIFY FIELD subcommands.
 */
static int gfx_modify_field_subcommands(struct Parse_state *state,
	void *field_void, void *region_void)
{
	int return_code = 1;
	ENTER(gfx_modify_field_subcommands);
	if (state)
	{
		Option_table *option_table = CREATE(Option_table)();
		Option_table_add_entry(option_table, "rename",
			field_void, region_void, gfx_modify_field_rename);
		return_code = Option_table_parse(option_table, state);
		DESTROY(Option_table)(&option_table);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_modify_field_subcommands.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;
	return (return_code);
}

/***************************************************************************//**
 * Executes a GFX MODIFY FIELD command.
 */
static int gfx_modify_field(struct Parse_state *state,
	void *dummy_to_be_modified, void *command_data_void)
{
	int return_code = 1;
	ENTER(gfx_modify_field);
	USE_PARAMETER(dummy_to_be_modified);
	Cmiss_command_data *command_data = (struct Cmiss_command_data *)command_data_void;
	if (state && command_data)
	{
		const char *current_token = state->current_token;
		if (current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				Cmiss_field_id field = 0;
				Cmiss_region *region = 0;
				char *region_path = NULL;
				char *field_name = NULL;
				if (Cmiss_region_get_partial_region_path(command_data->root_region,
					current_token, &region, &region_path, &field_name))
				{
					Cmiss_field_module_id field_module = Cmiss_region_get_field_module(region);
					field = Cmiss_field_module_find_field_by_name(field_module, field_name);
					Cmiss_field_module_destroy(&field_module);
				}
				if (field)
				{
					shift_Parse_state(state, 1);
					return_code = gfx_modify_field_subcommands(state,
						(void *)field, (void *)region);
					Cmiss_field_destroy(&field);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"gfx modify field:  Invalid region path or field name '%s'", current_token);
					display_parse_state_location(state);
					return_code = 0;
				}
				DEALLOCATE(region_path);
				DEALLOCATE(field_name);
			}
			else
			{
				/* Write out the help */
				Option_table *help_option_table = CREATE(Option_table)();
				Option_table_add_entry(help_option_table,
					"[REGION_PATH" CMISS_REGION_PATH_SEPARATOR_STRING "]FIELD_NAME",
					(void *)NULL, (void *)command_data->root_region, gfx_modify_field_subcommands);
				return_code = Option_table_parse(help_option_table,state);
				DESTROY(Option_table)(&help_option_table);
				return_code = 1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing region_path/field_name");
			display_parse_state_location(state);
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_modify_field.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;
	return (return_code);
}

static int gfx_modify_g_element(struct Parse_state *state,
	void *help_mode,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 6 March 2003

DESCRIPTION :
Executes a GFX MODIFY G_ELEMENT command.
Parameter <help_mode> should be NULL when calling this function.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;

	ENTER(gfx_modify_g_element);
	if (state && (command_data = (struct Cmiss_command_data *)command_data_void))
	{
		return_code = 1;
		Cmiss_region_id region = Cmiss_region_access(command_data->root_region);
		Cmiss_field_group_id group = 0;
		if (!help_mode)
		{
			struct Option_table *option_table = CREATE(Option_table)();
			if (!state->current_token ||
				(strcmp(PARSER_HELP_STRING, state->current_token) &&
					strcmp(PARSER_RECURSIVE_HELP_STRING, state->current_token)))
			{
				Option_table_add_region_or_group_entry(option_table, /*token*/0, &region, &group);
			}
			else
			{
				/* help: call this function in help_mode */
				Option_table_add_entry(option_table, "REGION_PATH",
					/*help_mode*/(void *)1, command_data_void, gfx_modify_g_element);
			}
			return_code = Option_table_parse(option_table, state);
			DESTROY(Option_table)(&option_table);
		}
		if (return_code)
		{
			struct Cmiss_rendition *rendition = Cmiss_region_get_rendition_internal(region);
			return_code = Cmiss_rendition_execute_command_internal(rendition, group, state);
			Cmiss_rendition_destroy(&rendition);
		} /* parse error,help */
		Cmiss_region_destroy(&region);
		if (group)
		{
			Cmiss_field_group_destroy(&group);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_modify_g_element.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_modify_g_element */

static int gfx_modify_glyph(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 4 June 1999

DESCRIPTION :
Executes a GFX MODIFY GRAPHICS_OBJECT command.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;
	struct GT_object *graphics_object;
	struct Graphical_material *material;
	struct Option_table *option_table;
	struct Spectrum *spectrum;

	ENTER(gfx_modify_glyph);
	USE_PARAMETER(dummy_to_be_modified);
	if (state&&(command_data=(struct Cmiss_command_data *)command_data_void))
	{
		graphics_object=(struct GT_object *)NULL;
		if (!state->current_token||
			(strcmp(PARSER_HELP_STRING,state->current_token)&&
			strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
		{
			if (NULL != (graphics_object=FIND_BY_IDENTIFIER_IN_MANAGER(GT_object,name)
				(state->current_token,command_data->glyph_manager)))
			{
				shift_Parse_state(state,1);
				/* initialise defaults */
				if (NULL != (material = get_GT_object_default_material(graphics_object)))
				{
					ACCESS(Graphical_material)(material);
				}
				if (NULL != (spectrum = get_GT_object_spectrum(graphics_object)))
				{
					ACCESS(Spectrum)(spectrum);
				}
				option_table = CREATE(Option_table)();
				Option_table_add_set_Material_entry(option_table, "material",&material,
					command_data->material_package);
				Option_table_add_entry(option_table,"spectrum",&spectrum,
					command_data->spectrum_manager,set_Spectrum);
				return_code = Option_table_multi_parse(option_table, state);
				DESTROY(Option_table)(&option_table);
				if (return_code)
				{
					set_GT_object_default_material(graphics_object, material);
					set_GT_object_Spectrum(graphics_object, spectrum);
				}
				if (material)
				{
					DEACCESS(Graphical_material)(&material);
				}
				if (spectrum)
				{
					DEACCESS(Spectrum)(&spectrum);
				}
			}
			else
			{
				if (state->current_token)
				{
					display_message(ERROR_MESSAGE,"Could not find object named '%s'",
						state->current_token);
				}
				else
				{
					display_message(ERROR_MESSAGE,"Missing graphics object name");
				}
				display_parse_state_location(state);
				return_code=0;
			}
		}
		else
		{
			/* Help */
			display_message(INFORMATION_MESSAGE,
				"\n      GRAPHICS_OBJECT_NAME");

			spectrum = (struct Spectrum *)NULL;
			material = (struct Graphical_material *)NULL;
			option_table=CREATE(Option_table)();
			Option_table_add_set_Material_entry(option_table, "material",&material,
				command_data->material_package);
			Option_table_add_entry(option_table,"spectrum",&spectrum,
				command_data->spectrum_manager,set_Spectrum);
			return_code=Option_table_parse(option_table,state);
			DESTROY(Option_table)(&option_table);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_modify_glyph.  Missing command_data");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_modify_glyph */

static int gfx_modify_graphics_object(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 4 June 1999

DESCRIPTION :
Executes a GFX MODIFY GRAPHICS_OBJECT command.
==============================================================================*/
{
	int return_code = 0;

	ENTER(gfx_modify_graphics_object);
	USE_PARAMETER(dummy_to_be_modified);
	USE_PARAMETER(command_data_void);
	if (state)
	{
		display_message(ERROR_MESSAGE,
			"gfx_modify_graphics_object.  This function is no longer supported,"
			" please use gfx modify glyph instead");
	}
	LEAVE;

	return (return_code);
} /* gfx_modify_graphics_object */

/***************************************************************************//**
 * Modifies the membership of a group.  Only one of <add> or <remove> can
 * be specified at once.
 * If <use_data> flag is set, work with data, otherwise nodes.
 */
static int gfx_modify_node_group(struct Parse_state *state,
	void *use_data, void *command_data_void)
{
	int return_code = 0;
	Cmiss_command_data *command_data = reinterpret_cast<Cmiss_command_data *>(command_data_void);
	if (state && command_data)
	{
		Cmiss_region_id region = Cmiss_region_access(command_data->root_region);
		Cmiss_field_group_id group = 0;
		const char *current_token = state->current_token;
		if (!set_Cmiss_region_or_group(state, &region, &group))
		{
			// message already output
		}
		else if (!Parse_state_help_mode(state) && (!group))
		{
			display_message(ERROR_MESSAGE, "Not a group: %s", current_token);
			return_code = 0;
		}
		else
		{
			/* initialise defaults */
			char add_flag = 0;
			char remove_flag = 0;
			Cmiss_field_id conditional_field = 0;
			char all_flag = 0;
			char selected_flag = 0;
			Multi_range *node_ranges = CREATE(Multi_range)();
			char *from_group_name = 0;
			double time = 0;
			if (command_data->default_time_keeper)
			{
				time = Time_keeper_get_time(command_data->default_time_keeper);
			}

			Option_table *option_table = CREATE(Option_table)();
			/* add */
			Option_table_add_char_flag_entry(option_table, "add", &add_flag);
			/* all */
			Option_table_add_char_flag_entry(option_table, "all", &all_flag);
			/* conditional_field */
			struct Set_Computed_field_conditional_data set_conditional_field_data;
			set_conditional_field_data.computed_field_manager =
				Computed_field_package_get_computed_field_manager(command_data->computed_field_package);
			set_conditional_field_data.conditional_function = 0;
			set_conditional_field_data.conditional_function_user_data = 0;
			Option_table_add_Computed_field_conditional_entry(option_table, "conditional_field",
				&conditional_field, &set_conditional_field_data);
			/* group */
			Option_table_add_string_entry(option_table, "group", &from_group_name, " GROUP_NAME");
			/* remove */
			Option_table_add_char_flag_entry(option_table, "remove", &remove_flag);
			/* selected */
			Option_table_add_char_flag_entry(option_table, "selected", &selected_flag);
			/* default option: node number ranges */
			Option_table_add_entry(option_table, (char *)NULL, (void *)node_ranges,
				NULL, set_Multi_range);
			return_code = Option_table_multi_parse(option_table, state);
			DESTROY(Option_table)(&option_table);

			Cmiss_field_group_id from_group = 0;
			if (return_code)
			{
				if (from_group_name)
				{
					Cmiss_field *field =	FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
						from_group_name, Cmiss_region_get_Computed_field_manager(region));
					from_group = Cmiss_field_cast_group(field);
					if (!from_group)
					{
						display_message(ERROR_MESSAGE, "gfx modify ngroup: '%s' is not a group.", from_group_name);
						return_code = 0;
					}
				}
				if ((add_flag + remove_flag) != 1)
				{
					display_message(ERROR_MESSAGE, "gfx modify ngroup:  Must specify operation 'add' or 'remove'.");
					return_code = 0;
				}
				if ((0 == Multi_range_get_number_of_ranges(node_ranges)) && (!selected_flag) && (0 == conditional_field) && (!all_flag) && (0 == from_group))
				{
					display_message(ERROR_MESSAGE, "gfx modify ngroup:  No nodes specified.");
					return_code = 0;
				}
			}
			if (return_code)
			{
				Cmiss_field_module_id field_module = Cmiss_region_get_field_module(region);
				Cmiss_nodeset_id master_nodeset = Cmiss_field_module_find_nodeset_by_name(field_module,
					use_data ? "cmiss_data" : "cmiss_nodes");
				Cmiss_nodeset_group_id selection_nodeset_group = 0;
				if (selected_flag)
				{
					Cmiss_rendition *rendition = Cmiss_region_get_rendition_internal(region);
					Cmiss_field_group_id selection_group = Cmiss_rendition_get_selection_group(rendition);
					if (selection_group)
					{
						Cmiss_field_node_group_id selection_node_group =
							Cmiss_field_group_get_node_group(selection_group, master_nodeset);
						if (selection_node_group)
						{
							selection_nodeset_group = Cmiss_field_node_group_get_nodeset(selection_node_group);
							Cmiss_field_node_group_destroy(&selection_node_group);
						}
					}
					Cmiss_field_group_destroy(&selection_group);
					Cmiss_rendition_destroy(&rendition);
				}
				Cmiss_nodeset_group_id from_nodeset_group = 0;
				if (from_group)
				{
					Cmiss_field_node_group_id from_node_group = Cmiss_field_group_get_node_group(from_group, master_nodeset);
					if (from_node_group)
					{
						from_nodeset_group = Cmiss_field_node_group_get_nodeset(from_node_group);
						Cmiss_field_node_group_destroy(&from_node_group);
					}
				}
				int nodes_processed = 0;
				int nodes_not_processed = 0;
				if (((!selected_flag) || selection_nodeset_group) && ((!from_group) || from_nodeset_group))
				{
					Cmiss_field_module_begin_change(field_module);
					Cmiss_field_node_group_id modify_node_group = Cmiss_field_group_get_node_group(group, master_nodeset);
					if (!modify_node_group)
					{
						modify_node_group = Cmiss_field_group_create_node_group(group, master_nodeset);
					}
					Cmiss_nodeset_group_id modify_nodeset_group = Cmiss_field_node_group_get_nodeset(modify_node_group);
					Cmiss_field_node_group_destroy(&modify_node_group);
					Cmiss_field_cache_id cache = Cmiss_field_module_create_cache(field_module);
					Cmiss_field_cache_set_time(cache, time);

					Cmiss_nodeset_id iteration_nodeset = master_nodeset;
					Cmiss_nodeset_id selection_nodeset = Cmiss_nodeset_group_base_cast(selection_nodeset_group);
					Cmiss_nodeset_id from_nodeset = Cmiss_nodeset_group_base_cast(from_nodeset_group);
					if (selected_flag && !Cmiss_nodeset_match(selection_nodeset, Cmiss_nodeset_group_base_cast(modify_nodeset_group)))
					{
						iteration_nodeset = selection_nodeset;
					}
					if (from_nodeset && (!Cmiss_nodeset_match(from_nodeset, Cmiss_nodeset_group_base_cast(modify_nodeset_group))) &&
						Cmiss_nodeset_get_size(from_nodeset) < Cmiss_nodeset_get_size(iteration_nodeset))
					{
						iteration_nodeset = from_nodeset;
					}

					Cmiss_node_iterator_id iter = Cmiss_nodeset_create_node_iterator(iteration_nodeset);
					const bool use_node_ranges = Multi_range_get_number_of_ranges(node_ranges) > 0;
					Cmiss_node_id node = 0;
					while (NULL != (node = Cmiss_node_iterator_next_non_access(iter)))
					{
						if (use_node_ranges && !Multi_range_is_value_in_range(node_ranges, Cmiss_node_get_identifier(node)))
							continue;
						if (selection_nodeset && (selection_nodeset != iteration_nodeset) && !Cmiss_nodeset_contains_node(selection_nodeset, node))
							continue;
						if (from_nodeset && (from_nodeset != iteration_nodeset) && !Cmiss_nodeset_contains_node(from_nodeset, node))
							continue;
						if (conditional_field)
						{
							Cmiss_field_cache_set_node(cache, node);
							if (!Cmiss_field_evaluate_boolean(conditional_field, cache))
								continue;
						}
						++nodes_processed;
						if (add_flag)
						{
							if (!Cmiss_nodeset_contains_node(Cmiss_nodeset_group_base_cast(modify_nodeset_group), node))
							{
								if (!Cmiss_nodeset_group_add_node(modify_nodeset_group, node))
								{
									display_message(ERROR_MESSAGE,
										"gfx modify ngroup:  Could not add node %d", Cmiss_node_get_identifier(node));
									return_code = 0;
									break;
								}
							}
						}
						else
						{
							if (Cmiss_nodeset_contains_node(Cmiss_nodeset_group_base_cast(modify_nodeset_group), node))
							{
								if (!Cmiss_nodeset_group_remove_node(modify_nodeset_group, node))
								{
									display_message(ERROR_MESSAGE,
										"gfx modify ngroup:  Could not remove node %d", Cmiss_node_get_identifier(node));
									return_code = 0;
									break;
								}
							}
						}
					}
					Cmiss_node_iterator_destroy(&iter);
					Cmiss_field_cache_destroy(&cache);
					Cmiss_nodeset_group_destroy(&modify_nodeset_group);
					Cmiss_field_node_group_destroy(&modify_node_group);
					Cmiss_field_module_end_change(field_module);
				}
				if (0 < nodes_not_processed)
				{
					display_message(WARNING_MESSAGE,
						"gfx modify ngroup:  %d nodes could not be removed", nodes_not_processed);
				}
				else if (0 == nodes_processed)
				{
					display_message(WARNING_MESSAGE, "gfx modify ngroup:  group unchanged");
				}
				Cmiss_nodeset_group_destroy(&from_nodeset_group);
				Cmiss_nodeset_group_destroy(&selection_nodeset_group);
				Cmiss_nodeset_destroy(&master_nodeset);
				Cmiss_field_module_destroy(&field_module);
			}
			if (from_group_name)
			{
				DEALLOCATE(from_group_name);
			}
			Cmiss_field_group_destroy(&from_group);
			DESTROY(Multi_range)(&node_ranges);
			Cmiss_field_destroy(&conditional_field);
		}
		Cmiss_field_group_destroy(&group);
		Cmiss_region_destroy(&region);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_modify_node_group.  Invalid argument(s)");
		return_code = 0;
	}
	return (return_code);
}

struct Cmiss_nodal_derivatives_data
{
	int number_of_derivatives; // initialise to -1
	enum Cmiss_nodal_value_type *derivatives; // initialise to NULL
};

/***************************************************************************//**
 * Modifier function for entering the derivatives types to be defined in a new
 * node field.
 * Idea: could make it re-enter to be different for different components.
 */
static int set_Cmiss_nodal_derivatives(struct Parse_state *state,
	void *derivatives_data_void, void *dummy_void)
{
	int return_code = 1;
	USE_PARAMETER(dummy_void);
	Cmiss_nodal_derivatives_data *derivatives_data = reinterpret_cast<Cmiss_nodal_derivatives_data *>(derivatives_data_void);
	if (state && derivatives_data && (0 > derivatives_data->number_of_derivatives))
	{
		if (0 == state->current_token)
		{
			display_message(ERROR_MESSAGE, "Missing derivatives");
			display_parse_state_location(state);
			return_code = 0;
		}
		else if (Parse_state_help_mode(state))
		{
			display_message(INFORMATION_MESSAGE, " DERIVATIVE_NAMES(D_DS1 D2_DS1DS2 etc.)|none[none]");
		}
		else if (fuzzy_string_compare("none", state->current_token))
		{
			return_code = shift_Parse_state(state, 1);
			derivatives_data->number_of_derivatives = 0;
		}
		else
		{
			derivatives_data->number_of_derivatives = 0;
			while (state->current_token)
			{
				// stop when derivatives not recognised
				enum Cmiss_nodal_value_type nodal_value_type =
					Cmiss_nodal_value_type_enum_from_string(state->current_token);
				if (nodal_value_type != CMISS_NODAL_VALUE_TYPE_INVALID)
				{
					enum Cmiss_nodal_value_type *temp;
					if (REALLOCATE(temp, derivatives_data->derivatives, enum Cmiss_nodal_value_type,
						derivatives_data->number_of_derivatives + 1))
					{
						derivatives_data->derivatives = temp;
						derivatives_data->derivatives[derivatives_data->number_of_derivatives] = nodal_value_type;
						++derivatives_data->number_of_derivatives;
						return_code = shift_Parse_state(state, 1);
					}
					else
					{
						return_code = 0;
						break;
					}
				}
				else
				{
					if (0 == derivatives_data->number_of_derivatives)
					{
						display_message(ERROR_MESSAGE, "Unrecognised derivative %s", state->current_token);
						display_parse_state_location(state);
						return_code = 0;
					}
					break;
				}
			}
		}
	}
	else
	{
		if (derivatives_data && (0 <= derivatives_data->number_of_derivatives))
		{
			display_message(ERROR_MESSAGE, "Derivatives have already been set");
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"set_Cmiss_nodal_derivatives.  Invalid argument(s)");
		}
		return_code = 0;
	}
	return (return_code);
}

/***************************************************************************//**
 * Executes a GFX MODIFY NODES/DATA command.
 * If <use_data_flag> is set, use cmiss_data, otherwise cmiss_nodes.
 */
static int gfx_modify_nodes(struct Parse_state *state,
	void *use_data, void *command_data_void)
{
	int return_code;
	Cmiss_command_data *command_data = reinterpret_cast<Cmiss_command_data *>(command_data_void);
	if (state && command_data)
	{
		/* initialise defaults */
		Cmiss_region_id region = Cmiss_region_access(command_data->root_region);
		Cmiss_field_group_id group = 0;
		char all_flag = 0;
		char selected_flag = 0;
		Multi_range *node_ranges = CREATE(Multi_range)();
		char *define_field_name = 0;
		char *undefine_field_name = 0;
		char *conditional_field_name = 0;
		Cmiss_nodal_derivatives_data derivatives_data;
		derivatives_data.number_of_derivatives = -1; // not set, = 0
		derivatives_data.derivatives = 0;
		int number_of_versions = 1;
		FE_value time = 0.0;
		if (command_data->default_time_keeper)
		{
			time = Time_keeper_get_time(command_data->default_time_keeper);
		}

		Option_table *option_table = CREATE(Option_table)();
		/* all */
		Option_table_add_entry(option_table, "all", &all_flag, NULL, set_char_flag);
		/* conditional_field */
		Option_table_add_string_entry(option_table,"conditional_field", &conditional_field_name,
			" FIELD_NAME");
		/* define */
		Option_table_add_string_entry(option_table, "define", &define_field_name,
			" FIELD_NAME");
		/* derivatives */
		Option_table_add_entry(option_table, "derivatives",
			(void *)&derivatives_data, (void *)0, set_Cmiss_nodal_derivatives);
		/* group */
		Option_table_add_region_or_group_entry(option_table, "group", &region, &group);
		/* selected */
		Option_table_add_entry(option_table, "selected", &selected_flag,
			NULL, set_char_flag);
		/* undefine */
		Option_table_add_string_entry(option_table, "undefine", &undefine_field_name,
			" FIELD_NAME");
		/* versions */
		Option_table_add_int_positive_entry(option_table, "versions", &number_of_versions);
		/* default option: node number ranges */
		Option_table_add_entry(option_table, (char *)NULL, (void *)node_ranges,
			NULL, set_Multi_range);
		return_code = Option_table_multi_parse(option_table, state);
		DESTROY(Option_table)(&option_table);

		if (return_code)
		{
			if ((0 == Multi_range_get_number_of_ranges(node_ranges)) && (!selected_flag) &&
				(0 == conditional_field_name) && (!all_flag) && (!group))
			{
				display_message(ERROR_MESSAGE, "gfx modify nodes:  No nodes specified.");
				return_code = 0;
			}
		}
		Cmiss_field_id conditional_field = 0;
		if (return_code && conditional_field_name)
		{
			Cmiss_field_module_id field_module = Cmiss_region_get_field_module(region);
			conditional_field = Cmiss_field_module_find_field_by_name(field_module, conditional_field_name);
			if (!conditional_field)
			{
				display_message(ERROR_MESSAGE,
					"gfx modify nodes:  conditional field cannot be found");
				return_code = 0;
			}
			Cmiss_field_module_destroy(&field_module);
		}
		if (return_code)
		{
			if (define_field_name && undefine_field_name)
			{
				display_message(WARNING_MESSAGE,
					"gfx modify nodes:  Only specify one of define or undefine field");
				return_code = 0;
			}
			if ((!define_field_name) && (!undefine_field_name))
			{
				display_message(WARNING_MESSAGE,
					"gfx modify nodes:  Must specify define or undefine field");
				return_code = 0;
			}
		}
		if (return_code)
		{
			Cmiss_field_module_id field_module = Cmiss_region_get_field_module(region);
			Cmiss_field_group_id selection_group = NULL;
			if (selected_flag)
			{
				Cmiss_rendition *rendition = Cmiss_region_get_rendition_internal(region);
				if (rendition)
				{
					selection_group = Cmiss_rendition_get_selection_group(rendition);
					Cmiss_rendition_destroy(&rendition);
				}
			}
			const char *field_name = define_field_name ? define_field_name : undefine_field_name;
			Cmiss_field_id field = Cmiss_field_module_find_field_by_name(field_module, field_name);
			if (!field)
			{
				display_message(ERROR_MESSAGE, "gfx modify nodes:  Cannot find field %s", field_name);
				return_code = 0;
			}
			Cmiss_nodeset_id nodeset = Cmiss_field_module_find_nodeset_by_name(field_module,
				use_data ? "cmiss_data" : "cmiss_nodes");
			Cmiss_node_template_id node_template = Cmiss_nodeset_create_node_template(nodeset);
			if (define_field_name)
			{
				if (!Cmiss_node_template_define_field(node_template, field))
				{
					return_code = 0;
				}
				else
				{
					for (int i = 0; i < derivatives_data.number_of_derivatives; ++i)
					{
						if (!Cmiss_node_template_define_derivative(node_template, field,
							/*component_number=all*/-1, derivatives_data.derivatives[i]))
						{
							return_code = 0;
							break;
						}
					}
					if ((number_of_versions > 1) && !Cmiss_node_template_define_versions(node_template, field,
						/*component_number=all*/-1, number_of_versions))
					{
						return_code = 0;
					}
				}
			}
			else
			{
				if (!Cmiss_node_template_undefine_field(node_template, field))
					return_code = 0;
			}
			if (group)
			{
				Cmiss_field_node_group_id node_group = Cmiss_field_group_get_node_group(group, nodeset);
				Cmiss_nodeset_destroy(&nodeset);
				nodeset = Cmiss_nodeset_group_base_cast(Cmiss_field_node_group_get_nodeset(node_group));
				Cmiss_field_node_group_destroy(&node_group);
			}
			int nodes_processed = 0;
			if (return_code && nodeset && ((!selected_flag) || selection_group))
			{
				Cmiss_field_module_begin_change(field_module);
				Cmiss_field_cache_id cache = Cmiss_field_module_create_cache(field_module);
				Cmiss_field_cache_set_time(cache, time);
				Cmiss_node_iterator_id iter = Cmiss_nodeset_create_node_iterator(nodeset);
				const bool use_node_ranges = Multi_range_get_number_of_ranges(node_ranges) > 0;
				Cmiss_node_id node = 0;
				while (NULL != (node = Cmiss_node_iterator_next_non_access(iter)))
				{
					if (use_node_ranges && !Multi_range_is_value_in_range(node_ranges, Cmiss_node_get_identifier(node)))
						continue;
					if (conditional_field || selection_group)
					{
						Cmiss_field_cache_set_node(cache, node);
					}
					if (conditional_field && !Cmiss_field_evaluate_boolean(conditional_field, cache))
						continue;
					if (selection_group && !Cmiss_field_evaluate_boolean(Cmiss_field_group_base_cast(selection_group), cache))
						continue;
					if (!Cmiss_node_merge(node, node_template))
					{
						display_message(WARNING_MESSAGE, "gfx modify nodes:  Failed to merge node");
						return_code = 0;
						break;
					}
					++nodes_processed;
				}
				Cmiss_node_iterator_destroy(&iter);
				Cmiss_field_cache_destroy(&cache);
				Cmiss_field_module_end_change(field_module);
			}
			if (return_code && (0 == nodes_processed))
			{
				if (use_data)
				{
					display_message(WARNING_MESSAGE, "gfx modify data:  No data specified");
				}
				else
				{
					display_message(WARNING_MESSAGE, "gfx modify nodes:  No nodes specified");
				}
			}
			if (node_template)
				Cmiss_node_template_destroy(&node_template);
			Cmiss_nodeset_destroy(&nodeset);
			Cmiss_field_destroy(&field);
			if (selection_group)
				Cmiss_field_group_destroy(&selection_group);
			Cmiss_field_module_destroy(&field_module);
		}
		DESTROY(Multi_range)(&node_ranges);
		if (define_field_name)
		{
			DEALLOCATE(define_field_name);
		}
		if (undefine_field_name)
		{
			DEALLOCATE(undefine_field_name);
		}
		if (conditional_field_name)
		{
			DEALLOCATE(conditional_field_name);
		}
		if (derivatives_data.derivatives)
		{
			DEALLOCATE(derivatives_data.derivatives);
		}
		if (conditional_field)
		{
			Cmiss_field_destroy(&conditional_field);
		}
		Cmiss_field_group_destroy(&group);
		Cmiss_region_destroy(&region);
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_modify_nodes.  Invalid argument(s)");
		return_code = 0;
	}
	return (return_code);
}

static int execute_command_gfx_modify(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 26 March 2001

DESCRIPTION :
Executes a GFX MODIFY command.
???DB.  Part of GFX EDIT ?
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Modify_environment_map_data modify_environment_map_data;
#if defined (USE_CMGUI_GRAPHICS_WINDOW)
	struct Modify_graphics_window_data modify_graphics_window_data;
#endif /* defined (USE_CMGUI_GRAPHICS_WINDOW) */
	struct Modify_light_data modify_light_data;
	struct Modify_light_model_data modify_light_model_data;
	struct Option_table *option_table;

	ENTER(execute_command_gfx_modify);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if (NULL != (command_data = (struct Cmiss_command_data *)command_data_void))
		{
			if (state->current_token)
			{
				option_table=CREATE(Option_table)();
				/* data */
				Option_table_add_entry(option_table, "data", /*use_data*/(void *)1,
					(void *)command_data, gfx_modify_nodes);
				/* dgroup */
				Option_table_add_entry(option_table,"dgroup",(void *)1/*data*/,
					(void *)command_data, gfx_modify_node_group);
				/* egroup */
				Option_table_add_entry(option_table,"egroup",NULL,
					(void *)command_data, gfx_modify_element_group);
				/* emoter */
				Option_table_add_entry(option_table,"emoter",NULL,
					(void *)command_data->emoter_slider_dialog,
					gfx_modify_emoter);
				/* environment_map */
				modify_environment_map_data.graphical_material_manager=
					Material_package_get_material_manager(command_data->material_package);
				modify_environment_map_data.environment_map_manager=
					command_data->environment_map_manager;
				Option_table_add_entry(option_table,"environment_map",NULL,
					(&modify_environment_map_data),modify_Environment_map);
				/* field */
				Option_table_add_entry(option_table,"field",NULL,
					(void *)command_data, gfx_modify_field);
				/* flow_particles */
				Option_table_add_entry(option_table,"flow_particles",NULL,
					(void *)command_data, gfx_modify_flow_particles);
				/* g_element */
				Option_table_add_entry(option_table,"g_element",NULL,
					(void *)command_data, gfx_modify_g_element);
				/* glyph */
				Option_table_add_entry(option_table,"glyph",NULL,
					(void *)command_data, gfx_modify_glyph);
				/* graphics_object */
				Option_table_add_entry(option_table,"graphics_object",NULL,
					(void *)command_data, gfx_modify_graphics_object);
				/* light */
				modify_light_data.default_light=command_data->default_light;
				modify_light_data.light_manager=command_data->light_manager;
				Option_table_add_entry(option_table,"light",NULL,
					(void *)(&modify_light_data), modify_Light);
				/* lmodel */
				modify_light_model_data.default_light_model=
					command_data->default_light_model;
				modify_light_model_data.light_model_manager=
					command_data->light_model_manager;
				Option_table_add_entry(option_table,"lmodel",NULL,
					(void *)(&modify_light_model_data), modify_Light_model);
				/* material */
				Option_table_add_entry(option_table,"material",NULL,
					(void *)command_data->material_package, modify_Graphical_material);
				/* ngroup */
				Option_table_add_entry(option_table,"ngroup",NULL,
					(void *)command_data, gfx_modify_node_group);
				/* nodes */
				Option_table_add_entry(option_table, "nodes", /*use_data*/(void *)0,
					(void *)command_data, gfx_modify_nodes);
				/* scene */
				Define_scene_data define_scene_data;
				define_scene_data.light_manager=command_data->light_manager;
				define_scene_data.scene_manager=command_data->scene_manager;
				define_scene_data.root_region = command_data->root_region;
				define_scene_data.graphics_module = command_data->graphics_module;
				Option_table_add_entry(option_table, "scene", NULL,
					(void *)(&define_scene_data), define_Scene);
				/* spectrum */
				Option_table_add_entry(option_table,"spectrum",NULL,
					(void *)command_data, gfx_modify_Spectrum);
				/* texture */
				Option_table_add_entry(option_table,"texture",NULL,
					(void *)command_data, gfx_modify_Texture);
#if defined (USE_CMGUI_GRAPHICS_WINDOW)
				/* window */
				modify_graphics_window_data.computed_field_package=
					command_data->computed_field_package;
				modify_graphics_window_data.graphics_window_manager=
					command_data->graphics_window_manager;
				modify_graphics_window_data.interactive_tool_manager=
					command_data->interactive_tool_manager;
				modify_graphics_window_data.light_manager=command_data->light_manager;
				modify_graphics_window_data.light_model_manager=
					command_data->light_model_manager;
				modify_graphics_window_data.scene_manager=command_data->scene_manager;
				modify_graphics_window_data.root_region=command_data->root_region;
				Option_table_add_entry(option_table,"window",NULL,
					(void *)(&modify_graphics_window_data), modify_Graphics_window);
#endif /* defined (USE_CMGUI_GRAPHICS_WINDOW) */

				return_code=Option_table_parse(option_table,state);
				DESTROY(Option_table)(&option_table);
			}
			else
			{
				set_command_prompt("gfx modify",command_data);
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_gfx_modify.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_gfx_modify.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_modify */

#if defined (GTK_USER_INTERFACE) || defined (WIN32_USER_INTERFACE) || defined (CARBON_USER_INTERFACE) || defined (WX_USER_INTERFACE)
static int execute_command_gfx_node_tool(struct Parse_state *state,
	void *data_tool_flag, void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 19 February 2008

DESCRIPTION :
Executes a GFX NODE_TOOL or GFX_DATA_TOOL command. If <data_tool_flag> is set,
then the <data_tool> is being modified, otherwise the <node_tool>.
Which tool that is being modified is passed in <node_tool_void>.
==============================================================================*/
{
	struct Cmiss_command_data *command_data;
	struct Node_tool *node_tool;
	int return_code = 0;
	ENTER(execute_command_gfx_node_tool);
	if (state && (command_data=(struct Cmiss_command_data *)command_data_void))
	{
		if (data_tool_flag)
		{
			node_tool = command_data->data_tool;
		}
		else
		{
			node_tool = command_data->node_tool;
		}
		return_code = Node_tool_execute_command_with_parse_state(node_tool, state);
		if (node_tool)
		{
#if defined (WX_USER_INTERFACE)
			FOR_EACH_OBJECT_IN_MANAGER(Graphics_window)(
				Graphics_window_update_Interactive_tool,
				(void *)Node_tool_get_interactive_tool(node_tool),
				command_data->graphics_window_manager);
			display_message(WARNING_MESSAGE,
				"This command changes the node tool settings for each window to the global settings. To change node tool settings for individual window, please see the command [gfx modify window <name> nodes ?]. \n");
#endif /*(WX_USER_INTERFACE)*/
			Cmiss_scene_viewer_package_update_Interactive_tool(
				command_data->scene_viewer_package,
				Node_tool_get_interactive_tool(node_tool));
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_gfx_node_tool.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_node_tool */
#endif /* defined (GTK_USER_INTERFACE) || defined
			  (WIN32_USER_INTERFACE) || defined (CARBON_USER_INTERFACE) || defined(WX_USER_INTERFACE */

#if defined (GTK_USER_INTERFACE) || defined (WIN32_USER_INTERFACE) || defined (WX_USER_INTERFACE)
static int execute_command_gfx_print(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 23 April 2002

DESCRIPTION :
Executes a GFX PRINT command.
==============================================================================*/
{
	char *file_name, force_onscreen_flag;
	const char*image_file_format_string, **valid_strings;
	enum Image_file_format image_file_format;
	enum Texture_storage_type storage;
	int antialias, height, number_of_valid_strings, return_code,
		transparency_layers, width;
	struct Cmgui_image *cmgui_image;
	struct Cmgui_image_information *cmgui_image_information;
	struct Cmiss_command_data *command_data;
	struct Graphics_window *window;
	struct Option_table *option_table;

	ENTER(execute_command_gfx_print);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (command_data=(struct Cmiss_command_data *)command_data_void))
	{
		/* initialize defaults */
		antialias = -1;
		file_name = (char *)NULL;
		height = 0;
		force_onscreen_flag = 0;
		storage = TEXTURE_RGBA;
		transparency_layers = 0;
		width = 0;
		/* default file format is to obtain it from the filename extension */
		image_file_format = UNKNOWN_IMAGE_FILE_FORMAT;
		/* must have at least one graphics_window to print */
		if (NULL != (window = FIRST_OBJECT_IN_MANAGER_THAT(
			Graphics_window)((MANAGER_CONDITIONAL_FUNCTION(Graphics_window) *)NULL,
				(void *)NULL, command_data->graphics_window_manager)))
		{
			ACCESS(Graphics_window)(window);
		}

		option_table = CREATE(Option_table)();
		/* antialias */
		Option_table_add_entry(option_table, "antialias",
			&antialias, NULL, set_int_positive);
		/* image file format */
		image_file_format_string =
			ENUMERATOR_STRING(Image_file_format)(image_file_format);
		valid_strings = ENUMERATOR_GET_VALID_STRINGS(Image_file_format)(
			&number_of_valid_strings,
			(ENUMERATOR_CONDITIONAL_FUNCTION(Image_file_format) *)NULL,
			(void *)NULL);
		Option_table_add_enumerator(option_table, number_of_valid_strings,
			valid_strings, &image_file_format_string);
		DEALLOCATE(valid_strings);
		/* file */
		Option_table_add_entry(option_table, "file", &file_name,
			(void *)1, set_name);
		/* force_onscreen */
		Option_table_add_entry(option_table, "force_onscreen",
			&force_onscreen_flag, NULL, set_char_flag);
		/* format */
		Option_table_add_entry(option_table, "format", &storage,
			NULL, set_Texture_storage);
		/* height */
		Option_table_add_entry(option_table, "height",
			&height, NULL, set_int_non_negative);
		/* transparency_layers */
		Option_table_add_entry(option_table, "transparency_layers",
			&transparency_layers, NULL, set_int_positive);
		/* width */
		Option_table_add_entry(option_table, "width",
			&width, NULL, set_int_non_negative);
		/* window */
		Option_table_add_entry(option_table, "window",
			&window, command_data->graphics_window_manager, set_Graphics_window);

		return_code = Option_table_multi_parse(option_table, state);
		DESTROY(Option_table)(&option_table);
		/* no errors, not asking for help */
		if (return_code)
		{
			if (!file_name)
			{
				if (!(file_name = confirmation_get_write_filename(NULL,
								 command_data->user_interface
#if defined (WX_USER_INTERFACE)
							 , command_data->execute_command
#endif /* defined (WX_USER_INTERFACE)*/
)))
				{
					display_message(ERROR_MESSAGE, "gfx print:  No file name specified");
					return_code = 0;
				}
			}
			if (!window)
			{
				display_message(ERROR_MESSAGE,
					"gfx print:  No graphics windows to print");
				return_code = 0;
			}
		}
		if (return_code)
		{
			cmgui_image_information = CREATE(Cmgui_image_information)();
			if (image_file_format_string)
			{
				STRING_TO_ENUMERATOR(Image_file_format)(
					image_file_format_string, &image_file_format);
			}
			else
			{
				image_file_format = UNKNOWN_IMAGE_FILE_FORMAT;
			}
			Cmgui_image_information_set_image_file_format(
				cmgui_image_information, image_file_format);
			Cmgui_image_information_add_file_name(cmgui_image_information,
				file_name);
			Cmgui_image_information_set_io_stream_package(cmgui_image_information,
				command_data->io_stream_package);
			if (NULL != (cmgui_image = Graphics_window_get_image(window,
				force_onscreen_flag, width, height, antialias,
				transparency_layers, storage)))
			{
				if (!Cmgui_image_write(cmgui_image, cmgui_image_information))
				{
					display_message(ERROR_MESSAGE,
						"gfx print:  Error writing image %s", file_name);
					return_code = 0;
				}
				DESTROY(Cmgui_image)(&cmgui_image);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"execute_command_gfx_print.  Could not get image from window");
				return_code = 0;
			}
			DESTROY(Cmgui_image_information)(&cmgui_image_information);
		}
		if (window)
		{
			DEACCESS(Graphics_window)(&window);
		}
		if (file_name)
		{
			DEALLOCATE(file_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_gfx_print.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_print */
#endif

static int gfx_read_Curve(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 7 January 2002

DESCRIPTION :
Reads a curve from 3 files: ~.curve.com, ~.curve.exnode, ~.curve.exelem, where
~ is the name of the curve/file specified here.
Works by executing the .curve.com file, which should have a gfx define curve
instruction to read in the mesh.
==============================================================================*/
{
	char *file_name;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Option_table *option_table;

	ENTER(gfx_read_Curve);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (NULL != (command_data = (struct Cmiss_command_data *)command_data_void))
		{
			file_name = (char *)NULL;
			option_table = CREATE(Option_table)();
			/* example */
			Option_table_add_entry(option_table, CMGUI_EXAMPLE_DIRECTORY_SYMBOL,
				&file_name, &(command_data->example_directory),
				set_file_name);
			/* default */
			Option_table_add_entry(option_table, NULL, &file_name,
				NULL, set_file_name);
			return_code = Option_table_multi_parse(option_table, state);
			DESTROY(Option_table)(&option_table);
			if (return_code)
			{
				if (!file_name)
				{
					if (!(file_name = confirmation_get_read_filename(".curve.com",
									 command_data->user_interface
#if defined (WX_USER_INTERFACE)
									 , command_data->execute_command
#endif /*defined (WX_USER_INTERFACE)*/
																													 )))
					{
						return_code = 0;
					}
				}
#if defined (WX_USER_INTERFACE) && defined (WIN32_SYSTEM)
			if (file_name)
			{
				 file_name = CMISS_set_directory_and_filename_WIN32(file_name,
						command_data);
			}
#endif /* defined (WIN32_SYSTEM)*/
				if (return_code)
				{
					/* open the file */
					if (0 != (return_code = check_suffix(&file_name, ".curve.com")))
					{
						return_code=execute_comfile(file_name, command_data->io_stream_package,
							 command_data->execute_command);
					}
				}
			}
			DEALLOCATE(file_name);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_read_Curve.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_read_Curve.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_read_Curve */

int offset_region_identifier(Cmiss_region_id region, char element_flag, int element_offset,
	char face_flag, int face_offset, char line_flag, int line_offset, char node_flag, int node_offset, int use_data)
{
	int return_code = 1;
	struct FE_region *fe_region = NULL;
	if (region)
		fe_region = Cmiss_region_get_FE_region(region);
	/* Offset these nodes and elements before merging */
	if (fe_region)
	{
		FE_region_begin_change(fe_region);
		int highest_dimension = FE_region_get_highest_dimension(fe_region);
		if (element_flag)
		{
			if (!FE_region_change_element_identifiers(fe_region,
				highest_dimension, element_offset,
				(struct Computed_field *)NULL, /*time*/0))
			{
				return_code = 0;
			}
		}
		if (face_flag && (highest_dimension > 2))
		{
			if (!FE_region_change_element_identifiers(fe_region,
				/*dimension*/2, face_offset,
				(struct Computed_field *)NULL, /*time*/0))
			{
				return_code = 0;
			}
		}
		if (line_flag && (highest_dimension > 1))
		{
			if (!FE_region_change_element_identifiers(fe_region,
				/*dimension*/1, line_offset,
				(struct Computed_field *)NULL, /*time*/0))
			{
				return_code = 0;
			}
		}
		if (node_flag)
		{
			if (!use_data)
			{
				if (!FE_region_change_node_identifiers(fe_region,
					node_offset, (struct Computed_field *)NULL, /*time*/0))
				{
					return_code = 0;
				}
			}
			else
			{
				struct FE_region *data_fe_region = FE_region_get_data_FE_region(fe_region);
				FE_region_begin_change(data_fe_region);
				if (!FE_region_change_node_identifiers(data_fe_region,
					node_offset, (struct Computed_field *)NULL, /*time*/0))
				{
					return_code = 0;
				}
				FE_region_end_change(data_fe_region);
			}
		}
		FE_region_end_change(fe_region);
		struct Cmiss_region *child_region = Cmiss_region_get_first_child(region);
		while ((NULL != child_region))
		{
			return_code = offset_region_identifier(child_region, element_flag, element_offset, face_flag,
				face_offset, line_flag, line_offset, node_flag, node_offset, use_data);
			Cmiss_region_reaccess_next_sibling(&child_region);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Unable to get fe_region to offset nodes or elements in file .");
		return_code = 0;
	}
	return return_code;
}

static int gfx_read_elements(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 6 March 2003

DESCRIPTION :
If an element file is not specified a file selection box is presented to the
user, otherwise the elements file is read.
==============================================================================*/
{
	char *file_name, *region_path,
		element_flag, face_flag, line_flag, node_flag;
	int element_offset, face_offset, line_offset, node_offset,
		return_code;
	struct Cmiss_command_data *command_data;
	struct Cmiss_region *region, *top_region;
	struct IO_stream *input_file;
	struct Option_table *option_table;

	ENTER(gfx_read_elements);
	USE_PARAMETER(dummy_to_be_modified);
	input_file = NULL;
	if (state && (command_data = (struct Cmiss_command_data *)command_data_void))
	{
		element_flag = 0;
		element_offset = 0;
		face_flag = 0;
		face_offset = 0;
		line_flag = 0;
		line_offset = 0;
		node_flag = 0;
		node_offset = 0;
		file_name = (char *)NULL;
		region_path = (char *)NULL;
		option_table = CREATE(Option_table)();
		/* element_offset */
		Option_table_add_entry(option_table, "element_offset", &element_offset,
			&element_flag, set_int_and_char_flag);
		/* example */
		Option_table_add_entry(option_table,CMGUI_EXAMPLE_DIRECTORY_SYMBOL,
			&file_name, &(command_data->example_directory), set_file_name);
		/* face_offset */
		Option_table_add_entry(option_table, "face_offset", &face_offset,
			&face_flag, set_int_and_char_flag);
		/* line_offset */
		Option_table_add_entry(option_table, "line_offset", &line_offset,
			&line_flag, set_int_and_char_flag);
		/* node_offset */
		Option_table_add_entry(option_table, "node_offset", &node_offset,
			&node_flag, set_int_and_char_flag);
		/* region */
		Option_table_add_entry(option_table,"region",
			&region_path, (void *)1, set_name);
		/* default */
		Option_table_add_entry(option_table,NULL,&file_name,
			NULL,set_file_name);
		return_code = Option_table_multi_parse(option_table, state);
		if (return_code)
		{
			if (!file_name)
			{
				if (!(file_name = confirmation_get_read_filename(".exelem",
								 command_data->user_interface
#if defined(WX_USER_INTERFACE)
								 , command_data->execute_command
#endif /*defined (WX_USER_INTERFACE) */
		 )))
				{
					return_code = 0;
				}
			}
#if defined (WX_USER_INTERFACE) && defined (WIN32_SYSTEM)
			if (file_name)
			{
				 file_name = CMISS_set_directory_and_filename_WIN32(file_name,
						command_data);
			}
#endif /* defined (WIN32_SYSTEM)*/

			if (return_code)
			{
				if (!check_suffix(&file_name,".exelem"))
				{
					return_code = 0;
				}
			}
			top_region = (struct Cmiss_region *)NULL;
			if (region_path)
			{
				top_region = Cmiss_region_find_subregion_at_path(
					command_data->root_region, region_path);
				if (NULL == top_region)
				{
					top_region = Cmiss_region_create_subregion(
						command_data->root_region, region_path);
					if (NULL == top_region)
					{
						display_message(ERROR_MESSAGE, "gfx_read_elements.  "
							"Unable to find or create region '%s'.", region_path);
						return_code = 0;
					}
				}
			}
			else
			{
				top_region = ACCESS(Cmiss_region)(command_data->root_region);
			}
			if (return_code)
			{
				/* open the file */
				if ((input_file = CREATE(IO_stream)(command_data->io_stream_package))
					&& (IO_stream_open_for_read(input_file, file_name)))
				{
					region = Cmiss_region_create_region(top_region);
					if (read_exregion_file(region, input_file,
						(struct FE_import_time_index *)NULL))
					{
						if (element_flag || face_flag || line_flag || node_flag)
						{
							return_code = offset_region_identifier(region, element_flag, element_offset, face_flag,
								face_offset, line_flag, line_offset, node_flag, node_offset, /*use_data*/0);
						}
						if (return_code)
						{
							if (Cmiss_region_can_merge(top_region, region))
							{
								if (!Cmiss_region_merge(top_region, region))
								{
									display_message(ERROR_MESSAGE,
										"Error merging elements from file: %s", file_name);
									return_code = 0;
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"Contents of file %s not compatible with global objects",
									file_name);
								return_code = 0;
							}
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Error reading element file: %s", file_name);
						return_code = 0;
					}
					DEACCESS(Cmiss_region)(&region);
					IO_stream_close(input_file);
					DESTROY(IO_stream)(&input_file);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Could not open element file: %s", file_name);
					return_code = 0;
				}
			}
			DEACCESS(Cmiss_region)(&top_region);
		}
		DESTROY(Option_table)(&option_table);
#if defined (WX_USER_INTERFACE)
		if (input_file)
			 DESTROY(IO_stream)(&input_file);
#endif /*defined (WX_USER_INTERFACE)*/
		if (file_name)
		{
			DEALLOCATE(file_name);
		}
		if (region_path)
		{
			DEALLOCATE(region_path);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "gfx_read_elements.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_read_elements */

static int gfx_read_nodes(struct Parse_state *state,
	void *use_data, void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 6 March 2003

DESCRIPTION :
If a nodes file is not specified a file selection box is presented to the user,
otherwise the nodes file is read.
If the <use_data> flag is set, then read data, otherwise nodes.
==============================================================================*/
{
	char *file_name, node_offset_flag, *region_path, time_set_flag;
	double maximum, minimum;
	float time;
	int node_offset, return_code;
	struct Cmiss_command_data *command_data;
	struct Cmiss_region *region, *top_region;
	struct FE_import_time_index *node_time_index, node_time_index_data;
	struct IO_stream *input_file;
	struct Option_table *option_table;

	ENTER(gfx_read_nodes);
	input_file=NULL;
	if (state)
	{
		if (NULL != (command_data = (struct Cmiss_command_data *)command_data_void))
		{
			file_name = (char *)NULL;
			node_offset_flag = 0;
			node_offset = 0;
			region_path = (char *)NULL;
			time = 0;
			time_set_flag = 0;
			node_time_index = (struct FE_import_time_index *)NULL;
			option_table=CREATE(Option_table)();
			/* example */
			Option_table_add_entry(option_table,CMGUI_EXAMPLE_DIRECTORY_SYMBOL,
				&file_name, &(command_data->example_directory), set_file_name);
			if (!use_data)
			{
				/* node_offset */
				Option_table_add_entry(option_table, "node_offset", &node_offset,
					&node_offset_flag, set_int_and_char_flag);
			}
			else
			{
				/* data_offset */
				Option_table_add_entry(option_table, "data_offset", &node_offset,
					&node_offset_flag, set_int_and_char_flag);
			}
			/* region */
			Option_table_add_entry(option_table,"region", &region_path, (void *)1, set_name);
			/* time */
			Option_table_add_entry(option_table,"time",
				&time, &time_set_flag, set_float_and_char_flag);
			/* default */
			Option_table_add_entry(option_table, NULL, &file_name,
				NULL, set_file_name);
			return_code = Option_table_multi_parse(option_table,state);
			if (return_code)
			{
				if (!file_name)
				{
					if (use_data)
					{
						if (!(file_name = confirmation_get_read_filename(".exdata",
										 command_data->user_interface
#if defined(WX_USER_INTERFACE)
								 , command_data->execute_command
#endif /*defined (WX_USER_INTERFACE) */
																														 )))
						{
							return_code = 0;
						}
					}
					else
					{
						if (!(file_name = confirmation_get_read_filename(".exnode",
							command_data->user_interface
#if defined(WX_USER_INTERFACE)
								 , command_data->execute_command
#endif /*defined (WX_USER_INTERFACE) */
																														 )))
						{
							return_code = 0;
						}
					}
				}
				if (time_set_flag)
				{
					node_time_index_data.time = time;
					node_time_index = &node_time_index_data;
				}
				if (return_code)
				{
#if defined (WX_USER_INTERFACE) && defined (WIN32_SYSTEM)
			if (file_name)
			{
				 file_name = CMISS_set_directory_and_filename_WIN32(file_name,
						command_data);
			}
#endif /* defined (WIN32_SYSTEM)*/
					/* open the file */
					if (use_data)
					{
						return_code = check_suffix(&file_name,".exdata");
					}
					else
					{
						return_code = check_suffix(&file_name,".exnode");
					}
					top_region = (struct Cmiss_region *)NULL;
					if (region_path)
					{
						top_region = Cmiss_region_find_subregion_at_path(
							command_data->root_region, region_path);
						if (NULL == top_region)
						{
							top_region = Cmiss_region_create_subregion(
								command_data->root_region, region_path);
							if (NULL == top_region)
							{
								display_message(ERROR_MESSAGE, "gfx read nodes.  "
									"Unable to find or create region '%s'.", region_path);
								return_code = 0;
							}
						}
					}
					else
					{
						top_region = ACCESS(Cmiss_region)(command_data->root_region);
					}
					if (return_code)
					{
						if ((input_file = CREATE(IO_stream)(command_data->io_stream_package))
							&& (IO_stream_open_for_read(input_file, file_name)))
						{
							region = Cmiss_region_create_region(top_region);
							if (use_data)
							{
								return_code = read_exdata_file(region, input_file, node_time_index);
							}
							else
							{
								return_code = read_exregion_file(region, input_file, node_time_index);
							}
							if (return_code)
							{
								if (node_offset_flag)
								{
									/* Offset these nodes before merging */
									if (use_data)
									{
										return_code = offset_region_identifier(region, 0, 0, 0,
											0, 0, 0, node_offset_flag, node_offset, /*use_data*/1);
									}
									else
									{
										return_code = offset_region_identifier(region, 0, 0, 0,
											0, 0, 0, node_offset_flag, node_offset, /*use_data*/0);
									}
								}
								if (Cmiss_region_can_merge(top_region, region))
								{
									if (!Cmiss_region_merge(top_region, region))
									{
										if (use_data)
										{
											display_message(ERROR_MESSAGE,
												"Error merging data from file: %s", file_name);
										}
										else
										{
											display_message(ERROR_MESSAGE,
												"Error merging nodes from file: %s", file_name);
										}
										return_code = 0;
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"Contents of file %s not compatible with global objects",
										file_name);
									return_code = 0;
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"Error reading node file: %s", file_name);
								return_code = 0;
							}
							DEACCESS(Cmiss_region)(&region);
							IO_stream_close(input_file);
							DESTROY(IO_stream)(&input_file);
							input_file =NULL;
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Could not open node file: %s", file_name);
							return_code = 0;
						}
					}
					DEACCESS(Cmiss_region)(&top_region);
					if (return_code && time_set_flag)
					{
						/* Increase the range of the default time keepeer and set the
						   minimum and maximum if we set anything */
						maximum = Time_keeper_get_maximum(
							command_data->default_time_keeper);
						minimum = Time_keeper_get_minimum(
							command_data->default_time_keeper);
						if (time < minimum)
						{
							Time_keeper_set_minimum(
								command_data->default_time_keeper, time);
							Time_keeper_set_maximum(
								command_data->default_time_keeper, maximum);
						}
						if (time > maximum)
						{
							Time_keeper_set_minimum(
								command_data->default_time_keeper, minimum);
							Time_keeper_set_maximum(
								command_data->default_time_keeper, time);
						}
					}
				}
			}
			DESTROY(Option_table)(&option_table);
#if defined (WX_USER_INTERFACE)
			if (input_file)
				 DESTROY(IO_stream)(&input_file);
			input_file =NULL;
#endif /*defined (WX_USER_INTERFACE)*/
			if (file_name)
			{
				DEALLOCATE(file_name);
			}
			if (region_path)
			{
				DEALLOCATE(region_path);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"gfx_read_nodes.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_read_nodes.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_read_nodes */

static int gfx_read_objects(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 7 January 2003

DESCRIPTION :
If a file is not specified a file selection box is presented to the user,
otherwise the file of graphics objects is read.
==============================================================================*/
{
	char *file_name;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Option_table *option_table;

	ENTER(gfx_read_objects);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (NULL != (command_data = (struct Cmiss_command_data *)command_data_void))
		{
			file_name = (char *)NULL;
			option_table=CREATE(Option_table)();
			/* example */
			Option_table_add_entry(option_table,CMGUI_EXAMPLE_DIRECTORY_SYMBOL,
				&file_name, &(command_data->example_directory), set_file_name);
			/* default */
			Option_table_add_entry(option_table,NULL,&file_name,
				NULL,set_file_name);
			return_code = Option_table_multi_parse(option_table,state);
			DESTROY(Option_table)(&option_table);
			if (return_code)
			{
				if (!file_name)
				{
					if (!(file_name = confirmation_get_read_filename(".exgobj",
						command_data->user_interface
#if defined(WX_USER_INTERFACE)
								 , command_data->execute_command
#endif /*defined (WX_USER_INTERFACE) */
																													 )))
					{
						return_code = 0;
					}
				}
				if (return_code)
				{
					/* open the file */
					if (0 != (return_code = check_suffix(&file_name, ".exgobj")))
					{
						return_code=file_read_graphics_objects(file_name, command_data->io_stream_package,
							Material_package_get_material_manager(command_data->material_package),
							command_data->glyph_manager);
					}
				}
			}
			DEALLOCATE(file_name);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_read_objects.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_read_objects.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_read_objects */

/***************************************************************************//**
 * Read regions and fields in FieldML 0.1, 0.4 and EX format.
 * If filename is not specified a file selection box is presented to the user.
 */
static int gfx_read_region(struct Parse_state *state,
	void *dummy, void *command_data_void)
{
	const char file_ext[] = ".fml";
	int return_code = 0;
	USE_PARAMETER(dummy);
	Cmiss_command_data *command_data = reinterpret_cast<Cmiss_command_data*>(command_data_void);
	if (state && command_data)
	{
		struct FE_field_order_info *field_order_info = (struct FE_field_order_info *)NULL;
		Cmiss_region_id region = Cmiss_region_access(command_data->root_region);
		char *file_name = (char *)NULL;

		struct Option_table *option_table = CREATE(Option_table)();
		/* fields */
		Option_table_add_entry(option_table, "fields", &field_order_info,
			Cmiss_region_get_FE_region(command_data->root_region),
			set_FE_fields_FE_region);
		/* region */
		Option_table_add_set_Cmiss_region(option_table, "region",
			command_data->root_region, &region);
		/* default option: file name */
		Option_table_add_default_string_entry(option_table, &file_name, "FILE_NAME");
		return_code = Option_table_multi_parse(option_table, state);
		DESTROY(Option_table)(&option_table);
		if (return_code)
		{
			if (!file_name)
			{
				if (!(file_name = confirmation_get_read_filename(file_ext,
					command_data->user_interface
#if defined(WX_USER_INTERFACE)
								 , command_data->execute_command
#endif /*defined (WX_USER_INTERFACE) */
																												 )))
				{
					return_code = 0;
				}
			}
			if (return_code)
			{
				Cmiss_stream_information_id stream_information =
					Cmiss_region_create_stream_information(region);
				Cmiss_stream_resource_id resource = Cmiss_stream_information_create_resource_file(
					stream_information, file_name);
				return_code = Cmiss_region_read(region, stream_information);
				Cmiss_stream_resource_destroy(&resource);
				Cmiss_stream_information_destroy(&stream_information);
				if (!return_code)
				{
					display_message(ERROR_MESSAGE,
						"Error reading region file: %s", file_name);
					return_code = 0;
				}
			}
		}
		if (field_order_info)
		{
			DESTROY(FE_field_order_info)(&field_order_info);
		}
		if (file_name)
		{
			DEALLOCATE(file_name);
		}
		Cmiss_region_destroy(&region);
	}
	return return_code;
}

static int gfx_read_wavefront_obj(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 19 March 2001

DESCRIPTION :
If a file is not specified a file selection box is presented to the user,
otherwise the wavefront obj file is read.
==============================================================================*/
{
	char *file_name, *graphics_object_name,	*specified_graphics_object_name;
	const char *render_type_string, **valid_strings;
	enum Cmiss_graphics_render_type render_type;
	float time;
	int number_of_valid_strings, return_code;
	struct Cmiss_command_data *command_data;
	struct Option_table *option_table;

	ENTER(gfx_read_wavefront_obj);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if ((command_data=(struct Cmiss_command_data *)command_data_void))
		{
			specified_graphics_object_name=(char *)NULL;
			graphics_object_name=(char *)NULL;
			time = 0;
			file_name=(char *)NULL;

			option_table=CREATE(Option_table)();
			/* example */
			Option_table_add_entry(option_table,CMGUI_EXAMPLE_DIRECTORY_SYMBOL,
			  &file_name, &(command_data->example_directory), set_file_name);
			/* as */
			Option_table_add_entry(option_table,"as",&specified_graphics_object_name,
				(void *)1,set_name);
			/* render_type */
			render_type = CMISS_GRAPHICS_RENDER_TYPE_SHADED;
			render_type_string = ENUMERATOR_STRING(Cmiss_graphics_render_type)(render_type);
			valid_strings = ENUMERATOR_GET_VALID_STRINGS(Cmiss_graphics_render_type)(
				&number_of_valid_strings,
				(ENUMERATOR_CONDITIONAL_FUNCTION(Cmiss_graphics_render_type) *)NULL, (void *)NULL);
			Option_table_add_enumerator(option_table,number_of_valid_strings,
				valid_strings,&render_type_string);
			DEALLOCATE(valid_strings);
			/* time */
			Option_table_add_entry(option_table,"time",&time,NULL,set_float);
			/* default */
			Option_table_add_entry(option_table,NULL,&file_name,
				NULL,set_file_name);

			return_code=Option_table_multi_parse(option_table,state);
			DESTROY(Option_table)(&option_table);
			if (return_code)
			{
				STRING_TO_ENUMERATOR(Cmiss_graphics_render_type)(render_type_string, &render_type);
				if (!file_name)
				{
					if (!(file_name = confirmation_get_read_filename(".obj",
						command_data->user_interface
#if defined(WX_USER_INTERFACE)
									 , command_data->execute_command
#endif /*defined (WX_USER_INTERFACE) */
																													 )))
					{
						return_code = 0;
					}
				}
				if (return_code)
				{
					if(specified_graphics_object_name)
					{
						graphics_object_name = specified_graphics_object_name;
					}
					else
					{
						graphics_object_name = file_name;
					}
					/* open the file */
					if (0 != (return_code = check_suffix(&file_name, ".obj")))
					{
						return_code=file_read_voltex_graphics_object_from_obj(file_name,
							command_data->io_stream_package,
							graphics_object_name, render_type, time,
							Material_package_get_material_manager(command_data->material_package),
							command_data->glyph_manager);
					}
				}
			}
			if (file_name)
			{
				DEALLOCATE(file_name);
			}
			if (specified_graphics_object_name)
			{
				DEALLOCATE(specified_graphics_object_name);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_read_wavefront_obj.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_read_wavefront_obj.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_read_wavefront_obj */

static int execute_command_gfx_read(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 7 January 2003

DESCRIPTION :
Executes a GFX READ command.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Option_table *option_table;

	ENTER(execute_command_gfx_read);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (command_data = (struct Cmiss_command_data *)command_data_void))
	{
		if (state->current_token)
		{
			option_table = CREATE(Option_table)();
			/* curve */
			Option_table_add_entry(option_table, "curve",
				NULL, command_data_void, gfx_read_Curve);
			/* data */
			Option_table_add_entry(option_table, "data",
				/*use_data*/(void *)1, command_data_void, gfx_read_nodes);
			/* elements */
			Option_table_add_entry(option_table, "elements",
				NULL, command_data_void, gfx_read_elements);
			/* nodes */
			Option_table_add_entry(option_table, "nodes",
				/*use_data*/(void *)0, command_data_void, gfx_read_nodes);
			/* objects */
			Option_table_add_entry(option_table, "objects",
				NULL, command_data_void, gfx_read_objects);
			/* region */
			Option_table_add_entry(option_table, "region",
				NULL, command_data_void, gfx_read_region);
			/* wavefront_obj */
			Option_table_add_entry(option_table, "wavefront_obj",
				NULL, command_data_void, gfx_read_wavefront_obj);
			return_code = Option_table_parse(option_table, state);
			DESTROY(Option_table)(&option_table);
		}
		else
		{
			set_command_prompt("gfx read", command_data);
			return_code = 1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_gfx_read.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_read */

static int execute_command_gfx_select(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 6 March 2003

DESCRIPTION :
Executes a GFX SELECT command.
==============================================================================*/
{
	char all_flag,data_flag,elements_flag,faces_flag,grid_points_flag, *conditional_field_name,
		lines_flag,nodes_flag, *region_path, verbose_flag;
	FE_value time;
	int return_code;
	struct Computed_field *conditional_field;
	struct Cmiss_command_data *command_data;
	struct Cmiss_region *region;
	struct Element_point_ranges *element_point_ranges;
	struct FE_element_grid_to_Element_point_ranges_list_data grid_to_list_data;
	struct FE_field *grid_field;
	struct FE_region *fe_region;
	struct LIST(FE_element) *element_list;
	struct LIST(FE_node) *node_list;
	struct Multi_range *multi_range;
	struct Option_table *option_table;
	struct Set_FE_field_conditional_FE_region_data set_grid_field_data;

	ENTER(execute_command_gfx_select);
	USE_PARAMETER(dummy_to_be_modified);
	if (state&&(command_data=(struct Cmiss_command_data *)command_data_void))
	{
		if (state->current_token)
		{
			region_path = Cmiss_region_get_root_region_path();
			fe_region = Cmiss_region_get_FE_region(command_data->root_region);
			all_flag = 0;
			conditional_field=(struct Computed_field *)NULL;
			data_flag = 0;
			element_point_ranges=(struct Element_point_ranges *)NULL;
			data_flag = 0;
			elements_flag = 0;
			faces_flag = 0;
			grid_points_flag = 0;
			lines_flag = 0;
			nodes_flag = 0;
			conditional_field_name = NULL;
			/* With the current method the selection is always additive
				and so to set the selected flag makes the command useless */
			multi_range=CREATE(Multi_range)();
			if ((grid_field = FE_region_get_FE_field_from_name(fe_region,
				"grid_point_number")) &&
				FE_field_is_1_component_integer(grid_field, (void *)NULL))
			{
				ACCESS(FE_field)(grid_field);
			}
			else
			{
				grid_field = (struct FE_field *)NULL;
			}
			if (command_data->default_time_keeper)
			{
				time = Time_keeper_get_time(command_data->default_time_keeper);
			}
			else
			{
				time = 0.0;
			}
			verbose_flag = 0;

			option_table=CREATE(Option_table)();
			/* all */
			Option_table_add_entry(option_table,"all", &all_flag,
				(void *)NULL,set_char_flag);
			/* conditional_field */
			Option_table_add_string_entry(option_table,"conditional_field",
				&conditional_field_name, " FIELD_NAME");
			/* data */
			Option_table_add_entry(option_table,"data", &data_flag,
				(void *)NULL,set_char_flag);
			/* elements */
			Option_table_add_entry(option_table,"elements",&elements_flag,
				(void *)NULL,set_char_flag);
			/* faces */
			Option_table_add_entry(option_table,"faces",&faces_flag,
				(void *)NULL,set_char_flag);
			/* grid_field */
			set_grid_field_data.conditional_function =
				FE_field_is_1_component_integer;
			set_grid_field_data.user_data = (void *)NULL;
			set_grid_field_data.fe_region = fe_region;
			Option_table_add_entry(option_table, "grid_field", &grid_field,
				(void *)&set_grid_field_data, set_FE_field_conditional_FE_region);
			/* grid_points */
			Option_table_add_entry(option_table,"grid_points",&grid_points_flag,
				(void *)NULL,set_char_flag);
			/* group */
			Option_table_add_entry(option_table, "group", &region_path,
				command_data->root_region, set_Cmiss_region_path);
			/* lines */
			Option_table_add_entry(option_table,"lines",&lines_flag,
				(void *)NULL,set_char_flag);
			/* nodes */
			Option_table_add_entry(option_table,"nodes",&nodes_flag,
				(void *)NULL,set_char_flag);
			/* points */
			Option_table_add_entry(option_table,"points",&element_point_ranges,
				(void *)fe_region, set_Element_point_ranges);
			/* verbose */
			Option_table_add_char_flag_entry(option_table,"verbose",
				&verbose_flag);
			/* default option: multi range */
			Option_table_add_entry(option_table, (char *)NULL, (void *)multi_range,
				NULL, set_Multi_range);
			if (0 != (return_code = Option_table_multi_parse(option_table, state)))
			{
				if ((data_flag + elements_flag + faces_flag + grid_points_flag
					+ lines_flag + nodes_flag) != 1)
				{
					display_message(ERROR_MESSAGE,"gfx select:  "
						"You must specify one and only one of "
						"data/elements/faces/lines/grid_points/nodes.");
					return_code = 0;
				}
				if (!(region_path &&
					Cmiss_region_get_region_from_path_deprecated(command_data->root_region,
						region_path, &region) &&
					(fe_region = Cmiss_region_get_FE_region(region))))
				{
					display_message(ERROR_MESSAGE, "gfx select:  Invalid region");
					return_code = 0;
				}
				else
				{
				if (conditional_field_name)
				{
					Cmiss_field_module_id field_module = Cmiss_region_get_field_module(region);
					conditional_field = Cmiss_field_module_find_field_by_name(field_module,
						conditional_field_name);
					Cmiss_field_module_destroy(&field_module);
					if (!conditional_field)
					{
						display_message(ERROR_MESSAGE,
							"gfx_select:  conditional field cannot be found");
						return_code = 0;
					}
				}
				}
			}
			if (return_code)
			{
				/* data */
				if (data_flag)
				{
					if (NULL != (node_list =
						FE_node_list_from_ranges(FE_region_get_data_FE_region(fe_region), multi_range, conditional_field, time)))
					{
						if (region)
						{
							Cmiss_rendition *local_rendition = Cmiss_graphics_module_get_rendition(
								command_data->graphics_module, region);
							Cmiss_rendition_add_selection_from_node_list(local_rendition,
								node_list, /* use_data */1);
							Cmiss_rendition_destroy(&local_rendition);
						}
						if (verbose_flag)
						{
							display_message(INFORMATION_MESSAGE,
								"Selected %d data points.\n",
								NUMBER_IN_LIST(FE_node)(node_list));
						}
						DESTROY(LIST(FE_node))(&node_list);
					}
				}
				/* element_points */
				if (element_point_ranges)
				{
					Element_point_ranges_selection_select_element_point_ranges(
						command_data->element_point_ranges_selection,element_point_ranges);
				}
				/* elements */
				if (elements_flag)
				{
					if (region)
					{
						int dimension = FE_region_get_highest_dimension(Cmiss_region_get_FE_region(region));
						if (NULL != (element_list =
								FE_element_list_from_region_and_selection_group(
									region, dimension, multi_range, NULL, conditional_field, time)))
						{
							Cmiss_rendition *local_rendition = Cmiss_graphics_module_get_rendition(
								command_data->graphics_module, region);
							Cmiss_rendition_add_selection_from_element_list_of_dimension(local_rendition,element_list, dimension);
							Cmiss_rendition_destroy(&local_rendition);
							if (verbose_flag)
							{
								display_message(INFORMATION_MESSAGE,
									"Selected %d elements.\n",
									NUMBER_IN_LIST(FE_element)(element_list));
							}
							DESTROY(LIST(FE_element))(&element_list);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"execute_command_gfx_select.  Invalid region.");
					}
				}
				/* faces */
				if (faces_flag)
				{
					if (region)
					{
						if (NULL != (element_list =
								FE_element_list_from_region_and_selection_group(
									region, /*dimension*/2, multi_range, NULL, conditional_field, time)))
						{
							Cmiss_rendition *local_rendition = Cmiss_graphics_module_get_rendition(
								command_data->graphics_module, region);
							Cmiss_rendition_add_selection_from_element_list_of_dimension(local_rendition,element_list, 2);
							Cmiss_rendition_destroy(&local_rendition);
							if (verbose_flag)
							{
								display_message(INFORMATION_MESSAGE,
									"Selected %d faces.\n",
									NUMBER_IN_LIST(FE_element)(element_list));
							}
							DESTROY(LIST(FE_element))(&element_list);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"execute_command_gfx_select.  Invalid region.");
					}
				}
				/* grid_points */
				if (grid_points_flag)
				{
					if (0<Multi_range_get_total_number_in_ranges(multi_range))
					{
						if (grid_field)
						{
							if (NULL != (grid_to_list_data.element_point_ranges_list=
								CREATE(LIST(Element_point_ranges))()))
							{
								grid_to_list_data.grid_fe_field=grid_field;
								grid_to_list_data.grid_value_ranges=multi_range;
								/* inefficient: go through every element in FE_region */
								FE_region_for_each_FE_element(fe_region,
									FE_element_grid_to_Element_point_ranges_list,
									(void *)&grid_to_list_data);
								if (0 < NUMBER_IN_LIST(Element_point_ranges)(
									grid_to_list_data.element_point_ranges_list))
								{
									Element_point_ranges_selection_begin_cache(
										command_data->element_point_ranges_selection);
									FOR_EACH_OBJECT_IN_LIST(Element_point_ranges)(
										Element_point_ranges_select,
										(void *)command_data->element_point_ranges_selection,
										grid_to_list_data.element_point_ranges_list);
									Element_point_ranges_selection_end_cache(
										command_data->element_point_ranges_selection);
								}
								DESTROY(LIST(Element_point_ranges))(
									&(grid_to_list_data.element_point_ranges_list));
							}
							else
							{
								display_message(ERROR_MESSAGE,"execute_command_gfx_select.  "
									"Could not create grid_point list");
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,"To select grid_points, "
								"need integer grid_field (eg. grid_point_number)");
						}
					}
				}
				/* lines */
				if (lines_flag)
				{
					if (region)
					{
						if (NULL != (element_list =
								FE_element_list_from_region_and_selection_group(
									region, /*dimension*/1, multi_range, NULL, conditional_field, time)))
						{
							Cmiss_rendition *local_rendition = Cmiss_graphics_module_get_rendition(
								command_data->graphics_module, region);
							Cmiss_rendition_add_selection_from_element_list_of_dimension(local_rendition,element_list, 1);
							Cmiss_rendition_destroy(&local_rendition);
							if (verbose_flag)
							{
								display_message(INFORMATION_MESSAGE,
									"Selected %d lines.\n",
									NUMBER_IN_LIST(FE_element)(element_list));
							}
							DESTROY(LIST(FE_element))(&element_list);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"execute_command_gfx_select.  Invalid region.");
					}
				}
				/* nodes */
				if (nodes_flag)
				{
					if (NULL != (node_list =
							FE_node_list_from_ranges(fe_region, multi_range, conditional_field, time)))
					{
						if (region)
						{
							Cmiss_rendition *local_rendition = Cmiss_graphics_module_get_rendition(
								command_data->graphics_module, region);
							Cmiss_rendition_add_selection_from_node_list(local_rendition,
								node_list, /* use_data */0);
							Cmiss_rendition_destroy(&local_rendition);
						}
						if (verbose_flag)
						{
							display_message(INFORMATION_MESSAGE,
								"Selected %d data points.\n",
								NUMBER_IN_LIST(FE_node)(node_list));
						}
						DESTROY(LIST(FE_node))(&node_list);
					}

				}
			}
			DESTROY(Option_table)(&option_table);
			DEALLOCATE(region_path);
			if (conditional_field_name)
				DEALLOCATE(conditional_field_name);
			if (conditional_field)
			{
				DEACCESS(Computed_field)(&conditional_field);
			}
			if (grid_field)
			{
				DEACCESS(FE_field)(&grid_field);
			}
			DESTROY(Multi_range)(&multi_range);
			if (element_point_ranges)
			{
				DESTROY(Element_point_ranges)(&element_point_ranges);
			}
		}
		else
		{
			set_command_prompt("gfx select",command_data);
			return_code = 1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_gfx_select.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_select */

static int execute_command_gfx_unselect(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 6 March 2003

DESCRIPTION :
Executes a GFX UNSELECT command.
==============================================================================*/
{
	char all_flag,data_flag,elements_flag,faces_flag,grid_points_flag, *conditional_field_name,
		lines_flag,nodes_flag, *region_path, verbose_flag;
	FE_value time;
	int return_code;
	struct Computed_field *conditional_field;
	struct Cmiss_command_data *command_data;
	struct Cmiss_region *region;
	struct Element_point_ranges *element_point_ranges;
	struct FE_element_grid_to_Element_point_ranges_list_data grid_to_list_data;
	struct FE_field *grid_field;
	struct FE_region *fe_region;
	struct LIST(FE_element) *element_list;
	struct LIST(FE_node) *node_list;
	struct Multi_range *multi_range;
	struct Option_table *option_table;
	struct Set_FE_field_conditional_FE_region_data set_grid_field_data;

	ENTER(execute_command_gfx_unselect);
	USE_PARAMETER(dummy_to_be_modified);
	if (state&&(command_data=(struct Cmiss_command_data *)command_data_void))
	{
		if (state->current_token)
		{
			region_path = Cmiss_region_get_root_region_path();
			fe_region = Cmiss_region_get_FE_region(command_data->root_region);
			all_flag = 0;
			conditional_field=(struct Computed_field *)NULL;
			conditional_field_name = NULL;
			data_flag = 0;
			element_point_ranges=(struct Element_point_ranges *)NULL;
			data_flag = 0;
			elements_flag = 0;
			faces_flag = 0;
			grid_points_flag = 0;
			lines_flag = 0;
			nodes_flag = 0;
			/* We only want to unselected from selected objects */
			multi_range=CREATE(Multi_range)();
			if ((grid_field = FE_region_get_FE_field_from_name(fe_region,
				"grid_point_number")) &&
				FE_field_is_1_component_integer(grid_field, (void *)NULL))
			{
				ACCESS(FE_field)(grid_field);
			}
			else
			{
				grid_field = (struct FE_field *)NULL;
			}
			if (command_data->default_time_keeper)
			{
				time = Time_keeper_get_time(command_data->default_time_keeper);
			}
			else
			{
				time = 0.0;
			}
			verbose_flag = 0;

			option_table=CREATE(Option_table)();
			/* all */
			Option_table_add_entry(option_table,"all", &all_flag,
				(void *)NULL,set_char_flag);
			/* conditional_field */
			Option_table_add_string_entry(option_table,"conditional_field",
				&conditional_field_name, " FIELD_NAME");
			/* data */
			Option_table_add_entry(option_table,"data", &data_flag,
				(void *)NULL,set_char_flag);
			/* elements */
			Option_table_add_entry(option_table,"elements",&elements_flag,
				(void *)NULL,set_char_flag);
			/* faces */
			Option_table_add_entry(option_table,"faces",&faces_flag,
				(void *)NULL,set_char_flag);
			/* grid_field */
			set_grid_field_data.conditional_function =
				FE_field_is_1_component_integer;
			set_grid_field_data.user_data = (void *)NULL;
			set_grid_field_data.fe_region = fe_region;
			Option_table_add_entry(option_table, "grid_field", &grid_field,
				(void *)&set_grid_field_data, set_FE_field_conditional_FE_region);
			/* grid_points */
			Option_table_add_entry(option_table,"grid_points",&grid_points_flag,
				(void *)NULL,set_char_flag);
			/* group */
			Option_table_add_entry(option_table, "group", &region_path,
				command_data->root_region, set_Cmiss_region_path);
			/* lines */
			Option_table_add_entry(option_table,"lines",&lines_flag,
				(void *)NULL,set_char_flag);
			/* nodes */
			Option_table_add_entry(option_table,"nodes",&nodes_flag,
				(void *)NULL,set_char_flag);
			/* points */
			Option_table_add_entry(option_table,"points",&element_point_ranges,
				(void *)fe_region, set_Element_point_ranges);
			/* verbose */
			Option_table_add_char_flag_entry(option_table,"verbose",
				&verbose_flag);
			/* default option: multi range */
			Option_table_add_entry(option_table, (char *)NULL, (void *)multi_range,
				NULL, set_Multi_range);
			if (0 != (return_code = Option_table_multi_parse(option_table, state)))
			{
				if ((data_flag + elements_flag + faces_flag + grid_points_flag
					+ lines_flag + nodes_flag) != 1)
				{
					display_message(ERROR_MESSAGE,"gfx unselect:  "
						"You must specify one and only one of "
						"data/elements/faces/lines/grid_points/nodes.");
					return_code = 0;
				}
				if (!(region_path &&
					Cmiss_region_get_region_from_path_deprecated(command_data->root_region,
						region_path, &region) &&
					(fe_region = Cmiss_region_get_FE_region(region))))
				{
					display_message(ERROR_MESSAGE, "gfx unselect:  Invalid region");
					return_code = 0;
				}
				else
				{
				if (conditional_field_name)
				{
					Cmiss_field_module_id field_module = Cmiss_region_get_field_module(region);
					conditional_field = Cmiss_field_module_find_field_by_name(field_module,
						conditional_field_name);
					Cmiss_field_module_destroy(&field_module);
					if (!conditional_field)
					{
						display_message(ERROR_MESSAGE,
							"gfx_unselect:  conditional field cannot be found");
						return_code = 0;
					}
				}
				}
			}
			if (return_code)
			{
				/* data */
				if (data_flag)
				{
					if (NULL != (node_list =
							FE_node_list_from_ranges(FE_region_get_data_FE_region(fe_region), multi_range, conditional_field, time)))
					{
						if (region)
						{
							Cmiss_rendition *local_rendition = Cmiss_graphics_module_get_rendition(
								command_data->graphics_module, region);
							Cmiss_rendition_remove_selection_from_node_list(local_rendition,
								node_list, /* use_data */1);
							Cmiss_rendition_destroy(&local_rendition);
						}
						if (verbose_flag)
						{
							display_message(INFORMATION_MESSAGE,
								"Unselected %d data points.\n",
								NUMBER_IN_LIST(FE_node)(node_list));
						}
						DESTROY(LIST(FE_node))(&node_list);
					}
				}
				/* element_points */
				if (element_point_ranges)
				{
					Element_point_ranges_selection_unselect_element_point_ranges(
						command_data->element_point_ranges_selection,element_point_ranges);
				}
				/* elements */
				if (elements_flag)
				{
					if (region)
					{
						int dimension = FE_region_get_highest_dimension(Cmiss_region_get_FE_region(region));
						if (NULL != (element_list =
								FE_element_list_from_region_and_selection_group(
									region, dimension, multi_range, NULL, conditional_field, time)))
						{
							Cmiss_rendition *local_rendition = Cmiss_graphics_module_get_rendition(
								command_data->graphics_module, region);
							Cmiss_rendition_remove_selection_from_element_list_of_dimension(local_rendition,element_list, dimension);
							Cmiss_rendition_destroy(&local_rendition);
							if (verbose_flag)
							{
								display_message(INFORMATION_MESSAGE,
									"Unselected %d elements.\n",
									NUMBER_IN_LIST(FE_element)(element_list));
							}
							DESTROY(LIST(FE_element))(&element_list);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"execute_command_gfx_select.  Invalid region.");
					}
				}
				/* faces */
				if (faces_flag)
				{
					if (region)
					{
						if (NULL != (element_list =
								FE_element_list_from_region_and_selection_group(
									region, /*dimension*/2, multi_range, NULL, conditional_field, time)))
						{
							Cmiss_rendition *local_rendition = Cmiss_graphics_module_get_rendition(
								command_data->graphics_module, region);
							Cmiss_rendition_remove_selection_from_element_list_of_dimension(local_rendition,element_list, 2);
							Cmiss_rendition_destroy(&local_rendition);
							if (verbose_flag)
							{
								display_message(INFORMATION_MESSAGE,
									"Unselected %d faces.\n",
									NUMBER_IN_LIST(FE_element)(element_list));
							}
							DESTROY(LIST(FE_element))(&element_list);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"execute_command_gfx_select.  Invalid region.");
					}
				}
				/* grid_points */
				if (grid_points_flag)
				{
					if (0<Multi_range_get_total_number_in_ranges(multi_range))
					{
						if (grid_field)
						{
							if (NULL != (grid_to_list_data.element_point_ranges_list=
								CREATE(LIST(Element_point_ranges))()))
							{
								grid_to_list_data.grid_fe_field=grid_field;
								grid_to_list_data.grid_value_ranges=multi_range;
								/* inefficient: go through every element in FE_region */
								FE_region_for_each_FE_element(fe_region,
									FE_element_grid_to_Element_point_ranges_list,
									(void *)&grid_to_list_data);
								if (0<NUMBER_IN_LIST(Element_point_ranges)(
									grid_to_list_data.element_point_ranges_list))
								{
									Element_point_ranges_selection_begin_cache(
										command_data->element_point_ranges_selection);
									FOR_EACH_OBJECT_IN_LIST(Element_point_ranges)(
										Element_point_ranges_unselect,
										(void *)command_data->element_point_ranges_selection,
										grid_to_list_data.element_point_ranges_list);
									Element_point_ranges_selection_end_cache(
										command_data->element_point_ranges_selection);
								}
								DESTROY(LIST(Element_point_ranges))(
									&(grid_to_list_data.element_point_ranges_list));
							}
							else
							{
								display_message(ERROR_MESSAGE,"execute_command_gfx_unselect.  "
									"Could not create grid_point list");
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,"To unselect grid_points, "
								"need integer grid_field (eg. grid_point_number)");
						}
					}
				}
				/* lines */
				if (lines_flag)
				{
					if (region)
					{
						if (NULL != (element_list =
								FE_element_list_from_region_and_selection_group(
									region, /*dimension*/1, multi_range, NULL, conditional_field, time)))
						{
							Cmiss_rendition *local_rendition = Cmiss_graphics_module_get_rendition(
								command_data->graphics_module, region);
							Cmiss_rendition_remove_selection_from_element_list_of_dimension(local_rendition, element_list, 1);
							Cmiss_rendition_destroy(&local_rendition);
							if (verbose_flag)
							{
								display_message(INFORMATION_MESSAGE,
									"Unselected %d lines.\n",
									NUMBER_IN_LIST(FE_element)(element_list));
							}
							DESTROY(LIST(FE_element))(&element_list);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"execute_command_gfx_select.  Invalid region.");
					}
				}
				/* nodes */
				if (nodes_flag)
				{
					if (NULL != (node_list =
							FE_node_list_from_ranges(fe_region, multi_range, conditional_field, time)))
					{
						if (region)
						{
							Cmiss_rendition *local_rendition = Cmiss_graphics_module_get_rendition(
								command_data->graphics_module, region);
							Cmiss_rendition_remove_selection_from_node_list(local_rendition,
								node_list, /* use_data */0);
							Cmiss_rendition_destroy(&local_rendition);
						}
						DESTROY(LIST(FE_node))(&node_list);
					}
				}
			}
			DESTROY(Option_table)(&option_table);
			DEALLOCATE(region_path);
			if (conditional_field_name)
				DEALLOCATE(conditional_field_name);
			if (conditional_field)
			{
				DEACCESS(Computed_field)(&conditional_field);
			}
			if (grid_field)
			{
				DEACCESS(FE_field)(&grid_field);
			}
			DESTROY(Multi_range)(&multi_range);
			if (element_point_ranges)
			{
				DESTROY(Element_point_ranges)(&element_point_ranges);
			}
		}
		else
		{
			set_command_prompt("gfx unselect",command_data);
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_gfx_unselect.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_unselect */

int gfx_set_FE_nodal_value(struct Parse_state *state,void *dummy_to_be_modified,
	void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 6 March 2003

DESCRIPTION :
Sets nodal field values from a command.
???DB.  Should it be here ?
==============================================================================*/
{
	const char *current_token;
	enum FE_nodal_value_type fe_nodal_d_ds1,fe_nodal_d_ds2,fe_nodal_d_ds3,
		fe_nodal_d2_ds1ds2,fe_nodal_d2_ds1ds3,fe_nodal_d2_ds2ds3,
		fe_nodal_d3_ds1ds2ds3,fe_nodal_value,value_type;
	FE_value value;
	int return_code;
	static struct Modifier_entry option_table[]=
	{
		{"value",NULL,NULL,set_enum},
		{"d/ds1",NULL,NULL,set_enum},
		{"d/ds2",NULL,NULL,set_enum},
		{"d/ds3",NULL,NULL,set_enum},
		{"d2/ds1ds2",NULL,NULL,set_enum},
		{"d2/ds1ds3",NULL,NULL,set_enum},
		{"d2/ds2ds3",NULL,NULL,set_enum},
		{"d3/ds1ds2ds3",NULL,NULL,set_enum},
		{NULL,NULL,NULL,NULL}
	};
	struct Cmiss_command_data *command_data;
	struct FE_field_component component;
	struct FE_node *node;
	struct FE_region *fe_region;
	struct LIST(FE_field) *fe_field_list;

	ENTER(gfx_set_FE_nodal_value);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (command_data = (struct Cmiss_command_data *)command_data_void))
	{
		fe_region = Cmiss_region_get_FE_region(command_data->root_region);
		node = (struct FE_node *)NULL;
		if (0 != (return_code = set_FE_node_FE_region(state,(void *)&node,
			(void *)fe_region)))
		{
			component.field = (struct FE_field *)NULL;
			component.number = 0;
			if (0 != (return_code = set_FE_field_component_FE_region(state,
				(void *)&component, (void *)fe_region)))
			{
				value_type=FE_NODAL_UNKNOWN;
				option_table[0].to_be_modified= &value_type;
				fe_nodal_value=FE_NODAL_VALUE;
				option_table[0].user_data= &fe_nodal_value;
				option_table[1].to_be_modified= &value_type;
				fe_nodal_d_ds1=FE_NODAL_D_DS1;
				option_table[1].user_data= &fe_nodal_d_ds1;
				option_table[2].to_be_modified= &value_type;
				fe_nodal_d_ds2=FE_NODAL_D_DS2;
				option_table[2].user_data= &fe_nodal_d_ds2;
				option_table[3].to_be_modified= &value_type;
				fe_nodal_d_ds3=FE_NODAL_D_DS3;
				option_table[3].user_data= &fe_nodal_d_ds3;
				option_table[4].to_be_modified= &value_type;
				fe_nodal_d2_ds1ds2=FE_NODAL_D2_DS1DS2;
				option_table[4].user_data= &fe_nodal_d2_ds1ds2;
				option_table[5].to_be_modified= &value_type;
				fe_nodal_d2_ds1ds3=FE_NODAL_D2_DS1DS3;
				option_table[5].user_data= &fe_nodal_d2_ds1ds3;
				option_table[6].to_be_modified= &value_type;
				fe_nodal_d2_ds2ds3=FE_NODAL_D2_DS2DS3;
				option_table[6].user_data= &fe_nodal_d2_ds2ds3;
				option_table[7].to_be_modified= &value_type;
				fe_nodal_d3_ds1ds2ds3=FE_NODAL_D3_DS1DS2DS3;
				option_table[7].user_data= &fe_nodal_d3_ds1ds2ds3;
				if (0 != (return_code = process_option(state,option_table)))
				{
					if (NULL != (current_token = state->current_token))
					{
						if (1 == sscanf(current_token, FE_VALUE_INPUT_STRING, &value))
						{
							if (!(set_FE_nodal_FE_value_value(node,
								component.field, component.number,
								/*version*/0, value_type, /*time*/0,value)))
							{
								return_code = 0;
							}
							if (!return_code)
							{
								display_message(ERROR_MESSAGE,
									"gfx_set_FE_nodal_value.  Failed");
								return_code = 0;
							}
							DESTROY(LIST(FE_field))(&fe_field_list);
						}
						else
						{
							display_message(ERROR_MESSAGE,"Invalid nodal value %s",
								current_token);
							display_parse_state_location(state);
							return_code=0;
						}
					}
					else
					{
						display_message(WARNING_MESSAGE,"Missing value for node");
						display_parse_state_location(state);
						return_code=1;
					}
				}
				else
				{
					if ((current_token=state->current_token)&&
						!(strcmp(PARSER_HELP_STRING,current_token)&&
						strcmp(PARSER_RECURSIVE_HELP_STRING,current_token)))
					{
						display_message(INFORMATION_MESSAGE," #\n");
						return_code=1;
					}
				}
			}
		}
		if (node)
		{
			DEACCESS(FE_node)(&node);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_set_FE_nodal_value.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_set_FE_nodal_value */

/***************************************************************************//**
 * Sets the order of regions in the region hierarchy.
 */
static int gfx_set_region_order(struct Parse_state *state,
	void *dummy_to_be_modified, void *root_region_void)
{
	int return_code = 0;

	ENTER(gfx_set_region_order);
	USE_PARAMETER(dummy_to_be_modified);
	Cmiss_region *root_region = (struct Cmiss_region *)root_region_void;
	if (state && root_region)
	{
		char *insert_before_sibling_name = NULL;
		Cmiss_region_id region = Cmiss_region_access(root_region);
		Cmiss_field_group_id group = 0;
		Option_table *option_table = CREATE(Option_table)();
		Option_table_add_help(option_table,
			"Change the order of regions in the region hierarchy. The 'region' "
			"option specifies the current path of the region to be moved. The "
			"'before' option gives the name of an existing sibling region "
			"which the region will be re-inserted before.");
		Option_table_add_string_entry(option_table, "before",
			&insert_before_sibling_name, " SIBLING_REGION_NAME");
		Option_table_add_region_or_group_entry(option_table,
			"region", &region, &group);
		return_code = Option_table_multi_parse(option_table, state);
		DESTROY(Option_table)(&option_table);
		if (group)
		{
			display_message(WARNING_MESSAGE,
				"gfx set order:  Obsolete for subgroups. To set rendering order within a region, create graphics in required order.");
			display_parse_state_location(state);
		}
		else if (return_code)
		{
			if (!region)
			{
				display_message(ERROR_MESSAGE,
					"gfx set order.  Must specify a region");
				return_code = 0;
			}
			if (!insert_before_sibling_name)
			{
				display_message(ERROR_MESSAGE,
					"gfx set order.  Must specify a sibling region name to insert before");
				return_code = 0;
			}
			if (return_code)
			{
				Cmiss_region *parent = Cmiss_region_get_parent(region);
				Cmiss_region *sibling = Cmiss_region_find_child_by_name(parent,
					insert_before_sibling_name);
				if (sibling)
				{
					Cmiss_region_insert_child_before(parent, region, sibling);
					Cmiss_region_destroy(&sibling);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"gfx set order.  Cannot find sibling '%s' to insert before",
						insert_before_sibling_name);
					return_code = 0;
				}
				Cmiss_region_destroy(&parent);
			}
		}
		DEALLOCATE(insert_before_sibling_name);
		Cmiss_field_group_destroy(&group);
		Cmiss_region_destroy(&region);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_set_region_order.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
}

#if defined (WX_USER_INTERFACE)
static int gfx_set_time(struct Parse_state *state,void *dummy_to_be_modified,
	void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 19 October 1998

DESCRIPTION :
Sets the time from the command line.
==============================================================================*/
{
	char *timekeeper_name;
	float time;
	int return_code;
	struct Cmiss_command_data *command_data;
	static struct Modifier_entry option_table[]=
	{
		{"timekeeper",NULL,NULL,set_name},
		{NULL,NULL,NULL,set_float}
	};

	ENTER(gfx_set_time);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (NULL != (command_data = (struct Cmiss_command_data *)command_data_void))
		{
			ALLOCATE(timekeeper_name,char,10);
			/* there is only a default timekeeper at the moment but I am making the
				commands with a timekeeper manager in mind */
			strcpy(timekeeper_name,"default");
			if (command_data->default_time_keeper)
			{
				time=Time_keeper_get_time(command_data->default_time_keeper);
			}
			else
			{
				/*This option is used so that help comes out*/
				time = 0;
			}
			(option_table[0]).to_be_modified= &timekeeper_name;
			(option_table[1]).to_be_modified= &time;
			return_code=process_multiple_options(state,option_table);
			/* no errors, not asking for help */
			if (return_code)
			{
				/* the old routine only use to call this if the time wasn't the
					same as the default time, but the timekeeper might not be the
					default */
				Time_keeper_request_new_time(command_data->default_time_keeper,time);
			} /* parse error, help */
			DEALLOCATE(timekeeper_name);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_set_time.  Invalid argument(s)");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_set_time.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_set_time */
#endif /* defined (WX_USER_INTERFACE)*/

static int set_transformation_matrix(struct Parse_state *state,
	void *transformation_matrix_void,void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
Sets a transformation matrix from the command line.
==============================================================================*/
{
	const char *current_token;
	gtMatrix *transformation_matrix;
	int i,j,return_code;

	ENTER(set_transformation_matrix);
	USE_PARAMETER(dummy_user_data);
	if (state)
	{
		if ((current_token=state->current_token)&&(
			!strcmp(PARSER_HELP_STRING,current_token)||
			!strcmp(PARSER_RECURSIVE_HELP_STRING,current_token)))
		{
			display_message(INFORMATION_MESSAGE,"# # # # # # # # # # # # # # # #");
			if (NULL != (transformation_matrix=(gtMatrix *)transformation_matrix_void))
			{
				display_message(INFORMATION_MESSAGE,
					" [%g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g]",
					(*transformation_matrix)[0][0],(*transformation_matrix)[0][1],
					(*transformation_matrix)[0][2],(*transformation_matrix)[0][3],
					(*transformation_matrix)[1][0],(*transformation_matrix)[1][1],
					(*transformation_matrix)[1][2],(*transformation_matrix)[1][3],
					(*transformation_matrix)[2][0],(*transformation_matrix)[2][1],
					(*transformation_matrix)[2][2],(*transformation_matrix)[2][3],
					(*transformation_matrix)[3][0],(*transformation_matrix)[3][1],
					(*transformation_matrix)[3][2],(*transformation_matrix)[3][3]);
			}
			return_code=1;
		}
		else
		{
			if (NULL != (transformation_matrix=(gtMatrix *)transformation_matrix_void))
			{
				return_code=1;
				i=0;
				j=0;
				while ((i<4)&&return_code&&current_token)
				{
					j=0;
					while ((j<4)&&return_code&&current_token)
					{
						if (1==sscanf(current_token,"%lf",&((*transformation_matrix)[i][j])))
						{
							shift_Parse_state(state,1);
							current_token=state->current_token;
						}
						else
						{
							return_code=0;
						}
						j++;
					}
					i++;
				}
				if (!return_code||(i<4)||(j<4))
				{
					if (current_token)
					{
						display_message(ERROR_MESSAGE,
							"Error reading transformation matrix: %s",current_token);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Error reading transformation matrix");
					}
					display_parse_state_location(state);
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"set_transformation_matrix.  Missing transformation_matrix");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_transformation_matrix.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_transformation_matrix */

static int gfx_set_transformation(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
{
	int return_code;

	ENTER(gfx_set_transformation);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		struct Cmiss_command_data *command_data = (struct Cmiss_command_data *)command_data_void;
		if (command_data)
		{
			 /* initialise defaults */
			struct Computed_field *computed_field=(struct Computed_field *)NULL;
			Cmiss_region *region = Cmiss_region_access(command_data->root_region);
			char *field_name = NULL;
			gtMatrix transformation_matrix;
			transformation_matrix[0][0]=1;
			transformation_matrix[0][1]=0;
			transformation_matrix[0][2]=0;
			transformation_matrix[0][3]=0;
			transformation_matrix[1][0]=0;
			transformation_matrix[1][1]=1;
			transformation_matrix[1][2]=0;
			transformation_matrix[1][3]=0;
			transformation_matrix[2][0]=0;
			transformation_matrix[2][1]=0;
			transformation_matrix[2][2]=1;
			transformation_matrix[2][3]=0;
			transformation_matrix[3][0]=0;
			transformation_matrix[3][1]=0;
			transformation_matrix[3][2]=0;
			transformation_matrix[3][3]=1;
			/* parse the command line */
			Option_table *option_table = CREATE(Option_table)();
			Option_table_add_set_Cmiss_region(option_table,
				"name", command_data->root_region, &region);
			Option_table_add_string_entry(option_table, "field",
				&field_name, " FIELD_NAME");
			/* default: set transformation matrix */
			Option_table_add_entry(option_table, (const char *)NULL,
				&transformation_matrix, /*user_data*/(void *)NULL, set_transformation_matrix);
			return_code = Option_table_multi_parse(option_table, state);
			DESTROY(Option_table)(&option_table);
			if (return_code)
			{
				if (field_name)
				{
					Cmiss_field_module_id field_module = Cmiss_region_get_field_module(region);
					computed_field = Cmiss_field_module_find_field_by_name(field_module, field_name);
					Cmiss_field_module_destroy(&field_module);
					if (!computed_field)
					{
						display_message(ERROR_MESSAGE,
							"gfx_set_transformation: transformation field cannot be found");
						return_code = 0;
					}
				}
				if (return_code)
				{
					Cmiss_rendition *rendition = Cmiss_region_get_rendition_internal(region);
					if (rendition)
					{
						if (computed_field)
						{
							Cmiss_rendition_set_transformation_with_time_callback(rendition,
								computed_field);
						}
						else
						{
							Cmiss_rendition_remove_time_dependent_transformation(rendition);
							Cmiss_rendition_set_transformation(rendition,&transformation_matrix);
						}
						DEACCESS(Cmiss_rendition)(&rendition);
					}
					return_code = 1;
				}
				if (computed_field)
					DEACCESS(Computed_field)(&computed_field);
			}
			Cmiss_region_destroy(&region);
			if (field_name)
				DEALLOCATE(field_name);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_set_transformation.  Missing command_data_void");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_set_transformation.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_set_transformation */

/***************************************************************************//**
 * Toggles the visibility of graphics objects on scenes from the command line.
 */
static int gfx_set_visibility(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
{
	int return_code;
	USE_PARAMETER(dummy_to_be_modified);
	Cmiss_command_data *command_data = reinterpret_cast<Cmiss_command_data *>(command_data_void);
	if (state && command_data)
	{
		char on_flag = 0;
		char off_flag = 0;
		char *part_graphic_name = 0;
		char *region_path = 0;
		Option_table *option_table = CREATE(Option_table)();
		Option_table_add_help(option_table,
			"Set the visibility of a whole region's rendition or individual graphics in it. "
			"Specify visibility as 'on', 'off' or omit both to toggle. If name option is "
			"specified, sets visibility for all graphics in region whose name contains the string.");
		Option_table_add_char_flag_entry(option_table, "off", &off_flag);
		Option_table_add_char_flag_entry(option_table, "on", &on_flag);
		Option_table_add_string_entry(option_table, "name", &part_graphic_name, " PART_GRAPHIC_NAME");
		Option_table_add_default_string_entry(option_table, &region_path, " REGION_PATH");
		return_code = Option_table_multi_parse(option_table, state);
		DESTROY(Option_table)(&option_table);

		Cmiss_region_id region = 0;
		if (return_code)
		{
			if (off_flag && on_flag)
			{
				display_message(ERROR_MESSAGE, "gfx set visibility:  Set only one of off|on");
				return_code = 0;
			}
			if (!region_path)
			{
				display_message(ERROR_MESSAGE, "gfx set visibility:  Must specify region_path");
				return_code = 0;
			}
			else
			{
				region = Cmiss_region_find_subregion_at_path(command_data->root_region, region_path);
				if (!region)
				{
					display_message(ERROR_MESSAGE, "gfx set visibility:  Could not find region %s", region_path);
					return_code = 0;
				}
			}
		}
		if (return_code)
		{
			Cmiss_rendition_id rendition = Cmiss_region_get_rendition_internal(region);
			if (part_graphic_name)
			{
				Cmiss_rendition_begin_change(rendition);
				// support legacy command files by changing visibility of each graphic using group as its subgroup field
				Cmiss_graphic_id graphic = Cmiss_rendition_get_first_graphic(rendition);
				int number_matched = 0;
				while (graphic)
				{
					char *graphic_name = Cmiss_graphic_get_name_internal(graphic);
					if (strstr(graphic_name, part_graphic_name))
					{
						int visibility_flag = on_flag ? 1 : (off_flag ? 0 : !Cmiss_graphic_get_visibility_flag(graphic));
						Cmiss_graphic_set_visibility_flag(graphic, visibility_flag);
						++number_matched;
					}
					DEALLOCATE(graphic_name);
					Cmiss_graphic_id temp = Cmiss_rendition_get_next_graphic(rendition, graphic);
					Cmiss_graphic_destroy(&graphic);
					graphic = temp;
				}
				Cmiss_rendition_end_change(rendition);
				if (0 == number_matched)
				{
					display_message(WARNING_MESSAGE, "gfx set visibility:  No graphics matched name '%s'", part_graphic_name);
				}
			}
			else
			{
				int visibility_flag = on_flag ? 1 : (off_flag ? 0 : !Cmiss_rendition_get_visibility_flag(rendition));
				Cmiss_rendition_set_visibility_flag(rendition, visibility_flag);
			}
			Cmiss_rendition_destroy(&rendition);
		}
		Cmiss_region_destroy(&region);
		if (region_path)
			DEALLOCATE(region_path);
		if (part_graphic_name)
			DEALLOCATE(part_graphic_name);
	}
	else
	{
		display_message(ERROR_MESSAGE, "gfx_set_visibility.  Invalid argument(s)");
		return_code = 0;
	}
	return (return_code);
}

static int execute_command_gfx_set(struct Parse_state *state,
	void *dummy_to_be_modified, void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 27 November 2001

DESCRIPTION :
Executes a GFX SET command.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Option_table *option_table;

	ENTER(execute_command_gfx_set);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (command_data = (struct Cmiss_command_data *)command_data_void))
	{
		if (state->current_token)
		{
			option_table=CREATE(Option_table)();
			Option_table_add_entry(option_table, "node_value", NULL,
				command_data_void, gfx_set_FE_nodal_value);
		 Option_table_add_entry(option_table, "order", NULL,
			(void *)command_data->root_region, gfx_set_region_order);
		 Option_table_add_entry(option_table, "point_size", &global_point_size,
				NULL, set_float_positive);
			Option_table_add_entry(option_table, "transformation", NULL,
				command_data_void, gfx_set_transformation);
#if defined (WX_USER_INTERFACE)
			Option_table_add_entry(option_table, "time", NULL,
				command_data_void, gfx_set_time);
#endif /* defined (WX_USER_INTERFACE)*/
			Option_table_add_entry(option_table, "visibility", NULL,
				command_data_void, gfx_set_visibility);
			return_code = Option_table_parse(option_table, state);
			DESTROY(Option_table)(&option_table);
		}
		else
		{
			set_command_prompt("gfx set", command_data);
			return_code = 1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_gfx_set.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_set */


static int execute_command_gfx_smooth(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 April 2003

DESCRIPTION :
Executes a GFX SMOOTH command.
==============================================================================*/
{
	char *region_path;
	FE_value time;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Cmiss_region *region;
	struct FE_field *fe_field;
	struct FE_region *fe_region;
	struct Option_table *option_table;

	ENTER(execute_command_gfx_smooth);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (command_data = (struct Cmiss_command_data *)command_data_void))
	{
		region_path = Cmiss_region_get_root_region_path();
		fe_field = (struct FE_field *)NULL;
		if (command_data->default_time_keeper)
		{
			time = Time_keeper_get_time(command_data->default_time_keeper);
		}
		else
		{
			time = 0;
		}

		option_table = CREATE(Option_table)();
		/* egroup */
		Option_table_add_entry(option_table, "egroup", &region_path,
			command_data->root_region, set_Cmiss_region_path);
		/* field */
		Option_table_add_set_FE_field_from_FE_region(
			option_table, "field" ,&fe_field,
			Cmiss_region_get_FE_region(command_data->root_region));
		/* time */
		Option_table_add_entry(option_table, "time", &time, NULL, set_FE_value);
		return_code = Option_table_multi_parse(option_table, state);
		if (return_code)
		{
			if (Cmiss_region_get_region_from_path_deprecated(command_data->root_region,
				region_path, &region) &&
				(fe_region = Cmiss_region_get_FE_region(region)))
			{
				if (fe_field)
				{
										////////////////////////////////////
					return_code = FE_region_smooth_FE_field(fe_region, fe_field, time);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"gfx smooth:  Must specify field to smooth");
					display_parse_state_location(state);
					return_code = 0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"execute_command_gfx_smooth.  Invalid region");
				return_code = 0;
			}
		}
		DESTROY(Option_table)(&option_table);
		DEALLOCATE(region_path);
		if (fe_field)
		{
			DEACCESS(FE_field)(&fe_field);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_gfx_smooth.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_smooth */

int gfx_timekeeper(struct Parse_state *state,void *dummy_to_be_modified,
	void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
==============================================================================*/
{
	char every, loop, maximum_flag, minimum_flag, once, play, set_time_flag,
		skip, speed_flag, stop, swing;
	double maximum, minimum, set_time, speed;
	int return_code;
	static struct Modifier_entry option_table[]=
	{
		{"every_frame",NULL,NULL,set_char_flag},
		{"loop",NULL,NULL,set_char_flag},
		{"maximum",NULL,NULL,set_double_and_char_flag},
		{"minimum",NULL,NULL,set_double_and_char_flag},
		{"once",NULL,NULL,set_char_flag},
		{"play",NULL,NULL,set_char_flag},
		{"set_time",NULL,NULL,set_double_and_char_flag},
		{"skip_frames",NULL,NULL,set_char_flag},
		{"speed",NULL,NULL,set_double_and_char_flag},
		{"stop",NULL,NULL,set_char_flag},
		{"swing",NULL,NULL,set_char_flag},
		{NULL,NULL,NULL,NULL}
	};
	struct Cmiss_command_data *command_data;
	struct Time_keeper *time_keeper;

	ENTER(gfx_timekeeper);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (NULL != (command_data = (struct Cmiss_command_data *)command_data_void))
		{
			if (state->current_token)
			{
				if (strcmp(PARSER_HELP_STRING,state->current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token))
				{
					if (!strcmp(state->current_token, "default"))
					{
						/* Continue */
						return_code = shift_Parse_state(state,1);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"gfx_timekeeper.  Only a default timekeeper at the moment");
						return_code=0;
					}
				}
				else
				{
					display_message(INFORMATION_MESSAGE,
						"\n      TIMEKEEPER_NAME");
					/* By not shifting the parse state the rest of the help should come out */
					return_code = 1;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_timekeeper.  Missing timekeeper name");
				return_code=0;
			}
			if (return_code)
			{
				/* initialise defaults */
				if (NULL != (time_keeper = command_data->default_time_keeper))
				{
					maximum = Time_keeper_get_maximum(time_keeper);
					minimum = Time_keeper_get_minimum(time_keeper);
					set_time = Time_keeper_get_time(time_keeper);
					speed = Time_keeper_get_speed(time_keeper);
				}
				else
				{
					maximum = 0.0;
					minimum = 0.0;
					set_time = 0.0;
					speed = 30;
				}
				every = 0;
				loop = 0;
				maximum_flag = 0;
				minimum_flag = 0;
				once = 0;
				play = 0;
				set_time_flag = 0;
				skip = 0;
				speed_flag = 0;
				stop = 0;
				swing = 0;

				(option_table[0]).to_be_modified = &every;
				(option_table[1]).to_be_modified = &loop;
				(option_table[2]).to_be_modified = &maximum;
				(option_table[2]).user_data = &maximum_flag;
				(option_table[3]).to_be_modified = &minimum;
				(option_table[3]).user_data = &minimum_flag;
				(option_table[4]).to_be_modified = &once;
				(option_table[5]).to_be_modified = &play;
				(option_table[6]).to_be_modified = &set_time;
				(option_table[6]).user_data = &set_time_flag;
				(option_table[7]).to_be_modified = &skip;
				(option_table[8]).to_be_modified = &speed;
				(option_table[8]).user_data = &speed_flag;
				(option_table[9]).to_be_modified = &stop;
				(option_table[10]).to_be_modified = &swing;
				return_code=process_multiple_options(state,option_table);

				if(return_code)
				{
					if((loop + once + swing) > 1)
					{
						display_message(ERROR_MESSAGE,
							"gfx_timekeeper.  Specify only one of loop, swing or once");
						return_code = 0;
					}
					if(every && skip)
					{
						display_message(ERROR_MESSAGE,
							"gfx_timekeeper.  Specify only one of every_frame or skip_frames");
						return_code = 0;
					}
					if(play && stop)
					{
						display_message(ERROR_MESSAGE,
							"gfx_timekeeper.  Specify only one of play or stop");
						return_code = 0;
					}
				}
				if (return_code)
				{
					if ( time_keeper )
					{
						if ( set_time_flag )
						{
							Time_keeper_request_new_time(time_keeper, set_time);
						}
						if ( speed_flag )
						{
							Time_keeper_set_speed(time_keeper, speed);
						}
						if ( maximum_flag )
						{
							Time_keeper_set_maximum(time_keeper, maximum);
						}
						if ( minimum_flag )
						{
							Time_keeper_set_minimum(time_keeper, minimum);
						}
						if ( loop )
						{
							Time_keeper_set_play_loop(time_keeper);
						}
						if ( swing )
						{
							Time_keeper_set_play_swing(time_keeper);
						}
						if ( once )
						{
							Time_keeper_set_play_once(time_keeper);
						}
						if ( every )
						{
							Time_keeper_set_play_every_frame(time_keeper);
						}
						if ( skip )
						{
							Time_keeper_set_play_skip_frames(time_keeper);
						}
						if ( play )
						{
							Time_keeper_play(time_keeper, TIME_KEEPER_PLAY_FORWARD);
						}
						if ( stop )
						{
							Time_keeper_stop(time_keeper);
						}
#if defined (WX_USER_INTERFACE)
						if (command_data->graphics_window_manager)
						{
							 FOR_EACH_OBJECT_IN_MANAGER(Graphics_window)(
									Graphics_window_update_time_settings_wx,(void *)NULL,
									command_data->graphics_window_manager);
						}
#endif /*defined (WX_USER_INTERFACE)*/
					}
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_timekeeper.  Missing command data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_timekeeper.  Missing state");
		return_code=0;
	}

	LEAVE;

	return (return_code);
} /* gfx_timekeeper */

static int gfx_transform_tool(struct Parse_state *state,
	void *dummy_user_data,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 6 October 2000

DESCRIPTION :
Executes a GFX TRANSFORM_TOOL command.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Interactive_tool *transform_tool;

	ENTER(execute_command_gfx_transform_tool);
	USE_PARAMETER(dummy_user_data);
	if (state&&(command_data=(struct Cmiss_command_data *)command_data_void)
		&& (transform_tool=command_data->transform_tool))
	{
		return_code = Transform_tool_execute_command_with_parse_state(transform_tool, state);
		if (return_code)
		{
#if defined (WX_USER_INTERFACE)
			FOR_EACH_OBJECT_IN_MANAGER(Graphics_window)(
				Graphics_window_update_Interactive_tool,(void *)transform_tool,
				command_data->graphics_window_manager);
#endif /*(WX_USER_INTERFACE)*/
			Cmiss_scene_viewer_package_update_Interactive_tool(
				command_data->scene_viewer_package,	transform_tool);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_gfx_transform_tool.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_transform_tool */

#if defined (WX_USER_INTERFACE)
static int execute_command_gfx_update(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 28 October 1998

DESCRIPTION :
Executes a GFX UPDATE command.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Graphics_window *window;
	static struct Modifier_entry option_table[]=
	{
		{"window",NULL,NULL,set_Graphics_window},/*???DB. "on" ? */
		{NULL,NULL,NULL,NULL}
	};

	ENTER(execute_command_gfx_update);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (NULL != (command_data = (struct Cmiss_command_data *)command_data_void))
		{
			/* initialize defaults */
			window=(struct Graphics_window *)NULL;
			(option_table[0]).to_be_modified= &window;
			(option_table[0]).user_data=command_data->graphics_window_manager;
			return_code=process_multiple_options(state,option_table);
			/* no errors, not asking for help */
			if (return_code)
			{
				if (window)
				{
					return_code=Graphics_window_update_now(window);
				}
				else
				{
					return_code=FOR_EACH_OBJECT_IN_MANAGER(Graphics_window)(
						Graphics_window_update_now_iterator,(void *)NULL,
						command_data->graphics_window_manager);
				}
			} /* parse error,help */
			if (window)
			{
				DEACCESS(Graphics_window)(&window);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_gfx_update.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_gfx_update.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_update */
#endif /* defined (WX_USER_INTERFACE) */

static int gfx_write_All(struct Parse_state *state,
	 void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 3 August 2007

DESCRIPTION :
If an zip file is not specified a file selection box is presented to the
user, otherwise files are written.
Can also write individual groups with the <group> option.
==============================================================================*/
{
	 FILE *com_file;
	 char *com_file_name, *data_file_name, *elem_file_name, *file_name,
		 *node_file_name;
	 enum FE_write_criterion write_criterion;
	 enum FE_write_recursion write_recursion;
	 int data_return_code, elem_return_code,
			node_return_code, return_code, data_fd,
			elem_fd, node_fd, com_return_code;
	 struct Cmiss_command_data *command_data;
	 struct Multiple_strings field_names;
	 struct Option_table *option_table;
	 struct MANAGER(Graphical_material) *graphical_material_manager;
	 struct MANAGER(Computed_field) *computed_field_manager;
	 struct LIST(Computed_field) *list_of_fields;
	 struct List_Computed_field_commands_data list_commands_data;
	 static const char	*command_prefix;
	 FE_value time;
#if defined (WX_USER_INTERFACE)
#if defined (WIN32_SYSTEM)
	 char temp_data[L_tmpnam];
	 char temp_elem[L_tmpnam];
	 char temp_node[L_tmpnam];
#else /* (WIN32_SYSTEM) */
	 char temp_data[] = "dataXXXXXX";
	 char temp_elem[] = "elemXXXXXX";
	 char temp_node[] = "nodeXXXXXX";
#endif /* (WIN32_SYSTEM) */
#else /* (WX_USER_INTERFACE) */
	 char *temp_data = NULL;
	 char *temp_elem = NULL;
	 char *temp_node = NULL;
#endif /* (WX_USER_INTERFACE) */

	 ENTER(gfx_write_all);
	 USE_PARAMETER(dummy_to_be_modified);
	 if (state && (command_data=(struct Cmiss_command_data *)command_data_void))
	 {
			data_fd = 1;
			elem_fd = 1;
			node_fd = 1;
			data_return_code = 1;
			elem_return_code = 1;
			node_return_code = 1;
			com_return_code = 1;
			return_code = 1;

			Cmiss_region_id root_region = Cmiss_region_access(command_data->root_region);
			char *region_or_group_path = 0;
			file_name = (char *)NULL;
			com_file_name = (char *)NULL;
			data_file_name = (char *)NULL;
			elem_file_name = (char *)NULL;
			node_file_name = (char *)NULL;
			time = 0.0;
			field_names.number_of_strings = 0;
			field_names.strings = (char **)NULL;
			write_criterion = FE_WRITE_COMPLETE_GROUP;
			write_recursion = FE_WRITE_RECURSIVE;

			option_table = CREATE(Option_table)();
			/* complete_group|with_all_listed_fields|with_any_listed_fields */
			OPTION_TABLE_ADD_ENUMERATOR(FE_write_criterion)(option_table, &write_criterion);
			/* fields */
			Option_table_add_multiple_strings_entry(option_table, "fields",
				&field_names, "FIELD_NAME [& FIELD_NAME [& ...]]|all|none|[all]");
			/* group */
			Option_table_add_string_entry(option_table, "group", &region_or_group_path,
				" RELATIVE_PATH_TO_REGION");
			/* recursion */
			OPTION_TABLE_ADD_ENUMERATOR(FE_write_recursion)(option_table, &write_recursion);
			/* root_region */
			Option_table_add_set_Cmiss_region(option_table, "root",
				command_data->root_region, &root_region);
			/* time */
			Option_table_add_entry(option_table, "time",
				&time, (void *)NULL, set_FE_value);
			/* default option: file name */
			Option_table_add_default_string_entry(option_table, &file_name, "FILE_NAME");

			if (0 != (return_code = Option_table_multi_parse(option_table, state)))
			{
				 if (file_name)
				 {
						int length = strlen(file_name);
						if (ALLOCATE(com_file_name, char, length+6))
						{
							 strcpy(com_file_name, file_name);
							 strcat(com_file_name, ".com");
							 com_file_name[length+5]='\0';
						}
						if (ALLOCATE(data_file_name, char, length+9))
						{
							 strcpy(data_file_name, file_name);
							 strcat(data_file_name, ".exdata");
							 data_file_name[length+8]='\0';
						}
						if (ALLOCATE(elem_file_name, char, length+9))
						{
							 strcpy(elem_file_name, file_name);
							 strcat(elem_file_name, ".exelem");
							 elem_file_name[length+8]='\0';
						}
						if (ALLOCATE(node_file_name, char, length+9))
						{
							 strcpy(node_file_name, file_name);
							 strcat(node_file_name, ".exnode");
							 node_file_name[length+8]='\0';
						}
				 }
				enum FE_write_fields_mode write_fields_mode = FE_WRITE_ALL_FIELDS;
				if ((1 == field_names.number_of_strings) && field_names.strings)
				{
					if (fuzzy_string_compare(field_names.strings[0], "all"))
					{
						write_fields_mode = FE_WRITE_ALL_FIELDS;
					}
					else if (fuzzy_string_compare(field_names.strings[0], "none"))
					{
						write_fields_mode = FE_WRITE_NO_FIELDS;
					}
					else
					{
						write_fields_mode = FE_WRITE_LISTED_FIELDS;
					}
				}
				else if (1 < field_names.number_of_strings)
				{
					write_fields_mode = FE_WRITE_LISTED_FIELDS;
				}
				if ((FE_WRITE_LISTED_FIELDS != write_fields_mode) &&
					(FE_WRITE_COMPLETE_GROUP != write_criterion))
				{
					display_message(WARNING_MESSAGE,
						"gfx write all:  Must specify fields to use %s",
						ENUMERATOR_STRING(FE_write_criterion)(write_criterion));
					return_code = 0;
					data_return_code = 0;
					elem_return_code = 0;
					node_return_code = 0;
				}
				Cmiss_region_id region = Cmiss_region_access(root_region);
				Cmiss_field_group_id group = 0;
				if (region_or_group_path)
				{
					Parse_state *temp_parse_state = create_Parse_state(region_or_group_path);
					return_code = set_Cmiss_region_or_group(temp_parse_state, (void *)&region, (void *)&group);
					destroy_Parse_state(&temp_parse_state);
				}
				if (return_code)
				{
					if (!file_name)
					{
/* 						com_file_name = "temp.com"; */
						if (!(com_file_name = confirmation_get_write_filename(".com",
										 command_data->user_interface
#if defined(WX_USER_INTERFACE)
										 , command_data->execute_command
#endif /*defined (WX_USER_INTERFACE) */
																																	 )))
						{
							 com_return_code = 0;
						}
						if (!(data_file_name = confirmation_get_write_filename(".exdata",
										 command_data->user_interface
#if defined(WX_USER_INTERFACE)
										 , command_data->execute_command
#endif /*defined (WX_USER_INTERFACE) */
																																	 )))
						{
							 data_return_code = 0;
						}
						if (!(elem_file_name = confirmation_get_write_filename(".exelem",
										 command_data->user_interface
#if defined(WX_USER_INTERFACE)
										 , command_data->execute_command
#endif /*defined (WX_USER_INTERFACE) */
																															)))
						{
							 elem_return_code = 0;
						}

						if (!(node_file_name = confirmation_get_write_filename(".exnode",
										 command_data->user_interface
#if defined(WX_USER_INTERFACE)
										 , command_data->execute_command
#endif /*defined (WX_USER_INTERFACE) */
																															)))
						{
							 node_return_code = 0;
						}
#if defined(WX_USER_INTERFACE)
						file_name = confirmation_get_write_filename(".zip",
							 command_data->user_interface
							 , command_data->execute_command);
#endif /*defined (WX_USER_INTERFACE) */
					}
#if defined (WX_USER_INTERFACE) && defined (WIN32_SYSTEM)
					if (com_file_name)
					{
						com_file_name = CMISS_set_directory_and_filename_WIN32(com_file_name,
							command_data);
					}
					if (data_file_name)
					{
						data_file_name = CMISS_set_directory_and_filename_WIN32(data_file_name,
							command_data);
					}
					if (elem_file_name)
					{
						elem_file_name = CMISS_set_directory_and_filename_WIN32(elem_file_name,
							command_data);
					}
					if (node_file_name)
					{
						node_file_name = CMISS_set_directory_and_filename_WIN32(node_file_name,
							 command_data);
					}
#endif /* defined (WX_USER_INTERFACE) && defined (WIN32_SYSTEM) */
				}
				if (com_return_code)
				{
					com_return_code = check_suffix(&com_file_name,".com");
				}
				if (data_return_code)
				{
					data_return_code = check_suffix(&data_file_name,".exdata");
				}
				if (elem_return_code)
				{
					elem_return_code = check_suffix(&elem_file_name,".exelem");
				}
				if (node_return_code)
				{
					node_return_code = check_suffix(&node_file_name,".exnode");
				}
#if defined (WX_USER_INTERFACE)
#if defined (WIN32_SYSTEM)
				 /* 	Non MS-windows platform does not have mkstemp implemented,
						therefore tmpnam is used.*/
				 if (data_return_code)
				 {
						tmpnam(temp_data);
						if (temp_data == NULL)
						{
							 data_fd = -1;
						}
				 }
				 if (elem_return_code)
				 {
							 tmpnam(temp_elem);
							 if (temp_elem == NULL)
							 {
									elem_fd = -1;
							 }
				 }
				 if (node_return_code)
				 {
						tmpnam(temp_node);
						if (temp_node == NULL)
						{
							 node_fd = -1;
						}
				 }
#else
				 /* Non MS-windows platform has mkstemp implemented into it*/
				 if (data_return_code)
				 {
							 data_fd = mkstemp((char *)temp_data);
				 }
				 if (elem_return_code)
				 {
							 elem_fd = mkstemp((char *)temp_elem);
				 }
				 if (node_return_code)
				 {
							 node_fd = mkstemp((char *)temp_node);
				 }
#endif /* (WIN32_SYSTEM) */
#else /* (WX_USER_INTERFACE) */
				 /* Non wx_user_interface won't be able to stored the file in
						a zip file at the moment */
				 if (data_return_code)
				 {
							 temp_data = data_file_name;
				 }
				 if (elem_return_code)
				 {
							 temp_elem = elem_file_name;
				 }
				 if (node_return_code)
				 {
							 temp_node = node_file_name;
				 }
#endif /* (WX_USER_INTERFACE) */
				 if (data_fd == -1)
				 {
						display_message(ERROR_MESSAGE,
							 "gfx_write_All.  Could not open temporary data file");
				 }
				 else
				 {
						if (!(data_return_code = write_exregion_file_of_name(temp_data,
							region, group, root_region,
							/*write_elements*/0, /*write_nodes*/0, /*write_data*/1,
							write_fields_mode, field_names.number_of_strings, field_names.strings,
							time, write_criterion, write_recursion)))
						{
							 display_message(ERROR_MESSAGE,
									"gfx_write_All.  Could not create temporary data file");
						}
				 }

				 if (elem_fd == -1)
				 {
						display_message(ERROR_MESSAGE,
							 "gfx_write_All.  Could not open temporary elem file");
				 }
				 else
				 {
						if (!(elem_return_code = write_exregion_file_of_name(temp_elem,
							region, group, root_region,
							/*write_elements*/1, /*write_nodes*/0, /*write_data*/0,
							write_fields_mode, field_names.number_of_strings, field_names.strings,
							time, write_criterion, write_recursion)))
						{
							 display_message(ERROR_MESSAGE,
									"gfx_write_All.  Could not create temporary elem file");
						}
				 }

				 if (node_fd == -1)
				 {
						display_message(ERROR_MESSAGE,
							 "gfx_write_All.  Could not open temporary node file");
				 }
				 else
				 {
						if (!(node_return_code = write_exregion_file_of_name(temp_node,
							region, group, root_region,
							/*write_elements*/0, /*write_nodes*/1, /*write_data*/0,
							write_fields_mode, field_names.number_of_strings, field_names.strings,
							time, write_criterion, write_recursion)))
						{
							 display_message(ERROR_MESSAGE,
									"gfx_write_All.  Could not create temporary node file");
						}
				 }
				 if (com_return_code)
				 {
						if (NULL != (com_file = fopen("temp_file_com.com", "w")))
						{
							 if (data_file_name)
							 {
									fprintf(com_file, "gfx read data %s\n",data_file_name);
							 }
							 if (node_file_name)
							 {
									fprintf(com_file, "gfx read nodes %s\n",node_file_name);
							 }
							 if (elem_file_name)
							 {
									fprintf(com_file, "gfx read elements %s\n",elem_file_name);
							 }
							 fclose(com_file);
							 if (command_data->computed_field_package && (computed_field_manager=
										 Computed_field_package_get_computed_field_manager(
												command_data->computed_field_package)))
							 {
									if (NULL != (list_of_fields = CREATE(LIST(Computed_field))()))
									{
										 command_prefix="gfx define field ";
										 list_commands_data.command_prefix = command_prefix;
										 list_commands_data.listed_fields = 0;
										 list_commands_data.computed_field_list = list_of_fields;
										 list_commands_data.computed_field_manager =
												computed_field_manager;
										 while (FOR_EACH_OBJECT_IN_MANAGER(Computed_field)(
															 write_Computed_field_commands_if_managed_source_fields_in_list_to_comfile,
															 (void *)&list_commands_data, computed_field_manager) &&
												(0 != list_commands_data.listed_fields))
										 {
												list_commands_data.listed_fields = 0;
										 }
										 DESTROY(LIST(Computed_field))(&list_of_fields);
									}
									else
									{
										 return_code=0;
									}
									if (!return_code)
									{
										 display_message(ERROR_MESSAGE,
												"gfx_write_All.  Could not list field commands");
									}
							 }

							 if (command_data->spectrum_manager)
							 {
									FOR_EACH_OBJECT_IN_MANAGER(Spectrum)(
										 for_each_spectrum_list_or_write_commands, (void *)"true", command_data->spectrum_manager);
							 }
							 if (NULL != (graphical_material_manager =
									Material_package_get_material_manager(command_data->material_package)))
							 {
									command_prefix="gfx create material ";
									return_code=FOR_EACH_OBJECT_IN_MANAGER(Graphical_material)(
										 write_Graphical_material_commands_to_comfile,(void *)command_prefix,
										 graphical_material_manager);
							 }
							 return_code =1;
#if defined (USE_CMGUI_GRAPHICS_WINDOW)
							 return_code=FOR_EACH_OBJECT_IN_MANAGER(Graphics_window)(
									write_Graphics_window_commands_to_comfile,(void *)NULL,
									command_data->graphics_window_manager);
#endif /*defined (USE_CMGUI_GRAPHICS_WINDOW)*/
							 rename("temp_file_com.com", com_file_name);
						}
				 }
#if defined (WX_USER_INTERFACE)
				 if (data_file_name && elem_file_name && node_file_name)
				 {
						filedir_compressing_process_wx_compress(com_file_name, data_file_name,
							 elem_file_name, node_file_name, data_return_code, elem_return_code,
							 node_return_code, file_name, temp_data, temp_elem, temp_node);
				 }
#if !defined (WIN32_SYSTEM)
				 if (unlink(temp_data) == -1)
				 {
						display_message(ERROR_MESSAGE,
							 "gfx_write_All.  Could not unlink temporary data file");
				 }
				 if (unlink(temp_elem) == -1)
				 {
						display_message(ERROR_MESSAGE,
							 "gfx_write_All.  Could not unlink temporary elem file");
				 }
				 if (unlink(temp_node) == -1)
				 {
						display_message(ERROR_MESSAGE,
							 "gfx_write_All.  Could not unlink temporary node file");
				 }
				 if (unlink(com_file_name) == -1)
				 {
						display_message(ERROR_MESSAGE,
							 "compressing_process_wx_compress.  Could not unlink temporary com file");
				 }
#endif /*!definde (WIN32_SYSTEM)*/
#endif /* defined (WX_USER_INTERFACE) */
				 if (com_file_name)
				 {
						DEALLOCATE(com_file_name);
				 }
				 if (node_file_name)
				 {
						DEALLOCATE(node_file_name);
				 }
				 if (elem_file_name)
				 {
						DEALLOCATE(elem_file_name);
				 }
				 if (data_file_name)
				 {
						DEALLOCATE(data_file_name);
				 }
				Cmiss_field_group_destroy(&group);
				Cmiss_region_destroy(&region);
			}
			DESTROY(Option_table)(&option_table);
			if (region_or_group_path)
			{
				 DEALLOCATE(region_or_group_path);
			}
			if (field_names.strings)
			{
				for (int i = 0; i < field_names.number_of_strings; i++)
				{
					DEALLOCATE(field_names.strings[i]);
				}
				DEALLOCATE(field_names.strings);
			}
			if (file_name)
			{
				 DEALLOCATE(file_name);
			}
			Cmiss_region_destroy(&root_region);
	 }
	 else
	 {
			display_message(ERROR_MESSAGE, "gfx_write_All.  Invalid argument(s)");
			return_code = 0;
	 }
	 LEAVE;

	 return (return_code);
} /* gfx_write_elements */

#if defined (NEW_CODE)
static int gfx_write_Com(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 25 November 1999

DESCRIPTION :
Writes a com file
==============================================================================*/
{
	char write_all_curves_flag;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Modifier_entry option_table[]=
	{
		{"all",NULL,NULL,set_char_flag},
		{NULL,NULL,NULL,set_Curve}
	};
	struct Curve *curve;

	ENTER(gfx_write_Curve);
	USE_PARAMETER(dummy_to_be_modified);
	if (state&&(command_data=(struct Cmiss_command_data *)command_data_void))
	{
		return_code=1;
		write_all_curves_flag=0;
		curve=(struct Curve *)NULL;
		(option_table[0]).to_be_modified= &write_all_curves_flag;
		(option_table[1]).to_be_modified= &curve;
		(option_table[1]).user_data=command_data->curve_manager;
		if (0 != (return_code = process_multiple_options(state,option_table)))
		{
			if (write_all_curves_flag&&!curve)
			{
				return_code=FOR_EACH_OBJECT_IN_MANAGER(Curve)(
					write_Curve,(void *)NULL,
					command_data->curve_manager);
			}
			else if (curve&&!write_all_curves_flag)
			{
				return_code=write_Curve(curve,(void *)NULL);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_write_Curve.  Specify either a curve name or 'all'");
				return_code=0;
			}
		}
		if (curve)
		{
			DEACCESS(Curve)(&curve);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_write_Curve.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_write_Curve */


#endif /* defined (NEW_CODE) */

static int gfx_write_Curve(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 25 November 1999

DESCRIPTION :
Writes an individual curve or all curves to filename(s) stemming from the name
of the curve, eg. "name" -> name.curve.com name.curve.exnode name.curve.exelem
==============================================================================*/
{
	char write_all_curves_flag;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Modifier_entry option_table[]=
	{
		{"all",NULL,NULL,set_char_flag},
		{NULL,NULL,NULL,set_Curve}
	};
	struct Curve *curve;

	ENTER(gfx_write_Curve);
	USE_PARAMETER(dummy_to_be_modified);
	if (state&&(command_data=(struct Cmiss_command_data *)command_data_void))
	{
		return_code=1;
		write_all_curves_flag=0;
		curve=(struct Curve *)NULL;
		(option_table[0]).to_be_modified= &write_all_curves_flag;
		(option_table[1]).to_be_modified= &curve;
		(option_table[1]).user_data=command_data->curve_manager;
		if (0 != (return_code = process_multiple_options(state,option_table)))
		{
			if (write_all_curves_flag&&!curve)
			{
				return_code=FOR_EACH_OBJECT_IN_MANAGER(Curve)(
					write_Curve,(void *)NULL,
					command_data->curve_manager);
			}
			else if (curve&&!write_all_curves_flag)
			{
				return_code=write_Curve(curve,(void *)NULL);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_write_Curve.  Specify either a curve name or 'all'");
				return_code=0;
			}
		}
		if (curve)
		{
			DEACCESS(Curve)(&curve);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_write_Curve.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_write_Curve */

static int gfx_write_elements(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 30 April 2009

DESCRIPTION :
If an element file is not specified a file selection box is presented to the
user, otherwise the element file is written.
Can also write individual element groups with the <group> option.
==============================================================================*/
{
	const char *file_ext = ".exelem";
	enum FE_write_criterion write_criterion;
	enum FE_write_recursion write_recursion;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Multiple_strings field_names;
	struct Option_table *option_table;
	FE_value time;

	ENTER(gfx_write_elements);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (command_data=(struct Cmiss_command_data *)command_data_void))
	{
		return_code = 1;
		Cmiss_region_id root_region = Cmiss_region_access(command_data->root_region);
		char *region_or_group_path = 0;
		field_names.number_of_strings = 0;
		field_names.strings = (char **)NULL;
		char *file_name = 0;
		char nodes_flag = 0;
		write_criterion = FE_WRITE_COMPLETE_GROUP;
		write_recursion = FE_WRITE_RECURSIVE;
		time = 0.0;
		option_table = CREATE(Option_table)();
		Option_table_add_help(option_table,
			"Write elements and element fields in EX format to FILE_NAME. "
			"Output is restricted to the specified <root> region which forms the "
			"root region in the EX file, and optionally a sub-group or region "
			"specified with the relative <group> path. "
			"Output can be restricted to just <fields> listed in required order, or "
			"\'none\' to list just object identifiers. "
			"Recursion options control whether sub-groups and sub-regions are output "
			"with the chosen root region or group. "
			"Specify <nodes> to include nodes and node fields in the same file. "
			"with the chosen root region or group. "
			"Time option specifies nodes and node fields at time to be output if nodes "
			"or node fields are time dependent. If time is out of range then the nodal "
			"values at the nearest valid time will be output. Time is ignored if node "
			"is not time dependent. ");
		/* complete_group|with_all_listed_fields|with_any_listed_fields */
		OPTION_TABLE_ADD_ENUMERATOR(FE_write_criterion)(option_table, &write_criterion);
		/* fields */
		Option_table_add_multiple_strings_entry(option_table, "fields",
			&field_names, "FIELD_NAME [& FIELD_NAME [& ...]]|all|none|[all]");
		/* group */
		Option_table_add_string_entry(option_table, "group", &region_or_group_path,
			" RELATIVE_PATH_TO_REGION/GROUP");
		/* nodes */
		Option_table_add_char_flag_entry(option_table, "nodes", &nodes_flag);
		/* recursion */
		OPTION_TABLE_ADD_ENUMERATOR(FE_write_recursion)(option_table, &write_recursion);
		/* root_region */
		Option_table_add_set_Cmiss_region(option_table, "root",
			command_data->root_region, &root_region);
		/* time */
		Option_table_add_entry(option_table, "time", &time, (void*)NULL, set_FE_value);
		/* default option: file name */
		Option_table_add_default_string_entry(option_table, &file_name, "FILE_NAME");

		if (0 != (return_code = Option_table_multi_parse(option_table, state)))
		{
			enum FE_write_fields_mode write_fields_mode = FE_WRITE_ALL_FIELDS;
			if ((1 == field_names.number_of_strings) && field_names.strings)
			{
				if (fuzzy_string_compare(field_names.strings[0], "all"))
				{
					write_fields_mode = FE_WRITE_ALL_FIELDS;
				}
				else if (fuzzy_string_compare(field_names.strings[0], "none"))
				{
					write_fields_mode = FE_WRITE_NO_FIELDS;
				}
				else
				{
					write_fields_mode = FE_WRITE_LISTED_FIELDS;
				}
			}
			else if (1 < field_names.number_of_strings)
			{
				write_fields_mode = FE_WRITE_LISTED_FIELDS;
			}
			if ((FE_WRITE_LISTED_FIELDS != write_fields_mode) &&
				(FE_WRITE_COMPLETE_GROUP != write_criterion))
			{
				display_message(WARNING_MESSAGE,
					"gfx write elements:  Must list fields to use %s",
					ENUMERATOR_STRING(FE_write_criterion)(write_criterion));
				return_code = 0;

			}
			Cmiss_region_id region = Cmiss_region_access(root_region);
			Cmiss_field_group_id group = 0;
			if (region_or_group_path)
			{
				Parse_state *temp_parse_state = create_Parse_state(region_or_group_path);
				return_code = set_Cmiss_region_or_group(temp_parse_state, (void *)&region, (void *)&group);
				destroy_Parse_state(&temp_parse_state);
			}
			if (return_code)
			{
				if (!file_name)
				{
					if (!(file_name = confirmation_get_write_filename(file_ext,
						command_data->user_interface
#if defined(WX_USER_INTERFACE)
						, command_data->execute_command
#endif /*defined (WX_USER_INTERFACE) */
						)))
					{
						return_code = 0;
					}
				}
#if defined (WX_USER_INTERFACE) && defined (WIN32_SYSTEM)
				if (file_name)
				{
					file_name = CMISS_set_directory_and_filename_WIN32(file_name,
						command_data);
				}
#endif /* defined (WX_USER_INTERFACE) && (WIN32_SYSTEM) */
			}
			if (return_code)
			{
				/* open the file */
				if (0 != (return_code = check_suffix(&file_name, ".exelem")))
				{
					return_code = write_exregion_file_of_name(file_name, region, group, root_region,
						/*write_elements*/1, (int)nodes_flag, /*write_data*/0,
						write_fields_mode, field_names.number_of_strings, field_names.strings,
						time, write_criterion, write_recursion);
				}
			}
			Cmiss_field_group_destroy(&group);
			Cmiss_region_destroy(&region);
		}
		DESTROY(Option_table)(&option_table);
		if (region_or_group_path)
		{
			DEALLOCATE(region_or_group_path);
		}
		if (field_names.strings)
		{
			for (int i = 0; i < field_names.number_of_strings; i++)
			{
				DEALLOCATE(field_names.strings[i]);
			}
			DEALLOCATE(field_names.strings);
		}
		if (file_name)
		{
			DEALLOCATE(file_name);
		}
		Cmiss_region_destroy(&root_region);
	}
	else
	{
		display_message(ERROR_MESSAGE, "gfx_write_elements.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_write_elements */

static int gfx_write_nodes(struct Parse_state *state,
	void *use_data, void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 30 April 2009

DESCRIPTION :
If a nodes file is not specified a file selection box is presented to the user,
otherwise the nodes file is written.
Can now specify individual node groups to write with the <group> option.
If <use_data> is set, writing data, otherwise writing nodes.
==============================================================================*/
{
	static const char *data_file_ext = ".exdata";
	static const char *node_file_ext = ".exnode";
	const char *file_ext;
	enum FE_write_criterion write_criterion;
	enum FE_write_recursion write_recursion;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Multiple_strings field_names;
	struct Option_table *option_table;
	FE_value time;

	ENTER(gfx_write_nodes);
	if (state && (command_data = (struct Cmiss_command_data *)command_data_void))
	{
		return_code = 1;
		time = 0.0;
		Cmiss_region_id root_region = Cmiss_region_access(command_data->root_region);
		char *region_or_group_path = 0;
		if (use_data)
		{
			file_ext = data_file_ext;
		}
		else
		{
			file_ext = node_file_ext;
		}
		field_names.number_of_strings = 0;
		field_names.strings = (char **)NULL;
		char *file_name = 0;
		write_criterion = FE_WRITE_COMPLETE_GROUP;
		write_recursion = FE_WRITE_RECURSIVE;

		option_table = CREATE(Option_table)();
		if (use_data)
		{
			Option_table_add_help(option_table, "Write data nodes and data node fields");
		}
		else
		{
			Option_table_add_help(option_table, "Write nodes and node fields");
		}
		Option_table_add_help(option_table,
			" in EX format to FILE_NAME. "
			"Output is restricted to the specified <root> region which forms the "
			"root region in the EX file, and optionally a region or sub-group "
			"specified by the relative path with the <group> option. "
			"Output can be restricted to just <fields> listed in required order, or "
			"\'none\' to list just object identifiers. "
			"Recursion options control whether sub-regions are output "
			"with the chosen root region or group. "
			"Time option allow user to specify at which time of nodes and node fields "
			"to be output if there nodes/node fields are time dependent. If time is out"
			"of range then the nodal values at the nearest valid time will be output. "
			"Time is ignored if node is not time dependent. ");

		/* complete_group|with_all_listed_fields|with_any_listed_fields */
		OPTION_TABLE_ADD_ENUMERATOR(FE_write_criterion)(option_table, &write_criterion);
		/* fields */
		Option_table_add_multiple_strings_entry(option_table, "fields",
			&field_names, "FIELD_NAME [& FIELD_NAME [& ...]]|all|none|[all]");
		/* group */
		Option_table_add_string_entry(option_table, "group", &region_or_group_path,
			" RELATIVE_PATH_TO_REGION/GROUP");
		/* recursion */
		OPTION_TABLE_ADD_ENUMERATOR(FE_write_recursion)(option_table, &write_recursion);
		/* root_region */
		Option_table_add_set_Cmiss_region(option_table, "root",
			command_data->root_region, &root_region);
		/* time */
		Option_table_add_entry(option_table, "time", &time, (void*)NULL, set_FE_value);
		/* default option: file name */
		Option_table_add_default_string_entry(option_table, &file_name, "FILE_NAME");

		if (0 != (return_code = Option_table_multi_parse(option_table, state)))
		{
			enum FE_write_fields_mode write_fields_mode = FE_WRITE_ALL_FIELDS;
			if ((1 == field_names.number_of_strings) && field_names.strings)
			{
				if (fuzzy_string_compare_same_length(field_names.strings[0], "all"))
				{
					write_fields_mode = FE_WRITE_ALL_FIELDS;
				}
				else if (fuzzy_string_compare_same_length(field_names.strings[0], "none"))
				{
					write_fields_mode = FE_WRITE_NO_FIELDS;
				}
				else
				{
					write_fields_mode = FE_WRITE_LISTED_FIELDS;
				}
			}
			else if (1 < field_names.number_of_strings)
			{
				write_fields_mode = FE_WRITE_LISTED_FIELDS;
			}
			if ((FE_WRITE_LISTED_FIELDS != write_fields_mode) &&
				(FE_WRITE_COMPLETE_GROUP != write_criterion))
			{
				display_message(WARNING_MESSAGE,
					"gfx write nodes/data:  Must list fields to use %s",
					ENUMERATOR_STRING(FE_write_criterion)(write_criterion));
				return_code = 0;
			}
			Cmiss_region_id region = Cmiss_region_access(root_region);
			Cmiss_field_group_id group = 0;
			if (region_or_group_path)
			{
				Parse_state *temp_parse_state = create_Parse_state(region_or_group_path);
				return_code = set_Cmiss_region_or_group(temp_parse_state, (void *)&region, (void *)&group);
				destroy_Parse_state(&temp_parse_state);
			}
			if (return_code)
			{
				if (!file_name)
				{
					if (!(file_name = confirmation_get_write_filename(file_ext,
						command_data->user_interface
#if defined (WX_USER_INTERFACE)
						, command_data->execute_command
#endif /* defined (WX_USER_INTERFACE) */
						)))
					{
						return_code = 0;
					}
				}
#if defined (WX_USER_INTERFACE) && defined (WIN32_SYSTEM)
				if (file_name)
				{
					 file_name = CMISS_set_directory_and_filename_WIN32(file_name,
							command_data);
				}
#endif /* defined (WX_USER_INTERFACE) && defined (WIN32_SYSTEM) */
			}
			if (return_code)
			{
				/* open the file */
				if (0 != (return_code = check_suffix(&file_name, file_ext)))
				{
					return_code = write_exregion_file_of_name(file_name, region, group, root_region,
						/*write_elements*/0, /*write_nodes*/!use_data, /*write_data*/(0 != use_data),
						write_fields_mode, field_names.number_of_strings, field_names.strings, time,
						write_criterion, write_recursion);
				}
			}
			Cmiss_field_group_destroy(&group);
			Cmiss_region_destroy(&region);
		}
		DESTROY(Option_table)(&option_table);
		if (region_or_group_path)
		{
			DEALLOCATE(region_or_group_path);
		}
		if (field_names.strings)
		{
			for (int i = 0; i < field_names.number_of_strings; i++)
			{
				DEALLOCATE(field_names.strings[i]);
			}
			DEALLOCATE(field_names.strings);
		}
		if (file_name)
		{
			DEALLOCATE(file_name);
		}
		Cmiss_region_destroy(&root_region);
	}
	else
	{
		display_message(ERROR_MESSAGE, "gfx_write_nodes.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_write_nodes */

static int gfx_write_region(struct Parse_state *state,
	void *dummy, void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 15 May 2003

DESCRIPTION :
If a nodes file is not specified a file selection box is presented to the user,
otherwise the nodes file is written.
Can now specify individual node groups to write with the <group> option.
If <use_data> is set, writing data, otherwise writing nodes.
==============================================================================*/
{
	char file_ext[] = ".fml", *file_name, *region_path;
	FILE *file;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct FE_field_order_info *field_order_info;
	struct Option_table *option_table;

	ENTER(gfx_write_region);
	USE_PARAMETER(dummy);
	if (state && (command_data = (struct Cmiss_command_data *)command_data_void))
	{
		return_code = 1;
		region_path = Cmiss_region_get_root_region_path();
		field_order_info = (struct FE_field_order_info *)NULL;
		file_name = (char *)NULL;

		option_table = CREATE(Option_table)();
		/* fields */
		Option_table_add_entry(option_table, "fields", &field_order_info,
			Cmiss_region_get_FE_region(command_data->root_region),
			set_FE_fields_FE_region);
		/* group */
		Option_table_add_entry(option_table, "group", &region_path,
			command_data->root_region, set_Cmiss_region_path);
		/* default option: file name */
		Option_table_add_default_string_entry(option_table, &file_name, "FILE_NAME");

		if (0 != (return_code = Option_table_multi_parse(option_table, state)))
		{
			if (!file_name)
			{
				if (!(file_name = confirmation_get_write_filename(file_ext,
					command_data->user_interface
#if defined (WX_USER_INTERFACE)
								 , command_data->execute_command
#endif /* defined (WX_USER_INTERFACE) */
																													)))
				{
					return_code = 0;
				}
			}
#if defined (WX_USER_INTERFACE) && defined (WIN32_SYSTEM)
			if (file_name)
			{
				 file_name = CMISS_set_directory_and_filename_WIN32(file_name,
						command_data);
			}
#endif /* defined (WX_USER_INTERFACE) && defined (WIN32_SYSTEM) */
			if (return_code)
			{
				/* open the file */
				if (0 != (return_code = check_suffix(&file_name, file_ext)))
				{
					file = fopen(file_name, "w");
					return_code =
						write_fieldml_01_file(file, command_data->root_region, region_path, 1, 1, field_order_info);
					fclose(file);
				}
			}
		}
		DESTROY(Option_table)(&option_table);
		if (field_order_info)
		{
			DESTROY(FE_field_order_info)(&field_order_info);
		}
		DEALLOCATE(region_path);
		if (file_name)
		{
			DEALLOCATE(file_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "gfx_write_region.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_write_region */

static int gfx_write_texture(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 12 March 2002

DESCRIPTION :
Executes a GFX WRITE TEXTURE command.
==============================================================================*/
{
	const char *current_token;
	char *file_name, *file_number_pattern;
	const char *image_file_format_string, **valid_strings;
	enum Image_file_format image_file_format;
	int number_of_bytes_per_component, number_of_valid_strings,
		original_depth_texels, original_height_texels, original_width_texels,
		return_code = 0;
	struct Cmgui_image *cmgui_image;
	struct Cmgui_image_information *cmgui_image_information;
	struct Cmiss_command_data *command_data;
	struct Option_table *option_table;
	struct Texture *texture;

	ENTER(gfx_write_texture);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (command_data=(struct Cmiss_command_data *)command_data_void))
	{
		texture = (struct Texture *)NULL;
		if (NULL != (current_token = state->current_token))
		{
			if (strcmp(PARSER_HELP_STRING, current_token) &&
				strcmp(PARSER_RECURSIVE_HELP_STRING, current_token))
			{
				if (command_data->root_region)
				{
					struct Cmiss_region *region = NULL;
					char *region_path = NULL, *field_name = NULL;
					if (Cmiss_region_get_partial_region_path(command_data->root_region,
						current_token, &region, &region_path, &field_name))
					{
						Cmiss_field_module *field_module = Cmiss_region_get_field_module(region);
						return_code=1;
						if (field_name && (strlen(field_name) > 0) &&
							(strchr(field_name, CMISS_REGION_PATH_SEPARATOR_CHAR)	== NULL))
						{
							struct Computed_field *temp_field =
								Cmiss_field_module_find_field_by_name(field_module, field_name);
							if (temp_field)
							{
								if (!Computed_field_is_image_type(temp_field,NULL))
								{
									DEACCESS(Computed_field)(&temp_field);
									display_message(ERROR_MESSAGE,
										"set_image_field.  Field specify does not contain image "
										"information.");
									return_code=0;
								}
								else
								{
									Cmiss_field_image_id image_field = Cmiss_field_cast_image(temp_field);
									texture = Cmiss_field_image_get_texture(image_field);
									Cmiss_field_image_destroy(&image_field);
									shift_Parse_state(state, 1);
								}
								DEACCESS(Computed_field)(&temp_field);
							}
						}
						else
						{
							if (field_name)
							{
								display_message(ERROR_MESSAGE,
									"gfx_write_texture:  Invalid region path or texture field name '%s'", field_name);
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"gfx_write_texture:  Missing texture field name or name matches child region '%s'", current_token);
							}
							display_parse_state_location(state);
							return_code = 0;
						}
						Cmiss_field_module_destroy(&field_module);
					}
					if (region_path)
						DEALLOCATE(region_path);
					if (field_name)
						DEALLOCATE(field_name);
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx write texture:  Missing texture name");
			return_code = 0;
		}
		if (return_code)
		{
			/* initialize defaults */
			file_name = (char *)NULL;
			file_number_pattern = (char *)NULL;
			/* default file format is to obtain it from the filename extension */
			image_file_format = UNKNOWN_IMAGE_FILE_FORMAT;
			if (texture)
			{
				/* by default, save as much information as there is in the texture */
				number_of_bytes_per_component =
					Texture_get_number_of_bytes_per_component(texture);
			}
			else
			{
				number_of_bytes_per_component = 1;
			}

			option_table = CREATE(Option_table)();
			/* image file format */
			image_file_format_string =
				ENUMERATOR_STRING(Image_file_format)(image_file_format);
			valid_strings = ENUMERATOR_GET_VALID_STRINGS(Image_file_format)(
				&number_of_valid_strings,
				(ENUMERATOR_CONDITIONAL_FUNCTION(Image_file_format) *)NULL,
				(void *)NULL);
			Option_table_add_enumerator(option_table, number_of_valid_strings,
				valid_strings, &image_file_format_string);
			/* bytes_per_component */
			Option_table_add_entry(option_table, "bytes_per_component",
				&number_of_bytes_per_component, (void *)NULL, set_int_positive);
			/* file */
			Option_table_add_entry(option_table, "file", &file_name,
				(void *)1, set_name);
			/* number_pattern */
			Option_table_add_entry(option_table, "number_pattern",
				&file_number_pattern, (void *)1, set_name);
			DEALLOCATE(valid_strings);
			return_code = Option_table_multi_parse(option_table, state);
			/* no errors, not asking for help */
			if (return_code)
			{
				if (!file_name)
				{
					if (!(file_name = confirmation_get_write_filename(NULL,
									 command_data->user_interface
#if defined (WX_USER_INTERFACE)
								 , command_data->execute_command
#endif /* defined (WX_USER_INTERFACE) */
																														)))
					{
						display_message(ERROR_MESSAGE,
							"gfx write texture:  No file name specified");
						return_code = 0;
					}
				}
				if ((1 != number_of_bytes_per_component) &&
					(2 != number_of_bytes_per_component))
				{
					display_message(ERROR_MESSAGE,
						"gfx write texture:  bytes_per_component may be 1 or 2");
					return_code = 0;
				}
			}
			if (return_code)
			{
				cmgui_image_information = CREATE(Cmgui_image_information)();
				if (image_file_format_string)
				{
					STRING_TO_ENUMERATOR(Image_file_format)(
						image_file_format_string, &image_file_format);
				}
				Cmgui_image_information_set_image_file_format(
					cmgui_image_information, image_file_format);
				Cmgui_image_information_set_number_of_bytes_per_component(
					cmgui_image_information, number_of_bytes_per_component);
				Cmgui_image_information_set_io_stream_package(cmgui_image_information,
					command_data->io_stream_package);
				if (file_number_pattern)
				{
					if (strstr(file_name, file_number_pattern))
					{
						/* number images from 1 to the number of depth texels used */
						if (Texture_get_original_size(texture, &original_width_texels,
							&original_height_texels, &original_depth_texels))
						{
							Cmgui_image_information_set_file_name_series(
								cmgui_image_information, file_name, file_number_pattern,
								/*start_file_number*/1,
								/*stop_file_number*/original_depth_texels,
								/*file_number_increment*/1);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE, "gfx write texture:  "
							"File number pattern \"%s\" not found in file name \"%s\"",
							file_number_pattern, file_name);
						return_code = 0;
					}
				}
				else
				{
					Cmgui_image_information_add_file_name(cmgui_image_information,
						file_name);
				}
				if (return_code)
				{
					if (NULL != (cmgui_image = Texture_get_image(texture)))
					{
						if (!Cmgui_image_write(cmgui_image, cmgui_image_information))
						{
							display_message(ERROR_MESSAGE,
								"gfx write texture:  Error writing image %s", file_name);
							return_code = 0;
						}
						DESTROY(Cmgui_image)(&cmgui_image);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"gfx_write_texture.  Could not get image from texture");
						return_code = 0;
					}
				}
				DESTROY(Cmgui_image_information)(&cmgui_image_information);
			}
			DESTROY(Option_table)(&option_table);
			if (file_name)
			{
				DEALLOCATE(file_name);
			}
			if (file_number_pattern)
			{
				DEALLOCATE(file_number_pattern);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "gfx_write_texture.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_write_texture */

static int execute_command_gfx_write(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 7 March 2003

DESCRIPTION :
Executes a GFX WRITE command.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Option_table *option_table;

	ENTER(execute_command_gfx_write);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (command_data = (struct Cmiss_command_data *)command_data_void))
	{
		if (state->current_token)
		{
			 //#if defined (NEW_CODE)
			option_table = CREATE(Option_table)();
			Option_table_add_entry(option_table, "all", NULL,
				command_data_void, gfx_write_All);
			//#endif /* defined NEW_CODE) */
			Option_table_add_entry(option_table, "curve", NULL,
				command_data_void, gfx_write_Curve);
			Option_table_add_entry(option_table, "data", /*use_data*/(void *)1,
				command_data_void, gfx_write_nodes);
			Option_table_add_entry(option_table, "elements", NULL,
				command_data_void, gfx_write_elements);
			Option_table_add_entry(option_table, "nodes", /*use_data*/(void *)0,
				command_data_void, gfx_write_nodes);
			Option_table_add_entry(option_table, "region", NULL,
				command_data_void, gfx_write_region);
			Option_table_add_entry(option_table, "texture", NULL,
				command_data_void, gfx_write_texture);
			return_code = Option_table_parse(option_table, state);
			DESTROY(Option_table)(&option_table);
		}
		else
		{
			set_command_prompt("gfx write", command_data);
			return_code = 1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_gfx_write.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_write */

static int execute_command_gfx(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 6 March 2003

DESCRIPTION :
Executes a GFX command.
==============================================================================*/
{
	int return_code;
	struct Option_table *option_table;
	struct Cmiss_command_data *command_data;

	ENTER(execute_command_gfx);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (command_data = (struct Cmiss_command_data *)command_data_void))
	{
		if (state->current_token)
		{
			option_table=CREATE(Option_table)();
			Option_table_add_entry(option_table, "change_identifier", NULL,
				command_data_void, gfx_change_identifier);
			Option_table_add_entry(option_table, "convert", NULL,
				command_data_void, gfx_convert);
			Option_table_add_entry(option_table, "create", NULL,
				command_data_void, execute_command_gfx_create);
#if defined (GTK_USER_INTERFACE) || defined (WIN32_USER_INTERFACE) || defined (WX_USER_INTERFACE)
			Option_table_add_entry(option_table, "data_tool", /*data_tool*/(void *)1,
			   command_data_void, execute_command_gfx_node_tool);
#endif /* defined (GTK_USER_INTERFACE) || defined (WIN32_USER_INTERFACE) || defined (WX_USER_INTERFACE)*/
			Option_table_add_entry(option_table, "define", NULL,
				command_data_void, execute_command_gfx_define);
			Option_table_add_entry(option_table, "destroy", NULL,
				command_data_void, execute_command_gfx_destroy);
			Option_table_add_entry(option_table, "draw", NULL,
				command_data_void, execute_command_gfx_draw);
			Option_table_add_entry(option_table, "edit", NULL,
				command_data_void, execute_command_gfx_edit);
#if defined (WX_USER_INTERFACE)
			Option_table_add_entry(option_table, "element_creator", NULL,
				command_data_void, execute_command_gfx_element_creator);
#endif /* defined (WX_USER_INTERFACE) */
#if defined (GTK_USER_INTERFACE) || defined (WIN32_USER_INTERFACE) || defined (CARBON_USER_INTERFACE) || defined (WX_USER_INTERFACE)
			Option_table_add_entry(option_table, "element_point_tool", NULL,
				command_data_void, execute_command_gfx_element_point_tool);
#endif /* defined (GTK_USER_INTERFACE) || defined	(WIN32_USER_INTERFACE) || defined (CARBON_USER_INTERFACE)  || defined (WX_USER_INTERFACE)*/
#if defined (GTK_USER_INTERFACE) || defined (WIN32_USER_INTERFACE) || defined (CARBON_USER_INTERFACE) || defined (WX_USER_INTERFACE)
			Option_table_add_entry(option_table, "element_tool", NULL,
				command_data_void, execute_command_gfx_element_tool);
#endif /* defined (GTK_USER_INTERFACE) || defined (WIN32_USER_INTERFACE) || defined (CARBON_USER_INTERFACE) || defined (WX_USER_INTERFACE) */
			Option_table_add_entry(option_table, "evaluate", NULL,
				command_data_void, gfx_evaluate);
			Option_table_add_entry(option_table, "export", NULL,
				command_data_void, execute_command_gfx_export);
#if defined (USE_OPENCASCADE)
			Option_table_add_entry(option_table, "import", NULL,
				command_data_void, execute_command_gfx_import);
#endif /* defined (USE_OPENCASCADE) */
			Option_table_add_entry(option_table, "list", NULL,
				command_data_void, execute_command_gfx_list);
			Option_table_add_entry(option_table, "minimise",
				NULL, (void *)command_data->root_region, gfx_minimise);
			Option_table_add_entry(option_table, "modify", NULL,
				command_data_void, execute_command_gfx_modify);
#if defined (SGI_MOVIE_FILE)
			Option_table_add_entry(option_table, "movie", NULL,
				command_data_void, gfx_movie);
#endif /* defined (SGI_MOVIE_FILE) */
#if defined (GTK_USER_INTERFACE) || defined (WIN32_USER_INTERFACE) || defined (CARBON_USER_INTERFACE) || defined (WX_USER_INTERFACE)
			Option_table_add_entry(option_table, "node_tool", /*data_tool*/(void *)0,
				command_data_void, execute_command_gfx_node_tool);
#endif /* defined (GTK_USER_INTERFACE) || defined	(WIN32_USER_INTERFACE) || defined (CARBON_USER_INTERFACE) || defined (WX_USER_INTERFACE) */
#if defined (GTK_USER_INTERFACE) || defined (WIN32_USER_INTERFACE) || defined (WX_USER_INTERFACE)
			Option_table_add_entry(option_table, "print", NULL,
				command_data_void, execute_command_gfx_print);
#endif
			Option_table_add_entry(option_table, "read", NULL,
				command_data_void, execute_command_gfx_read);
			Option_table_add_entry(option_table, "select", NULL,
				command_data_void, execute_command_gfx_select);
			Option_table_add_entry(option_table, "set", NULL,
				command_data_void, execute_command_gfx_set);
			Option_table_add_entry(option_table, "mesh", NULL,
				command_data_void, execute_command_gfx_mesh);
			Option_table_add_entry(option_table, "smooth", NULL,
				command_data_void, execute_command_gfx_smooth);
			Option_table_add_entry(option_table, "timekeeper", NULL,
				command_data_void, gfx_timekeeper);
			Option_table_add_entry(option_table, "transform_tool", NULL,
				command_data_void, gfx_transform_tool);
			Option_table_add_entry(option_table, "unselect", NULL,
				command_data_void, execute_command_gfx_unselect);
#if defined (WX_USER_INTERFACE)
			Option_table_add_entry(option_table, "update", NULL,
				command_data_void, execute_command_gfx_update);
#endif /* defined (WX_USER_INTERFACE) */
			Option_table_add_entry(option_table, "write", NULL,
				command_data_void, execute_command_gfx_write);
			return_code = Option_table_parse(option_table, state);
			DESTROY(Option_table)(&option_table);
		}
		else
		{
			set_command_prompt("gfx",command_data);
			return_code = 1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "execute_command_gfx.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx */

static int execute_command_list_memory(struct Parse_state *state,
	void *dummy_to_be_modified,void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
Executes a LIST_MEMORY command.
==============================================================================*/
{
	char increment_counter, suppress_pointers;
	int count_number,return_code,set_counter;
	static struct Modifier_entry option_table[]=
	{
		{"increment_counter",NULL,NULL,set_char_flag},
		{"suppress_pointers",NULL,NULL,set_char_flag},
		{NULL,NULL,NULL,set_int}
	};

	ENTER(execute_command_list_memory);
	USE_PARAMETER(dummy_to_be_modified);
	USE_PARAMETER(dummy_user_data);
	if (state)
	{
		count_number=0;
		increment_counter = 0;
		suppress_pointers = 0;
		(option_table[0]).to_be_modified= &increment_counter;
		(option_table[1]).to_be_modified= &suppress_pointers;
		(option_table[2]).to_be_modified= &count_number;
		return_code=process_multiple_options(state,option_table);
		/* no errors, not asking for help */
		if (return_code)
		{
			if (increment_counter)
			{
				set_counter = -1;
			}
			else
			{
				set_counter = 0;
			}
			if (suppress_pointers)
			{
				return_code=list_memory(count_number, /*show_pointers*/0,
					set_counter, /*show_structures*/0);
			}
			else
			{
				return_code=list_memory(count_number, /*show_pointers*/1,
					set_counter, /*show_structures*/1);
			}
		} /* parse error, help */
		else
		{
			/* no help */
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_list_memory.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_list_memory */

static int execute_command_read(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 27 April 1999

DESCRIPTION :
Executes a READ command.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Open_comfile_data open_comfile_data;
	struct Option_table *option_table;

	ENTER(execute_command_read);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if (NULL != (command_data = (struct Cmiss_command_data *)command_data_void))
		{
			if (state->current_token)
			{
				option_table = CREATE(Option_table)();
				/* comfile */
				open_comfile_data.file_name=(char *)NULL;
				open_comfile_data.example_flag=0;
				open_comfile_data.execute_count=1;
				open_comfile_data.examples_directory=command_data->example_directory;
				open_comfile_data.example_symbol=CMGUI_EXAMPLE_DIRECTORY_SYMBOL;
				open_comfile_data.execute_command=command_data->execute_command;
				open_comfile_data.set_command=command_data->set_command;
				open_comfile_data.io_stream_package=command_data->io_stream_package;
				open_comfile_data.file_extension=".com";
#if defined (WX_USER_INTERFACE)
				open_comfile_data.comfile_window_manager =
					command_data->comfile_window_manager;
#endif /* defined (WX_USER_INTERFACE)*/
/* #if defined (WX_USER_INTERFACE) */
/* 				change_dir(state,NULL,command_data); */
/* #endif  (WX_USER_INTERFACE)*/
				open_comfile_data.user_interface=command_data->user_interface;
				Option_table_add_entry(option_table, "comfile", NULL,
					(void *)&open_comfile_data, open_comfile);
				return_code=Option_table_parse(option_table, state);
				DESTROY(Option_table)(&option_table);
			}
			else
			{
				set_command_prompt("read",command_data);
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_read.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_read.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_read */

static int open_example(struct Parse_state *state,void *dummy_to_be_modified,
	void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 12 December 2002

DESCRIPTION :
Opens an example.
==============================================================================*/
{
	char *example, *execute_flag, *found_cm, temp_string[100];
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Option_table *option_table;

	ENTER(open_example);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (NULL != (command_data = (struct Cmiss_command_data *)command_data_void))
		{
		   example = (char *)NULL;
			execute_flag = 0;
			option_table = CREATE(Option_table)();
			/* example */
			Option_table_add_entry(option_table, "example",
				&example, (void *)1, set_name);
			/* execute */
			Option_table_add_entry(option_table, "execute",
				&execute_flag, NULL, set_char_flag);
			/* default */
			Option_table_add_entry(option_table, (const char *)NULL,
				&example, NULL, set_name);
			return_code=Option_table_multi_parse(option_table,state);
			DESTROY(Option_table)(&option_table);
			/* no errors, not asking for help */
			if (return_code)
			{
				if (!example)
				{
					display_message(ERROR_MESSAGE,
						"open_example.  You must specify an example name");
					return_code = 0;
				}
			}
			if (return_code)
			{
				/* set the examples directory */
				sprintf(temp_string,"set dir ");
				strcat(temp_string,CMGUI_EXAMPLE_DIRECTORY_SYMBOL);
				strcat(temp_string," ");
				strcat(temp_string,example);
				Execute_command_execute_string(command_data->execute_command,temp_string);
				/* The example_comfile and example_requirements strings are
					currently set as a sideeffect of "set dir" */
				if (command_data->example_requirements)
				{
					if (NULL != (found_cm = strstr(command_data->example_requirements, "cm")))
					{
						if ((found_cm[2] == 0) || (found_cm[2] == ':') ||
							(found_cm[2] == ','))
						{
							sprintf(temp_string,"create cm");
							Execute_command_execute_string(command_data->execute_command,
								temp_string);
						}
					}
				}
				sprintf(temp_string,"open comfile ");
				if (command_data->example_comfile)
				{
					strcat(temp_string,command_data->example_comfile);
				}
				else
				{
					strcat(temp_string,"example_");
					strcat(temp_string,example);
				}
				strcat(temp_string,";");
				strcat(temp_string,CMGUI_EXAMPLE_DIRECTORY_SYMBOL);
				if (execute_flag)
				{
					strcat(temp_string," execute");
				}
				return_code=Execute_command_execute_string(command_data->execute_command,
					temp_string);
			}
			if (example)
			{
				DEALLOCATE(example);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"open_example.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"open_example.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* open_comfile */

static int execute_command_open(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 18 April 2002

DESCRIPTION :
Executes a OPEN command.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Open_comfile_data open_comfile_data;
	struct Option_table *option_table;

	ENTER(execute_command_open);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if (NULL != (command_data = (struct Cmiss_command_data *)command_data_void))
		{
			if (state->current_token)
			{
				option_table = CREATE(Option_table)();
				/* comfile */
				open_comfile_data.file_name=(char *)NULL;
				open_comfile_data.example_flag=0;
				open_comfile_data.execute_count=0;
				open_comfile_data.examples_directory=command_data->example_directory;
				open_comfile_data.example_symbol=CMGUI_EXAMPLE_DIRECTORY_SYMBOL;
				open_comfile_data.execute_command=command_data->execute_command;
				open_comfile_data.set_command=command_data->set_command;
				open_comfile_data.io_stream_package=command_data->io_stream_package;
				open_comfile_data.file_extension=".com";
#if defined (WX_USER_INTERFACE)
				open_comfile_data.comfile_window_manager =
					command_data->comfile_window_manager;
#endif /* defined (WX_USER_INTERFACE) */
				open_comfile_data.user_interface=command_data->user_interface;
				Option_table_add_entry(option_table, "comfile", NULL,
					(void *)&open_comfile_data, open_comfile);
				Option_table_add_entry(option_table, "example", NULL,
					command_data_void, open_example);
				return_code=Option_table_parse(option_table, state);
/* #if defined (WX_USER_INTERFACE)  */
/*  				change_dir(state,NULL,command_data); */
/* #endif (WX_USER_INTERFACE)*/
				DESTROY(Option_table)(&option_table);
			}
			else
			{
				set_command_prompt("open",command_data);
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_open.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_open.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_open */

static int execute_command_quit(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
Executes a QUIT command.
==============================================================================*/
{
	const char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;

	ENTER(execute_command_quit);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if (!((current_token=state->current_token)&&
			!(strcmp(PARSER_HELP_STRING,current_token)&&
			strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))))
		{
			if (NULL != (command_data = (struct Cmiss_command_data *)command_data_void))
			{
				User_interface_end_application_loop(command_data->user_interface);
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"execute_command_quit.  Invalid command_data");
				return_code=0;
			}
		}
		else
		{
			/* no help */
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_quit.	Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_quit */

static int set_dir(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 17 May 2003

DESCRIPTION :
Executes a SET DIR command.
==============================================================================*/
{
	char *comfile_name, *directory_name, *example_directory, example_flag,
		*example_requirements;
	int file_name_length, return_code;
	struct Cmiss_command_data *command_data;
	static struct Modifier_entry option_table[]=
	{
		{CMGUI_EXAMPLE_DIRECTORY_SYMBOL,const_cast<char *>(CMGUI_EXAMPLE_DIRECTORY_SYMBOL),
			NULL,set_char_flag},
		{NULL,NULL,NULL,set_name}
	};

	ENTER(set_dir);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if (NULL != (command_data = (struct Cmiss_command_data *)command_data_void))
		{
			if (state->current_token)
			{
				directory_name = (char *)NULL;
				example_flag = 0;
				(option_table[0]).to_be_modified = &example_flag;
				(option_table[1]).to_be_modified = &directory_name;
				return_code=process_multiple_options(state,option_table);
				if (return_code)
				{
					if (example_flag)
					{
						if (directory_name)
						{
							/* Lookup the example path */
							if (NULL != (example_directory =
								resolve_example_path(command_data->examples_directory,
								directory_name, &comfile_name, &example_requirements)))
							{
								if (command_data->example_directory)
								{
									DEALLOCATE(command_data->example_directory);
								}
								command_data->example_directory=example_directory;
								if (command_data->example_comfile)
								{
									DEALLOCATE(command_data->example_comfile);
								}
								if (comfile_name)
								{
									command_data->example_comfile = comfile_name;
								}
								else
								{
									command_data->example_comfile = (char *)NULL;
								}
								if (command_data->example_requirements)
								{
									DEALLOCATE(command_data->example_requirements);
								}
								if (example_requirements)
								{
									command_data->example_requirements =
										example_requirements;
								}
								else
								{
									 command_data->example_requirements = (char *)NULL;
								}
#if defined (USE_PERL_INTERPRETER)
								/* Set the interpreter variable */
								interpreter_set_string(command_data->interpreter, "example",
									example_directory, &return_code);
#endif /* defined (USE_PERL_INTERPRETER) */
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"set_dir.  Unable to resolve example path.");
								return_code = 0;
							}
						}
						else
						{
							file_name_length = 1;
							if (command_data->examples_directory)
							{
								file_name_length += strlen(command_data->examples_directory);
							}
							if (ALLOCATE(example_directory,char,file_name_length))
							{
								*example_directory='\0';
								if (command_data->examples_directory)
								{
									strcat(example_directory,command_data->examples_directory);
								}
								DEALLOCATE(command_data->example_directory);
								command_data->example_directory=example_directory;
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"set_dir.  Insufficient memory");
							}
						}
					}
					else
					{
						if(chdir(directory_name))
						{
							display_message(ERROR_MESSAGE,
								"set_dir.  Unable to change to directory %s",
								directory_name);
						}
						return_code = 1;
					}
				}
				if (directory_name)
				{
					DEALLOCATE(directory_name);
				}
			}
			else
			{
				set_command_prompt("set dir",command_data);
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_set_dir.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_set_dir.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_dir */

static int execute_command_set(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 October 1998

DESCRIPTION :
Executes a SET command.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Option_table *option_table;

	ENTER(execute_command_set);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if (NULL != (command_data = (struct Cmiss_command_data *)command_data_void))
		{
			if (state->current_token)
			{
				option_table = CREATE(Option_table)();
				/* directory */
				Option_table_add_entry(option_table, "directory", NULL,
					command_data_void, set_dir);
				return_code=Option_table_parse(option_table, state);
				DESTROY(Option_table)(&option_table);
			}
			else
			{
				set_command_prompt("set",command_data);
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_set.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_set.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_set */

static int execute_command_system(struct Parse_state *state,
	void *dummy_to_be_modified,void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 19 February 1998

DESCRIPTION :
Executes a SET DIR #CMGUI_EXAMPLE_DIRECTORY_SYMBOL command.
???RC Obsolete?
==============================================================================*/
{
	char *command, *system_command;
	const char *current_token;
	int return_code;

	ENTER(execute_command_system);
	USE_PARAMETER(dummy_to_be_modified);
	USE_PARAMETER(dummy_user_data);
	return_code=0;
	/* check argument */
	if (state)
	{
		if (NULL != (current_token = state->current_token))
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				command=strstr(state->command_string,current_token);
				if (ALLOCATE(system_command,char,strlen(command)+1))
				{
					strcpy(system_command,command);
					parse_variable(&system_command);
					//system commands return 0 for no error
					return_code = !system(system_command);
					DEALLOCATE(system_command);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"execute_command_system.  Insufficient memory");
					return_code=0;
				}
			}
			else
			{
				/* no help */
				return_code=1;
			}
		}
		else
		{
			display_message(WARNING_MESSAGE,"Missing graphics object name");
			display_parse_state_location(state);
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_system.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_system */

/*
Global functions
----------------
*/
#if defined (F90_INTERPRETER) || defined (USE_PERL_INTERPRETER)
void execute_command(const char *command_string,void *command_data_void, int *quit,
  int *error)
/*******************************************************************************
LAST MODIFIED : 17 July 2002

DESCRIPTION:
==============================================================================*/
{
	char **token;
	int i,return_code = 1;
	struct Cmiss_command_data *command_data;
	struct Option_table *option_table;
	struct Parse_state *state;

	ENTER(execute_command);
	USE_PARAMETER(quit);
	if (NULL != (command_data = (struct Cmiss_command_data *)command_data_void))
	{
		if (NULL != (state = create_Parse_state(command_string)))
			/*???DB.  create_Parse_state has to be extended */
		{
			i=state->number_of_tokens;
			/* check for comment */
			if (i>0)
			{
				/* check for a "<" as one of the of the tokens */
					/*???DB.  Include for backward compatability.  Remove ? */
				token=state->tokens;
				while ((i>0)&&strcmp(*token,"<"))
				{
					i--;
					token++;
				}
				if (i>0)
				{
					/* return to tree root */
					return_code=set_command_prompt("",command_data);
				}
				else
				{
					option_table = CREATE(Option_table)();
#if defined (SELECT_DESCRIPTORS)
					/* attach */
					Option_table_add_entry(option_table, "attach", NULL, command_data_void,
						execute_command_attach);
#endif /* !defined (SELECT_DESCRIPTORS) */
#if defined (WIN32_USER_INTERFACE) || defined (GTK_USER_INTERFACE)
					/* command_window */
					Option_table_add_entry(option_table, "command_window", NULL, command_data->command_window,
						modify_Command_window);
#endif /* defined (WIN32_USER_INTERFACE) || defined (GTK_USER_INTERFACE) */
#if defined (SELECT_DESCRIPTORS)
					/* detach */
					Option_table_add_entry(option_table, "detach", NULL, command_data_void,
						execute_command_detach);
#endif /* !defined (SELECT_DESCRIPTORS) */
					/* gfx */
					Option_table_add_entry(option_table, "gfx", NULL, command_data_void,
						execute_command_gfx);
					/* open */
					Option_table_add_entry(option_table, "open", NULL, command_data_void,
						execute_command_open);
					/* quit */
					Option_table_add_entry(option_table, "quit", NULL, command_data_void,
						execute_command_quit);
					/* list_memory */
					Option_table_add_entry(option_table, "list_memory", NULL, NULL,
						execute_command_list_memory);
					/* read */
					Option_table_add_entry(option_table, "read", NULL, command_data_void,
						execute_command_read);
					/* set */
					Option_table_add_entry(option_table, "set", NULL, command_data_void,
						execute_command_set);
					/* system */
					Option_table_add_entry(option_table, "system", NULL, command_data_void,
						execute_command_system);
					return_code=Option_table_parse(option_table, state);
					DESTROY(Option_table)(&option_table);
				}
				// Catching case where a fail returned code is returned but we are
				// asking for help, reseting the return code to pass if this is the case.
				if (!return_code && state->current_token &&
				   ((0==strcmp(PARSER_HELP_STRING,state->current_token)) ||
				   (0==strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token))))
				{
					return_code = 1;
				}

			}
			destroy_Parse_state(&state);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"cmiss_execute_command.  Could not create parse state");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmiss_execute_command.  Missing command_data");
		return_code=0;
	}

	*error = return_code;

	LEAVE;

} /* execute_command */

int cmiss_execute_command(const char *command_string,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 28 August 2000

DESCRIPTION:
Takes a <command_string>, processes this through the F90 interpreter
and then executes the returned strings
==============================================================================*/
{
	int quit,return_code;
	struct Cmiss_command_data *command_data;

	ENTER(cmiss_execute_command);
	command_data = (struct Cmiss_command_data *)NULL;
	if (NULL != (command_data = (struct Cmiss_command_data *)command_data_void))
	{
#if defined (WIN32_USER_INTERFACE) || defined (GTK_USER_INTERFACE) || defined (WX_USER_INTERFACE)
		if (command_data->command_window)
		{
			add_to_command_list(command_string,command_data->command_window);
		}
#endif /* defined (WIN32_USER_INTERFACE) || defined (GTK_USER_INTERFACE) */
		quit = 0;

		interpret_command(command_data->interpreter, command_string, (void *)command_data, &quit, &execute_command, &return_code);

#if defined (WIN32_USER_INTERFACE) || defined (GTK_USER_INTERFACE) || defined (WX_USER_INTERFACE)
		if (command_data->command_window)
		{
			reset_command_box(command_data->command_window);
		}
#endif /* defined (WIN32_USER_INTERFACE) || defined (GTK_USER_INTERFACE) */

		if (quit)
		{
			Event_dispatcher_end_main_loop(command_data->event_dispatcher);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmiss_execute_command.  Missing command_data");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* cmiss_execute_command */
#else /* defined (F90_INTERPRETER) || defined (USE_PERL_INTERPRETER) */
int cmiss_execute_command(const char *command_string,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 17 July 2002

DESCRIPTION:
Execute a <command_string>. If there is a command
==============================================================================*/
{
	char **token;
	int i,return_code = 0;
	struct Cmiss_command_data *command_data;
	struct Option_table *option_table;
	struct Parse_state *state;

	ENTER(cmiss_execute_command);
	if (NULL != (command_data = (struct Cmiss_command_data *)command_data_void))
	{
		if (NULL != (state = create_Parse_state(command_string)))
			/*???DB.  create_Parse_state has to be extended */
		{
			i=state->number_of_tokens;
			/* check for comment */
			if (i>0)
			{
				/* add command to command history */
				/*???RC put out processed tokens instead? */
#if defined (WIN32_USER_INTERFACE) || defined (GTK_USER_INTERFACE) || defined (WX_USER_INTERFACE)
				if (command_data->command_window)
				{
					add_to_command_list(command_string,command_data->command_window);
				}
#endif /* defined (WIN32_USER_INTERFACE) || defined (GTK_USER_INTERFACE) */
				/* check for a "<" as one of the of the tokens */
					/*???DB.  Include for backward compatability.  Remove ? */
				token=state->tokens;
				while ((i>0)&&strcmp(*token,"<"))
				{
					i--;
					token++;
				}
				if (i>0)
				{
					/* return to tree root */
					return_code=set_command_prompt("", command_data);
				}
				else
				{
					option_table = CREATE(Option_table)();
#if defined (SELECT_DESCRIPTORS)
					/* attach */
					Option_table_add_entry(option_table, "attach", NULL, command_data_void,
						execute_command_attach);
#endif /* !defined (SELECT_DESCRIPTORS) */
#if defined (WIN32_USER_INTERFACE) || defined (GTK_USER_INTERFACE)
					/* command_window */
					Option_table_add_entry(option_table, "command_window", NULL, command_data->command_window,
						modify_Command_window);
#endif /* defined (WIN32_USER_INTERFACE) || defined (GTK_USER_INTERFACE) */
#if defined (SELECT_DESCRIPTORS)
					/* detach */
					Option_table_add_entry(option_table, "detach", NULL, command_data_void,
						execute_command_detach);
#endif /* !defined (SELECT_DESCRIPTORS) */
					/* gfx */
					Option_table_add_entry(option_table, "gfx", NULL, command_data_void,
						execute_command_gfx);
					/* open */
					Option_table_add_entry(option_table, "open", NULL, command_data_void,
						execute_command_open);
					/* quit */
					Option_table_add_entry(option_table, "quit", NULL, command_data_void,
						execute_command_quit);
					/* list_memory */
					Option_table_add_entry(option_table, "list_memory", NULL, NULL,
						execute_command_list_memory);
					/* read */
					Option_table_add_entry(option_table, "read", NULL, command_data_void,
						execute_command_read);
					/* set */
					Option_table_add_entry(option_table, "set", NULL, command_data_void,
						execute_command_set);
					/* system */
					Option_table_add_entry(option_table, "system", NULL, command_data_void,
						execute_command_system);
					return_code=Option_table_parse(option_table, state);
					DESTROY(Option_table)(&option_table);
				}
			}
#if defined (WIN32_USER_INTERFACE) || defined (GTK_USER_INTERFACE) || defined (WX_USER_INTERFACE)
			if (command_data->command_window)
			{
				reset_command_box(command_data->command_window);
			}
#endif /* defined (WIN32_USER_INTERFACE) || defined (GTK_USER_INTERFACE) */
			destroy_Parse_state(&state);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"cmiss_execute_command.  Could not create parse state");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmiss_execute_command.  Missing command_data");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* cmiss_execute_command */
#endif  /* defined (F90_INTERPRETER) || defined (USE_PERL_INTERPRETER) */

int cmiss_set_command(const char *command_string,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 17 May 2003

DESCRIPTION:
Sets the <command_string> in the command box of the CMISS command_window, ready
for editing and entering. If there is no command_window, does nothing.
==============================================================================*/
{
	int return_code;
#if defined (WX_USER_INTERFACE)
	struct Cmiss_command_data *command_data;
#endif /* defined (WX_USER_INTERFACE) */

	ENTER(cmiss_set_command);
	if (command_string
#if defined (WX_USER_INTERFACE)
		&& (command_data=(struct Cmiss_command_data *)command_data_void)
#endif /* defined (WX_USER_INTERFACE) */
			)
	{
#if defined (WX_USER_INTERFACE)
		if (command_data->command_window)
		{
			return_code=Command_window_set_command_string(
				command_data->command_window,command_string);
		}
#else
		USE_PARAMETER(command_data_void);
#endif /* defined (WX_USER_INTERFACE) */
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"cmiss_set_command.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* cmiss_set_command */

#if defined(USE_CMGUI_COMMAND_WINDOW)
static int display_error_message(const char *message,void *command_window_void)
/*******************************************************************************
LAST MODIFIED : 25 July 1999

DESCRIPTION :
Display a cmgui error message.
==============================================================================*/
{
	int return_code = 0;

	ENTER(display_error_message);
	if (command_window_void)
	{
		write_command_window("ERROR: ",
			(struct Command_window *)command_window_void);
		return_code=write_command_window(message,
			(struct Command_window *)command_window_void);
		write_command_window("\n",(struct Command_window *)command_window_void);
	}
	else
	{
		printf("ERROR: %s\n",message);
	}
	LEAVE;

	return (return_code);
} /* display_error_message */
#endif /* defined(USE_CMGUI_COMMAND_WINDOW) */

#if defined(USE_CMGUI_COMMAND_WINDOW)
static int display_information_message(const char *message,void *command_window_void)
/*******************************************************************************
LAST MODIFIED : 25 July 1999

DESCRIPTION :
Display a cmgui information message.
==============================================================================*/
{
	int return_code = 0;

	ENTER(display_error_message);
	if (command_window_void)
	{
		return_code=write_command_window(message,
			(struct Command_window *)command_window_void);
	}
	else
	{
		printf("%s", message);
	}
	LEAVE;

	return (return_code);
} /* display_information_message */
#endif /* defined(USE_CMGUI_COMMAND_WINDOW) */

#if defined(USE_CMGUI_COMMAND_WINDOW)
static int display_warning_message(const char *message,void *command_window_void)
/*******************************************************************************
LAST MODIFIED : 25 July 1999

DESCRIPTION :
Display a cmgui warning message.
???DB.  write_output is for the command_window - needs a better name.
==============================================================================*/
{
	int return_code = 0;

	ENTER(display_warning_message);
	if (command_window_void)
	{
		write_command_window("WARNING: ",
			(struct Command_window *)command_window_void);
		return_code=write_command_window(message,
			(struct Command_window *)command_window_void);
		write_command_window("\n",(struct Command_window *)command_window_void);
	}
	else
	{
		printf("WARNING: %s\n",message);
	}
	LEAVE;

	return (return_code);
} /* display_warning_message */
#endif /* defined(USE_CMGUI_COMMAND_WINDOW) */

static int cmgui_execute_comfile(const char *comfile_name,const char *example_id,
	const char *examples_directory,const char *example_symbol,char **example_comfile_name,
	struct Execute_command *execute_command)
/*******************************************************************************
LAST MODIFIED : 16 October 1998

DESCRIPTION :
Executes the comfile specified on the command line.
==============================================================================*/
{
	int return_code;
	char global_temp_string[1000];

	ENTER(cmgui_execute_comfile);
	return_code=0;
	if ((comfile_name||example_id)&&execute_command)
	{
		if (example_id)
		{
			if (examples_directory&&example_symbol)
			{
				/* set the examples directory */
				sprintf(global_temp_string,"set dir ");
				strcat(global_temp_string,example_symbol);
				strcat(global_temp_string,"=");
				strcat(global_temp_string,example_id);
				Execute_command_execute_string(execute_command,global_temp_string);
				sprintf(global_temp_string,"open comfile ");
				if (comfile_name)
				{
					strcat(global_temp_string,comfile_name);
				}
				else
				{
					if (*example_comfile_name)
					{
						strcat(global_temp_string,*example_comfile_name);
					}
					else
					{
						strcat(global_temp_string,"example_");
						strcat(global_temp_string,example_id);
					}
				}
				strcat(global_temp_string,";");
				strcat(global_temp_string,example_symbol);
				strcat(global_temp_string," execute");
				return_code=Execute_command_execute_string(execute_command,global_temp_string);
			}
			else
			{
				display_message(ERROR_MESSAGE,
				"cmgui_execute_comfile.  Missing examples_directory or example_symbol");
			}
		}
		else
		{
			/* open the command line comfile */
			sprintf(global_temp_string,"open comfile ");
			strcat(global_temp_string,comfile_name);
			strcat(global_temp_string," execute");
			return_code=Execute_command_execute_string(execute_command, global_temp_string);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmgui_execute_comfile.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* cmgui_execute_comfile */

int set_string_no_command_line_option(struct Parse_state *state,
	void *string_address_void, void *string_description_void)
/*******************************************************************************
LAST MODIFIED : 6 August 2002

DESCRIPTION :
Calls set_string unless the first character of the current token is a hyphen.
Used to avoid parsing possible command line switches.
==============================================================================*/
{
	const char *current_token;
	int return_code;

	ENTER(set_string_no_command_line_option);
	if (state)
	{
		if (NULL != (current_token = state->current_token))
		{
			if ('-' != current_token[0])
			{
				return_code = set_string(state, string_address_void,
					string_description_void);
			}
			else
			{
				display_message(ERROR_MESSAGE, "Invalid command line option \"%s\"",
					current_token);
				display_parse_state_location(state);
				return_code = 0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE, "Missing string");
			display_parse_state_location(state);
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_string_no_command_line_option.  Missing state");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* set_string_no_command_line_option */

int ignore_entry_and_next_token(struct Parse_state *state,
	void *dummy_void, void *entry_description_void)
/*******************************************************************************
LAST MODIFIED : 8 August 2002

DESCRIPTION :
Used to consume and write help for command line parameters handled outside
of this parse state routine.
==============================================================================*/
{
	const char *current_token;
	int return_code;

	ENTER(ignore_entry_and_next_token);
	USE_PARAMETER(dummy_void);
	if (state)
	{
		if (NULL != (current_token = state->current_token))
		{
			if (strcmp(PARSER_HELP_STRING, current_token) &&
				strcmp(PARSER_RECURSIVE_HELP_STRING, current_token))
			{
				return_code = shift_Parse_state(state,1);
			}
			else
			{
				display_message(INFORMATION_MESSAGE, (char *)entry_description_void);
				return_code = 1;
			}

		}
		else
		{
			display_message(ERROR_MESSAGE, "Missing string");
			display_parse_state_location(state);
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"ignore_entry_and_next_token.  Missing state");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* ignore_entry_and_next_token */

static int read_cmgui_command_line_options(struct Parse_state *state,
	void *dummy_to_be_modified, void *cmgui_command_line_options_void)
/*******************************************************************************
LAST MODIFIED : 5 August 2002

DESCRIPTION :
Parses command line options from <state>.
==============================================================================*/
{
	int return_code;
	struct Cmgui_command_line_options *command_line_options;
	struct Option_table *option_table;

	ENTER(read_cmgui_command_line_options);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (command_line_options =
		(struct Cmgui_command_line_options *)cmgui_command_line_options_void))
	{
		option_table = CREATE(Option_table)();
		/* -batch */
		Option_table_add_entry(option_table, "-batch",
			&(command_line_options->batch_mode_flag), NULL, set_char_flag);
		/* -cm */
		Option_table_add_entry(option_table, "-cm",
			&(command_line_options->cm_start_flag), NULL, set_char_flag);
		/* -cm_epath */
		Option_table_add_entry(option_table, "-cm_epath",
			&(command_line_options->cm_epath_directory_name),
			(void *)" PATH_TO_EXAMPLES_DIRECTORY", set_string);
		/* -cm_parameters */
		Option_table_add_entry(option_table, "-cm_parameters",
			&(command_line_options->cm_parameters_file_name),
			(void *)" PARAMETER_FILE_NAME", set_string);
		/* -command_list */
		Option_table_add_entry(option_table, "-command_list",
			&(command_line_options->command_list_flag), NULL, set_char_flag);
		/* -console */
		Option_table_add_entry(option_table, "-console",
			&(command_line_options->console_mode_flag), NULL, set_char_flag);
#if defined (GTK_USER_INTERFACE) || defined (__WXGTK__)
		/* --display, support the gtk convention for this tool */
		Option_table_add_entry(option_table, "--display", NULL,
			(void *)" X11_DISPLAY_NUMBER", ignore_entry_and_next_token);
#endif /* defined (GTK_USER_INTERFACE) || defined (__WXGTK__) */
		/* -epath */
		Option_table_add_entry(option_table, "-epath",
			&(command_line_options->epath_directory_name),
			(void *)" PATH_TO_EXAMPLES_DIRECTORY", set_string);
		/* -example */
		Option_table_add_entry(option_table, "-example",
			&(command_line_options->example_file_name),
			(void *)" EXAMPLE_ID", set_string);
		/* -execute */
		Option_table_add_entry(option_table, "-execute",
			&(command_line_options->execute_string),
			(void *)" EXECUTE_STRING", set_string);
		/* -help */
		Option_table_add_entry(option_table, "-help",
			&(command_line_options->write_help_flag), NULL, set_char_flag);
		/* -id */
		Option_table_add_entry(option_table, "-id",
			&(command_line_options->id_name), (void *)" ID", set_string);
		/* -mycm */
		Option_table_add_entry(option_table, "-mycm",
			&(command_line_options->mycm_start_flag), NULL, set_char_flag);
		/* -no_display */
		Option_table_add_entry(option_table, "-no_display",
			&(command_line_options->no_display_flag), NULL, set_char_flag);
		/* -random */
		Option_table_add_entry(option_table, "-random",
			&(command_line_options->random_number_seed),
			(void *)" NUMBER_SEED", set_int_with_description);
		/* -server */
		Option_table_add_entry(option_table, "-server",
			&(command_line_options->server_mode_flag), NULL, set_char_flag);
		/* -visual */
		Option_table_add_entry(option_table, "-visual",
			&(command_line_options->visual_id_number),
			(void *)" NUMBER", set_int_with_description);
		/* [default option == command_file_name] */
		Option_table_add_entry(option_table, (char *)NULL,
			&(command_line_options->command_file_name),
			(void *)"COMMAND_FILE_NAME", set_string_no_command_line_option);
		return_code = Option_table_multi_parse(option_table, state);
		DESTROY(Option_table)(&option_table);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"read_cmgui_command_line_options.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* read_cmgui_command_line_options */

int Cmiss_command_data_process_command_line(int argc, const char *argv[],
	struct Cmgui_command_line_options *command_line_options)
{
	int return_code = 1;
	struct Option_table *option_table = NULL;
	struct Parse_state *state = NULL;

	/* put command line options into structure for parsing & extract below */
	command_line_options->batch_mode_flag = (char)0;
	command_line_options->cm_start_flag = (char)0;
	command_line_options->cm_epath_directory_name = NULL;
	command_line_options->cm_parameters_file_name = NULL;
	command_line_options->command_list_flag = (char)0;
	command_line_options->console_mode_flag = (char)0;
	command_line_options->epath_directory_name = NULL;
	command_line_options->example_file_name = NULL;
	command_line_options->execute_string = NULL;
	command_line_options->write_help_flag = (char)0;
	command_line_options->id_name = NULL;
	command_line_options->mycm_start_flag = (char)0;
	command_line_options->no_display_flag = (char)0;
	command_line_options->random_number_seed = -1;
	command_line_options->server_mode_flag = (char)0;
	command_line_options->visual_id_number = 0;
	command_line_options->command_file_name = NULL;

	if (argc >  0 && argv != NULL)
	{
		if (NULL != (state = create_Parse_state_from_tokens(argc, argv)))
		{
			option_table = CREATE(Option_table)();
			Option_table_add_entry(option_table, argv[0], NULL,
				(void *)command_line_options, read_cmgui_command_line_options);
			if (!Option_table_parse(option_table, state))
			{
				command_line_options->write_help_flag = (char)1;
				return_code = 0;
			}
			DESTROY(Option_table)(&option_table);
			destroy_Parse_state(&state);
		}
		else
		{
			return_code = 0;
		}
	}

	return return_code;
}

struct Cmiss_command_data *CREATE(Cmiss_command_data)(struct Cmiss_context_app *context,
	struct User_interface_module *UI_module)
/*******************************************************************************
LAST MODIFIED : 3 April 2003

DESCRIPTION :
Initialise all the subcomponents of cmgui and create the Cmiss_command_data
==============================================================================*/
{
	char *cm_examples_directory,*cm_parameters_file_name,*comfile_name,
		*example_id,*examples_directory,*examples_environment,*execute_string,
		*version_command_id;
	char global_temp_string[1000];
	int return_code;
	int batch_mode, console_mode, command_list, no_display, non_random,
		server_mode, start_cm, start_mycm, visual_id, write_help;
#if defined (F90_INTERPRETER) || defined (USE_PERL_INTERPRETER)
	int status;
#endif /* defined (F90_INTERPRETER) || defined (USE_PERL_INTERPRETER) */
	struct Cmgui_command_line_options command_line_options;
	struct Cmiss_command_data *command_data;
#if defined(USE_CMGUI_COMMAND_WINDOW)
	struct Command_window *command_window;
#endif /* defined(USE_CMGUI_COMMAND_WINDOW) */
	struct Graphical_material *material;
	struct Option_table *option_table;
	struct Parse_state *state;
	User_settings user_settings;
#if defined (WIN32_USER_INTERFACE)
	ENTER(WinMain);
#endif /* defined (WIN32_USER_INTERFACE) */
	return_code = 1;

	if (ALLOCATE(command_data, struct Cmiss_command_data, 1))
	{
		command_data->access_count = 1;
		// duplicate argument list so it can be modified by User Interface
		/* initialize application specific global variables */
		command_data->execute_command = CREATE(Execute_command)();;
		command_data->set_command = CREATE(Execute_command)();
		command_data->event_dispatcher = (struct Event_dispatcher *)NULL;
		command_data->user_interface= (struct User_interface *)NULL;
		command_data->emoter_slider_dialog=(struct Emoter_dialog *)NULL;
#if defined (WX_USER_INTERFACE)
		command_data->data_viewer=(struct Node_viewer *)NULL;
		command_data->node_viewer=(struct Node_viewer *)NULL;
		command_data->element_point_viewer=(struct Element_point_viewer *)NULL;
#endif /* defined (WX_USER_INTERFACE) */
#if defined (WX_USER_INTERFACE)
		command_data->material_editor = (struct Material_editor *)NULL;
		command_data->region_tree_viewer = (struct Region_tree_viewer *)NULL;
		command_data->spectrum_editor_dialog = (struct Spectrum_editor_dialog *)NULL;
#endif /*defined (WX_USER_INTERFACE) */
		command_data->command_console = (struct Console *)NULL;
		command_data->example_directory=(char *)NULL;

#if defined (WX_USER_INTERFACE)
		command_data->comfile_window_manager=(struct MANAGER(Comfile_window) *)NULL;
#endif /* defined WX_USER_INTERFACE*/
		command_data->default_light=(struct Light *)NULL;
		command_data->light_manager=(struct MANAGER(Light) *)NULL;
		command_data->default_light_model=(struct Light_model *)NULL;
		command_data->light_model_manager=(struct MANAGER(Light_model) *)NULL;
		command_data->environment_map_manager=(struct MANAGER(Environment_map) *)NULL;
		command_data->volume_texture_manager=(struct MANAGER(VT_volume_texture) *)NULL;
		command_data->default_spectrum=(struct Spectrum *)NULL;
		command_data->spectrum_manager=(struct MANAGER(Spectrum) *)NULL;
		command_data->graphics_buffer_package=(struct Graphics_buffer_app_package *)NULL;
		command_data->scene_viewer_package=(struct Cmiss_scene_viewer_app_package *)NULL;
		command_data->graphics_module = (struct Cmiss_graphics_module *)NULL;
#if defined (USE_CMGUI_GRAPHICS_WINDOW)
		command_data->graphics_window_manager=(struct MANAGER(Graphics_window) *)NULL;
#endif /* defined (USE_CMGUI_GRAPHICS_WINDOW) */
		command_data->root_region = (struct Cmiss_region *)NULL;
		command_data->curve_manager=(struct MANAGER(Curve) *)NULL;
		command_data->basis_manager=(struct MANAGER(FE_basis) *)NULL;
		command_data->streampoint_list=(struct Streampoint *)NULL;
#if defined (SELECT_DESCRIPTORS)
		command_data->device_list=(struct LIST(Io_device) *)NULL;
#endif /* defined (SELECT_DESCRIPTORS) */
		command_data->glyph_manager=(struct MANAGER(GT_object) *)NULL;
		command_data->any_object_selection=(struct Any_object_selection *)NULL;
		command_data->element_point_ranges_selection=(struct Element_point_ranges_selection *)NULL;
		command_data->interactive_tool_manager=(struct MANAGER(Interactive_tool) *)NULL;
		command_data->io_stream_package = (struct IO_stream_package *)NULL;
		command_data->computed_field_package=(struct Computed_field_package *)NULL;
		command_data->default_scene=(struct Scene *)NULL;
		command_data->scene_manager=(struct MANAGER(Scene) *)NULL;
#if defined (WIN32_USER_INTERFACE) || defined (GTK_USER_INTERFACE) || defined (WX_USER_INTERFACE)
		command_data->command_window=(struct Command_window *)NULL;
#endif /* defined (WIN32_USER_INTERFACE) || defined (GTK_USER_INTERFACE) */
		command_data->transform_tool=(struct Interactive_tool *)NULL;
		command_data->node_tool=(struct Node_tool *)NULL;
		command_data->element_tool=(struct Element_tool *)NULL;
#if defined (USE_OPENCASCADE)
		command_data->cad_tool = (struct Cad_tool *)NULL;
#endif /* defined (USE_OPENCASCADE) */
		command_data->data_tool=(struct Node_tool *)NULL;
		command_data->element_point_tool=(struct Element_point_tool *)NULL;
		command_data->examples_directory=(char *)NULL;
		command_data->example_comfile=(char *)NULL;
		command_data->example_requirements=(char *)NULL;
		command_data->cm_examples_directory=(char *)NULL;
		command_data->cm_parameters_file_name=(char *)NULL;
		command_data->default_time_keeper = (struct Time_keeper *)NULL;
		command_data->background_colour.red=(float)0;
		command_data->background_colour.green=(float)0;
		command_data->background_colour.blue=(float)0;
		command_data->foreground_colour.red=(float)1;
		command_data->foreground_colour.green=(float)1;
		command_data->foreground_colour.blue=(float)1;
		command_data->help_directory=(char *)NULL;
		command_data->help_url=(char *)NULL;
#if defined (USE_PERL_INTERPRETER)
		command_data->interpreter = (struct Interpreter *)NULL;
#endif /* defined (USE_PERL_INTERPRETER) */

		/* set default values for command-line modifiable options */
		/* Note User_interface will not be created if command_list selected */
		batch_mode = 0;
		command_list = 0;
		console_mode = 0;
		no_display = 0;
		server_mode = 0;
		visual_id = 0;
		write_help = 0;
		/* flag to say randomise */
		non_random = -1;
		/* flag for starting cm */
		start_cm = 0;
		/* flag for starting mycm */
		start_mycm = 0;
		/* to over-ride all other example directory settings */
		examples_directory = (char *)NULL;
		/* back-end examples directory */
		cm_examples_directory = (char *)NULL;
		/* back-end parameters file */
		cm_parameters_file_name = (char *)NULL;
		/* the comfile is in the examples directory */
		example_id = (char *)NULL;
		/* a string executed by the interpreter before loading any comfiles */
		execute_string = (char *)NULL;
		/* set no command id supplied */
		version_command_id = (char *)NULL;
		/* the name of the comfile to be run on startup */
		comfile_name = (char *)NULL;

		user_settings.examples_directory = (char *)NULL;
		user_settings.help_directory = (char *)NULL;
		user_settings.help_url = (char *)NULL;
		user_settings.startup_comfile = (char *)NULL;

		/* parse commmand line options */

		/* put command line options into structure for parsing & extract below */
		command_line_options.batch_mode_flag = (char)batch_mode;
		command_line_options.cm_start_flag = (char)start_cm;
		command_line_options.cm_epath_directory_name = cm_examples_directory;
		command_line_options.cm_parameters_file_name = cm_parameters_file_name;
		command_line_options.command_list_flag = (char)command_list;
		command_line_options.console_mode_flag = (char)console_mode;
		command_line_options.epath_directory_name = examples_directory;
		command_line_options.example_file_name = example_id;
		command_line_options.execute_string = execute_string;
		command_line_options.write_help_flag = (char)write_help;
		command_line_options.id_name = version_command_id;
		command_line_options.mycm_start_flag = (char)start_mycm;
		command_line_options.no_display_flag = (char)no_display;
		command_line_options.random_number_seed = non_random;
		command_line_options.server_mode_flag = (char)server_mode;
		command_line_options.visual_id_number = visual_id;
		command_line_options.command_file_name = comfile_name;

		if (UI_module->argc > 0 && UI_module->argv)
		{
			if (NULL != (state = create_Parse_state_from_tokens(UI_module->argc, (const char **)(UI_module->argv))))
			{
				option_table = CREATE(Option_table)();
				Option_table_add_entry(option_table, UI_module->argv[0], NULL,
				(void *)&command_line_options, read_cmgui_command_line_options);
				if (!Option_table_parse(option_table, state))
				{
					write_help = 1;
					return_code = 0;
				}
				DESTROY(Option_table)(&option_table);
				destroy_Parse_state(&state);
			}
			else
			{
				return_code = 0;
			}
		}
		/* copy command line options to local vars for use and easy clean-up */
		batch_mode = (int)command_line_options.batch_mode_flag;
		start_cm = command_line_options.cm_start_flag;
		cm_examples_directory = command_line_options.cm_epath_directory_name;
		cm_parameters_file_name = command_line_options.cm_parameters_file_name;
		command_list = command_line_options.command_list_flag;
		console_mode = command_line_options.console_mode_flag;
		examples_directory = command_line_options.epath_directory_name;
		example_id = command_line_options.example_file_name;
		execute_string = command_line_options.execute_string;
		write_help = command_line_options.write_help_flag;
		version_command_id = command_line_options.id_name;
		start_mycm = command_line_options.mycm_start_flag;
		no_display = command_line_options.no_display_flag;
		non_random = command_line_options.random_number_seed;
		server_mode = (int)command_line_options.server_mode_flag;
		visual_id = command_line_options.visual_id_number;
		comfile_name = command_line_options.command_file_name;
		if (write_help)
		{
			const char *double_question_mark = "??";

			/* write question mark help for command line options */
			state = create_Parse_state_from_tokens(1, &double_question_mark);
			option_table = CREATE(Option_table)();
			Option_table_add_entry(option_table,UI_module->argv[0], NULL,
				(void *)&command_line_options, read_cmgui_command_line_options);
			Option_table_parse(option_table, state);
			DESTROY(Option_table)(&option_table);
			destroy_Parse_state(&state);
		}

		command_data->io_stream_package = Cmiss_context_get_default_IO_stream_package(Cmiss_context_app_get_core_context(context));

#if defined (F90_INTERPRETER) || defined (USE_PERL_INTERPRETER)
		/* SAB I want to do this before CREATEing the User_interface
			as X modifies the argc, argv removing the options it understands
			however I want a full copy for the interpreter so that we can use
			-display for example for both */
		create_interpreter(UI_module->argc, UI_module->unmodified_argv, comfile_name, &command_data->interpreter, &status);

		if (!status)
		{
			return_code=0;
		}

		interpreter_set_display_message_function(command_data->interpreter, display_message, &status);

		/* SAB Set a useful default for the interpreter variable, need to
			specify full name as this function does not run embedded by
			a package directive */
		interpreter_set_string(command_data->interpreter, "cmiss::example", ".", &status);

		/* SAB Set the cmgui command data into the interpreter.  The Cmiss package
			is then able to export this when it is called from inside cmgui or
			when called directly from perl to load the appropriate libraries to
			create a cmgui externally. */
		interpreter_set_pointer(command_data->interpreter, "Cmiss::Cmiss_context",
			"Cmiss::Cmiss_context", context, &status);

#endif /* defined (F90_INTERPRETER) || defined (USE_PERL_INTERPRETER) */

		if ((!command_list) && (!write_help))
		{
			if (NULL != (command_data->event_dispatcher =
					Cmiss_context_app_get_default_event_dispatcher(context)))
			{
				if (!no_display)
				{
					if (NULL == (command_data->user_interface = UI_module->user_interface))
					{
						return_code=0;
					}
				}
			}
			else
			{
				return_code = 0;
			}
		}

		/* use command line options in preference to defaults read from XResources */

		if (examples_directory)
		{
			command_data->examples_directory = examples_directory;
		}
		else if (NULL != (examples_environment = getenv("CMISS_EXAMPLES")))
		{
			command_data->examples_directory = duplicate_string(examples_environment);
		}
		else
		{
			command_data->examples_directory = (char *)NULL;
		}
#if defined (WIN32_SYSTEM)
		/* We don't know about cygdrive as we are using the win32 api,
		   but try and interpret the variable anyway.  We can't handle
			other cygwin paths unless we call out to cygpath. */
		if (command_data->examples_directory
			&& (strlen(command_data->examples_directory) > 11)
			&& (!strncmp(command_data->examples_directory, "/cygdrive", 9)))
		{
			char *new_examples_string;
			ALLOCATE(new_examples_string, char,
				strlen(command_data->examples_directory) + 10);
			new_examples_string[0] = command_data->examples_directory[10];
		   new_examples_string[1] = ':';
		   new_examples_string[2] = '\\';
		   strcpy(new_examples_string + 3, command_data->examples_directory + 12);
		   DEALLOCATE(command_data->examples_directory);
		   command_data->examples_directory = new_examples_string;
		}
#endif /* defined (WIN32_SYSTEM) */
		command_data->cm_examples_directory = cm_examples_directory;
		command_data->cm_parameters_file_name = cm_parameters_file_name;
		command_data->help_directory = user_settings.help_directory;
		command_data->help_url = user_settings.help_url;

		/* create the managers */

#if defined (WX_USER_INTERFACE)
		/* comfile window manager */
		command_data->comfile_window_manager = UI_module->comfile_window_manager;
#endif /* defined (WX_USER_INTERFACE) */
		command_data->graphics_module =
			Cmiss_context_get_default_graphics_module(Cmiss_context_app_get_core_context(context));
		/* light manager */
		if (NULL != (command_data->light_manager=Cmiss_graphics_module_get_light_manager(
						command_data->graphics_module)))
		{
			command_data->default_light=Cmiss_graphics_module_get_default_light(
				command_data->graphics_module);
		}
		command_data->light_model_manager=
			Cmiss_graphics_module_get_light_model_manager(command_data->graphics_module);

		command_data->default_light_model=
			Cmiss_graphics_module_get_default_light_model(command_data->graphics_module);
		// ensure we have a default tessellation
		Cmiss_tessellation_id default_tessellation = Cmiss_graphics_module_get_default_tessellation(command_data->graphics_module);
		Cmiss_tessellation_destroy(&default_tessellation);

		/* environment map manager */
		command_data->environment_map_manager=CREATE(MANAGER(Environment_map))();
		/* volume texture manager */
		command_data->volume_texture_manager=CREATE(MANAGER(VT_volume_texture))();
		/* spectrum manager */
		if (NULL != (command_data->spectrum_manager=
				Cmiss_graphics_module_get_spectrum_manager(
					command_data->graphics_module)))
		{
			command_data->default_spectrum=
				Cmiss_graphics_module_get_default_spectrum(command_data->graphics_module);
		}
		/* create Material package and CMGUI default materials */
		if (NULL != (command_data->material_package =
				Cmiss_graphics_module_get_material_package(command_data->graphics_module)))
		{
			if (NULL != (material = Material_package_get_default_material(command_data->material_package)))
			{
				Graphical_material_set_alpha(material, 1.0);
			}
		}
		command_data->default_font = Cmiss_graphics_module_get_default_font(
			command_data->graphics_module);

#if defined (USE_CMGUI_GRAPHICS_WINDOW)
		command_data->graphics_buffer_package = UI_module->graphics_buffer_package;
		/* graphics window manager.  Note there is no default window. */
		command_data->graphics_window_manager = UI_module->graphics_window_manager;
#endif /* defined (USE_CMGUI_GRAPHICS_WINDOW) */
		/* FE_element_shape manager */
		/*???DB.  To be done */
		command_data->element_shape_list=CREATE(LIST(FE_element_shape))();

		command_data->curve_manager=Cmiss_context_get_default_curve_manager(Cmiss_context_app_get_core_context(context));

		command_data->basis_manager=CREATE(MANAGER(FE_basis))();

		command_data->root_region = Cmiss_context_get_default_region(Cmiss_context_app_get_core_context(context));

#if defined (SELECT_DESCRIPTORS)
		/* create device list */
		/*SAB.  Eventually want device manager */
		command_data->device_list=CREATE(LIST(Io_device))();
#endif /* defined (SELECT_DESCRIPTORS) */

		command_data->glyph_manager = Cmiss_graphics_module_get_default_glyph_manager(
			command_data->graphics_module);

		/* global list of selected objects */
		command_data->any_object_selection = Cmiss_context_get_any_object_selection(Cmiss_context_app_get_core_context(context));
		command_data->element_point_ranges_selection =
			Cmiss_context_get_element_point_ranges_selection(Cmiss_context_app_get_core_context(context));

		/* interactive_tool manager */
		command_data->interactive_tool_manager=UI_module->interactive_tool_manager;
		/* computed field manager and default computed fields zero, xi,
			default_coordinate, etc. */
		/*???RC should the default computed fields be established in
		  CREATE(Computed_field_package)? */

		/*???GRC will eventually remove manager from field package so it is
		  purely type-specific data. Field manager is now owned by region.
		  Temporarily passing it to package to keep existing code running */
		struct MANAGER(Computed_field) *computed_field_manager=
			Cmiss_region_get_Computed_field_manager(command_data->root_region);
		command_data->computed_field_package =
			CREATE(Computed_field_package)(computed_field_manager);
		/* Add Computed_fields to the Computed_field_package */
		if (command_data->computed_field_package)
		{
			Computed_field_register_types_coordinate(
				command_data->computed_field_package);
			if (command_data->root_region)
			{
				Computed_field_register_type_alias(
					command_data->computed_field_package,
					command_data->root_region);
			}
			Computed_field_register_types_arithmetic_operators(
				command_data->computed_field_package);
			Computed_field_register_types_trigonometry(
				command_data->computed_field_package);
			Computed_field_register_types_format_output(
				command_data->computed_field_package);
			if (command_data->root_region)
			{
				Computed_field_register_types_compose(
					command_data->computed_field_package,
					command_data->root_region);
			}
			Computed_field_register_types_composite(
				command_data->computed_field_package);
			Computed_field_register_types_conditional(
				command_data->computed_field_package);
			if (command_data->curve_manager)
			{
				Computed_field_register_types_curve(
					command_data->computed_field_package,
					command_data->curve_manager);
			}
#if defined (USE_ITK)
			Computed_field_register_types_derivatives(
				command_data->computed_field_package);
#endif /* defined (USE_ITK) */
			Computed_field_register_types_fibres(
				command_data->computed_field_package);
			Computed_field_register_types_function(
					command_data->computed_field_package);
			Computed_field_register_types_logical_operators(
				command_data->computed_field_package);
			if (command_data->root_region)
			{
				Computed_field_register_types_lookup(
					command_data->computed_field_package,
					command_data->root_region);
			}
			Computed_field_register_types_matrix_operators(
				command_data->computed_field_package);
			Computed_field_register_types_nodeset_operators(
				command_data->computed_field_package);
			Computed_field_register_types_vector_operators(
				command_data->computed_field_package);
#if defined (USE_CMGUI_GRAPHICS_WINDOW)
			if (command_data->graphics_window_manager)
			{
				Computed_field_register_types_scene_viewer_projection(
					command_data->computed_field_package,
					command_data->graphics_window_manager);
			}
#endif /* defined (USE_CMGUI_GRAPHICS_WINDOW) */
			Computed_field_register_types_image(
				command_data->computed_field_package);
			if (command_data->root_region)
			{
				Computed_field_register_types_integration(
					command_data->computed_field_package,
					command_data->root_region);
			}
			Computed_field_register_types_finite_element(
				command_data->computed_field_package);
			Computed_field_register_types_deformation(
				command_data->computed_field_package);
			Computed_field_register_types_string_constant(
				command_data->computed_field_package);

			Computed_field_register_types_image_resample(
				command_data->computed_field_package);
#if defined (USE_ITK)
			Computed_field_register_types_threshold_image_filter(
				command_data->computed_field_package);
			Computed_field_register_types_binary_threshold_image_filter(
				command_data->computed_field_package);
			Computed_field_register_types_canny_edge_detection_image_filter(
				command_data->computed_field_package);
			Computed_field_register_types_mean_image_filter(
				command_data->computed_field_package);
			Computed_field_register_types_sigmoid_image_filter(
				command_data->computed_field_package);
			Computed_field_register_types_discrete_gaussian_image_filter(
				command_data->computed_field_package);
			Computed_field_register_types_histogram_image_filter(
				command_data->computed_field_package);
			Computed_field_register_types_curvature_anisotropic_diffusion_image_filter(
				command_data->computed_field_package);
			Computed_field_register_types_derivative_image_filter(
				command_data->computed_field_package);
			Computed_field_register_types_rescale_intensity_image_filter(
				command_data->computed_field_package);
			Computed_field_register_types_connected_threshold_image_filter(
				command_data->computed_field_package);
			Computed_field_register_types_gradient_magnitude_recursive_gaussian_image_filter(
				command_data->computed_field_package);
			Computed_field_register_types_fast_marching_image_filter(
				command_data->computed_field_package);
			Computed_field_register_types_binary_dilate_image_filter(
				command_data->computed_field_package);
			Computed_field_register_types_binary_erode_image_filter(
				command_data->computed_field_package);
#endif /* defined (USE_ITK) */
		}
		/* graphics_module */
		command_data->default_time_keeper=ACCESS(Time_keeper)(UI_module->default_time_keeper);

		/* scene manager */
		/*???RC & SAB.   LOTS of managers need to be created before this
		  and the User_interface too */
		if (NULL != (command_data->scene_manager=Cmiss_graphics_module_get_scene_manager(
									 command_data->graphics_module)))
		Cmiss_graphics_module_enable_renditions(
			command_data->graphics_module,command_data->root_region);
		{
			command_data->default_scene = Cmiss_graphics_module_get_default_scene(
				command_data->graphics_module);
			if (command_data->default_scene)
			{
//			display_message(INFORMATION_MESSAGE,"Cmiss_command_data *CREATE\n");
				Cmiss_scene_set_region(command_data->default_scene, command_data->root_region);
			}
		}

		if (command_data->computed_field_package && command_data->default_time_keeper)
		{
			Computed_field_register_types_time(command_data->computed_field_package,
				command_data->default_time_keeper);
		}

		if (command_data->user_interface)
		{
			command_data->transform_tool=UI_module->transform_tool;
			command_data->node_tool=UI_module->node_tool;
			Node_tool_set_execute_command(command_data->node_tool, command_data->execute_command);
			command_data->data_tool=UI_module->data_tool;
			Node_tool_set_execute_command(command_data->data_tool, command_data->execute_command);
			command_data->element_tool=UI_module->element_tool;
			Element_tool_set_execute_command(command_data->element_tool,
				command_data->execute_command);
#if defined (USE_OPENCASCADE)
			command_data->cad_tool = UI_module->cad_tool;
			Cad_tool_set_execute_command(command_data->cad_tool,
				command_data->execute_command);
#endif /* defined (USE_OPENCASCADE) */
			command_data->element_point_tool=UI_module->element_point_tool;
			Element_point_tool_set_execute_command(command_data->element_point_tool,
				command_data->execute_command);
		}
#if defined (USE_CMGUI_GRAPHICS_WINDOW)
		if (command_data->user_interface)
		{
			command_data->scene_viewer_package = UI_module->scene_viewer_package;
		}
#endif /* defined (USE_CMGUI_GRAPHICS_WINDOW) */

		/* properly set up the Execute_command objects */
		Execute_command_set_command_function(command_data->execute_command,
			cmiss_execute_command, (void *)command_data);
		Execute_command_set_command_function(command_data->set_command,
			cmiss_set_command, (void *)command_data);
		/* initialize random number generator */
		if (-1 == non_random)
		{
			/* randomise */
			srand(time(NULL));
			/*???DB.  time is not ANSI */
		}
		else
		{
			/* randomise using given seed */
			srand(non_random);
		}

		if (return_code && (!command_list) && (!write_help))
		{
			if (!no_display)
			{
				/* create the main window */
				if (!server_mode)
				{
#if defined(USE_CMGUI_COMMAND_WINDOW)
					if (console_mode)
					{
#endif /* defined(USE_CMGUI_COMMAND_WINDOW) */
						if (!(command_data->command_console = CREATE(Console)(
							command_data->execute_command,
							command_data->event_dispatcher, /*stdin*/0)))
						{
							display_message(ERROR_MESSAGE,"main.  "
								"Unable to create console.");
						}
#if defined(USE_CMGUI_COMMAND_WINDOW)
					}
					else if (!UI_module->external)
					{
						if (NULL != (command_window = CREATE(Command_window)(command_data->execute_command,
							command_data->user_interface)))
						{
							command_data->command_window=command_window;
							if (!batch_mode)
							{
								/* set up messages */
								set_display_message_function(ERROR_MESSAGE,
									display_error_message,command_window);
								set_display_message_function(INFORMATION_MESSAGE,
									display_information_message,command_window);
								set_display_message_function(WARNING_MESSAGE,
									display_warning_message,command_window);
#if defined (USE_PERL_INTERPRETER)
								redirect_interpreter_output(command_data->interpreter, &return_code);
#endif /* defined (USE_PERL_INTERPRETER) */
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,"Unable to create command window");
							return_code=0;
						}
					}
#endif /* defined(USE_CMGUI_COMMAND_WINDOW) */
				}
			}
		}

		if (return_code && (!command_list) && (!write_help))
		{
			if (start_cm||start_mycm)
			{
				sprintf(global_temp_string,"create cm");
				if (start_mycm)
				{
					strcat(global_temp_string," mycm");
				}
				if (cm_parameters_file_name)
				{
					strcat(global_temp_string," parameters ");
					strcat(global_temp_string,cm_parameters_file_name);
				}
				if (cm_examples_directory)
				{
					strcat(global_temp_string," examples_directory ");
					strcat(global_temp_string,cm_examples_directory);
				}
				/* start the back-end */
				cmiss_execute_command(global_temp_string,
					(void *)command_data);
			}
			if (user_settings.startup_comfile)
			{
				/* Can't get the startupComfile name without X at the moment */
				cmgui_execute_comfile(user_settings.startup_comfile, NULL,
					NULL, NULL, (char **)NULL, command_data->execute_command);
			}
			if (execute_string)
			{
				cmiss_execute_command(execute_string,(void *)command_data);
			}
			if (example_id||comfile_name)
			{
				/* open the command line comfile */
				cmgui_execute_comfile(comfile_name,example_id,
					command_data->examples_directory,
					CMGUI_EXAMPLE_DIRECTORY_SYMBOL, &command_data->example_comfile,
					command_data->execute_command);
			}
		}

		if ((!command_list) && (!write_help))
		{
			/* START_ERROR_HANDLING;*/
			switch (signal_code)
			{
				case SIGFPE:
				{
					printf("Floating point exception occurred\n");
					display_message(ERROR_MESSAGE,
						"Floating point exception occurred");
				} break;
				case SIGILL:
				{
					printf("Illegal instruction occurred\n");
					display_message(ERROR_MESSAGE,
						"Illegal instruction occurred");
				} break;
				case SIGSEGV:
				{
					printf("Invalid memory reference occurred\n");
					display_message(ERROR_MESSAGE,
						"Invalid memory reference occurred");
				} break;
			}
		}
		if (command_list)
		{
			cmiss_execute_command("??", (void *)command_data);
		}
		if (example_id)
		{
			DEALLOCATE(example_id);
		}
		if (execute_string)
		{
			DEALLOCATE(execute_string);
		}
		if (version_command_id)
		{
			DEALLOCATE(version_command_id);
		}
		if (comfile_name)
		{
			DEALLOCATE(comfile_name);
		}

		if (command_list || write_help || batch_mode || !return_code)
		{
			Cmiss_command_data_destroy(&command_data);
		}
	}
	else
	{
		command_data = (struct Cmiss_command_data *)NULL;
	}
	LEAVE;

	return (command_data);
} /* CREATE(Cmiss_command_data) */

int DESTROY(Cmiss_command_data)(struct Cmiss_command_data **command_data_address)
/*******************************************************************************
LAST MODIFIED : 19 December 2002

DESCRIPTION :
Clean up the command_data, deallocating all the associated memory and resources.
NOTE: Do not call this directly: call Cmiss_command_data_destroy() to deaccess.
==============================================================================*/
{
	int return_code = 0;
#if defined (F90_INTERPRETER) || defined (USE_PERL_INTERPRETER)
	int status;
#endif /* defined (F90_INTERPRETER) || defined (USE_PERL_INTERPRETER) */
	struct Cmiss_command_data *command_data;
	ENTER(DESTROY(Cmiss_command_data));

	if (command_data_address && (command_data = *command_data_address))
	{
		if (command_data->access_count != 0)
		{
			display_message(ERROR_MESSAGE,
				"Call to DESTROY(Cmiss_command_data) while still in use");
			return 0;
		}
		if (command_data->emoter_slider_dialog)
		{
			DESTROY(Emoter_dialog)(&command_data->emoter_slider_dialog);
		}
#if defined (WX_USER_INTERFACE)
		/* viewers */
		if (command_data->data_viewer)
		{
			Node_viewer_destroy(&(command_data->data_viewer));
		}
		if (command_data->node_viewer)
		{
			Node_viewer_destroy(&(command_data->node_viewer));
		}
		if (command_data->element_point_viewer)
		{
			DESTROY(Element_point_viewer)(&(command_data->element_point_viewer));
		}
#endif /* defined (WX_USER_INTERFACE) */
#if defined (WX_USER_INTERFACE)
		if (command_data->material_editor)
		{
			DESTROY(Material_editor)(&(command_data->material_editor));
		}
		if (command_data->region_tree_viewer)
		{
			DESTROY(Region_tree_viewer)(&(command_data->region_tree_viewer));
		}
		if (command_data->spectrum_editor_dialog)
		{
			DESTROY(Spectrum_editor_dialog)(&(command_data->spectrum_editor_dialog));
		}
#endif /* defined (WX_USER_INTERFACE) */

		DEACCESS(Scene)(&command_data->default_scene);
		if (command_data->graphics_module)
		{
			Cmiss_graphics_module_destroy(&command_data->graphics_module);
		}
		DEACCESS(Time_keeper)(&command_data->default_time_keeper);
		if (command_data->computed_field_package)
		{
			Computed_field_package_remove_types(command_data->computed_field_package);
			DESTROY(Computed_field_package)(&command_data->computed_field_package);
		}
#if defined (SELECT_DESCRIPTORS)
		DESTROY(LIST(Io_device))(&command_data->device_list);
#endif /* defined (SELECT_DESCRIPTORS) */

		DEACCESS(Cmiss_region)(&(command_data->root_region));
		DESTROY(MANAGER(FE_basis))(&command_data->basis_manager);
		DESTROY(LIST(FE_element_shape))(&command_data->element_shape_list);

		/* some fields register for changes with the following managers,
			 hence must destroy after regions and their fields */
		command_data->curve_manager = NULL;
		DEACCESS(Spectrum)(&(command_data->default_spectrum));
		command_data->spectrum_manager=NULL;
		DEACCESS(Material_package)(&command_data->material_package);
		DEACCESS(Cmiss_graphics_font)(&command_data->default_font);
		DESTROY(MANAGER(VT_volume_texture))(&command_data->volume_texture_manager);
		DESTROY(MANAGER(Environment_map))(&command_data->environment_map_manager);
		DEACCESS(Light_model)(&(command_data->default_light_model));
		DEACCESS(Light)(&(command_data->default_light));
		command_data->light_manager = NULL;
		if (command_data->example_directory)
		{
			DEALLOCATE(command_data->example_directory);
		}
		if (command_data->example_comfile)
		{
			DEALLOCATE(command_data->example_comfile);
		}
		if (command_data->example_requirements)
		{
			DEALLOCATE(command_data->example_requirements);
		}

		Close_image_environment();

		DESTROY(Execute_command)(&command_data->execute_command);
		DESTROY(Execute_command)(&command_data->set_command);

#if defined (F90_INTERPRETER) || defined (USE_PERL_INTERPRETER)
		destroy_interpreter(command_data->interpreter, &status);
#endif /* defined (F90_INTERPRETER) || defined (USE_PERL_INTERPRETER) */

		if (command_data->command_console)
		{
			DESTROY(Console)(&command_data->command_console);
		}
#if defined (WIN32_USER_INTERFACE) || defined (GTK_USER_INTERFACE) || defined (WX_USER_INTERFACE)
		if (command_data->command_window)
		{
			DESTROY(Command_window)(&command_data->command_window);
		}
#endif /* defined (WIN32_USER_INTERFACE) || defined (GTK_USER_INTERFACE) */

		if (command_data->user_interface)
			command_data->user_interface = NULL;
		if (command_data->event_dispatcher)
			command_data->event_dispatcher = NULL;

		/* clean up command-line options */

		if (command_data->examples_directory)
		{
			DEALLOCATE(command_data->examples_directory);
		}
		if (command_data->cm_examples_directory)
		{
			DEALLOCATE(command_data->cm_examples_directory);
		}
		if (command_data->cm_parameters_file_name)
		{
			DEALLOCATE(command_data->cm_parameters_file_name);
		}

		DEALLOCATE(*command_data_address);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"DESTROY(Cmiss_command_data).  "
			"Invalid arguments");
	}


	LEAVE;

	return (return_code);
} /* DESTROY(Cmiss_command_data) */

struct Cmiss_command_data *Cmiss_command_data_access(struct Cmiss_command_data *command_data)
{
	if (command_data)
	{
		command_data->access_count++;
	}
	return command_data;
}

int Cmiss_command_data_destroy(
	struct Cmiss_command_data **command_data_address)
{
	int return_code;
	struct Cmiss_command_data *command_data;

	ENTER(Cmiss_command_data_destroy);
	if (command_data_address && (NULL != (command_data = *command_data_address)))
	{
		command_data->access_count--;
		if (0 == command_data->access_count)
		{
			DESTROY(Cmiss_command_data)(command_data_address);
		}
		*command_data_address = NULL;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_command_data_destroy.  Missing command data");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

int Cmiss_command_data_main_loop(struct Cmiss_command_data *command_data)
/*******************************************************************************
LAST MODIFIED : 19 December 2002

DESCRIPTION :
Process events until some events request the program to finish.
==============================================================================*/
{
	int return_code = 0;

	ENTER(Cmiss_command_data_main_loop);
	/* main processing / loop */
	if (command_data && command_data->event_dispatcher)
	{
		/* user interface loop */
		return_code=Event_dispatcher_main_loop(command_data->event_dispatcher);
	}
	LEAVE;

	return (return_code);
} /* Cmiss_command_data_main_loop */

struct Cmiss_region *Cmiss_command_data_get_root_region(
	struct Cmiss_command_data *command_data)
/*******************************************************************************
LAST MODIFIED : 18 April 2003

DESCRIPTION :
Returns the root region from the <command_data>.
==============================================================================*/
{
	struct Cmiss_region *root_region;

	ENTER(Cmiss_command_data_get_root_region);
	root_region=(struct Cmiss_region *)NULL;
	if (command_data)
	{
		/* API functions return accessed values */
		root_region=ACCESS(Cmiss_region)(command_data->root_region);
	}
	LEAVE;

	return (root_region);
} /* Cmiss_command_data_get_root_region */

struct Cmiss_time_keeper *Cmiss_command_data_get_default_time_keeper(
	struct Cmiss_command_data *command_data)
{
	struct Cmiss_time_keeper *default_time_keeper;

	ENTER(Cmiss_command_data_get_default_time_keeper);
	default_time_keeper=(struct Cmiss_time_keeper *)NULL;
	if (command_data)
	{
		default_time_keeper=command_data->default_time_keeper;
	}
	LEAVE;

	return (default_time_keeper);
} /* Cmiss_command_data_get_default_time_keeper */

struct Execute_command *Cmiss_command_data_get_execute_command(
	struct Cmiss_command_data *command_data)
/*******************************************************************************
LAST MODIFIED : 28 May 2003

DESCRIPTION :
Returns the execute command structure from the <command_data>, useful for
executing cmiss commands from C.
==============================================================================*/
{
	struct Execute_command *execute_command;

	ENTER(Cmiss_command_data_get_execute_command);
	execute_command=(struct Execute_command *)NULL;
	if (command_data)
	{
		execute_command=command_data->execute_command;
	}
	LEAVE;

	return (execute_command);
} /* Cmiss_command_data_get_execute_command */

struct IO_stream_package *Cmiss_command_data_get_IO_stream_package(
	struct Cmiss_command_data *command_data)
/*******************************************************************************
LAST MODIFIED : 16 September 2004

DESCRIPTION :
Returns the io_stream_package structure from the <command_data>
==============================================================================*/
{
	struct IO_stream_package *io_stream_package;

	ENTER(Cmiss_command_data_get_io_stream_package);
	io_stream_package=(struct IO_stream_package *)NULL;
	if (command_data)
	{
		io_stream_package = command_data->io_stream_package;
	}
	LEAVE;

	return (io_stream_package);
} /* Cmiss_command_data_get_io_stream_package */

struct Fdio_package* Cmiss_command_data_get_fdio_package(
	struct Cmiss_command_data *command_data)
/*******************************************************************************
LAST MODIFIED : 10 March 2005

DESCRIPTION :
Gets an Fdio_package for this <command_data>
==============================================================================*/
{
	struct Fdio_package *fdio_package;

	ENTER(Cmiss_command_data_get_fdio_package);
	fdio_package = (struct Fdio_package *)NULL;
	if (command_data)
	{
		fdio_package = CREATE(Fdio_package)(command_data->event_dispatcher);
	}
	LEAVE;

	return (fdio_package);
}

Idle_package_id Cmiss_command_data_get_idle_package(
	struct Cmiss_command_data *command_data
)
/*******************************************************************************
LAST MODIFIED : 21 March 2005

DESCRIPTION :
Gets an Idle_package for this <command_data>
==============================================================================*/
{
	struct Idle_package *idle_package;

	ENTER(Cmiss_command_data_get_idle_package);
	idle_package = (struct Idle_package *)NULL;
	if (command_data)
	{
		idle_package = CREATE(Idle_package)(command_data->event_dispatcher);
	}
	LEAVE;

	return (idle_package);
}

struct MANAGER(Computed_field) *Cmiss_command_data_get_computed_field_manager(
	struct Cmiss_command_data *command_data)
/*******************************************************************************
LAST MODIFIED : 16 September 2004

DESCRIPTION :
Returns the root region from the <command_data>.
==============================================================================*/
{
	struct MANAGER(Computed_field) *computed_field_manager;

	ENTER(Cmiss_command_data_get_computed_field_manager);
	computed_field_manager=(struct MANAGER(Computed_field) *)NULL;
	if (command_data)
	{
		computed_field_manager = Computed_field_package_get_computed_field_manager(
			command_data->computed_field_package);
	}
	LEAVE;

	return (computed_field_manager);
} /* Cmiss_command_data_get_computed_field_manager */

struct User_interface *Cmiss_command_data_get_user_interface(
	struct Cmiss_command_data *command_data)
/*******************************************************************************
LAST MODIFIED : 25 January 2006

DESCRIPTION :
Gets the user_interface for this <command_data>
==============================================================================*/
{
	struct User_interface *user_interface;

	ENTER(Cmiss_command_data_get_user_interface);
	user_interface = (struct User_interface *)NULL;
	if (command_data)
	{
		user_interface = command_data->user_interface;
	}
	LEAVE;

	return (user_interface);
} /* Cmiss_command_data_get_user_interface */

struct Cmiss_scene_viewer_app_package *Cmiss_command_data_get_scene_viewer_package(
	struct Cmiss_command_data *command_data)
/*******************************************************************************
LAST MODIFIED : 19 January 2007

DESCRIPTION :
Returns the scene viewer data from the <command_data>.
==============================================================================*/
{
	struct Cmiss_scene_viewer_app_package *cmiss_scene_viewer_package;

	ENTER(Cmiss_command_package_get_scene_viewer_package);
	cmiss_scene_viewer_package=(struct Cmiss_scene_viewer_app_package *)NULL;
	if (command_data)
	{
		cmiss_scene_viewer_package = command_data->scene_viewer_package;
	}
	LEAVE;

	return (cmiss_scene_viewer_package);
}

struct MANAGER(Graphics_window) *Cmiss_command_data_get_graphics_window_manager(
	struct Cmiss_command_data *command_data)
/*******************************************************************************
LAST MODIFIED : 26 January 2007

DESCRIPTION :
Returns the graphics_window manager from the <command_data>.
==============================================================================*/
{
	struct MANAGER(Graphics_window) *graphics_window_manager;

	ENTER(Cmiss_command_data_get_graphics_window_manager);
	graphics_window_manager=(struct MANAGER(Graphics_window) *)NULL;
#if defined (USE_CMGUI_GRAPHICS_WINDOW)
	if (command_data)
	{
		graphics_window_manager = command_data->graphics_window_manager;
	}
#else /* defined (USE_CMGUI_GRAPHICS_WINDOW) */
	USE_PARAMETER(command_data);
#endif /* defined (USE_CMGUI_GRAPHICS_WINDOW) */
	LEAVE;

	return (graphics_window_manager);
} /* Cmiss_command_data_get_graphics_window_manager */

int Cmiss_command_data_set_cmgui_string(Cmiss_command_data *command_data, const char *name_string,
	const char *version_string,const char *date_string, const char *copyright_string,
	const char *build_string, const char *revision_string)
{
	int return_code = 1;
	if (command_data)
	{
#if defined(USE_CMGUI_COMMAND_WINDOW)
		if (command_data->command_window)
		{
			return_code =Command_window_set_cmgui_string(command_data->command_window,
				name_string, version_string, date_string, copyright_string, build_string, revision_string);
		}
		else
		{
			return_code = 0;
		}
#endif
	}

	return return_code;
}

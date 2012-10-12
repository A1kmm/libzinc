
#include <math.h>

#include "api/cmiss_field_module.h"
#include "api/cmiss_field_scene_viewer_projection.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_private.hpp"
#include "computed_field/computed_field_image.h"
#include "computed_field/computed_field_set.h"
#include "general/debug.h"
#include "general/matrix_vector.h"
#include "general/mystring.h"
#include "graphics/rendition.h"
#include "graphics/scene_viewer.h"
#include "general/message.h"
#include "computed_field/computed_field_scene_viewer_projection.h"
#include "graphics/graphics_coordinate_system.hpp"

namespace {

const char computed_field_scene_viewer_projection_type_string[] = "window_projection";

void Computed_field_scene_viewer_projection_scene_viewer_callback(
	struct Scene_viewer *scene_viewer, void *dummy_void, void *field_void);

void Computed_field_scene_viewer_projection_scene_viewer_destroy_callback(
	struct Scene_viewer *scene_viewer, void *dummy_void, void *field_void);

void Computed_field_scene_viewer_projection_transformation_callback(
	Cmiss_rendition_id rendition, gtMatrix *matrix, void *field_void);

void Computed_field_scene_viewer_scene_region_change_callback(
	Cmiss_rendition_id rendition, struct Cmiss_scene *scene, void *field_void);

class Computed_field_scene_viewer_projection : public Computed_field_core
{
public:
	/* Hold onto the graphics window name so we can write out the command. */
	char *graphics_window_name;
	int pane_number;

	/* The scene_viewer is not accessed by the computed field so that we
		can still destroy the window and so the computed field responds to
		a destroy callback and then must evaluate correctly with a NULL scene_viewer */
	struct Scene_viewer *scene_viewer;
	gtMatrix *current_local_transformation;
	enum Cmiss_graphics_coordinate_system from_coordinate_system;
	enum Cmiss_graphics_coordinate_system to_coordinate_system;
	int change_required;
	Cmiss_scene_id current_scene;

	/* This flag indicates if the field has registered for callbacks with the
		scene_viewer */
	int scene_viewer_callback_flag;
	int transformation_callback_flag;

	Computed_field_scene_viewer_projection(
		Scene_viewer *scene_viewer,
		enum Cmiss_graphics_coordinate_system from_coordinate_system,
		enum Cmiss_graphics_coordinate_system to_coordinate_system) :
		Computed_field_core(),
		graphics_window_name(NULL),
		pane_number(-1), scene_viewer(scene_viewer),
		current_local_transformation(NULL),
		from_coordinate_system(from_coordinate_system),
		to_coordinate_system(to_coordinate_system),
		change_required(1),
		current_scene(NULL)
	{
		scene_viewer_callback_flag = 0;
		transformation_callback_flag = 0;
		projection_matrix = (double*)NULL;
	};

	virtual bool attach_to_field(Computed_field *parent)
	{
		if (Computed_field_core::attach_to_field(parent))
		{
			if (Scene_viewer_add_destroy_callback(scene_viewer,
				Computed_field_scene_viewer_projection_scene_viewer_destroy_callback,
				(void *)parent))
			{
				return true;
			}
		}
		return false;
	}

	~Computed_field_scene_viewer_projection();

	void set_graphics_window_name(const char *name);

	void set_pane_number(int number);

	int requiredProjectionMatrixUpdate();

	void update_current_scene();

	void remove_transformation_callback();

private:

	double *projection_matrix;

	Computed_field_core *copy()
	{
		Computed_field_scene_viewer_projection *projection_field = new Computed_field_scene_viewer_projection(
			scene_viewer, from_coordinate_system, to_coordinate_system);
		projection_field->set_graphics_window_name(graphics_window_name);
		projection_field->set_pane_number(pane_number);
		return projection_field;
	}

	const char *get_type_string()
	{
		return(computed_field_scene_viewer_projection_type_string);
	}

	int compare(Computed_field_core* other_field);

	int evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache);

	int list();

	char* get_command_string();

	int calculate_matrix();

	void add_transformation_callback();

};

int Computed_field_scene_viewer_projection::calculate_matrix()
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
==============================================================================*/
{
	ENTER(Computed_field_scene_viewer_projection_calculate_matrix);
	int return_code = 0;
	change_required = 0;
	if (field)
	{
		if (projection_matrix || ALLOCATE(projection_matrix, double, 16))
		{
			return_code = 1;
			/* make sure we are getting scene viewer callbacks once the
				 projection matrix exists */
			if (!scene_viewer_callback_flag)
			{
				scene_viewer_callback_flag = 0;
				//-- 	Scene_viewer_add_transform_callback(scene_viewer,
				//-- 		Computed_field_scene_viewer_projection_scene_viewer_callback,
				//-- 		(void *)field);
			}
			if ((from_coordinate_system == CMISS_GRAPHICS_COORDINATE_SYSTEM_LOCAL) ||
				(to_coordinate_system == CMISS_GRAPHICS_COORDINATE_SYSTEM_LOCAL))
			{
				add_transformation_callback();
			}

			double from_projection[16], inverse_to_projection[16];
			if (from_coordinate_system == to_coordinate_system)
			{
				/* identity_projection */
				for (int i=0;i<16;i++)
				{
					if (i % 5)
					{
						projection_matrix[i] = 0;
					}
					else
					{
						projection_matrix[i] = 1;
					}
				}
				return return_code;
			}
			if (Scene_viewer_get_transformation_to_window(scene_viewer, from_coordinate_system,
				current_local_transformation, from_projection) &&
				Scene_viewer_get_transformation_to_window(scene_viewer, to_coordinate_system,
					current_local_transformation, inverse_to_projection))
			{
				double lu_d, temp;
				int i, j, lu_index[4];
				if (LU_decompose(/*dimension*/4, inverse_to_projection,
					lu_index, &lu_d,/*singular_tolerance*/1.0e-12))
				{
					double to_projection[16];
					for (i = 0 ; i < 4 ; i++)
					{
						for (j = 0 ; j < 4 ; j++)
						{
							to_projection[i * 4 + j] = 0.0;
						}
						to_projection[i * 4 + i] = 1.0;
						LU_backsubstitute(/*dimension*/4, inverse_to_projection,
							lu_index, to_projection + i * 4);
					}
					/* transpose */
					for (i = 0 ; i < 4 ; i++)
					{
						for (j = i + 1 ; j < 4 ; j++)
						{
							temp = to_projection[i*4 + j];
							to_projection[i*4 + j] = to_projection[j*4 + i];
							to_projection[j*4 + i] = temp;
						}
					}
					multiply_matrix(4, 4, 4, to_projection, from_projection, projection_matrix);
				}
			}
#if defined (TEXTURE_PROJECTION)
			if (Scene_viewer_get_modelview_matrix(scene_viewer,modelview_matrix)&&
				Scene_viewer_get_window_projection_matrix(scene_viewer,
					window_projection_matrix))
			{
				/* Multiply these matrices */
				for (i=0;i<4;i++)
				{
					for (j=0;j<4;j++)
					{
						total_projection_matrix[i * 4 + j] = 0.0;
						for (k=0;k<4;k++)
						{
							total_projection_matrix[i * 4 + j] +=
								window_projection_matrix[i * 4 + k]	*
								modelview_matrix[k * 4 + j];
						}
					}
				}

				/* Get the viewport transformation too */
				Scene_viewer_get_viewport_info(scene_viewer,
					&viewport_left, &viewport_top,
					&viewport_pixels_per_unit_x, &viewport_pixels_per_unit_y);
				Scene_viewer_get_viewport_size(scene_viewer,
					&viewport_width, &viewport_height);
				/* Multiply total_projection by viewport matrices */
				for (i=0;i<4;i++)
				{
					for (j=0;j<4;j++)
					{
						viewport_matrix[i * 4 + j] = 0.0;
						for (k=0;k<4;k++)
						{
							if ((i == 0) && (k == 0))
							{
								viewport_matrix[i * 4 + j] +=
									0.5 * viewport_width / viewport_pixels_per_unit_x  *
									total_projection_matrix[k * 4 + j];
							}
							else if((i == 1) && (k == 1))
							{
								viewport_matrix[i * 4 + j] +=
									0.5 * viewport_height / viewport_pixels_per_unit_y *
									total_projection_matrix[k * 4 + j];
							}
							else if((i == 0) && (k == 3))
							{
								viewport_matrix[i * 4 + j] +=
									(viewport_left + 0.5 * viewport_width
										/ viewport_pixels_per_unit_x) *
										total_projection_matrix[k * 4 + j];
							}
							else if((i == 1) && (k == 3))
							{
								viewport_matrix[i * 4 + j] +=
									(viewport_top - 0.5 * viewport_height
										/ viewport_pixels_per_unit_y) *
										total_projection_matrix[k * 4 + j];
							}
							else if((i == 2) && (k == 2))
							{
								viewport_matrix[i * 4 + j] +=
									total_projection_matrix[k * 4 + j];
							}
							else if((i == 3) && (k == 3))
							{
								viewport_matrix[i * 4 + j] +=
									total_projection_matrix[k * 4 + j];
							}
							else
							{
								viewport_matrix[i * 4 + j] += 0.0;
							}
						}

					}
				}
				/* texture_projection */
				Scene_viewer_get_background_texture_info(scene_viewer,
					&bk_texture_left, &bk_texture_top, &bk_texture_width, &bk_texture_height,
					&bk_texture_undistort_on, &bk_texture_max_pixels_per_polygon);
				Cmiss_field_image_id image_field=
					Scene_viewer_get_background_image_field(scene_viewer);
				texture = Cmiss_field_image_get_texture(image_field);
				Cmiss_field_image_destroy(&image_field);
				if (texture)
				{
					Texture_get_distortion_info(texture, &distortion_centre_x,
						&distortion_centre_y, &distortion_factor_k1);
					Texture_get_physical_size(texture, &texture_width,
						&texture_height, &texture_depth);
					if (bk_texture_undistort_on && distortion_factor_k1)
					{
						display_message(ERROR_MESSAGE,
							"gfx_apply_projection.  Distortion corrected textures are not supported yet");
						return_code=0;
						/* identity_projection */
						for (i=0;i<16;i++)
						{
							if (i % 5)
							{
								projection_matrix[i] = 0;
							}
							else
							{
								projection_matrix[i] = 1;
							}
						}
					}
					else
					{
						/* Multiply viewport_matrix by background texture */
						for (i=0;i<4;i++)
						{
							for (j=0;j<4;j++)
							{
								texture_projection_matrix[i * 4 + j] = 0.0;
								for (k=0;k<4;k++)
								{
									if ((i == 0) && (k == 0))
									{
										texture_projection_matrix[i * 4 + j] +=
											(texture_width / bk_texture_width) *
											viewport_matrix[k * 4 + j];
									}
									else if((i == 1) && (k == 1))
									{
										texture_projection_matrix[i * 4 + j] +=
											(texture_height / bk_texture_height) *
											viewport_matrix[k * 4 + j];
									}
									else if((i == 0) && (k == 3))
									{
										texture_projection_matrix[i * 4 + j] +=
											(-bk_texture_left *
												(texture_width / bk_texture_width)) *
												viewport_matrix[k * 4 + j];
									}
									else if((i == 1) && (k == 3))
									{
										texture_projection_matrix[i * 4 + j] +=
											(- bk_texture_top *
												(texture_height / bk_texture_height)
												+ texture_height) *
												viewport_matrix[k * 4 + j];
									}
									else if((i == 2) && (k == 2))
									{
										texture_projection_matrix[i * 4 + j] +=
											viewport_matrix[k * 4 + j];
									}
									else if((i == 3) && (k == 3))
									{
										texture_projection_matrix[i * 4 + j] +=
											viewport_matrix[k * 4 + j];
									}
									else
									{
										texture_projection_matrix[i * 4 + j] += 0.0;
									}
								}
							}
						}
						for (i=0;i<16;i++)
						{
							projection_matrix[i] = texture_projection_matrix[i];
						}
					}
				}
			}
#endif
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_scene_viewer_projection_calculate_matrix.  "
				"Not enough memory for matrix");
			return_code = 0;
		}
		if (!return_code)
		{
			/* Only keep projection_matrix if valid */
			if (projection_matrix)
			{
				DEALLOCATE(projection_matrix);
				projection_matrix = (double *)NULL;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_scene_viewer_projection_calculate_matrix.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_scene_viewer_projection_calculate_matrix */

Computed_field_scene_viewer_projection::~Computed_field_scene_viewer_projection()
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Clear the type specific data used by this type.
==============================================================================*/
{

	ENTER(Computed_field_scene_viewer_projection::~Computed_field_scene_viewer_projection);
	if (field)
	{
		if (current_local_transformation)
		{
			DEALLOCATE(current_local_transformation);
		}
		if (scene_viewer_callback_flag)
		{
		//-- 	Scene_viewer_remove_transform_callback(scene_viewer,
		//-- 	  Computed_field_scene_viewer_projection_scene_viewer_callback,
		//-- 	  (void *)field);
			scene_viewer_callback_flag = 0;
		}
		remove_transformation_callback();
		if (projection_matrix)
		{
			DEALLOCATE(projection_matrix);
		}
		if (scene_viewer)
		{
			Scene_viewer_remove_destroy_callback(scene_viewer,
				Computed_field_scene_viewer_projection_scene_viewer_destroy_callback,
				(void *)field);
		}
		if (graphics_window_name)
		{
			DEALLOCATE(graphics_window_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_scene_viewer_projection::~Computed_field_scene_viewer_projection.  "
			"Invalid arguments.");
	}
	LEAVE;

} /* Computed_field_scene_viewer_projection::~Computed_field_scene_viewer_projection */

int Computed_field_scene_viewer_projection::compare(Computed_field_core *other_core)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Compare the type specific data
==============================================================================*/
{
	Computed_field_scene_viewer_projection *other;
	int return_code;

	ENTER(Computed_field_scene_viewer_projection::compare);
	if (field && (other = dynamic_cast<Computed_field_scene_viewer_projection*>(other_core)))
	{
		if ((scene_viewer == other->scene_viewer) &&
			(from_coordinate_system == other->from_coordinate_system) &&
			(to_coordinate_system == other->to_coordinate_system))
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
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_scene_viewer_projection::compare */

int Computed_field_scene_viewer_projection::evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache)
{
	RealFieldValueCache &valueCache = RealFieldValueCache::cast(inValueCache);
	if (scene_viewer)
	{
		// note, assuming matrix is owned by field not cache. Not thread safe.
		if (!requiredProjectionMatrixUpdate() || calculate_matrix())
		{
			for (int i = 0 ; i < field->number_of_components ; i++)
			{
				valueCache.values[i] = projection_matrix[i];
			}
			int number_of_xi = cache.getRequestedDerivatives();
			if (number_of_xi)
			{
				for (int i = 0 ; i < field->number_of_components ; i++)
				{
					for (int k = 0; k < number_of_xi; k++)
					{
						valueCache.derivatives[i*number_of_xi + k] = 0.0;
					}
				}
				valueCache.derivatives_valid = 1;
			}
			else
			{
				valueCache.derivatives_valid = 0;
			}
		}
		else
		{
			return 0;
		}
	}
	else
	{
		/* Just set everything to zero */
		for (int i = 0 ; i < field->number_of_components ; i++)
		{
			valueCache.values[i] = 0.0;
		}
	}
	return 1;
}

/* return 1 if projection matrix requires an update */
int Computed_field_scene_viewer_projection::requiredProjectionMatrixUpdate()
{
	int return_code = 0;
	if ((from_coordinate_system == CMISS_GRAPHICS_COORDINATE_SYSTEM_LOCAL) ||
		(to_coordinate_system == CMISS_GRAPHICS_COORDINATE_SYSTEM_LOCAL))
	{
		Cmiss_field_id field = getField();
		Cmiss_field_module_id field_module = Cmiss_field_get_field_module(field);
		if (!field_module)
		{
			return 0;
		}
		Cmiss_region_id region = Cmiss_field_module_get_region_internal(field_module);
		struct Cmiss_rendition *rendition = Cmiss_region_get_rendition_internal(region);
		gtMatrix *local_transformation_matrix = Cmiss_rendition_get_total_transformation_on_scene(
			rendition, Scene_viewer_get_scene(scene_viewer));
		if (!current_local_transformation && local_transformation_matrix)
		{
			return_code = 1;
		}
		else
		{
			if (!change_required && local_transformation_matrix)
			{
				int i, j;
				for (i=0; i<4 && !return_code; i++)
				{
					for (j=0; j<4 && !return_code; j++)
					{
						if (fabs(((*(local_transformation_matrix))[i][j]) -
							((*(current_local_transformation))[i][j])) > 0.0000001)
						{
							return_code = 1;
							break;
						}
					}
				}
			}
			else
			{
				return_code = 1;
			}
		}
		Cmiss_field_module_destroy(&field_module);
		Cmiss_rendition_destroy(&rendition);
		if (current_local_transformation)
			DEALLOCATE(current_local_transformation);
		current_local_transformation = local_transformation_matrix;
	}
	else
	{
		if (change_required)
			return_code = 1;
	}
	return return_code;
}


int Computed_field_scene_viewer_projection::list()
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_scene_viewer_projection);
	if (field)
	{
		return_code = 1;
		if (graphics_window_name)
			display_message(INFORMATION_MESSAGE,"    window : %s\n",
				graphics_window_name);
		if (pane_number > -1)
			display_message(INFORMATION_MESSAGE,"    pane number : %d\n",
				pane_number + 1);
		display_message(INFORMATION_MESSAGE,"    from_coordinate_system : %s\n",
			ENUMERATOR_STRING(Cmiss_graphics_coordinate_system)(from_coordinate_system));
		display_message(INFORMATION_MESSAGE,"    to_coordinate_system : %s\n",
			ENUMERATOR_STRING(Cmiss_graphics_coordinate_system)(to_coordinate_system));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_scene_viewer_projection.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_scene_viewer_projection */

char *Computed_field_scene_viewer_projection::get_command_string()
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string;
	int error;

	ENTER(Computed_field_scene_viewer_projection::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_scene_viewer_projection_type_string, &error);
		append_string(&command_string, " field ", &error);
		if (graphics_window_name)
		{
			append_string(&command_string, " window ", &error);
			append_string(&command_string, graphics_window_name, &error);
		}
		if (pane_number > -1)
		{
			char temp_string[40];
			sprintf(temp_string, " pane_number %d ", pane_number + 1);
			append_string(&command_string, temp_string, &error);
		}
		append_string(&command_string, " from_coordinate_system ", &error);
		append_string(&command_string,
			ENUMERATOR_STRING(Cmiss_graphics_coordinate_system)(from_coordinate_system), &error);
		append_string(&command_string, " to_coordinate_system ", &error);
		append_string(&command_string,
			ENUMERATOR_STRING(Cmiss_graphics_coordinate_system)(to_coordinate_system), &error);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_scene_viewer_projection::get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_scene_viewer_projection::get_command_string */

void Computed_field_scene_viewer_projection::set_graphics_window_name(const char *name)
{
	if (name)
		graphics_window_name = duplicate_string(name);

}

void Computed_field_scene_viewer_projection::set_pane_number(int number)
{
	pane_number = number;
}

void Computed_field_scene_viewer_projection::add_transformation_callback()
{
	if (!transformation_callback_flag)
	{
		current_scene = Scene_viewer_get_scene(scene_viewer);
		Cmiss_field_id field = getField();
		Cmiss_field_module_id field_module = Cmiss_field_get_field_module(field);
		if (field_module)
		{
			Cmiss_region_id region = Cmiss_field_module_get_region_internal(field_module);
			struct Cmiss_rendition *rendition = Cmiss_region_get_rendition_internal(region);
			transformation_callback_flag = Cmiss_rendition_add_total_transformation_callback(
				rendition, Scene_viewer_get_scene(scene_viewer),
				Computed_field_scene_viewer_projection_transformation_callback,
				Computed_field_scene_viewer_scene_region_change_callback, (void *)field);
			Cmiss_rendition_destroy(&rendition);
			Cmiss_field_module_destroy(&field_module);
		}
	}
}

void Computed_field_scene_viewer_projection::remove_transformation_callback()
{
	if (transformation_callback_flag)
	{
		Cmiss_field_id field = getField();
		Cmiss_field_module_id field_module = Cmiss_field_get_field_module(field);
		if (field_module)
		{
			Cmiss_region_id region = Cmiss_field_module_get_region_internal(field_module);
			struct Cmiss_rendition *rendition = Cmiss_region_get_rendition_internal(region);
			Cmiss_rendition_remove_total_transformation_callback(rendition,
				current_scene,	Computed_field_scene_viewer_projection_transformation_callback,
				Computed_field_scene_viewer_scene_region_change_callback,	(void *)field);
			Cmiss_rendition_destroy(&rendition);
			Cmiss_field_module_destroy(&field_module);
			transformation_callback_flag = 0;
		}
	}
}

void Computed_field_scene_viewer_projection::update_current_scene()
{
	if (current_scene != Scene_viewer_get_scene(scene_viewer))
	{
		if ((from_coordinate_system == CMISS_GRAPHICS_COORDINATE_SYSTEM_LOCAL) ||
			(to_coordinate_system == CMISS_GRAPHICS_COORDINATE_SYSTEM_LOCAL))
		{
			remove_transformation_callback();
			add_transformation_callback();
		}
		current_scene = Scene_viewer_get_scene(scene_viewer);
	}
}

void Computed_field_scene_viewer_projection_scene_viewer_callback(
	struct Scene_viewer *scene_viewer, void *dummy_void, void *field_void)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Clear the projection matrix as it is no longer valid and notify the manager
that the computed field has changed.
==============================================================================*/
{
	Computed_field* field;
	Computed_field_scene_viewer_projection* core;

	USE_PARAMETER(dummy_void);
	ENTER(Computed_field_scene_viewer_projection_scene_viewer_callback);
	if (scene_viewer && (field = (Computed_field *)field_void) &&
		(core = dynamic_cast<Computed_field_scene_viewer_projection*>(field->core)))
	{
		core->update_current_scene();
		if (!core->change_required)
		{
			if (field->manager)
			{
				Computed_field_dependency_changed(field);
			}
			core->change_required = 1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_scene_viewer_projection_scene_viewer_callback.  "
			"Invalid arguments.");
	}
	LEAVE;
} /* Computed_field_scene_viewer_projection_scene_viewer_callback */

void Computed_field_scene_viewer_projection_scene_viewer_destroy_callback(
	struct Scene_viewer *scene_viewer, void *dummy_void, void *field_void)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Clear the scene viewer reference when it is no longer valid.
==============================================================================*/
{
	Computed_field* field;
	Computed_field_scene_viewer_projection* core;

	USE_PARAMETER(dummy_void);
	ENTER(Computed_field_scene_viewer_projection_scene_viewer_destroy_callback);
	if (scene_viewer && (field = (Computed_field *)field_void) &&
		(core = dynamic_cast<Computed_field_scene_viewer_projection*>(field->core)))
	{
		if (core->scene_viewer_callback_flag)
		{
			//-- Scene_viewer_remove_transform_callback(scene_viewer,
			//--   Computed_field_scene_viewer_projection_scene_viewer_callback,
			//--   (void *)field);
			core->scene_viewer_callback_flag = 0;
		}
		if (core->graphics_window_name)
		{
			DEALLOCATE(core->graphics_window_name);
		}
		core->pane_number = 0;
		core->scene_viewer = (struct Scene_viewer *)NULL;
		if (field->manager)
		{
			Computed_field_changed(field);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_scene_viewer_projection_scene_viewer_callback.  "
			"Invalid arguments.");
	}
	LEAVE;
} /* Computed_field_scene_viewer_projection_scene_viewer_callback */

void Computed_field_scene_viewer_projection_transformation_callback(
	Cmiss_rendition_id rendition, gtMatrix *matrix,
	void *field_void)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Clear the projection matrix as it is no longer valid and notify the manager
that the computed field has changed.
==============================================================================*/
{
	Computed_field* field;
	Computed_field_scene_viewer_projection* core;
	USE_PARAMETER(matrix);

	if (rendition && (field = (Computed_field *)field_void) &&
		(core = dynamic_cast<Computed_field_scene_viewer_projection*>(field->core)))
	{
		if (!core->change_required)
		{
			if (field->manager)
			{
				Computed_field_dependency_changed(field);
			}
			core->change_required = 1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_scene_viewer_projection_transformation_callback.  "
			"Invalid arguments.");
	}
} /* Computed_field_scene_viewer_projection_scene_viewer_callback */

void Computed_field_scene_viewer_scene_region_change_callback(
	Cmiss_rendition_id rendition, struct Cmiss_scene *scene, void *field_void)
{
	Computed_field* field;
	Computed_field_scene_viewer_projection* core;

	if (rendition && (field = (Computed_field *)field_void) &&
		(core = dynamic_cast<Computed_field_scene_viewer_projection*>(field->core)))
	{
		if (core->current_scene == scene)
		{
			core->remove_transformation_callback();
			if (!core->change_required)
			{
				if (field->manager)
				{
					Computed_field_dependency_changed(field);
				}
				core->change_required = 1;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_scene_viewer_projection_transformation_callback.  "
			"Invalid arguments.");
	}
}

} //namespace

Cmiss_field_id Cmiss_field_module_create_scene_viewer_projection(
	Cmiss_field_module_id field_module,
	struct Scene_viewer *scene_viewer,
	enum Cmiss_graphics_coordinate_system from_coordinate_system,
	enum Cmiss_graphics_coordinate_system to_coordinate_system)
{
	Computed_field *field = NULL;
	if (scene_viewer)
	{
		field = Computed_field_create_generic(field_module,
			/*check_source_field_regions*/false,
			/*number_of_components*/16,
			/*number_of_source_fields*/0, NULL,
			/*number_of_source_values*/0, NULL,
			new Computed_field_scene_viewer_projection(scene_viewer, from_coordinate_system,
				to_coordinate_system));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_create_scene_viewer_projection.  Invalid argument(s)");
	}

	return (field);
}

int Cmiss_field_projection_set_window_name(struct Computed_field *field, const char *graphics_window_name)
{
	int return_code = 0;
	Computed_field_scene_viewer_projection* core = 0;

	if (field && (core = dynamic_cast<Computed_field_scene_viewer_projection*>(field->core)))
	{
		core->set_graphics_window_name(graphics_window_name);
		return_code = 1;
	}

	return return_code;
}

int Cmiss_field_projection_set_pane_number(struct Computed_field *field, int pane_number)
{
	int return_code = 0;
	Computed_field_scene_viewer_projection* core = 0;

	if (field && (core = dynamic_cast<Computed_field_scene_viewer_projection*>(field->core)))
	{
		core->set_pane_number(pane_number);
		return_code = 1;
	}

	return return_code;
}

int Computed_field_get_type_scene_viewer_projection(struct Computed_field *field,
	struct Scene_viewer **scene_viewer, char **graphics_window_name, int *pane_number,
	enum Cmiss_graphics_coordinate_system *from_coordinate_system,
	enum Cmiss_graphics_coordinate_system *to_coordinate_system)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
If the field is of type COMPUTED_FIELD_scene_viewer_projection,
<graphics_window>, <pane_number> and <projection_type> used by it are returned.
Use function Computed_field_get_type to determine the field type.
==============================================================================*/
{
	Computed_field_scene_viewer_projection* core;
	int return_code;

	ENTER(Computed_field_get_type_scene_viewer_projection);
	if (field&&(core = dynamic_cast<Computed_field_scene_viewer_projection*>(field->core))
		&& scene_viewer)
	{
		*scene_viewer = core->scene_viewer;
		if (core->graphics_window_name)
			*graphics_window_name = duplicate_string(core->graphics_window_name);
		else
			*graphics_window_name = NULL;
		*pane_number = core->pane_number;
		*from_coordinate_system = core->from_coordinate_system;
		*to_coordinate_system = core->to_coordinate_system;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_scene_viewer_projection.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_scene_viewer_projection */


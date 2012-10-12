/*******************************************************************************
FILE : spectrum.c

LAST MODIFIED : 22 January 2002

DESCRIPTION :
Spectrum functions and support code.
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
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "api/cmiss_spectrum.h"
#include "general/debug.h"
#include "general/indexed_list_private.h"
#include "general/manager_private.h"
#include "general/object.h"
#include "general/mystring.h"
#include "graphics/graphics_library.h"
#include "graphics/graphics_module.h"
#include "graphics/material.h"
#include "graphics/spectrum_settings.h"
#include "graphics/spectrum.h"
#include "general/message.h"
#include "general/enumerator_conversion.hpp"
#include "graphics/render_gl.h"
#include "graphics/spectrum.hpp"

/*
Module types
------------
*/

FULL_DECLARE_INDEXED_LIST_TYPE(Spectrum);

FULL_DECLARE_MANAGER_TYPE_WITH_OWNER(Spectrum, Cmiss_graphics_module, void *);

/*
Module functions
----------------
*/
DECLARE_INDEXED_LIST_MODULE_FUNCTIONS(Spectrum,name,const char *,strcmp)

DECLARE_LOCAL_MANAGER_FUNCTIONS(Spectrum)

/*
Global functions
----------------
*/
DECLARE_OBJECT_FUNCTIONS(Spectrum)

DECLARE_INDEXED_LIST_FUNCTIONS(Spectrum)

DECLARE_FIND_BY_IDENTIFIER_IN_INDEXED_LIST_FUNCTION(Spectrum,name,const char *,strcmp)

DECLARE_INDEXED_LIST_IDENTIFIER_CHANGE_FUNCTIONS(Spectrum,name)

PROTOTYPE_MANAGER_COPY_WITH_IDENTIFIER_FUNCTION(Spectrum,name)
{
	char *name;
	int return_code;

	ENTER(MANAGER_COPY_WITH_IDENTIFIER(Spectrum,name));
	/* check arguments */
	if (source&&destination)
	{
		if (source->name)
		{
			if (ALLOCATE(name,char,strlen(source->name)+1))
			{
				strcpy(name,source->name);
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"MANAGER_COPY_WITH_IDENTIFIER(Spectrum,name).  Insufficient memory");
				return_code=0;
			}
		}
		else
		{
			name=(char *)NULL;
			return_code=1;
		}
		if (return_code)
		{
			return_code = MANAGER_COPY_WITHOUT_IDENTIFIER(Spectrum,name)(destination, source);
			if (return_code)
			{
				/* copy values */
				DEALLOCATE(destination->name);
				destination->name=name;
			}
			else
			{
				DEALLOCATE(name);
				display_message(ERROR_MESSAGE,
"MANAGER_COPY_WITH_IDENTIFIER(Spectrum,name).  Could not copy without identifier");
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
"MANAGER_COPY_WITH_IDENTIFIER(Spectrum,name).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* MANAGER_COPY_WITH_IDENTIFIER(Spectrum,name) */

PROTOTYPE_MANAGER_COPY_WITHOUT_IDENTIFIER_FUNCTION(Spectrum,name)
{
	int return_code;

	ENTER(MANAGER_COPY_WITHOUT_IDENTIFIER(Spectrum,name));
	/* check arguments */
	if (source&&destination)
	{
		/* copy values */
		destination->maximum = source->maximum;
		destination->minimum = source->minimum;
		destination->clear_colour_before_settings =
			source->clear_colour_before_settings;

		REACCESS(Texture)(&destination->colour_lookup_texture,
			source->colour_lookup_texture);

		/* empty original list_of_settings */
		REMOVE_ALL_OBJECTS_FROM_LIST(Spectrum_settings)(
			destination->list_of_settings);
		/* put copy of each settings in source list in destination list */
		FOR_EACH_OBJECT_IN_LIST(Spectrum_settings)(
			Spectrum_settings_copy_and_put_in_list,
			(void *)destination->list_of_settings,source->list_of_settings);

		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
"MANAGER_COPY_WITHOUT_IDENTIFIER(Spectrum,name).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* MANAGER_COPY_WITHOUT_IDENTIFIER(Spectrum,name) */

PROTOTYPE_MANAGER_COPY_IDENTIFIER_FUNCTION(Spectrum,name,const char *)
{
	char *destination_name = NULL;
	int return_code;

	ENTER(MANAGER_COPY_IDENTIFIER(Spectrum,name));
	/* check arguments */
	if (name&&destination)
	{
		if (name)
		{
			if (ALLOCATE(destination_name,char,strlen(name)+1))
			{
				strcpy(destination_name,name);
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
			"MANAGER_COPY_IDENTIFIER(Spectrum,name).  Insufficient memory");
				return_code=0;
			}
		}
		else
		{
			name=(char *)NULL;
			return_code=1;
		}
		if (return_code)
		{
			/* copy name */
			DEALLOCATE(destination->name);
			destination->name=destination_name;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"MANAGER_COPY_IDENTIFIER(Spectrum,name).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* MANAGER_COPY_IDENTIFIER(Spectrum,name) */

DECLARE_MANAGER_FUNCTIONS(Spectrum,manager)

DECLARE_DEFAULT_MANAGED_OBJECT_NOT_IN_USE_FUNCTION(Spectrum,manager)

DECLARE_MANAGER_IDENTIFIER_FUNCTIONS(Spectrum,name,const char *,manager)

DECLARE_MANAGER_OWNER_FUNCTIONS(Spectrum, struct Cmiss_graphics_module)

int Spectrum_manager_set_owner(struct MANAGER(Spectrum) *manager,
	struct Cmiss_graphics_module *graphics_module)
{
	return MANAGER_SET_OWNER(Spectrum)(manager, graphics_module);
}
/*
Global functions
----------------
*/

struct Spectrum_settings *get_settings_at_position_in_Spectrum(
	 struct Spectrum *spectrum,int position)
/*******************************************************************************
LAST MODIFIED : 30 August 2007

DESCRIPTION :
Wrapper for accessing the settings in <spectrum>.
==============================================================================*/
{
	 struct Spectrum_settings *settings;

	 ENTER(get_settings_at_position_in_GT_element_group);
	 if (spectrum)
	 {
			settings=FIND_BY_IDENTIFIER_IN_LIST(Spectrum_settings,
				 position)(position,spectrum->list_of_settings);
	 }
	 else
	 {
			display_message(ERROR_MESSAGE,
				 "get_settings_at_position_in_Spectrum.  Invalid arguments");
			settings=(struct Spectrum_settings *)NULL;
	 }
	 LEAVE;

	 return (settings);
} /* get_settings_at_position_in_GT_element_group */

int Spectrum_set_simple_type(struct Spectrum *spectrum,
	enum Spectrum_simple_type type)
/*******************************************************************************
LAST MODIFIED : 7 February 2002

DESCRIPTION :
A convienience routine that allows a spectrum to be automatically set into
some predetermined simple types.
==============================================================================*/
{
	struct LIST(Spectrum_settings) *spectrum_settings_list;
	struct Spectrum_settings *settings, *second_settings;
	ZnReal maximum, minimum;
	int number_in_list, return_code;

	ENTER(Spectrum_set_simple_type);
	if (spectrum)
	{
		return_code = 1;
		minimum = spectrum->minimum;
		maximum = spectrum->maximum;
		switch(type)
		{
			case RED_TO_BLUE_SPECTRUM:
			case BLUE_TO_RED_SPECTRUM:
			{
				spectrum_settings_list = get_Spectrum_settings_list(spectrum);
				number_in_list = NUMBER_IN_LIST(Spectrum_settings)(spectrum_settings_list);
				if ( number_in_list > 0 )
				{
					REMOVE_ALL_OBJECTS_FROM_LIST(Spectrum_settings)(spectrum_settings_list);
				}
				settings = CREATE(Spectrum_settings)();
				Spectrum_settings_add(settings, /* end of list = 0 */0,
					spectrum_settings_list);

				Spectrum_settings_set_type(settings, SPECTRUM_LINEAR);
				Spectrum_settings_set_colour_mapping(settings, SPECTRUM_RAINBOW);
				Spectrum_settings_set_extend_above_flag(settings, 1);
				Spectrum_settings_set_extend_below_flag(settings, 1);
				switch (type)
				{
					case RED_TO_BLUE_SPECTRUM:
					{
						Spectrum_settings_set_reverse_flag(settings, 0);
					} break;
					case BLUE_TO_RED_SPECTRUM:
					{
						Spectrum_settings_set_reverse_flag(settings, 1);
					} break;
					default:
					{
					} break;
				}
			} break;
			case LOG_RED_TO_BLUE_SPECTRUM:
			case LOG_BLUE_TO_RED_SPECTRUM:
			{
				spectrum_settings_list = get_Spectrum_settings_list(spectrum);
				number_in_list = NUMBER_IN_LIST(Spectrum_settings)(spectrum_settings_list);
				if ( number_in_list > 0 )
				{
					REMOVE_ALL_OBJECTS_FROM_LIST(Spectrum_settings)(spectrum_settings_list);
				}
				settings = CREATE(Spectrum_settings)();
				second_settings = CREATE(Spectrum_settings)();
				Spectrum_settings_add(settings, /* end of list = 0 */0,
					spectrum_settings_list);
				Spectrum_settings_add(second_settings, /* end of list = 0 */0,
					spectrum_settings_list);


				Spectrum_settings_set_type(settings, SPECTRUM_LOG);
				Spectrum_settings_set_exaggeration(settings, 1.0);
				Spectrum_settings_set_colour_mapping(settings, SPECTRUM_RAINBOW);
				Spectrum_settings_set_range_minimum(settings, -1.0);
				Spectrum_settings_set_range_maximum(settings, 0.0);

				Spectrum_settings_set_type(second_settings, SPECTRUM_LOG);
				Spectrum_settings_set_exaggeration(second_settings, -1.0);
				Spectrum_settings_set_colour_mapping(second_settings, SPECTRUM_RAINBOW);
				Spectrum_settings_set_range_minimum(second_settings, 0.0);
				Spectrum_settings_set_range_maximum(second_settings, 1.0);

				Spectrum_settings_set_extend_below_flag(settings, 1);
				Spectrum_settings_set_extend_above_flag(second_settings, 1);

				switch(type)
				{
					case LOG_RED_TO_BLUE_SPECTRUM:
					{
						Spectrum_settings_set_reverse_flag(settings, 0);
						Spectrum_settings_set_colour_value_minimum(settings, 0);
						Spectrum_settings_set_colour_value_maximum(settings, 0.5);
						Spectrum_settings_set_reverse_flag(second_settings, 0);
						Spectrum_settings_set_colour_value_minimum(second_settings, 0.5);
						Spectrum_settings_set_colour_value_maximum(second_settings, 1.0);
					} break;
					case LOG_BLUE_TO_RED_SPECTRUM:
					{
						Spectrum_settings_set_reverse_flag(settings, 1);
						Spectrum_settings_set_colour_value_minimum(settings, 0.5);
						Spectrum_settings_set_colour_value_maximum(settings, 1.0);
						Spectrum_settings_set_reverse_flag(second_settings, 1);
						Spectrum_settings_set_colour_value_minimum(second_settings, 0.0);
						Spectrum_settings_set_colour_value_maximum(second_settings, 0.5);
					} break;
					default:
					{
					} break;
				}
			} break;
			case BLUE_WHITE_RED_SPECTRUM:
			{
				spectrum_settings_list = get_Spectrum_settings_list(spectrum);
				number_in_list = NUMBER_IN_LIST(Spectrum_settings)(spectrum_settings_list);
				if ( number_in_list > 0 )
				{
					REMOVE_ALL_OBJECTS_FROM_LIST(Spectrum_settings)(spectrum_settings_list);
				}
				settings = CREATE(Spectrum_settings)();
				second_settings = CREATE(Spectrum_settings)();
				Spectrum_settings_add(settings, /* end of list = 0 */0,
					spectrum_settings_list);
				Spectrum_settings_add(second_settings, /* end of list = 0 */0,
					spectrum_settings_list);

				Spectrum_settings_set_type(settings, SPECTRUM_LOG);
				Spectrum_settings_set_exaggeration(settings, -10.0);
				Spectrum_settings_set_range_minimum(settings, -1.0);
				Spectrum_settings_set_range_maximum(settings, 0.0);
				Spectrum_settings_set_reverse_flag(settings,1);
				/* fix the maximum (white ) at zero */
				Spectrum_settings_set_fix_maximum_flag(settings,1);
				Spectrum_settings_set_extend_below_flag(settings, 1);
				Spectrum_settings_set_colour_mapping(settings, SPECTRUM_WHITE_TO_BLUE);
				Spectrum_settings_set_colour_value_minimum(settings, 0);
				Spectrum_settings_set_colour_value_maximum(settings, 1);

				Spectrum_settings_set_type(second_settings, SPECTRUM_LOG);
				Spectrum_settings_set_exaggeration(second_settings, 10.0);
				Spectrum_settings_set_range_minimum(second_settings, 0.0);
				Spectrum_settings_set_range_maximum(second_settings, 1.0);
				/* fix the minimum (white ) at zero */
				Spectrum_settings_set_fix_minimum_flag(second_settings,1);
				Spectrum_settings_set_extend_above_flag(second_settings, 1);
				Spectrum_settings_set_colour_mapping(second_settings, SPECTRUM_WHITE_TO_RED);
				Spectrum_settings_set_colour_value_minimum(second_settings, 0);
				Spectrum_settings_set_colour_value_maximum(second_settings, 1);
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"Spectrum_set_simple_type.  Unknown simple spectrum type");
				return_code=0;
			} break;
		}
		/* Rerange the settings so that it matches the minimum and maximum of the spectrum */
		Spectrum_calculate_range(spectrum);
		Spectrum_set_minimum_and_maximum(spectrum, minimum, maximum);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Spectrum_set_simple_type.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Spectrum_set_simple_type */

enum Spectrum_simple_type Spectrum_get_simple_type(struct Spectrum *spectrum)
/*******************************************************************************
LAST MODIFIED : 18 May 2000

DESCRIPTION :
A convienience routine that interrogates a spectrum to see if it is one of the
simple types.  If it does not comform exactly to one of the simple types then
it returns UNKNOWN_SPECTRUM
==============================================================================*/
{
	struct LIST(Spectrum_settings) *spectrum_settings_list;
	struct Spectrum_settings *settings, *second_settings;
	enum Spectrum_settings_type settings_type, second_settings_type;
	int number_in_list, reverse, second_reverse;
	enum Spectrum_settings_colour_mapping colour_mapping, second_colour_mapping;
	enum Spectrum_simple_type type;

	ENTER(Spectrum_get_simple_type);

	if (spectrum)
	{
		type = UNKNOWN_SPECTRUM;

		spectrum_settings_list = get_Spectrum_settings_list(spectrum);
		number_in_list = NUMBER_IN_LIST(Spectrum_settings)(spectrum_settings_list);
		switch( number_in_list )
		{
			case 1:
			{
				settings = FIRST_OBJECT_IN_LIST_THAT(Spectrum_settings)
					((LIST_CONDITIONAL_FUNCTION(Spectrum_settings) *)NULL, NULL,
					spectrum_settings_list);
				settings_type = Spectrum_settings_get_type(settings);
				reverse = Spectrum_settings_get_reverse_flag(settings);
				colour_mapping = Spectrum_settings_get_colour_mapping(settings);

				if ( settings_type == SPECTRUM_LINEAR )
				{
					if ( colour_mapping == SPECTRUM_RAINBOW )
					{
						if ( reverse )
						{
							type = BLUE_TO_RED_SPECTRUM;
						}
						else
						{
							type = RED_TO_BLUE_SPECTRUM;
						}
					}
				}
			} break;
			case 2:
			{
				settings = FIND_BY_IDENTIFIER_IN_LIST(Spectrum_settings,position)
					(1, spectrum_settings_list);
				second_settings = FIND_BY_IDENTIFIER_IN_LIST(Spectrum_settings,position)
					(2, spectrum_settings_list);
				if ( settings && second_settings )
				{
					settings_type = Spectrum_settings_get_type(settings);
					reverse = Spectrum_settings_get_reverse_flag(settings);
					colour_mapping = Spectrum_settings_get_colour_mapping(settings);
					second_settings_type = Spectrum_settings_get_type(second_settings);
					second_reverse = Spectrum_settings_get_reverse_flag(second_settings);
					second_colour_mapping = Spectrum_settings_get_colour_mapping
						(second_settings);

					if((settings_type == SPECTRUM_LOG)
						&& (second_settings_type == SPECTRUM_LOG))
					{
						if ((colour_mapping == SPECTRUM_RAINBOW)
							&& (second_colour_mapping == SPECTRUM_RAINBOW))
						{
							if ( reverse && second_reverse )
							{
								type = LOG_BLUE_TO_RED_SPECTRUM;
							}
							else if (!(reverse || second_reverse))
							{
								type = LOG_RED_TO_BLUE_SPECTRUM;
							}
						}
						else if ((colour_mapping == SPECTRUM_WHITE_TO_BLUE)
							&& (second_colour_mapping == SPECTRUM_WHITE_TO_RED))
						{
							type = BLUE_WHITE_RED_SPECTRUM;
						}
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Spectrum_set_simple_type.  Bad position numbers in settings");
				}
			}break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Spectrum_set_simple_type.  Invalid argument(s)");
		type = UNKNOWN_SPECTRUM;
	}
	LEAVE;

	return (type);
} /* Spectrum_get_simple_type */

enum Spectrum_simple_type Spectrum_get_contoured_simple_type(struct Spectrum *spectrum)
/*******************************************************************************
LAST MODIFIED : 23 May 2000

DESCRIPTION :
A convienience routine that interrogates a spectrum to see if it is one of the
simple types, or a simple type with a contour(colour_mapping==SPECTRUM_BANDED)
added as an extra, last, setting If it does not comform exactly to one of the
simple types (or a simple type with a contour) then it returns UNKNOWN_SPECTRUM.
See also Spectrum_get_simple_type.
==============================================================================*/
{
	enum Spectrum_settings_colour_mapping colour_mapping,second_colour_mapping;
	enum Spectrum_settings_type settings_type, second_settings_type;
	enum Spectrum_simple_type spectrum_simple_type;
	int number_of_settings,reverse,second_reverse;
	struct LIST(Spectrum_settings) *spectrum_settings_list=
		(struct LIST(Spectrum_settings) *)NULL;
	struct Spectrum_settings *settings,*second_settings,*spectrum_settings;

	ENTER(Spectrum_get_contoured_simple_type);
	settings=(struct Spectrum_settings *)NULL;
	second_settings=(struct Spectrum_settings *)NULL;
	spectrum_settings=(struct Spectrum_settings *)NULL;
	spectrum_simple_type=UNKNOWN_SPECTRUM;
	if(spectrum)
	{
		/* if spectrum is a simple type, nothing else to do*/
		spectrum_simple_type=Spectrum_get_simple_type(spectrum);
		if(spectrum_simple_type==UNKNOWN_SPECTRUM)
		{
			/* is the last settings a contour? (SPECTRUM_BANDED)*/
			spectrum_settings_list = get_Spectrum_settings_list(spectrum);
			number_of_settings = NUMBER_IN_LIST(Spectrum_settings)(spectrum_settings_list);
			spectrum_settings = FIND_BY_IDENTIFIER_IN_LIST(Spectrum_settings,position)
				(number_of_settings, spectrum_settings_list);
			colour_mapping=	Spectrum_settings_get_colour_mapping(spectrum_settings);
			/*if so, proceed as for Spectrum_get_simple_type */
			if(colour_mapping==SPECTRUM_BANDED)
			{
				switch( number_of_settings )
				{
					case 2:
					{
						settings = FIRST_OBJECT_IN_LIST_THAT(Spectrum_settings)
							((LIST_CONDITIONAL_FUNCTION(Spectrum_settings) *)NULL, NULL,
								spectrum_settings_list);
						settings_type = Spectrum_settings_get_type(settings);
						reverse = Spectrum_settings_get_reverse_flag(settings);
						colour_mapping = Spectrum_settings_get_colour_mapping(settings);

						if ( settings_type == SPECTRUM_LINEAR )
						{
							if ( colour_mapping == SPECTRUM_RAINBOW )
							{
								if ( reverse )
								{
									spectrum_simple_type = BLUE_TO_RED_SPECTRUM;
								}
								else
								{
									spectrum_simple_type = RED_TO_BLUE_SPECTRUM;
								}
							}
						}
					} break;
					case 3:
					{
						settings = FIND_BY_IDENTIFIER_IN_LIST(Spectrum_settings,position)
							(1, spectrum_settings_list);
						second_settings = FIND_BY_IDENTIFIER_IN_LIST(Spectrum_settings,position)
							(2, spectrum_settings_list);
						if ( settings && second_settings )
						{
							settings_type = Spectrum_settings_get_type(settings);
							reverse = Spectrum_settings_get_reverse_flag(settings);
							colour_mapping = Spectrum_settings_get_colour_mapping(settings);
							second_settings_type = Spectrum_settings_get_type(second_settings);
							second_reverse = Spectrum_settings_get_reverse_flag(second_settings);
							second_colour_mapping = Spectrum_settings_get_colour_mapping
								(second_settings);

							if((settings_type == SPECTRUM_LOG)
								&& (second_settings_type == SPECTRUM_LOG))
							{
								if ((colour_mapping == SPECTRUM_RAINBOW)
									&& (second_colour_mapping == SPECTRUM_RAINBOW))
								{
									if ( reverse && second_reverse )
									{
										spectrum_simple_type = LOG_BLUE_TO_RED_SPECTRUM;
									}
									else if (!(reverse || second_reverse))
									{
										spectrum_simple_type = LOG_RED_TO_BLUE_SPECTRUM;
									}
								}
								else if ((colour_mapping == SPECTRUM_WHITE_TO_BLUE)
									&& (second_colour_mapping == SPECTRUM_WHITE_TO_RED))
								{
									spectrum_simple_type = BLUE_WHITE_RED_SPECTRUM;
								}
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Spectrum_set_simple_type.  Bad position numbers in settings");
						}
					}break;
				}/*switch( number_in_list ) */
			}/* if(spectrum_settings_colour_mapping==SPECTRUM_BANDED) */
			else
			{
				display_message(WARNING_MESSAGE,
					"Spectrum_get_contoured_simple_type. Spectrum not simple type or\n"
					" contoured simple type ");
			}
		}/* if(spectrum_simple_type==UNKNOWN_SPECTRUM) */
	}/* if(spectrum) */
	else
	{
		display_message(ERROR_MESSAGE,
			"Spectrum_get_contoured_simple_type. Invalid argument(s)");
	}
	LEAVE;
	return(spectrum_simple_type);
}/* Spectrum_get_contoured_simple_type */

int Spectrum_overlay_contours(struct MANAGER(Spectrum) *spectrum_manager,
	struct Spectrum *spectrum,int number_of_bands,int band_proportions)
/*******************************************************************************
LAST MODIFIED : 22 May 2000

DESCRIPTION :
Checks if the last spectrum setting is SPECTRUM_BANDED, removes it if it is,
then adds a SPECTRUM_BANDED setting to the <spectrum> with <number_of_bands>,
<band_proportions>. Setting is added at the end of the list.
This function assumes the <spectum> is a simple with an added SPECTRUM_BANDED
settings holding for the contours.
If <number_of_bands>==0, simply removes any existing contour band settings.
==============================================================================*/
{
	int return_code;
	enum Spectrum_settings_colour_mapping spectrum_settings_colour_mapping;
	FE_value min,max,number_of_settings;
	struct LIST(Spectrum_settings) *spectrum_settings_list=
					(struct LIST(Spectrum_settings) *)NULL;
	struct Spectrum *spectrum_to_be_modified_copy=
		(struct Spectrum *)NULL;
	struct Spectrum_settings *spectrum_settings=
					(struct Spectrum_settings *)NULL;

	ENTER(Spectrum_overlay_contours);
	if(spectrum_manager&&spectrum)
	{
		if (IS_MANAGED(Spectrum)(spectrum,spectrum_manager))
		{
			return_code=1;
			/* get the last settings */
			spectrum_settings_list = get_Spectrum_settings_list(spectrum);
			number_of_settings=NUMBER_IN_LIST(Spectrum_settings)
				(spectrum_settings_list);
			spectrum_settings = FIND_BY_IDENTIFIER_IN_LIST(Spectrum_settings,position)
				 ((int)number_of_settings, spectrum_settings_list);
			/*if a contour, SPECTRUM_BANDED, remove */
			spectrum_settings_colour_mapping=
				Spectrum_settings_get_colour_mapping(spectrum_settings);
			if(spectrum_settings_colour_mapping==SPECTRUM_BANDED)
			{
				Spectrum_settings_remove(spectrum_settings,spectrum_settings_list);
				spectrum_settings=(struct Spectrum_settings *)NULL;
			}
			spectrum_to_be_modified_copy=CREATE(Spectrum)("spectrum_modify_temp");
			if (spectrum_to_be_modified_copy)
			{
				/* if required,generate and set the contours setting */
				if(number_of_bands)
				{
					MANAGER_COPY_WITHOUT_IDENTIFIER(Spectrum,name)
						(spectrum_to_be_modified_copy,spectrum);
					spectrum_settings=CREATE(Spectrum_settings)();
					max=get_Spectrum_maximum(spectrum);
					min=get_Spectrum_minimum(spectrum);
					Spectrum_settings_set_range_maximum(spectrum_settings,max);
					Spectrum_settings_set_range_minimum(spectrum_settings,min);
					Spectrum_settings_set_extend_below_flag(spectrum_settings,1);
					Spectrum_settings_set_extend_above_flag(spectrum_settings,1);
					Spectrum_settings_set_type(spectrum_settings,SPECTRUM_LINEAR);
					Spectrum_settings_set_number_of_bands(spectrum_settings,number_of_bands);
					Spectrum_settings_set_black_band_proportion(spectrum_settings,
						band_proportions);
					Spectrum_settings_set_colour_mapping(spectrum_settings,SPECTRUM_BANDED);
					Spectrum_add_settings(spectrum_to_be_modified_copy,spectrum_settings,0);
					MANAGER_MODIFY_NOT_IDENTIFIER(Spectrum,name)(spectrum,
						spectrum_to_be_modified_copy,spectrum_manager);
					DEACCESS(Spectrum)(&spectrum_to_be_modified_copy);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Spectrum_overlay_contours. Could not create spectrum copy.");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Spectrum_overlay_contours. Spectrum is not in manager!");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Spectrum_overlay_contours.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;
	return (return_code);
} /* Spectrum_overlay_contours() */

int Spectrum_add_settings(struct Spectrum *spectrum,
	struct Spectrum_settings *settings,int position)
/*******************************************************************************
LAST MODIFIED : 24 June 1998

DESCRIPTION :
Adds the <settings> to <spectrum> at the given <position>, where 1 is
the top of the list (rendered first), and values less than 1 or greater than the
last position in the list cause the settings to be added at its end, with a
position one greater than the last.
==============================================================================*/
{
	int return_code;

	ENTER(Spectrum_add_settings);
	if (spectrum&&settings)
	{
		return_code=Spectrum_settings_add(settings,position,
			spectrum->list_of_settings);
		Spectrum_calculate_range(spectrum);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Spectrum_add_settings.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Spectrum_add_settings */

int Spectrum_remove_settings(struct Spectrum *spectrum,
	struct Spectrum_settings *settings)
/*******************************************************************************
LAST MODIFIED : 24 June 1998

DESCRIPTION :
Removes the <settings> from <spectrum> and decrements the position
of all subsequent settings.
==============================================================================*/
{
	int return_code;

	ENTER(Spectrum_remove_settings);
	if (spectrum&&settings)
	{
		return_code=Spectrum_settings_remove(settings,
			spectrum->list_of_settings);
		Spectrum_calculate_range(spectrum);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Spectrum_remove_settings.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Spectrum_remove_settings */

int Spectrum_remove_all_settings(struct Spectrum *spectrum)
/*******************************************************************************
LAST MODIFIED : 4 August 1998

DESCRIPTION :
Removes the all the settings from <spectrum>.
==============================================================================*/
{
	int return_code = 0;

	ENTER(Spectrum_remove_all_settings);
	if (spectrum)
	{
		REMOVE_ALL_OBJECTS_FROM_LIST(Spectrum_settings)
			(get_Spectrum_settings_list(spectrum));
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Spectrum_remove_all_settings.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* Spectrum_remove_all_settings */

int Spectrum_get_settings_position(struct Spectrum *spectrum,
	struct Spectrum_settings *settings)
/*******************************************************************************
LAST MODIFIED : 16 June 1998

DESCRIPTION :
Returns the position of <settings> in <spectrum>.
==============================================================================*/
{
	int position;

	ENTER(Spectrum_get_settings_position);
	if (spectrum&&settings)
	{
		position=Spectrum_settings_get_position(settings,
			spectrum->list_of_settings);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Spectrum_get_settings_position.  Invalid argument(s)");
		position=0;
	}
	LEAVE;

	return (position);
} /* Spectrum_get_settings_position */

int set_Spectrum_minimum(struct Spectrum *spectrum,ZnReal minimum)
/*******************************************************************************
LAST MODIFIED : 29 July 1998

DESCRIPTION :
A function to set the spectrum minimum.
==============================================================================*/
{
	ZnReal maximum;
	int return_code;

	ENTER(set_Spectrum_minimum);
	if (spectrum)
	{
		if (spectrum->maximum < minimum)
		{
			maximum = minimum;
		}
		else
		{
			maximum = spectrum->maximum;
		}
		Spectrum_set_minimum_and_maximum(spectrum, minimum, maximum);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
							"set_Spectrum_minimum.  Invalid spectrum object.");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Spectrum_minimum */

int set_Spectrum_maximum(struct Spectrum *spectrum,ZnReal maximum)
/*******************************************************************************
LAST MODIFIED : 29 July 1998

DESCRIPTION :
A function to set the spectrum maximum.
==============================================================================*/
{
	ZnReal minimum;
	int return_code;

	ENTER(set_Spectrum_maximum);
	if (spectrum)
	{
		if (spectrum->minimum > maximum)
		{
			minimum = maximum;
		}
		else
		{
			minimum = spectrum->minimum;
		}
		Spectrum_set_minimum_and_maximum(spectrum, minimum, maximum);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
							"set_Spectrum_maximum.  Invalid spectrum object.");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Spectrum_maximum */

ZnReal get_Spectrum_minimum(struct Spectrum *spectrum)
/*******************************************************************************
LAST MODIFIED : 13 August 1997

DESCRIPTION :
Returns the value of the spectrum minimum.
==============================================================================*/
{
	ZnReal minimum;
	ENTER(get_Spectrum_minimum);

	if (spectrum)
	{
		minimum=spectrum->minimum;
	}
	else
	{
		display_message(ERROR_MESSAGE,
							"get_Spectrum_minimum.  Invalid spectrum object.");
		minimum=0;
	}

	LEAVE;
	return (minimum);
} /* get_Spectrum_minimum */

ZnReal get_Spectrum_maximum(struct Spectrum *spectrum)
/*******************************************************************************
LAST MODIFIED : 13 August 1997

DESCRIPTION :
Returns the value of the spectrum maximum.
==============================================================================*/
{
	ZnReal maximum;

	ENTER(get_Spectrum_maximum);
	if (spectrum)
	{
		maximum=spectrum->maximum;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_Spectrum_maximum.  Invalid spectrum object.");
		maximum=0;
	}
	LEAVE;

	return (maximum);
} /* get_Spectrum_maximum */

int Spectrum_get_number_of_components(struct Spectrum *spectrum)
/*******************************************************************************
LAST MODIFIED : 29 September 2006

DESCRIPTION :
Returns the number_of_components used by the spectrum.
==============================================================================*/
{
	int number_of_components;

	ENTER(Spectrum_get_number_of_components);
	if (spectrum)
	{
		number_of_components = 0;
		FOR_EACH_OBJECT_IN_LIST(Spectrum_settings)(
			Spectrum_settings_expand_maximum_component_index,
			(void *)&number_of_components, spectrum->list_of_settings);

		/* indices start at 0, so add one for number */
		number_of_components++;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Spectrum_get_number_of_components.  Invalid spectrum object.");
		number_of_components = 0;
	}
	LEAVE;

	return (number_of_components);
} /* Spectrum_get_number_of_components */

enum Spectrum_colour_components
	Spectrum_get_colour_components(struct Spectrum *spectrum)
/*******************************************************************************
LAST MODIFIED : 4 October 2006

DESCRIPTION :
Returns a bit mask for the colour components modified by the spectrum.
==============================================================================*/
{
	enum Spectrum_colour_components colour_components;

	ENTER(Spectrum_get_colour_components);
	if (spectrum)
	{
		colour_components = SPECTRUM_COMPONENT_NONE;
		FOR_EACH_OBJECT_IN_LIST(Spectrum_settings)(
			Spectrum_settings_set_colour_components,
			(void *)&colour_components, spectrum->list_of_settings);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Spectrum_get_colour_components.  Invalid spectrum object.");
		colour_components = SPECTRUM_COMPONENT_NONE;
	}
	LEAVE;

	return (colour_components);
} /* Spectrum_get_colour_components */

char *Spectrum_get_name(struct Spectrum *spectrum)
/*******************************************************************************
LAST MODIFIED : 28 August 2007

DESCRIPTION :
Returns the string of the spectrum.
==============================================================================*/
{
	char *name;

	ENTER(Spectrum_get_name);
	if (spectrum)
	{
		 name = spectrum->name;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Spectrum_get_name.  Invalid spectrum object.");
		name = (char *)NULL;
	}
	LEAVE;

	return (name);
} /* Spectrum_get_name */

int Spectrum_get_opaque_colour_flag(struct Spectrum *spectrum)
/*******************************************************************************
LAST MODIFIED : 30 July 1998

DESCRIPTION :
Returns the value of the spectrum opaque flag which indicates whether the
spectrum clears the material colour before applying the settings or not.
==============================================================================*/
{
	int opaque;

	ENTER(get_Spectrum_maximum);
	if (spectrum)
	{
		opaque = spectrum->clear_colour_before_settings;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Spectrum_get_opaque_colour_flag.  Invalid spectrum object.");
		opaque = 0;
	}
	LEAVE;

	return (opaque);
} /* Spectrum_get_opaque_colour_flag */

int Spectrum_set_opaque_colour_flag(struct Spectrum *spectrum, int opaque)
/*******************************************************************************
LAST MODIFIED : 30 July 1998

DESCRIPTION :
Sets the value of the spectrum opaque flag which indicates whether the
spectrum clears the material colour before applying the settings or not.
==============================================================================*/
{
	int return_code;

	ENTER(get_Spectrum_maximum);
	if (spectrum)
	{
		spectrum->clear_colour_before_settings = opaque;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Spectrum_set_opaque_colour_flag.  Invalid spectrum object.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Spectrum_set_opaque_colour_flag */

#if defined (OPENGL_API)
struct Spectrum_render_data *spectrum_start_renderGL(
	struct Spectrum *spectrum,struct Graphical_material *material,
	int number_of_data_components)
/*******************************************************************************
LAST MODIFIED : 3 June 1999

DESCRIPTION :
Initialises the graphics state for rendering values on the current material.
==============================================================================*/
{
	ZnReal alpha;
	struct Colour value;
	struct Spectrum_render_data *render_data;

	ENTER(spectrum_start_renderGL);
	if (spectrum)
	{
		if (material)
		{
			if (ALLOCATE(render_data,struct Spectrum_render_data,1))
			{
				render_data->number_of_data_components = number_of_data_components;

				if (spectrum->clear_colour_before_settings)
				{
					render_data->material_rgba[0] = 0.0;
					render_data->material_rgba[1] = 0.0;
					render_data->material_rgba[2] = 0.0;
					render_data->material_rgba[3] = 1.0;
				}
				else
				{
					Graphical_material_get_diffuse(material, &value);
					Graphical_material_get_alpha(material, &alpha);
					render_data->material_rgba[0] = value.red;
					render_data->material_rgba[1] = value.green;
					render_data->material_rgba[2] = value.blue;
					render_data->material_rgba[3] = alpha;
				}

				FOR_EACH_OBJECT_IN_LIST(Spectrum_settings)(
					Spectrum_settings_enable,(void *)render_data,
					spectrum->list_of_settings);

				/* Always ambient and diffuse */
				glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
				glEnable(GL_COLOR_MATERIAL);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"spectrum_start_renderGL.  Unable to allocate render data.");
				render_data=(struct Spectrum_render_data *)NULL;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"spectrum_start_renderGL.  Invalid material.");
			render_data=(struct Spectrum_render_data *)NULL;
		}
	}
	else
	{
		render_data=(struct Spectrum_render_data *)NULL;
	}
	LEAVE;

	return (render_data);
} /* spectrum_start_renderGL */

int spectrum_renderGL_value(struct Spectrum *spectrum,
	struct Graphical_material *material,struct Spectrum_render_data *render_data,
	GLfloat *data)
/*******************************************************************************
LAST MODIFIED : 1 June 1999

DESCRIPTION :
Sets the graphics rendering state to represent the value 'data' in
accordance with the spectrum.
==============================================================================*/
{
	GLfloat rgba[4];
	int return_code = 1;

	ENTER(spectrum_renderGL_value);
	USE_PARAMETER(material);
	if (spectrum&&render_data)
	{
		rgba[0] = render_data->material_rgba[0];
		rgba[1] = render_data->material_rgba[1];
		rgba[2] = render_data->material_rgba[2];
		rgba[3] = render_data->material_rgba[3];

		render_data->rgba = rgba;
		render_data->data = data;

		FOR_EACH_OBJECT_IN_LIST(Spectrum_settings)(
			Spectrum_settings_activate,(void *)render_data,
			spectrum->list_of_settings);

		glColor4fv(rgba);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_renderGL_value.  Invalid arguments given.");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* spectrum_renderGL_value */

int spectrum_end_renderGL(struct Spectrum *spectrum,
	struct Spectrum_render_data *render_data)
/*******************************************************************************
LAST MODIFIED : 14 May 1998

DESCRIPTION :
Resets the graphics state after rendering values on current material.
==============================================================================*/
{
	int return_code;

	ENTER(spectrum_end_renderGL);
	if (spectrum&&render_data)
	{
		glDisable(GL_COLOR_MATERIAL);

		FOR_EACH_OBJECT_IN_LIST(Spectrum_settings)(
			Spectrum_settings_disable,(void *)render_data,
			spectrum->list_of_settings);
		DEALLOCATE(render_data);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_end_renderGL.  Invalid spectrum object");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* spectrum_end_renderGL */
#endif /* defined (OPENGL_API) */

struct Spectrum_calculate_range_iterator_data
{
	int first;
	ZnReal min;
	ZnReal max;
};

static int Spectrum_calculate_range_iterator(
	struct Spectrum_settings *settings, void *data_void)
/*******************************************************************************
LAST MODIFIED : 22 July 1998

DESCRIPTION :
Iterator function to calculate the range of the spectrum by expanding
the range from the settings.  Could be in spectrum_settings.h but putting
it here means that the iterator data structure is local and these two interdependent
functions are in one place and the iterator can have local scope.
==============================================================================*/
{
	ZnReal min, max;
	int fixed_minimum, fixed_maximum, return_code;
	struct Spectrum_calculate_range_iterator_data *data;

	ENTER(spectrum_calculate_range_iterator);
	if (settings && (data = (struct Spectrum_calculate_range_iterator_data *)data_void))
	{
		min = Spectrum_settings_get_range_minimum(settings);
		max = Spectrum_settings_get_range_maximum(settings);
		fixed_minimum = Spectrum_settings_get_fix_minimum_flag(settings);
		fixed_maximum = Spectrum_settings_get_fix_maximum_flag(settings);

		if ( data->first )
		{
// 			if (!fixed_minimum && !fixed_maximum)
// 			{
				data->min = min;
				data->max = max;
				data->first = 0;
// 			}
// 			else if (!fixed_minimum)
// 			{
// 				data->min = min;
// 				data->max = min;
// 				data->first = 0;
// 			}
// 			else if (!fixed_maximum)
// 			{
// 				data->min = max;
// 				data->max = max;
// 				data->first = 0;
// 			}
		}
		else
		{
			if (!fixed_minimum && (min < data->min))
			{
				data->min = min;
			}
			if (!fixed_maximum && (max > data->max))
			{
				data->max = max;
			}
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_calculate_range_iterator.  Invalid spectrum object");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Spectrum_calculate_range_iterator */

int Spectrum_calculate_range(struct Spectrum *spectrum)
/*******************************************************************************
LAST MODIFIED : 29 July 1998

DESCRIPTION :
Calculates the range of the spectrum from the settings it contains and updates
the minimum and maximum contained inside it.
==============================================================================*/
{
	int return_code = 0;
	struct Spectrum_calculate_range_iterator_data data;

	ENTER(spectrum_calculate_range);
	if (spectrum)
	{
		data.first = 1;
		data.min = 0;
		data.max = 0;
		FOR_EACH_OBJECT_IN_LIST(Spectrum_settings)(
			Spectrum_calculate_range_iterator,
			(void *)&data, spectrum->list_of_settings);
		if (!data.first)
		{
			spectrum->minimum = data.min;
			spectrum->maximum = data.max;
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_calculate_range.  Invalid spectrum object");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* spectrum_calculate_range */

struct Spectrum_rerange_data
{
	ZnReal old_min, old_range, old_max, min, range, max;
};
static int Spectrum_rerange_iterator(
	struct Spectrum_settings *settings, void *data_void)
/*******************************************************************************
LAST MODIFIED : 29 July 1998

DESCRIPTION :
Iterator function to calculate the range of the spectrum by expanding
the range from the settings.  Could be in spectrum_settings.h but putting
it here means that the iterator data structure is local and these two interdependent
functions are in one place and the iterator can have local scope.
==============================================================================*/
{
	ZnReal min, max;
	int return_code;
	struct Spectrum_rerange_data *data;

	ENTER(spectrum_rerange_iterator);
	if (settings && (data = (struct Spectrum_rerange_data *)data_void))
	{
		min = Spectrum_settings_get_range_minimum(settings);
		max = Spectrum_settings_get_range_maximum(settings);

		if ( data->old_range > 0.0 )
		{
			min = data->min + data->range *
				((min - data->old_min) / data->old_range);
			max = data->max - data->range *
				((data->old_max - max) / data->old_range);
		}
		else
		{
			min = data->min;
			max = data->max;
		}

		Spectrum_settings_set_range_minimum(settings, min);
		Spectrum_settings_set_range_maximum(settings, max);

		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_rerange_iterator.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Spectrum_rerange_iterator */

int Spectrum_clear_all_fixed_flags(struct Spectrum *spectrum)
/*******************************************************************************
LAST MODIFIED : 16 January 2001

DESCRIPTION :
clears the fix_maximum, fix_minimum flag for all the settings in <spectrum>
==============================================================================*/
{
	int return_code = 0;
	struct LIST(Spectrum_settings) *list_of_settings;

	ENTER(Spectrum_clear_all_fixed_flags);
	list_of_settings=(struct LIST(Spectrum_settings) *)NULL;
	if(spectrum)
	{
		list_of_settings=spectrum->list_of_settings;
		if (list_of_settings)
		{
			if (!(return_code=FOR_EACH_OBJECT_IN_LIST(Spectrum_settings)(
				Spectrum_settings_clear_fixed,(void *)NULL,list_of_settings)))
			{
				display_message(ERROR_MESSAGE,
					"Spectrum_clear_all_fixed_flags. Error setting changes");
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Spectrum_clear_all_fixed_flags.  Invalid arguments");
		return_code=0;
	}
	LEAVE;
	return(return_code);
} /* Spectrum_clear_all_fixed_flags */

/*******************************************************************************
 * Mark spectrum as changed; inform clients of its manager that it has changed
 */
static int Spectrum_changed(struct Spectrum *spectrum)
{
	int return_code;

	ENTER(Spectrum_changed);
	if (spectrum)
	{
		if (spectrum->manager)
		{
			return_code = MANAGED_OBJECT_CHANGE(Spectrum)(spectrum,
				MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(Spectrum));
		}
		else
		{
			return_code = 1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "Spectrum_changed.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

int Spectrum_set_minimum_and_maximum(struct Spectrum *spectrum,
	ZnReal minimum, ZnReal maximum)
/*******************************************************************************
LAST MODIFIED : 29 July 1998

DESCRIPTION :
Expands the range of this spectrum by adjusting the range of each settings
it contains.  The ratios of the different settings are preserved.
==============================================================================*/
{
	int return_code = 0;
	struct Spectrum_rerange_data data;

	ENTER(spectrum_set_minimum_and_maximum);
	if (spectrum && (minimum <= maximum))
	{
		if ( minimum != spectrum->minimum
			|| maximum != spectrum->maximum )
		{
			data.old_min = spectrum->minimum;
			/* Keep the range to speed calculation and the
				maximum so we get exact values */
			data.old_range = spectrum->maximum - spectrum->minimum;
			data.old_max = spectrum->maximum;
			data.min = minimum;
			data.range = maximum - minimum;
			data.max = maximum;
			FOR_EACH_OBJECT_IN_LIST(Spectrum_settings)(
				Spectrum_rerange_iterator,
				(void *)&data, spectrum->list_of_settings);
			/* Cannot assume that the minimum and maximum are now what was requested
				as the fix minimum and fix maximum flags may have overridden our re-range. */
			Spectrum_calculate_range(spectrum);
			Spectrum_changed(spectrum);
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Spectrum_set_minimum_and_maximum.  Invalid spectrum or range");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Spectrum_set_minimum_and_maximum */

int Spectrum_render_value_on_material(struct Spectrum *spectrum,
	struct Graphical_material *material, int number_of_data_components,
	GLfloat *data)
/*******************************************************************************
LAST MODIFIED : 4 October 2006

DESCRIPTION :
Uses the <spectrum> to modify the <material> to represent the <number_of_data_components>
<data> values given.
==============================================================================*/
{
	GLfloat rgba[4];
	int return_code;
	struct Colour diffuse;
	struct Spectrum_render_data render_data;

	ENTER(Spectrum_render_value_on_material);
	if (spectrum && material)
	{
		if (spectrum->clear_colour_before_settings)
		{
			rgba[0] = 0.0;
			rgba[1] = 0.0;
			rgba[2] = 0.0;
			rgba[3] = 1.0;
		}
		else
		{
			Graphical_material_get_diffuse(material, &diffuse);
			rgba[0] = (GLfloat)diffuse.red;
			rgba[1] = (GLfloat)diffuse.green;
			rgba[2] = (GLfloat)diffuse.blue;
			MATERIAL_PRECISION value;
			Graphical_material_get_alpha(material, &value);
			rgba[3] = (GLfloat)value;
		}
		render_data.rgba = rgba;
		render_data.data = data;
		render_data.number_of_data_components = number_of_data_components;

		return_code = FOR_EACH_OBJECT_IN_LIST(Spectrum_settings)(
			Spectrum_settings_activate,(void *)&render_data,
			spectrum->list_of_settings);

		diffuse.red = rgba[0];
		diffuse.green = rgba[1];
		diffuse.blue = rgba[2];

		Graphical_material_set_ambient(material, &diffuse);
		Graphical_material_set_diffuse(material, &diffuse);
		Graphical_material_set_alpha(material, rgba[3]);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_render_value_on_material.  Invalid spectrum object");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Spectrum_render_value_on_material */

int Spectrum_value_to_rgba(struct Spectrum *spectrum,int number_of_data_components,
	FE_value *data, ZnReal *rgba)
/*******************************************************************************
LAST MODIFIED : 4 October 2006

DESCRIPTION :
Uses the <spectrum> to calculate RGBA components to represent the
<number_of_data_components> <data> values.
<rgba> is assumed to be an array of four values for red, green, blue and alpha.
==============================================================================*/
{
	int return_code;
	struct Spectrum_render_data render_data;

	ENTER(spectrum_value_to_rgba);
	if (spectrum)
	{
		if (spectrum->clear_colour_before_settings)
		{
			rgba[0] = 0.0;
			rgba[1] = 0.0;
			rgba[2] = 0.0;
			rgba[3] = 1.0;
		}
		GLfloat *frgba = new GLfloat[number_of_data_components];
		CAST_TO_OTHER(frgba,data,GLfloat,number_of_data_components);
		render_data.rgba = frgba;
		GLfloat *fData = new GLfloat[number_of_data_components];
		CAST_TO_OTHER(fData,data,GLfloat,number_of_data_components);
		render_data.data = fData;
		render_data.number_of_data_components = number_of_data_components;

		return_code = FOR_EACH_OBJECT_IN_LIST(Spectrum_settings)(
			Spectrum_settings_activate,(void *)&render_data,
			spectrum->list_of_settings);

		delete[] fData;
		delete[] frgba;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_value_to_rgba.  Invalid spectrum object");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* spectrum_value_to_rgba */

int Spectrum_end_value_to_rgba(struct Spectrum *spectrum)
/*******************************************************************************
LAST MODIFIED : 13 September 2007

DESCRIPTION :
Resets the caches and graphics state after rendering values.
==============================================================================*/
{
	int return_code;
	struct Spectrum_render_data render_data;

	ENTER(Spectrum_end_value_to_rgba);
	if (spectrum)
	{
		/* render data is currently not used in disable */
		FOR_EACH_OBJECT_IN_LIST(Spectrum_settings)(
			Spectrum_settings_disable,(void *)&render_data,
			spectrum->list_of_settings);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Spectrum_end_value_to_rgba.  Invalid spectrum object");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Spectrum_end_value_to_rgba */

struct LIST(Spectrum_settings) *get_Spectrum_settings_list(
	struct Spectrum *spectrum )
/*******************************************************************************
LAST MODIFIED : 14 May 1998

DESCRIPTION :
Returns the settings list that describes the spectrum.  This is the pointer to
the object inside the spectrum so do not destroy it, any changes to it change
that spectrum.
==============================================================================*/
{
	struct LIST(Spectrum_settings) *settings_list;

	ENTER(get_Spectrum_settings_list);
	if (spectrum)
	{
		settings_list=spectrum->list_of_settings;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_Spectrum_settings_list.  Invalid argument(s)");
		settings_list=(struct LIST(Spectrum_settings) *)NULL;
	}
	LEAVE;

	return (settings_list);
} /* get_Spectrum_settings_list */

int Spectrum_list_contents(struct Spectrum *spectrum,void *dummy)
/*******************************************************************************
LAST MODIFIED : 18 May 2000

DESCRIPTION :
Writes the properties of the <spectrum> to the command window.
==============================================================================*/
{
	enum Spectrum_simple_type simple_type;
	int return_code;
	struct Spectrum_settings_list_data list_data;

	ENTER(Spectrum_list_contents);
	/* check the arguments */
	if (spectrum && (!dummy))
	{
		display_message(INFORMATION_MESSAGE,"spectrum : ");
		display_message(INFORMATION_MESSAGE,spectrum->name);
		display_message(INFORMATION_MESSAGE,"\n");
		simple_type = Spectrum_get_simple_type(spectrum);
		switch(simple_type)
		{
			case BLUE_TO_RED_SPECTRUM:
			{
				display_message(INFORMATION_MESSAGE,"  simple spectrum type: BLUE_TO_RED\n");
			} break;
			case RED_TO_BLUE_SPECTRUM:
			{
				display_message(INFORMATION_MESSAGE,"  simple spectrum type: RED_TO_BLUE\n");
			} break;
			case LOG_BLUE_TO_RED_SPECTRUM:
			{
				display_message(INFORMATION_MESSAGE,"  simple spectrum type: LOG_BLUE_TO_RED\n");
			} break;
			case LOG_RED_TO_BLUE_SPECTRUM:
			{
				display_message(INFORMATION_MESSAGE,"  simple spectrum type: LOG_RED_TO_BLUE\n");
			} break;
			case BLUE_WHITE_RED_SPECTRUM:
			{
				display_message(INFORMATION_MESSAGE,"  simple spectrum type: BLUE_WHITE_RED\n");
			} break;
			default:
			{
				display_message(INFORMATION_MESSAGE,"  simple spectrum type: UNKNOWN\n");
			} break;
		}
		display_message(INFORMATION_MESSAGE,"  minimum=%.3g, maximum=%.3g\n",
			spectrum->minimum,spectrum->maximum);
		if (spectrum->clear_colour_before_settings)
		{
			display_message(INFORMATION_MESSAGE,"  clear before settings\n");
		}
		list_data.settings_string_detail=SPECTRUM_SETTINGS_STRING_COMPLETE_PLUS;
		list_data.line_prefix="  ";
		list_data.line_suffix="";
		return_code=FOR_EACH_OBJECT_IN_LIST(Spectrum_settings)(
			Spectrum_settings_list_contents,(void *)&list_data,
			spectrum->list_of_settings);
		display_message(INFORMATION_MESSAGE,"  access count=%d\n",
			spectrum->access_count);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Spectrum_list_contents.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Spectrum_list_contents */

struct Spectrum *CREATE(Spectrum)(const char *name)
/*******************************************************************************
LAST MODIFIED : 14 May 1998

DESCRIPTION :
Allocates memory and assigns fields for a Spectrum object.
==============================================================================*/
{
	struct Spectrum *spectrum;

	ENTER(CREATE(Spectrum));
	if (name)
	{
		if (ALLOCATE(spectrum,struct Spectrum,1))
		{
			spectrum->maximum=0;
			spectrum->minimum=0;
			spectrum->clear_colour_before_settings = 1;
			spectrum->manager = (struct MANAGER(Spectrum) *)NULL;
			spectrum->manager_change_status = MANAGER_CHANGE_NONE(Spectrum);
			spectrum->access_count=1;
			spectrum->colour_lookup_texture = (struct Texture *)NULL;
			spectrum->is_managed_flag = false;
			spectrum->list_of_settings=CREATE(LIST(Spectrum_settings))();
			if (spectrum->list_of_settings)
			{
				if (name)
				{
					if (ALLOCATE(spectrum->name,char,strlen(name)+1))
					{
						strcpy(spectrum->name,name);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"CREATE(Spectrum).  Cannot allocate spectrum name");
						DEALLOCATE(spectrum);
						spectrum=(struct Spectrum *)NULL;
					}
				}
				else
				{
					spectrum->name=NULL;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"CREATE(Spectrum).  Cannot create settings list");
				DEALLOCATE(spectrum);
				spectrum=(struct Spectrum *)NULL;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"CREATE(Spectrum).  Cannot allocate spectrum");
			spectrum=(struct Spectrum *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"CREATE(Spectrum).  Missing name");
		spectrum=(struct Spectrum *)NULL;
	}
	LEAVE;

	return (spectrum);
} /* CREATE(Spectrum) */

int DESTROY(Spectrum)(struct Spectrum **spectrum_ptr)
/*******************************************************************************
LAST MODIFIED : 28 October 1997

DESCRIPTION :
Frees the memory for the fields of <**spectrum>, frees the memory for
<**spectrum> and sets <*spectrum> to NULL.
==============================================================================*/
{
	int return_code;

	ENTER(DESTROY(Spectrum));
	if (spectrum_ptr)
	{
		if (*spectrum_ptr)
		{
			if ((*spectrum_ptr)->name)
			{
				DEALLOCATE((*spectrum_ptr)->name);
			}
			if ((*spectrum_ptr)->colour_lookup_texture)
			{
				DEACCESS(Texture)(&((*spectrum_ptr)->colour_lookup_texture));
			}
			DESTROY(LIST(Spectrum_settings))(&((*spectrum_ptr)->list_of_settings));
			DEALLOCATE(*spectrum_ptr);
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"DESTROY(Spectrum).  Invalid argument");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Spectrum) */

DECLARE_DEFAULT_GET_OBJECT_NAME_FUNCTION(Spectrum)

static int Spectrum_render_colour_lookup(struct Spectrum *spectrum)
/*******************************************************************************
LAST MODIFIED : 23 May 2005

DESCRIPTION :
Rebuilds the display_list for <spectrum> if it is not current.
==============================================================================*/
{
	enum Spectrum_colour_components colour_components;
	int i, indices[3], number_of_data_components, number_of_texture_components,
		number_of_values, return_code, table_size;
	GLfloat data[3], rgba[4];
	unsigned char *colour_table, *colour_table_ptr;
	struct Spectrum_render_data render_data;
	enum Texture_storage_type storage;

	ENTER(Spectrum_render_colour_lookup);
	if (spectrum)
	{
		colour_components = Spectrum_get_colour_components(spectrum);
		/* The component layout here must match the understanding of
			the texture components in the material programs */
		/* Could save memory by having a special treatment for MONOCHROME
			spectrums */
		if (colour_components & SPECTRUM_COMPONENT_ALPHA)
		{
			if (colour_components == SPECTRUM_COMPONENT_ALPHA)
			{
				/* Alpha only */
				number_of_texture_components = 1;
				/* We don't have an ALPHA only format and the material program
					now does the interpretation anyway */
				storage = TEXTURE_LUMINANCE;
			}
			else
			{
				/* Colour and alpha */
				number_of_texture_components = 4;
				storage = TEXTURE_RGBA;
			}
		}
		else
		{
			/* Colour only */
			number_of_texture_components = 3;
			storage = TEXTURE_RGB;
		}

		number_of_data_components = Spectrum_get_number_of_components(
			spectrum);
		switch (number_of_data_components)
		{
			case 1:
			{
				number_of_values = 1024;
			} break;
			case 2:
			{
				number_of_values = 256;
			} break;
			case 3:
			{
				number_of_values = 32;
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"Spectrum_render_colour_lookup.  "
					"The spectrum %d uses more than 3 components, only the first 3 will be used.",
					spectrum->name);
				number_of_data_components = 3;
				number_of_values = 32;
			}
		}
		table_size = (int) (number_of_texture_components *
			 pow((double) number_of_values, number_of_data_components));
		if (ALLOCATE(colour_table, unsigned char, table_size))
		{
			colour_table_ptr = colour_table;

			for (i = 0 ; i < number_of_data_components ; i++)
			{
				indices[i] = 0;
				data[i] = 0.0;
			}
			colour_table_ptr = colour_table;

			render_data.rgba = rgba;
			render_data.data = data;
			render_data.number_of_data_components = number_of_data_components;

			FOR_EACH_OBJECT_IN_LIST(Spectrum_settings)(
				Spectrum_settings_enable,(void *)&render_data,
				spectrum->list_of_settings);

			while (indices[number_of_data_components - 1] < number_of_values)
			{
				if (spectrum->clear_colour_before_settings)
				{
					rgba[0] = 0.0;
					rgba[1] = 0.0;
					rgba[2] = 0.0;
					rgba[3] = 1.0;
				}
				return_code = FOR_EACH_OBJECT_IN_LIST(Spectrum_settings)(
					Spectrum_settings_activate,(void *)&render_data,spectrum->list_of_settings);
				if (return_code)
				{
					if (colour_components != SPECTRUM_COMPONENT_ALPHA)
					{
						/* Not alpha only */
						 *colour_table_ptr =(unsigned char) (rgba[0] * 255.0);
						colour_table_ptr++;
						*colour_table_ptr = (unsigned char) (rgba[1] * 255.0);
						colour_table_ptr++;
						*colour_table_ptr = (unsigned char) (rgba[2] * 255.0);
						colour_table_ptr++;
					}
					if (colour_components & SPECTRUM_COMPONENT_ALPHA)
					{
						/* Alpha with or without colour */
						 *colour_table_ptr = (unsigned char) (rgba[3] * 255.0);
						colour_table_ptr++;
					}
				}

				indices[0]++;
				i = 0;
				data[0] = (GLfloat)indices[0] / (GLfloat)(number_of_values - 1);
				while ((i < number_of_data_components - 1) &&
					(indices[i] == number_of_values))
				{
					indices[i] = 0;
					data[i] = 0.0;
					i++;
					indices[i]++;
					data[i] = (GLfloat)indices[i] / (GLfloat)(number_of_values - 1);
				}
			}

			/* Finished using the spectrum for now (clear computed field cache) */
			FOR_EACH_OBJECT_IN_LIST(Spectrum_settings)(
				Spectrum_settings_disable,(void *)&render_data,
				spectrum->list_of_settings);

			{
				struct Texture *texture;

				if (spectrum->colour_lookup_texture)
				{
					DEACCESS(Texture)(&spectrum->colour_lookup_texture);
				}

				texture = CREATE(Texture)("spectrum_texture");
				/* The mode of this texture must match that relied upon by
					the fragment program in material.c.  Specifically, to provide
					correct linear interpolation the input value has to be offset
					by half a pixel and scaled by (number_of_pixels-1)/(number_of_pixels)
					as linear interpolation starts at the centres of each pixel. */
				Texture_set_filter_mode(texture, TEXTURE_LINEAR_FILTER);
				Texture_set_wrap_mode(texture, TEXTURE_CLAMP_WRAP);
				colour_table_ptr = colour_table;
				switch (number_of_data_components)
				{
					case 1:
					{
						Texture_allocate_image(texture, number_of_values, 1,
							1, storage,
							/*number_of_bytes_per_component*/1, "bob");
						Texture_set_image_block(texture,
							/*left*/0, /*bottom*/0, number_of_values, 1,
							/*depth_plane*/0, number_of_values * number_of_texture_components,
							colour_table_ptr);
					} break;
					case 2:
					{
						Texture_allocate_image(texture, number_of_values, number_of_values,
							1, storage,
							/*number_of_bytes_per_component*/1, "bob");
						Texture_set_image_block(texture,
							/*left*/0, /*bottom*/0, number_of_values, number_of_values,
							/*depth_plane*/0, number_of_values * number_of_texture_components,
							colour_table_ptr);
					} break;
					case 3:
					{
						Texture_allocate_image(texture, number_of_values, number_of_values,
							number_of_values, storage,
							/*number_of_bytes_per_component*/1, "bob");
						for (i = 0 ; i < number_of_values ; i++)
						{
							Texture_set_image_block(texture,
								/*left*/0, /*bottom*/0, number_of_values, number_of_values,
								/*depth_plane*/i,
								number_of_values * number_of_texture_components,
								colour_table_ptr);
							colour_table_ptr += number_of_values * number_of_values *
								number_of_texture_components;
						}
					} break;
				}


				spectrum->colour_lookup_texture = ACCESS(Texture)(texture);
			}

			return_code = 1;
			DEALLOCATE(colour_table);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"compile_Graphical_spectrum.  Could not allocate temporary storage.");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Spectrum_render_colour_lookup.  Missing spectrum");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Spectrum_render_colour_lookup */

int Spectrum_compile_colour_lookup(struct Spectrum *spectrum,
	Render_graphics_opengl *renderer)
/*******************************************************************************
LAST MODIFIED : 10 May 2005

DESCRIPTION :
Rebuilds the display_list for <spectrum> if it is not current.
==============================================================================*/
{
	int return_code;

	ENTER(Spectrum_compile_colour_lookup);
	if (spectrum)
	{
		Spectrum_render_colour_lookup(spectrum);

		return_code = renderer->Texture_compile(spectrum->colour_lookup_texture);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Spectrum_compile_colour_lookup.  Missing spectrum");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Spectrum_compile_colour_lookup */

int Spectrum_execute_colour_lookup(struct Spectrum *spectrum,
	Render_graphics_opengl *renderer)
/*******************************************************************************
LAST MODIFIED : 10 May 2005

DESCRIPTION :
Activates <spectrum> by calling its display list. If the display list is not
current, an error is reported.
If a NULL <spectrum> is supplied, spectrums are disabled.
==============================================================================*/
{
	int return_code;

	ENTER(Spectrum_execute_colour_lookup);
	return_code=0;
	if (spectrum && spectrum->colour_lookup_texture)
	{
		return_code = renderer->Texture_execute(spectrum->colour_lookup_texture);
	}
	else
	{
		return_code=1;
	}
	LEAVE;

	return (return_code);
} /* Spectrum_execute_colour_lookup */

int Spectrum_get_colour_lookup_sizes(struct Spectrum *spectrum,
	int *lookup_dimension, int **lookup_sizes)
/*******************************************************************************
LAST MODIFIED : 2 May 2007

DESCRIPTION :
Returns the sizes used for the colour lookup spectrums internal texture.
==============================================================================*/
{
	int return_code, width, height, depth;

	ENTER(Spectrum_get_colour_lookup_sizes);
	return_code=0;
	if (spectrum && spectrum->colour_lookup_texture)
	{
		Texture_get_dimension(spectrum->colour_lookup_texture, lookup_dimension);

		ALLOCATE(*lookup_sizes, int, *lookup_dimension);
		Texture_get_size(spectrum->colour_lookup_texture,
			&width, &height, &depth);
		if (0 < *lookup_dimension)
		{
			(*lookup_sizes)[0] = width;
		}
		if (1 < *lookup_dimension)
		{
			(*lookup_sizes)[1] = height;
		}
		if (2 < *lookup_dimension)
		{
			(*lookup_sizes)[2] = depth;
		}
		return_code = 1;
	}
	else
	{
		return_code=1;
	}
	LEAVE;

	return (return_code);
} /* Spectrum_get_colour_lookup_sizes */

int Cmiss_spectrum_set_name(
	Cmiss_spectrum_id spectrum, const char *name)
{
	int return_code = 0;

	ENTER(Cmiss_spectrum_set_name);
	if (spectrum && spectrum->manager && name)
	{
		return_code = MANAGER_MODIFY_IDENTIFIER(Spectrum, name)(
			spectrum, name, spectrum->manager);
	}
	LEAVE;

	return return_code;
}

char *Cmiss_spectrum_get_name(Cmiss_spectrum_id spectrum)
{
	char *name = NULL;
	if (spectrum)
	{
		name = duplicate_string(spectrum->name);
	}

	return name;
}

Cmiss_spectrum_id Cmiss_spectrum_access(Cmiss_spectrum_id spectrum)
{
	ENTER(Cmiss_spectrum_destroy);
	if (spectrum)
	{
		return ACCESS(Spectrum)(spectrum);
	}

	return NULL;
}

int Cmiss_spectrum_destroy(Cmiss_spectrum_id *spectrum_address)
{
	int return_code = 0;
	struct Spectrum *spectrum;

	ENTER(Cmiss_spectrum_destroy);
	if (spectrum_address && (spectrum = *spectrum_address))
	{
		(spectrum->access_count)--;
		if (spectrum->access_count <= 0)
		{
			return_code = DESTROY(Spectrum)(spectrum_address);
		}
		else if ((!spectrum->is_managed_flag) && (spectrum->manager) &&
			((1 == spectrum->access_count) || ((2 == spectrum->access_count) &&
				(MANAGER_CHANGE_NONE(Spectrum) != spectrum->manager_change_status))))
		{
			return_code = REMOVE_OBJECT_FROM_MANAGER(Spectrum)(spectrum, spectrum->manager);
		}
		else
		{
			return_code = 1;
		}
		*spectrum_address = (struct Spectrum *)NULL;
	}
	LEAVE;

	return return_code;
}

int Cmiss_spectrum_get_attribute_integer(Cmiss_spectrum_id spectrum,
	enum Cmiss_spectrum_attribute attribute)
{
	int value = 0;
	if (spectrum)
	{
		switch (attribute)
		{
		case CMISS_SPECTRUM_ATTRIBUTE_IS_MANAGED:
			value = (int)spectrum->is_managed_flag;
			break;
		default:
			display_message(ERROR_MESSAGE,
				"Cmiss_spectrum_get_attribute_integer.  Invalid attribute");
			break;
		}
	}
	return value;
}

int Cmiss_spectrum_set_attribute_integer(Cmiss_spectrum_id spectrum,
	enum Cmiss_spectrum_attribute attribute, int value)
{
	int return_code = 0;
	if (spectrum)
	{
		return_code = 1;
		int old_value = Cmiss_spectrum_get_attribute_integer(spectrum, attribute);
		enum MANAGER_CHANGE(Spectrum) change =
			MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(Spectrum);
		switch (attribute)
		{
		case CMISS_SPECTRUM_ATTRIBUTE_IS_MANAGED:
			spectrum->is_managed_flag = (value != 0);
			change = MANAGER_CHANGE_NOT_RESULT(Spectrum);
			break;
		default:
			display_message(ERROR_MESSAGE,
				"Cmiss_spectrum_set_attribute_integer.  Invalid attribute");
			return_code = 0;
			break;
		}
		if (Cmiss_spectrum_get_attribute_integer(spectrum, attribute) != old_value)
		{
			MANAGED_OBJECT_CHANGE(Spectrum)(spectrum, change);
		}
	}
	return return_code;
}

class Cmiss_spectrum_attribute_conversion
{
public:
	static const char *to_string(enum Cmiss_spectrum_attribute attribute)
	{
		const char *enum_string = 0;
		switch (attribute)
		{
			case CMISS_SPECTRUM_ATTRIBUTE_IS_MANAGED:
				enum_string = "IS_MANAGED";
				break;
			default:
				break;
		}
		return enum_string;
	}
};

enum Cmiss_spectrum_attribute Cmiss_spectrum_attribute_enum_from_string(
	const char *string)
{
	return string_to_enum<enum Cmiss_spectrum_attribute,
	Cmiss_spectrum_attribute_conversion>(string);
}

char *Cmiss_spectrum_attribute_enum_to_string(enum Cmiss_spectrum_attribute attribute)
{
	const char *attribute_string = Cmiss_spectrum_attribute_conversion::to_string(attribute);
	return (attribute_string ? duplicate_string(attribute_string) : 0);
}

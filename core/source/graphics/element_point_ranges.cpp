/*******************************************************************************
FILE : element_point_ranges.c

LAST MODIFIED : 19 March 2003

DESCRIPTION :
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
#include <stdlib.h>
#include <stdio.h>

#include "computed_field/computed_field.h"
#include "computed_field/computed_field_finite_element.h"
#include "finite_element/finite_element.h"
#include "finite_element/finite_element_discretization.h"
#include "finite_element/finite_element_region.h"
#include "general/debug.h"
#include "general/indexed_list_private.h"
#include "graphics/element_point_ranges.h"
#include "general/message.h"

/*
Module types
------------
*/

struct Element_point_ranges
/*******************************************************************************
LAST MODIFIED : 25 February 2000

DESCRIPTION :
Stores ranges of element/grid points in an element, used for selection.
==============================================================================*/
{
	/* identifier points at id for indexing in lists */
	struct Element_point_ranges_identifier id,*identifier;
	struct Multi_range *ranges;
	int access_count;
}; /* struct Element_point_ranges */

FULL_DECLARE_INDEXED_LIST_TYPE(Element_point_ranges);

/*
Module functions
----------------
*/

DECLARE_INDEXED_LIST_MODULE_FUNCTIONS(Element_point_ranges,identifier, \
	struct Element_point_ranges_identifier *, \
	compare_Element_point_ranges_identifier)

/*
Global functions
----------------
*/

const char **Xi_discretization_mode_get_valid_strings_for_Element_point_ranges(
	int *number_of_valid_strings)
/*******************************************************************************
LAST MODIFIED : 19 March 2001

DESCRIPTION :
Returns an allocated array of pointers to all static strings for valid
Xi_discretization_modes that can be used for Element_point_ranges, obtained
from function ENUMERATOR_STRING.
Up to calling function to deallocate returned array - but not the strings in it!
==============================================================================*/
{
	const char **valid_strings;

	ENTER(Xi_discretization_mode_get_valid_strings_for_Element_point_ranges);
	if (number_of_valid_strings)
	{
		*number_of_valid_strings=3;
		if (ALLOCATE(valid_strings,const char *,*number_of_valid_strings))
		{
			valid_strings[0] = ENUMERATOR_STRING(Xi_discretization_mode)(
				XI_DISCRETIZATION_CELL_CENTRES);
			valid_strings[1] = ENUMERATOR_STRING(Xi_discretization_mode)(
				XI_DISCRETIZATION_CELL_CORNERS);
			valid_strings[2] = ENUMERATOR_STRING(Xi_discretization_mode)(
				XI_DISCRETIZATION_EXACT_XI);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Xi_discretization_mode_get_valid_strings_for_Element_point_ranges.  "
				"Not enough memory");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Xi_discretization_mode_get_valid_strings_for_Element_point_ranges.  "
			"Invalid argument(s)");
		valid_strings=(const char **)NULL;
	}
	LEAVE;

	return (valid_strings);
} /* Xi_discretization_mode_get_valid_strings_for_Element_point_ranges */

int compare_Element_point_ranges_identifier(
	struct Element_point_ranges_identifier *identifier1,
	struct Element_point_ranges_identifier *identifier2)
/*******************************************************************************
LAST MODIFIED : 8 June 2000

DESCRIPTION :
Returns -1 (identifier1 less), 0 (equal) or +1 (identifier1 greater) for
indexing lists of Element_point_ranges.
First the elements are compared, then the Xi_discretization_mode, then the
identifying values depending on this mode.
==============================================================================*/
{
	int dimension,i,return_code;

	ENTER(compare_Element_point_ranges_identifier);
	if (identifier1&&identifier2)
	{
		if (identifier1->element < identifier2->element)
		{
			return_code = -1;
		}
		else if (identifier1->element > identifier2->element)
		{
			return_code = 1;
		}
		else
		{
			/* same element; now compare top_level_element */
			if (identifier1->top_level_element < identifier2->top_level_element)
			{
				return_code = -1;
			}
			else if (identifier1->top_level_element > identifier2->top_level_element)
			{
				return_code = 1;
			}
			else
			{
				/* same elements; now compare xi_discretization_mode */
				if (identifier1->xi_discretization_mode <
					identifier2->xi_discretization_mode)
				{
					return_code = -1;
				}
				else if (identifier1->xi_discretization_mode >
					identifier2->xi_discretization_mode)
				{
					return_code = 1;
				}
				else
				{
					/* same xi_discretization mode; now compare identifying values
						 depending on this mode */
					dimension=get_FE_element_dimension(identifier1->element);
					switch (identifier1->xi_discretization_mode)
					{
						case XI_DISCRETIZATION_CELL_CENTRES:
						case XI_DISCRETIZATION_CELL_CORNERS:
						{
							return_code=0;
							for (i=0;!return_code&&(i<dimension);i++)
							{
								if (identifier1->number_in_xi[i] < identifier2->number_in_xi[i])
								{
									return_code = -1;
								}
								else if (identifier1->number_in_xi[i] >
									identifier2->number_in_xi[i])
								{
									return_code = 1;
								}
							}
						} break;
						case XI_DISCRETIZATION_EXACT_XI:
						{
							return_code=0;
							for (i=0;!return_code&&(i<dimension);i++)
							{
								if (identifier1->exact_xi[i] < identifier2->exact_xi[i])
								{
									return_code = -1;
								}
								else if (identifier1->exact_xi[i] > identifier2->exact_xi[i])
								{
									return_code = 1;
								}
							}
						} break;
						default:
						{
							display_message(ERROR_MESSAGE,
								"compare_Element_point_ranges_identifier.  "
								"Invalid Xi_discretization_mode");
							/* error defaults to the same? */
							return_code=0;
						} break;
					}
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"compare_Element_point_ranges_identifier.  Invalid argument(s)");
		/* error defaults to the same? */
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* compare_Element_point_ranges_identifier */

int Element_point_ranges_identifier_is_valid(
	struct Element_point_ranges_identifier *identifier)
/*******************************************************************************
LAST MODIFIED : 8 June 2000

DESCRIPTION :
Returns true if <identifier> has a valid element, Xi_discretization_mode and
number_in_xi for being used in an Element_point_ranges structure.
Writes what is invalid about the identifier.
==============================================================================*/
{
	int dimension,i,return_code;

	ENTER(Element_point_ranges_identifier_is_valid);
	if (identifier)
	{
		if (identifier->element&&identifier->top_level_element&&
			FE_element_has_top_level_element(identifier->element,
				(void *)identifier->top_level_element))
		{
			return_code=1;
			dimension=get_FE_element_dimension(identifier->element);
			switch (identifier->xi_discretization_mode)
			{
				case XI_DISCRETIZATION_CELL_CENTRES:
				case XI_DISCRETIZATION_CELL_CORNERS:
				{
					for (i=0;i<dimension;i++)
					{
						/* number_in_xi must be <= 1 in each direction */
						if (1 > identifier->number_in_xi[i])
						{
							display_message(ERROR_MESSAGE,
								"Element_point_ranges_identifier_is_valid.  "
								"Invalid number_in_xi[%d] of %d",i,
								identifier->number_in_xi[i]);
							return_code=0;
						}
					}
				} break;
				case XI_DISCRETIZATION_EXACT_XI:
				{
					for (i=0;i<dimension;i++)
					{
						/* number_in_xi must be 1 in each direction */
						if (1 != identifier->number_in_xi[i])
						{
							display_message(ERROR_MESSAGE,
								"Element_point_ranges_identifier_is_valid.  "
								"Invalid EXACT_XI number_in_xi[%d] of %d; should be 1",i,
								identifier->number_in_xi[i]);
							return_code=0;
						}
					}
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"Element_point_ranges_identifier_is_valid.  "
						"Invalid Xi_discretization_mode: %s",
						ENUMERATOR_STRING(Xi_discretization_mode)(
							identifier->xi_discretization_mode));
					return_code=0;
				} break;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Element_point_ranges_identifier_is_valid.  Invalid element(s)");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_ranges_identifier_is_valid.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return(return_code);
} /* Element_point_ranges_identifier_is_valid */

int Element_point_ranges_identifier_element_point_number_is_valid(
	struct Element_point_ranges_identifier *identifier,int element_point_number)
/*******************************************************************************
LAST MODIFIED : 23 April 2001

DESCRIPTION :
Returns true if <element_point_number> is in the number_in_xi range for
<identifier>. Assumes <identifier> is already validated by
Element_point_ranges_identifier_is_valid.
==============================================================================*/
{
	int number_of_xi_points, return_code;

	ENTER(Element_point_ranges_identifier_element_point_number_is_valid);
	if (identifier)
	{
		return_code = ((0 <= element_point_number) &&
			FE_element_get_xi_points(identifier->element,
				identifier->xi_discretization_mode,
				identifier->number_in_xi, identifier->exact_xi,
				(Cmiss_field_cache_id)0,
				/*coordinate_field*/(struct Computed_field *)NULL,
				/*density_field*/(struct Computed_field *)NULL,
				&number_of_xi_points, /*xi_points_address*/(FE_value_triple **)NULL) &&
			(element_point_number < number_of_xi_points));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_ranges_identifier_element_point_number_is_valid.  "
			"Missing identifier");
		return_code=0;
	}
	LEAVE;

	return(return_code);
} /* Element_point_ranges_identifier_element_point_number_is_valid */

PROTOTYPE_COPY_OBJECT_FUNCTION(Element_point_ranges_identifier)
/*******************************************************************************
LAST MODIFIED : 8 June 2000

DESCRIPTION :
syntax: COPY(Element_point_ranges_identifier)(destination,source)
Copies the contents of source to destination.
Note! No accessing of elements is assumed or performed by this function; it is
purely a copy. [DE]ACCESSing must be handled by calling function if required.
==============================================================================*/
{
	int i,return_code;

	ENTER(COPY(Element_point_ranges_identifier));
	if (destination&&source&&(destination!=source))
	{
		destination->element=source->element;
		destination->top_level_element=source->top_level_element;
		destination->xi_discretization_mode=source->xi_discretization_mode;
		for (i=0;i<MAXIMUM_ELEMENT_XI_DIMENSIONS;i++)
		{
			destination->number_in_xi[i]=source->number_in_xi[i];
			destination->exact_xi[i]=source->exact_xi[i];
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"COPY(Element_point_ranges_identifier).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* COPY(Element_point_ranges_identifier) */

int Element_point_make_top_level(
	struct Element_point_ranges_identifier *identifier,int *element_point_number)
/*******************************************************************************
LAST MODIFIED : 23 April 2001

DESCRIPTION :
If <identifier> does not already refer to a top_level_element - ie. element
and top_level_element are not the same, converts it to an EXACT_XI point that is
top_level. Assumes <identifier> has been validated.
==============================================================================*/
{
	FE_value element_to_top_level[9],exact_xi,
		face_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS],*source;
	int element_dimension, i, j, return_code, top_level_element_dimension;
	struct FE_element *top_level_element;

	ENTER(Element_point_make_top_level);
	if (identifier && element_point_number)
	{
		if (identifier->element != identifier->top_level_element)
		{
			if ((top_level_element=FE_element_get_top_level_element_conversion(
				identifier->element,identifier->top_level_element,
				(LIST_CONDITIONAL_FUNCTION(FE_element) *)NULL, (void *)NULL,
				/*face_number*/-1, element_to_top_level)) &&
				(top_level_element==identifier->top_level_element)&&
				(element_dimension=get_FE_element_dimension(identifier->element))&&
				FE_element_get_numbered_xi_point(identifier->element,
					identifier->xi_discretization_mode,
					identifier->number_in_xi, identifier->exact_xi,
					(Cmiss_field_cache_id)0,
					/*coordinate_field*/(struct Computed_field *)NULL,
					/*density_field*/(struct Computed_field *)NULL,
					*element_point_number, face_xi) &&
				(top_level_element_dimension=
					get_FE_element_dimension(identifier->top_level_element)))
			{
				identifier->element=top_level_element;
				identifier->xi_discretization_mode=XI_DISCRETIZATION_EXACT_XI;
				for (i=0;i<MAXIMUM_ELEMENT_XI_DIMENSIONS;i++)
				{
					identifier->number_in_xi[i]=1;
				}
				source=element_to_top_level;
				for (i=0;i<top_level_element_dimension;i++)
				{
					exact_xi = *source;
					source++;
					for (j=0;j<element_dimension;j++)
					{
						exact_xi += (*source) * face_xi[j];
						source++;
					}
					identifier->exact_xi[i]=exact_xi;
				}
				*element_point_number = 0;
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Element_point_make_top_level.  Could not convert point");
				return_code=0;
			}
		}
		else
		{
			/* already top_level */
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_make_top_level.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return(return_code);
} /* Element_point_make_top_level */

struct Element_point_ranges *CREATE(Element_point_ranges)(
	struct Element_point_ranges_identifier *identifier)
/*******************************************************************************
LAST MODIFIED : 8 June 2000

DESCRIPTION :
Creates an Element_point_ranges object that can store ranges of points in the
element:Xi_discretization_mode of the <identifier>.
==============================================================================*/
{
	struct Element_point_ranges *element_point_ranges;

	ENTER(CREATE(Element_point_ranges));
	element_point_ranges=(struct Element_point_ranges *)NULL;
	if (Element_point_ranges_identifier_is_valid(identifier))
	{
		if (ALLOCATE(element_point_ranges,struct Element_point_ranges,1)&&
			(element_point_ranges->ranges=CREATE(Multi_range)()))
		{
			/* ensure identifier points at id for indexed lists */
			element_point_ranges->identifier = &(element_point_ranges->id);
			COPY(Element_point_ranges_identifier)(
				element_point_ranges->identifier,identifier);
			/* struct Element_point_ranges ACCESSes the elements in the identifier */
			ACCESS(FE_element)(element_point_ranges->id.element);
			ACCESS(FE_element)(element_point_ranges->id.top_level_element);
			element_point_ranges->access_count=0;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"CREATE(Element_point_ranges).  Not enough memory");
			DEALLOCATE(element_point_ranges);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(Element_point_ranges).  Invalid identifier");
	}
	LEAVE;

	return (element_point_ranges);
} /* CREATE(Element_point_ranges) */

int DESTROY(Element_point_ranges)(
	struct Element_point_ranges **element_point_ranges_address)
/*******************************************************************************
LAST MODIFIED : 8 June 2000

DESCRIPTION :
Destroys the Element_point_ranges.
==============================================================================*/
{
	int return_code;
	struct Element_point_ranges *element_point_ranges;

	ENTER(DESTROY(Element_point_ranges));
	if (element_point_ranges_address&&
		(element_point_ranges= *element_point_ranges_address))
	{
		if (0==element_point_ranges->access_count)
		{
			DEACCESS(FE_element)(&(element_point_ranges->id.element));
			DEACCESS(FE_element)(&(element_point_ranges->id.top_level_element));
			DESTROY(Multi_range)(&(element_point_ranges->ranges));
			DEALLOCATE(*element_point_ranges_address);
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"DESTROY(Element_point_ranges).  Non-zero access count!");
			*element_point_ranges_address=(struct Element_point_ranges *)NULL;
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Element_point_ranges).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Element_point_ranges) */

DECLARE_OBJECT_FUNCTIONS(Element_point_ranges)
DECLARE_INDEXED_LIST_FUNCTIONS(Element_point_ranges)
DECLARE_FIND_BY_IDENTIFIER_IN_INDEXED_LIST_FUNCTION(Element_point_ranges, \
	identifier,struct Element_point_ranges_identifier *, \
	compare_Element_point_ranges_identifier)

int Element_point_ranges_get_identifier(
	struct Element_point_ranges *element_point_ranges,
	struct Element_point_ranges_identifier *identifier)
/*******************************************************************************
LAST MODIFIED : 8 June 2000

DESCRIPTION :
Puts the contents of the identifier for <element_point_ranges> in the
caller-supplied <identifier>.
==============================================================================*/
{
	int return_code;

	ENTER(Element_point_ranges_get_identifier);
	if (element_point_ranges&&identifier)
	{
		return_code=COPY(Element_point_ranges_identifier)(
			identifier,element_point_ranges->identifier);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_ranges_get_identifier.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Element_point_ranges_get_identifier */

int Element_point_ranges_add_range(
	struct Element_point_ranges *element_point_ranges,int start,int stop)
/*******************************************************************************
LAST MODIFIED : 23 April 2001

DESCRIPTION :
Adds the range from <start> to <stop> to the ranges in <element_point_ranges>.
==============================================================================*/
{
	int maximum_element_point_number,return_code;

	ENTER(Element_point_ranges_add_range);
	if (element_point_ranges)
	{
		/* check start/stop are within allowed ranges for identifier */
		FE_element_get_xi_points(element_point_ranges->id.element,
			element_point_ranges->id.xi_discretization_mode,
			element_point_ranges->id.number_in_xi,
			element_point_ranges->id.exact_xi,
			(Cmiss_field_cache_id)0,
			/*coordinate_field*/(struct Computed_field *)NULL,
			/*density_field*/(struct Computed_field *)NULL,
			&maximum_element_point_number, /*xi_points_address*/(FE_value_triple **)NULL);
		if ((0<=start)&&(start<maximum_element_point_number)&&
			(0<=stop)&&(stop<maximum_element_point_number))
		{
			return_code=
				Multi_range_add_range(element_point_ranges->ranges,start,stop);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Element_point_ranges_add_range.  Invalid range");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_ranges_add_range.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Element_point_ranges_add_range */

struct Multi_range *Element_point_ranges_get_ranges(
	struct Element_point_ranges *element_point_ranges)
/*******************************************************************************
LAST MODIFIED : 29 February 2000

DESCRIPTION :
Returns a pointer to the ranges in <element_point_ranges>. This should not be
modified in any way.
==============================================================================*/
{
	struct Multi_range *ranges;

	ENTER(Element_point_ranges_get_ranges);
	if (element_point_ranges)
	{
		ranges=element_point_ranges->ranges;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_ranges_get_ranges.  Invalid argument(s)");
		ranges=(struct Multi_range *)NULL;
	}
	LEAVE;

	return (ranges);
} /* Element_point_ranges_get_ranges */

int Element_point_ranges_has_ranges(
	struct Element_point_ranges *element_point_ranges)
/*******************************************************************************
LAST MODIFIED : 25 May 2000

DESCRIPTION :
Returns true if <element_point_ranges> has ranges, ie. is not empty.
==============================================================================*/
{
	int return_code;

	ENTER(Element_point_ranges_has_ranges);
	if (element_point_ranges)
	{
		return_code=
			(0<Multi_range_get_number_of_ranges(element_point_ranges->ranges));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_ranges_has_ranges.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Element_point_ranges_has_ranges */

int Element_point_ranges_add_to_list(
	struct Element_point_ranges *element_point_ranges,
	void *element_point_ranges_list_void)
/*******************************************************************************
LAST MODIFIED : 25 May 2000

DESCRIPTION :
Ensures the <element_point_ranges> are in <element_point_ranges_list>.
==============================================================================*/
{
	int i,number_of_ranges,return_code,start,stop;
	struct Element_point_ranges *existing_element_point_ranges,
		*new_element_point_ranges;
	struct LIST(Element_point_ranges) *element_point_ranges_list;

	ENTER(Element_point_ranges_add_to_list);
	if (element_point_ranges&&(element_point_ranges_list=
		(struct LIST(Element_point_ranges) *)element_point_ranges_list_void)&&
		(0<(number_of_ranges=
			Multi_range_get_number_of_ranges(element_point_ranges->ranges))))
	{
		existing_element_point_ranges=
			FIND_BY_IDENTIFIER_IN_LIST(Element_point_ranges,identifier)(
				element_point_ranges->identifier,element_point_ranges_list);
		if (existing_element_point_ranges)
		{
			return_code=1;
			for (i=0;(i<number_of_ranges)&&return_code;i++)
			{
				if (!(Multi_range_get_range(element_point_ranges->ranges,i,
					&start,&stop)&&
					Multi_range_add_range(existing_element_point_ranges->ranges,
						start,stop)))
				{
					display_message(ERROR_MESSAGE,
						"Element_point_ranges_add_to_list.  Could not add range");
					return_code=0;
				}
			}
		}
		else
		{
			if ((new_element_point_ranges=CREATE(Element_point_ranges)(
				element_point_ranges->identifier))&&
				Multi_range_copy(new_element_point_ranges->ranges,
					element_point_ranges->ranges)&&
				ADD_OBJECT_TO_LIST(Element_point_ranges)(
					new_element_point_ranges,element_point_ranges_list))
			{
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Element_point_ranges_add_to_list.  Could not add");
				if (new_element_point_ranges)
				{
					DESTROY(Element_point_ranges)(&new_element_point_ranges);
				}
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_ranges_add_to_list.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Element_point_ranges_add_to_list */

int Element_point_ranges_list_add_element_point(
	struct LIST(Element_point_ranges) *element_point_ranges_list,
	struct Element_point_ranges_identifier *element_point_ranges_identifier,
	int element_point_number)
/*******************************************************************************
LAST MODIFIED : 16 June 2000

DESCRIPTION :
Shortcut for ensuring the element point indicated by
<element_point_ranges_identifier> <element_point_number> is in the
<element_point_ranges_list>.
==============================================================================*/
{
	int return_code;
	struct Element_point_ranges *existing_element_point_ranges,
		*new_element_point_ranges;

	ENTER(Element_point_ranges_list_add_element_point);
	if (element_point_ranges_list&&
		Element_point_ranges_identifier_is_valid(element_point_ranges_identifier)&&
		Element_point_ranges_identifier_element_point_number_is_valid(
			element_point_ranges_identifier,element_point_number))
	{
		existing_element_point_ranges=
			FIND_BY_IDENTIFIER_IN_LIST(Element_point_ranges,identifier)(
				element_point_ranges_identifier,element_point_ranges_list);
		if (existing_element_point_ranges)
		{
			return_code=Multi_range_add_range(existing_element_point_ranges->ranges,
				element_point_number,element_point_number);
		}
		else
		{
			if ((new_element_point_ranges=CREATE(Element_point_ranges)(
				element_point_ranges_identifier))&&
				Multi_range_add_range(new_element_point_ranges->ranges,
					element_point_number,element_point_number)&&
				ADD_OBJECT_TO_LIST(Element_point_ranges)(
					new_element_point_ranges,element_point_ranges_list))
			{
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Element_point_ranges_list_add_element_point.  Could not add point");
				if (new_element_point_ranges)
				{
					DESTROY(Element_point_ranges)(&new_element_point_ranges);
				}
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_ranges_list_add_element_point.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Element_point_ranges_list_add_element_point */

int Element_point_ranges_remove_from_list(
	struct Element_point_ranges *element_point_ranges,
	void *element_point_ranges_list_void)
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
Ensures the <element_point_ranges> is not in <element_point_ranges_list>.
==============================================================================*/
{
	int i,number_of_ranges,return_code,start,stop;
	struct Element_point_ranges *existing_element_point_ranges;
	struct LIST(Element_point_ranges) *element_point_ranges_list;

	ENTER(Element_point_ranges_remove_from_list);
	if (element_point_ranges&&(element_point_ranges_list=
		(struct LIST(Element_point_ranges) *)element_point_ranges_list_void)&&
		(0<(number_of_ranges=
			Multi_range_get_number_of_ranges(element_point_ranges->ranges))))
	{
		existing_element_point_ranges=
			FIND_BY_IDENTIFIER_IN_LIST(Element_point_ranges,identifier)(
				element_point_ranges->identifier,element_point_ranges_list);
		if (existing_element_point_ranges)
		{
			/* handle case where object from list is being removed */
			if (existing_element_point_ranges == element_point_ranges)
			{
				return_code=REMOVE_OBJECT_FROM_LIST(Element_point_ranges)(
					existing_element_point_ranges,element_point_ranges_list);
			}
			else
			{
				return_code=1;
				for (i=0;(i<number_of_ranges)&&return_code;i++)
				{
					if (!(Multi_range_get_range(element_point_ranges->ranges,i,
						&start,&stop)&&
						Multi_range_remove_range(existing_element_point_ranges->ranges,
							start,stop)))
					{
						display_message(ERROR_MESSAGE,
							"Element_point_ranges_remove_from_list.  Could not remove range");
						return_code=0;
					}
				}
				/* remove existing_element_point_ranges if empty */
				if (0==Multi_range_get_number_of_ranges(
					existing_element_point_ranges->ranges))
				{
					REMOVE_OBJECT_FROM_LIST(Element_point_ranges)(
						existing_element_point_ranges,element_point_ranges_list);
					return_code=1;
				}
			}
		}
		else
		{
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_ranges_remove_from_list.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Element_point_ranges_remove_from_list */

int Element_point_ranges_toggle_in_list(
	struct Element_point_ranges *element_point_ranges,
	void *element_point_ranges_list_void)
/*******************************************************************************
LAST MODIFIED : 28 February 2000

DESCRIPTION :
Toggles the <element_point_ranges> in <element_point_ranges_list>.
==============================================================================*/
{
	int i,number_of_ranges,return_code,start,stop;
	struct Element_point_ranges *existing_element_point_ranges;
	struct LIST(Element_point_ranges) *element_point_ranges_list;

	ENTER(Element_point_ranges_toggle_in_list);
	if (element_point_ranges&&(element_point_ranges_list=
		(struct LIST(Element_point_ranges) *)element_point_ranges_list_void)&&
		(0<(number_of_ranges=
			Multi_range_get_number_of_ranges(element_point_ranges->ranges))))
	{
		existing_element_point_ranges=
			FIND_BY_IDENTIFIER_IN_LIST(Element_point_ranges,identifier)(
				element_point_ranges->identifier,element_point_ranges_list);
		if (existing_element_point_ranges)
		{
			return_code=1;
			for (i=0;(i<number_of_ranges)&&return_code;i++)
			{
				if (!(Multi_range_get_range(element_point_ranges->ranges,i,
					&start,&stop)&&
					Multi_range_toggle_range(existing_element_point_ranges->ranges,
						start,stop)))
				{
					display_message(ERROR_MESSAGE,
						"Element_point_ranges_toggle_in_list.  Could not toggle range");
					return_code=0;
				}
			}
		}
		else
		{
			if ((existing_element_point_ranges=CREATE(Element_point_ranges)(
				element_point_ranges->identifier))&&
				Multi_range_copy(existing_element_point_ranges->ranges,
					element_point_ranges->ranges)&&
				ADD_OBJECT_TO_LIST(Element_point_ranges)(
					existing_element_point_ranges,element_point_ranges_list))
			{
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Element_point_ranges_toggle_in_list.  Not enough memory");
				DESTROY(Element_point_ranges)(&existing_element_point_ranges);
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_ranges_toggle_in_list.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Element_point_ranges_toggle_in_list */

int Element_point_ranges_is_wholly_within_element_list_tree(
	struct Element_point_ranges *element_point_ranges, void *element_list_void)
/*******************************************************************************
LAST MODIFIED : 2 March 2001

DESCRIPTION :
Returns true if either the top_level_element in the <element_point_ranges>
identifier is in <element_list>, or if the element is wholly within the list
tree with FE_element_is_wholly_within_element_list_tree function. Used to check
if element or top_level_element used by element_point_ranges will be destroyed,
since faces and lines are destroyed with their parents if they are not also
faces or lines of other elements not being destroyed.
==============================================================================*/
{
	int return_code;
	struct LIST(FE_element) *element_list;

	ENTER(Element_point_ranges_is_wholly_within_element_list_tree);
	if (element_point_ranges &&
		(element_list = (struct LIST(FE_element) *)element_list_void))
	{
		return_code = IS_OBJECT_IN_LIST(FE_element)(
			element_point_ranges->id.top_level_element, element_list) ||
			FE_element_is_wholly_within_element_list_tree(
				element_point_ranges->id.element, element_list_void);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_ranges_is_wholly_within_element_list_tree.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Element_point_ranges_is_wholly_within_element_list_tree */

int Element_point_ranges_element_is_in_FE_region(
	struct Element_point_ranges *element_point_ranges, void *fe_region_void)
/*******************************************************************************
LAST MODIFIED : 19 March 2003

DESCRIPTION :
Returns true if the element for <element_point_ranges> is in <fe_region>.
==============================================================================*/
{
	int return_code;
	struct FE_region *fe_region;

	ENTER(Element_point_ranges_element_is_in_FE_region);
	if (element_point_ranges && (fe_region = (struct FE_region *)fe_region_void))
	{
		return_code = FE_region_contains_FE_element(fe_region,
			element_point_ranges->id.element);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_ranges_element_is_in_FE_region.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Element_point_ranges_element_is_in_FE_region */

struct Element_point_ranges *Element_point_ranges_from_grid_field_ranges(
	struct FE_element *element,struct FE_field *grid_field,
	struct Multi_range *ranges)
/*******************************************************************************
LAST MODIFIED : 19 March 2003

DESCRIPTION :
If <grid_field> is a single component grid-based field in <element>, creates and
returns an Element_point_ranges containing all the grid points at which the
value of <grid_field> is in the <ranges>.
No Element_point_ranges object is returned without error if:
- <grid_field> is not grid-based in <element>.
- No grid points in <element> have <grid_field> value in the given <ranges>.
==============================================================================*/
{
	int grid_value_in_range,i,number_of_grid_values,*values;
	struct CM_element_information element_identifier;
	struct Element_point_ranges *element_point_ranges;
	struct Element_point_ranges_identifier identifier;

	ENTER(Element_point_ranges_from_grid_field_ranges);
	element_point_ranges=(struct Element_point_ranges *)NULL;
	if (element && get_FE_element_identifier(element, &element_identifier) &&
		(CM_ELEMENT == element_identifier.type) && grid_field &&
		(1==get_FE_field_number_of_components(grid_field))&&
		(INT_VALUE==get_FE_field_value_type(grid_field))&&ranges)
	{
		if (FE_element_field_is_grid_based(element,grid_field))
		{
			if (get_FE_element_field_component_grid_int_values(element,
				grid_field,/*component_number*/0,&values))
			{
				number_of_grid_values = get_FE_element_field_component_number_of_grid_values(
					element, grid_field, /*component_number*/0);
				/* work out if any values are in the given ranges */
				grid_value_in_range=0;
				for (i=0;(i<number_of_grid_values)&&(!grid_value_in_range);i++)
				{
					grid_value_in_range=Multi_range_is_value_in_range(ranges,values[i]);
				}
				if (grid_value_in_range)
				{
					identifier.element=element;
					identifier.top_level_element=element;
					identifier.xi_discretization_mode=XI_DISCRETIZATION_CELL_CORNERS;
					get_FE_element_field_component_grid_map_number_in_xi(element,grid_field,
						/*component_number*/0, identifier.number_in_xi);
					/* set exact_xi to something reasonable, just in case it is used */
					for (i=0;i<MAXIMUM_ELEMENT_XI_DIMENSIONS;i++)
					{
						identifier.exact_xi[i]=0.5;
					}
					element_point_ranges=CREATE(Element_point_ranges)(&identifier);
					if (element_point_ranges)
					{
						for (i=0;i<number_of_grid_values;i++)
						{
							if (Multi_range_is_value_in_range(ranges,values[i]))
							{
								Element_point_ranges_add_range(element_point_ranges,i,i);
							}
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Element_point_ranges_from_grid_field_ranges.  "
							"Could not create Element_point_ranges");
					}
				}
				DEALLOCATE(values);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Element_point_ranges_from_grid_field_ranges.  "
					"Error reading grid field");
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_ranges_from_grid_field_ranges.  Invalid argument(s)");
	}
	LEAVE;

	return (element_point_ranges);
} /* Element_point_ranges_from_grid_field_ranges */

int FE_element_grid_to_Element_point_ranges_list(struct FE_element *element,
	void *grid_to_list_data_void)
/*******************************************************************************
LAST MODIFIED : 19 March 2003

DESCRIPTION :
Iterator function that gets an Element_point_ranges structure representing all
the grid_points in <element> with discretization of the single component
integer <grid_field>, for which the field value is in the given <ranges>.
Note that there may legitimately be none if <grid_field> is not grid-based in
<element> or the ranges do not intersect with the values in the field.
The structure is then added to the <element_point_ranges_list>.
select_data_void should point to a
struct FE_element_grid_to_Element_point_ranges_list_data.
Uses only top level elements, type CM_ELEMENT.
==============================================================================*/
{
	int return_code;
	struct CM_element_information element_identifier;
	struct Element_point_ranges *element_point_ranges;
	struct FE_element_grid_to_Element_point_ranges_list_data *grid_to_list_data;

	ENTER(FE_element_grid_to_Element_point_ranges_list);
	if (element&&(grid_to_list_data=
		(struct FE_element_grid_to_Element_point_ranges_list_data *)
		grid_to_list_data_void))
	{
		if (get_FE_element_identifier(element, &element_identifier) &&
			(CM_ELEMENT == element_identifier.type) &&
			(element_point_ranges=Element_point_ranges_from_grid_field_ranges(
				element,grid_to_list_data->grid_fe_field,
				grid_to_list_data->grid_value_ranges)))
		{
			return_code=Element_point_ranges_add_to_list(element_point_ranges,
				(void *)grid_to_list_data->element_point_ranges_list);
			DESTROY(Element_point_ranges)(&element_point_ranges);
		}
		else
		{
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_grid_to_Element_point_ranges_list.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* FE_element_grid_to_Element_point_ranges_list */

int Element_point_ranges_grid_to_multi_range(
	struct Element_point_ranges *element_point_ranges,
	void *grid_to_multi_range_data_void)
/*******************************************************************************
LAST MODIFIED : 19 March 2003

DESCRIPTION :
Last parameter is a struct Element_point_ranges_grid_to_multi_range_data.
If <grid_fe_field> is grid-based as in <element_point_ranges>, adds the values
for this field for points in the ranges to the <multi_range>.
If field and element_point_ranges not identically grid-based, clear
<all_points_native> flag.
==============================================================================*/
{
	int dimension,native,number_in_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS],i,
		number_of_grid_values,return_code,*values;
	struct CM_element_information element_identifier;
	struct Element_point_ranges_grid_to_multi_range_data
		*grid_to_multi_range_data;
	struct FE_element *element;
	struct FE_field *grid_fe_field;

	ENTER(Element_point_ranges_grid_to_multi_range);
	if (element_point_ranges&&(element=element_point_ranges->id.element)&&
		(dimension=get_FE_element_dimension(element))&&
		(grid_to_multi_range_data=
			(struct Element_point_ranges_grid_to_multi_range_data *)
			grid_to_multi_range_data_void)&&
		(grid_fe_field=grid_to_multi_range_data->grid_fe_field))
	{
		return_code=1;
		/* work out if element_point_ranges matches the native discretization of
			 grid_fe_field in element */
		native=0;
		if (get_FE_element_identifier(element, &element_identifier) &&
			(CM_ELEMENT == element_identifier.type) &&
			(XI_DISCRETIZATION_CELL_CORNERS ==
				element_point_ranges->id.xi_discretization_mode)&&
			FE_element_field_is_grid_based(element,grid_fe_field))
		{
			return_code=get_FE_element_field_component_grid_map_number_in_xi(element,
				grid_fe_field,/*component_number*/0,number_in_xi);
			if (return_code)
			{
				native=1;
				for (i=0;(i<dimension)&&native;i++)
				{
					if (number_in_xi[i] != element_point_ranges->id.number_in_xi[i])
					{
						native=0;
					}
				}
			}
		}
		if (native)
		{
			return_code=get_FE_element_field_component_grid_int_values(element,
				grid_fe_field,/*component_number*/0,&values);
			if (return_code)
			{
				number_of_grid_values = get_FE_element_field_component_number_of_grid_values(
					element, grid_fe_field, /*component_number*/0);
				for (i=0;(i<number_of_grid_values)&&return_code;i++)
				{
					if (Multi_range_is_value_in_range(element_point_ranges->ranges,i))
					{
						return_code=
							Multi_range_add_range(grid_to_multi_range_data->multi_range,
								values[i],values[i]);
					}
				}
				DEALLOCATE(values);
			}
		}
		else
		{
			/* clear flag to allow problem to be reported later */
			grid_to_multi_range_data->all_points_native=0;
		}
		if (!return_code)
		{
			display_message(ERROR_MESSAGE,
				"Element_point_ranges_grid_to_multi_range.  Failed");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_ranges_grid_to_multi_range.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Element_point_ranges_grid_to_multi_range */

int FE_element_grid_to_multi_range(struct FE_element *element,
	void *grid_to_multi_range_data_void)
/*******************************************************************************
LAST MODIFIED : 19 March 2003

DESCRIPTION :
Last parameter is a struct FE_element_grid_to_multi_range_data.
If <grid_fe_field> is grid-based as in <element>, adds all values for this field
in <element> to the <multi_range>.
==============================================================================*/
{
	int i,number_of_grid_values,return_code,*values;
	struct CM_element_information element_identifier;
	struct FE_element_grid_to_multi_range_data *grid_to_multi_range_data;
	struct FE_field *grid_fe_field;
	struct Multi_range *multi_range;

	ENTER(FE_element_grid_to_multi_range);
	if (element&&(grid_to_multi_range_data=
		(struct FE_element_grid_to_multi_range_data *)
		grid_to_multi_range_data_void)&&
		(grid_fe_field=grid_to_multi_range_data->grid_fe_field)&&
		(1==get_FE_field_number_of_components(grid_fe_field))&&
		(INT_VALUE==get_FE_field_value_type(grid_fe_field))&&
		(multi_range=grid_to_multi_range_data->multi_range))
	{
		return_code = 1;
		if (get_FE_element_identifier(element, &element_identifier) &&
			(CM_ELEMENT == element_identifier.type) &&
			FE_element_field_is_grid_based(element, grid_fe_field))
		{
			if (get_FE_element_field_component_grid_int_values(element,
				grid_fe_field,/*component_number*/0,&values))
			{
				number_of_grid_values = get_FE_element_field_component_number_of_grid_values(
					element, grid_fe_field, /*component_number*/0);
				for (i=0;(i<number_of_grid_values)&&return_code;i++)
				{
					return_code=Multi_range_add_range(multi_range,values[i],values[i]);
				}
				DEALLOCATE(values);
			}
			else
			{
				return_code=0;
			}
			if (!return_code)
			{
				display_message(ERROR_MESSAGE,
					"FE_element_grid_to_multi_range.  Failed");
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_grid_to_multi_range.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* FE_element_grid_to_multi_range */

static int Field_value_index_ranges_set_grid_values(
	struct Field_value_index_ranges *field_value_index_ranges,
	void *set_grid_values_data_void)
/*******************************************************************************
LAST MODIFIED : 28 October 2004

DESCRIPTION :
Last parameter is a struct Element_point_ranges_set_grid_values_data. Sets
grid values for the field components in the <field_value_index_ranges> at the
discretization of the grid-based field in element.
If field and element_point_ranges not identically grid-based, clear
<all_points_native> flag.
==============================================================================*/
{
	FE_value *values, xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	int grid_point_number, return_code = 0, start, stop;
	struct Computed_field *field;
	struct Element_point_ranges_identifier *destination_identifier, *source_identifier;
	struct Element_point_ranges_set_grid_values_data *set_grid_values_data;
	struct FE_element *destination_element, *source_element;

	ENTER(Field_value_index_ranges_set_grid_values);
	if (field_value_index_ranges&&
		(field=Field_value_index_ranges_get_field(field_value_index_ranges))&&
		/*(components=Field_value_index_ranges_get_ranges(field_value_index_ranges))&&
		 You used to be able to set just some components, but that requires evaluating
		the old values so you can set the new ones and I (Shane) am not sure it is 
		useful.*/
		(set_grid_values_data=(struct Element_point_ranges_set_grid_values_data *)
			set_grid_values_data_void)&&
		(source_identifier = set_grid_values_data->source_identifier) &&
		(source_element = source_identifier->element)&&
		(destination_identifier = set_grid_values_data->destination_identifier) &&
		(destination_element=set_grid_values_data->element)&&
		set_grid_values_data->destination_element_point_numbers)
	{
		int number_of_components = Cmiss_field_get_number_of_components(field);
		if (FE_element_get_numbered_xi_point(
				 source_element, source_identifier->xi_discretization_mode,
				 source_identifier->number_in_xi, source_identifier->exact_xi,
				 (Cmiss_field_cache_id)0,
				 /*coordinate_field*/(struct Computed_field *)NULL,
				 /*density_field*/(struct Computed_field *)NULL,
				 set_grid_values_data->source_element_point_number, xi)
			&& ALLOCATE(values, FE_value, number_of_components)
			&& Cmiss_field_cache_set_mesh_location(set_grid_values_data->field_cache,
				source_element, MAXIMUM_ELEMENT_XI_DIMENSIONS, xi)
			&& Cmiss_field_evaluate_real(field, set_grid_values_data->field_cache,
				number_of_components, values))
		{
			start = 0;
			stop = 0;
			while (Multi_range_get_next_start_value(
				set_grid_values_data->destination_element_point_numbers, start, &start) 
				&& Multi_range_get_next_stop_value(
				set_grid_values_data->destination_element_point_numbers, stop, &stop))
			{
				for (grid_point_number = start ; grid_point_number <= stop ; grid_point_number++)
				{
					set_grid_values_data->number_of_points++;
					if (FE_element_get_numbered_xi_point(
							 destination_element, destination_identifier->xi_discretization_mode,
							 destination_identifier->number_in_xi, destination_identifier->exact_xi,
							 (Cmiss_field_cache_id)0,
							 /*coordinate_field*/(struct Computed_field *)NULL,
							 /*density_field*/(struct Computed_field *)NULL,
							 grid_point_number, xi) &&
						Cmiss_field_cache_set_mesh_location(set_grid_values_data->field_cache,
							destination_element, MAXIMUM_ELEMENT_XI_DIMENSIONS, xi) &&
						Cmiss_field_assign_real(field, set_grid_values_data->field_cache, number_of_components, values))
					{
						set_grid_values_data->number_of_points_set++;
					}
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Field_value_index_ranges_set_grid_values.  "
				"Source element point is not a grid point");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Field_value_index_ranges_set_grid_values.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Field_value_index_ranges_set_grid_values */

int Element_point_ranges_set_grid_values(
	struct Element_point_ranges *element_point_ranges,
	void *set_grid_values_data_void)
/*******************************************************************************
LAST MODIFIED : 27 March 2003

DESCRIPTION :
Last parameter is a struct Element_point_ranges_set_grid_values_data. Sets the
listed field components in <element_point_ranges> to the values taken from
<source_identifier><element_point_number>..
==============================================================================*/
{
	int return_code;
	struct Element_point_ranges_identifier element_point_ranges_identifier;
	struct Element_point_ranges_set_grid_values_data *set_grid_values_data;
	struct FE_element *element;

	ENTER(Element_point_ranges_set_grid_values);
	if (element_point_ranges&&
		(set_grid_values_data=(struct Element_point_ranges_set_grid_values_data *)
			set_grid_values_data_void)&&
		set_grid_values_data->source_identifier&&
		set_grid_values_data->field_component_ranges_list&&
		set_grid_values_data->fe_region)
	{
		/* make local element_copy from that in element_point_ranges */
		if (Element_point_ranges_get_identifier(element_point_ranges,
			&element_point_ranges_identifier) &&
			(element = element_point_ranges_identifier.element))
		{
			FE_region_begin_change(set_grid_values_data->fe_region);
			/* access element_copy to be safe from ACCESS/DEACCESS cycles in
				 computed field evaluations */
			set_grid_values_data->element = ACCESS(FE_element)(element);
			/* pass element_point_ranges to compare discretizations */
			set_grid_values_data->destination_identifier=
				&element_point_ranges_identifier;
			set_grid_values_data->destination_element_point_numbers=
				Element_point_ranges_get_ranges(element_point_ranges);
			/* set values in the element */
			if (!(return_code = FOR_EACH_OBJECT_IN_LIST(Field_value_index_ranges)(
				Field_value_index_ranges_set_grid_values,set_grid_values_data_void,
				set_grid_values_data->field_component_ranges_list)))
			{
				display_message(ERROR_MESSAGE,
					"Element_point_ranges_set_grid_values.  Could not set values");
			}
			DEACCESS(FE_element)(&(set_grid_values_data->element));
			FE_region_end_change(set_grid_values_data->fe_region);
		}
		else
		{
			display_message(ERROR_MESSAGE,"Element_point_ranges_set_grid_values.  "
				"Invalid element");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_ranges_set_grid_values.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Element_point_ranges_set_grid_values */


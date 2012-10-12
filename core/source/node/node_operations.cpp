/*******************************************************************************
FILE : node_operations.c

LAST MODIFIED : 17 January 2003

DESCRIPTION :
FE_node functions that utilise non finite element data structures and therefore
cannot reside in finite element modules.
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
#include <math.h>

#include "api/cmiss_field_module.h"
#include "general/debug.h"
#include "node/node_operations.h"
#include "general/message.h"

/*
Global functions
----------------
*/

struct FE_node_fe_region_selection_ranges_condition_data
/*******************************************************************************
LAST MODIFIED : 15 May 2006

DESCRIPTION :
==============================================================================*/
{
	struct FE_region *fe_region;
	struct Multi_range *node_ranges;
	Cmiss_field_cache_id field_cache;
	struct Computed_field *group_field;
	struct Computed_field *conditional_field;
	struct LIST(FE_node) *node_list;
}; /* struct FE_node_fe_region_selection_ranges_condition_data */

static int FE_node_add_if_selection_ranges_condition(struct FE_node *node,
	void *data_void)
/*******************************************************************************
LAST MODIFIED : 15 May 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code, selected;
	struct FE_node_fe_region_selection_ranges_condition_data *data;

	ENTER(FE_node_fe_region_ranges_condition);
	if (node && 
		(data = (struct FE_node_fe_region_selection_ranges_condition_data *)data_void))
	{
		return_code = 1;
		selected = 1;
		if (selected && data->node_ranges)
		{
			selected = FE_node_is_in_Multi_range(node, data->node_ranges);
		}
		if (selected)
		{
			if (data->group_field || data->conditional_field)
			{
				Cmiss_field_cache_set_node(data->field_cache, node);
				if ((data->group_field && !Cmiss_field_evaluate_boolean(data->group_field, data->field_cache)) ||
					(data->conditional_field && !Cmiss_field_evaluate_boolean(data->conditional_field, data->field_cache)))
				{
					selected = 0;
				}
			}
		}
		if (selected)
		{
			return_code = ADD_OBJECT_TO_LIST(FE_node)(node, data->node_list);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_node_set_FE_node_field_info.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_node_set_FE_node_field_info */

struct FE_node_values_number
/*******************************************************************************
LAST MODIFIED : 22 December 2000

DESCRIPTION :
Data for changing node identifiers.
==============================================================================*/
{
	struct FE_node *node;
	int number_of_values;
	FE_value *values;
	int new_number;
};

static int compare_FE_node_values_number_values(
	const void *node_values1_void, const void *node_values2_void)
/*******************************************************************************
LAST MODIFIED : 22 December 2000

DESCRIPTION :
Compares the values in <node_values1> and <node_values2> from the last to the
first, returning -1 as soon as a value in <node_values1> is less than its
counterpart in <node_values2>, or 1 if greater. 0 is returned if all values
are identival. Used as a compare function for qsort.
==============================================================================*/
{
	int i, number_of_values, return_code;
	struct FE_node_values_number *node_values1, *node_values2;

	ENTER(compare_FE_node_values_number_values);
	return_code = 0;
	if ((node_values1 = (struct FE_node_values_number *)node_values1_void) &&
		(node_values2 = (struct FE_node_values_number *)node_values2_void) &&
		(0 < (number_of_values = node_values1->number_of_values)) &&
		(number_of_values == node_values2->number_of_values))
	{
		for (i = number_of_values - 1; (!return_code) && (0 <= i); i--)
		{
			if (node_values1->values[i] < node_values2->values[i])
			{
				return_code = -1;
			}
			else if (node_values1->values[i] > node_values2->values[i])
			{
				return_code = 1;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"compare_FE_node_values_number_values.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* compare_FE_node_values_number_values */

struct FE_node_and_values_to_array_data
{
	Cmiss_field_cache_id field_cache;
	struct FE_node_values_number *node_values;
	struct Computed_field *sort_by_field;
	int number_of_values;
}; /* FE_node_and_values_to_array_data */

static int FE_node_and_values_to_array(struct FE_node *node,
	void *array_data_void)
{
	int return_code;
	struct FE_node_and_values_to_array_data *array_data;

	ENTER(FE_node_and_values_to_array);
	if (node && (array_data =
		(struct FE_node_and_values_to_array_data *)array_data_void) &&
		array_data->node_values)
	{
		return_code = 1;
		Cmiss_field_cache_set_node(array_data->field_cache, node);
		array_data->node_values->node = node;
		if (array_data->sort_by_field)
		{
			if (!(array_data->node_values->values &&
				Cmiss_field_evaluate_real(array_data->sort_by_field, array_data->field_cache,
					array_data->number_of_values, array_data->node_values->values)))
			{
				display_message(ERROR_MESSAGE, "FE_node_and_values_to_array.  "
					"sort_by field could not be evaluated at node");
				return_code = 0;
			}
		}
		(array_data->node_values)++;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_node_and_values_to_array.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_node_and_values_to_array */

int FE_region_change_node_identifiers(struct FE_region *fe_region,
	int node_offset, struct Computed_field *sort_by_field, FE_value time)
/*******************************************************************************
LAST MODIFIED : 18 February 2003

DESCRIPTION :
Changes the identifiers of all nodes in <fe_region>.
If <sort_by_field> is NULL, adds <node_offset> to the identifiers.
If <sort_by_field> is specified, it is evaluated for all nodes
in <fe_region> and they are sorted by it - changing fastest with the first
component and keeping the current order where the field has the same values.
Checks for and fails if attempting to give any of the nodes in <fe_region> an
identifier already used by a node in the same master FE_region.
Calls to this function should be enclosed in FE_region_begin_change/end_change.
Note function avoids iterating through FE_region node lists as this is not
allowed during identifier changes.
==============================================================================*/
{
	int i, next_spare_node_number, number_of_nodes, number_of_values, return_code;
	struct FE_node *node_with_identifier;
	struct FE_node_and_values_to_array_data array_data;
	struct FE_node_values_number *node_values;
	struct FE_region *master_fe_region;

	ENTER(FE_region_change_node_identifiers);
	if (fe_region)
	{
		return_code = 1;
		number_of_nodes = FE_region_get_number_of_FE_nodes(fe_region);
		if (0 < number_of_nodes)
		{
			Cmiss_field_module_id field_module;
			Cmiss_field_cache_id field_cache;
			FE_region_get_ultimate_master_FE_region(fe_region, &master_fe_region);
			if (sort_by_field)
			{
				number_of_values =
					Computed_field_get_number_of_components(sort_by_field);
			}
			else
			{
				number_of_values = 0;
			}
			if (ALLOCATE(node_values, struct FE_node_values_number,
				number_of_nodes))
			{
				for (i = 0; i < number_of_nodes; i++)
				{
					node_values[i].number_of_values = number_of_values;
					node_values[i].values = (FE_value *)NULL;
				}
				if (sort_by_field)
				{
					for (i = 0; (i < number_of_nodes) && return_code; i++)
					{
						if (!ALLOCATE(node_values[i].values, FE_value, number_of_values))
						{
							display_message(ERROR_MESSAGE,
								"FE_region_change_node_identifiers.  "
								"Not enough memory");
							return_code = 0;
						}
					}
				}
				if (return_code)
				{
					field_module = Cmiss_region_get_field_module(FE_region_get_Cmiss_region(fe_region));
					field_cache = Cmiss_field_module_create_cache(field_module);
					Cmiss_field_cache_set_time(field_cache, time);
					/* make a linear array of the nodes in the group in current order */
					array_data.field_cache = field_cache;
					array_data.node_values = node_values;
					array_data.sort_by_field = sort_by_field;
					array_data.number_of_values = number_of_values;
					if (!FE_region_for_each_FE_node(fe_region,
						FE_node_and_values_to_array, (void *)&array_data))
					{
						display_message(ERROR_MESSAGE,
							"FE_region_change_node_identifiers.  "
							"Could not build node/field values array");
						return_code = 0;
					}
					Cmiss_field_cache_destroy(&field_cache);
					Cmiss_field_module_destroy(&field_module);
				}
				if (return_code)
				{
					if (sort_by_field)
					{
						/* sort by field values with higher components more significant */
						qsort(node_values, number_of_nodes,
							sizeof(struct FE_node_values_number),
							compare_FE_node_values_number_values);
						/* give the nodes sequential values starting at node_offset */
						for (i = 0; i < number_of_nodes; i++)
						{
							node_values[i].new_number = node_offset + i;
						}
					}
					else
					{
						/* offset node numbers by node_offset */
						for (i = 0; i < number_of_nodes; i++)
						{
							node_values[i].new_number =
								get_FE_node_identifier(node_values[i].node) + node_offset;
						}
					}
					/* check node numbers are positive and ascending */
					for (i = 0; (i < number_of_nodes) && return_code; i++)
					{
						if (0 >= node_values[i].new_number)
						{
							display_message(ERROR_MESSAGE,
								"FE_region_change_node_identifiers.  "
								"node_offset would give negative node numbers");
							return_code = 0;
						}
						else if ((0 < i) &&
							(node_values[i].new_number <= node_values[i - 1].new_number))
						{
							display_message(ERROR_MESSAGE,
								"FE_region_change_node_identifiers.  "
								"Node numbers are not strictly increasing");
							return_code = 0;
						}
					}
				}
				if (return_code)
				{
					/* check no new numbers are in use by nodes not in node_group */
					for (i = 0; (i < number_of_nodes) && return_code; i++)
					{
						if ((node_with_identifier = FE_region_get_FE_node_from_identifier(
							master_fe_region, node_values[i].new_number)) &&
							(!FE_region_contains_FE_node(fe_region, node_with_identifier)))
						{
							display_message(ERROR_MESSAGE,
								"FE_region_change_node_identifiers.  "
								"Node using new number exists in master region");
							return_code = 0;
						}
					}
				}
				if (return_code)
				{
					/* change identifiers */
					/* maintain next_spare_node_number to renumber nodes in same group
						 which already have the same number as the new_number */
					next_spare_node_number =
						node_values[number_of_nodes - 1].new_number + 1;
					for (i = 0; (i < number_of_nodes) && return_code; i++)
					{
						node_with_identifier = FE_region_get_FE_node_from_identifier(
							fe_region, node_values[i].new_number);
						/* only modify if node doesn't already have correct identifier */
						if (node_with_identifier != node_values[i].node)
						{
							if (node_with_identifier)
							{
								while ((struct FE_node *)NULL !=
									FE_region_get_FE_node_from_identifier(fe_region,
										next_spare_node_number))
								{
									next_spare_node_number++;
								}
								if (!FE_region_change_FE_node_identifier(master_fe_region,
									node_with_identifier, next_spare_node_number))
								{
									return_code = 0;
								}
							}
							if (!FE_region_change_FE_node_identifier(master_fe_region,
								node_values[i].node, node_values[i].new_number))
							{
								display_message(ERROR_MESSAGE,
									"FE_region_change_node_identifiers.  "
									"Could not change node identifier");
								return_code = 0;
							}
						}
					}
				}
				for (i = 0; i < number_of_nodes; i++)
				{
					if (node_values[i].values)
					{
						DEALLOCATE(node_values[i].values);
					}
				}
				DEALLOCATE(node_values);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"FE_region_change_node_identifiers.  Not enough memory");
				return_code = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_region_change_node_identifiers.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_region_change_node_identifiers */

struct LIST(FE_node) *
	FE_node_list_from_region_and_selection_group(
		struct Cmiss_region *region, struct Multi_range *node_ranges,
		struct Computed_field *group_field, struct Computed_field *conditional_field,
		FE_value time, int use_data)
{
	int i, node_number, nodes_in_region, nodes_in_ranges = 0,
		number_of_ranges = 0, return_code, start, stop;
	struct FE_node *node;
	struct FE_node_fe_region_selection_ranges_condition_data data;
	struct FE_region *fe_region = NULL;

	ENTER(FE_node_list_from_region_selection_ranges_condition);
	data.node_list = (struct LIST(FE_node) *)NULL;
	if (region)
	{
		fe_region = Cmiss_region_get_FE_region(region);
		if (use_data)
			fe_region=FE_region_get_data_FE_region(fe_region);
	}
	if (fe_region)
	{
		data.node_list = CREATE(LIST(FE_node))();
		if (NULL != data.node_list)
		{
			Cmiss_field_module_id field_module = Cmiss_region_get_field_module(region);
			data.field_cache = Cmiss_field_module_create_cache(field_module);
			Cmiss_field_cache_set_time(data.field_cache, time);
			nodes_in_region = FE_region_get_number_of_FE_nodes(fe_region);
			if (node_ranges)
			{
				nodes_in_ranges = Multi_range_get_total_number_in_ranges(node_ranges);
			}
			data.fe_region = fe_region;
			/* Seems odd to specify an empty node_ranges but I have
				maintained the previous behaviour */
			if (node_ranges &&
				(0 < (number_of_ranges = Multi_range_get_number_of_ranges(node_ranges))))
			{
				data.node_ranges = node_ranges;
			}
			else
			{
				data.node_ranges = (struct Multi_range *)NULL;
			}
			data.conditional_field = conditional_field;
			data.group_field = group_field;

			if (data.node_ranges
				&& (nodes_in_ranges < nodes_in_region))
			{
				return_code = 1;
				for (i = 0 ; i < number_of_ranges ; i++)
				{
					Multi_range_get_range(node_ranges, i, &start, &stop);
					for (node_number = start ; node_number <= stop ; node_number++)
					{
						node = FE_region_get_FE_node_from_identifier(fe_region, node_number);
						if (node != NULL)
						{
							int selected = 1;
							if (group_field || conditional_field)
							{
								Cmiss_field_cache_set_node(data.field_cache, node);
								if ((group_field && !Cmiss_field_evaluate_boolean(group_field, data.field_cache)) ||
									(conditional_field && !Cmiss_field_evaluate_boolean(conditional_field, data.field_cache)))
								{
									selected = 0;
								}
							}
							if (selected)
							{
								ADD_OBJECT_TO_LIST(FE_node)(node, data.node_list);
							}
						}
					}
				}
			}
			else
			{
				return_code =  FE_region_for_each_FE_node(fe_region,
					FE_node_add_if_selection_ranges_condition, (void *)&data);
			}
			if (!return_code)
			{
				display_message(ERROR_MESSAGE,
					"FE_node_list_from_region_and_selection_group.  "
					"Error building list");
				DESTROY(LIST(FE_node))(&data.node_list);
			}
			Cmiss_field_cache_destroy(&data.field_cache);
			Cmiss_field_module_destroy(&field_module);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"FE_node_list_from_region_and_selection_group.  "
				"Could not create list");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_node_list_from_region_and_selection_group.  "
			"Invalid argument(s)");
	}
	LEAVE;

	return (data.node_list);
} /* FE_node_list_from_region_and_selection_group */

struct LIST(FE_node) *
	FE_node_list_from_ranges(
		struct FE_region *fe_region, struct Multi_range *node_ranges,
		struct Computed_field *conditional_field, FE_value time)
{
	return FE_node_list_from_region_and_selection_group(
		FE_region_get_Cmiss_region(fe_region), node_ranges,
		/*group_field*/(Cmiss_field_id)0, conditional_field,
		time, FE_region_is_data_FE_region(fe_region));
}

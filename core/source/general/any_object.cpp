/*******************************************************************************
FILE : any_object.c

LAST MODIFIED : 2 October 2002

DESCRIPTION :
Definition of Any_object structure for storing any object which can be uniquely
identified by a pointer with strong typing. LIST(Any_object) is declared.

Internally, type is represented by a static type_string pointer, constructed
from the type name itself. A void pointer is used for the object reference.

Macro prototypes in any_object_prototype.h and definitions in
any_object_definition.h allow Any_objects to be established for a particular
structure and through that interface remain strongly typed. Interfaces allow
a LIST(Any_object) to be treated as if it is a list of just the object_type for
which the macros are declared.
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
#include "general/any_object.h"
#include "general/any_object_private.h"
#include "general/compare.h"
#include "general/debug.h"
#include "general/indexed_list_private.h"
#include "general/object.h"
#include "general/message.h"

/*
Global types
------------
*/

FULL_DECLARE_INDEXED_LIST_TYPE(Any_object);

/*
Module functions
----------------
*/

DECLARE_CREATE_INDEX_NODE_FUNCTION(Any_object)
DECLARE_DESTROY_INDEX_NODE_FUNCTION(Any_object)
DECLARE_DUPLICATE_INDEX_NODE_FUNCTION(Any_object)
DECLARE_FIND_LEAF_NODE_IN_INDEX_FUNCTION(Any_object, subobject, void *,
	compare_pointer)
DECLARE_REMOVE_OBJECT_FROM_INDEX_FUNCTION(Any_object, subobject,
	compare_pointer)
DECLARE_REMOVE_OBJECTS_FROM_INDEX_THAT_FUNCTION(Any_object)
DECLARE_ADD_INDEX_TO_NODE_PARENT_FUNCTION(Any_object, subobject,
	compare_pointer)
DECLARE_ADD_OBJECT_TO_INDEX_FUNCTION(Any_object, subobject, compare_pointer)
DECLARE_FIRST_OBJECT_IN_INDEX_THAT_FUNCTION(Any_object)
DECLARE_FOR_EACH_OBJECT_IN_INDEX_FUNCTION(Any_object)

/*
Global functions
----------------
*/

DECLARE_OBJECT_FUNCTIONS(Any_object)

DECLARE_CREATE_INDEXED_LIST_FUNCTION(Any_object)
DECLARE_DESTROY_INDEXED_LIST_FUNCTION(Any_object)
DECLARE_COPY_INDEXED_LIST_FUNCTION(Any_object)

PROTOTYPE_REMOVE_OBJECT_FROM_LIST_FUNCTION(Any_object)
/*******************************************************************************
LAST MODIFIED : 23 August 2000

DESCRIPTION :
Override REMOVE_OBJECT_FROM_LIST function since a different address could refer
to the same subobject. Hence, find by identifier and check type_string matches.
==============================================================================*/
{
	int return_code;
	struct Any_object *temp_any_object;

	ENTER(REMOVE_OBJECT_FROM_LIST(Any_object));
	if (object&&list)
	{
		if (NULL != (temp_any_object=FIND_BY_IDENTIFIER_IN_LIST(Any_object,subobject)(
					 object->subobject,list)))
		{
			if (object->type_string == temp_any_object->type_string)
			{
				if (REMOVE_OBJECT_FROM_INDEX(Any_object)(temp_any_object,
					&(list->index)))
				{
					(list->count)--;
					return_code=1;
				}
				else
				{
					display_message(ERROR_MESSAGE,"REMOVE_OBJECT_FROM_LIST(Any_object).  "
						"Could not remove from index");
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"REMOVE_OBJECT_FROM_LIST(Any_object).  "
					"Object of different type but same address in list");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"REMOVE_OBJECT_FROM_LIST(Any_object).  Object not in list");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"REMOVE_OBJECT_FROM_LIST(Any_object).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* REMOVE_OBJECT_FROM_LIST(Any_object) */

PROTOTYPE_REMOVE_OBJECTS_FROM_LIST_THAT_FUNCTION(Any_object)
/*******************************************************************************
LAST MODIFIED : 23 August 2000

DESCRIPTION :
Not implemented.
==============================================================================*/
{
	int return_code;

	ENTER(REMOVE_OBJECTS_FROM_LIST_THAT(Any_object));
	USE_PARAMETER(conditional);
	USE_PARAMETER(user_data);
	USE_PARAMETER(list);
	display_message(ERROR_MESSAGE,
		"REMOVE_OBJECTS_FROM_LIST_THAT(Any_object).  Not implemented");
	return_code=0;
	LEAVE;

	return (return_code);
} /* REMOVE_OBJECTS_FROM_LIST_THAT(Any_object) */

DECLARE_REMOVE_ALL_OBJECTS_FROM_INDEXED_LIST_FUNCTION(Any_object)

PROTOTYPE_ADD_OBJECT_TO_LIST_FUNCTION(Any_object)
/*******************************************************************************
LAST MODIFIED : 23 August 2000

DESCRIPTION :
Override ADD_OBJECT_TO_LIST function since a different address could refer
to the same subobject. Hence, find by identifier and check type_string matches.
==============================================================================*/
{
	int return_code;
	struct Any_object *temp_any_object;
	struct INDEX_NODE(Any_object) *node;

	ENTER(ADD_OBJECT_TO_LIST(Any_object));
	if (object&&list)
	{
		if (NULL != (temp_any_object=FIND_BY_IDENTIFIER_IN_LIST(Any_object,subobject)(
					 object->subobject,list)))
		{
			if (object->type_string == temp_any_object->type_string)
			{
				display_message(ERROR_MESSAGE,
					"ADD_OBJECT_TO_LIST(Any_object).  Object is already in list");
				/* should this be an error? */
			}
			else
			{
				display_message(ERROR_MESSAGE,"ADD_OBJECT_TO_LIST(Any_object).  "
					"Object of different type but same address in list");
			}
			return_code=0;
		}
		else
		{
			if (list->index)
			{
				/* list is not empty */
				if (ADD_OBJECT_TO_INDEX(Any_object)(object,&(list->index)))
				{
					(list->count)++;
					return_code=1;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"ADD_OBJECT_TO_LIST(Any_object).  Could not add to index");
					return_code=0;
				}
			}
			else
			{
				/* list is empty */
				/* create the root node (a leaf node) */
				if (NULL != (node=CREATE_INDEX_NODE(Any_object)(1)))
				{
					list->index=node;
					list->count=1;
					node->number_of_indices=1;
					(node->indices)[0]=ACCESS(Any_object)(object);
					return_code=1;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"ADD_OBJECT_TO_LIST(Any_object).  Could not create index");
					return_code=0;
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"ADD_OBJECT_TO_LIST(Any_object).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* ADD_OBJECT_TO_LIST(Any_object) */

DECLARE_NUMBER_IN_INDEXED_LIST_FUNCTION(Any_object)

PROTOTYPE_IS_OBJECT_IN_LIST_FUNCTION(Any_object)
/*******************************************************************************
LAST MODIFIED : 23 August 2000

DESCRIPTION :
Override IS_OBJECT_IN_LIST function since a different address could refer to
the same subobject. Hence, find by identifier and check type_string matches.
==============================================================================*/
{
	int return_code;
	struct Any_object *temp_any_object;

	ENTER(IS_OBJECT_IN_LIST(Any_object));
	if (object&&list)
	{
		if (NULL != (temp_any_object=FIND_BY_IDENTIFIER_IN_LIST(Any_object,subobject)(
					 object->subobject,list)))
		{
			if (object->type_string == temp_any_object->type_string)
			{
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,"IS_OBJECT_IN_LIST(Any_object).  "
					"Object of different type but same address in list");
				return_code=0;
			}
		}
		else
		{
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"IS_OBJECT_IN_LIST(Any_object).  Invalid argument");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* IS_OBJECT_IN_LIST(Any_object) */

DECLARE_FIRST_OBJECT_IN_INDEXED_LIST_THAT_FUNCTION(Any_object)
DECLARE_FOR_EACH_OBJECT_IN_INDEXED_LIST_FUNCTION(Any_object)

DECLARE_FIND_BY_IDENTIFIER_IN_INDEXED_LIST_FUNCTION(Any_object, \
	subobject,void *,compare_pointer)

struct Any_object *CREATE(Any_object)(char *type_string,void *subobject)
/*******************************************************************************
LAST MODIFIED : 2 October 2002

DESCRIPTION :
Creates an Any_object which uses the static <type_string> to identify the type
of the <subobject>.
This function is private to objects using Any_object interface.
==============================================================================*/
{
	struct Any_object *any_object;

	ENTER(CREATE(Any_object));
	if (type_string && subobject)
	{
		if (ALLOCATE(any_object, struct Any_object, 1))
		{
			any_object->type_string = type_string;
			any_object->subobject = subobject;
			any_object->any_object_cleanup_function =
				(Any_object_cleanup_function)NULL;
			any_object->access_count = 0;
		}
		else
		{
			display_message(ERROR_MESSAGE,"CREATE(Any_object).  Not enough memory");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"CREATE(Any_object).  Invalid argument(s)");
		any_object=(struct Any_object *)NULL;
	}
	LEAVE;

	return (any_object);
} /* CREATE(Any_object) */

int DESTROY(Any_object)(struct Any_object **any_object_address)
/*******************************************************************************
LAST MODIFIED : 2 October 2002

DESCRIPTION :
Destroys the Any_object.
==============================================================================*/
{
	int return_code;
	struct Any_object *any_object;

	ENTER(DESTROY(Any_object));
	if (any_object_address && (NULL != (any_object = *any_object_address)))
	{
		/* call cleanup function for subobject if provided */
		if (any_object->any_object_cleanup_function)
		{
			(any_object->any_object_cleanup_function)(any_object->subobject);
		}
		DEALLOCATE(*any_object_address);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Any_object).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Any_object) */

int Any_object_set_cleanup_function(struct Any_object *any_object,
	Any_object_cleanup_function any_object_cleanup_function)
/*******************************************************************************
LAST MODIFIED : 2 October 2002

DESCRIPTION :
Adds function to <any_object> which will be called when it is destroyed. The
single argument to this function is a void pointer to the subobject in the
Any_object, NOT the address of a pointer to it.
This function is private to objects using Any_object interface.
==============================================================================*/
{
	int return_code;

	ENTER(Any_object_set_cleanup_function);
	if (any_object && any_object_cleanup_function)
	{
		any_object->any_object_cleanup_function = any_object_cleanup_function;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Any_object_set_cleanup_function.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Any_object_set_cleanup_function */

int ensure_Any_object_is_in_list(struct Any_object *any_object,
	void *any_object_list_void)
/*******************************************************************************
LAST MODIFIED : 23 August 2000

DESCRIPTION :
Iterator function for adding <any_object> to <any_object_list> if not currently
in it.
==============================================================================*/
{
	int return_code;
	struct LIST(Any_object) *any_object_list;

	ENTER(ensure_Any_object_is_in_list);
	if (any_object&&
		(NULL != (any_object_list=(struct LIST(Any_object) *)any_object_list_void)))
	{
		if (IS_OBJECT_IN_LIST(Any_object)(any_object,any_object_list))
		{
			return_code=1;
		}
		else
		{
			return_code=ADD_OBJECT_TO_LIST(Any_object)(any_object,any_object_list);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"ensure_Any_object_is_in_list.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* ensure_Any_object_is_in_list */

int ensure_Any_object_is_not_in_list(struct Any_object *any_object,
	void *any_object_list_void)
/*******************************************************************************
LAST MODIFIED : 23 August 2000

DESCRIPTION :
Iterator function for removing <any_object> from <any_object_list> if currently
in it.
==============================================================================*/
{
	int return_code;
	struct LIST(Any_object) *any_object_list;

	ENTER(ensure_Any_object_is_not_in_list);
	if (any_object&&
		(NULL != (any_object_list=(struct LIST(Any_object) *)any_object_list_void)))
	{
		if (IS_OBJECT_IN_LIST(Any_object)(any_object,any_object_list))
		{
			return_code=REMOVE_OBJECT_FROM_LIST(Any_object)(any_object,
				any_object_list);
		}
		else
		{
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"ensure_Any_object_is_not_in_list.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* ensure_Any_object_is_not_in_list */

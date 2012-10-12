/*******************************************************************************
FILE : time.c

LAST MODIFIED : 25 November 1999

DESCRIPTION :
This provides an object which supplies a concept of time to Cmgui
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
#include <math.h>
#include <stdio.h>

#include "general/debug.h"
#include "general/object.h"
#include "general/message.h"
#include "time/time.h"
#include "time/time_keeper.h"

enum Time_object_type
{
	TIME_OBJECT_REGULAR,
	TIME_OBJECT_NEXT_TIME_FUNCTION
};

struct Time_object_callback_data
{
	Time_object_callback callback;
	void *callback_user_data;

	struct Time_object_callback_data *next;
};

struct Time_object
{
	char *name;
	double current_time;
	double update_frequency;
	double time_offset;
	enum Time_object_type type;
	struct Time_object_callback_data *callback_list;
	struct Time_keeper *time_keeper;
	Time_object_next_time_function next_time_function;
	void *next_time_user_data;

	int access_count;
};

int DESTROY(Time_object)(struct Time_object **time)
/*******************************************************************************
LAST MODIFIED : 29 September 1998

DESCRIPTION :
Destroys a Time_object object
==============================================================================*/
{
	int return_code;
	struct Time_object_callback_data *callback_data, *next;

	ENTER(DESTROY(Time_object));

	if (time && *time)
	{
		return_code=1;

		if((*time)->time_keeper)
		{
			Time_keeper_remove_time_object((*time)->time_keeper, *time);
		}

		callback_data = (*time)->callback_list;
		while(callback_data)
		{
			next = callback_data->next;
			DEALLOCATE(callback_data);
			callback_data = next;
		}

		DEALLOCATE((*time)->name);
		DEALLOCATE(*time);
		*time = (struct Time_object *)NULL;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Time_object).  Missing time object");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Time_object) */

DECLARE_OBJECT_FUNCTIONS(Time_object)
DECLARE_DEFAULT_GET_OBJECT_NAME_FUNCTION(Time_object)

struct Time_object *CREATE(Time_object)(void)
/*******************************************************************************
LAST MODIFIED : 29 September 1998

DESCRIPTION :
==============================================================================*/
{
	struct Time_object *time;

	ENTER(CREATE(Time_object));

	if (ALLOCATE(time, struct Time_object, 1))
	{
		time->name = (char *)NULL;
		time->current_time = 0.0;
		time->time_keeper = (struct Time_keeper *)NULL;
		time->callback_list = (struct Time_object_callback_data *)NULL;
		time->update_frequency = 10.0;
		time->time_offset = 0.0;
		/* after setting the time notifier type in the type specific constructor,
			 it should not be allowed to change it later. This can be enforced when 
			 other types are added. */
		time->type = TIME_OBJECT_REGULAR;
		time->next_time_function = (Time_object_next_time_function)NULL;
		time->next_time_user_data = NULL;
		time->access_count = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"CREATE(Time_object). Unable to allocate buffer structure");
		time = (struct Time_object *)NULL;
	}

	LEAVE;

	return (time);
} /* CREATE(Time_object) */

struct Time_object *Time_object_create_regular(double update_frequency, 
	double time_offset)
{
	struct Time_object *time;

	ENTER(Time_object_create_regular);
	if (NULL != (time = CREATE(Time_object)()))
	{
		time->update_frequency = update_frequency;
		time->time_offset = time_offset;
		time->type = TIME_OBJECT_REGULAR;
	}
	else
	{
		time = (struct Time_object *)NULL;
	}
	LEAVE;
	
	return (time);
}

int Time_object_set_name(struct Time_object *time, const char *name)
{
	int return_code;
	char *temp_name;
	
	ENTER(Time_object_set_name);
	if(time && name)
	{
		if (REALLOCATE(temp_name, time->name, char, strlen(name) + 1))
		{
			time->name = temp_name;
			strcpy(time->name, name);
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,"Time_object_set_name. "
				"Unable to reallocate memory for name.");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Time_object_set_name. Invalid arguments");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Time_object_set_name */

double Time_object_get_current_time(struct Time_object *time)
/*******************************************************************************
LAST MODIFIED : 29 September 1998

DESCRIPTION :
==============================================================================*/
{
	double return_code;

	ENTER(Time_object_get_current_time);
	if (time)
	{
		return_code = time->current_time;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Time_object_get_current_time. Invalid time object");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Time_object_get_current_time */

int Time_object_check_valid_callback_time(struct Time_object *time_object,
	double time,enum Time_keeper_play_direction play_direction)
{
	int return_code = 0;

	ENTER(Time_object_check_valid_callback_time);
	if (time_object)
	{
		switch(time_object->type)
		{
			default:
			{
				switch(play_direction)
				{
					case TIME_KEEPER_PLAY_FORWARD:
					{
						/* minus the current time by a faction of the update frequency
							 then workout the closest callback time */
						if (time == (1.0 + 
								floor((time - 1.0 / time_object->update_frequency) *
								time_object->update_frequency)) /
								time_object->update_frequency)
						{
							return_code = 1;
						}
					} break;
					case TIME_KEEPER_PLAY_BACKWARD:
					{
						/* add the current time by a faction of the update frequency
							 then workout the closest callback time */
						if (time == (1.0 + 
								ceil((time + 1.0 / time_object->update_frequency) *
								time_object->update_frequency)) /
								time_object->update_frequency)
						{
							return_code = 1;
						}
					} break;
					default:
					{
						display_message(ERROR_MESSAGE,
							"Time_object_check_valid_callback_time.  Unknown play direction");
						return_code=0;
					} break;
				}
			} break;
			case TIME_OBJECT_NEXT_TIME_FUNCTION:
			{
				if(time_object->next_time_function)
				{
					if (time == (time_object->next_time_function)(time, play_direction,
							time_object->next_time_user_data))
					{
						return_code = 1;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Time_object_get_next_callback_time.  type TIME_OBJECT_NEXT_TIME_FUNCTION but no function");
					return_code=0;
				}
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Time_object_check_valid_callback_time.  Invalid time object");
		return_code=0;
	}
	LEAVE;

	return (return_code);
}
double Time_object_get_next_callback_time(struct Time_object *time,
	double time_after, enum Time_keeper_play_direction play_direction)
/*******************************************************************************
LAST MODIFIED : 9 December 1998

DESCRIPTION :
==============================================================================*/
{
	double return_code;

	ENTER(Time_object_get_next_callback_time);

	if (time)
	{
		switch(time->type)
		{
			default:
			{
				switch(play_direction)
				{
					case TIME_KEEPER_PLAY_FORWARD:
					{
						return_code = time->time_offset + (1.0 +
							floor((time_after - time->time_offset) * time->update_frequency)) / time->update_frequency;
					} break;
					case TIME_KEEPER_PLAY_BACKWARD:
					{
						return_code = time->time_offset + (-1.0 +
							ceil((time_after -  time->time_offset) * time->update_frequency)) / time->update_frequency;
					} break;
					default:
					{
						display_message(ERROR_MESSAGE,
							"Time_object_get_next_callback_time.  Unknown play direction");
						return_code=0;
					} break;
				}
			} break;
			case TIME_OBJECT_NEXT_TIME_FUNCTION:
			{
				if(time->next_time_function)
				{
					return_code = (time->next_time_function)(time_after, play_direction,
						time->next_time_user_data);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Time_object_get_next_callback_time.  type TIME_OBJECT_NEXT_TIME_FUNCTION but no function");
					return_code=0;
				}
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Time_object_get_next_callback_time.  Invalid time object");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Time_object_get_next_callback_time */

int Time_object_set_current_time_privileged(struct Time_object *time,
	double new_time)
/*******************************************************************************
LAST MODIFIED : 17 January 2002

DESCRIPTION :
This routine allows the timekeeper to explicitly set the time.
Separated Time_object_notify_clients_privileged so that all the clients
can be updated with the new time before any of them call back to their clients.
Users of a time object that is controlled by a timekeeper should set the time 
through the timekeeper.
==============================================================================*/
{
	int return_code;

	ENTER(Time_object_set_current_time_privileged);

	if (time)
	{
		time->current_time = new_time;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Time_object_set_current_time_privileged. Invalid time object");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Time_object_set_current_time_privileged */

int Time_object_notify_clients_privileged(struct Time_object *time)
/*******************************************************************************
LAST MODIFIED : 17 January 2002

DESCRIPTION :
This routine allows the timekeeper to tell the time_object to notify its clients.
Separated off from Time_object_set_current_time_privileged so that all the clients
can be updated with the new time before any of them call back to their clients.
Users of a time object that is controlled by a timekeeper should set the time 
through the timekeeper.
==============================================================================*/
{
	int return_code;
	struct Time_object_callback_data *callback_data;

	ENTER(Time_object_set_current_time_privileged);

	if (time)
	{
		callback_data = time->callback_list;
		while(callback_data)
		{
			(callback_data->callback)(time, time->current_time,
				callback_data->callback_user_data);
			callback_data = callback_data->next;
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Time_object_set_current_time_privileged. Invalid time object");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Time_object_set_current_time_privileged */

int Time_object_regular_set_frequency(struct Time_object *time, double frequency)
/*******************************************************************************
LAST MODIFIED : 25 November 1999

DESCRIPTION :
This controls the rate per second which the time depedent object is called back
when in play mode.
==============================================================================*/
{
	int return_code;

	ENTER(Time_object_regular_set_update_frequency);
	if (time)
	{
		if (time->type == TIME_OBJECT_REGULAR)
		{
			time->update_frequency = frequency;
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Time_object_regular_set_frequency. Change of frequency is not allowed"
				"for this time object/notifier type");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Time_object_regular_set_frequency. Invalid time object");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Time_object_set_update_frequency */

int Time_object_regular_set_offset(struct Time_object *time,double time_offset)
{
	int return_code;

	ENTER(Time_object_regular_set_offset);
	if (time)
	{
		if (time->type == TIME_OBJECT_REGULAR)
		{
			time->time_offset = time_offset;
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Time_object_regular_set_offset. Change of time offset is not allowed"
				"for this time object/notifier type");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Time_object_set_offset. Invalid time object");
		return_code=0;
	}
	LEAVE;

	return (return_code);
}

int Time_object_set_next_time_function(struct Time_object *time,
	Time_object_next_time_function next_time_function, void *user_data)
/*******************************************************************************
LAST MODIFIED : 25 November 1999

DESCRIPTION :
By setting this function when the time_keeper requests the update_frequency is
not used.  Instead the next_time_function is called to evaluate the next valid
time.
==============================================================================*/
{
	int return_code;

	ENTER(Time_object_set_next_time_function);
	if (time && next_time_function)
	{
		time->type = TIME_OBJECT_NEXT_TIME_FUNCTION;
		time->next_time_function = next_time_function;
		time->next_time_user_data = user_data;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Time_object_set_next_time_function. Invalid arguments");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Time_object_set_next_time_function */

struct Time_keeper *Time_object_get_time_keeper(struct Time_object *time)
/*******************************************************************************
LAST MODIFIED : 29 September 1998

DESCRIPTION :
==============================================================================*/
{
	struct Time_keeper *return_code;

	ENTER(Time_object_get_time_keeper);

	if (time)
	{
		return_code = time->time_keeper;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Time_object_get_time_keeper. Invalid time object");
		return_code=(struct Time_keeper *)NULL;
	}
	LEAVE;

	return (return_code);
} /* Time_object_get_time_keeper */

int Time_object_set_time_keeper(struct Time_object *time,
	struct Time_keeper *time_keeper)
/*******************************************************************************
LAST MODIFIED : 29 September 1998

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Time_object_set_time_keeper);

	if (time)
	{
		return_code = REACCESS(Time_keeper)(&(time->time_keeper), time_keeper);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Time_object_set_time_keeper. Invalid time object");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Time_object_set_time_keeper */

int Time_object_add_callback(struct Time_object *time,
	Time_object_callback callback, void *user_data)
/*******************************************************************************
LAST MODIFIED : 29 September 1998

DESCRIPTION :
Adds a callback routine which is called whenever the current time is changed.
==============================================================================*/
{
	int return_code = 0;
	struct Time_object_callback_data *callback_data, *previous;

	ENTER(Time_object_add_callback);

	if (time && callback)
	{
		if(ALLOCATE(callback_data, struct Time_object_callback_data, 1))
		{
			callback_data->callback = callback;
			callback_data->callback_user_data = user_data;
			callback_data->next = (struct Time_object_callback_data *)NULL;
			if(time->callback_list)
			{
				previous = time->callback_list;
				while(previous->next)
				{
					previous = previous->next;
				}
				previous->next = callback_data;
			}
			else
			{
				time->callback_list = callback_data;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Time_object_add_callback.  Unable to allocate callback data structure");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Time_object_add_callback.  Missing time object or callback");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Time_object_add_callback */

int Time_object_remove_callback(struct Time_object *time,
	Time_object_callback callback, void *user_data)
/*******************************************************************************
LAST MODIFIED : 29 September 1998

DESCRIPTION :
Removes a callback which was added previously
==============================================================================*/
{
	int return_code;
	struct Time_object_callback_data *callback_data, *previous;

	ENTER(Time_object_remove_callback);

	if (time && callback && time->callback_list)
	{
		callback_data = time->callback_list;
		if((callback_data->callback == callback)
			&& (callback_data->callback_user_data == user_data))
		{
			time->callback_list = callback_data->next;
			DEALLOCATE(callback_data);
			return_code = 1;
		}
		else
		{
			return_code = 0;
			while(!return_code && callback_data->next)
			{
				previous = callback_data;
				callback_data = callback_data->next;
				if((callback_data->callback == callback)
					&& (callback_data->callback_user_data == user_data))
				{
					previous->next = callback_data->next;
					DEALLOCATE(callback_data);
					return_code = 1;		
				}
			}
			if (!return_code)
			{
				display_message(ERROR_MESSAGE,
					"Time_object_remove_callback.  Unable to find callback and user_data specified");
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Time_object_remove_callback.  Missing time, callback or callback list");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Time_object_remove_callback */

Cmiss_time_notifier_id Cmiss_time_notifier_access(Cmiss_time_notifier_id time_notifier)
{
	Cmiss_time_notifier_id local_time_notifier;

	if (time_notifier)
	{
		local_time_notifier = ACCESS(Time_object)(time_notifier);
	}
	else
	{
		local_time_notifier = 0;
	}

	return local_time_notifier;
}

int Cmiss_time_notifier_destroy(Cmiss_time_notifier_id *time_notifier_address)
{
	int return_code;

	ENTER(Cmiss_time_notifier_destroy);
	if (time_notifier_address && *time_notifier_address)
	{
		return_code = DEACCESS(Time_object)(time_notifier_address);
	}
	else
	{
		return_code = 0;
	}
	LEAVE;

	return return_code;
}

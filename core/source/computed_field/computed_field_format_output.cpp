/*******************************************************************************
FILE : computed_field_format_output.c

LAST MODIFIED : 14 December 2010

DESCRIPTION :
Implements a field which formats numeric values as a string.
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
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_private.hpp"
#include "general/debug.h"
#include "general/mystring.h"
#include "general/message.h"
#include "computed_field/computed_field_format_output.h"
#include "computed_field/computed_field_set.h"

namespace {

const char computed_field_format_output_type_string[] = "format_output";

class Computed_field_format_output : public Computed_field_core
{
public:
	char *format_string;
	/* Estimate from the format_string and number of components
	 * a sufficient output string allocation. */
	int output_allocation_size;

	Computed_field_format_output(int number_of_components, char* format_string_in) :
		Computed_field_core()
	{
		format_string = duplicate_string(format_string_in);
		output_allocation_size = number_of_components * 30 + strlen(format_string_in);
	}

	~Computed_field_format_output()
	{
		if (format_string)
			DEALLOCATE(format_string);
	}

private:
	Computed_field_core *copy()
	{
		return new Computed_field_format_output(field->number_of_components, format_string);
	}

	const char *get_type_string()
	{
		return(computed_field_format_output_type_string);
	}

	int compare(Computed_field_core* other_field)
	{
		Computed_field_format_output *other_format_output;
		if ((other_format_output = dynamic_cast<Computed_field_format_output*>(other_field)))
		{
			if (0 == strcmp(format_string, other_format_output->format_string))
				return 1;
			else
				return 0;
		}
		else
			return 0;
	}

	virtual FieldValueCache *createValueCache(Cmiss_field_cache& /*parentCache*/)
	{
		return new StringFieldValueCache();
	}

	int evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache);

	int list();

	char* get_command_string();

	/*****************************************************************************//**
	 * Computed_field_format_output never has numerical components
	 */
	int has_numerical_components()
	{
		return 0;
	}

	virtual Cmiss_field_value_type get_value_type() const
	{
		return CMISS_FIELD_VALUE_TYPE_STRING;
	}

};

int Computed_field_format_output::evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache)
{
	StringFieldValueCache &valueCache = StringFieldValueCache::cast(inValueCache);
	RealFieldValueCache *sourceCache = RealFieldValueCache::cast(getSourceField(0)->evaluate(cache));
	if (sourceCache)
	{
		/* 2. Write out the source field values using the format_string */
		/* Allocate a generous string.
		 */
		if (valueCache.stringValue)
			DEALLOCATE(valueCache.stringValue);
		ALLOCATE(valueCache.stringValue, char, output_allocation_size);
#if defined (_MSC_VER)
		/* If the MSVC _snprintf overflows then it won't be null terminated so ensure this 0. */
		valueCache.stringValue[output_allocation_size-1] = 0;
#endif // defined (_MSC_VER)
		switch (field->number_of_components)
		{
			case 1:
			{
				snprintf(valueCache.stringValue, output_allocation_size-1, format_string,
					sourceCache->values[0]);
			} break;
			case 2:
			{
				snprintf(valueCache.stringValue, output_allocation_size-1, format_string,
					sourceCache->values[0], sourceCache->values[1]);
			} break;
			case 3:
			{
				snprintf(valueCache.stringValue, output_allocation_size-1, format_string,
					sourceCache->values[0], sourceCache->values[1], sourceCache->values[2]);
			} break;
			case 4:
			{
				snprintf(valueCache.stringValue, output_allocation_size-1, format_string,
					sourceCache->values[0], sourceCache->values[1], sourceCache->values[2], sourceCache->values[3]);
			} break;
			default:
				return 0;
		}
		return 1;
	}
	return 0;
}

int Computed_field_format_output::list()
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_format_output);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,
			"    format_string : \"%s\"\n", format_string);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_format_output.  Invalid field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_format_output */

char *Computed_field_format_output::get_command_string()
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string;
	int error;

	ENTER(Computed_field_format_output::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string, " format_string", &error);
		append_string(&command_string, "\"", &error);
		append_string(&command_string,
			format_string, &error);
		append_string(&command_string, "\"", &error);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_format_output::get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_format_output::get_command_string */

} //namespace

struct Computed_field *Computed_field_create_format_output(
	struct Cmiss_field_module *field_module,
	struct Computed_field *source_field, char *format_string)
{
	Cmiss_field_id field = 0;
	if (source_field && format_string)
	{
		if (source_field->number_of_components <= 4)
		{
			int valid_string = 1;
			char *remaining_string = format_string;
			int number_of_format_specifiers = 0;
			while (valid_string && (remaining_string = strchr(remaining_string, '%')))
			{
				number_of_format_specifiers++;
				remaining_string++;
				/* Ignore modifiers */
				int specifiers = strspn(remaining_string, "0123456789.hlL -+#");
				remaining_string += specifiers;
				/* Fail if we don't get the expected format codes */
				if (0 != strcspn(remaining_string, "eEfgG"))
				{
					valid_string = 0;
				}
				remaining_string++;
			}
			if (number_of_format_specifiers != source_field->number_of_components)
				valid_string = 0;
			if (valid_string)
			{
				field = Computed_field_create_generic(field_module,
					/*check_source_field_regions*/true, source_field->number_of_components,
					/*number_of_source_fields*/1, &source_field,
					/*number_of_source_values*/0, NULL,
					new Computed_field_format_output(source_field->number_of_components, format_string));
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_create_format_output.  Invalid or unsupported format_string.");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_create_format_output.  Only source fields with between 1 and 4 components are currently supported.");
		}
	}
	return (field);
}

int Computed_field_get_type_format_output(struct Computed_field *field,
	struct Computed_field **source_field, char **format_string_out)
{
	int return_code = 1;

	ENTER(Computed_field_get_type_format_output);
	Computed_field_format_output *this_field;
	if (field&&(this_field = dynamic_cast<Computed_field_format_output*>(field->core)))
	{
		*source_field = field->source_fields[0];
		*format_string_out = duplicate_string(this_field->format_string);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_format_output.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_format_output */


#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <string.h>
#include "api/cmiss_variable_composite.h"
#include "typemap.h"

MODULE = Cmiss::Variable::Composite  PACKAGE = Cmiss::Variable::Composite  PREFIX = Cmiss_variable_composite_

PROTOTYPES: DISABLE

Cmiss::Variable
create(char *name,AV *variables_array)
	CODE:
		/* the result, in Perl, is a reference to a stash (which is a pointer to the
			Cmiss_variable structure).  This means that don't need to worry about
			ACCESSing for Perl assignment/copy, $cmiss_variable_2=$cmiss_variable_1,
			because this increments the reference count for the stash (DESTROY is
			called when the stash reference count gets to zero) */
		{
			int i,number_of_variables;
			IV tmp_IV;
			Cmiss_variable_id *variables;

			if (variables_array&&(0<(number_of_variables=av_len(variables_array)+1)))
			{
				variables=(Cmiss_variable_id *)malloc(
					number_of_variables*sizeof(Cmiss_variable_id));
				if (variables)
				{
					i=0;
					while ((i<number_of_variables)&&sv_derived_from(
						AvARRAY(variables_array)[i],"Cmiss::Variable"))
					{
						tmp_IV=SvIV((SV*)(SvRV(AvARRAY(variables_array)[i])));
						variables[i]=INT2PTR(Cmiss__Variable,tmp_IV);
						i++;
					}
					if (!((i>=number_of_variables)&&(RETVAL=CREATE(Cmiss_variable_composite)(
						name,number_of_variables,variables))))
					{
						free(variables);
					}
				}
				else
				{
					if (variables)
					{
						free(variables);
					}
				}
			}
		}
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL

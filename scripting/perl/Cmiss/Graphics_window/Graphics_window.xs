#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include "api/cmiss_context.h"
#include "api/cmiss_scene_viewer.h"
#include "perl/Cmiss/typemap.h"
#include "perl/Cmiss/Scene_viewer/typemap.h"

static double
constant(char *name, int len, int arg)
{
    errno = EINVAL;
    return 0;
}

MODULE = Cmiss::Graphics_window		PACKAGE = Cmiss::Graphics_window		PREFIX = Cmiss_graphics_window_

PROTOTYPES: DISABLE

double
constant(sv,arg)
    PREINIT:
	STRLEN		len;
    INPUT:
	SV *		sv
	char *		s = SvPV(sv, len);
	int		arg
    CODE:
	RETVAL = constant(s,len,arg);
    OUTPUT:
	RETVAL

Cmiss::Scene_viewer
get_scene_viewer_by_name_xs(Cmiss::Cmiss_context context,char *name,int pane_number)
	CODE:
		RETVAL=0;
		if (context&&name)
		{
			RETVAL = Cmiss_context_get_graphics_window_pane_by_name(
				context, name, pane_number);
		}
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL

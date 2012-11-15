/*******************************************************************************
FILE : cmgui.cpp

LAST MODIFIED : 7 January 2003

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
 * Portions created by the Initial Developer are Copyright (C) 2005-2011
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
/*SAB I have concatenated the correct version file for each version
  externally in the shell with cat #include "version.h"*/

#include "configure/cmgui_configure.h"
#include "configure/version.h"

#include "context/context_app.h"
#include "zinc/graphicsmodule.h"
#include "command/cmiss.h"
#include "context/context_app.h"
#include "context/user_interface_module.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "general/message.h"

#if defined (WX_USER_INTERFACE)
# include "user_interface/user_interface_wx.hpp"
# if defined (DARWIN)
#  include <ApplicationServices/ApplicationServices.h>
# endif
#endif
#include <libxml/catalog.h>
#include <libxml/parser.h>
#include <libxml/xmlschemastypes.h>

#if defined (WX_USER_INTERFACE)
#include <wx/wx.h>
#include <wx/apptrait.h>
#include <wx/xrc/xmlres.h>
#endif

/*
Global functions
----------------
*/

#if defined (WX_USER_INTERFACE)

bool wxCmguiApp::OnInit()
{
	return (true);
}

wxAppTraits * wxCmguiApp::CreateTraits()
{
	return new wxGUIAppTraits;
}

void wxCmguiApp::OnIdle(wxIdleEvent& event)
{
	if (event_dispatcher)
	{
		if (Event_dispatcher_process_idle_event(event_dispatcher))
		{
			event.RequestMore();
		}
	}
}

void wxCmguiApp::SetEventDispatcher(Event_dispatcher *event_dispatcher_in)
{
	event_dispatcher = event_dispatcher_in;
}

BEGIN_EVENT_TABLE(wxCmguiApp, wxApp)
	EVT_IDLE(wxCmguiApp::OnIdle)
END_EVENT_TABLE()

IMPLEMENT_APP_NO_MAIN(wxCmguiApp)

#endif /*defined (WX_USER_INTERFACE)*/

#if !defined (WIN32_USER_INTERFACE) && !defined (_MSC_VER)
int main(int argc,const char *argv[])
#else /* !defined (WIN32_USER_INTERFACE)  && !defined (_MSC_VER)*/
int WINAPI WinMain(HINSTANCE current_instance,HINSTANCE previous_instance,
	LPSTR command_line,int initial_main_window_state)
	/* using WinMain as the entry point tells Windows that it is a gui and to use
		the graphics device interface functions for I/O */
	/*???DB.  WINDOWS a zero return code if WinMain does get into the message
		loop.  Other application interfaces may expect something else.  Should this
		failure code be #define'd ? */
	/*???DB. Win32 SDK says that don't have to call it WinMain */
#endif /* !defined (WIN32_USER_INTERFACE)  && !defined (_MSC_VER)*/
/*******************************************************************************
LAST MODIFIED : 7 January 2003

DESCRIPTION :
Main program for the CMISS Graphical User Interface
==============================================================================*/
{
	int return_code = 0;
#if defined (WIN32_USER_INTERFACE) || defined (_MSC_VER)
	int argc = 1, i;
	const char **argv;
	char *p, *q;
#endif /* defined (WIN32_USER_INTERFACE) */
	struct Cmiss_context_app *context = NULL;
	struct User_interface_module *UI_module = NULL;
	struct Cmiss_command_data *command_data;

#if !defined (WIN32_USER_INTERFACE) && !defined (_MSC_VER)
	ENTER(main);
#else /* !defined (WIN32_USER_INTERFACE)  && !defined (_MSC_VER)*/
	ENTER(WinMain);

	//_CrtSetBreakAlloc(28336);
	for (p = command_line; p != NULL && *p != 0;)
	{
		p = strchr(p, ' ');
		if (p != NULL)
			p++;
		argc++;
	}

	argv = (const char **)malloc(sizeof(*argv) * argc);

	argv[0] = "cmgui";

	for (i = 1, p = command_line; p != NULL && *p != 0;)
	{
		q = strchr(p, ' ');
		if (q != NULL)
			*q++ = 0;
		if (p != NULL)
			argv[i++] = p;
		p = q;
	}
#endif /* !defined (WIN32_USER_INTERFACE)  && !defined (_MSC_VER)*/

	/* display the version */
	display_message(INFORMATION_MESSAGE, "%s version %s\n%s\n"
		"Build information: %s %s\n", CMISS_NAME_STRING, CMISS_VERSION_STRING,
		CMISS_COPYRIGHT_STRING, CMISS_BUILD_STRING,
		CMISS_SVN_REVISION_STRING);

#if defined (CARBON_USER_INTERFACE) || (defined (WX_USER_INTERFACE) && defined (DARWIN))
	ProcessSerialNumber PSN;
	GetCurrentProcess(&PSN);
	TransformProcessType(&PSN,kProcessTransformToForegroundApplication);
#endif
	context = Cmiss_context_app_create("default");
#if defined (WX_USER_INTERFACE)
	int wx_entry_started = 0;
#endif
	if (context)
	{
#if defined (WX_USER_INTERFACE) || (!defined (WIN32_USER_INTERFACE) && !defined (_MSC_VER))
		UI_module = Cmiss_context_create_user_interface(context, argc, argv, NULL);
#else /* !defined (WIN32_USER_INTERFACE)  && !defined (_MSC_VER)*/
		UI_module = Cmiss_context_create_user_interface(context, argc, argv, current_instance,
			previous_instance, command_line, initial_main_window_state, NULL);
#endif /* !defined (WIN32_USER_INTERFACE)  && !defined (_MSC_VER)*/
		if (UI_module)
		{
#if defined (WX_USER_INTERFACE)
			if (UI_module->user_interface)
			{
				char **temp_argv = NULL, **cleanup_argv = NULL;
				int temp_argc = argc, cleanup_argc = argc;
				if (cleanup_argc > 0)
				{
					ALLOCATE(temp_argv, char *, cleanup_argc);
					ALLOCATE(cleanup_argv, char *, cleanup_argc);
					for (int i = 0; i < cleanup_argc; i++)
					{
						cleanup_argv[i] = temp_argv[i] = duplicate_string(argv[i]);
					}
				}
				if (wxEntryStart(temp_argc, temp_argv))
				{
					wx_entry_started = 1;
					wxXmlResource::Get()->InitAllHandlers();
					wxCmguiApp &app = wxGetApp();
					if (&app)
					{
						app.SetEventDispatcher(UI_module->event_dispatcher);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"initialiseWxApp.  wxCmguiApp not initialised.");
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"initialiseWxApp.  Invalid arguments.");
				}
				if (cleanup_argv)
				{
					for (int i = 0; i < cleanup_argc; i++)
					{
						DEALLOCATE(cleanup_argv[i]);
					}
					DEALLOCATE(temp_argv);
					DEALLOCATE(cleanup_argv);
				}
			}
#endif
			Cmiss_graphics_module_id graphics_module = NULL;
			if (NULL != (graphics_module = Cmiss_context_get_default_graphics_module(Cmiss_context_app_get_core_context(context))))
			{
				Cmiss_graphics_module_define_standard_materials(graphics_module);
				Cmiss_graphics_module_destroy(&graphics_module);
			}
			if (NULL != (command_data = Cmiss_context_get_default_command_interpreter(context)))
			{
				Cmiss_command_data_set_cmgui_string(command_data, CMISS_NAME_STRING,
					CMISS_VERSION_STRING, "CMISS_DATE_STRING", CMISS_COPYRIGHT_STRING, CMISS_BUILD_STRING,
					CMISS_SVN_REVISION_STRING);
				Cmiss_command_data_main_loop(command_data);
				Cmiss_command_data_destroy(&command_data);
				return_code = 0;
			}
			else
			{
				return_code = 1;
			}
			User_interface_module_destroy(&UI_module);
		}
		else
		{
			return_code = 1;
		}
		Cmiss_context_app_destroy(&context);
		Context_internal_cleanup();
#if defined (WX_USER_INTERFACE)
		if (wx_entry_started)
			wxEntryCleanup();
#endif
		/* FieldML does not cleanup the global varaibles xmlParser, xmlSchematypes and
		 * xmlCatalog at this moment, so we clean it up here instead*/
		xmlCatalogCleanup();
		xmlSchemaCleanupTypes();
		xmlCleanupParser();
	}
	else
	{
		return_code = 1;
	}
#if defined (WIN32_USER_INTERFACE) || defined (_MSC_VER)
	free(argv)
#endif
	LEAVE;

	return (return_code);
} /* main */

/***************************************************************************//**
 * FILE : context.hpp
 */
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
 * The Original Code is libZinc.
 *
 * The Initial Developer of the Original Code is
 * Auckland Uniservices Ltd, Auckland, New Zealand.
 * Portions created by the Initial Developer are Copyright (C) 2010
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

#ifndef __ZN_CONTEXT_HPP__
#define __ZN_CONTEXT_HPP__

#include "api/cmiss_context.h"
#include "api++/region.hpp"
#include "api++/graphicsmodule.hpp"
#include "api++/timekeeper.hpp"
#include "api++/sceneviewer.hpp"

namespace Zn
{

class Context
{
private:

	Cmiss_context_id id;

public:

	Context() : id(0)
	{ }
	// Creates a new Cmiss Context instance
	Context(const char *contextName) :
		id(Cmiss_context_create(contextName))
	{ }

	// takes ownership of C handle, responsibility for destroying it
	explicit Context(Cmiss_context_id context_id) :
		id(context_id)
	{ }

	Context(const Context& context) :
		id(Cmiss_context_access(context.id))
	{ }

	~Context()
	{
		if (0 != id)
		{
			Cmiss_context_destroy(&id);
		}
	}

	bool isValid()
	{
		return (0 != id);
	}

	Context& operator=(const Context& context)
	{
		Cmiss_context_id temp_id = Cmiss_context_access(context.id);
		if (0 != id)
		{
			Cmiss_context_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	Cmiss_context_id getId()
	{
		return id;
	}

	Region getDefaultRegion()
	{
		return Region(Cmiss_context_get_default_region(id));
	}

	Region createRegion()
	{
		return Region(Cmiss_context_create_region(id));
	}

	GraphicsModule getDefaultGraphicsModule()
	{
		return GraphicsModule(Cmiss_context_get_default_graphics_module(id));
	}

	SceneViewerPackage getDefaultSceneViewerPackage()
 	{
		return SceneViewerPackage(Cmiss_context_get_default_scene_viewer_package(id));
 	}

//	TimeKeeper getDefaultTimeKeeper()
//	{
//		return TimeKeeper(Cmiss_context_get_default_time_keeper(id));
//	}

};

}  // namespace Zn


#endif /* __ZN_CONTEXT_HPP__ */

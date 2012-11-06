/***************************************************************************//**
 * FILE : tessellation.hpp
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
 * Portions created by the Initial Developer are Copyright (C) 2012
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
#ifndef __ZN_TESSELLATION_HPP__
#define __ZN_TESSELLATION_HPP__

#include "zinc/tessellation.h"

namespace zinc
{

class Tessellation
{
protected:
	Cmiss_tessellation_id id;

public:

	Tessellation() : id(0)
	{  }

	// takes ownership of C handle, responsibility for destroying it
	explicit Tessellation(Cmiss_tessellation_id in_tessellation_id) :
		id(in_tessellation_id)
	{  }

	Tessellation(const Tessellation& tessellation) :
		id(Cmiss_tessellation_access(tessellation.id))
	{  }

	Tessellation& operator=(const Tessellation& tessellation)
	{
		Cmiss_tessellation_id temp_id = Cmiss_tessellation_access(tessellation.id);
		if (0 != id)
		{
			Cmiss_tessellation_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~Tessellation()
	{
		if (0 != id)
		{
			Cmiss_tessellation_destroy(&id);
		}
	}

	bool isValid()
	{
		return (0 != id);
	}

	enum Attribute
	{
		ATTRIBUTE_INVALID = CMISS_TESSELLATION_ATTRIBUTE_INVALID,
		ATTRIBUTE_IS_MANAGED = CMISS_TESSELLATION_ATTRIBUTE_IS_MANAGED,
		ATTRIBUTE_MINIMUM_DIVISIONS_SIZE = CMISS_TESSELLATION_ATTRIBUTE_MINIMUM_DIVISIONS_SIZE,
		ATTRIBUTE_REFINEMENT_FACTORS_SIZE = CMISS_TESSELLATION_ATTRIBUTE_REFINEMENT_FACTORS_SIZE,
	};

	Cmiss_tessellation_id getId()
	{
		return id;
	}

	int getAttributeInteger(Attribute attribute)
	{
		return Cmiss_tessellation_get_attribute_integer(id,
			static_cast<Cmiss_tessellation_attribute>(attribute));
	}

	int setAttributeInteger(Attribute attribute, int value)
	{
		return Cmiss_tessellation_set_attribute_integer(id,
			static_cast<Cmiss_tessellation_attribute>(attribute), value);
	}

	char *getName()
	{
		return Cmiss_tessellation_get_name(id);
	}

	int setName(const char *name)
	{
		return Cmiss_tessellation_set_name(id, name);
	}

	int getMinimumDivisions(int size, int *outMinimumDivisions)
	{
		return Cmiss_tessellation_get_minimum_divisions(id, size, outMinimumDivisions);
	}

	int setMinimumDivisions(int size, int *minimumDivisions)
	{
		return Cmiss_tessellation_set_minimum_divisions(id, size, minimumDivisions);
	}

	int getRefinementFactors(int size, int *outRefinementFactors)
	{
		return Cmiss_tessellation_get_refinement_factors(id, size, outRefinementFactors);
	}

	int setRefinementFactors(int size, int *refinementFactors)
	{
		return Cmiss_tessellation_set_refinement_factors(id, size, refinementFactors);
	}

};

}  // namespace zinc

#endif /* __ZN_TESSELLATION_HPP__ */

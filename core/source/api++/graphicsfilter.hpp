/***************************************************************************//**
 * FILE : graphicsfilter.hpp
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
#ifndef __ZN_CMISS_GRAPHICS_FILTER_HPP__
#define __ZN_CMISS_GRAPHICS_FILTER_HPP__

#include "api/cmiss_graphics_filter.h"
#include "api++/graphic.hpp"

namespace Zn
{

class GraphicsFilter
{
protected:
	Cmiss_graphics_filter_id id;

public:

	GraphicsFilter() : id(0)
	{  }

	// takes ownership of C handle, responsibility for destroying it
	explicit GraphicsFilter(Cmiss_graphics_filter_id in_graphics_filter_id) :
		id(in_graphics_filter_id)
	{  }

	GraphicsFilter(const GraphicsFilter& graphicsFilter) :
		id(Cmiss_graphics_filter_access(graphicsFilter.id))
	{  }

	GraphicsFilter& operator=(const GraphicsFilter& graphicsFilter)
	{
		Cmiss_graphics_filter_id temp_id = Cmiss_graphics_filter_access(graphicsFilter.id);
		if (0 != id)
		{
			Cmiss_graphics_filter_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~GraphicsFilter()
	{
		if (0 != id)
		{
			Cmiss_graphics_filter_destroy(&id);
		}
	}

	bool isValid()
	{
		return (0 != id);
	}

	enum Attribute
	{
		ATTRIBUTE_INVALID = CMISS_GRAPHICS_FILTER_ATTRIBUTE_INVALID,
		ATTRIBUTE_IS_MANAGED = CMISS_GRAPHICS_FILTER_ATTRIBUTE_IS_MANAGED,
		ATTRIBUTE_IS_INVERSE = CMISS_GRAPHICS_FILTER_ATTRIBUTE_IS_INVERSE,
	};

	Cmiss_graphics_filter_id getId()
	{
		return id;
	}

	int evaluateGraphic(Graphic& graphic)
	{
		return Cmiss_graphics_filter_evaluate_graphic(id, graphic.getId());
	}

	char *getName()
	{
		return Cmiss_graphics_filter_get_name(id);
	}

	int setName(Cmiss_graphics_filter_id filter, const char *name)
	{
		return Cmiss_graphics_filter_set_name(id, name);
	}

	int getAttributeInteger(Attribute attribute)
	{
		return Cmiss_graphics_filter_get_attribute_integer(id,
			static_cast<Cmiss_graphics_filter_attribute>(attribute));
	}

	int setAttributeInteger(Attribute attribute, int value)
	{
		return Cmiss_graphics_filter_set_attribute_integer(id,
			static_cast<Cmiss_graphics_filter_attribute>(attribute), value);
	}
};

class GraphicsFilterOperator : public GraphicsFilter
{
public:

	// takes ownership of C handle, responsibility for destroying it
	explicit GraphicsFilterOperator(Cmiss_graphics_filter_operator_id graphics_filter_id) :
		GraphicsFilter(reinterpret_cast<Cmiss_graphics_filter_id>(graphics_filter_id))
	{ }

	GraphicsFilterOperator(GraphicsFilter& graphicsFilter) :
		GraphicsFilter(reinterpret_cast<Cmiss_graphics_filter_id>(
			Cmiss_graphics_filter_cast_operator(graphicsFilter.getId())))
	{	}

	int appendOperand(GraphicsFilter& operand)
	{
		return Cmiss_graphics_filter_operator_append_operand(
			reinterpret_cast<Cmiss_graphics_filter_operator_id>(id), operand.getId());
	}

	GraphicsFilter getFirstOperand()
	{
		return GraphicsFilter(Cmiss_graphics_filter_operator_get_first_operand(
			reinterpret_cast<Cmiss_graphics_filter_operator_id>(id)));
	}

	GraphicsFilter getNextOperand(GraphicsFilter& refOperand)
	{
		return GraphicsFilter(Cmiss_graphics_filter_operator_get_next_operand(
			reinterpret_cast<Cmiss_graphics_filter_operator_id>(id), refOperand.getId()));
	}

	int getOperandIsActive(GraphicsFilter& operand)
	{
		return Cmiss_graphics_filter_operator_get_operand_is_active(
			reinterpret_cast<Cmiss_graphics_filter_operator_id>(id), operand.getId());
	}

	int setOperandIsActive(GraphicsFilter& operand, int isActive)
	{
		return Cmiss_graphics_filter_operator_set_operand_is_active(
			reinterpret_cast<Cmiss_graphics_filter_operator_id>(id), operand.getId(), isActive);
	}

	int insertOperandBefore(GraphicsFilter& operand, GraphicsFilter& refOperand)
	{
		return Cmiss_graphics_filter_operator_insert_operand_before(
			reinterpret_cast<Cmiss_graphics_filter_operator_id>(id), operand.getId(), refOperand.getId());
	}

	int removeOperand(GraphicsFilter& operand)
	{
		return Cmiss_graphics_filter_operator_remove_operand(
			reinterpret_cast<Cmiss_graphics_filter_operator_id>(id), operand.getId());
	}
};

}  // namespace Zn

#endif /* __ZN_CMISS_GRAPHICS_FILTER_HPP__ */

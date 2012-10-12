/***************************************************************************//**
 * FILE : fieldtypesnodesetoperators.hpp
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
#ifndef __ZN_FIELD_TYPES_NODESET_OPERATORS_HPP__
#define __ZN_FIELD_TYPES_NODESET_OPERATORS_HPP__

#include "api/cmiss_field_nodeset_operators.h"
#include "api++/field.hpp"
#include "api++/node.hpp"

namespace Zn
{

class FieldNodesetSum : public Field
{
private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldNodesetSum(Cmiss_field_id field_id) : Field(field_id)
	{	}

	friend FieldNodesetSum FieldModule::createNodesetSum(Field& sourceField, Nodeset& nodeset);

public:

	FieldNodesetSum() : Field(0)
	{	}

};

class FieldNodesetMean : public Field
{
private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldNodesetMean(Cmiss_field_id field_id) : Field(field_id)
	{	}

	friend FieldNodesetMean FieldModule::createNodesetMean(Field& sourceField,
		Nodeset& nodeset);

public:

	FieldNodesetMean() : Field(0)
	{	}

};

class FieldNodesetSumSquares : public Field
{
private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldNodesetSumSquares(Cmiss_field_id field_id) : Field(field_id)
	{	}

	friend FieldNodesetSumSquares FieldModule::createNodesetSumSquares(
		Field& sourceField, Nodeset& nodeset);

public:

	FieldNodesetSumSquares() : Field(0)
	{	}

};

class FieldNodesetMeanSquares : public Field
{
private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldNodesetMeanSquares(Cmiss_field_id field_id) : Field(field_id)
	{	}

	friend FieldNodesetMeanSquares FieldModule::createNodesetMeanSquares(
		Field& sourceField, Nodeset& nodeset);

public:

	FieldNodesetMeanSquares() : Field(0)
	{	}

};

inline FieldNodesetSum FieldModule::createNodesetSum(Field& sourceField, Nodeset& nodeset)
{
	return FieldNodesetSum(Cmiss_field_module_create_nodeset_sum(id,
		sourceField.getId(), nodeset.getId()));
}

inline FieldNodesetMean FieldModule::createNodesetMean(Field& sourceField, Nodeset& nodeset)
{
	return FieldNodesetMean(Cmiss_field_module_create_nodeset_mean(id,
		sourceField.getId(), nodeset.getId()));
}

inline FieldNodesetSumSquares FieldModule::createNodesetSumSquares(
	Field& sourceField, Nodeset& nodeset)
{
	return FieldNodesetSumSquares(Cmiss_field_module_create_nodeset_sum_squares(id,
		sourceField.getId(), nodeset.getId()));
}

inline FieldNodesetMeanSquares FieldModule::createNodesetMeanSquares(
	Field& sourceField, Nodeset& nodeset)
{
	return FieldNodesetMeanSquares(Cmiss_field_module_create_nodeset_mean_squares(id,
		sourceField.getId(), nodeset.getId()));
}

}  // namespace Zn

#endif /* __ZN_FIELD_TYPES_NODESET_OPERATORS_HPP__ */

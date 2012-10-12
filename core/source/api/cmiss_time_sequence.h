/***************************************************************************//**
 * FILE : cmiss_time_sequence.h
 *
 * The public interface to Cmiss_time_sequence.
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
#ifndef __CMISS_TIME_SEQUENCE_H__
#define __CMISS_TIME_SEQUENCE_H__

#include "types/cmiss_field_module_id.h"
#include "types/cmiss_time_sequence_id.h"

#include "cmiss_shared_object.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************//**
 * Finds or creates a Cmiss_time_sequence in the field module which matches the
 * sequence of times provided.
 * @param field_module  The field module to search or create in.
 * @param number_of_times  The size of the times array.
 * @param times  Array of times. Note later times must not be less than earlier
 * times.
 * @return  The time sequence matching the times array, or NULL if failed.
 */
ZINC_API Cmiss_time_sequence_id Cmiss_field_module_get_matching_time_sequence(
	Cmiss_field_module_id field_module, int number_of_times, double *times);

/***************************************************************************//**
 * Returns a new reference to the time sequence with reference count
 * incremented. Caller is responsible for destroying the new reference.
 *
 * @param time_sequence  The time sequence to obtain a new reference to.
 * @return  New time sequence reference with incremented reference count.
 */
ZINC_API Cmiss_time_sequence_id Cmiss_time_sequence_access(
	Cmiss_time_sequence_id time_sequence);

/***************************************************************************//**
 * Destroys reference to the time sequence and sets pointer/handle to NULL.
 * Internally this just decrements the reference count.
 *
 * @param time_sequence_address  Address of time sequence reference.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_time_sequence_destroy(Cmiss_time_sequence_id *time_sequence_address);

/***************************************************************************//**
 * Sets the time for the given time_index in the time sequence.
 * This can only be done while the time sequence is not in use by other objects.
 * If the sequence does not have as many times as the <time_index> then it will
 * be expanded and the unspecified times also set to <time>.
 * @param time_sequence  The time sequence to modify.
 * @param time_index  The index of the time to set, starting at 0.
 * @param time  The time to set.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_time_sequence_set_value(Cmiss_time_sequence_id time_sequence,
	int time_index, double time);

#ifdef __cplusplus
}
#endif

#endif /* __CMISS_TIME_SEQUENCE_H__ */

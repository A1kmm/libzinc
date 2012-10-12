/***************************************************************************//**
 * FILE : cmiss_field_image_id.h
 *
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

#ifndef CMISS_FIELD_IMAGE_ID_H

/*****************************************************************************//**
 * The image field specific handle to a image Cmiss_field.
 */
	struct Cmiss_field_image;
	typedef struct Cmiss_field_image *Cmiss_field_image_id;

/***************************************************************************//**
 * A handle to cmiss stream information image. Stream information image is a
 * derived type of Cmiss_stream_information_id.
 * User can create and get a handle to stream information image with functions
 * provided with Cmiss_field_image.
 * User can use this derived type to set number of informations associate with
 * images inputs and outputs. See cmiss_field_image.h for more information.
 *
 * #see Cmiss_stream_information_id
 * #see Cmiss_field_image_create_stream_information
 * #see Cmiss_stream_information_cast_image
 * #see Cmiss_stream_information_image_base_cast
 */
	struct Cmiss_stream_information_image;
	typedef struct Cmiss_stream_information_image *Cmiss_stream_information_image_id;

	/*****************************************************************************//**
	 * Optional information used to describe the binary data supplied to the images.
	 */
	enum Cmiss_stream_information_image_pixel_format
	{
		CMISS_STREAM_INFORMATION_IMAGE_PIXEL_FORMAT_INVALID = 0,
		CMISS_STREAM_INFORMATION_IMAGE_PIXEL_FORMAT_LUMINANCE = 1,
		CMISS_STREAM_INFORMATION_IMAGE_PIXEL_FORMAT_LUMINANCE_ALPHA = 2,
		CMISS_STREAM_INFORMATION_IMAGE_PIXEL_FORMAT_RGB = 3,
		CMISS_STREAM_INFORMATION_IMAGE_PIXEL_FORMAT_RGBA = 4,
		CMISS_STREAM_INFORMATION_IMAGE_PIXEL_FORMAT_ABGR = 5,
		CMISS_STREAM_INFORMATION_IMAGE_PIXEL_FORMAT_BGR = 6
	};

	#define CMISS_FIELD_IMAGE_ID_H

#endif /* CMISS_FIELD_IMAGE_ID_H */

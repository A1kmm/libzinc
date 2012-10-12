/*******************************************************************************
FILE : time.h

LAST MODIFIED : 26 June 2002

DESCRIPTION :
Defines the gettimeofday and relevant structure for UNIX and WIN32_SYSTEM
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
#if !defined (GENERAL_TIME_H) /* Distinguish general/time.h and time/time.h */
#define GENERAL_TIME_H


#include "api/cmiss_zinc_configure.h"


#if defined (UNIX) /* switch (OPERATING_SYSTEM) */
#include <sys/time.h>
#include <sys/times.h>
#elif defined (WIN32_SYSTEM) /* switch (OPERATING_SYSTEM) */
#if defined (_MSC_VER)
	#ifndef _CRTDBG_MAP_ALLOC
		#define _CRTDBG_MAP_ALLOC
	#endif
	#include <stdlib.h>
	#include <crtdbg.h>
#endif /* defined (_MSC_VER) */
//#define WINDOWS_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int gettimeofday(struct timeval *time, void *timezone);

#ifdef __cplusplus
}
#endif /* __cplusplus */

typedef long clock_t;
struct tms 
{
	/* The times function in cmgui is just used to get a timestamp at
		the moment, if more than this is required it will need to be 
		implemented in the c function as well as added to this structure */
	int dummy;
};
clock_t times(struct tms *buffer);
#endif /* switch (OPERATING_SYSTEM) */

#endif /* !defined (GENERAL_TIME_H) */

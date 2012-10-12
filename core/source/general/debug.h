/*******************************************************************************
FILE : debug.h

LAST MODIFIED : 10 October 2003

DESCRIPTION :
Function definitions for debugging.

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
#if !defined (DEBUG_H)
#define DEBUG_H

#include <stdarg.h>

#include "api/cmiss_zinc_configure.h"
#include "api/cmiss_shared_object.h"

/*
Macros
------
*/

#define ENTER( function_name )

#define LEAVE

/* Following are the only question mark operators allowed in CMGUI, needed
	because it is machine dependent whether a pointer or NULL is returned for an
	allocation of zero size. Inlined for optimisation */
#if defined (OPTIMISED)

#include <stdlib.h>

#define ALLOCATE( result , type , number ) \
 ( result = ( 0 < ( number ) ) ? ( type * )malloc( ( number ) * sizeof( type ) ) : ( type * )NULL )

#define DEALLOCATE( ptr ) { if ( ptr ) { free( (char *)( ptr ) ); ( ptr ) = NULL; } }

#define REALLOCATE( final , initial , type , number ) \
 ( final = ( 0 < ( number ) ) ? ( type * )realloc( (char *)( initial ) , ( number ) * sizeof( type ) ) : ( type * )NULL )

#define ASSERT_IF( expression , return_code , error_value )

#else /* defined (OPTIMISED) */

#define ALLOCATE( result , type , number ) \
( result = ( type *) allocate( ( number ) * sizeof( type ) , __FILE__ , \
	__LINE__, #type ))

#define DEALLOCATE( ptr ) \
{ deallocate((char *) ptr , __FILE__ , __LINE__ ); ( ptr )=NULL;}

#define REALLOCATE( final , initial , type , number ) \
( final = ( type *) reallocate( (char *)( initial ) , \
	( number ) * sizeof( type ) , __FILE__ , __LINE__, #type ))

#endif /* defined (OPTIMISED) */

/* Treatment of an int to store the value in a void* and to extract it later.
   The pointer to/from integer conversions are not necessary portable so a
   macro is defined here.  */
/*
  The following would would on many machines but the pointer difference is
  only defined if both pointers point at the same object.  Pointers could
  contain segment information and this may be bad if the segment doesn't
  exist.
#define INT2VOIDPTR( i ) ( (void *)((char *)0 - i ) )
#define VOIDPTR2INT( vp ) ( (char *) vp - (char *)0 )

  gcc complains about conversions between integer and pointer types when the
  types are of different sizes.  A double explicit cast with an intermediate
  integer the same size as a pointer silences these errors.  One explicit cast
  allows the pointer/integer conversion while the other silences the possible
  truncation warning.

  An integer type is required for this intermediate type.  It should be at
  least as large as the smallest of the integer or pointer types to avoid an
  unnecessary truncation.

  ptrdiff_t is widely available (in stddef.h) and large enough to hold the
  difference between two pointers (to the same object).
*/
#define INT2VOIDPTR( i ) ( (void *)(ptrdiff_t) i )
#define VOIDPTR2INT( vp ) ( (ptrdiff_t) vp )
/*
  It is possible that on some platforms the integer type (long maybe) and the
  pointer are both larger than ptrdiff_t, in which case the above would cause
  an unnecessary truncation.

  intptr_t is defined as an integer large enough to hold a pointer, but is not
  necessarily defined.  If available, this would provide an intermediate type
  that would avoid an unnecessary truncation.  If not defined in standard
  headers, we may be able to define it ourselves.

#if defined (AIX)
#include <inttypes.h>
#else
#include <stdint.h>
#endif
#define INT2VOIDPTR( i ) ( (void *)(intptr_t) i )
#define VOIDPTR2INT( vp ) ( (intptr_t) vp )
*/

/*
Global functions
----------------
*/
#if defined (USE_PARAMETER_ON)
void use_parameter(int dummy, ... );
/*******************************************************************************
LAST MODIFIED : 10 June 1999

DESCRIPTION :
Prototype for function which is called in the development stage (when
USE_PARAMETER_ON is defined) to swallow unused parameters to functions which
would otherwise cause compiler warnings. For example, parameter <dummy_void>
is swallowed with the call USE_PARAMETER(dummy_void); at the start of function.
==============================================================================*/
#  if !defined(__cplusplus)
#    define USE_PARAMETER(dummy) use_parameter(0,dummy)
#  else
#    define USE_PARAMETER(dummy) (void)dummy;
#  endif
#else /* defined (USE_PARAMETER_ON) */
#define USE_PARAMETER(dummy)
#endif /* defined (USE_PARAMETER_ON) */

#if !defined (OPTIMISED)
char *allocate(unsigned long int size, const char *file_name,int line_number,const char *type);
/*******************************************************************************
LAST MODIFIED : 7 January 1998

DESCRIPTION :
Wrapper for malloc.
==============================================================================*/

void deallocate(char *ptr,const char *file_name,int line_number);
/*******************************************************************************
LAST MODIFIED : 7 January 1998

DESCRIPTION :
Wrapper for free.
==============================================================================*/

char *reallocate(char *ptr,unsigned long int size,const char *file_name,int line_number,
	const char *type);
/*******************************************************************************
LAST MODIFIED : 7 January 1998

DESCRIPTION :
Wrapper for realloc.
==============================================================================*/
#endif /* !defined (OPTIMISED) */

int list_memory(int count, int show_pointers, int increment_counter,
	int show_structures);
/*******************************************************************************
LAST MODIFIED : 29 February 2000

DESCRIPTION :
Writes out memory blocks currently allocated.  Each time this is called an
internal counter is incremented and all subsequent ALLOCATIONS marked with this
new count_number.  i.e. To find a leak run till before the leak (all these
allocations will be marked count 1), call list_memory increment, do the leaky
thing several times (these have count 2), call list_memory increment, do the
leaky thing once (so that any old stuff with count 2 should have been
deallocated and recreated with count 3) and then list_memory 2.  This should
list no memory.
If <count_number> is zero all the memory allocated is written out.
If <count_number> is negative no memory is written out, just the total.
If <count_number> is positive only the memory with that count is written out.
???DB.  printf used because want to make sure that no allocation is going on
	while printing.
<show_pointers> toggles the output format to include the actual memory addresses
or not.  (It isn't useful for testing and output to record the changing
addresses).  If <show_structures> is set then for known types the objects are
cast to the actual object type and then the appropriate list function is called.
==============================================================================*/

int set_check_memory_output(int on);
/*******************************************************************************
LAST MODIFIED : 19 November 2001

DESCRIPTION :
If <on> is non-zero then check memory output is turned on, otherwise, it is
turned off.  Check memory involves calling display_message to give memory
change information for ALLOCATE, DEALLOCATE and REALLOCATE.  display_message
is allowed to use ALLOCATE, DEALLOCATE or REALLOCATE (infinite recursion
prevented).
==============================================================================*/
#endif /* !defined (DEBUG_H) */

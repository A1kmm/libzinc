/*******************************************************************************
FILE : myio.c

LAST MODIFIED : 23 August 2004

DESCRIPTION :
Some additions/modifications to stdio.
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

#include "general/debug.h"
#include "general/myio.h"
#include "general/message.h"

/*
Global functions
----------------
*/

#if defined (BYTE_ORDER) && (1234==BYTE_ORDER)
int fread_little_to_big_endian(char *char_ptr,unsigned sizeof_type,int count,
	FILE* binary_file)
/*******************************************************************************
LAST MODIFIED : 4 September 1995

DESCRIPTION :
Does an fread and then converts from little to big endian.
==============================================================================*/
{
  char *bottom_byte,byte,*element,*top_byte;
	int fread_result,i,j;

	ENTER(fread_little_to_big_endian);
	if (count==(fread_result=fread(char_ptr,sizeof_type,count,binary_file)))
	{
		if ((sizeof(short)==sizeof_type)||(sizeof(int)==sizeof_type)||
			(sizeof(float)==sizeof_type)||(sizeof(double)==sizeof_type))
		{
			element=char_ptr;
			for (j=count;j>0;j--)
			{
				bottom_byte=element;
				top_byte=element+sizeof_type;
				for (i=sizeof_type/2;i>0;i--)
				{
					top_byte--;
					byte= *bottom_byte;
					*bottom_byte= *top_byte;
					*top_byte=byte;
					bottom_byte++;
				}
				element += sizeof_type;
			}
		}
	}
	LEAVE;

	return (fread_result);
} /* fread_little_to_big_endian */

int fwrite_big_to_little_endian(char *char_ptr,unsigned sizeof_type,int count,
	FILE* binary_file)
/*******************************************************************************
LAST MODIFIED : 4 September 1995

DESCRIPTION :
fwrites the little endian form of <char_ptr>.
==============================================================================*/
{
  char *bottom_byte,*element,*top_byte,*temp_char_ptr;
	int fwrite_result,i,j;

	ENTER(fwrite_big_to_little_endian);
	if ((sizeof(short)==sizeof_type)||(sizeof(int)==sizeof_type)||
		(sizeof(float)==sizeof_type)||(sizeof(double)==sizeof_type))
	{
		if (ALLOCATE(temp_char_ptr,char,sizeof_type*count))
		{
			element=temp_char_ptr;
			bottom_byte=char_ptr;
			for (j=count;j>0;j--)
			{
				top_byte=element+sizeof_type;
				for (i=sizeof_type;i>0;i--)
				{
					top_byte--;
					*top_byte= *bottom_byte;
					bottom_byte++;
				}
				element += sizeof_type;
			}
			fwrite_result=fwrite(temp_char_ptr,sizeof_type,count,binary_file);
			DEALLOCATE(temp_char_ptr);
		}
		else
		{
			fwrite_result=0;
		}
	}
	else
	{
		fwrite_result=fwrite(char_ptr,sizeof_type,count,binary_file);
	}
	LEAVE;

	return (fwrite_result);
} /* fwrite_big_to_little_endian */
#endif /* defined (BYTE_ORDER) && (1234==BYTE_ORDER) */

int get_line_number(FILE *stream)
/*******************************************************************************
LAST MODIFIED : 21 June 2001

DESCRIPTION :
Function for calculating the current line in a file.
==============================================================================*/
{
	int c,line_number;
	long location,temp_location;

	ENTER(get_line_number);
	line_number=0;
	if (stream)
	{
		location = ftell(stream);
		rewind(stream);
		temp_location = ftell(stream);
		while (temp_location<location)
		{
			do
			{
				c=fgetc(stream);
			} while (('\n'!=c)&&(EOF!=c));
			temp_location = ftell(stream);
			line_number++;
		}
		/* Re-set the position in the stream to the original location */
		fseek(stream,location,SEEK_SET);
	}
	LEAVE;

	return (line_number);
} /* get_line_number */

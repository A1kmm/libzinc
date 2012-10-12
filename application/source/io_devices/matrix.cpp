/*******************************************************************************
FILE : matrix.c

LAST MODIFIED : 6 January 1998

DESCRIPTION :
Contains routines equivalent in function to those from GL.
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
#include <string.h>
/* for memcpy() */
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "general/debug.h"
#include "io_devices/matrix.h"
#include "io_devices/conversion.h"

void matrix_print(Gmatrix *current)
/*******************************************************************************
LAST MODIFIED : 1 December 1994

DESCRIPTION :
Used for debugging to view a matrix.
==============================================================================*/
{
	int i,j;

	ENTER(matrix_print);
	for (i=0;i<GMATRIX_SIZE;i++)
	{
		for (j=0;j<GMATRIX_SIZE;j++)
		{
			printf("% lf ",current->data[i][j]);
		}
		printf("\n");
	}
	LEAVE;
} /* matrix_print */


void matrix_I(Gmatrix *current)
/*******************************************************************************
LAST MODIFIED : 18 July 1994

DESCRIPTION :
Turns the matrix into the identity
==============================================================================*/
{
	int i,j;

	ENTER(matrix_I);
	for (i=0;i<GMATRIX_SIZE;i++)
	{
		for (j=0;j<GMATRIX_SIZE;j++)
		{
			if (j==i)
			{
				current->data[i][j] = 1.0;
			}
			else
			{
				current->data[i][j] = 0.0;
			}
		}
	}
	LEAVE;
} /* matrix_I */


void matrix_inverse(Gmatrix *current,Gmatrix *inverse)
/*******************************************************************************
LAST MODIFIED : 4 November 1994

DESCRIPTION :
Returns the inverse of current in inverse.
==============================================================================*/
{
	GMATRIX_PRECISION determinant;

	ENTER(matrix_I);
	/* GMH hack to get 2d inverse working */
	/* check for 2d */
	if((current->data[2][2]==1.0)&&
		(current->data[0][2]==0.0)&&(current->data[2][0]==0.0)&&
		(current->data[1][2]==0.0)&&(current->data[2][1]==0.0))
	{
		/* 2d matrix */
		matrix_I(inverse);
		determinant = current->data[0][0]*current->data[1][1]-
			current->data[1][0]*current->data[0][1];
		inverse->data[0][0] = current->data[1][1]/determinant;
		inverse->data[1][1] = current->data[0][0]/determinant;
		inverse->data[1][0] = -current->data[1][0]/determinant;
		inverse->data[0][1] = -current->data[0][1]/determinant;
	}
	else
	{
		printf("???TEMP matrix_inverse is not implemented yet!");
	}
	LEAVE;
} /* matrix_inverse */


void matrix_copy(Gmatrix *dest,Gmatrix *source)
/*******************************************************************************
LAST MODIFIED : 21 July 1994

DESCRIPTION :
Copies source to destination.
==============================================================================*/
{
	ENTER(matrix_copy);
	memcpy(dest,source,sizeof(Gmatrix));
	LEAVE;
} /* matrix_copy */


void matrix_copy_transpose(Gmatrix *dest,Gmatrix *source)
/*******************************************************************************
LAST MODIFIED : 2 December 1994

DESCRIPTION :
Copies the transpose of source to destination.
==============================================================================*/
{
	int i,j;

	ENTER(matrix_copy_transpose);
	for(i=0;i<GMATRIX_SIZE;i++)
	{
		for(j=0;j<GMATRIX_SIZE;j++)
		{
			dest->data[i][j] = source->data[j][i];
		}
	}
	LEAVE;
} /* matrix_copy */


void matrix_vector_unit(GMATRIX_PRECISION *vector)
/*******************************************************************************
LAST MODIFIED : 02 April 1995

DESCRIPTION :
Changes the vector to unit length.
==============================================================================*/
{
	int i;
	GMATRIX_PRECISION sum;

	ENTER(matrix_vector_unit);
	sum = 0.0;
	for(i=0;i<3;i++)
	{
		sum += vector[i]*vector[i];
	}
	sum = sqrt(sum);
	if (sum>0)
	{
		for(i=0;i<3;i++)
		{
			vector[i] = vector[i]/sum;
		}
	}
	LEAVE;
} /* matrix_vector_unit */


void matrix_premult(Gmatrix *current,Gmatrix *pre_matrix)
/*******************************************************************************
LAST MODIFIED : 18 July 1994

DESCRIPTION :
Premultiplies current by pre_matrix.
==============================================================================*/
{
	int i,j,k;
	Gmatrix temp;

	ENTER(matrix_premult);
	matrix_copy(&temp,current);
	for (i=0;i<GMATRIX_SIZE;i++)
	{
		for (j=0;j<GMATRIX_SIZE;j++)
		{
			current->data[i][j] = 0.0;
			for (k=0;k<GMATRIX_SIZE;k++)
			{
				current->data[i][j] += pre_matrix->data[i][k]*temp.data[k][j];
			}
		}
	}
	LEAVE;
} /* matrix_premult */


void matrix_postmult(Gmatrix *current,Gmatrix *post_matrix)
/*******************************************************************************
LAST MODIFIED : 2 December 1994

DESCRIPTION :
Postmultiplies current by post_matrix.
==============================================================================*/
{
	int i,j,k;
	Gmatrix temp;

	ENTER(matrix_postmult);
	matrix_copy(&temp,current);
	for (i=0;i<GMATRIX_SIZE;i++)
	{
		for (j=0;j<GMATRIX_SIZE;j++)
		{
			current->data[i][j] = 0.0;
			for (k=0;k<GMATRIX_SIZE;k++)
			{
				current->data[i][j] += temp.data[i][k]*post_matrix->data[k][j];
			}
		}
	}
	LEAVE;
} /* matrix_premult */


void matrix_premult_vector(GMATRIX_PRECISION *current,Gmatrix *pre_matrix)
/*******************************************************************************
LAST MODIFIED : 18 July 1994

DESCRIPTION :
Premultiplies current by pre_matrix.
==============================================================================*/
{
	int i,j;
	GMATRIX_PRECISION temp[GMATRIX_SIZE];

	ENTER(matrix_premult_vector);
	for (i=0;i<GMATRIX_SIZE;i++)
	{
		temp[i] = current[i];
	}
	for (i=0;i<GMATRIX_SIZE;i++)
	{
		current[i] = 0.0;
		for (j=0;j<GMATRIX_SIZE;j++)
		{
			current[i] += pre_matrix->data[i][j]*temp[j];
		}
	}
	LEAVE;
} /* matrix_premult_vector */


void matrix_postmult_vector(GMATRIX_PRECISION *current,Gmatrix *post_matrix)
/*******************************************************************************
LAST MODIFIED : 2 December 1994

DESCRIPTION :
Postmultiplies current by post_matrix.
==============================================================================*/
{
	int i,j;
	GMATRIX_PRECISION temp[GMATRIX_SIZE];

	ENTER(matrix_postmult_vector);
	for (i=0;i<GMATRIX_SIZE;i++)
	{
		temp[i] = current[i];
	}
	for (i=0;i<GMATRIX_SIZE;i++)
	{
		current[i] = 0.0;
		for (j=0;j<GMATRIX_SIZE;j++)
		{
			current[i] += temp[j]*post_matrix->data[j][i];
		}
	}
	LEAVE;
} /* matrix_postmult_vector */


void matrix_mult(Gmatrix *pre_matrix,Gmatrix *post_matrix,Gmatrix *new_matrix)
/*******************************************************************************
LAST MODIFIED : 2 December 1994

DESCRIPTION :
Multiplies pre_matrix by post_matrix, and then returns the answer in new_matrix.
==============================================================================*/
{
	int i,j,k;

	ENTER(matrix_mult);
	for (i=0;i<GMATRIX_SIZE;i++)
	{
		for (j=0;j<GMATRIX_SIZE;j++)
		{
			new_matrix->data[i][j] = 0.0;
			for (k=0;k<GMATRIX_SIZE;k++)
			{
				new_matrix->data[i][j] += pre_matrix->data[i][k]*post_matrix->data[k][j];
			}
		}
	}
	LEAVE;
} /* matrix_mult */


void matrix_rotate(Gmatrix *current,double angle,char axis)
/*******************************************************************************
LAST MODIFIED : 4 November 1995

DESCRIPTION :
Rotates in a right hand sense about the axis.
==============================================================================*/
{
	double cos_angle,sin_angle,new_angle;
	Gmatrix temp;

	ENTER(matrix_rotate);
	new_angle = angle*PI_180;
	sin_angle = sin(new_angle);
	cos_angle = cos(new_angle);
	matrix_I(&temp);
	switch (axis)
	{
		case 'x':
		case 'X':
		{
			temp.data[1][1] = cos_angle;
			temp.data[2][2] = cos_angle;
			temp.data[2][1] = sin_angle;
			temp.data[1][2] = -sin_angle;
		}; break;
		case 'y':
		case 'Y':
		{
			temp.data[0][0] = cos_angle;
			temp.data[2][2] = cos_angle;
			temp.data[2][0] = -sin_angle;
			temp.data[0][2] = sin_angle;
		}; break;
		case 'z':
		case 'Z':
		{
			temp.data[0][0] = cos_angle;
			temp.data[1][1] = cos_angle;
			temp.data[1][0] = sin_angle;
			temp.data[0][1] = -sin_angle;
		}; break;
	};
	matrix_premult(current,&temp);
	LEAVE;
} /* matrix_rotate */

void matrix_euler(Gmatrix *direction,struct Dof3_data *euler)
/*******************************************************************************
LAST MODIFIED : 3 March 2008

DESCRIPTION :
In translating formulae just think data[across][down].  All other routines
use data[down][across]
Takes a direction cosine matrix and returns the equivalent euler angles in
degrees.
Note that when the x axis is aligned with the z axis, then the distribution
between azimuth and roll is arbitrary, so we will say that it is solely made
up of roll.  Inverse formulae are taken from the Polhemus manual, page 156.
==============================================================================*/
{
	int i;

	ENTER(matrix_euler);
	if ((fabs(direction->data[0][0])>1.0E-12) &&
		(fabs(direction->data[0][1])>1.0E-12))
	{
		euler->data[0] = atan2(direction->data[0][1],direction->data[0][0]);
		euler->data[2] = atan2(direction->data[1][2],direction->data[2][2]);
		euler->data[1] = atan2(-direction->data[0][2],direction->data[0][0]/
			cos(euler->data[0]));
	}
	else
	{
		if (fabs(direction->data[0][0])>1.0E-12)
		{
			euler->data[0] = atan2(direction->data[0][1],direction->data[0][0]);
			euler->data[2] = atan2(direction->data[1][2],direction->data[2][2]);
			euler->data[1] = atan2(-direction->data[0][2],direction->data[0][0]/
				cos(euler->data[0]));
		}
		else
		{
			if (fabs(direction->data[0][1])>1.0E-12)
			{
				euler->data[0] = atan2(direction->data[0][1],direction->data[0][0]);
				euler->data[2] = atan2(direction->data[1][2],direction->data[2][2]);
				euler->data[1] = atan2(-direction->data[0][2],direction->data[0][1]/
					sin(euler->data[0]));
			}
			else
			{
				euler->data[1] = atan2(-direction->data[0][2],0); /* get +/-1 */
				euler->data[0] = 0;
				euler->data[2] = atan2(-direction->data[2][1],
					-direction->data[2][0]*direction->data[0][2]);
			}
		}
	}
	for(i=0;i<3;i++)
		euler->data[i] /= PI_180;
	LEAVE;
} /* matrix_euler */

void euler_matrix(struct Dof3_data *euler,Gmatrix *direction)
/*******************************************************************************
LAST MODIFIED : 3 March 2008

DESCRIPTION :
In translating formulae just think data[across][down].  All other routines
use data[down][across]
Returns the equivalent direction cosine matrix of the passed euler values.
Formulae are taken from the Polhemus manual, page 156.
==============================================================================*/
{
	int i;
	struct Dof3_data euler_degree;
	double cos_azimuth,cos_elevation,cos_roll,sin_azimuth,sin_elevation,
		sin_roll;

	ENTER(euler_matrix);
	for(i=0;i<3;i++)
	{
		euler_degree.data[i] = euler->data[i]*PI_180;
	}
	cos_azimuth = cos(euler_degree.data[0]);
	sin_azimuth = sin(euler_degree.data[0]);
	cos_elevation = cos(euler_degree.data[1]);
	sin_elevation = sin(euler_degree.data[1]);
	cos_roll = cos(euler_degree.data[2]);
	sin_roll = sin(euler_degree.data[2]);
	direction->data[0][0] = cos_azimuth*cos_elevation;
	direction->data[0][1] = sin_azimuth*cos_elevation;
	direction->data[0][2] = -sin_elevation;
	direction->data[1][0] = cos_azimuth*sin_elevation*sin_roll-
		sin_azimuth*cos_roll;
	direction->data[1][1] = sin_azimuth*sin_elevation*sin_roll+
		cos_azimuth*cos_roll;
	direction->data[1][2] = cos_elevation*sin_roll;
	direction->data[2][0] = cos_azimuth*sin_elevation*cos_roll+
		sin_azimuth*sin_roll;
	direction->data[2][1] = sin_azimuth*sin_elevation*cos_roll-
		cos_azimuth*sin_roll;
	direction->data[2][2] = cos_elevation*cos_roll;
	LEAVE;
} /* euler_matrix */

void matrix_scalefactor(Gmatrix *matrix, Triple scale_factor)
/*******************************************************************************
LAST MODIFIED : 10 March 2008

DESCRIPTION :
***Returns the equivalent scale factor of the matrix.
==============================================================================*/
{
	 int i,j;	
	 float sum;

	ENTER(matrix_scalefactor);

	for (i=0; i<3;i++)
	{
		 sum = 0.0;
		 for (j=0; j<3;j++)
		 {
				sum += pow((float)matrix->data[i][j],2);
		 }
		 scale_factor[i] = (float)sqrt(sum);
	}

	LEAVE;
}

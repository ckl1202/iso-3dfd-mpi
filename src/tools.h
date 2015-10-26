/*****************************************************************************
! Copyright(C) 2012 Intel Corporation. All Rights Reserved.
!
! The source code, information  and  material ("Material") contained herein is
! owned  by Intel Corporation or its suppliers or licensors, and title to such
! Material remains  with Intel Corporation  or its suppliers or licensors. The
! Material  contains proprietary information  of  Intel or  its  suppliers and
! licensors. The  Material is protected by worldwide copyright laws and treaty
! provisions. No  part  of  the  Material  may  be  used,  copied, reproduced,
! modified, published, uploaded, posted, transmitted, distributed or disclosed
! in any way  without Intel's  prior  express written  permission. No  license
! under  any patent, copyright  or  other intellectual property rights  in the
! Material  is  granted  to  or  conferred  upon  you,  either  expressly,  by
! implication, inducement,  estoppel or  otherwise.  Any  license  under  such
! intellectual  property  rights must  be express  and  approved  by  Intel in
! writing.
!
! *Third Party trademarks are the property of their respective owners.
!
! Unless otherwise  agreed  by Intel  in writing, you may not remove  or alter
! this  notice or  any other notice embedded  in Materials by Intel or Intel's
! suppliers or licensors in any way.
!
!*****************************************************************************
! Content:
! Source component of a simple example of ISO-3DFD implementation.
! leonardo.borges@intel.com
!****************************************************************************/

#ifndef _TOOLS_INCLUDE
#define _TOOLS_INCLUDE

#include <stddef.h>
#include <sys/time.h>
#include <math.h>

double walltime() // seconds
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    return ((double)(tv.tv_sec) +
            1e-06 * (double)(tv.tv_usec));
}

void random_data(float *data, const int dimx, const int dimy, const int dimz, const int lower_bound, const int upper_bound)
{
  srand(0);

  int ix, iy, iz;
  for(iz=0; iz<dimz; iz++)
    for(iy=0; iy<dimy; iy++)
      for(ix=0; ix<dimx; ix++) {
	*data = (float) (lower_bound + ( (rand()/(float)RAND_MAX) / (upper_bound - lower_bound)));
	++data;
      }
}

// naive and slow implementation
void reference_3D(float *output, float *input, float *vel, float *coeff,
		  const int dimx, const int dimy, const int dimz, const int radius)
{
  int dimxy = dimx*dimy;

  int ix, iy, iz, ir;
  for(iz=0; iz<dimz; iz++) {
    for(iy=0; iy<dimy; iy++) {
      for(ix=0; ix<dimx; ix++) {
	if( ix>=radius && ix<(dimx-radius) && iy>=radius && iy<(dimy-radius) && iz>=radius && iz<(dimz-radius) ) {
	  float value = 0.0;
	  value += (*input)*coeff[0];
	  for(ir=1; ir<=radius; ir++) {
	    value += coeff[ir] * (*(input+ir) + *(input-ir));	      // horizontal
	    value += coeff[ir] * (*(input+ir*dimx) + *(input-ir*dimx));   // vertical
	    value += coeff[ir] * (*(input+ir*dimxy) + *(input-ir*dimxy)); // in front / behind
	  }
	  *output = 2.0 * (*input) - (*output) + value* (*vel);
	}
	++output;
	++input;
	++vel;
      }
    }
  }
}

int within_epsilon(float* output, float *reference, const int dimx, const int dimy, const int dimz, const int radius, const int zadjust, const float delta)
{
  int retval = 1;
  float abs_delta = fabsf(delta);
  int ix, iy, iz;
  for(iz=0; iz<dimz; iz++) {
    for(iy=0; iy<dimy; iy++) {
      for(ix=0; ix<dimx; ix++) {
	if( ix>=radius && ix<(dimx-radius) && iy>=radius && iy<(dimy-radius) && iz>=radius && iz<(dimz-radius+zadjust) ) {
	  float difference = fabsf( *reference - *output);
	  if( difference > delta ) {
	    retval = 0;
	    printf(" ERROR: (%d,%d,%d)\t%.2f instead of %.2f\n", ix,iy,iz, *output, *reference);
	    return 0;
	  }
	}
	++output;
	++reference;
      }
    }
  }
  return retval;
}

#endif /*_TOOLS_INCLUDE */



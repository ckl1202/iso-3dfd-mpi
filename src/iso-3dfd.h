/*****************************************************************************
! Copyright(C) 2013 Intel Corporation. All Rights Reserved.
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
! Source component of a simple example of ISO-3DFD implementation for 
!   Intel(R) Xeon Phi(TM)
! Version V2.0
! leonardo.borges@intel.com
!****************************************************************************/

#ifndef _ISO_3DFD_INCLUDE
#define _ISO_3DFD_INCLUDE

/**** Verify results after one ietration? *******/
#include "mpi.h"
#define VERIFY_RESULTS
#define HALF_LENGTH 8

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define CEILING(X) (X-(int)(X) > 0 ? (int)(X+1) : (int)(X))

void iso_3dfd(float *next,  float *prev,  float *vel,   float *coeff,
	      const int n1, const int n2, const int n3, int nreps);

#endif /* _ISO_3DFD_INCLUDE */

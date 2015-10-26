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
! Source component of a simple example of ISO-3DFD implementation
!
!****************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "iso-3dfd.h"

/***************************************************************
 *
 * iso_3dfd_stencil: apply 8th order ISO stencil
 *
 ***************************************************************/
 void iso_3dfd_stencil (float *ptr_next, float *ptr_prev, float *ptr_vel, float *coeff,
	       	       const int n1, const int n2, const int n3) {
  const int n1n2 = n1*n2;

  float *prev  = ptr_prev;
  float *next  = ptr_next;
  float *vel   = ptr_vel;

  int i1, i2, i3, ir;
  for(i3=0; i3<n3; i3++) {
    for(i2=0; i2<n2; i2++) {
      for(i1=0; i1<n1; i1++) {
        if( i1>=HALF_LENGTH && i1<(n1-HALF_LENGTH) && i2>=HALF_LENGTH && i2<(n2-HALF_LENGTH) && i3>=HALF_LENGTH && i3<(n3-HALF_LENGTH) ) {
          float div = 0.0;
          div += (*prev)*coeff[0];
          for(ir=1; ir<=HALF_LENGTH; ir++) {
            div += coeff[ir] * (*(prev+ir) + *(prev-ir));         // horizontal
            div += coeff[ir] * (*(prev+ir*n1) + *(prev-ir*n1));   // vertical
            div += coeff[ir] * (*(prev+ir*n1n2) + *(prev-ir*n1n2)); // in front / behind
          }
          *next = 2.0 * (*prev) - (*next) + div * (*vel);
        }
        ++next;
        ++prev;
        ++vel;
      }
    }
  }
}

void iso_3dfd_stencil2 (float *ptr_next, float *ptr_prev, float *ptr_vel, float *coeff,
	       	       const int n1, const int n2, const int n3) {
  const int n1n2 = n1*n2;
  int delta = HALF_LENGTH * n1n2;

  float *prev  = ptr_prev + delta;
  float *next  = ptr_next + delta;
  float *vel   = ptr_vel + delta;

  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  int i1, i2, i3, ir;
  for(i3=HALF_LENGTH; i3< n3 / 4 + HALF_LENGTH; i3++) {
    for(i2=0; i2<n2; i2++) {
      for(i1=0; i1<n1; i1++) {
        if( i1>=HALF_LENGTH && i1<(n1-HALF_LENGTH) && i2>=HALF_LENGTH && i2<(n2-HALF_LENGTH) && i3>=HALF_LENGTH && i3<(n3/4+HALF_LENGTH) ) {
          if ((rank == 0) && (i3 - 2 * HALF_LENGTH < 0)) break;
          if ((rank == 3) && (i3 >= n3 / 4)) break;
          float div = 0.0;
          div += (*prev)*coeff[0];
          for(ir=1; ir<=HALF_LENGTH; ir++) {
            div += coeff[ir] * (*(prev+ir) + *(prev-ir));         // horizontal
            div += coeff[ir] * (*(prev+ir*n1) + *(prev-ir*n1));   // vertical
            div += coeff[ir] * (*(prev+ir*n1n2) + *(prev-ir*n1n2)); // in front / behind
          }
          *next = 2.0 * (*prev) - (*next) + div * (*vel);
        }
        ++next;
        ++prev;
        ++vel;
      }
    }
  }
}

void transfer(float *ptr, const int n1, const int n2, const int n3){
    MPI_Status status;
    int delta = n1 * n2 * HALF_LENGTH;
    int num = n1 * n2 * HALF_LENGTH;
    int n1n2n3div4 = n1 * n2 * (n3 / 4);
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    if (rank > 0){
        MPI_Send(ptr + delta, num, MPI_FLOAT, rank - 1, rank, MPI_COMM_WORLD);
    }
    if (rank < 3){
        MPI_Recv(ptr + delta + n1n2n3div4, num, MPI_FLOAT, rank + 1, rank + 1, MPI_COMM_WORLD, &status);
        MPI_Send(ptr + n1n2n3div4, num, MPI_FLOAT, rank + 1, rank, MPI_COMM_WORLD);
    }
    if (rank > 0){
        MPI_Recv(ptr, num, MPI_FLOAT, rank - 1, rank - 1, MPI_COMM_WORLD, &status);
    }
}

void iso_3dfd(float *ptr_next, float *ptr_prev, float *ptr_vel, float *coeff,
	      const int n1, const int n2, const int n3, int nreps) {
  int it;
  for(it=0; it<nreps; it+=2){
   	iso_3dfd_stencil ( ptr_next, ptr_prev, ptr_vel, coeff, n1, n2, n3);
	// here's where boundary conditions+halo exchanges happen

	// Swap previous & next between iterations
    iso_3dfd_stencil ( ptr_prev, ptr_next, ptr_vel, coeff, n1, n2, n3);

  } // time loop
}

void iso_3dfd2(float *ptr_next, float *ptr_prev, float *ptr_vel, float *coeff,
	      const int n1, const int n2, const int n3, int nreps) {
  int it;
  transfer(ptr_vel, n1, n2, n3);
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  for(it=0; it<nreps; it+=2){
    double wstart = walltime();
    transfer(ptr_prev, n1, n2, n3);
    double wend = walltime();
    float delta = wend - wstart;
    if (rank == 0) printf("%8.2f\n", delta);
   	iso_3dfd_stencil2( ptr_next, ptr_prev, ptr_vel, coeff, n1, n2, n3);
   	wend = walltime();
   	delta = wend - wstart;
   	if (rank == 0) printf("%8.2f\n", delta);
	// here's where boundary conditions+halo exchanges happen
    MPI_Barrier(MPI_COMM_WORLD);
    transfer(ptr_next, n1, n2, n3);
	// Swap previous & next between iterations
    iso_3dfd_stencil2 ( ptr_prev, ptr_next, ptr_vel, coeff, n1, n2, n3);
    MPI_Barrier(MPI_COMM_WORLD);
  } // time loop
}

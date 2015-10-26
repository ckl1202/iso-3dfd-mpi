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
! Source component of a simple example of ISO-3DFD implementation.
!
! leonardo.borges@intel.com
!
!****************************************************************************/
/* 1. align
 * 2. offset cacheline
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "iso-3dfd.h"
#include "tools.h"

int main(int argc, char** argv)
{
  MPI_Init(&argc, &argv);

  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  // Defaults
  int n1 = 528;   // First dimension
  int n2 = 528;   // Second dimension
  int n3 = 736;   // Third dimension
  int nreps = 2;     // number of time-steps, over which performance is averaged

  if( (argc > 1) && (argc < 4) ) {
    printf(" usage: [n1 n2 n3] [# iterations]\n");
    exit(1);
  }
  // [n1 n2 n3]
  if( argc >= 4 ) {
    n1  = atoi(argv[1]);
    n2  = atoi(argv[2]);
    n3  = atoi(argv[3]);
  }
  //  [# iterations]
  if( argc >= 5)
    nreps = atoi(argv[4]);

  // Make sure nreps is rouded up to next even number (to support swap)
  nreps = ((nreps+1)/2)*2;

  //printf("n1=%d n2=%d n3=%d nreps=%d\n",n1, n2, n3, nreps);

  float coeff[HALF_LENGTH+1] = {
    -9.164532312924e-1,
    +1.777777777777e-1,
    -3.111111111111e-1,
    +7.542087542087e-2,
    -1.767676767676e-2,
    +3.480963480963e-3,
    -5.180005180005e-4,
    +5.074290788576e-5,
    -2.428127428127e-6};

// Data Arrays
  float *prev=NULL, *next=NULL, *vel=NULL;

  // variables for measuring performance
  double wstart, wstop;
  float elapsed_time=0.0f, throughput_mpoints=0.0f, mflops=0.0f;

  // allocate data memory
  int nbytes;
  if (rank == 0) nbytes = n1*n2*(n3 +HALF_LENGTH)*sizeof(float);
  else nbytes = n1*n2*(n3/4 + 2*HALF_LENGTH)*sizeof(float);

  printf("allocating prev, next and vel: total %g Mbytes\n",(3.0*(nbytes+16))/(1024*1024));fflush(NULL);

  prev = (float*)malloc(nbytes);
  next = (float*)malloc(nbytes);
  vel  = (float*)malloc(nbytes);

  if( prev==NULL || next==NULL || vel==NULL ){
    printf("couldn't allocate CPU memory prev=%p next=%p vel=%p\n",prev, next, vel);
    printf("  TEST FAILED!\n"); fflush(NULL);
    exit(-1);
  }
  printf ("pointers : prev: 0x%x, next: 0x%x, vel: 0x%x\n", prev, next, vel);
  memset(prev, 0, nbytes);
  memset(next, 0, nbytes);
  memset(vel,  0, nbytes);

  // A couple of run to start threading library

  int tmp_nreps=2;
  iso_3dfd2(next, prev, vel, coeff, n1, n2, n3, tmp_nreps);

  // Fill array with random data.
  int delta = n1 * n2 * HALF_LENGTH;
  int n1n2n3div4 = n1 * n2 * (n3 / 4);
  if (rank == 0){
    random_data( prev + delta, n1, n2, n3, 1, 5 );
    random_data( next + delta, n1, n2, n3, -2, 2 );
    random_data( vel + delta,  n1, n2, n3, 2, 7 );
    int i;
    for (i = 1; i < 4; ++i){
        MPI_Send(prev + delta + i * n1n2n3div4, n1n2n3div4, MPI_FLOAT, i, i, MPI_COMM_WORLD);
        MPI_Send(next + delta + i * n1n2n3div4, n1n2n3div4, MPI_FLOAT, i, i, MPI_COMM_WORLD);
        MPI_Send(vel + delta + i * n1n2n3div4, n1n2n3div4, MPI_FLOAT, i, i, MPI_COMM_WORLD);
    }
  }
  else{
    MPI_Status status;
    MPI_Recv(prev + delta, n1n2n3div4, MPI_FLOAT, 0, rank, MPI_COMM_WORLD, &status);
    MPI_Recv(next + delta, n1n2n3div4, MPI_FLOAT, 0, rank, MPI_COMM_WORLD, &status);
    MPI_Recv(vel + delta, n1n2n3div4, MPI_FLOAT, 0, rank, MPI_COMM_WORLD, &status);
  }

  wstart = walltime();
  unsigned __int64 t1 = _rdtsc();

  iso_3dfd2(next, prev, vel, coeff, n1, n2, n3, nreps);

  unsigned __int64 t2 = _rdtsc();
  wstop =  walltime();

  // report time
  elapsed_time = wstop - wstart;
  float normalized_time = elapsed_time/nreps;
  throughput_mpoints = ((n1-2*HALF_LENGTH)*(n2-2*HALF_LENGTH)*(n3-2*HALF_LENGTH))/(normalized_time*1e6f);
  mflops = (7.0f*HALF_LENGTH + 5.0f)* throughput_mpoints;

  /*printf("-------------------------------\n");
  printf("Cycles:       %ld \n", (t2 - t1));
  printf("time:       %8.2f sec\n", elapsed_time );
  printf("throughput: %8.2f MPoints/s\n", throughput_mpoints );
  printf("flops:      %8.2f GFlops\n", mflops/1e3f );
*/
  free(prev);
  free(next);
  free(vel);

#if defined(VERIFY_RESULTS)
  if (rank == 0){
  nbytes = n1*n2*n3*sizeof(float);
  prev = (float*)malloc(nbytes);
  next = (float*)malloc(nbytes);
  vel  = (float*)malloc(nbytes);

  // check the correctness of one iteration
  printf("\n-------------------------------\n");
  printf("comparing one iteration to reference implementation result...\n");

  random_data( prev, n1, n2, n3, 1, 5 );
  random_data( next, n1, n2, n3, -2, 2 );
  random_data( vel,  n1, n2, n3, 2, 7 );

  nreps=2;
  iso_3dfd(next, prev, vel, coeff, n1, n2, n3, nreps);

  float *p_ref = (float*)malloc(n1*n2*n3*sizeof(float));
  if( p_ref==NULL ){
    printf("couldn't allocate memory for p_ref\n");
    printf("  TEST FAILED!\n"); fflush(NULL);
    exit(-1);
  }
  printf("calling the scalar implementation...\n");

  random_data( prev, n1, n2, n3, 1, 5 );
  random_data( p_ref, n1, n2, n3, -2, 2 );
  reference_3D( p_ref, prev, vel, coeff, n1, n2, n3, HALF_LENGTH );
  if( within_epsilon( next, p_ref, n1, n2, n3, HALF_LENGTH, 0, 0.0001f ) ) {
    printf("  Result within epsilon\n");
    printf("  TEST PASSED!\n");
  } else {
    printf("  Incorrect result\n");
    printf("  TEST FAILED!\n");
  }
  free(p_ref);

#endif /* VERIFY_RESULTS */

  free(prev);
  free(next);
  free(vel);
  }
  MPI_Finalize();
}

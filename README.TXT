3D Finite Difference Stencil
============================

Simple implementation for 3D FD stencil: 16-th order (49 points) with symmetric
and constant coefficients.

Original source from Leo Borges (SSG) - leonardo.borges@intel.com
Modified by:  Naveen Mellempudi (PCL-India) 

Key files:
=========
iso-3dfd.h	    Definition
iso-3dfd_main.c     Main driver
iso-3dfd_stencil.c  Main function

Algorithm:
=========
Perform isotopic 49-pt 3D stencil on one input data structure and write to a second data structure.   
Swap the two data structure and repeat for niterations.

Build:
=====
Do make: make model={cpu|mic}
Executable Binary: iso-3dfd_hl-8.out

Run:
===
See run.sh script file.

=====================================================
Reference Output 1(from 2-socket Xeon 2697v2 2.7GHz):
=====================================================
[root@crt02 iso-3dfd]# ./iso-3dfd_hl-8.cpu 528 528 736 2
n1=528 n2=528 n3=736 nreps=2
allocating prev, next and vel: total 2348.16 Mbytes
pointers : prev: 0x54e6a010, next: 0x23fb1010, vel: 0xf30f8010
-------------------------------
Cycles:       77581250136
time:          28.80 sec
throughput:    13.11 MPoints/s
flops:          0.80 GFlops

-------------------------------
comparing one iteration to reference implementation result...
calling the scalar implementation...
  Result within epsilon
  TEST PASSED!


========================================
Reference Output 2(from Xeon Phi 7120a):
========================================
[root@crt02-mic0 iso-3dfd]# ./iso-3dfd_hl-8.mic 528 528 736 2
n1=528 n2=528 n3=736 nreps=2
allocating prev, next and vel: total 2348.16 Mbytes
pointers : prev: 0xf14a9010, next: 0xc05f0010, vel: 0x8f737010
-------------------------------
Cycles:       932422757726
time:         756.67 sec
throughput:     0.50 MPoints/s
flops:          0.03 GFlops

-------------------------------
comparing one iteration to reference implementation result...
calling the scalar implementation...
  Result within epsilon
  TEST PASSED!


##*****************************************************************************
## Copyright(C) 2013 Intel Corporation. All Rights Reserved.
##
## The source code, information  and  material ("Material") contained herein is
## owned  by Intel Corporation or its suppliers or licensors, and title to such
## Material remains  with Intel Corporation  or its suppliers or licensors. The
## Material  contains proprietary information  of  Intel or  its  suppliers and
## licensors. The  Material is protected by worldwide copyright laws and treaty
## provisions. No  part  of  the  Material  may  be  used,  copied, reproduced,
## modified, published, uploaded, posted, transmitted, distributed or disclosed
## in any way  without Intel's  prior  express written  permission. No  license
## under  any patent, copyright  or  other intellectual property rights  in the
## Material  is  granted  to  or  conferred  upon  you,  either  expressly,  by
## implication, inducement,  estoppel or  otherwise.  Any  license  under  such
## intellectual  property  rights must  be express  and  approved  by  Intel in
## writing.
##
## *Third Party trademarks are the property of their respective owners.
##
## Unless otherwise  agreed  by Intel  in writing, you may not remove  or alter
## this  notice or  any other notice embedded  in Materials by Intel or Intel's
## suppliers or licensors in any way.
##
##*****************************************************************************
## Content:
##      Build and run Intel(R) Xeon Phi(TM) ISO-3DFD example. V2.0
##      ! leonardo.borges@intel.com
##*****************************************************************************

# default is to build 8th order in space
HALF_LENGTH?=8
EXEC_NAME	=	iso-3dfd_hl-$(HALF_LENGTH)

CC		=	icc
FC		=	ifort
LD		=	$(CC)
MAKE		=	make

ifeq ($(model),cpu)  # Flags for CPU 
CFLAGS		=	-g -restrict -no-ip -ansi-alias -fno-alias
CPPFLAGS	=	$(CFLAGS)
LFLAGS		=	
LDFLAGS		=	$(LFLAGS)
EXEC_POSTFIX	=	.cpu

else 
ifeq ($(model),mic)  # Flags for MIC 
OPT_NO_FUSION	=
OPT_REPORT	=

CFLAGS		=	-g -restrict -mmic -no-ip -ansi-alias -fno-alias
CPPFLAGS	=	$(CFLAGS) 
LFLAGS		=	-mmic
LDFLAGS		=	$(LFLAGS)
EXEC_POSTFIX	=	.mic

else
.SILENT:
err:
	echo "Use make model={cpu|mic}"
	echo "     cpu:     CPU standonly model"
	echo "     mic:     MIC native model"
endif
endif

C_OBJS		=	src/iso-3dfd_main.o src/iso-3dfd_stencil.o
OBJS		=	$(C_OBJS)
EXEC		=	$(EXEC_NAME)$(EXEC_POSTFIX)

.SUFFIXES:
.SUFFIXES: .cc .c .o

all:	$(EXEC)
	@rm -f $(C_OBJS)

$(EXEC): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)

$(C_OBJS):
	@cd ${@D}; \
	$(MAKE) CC=$(CC) ${@F}

clean:
	@echo Cleaning Files
	rm -f $(OBJS) $(OBJS:.o=MIC.o) $(EXEC_NAME).cpu $(EXEC_NAME).mic *.xnlog

.cc.o:
	$(CC) $(CPPFLAGS) -c -o $@ $<

.c.o:
	$(CC) $(CPPFLAGS) -c -o $@ $<

#
#
#

PROG = make

ifdef WATCOM
TARGET = watcom
FILE   = makefile.w11
endif

ifdef DJGPP
TARGET = djgpp
FILE   = makefile.dj
endif

ifndef TARGET

ifeq ($(SYSNAME),qnx4)
TARGET = qnx
FILE   = makefile.qnx
PROG = gmake
else
ifeq ($(OSTYPE),Linux)
TARGET = linux
FILE   = makefile.lin
else
ifeq ($(OSTYPE),linux-gnu)
TARGET = linux
FILE   = makefile.lin
else
ifeq ($(OSTYPE),Linux-gnu)
TARGET = linux
FILE   = makefile.lin
else
ifeq ($(OSTYPE),solaris)
TARGET = solaris
FILE   = makefile.sol
else
TARGET = linux
FILE   = makefile.lin
endif
endif
endif
endif
endif
endif

all: 
	$(PROG) -f $(FILE)
	
clean: 
	make -f $(FILE) clean

linux:
	make -f makefile.lin
	
qnx:
	gmake -f makefile.qnx
	
qcc:
	gmake -f makefile.qcc
	
watcom:
	wmake -f makefile.w11

djgpp:
	make -f makefile.dj
	
rtems:
	make -f makefile.rtems

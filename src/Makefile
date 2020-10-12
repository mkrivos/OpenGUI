include ../ostest.mak
include ../config.mak
include ../cflags.mak

ifndef BPP
BPP = 32
LIBNAME	= fgl
endif

COLORDEPTH	= -DBPP$(BPP)
STARGET		= lib$(LIBNAME).a
DTARGET		= lib$(LIBNAME).so

STARGETF = ../lib/$(STARGET)
LIBCORE = ../lib/libFGCore$(BPP).a
SO = ../obj/gcc/$(BPP)

ifeq ($(OS),solaris)
OS=solaris
solaris_dirs := solaris x11
os_include := -I/usr/X/include
os_lib := -L/usr/X/lib
endif

ifeq ($(OS),win32)
OS=win32
win32_dirs := win32
os_include :=
os_lib :=
endif

ifeq ($(OS),linux)
OS=linux
linux_dirs := linux x11
os_include := -I/usr/X11R6/include
os_lib := -L/usr/X11R6/lib64 -L/usr/X11R6/lib

DTARGETF = ../lib/$(DTARGET).4

ifeq ($(BPP),8)
ifdef GLSUPPORT
OPENGL_SUPPORT = ../lib/libFGMesa.a 
OPENGL_SUPPORT_LO = ../lib/libFGMesa.so.3
endif
endif
endif

dirs := . agg2 ../include/fastgl

all_dirs := $(dirs) $(linux_dirs) $(solaris_dirs) $(win32_dirs)

all_files := $(foreach dir,$(all_dirs),$(wildcard $(dir)/*.cpp))
all_files := $(subst ./,, $(all_files))

all_deps := $(subst .cpp,.d, $(all_files) )

all_objs := $(subst .cpp,.o, $(all_files) )
all_objs := $(addprefix $(SO)/, $(all_objs) )

all_lobjs := $(subst .cpp,.lo, $(all_files) )
all_lobjs := $(addprefix $(SO)/, $(all_lobjs) )

fgl_inc := $(patsubst %,-I%, $(all_dirs)) -I../xml

HDRS = #$(foreach dir,$(dirs),$(wildcard $(dir)/*.h)) # _fastgl.h config.h internal.h base.h fastgl.h

vpath %.o $(SO) ../mesa
vpath %.cpp ../mesa
vpath %.lo $(SO)
vpath %.h $(all_dirs)

SOURCES = $(all_files)
OBJS = $(all_objs)
LOBJS = $(all_lobjs)

OPTIONS = -DINTO_FGL
INCPATH	= $(fgl_inc) $(os_include)
LIBPATH	= $(os_lib)

COREOBJS = fgbase.o fgbitmap.o fgcolor.o fgconcurency.o fgdrawbuffer.o fgfontmanager.o fgmatrix.o fgshape.o resource.o
COREOBJS := $(addprefix $(SO)/, $(COREOBJS) )

ifdef THREADSUPPORT
OPTIONS	+= -DFG_THREADED
endif

ifdef X11SUPPORT
OPTIONS	+= -DX11_DRIVER
endif

ifdef DGASUPPORT
OPTIONS	+= -DDGA_DRIVER
endif

ifdef PNGSUPPORT
OPTIONS	+= -DFG_PNGx
endif

ifdef JPGSUPPORT
OPTIONS	+= -DFG_JPEGx
endif

ifdef TIFFSUPPORT
OPTIONS	+= -DFG_TIFF
endif

ifdef TTFSUPPORT
OPTIONS	+= -DFG_TTF
INCPATH += -I/usr/include/freetype2 -I/usr/sfw/include/ -I/usr/sfw/include/freetype2
endif

CFLAGS += $(COLORDEPTH) $(OPTIONS) $(INCPATH)
CXXFLAGS = $(CFLAGS)

all: $(all_deps) $(STARGETF) $(DTARGETF) $(LIBCORE) $(OPENGL_SUPPORT) $(OPENGL_SUPPORT_LO)

$(all_deps):
	    	
$(STARGETF): $(OBJS)
	ar rcs $(STARGETF) $(OBJS)
	@echo
	@echo '########     Static version: $(STARGET) #######'
	@echo

$(DTARGETF): $(LOBJS)
	$(CC) -o $(DTARGETF) -L../lib -g -fnostdinc -shared -Wl,-soname,$(subst %.4,,$(DTARGET)) $(LOBJS) $(LIBS) $(LIBPATH) $(X11SUPPORT) $(DGASUPPORT) $(JPGSUPPORT) $(PNGSUPPORT) $(TTFSUPPORT) $(THREADSUPPORT) $(TIFFSUPPORT)
	@echo
	@echo '########     Dynamic version: $(DTARGET) #######'
	@echo

$(LIBCORE): 
	ar rcs $(LIBCORE) $(COREOBJS)
	@echo
	@echo '########     Static version: $(LIBCORE) #######'
	@echo


$(OPENGL_SUPPORT): ../mesa/fglmesa.o
	@echo 'Building library for Mesa3D support ...'
	ar   rcs $@ $<

$(OPENGL_SUPPORT_LO): ../mesa/fglmesa.lo
	$(CXX) -L/usr/X11R6/lib64 -L/usr/X11R6/lib -g -fnostdinc -shared -Wl,-soname,libFGMesa.so -o ../lib/libFGMesa.so.1 $< -lOSMesa

define DEPENDENCY
	echo -n $(@:%.o=%.d) '' > $(@:%.o=%.d) && \
	$(CXX) $(CFLAGS) -MM -MG -P -w -fmessage-length=0  $< >> $(@:%.o=%.d)
endef

define DEPENDENCY_LO
	echo -n $(@:%.lo=%.d) '' > $(@:%.lo=%.d) && \
	$(CXX) $(CFLAGS) -MM -MG -P -w -fmessage-length=0  $< >> $(@:%.lo=%.d)
endef

$(SO)/%.o: %.cpp $(HDRS)   
	$(CXX) $(CFLAGS) -o $@ $< 
	$(DEPENDENCY)
	@echo

$(SO)/%.lo: %.cpp $(HDRS)  
	$(CXX) $(CFLAGS) $(DSO) -o $@  $<
	$(DEPENDENCY_LO)
	@echo

%.o:  %.cpp $(HDRS)   
	$(CXX) $(CFLAGS) -o $@ $< 
	$(DEPENDENCY)
	@echo

%.lo:  %.cpp $(HDRS)  
	$(CXX) $(CFLAGS) $(DSO) -o $@  $<
	$(DEPENDENCY_LO)
	@echo

#%.d: %.cpp

-include $(all_deps)

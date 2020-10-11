# debug
OPTS := -fno-exceptions -fno-rtti -s -I/usr/local/include -L/usr/local/lib -L/usr/X11R6/lib64
COLORMODE := 32

include ../ostest.mak

ifeq ($(OS),win32)
LIBS = -lgdi32 -luser32 -lddraw -ldxguid
endif

ifeq ($(OS),solaris)
LIBS = -lXext -lX11 -ltiff -lrt -L/usr/sfw/lib
endif

LIBS += -lm -ltinycfg -lstdc++ -ljpeg 

# USER defined opts
ifeq ($(wildcard $(target)*.options),$(target).options)
include $(target).options
endif

$(target): $(target).cpp
ifneq ($(COLORMODE),8)
	gcc $(OPTS) -o $(target) $(target).cpp -lfgl$(COLORMODE) -DBPP$(COLORMODE) $(LIBS)
else
	gcc $(OPTS) -o $(target) $(target).cpp -lfgl -DBPP$(COLORMODE) $(LIBS)
endif
	chown 0 $(target)
	chmod 4755 $(target)

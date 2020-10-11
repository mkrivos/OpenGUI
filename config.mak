#########################################################
#
# configuration to make OpenGUI library
#
#########################################################

# 
# enable X11 support
#

X11SUPPORT	=  -lXext -lX11

# 
# enable X11 DGA2 support
#

#DGASUPPORT	= -lXxf86dga -lXext -lX11

#
# enable PNG file format
#

#PNGSUPPORT	= -lpng -lz

#
# enable TIFF file format
#

#TIFFSUPPORT	= -ltiff -ljpeg -lz

#
# enable JPEG file format (needed by examples)
#

JPGSUPPORT	= -ljpeg

#
# enable FREETYPE2 support
#

#TTFSUPPORT	= -lfreetype

#
# enable THREAD-SAFE improvements (if you can acceas library from more threads only)
#

THREADSUPPORT	= -lpthread

#
# enable OpenGL support (MesaGL 3 & above)
#

GLSUPPORT = YES

#########################################################


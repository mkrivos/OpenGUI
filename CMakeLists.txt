cmake_minimum_required(VERSION 3.18)
project(OpenGUI)

ADD_DEFINITIONS ( -O2 -fno-exceptions -DFG_THREADED -D_REENTRANT -DINTO_FGL -DFG_TTF -DFG_PNGx -DFG_JPEGx -DX11_DRIVER ) 

SET (FGL_SRCS
src/detect.cpp
src/dll.cpp
src/drivers.cpp
src/fastgl.cpp
src/fgapp.cpp
src/fgbase.cpp
src/fgbitmap.cpp
src/fgcolor.cpp
src/fgconcurency.cpp
src/fgcontrols.cpp
src/fgdrawbuffer.cpp
src/fgeditor.cpp
src/fgevent.cpp
src/fgfontmanager.cpp
src/fggradient.cpp
src/fglistbox.cpp
src/fgmatrix.cpp
src/fgpane.cpp
src/fgscheme.cpp
src/fgshape.cpp
src/fgtimer.cpp
src/fgttf.cpp
src/fgx.cpp
src/listbox.cpp
src/resource.cpp
src/widgets.cpp
src/linux/linkeyb.cpp
src/linux/linux.cpp 
src/agg2/agg_rasterizer_scanline_aa.cpp
src/agg2/agg_sqrt_tables.cpp
src/agg2/agg_trans_affine.cpp )

INCLUDE_DIRECTORIES ( xml src include/fastgl src/linux src/agg2 /usr/X11R6/include /usr/include/freetype2 /opt/csw/include /opt/csw/include/freetype2 )

ADD_LIBRARY ( fgl     EXCLUDE_FROM_ALL ${FGL_SRCS})
ADD_LIBRARY ( fgl15   EXCLUDE_FROM_ALL ${FGL_SRCS})
ADD_LIBRARY ( fgl16   EXCLUDE_FROM_ALL ${FGL_SRCS})
ADD_LIBRARY ( fgl32   ${FGL_SRCS})

SET_TARGET_PROPERTIES( fgl    PROPERTIES COMPILE_FLAGS "${CFLAGS} -DBPP8" )
SET_TARGET_PROPERTIES( fgl16  PROPERTIES COMPILE_FLAGS "${CFLAGS} -DBPP16" )
SET_TARGET_PROPERTIES( fgl15  PROPERTIES COMPILE_FLAGS "${CFLAGS} -DBPP15" )
SET_TARGET_PROPERTIES( fgl32  PROPERTIES COMPILE_FLAGS "${CFLAGS} -DBPP32" )

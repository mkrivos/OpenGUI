cmake_minimum_required(VERSION 3.18)
project(OpenGUI)

set(BUILD_SHARED_LIBS ON)

ADD_DEFINITIONS (-fno-exceptions  ) 

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
src/agg2/agg_trans_affine.cpp 
xml/tinystr.cpp
xml/tinyxml.cpp
xml/tinyxmlerror.cpp
xml/tinyxmlparser.cpp
xml/xmlconfig.cpp
)

INCLUDE_DIRECTORIES ( xml src include/fastgl src/linux src/agg2 /usr/X11R6/include /usr/include/freetype2 /opt/csw/include /opt/csw/include/freetype2 )

ADD_LIBRARY(fgl32   ${FGL_SRCS})
target_include_directories(fgl32 
    PRIVATE
         xml src src/linux src/agg2 
    PUBLIC
         include
)
target_compile_definitions(fgl32
    PRIVATE
        INTO_FGL 
        X11_DRIVER
    PUBLIC
        FG_THREADED 
        D_REENTRANT 
        FG_TTF 
        FG_PNGx 
        FG_JPEGx 
        FASTGL_BPP=32
        #TRUE_COLORS
)
target_link_libraries(fgl32 
    PUBLIC
        Xext
        X11
        freetype
        pthread
        dl
        m
    )

set(target hello)
add_executable(${target} examples/${target}/${target}.cpp)
target_link_libraries(${target} fgl32)

set(target alpha)
add_executable(${target} examples/${target}/${target}.cpp)
target_link_libraries(${target} fgl32)

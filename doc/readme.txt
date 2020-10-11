***************************************************************************
***
**		README for OpenGUI Graphics Library
***
***************************************************************************

                         What is OpenGUI library? 
  
OpenGUI is a high-Level C/C++ graphics library built upon a fast, low-level x86 
asm graphics kernel. OpenGUI provides 2D drawing primitives and an event-driven 
windowing API for easy application development, and it supports the BMP image 
file format. It's very powerful, but very easy to use. You can write apps 
in the old Borland BGI style or in a windowed style like QT. OpenGUI supports 
the keyboard and mouse as event sources, and the Linux framebuffer/svgalib 
as drawing backends. Mesa3D is also supported under Linux. 
On now are available only 8, 15, 16 and 32-bit color modes.

That's not specifically for games but you can use it to create a game gui 
or at least peruse the code to get an idea on how to implement a windowing 
framework in your own game engine. 

                                 FEATURES

- ultra-fast (asm kernel & MMX support) 
- the most robust library for Linux FrameBuffer 
- support for resolutions ranging from 320x200 to 1600x1200 
- windowing system... 
- object-oriented multi-platform API (DJGPP, WATCOM, GCC, LINUX, QNX, SOLARIS) 
- full application development environment (configuration file, file dialogs, etc.) 
- professionally tested & actively used for one year (see examples) 
- a tool for interactive code generation (draw & run) 
- and much more 

                                   INTRO

The library consists of three layers. The first layer is a hand-coded and fast 
assembler kernel. This layer does the biggest piece of hard work. The second 
layer implements the API for drawing graphics primitives like lines, rectangles, 
circles etc. This layer is comparable to Borland BGI API. The third layer is 
written with C++ and offers a complete object set for the GUI developer. 
The third layer implements objects like input windows, buttons, menus, bitmaps 
etc, with addition of integrated mouse & keyboards support. 


-------------------------- INSTALL ----------------------------------------
To installing OpenGUI library (you must be root!) type : 

	cd fastgl/src
	make 

NOTE! to compile you must have NASM (Net wide assembler) or WASM (for Watcom) !
NOTE! under QNX OS you must run 'int10 &' before
NOTE! if your mouse under LINUX not work correctly, you must change file
      /etc/vga/libvga.config to disable 'mouse_accel' option

You can type the same command [make] as above in the directory 'rad' 
and 'examples'. To compile this library you can/must use these tools:

MS_DOS:	GCC & NASM (RHIDE compatible (I think "in graphics modes"))
	WATCOM C++ & WASM (WD Debugger compatible)
QNX:	WATCOM C++ & WASM
LINUX:  GCC (or EGCS or PGCC) & NASM (0.97) & svgalib developer package
SOLARIS:GCC
---------------------------------------------------------------------------

 You must have nasm 0.98.23 (worldwide net assembler). 
 Also you must have svgalib library. (I use from one only little bit ..)
 Get it from ftp://metalab.unc.edu/pub/Linux/libs/graphics/svgalib*
 Installing it is another story on another time.
	
-------------------------- WARNING ----------------------------------------

If you use this library under LINUX as shared, don't forget to:
	    
		 "-fPIC" compiler options

---------------------------------------------------------------------------
Who am I?

 My name is Marian Krivos. I live at the moment in Slovakia.
 Here is my address:
                         Marian Krivos
			 NABREZIE E/2
			 LIPT. MIKULAS
			 031 01
			 SLOVAKIA	
email:
			nezmar@atlas.sk



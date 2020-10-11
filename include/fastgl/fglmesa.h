/*
    FGLMESA:  An example of integrating FGL with Mesa for OpenGL programs
    Copyright (C) 1997  Sam Lantinga, 1999 Marian Krivos

    OpenGUI - Drawing & Windowing library

	Copyright (C) 1996,2004  Marian Krivos

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
	License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

	You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    Sam Lantinga
    5635-34 Springhouse Dr.
    Pleasanton, CA 94588 (USA)
    slouken@devolution.com

    Marian Krivos
    nezmar@atlas.sk

  $Id: fglmesa.h 2086 2005-05-12 10:52:34Z majo $

  $Log$
  Revision 1.1  2005/05/12 10:52:38  majo
  Initial revision

  Revision 1.1.1.1  2004/01/22 16:25:08  majo
  initial release

*/

#ifndef _FGL_Mesa_h
#define _FGL_Mesa_h

#ifdef __BORLANDC__
#pragma option push -b
#pragma option push -a4
#endif

#ifdef __BORLANDC__
#ifdef __linux__
#pragma link "FGMesa.a"
#pragma link "libOSMesa.so"
#pragma link "libGL.so"
#else
//#pragma link "MesaGL.lib"// don't work properly - buggy BC++ 6 linker????
#endif
#endif

/* This integrates with Mesa using the OSMesa extensions for GL output */
#ifndef WINGDIAPI
#define WINGDIAPI extern
#define APIENTRY
#endif    

#include <GL/osmesa.h>

#ifdef FG_NAMESPACE
namespace fgl {
#endif

/**
The wrapper to MesaGL (offscreen SW rendering).
The basic is this surface. You can render the 3D scene
into the Windowed or full-screen mode. In the Windowed mode
there is one GL context for each Window. The mode is taken
from the type of passed Window - if the Window is of the type ROOTWND,
the rendering will runs in the full-screen mode.
*/
class GLSurface : public FGConnector
{
		void *glsurface;
		OSMesaContext context;
		DrawBuffer *db;
		int h,w;
		int ok;
		int type;
	public:
		GLSurface(DrawBuffer *, int ww=0, int hh=0, bool direct_access=0);
		int MakeCurrent(void);
		void DestroyContext(void);
		void Draw(void);
};

#ifdef FG_NAMESPACE
}
#endif

#ifdef __BORLANDC__
#pragma option pop /* pop -a switch */
#pragma option pop /* pop -b */
#endif

#endif /* _FGL_Mesa_h */


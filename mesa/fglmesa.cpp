/*
    FGLMESA:  An example of integrating OpenGUI with MesaGL for OpenGL programs
    Copyright (C) 1997  Sam Lantinga

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

  $Id: fglmesa.cpp 2086 2005-05-12 10:52:34Z majo $

  $Log$
  Revision 1.1  2005/05/12 10:52:35  majo
  Initial revision

  Revision 1.2  2004/02/29 19:58:20  majo
  devel state

  Revision 1.1.1.1  2004/01/22 16:25:08  majo
  initial release

*/

#include "fastgl.h"
#include "_fastgl.h"
#include "fglmesa.h"

#ifdef FG_NAMESPACE
namespace fgl {
#endif

/**
	Creates and initializes an OpenGL context (via MesaGL library) for 3D rendering.
	This context is connected with DrawBuffer or Window (as child of DrawBuffer).
	The size of rendered surface is taken from the size of connected DrawBuffer
	or size is explicitly passed to the constructor.
	@param d the pointer to the connected surface
	@param ww width of surface (less or equal to the connected surface)
	@param hh height of surface (less or equal to the connected surface)
	@param direct_access if true then rendering will be performed into the VIDEORAM directly, otherwise into RAM buffer. In some cases it takes some more FPS.
*/
GLSurface::GLSurface(DrawBuffer *d, int ww, int hh, bool direct_access)
	:db(d)
{
	context = 0;
	ok = 0;
	if (ww==0)
	{
		w = db->GetWW();
		h = db->GetHW();
	}
	else
	{
		w = ww;
		h = hh;
	}

	if (fgstate.verbose)
	{
		 printf("GL_VERSION    = %d.%d\n", OSMESA_MAJOR_VERSION, OSMESA_MINOR_VERSION);
//		 printf("GL_RENDERER   = %s\n", (char *) glGetString(GL_RENDERER));
//		 printf("GL_VENDOR     = %s\n", (char *) glGetString(GL_VENDOR));
//		 printf("GL_EXTENSIONS = %s\n", (char *) glGetString(GL_EXTENSIONS));
	}

	// allocate own buffer if size differs only
	if (direct_access == 1)
	{
		// force buffer addr to be VRAM addr
		glsurface = videobase;
	}
	else
	{
		glsurface = (char *)db->GetArray() + (db->GetYW()*db->GetW() + db->GetXW())*bpp;
	}

	assert(glsurface);

	/* Create a GL context */
	GLenum format = (GLenum)0;

	/* Figure out the Mesa visual format */
	switch (get_colordepth())
	{
	case 8:
		format = OSMESA_COLOR_INDEX;
		break;
#if (OSMESA_MAJOR_VERSION >= 4)
	case 16:
		format = OSMESA_RGB_565;
		break;
#endif
	case 32:
		format = OSMESA_RGBA;
		break;
	default:
		printf("FGMesa: Bad format %d bits - upgrade MesaGL up to 4.x please (now %d)\n", get_colordepth(), OSMESA_MAJOR_VERSION);
	  exit(-1);
		return;
	}

	type = format;

#if (OSMESA_MAJOR_VERSION >= 4)
	context = OSMesaCreateContextExt(format, 16, 8, get_colordepth()==8?0:16, NULL);
#else
	context = OSMesaCreateContext(format, NULL);
#endif

	if ( context == 0 )
	{
		printf("OSMesaCreateContext() failed!\n");
		return;
	}

	/* Make it current */
	if ( !MakeCurrent() )
	{
		DestroyContext();
		return;
	}
	/* That's it!  We're ready to render */
	ok = 1;

	// disable SCISSORS for MUCH faster windowed rendering
	if (db->GetType() == DrawBuffer::WINDOW)
		glEnable (GL_SCISSOR_TEST);
}

/**
	Draws rendered scene on the screen if needed.
*/
void GLSurface::Draw(void)
{
	if (ok && glsurface != (void *)videobase )
		if (db->GetType() == DrawBuffer::WINDOW || db->GetType() == DrawBuffer::ROOTWINDOW)
		{
//			((Window *)db)->frame();
			((Window *)db)->WindowRepaintUser(0, 0, w, h );
		}
}

/**
	Makes this GL context current.
	See an OpenGL architecture guide for more details.
*/
int GLSurface::MakeCurrent(void)
{
	/* Make the context current */
	if ( ! OSMesaMakeCurrent(
			context,
			glsurface,
#if (OSMESA_MAJOR_VERSION >= 4)
			type==OSMESA_RGB_565?GL_UNSIGNED_SHORT_5_6_5:GL_UNSIGNED_BYTE,
#else
			GL_UNSIGNED_BYTE,
#endif
			w, h ))
	{
		printf("OSMesaMakeCurrent() failed!\n");
		return(GL_FALSE);
	}

	/* Tell OSMesa about the surface pitch */
	/* Note: Mesa 3.0 has a bug where OSMESA_ROW_LENGTH signifies both
			 length in bytes and length in words in different places.
			 For now, leave this commented and hope it doesn't break,
			 or Mesa will crash.
	*/
	OSMesaPixelStore(OSMESA_ROW_LENGTH, db->GetW());

	/* Tell OSMesa that we use top-down bitmaps */
	OSMesaPixelStore(OSMESA_Y_UP, 0);

	/* Yay!  We're done! */
	return(GL_TRUE);
}

/**
	Destroy GL context.
	See an OpenGL architecture guide for more details.
*/
void GLSurface::DestroyContext(void)
{
	OSMesaDestroyContext(context);
}

#ifdef FG_NAMESPACE
}
#endif


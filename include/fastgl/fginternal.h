/*
  $Id: fginternal.h 2524 2005-07-04 06:40:22Z majo $

  $Log$
  Revision 1.2  2005/07/04 06:40:22  majo
  QtCore

  Revision 1.1.1.1  2005/05/12 10:52:38  majo
  i

  Revision 1.8  2005/02/11 14:22:01  majo
  *** empty log message ***

  Revision 1.7  2004/12/10 07:52:22  majo
  documentation update
  fastcall & wakref optimizations removed!!!

  Revision 1.5  2004/12/02 09:20:52  majo
  *patchlevel

  Revision 1.4  2004/03/25 17:38:40  majo
  fixed 16bit colors (FGMakeColor) and optimized FGColor::RecomputeValue()

  Revision 1.3  2004/03/17 21:19:12  majo
  added FGColor, FGImage

  Revision 1.2  2004/03/03 20:12:39  majo
  added class FGColor
  fixed Solaris compiling

  Revision 1.1.1.1  2004/01/22 16:25:08  majo
  initial release

*/

#undef NONAMELESSUNION

#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <ctype.h>
#include <time.h>

#ifdef __CYGWIN__
#define _WIN32
#endif

#ifndef FASTGL_BPP
#ifdef BPP8
#define FASTGL_BPP		8
#endif

#ifdef BPP15
#define FASTGL_BPP		15
#endif

#ifdef BPP16
#define FASTGL_BPP		16
#endif

#ifdef BPP32
#define FASTGL_BPP		32
#endif
#endif // FASTGL_BPP

#if (FASTGL_BPP==8)
#define	INDEX_COLORS

#elif (FASTGL_BPP==15)
#define	DIRECT_COLORS	15
#define FGDirectColor(rgb) (((rgb&0xF80000)>>9) | ((rgb&0xF800)>>6) | ((rgb&0xF8)>>3))
#define FGMakeColor(r,g,b) (((r&0xf8)<<7)|((g&0xf8)<<3)|(b&0xf8)>>3)
#define ExpandColor(rgb) (((rgb<<9)&0xF80000) | ((rgb<<6)&0xF800) | ((rgb<<3)&0xF8))
#define kBlueComponent(pix) ((pix<<3)&0xf8)
#define kGreenComponent(pix) ((pix>>2)&0xf8)
#define kRedComponent(pix) ((pix>>7)&0xf8)

#elif (FASTGL_BPP==16)
#define	DIRECT_COLORS	16
#define FGDirectColor(rgb) (((rgb&0xF80000)>>8) | ((rgb&0xFc00)>>5) | ((rgb&0xF8)>>3))
#define FGMakeColor(r,g,b) (((r&0xf8)<<8)|((g&0xfc)<<3)|(b&0xf8)>>3)
#define ExpandColor(rgb) (((rgb<<8)&0xF80000) | ((rgb<<5)&0xFc00) | ((rgb<<3)&0xF8))
#define kBlueComponent(pix) ((pix<<3)&0xf8)
#define kGreenComponent(pix) ((pix>>3)&0xfc)
#define kRedComponent(pix) ((pix>>8)&0xf8)

#elif (FASTGL_BPP==32)
#define	TRUE_COLORS
#define FGDirectColor
#define FGMakeColor(r,g,b) ((r<<16)|(g<<8)|(b))
#define ExpandColor
#define kBlueComponent(pix) (pix&0xff)
#define kGreenComponent(pix) ((pix>>8)&0xff)
#define kRedComponent(pix) ((pix>>16)&0xff)
#endif // fastgl_bpp

// as default
#ifndef FASTGL_BPP
#error "You must define color mode explicitly, use one from [-D BPP8, -D BPP15, -D BPP16, -D BPP32] "
#endif

// disable for index colors
#if defined(BPP8)
#undef				FG_JPEG
#undef				FG_PNG
#endif

#if defined(__BORLANDC__) && defined(_WIN32)
#include <alloc.h>
#include <_str.h>
#elif defined(_MSC_VER)
#else
#include <unistd.h>
#endif

// force link
#ifdef __BORLANDC__
#ifndef INTO_FGL
#ifndef __linux__
#if (FASTGL_BPP == 8)
	#pragma link "fgl.lib"
#elif (FASTGL_BPP == 15 || FASTGL_BPP == 16)
	#pragma link "fgl16.lib"
#elif (FASTGL_BPP == 32)
	#pragma link "fgl32.lib"
#endif

#else	// LINUX libs

#if (FASTGL_BPP == 8)
	#pragma link "fgl.a"
#elif (FASTGL_BPP == 15 || FASTGL_BPP == 16)
	#pragma link "fgl16.a"
#elif (FASTGL_BPP == 32)
	#pragma link "fgl32.a"
#endif
#endif // INTO_FGL
#endif // __linux__

#ifdef __linux__
#ifdef X11_DRIVER
#pragma link "libX11.so"
#pragma link "libXext.so"
#ifdef DGA_DRIVER
#pragma link "libXxf86dga.a"
#endif
#endif

#endif // __linux__
#endif // __borlandc__

/*
#if (((__GNUC__<<16) + (__GNUC_MINOR__ <<8) + (__GNUC_PATCHLEVEL__)) >= 0x030201)
#define HIDDEN __attribute__ ((weak))
#else
#define HIDDEN
#endif
*/

#define HIDDEN

/*
#ifdef __BORLANDC__
#define FGAPI   __fastcall
#define APISTR "__fastcall"
#elif  __GNUC__
#define FGAPI   __attribute__ ((regparm(3)))
#define APISTR "__attribute__ ((regparm(3)))"
#endif
*/

#ifndef FGAPI
#define FGAPI
#define APISTR ""
#endif

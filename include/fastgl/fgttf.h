/*
	SDL_ttf:  A companion library to SDL for working with TrueType (tm) fonts
	Copyright (C) 1997, 1998, 1999, 2000, 2001  Sam Lantinga

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
	slouken@libsdl.org

	22.09.2002 - modification for OpenGUI library
	Marian Krivos
	nezmar@atlas.sk

  $Id: fgttf.h 3694 2005-12-20 09:45:14Z majo $

  $Log$
  Revision 1.3  2005/12/20 09:45:14  majo
  formating

  Revision 1.2  2005/12/12 13:52:04  majo
  fix for cachcing

  Revision 1.1  2005/11/25 11:04:47  majo
  added TTF support

  Revision 1.2  2005/11/15 09:42:56  majo
  fixed ttf cache

  Revision 1.1.1.1  2005/05/12 10:52:38  majo
  i

  Revision 1.2  2004/02/23 20:08:01  majo
  all classes are with prefix FG* on now
  polygon functions uses FGPointArray from now
  class GuiEvent is renamed to FGEvent
  some by parameters overloaded methods was removed (class FGWindow)
  many other small changes

  Revision 1.1.1.1  2004/01/22 16:25:08  majo
  initial release

*/

/* This library is a wrapper around the excellent FreeType 2.0 library,
   available at:
	http://www.freetype.org/
*/

#ifndef _SDLttf_h
#define _SDLttf_h

/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
//extern "C" {
#endif

#ifdef __BORLANDC__
#pragma option push -b
#pragma option push -a4
#endif

#ifdef FG_NAMESPACE
namespace fgl
{
#endif

#ifdef FG_TTF
/* The internal structure containing font information */
	typedef struct _TTF_Font TTF_Font;

/* cache size of TTF */
#ifndef	FONTCACHE_SIZE
#define	FONTCACHE_SIZE	384
#endif

/* Initialize the TTF engine - returns 0 if successful, -1 on error */
	extern int TTF_Init(void);

/* Open a font file and create a font of the specified point size */
	extern TTF_Font *TTF_OpenFont(const char *file, int ptsize);
	extern TTF_Font *TTF_OpenFontIndex(const char *file, int ptsize, long index);

/* Set and retrieve the font style
   This font style is implemented by modifying the font glyphs, and
   doesn't reflect any inherent properties of the truetype font file.
*/
#define TTF_STYLE_NORMAL	0x00
#define TTF_STYLE_BOLD		0x01
#define TTF_STYLE_ITALIC	0x02
#define TTF_STYLE_UNDERLINE	0x04
	extern int TTF_GetFontStyle(TTF_Font * font);
	extern void TTF_SetFontStyle(TTF_Font * font, int style);

/* Get the total height of the font - usually equal to point size */
	extern int TTF_FontHeight(TTF_Font * font);

/* Get the offset from the baseline to the top of the font
   This is a positive value, relative to the baseline.
 */
	extern int TTF_FontAscent(TTF_Font * font);

/* Get the offset from the baseline to the bottom of the font
   This is a negative value, relative to the baseline.
 */
	extern int TTF_FontDescent(TTF_Font * font);

/* Get the recommended spacing between lines of text for this font */
	extern int TTF_FontLineSkip(TTF_Font * font);

/* Get the number of faces of the font */
	extern long TTF_FontFaces(TTF_Font * font);

/* Get the font face attributes, if any */
	extern int TTF_FontFaceIsFixedWidth(TTF_Font * font);
	extern char *TTF_FontFaceFamilyName(TTF_Font * font);
	extern char *TTF_FontFaceStyleName(TTF_Font * font);

/* Get the metrics (dimensions) of a glyph */
	extern int TTF_GlyphMetrics(TTF_Font * font, unsigned short ch, int *minx, int *maxx, int *miny, int *maxy, int *advancex, int *advancey);

/* Get the dimensions of a rendered string of text */
	extern int TTF_SizeText(TTF_Font * font, const char *text, int *w, int *h, int mode);
	extern int TTF_SizeUTF8(TTF_Font * font, const char *text, int *w, int *h, int mode);
	extern int TTF_SizeUNICODE(TTF_Font * font, const unsigned short *text, int *w, int *h, int mode);

	char *TTF_RenderChar(TTF_Font * font, unsigned char c, int &w, int &h, int &sz);

/* Create an 8-bit palettized surface and render the given text at
   fast quality with the given font and color.  The 0 pixel is the
   colorkey, giving a transparent background, and the 1 pixel is set
   to the text color.
   This function returns the new surface, or NULL if there was an error.
*/
	extern FGDrawBuffer *TTF_RenderText_Solid(TTF_Font * font, const char *text, FGPixel fg, FGPixel bg);
	extern FGDrawBuffer *TTF_RenderUTF8_Solid(TTF_Font * font, const char *text, FGPixel fg, FGPixel bg);
	extern FGDrawBuffer *TTF_RenderUNICODE_Solid(TTF_Font * font, const unsigned short *text, FGPixel fg, FGPixel bg);

/* Create an 8-bit palettized surface and render the given glyph at
   fast quality with the given font and color.  The 0 pixel is the
   colorkey, giving a transparent background, and the 1 pixel is set
   to the text color.  The glyph is rendered without any padding or
   centering in the X direction, and aligned normally in the Y direction.
   This function returns the new surface, or NULL if there was an error.
*/
	extern FGDrawBuffer *TTF_RenderGlyph_Solid(TTF_Font * font, unsigned short ch, FGPixel fg);

/* Create an 8-bit palettized surface and render the given text at
   high quality with the given font and colors.  The 0 pixel is background,
   while other pixels have varying degrees of the foreground color.
   This function returns the new surface, or NULL if there was an error.
*/
	extern FGDrawBuffer *TTF_RenderText_Shaded(TTF_Font * font, const char *text, FGPixel fg, FGPixel bg);
	extern FGDrawBuffer *TTF_RenderUTF8_Shaded(TTF_Font * font, const char *text, FGPixel fg, FGPixel bg);
	extern FGDrawBuffer *TTF_RenderUNICODE_Shaded(TTF_Font * font, const unsigned short *text, FGPixel fg, FGPixel bg);

/* Create an 8-bit palettized surface and render the given glyph at
   high quality with the given font and colors.  The 0 pixel is background,
   while other pixels have varying degrees of the foreground color.
   The glyph is rendered without any padding or centering in the X
   direction, and aligned normally in the Y direction.
   This function returns the new surface, or NULL if there was an error.
*/
	extern FGDrawBuffer *TTF_RenderGlyph_Shaded(TTF_Font * font, unsigned short ch, FGPixel fg, FGPixel bg);

/* Create a 32-bit ARGB surface and render the given text at high quality,
   using alpha blending to dither the font with the given color.
   This function returns the new surface, or NULL if there was an error.
*/
	extern FGDrawBuffer *TTF_RenderText_Blended(TTF_Font * font, const char *text, FGPixel fg);
	extern FGDrawBuffer *TTF_RenderUTF8_Blended(TTF_Font * font, const char *text, FGPixel fg);
	extern FGDrawBuffer *TTF_RenderUNICODE_Blended(TTF_Font * font, const unsigned short *text, FGPixel fg);

/* Create a 32-bit ARGB surface and render the given glyph at high quality,
   using alpha blending to dither the font with the given color.
   The glyph is rendered without any padding or centering in the X
   direction, and aligned normally in the Y direction.
   This function returns the new surface, or NULL if there was an error.
*/
	extern FGDrawBuffer *TTF_RenderGlyph_Blended(TTF_Font * font, unsigned short ch, FGPixel fg);

/* For compatibility with previous versions, here are the old functions */
#define TTF_RenderText(font, text, fg, bg)	\
	TTF_RenderText_Shaded(font, text, fg, bg)
#define TTF_RenderUTF8(font, text, fg, bg)	\
	TTF_RenderUTF8_Shaded(font, text, fg, bg)
#define TTF_RenderUNICODE(font, text, fg, bg)	\
	TTF_RenderUNICODE_Shaded(font, text, fg, bg)

/* Close an opened font file */
	extern void TTF_CloseFont(TTF_Font * font);

/* De-initialize the TTF engine */
	extern void TTF_Quit(void);

#endif

#ifdef FG_NAMESPACE
}
#endif

#ifdef __BORLANDC__
#pragma option pop				/* pop -a switch */
#pragma option pop				/* pop -b */
#endif

/* Ends C function definitions when using C++ */
#ifdef __cplusplus
//}
#endif

#endif							/* _SDLttf_h */

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

    Portions of this software are copyright © 1996-2002 The FreeType
    Project (www.freetype.org).  All rights reserved.

*/

//#define DEBUG_FONTS

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#ifdef FG_TTF

#include <ft2build.h>
#include FT_FREETYPE_H
#include <freetype/ftoutln.h>

#include "fgbase.h"
#include "_fastgl.h"
#include "fgttf.h"


#pragma link "libfreetype.so"

/* Macro to convert a character to a Unicode value -- assume already Unicode */
#define UNICODE(c)	c

/* FIXME: Right now we assume the gray-scale renderer Freetype is using
   supports 256 shades of gray, but we should instead key off of num_grays
   in the result FT_Bitmap after the FT_Render_Glyph() call. */
#define NUM_GRAYS       256

/* Handy routines for converting from fixed point */
#define FT_FLOOR(X)	((X & -64) / 64)
#define FT_CEIL(X)	(((X + 63) & -64) / 64)

#define CACHED_METRICS	0x10
#define CACHED_BITMAP	0x01
#define CACHED_PIXMAP	0x02

#ifdef FG_NAMESPACE
namespace fgl
{
#endif

/* Cached glyph information */
	struct c_glyph
	{
		int stored;
		FT_UInt index;
		FT_Bitmap bitmap;
		FT_Bitmap pixmap;
		int minx;
		int maxx;
		int miny;
		int maxy;
		int yoffset;
		int advancex;
		int advancey;
		unsigned short cached;
		time_t last_access;
		  c_glyph()
		{
			memset(this, 0, sizeof (*this));
			index = 0xffffffff;
		}
	};

/* The structure used to hold internal font information */
	struct _TTF_Font
	{
		/* Freetype2 maintains all sorts of useful info itself */
		FT_Face face;

		/* We'll cache these ourselves */
		int height;
		int ascent;
		int descent;
		int lineskip;

		/* The font style */
		int style;

		/* Extra width in glyph bounds for text styles */
		int glyph_overhang;
		float glyph_italics;

		/* Information in the font for underlining */
		int underline_offset;
		int underline_height;

		/* Cache for style-transformed glyphs */
		c_glyph *current;
		c_glyph cache[FONTCACHE_SIZE];
		int used_cache;
	};

/* The FreeType font engine/library */
	static FT_Library library;
	static int TTF_initialized = 0;

	void TTF_SetError(const char *)
	{
	}

	static void TTF_SetFTError(const char *msg, FT_Error error)
	{
#ifdef USE_FREETYPE_ERRORS
#undef FTERRORS_H
#define FT_ERRORDEF( e, v, s )  { e, s },
		static const struct
		{
			int err_code;
			const char *err_msg;
		}
		ft_errors[] =
		{
#include <freetype/fterrors.h>
		};
		int i;
		const char *err_msg;
		char buffer[1024];

		err_msg = NULL;
		for (i = 0; i < ((sizeof ft_errors) / (sizeof ft_errors[0])); ++i)
		{
			if (error == ft_errors[i].err_code)
			{
				err_msg = ft_errors[i].err_msg;
				break;
			}
		}
		if (!err_msg)
		{
			err_msg = "unknown FreeType error";
		}
		sprintf(buffer, "%s: %s", msg, err_msg);
		TTF_SetError(buffer);
#else
		TTF_SetError(msg);
#endif /* USE_FREETYPE_ERRORS */
	}

	int TTF_Init(void)
	{
		int status;
		FT_Error error;

		status = 0;
		error = FT_Init_FreeType(&library);
		if (error)
		{
			TTF_SetFTError("Couldn't init FreeType engine", error);
			status = -1;
		}
		else
		{
			TTF_initialized = 1;
		}
		return status;
	}

	TTF_Font *TTF_OpenFontIndex(const char *file, int ptsize, long index)
	{
		TTF_Font *font;
		FT_Error error;
		FT_Face face;
		FT_Fixed scale;

		font = (TTF_Font *) malloc(sizeof *font);
		if (font == NULL)
		{
			TTF_SetError("Out of memory");
			return NULL;
		}
		memset(font, 0, sizeof (*font));
		memset(&face, 0, sizeof (face));
		memset(&scale, 0, sizeof (scale));

		/* Open the font and create ancillary data */
		error = FT_New_Face(library, file, 0, &font->face);
		if (error)
		{
			TTF_SetFTError("Couldn't load font file", error);
			free(font);
			return NULL;
		}
		if (index != 0)
		{
			if (font->face->num_faces > index)
			{
				FT_Done_Face(font->face);
				error = FT_New_Face(library, file, index, &font->face);
				if (error)
				{
					TTF_SetFTError("Couldn't get font face", error);
					free(font);
					return NULL;
				}
			}
			else
			{
				TTF_SetFTError("No such font face", error);
				free(font);
				return NULL;
			}
		}
		face = font->face;

		/* Make sure that our font face is scalable (global metrics) */
		if (!FT_IS_SCALABLE(face))
		{
			TTF_SetError("Font face is not scalable");
			TTF_CloseFont(font);
			return NULL;
		}

		/* Set the character size and use default DPI (72) */
		error = FT_Set_Char_Size(font->face, 0, ptsize * 64, 0, 0);
		if (error)
		{
			TTF_SetFTError("Couldn't set font size", error);
			TTF_CloseFont(font);
			return NULL;
		}

		/* Get the scalable font metrics for this font */
		scale = face->size->metrics.y_scale;
		font->ascent = FT_CEIL(FT_MulFix(face->bbox.yMax, scale));
		font->descent = FT_CEIL(FT_MulFix(face->bbox.yMin, scale));
		font->height = font->ascent - font->descent + /* baseline */ 1;
		font->lineskip = FT_CEIL(FT_MulFix(face->height, scale));
		font->underline_offset = FT_FLOOR(FT_MulFix(face->underline_position, scale));
		font->underline_height = FT_FLOOR(FT_MulFix(face->underline_thickness, scale));
		if (font->underline_height < 1)
		{
			font->underline_height = 1;
		}
#ifdef DEBUG_FONTS
		printf("Font metrics:\n");
		printf("\tascent = %d, descent = %d\n", font->ascent, font->descent);
		printf("\theight = %d, lineskip = %d\n", font->height, font->lineskip);
		printf("\tunderline_offset = %d, underline_height = %d\n", font->underline_offset, font->underline_height);
#endif

		/* Set the default font style */
		font->style = TTF_STYLE_NORMAL;
		font->glyph_overhang = face->size->metrics.y_ppem / 10;
		/* x offset = cos(((90.0-12)/360)*2*M_PI), or 12 degree angle */
		font->glyph_italics = 0.207f;
		font->glyph_italics *= font->height;

		/* Set the number of used cache */
		font->used_cache = 0;

		return font;
	}

	TTF_Font *TTF_OpenFont(const char *file, int ptsize)
	{
		return TTF_OpenFontIndex(file, ptsize, 0);
	}

	static void Flush_Glyph(c_glyph * glyph)
	{
		glyph->stored = 0;
		glyph->index = 0;
		if (glyph->bitmap.buffer)
		{
			free(glyph->bitmap.buffer);
			glyph->bitmap.buffer = 0;
		}
		if (glyph->pixmap.buffer)
		{
			free(glyph->pixmap.buffer);
			glyph->pixmap.buffer = 0;
		}
		glyph->cached = 0;
		glyph->last_access = 0;
	}

	static void Flush_Cache(TTF_Font * font)
	{
		int i;
		int size = FONTCACHE_SIZE;

		for (i = 0; i < size; ++i)
		{
			if (font->cache[i].cached)
			{
				Flush_Glyph(&font->cache[i]);
				font->used_cache--;
			}

		}
	}

	static FT_Error Load_Glyph(TTF_Font * font, unsigned short ch, c_glyph * cached, int want)
	{
		FT_Face face;
		FT_Error error;
		FT_GlyphSlot glyph;
		FT_Glyph_Metrics *metrics;
		FT_Outline *outline;

		assert(font);
		assert(font->face);

		face = font->face;

		if (face->charmap == 0)
		{
			face->charmap = face->charmaps[0];
		}
		/* Load the glyph */
		if (!cached->index)
		{
			cached->index = FT_Get_Char_Index(face, ch);
		}
		error = FT_Load_Glyph(face, cached->index, FT_LOAD_DEFAULT);
		if (error)
		{
			return error;
		}

		/* Get our glyph shortcuts */
		glyph = face->glyph;
		metrics = &glyph->metrics;
		outline = &glyph->outline;

		/* Get the glyph metrics if desired */
		if ((want & CACHED_METRICS) && !(cached->stored & CACHED_METRICS))
		{
			/* Get the bounding box */
			cached->minx = FT_FLOOR(metrics->horiBearingX);
			cached->maxx = cached->minx + FT_CEIL(metrics->width);
			cached->maxy = FT_FLOOR(metrics->horiBearingY);
			cached->miny = cached->maxy - FT_CEIL(metrics->height);
			cached->yoffset = font->ascent - cached->maxy;
			cached->advancex = FT_CEIL(metrics->horiAdvance);
			cached->advancey = FT_CEIL(metrics->vertAdvance);

			/* Adjust for bold and italic text */
			if (font->style & TTF_STYLE_BOLD)
			{
				cached->maxx += font->glyph_overhang;
			}
			if (font->style & TTF_STYLE_ITALIC)
			{
				cached->maxx += (int) ceil(font->glyph_italics);
			}
			cached->stored |= CACHED_METRICS;
		}

		if (((want & CACHED_BITMAP) && !(cached->stored & CACHED_BITMAP)) || ((want & CACHED_PIXMAP) && !(cached->stored & CACHED_PIXMAP)))
		{
			int mono = (want & CACHED_BITMAP);
			int i;
			FT_Bitmap *src;
			FT_Bitmap *dst;

			/* Handle the italic style */
			if (font->style & TTF_STYLE_ITALIC)
			{
				FT_Matrix shear;

				shear.xx = 1 << 16;
				shear.xy = (int) (font->glyph_italics * (1 << 16)) / font->height;
				shear.yx = 0;
				shear.yy = 1 << 16;

				FT_Outline_Transform(outline, &shear);
			}

			/* Render the glyph */
			if (mono)
			{
				error = FT_Render_Glyph(glyph, ft_render_mode_mono);
			}
			else
			{
				error = FT_Render_Glyph(glyph, ft_render_mode_normal);
			}
			if (error)
			{
				return error;
			}

			/* Copy over information to cache */
			src = &glyph->bitmap;
			if (mono)
			{
				dst = &cached->bitmap;
			}
			else
			{
				dst = &cached->pixmap;
			}
			memcpy(dst, src, sizeof (*dst));
			if (mono)
			{
				dst->pitch *= 8;
			}

			/* Adjust for bold and italic text */
			if (font->style & TTF_STYLE_BOLD)
			{
				int bump = font->glyph_overhang;

				dst->pitch += bump;
				dst->width += bump;
			}
			if (font->style & TTF_STYLE_ITALIC)
			{
				int bump = (int) ceil(font->glyph_italics);

				dst->pitch += bump;
				dst->width += bump;
			}

			if (dst->rows != 0)
			{
				dst->buffer = (unsigned char *) malloc(dst->pitch * dst->rows);
				if (!dst->buffer)
				{
					return FT_Err_Out_Of_Memory;
				}
				memset(dst->buffer, 0, dst->pitch * dst->rows);

				for (i = 0; i < src->rows; i++)
				{
					int soffset = i * src->pitch;
					int doffset = i * dst->pitch;

					if (mono)
					{
						unsigned char *srcp = src->buffer + soffset;
						unsigned char *dstp = dst->buffer + doffset;
						int j;

						for (j = 0; j < src->width; j += 8)
						{
							unsigned char ch = *srcp++;

							*dstp++ = (ch & 0x80) >> 7;
							ch <<= 1;
							*dstp++ = (ch & 0x80) >> 7;
							ch <<= 1;
							*dstp++ = (ch & 0x80) >> 7;
							ch <<= 1;
							*dstp++ = (ch & 0x80) >> 7;
							ch <<= 1;
							*dstp++ = (ch & 0x80) >> 7;
							ch <<= 1;
							*dstp++ = (ch & 0x80) >> 7;
							ch <<= 1;
							*dstp++ = (ch & 0x80) >> 7;
							ch <<= 1;
							*dstp++ = (ch & 0x80) >> 7;
						}
					}
					else
					{
						memcpy(dst->buffer + doffset, src->buffer + soffset, src->pitch);
					}
				}
			}

			/* Handle the bold style */
			if (font->style & TTF_STYLE_BOLD)
			{
				int row;
				int col;
				int offset;
				int pixel;
				unsigned char *pixmap;

				/* The pixmap is a little hard, we have to add and clamp */
				for (row = dst->rows - 1; row >= 0; --row)
				{
					pixmap = (unsigned char *) dst->buffer + row * dst->pitch;
					for (offset = 1; offset <= font->glyph_overhang; ++offset)
					{
						for (col = dst->width - 1; col > 0; --col)
						{
							pixel = (pixmap[col] + pixmap[col - 1]);
							if (pixel > NUM_GRAYS - 1)
							{
								pixel = NUM_GRAYS - 1;
							}
							pixmap[col] = (unsigned char) pixel;
						}
					}
				}
			}

			/* Mark that we rendered this format */
			if (mono)
			{
				cached->stored |= CACHED_BITMAP;
			}
			else
			{
				cached->stored |= CACHED_PIXMAP;
			}
		}

		/* We're done, mark this glyph cached */
		if (cached->cached != ch)
		{
			cached->cached = ch;
			font->used_cache++;	/* a new cached glyph */
		}
		cached->last_access = time(0);

		return 0;
	}

	static int Find_Cache(TTF_Font * font, unsigned short ch)
	{
		int i;

		for (i = 0; i < FONTCACHE_SIZE; i++)
		{
			if (font->cache[i].cached == ch)
			{
				return i;
			}
		}
		return -1;
	}

	static int ClearOldCache(TTF_Font * font)
	{
		int i, pos;
		time_t oldest;

		/* assume the cache[0] is the oldest */
		pos = 0;
		oldest = font->cache[0].last_access;

		for (i = 1; i < FONTCACHE_SIZE; i++)
		{
			if (font->cache[i].last_access < oldest)
			{
				oldest = font->cache[i].last_access;
				pos = i;
			}
		}

		Flush_Glyph(&font->cache[pos]);
		font->used_cache--;
		return pos;
	}

	static int Find_FreeCache(TTF_Font * font)
	{
		int i;

		if (font->used_cache == FONTCACHE_SIZE)
			return ClearOldCache(font);

		for (i = 0; i < FONTCACHE_SIZE; i++)
		{
			if (font->cache[i].cached == 0)
			{
				return i;
			}
		}
		return -1;
	}

	static FT_Error Find_Glyph(TTF_Font * font, unsigned short ch, int want)
	{
		int retval = 0;

		if ((retval = Find_Cache(font, ch)) == -1)
		{
			if ((retval = Find_FreeCache(font)) == -1)
			{
				printf("Error! Cannot Find Free Cache!\n");
				return FT_Err_Out_Of_Memory;
			}
		}
		font->current = &font->cache[retval];
		retval = 0;

		if ((font->current->stored & want) != want)
		{
			retval = Load_Glyph(font, ch, font->current, want);
		}
		else
		{
			font->current->last_access = time(0);
		}

		return retval;
	}

	void TTF_CloseFont(TTF_Font * font)
	{
		Flush_Cache(font);
		FT_Done_Face(font->face);
		free(font);
	}

	static unsigned short *ASCII_to_UNICODE(unsigned short *unicode, const char *text, int len)
	{
		int i;

		for (i = 0; i < len; ++i)
		{
			unicode[i] = ((const unsigned char *) text)[i];
		}
		unicode[i] = 0;

		return unicode;
	}

	static unsigned short *UTF8_to_UNICODE(unsigned short *unicode, const char *utf8, int len)
	{
		int i, j;
		unsigned short ch;

		for (i = 0, j = 0; i < len; ++i, ++j)
		{
			ch = ((const unsigned char *) utf8)[i];
			if (ch >= 0xF0)
			{
				ch = (unsigned short) (utf8[i] & 0x07) << 18;
				ch |= (unsigned short) (utf8[++i] & 0x3F) << 12;
				ch |= (unsigned short) (utf8[++i] & 0x3F) << 6;
				ch |= (unsigned short) (utf8[++i] & 0x3F);
			}
			else if (ch >= 0xE0)
			{
				ch = (unsigned short) (utf8[i] & 0x3F) << 12;
				ch |= (unsigned short) (utf8[++i] & 0x3F) << 6;
				ch |= (unsigned short) (utf8[++i] & 0x3F);
			}
			else if (ch >= 0xC0)
			{
				ch = (unsigned short) (utf8[i] & 0x3F) << 6;
				ch |= (unsigned short) (utf8[++i] & 0x3F);
			}
			unicode[j] = ch;
		}
		unicode[j] = 0;

		return unicode;
	}

	int TTF_FontHeight(TTF_Font * font)
	{
		return (font->height);
	}

	int TTF_FontAscent(TTF_Font * font)
	{
		return (font->ascent);
	}

	int TTF_FontDescent(TTF_Font * font)
	{
		return (font->descent);
	}

	int TTF_FontLineSkip(TTF_Font * font)
	{
		return (font->lineskip);
	}

	long TTF_FontFaces(TTF_Font * font)
	{
		return (font->face->num_faces);
	}

	int TTF_FontFaceIsFixedWidth(TTF_Font * font)
	{
		return (FT_IS_FIXED_WIDTH(font->face));
	}

	char *TTF_FontFaceFamilyName(TTF_Font * font)
	{
		return (font->face->family_name);
	}

	char *TTF_FontFaceStyleName(TTF_Font * font)
	{
		return (font->face->style_name);
	}

	int TTF_GlyphMetrics(TTF_Font * font, unsigned short ch, int *minx, int *maxx, int *miny, int *maxy, int *advancex, int *advancey)
	{
		FT_Error error;

		error = Find_Glyph(font, ch, CACHED_METRICS);
		if (error)
		{
			TTF_SetFTError("Couldn't find glyph", error);
			return -1;
		}

		if (minx)
		{
			*minx = font->current->minx;
		}
		if (maxx)
		{
			*maxx = font->current->maxx;
		}
		if (miny)
		{
			*miny = font->current->miny;
		}
		if (maxy)
		{
			*maxy = font->current->maxy;
		}
		if (advancex)
			*advancex = font->current->advancex;
		if (advancey)
			*advancey = font->current->advancey;
		return 0;
	}

	int TTF_SizeText(TTF_Font * font, const char *text, int *w, int *h, int mode)
	{
		unsigned short *unicode_text;
		int unicode_len;
		int status;

		/* Copy the Latin-1 text to a UNICODE text buffer */
		unicode_len = strlen(text);
		unicode_text = (unsigned short *) malloc((unicode_len + 1) * (sizeof *unicode_text));
		if (unicode_text == NULL)
		{
			TTF_SetError("Out of memory");
			return -1;
		}
		ASCII_to_UNICODE(unicode_text, text, unicode_len);

		/* Render the new text */
		status = TTF_SizeUNICODE(font, unicode_text, w, h, mode);

		/* Free the text buffer and return */
		free(unicode_text);
		return status;
	}

	int TTF_SizeUTF8(TTF_Font * font, const char *text, int *w, int *h, int mode)
	{
		unsigned short *unicode_text;
		int unicode_len;
		int status;

		/* Copy the UTF-8 text to a UNICODE text buffer */
		unicode_len = strlen(text);
		unicode_text = (unsigned short *) malloc((unicode_len + 1) * (sizeof *unicode_text));
		if (unicode_text == NULL)
		{
			TTF_SetError("Out of memory");
			return -1;
		}
		UTF8_to_UNICODE(unicode_text, text, unicode_len);

		/* Render the new text */
		status = TTF_SizeUNICODE(font, unicode_text, w, h, mode);

		/* Free the text buffer and return */
		free(unicode_text);
		return status;
	}

	int TTF_SizeUNICODE(TTF_Font * font, const unsigned short *text, int *w, int *h, int mode)
	{
		int status;
		const unsigned short *ch;
		int x, z;
		int minx, maxx;
		int miny, maxy;
		c_glyph *glyph;
		FT_Error error;

		/* Initialize everything to 0 */
		if (!TTF_initialized)
		{
			return -1;
		}
		status = 0;
		minx = maxx = 0;
		miny = maxy = 0;

		/* Load each character and sum it's bounding box */
		x = 0;
		for (ch = text; *ch; ++ch)
		{
			error = Find_Glyph(font, *ch, CACHED_METRICS | mode);
			if (error)
			{
				return -1;
			}
			glyph = font->current;

			z = x + glyph->minx;
			if (minx > z)
			{
				minx = z;
			}
			if (font->style & TTF_STYLE_BOLD)
			{
				x += font->glyph_overhang;
			}
			if (glyph->advancex > glyph->maxx)
			{
				z = x + glyph->advancex;
			}
			else
			{
				z = x + glyph->maxx;
			}
			if (maxx < z)
			{
				maxx = z;
			}
			x += glyph->advancex;

			if (glyph->miny < miny)
			{
				miny = glyph->miny;
			}
			if (glyph->maxy > maxy)
			{
				maxy = glyph->maxy;
			}
		}

		/* Fill the bounds rectangle */
		if (w)
		{
			*w = (maxx - minx);
		}
		if (h)
		{
#if 0	/* This is correct, but breaks many applications */
			*h = (maxy - miny);
#else
			*h = font->height;
#endif
		}
		return status;
	}

	char *TTF_RenderChar(TTF_Font * font, unsigned char c, int &width, int &height, int &sz)
	{
		char *src, *dst, *retdst;
		c_glyph *glyph;
		FT_Error error;
		FT_Bitmap *current = NULL;
		int width2;

		error = Find_Glyph(font, c, CACHED_METRICS | CACHED_BITMAP);
		if (error)
		{
			return 0;
		}
		glyph = font->current;
		current = &glyph->bitmap;
		height = glyph->advancey;
		if (height < glyph->yoffset + current->rows)
			height = glyph->yoffset + current->rows;
		width = glyph->advancex;
		if (width < current->width)
			width = current->width;
		if (font->style & TTF_STYLE_BOLD)
		{
			width += font->glyph_overhang;
		}
		width2 = (width + 3) & ~3;
		sz = width2 * height;

		if (sz == 0)
			return 0;

		retdst = (char *) calloc(sz, 1);

		for (int row = 0; row < current->rows; row++)
		{
			dst = retdst + (row + glyph->yoffset) * width2 + glyph->minx;
			src = (char *) current->buffer + row * current->pitch;
			for (int col = 0; col < current->width; col++)
				*dst++ = *src++ ? 0xff : 0;
		}
		if (current->rows)
			assert(retdst + sz >= dst);
		return retdst;
	}

/*
	Convert the Latin-1 text to UNICODE and render it
*/
	FGDrawBuffer *TTF_RenderText_Solid(TTF_Font * font, const char *text, FGPixel fg, FGPixel bg)
	{
		FGDrawBuffer *textbuf;
		unsigned short *unicode_text;
		int unicode_len;

		/* Copy the Latin-1 text to a UNICODE text buffer */
		unicode_len = strlen(text);
		unicode_text = (unsigned short *) malloc((unicode_len + 1) * (sizeof *unicode_text));
		if (unicode_text == NULL)
		{
			TTF_SetError("Out of memory");
			return (NULL);
		}
		ASCII_to_UNICODE(unicode_text, text, unicode_len);

		/* Render the new text */
		textbuf = TTF_RenderUNICODE_Solid(font, unicode_text, fg, bg);

		/* Free the text buffer and return */
		free(unicode_text);
		return (textbuf);
	}

/* Convert the UTF-8 text to UNICODE and render it
*/
	FGDrawBuffer *TTF_RenderUTF8_Solid(TTF_Font * font, const char *text, FGPixel fg, FGPixel bg)
	{
		FGDrawBuffer *textbuf;
		unsigned short *unicode_text;
		int unicode_len;

		/* Copy the UTF-8 text to a UNICODE text buffer */
		unicode_len = strlen(text);
		unicode_text = (unsigned short *) malloc((unicode_len + 1) * (sizeof *unicode_text));
		if (unicode_text == NULL)
		{
			TTF_SetError("Out of memory");
			return (NULL);
		}
		UTF8_to_UNICODE(unicode_text, text, unicode_len);

		/* Render the new text */
		textbuf = TTF_RenderUNICODE_Solid(font, unicode_text, fg, bg);

		/* Free the text buffer and return */
		free(unicode_text);
		return (textbuf);
	}

	FGDrawBuffer *TTF_RenderUNICODE_Solid(TTF_Font * font, const unsigned short *text, FGPixel fg, FGPixel bg)
	{
		int xstart;
		int width;
		int height;
		FGDrawBuffer *textbuf;
		const unsigned short *ch;
		unsigned char *src;
		FGPixel *dst;
		int row;
		int col;
		c_glyph *glyph;
		FT_Error error;

		/* Get the dimensions of the text surface */
		if ((TTF_SizeUNICODE(font, text, &width, NULL, CACHED_BITMAP) < 0) || !width)
		{
			TTF_SetError("Text has zero width");
			return NULL;
		}
		height = font->height;

		/* Create the target surface */
		textbuf = new FGDrawBuffer(width, height);
		textbuf->clear(bg);

		textbuf->SetColorKey(0);

		/* Load and render each character */
		xstart = 0;

		for (ch = text; *ch; ++ch)
		{
			FT_Bitmap *current = NULL;

			error = Find_Glyph(font, *ch, CACHED_METRICS | CACHED_BITMAP);
			if (error)
			{
				delete textbuf;

				return NULL;
			}
			glyph = font->current;

			current = &glyph->bitmap;
			for (row = 0; row < current->rows; ++row)
			{
				dst = textbuf->GetArray() + (row + glyph->yoffset) * textbuf->GetW() + xstart + glyph->minx;
				src = current->buffer + row * current->pitch;

				for (col = current->width; col > 0; --col)
				{
					*dst++ = *src++ ? fg : bg;
				}
			}

			xstart += glyph->advancex;
			if (font->style & TTF_STYLE_BOLD)
			{
				xstart += font->glyph_overhang;
			}
		}

		/* Handle the underline style */
		if (font->style & TTF_STYLE_UNDERLINE)
		{
			row = font->ascent - font->underline_offset - 1;
			if (row >= textbuf->GetH())
			{
				row = (textbuf->GetH() - 1) - font->underline_height;
			}
			dst = textbuf->GetArray() + row * textbuf->GetW();
			for (row = font->underline_height; row > 0; --row)
			{
				/* 1 because 0 is the bg color */
				FGmemset(dst, fg, textbuf->GetW());
				dst += textbuf->GetW();
			}
		}
		return textbuf;
	}


	FGDrawBuffer *TTF_RenderGlyph_Solid(TTF_Font * font, unsigned short ch, FGPixel fg, FGPixel bg)
	{
		FGDrawBuffer *textbuf;
		unsigned char *src;
		FGPixel *dst;
		int row;
		FT_Error error;
		c_glyph *glyph;

		/* Get the glyph itself */
		error = Find_Glyph(font, ch, CACHED_METRICS | CACHED_BITMAP);
		if (error)
		{
			return (NULL);
		}
		glyph = font->current;

		/* Create the target surface */
		textbuf = new FGDrawBuffer(glyph->pixmap.width, glyph->pixmap.rows);
		textbuf->clear(bg);

		/* Copy the character from the pixmap */
		src = glyph->pixmap.buffer;
		dst = textbuf->GetArray();
		for (row = 0; row < textbuf->GetH(); ++row)
		{
			/* Make sure we don't go either over, or under the limit */
			if (row + glyph->yoffset < 0)
				continue;
			if (row + glyph->yoffset >= textbuf->GetH())
				continue;

			memcpy(dst, src, glyph->pixmap.pitch);
			src += glyph->pixmap.pitch;
			dst += textbuf->GetW();
		}

		/* Handle the underline style */
		if (font->style & TTF_STYLE_UNDERLINE)
		{
			row = font->ascent - font->underline_offset - 1;
			if (row >= textbuf->GetH())
			{
				row = (textbuf->GetH() - 1) - font->underline_height;
			}
			dst = textbuf->GetArray() + row * textbuf->GetW();
			for (row = font->underline_height; row > 0; --row)
			{
				/* 1 because 0 is the bg color */
				FGmemset(dst, fg, textbuf->GetW());
				dst += textbuf->GetW();
			}
		}
		return (textbuf);
	}

#ifndef BPP8
/* Convert the Latin-1 text to UNICODE and render it
*/
	FGDrawBuffer *TTF_RenderText_Shaded(TTF_Font * font, const char *text, FGPixel fg, FGPixel bg)
	{
		FGDrawBuffer *textbuf;
		unsigned short *unicode_text;
		int unicode_len;

		/* Copy the Latin-1 text to a UNICODE text buffer */
		unicode_len = strlen(text);
		unicode_text = (unsigned short *) malloc((unicode_len + 1) * (sizeof *unicode_text));
		if (unicode_text == NULL)
		{
			TTF_SetError("Out of memory");
			return (NULL);
		}
		ASCII_to_UNICODE(unicode_text, text, unicode_len);

		/* Render the new text */
		textbuf = TTF_RenderUNICODE_Shaded(font, unicode_text, fg, bg);

		/* Free the text buffer and return */
		free(unicode_text);
		return (textbuf);
	}

/* Convert the UTF-8 text to UNICODE and render it
*/
	FGDrawBuffer *TTF_RenderUTF8_Shaded(TTF_Font * font, const char *text, FGPixel fg, FGPixel bg)
	{
		FGDrawBuffer *textbuf;
		unsigned short *unicode_text;
		int unicode_len;

		/* Copy the UTF-8 text to a UNICODE text buffer */
		unicode_len = strlen(text);
		unicode_text = (unsigned short *) malloc((unicode_len + 1) * (sizeof *unicode_text));
		if (unicode_text == NULL)
		{
			TTF_SetError("Out of memory");
			return (NULL);
		}
		UTF8_to_UNICODE(unicode_text, text, unicode_len);

		/* Render the new text */
		textbuf = TTF_RenderUNICODE_Shaded(font, unicode_text, fg, bg);

		/* Free the text buffer and return */
		free(unicode_text);
		return (textbuf);
	}

	static void shaded_helper(FGPalette colors[], FGPixel fg, FGPixel bg)
	{
		int index;
		int rdiff;
		int gdiff;
		int bdiff;

		rdiff = kRedComponent(fg) - kRedComponent(bg);
		gdiff = kGreenComponent(fg) - kGreenComponent(bg);
		bdiff = kBlueComponent(fg) - kBlueComponent(bg);

		for (index = 0; index < NUM_GRAYS; ++index)
		{
			colors[index].r = kRedComponent(bg) + (index * rdiff) / (NUM_GRAYS - 1);
			colors[index].g = kGreenComponent(bg) + (index * gdiff) / (NUM_GRAYS - 1);
			colors[index].b = kBlueComponent(bg) + (index * bdiff) / (NUM_GRAYS - 1);
		}
		colors[0] = colors[1] = FGPalette(ExpandColor(bg));
		colors[NUM_GRAYS - 1] = FGPalette(ExpandColor(fg));
	}

	FGDrawBuffer *TTF_RenderUNICODE_Shaded(TTF_Font * font, const unsigned short *text, FGPixel fg, FGPixel bg)
	{
		int xstart;
		int width = 0;
		int height = 0;
		FGDrawBuffer *textbuf;
		const unsigned short *ch;
		unsigned char *src;
		FGPixel *dst;
		int row, col;
		c_glyph *glyph;
		FT_Error error;
		unsigned i;
		FT_Bitmap *current = 0;

		// Get the dimensions of the text surface
		if ((TTF_SizeUNICODE(font, text, &width, NULL, CACHED_PIXMAP) < 0) || !width)
		{
			TTF_SetError("Text has zero width");
			return NULL;
		}
		height = font->height;

		/* Create the target surface */
		textbuf = new FGDrawBuffer(width, height);
		textbuf->clear(bg);

		/* Fill the palette with NUM_GRAYS levels of shading from bg to fg */
		FGPalette colors[NUM_GRAYS];

		shaded_helper(colors, fg, bg);

		textbuf->SetColorKey(0);

		/* Load and render each character */
		xstart = 0;

		/* Adding bound checking to avoid all kinds of memory corruption errors that may occur. */
		FGPixel *dst_check = textbuf->GetArray() + (textbuf->GetH() * textbuf->GetW());

		for (ch = text; *ch; ++ch)
		{
			error = Find_Glyph(font, *ch, CACHED_METRICS | CACHED_PIXMAP);
			if (error)
			{
				delete textbuf;

				return NULL;
			}
			glyph = font->current;
			current = &glyph->pixmap;

			/* Ensure the width of the pixmap is correct. On some cases, freetype may report a larger pixmap than possible. */
			int ww = glyph->pixmap.width;

			if (ww > glyph->maxx - glyph->minx)
			{
				ww = glyph->maxx - glyph->minx;
			}

			/* Compensate for the wrap around with negative minx's */
			if ((ch == text) && (glyph->minx < 0))
				xstart -= glyph->minx;

			for (row = 0; row < current->rows; ++row)
			{
				/* Make sure we don't go either over, or under the limit */
				if (row + glyph->yoffset < 0)
					continue;
				if (row + glyph->yoffset >= textbuf->GetH())
					continue;

				dst = textbuf->GetArray() + ((row + glyph->yoffset) * textbuf->GetW() + glyph->minx) + xstart;

				src = current->buffer + row * current->pitch;

				for (col = ww; col > 0 && dst < dst_check; --col)
				{
					i = colors[*src++].GetValue();
					*dst++ = (FGDirectColor(i));
				}
			}

			xstart += glyph->advancex;
			if (font->style & TTF_STYLE_BOLD)
			{
				xstart += font->glyph_overhang;
			}
		}

		/* Handle the underline style */
		if (font->style & TTF_STYLE_UNDERLINE)
		{
			row = font->ascent - font->underline_offset - 1;
			if (row >= textbuf->GetH())
			{
				row = (textbuf->GetH() - 1) - font->underline_height;
			}
			dst = textbuf->GetArray() + row * textbuf->GetW();
			for (row = font->underline_height; row > 0; --row)
			{
				FGmemset(dst, fg, textbuf->GetW());
				dst += textbuf->GetW();
			}
		}
		return textbuf;
	}

	FGDrawBuffer *TTF_RenderGlyph_Shaded(TTF_Font * font, unsigned short ch, FGPixel fg, FGPixel bg)
	{
		FGDrawBuffer *textbuf;
		int index;
		int rdiff;
		int gdiff;
		int bdiff;
		unsigned char *src;
		FGPixel *dst;
		int row;
		FT_Error error;
		c_glyph *glyph;

		/* Get the glyph itself */
		error = Find_Glyph(font, ch, CACHED_METRICS | CACHED_PIXMAP);
		if (error)
		{
			return NULL;
		}
		glyph = font->current;

		/* Create the target surface */
		textbuf = new FGDrawBuffer(glyph->pixmap.width, glyph->pixmap.rows);

		/* Fill the palette with NUM_GRAYS levels of shading from bg to fg */
		FGPalette colors[NUM_GRAYS];

		shaded_helper(colors, fg, bg);

		/* Copy the character from the pixmap */
		src = glyph->pixmap.buffer;
		dst = textbuf->GetArray();
		for (row = 0; row < textbuf->GetH(); ++row)
		{
			memcpy(dst, src, glyph->pixmap.pitch);
			src += glyph->pixmap.pitch;
			dst += textbuf->GetW();
		}

		/* Handle the underline style */
		if (font->style & TTF_STYLE_UNDERLINE)
		{
			row = font->ascent - font->underline_offset - 1;
			if (row >= textbuf->GetH())
			{
				row = (textbuf->GetH() - 1) - font->underline_height;
			}
			dst = textbuf->GetArray() + row * textbuf->GetW();
			for (row = font->underline_height; row > 0; --row)
			{
				FGmemset(dst, fg, textbuf->GetW());
				dst += textbuf->GetW();
			}
		}
		return textbuf;
	}
#endif // bpp8

	void TTF_SetFontStyle(TTF_Font * font, int style)
	{
		font->style = style;
		Flush_Cache(font);
	}

	int TTF_GetFontStyle(TTF_Font * font)
	{
		return font->style;
	}

	void TTF_Quit(void)
	{
		if (TTF_initialized)
		{
			FT_Done_FreeType(library);
		}
		TTF_initialized = 0;
	}
#endif

#ifdef FG_NAMESPACE
}
#endif

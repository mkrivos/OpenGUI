/*
OpenGUI - Drawing & Windowing library

Copyright (C) 1996,2005  Marian Krivos

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

nezmar@atlas.sk

base color support headers

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "fgbase.h"
#include "_fastgl.h"

#define	min(a,b)	( (a) < (b) ? (a) : (b) )
#define max(a,b)	( (a) > (b) ? (a) : (b) )

#ifdef FG_NAMESPACE
namespace fgl
{
#endif

const FGNamedColor FGColor::m_namedColor[FGColor::numNamedColors] =
{
  {aliceblue            , "aliceblue"},
  {antiquewhite         , "antiquewhite"},
  {aqua                 , "aqua"},
  {aquamarine           , "aquamarine"},
  {azure                , "azure"},
  {beige                , "beige"},
  {bisque               , "bisque"},
  {black                , "blue"},
  {blueviolet           , "blueviolet"},
  {brown                , "brown"},
  {burlywood            , "burlywood"},
  {cadetblue            , "cadetblue"},
  {chartreuse           , "chartreuse"},
  {chocolate            , "chocolate"},
  {coral                , "coral"},
  {cornflower           , "cornflower"},
  {cornsilk             , "cornsilk"},
  {crimson              , "crimson"},
  {cyan                 , "cyan"},
  {darkblue             , "darkblue"},
  {darkcyan             , "darkcyan"},
  {darkgoldenrod        , "darkgoldenrod"},
  {darkgray             , "darkgray"},
  {darkgreen            , "darkgreen"},
  {darkkhaki            , "darkkhaki"},
  {darkmagenta          , "darkmagenta"},
  {darkolivegreen       , "darkolivegreen"},
  {darkorange           , "darkorange"},
  {darkorchid           , "darkorchid"},
  {darkred              , "darkred"},
  {darksalmon           , "darksalmon"},
  {darkseagreen         , "darkseagreen"},
  {darkslateblue        , "darkslateblue"},
  {darkslategray        , "darkslategray"},
  {darkturquoise        , "darkturquoise"},
  {darkviolet           , "darkviolet"},
  {deeppink             , "deeppink"},
  {deepskyblue          , "deepskyblue"},
  {dimgray              , "dimgray"},
  {dodgerblue           , "dodgerblue"},
  {firebrick            , "firebrick"},
  {floralwhite          , "floralwhite"},
  {forestgreen          , "forestgreen"},
  {fuchsia              , "fuchsia"},
  {gainsboro            , "gainsboro"},
  {ghostwhite           , "ghostwhite"},
  {gold                 , "gold"},
  {goldenrod            , "goldenrod"},
  {gray                 , "gray"},
  {green                , "green"},
  {greenyellow          , "greenyellow"},
  {honeydew             , "honeydew"},
  {hotpink              , "hotpink"},
  {indianred            , "indianred"},
  {indigo               , "indigo"},
  {ivory                , "ivory"},
  {khaki                , "khaki"},
  {lavender             , "lavender"},
  {lavenderblush        , "lavenderblush"},
  {lawngreen            , "lawngreen"},
  {lemonchiffon         , "lemonchiffon"},
  {lightblue            , "lightblue"},
  {lightcoral           , "lightcoral"},
  {lightcyan            , "lightcyan"},
  {lightgoldenrodyellow , "lightgoldenrodyellow"},
  {lightgreen           , "lightgreen"},
  {lightgrey            , "lightgrey"},
  {lightpink            , "lightpink"},
  {lightsalmon          , "lightsalmon"},
  {lightseagreen        , "lightseagreen"},
  {lightskyblue         , "lightskyblue"},
  {lightslategray       , "lightslategray"},
  {lightsteelblue       , "lightsteelblue"},
  {lightyellow          , "lightyellow"},
  {lime                 , "lime"},
  {limegreen            , "limegreen"},
  {linen                , "linen"},
  {magenta              , "magenta"},
  {maroon               , "maroon"},
  {mediumaquamarine     , "mediumaquamarine"},
  {mediumblue           , "mediumblue"},
  {mediumorchid         , "mediumorchid"},
  {mediumpurple         , "mediumpurple"},
  {mediumseagreen       , "mediumseagreen"},
  {mediumslateblue      , "mediumslateblue"},
  {mediumspringgreen    , "mediumspringgreen"},
  {mediumturquoise      , "mediumturquoise"},
  {mediumvioletred      , "mediumvioletred"},
  {midnightblue         , "midnightblue"},
  {mintcream            , "mintcream"},
  {mistyrose            , "mistyrose"},
  {moccasin             , "moccasin"},
  {navajowhite          , "navajowhite"},
  {navy                 , "navy"},
  {oldlace              , "oldlace"},
  {olive                , "olive"},
  {olivedrab            , "olivedrab"},
  {orange               , "orange"},
  {orangered            , "orangered"},
  {orchid               , "orchid"},
  {palegoldenrod        , "palegoldenrod"},
  {palegreen            , "palegreen"},
  {paleturquoise        , "paleturquoise"},
  {palevioletred        , "palevioletred"},
  {papayawhip           , "papayawhip"},
  {peachpuff            , "peachpuff"},
  {peru                 , "peru"},
  {pink                 , "pink"},
  {plum                 , "plum"},
  {powderblue           , "powderblue"},
  {purple               , "purple"},
  {red                  , "red"},
  {rosybrown            , "rosybrown"},
  {royalblue            , "royalblue"},
  {saddlebrown          , "saddlebrown"},
  {salmon               , "salmon"},
  {sandybrown           , "sandybrown"},
  {seagreen             , "seagreen"},
  {seashell             , "seashell"},
  {sienna               , "sienna"},
  {silver               , "silver"},
  {skyblue              , "skyblue"},
  {slateblue            , "slateblue"},
  {slategray            , "slategray"},
  {snow                 , "snow"},
  {springgreen          , "springgreen"},
  {steelblue            , "steelblue"},
  {tan                  , "tan"},
  {teal                 , "teal"},
  {thistle              , "thistle"},
  {tomato               , "tomato"},
  {turquoise            , "turquoise"},
  {violet               , "violet"},
  {wheat                , "wheat"},
  {white                , "white"},
  {whitesmoke           , "whitesmoke"},
  {yellow               , "yellow"},
  {yellowgreen          , "yellowgreen"}
};

//! current palette [ABGR]
unsigned int _fgl_palette[256];
FGPalette ColorsArray[256];

//! current bytes per pixel - for future
int bpp = sizeof(FGPixel);

/**
* Gets the user friendly name for the index i.
*/
const char* FGColor::GetNameFromIndex(int i)
{
  assert(0 <= i && i < numNamedColors);
  return m_namedColor[i].name;
}

/**
* Gets the color for the index i.
*/

FGColor FGColor::GetColorFromIndex(int i)
{
  assert(0 <= i && i < numNamedColors);
  return m_namedColor[i].color;
}

FGColor::FGColor(int r, int g, int b)
: m_blue(b), m_green(g), m_red(r), m_alpha(0)
{
#ifdef INDEX_COLORS
	GetPaletteEntry(&m_red, &m_green, &m_blue, m_pixel);
#endif
	RecomputeHLS();
	RecomputeValue();
}

FGColor::FGColor()
: m_blue(0), m_green(0), m_red(0), m_alpha(0)
{
#ifdef INDEX_COLORS
	GetPaletteEntry(&m_red, &m_green, &m_blue, m_pixel);
#endif
	RecomputeHLS();
	RecomputeValue();
}

FGColor::FGColor(const char* name)
 : m_alpha(0)
{
	SetString(name);
	RecomputeHLS();
	RecomputeValue();
}

// ARGB
FGColor::FGColor(PIXEL_ARGB cr)
{
#ifdef INDEX_COLORS
	GetPaletteEntry(&m_red, &m_green, &m_blue, cr);
/*
	The input Value MUST BE 3 * 8.bit triplet color!!!!

#elif (FASTGL_BPP==15)
	m_red   = (cr>>7)&0xf8;
	m_green = (cr>>2)&0xf8;
	m_blue  = (cr<<3)&0xf8;
#elif (FASTGL_BPP==16)
	m_red   = (cr>>8)&0xf8;
	m_green = (cr>>3)&0xfc;
	m_blue  = (cr<<3)&0xf8;
#elif (FASTGL_BPP==32)
*/
#else
	m_red   = cr>>16;
	m_green = cr>>8;
	m_blue  = cr;
#endif
	RecomputeHLS();
	RecomputeValue();
}

FGColor::~FGColor()
{
#ifdef INDEX_COLORS
    DeleteColor(m_pixel);
#endif
}

// RGB

void FGColor::Desaturate(void)
{
	unsigned intensity = (m_red*11U + m_green*16U + m_blue*5U) / 32U;
	m_red = m_green = m_blue = intensity;
	RecomputeHLS();
	RecomputeValue();
}

void FGColor::Lighter(float factor)
{
	if (m_luminance == 0)
	    m_luminance = 0.01;

	m_luminance *= factor;

	if (m_luminance < 0)
	    m_luminance = 0;
	if (m_luminance > 1.)
		m_luminance = 1.;
	RecomputeRGB();
	RecomputeValue();
}

void FGColor::Darker(float factor)
{
	if (factor != 0)
	    m_luminance /= factor;

	if (m_luminance < 0)
		m_luminance = 0;
	if (m_luminance > 1.)
		m_luminance = 1.;
	RecomputeRGB();
	RecomputeValue();
}

void FGColor::Opposite(void)
{
    if (m_hue >=180.)
        SetHue(m_hue-180.);
    else
        SetHue(m_hue+180.);
}

/**
* Sets the red portion of the color. Values must be in the range from 0 to 255.
*/
void FGColor::SetRed(int red)
{
  m_red = static_cast<unsigned char>(red);
  RecomputeHLS();
  RecomputeValue();
}

/**
* Sets the green portion of the color. Values must be in the range from 0 to 255.
*/
void FGColor::SetGreen(int green)
{
  m_green = static_cast<unsigned char>(green);
  RecomputeHLS();
  RecomputeValue();
}

/**
* Sets the blue portion of the color. Values must be in the range from 0 to 255.
*/
void FGColor::SetBlue(int blue)
{
  m_blue = static_cast<unsigned char>(blue);
  RecomputeHLS();
  RecomputeValue();
}

/**
* Combines the methods SetRed, SetGreen and SetBlue in one step.
*/

void FGColor::SetRGB(int red, int green, int blue)
{
  m_red   = static_cast<unsigned char>(red);
  m_green = static_cast<unsigned char>(green);
  m_blue  = static_cast<unsigned char>(blue);
  RecomputeHLS();
  RecomputeValue();
}

// HSL
/**
* Sets the hue. The parameter is interpreted as an angle in the color circle(0.0 - 360.0
* Degree). Red is positioned at 0 degree, green at 120 degree and blue at 240 Degree.
*/
void FGColor::SetHue(float hue)
{
  if (hue < 0.0 && hue > 360.0)
    hue = ((int)hue) % 360;
  m_hue = hue;
  RecomputeRGB();
  RecomputeValue();
}

/**
* Sets the saturation of the color. The parameter is normed, its value must be in
* the range between 0.0 (gray, absence of all color) and 1.0 (pure colors).
*/
void FGColor::SetSaturation(float saturation)
{
  if (saturation < 0.0 && saturation > 1.0) saturation = 0.5;
  m_saturation = saturation;
  RecomputeRGB();
  RecomputeValue();
}

/**
* Sets the luminance of the color. The parameter is normed, its value must be in
* the range between 0.0 (black) and 1.0 (white).
*/
void FGColor::SetLuminance(float luminance)
{
  if (luminance < 0.0 && luminance > 1.0) luminance = 0.5;
  m_luminance = luminance;
  RecomputeRGB();
  RecomputeValue();
}

/**
* Combines the methods SetHue, SetLuminance and SetSaturation in one step.
*/
void FGColor::SetHLS(float hue, float luminance, float saturation)
{
  if (hue < 0.0 && hue > 360.0)
    hue = ((int)hue) % 360;
  if (luminance < 0.0 && luminance > 1.0) luminance = 0.5;
  if (saturation < 0.0 && saturation > 1.0) saturation = 0.5;

  m_hue = hue;
  m_luminance = luminance;
  m_saturation = saturation;
  RecomputeRGB();
  RecomputeValue();
}

// Konverting

void FGColor::RecomputeValue(void)
{
#ifdef INDEX_COLORS
    m_pixel = CreateColor(m_red, m_green, m_blue, -1);
#else
	#ifdef BPP32
	m_pixel = *(unsigned *)&m_blue;
	#else
	m_pixel = FGMakeColor(m_red, m_green, m_blue);
	#endif
#endif
}

void FGColor::RecomputeHLS(void)
{
    unsigned char minval = min(m_red, min(m_green, m_blue));
    unsigned char maxval = max(m_red, max(m_green, m_blue));
    float mdiff  = float(maxval) - float(minval);
    float msum   = float(maxval) + float(minval);

    m_luminance = msum / 510.0f;

    if (maxval == minval)
    {
      m_saturation = 0.0f;
      m_hue = 0.0f;
    }
    else
    {
      float rnorm = (maxval - m_red  ) / mdiff;
      float gnorm = (maxval - m_green) / mdiff;
      float bnorm = (maxval - m_blue ) / mdiff;

      m_saturation = (m_luminance <= 0.5f) ? (mdiff / msum) : (mdiff / (510.0f - msum));

      if (m_red   == maxval) m_hue = 60.0f * (6.0f + bnorm - gnorm);
      if (m_green == maxval) m_hue = 60.0f * (2.0f + rnorm - bnorm);
      if (m_blue  == maxval) m_hue = 60.0f * (4.0f + gnorm - rnorm);
      if (m_hue > 360.0f) m_hue = m_hue - 360.0f;
    }
}

void FGColor::RecomputeRGB(void)
{
	if (m_saturation == 0.0) // Grauton, einfacher Fall
    {
      m_red = m_green = m_blue = (unsigned char)(m_luminance * 255.0);
    }
    else
    {
      float rm1, rm2;

      if (m_luminance <= 0.5f) rm2 = m_luminance + m_luminance * m_saturation;
      else                     rm2 = m_luminance + m_saturation - m_luminance * m_saturation;
      rm1 = 2.0f * m_luminance - rm2;
      m_red   = ToRGB1(rm1, rm2, m_hue + 120.0f);
      m_green = ToRGB1(rm1, rm2, m_hue);
      m_blue  = ToRGB1(rm1, rm2, m_hue - 120.0f);
    }
}

unsigned char FGColor::ToRGB1(float rm1, float rm2, float rh)
{
  if      (rh > 360.0f) rh -= 360.0f;
  else if (rh <   0.0f) rh += 360.0f;

  if      (rh <  60.0f) rm1 = rm1 + (rm2 - rm1) * rh / 60.0f;
  else if (rh < 180.0f) rm1 = rm2;
  else if (rh < 240.0f) rm1 = rm1 + (rm2 - rm1) * (240.0f - rh) / 60.0f;

  return static_cast<unsigned char>(rm1 * 255);
}

/**
* Gets the RGB color value as text in a hexadecimal representation int
* the format "RRGGBB". Example: red is returned as "FF0000", green as "00FF00"
* and blue as "0000FF".
* @note you must free() returned pointer!!!
*/
char* FGColor::GetString() const
{
  char* color = (char *)malloc(8);
  sprintf(color, "%02X%02X%02X", GetRed(), GetGreen(), GetBlue());
  return color;
}

/**
* Initializes the object with the color, whose RGB color value is scanned from
* the passed string pcColor. It should contain a string in the hexadecimal
* format RRGGBB. If no valid color could be scanned from the string, the method
* returns false, else true. Example: "FF0000" sets the color to red,  "00FF00"
* to green and "0000FF" to blue.
*/
bool FGColor::SetString(const char* pcColor)
{
  assert(pcColor);
  int r, g, b;

  if (sscanf(pcColor, "%2x%2x%2x", &r, &g, &b) != 3)
  {
    m_red = m_green = m_blue = 0;
	RecomputeHLS();
	RecomputeValue();
    return false;
  }
  else
  {
    m_red   = static_cast<unsigned char>(r);
    m_green = static_cast<unsigned char>(g);
    m_blue  = static_cast<unsigned char>(b);
	RecomputeHLS();
	RecomputeValue();
    return true;
  }
}

/**
* Gets the user friendly name of the color. When no name is known for the color,
* a string in the html format "#RRGGBB" will be returned.
*/
const char* FGColor::GetName() const
{
  int i = numNamedColors;
  while (i-- && m_pixel != m_namedColor[i].color);
  if (i < 0)
  {
	char *ptr = GetString();
	memmove(ptr, ptr+1, 7);
	ptr[0] = '#';
	return ptr;
  }
  else return m_namedColor[i].name;
}

#ifdef INDEX_COLORS

static int colorsFree = 256, startFree;
static int range;

// set color in palette
void _palette(unsigned col, unsigned rgb)
{
	col &= 255;
	// call palette trought vector
	if (vector_palette) vector_palette(col, rgb);
	_fgl_palette[col] = rgb;
}

// absolete
void set_palette_map(int cnt, int ind, int *pal)
{
	unsigned int color, x = 0;

	for (; cnt; cnt--)
	{
		color = (unsigned int) pal[x++];
		_palette(ind++, color);
	}
}

//! get color from palette
unsigned int FGAPI get_palette(unsigned int i)
{
	return _fgl_palette[i];
}

//! refresh current palette
void FGAPI _set_fgl_palette(void)
{
	int i;

	for (i = 0; i < 256; i++)
		_palette(i, _fgl_palette[i]);
}

//! set startup palette
void FGAPI _set_default_palette(void)
{
	memmove(_fgl_palette, _default_palette, sizeof(_fgl_palette));
	_set_fgl_palette();
}

void FGAPI GetPaletteEntry(unsigned char *rc, unsigned char *gc, unsigned char *bc, int i)
{
	if (i < 0 || i > 255)
		return;
	*rc = ColorsArray[i].r;
	*gc = ColorsArray[i].g;
	*bc = ColorsArray[i].b;
}

void FGAPI DeleteColor(int idx)
{
	if (idx < 0 || idx > 255)
		return;

	if (ColorsArray[idx].alfa == 1)		// is free
	{
		ColorsArray[idx].r = (char) idx;	// create an gray color
		ColorsArray[idx].g = (char) idx;
		ColorsArray[idx].b = (char) idx;
		ColorsArray[idx].alfa = 0;
		colorsFree++;
	}
	else if (ColorsArray[idx].alfa > 1)
		ColorsArray[idx].alfa--;
}

void FGAPI SetColorFuzzy(int a)
{
	range = (a*4) & 15;
}

int FGAPI GetFreeColors(void)
{
	return colorsFree;
}

void FGAPI CreatePaletteEntry(int rc, int gc, int bc, int idx)
{
	if (idx < 0 || idx > 255)
		return;
	ColorsArray[idx].r = (char) rc;
	ColorsArray[idx].g = (char) gc;
	ColorsArray[idx].b = (char) bc;
	ColorsArray[idx].alfa++;	// incr used counter

	_palette(idx, bc + gc * 256U + rc * 65536U);
}

static FGPixel FGAPI CreateColor2(int r, int g, int b)
{
	// first we look at current colors for identical (+-3%) color
	int i, r0, g0, b0, first = 0;

	for (i = 0; i < 256; i++)
	{
		r0 = ColorsArray[i].r;
		g0 = ColorsArray[i].g;
		b0 = ColorsArray[i].b;
		if (ColorsArray[i].alfa)	// is existing colors ?

		{
			if ((r0 - range <= r && r0 + range >= r)
				&& (g0 - range <= g && g0 + range >= g)
				&& (b0 - range <= b && b0 + range >= b))
			{
				// WOW !!! we founds loved color ...
				first = i;
				if (r0 == r && g0 == g && b0 == b)	// must be equivalent

				{
					ColorsArray[i].alfa++;
					return i;	// return palette index

				}
			}
		}
	}

	if (first)
	{
		ColorsArray[first].alfa++;
		return first;			// return palette index

	}

	// we must create new palette entry for your color

	if (colorsFree == 0)
		return (FGPixel) - 1;	// ERROR I haven't any palette entry for one

	colorsFree--;
	for (i = startFree; i < 256; i++)
	{
		if (ColorsArray[i].alfa == 0)	// is free color ?

		{
			CreatePaletteEntry(r, g, b, i);
			return (FGPixel) i;
		}
	}
	return (FGPixel) - 2;		// ERROR - data mismatch !!!

}

//  WARNING colors are in range 0..255 !!!

FGPixel FGAPI CreateColor(int r, int g, int b, int ind)
{
	if (ind < 0 || ind > 255)
	{
		return CreateColor2(r, g, b);
	}
	CreatePaletteEntry(r, g, b, ind);
	if (ColorsArray[ind].alfa > 1)
		return (FGPixel) ind;	// return palette index

	if (colorsFree == 0)
		return (FGPixel) - 1;	// ERROR I haven't any palette entry for one

	colorsFree--;
	return (FGPixel) ind;		// ERROR - data mismatch !!!

}

#else

int FGAPI GetFreeColors(void)
{
	return 0xFFFFFF;
}

void FGAPI SetColorFuzzy(int a)
{}

FGPixel FGAPI CreateColor(int r, int g, int b, int)
{
	return FGPixel(FGMakeColor(r, g, b));
}
#endif


#ifdef FG_NAMESPACE
}
#endif

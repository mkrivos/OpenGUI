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

*/

#ifndef __COLOR_H
#define __COLOR_H

#ifdef __BORLANDC__
#pragma option push -b
#pragma option push -a4
#endif

#ifdef FG_NAMESPACE
namespace fgl
{
#endif

#ifdef INDEX_COLORS
/**
	The unsigned integer that is equal in size with the one graphic pixel
	representation in VRAM, i.e. char for indexed colors, int for true colors (24 or 32 bit modes).
	@ingroup Types
*/
typedef unsigned char FGPixel;	// type of color (must correspond with bpp)
#endif

#if (DIRECT_COLORS==15) | (DIRECT_COLORS==16)
typedef unsigned short FGPixel;		// type of color (must correspond with bpp)
#endif

#ifdef TRUE_COLORS
typedef unsigned int FGPixel;	// type of color (must correspond with bpp)
#endif

/**
	@addtogroup Colors
	@{
*/

#ifdef INDEX_COLORS
//! Constant for the BLACK color
const FGPixel CBLACK =  0;
//! Constant for the DARK_GRAY color
const FGPixel CDARK	= 1;
//! Constant for the another GRAY color
const FGPixel CGRAYED =  2;
//! Constant for the GRAY color #1
const FGPixel CGRAY1 =  3;
//! Constant for the GRAY color #2
const FGPixel CGRAY2 =  4;
//! Constant for the GRAY color #3
const FGPixel CGRAY3 =  5;
//! Constant for the BLUE color
const FGPixel CBLUE	=  6;
//! Constant for the LIGHT BLUE color
const FGPixel CBLUELIGHT=  7;
//! Constant for the GREEN color
const FGPixel CGREEN =  8;
//! Constant for the LIGHT GREEN color
const FGPixel CGREENLIGHT = 9;
//! Constant for the RED color
const FGPixel CRED	=  10;
//! Constant for the LIGHT RED color
const FGPixel CREDLIGHT =  11;
//! Constant for the BROWN color
const FGPixel CBROWN =  12;
//! Constant for the YELLOW color
const FGPixel CYELLOW = 13;
//! Constant for the DUN WHITE color
const FGPixel CWHITED = 14;
//! Constant for the 100% WHITE color
const FGPixel CWHITE =  15;

#else
const FGPixel CBLACK = 0;
const FGPixel CDARK	= FGDirectColor(0x444444U);
const FGPixel CGRAYED = FGDirectColor(0x585858U);
const FGPixel CGRAY1 = FGDirectColor(0x7c8074U);
const FGPixel CGRAY2 = FGDirectColor(0x687884U);
const FGPixel CGRAY3 =	FGDirectColor(0xa8b8acU);
const FGPixel CBLUE	= FGDirectColor(0x000078U);
const FGPixel CBLUELIGHT = FGDirectColor(0x0000f8U);
const FGPixel CGREEN = FGDirectColor(0x007800U);
const FGPixel CGREENLIGHT = FGDirectColor(0x00f800U);
const FGPixel CRED = FGDirectColor(0xd44448U);
const FGPixel CREDLIGHT	= FGDirectColor(0xf80000U);
const FGPixel CBROWN = FGDirectColor(0x684028U);
const FGPixel CYELLOW = FGDirectColor(0xfce000U);
const FGPixel CWHITED = FGDirectColor(0xd8d0b8U);
const FGPixel CWHITE = FGDirectColor(0xffffffU);
#endif // TRUE_COLORS

const unsigned UNDEFINED_COLOR = unsigned(0xffffffffU);
const unsigned DEFAULT_COLOR_KEY = unsigned(0x12345678U);
const unsigned DEFAULT_ALPHA_VALUE = unsigned(0);

#ifdef INDEX_COLORS
/**
	Remap the indexed colors to the truecolor directly by palette color map.
	Useful macro when you port your 256 color modes app to the truecolors.
*/
#define FGCOLOR(i) (i)
#else
#define FGCOLOR(i) ((fgl::FGPixel) FGDirectColor(fgl::_fgl_palette[i]))
#endif

/**
	Internal - one palette record
*/
struct FGPalette
{
		unsigned char b;
		unsigned char g;
		unsigned char r;
		unsigned char alfa;
	  public:
		FGPalette(void)
		{
			r = g = b = alfa = 0;
		}
		FGPalette(unsigned p)
		{
			*((unsigned *) this) = p;
		}
		FGPalette(unsigned char _r, unsigned char _g, unsigned char _b)
		{
			r = _r;
			g = _g;
			b = _b;
			alfa = 0;
		}
		unsigned int GetValue(void)
		{
			return *((unsigned *) this);
		}
};

extern unsigned int _fgl_palette[256];

/**
* ABGR components
*/
typedef unsigned PIXEL_ARGB;

struct FGNamedColor
{
	PIXEL_ARGB  color;
	const char*  name;
};

/*!
	\brief The FGColor class provides colors based on RGB or HSV values.

	A color is normally specified in terms of RGB (red, green and blue)
	components, but it is also possible to specify HSV (hue, saturation
	and value) or set a color name (the names are copied from from the
	X11 color database).

	In addition to the RGB value, a FGColor also has a PIXEL_ARGB value and a
	validity. The PIXEL_ARGB value is used by the underlying window system
	to refer to a color. It can be thought of as an index into the
	display hardware's color table.

	\image html palette1.png
	\image html palette2.png
	\image html palette3.png Predefined X11 colors

	A color can be set by passing setNamedColor() an RGB string like
	"#112233", or a color name, e.g. "blue". The names are taken from
	X11's rgb.txt database but can also be used under Windows. To get
	a lighter or darker color use light() and dark() respectively.
	Colors can also be set using setRgb() and setHsv(). The color
	components can be accessed in one go with rgb() and hsv(), or
	individually with red(), green() and blue().

	Because many people don't know the HSV color model very well, we'll
	cover it briefly here.

	The RGB model is hardware-oriented. Its representation is close to
	what most monitors show. In contrast, HSV represents color in a way
	more suited to the human perception of color. For example, the
	relationships "stronger than", "darker than" and "the opposite of"
	are easily expressed in HSV but are much harder to express in RGB.

	HSV, like RGB, has three components:

	- H, for hue, is either 0-359 if the color is chromatic (not
	gray), or meaningless if it is gray. It represents degrees on the
	color wheel familiar to most people. Red is 0 (degrees), green is
	120 and blue is 240.

	- S, for saturation, is 0-1.0, and the bigger it is, the
	stronger the color is. Grayish colors have saturation near 0; very
	strong colors have saturation near 1.0.

	- V, for value, is 0-1.0 and represents lightness or brightness
	of the color. 0 is black; 1.0 is as far from black as possible.


	Here are some examples: Pure red is H=0, S=1.0, V=1.0. A dark red,
	moving slightly towards the magenta, could be H=350 (equivalent to -10), S=1.0, V=180. A grayish light red could have H about 0 (say
	350-359 or 0-10), S about 50-100, and S=1.0.

	OpenGUI returns a hue value of -1 for achromatic colors. If you pass a
	too-big hue value, it forces it into range. Hue 360 or 720 is
	treated as 0; hue 540 is treated as 180.

	@see FGColorPicker FGColor, FG2Colors, FG3Colors, FG4Colors, FGRelatedColors

	http://www.inforamp.net/~poynton/Poynton-color.html Color FAQ
*/
class FGColor
{
  public:

/**
	For 140 colors names are defined. The same names are used in html, the names
	and corresponding color values are copied from the Internet Explorer
	documentation (MSDN January 99: "Platform SDK/Internet/DHTML/Additional References/Color Table").
	Because the elements of an enumeration are implicitly converted to integer and
	a PIXEL_ARGB is only a typedef for an uint, ENamedColor values can be used everywhere
	a PIXEL_ARGB value or a CColor object is expected. Example:
	@code
	...
	my_window->FGWindow::WindowText(0,0,"Hello", FGColor(FGColor::coral), FGColor(FGColor::blue) );
	...
	@endcode
*/
  enum ENamedColor // Named Colors, could be used as PIXEL_ARGB
  {
	none                 = 0xFFFFFFFF,   // no color
	aliceblue            = 0x00F0F8FF,   // RGB(0xF0, 0xF8, 0xFF)
	antiquewhite         = 0x00FAEBD7,   // RGB(0xFA, 0xEB, 0xD7)
	aqua                 = 0x0000FFFF,   // RGB(0x00, 0xFF, 0xFF)
	aquamarine           = 0x007FFFD4,   // RGB(0x7F, 0xFF, 0xD4)
	azure                = 0x00F0FFFF,   // RGB(0xF0, 0xFF, 0xFF)
	beige                = 0x00F5F5DC,   // RGB(0xF5, 0xF5, 0xDC)
	bisque               = 0x00FFE4C4,   // RGB(0xFF, 0xE4, 0xC4)
	black                = 0x00000000,   // RGB(0x00, 0x00, 0x00)
	blanchedalmond       = 0x00FFEBCD,   // RGB(0xFF, 0xEB, 0xCD)
	blue                 = 0x000000FF,   // RGB(0x00, 0x00, 0xFF)
	blueviolet           = 0x008A2BE2,   // RGB(0x8A, 0x2B, 0xE2)
	brown                = 0x00A52A2A,   // RGB(0xA5, 0x2A, 0x2A)
	burlywood            = 0x00DEB887,   // RGB(0xDE, 0xB8, 0x87)
	cadetblue            = 0x005F9EA0,   // RGB(0x5F, 0x9E, 0xA0)
	chartreuse           = 0x007FFF00,   // RGB(0x7F, 0xFF, 0x00)
	chocolate            = 0x00D2691E,   // RGB(0xD2, 0x69, 0x1E)
	coral                = 0x00FF7F50,   // RGB(0xFF, 0x7F, 0x50)
	cornflower           = 0x006495ED,   // RGB(0x64, 0x95, 0xED)
	cornsilk             = 0x00FFF8DC,   // RGB(0xFF, 0xF8, 0xDC)
	crimson              = 0x00DC143C,   // RGB(0xDC, 0x14, 0x3C)
	cyan                 = 0x0000FFFF,   // RGB(0x00, 0xFF, 0xFF)
	darkblue             = 0x0000008B,   // RGB(0x00, 0x00, 0x8B)
	darkcyan             = 0x00008B8B,   // RGB(0x00, 0x8B, 0x8B)
	darkgoldenrod        = 0x00B8860B,   // RGB(0xB8, 0x86, 0x0B)
	darkgray             = 0x00A9A9A9,   // RGB(0xA9, 0xA9, 0xA9)
	darkgreen            = 0x00006400,   // RGB(0x00, 0x64, 0x00)
	darkkhaki            = 0x00BDB76B,   // RGB(0xBD, 0xB7, 0x6B)
	darkmagenta          = 0x008B008B,   // RGB(0x8B, 0x00, 0x8B)
	darkolivegreen       = 0x00556B2F,   // RGB(0x55, 0x6B, 0x2F)
	darkorange           = 0x00FF8C00,   // RGB(0xFF, 0x8C, 0x00)
	darkorchid           = 0x009932CC,   // RGB(0x99, 0x32, 0xCC)
	darkred              = 0x008B0000,   // RGB(0x8B, 0x00, 0x00)
	darksalmon           = 0x00E9967A,   // RGB(0xE9, 0x96, 0x7A)
	darkseagreen         = 0x008FBC8B,   // RGB(0x8F, 0xBC, 0x8B)
	darkslateblue        = 0x00483D8B,   // RGB(0x48, 0x3D, 0x8B)
	darkslategray        = 0x002F4F4F,   // RGB(0x2F, 0x4F, 0x4F)
	darkturquoise        = 0x0000CED1,   // RGB(0x00, 0xCE, 0xD1)
	darkviolet           = 0x009400D3,   // RGB(0x94, 0x00, 0xD3)
	deeppink             = 0x00FF1493,   // RGB(0xFF, 0x14, 0x93)
	deepskyblue          = 0x0000BFFF,   // RGB(0x00, 0xBF, 0xFF)
	dimgray              = 0x00696969,   // RGB(0x69, 0x69, 0x69)
	dodgerblue           = 0x001E90FF,   // RGB(0x1E, 0x90, 0xFF)
	firebrick            = 0x00B22222,   // RGB(0xB2, 0x22, 0x22)
	floralwhite          = 0x00FFFAF0,   // RGB(0xFF, 0xFA, 0xF0)
	forestgreen          = 0x00228B22,   // RGB(0x22, 0x8B, 0x22)
	fuchsia              = 0x00FF00FF,   // RGB(0xFF, 0x00, 0xFF)
	gainsboro            = 0x00DCDCDC,   // RGB(0xDC, 0xDC, 0xDC)
	ghostwhite           = 0x00F8F8FF,   // RGB(0xF8, 0xF8, 0xFF)
	gold                 = 0x00FFD700,   // RGB(0xFF, 0xD7, 0x00)
	goldenrod            = 0x00DAA520,   // RGB(0xDA, 0xA5, 0x20)
	gray                 = 0x00808080,   // RGB(0x80, 0x80, 0x80)
	green                = 0x00008000,   // RGB(0x00, 0x80, 0x00)
	greenyellow          = 0x00ADFF2F,   // RGB(0xAD, 0xFF, 0x2F)
	honeydew             = 0x00F0FFF0,   // RGB(0xF0, 0xFF, 0xF0)
	hotpink              = 0x00FF69B4,   // RGB(0xFF, 0x69, 0xB4)
	indianred            = 0x00CD5C5C,   // RGB(0xCD, 0x5C, 0x5C)
	indigo               = 0x004B0082,   // RGB(0x4B, 0x00, 0x82)
	ivory                = 0x00FFFFF0,   // RGB(0xFF, 0xFF, 0xF0)
	khaki                = 0x00F0E68C,   // RGB(0xF0, 0xE6, 0x8C)
	lavender             = 0x00E6E6FA,   // RGB(0xE6, 0xE6, 0xFA)
	lavenderblush        = 0x00FFF0F5,   // RGB(0xFF, 0xF0, 0xF5)
	lawngreen            = 0x007CFC00,   // RGB(0x7C, 0xFC, 0x00)
	lemonchiffon         = 0x00FFFACD,   // RGB(0xFF, 0xFA, 0xCD)
	lightblue            = 0x00ADD8E6,   // RGB(0xAD, 0xD8, 0xE6)
	lightcoral           = 0x00F08080,   // RGB(0xF0, 0x80, 0x80)
	lightcyan            = 0x00E0FFFF,   // RGB(0xE0, 0xFF, 0xFF)
	lightgoldenrodyellow = 0x00FAFAD2,   // RGB(0xFA, 0xFA, 0xD2)
	lightgreen           = 0x0090EE90,   // RGB(0x90, 0xEE, 0x90)
	lightgrey            = 0x00D3D3D3,   // RGB(0xD3, 0xD3, 0xD3)
	lightpink            = 0x00FFB6C1,   // RGB(0xFF, 0xB6, 0xC1)
	lightsalmon          = 0x00FFA07A,   // RGB(0xFF, 0xA0, 0x7A)
	lightseagreen        = 0x0020B2AA,   // RGB(0x20, 0xB2, 0xAA)
	lightskyblue         = 0x0087CEFA,   // RGB(0x87, 0xCE, 0xFA)
	lightslategray       = 0x00778899,   // RGB(0x77, 0x88, 0x99)
	lightsteelblue       = 0x00B0C4DE,   // RGB(0xB0, 0xC4, 0xDE)
	lightyellow          = 0x00FFFFE0,   // RGB(0xFF, 0xFF, 0xE0)
	lime                 = 0x0000FF00,   // RGB(0x00, 0xFF, 0x00)
	limegreen            = 0x0032CD32,   // RGB(0x32, 0xCD, 0x32)
	linen                = 0x00FAF0E6,   // RGB(0xFA, 0xF0, 0xE6)
	magenta              = 0x00FF00FF,   // RGB(0xFF, 0x00, 0xFF)
	maroon               = 0x00800000,   // RGB(0x80, 0x00, 0x00)
	mediumaquamarine     = 0x0066CDAA,   // RGB(0x66, 0xCD, 0xAA)
	mediumblue           = 0x000000CD,   // RGB(0x00, 0x00, 0xCD)
	mediumorchid         = 0x00BA55D3,   // RGB(0xBA, 0x55, 0xD3)
	mediumpurple         = 0x009370DB,   // RGB(0x93, 0x70, 0xDB)
	mediumseagreen       = 0x003CB371,   // RGB(0x3C, 0xB3, 0x71)
	mediumslateblue      = 0x007B68EE,   // RGB(0x7B, 0x68, 0xEE)
	mediumspringgreen    = 0x0000FA9A,   // RGB(0x00, 0xFA, 0x9A)
	mediumturquoise      = 0x0048D1CC,   // RGB(0x48, 0xD1, 0xCC)
	mediumvioletred      = 0x00C71585,   // RGB(0xC7, 0x15, 0x85)
	midnightblue         = 0x00191970,   // RGB(0x19, 0x19, 0x70)
	mintcream            = 0x00F5FFFA,   // RGB(0xF5, 0xFF, 0xFA)
	mistyrose            = 0x00FFE4E1,   // RGB(0xFF, 0xE4, 0xE1)
	moccasin             = 0x00FFE4B5,   // RGB(0xFF, 0xE4, 0xB5)
	navajowhite          = 0x00FFDEAD,   // RGB(0xFF, 0xDE, 0xAD)
	navy                 = 0x00000080,   // RGB(0x00, 0x00, 0x80)
	oldlace              = 0x00FDF5E6,   // RGB(0xFD, 0xF5, 0xE6)
	olive                = 0x00808000,   // RGB(0x80, 0x80, 0x00)
	olivedrab            = 0x006B8E23,   // RGB(0x6B, 0x8E, 0x23)
	orange               = 0x00FFA500,   // RGB(0xFF, 0xA5, 0x00)
	orangered            = 0x00FF4500,   // RGB(0xFF, 0x45, 0x00)
	orchid               = 0x00DA70D6,   // RGB(0xDA, 0x70, 0xD6)
	palegoldenrod        = 0x00EEE8AA,   // RGB(0xEE, 0xE8, 0xAA)
	palegreen            = 0x0098FB98,   // RGB(0x98, 0xFB, 0x98)
	paleturquoise        = 0x00AFEEEE,   // RGB(0xAF, 0xEE, 0xEE)
	palevioletred        = 0x00DB7093,   // RGB(0xDB, 0x70, 0x93)
	papayawhip           = 0x00FFEFD5,   // RGB(0xFF, 0xEF, 0xD5)
	peachpuff            = 0x00FFDAB9,   // RGB(0xFF, 0xDA, 0xB9)
	peru                 = 0x00CD853F,   // RGB(0xCD, 0x85, 0x3F)
	pink                 = 0x00FFC0CB,   // RGB(0xFF, 0xC0, 0xCB)
	plum                 = 0x00DDA0DD,   // RGB(0xDD, 0xA0, 0xDD)
	powderblue           = 0x00B0E0E6,   // RGB(0xB0, 0xE0, 0xE6)
	purple               = 0x00800080,   // RGB(0x80, 0x00, 0x80)
	red                  = 0x00FF0000,   // RGB(0xFF, 0x00, 0x00)
	rosybrown            = 0x00BC8F8F,   // RGB(0xBC, 0x8F, 0x8F)
	royalblue            = 0x004169E1,   // RGB(0x41, 0x69, 0xE1)
	saddlebrown          = 0x008B4513,   // RGB(0x8B, 0x45, 0x13)
	salmon               = 0x00FA8072,   // RGB(0xFA, 0x80, 0x72)
	sandybrown           = 0x00F4A460,   // RGB(0xF4, 0xA4, 0x60)
	seagreen             = 0x002E8B57,   // RGB(0x2E, 0x8B, 0x57)
	seashell             = 0x00FFF5EE,   // RGB(0xFF, 0xF5, 0xEE)
	sienna               = 0x00A0522D,   // RGB(0xA0, 0x52, 0x2D)
	silver               = 0x00C0C0C0,   // RGB(0xC0, 0xC0, 0xC0)
	skyblue              = 0x0087CEEB,   // RGB(0x87, 0xCE, 0xEB)
	slateblue            = 0x006A5ACD,   // RGB(0x6A, 0x5A, 0xCD)
	slategray            = 0x00708090,   // RGB(0x70, 0x80, 0x90)
	snow                 = 0x00FFFAFA,   // RGB(0xFF, 0xFA, 0xFA)
	springgreen          = 0x0000FF7F,   // RGB(0x00, 0xFF, 0x7F)
	steelblue            = 0x004682B4,   // RGB(0x46, 0x82, 0xB4)
	tan                  = 0x00D2B48C,   // RGB(0xD2, 0xB4, 0x8C)
	teal                 = 0x00008080,   // RGB(0x00, 0x80, 0x80)
	thistle              = 0x00D8BFD8,   // RGB(0xD8, 0xBF, 0xD8)
	tomato               = 0x00FF6347,   // RGB(0xFF, 0x63, 0x47)
	turquoise            = 0x0040E0D0,   // RGB(0x40, 0xE0, 0xD0)
	violet               = 0x00EE82EE,   // RGB(0xEE, 0x82, 0xEE)
	wheat                = 0x00F5DEB3,   // RGB(0xF5, 0xDE, 0xB3)
	white                = 0x00FFFFFF,   // RGB(0xFF, 0xFF, 0xFF)
	whitesmoke           = 0x00F5F5F5,   // RGB(0xF5, 0xF5, 0xF5)
	yellow               = 0x00FFFF00,   // RGB(0xFF, 0xFF, 0x00)
	yellowgreen          = 0x009ACD32    // RGB(0x9A, 0xCD, 0x32)
  };

  /**
	Constructs an object of the class FGColor and initializes it with the passed color cr.
	At the input is ARGB PIXEL_ARGB (or palette entry index for indexed colors).
	The use of a default parameter implements in one step the default constructor
	and the conversion constructor from type PIXEL_ARGB to type FGColor. Because of this,
	the following code is legal, although no special assignment operator is defined:
	@code
	FGColor black;                      // intialized to black by default
	FGColor c1(FGDirectColor(255, 0, 0));   // initialized to red
	FGColor c2(FGColor::seagreen);      // FGColor::ENamedColor is PIXEL_ARGB compatible
	FGColor c3(80,80,80);               // R,G,B

	PIXEL_ARGB r = FGDirectColor(200, 50, 200);

	c1 = r;                  // legal, implicit call of FGColor::FGColor(r)
	c2 = FGDirectColor(100, 150, 200); // dito, evaluates to FGColor::FGColor(FGDirectColor(100, 150, 250));

	@endcode
  */
  FGColor(PIXEL_ARGB cr);
  //! Connstruct object from R, G and B components.
  FGColor(int r, int g, int b);
  /**
  *  Connstruct object aka named color.
  *  @see enum ENamedColor
  */
  FGColor(const char* name);
  FGColor();
  ~FGColor();

	/**
	* Returns PIXEL_ARGB value for this color.
	*/
	operator PIXEL_ARGB() const
	{
		return m_pixel;
	}
	//! Desaturates color.
	void Desaturate(void);
	/*!
		Returns a lighter (or darker) color.

		Returns a lighter color if \a factor is greater than 1.0 Setting
		\a factor to 1.5 returns a color that is 50% brighter.

		Returns a darker color if \a factor is less than 1.0. We recommend
		using Darker() for this purpose. If \a factor is 0 or negative, the
		return value is unspecified.

		(This function converts the current RGB color to HSV, multiplies V
		by \a factor, and converts the result back to RGB.)

		\sa Darker()
	*/
	void Lighter(float factor=1.5);
	/*!
		Returns a darker (or lighter) color.

		Returns a darker color if \a factor is greater than 1.0 Setting
		\a factor to 3.0 returns a color that has one-third the
		brightness.

		Returns a lighter color if \a factor is less than 1.0 We
		recommend using lighter() for this purpose. If \a factor is 0 or
		negative, the return value is unspecified.

		(This function converts the current RGB color to HSV, divides V by
		\a factor and converts back to RGB.)

		\sa Lighter()
	*/
	void Darker(float factor=2.0);

	/**
	* Changes the color to the HUE+180deg, i.e. opposite color.
	* By example: yellow to blue
	*/
    void Opposite(void);

  // RGB - Routinen
  // --------------
  void SetRed(int red);     // 0..255
  void SetGreen(int green); // 0..255
  void SetBlue(int blue);   // 0..255
  void SetRGB(int red, int green, int blue);
  //! Gets the red portion of the color. The return value lies in the range from 0 to 255.
  int GetRed() const
  {
	return m_red;
  }
  //! Gets the green portion of the color. The return value lies in the range from 0 to 255.
  int GetGreen() const
  {
	return m_green;
  }
  //! Gets the blue portion of the color. The return value lies in the range from 0 to 255.
  int GetBlue() const
  {
	return m_blue;
  }


  // HLS - Routinen
  // --------------
  void SetHue(float hue);               // 0.0 .. 360.0
  void SetLuminance(float luminance);   // 0.0 .. 1.0
  void SetSaturation(float saturation); // 0.0 .. 1.0

  void SetHLS(float hue, float luminance, float saturation);
  /**
  * Gets the hue. The return value is interpreted as an angle in the color circle(0.0 - 360.0 Degree).
  * Red is positioned at 0 degree, green at 120 degree and blue at 240 Degree.
  */
  float GetHue() const
  {
	return m_hue;
  }
  /**
  * Gets the saturation of the color. The return value is normed, its value lies in
  * the range between 0.0 (gray, absence of all colors) and 1.0 (pure color).
  */
  float GetSaturation() const
  {
	return m_saturation;
  }
  /**
  * Gets the luminance of the color. The return value is normed, its value lies in
  * the range between 0.0 (black) and 1.0 (white).
  */
  float GetLuminance() const
  {
	return m_luminance;
  }

  char*   GetString() const;          // String im Format RRGGBB
  bool    SetString(const char* pcColor); // String im Format RRGGBB

  const char* GetName() const;

  static const char* GetNameFromIndex(int i);
  static class FGColor GetColorFromIndex(int i);
  /**
  * Gets the number of named colors (the number of values enumerated
  * in ENamedColor and ENamedColorIndex).
  */
  static int    GetNumNames() { return numNamedColors; }

private:

  // Konvertierung
  // -------------
  void RecomputeRGB(void);
  void RecomputeHLS(void);
  void RecomputeValue(void);
  static unsigned char ToRGB1(float rm1, float rm2, float rh);

  // Daten
  // -----
  PIXEL_ARGB         m_pixel;

  unsigned char m_blue;
  unsigned char m_green;
  unsigned char m_red;
  unsigned char m_alpha;

  float m_hue;         // 0.0 .. 360.0  // Winkel
  float m_saturation;  // 0.0 .. 1.0    // Prozent
  float m_luminance;   // 0.0 .. 1.0    // Prozent

  enum ENamedColorIndex
  {
	i_aliceblue, i_antiquewhite, i_aqua, i_aquamarine, i_azure, i_beige, i_bisque, i_black,
	i_blanchedalmond, i_blue, i_blueviolet, i_brown, i_burlywood, i_cadetblue, i_chartreuse,
	i_chocolate, i_coral, i_cornflower, i_cornsilk, i_crimson, i_cyan, i_darkblue, i_darkcyan,
	i_darkgoldenrod, i_darkgray, i_darkgreen, i_darkkhaki, i_darkmagenta, i_darkolivegreen,
	i_darkorange, i_darkorchid, i_darkred, i_darksalmon, i_darkseagreen, i_darkslateblue,
	i_darkslategray, i_darkturquoise, i_darkviolet, i_deeppink, i_deepskyblue, i_dimgray,
	i_dodgerblue, i_firebrick, i_floralwhite, i_forestgreen, i_fuchsia, i_gainsboro,
	i_ghostwhite, i_gold, i_goldenrod, i_gray, i_green, i_greenyellow, i_honeydew, i_hotpink,
	i_indianred, i_indigo, i_ivory, i_khaki, i_lavender, i_lavenderblush, i_lawngreen,
	i_lemonchiffon, i_lightblue, i_lightcoral, i_lightcyan, i_lightgoldenrodyellow,
	i_lightgreen, i_lightgrey, i_lightpink, i_lightsalmon, i_lightseagreen, i_lightskyblue,
	i_lightslategray, i_lightsteelblue, i_lightyellow, i_lime, i_limegreen, i_linen,
	i_magenta, i_maroon, i_mediumaquamarine, i_mediumblue, i_mediumorchid, i_mediumpurple,
	i_mediumseagreen, i_mediumslateblue, i_mediumspringgreen, i_mediumturquoise,
	i_mediumvioletred, i_midnightblue, i_mintcream, i_mistyrose, i_moccasin, i_navajowhite,
	i_navy, i_oldlace, i_olive, i_olivedrab, i_orange, i_orangered, i_orchid, i_palegoldenrod,
	i_palegreen, i_paleturquoise, i_palevioletred, i_papayawhip, i_peachpuff, i_peru, i_pink,
	i_plum, i_powderblue, i_purple, i_red, i_rosybrown, i_royalblue, i_saddlebrown, i_salmon,
	i_sandybrown, i_seagreen, i_seashell, i_sienna, i_silver, i_skyblue, i_slateblue,
	i_slategray, i_snow, i_springgreen, i_steelblue, i_tan, i_teal, i_thistle, i_tomato,
	i_turquoise, i_violet, i_wheat, i_white, i_whitesmoke, i_yellow, i_yellowgreen,
	numNamedColors
  };

  static const FGNamedColor m_namedColor[numNamedColors];
};

/**
    Creates the opposite color to one, i.e. with HUE+180.
    By example from blue there will be blue and yellow.
	@see FGColorPicker FGColor, FG2Colors, FG3Colors, FG4Colors, FGRelatedColors
*/
class FG2Colors
{
        FGColor color0;
        FGColor color180;
    public:
        //! Creates opposite colors from @a base color.
        FG2Colors(FGColor& base)
        {
            color0 = color180 = base;
            color180.SetHue(color180.GetHue()+180);
        }
        //! Returns original color.
        FGColor GetColor0(void) { return color0; }
        //! Returns opposite color.
        FGColor GetColor1(void) { return color180; }
};

/**
    Creates the tricolor to one, i.e. with HUE+-120.
    By example from blue there will be blue, red and green.
	@see FGColorPicker FGColor, FG2Colors, FG3Colors, FG4Colors, FGRelatedColors
*/
class FG3Colors
{
        FGColor color0;
        FGColor color120;
        FGColor color240;
    public:
        //! Creates 3 opposite colors from @a base color.
        FG3Colors(FGColor& base)
        {
            color0 = color120 = color240 = base;
            color120.SetHue(color120.GetHue()+120);
            color240.SetHue(color240.GetHue()+240);
        }
        //! Returns original color.
		FGColor GetColor0(void) { return color0; }
        //! Returns opposite color with HUE+120.
        FGColor GetColor1(void) { return color120; }
        //! Returns opposite color with HUW+240.
        FGColor GetColor2(void) { return color240; }
};

/**
    Creates the opposite color to one, i.e. with HUE+-90 and 180.
	@see FGColorPicker FGColor, FG2Colors, FG3Colors, FG4Colors, FGRelatedColors
*/
class FG4Colors
{
        FGColor color0;
        FGColor color90;
        FGColor color180;
        FGColor color270;
    public:
        //! Creates tetracolor combination from @a base color.
        FG4Colors(FGColor& base)
        {
            color0 = color90 = color180  = color270 = base;
            color90.SetHue(color90.GetHue()+90);
            color180.SetHue(color180.GetHue()+180);
            color270.SetHue(color270.GetHue()+270);
		}
        //! Returns original color.
        FGColor GetColor0(void) { return color0; }
        //! Returns opposite color with HUE+90.
        FGColor GetColor1(void) { return color90; }
        //! Returns opposite color with HUE+180.
        FGColor GetColor2(void) { return color180; }
		//! Returns opposite color with HUE+270.
        FGColor GetColor3(void) { return color270; }
};

/**
    Creates four related colors with position +- 15 and 30 degree at HUE circle.
	@see FGColorPicker FGColor, FG2Colors, FG3Colors, FG4Colors, FGRelatedColors
*/
class FGRelatedColors
{
        FGColor colorminus30;
        FGColor colorminus15;
        FGColor color15;
        FGColor color30;
	public:
        //! Creates two pairs of related colors from @a base color.
        //! The base one and the opposite one.
        FGRelatedColors(FGColor& base)
        {
            colorminus30 = colorminus15 = color15  = color30 = base;
            colorminus30.SetHue(colorminus30.GetHue()-30);
            colorminus15.SetHue(colorminus15.GetHue()-15);
            color15.SetHue(color15.GetHue()+15);
            color30.SetHue(color30.GetHue()+30);
        }
        //! Returns related color at HUE-30.
        FGColor GetColor0(void) { return colorminus30; }
        //! Returns related color at HUE-15.
        FGColor GetColor1(void) { return colorminus15; }
        //! Returns related color at HUE+15.
        FGColor GetColor2(void) { return color15; }
        //! Returns related color at HUE+30.
        FGColor GetColor3(void) { return color30; }
};

/**
	@}
*/

#ifdef FG_NAMESPACE
}
#endif

#ifdef __BORLANDC__
#pragma option pop /* pop -a switch */
#pragma option pop /* pop -b */
#endif

#endif


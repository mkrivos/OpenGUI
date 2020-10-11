#ifdef __BORLANDC__
#pragma option push -b
#pragma option push -a4
#endif

#ifdef FG_NAMESPACE
namespace fgl {
#endif

struct variable_record;
struct fonthdr;

/**
Internal structure to describe built-in bitmap fonts;
*/
struct FGFont
{
		char	name[24];
		int type, width, height;
		variable_record *var; // null or ptr
		FGPixel *fontimg;
	public:
		FGFont()
		{
			memset(this, 0, sizeof(*this));
		}
};

#define MAX_FONTS		64

/**
	Predefined const for a built-in fixed and proportional fonts.
	@ingroup Enums
*/
enum ENUM_FONTS
{
	FONT0406,
	FONT0808,
	FONT0816,
	FONT1222,
	FONT1625,
	FONT2034,
	FONTSYS,
	FONTSYSMEDIUM,
	FONTSYSLIGHT,

	FONTLAST=(MAX_FONTS-1)
};

/**
 Describes the TrueType font provided by freetype library.
*/
class FGFontProperty
{
	public:
		/** for rendering */
		enum FONT_STYLE { ttfSolid, ttfShaded, ttfBlended };
	private:
		void	 	*_font;
		FONT_STYLE 	algorithm;
	public:
		/**
			Creates a TTF font face object for a Window text output.
			You can use a function Valid() to test for a valid font face.
			@param n a filename ("arial.ttf" by example).
			@param sz a size of font face in pixels.
		*/
		FGFontProperty(const char *n, int sz);
		/**
		Unload the font face from memory.
		*/
		~FGFontProperty();
		/**
		Sets a 'bold' font style.
		*/
		void Bold(int value);
		/**
		Sets an 'underline' font style.
		*/
		void Underline(int value);
		/**
		Sets an 'italic' font style.
		*/
		void Italic(int value);
		/**
		Sets a solid rendering, fast but not very nice. Default.
		*/
		void Solid(void) { algorithm = ttfSolid; }
		/**
		Sets a shaded rendering, slow but very nice.
		*/
		void Shaded(void) { algorithm = ttfShaded; }
		/**
		Sets a blended rendering, slow but very nice.
		@bug still not implemented.
		*/
		void Blended(void) { algorithm = ttfBlended; } // not implemented yet
		/**
		Test the object for a valid data.
		@return true if object contains a valid font face.
		*/
		bool Valid(void) { return !!_font; }
		FONT_STYLE GetAlgorithm(void) { return algorithm; }
		void * GetFace(void) { return _font; }
		char * PrepareFont(variable_record *var, int& size, int count, int offset, char *name);
		
		/**
		@return Text width in pixels on success, or 0 on error.
		 */
		int textwidth(const char* txt);
		/**
		Calculates text size (width and height) and set values into ww and hh
		@return Text true on success, false on error.
		 */
		bool textsize(const char* txt, int* ww, int* hh);
};

/**
	The OpenGUI interface to the font manager.
	It registers and deregisters variable in-memory or in-file fonts.
	These fonts maybe with fixed width or proportional width.
	Object provides these fonts to others.
*/
class FGFontManager
{
		static FGFont 	FGFonts[MAX_FONTS];
		static int 		font_counter;
		// register variable font from memory
		// img and data must be allocated with malloc
		static int FGAPI register_font_var(variable_record *data, char *img, int size, const char *desc);
		// register variable font from compiled font
		static int FGAPI register_font_image(fonthdr * hdr, char *img, const char *desc);
		static void FGAPI deregister_fonts(void);
		static int FGAPI _register_font(unsigned char *source, int width, int height, int count, int offset, variable_record * type, int index, const char *desc);
	public:
		FGFontManager();
		~FGFontManager();

		static int FGAPI register_font_ttf(const char *filename, int points, int count=128-' ', int offset=' ', const char *desc=0);
		static int FGAPI register_font_fix(unsigned char *source, int width, int height, int count, int offset, const char *desc);
		static int FGAPI register_font_file(const char *source, const char *desc);

		static int FGAPI textwidth(int ID, const char *txt);
		static int FGAPI textwidth(int ID, const char *txt, int cnt);
		static int FGAPI charwidth(int ID, int c);

		/**
		Tests font ID and returns pointer to.
		@return a valid font record if exists one, or 0 if doesn't exist.
		@param ID font ID for prepared font
		*/
		static FGFont * FGAPI get_font_ptr(int ID)
		{
			if (ID<0 || ID>font_counter)
				return 0;
			// test for defined fonts
			if (!FGFonts[ID].fontimg)
				return 0;
			return &FGFonts[ID];
		}
		/**
		@return returns the width in pixels for font ID or 0 if doesn't exists.
		for proportional fonts there is the width of bigger glyph in the font.
		@param ID font ID
		*/
		static inline int FGAPI GetW(int ID)
		{
			return FGFonts[ID].width;
		}
		/**
		@return returns the height in pixels for font ID or 0 if doesn't exists.
		@param ID font ID
		*/
		static inline int FGAPI GetH(int ID)
		{
			return FGFonts[ID].height;
		}
		/**
		@return returns the pointer to a structure for proportional
		font description or 0 if font is fixed size.
		@param ID font ID
		*/
		static inline variable_record * FGAPI IsVariableFont(int ID)
		{
			return FGFonts[ID].var;
		}
		/**
		@return returns the font pixelmap or 0 if desn't exists.
		@param ID font ID
		*/
		static inline FGPixel * FGAPI GetFontImage(int ID)
		{
			return FGFonts[ID].fontimg;
		}
		/**
		@return the font name (max. 23 chars in length+'\\0').
		@param ID font ID
		*/
		static inline char * FGAPI GetFontName(int ID)
		{
			return FGFonts[ID].name;
		}
		/**
		@return the number of registered fonts in system.
		*/
		static inline int FGAPI GetCounter(void)
		{
			return font_counter;
		}
};

extern FGFontManager fontman;

#ifdef FG_NAMESPACE
}
#endif

#ifdef __BORLANDC__
#pragma option pop /* pop -a switch */
#pragma option pop /* pop -b */
#endif



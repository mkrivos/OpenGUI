#include "fgbase.h"
//#include "fastgl.h"
#include "_fastgl.h"

#ifdef FG_TTF
#include "fgttf.h"
#endif

#ifdef FG_NAMESPACE
namespace fgl {
#endif

FGFont 	FGFontManager::FGFonts[MAX_FONTS];
int FGFontManager::font_counter = 0;
FGFontManager fontman;

////////////////// FONT MANAGER //////////////////////////////////////////////

/**
	Initializes font manager and creates all default system fonts.
	@see FGFont FontDialog
*/
FGFontManager::FGFontManager()
{
	memset(FGFonts, 0, sizeof(FGFonts));
	font_counter = 0;

	_register_font(miro0406, 4, 6, 95, ' ', 0, FONT0406, "fixed-04x06");
	_register_font(miro0808, 8, 8, 256, 0, 0, FONT0808, "fixed-08x08");
	_register_font(miro0816, 8, 16, 256, 0, 0, FONT0816, "fixed-08x16");
	_register_font(miro1220, 12, 20, 95, ' ', 0, FONT1222, "fixed-12x20");
	_register_font(miro1625, 16, 25, 95, ' ', 0, FONT1625, "fixed-16x25");
	_register_font(miro2034, 20, 34, 95, ' ', 0, FONT2034, "fixed-20x34");
	register_font_image((fonthdr *)msss16b, sizeof(fonthdr)+(char *)msss16b, "ms-sans 16 bold");
	register_font_image((fonthdr *)msss16, sizeof(fonthdr)+(char *)msss16, "ms-sans 16");
	register_font_image((fonthdr *)msss13, sizeof(fonthdr)+(char *)msss13, "ms-sans 13");
}

/**
	Destroys all system's registered fonts.
*/
FGFontManager::~FGFontManager()
{
	deregister_fonts();
}

/**
	registers fixed binary font
	@return	-1 if error, or font ID (to use with FGDrawBuffer::set_font())
	@param source the font pixmap (8-bit for a pixel, possible values are 0x00 and 0xff)
	@param width a width of the font
	@param height a height of the font
	@param count number of glyphs to extract from TTF file (128-32)
	@param offset offset of first glyph (32)
	@param desc the font face name.
	It is used with FontDialog to choose.
*/
int FGAPI FGFontManager::register_font_fix(unsigned char *source, int width, int height, int count, int offset, const char *desc)
{
	return _register_font(source, width, height, count, offset, 0, -1, desc);
}

//! register variable font from memory
//! img and data must be allocated with malloc
int FGAPI FGFontManager::register_font_var(variable_record *data, char *img, int size, const char *desc)
{
	img = (char *)realloc(img, size*sizeof(FGPixel));
	char *p1 = img+size-1;
	FGPixel *p2=(FGPixel *)img+size-1;
	while(size--)
		*p2-- = *p1--; // copy with color expand
	return _register_font((unsigned char *)img, data->mw, data->mh, 256, 0, data, -1, desc);
}

//! registers a variable font from the special font file
/**
	@return	-1 if error, or font ID (to use with FGDrawBuffer::set_font())
	@param source the filename
	@param desc the font face name.
	It is used with FontDialog to choose.
*/
int FGAPI FGFontManager::register_font_file(const char *source, const char *desc)
{
	fonthdr hdr;
	int size;
	char *img;
	FILE *f = fopen(source, "rb");

	if (f==0)
	{
		perror("register_font ");
		return -1;
	}
	fread(&hdr, sizeof(hdr), 1, f);

	fseek(f,0,SEEK_END);
	size=ftell(f)-sizeof(fonthdr);
	fseek(f,sizeof(fonthdr),SEEK_SET);

	if ((img=(char *)malloc(size))==0)
	{
		perror("font malloc ");
		fclose(f);
		return -1;
	}
	fread(img,1,size,f);
	fclose(f);
	int rc = register_font_image(&hdr, img, desc);
	free(img);
	return rc;
}

//! register variable font from compiled font
int FGAPI FGFontManager::register_font_image(fonthdr * hdr, char *img, const char *desc)
{
	int w,ww,hh,ww32;
	char *img4;

	// alokuj vlastny font
	hdr->font_maxcharw = (hdr->font_maxcharw+3)&~3;
	variable_record *img2 = (variable_record *)calloc(sizeof(variable_record), 1);
	char *img3 = (char *) calloc(hdr->font_maxcharw*(hdr->font_maxcharh+1)*256, 1), *fimg=img3;
	// nastav pevnu max. velkost jedneho znaku
	img2->mw = hdr->font_maxcharw;
	hh = img2->mh = hdr->font_maxcharh+1;
	//
	img4 = img+hdr->ftable[0].fontchar_fp-sizeof(hdr->ftable);
	for(int i=0;i<256;i++)
	{
		// one char, oofset from the total begin to begin of the char
		img2->off[i] = img3-fimg;
		ww = img2->w[i] = hdr->ftable[i].fontchar_w;
		ww32 = (ww+3)&~3;
		// test for SPACE
		if (ww==0)
		{
			img2->w[i] = hdr->font_spacesize;
			img3 += hh * ((img2->w[i]+3)&~3);
			continue;
		}

		for(int h=0; hdr->ftable[i].fontchar_h > h; h++)
		{
			for(w=0; ww > w; w++)
			{
				if (*img4++)
					*img3 = -1;
				img3++;
			}
			while (ww32>w) { img3++; w++; }
		}
		img3 += ww32 * (hh-hdr->ftable[i].fontchar_h);
	}
	return register_font_var(img2, fimg, int(img3-fimg), desc);
}

/**
	registers truetype font (if TTF support is enabled)
	@return	-1 if error, or font ID (to use with FGDrawBuffer::set_font())
	@param filename full file name of ttf font (with path if needed)
	@param points font size to build
	@param count number of glyphs to extract from TTF file (128-32)
	@param offset offset of first glyph (32)
	@param desc optional face name (if null, then it is extracted from ttf file).
	It is used with FontDialog to choose.
*/
int FGAPI FGFontManager::register_font_ttf(const char *filename, int points, int count, int offset, const char *desc)
{
#ifdef FG_TTF
	char name[24];
	int size=0;
	char *img=0;
	FGFontProperty fp(filename, points);

	if (offset+count > 256)	// out of range - only 0 .. 256 codes are OK
		return -1;
	if (fp.Valid() == 0)	// not a valid face
		return -1;

	variable_record *var = (variable_record *)calloc(sizeof(variable_record), 1);

	img = fp.PrepareFont(var, size, count, offset, name);

	if (img==0)
	{
		free(var);
		return -1;
	}

	int code = register_font_var(var, img, size, desc?desc:name);

	if (code<0)
	{
		free(var);
		free(img);
		return -1;
	}
	return code;
#else
	return -1;
#endif	
}

//! Support fots up to 32 pixels in width
int FGAPI FGFontManager::_register_font(unsigned char *source, int width, int height, int count, int offset, variable_record * type, int index, const char *desc)
{
	int i;
	register int c;

	if (index==-1) // calculate first free slot
	{
//		for (index=FONT2034+1; index<=FONTLAST; index++)
// 21.6.2003 - remove on-demand loading fonts
		for (index=0; index<=FONTLAST; index++)
			if (FGFonts[index].fontimg==0) break;
		if (index>FONTLAST) return -1;
	}

	strncpy(FGFonts[index].name, desc, 23);
	FGFonts[index].name[23] = 0;
	FGFonts[index].height = height;
	FGFonts[index].width = width;
	FGFonts[index].var = 0;

	if (type == 0) // fixed width
	{
		FGPixel *dst;
		dst = (FGPixel *) calloc(width * height * (count+offset) * bpp, 1);
		assert(dst);
		FGFonts[index].fontimg = dst; // ????
		dst = dst + width*height*offset;
		width = width / 4;
		for (; count--;)
		for (i = 0; i < height; i++)
			switch (width)
			{
				case 8:
					c = *source++;
					*dst++ = c & 128 ? -1 : 0;
					*dst++ = c & 64 ? -1 : 0;
					*dst++ = c & 32 ? -1 : 0;
					*dst++ = c & 16 ? -1 : 0;
					*dst++ = c & 8 ? -1 : 0;
					*dst++ = c & 4 ? -1 : 0;
					*dst++ = c & 2 ? -1 : 0;
					*dst++ = c & 1 ? -1 : 0;
					c = *source++;
					*dst++ = c & 128 ? -1 : 0;
					*dst++ = c & 64 ? -1 : 0;
					*dst++ = c & 32 ? -1 : 0;
					*dst++ = c & 16 ? -1 : 0;
					*dst++ = c & 8 ? -1 : 0;
					*dst++ = c & 4 ? -1 : 0;
					*dst++ = c & 2 ? -1 : 0;
					*dst++ = c & 1 ? -1 : 0;
					c = *source++;
					*dst++ = c & 128 ? -1 : 0;
					*dst++ = c & 64 ? -1 : 0;
					*dst++ = c & 32 ? -1 : 0;
					*dst++ = c & 16 ? -1 : 0;
					*dst++ = c & 8 ? -1 : 0;
					*dst++ = c & 4 ? -1 : 0;
					*dst++ = c & 2 ? -1 : 0;
					*dst++ = c & 1 ? -1 : 0;
					c = *source++;
					*dst++ = c & 128 ? -1 : 0;
					*dst++ = c & 64 ? -1 : 0;
					*dst++ = c & 32 ? -1 : 0;
					*dst++ = c & 16 ? -1 : 0;
					*dst++ = c & 8 ? -1 : 0;
					*dst++ = c & 4 ? -1 : 0;
					*dst++ = c & 2 ? -1 : 0;
					*dst++ = c & 1 ? -1 : 0;
					break;
				case 7:
					c = *source++;
					*dst++ = c & 128 ? -1 : 0;
					*dst++ = c & 64 ? -1 : 0;
					*dst++ = c & 32 ? -1 : 0;
					*dst++ = c & 16 ? -1 : 0;
					*dst++ = c & 8 ? -1 : 0;
					*dst++ = c & 4 ? -1 : 0;
					*dst++ = c & 2 ? -1 : 0;
					*dst++ = c & 1 ? -1 : 0;
					c = *source++;
					*dst++ = c & 128 ? -1 : 0;
					*dst++ = c & 64 ? -1 : 0;
					*dst++ = c & 32 ? -1 : 0;
					*dst++ = c & 16 ? -1 : 0;
					*dst++ = c & 8 ? -1 : 0;
					*dst++ = c & 4 ? -1 : 0;
					*dst++ = c & 2 ? -1 : 0;
					*dst++ = c & 1 ? -1 : 0;
					c = *source++;
					*dst++ = c & 128 ? -1 : 0;
					*dst++ = c & 64 ? -1 : 0;
					*dst++ = c & 32 ? -1 : 0;
					*dst++ = c & 16 ? -1 : 0;
					*dst++ = c & 8 ? -1 : 0;
					*dst++ = c & 4 ? -1 : 0;
					*dst++ = c & 2 ? -1 : 0;
					*dst++ = c & 1 ? -1 : 0;
					c = *source++;
					*dst++ = c & 128 ? -1 : 0;
					*dst++ = c & 64 ? -1 : 0;
					*dst++ = c & 32 ? -1 : 0;
					*dst++ = c & 16 ? -1 : 0;
					break;
				case 6:
					c = *source++;
					*dst++ = c & 128 ? -1 : 0;
					*dst++ = c & 64 ? -1 : 0;
					*dst++ = c & 32 ? -1 : 0;
					*dst++ = c & 16 ? -1 : 0;
					*dst++ = c & 8 ? -1 : 0;
					*dst++ = c & 4 ? -1 : 0;
					*dst++ = c & 2 ? -1 : 0;
					*dst++ = c & 1 ? -1 : 0;
					c = *source++;
					*dst++ = c & 128 ? -1 : 0;
					*dst++ = c & 64 ? -1 : 0;
					*dst++ = c & 32 ? -1 : 0;
					*dst++ = c & 16 ? -1 : 0;
					*dst++ = c & 8 ? -1 : 0;
					*dst++ = c & 4 ? -1 : 0;
					*dst++ = c & 2 ? -1 : 0;
					*dst++ = c & 1 ? -1 : 0;
					c = *source++;
					*dst++ = c & 128 ? -1 : 0;
					*dst++ = c & 64 ? -1 : 0;
					*dst++ = c & 32 ? -1 : 0;
					*dst++ = c & 16 ? -1 : 0;
					*dst++ = c & 8 ? -1 : 0;
					*dst++ = c & 4 ? -1 : 0;
					*dst++ = c & 2 ? -1 : 0;
					*dst++ = c & 1 ? -1 : 0;
					break;
				case 5:
					c = *source++;
					*dst++ = c & 128 ? -1 : 0;
					*dst++ = c & 64 ? -1 : 0;
					*dst++ = c & 32 ? -1 : 0;
					*dst++ = c & 16 ? -1 : 0;
					*dst++ = c & 8 ? -1 : 0;
					*dst++ = c & 4 ? -1 : 0;
					*dst++ = c & 2 ? -1 : 0;
					*dst++ = c & 1 ? -1 : 0;
					c = *source++;
					*dst++ = c & 128 ? -1 : 0;
					*dst++ = c & 64 ? -1 : 0;
					*dst++ = c & 32 ? -1 : 0;
					*dst++ = c & 16 ? -1 : 0;
					*dst++ = c & 8 ? -1 : 0;
					*dst++ = c & 4 ? -1 : 0;
					*dst++ = c & 2 ? -1 : 0;
					*dst++ = c & 1 ? -1 : 0;
					c = *source++;
					*dst++ = c & 128 ? -1 : 0;
					*dst++ = c & 64 ? -1 : 0;
					*dst++ = c & 32 ? -1 : 0;
					*dst++ = c & 16 ? -1 : 0;
					break;
				case 4:
					c = *source++;
					*dst++ = c & 128 ? -1 : 0;
					*dst++ = c & 64 ? -1 : 0;
					*dst++ = c & 32 ? -1 : 0;
					*dst++ = c & 16 ? -1 : 0;
					*dst++ = c & 8 ? -1 : 0;
					*dst++ = c & 4 ? -1 : 0;
					*dst++ = c & 2 ? -1 : 0;
					*dst++ = c & 1 ? -1 : 0;
					c = *source++;
					*dst++ = c & 128 ? -1 : 0;
					*dst++ = c & 64 ? -1 : 0;
					*dst++ = c & 32 ? -1 : 0;
					*dst++ = c & 16 ? -1 : 0;
					*dst++ = c & 8 ? -1 : 0;
					*dst++ = c & 4 ? -1 : 0;
					*dst++ = c & 2 ? -1 : 0;
					*dst++ = c & 1 ? -1 : 0;
					break;
				case 3:
					c = *source++;
					*dst++ = c & 128 ? -1 : 0;
					*dst++ = c & 64 ? -1 : 0;
					*dst++ = c & 32 ? -1 : 0;
					*dst++ = c & 16 ? -1 : 0;
					*dst++ = c & 8 ? -1 : 0;
					*dst++ = c & 4 ? -1 : 0;
					*dst++ = c & 2 ? -1 : 0;
					*dst++ = c & 1 ? -1 : 0;
					c = *source++;
					*dst++ = c & 128 ? -1 : 0;
					*dst++ = c & 64 ? -1 : 0;
					*dst++ = c & 32 ? -1 : 0;
					*dst++ = c & 16 ? -1 : 0;
					break;
				case 2:
					c = *source++;
					*dst++ = c & 128 ? -1 : 0;
					*dst++ = c & 64 ? -1 : 0;
					*dst++ = c & 32 ? -1 : 0;
					*dst++ = c & 16 ? -1 : 0;
					*dst++ = c & 8 ? -1 : 0;
					*dst++ = c & 4 ? -1 : 0;
					*dst++ = c & 2 ? -1 : 0;
					*dst++ = c & 1 ? -1 : 0;
					break;
				case 1:
					c = *source++;
					*dst++ = c & 128 ? -1 : 0;
					*dst++ = c & 64 ? -1 : 0;
					*dst++ = c & 32 ? -1 : 0;
					*dst++ = c & 16 ? -1 : 0;
					break;
			}
	}
	else	// proportional width
	{
		FGFonts[index].var = type;
		FGFonts[index].fontimg = (FGPixel *)source;
	}

	++font_counter;
	return index;
}

void FGAPI FGFontManager::deregister_fonts(void)
{
	int i;

	for (i = 0; i < font_counter; i++)
	{
		if (FGFonts[i].var)
			free(FGFonts[i].var);
		free(FGFonts[i].fontimg);
		FGFonts[i].fontimg = 0;
	}
	font_counter=0;
}

/**
	@return the width of rendered string txt with the font ID.
	@param ID font ID for prepared font
	@param txt desired text string in ASCII format terminated with null byte.
*/
int FGAPI FGFontManager::textwidth(int ID, const char *txt)
{
	variable_record *fontvar = IsVariableFont(ID);
	if (!fontvar)
		return strlen(txt)*GetW(ID);
	int c=0;
	while(*txt)
		c += fontvar->w[*(unsigned char *)txt++];
	return c;
}

/**
	@return the width of first cnt characters from rendered string txt with the font ID.
	@note if the string contains a null byte, then this place stops the function early.
	@param ID font ID for prepared font
	@param txt desired text string in ASCII format.
	@param cnt desired number of character to compute the width
*/
int FGAPI FGFontManager::textwidth(int ID, const char *txt, int cnt)
{
	variable_record *fontvar = IsVariableFont(ID);
	if (!fontvar)
		return cnt*GetW(ID);
	int c=0;
	while(cnt-- && *txt)
		c += fontvar->w[*(unsigned char *)txt++];
	return c;
}

/**
	@return the width of the character c for the font ID.
	@param ID font ID for prepared font
	@param c desired character
*/
int FGAPI FGFontManager::charwidth(int ID, int c)
{
	variable_record *fontvar = IsVariableFont(ID);
	if (!fontvar)
		return GetW(ID);
	return fontvar->w[c];
}

// -----------------------------------------------------------------------------

FGFontProperty::FGFontProperty(const char *n, int sz)
{
#ifdef FG_TTF
	_font = TTF_OpenFont(n, sz);
	if (Valid() == false)
		__fg_error("TrueType font open error!",0);
	else
		Solid();
#else
	__fg_error("Using TTF code, but not compiled-in support for one (did you see config.mak?)", 0);
#endif
}

FGFontProperty::~FGFontProperty()
{
#ifdef FG_TTF
	if (_font) TTF_CloseFont((TTF_Font *)_font);
#endif
}

void FGFontProperty::Bold(int value)
{
#ifdef FG_TTF
	int tmp = TTF_GetFontStyle((TTF_Font *)_font);
	if (value)
		TTF_SetFontStyle((TTF_Font *)_font, tmp | TTF_STYLE_BOLD);
	else
		TTF_SetFontStyle((TTF_Font *)_font, tmp & ~TTF_STYLE_BOLD);
#endif
}

void FGFontProperty::Italic(int value)
{
#ifdef FG_TTF
	int tmp = TTF_GetFontStyle((TTF_Font *)_font);
	if (value)
		TTF_SetFontStyle((TTF_Font *)_font, tmp | TTF_STYLE_ITALIC);
	else
		TTF_SetFontStyle((TTF_Font *)_font, tmp & ~TTF_STYLE_ITALIC);
#endif
}

void FGFontProperty::Underline(int value)
{
#ifdef FG_TTF
	int tmp = TTF_GetFontStyle((TTF_Font *)_font);
	if (value)
		TTF_SetFontStyle((TTF_Font *)_font, tmp | TTF_STYLE_UNDERLINE);
	else
		TTF_SetFontStyle((TTF_Font *)_font, tmp & ~TTF_STYLE_UNDERLINE);
#endif
}

char * FGFontProperty::PrepareFont(variable_record *var, int& size, int count, int offset, char *name)
{
#ifdef FG_TTF
	char *pixdata[256], *ptr, *retptr;
	int sizes[256];
	char str[64];
	TTF_Font * f;
	int mw=1, mh=1, sz=0, w, h;

	size = 0;
	memset(pixdata, 0, sizeof(pixdata));
	memset(sizes, 0, sizeof(sizes));

	f = (TTF_Font *)GetFace();
	sprintf(str,"%s-%s-%d", TTF_FontFaceFamilyName(f) , TTF_FontFaceStyleName(f), TTF_FontHeight(f) );

	for (int i=offset; i<(offset+count); i++)
	{
		w = h = 0;
		ptr = TTF_RenderChar(f, i, w, h, sz);

		if (ptr && sz)
		{
			if (mw<w) mw=w;
			if (mh<h) mh=h;
			pixdata[i] = (char *)calloc(sz,1);
			sizes[i] = sz;
			memcpy(pixdata[i], ptr, sz);
		}
		else
		{
			pixdata[i] = 0;
			sizes[i] = 0;
		}
		var->off[i] = size;
		size += sz;
		var->w[i] = w;
	}
	// alloc for entire bitmap
	retptr = ptr = (char *)malloc(size);

	for (int i=offset; i<(offset+count); i++)
	{
		if (pixdata[i])
		{
			memcpy(ptr, pixdata[i], sizes[i]);
			ptr += sizes[i];
			free(pixdata[i]);
		}
	}
	var->mw = mw;
	var->mh = mh;
	strncpy(name,str,23);
	name[23]=0;
	return retptr;
#else
     return 0;
#endif
}


int FGFontProperty::textwidth(const char* txt)
{
#ifdef FG_TTF
	int w = 0, h = 0;
	int result = TTF_SizeUTF8((TTF_Font *)_font, txt, &w, &h, 0);
	if (result == 0)
		return w;
#endif
	return 0;
}


bool FGFontProperty::textsize(const char* txt, int* ww, int* hh)
{
#ifdef FG_TTF
	int w = 0, h = 0;
	int result = TTF_SizeUTF8((TTF_Font *)_font, txt, &w, &h, 0);
	if (result == 0)
	{
		*ww = w;
		*hh = h;
		return true;
	}
#endif
	return false;
}


#ifdef FG_NAMESPACE
}
#endif



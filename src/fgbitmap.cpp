/*
   Copyright (C) 1996,2004  Marian Krivos

   nezmar@atlas.sk

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   bitmap.cc - bitmap engine

 */

#include <stdarg.h>
#include <errno.h>

#include "fgbase.h"
#include "fgbitmap.h"
#include "_fastgl.h"

#ifdef FG_JPEG
extern "C" {		// jpeglib.h forgot this... (Oron Peled)
#include <jpeglib.h>
#include <jerror.h>
#ifdef __BORLANDC__
#ifdef __linux__
#pragma link "libjpeg.so"
#else
#pragma link "libjpeg.lib"
#endif
#endif
}
#endif

#ifdef FG_TIFF
#include <tiffio.h>
#ifdef __BORLANDC__
#ifdef __linux__
#pragma link "libtiff.so"
#else
#pragma link "libtiff.lib"
#endif
#endif
#endif

#ifdef FG_PNG
#include <png.h>
#ifdef __BORLANDC__
#ifdef __linux__
#pragma link "libpng.so"
#pragma link "libz.so"
#else
#pragma link "libpng.lib"
#pragma link "zlib.lib"
#endif
#endif
#else
#include <setjmp.h>
#endif

#include "gif.h"

#define 	TRANSP_KEY			0xFE00FE
#define     MAX_LWZ_BITS        12
#define		INTERLACE			0x40
#define		LOCALCOLORMAP		0x80

#define BitSet(byte, bit)	(((byte) & (bit)) == (bit))
#define	ReadOK(file,buffer,len) (fread(buffer, len, 1, file) != 0)
#define MKINT(a,b)		(((b)<<8)|(a))
#define NEW(x)			((x *)malloc(sizeof(x)))

/***************************************************************************
*
*  ERROR()    --  should not return
*  INFO_MSG() --  info message, can be ignored
*
***************************************************************************/

#define INFO_MSG(fmt)
#define _ERROR(str) 	__fg_error(str, 0), longjmp(setjmp_buffer, 1)

/***************************************************************************/

#ifdef FG_NAMESPACE
namespace fgl {
#endif

#ifdef FG_JPEG
struct ima_error_mgr {
	struct jpeg_error_mgr pub;	/* "public" fields */
	jmp_buf setjmp_buffer;		/* for return to caller */
	char* buffer;				/* error message <CSC>*/
};

typedef ima_error_mgr *ima_error_ptr;

static void fgl_jpeg_error_exit(j_common_ptr cinfo)
{
	/* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
	ima_error_ptr myerr = (ima_error_ptr) cinfo->err;
	/* Create the message */
	myerr->pub.format_message (cinfo, myerr->buffer);
	/* Send it to stderr, adding a newline */
	/* Return control to the setjmp point */
	longjmp(myerr->setjmp_buffer, 1);
}

#endif

static int readColorMap(FILE *, int, unsigned char[GIF_MAXCOLORS][3]);
static int GetDataBlock(FILE *, unsigned char *);
static void readImage(FILE *, int, int, int, unsigned char *);
static void read_RLE8_compressed_image(FILE * f, unsigned char *bmp, int w, int h);

static jmp_buf setjmp_buffer;

/* TRUEVISION-XFILE magic signature string */
static unsigned char magic[18] =
{
  0x54, 0x52, 0x55, 0x45, 0x56, 0x49, 0x53, 0x49, 0x4f,
  0x4e, 0x2d, 0x58, 0x46, 0x49, 0x4c, 0x45, 0x2e, 0x0
};

/***************************************************************************/

#ifdef FG_JPEG
// thanks to Chris Shearer Cooper <cscooper(at)frii(dot)com>
class CxFileJpg : public jpeg_destination_mgr, public jpeg_source_mgr
	{
public:
	enum { eBufSize = 4096 };

	CxFileJpg(FILE* pFile)
	{
		m_pFile = pFile;

		init_destination = InitDestination;
		empty_output_buffer = EmptyOutputBuffer;
		term_destination = TermDestination;

		init_source = InitSource;
		fill_input_buffer = FillInputBuffer;
		skip_input_data = SkipInputData;
		resync_to_restart = jpeg_resync_to_restart; // use default method
		term_source = TermSource;
		next_input_byte = NULL; // => next byte to read from buffer
		bytes_in_buffer = 0;	// # of bytes remaining in buffer

		m_pBuffer = new unsigned char[eBufSize];
	}
	~CxFileJpg()
	{
		delete [] m_pBuffer;
	}

	static void InitDestination(j_compress_ptr cinfo)
	{
		CxFileJpg* pDest = (CxFileJpg*)cinfo->dest;
		pDest->next_output_byte = pDest->m_pBuffer;
		pDest->free_in_buffer = eBufSize;
	}

	static boolean EmptyOutputBuffer(j_compress_ptr cinfo)
	{
		CxFileJpg* pDest = (CxFileJpg*)cinfo->dest;
		if (fwrite(pDest->m_pBuffer,1,eBufSize, pDest->m_pFile)!=(size_t)eBufSize)
			ERREXIT(cinfo, JERR_FILE_WRITE);
		pDest->next_output_byte = pDest->m_pBuffer;
		pDest->free_in_buffer = eBufSize;
		return true;
	}

	static void TermDestination(j_compress_ptr cinfo)
	{
		CxFileJpg* pDest = (CxFileJpg*)cinfo->dest;
		size_t datacount = eBufSize - pDest->free_in_buffer;
		/* Write any data remaining in the buffer */
		if (datacount > 0) {
			if (!fwrite(pDest->m_pBuffer,1,datacount,pDest->m_pFile))
				ERREXIT(cinfo, JERR_FILE_WRITE);
		}
		fflush(pDest->m_pFile);
		/* Make sure we wrote the output file OK */
		if (errno) ERREXIT(cinfo, JERR_FILE_WRITE);
		return;
	}

	static void InitSource(j_decompress_ptr cinfo)
	{
		CxFileJpg* pSource = (CxFileJpg*)cinfo->src;
		pSource->m_bStartOfFile = true;
	}

	static boolean FillInputBuffer(j_decompress_ptr cinfo)
	{
		size_t nbytes;
		CxFileJpg* pSource = (CxFileJpg*)cinfo->src;
		nbytes = fread(pSource->m_pBuffer,1,eBufSize,pSource->m_pFile);
		if (nbytes <= 0){
			if (pSource->m_bStartOfFile)	// Treat empty input file as fatal error
				ERREXIT(cinfo, JERR_INPUT_EMPTY);
			WARNMS(cinfo, JWRN_JPEG_EOF);
			// Insert a fake EOI marker
			pSource->m_pBuffer[0] = (JOCTET) 0xFF;
			pSource->m_pBuffer[1] = (JOCTET) JPEG_EOI;
			nbytes = 2;
		}
		pSource->next_input_byte = pSource->m_pBuffer;
		pSource->bytes_in_buffer = nbytes;
		pSource->m_bStartOfFile = false;
		return true;
	}

	static void SkipInputData(j_decompress_ptr cinfo, long num_bytes)
	{
		CxFileJpg* pSource = (CxFileJpg*)cinfo->src;
		if (num_bytes > 0){
			while (num_bytes > (long)pSource->bytes_in_buffer){
				num_bytes -= (long)pSource->bytes_in_buffer;
				FillInputBuffer(cinfo);
				// note we assume that fill_input_buffer will never return false,
				// so suspension need not be handled.
			}
			pSource->next_input_byte += (size_t) num_bytes;
			pSource->bytes_in_buffer -= (size_t) num_bytes;
		}
	}

	static void TermSource(j_decompress_ptr cinfo)
	{
		return;
	}
protected:
	FILE  *m_pFile;
	unsigned char *m_pBuffer;
	bool m_bStartOfFile;
public:
	enum CODEC_OPTION
	{
		ENCODE_BASELINE = 0x1,
		ENCODE_ARITHMETIC = 0x2,
		ENCODE_GRAYSCALE = 0x4,
		ENCODE_OPTIMIZE = 0x8,
		ENCODE_PROGRESSIVE = 0x10,
		ENCODE_LOSSLESS = 0x20,
		ENCODE_SMOOTHING = 0x40,
		DECODE_GRAYSCALE = 0x80,
		DECODE_QUANTIZE = 0x100,
		DECODE_DITHER = 0x200,
		DECODE_ONEPASS = 0x400,
		DECODE_NOSMOOTH = 0x800
	};

	int m_nPredictor;
	int m_nPointTransform;
	int m_nSmoothing;
	int m_nQuantize;
	J_DITHER_MODE m_nDither;

};
#endif // jpeg

/***************************************************************************/

FGBitmap::ReturnValues FGBitmap::LoadImage(const char* nm, int imagefmt)
{
	int  one;
	ReturnValues retval = IMAGE_OK;
	unsigned int c, bits = 0;
	register unsigned char *src;
#ifndef INDEX_COLORS
	register FGPixel *bp;
	int sz, d, i;
	FGPixel *pom;
#endif

	type = BMP_NONE;
	init();

    SetName(nm);

	if (imagefmt == IMAGE_AUTO)
		format = detect(name);
	else
	    format = imagefmt;

	switch(format)
	{
		case IMAGE_GIF:
            retval = gif(name,one,bits,c);
			if (retval) return retval;
			break;
		case IMAGE_PCX:
			retval = pcx(name,one,bits,c);
			if (retval) return retval;
			break;
#ifdef FG_PNG
		case IMAGE_PNG:
			retval = png(name,one,bits,c);
			if (retval) return retval;
			break;
#endif
#ifdef FG_JPEG
		case IMAGE_JPG:
			retval = jpg(name,one,bits,c);
			if (retval) return retval;
			break;
#endif
		case IMAGE_TGA:
			retval = tga(name,one,bits,c);
			if (retval) return retval;
			break;
#ifdef FG_TIFF
		case IMAGE_TIF:
			retval = tiff(name,one,bits,c);
			if (retval) return retval;
			break;
#endif
		case IMAGE_BMP:
			retval = bmp(name,one,bits,c);
			if (retval) return retval;
			break;
		default:
			__fg_error("Unknown image format!", 0);
			return IMAGE_UNKNOWN_FORMAT;
	}

#ifndef INDEX_COLORS
	if (bits<=8)
	{
		bp = (FGPixel *) calloc(areasize(w, h), 1);
		pom = bp;
		src = (unsigned char *) GetArray();
		sz = w * h;

		if (state.colorkey == 0xffffffffU)
			for (i = 0; i < sz; i++)
		{
			d = *((unsigned *) ((char *) &rgb + *src * one));
			*pom = FGDirectColor(d);
			pom++;
			src++;
		}
		else
		{
			for (i = 0; i < sz; i++)
			{
				if (*src == state.colorkey)
					d = TRANSP_KEY;
				else
					d = *((unsigned *) ((char *) &rgb + *src * one));
				*pom++ = FGDirectColor(d);
				src++;
			}
			state.colorkey = FGDirectColor(TRANSP_KEY);
		}
		free(image);
		image = bp;
	}
#endif
	type = BMP_FILE;
	wwrk = w;
	hwrk = h;
#ifdef INDEX_COLORS
	DoPalette(one);
#endif
	return retval;
}

FGBitmap::ReturnValues FGBitmap::SaveImage(const char* name, int imagefmt)
{
	switch(imagefmt)
	{
		case IMAGE_BMP:
		    return BitmapSave(name, false);
		case IMAGE_PNG:
		    return PngSave(name);
		case IMAGE_JPG:
		    return JpegSave(name);
	}
	return IMAGE_UNKNOWN_FORMAT;
}

int FGBitmap::detect(const char *nm)
{
	if ( strstr(nm, ".gif") || strstr(nm, ".GIF") )
	{							// decode *.gif file
		return IMAGE_GIF;
	}
	else if ( strstr(nm, ".pcx") || strstr(nm, ".PCX") )
	{							// decode *.gif file
		return IMAGE_PCX;
	}
	else if ((strstr(nm, ".tga")) || (strstr(nm, ".TGA")))
	{							// decode *.gif file
		return IMAGE_TGA;
	}
	else if ((strstr(nm, ".png")) || (strstr(nm, ".PNG")))
	{							// decode *.gif file
		return IMAGE_PNG;
	}
	else if ((strstr(nm, ".jpg")) || (strstr(nm, ".JPG")) || (strstr(nm, ".jpeg")) || (strstr(nm, ".JPEG")))
	{
		return IMAGE_JPG;
	}
	else if ((strstr(nm, ".tiff")) || (strstr(nm, ".TIFF")) || (strstr(nm, ".tif")) || (strstr(nm, ".TIF")))
	{
		return IMAGE_TIF;
	}
	else if ((strstr(nm, ".bmp")) || (strstr(nm, ".BMP")))
	{							// decode *.gif file
		return IMAGE_BMP;
	}
	return IMAGE_AUTO;
}

FGBitmap::FGBitmap()
 : FGDrawBuffer()
{
	type = BMP_NONE;
	init();
}

/**
	Creates a BITMAP object from one of supported image formats.
	Parameter 'n' is pointer to filename. You can test to fail
	after return. The member 'type' must not be BMP_NONE, else
	object is no valid. Supported file formats are BMP, TGA, GIF
	and PCX. You can enable JPG and PNG by editing 'config.mak'
	optionaly too. These two are supported by third party libs
	(libjpeg & libpng).
*/
FGBitmap::FGBitmap(const char *n)
: FGDrawBuffer()
{
    LoadImage(n, IMAGE_AUTO);
}

FGBitmap::ReturnValues FGBitmap::bmp(char *n, int& one, unsigned int& bits, unsigned int& c)
{
	int ys;
	int	compressed = 0;
	int rest;
	FILE *f=0;
	unsigned pixel;
	register unsigned char *src;
#ifndef INDEX_COLORS
	register FGPixel *bp;
#endif

	f = fopen(n, "rb");
	if (!f)
	{
		char nm[128];
		sprintf(nm, "FGBitmap '%s' not found!", n);
		__fg_error(nm, 0);
		return IMAGE_FILE_NOT_FOUND;
	}
	name = new char[strlen(n) + 1];
	strcpy(name, n);
	if (sizeof(bmfh) != 10)
	{
		::printf("Ooops, your data structure is not good aligned!!!\a\n");
		assert(0);
		abort();			// 100% sure to stop
	}
	fread(&bmfh, sizeof(bmfh), 1, f);	// read first 10 bytes
	// return if no BMP
	if (bmfh.usType != (unsigned short) ('M' * 256 + 'B'))
	{
		char jj[128];
		sprintf(jj,"%s isn't supported image file!", n);
		fclose(f);
		__fg_error(jj, 0);
		return IMAGE_UNKNOWN_FORMAT;
	}

	fread(&bmi, sizeof(bmi), 1, f);		// read next 4 bytes
	memset(&bmih, 0, sizeof(bmih));
	fread(&bmih, 4, 1, f);	// read size of bitmap info header
	fread(&bmih.bmih1.cx, bmih.bmih1.size - 4, 1, f);	// if 0x0c then OS/2 bitmap
	if (bmih.bmih1.size == 0x0C)
		one = 3;
	else
		one = 4;

	c = (bmi.imageOffset - (bmih.bmih1.size + 14)) / one;	// compute numbers of used colors

	if (c == 0)
	{
#ifdef INDEX_COLORS
		fclose(f);
		__fg_error("True colors BMP not supported!", 0);
		return IMAGE_UNKNOWN_FORMAT;
#else
		w = bmih.bmih2.cx;
		h = bmih.bmih2.cy;
		bits = bmih.bmih2.cBitCount;
		if (bmih.bmih2.cPlanes != 1)
		{
			fclose(f);
			__fg_error("This color depth BMP is not supported!", 0);
			return IMAGE_UNKNOWN_FORMAT;
		}
#endif
	}
	else if (one == 3)
	{
		if ((compressed = bmih.bmih1.ulCompression) > 1)
		{
			__fg_error("No this compressed BMP supported!", 0);
			return IMAGE_UNKNOWN_FORMAT;
		}
		fread(&rgb, sizeof(RGB3), c, f);
		w = bmih.bmih1.cx;
		h = bmih.bmih1.cy;
		bits = bmih.bmih1.cBitCount;
		if (bmih.bmih1.cPlanes != 1)
		{
			fclose(f);
			__fg_error("Multi-planes BMP not supported!", 0);
			return IMAGE_UNKNOWN_FORMAT;
		}
	}
	else if (one == 4)
	{
		if ((compressed = bmih.bmih2.ulCompression) > 1)
		{
			__fg_error("Unknown compress format!", 0);
			return IMAGE_UNKNOWN_FORMAT;
		}
		fread(&rgb, sizeof(RGB4), c, f);
		w = bmih.bmih2.cx;
		h = bmih.bmih2.cy;
		bits = bmih.bmih2.cBitCount;
		if (bmih.bmih2.cPlanes != 1)
		{
			fclose(f);
			__fg_error("Multi-planes BMP not supported!", 0);
			return IMAGE_UNKNOWN_FORMAT;
		}
	}

	assert(SetArray((FGPixel *) calloc(areasize(w, h), 1)));

	//
	// common image decoder
	//
	if (f) switch (bits)
	{
		case 1:
			for (ys = h - 1; ys >= 0; ys--)
			{
				src = (unsigned char *) GetArray() + ys * w;
				for (int xs = w / 8; xs > 0; xs--)
				{
					fread((unsigned char *) &pixel, 1, 1, f);
					*src++ = (pixel >> 7) & 1;
					*src++ = (pixel >> 6) & 1;
					*src++ = (pixel >> 5) & 1;
					*src++ = (pixel >> 4) & 1;
					*src++ = (pixel >> 3) & 1;
					*src++ = (pixel >> 2) & 1;
					*src++ = (pixel >> 1) & 1;
					*src++ = (pixel) & 1;
				}
				rest = w % 8;
				if (rest)
				{
					fread((unsigned char *) &pixel, 1, 1, f);
					*src++ = (pixel >> 7) & 1;
					if (--rest == 0)
						goto end_expand;
					*src++ = (pixel >> 6) & 1;
					if (--rest == 0)
						goto end_expand;
					*src++ = (pixel >> 5) & 1;
					if (--rest == 0)
						goto end_expand;
					*src++ = (pixel >> 4) & 1;
					if (--rest == 0)
						goto end_expand;
					*src++ = (pixel >> 3) & 1;
					if (--rest == 0)
						goto end_expand;
					*src++ = (pixel >> 2) & 1;
					if (--rest == 0)
						goto end_expand;
					*src++ = (pixel >> 1) & 1;
				}
			  end_expand:
				rest = (w * bits / 8) % 4;
				if (w % 8)
					rest++;
				while (rest & 3)
				{
					rest++;
					fgetc(f);
				}
			}
			break;
		case 4:
			for (ys = h - 1; ys >= 0; ys--)
			{
				src = (unsigned char *) GetArray() + ys * w;
				for (int xs = w / 2; xs > 0; xs--)
				{
					fread((unsigned char *) &pixel, 1, 1, f);
					*src++ = (pixel >> 4) & 15;
					*src++ = pixel & 15;
				}
				rest = w % 2;
				if (rest)
				{
					fread((unsigned char *) &pixel, 1, 1, f);
					*(src++) = (pixel >> 4) & 15;
				}
				rest = (w * bits / 8) % 4;
				if (w % 2)
					rest++;
				while (rest & 3)
				{
					rest++;
					fgetc(f);
				}
			}
			break;
		case 8:
			if (!compressed)
				for (ys = h - 1; ys >= 0; ys--)
				{
					fread((unsigned char *) GetArray() + ys * w, w, 1, f);
					rest = (w * bits / 8) % 4;
					while (rest & 3)
					{
						rest++;
						fgetc(f);
					}
				}
			else read_RLE8_compressed_image(f, (unsigned char *) GetArray(), w, h);
			break;
#ifndef INDEX_COLORS
		case 24:
			for (ys = h - 1; ys >= 0; ys--)
			{
				bp = (image + ys * w);
				for (int xs = 0; xs < w; xs++)
				{
					fread(&pixel, 3, 1, f);
					*bp = FGDirectColor(pixel);
					bp++;
				}
			}
			rest = (w * bits / 8) % 4;
			while (rest & 3)
			{
				rest++;
				fgetc(f);
			}
			break;
#endif
#ifdef DIRECT_COLORS
		case 16:
		case 15:
			for (ys = h - 1; ys >= 0; ys--)
			{
				bp = (image + ys * w);
				for (int xs = 0; xs < w; xs++)
				{
					fread(bp, 2, 1, f);
					bp++;
				}
			}
			rest = (w * bits / 8) % 4;
			while (rest & 3)
			{
				rest++;
				fgetc(f);
			}
			break;
#endif
		default:
			__fg_error("Bad FGBitmap format!", 0);
			fclose(f);
			free(GetArray());
			SetArray(0);
			return IMAGE_UNKNOWN_FORMAT;
	}
	if (f) fclose(f);

	return IMAGE_OK;
}

#ifdef INDEX_COLORS
void FGBitmap::DoPalette(int one)
{
	cUsedTable = new int[256];
	int sz = w * h, d, i;
	FGPixel *bm = GetArray();

	memset(cUsedTable, 0, sizeof(int) * 256);	// reset color table
	// calculate which colors is used in image
	for (i = 0; i < sz; i++)
	{
		cUsedTable[*bm++]++;
	}

	bm = GetArray();
	// allocating colors
	for (i = 0; i < 256; i++)
	{
		if (cUsedTable[i])
		{
			d = *((unsigned *) ((char *) &rgb + i * one));
			if (i!=state.colorkey)
				cUsedTable[i] = (char) CreateColor((d >> 16)&0xff, (d >> 8)&0xff, d&0xff, -1); // CORRECTED
			else cUsedTable[i] = i;
		}
		else cUsedTable[i] = -1;
	}

	// remapping colors
	for (i = 0; i < sz; i++)
	{
		bm[i] = (char) cUsedTable[bm[i]];
	}
}
#endif

/* read_RLE8_compressed_image:
 *  For reading the 8 bit RLE compressed BMP image format.
 */
static void read_RLE8_compressed_image(FILE * f, unsigned char *bmp, int w, int h)
{
	unsigned char count, val, val0;
	int j, pos, line;
	int eolflag, eopicflag;

	eopicflag = 0;
	line = h - 1;

	while (eopicflag == 0)
	{
		pos = 0;				/* x position in bitmap */
		eolflag = 0;			/* end of line flag */

		while ((eolflag == 0) && (eopicflag == 0))
		{
			count = (char) getc(f);
			val = (char) getc(f);

			if (count > 0)
			{					/* repeat pixel count times */
				for (j = 0; j < count; j++)
				{
					bmp[line * w + pos] = val;
					pos++;
				}
			}
			else
			{
				switch (val)
				{
					case 0:		/* end of line flag */
						eolflag = 1;
						break;
					case 1:		/* end of picture flag */
						eopicflag = 1;
						break;
					case 2:		/* displace picture */
						count = (char) getc(f);
						val = (char) getc(f);
						pos += count;
						line += val;
						break;
					default:	/* read in absolute mode */
						for (j = 0; j < val; j++)
						{
							val0 = (char) getc(f);
							bmp[line * w + pos] = val0;
							pos++;
						}

						if (j % 2 == 1)
							/*val0 = (char) */getc(f);	/* align on word boundary */
						break;
				}
			}
			if (pos > w)
				eolflag = 1;
		}
		line--;
		if (line < 0)
			eopicflag = 1;
	}
}

static void copy_gif_palette(unsigned char *to, unsigned char *data, int cnt)
{
	int r,g,b;
	for(;cnt>0;cnt--)
	{
		r = *data++;
		g = *data++;
		b = *data++;
		*to++ = b;
		*to++ = g;
		*to++ = r;
	}
}

#ifdef FG_JPEG
struct r_jpeg_error_mgr
{
	struct jpeg_error_mgr pub;
    jmp_buf envbuffer;
};

int fh_jpeg_id(char *name)
{
    FILE * fd;
	unsigned char id[10];
	fd=fopen(name,"rb");
    if(fd==0)
        return 0;
    fread(id,1,10,fd);
	fclose(fd);

    if(id[6]=='J' && id[7]=='F' && id[8]=='I' && id[9]=='F') return(1);
    if(id[0]==0xff && id[1]==0xd8 && id[2]==0xff) return(1);

	return 0;
}


void jpeg_cb_error_exit(j_common_ptr cinfo)
{
    struct r_jpeg_error_mgr *mptr;
    mptr=(struct r_jpeg_error_mgr*) cinfo->err;
    (*cinfo->err->output_message) (cinfo);
    longjmp(mptr->envbuffer,1);
}

#endif

FGBitmap::ReturnValues FGBitmap::jpg(char *n, int& one, unsigned int& bits, unsigned int& c)
{
#ifdef FG_JPEG
	struct jpeg_decompress_struct cinfo;
	struct jpeg_decompress_struct *ciptr;
	struct r_jpeg_error_mgr emgr;
	FGPixel *bp;
	FILE *fh;
	JSAMPLE *lb;

	ciptr=&cinfo;
	if(fh_jpeg_id(n)==0) return IMAGE_FILE_NOT_FOUND;
    if(!(fh=fopen(n,"rb"))) return IMAGE_FILE_NOT_FOUND;
	ciptr->err=jpeg_std_error(&emgr.pub);
    emgr.pub.error_exit=jpeg_cb_error_exit;
    if(setjmp(emgr.envbuffer)==1)
    {
		// FATAL ERROR - Free the object and return...
		jpeg_destroy_decompress(ciptr);
		fclose(fh);
		return IMAGE_UNKNOWN_FORMAT;
    }

    jpeg_create_decompress(ciptr);
    jpeg_stdio_src(ciptr,fh);
    jpeg_read_header(ciptr,true);
    ciptr->out_color_space=JCS_RGB;
    jpeg_start_decompress(ciptr);

    w = ciptr->output_width;
	h = ciptr->output_height;
	int cc = ciptr->output_components;
	c = 0;
	bits = 24;
	one = 0;

	assert(SetArray((FGPixel *) calloc(areasize(w, h), 1)));
	bp = GetArray();

    if(cc==3)
    {
		unsigned char pixel[4];
		lb = (JSAMPLE *)(*ciptr->mem->alloc_small)(
			(j_common_ptr) ciptr,
			JPOOL_PERMANENT,
			cc*w);
		while (ciptr->output_scanline < ciptr->output_height)
		{
		    jpeg_read_scanlines(ciptr, &lb, 1);
			for (int xs = 0; xs < w; xs++)
			{
				pixel[2] = *(lb+3*xs);
				pixel[1] = *(lb+3*xs+1);
				pixel[0] = *(lb+3*xs+2);
				*bp = FGDirectColor(*(unsigned int *)(&pixel));
				bp++;
			}
		}
    }
    jpeg_finish_decompress(ciptr);
    jpeg_destroy_decompress(ciptr);
	fclose(fh);
	return IMAGE_OK;
#else
	return IMAGE_UNSUPPORTED_FORMAT;
#endif
}

FGBitmap::ReturnValues FGBitmap::tiff(char *n, int& one, unsigned int& bits, unsigned int& c)
{
#if defined(FG_TIFF) && !defined(INDEX_COLORS)
	TIFF* tif = TIFFOpen(n, "r");
	if (tif)
	{
		TIFFRGBAImage img;
		char emsg[1024];

		if (TIFFRGBAImageBegin(&img, tif, 0, emsg)) {
			size_t npixels;
			uint32* raster;

			npixels = img.width * img.height;
			raster = (uint32*) _TIFFmalloc(npixels * sizeof (uint32));
			if (raster != NULL)
			{
				if (TIFFRGBAImageGet(&img, raster, img.width, img.height))
				{
					w = img.width;
					h = img.height;
					c = 0;
					bits = 24;
					one = 0;

					assert(SetArray((FGPixel *) calloc(areasize(w, h), 1)));
					unsigned char pixel[4]={0,0,0,0}, *lb = (unsigned char *)raster;
					FGPixel *bp = GetArray();

					for (int ys = img.height-1; ys>=0; ys--)
					{
						lb = (unsigned char *)(raster+img.width*ys);
						for (int xs = 0; xs < w; xs++)
						{
							pixel[2] = *(lb+4*xs);
							pixel[1] = *(lb+4*xs+1);
							pixel[0] = *(lb+4*xs+2);
							*bp = FGDirectColor(*(unsigned int *)(&pixel));
							bp++;
						}
					}
				}
				_TIFFfree(raster);
			}
			TIFFRGBAImageEnd(&img);
		}
		else
		{
			TIFFError(n, emsg);
			TIFFClose(tif);
			return IMAGE_FILE_NOT_FOUND;
		}
		TIFFClose(tif);
	}
	return IMAGE_OK;
#else
	return IMAGE_UNSUPPORTED_FORMAT;
#endif
}

#ifdef FG_PNG

#define PNG_BYTES_TO_CHECK 4
#define min(x,y) ((x) < (y) ? (x) : (y))

int fh_png_id(char *name)
{
	FILE* fd;
	char id[4];
	fd=fopen(name,"rb");
	if(fd==0) return(0);
	fread(id,1,4,fd);
	fclose(fd);
	if(id[1]=='P' && id[2]=='N' && id[3]=='G') return(1);
	return(0);
}
#endif // PNG

FGBitmap::ReturnValues FGBitmap::png(char *n, int& one, unsigned int& bits, unsigned int& c)
{
#ifdef FG_PNG
	png_structp png_ptr;
	png_infop info_ptr;
	png_uint_32 width, height;
	int i;
	int bit_depth, color_type, interlace_type;
	int number_passes,pass;
	char *rp;
	png_bytep rptr[2];
	FGPixel *fbptr;
	FILE *fh;

	if(fh_png_id(n)==0) return IMAGE_FILE_NOT_FOUND;
	if(!(fh=fopen(n,"rb"))) return IMAGE_FILE_NOT_FOUND;

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,NULL,NULL,NULL);
	if (png_ptr == NULL) return IMAGE_UNKNOWN_FORMAT;
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL)
	{
		png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
		fclose(fh);
        return IMAGE_UNKNOWN_FORMAT;
	}
	rp=0;
	if (setjmp(png_ptr->jmpbuf))
	{
		png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
		if(rp) free(rp);
		fclose(fh);
        return IMAGE_UNKNOWN_FORMAT;
	}

	png_init_io(png_ptr,fh);

	png_read_info(png_ptr, info_ptr);
	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,&interlace_type, NULL, NULL);

	if (color_type == PNG_COLOR_TYPE_PALETTE)
		png_set_expand(png_ptr);
	if (bit_depth < 8)
		png_set_packing(png_ptr);
	if (color_type == PNG_COLOR_TYPE_GRAY || color_type== PNG_COLOR_TYPE_GRAY_ALPHA)
		png_set_gray_to_rgb(png_ptr);
    if( bit_depth==16)
		png_set_strip_16(png_ptr);
	png_set_filter(png_ptr, 0, PNG_NO_FILTERS);
/*
	If, for some reason, you don't need the alpha channel on an image,
	and you want to remove it rather than combining it with the background
	(but the image author certainly had in mind that you *would* combine
	it with the background, so that's what you should probably do):
*/
    if (color_type & PNG_COLOR_MASK_ALPHA)
		png_set_strip_alpha(png_ptr);

	if (info_ptr->valid & PNG_INFO_bKGD)
	{
		FGColor key(info_ptr->background.red, info_ptr->background.green, info_ptr->background.blue);
		state.colorkey = key;
		png_set_background (png_ptr, &info_ptr->background, PNG_BACKGROUND_GAMMA_FILE, 1, 1.0);
	}

	number_passes = png_set_interlace_handling(png_ptr);
	png_read_update_info(png_ptr,info_ptr);

    rp=(char*) malloc(width*3);
    rptr[0]=(png_bytep) rp;
	w = width;
	h = height;
	c = 0;
	bits = 24;
	one = 0;
	assert(SetArray((FGPixel *) calloc(areasize(w, h), 1)));
	unsigned char pixel[4];

	for (pass = 0; pass < number_passes; pass++)
	{
		fbptr=GetArray();
		for(i=0;i<(int)height;i++)
		{
			png_read_rows(png_ptr, rptr, NULL, 1);
			for (int xs = 0; xs < w; xs++)
			{
				pixel[2] = *(rp+3*xs);
				pixel[1] = *(rp+3*xs+1);
				pixel[0] = *(rp+3*xs+2);
				*fbptr = FGDirectColor(*(unsigned int *)(&pixel));
				fbptr++;
			}
		}
	}
	free(rp);
	png_read_end(png_ptr, info_ptr);
	png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
	fclose(fh);
	return IMAGE_OK;
#else
	return IMAGE_UNSUPPORTED_FORMAT;
#endif
}

FGBitmap::ReturnValues FGBitmap::pcx(char *n, int& one, unsigned int& bits, unsigned int& c)
{
	char hdr[128];
	FILE *f=0;
	char nm[128];
	int counter, byte, data;
	register unsigned char *src;

		f = fopen(n, "rb");
		if (!f)
		{
			sprintf(nm, "FGBitmap '%s' not found!", n);
			__fg_error(nm, 0);
			return IMAGE_FILE_NOT_FOUND;
		}
		fread(hdr, 128, 1, f);
		if (hdr[0] != 0xA || hdr[1] != 5)
		{
error:
			sprintf(nm, "FGBitmap '%s' isn't PCX 5.0 file!", n);
			__fg_error(nm, 0);
			fclose(f);
			return IMAGE_UNKNOWN_FORMAT;
		}
		w = *(short *)(hdr+66);
		h = *(short *)(hdr+10)+1;
		bits = hdr[3];
		assert(SetArray((FGPixel *) calloc(areasize(w, h), 1)));
		int sz = w * h;
		src = (unsigned char *)GetArray();
		unsigned char *end = src+sz;
		while (src < end)
		{
			byte = fgetc(f);
			if (0xC0 == (0xC0 & byte))
			{
				counter = 0x3F & byte;
				data = fgetc(f);
				while(counter--)
				{
					*src++ = data;
				}
			}
			else
			{
				*src++ = byte;
			}
		}
		byte = fgetc(f);
		if (byte != 0xC) goto error;
		fread(&rgb, 768, 1, f);
		copy_gif_palette((unsigned char *)&rgb, (unsigned char *)&rgb, 256);
		fclose(f);
//		f = 0;
		c = 256;
		one = 3;
		return IMAGE_OK;
}

static int rle_read(FILE * fp, unsigned char * buffer, tga_info * info)
{
	static int repeat = 0;
	static int direct = 0;
	static unsigned char sample[4];
	int head;
	int x, k;

	for (x = 0; x < info->width; x++)
	{
		if (repeat == 0 && direct == 0)
		{
			head = getc(fp);

			if (head == EOF)
			{
				return EOF;
			}
			else if (head >= 128)
			{
				repeat = head - 127;

				if (fread(sample, info->bytes, 1, fp) < 1)
					return EOF;
			}
			else
			{
				direct = head + 1;
			}
		}

		if (repeat > 0)
		{
			for (k = 0; k < info->bytes; ++k)
			{
				buffer[k] = sample[k];
			}

			repeat--;
		}
		else					/* direct > 0 */
		{
			if (fread(buffer, info->bytes, 1, fp) < 1)
				return EOF;

			direct--;
		}

		buffer += info->bytes;
	}

	return 0;
}

static void flip_line(unsigned char * buffer, tga_info * info)
{
	unsigned char temp;
	unsigned char *alt;
	int x, s;

	alt = buffer + (info->bytes * (info->width - 1));

	for (x = 0; x * 2 <= info->width; x++)
	{
		for (s = 0; s < info->bytes; ++s)
		{
			temp = buffer[s];
			buffer[s] = alt[s];
			alt[s] = temp;
		}

		buffer += info->bytes;
		alt -= info->bytes;
	}
}

static void read_line(FILE * fp,
		  FGPixel * row, unsigned char * buffer, tga_info * info, unsigned *palette)
{
 	int width=info->width;
#ifndef INDEX_COLORS
	int x;
	unsigned char *dest = (unsigned char *)row;
#endif
	// read from the file
	if (info->imageCompression == TGA_COMP_RLE)
	{
		rle_read(fp, buffer, info);
	}
	else
	{
		fread(buffer, info->bytes, info->width, fp);
	}
	// flip
	if (info->flipHoriz)
	{
		flip_line(buffer, info);
	}

	switch(info->imageType)
	{
	case TGA_TYPE_COLOR:
#ifdef INDEX_COLORS
		__fg_error("Not supported BPP", 1);
#endif
#ifdef DIRECT_COLORS
		if (info->bpp == 16)
		{
			for (x = 0; x < width; x++)
			{
				dest[0] = buffer[0];
				dest[1] = buffer[1];
#ifdef BPP16
				dest[1] <<= 1;
				dest[1] |= (dest[0]&0x80? 1:0);
				dest[0] = (dest[0]&0x1f)+((dest[0]&0x60)<<1);
#endif
				buffer+=2;
				dest+=2;
			}
		}
		else
		{
			int pad=info->bytes;
			for (x = 0; x < width; x++)
			{
				*((short *)dest) = FGDirectColor(*(unsigned *)buffer);
				buffer+=pad;
				dest+=2;
			}
		}
#endif
#ifdef TRUE_COLORS
		switch (info->bpp)
		{
		case 16:	// 16 > 32bit
			for (x = 0; x < width; x++)
			{
				dest[0] = ((buffer[0] << 3) & 0xf8);
				dest[0] += (dest[2] >> 5);

				dest[1] = ((buffer[0] & 0xe0) >> 2) + ((buffer[1] & 0x03) << 6);
				dest[1] += (dest[1] >> 5);

				dest[2] = ((buffer[1] << 1) & 0xf8);
				dest[2] += (dest[0] >> 5);

				buffer+=2;
				dest+=4;
			}
			break;
		case 24:	// 24 > 32bit
			for (x = 0; x < width; x++)
			{
				dest[0] = buffer[0];
				dest[1] = buffer[1];
				dest[2] = buffer[2];
				dest[3] = 0;
				buffer+=3;
				dest+=4;
			}
			break;
		case 32:  // ONE to ONE
			memcpy (row, buffer, info->width * info->bytes);
			break;
		}
#endif
		break;

	case TGA_TYPE_GRAY:
#ifdef INDEX_COLORS
		memcpy (row, buffer, info->width * info->bytes);
#endif
#ifdef DIRECT_COLORS
		for (x = 0; x < width; x++)
		{
			*((short *)dest) = FGDirectColor(palette[*buffer]);
			buffer++;
			dest+=2;
		}
#endif
#ifdef TRUE_COLORS
		for (x = 0; x < width; x++)
		{
			*(unsigned *)dest = palette[*buffer];
			buffer++;
			dest+=4;
		}
#endif
		break;
	case TGA_TYPE_MAPPED:
#ifdef TRUE_COLORS
			for (x = 0; x < width; x++)
			{
				*(unsigned *)dest = palette[*buffer];
				buffer++;
				dest+=4;
			}
#endif
#ifdef DIRECT_COLORS
		for (x = 0; x < width; x++)
		{
			*((short *)dest) = FGDirectColor(palette[*buffer]);
			buffer++;
			dest+=2;
		}
#endif
#ifdef INDEX_COLORS
		// this works only for BPP8, either this point will be never reached
		memcpy (row, buffer, width * info->bytes);
#endif
		break;
	}
}

//
// tga helper
//
static int ReadImage(FILE * fp, tga_info * info, char * filename, unsigned char *tga_cmap, FGPixel *data)
{
	unsigned char *buffer;
 	FGPixel *row;
	int i, y;

	unsigned int cmap_bytes;
	/* Handle colormap */

	if (info->colorMapType == 1)
	{
		memset(tga_cmap, 0, sizeof(tga_cmap));
		cmap_bytes = (info->colorMapSize + 7) / 8;
		if (cmap_bytes <= 4)
		{
			for (i=0; i<info->colorMapLength; i++)
				fread(tga_cmap+(i*4), cmap_bytes, 1, fp);
		}
		else
		{
			::printf("TGA: File is truncated or corrupted \"%s\"\n", filename);
			return -1;
		}
	}

	/* Allocate the data buffer, with file BPP */
	buffer = (unsigned char *) malloc (info->width * info->bytes);

	if (info->flipVert)
		row = data + (info->width * info->height);
	else
		row = data;

	for (y = 0; y < info->height; ++y)
	{
		if (info->flipVert)
			row -= info->width;

		// read the one line
		read_line(fp, row, buffer, info, (unsigned *)tga_cmap);
//::printf("row = %x\n",row);
		if (!info->flipVert)
			row += info->width;
	}
	free(buffer);
	return 0;
}

FGBitmap::ReturnValues FGBitmap::tga(char *filename, int& one, unsigned int& bits, unsigned int& c)
{
	FILE *fp;
	char nm[128];
//	int counter, byte, data;
	register unsigned char *src;

	tga_info info;
	unsigned char header[18],
		footer[26],
		extension[495];
	long offset;

	memset(&info, 0, sizeof(info));
	fp = fopen(filename, "rb");
	if (!fp)
	{
		sprintf(nm, "FGBitmap '%s' not found!", filename);
		__fg_error(nm, 0);
		return IMAGE_FILE_NOT_FOUND;
	}

	if (!fseek(fp, -26L, SEEK_END))
	{							/* Is file big enough for a footer? */
		if (fread(footer, sizeof(footer), 1, fp) != 1)
		{
			::printf("TGA: Cannot read footer from %s\n", filename);
			return IMAGE_UNKNOWN_FORMAT;
		}
		else if (memcmp(footer + 8, magic, sizeof(magic)) == 0)
		{
			/* Check the signature. */
			offset = footer[0] + (footer[1] * 256) + (footer[2] * 65536)
				+ (footer[3] * 16777216);

			if (fseek(fp, offset, SEEK_SET) ||
				fread(extension, sizeof(extension), 1, fp) != 1)
			{
				::printf("TGA: Cannot read extension from \"%s\"\n", filename);
				return IMAGE_UNKNOWN_FORMAT;
			}

			/* Eventually actually handle version 2 TGA here */
		}
	}

	if (fseek(fp, 0, SEEK_SET) || fread(header, sizeof(header), 1, fp) != 1)
	{
		::printf("TGA: Cannot read header from \"%s\"\n", filename);
		return IMAGE_UNKNOWN_FORMAT;
	}

	switch (header[2])
	{
		case 1:
			info.imageType = TGA_TYPE_MAPPED;
			info.imageCompression = TGA_COMP_NONE;
			break;
		case 2:
			info.imageType = TGA_TYPE_COLOR;
			info.imageCompression = TGA_COMP_NONE;
			break;
		case 3:
			info.imageType = TGA_TYPE_GRAY;
			info.imageCompression = TGA_COMP_NONE;
			break;

		case 9:
			info.imageType = TGA_TYPE_MAPPED;
			info.imageCompression = TGA_COMP_RLE;
			break;
		case 10:
			info.imageType = TGA_TYPE_COLOR;
			info.imageCompression = TGA_COMP_RLE;
			break;
		case 11:
			info.imageType = TGA_TYPE_GRAY;
			info.imageCompression = TGA_COMP_RLE;
			break;

		default:
			info.imageType = 0;
	}

	info.idLength = header[0];
	info.colorMapType = header[1];

	info.colorMapIndex = header[3] + header[4] * 256;
	info.colorMapLength = header[5] + header[6] * 256;
	info.colorMapSize = header[7];

	info.xOrigin = header[8] + header[9] * 256;
	info.yOrigin = header[10] + header[11] * 256;
	info.width = header[12] + header[13] * 256;
	info.height = header[14] + header[15] * 256;

	bits = info.bpp = header[16];
	info.bytes = (info.bpp + 7) / 8;
	info.alphaBits = header[17] & 0x0f;	/* Just the low 4 bits */
	info.flipHoriz = (header[17] & 0x10) ? 1 : 0;
	info.flipVert = (header[17] & 0x20) ? 0 : 1;

	switch (info.imageType)
	{
		case TGA_TYPE_MAPPED:
			if (info.bpp != 8)
			{
				::printf("TGA: Unhandled sub-format in \"%s\"\n", filename);
				return IMAGE_UNSUPPORTED_FORMAT;
			}
			break;
		case TGA_TYPE_COLOR:
			if (info.alphaBits == 0 && info.bpp == 32)
			{
				::printf("TGA: Possibly incorrect alpha in \"%s\"\n",
						  filename);
				info.alphaBits = 8;
			}
			if (info.bpp != 16 && info.bpp != 24
				&& (info.alphaBits != 8 || info.bpp != 32))
			{
				::printf("TGA: Unhandled sub-format in \"%s\"\n", filename);
				return IMAGE_UNSUPPORTED_FORMAT;
			}
			break;
		case TGA_TYPE_GRAY:
			if (info.alphaBits == 0 && info.bpp == 16)
			{
				::printf("TGA: Possibly incorrect alpha in \"%s\"\n",
						  filename);
				info.alphaBits = 8;
			}
			if (info.bpp != 8 && (info.alphaBits != 8 || info.bpp != 16))
			{
				::printf("TGA: Unhandled sub-format in \"%s\"\n", filename);
				return IMAGE_UNSUPPORTED_FORMAT;
			}
			break;

		default:
			::printf("TGA: Unknown image type for \"%s\"\n", filename);
			return IMAGE_UNSUPPORTED_FORMAT;
	}

	/* Plausible but unhandled formats */
	if (info.bytes * 8 != info.bpp)
	{
		::printf("TGA: No support yet for TGA with these parameters\n");
		return IMAGE_UNSUPPORTED_FORMAT;
	}

	/* Check that we have a color map only when we need it. */
	if (info.imageType == TGA_TYPE_MAPPED && info.colorMapType != 1)
	{
		::printf("TGA: indexed image has invalid color map type %d\n",
				  info.colorMapType);
		return IMAGE_UNSUPPORTED_FORMAT;
	}
	else if (info.imageType != TGA_TYPE_MAPPED && info.colorMapType != 0)
	{
		::printf("TGA: non-indexed image has invalid color map type %d\n",
				  info.colorMapType);
		return IMAGE_UNSUPPORTED_FORMAT;
	}
#ifdef INDEX_COLORS
	else if (info.imageType == TGA_TYPE_COLOR)
	{
		::printf("TGA: non-indexed image not supported with BPP==8\n");
		return IMAGE_UNSUPPORTED_FORMAT;
	}
#endif
	/* Skip the image ID field. */
	if (info.idLength && fseek(fp, info.idLength, SEEK_CUR))
	{
		::printf("TGA: File is truncated or corrupted \"%s\"\n", filename);
		return IMAGE_UNKNOWN_FORMAT;
	}

	w = info.width;
	h = info.height;
	bits = info.bpp;

	if (info.bpp>8)
	{
		c = 0;
		one = 0;
	}
	else
	{
		c = 256;
		one = 4;
	}

	assert(SetArray((FGPixel *) calloc(areasize(w, h), 1)));
	src = (unsigned char *)GetArray();

	// generate gray ramp
	if (info.imageType == TGA_TYPE_GRAY)
		for(int i=0; i<(int)c; i++)
			*(unsigned *)(&rgb.rgb4[i]) = i+(i<<8)+(i<<16);

	ReadImage(fp, &info, filename, (unsigned char *)&rgb, GetArray());

	// mark as already converted
	bits = 24;
	fclose(fp);
	return IMAGE_OK;
}

FGBitmap::ReturnValues FGBitmap::gif(char *n, int& one, unsigned int& bits, unsigned int& c)
{
	GIFStream *gif=0;
	char nm[128];

		gif = GIFRead(n);
		if (gif)
		{
			w = gif->width;
			h = gif->height;
			one = 3;
			c = gif->cmapSize;
			bits = 8;
			if (c) copy_gif_palette((unsigned char *)&rgb, gif->cmapData[0], c);
			assert(SetArray((FGPixel *) malloc(areasize(w, h))));
			memset(GetArray(), gif->background, areasize(w, h));
			GIFData *d = gif->data;
			while(d)
			{
				if (d->type == gif_image)
				{
					if (d->data.image.cmapSize)
					{
						c = d->data.image.cmapSize;
						if (c) copy_gif_palette((unsigned char *)&rgb, d->data.image.cmapData[0], c);
					}
					state.colorkey = d->info.transparent;
					memcpy(GetArray(), d->data.image.data, d->width*d->height);
					free(d->data.image.data);
				}
				else if(d->type == gif_text || d->type == gif_comment)
				{
					if (d->data.comment.text) free(d->data.comment.text);
				}
				GIFData *pom = d;
				d = d->next;
				free(pom);
			}
			free(gif);
		}
		else
		{
			sprintf(nm, "GIF file '%s' loading error!", n);
			__fg_error(nm, 0);
			return IMAGE_FILE_NOT_FOUND;
		}
		return IMAGE_OK;
}

FGBitmap::~FGBitmap()
{
#ifdef INDEX_COLORS
	if (cUsedTable)
	{
		for (int i = 0; i < 256; i++)
			if (cUsedTable[i] != -1)
				DeleteColor(cUsedTable[i]);
		delete cUsedTable;
	}
#endif
}

/**
	Create a BITMAP object in memory with specified size and color layout.
*/
FGBitmap::FGBitmap(int ww, int hh, FGPixel color)
: FGDrawBuffer(ww, hh, BMP_MEM, color)
{
	init();
}

/**
	Create a BITMAP object in memory as exact copy of the FGDrawBuffer object.
	This one has dimensions and contents the same as source buffer.
	The image has been copied from, that you can't expect that any next changes
	are appears in it. The application of one are when you want save
	the contents of a window to the BMP file by example.
*/
FGBitmap::FGBitmap(FGDrawBuffer * wPtr)
:FGDrawBuffer(wPtr->GetW(), wPtr->GetH())
{
	init();
	memcpy(image, wPtr->GetArray(), areasize(wPtr->GetW(), wPtr->GetH()));
}

void FGBitmap::init(void)
{
	cUsedTable = 0;
	format = IMAGE_UNKNOWN;
	name = 0;
	memset(&bmih, 0, sizeof(bmih));
	state._ink = CWHITE;
	state._paper = CBLACK;
	state._font = 2;
	state._ppop = _GSET;
	state.colorkey = DEFAULT_COLOR_KEY;
	state.alpha = DEFAULT_ALPHA_VALUE;
}

/**
	Saves a BITMAP object into the current directory.
	The file will be standard Windows BMP file. Extension
	is added automagically.
	@param filename
	@param convert convert to PNG image too.
	@return value is false if operation fail. On linux platform
	is file saved to PNG format also, if imagemagic is installed.
*/
FGBitmap::ReturnValues FGBitmap::BitmapSave(const char *filename, bool convert)
{
	FILE *fp=0;

	if (GetType() == BMP_NONE)
		return IMAGE_UNKNOWN_FORMAT;

#ifndef INDEX_COLORS
	int j, rest;
	char *bmp2;
#endif

	bmfh.usType = 'M' * 256 + 'B';
	bmfh.reserve1 = 0;
	bmfh.reserve2 = 0;
	bmi.reserve = 0;
	bmih.bmih2.size = 0x28;
	bmih.bmih2.cx = w;
	bmih.bmih2.cy = h;
	bmih.bmih2.cPlanes = 1;
	bmih.bmih2.ulCompression = 0;
	bmih.bmih2.cbImage = w * h;
	bmih.bmih2.cxResolution = 0;
	bmih.bmih2.cyResolution = 0;
#ifdef INDEX_COLORS
	bmi.imageOffset = 0x36 + 0x400;
	bmfh.cbSizeLow = w * h + 1024 + 0x36;
	bmfh.cbSizeHigh = (w * h + 1024 + 0x36) >> 16;
	bmih.bmih2.cclrUsed = 256;
	bmih.bmih2.cclrImportant = 256;
	bmih.bmih2.cBitCount = 8;
	memcpy(&rgb, &_fgl_palette, sizeof(rgb));
#else
	bmi.imageOffset = 0x36;
	bmfh.cbSizeLow = w * h + 0x36;
	bmfh.cbSizeHigh = (w * h + 0x36) >> 16;
	bmih.bmih2.cclrUsed = 0;
	bmih.bmih2.cclrImportant = 0;
	bmih.bmih2.cBitCount = 24;
#endif // index

	char* ext;
	if ((ext = (char *)strstr(filename,".bmp")) == 0)
	{
		char new_filename[128];
		strncpy(new_filename, filename, sizeof(new_filename)-5);
		strcat(new_filename, ".bmp");
		fp = fopen(new_filename, "wb");
	}
	else
		fp = fopen(filename, "wb");

	if (fp == 0)
	{
		__fg_error("Bitmap not saved because of disk write error!", 0);
		return IMAGE_FILE_NOT_FOUND;
	}
	assert(sizeof(bmfh) == 10);
	fwrite(&bmfh, sizeof(bmfh), 1, fp);
	fwrite(&bmi, sizeof(bmi), 1, fp);
	fwrite(&bmih, sizeof(bmih.bmih2), 1, fp);

#ifdef INDEX_COLORS
	fwrite(&rgb, sizeof(rgb.rgb4), 1, fp);
	rotate(ROTATE_VERTICAL);
	fwrite(image, w * h, 1, fp);
	rotate(ROTATE_VERTICAL);
#endif // index

#ifndef INDEX_COLORS
#if (FASTGL_BPP == 15 || FASTGL_BPP == 16)
	unsigned cc, aa;
#endif // BPP16
	for (int i = h - 1; i >= 0; i--)
	{
		bmp2 = (char *) (image + w * i);
		for (j = 0; j < w; j++)
		{
#ifdef TRUE_COLORS
			fwrite(bmp2, 3, 1, fp);
			bmp2 += 4;
#else // true
			cc = *(FGPixel *) bmp2;
#if (FASTGL_BPP == 15)
			aa = ((cc << 9) & 0xF80000) | ((cc << 6) & 0xF800) | ((cc << 3) & 0xF8);
#else
			aa = ((cc << 8) & 0xF80000) | ((cc << 5) & 0xFc00) | ((cc << 3) & 0xF8);
#endif // direct
			fwrite(&aa, 3, 1, fp);
			bmp2 += 2;
#endif // true
		}
#ifdef TRUE_COLORS
		rest = (w * 3) % 4;
#else
		rest = (w * 2) % 4;
#endif // true
		while (rest & 3)
		{
			rest++;
			fputc(0, fp);
		}
	}
#endif // not index_color
	fclose(fp);
#ifdef __linux__
	if (convert)
	{
		char ss[64];
		sprintf(ss, "/usr/X11R6/bin/convert %s.bmp %s.png", filename, filename);
		system(ss);
	}
#endif
	return IMAGE_OK;
}

// swaps the blue and red components (for RGB images)
// > buffer: pointer to the pixels
// > lenght: number of bytes to swap. lenght may not exceed the scan line.
void FGBitmap::RGBtoBGR(unsigned char *buffer, unsigned char *input, int length)
{
	if (buffer)
	{
		length *= 3;
		for (int i=0;i<length;i+=3)
		{
			buffer[i+2] = *input++;
			buffer[i+1] = *input++;
			buffer[i] = *input++;
			input++;
		}
	}
}

FGBitmap::ReturnValues FGBitmap::JpegSave(const char *filename)
{
#ifdef FG_JPEG

#ifdef INDEX_COLORS
	__fg_error("JPEG can save only RGB or GreyScale images", 0);
	return IMAGE_UNSUPPORTED_FORMAT;
#endif

	FILE *outfile = fopen(filename, "wb");

	if (outfile == 0)
	{
		__fg_error("Bitmap not saved because of disk write error!", 0);
		return IMAGE_FILE_WRITE_ERROR;
	}

	// This struct contains the JPEG compression parameters and pointers to
	// working space (which is allocated as needed by the JPEG library).
	// It is possible to have several such structures, representing multiple
	// compression/decompression processes, in existence at once.  We refer
	// to any one struct (and its associated working data) as a "JPEG object".
	struct jpeg_compress_struct cinfo;

	// This struct represents a JPEG error handler.  It is declared separately
	// because applications often want to supply a specialized error handler
	// (see the second half of this file for an example).  But here we just
	// take the easy way out and use the standard error handler, which will
	// print a message on stderr and call exit() if compression fails.
	// Note that this struct must live as long as the main JPEG parameter
	// struct, to avoid dangling-pointer problems.
	//
	struct ima_error_mgr jerr;
	jerr.buffer = szLastError;
	// More stuff
	int row_stride;
	JSAMPARRAY buffer;

	// Step 1: allocate and initialize JPEG compression object
	// We have to set up the error handler first, in case the initialization
	// step fails.  (Unlikely, but it could happen if you are out of memory.)
	// This routine fills in the contents of struct jerr, and returns jerr's
	// address which we place into the link field in cinfo.
	//
	// We set up the normal JPEG error routines, then override error_exit.
	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = fgl_jpeg_error_exit;

	// Establish the setjmp return context for my_error_exit to use.
	if (setjmp(jerr.setjmp_buffer))
	{
		// If we get here, the JPEG code has signaled an error.
		// We need to clean up the JPEG object, close the input file, and return.
		strcpy(szLastError, jerr.buffer);
		jpeg_destroy_compress(&cinfo);
		return IMAGE_FILE_WRITE_ERROR;
	}

	// Now we can initialize the JPEG compression object.
	jpeg_create_compress(&cinfo);
	// Step 2: specify data destination (eg, a file)
	// Note: steps 2 and 3 can be done in either order.
	// Here we use the library-supplied code to send compressed data to a
	// stdio stream.  You can also write your own code to do something else.
	// VERY IMPORTANT: use "b" option to fopen() if you are on a machine that
	// requires it in order to write binary files.

//	jpeg_stdio_dest(&cinfo, outfile);
	CxFileJpg dest(outfile);
	cinfo.dest = &dest;

	// Step 3: set parameters for compression
	// First we supply a description of the input image.
	// Four fields of the cinfo struct must be filled in:
	cinfo.image_width = w; 	// image width and height, in pixels
	cinfo.image_height = h;
/*
	if (IsGrayScale())
	{
		cinfo.input_components = 1;			// # of color components per pixel
		cinfo.in_color_space = JCS_GRAYSCALE; // colorspace of input image
	}
	else
	{
*/		cinfo.input_components = 3; 	// # of color components per pixel
		cinfo.in_color_space = JCS_RGB; // colorspace of input image
//	}

	/* Now use the library's routine to set default compression parameters.
	* (You must set at least cinfo.in_color_space before calling this,
	* since the defaults depend on the source color space.)
	*/
	jpeg_set_defaults(&cinfo);
	/* Now you can set any non-default parameters you wish to.
	* Here we just illustrate the use of quality (quantization table) scaling:
	*/
	//jpeg_set_quality(&cinfo, info.nQuality, true /* limit to baseline-JPEG values */);

#ifdef C_ARITH_CODING_SUPPORTED
	if ((GetCodecOption() & ENCODE_ARITHMETIC) != 0)
		cinfo.arith_code = true;
#endif
#ifdef ENTRPY_OPT_SUPPORTED
	if ((GetCodecOption() & ENCODE_OPTIMIZE) != 0)
		cinfo.optimize_coding = true;
#endif
//	if ((GetCodecOption() & ENCODE_GRAYSCALE) != 0)
//		jpeg_set_colorspace(&cinfo, JCS_GRAYSCALE);
//	if ((GetCodecOption() & ENCODE_SMOOTHING) != 0)
//		cinfo.smoothing_factor = m_nSmoothing;
	jpeg_set_quality(&cinfo, 60, true);
	cinfo.optimize_coding = true;
#ifdef C_PROGRESSIVE_SUPPORTED
	if ((GetCodecOption() & ENCODE_PROGRESSIVE) != 0)
		jpeg_simple_progression(&cinfo);
#endif
#ifdef C_LOSSLES_SUPPORTED
	if ((GetCodecOption() & ENCODE_LOSSLESS) != 0)
		jpeg_simple_lossless(&cinfo, m_nPredictor, m_nPointTransform);
#endif

	cinfo.density_unit=1;
	cinfo.X_density=(unsigned short)75;
	cinfo.Y_density=(unsigned short)75;

	/* Step 4: Start compressor */
	/* true ensures that we will write a complete interchange-JPEG file.
	* Pass true unless you are very sure of what you're doing.
	*/
	jpeg_start_compress(&cinfo, true);

	/* Step 5: while (scan lines remain to be written) */
	/*           jpeg_write_scanlines(...); */
	/* Here we use the library's state variable cinfo.next_scanline as the
	* loop counter, so that we don't have to keep track ourselves.
	* To keep things simple, we pass one scanline per call; you can pass
	* more if you wish, though.
	*/
	row_stride = w;	/* JSAMPLEs per row in image_buffer */

	//<DP> "8+row_stride" fix heap deallocation problem during debug???
	buffer = (*cinfo.mem->alloc_sarray)
		((j_common_ptr) &cinfo, JPOOL_IMAGE, 8+row_stride*3, 1);

	while (cinfo.next_scanline < cinfo.image_height)
	{
		// not necessary if swapped red and blue definition in jmorecfg.h;ln322 <W. Morrison>
		// swap R & B for RGB images
		RGBtoBGR(buffer[0], (unsigned char *)CalcAddr(0, cinfo.next_scanline), row_stride); // Lance : 1998/09/01 : Bug ID: EXP-2.1.1-9
		jpeg_write_scanlines(&cinfo, buffer, 1);
	}

	jpeg_finish_compress(&cinfo);

	/* And we're done! */
	return IMAGE_OK;
#else
	return IMAGE_UNSUPPORTED_FORMAT;
#endif
}


FGBitmap::ReturnValues FGBitmap::PngSave(const char *filename)
{
	return IMAGE_UNSUPPORTED_FORMAT;
}


/* +-------------------------------------------------------------------+ */
/* | Copyright 1990 - 1994, David Koblas. (koblas@netcom.com)          | */
/* |   Permission to use, copy, modify, and distribute this software   | */
/* |   and its documentation for any purpose and without fee is hereby | */
/* |   granted, provided that the above copyright notice appear in all | */
/* |   copies and that both that copyright notice and this permission  | */
/* |   notice appear in supporting documentation.  This software is    | */
/* |   provided "as is" without express or implied warranty.           | */
/* +-------------------------------------------------------------------+ */

GIFStream *GIFReadFP(FILE * fd)
{
	unsigned char buf[256];
	unsigned char c;
	GIFStream *stream=0;
	GIFData *cur, **end;
	GIF89info info;
	int resetInfo = true;
	int n;

	if (fd == NULL)
		return NULL;

	if (setjmp(setjmp_buffer))
		goto out;

	if (!ReadOK(fd, buf, 6))
		_ERROR("error reading magic number");

	if (strncmp((char *) buf, "GIF", 3) != 0)
		_ERROR("not a GIF file");

	if ((strncmp((const char *) buf + 3, "87a", 3) != 0) &&
		(strncmp((const char *) buf + 3, "89a", 3) != 0))
		_ERROR("bad version number, not '87a' or '89a'");

	if (!ReadOK(fd, buf, 7))
		_ERROR("failed to read screen descriptor");

	stream = NEW(GIFStream);

	stream->width = MKINT(buf[0], buf[1]);
	stream->height = MKINT(buf[2], buf[3]);

	stream->cmapSize = 2 << (buf[4] & 0x07);
	stream->colorMapSize = stream->cmapSize;
	stream->colorResolution = ((int) (buf[4] & 0x70) >> 3) + 1;
	stream->background = buf[5];
	stream->aspectRatio = buf[6];

	stream->data = NULL;

	end = &stream->data;

	/*
	   **  Global colormap is present.
	 */
	if (BitSet(buf[4], LOCALCOLORMAP))
	{
		if (readColorMap(fd, stream->cmapSize, stream->cmapData))
			_ERROR("unable to get global colormap");
	}
	else
	{
		stream->cmapSize = 0;
		stream->background = -1;
	}

	if (stream->aspectRatio != 0 && stream->aspectRatio != 49)
	{
		float r;

		r = ((float) stream->aspectRatio + 15.0) / 64.0;
		INFO_MSG(("warning - non-square pixels; to fix do a 'pnmscale -%cscale %g'",
				  r < 1.0 ? 'x' : 'y',
				  r < 1.0 ? 1.0 / r : r));
	}

	while (ReadOK(fd, &c, 1) && c != ';')
	{
		if (resetInfo)
		{
			info.disposal = (GIFDisposalType) 0;
			info.inputFlag = 0;
			info.delayTime = 0;
			info.transparent = -1;
			resetInfo = false;
		}
		cur = NULL;

		if (c == '!')
		{						/* Extension */
			if (!ReadOK(fd, &c, 1))
				_ERROR("EOF / read error on extention function code");
			if (c == 0xf9)
			{					/* graphic control */
				(void) GetDataBlock(fd, buf);
				info.disposal = (GIFDisposalType) ((buf[0] >> 2) & 0x7);
				info.inputFlag = (buf[0] >> 1) & 0x1;
				info.delayTime = MKINT(buf[1], buf[2]);
				if (BitSet(buf[0], 0x1))
					info.transparent = buf[3];

				while (GetDataBlock(fd, buf) != 0)
					;
			}
			else if (c == 0xfe || c == 0x01)
			{
				int len = 0;
				int size = 256;
				char *text;

				/*
				   **  Comment or Plain Text
				 */

				cur = NEW(GIFData);

				if (c == 0x01)
				{
					(void) GetDataBlock(fd, buf);

					cur->type = gif_text;
					cur->info = info;
					cur->x = MKINT(buf[0], buf[1]);
					cur->y = MKINT(buf[2], buf[3]);
					cur->width = MKINT(buf[4], buf[5]);
					cur->height = MKINT(buf[6], buf[7]);

					cur->data.text.cellWidth = buf[8];
					cur->data.text.cellHeight = buf[9];
					cur->data.text.fg = buf[10];
					cur->data.text.bg = buf[11];

					resetInfo = true;
				}
				else
				{
					cur->type = gif_comment;
				}

				text = (char *) malloc(size);

				while ((n = GetDataBlock(fd, buf)) != 0)
				{
					if (n + len >= size)
						text = (char *) realloc(text, size += 256);
					memcpy(text + len, buf, n);
					len += n;
				}

				if (c == 0x01)
				{
					cur->data.text.len = len;
					cur->data.text.text = text;
				}
				else
				{
					cur->data.comment.len = len;
					cur->data.comment.text = text;
				}
			}
			else
			{
				/*
				   **  Unrecogonized extension, consume it.
				 */
				while (GetDataBlock(fd, buf) > 0)
					;
			}
		}
		else if (c == ',')
		{
			if (!ReadOK(fd, buf, 9))
				_ERROR("couldn't read left/top/width/height");

			cur = NEW(GIFData);

			cur->type = gif_image;
			cur->info = info;
			cur->x = MKINT(buf[0], buf[1]);
			cur->y = MKINT(buf[2], buf[3]);
			cur->width = MKINT(buf[4], buf[5]);
			cur->height = MKINT(buf[6], buf[7]);
			cur->data.image.cmapSize = 1 << ((buf[8] & 0x07) + 1);
			if (BitSet(buf[8], LOCALCOLORMAP))
			{
				if (readColorMap(fd, cur->data.image.cmapSize,
								 cur->data.image.cmapData))
					_ERROR("unable to get local colormap");
			}
			else
			{
				cur->data.image.cmapSize = 0;

			}
			cur->data.image.data = (unsigned char *) malloc(cur->width * cur->height);
			cur->data.image.interlaced = BitSet(buf[8], INTERLACE);
			readImage(fd, BitSet(buf[8], INTERLACE),
					  cur->width, cur->height, cur->data.image.data);

			resetInfo = true;
		}
		else
		{
			INFO_MSG(("bogus character 0x%02x, ignoring", (int) c));
		}

		if (cur != NULL)
		{
			*end = cur;
			end = &cur->next;
			cur->next = NULL;
		}
	}

	if (c != ';')
		_ERROR("EOF / data stream");

  out:
	return stream;
}

GIFStream *GIFRead(char *file)
{
	FILE *fp = fopen(file, "rb");
	GIFStream *stream = NULL;

	if (fp != NULL)
	{
		stream = GIFReadFP(fp);
		fclose(fp);
	}
	return stream;
}

static int readColorMap(FILE * fd, int size,
						unsigned char data[GIF_MAXCOLORS][3])
{
	int i;
	unsigned char rgb[3 * GIF_MAXCOLORS];
	unsigned char *cp = rgb;

	if (!ReadOK(fd, rgb, size * 3))
		return true;

	for (i = 0; i < size; i++)
	{
		data[i][0] = *cp++;
		data[i][1] = *cp++;
		data[i][2] = *cp++;
	}

	return false;
}

/*
   **
 */

static int ZeroDataBlock = false;

static int GetDataBlock(FILE * fd, unsigned char *buf)
{
	unsigned char count;

	if (!ReadOK(fd, &count, 1))
	{
		INFO_MSG(("error in getting DataBlock size"));
		return -1;
	}

	ZeroDataBlock = count == 0;
	if ((count != 0) && (!ReadOK(fd, buf, count)))
	{
		INFO_MSG(("error in reading DataBlock"));
		return -1;
	}
	return count;
}

/*
   **
   **
 */

/*
   **  Pulled out of nextCode
 */
static int curbit, lastbit, get_done, last_byte;
static int return_clear;

/*
   **  Out of nextLWZ
 */
static int stack[(1 << (MAX_LWZ_BITS)) * 2], *sp;
static int code_size, set_code_size;
static int max_code, max_code_size;
static int clear_code, end_code;

static void initLWZ(int input_code_size)
{
	set_code_size = input_code_size;
	code_size = set_code_size + 1;
	clear_code = 1 << set_code_size;
	end_code = clear_code + 1;
	max_code_size = 2 * clear_code;
	max_code = clear_code + 2;

	curbit = lastbit = 0;
	last_byte = 2;
	get_done = false;

	return_clear = true;

	sp = stack;
}

static int nextCode(FILE * fd, int code_size)
{
	static unsigned char buf[280];
	static int maskTbl[16] =
	{
		0x0000, 0x0001, 0x0003, 0x0007,
		0x000f, 0x001f, 0x003f, 0x007f,
		0x00ff, 0x01ff, 0x03ff, 0x07ff,
		0x0fff, 0x1fff, 0x3fff, 0x7fff,
	};
	int i, j, ret, end;

	if (return_clear)
	{
		return_clear = false;
		return clear_code;
	}

	end = curbit + code_size;

	if (end >= lastbit)
	{
		int count;

		if (get_done)
		{
			if (curbit >= lastbit)
				_ERROR("ran off the end of my bits");
			return -1;
		}
		buf[0] = buf[last_byte - 2];
		buf[1] = buf[last_byte - 1];

		if ((count = GetDataBlock(fd, &buf[2])) == 0)
			get_done = true;

		last_byte = 2 + count;
		curbit = (curbit - lastbit) + 16;
		lastbit = (2 + count) * 8;

		end = curbit + code_size;
	}

	j = end / 8;
	i = curbit / 8;

	if (i == j)
		ret = buf[i];
	else if (i + 1 == j)
		ret = buf[i] | (buf[i + 1] << 8);
	else
		ret = buf[i] | (buf[i + 1] << 8) | (buf[i + 2] << 16);

	ret = (ret >> (curbit % 8)) & maskTbl[code_size];

	curbit += code_size;

	return ret;
}

#define readLWZ(fd) ((sp > stack) ? *--sp : nextLWZ(fd))

static int nextLWZ(FILE * fd)
{
	static int table[2][(1 << MAX_LWZ_BITS)];
	static int firstcode, oldcode;
	int code, incode;
	register int i;

	while ((code = nextCode(fd, code_size)) >= 0)
	{
		if (code == clear_code)
		{
			for (i = 0; i < clear_code; ++i)
			{
				table[0][i] = 0;
				table[1][i] = i;
			}
			for (; i < (1 << MAX_LWZ_BITS); ++i)
				table[0][i] = table[1][i] = 0;
			code_size = set_code_size + 1;
			max_code_size = 2 * clear_code;
			max_code = clear_code + 2;
			sp = stack;
			do
			{
				firstcode = oldcode = nextCode(fd, code_size);
			}
			while (firstcode == clear_code);

			return firstcode;
		}
		if (code == end_code)
		{
			int count;
			unsigned char buf[260];

			if (ZeroDataBlock)
				return -2;

			while ((count = GetDataBlock(fd, buf)) > 0)
				;

//			if (count != 0)
//				INFO_MSG(("missing EOD in data stream"));
			return -2;
		}

		incode = code;

		if (code >= max_code)
		{
			*sp++ = firstcode;
			code = oldcode;
		}

		while (code >= clear_code)
		{
			*sp++ = table[1][code];
			if (code == table[0][code])
				_ERROR("circular table entry BIG ERROR");
			code = table[0][code];
		}

		*sp++ = firstcode = table[1][code];

		if ((code = max_code) < (1 << MAX_LWZ_BITS))
		{
			table[0][code] = oldcode;
			table[1][code] = firstcode;
			++max_code;
			if ((max_code >= max_code_size) &&
				(max_code_size < (1 << MAX_LWZ_BITS)))
			{
				max_code_size *= 2;
				++code_size;
			}
		}

		oldcode = incode;

		if (sp > stack)
			return *--sp;
	}
	return code;
}

static void readImage(FILE * fd, int interlace, int width, int height,
					  unsigned char *data)
{
	unsigned char *dp, c;
	int v, xpos = 0, ypos = 0;

	/*
	   **  Initialize the Compression routines
	 */
	if (!ReadOK(fd, &c, 1))
		_ERROR("EOF / read error on image data");

	initLWZ(c);

	if (fgstate.verbose)
		INFO_MSG(("reading %d by %d%s GIF image",
				  width, height, interlace ? " interlaced" : ""));

	if (interlace)
	{
		int i;
		int pass = 0, step = 8;

		for (i = 0; i < height; i++)
		{
			dp = &data[width * ypos];
			for (xpos = 0; xpos < width; xpos++)
			{
				if ((v = readLWZ(fd)) < 0)
					goto fini;

				*dp++ = v;
			}
			if ((ypos += step) >= height)
			{
				do
				{
					if (pass++ > 0)
						step /= 2;
					ypos = step / 2;
				}
				while (ypos > height);
			}
		}
	}
	else
	{
		dp = data;
		for (ypos = 0; ypos < height; ypos++)
		{
			for (xpos = 0; xpos < width; xpos++)
			{
				if ((v = readLWZ(fd)) < 0)
					goto fini;

				*dp++ = v;
			}
		}
	}

  fini:
	if (readLWZ(fd) >= 0)
		INFO_MSG(("too much input data, ignoring extra..."));

	return;
}

/*
 *  geometry is 0 .. w       in X
 *				0 .. h       in Y
 *	clipping is 0 .. wwrk in X
 *				0 .. hwrk in Y
 *	offset is xoff & yoff 0 .. w-1
 *
 *	NOTE: you must applicate offset first before clipping!
 */

#ifdef FG_NAMESPACE
}
#endif








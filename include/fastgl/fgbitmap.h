#ifndef _FGBITMAP_H_
#define _FGBITMAP_H_

#ifdef __BORLANDC__
#pragma option push -b
#pragma option push -a4
#endif

#ifdef FG_NAMESPACE
namespace fgl {
#endif

#ifndef __GNUC__
#pragma	pack(1)
#endif

//!	Used for an image file manipulating (load/save of various formats is implemented).
class FGBitmap : public FGDrawBuffer
{
	public:
        //! return values from LoadImage
		enum ReturnValues { IMAGE_OK, IMAGE_FILE_NOT_FOUND, IMAGE_UNKNOWN_FORMAT,
		    IMAGE_UNSUPPORTED_FORMAT, IMAGE_FILE_WRITE_ERROR };
    private:
		struct RGB3
		{
			char bBlue;				 /* Blue component of the color definition */
			char bGreen;			 /* Green component of the color definition*/
	    	char bRed;				 /* Red component of the color definition  */
    	};
		struct RGB4
    	{
	    	char bBlue;				 /* Blue component of the color definition */
		    char bGreen;			 /* Green component of the color definition*/
    		char bRed;				 /* Red component of the color definition  */
	    	char fcOptions;			 /* Reserved, must be zero                 */
    	};
		struct bitmap_file_header
    	{
			unsigned short	usType;
		    unsigned short	cbSizeLow, cbSizeHigh;
			short			reserve1, reserve2;
		};
    	struct bitmap_info
		{
			unsigned short	imageOffset;
	    	short			reserve;
    	};
    	struct bitmap_info_header1
		{
    	  unsigned int	 size;
		  unsigned short cx;			   /* Bit-map width in pels                  */
		  unsigned short cy;			   /* Bit-map height in pels                 */
		  unsigned short cPlanes;		   /* Number of bit planes                   */
		  unsigned short cBitCount;		   /* Number of bits per pel within a plane  */
		  unsigned int	 ulCompression;	   /* Compression scheme used to store the bitmap */
    	  unsigned int	 cbImage;		   /* Length of bit-map storage data in chars*/
    	  unsigned int	 cxResolution;	   /* x resolution of target device          */
		  unsigned int	 cyResolution;	   /* y resolution of target device          */
		  unsigned int	 cclrUsed;		   /* Number of color indices used           */
		  unsigned int	 cclrImportant;	   /* Number of important color indices      */
    	};
		struct bitmap_info_header2
    	{
		  unsigned int	 size;
		  unsigned int	 cx;			   /* Bit-map width in pels                  */
    	  unsigned int	 cy;			   /* Bit-map height in pels                 */
    	  unsigned short cPlanes;		   /* Number of bit planes                   */
		  unsigned short cBitCount;		   /* Number of bits per pel within a plane  */
    	  unsigned int	 ulCompression;	   /* Compression scheme used to store the bitmap */
		  unsigned int	 cbImage;		   /* Length of bit-map storage data in chars*/
    	  unsigned int	 cxResolution;	   /* x resolution of target device          */
    	  unsigned int	 cyResolution;	   /* y resolution of target device          */
    	  unsigned int	 cclrUsed;		   /* Number of color indices used           */
		  unsigned int	 cclrImportant;	   /* Number of important color indices      */
    	};

		void init(void);
		bitmap_file_header	bmfh;
		bitmap_info			bmi;
		union {
			bitmap_info_header1	bmih1;
			bitmap_info_header2	bmih2;
		} bmih;
		union {
			RGB3 rgb3[256];
			RGB4 rgb4[256];
		} rgb;

		int*		cUsedTable;
		int			format;			    // IMAGE_*
		char	    szLastError[256];	//debugging

		void		DoPalette(int);
		void 		RGBtoBGR(unsigned char *buffer, unsigned char *input, int length);

		ReturnValues pcx(char *n, int& one, unsigned int& bits, unsigned int& c);
		ReturnValues bmp(char *n, int& one, unsigned int& bits, unsigned int& c);
		ReturnValues gif(char *n, int& one, unsigned int& bits, unsigned int& c);
		ReturnValues jpg(char *n, int& one, unsigned int& bits, unsigned int& c);
		ReturnValues png(char *n, int& one, unsigned int& bits, unsigned int& c);
		ReturnValues tga(char *n, int& one, unsigned int& bits, unsigned int& c);
		ReturnValues tiff(char *n, int& one, unsigned int& bits, unsigned int& c);

		int detect(const char *);

	public:
		enum ImageFormats { IMAGE_AUTO, IMAGE_BMP, IMAGE_GIF, IMAGE_TGA,
			IMAGE_PNG, IMAGE_JPG, IMAGE_TIF, IMAGE_PCX, IMAGE_UNKNOWN,
			IMAGE_LAST };
		/**
			Creates an empty bitmap image object.
			You must use LoadImage() to load from file.
		*/

		FGBitmap();
		/**
		Creates "in memory" object from the bitmap file with name.
		The file must be one from supported and enabled image formats.
		If an operation is OK, then data member type is != BMP_NONE,
		else file doesn't exist or isn't a valid image file.
		@deprecated
		*/
		FGBitmap(const char	*name);
		FGBitmap(FGDrawBuffer *);
		FGBitmap(int,int,FGPixel=CBLACK);
		FGBitmap(FGBitmap& old) : FGDrawBuffer(old)
		{
			bmfh = old.bmfh;
			bmi = old.bmi;
			bmih = old.bmih;
			rgb = old.rgb;
			cUsedTable = 0;
#ifdef INDEX_COLORS
			if (old.cUsedTable)
			{
				cUsedTable = new int[256];
				memcpy(cUsedTable, old.cUsedTable, sizeof(int)*256);
			}
#endif
		}
		virtual ~FGBitmap();
        //! @deprecated
		ReturnValues BitmapSave(const char *, bool convert=false);
		ReturnValues JpegSave(const char *name);
		ReturnValues PngSave(const char *name);

		ReturnValues LoadImage(const char* name, int imagefmt = IMAGE_AUTO);
		ReturnValues SaveImage(const char* name, int imagefmt);

};
#ifndef __GNUC__
#pragma	pack()
#endif

#ifdef FG_NAMESPACE
}
#endif

#ifdef __BORLANDC__
#pragma option pop /* pop -a switch */
#pragma option pop /* pop -b */
#endif

#endif

/*
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

    nezmar@atlas.sk

*/

#define GIF_MAXCOLORS	256

#ifdef FG_NAMESPACE
namespace fgl {
#endif

typedef enum {
	gif_image, gif_comment, gif_text
} GIFStreamType;

typedef enum {
	gif_no_disposal = 0, gif_keep_disposal = 1, 
	gif_color_restore = 2, gif_image_restore = 3
} GIFDisposalType;

typedef struct {
	int		transparent;	/* transparency index */
	int		delayTime;	/* Time in 1/100 of a second */
	int		inputFlag;	/* wait for input after display */
	GIFDisposalType	disposal;
} GIF89info;

typedef struct GIFData {
	GIF89info	info;
	int		x, y;
	int		width, height;
	GIFStreamType	type;
	union {
		struct {
			int		cmapSize;
			unsigned char	cmapData[GIF_MAXCOLORS][3];
			unsigned char	*data;
			int		interlaced;
		} image;
		struct {
			int	fg, bg;
			int	cellWidth, cellHeight;
			int	len;
			char	*text;
		} text;
		struct {
			int	len;
			char	*text;
		} comment;
	} data;

	struct GIFData	*next;
} GIFData;

typedef struct {
	int		width, height;

	int		colorResolution;
	int		colorMapSize;
	int		cmapSize;
	unsigned char	cmapData[GIF_MAXCOLORS][3];

	int		background;
	int		aspectRatio;

	GIFData		*data;
} GIFStream;

GIFStream*	GIFRead(char *), *GIFReadFP(FILE *);
 int		GIFTest(char *);
int		GIFWrite(char *, GIFStream *, int);
int		GIFWriteFP(FILE *, GIFStream *, int);

#ifdef FG_NAMESPACE
}
#endif


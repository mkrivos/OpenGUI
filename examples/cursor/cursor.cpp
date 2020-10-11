/*
 *  This example shows using an user defined mouse cursor
 *
*/

#include <fastgl/fastgl.h>


#ifdef INDEX_COLORS
#define O 0xffU
#endif
#ifdef DIRECT_COLORS
#define O 0xFFFFU
#endif
#ifdef TRUE_COLORS
#define O 0xFFFFFFU
#endif
#define I 0x00

static fgl::FGPixel mask[16 * 16] =
{
	O, O, O, O, O, O, O, I, I, O, O, O, O, O, O, O,
	O, O, O, O, O, O, O, I, I, O, O, O, O, O, O, O,
	O, O, O, O, O, O, O, I, I, O, O, O, O, O, O, O,
	O, O, O, O, O, O, O, I, I, O, O, O, O, O, O, O,
	O, O, O, O, O, O, O, I, I, O, O, O, O, O, O, O,
	O, O, O, O, O, O, O, O, O, O, O, O, O, O, O, O,
	O, O, O, O, O, O, O, I, I, O, O, O, O, O, O, O,
	I, I, I, I, I, O, I, O, O, I, O, I, I, I, I, I,
	I, I, I, I, I, O, I, O, O, I, O, I, I, I, I, I,
	O, O, O, O, O, O, O, I, I, O, O, O, O, O, O, O,
	O, O, O, O, O, O, O, O, O, O, O, O, O, O, O, O,
	O, O, O, O, O, O, O, I, I, O, O, O, O, O, O, O,
	O, O, O, O, O, O, O, I, I, O, O, O, O, O, O, O,
	O, O, O, O, O, O, O, I, I, O, O, O, O, O, O, O,
	O, O, O, O, O, O, O, I, I, O, O, O, O, O, O, O,
	O, O, O, O, O, O, O, I, I, O, O, O, O, O, O, O,
};

#undef  I
#undef  O
#define I fgl::CWHITE
#define O 0x00

static fgl::FGPixel bmap[16 * 16] =
{
	O, O, O, O, O, O, O, I, I, O, O, O, O, O, O, O,
	O, O, O, O, O, O, O, I, I, O, O, O, O, O, O, O,
	O, O, O, O, O, O, O, I, I, O, O, O, O, O, O, O,
	O, O, O, O, O, O, O, I, I, O, O, O, O, O, O, O,
	O, O, O, O, O, O, O, I, I, O, O, O, O, O, O, O,
	O, O, O, O, O, O, O, I, I, O, O, O, O, O, O, O,
	O, O, O, O, O, O, O, I, I, O, O, O, O, O, O, O,
	I, I, I, I, I, I, I, O, O, I, I, I, I, I, I, I,
	I, I, I, I, I, I, I, O, O, I, I, I, I, I, I, I,
	O, O, O, O, O, O, O, I, I, O, O, O, O, O, O, O,
	O, O, O, O, O, O, O, I, I, O, O, O, O, O, O, O,
	O, O, O, O, O, O, O, I, I, O, O, O, O, O, O, O,
	O, O, O, O, O, O, O, I, I, O, O, O, O, O, O, O,
	O, O, O, O, O, O, O, I, I, O, O, O, O, O, O, O,
	O, O, O, O, O, O, O, I, I, O, O, O, O, O, O, O,
	O, O, O, O, O, O, O, I, I, O, O, O, O, O, O, O,
};

static fgl::FGMouseCursor IDC_MYCURSOR(bmap, mask, 8, 8, 16, 16);

int main(int argc, char **argv)
{
	fgl::FGApp app(fgl::G640x480,argc,argv,0,fgl::APP_ENABLEALTX);
	app.CursorLoad(&IDC_MYCURSOR);
	app.Run();
	return 0;
}


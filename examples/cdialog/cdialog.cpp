#include <fastgl/fastgl.h>
#include <fastgl/widgets.h>

static void routine(fgl::FGPixel c)
{
	fgl::clear_frame_buffer(c);
}

int main(int argc, char **argv)
{
	fgl::FGApp MyApp(3, argc, argv, fgl::CBLACK);
	fgl::FGColorDialog::Import("palette.dat");
	fgl::FGColorDialog * cd = new fgl::FGColorDialog("Select your color", routine);
	MyApp.Run(0);
	fgl::FGColorDialog::Export("palette.dat");
	delete cd;
	return 0;
}

#include <fastgl/fastgl.h>
#include <fastgl/widgets.h>

static void routine(int fnt)
{
	fgl::FGApp::GetRootWindow()->set_font(fnt);
	fgl::FGApp::GetRootWindow()->WindowText(100,100,"A lazzy dog jumps a brown fox!");
}

int main(int argc, char **argv)
{
	fgl::FGApp MyApp(3, argc, argv, fgl::CBLACK, fgl::APP_ENABLEALTX | fgl::APP_ROOTWINDOW);
	fgl::FGFontDialog * cd = new fgl::FGFontDialog(routine);
	MyApp.Run(0);
	delete cd;
	return 0;
}

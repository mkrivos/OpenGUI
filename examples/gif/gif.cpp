#include <fastgl/fastgl.h>

int main(int argc, char **argv)
{
	fgl::FGApp MyApp(3, argc, argv,fgl:: CDARK,fgl::APP_ROOTWINDOW|fgl::APP_ENABLEALTX);
	fgl::FGBitmap icon("../data/logo256.gif");
	fgl::FGApp::GetRootWindow()->WindowPutBitmap(0, 0, 0,0,fgl::GetXRes(), fgl::GetYRes(),&icon);
	MyApp.Run(0);
	return 0;
}

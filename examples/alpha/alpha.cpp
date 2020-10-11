/*
	TEST FOR TRANSPARENCY & ALPHA BLENDING
*/

#include <fastgl/fastgl.h>

int main(int argc, char **argv)
{
	fgl::FGApp MyApp(3, argc, argv, fgl::CBLACK, fgl::APP_ROOTWINDOW | fgl::APP_ENABLEALTX);
	fgl::FGBitmap icon("../data/logo256.gif");
	fgl::FGBitmap image("../data/texture.bmp");

	fgl::FGDrawBuffer modra(300,300,fgl::FGDrawBuffer::BMP_MEM, CBLUE);
	modra.SetAlpha(109);

	fgl::FGApp::GetRootWindow()->WindowPutBitmap(0, 0, 0,0,fgl::GetXRes(), fgl::GetYRes(),&icon);
	fgl::FGApp::GetRootWindow()->WindowPutBitmap(16, 40, 0,0,fgl::GetXRes(), fgl::GetYRes(),&icon);

	image.SetAlpha(128); // 50% transparency
	fgl::FGApp::GetRootWindow()->WindowPutBitmap(300, 200, 0,0, 200, 160, &image);
	image.SetAlpha(80); // 30% transparency
	fgl::FGApp::GetRootWindow()->WindowPutBitmap(400, 300, 0,0, 200, 160, &image);
	image.SetAlpha(200); // 80% transparency
	fgl::FGApp::GetRootWindow()->WindowPutBitmap(500, 400, 0,0, 200, 160, &image);

	fgl::FGApp::GetRootWindow()->WindowPutBitmap(300, 200, 0,0, 300, 300, &modra);

#if FASTGL_BPP == 8
	fgl::IError("Alpha blending runs with BPP>=15 only!", 0);
#endif
	MyApp.Run(0);
	return 0;
}

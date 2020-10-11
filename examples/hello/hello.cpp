#include <fastgl/fastgl.h>

int main(int argc, char **argv)
{
	fgl::FGApp MyApp(fgl::G640x480, argc, argv, fgl::CDARK, fgl::APP_ALL);
	fgl::FGWindow *okno = new fgl::FGWindow(0, fgl::GetXRes() / 2 - 200, fgl::GetYRes() / 2 - 100, 400, 200, " Your Pretty Window", 0, fgl::CBLACK, fgl::CGRAY2, fgl::WFRAMED | fgl::WTITLED | fgl::WMINIMIZE);


	// icon image will be released automatically on Window destroy ...
	fgl::FGBitmap* bmp = new fgl::FGBitmap("../data/opengl.bmp");
	fgl::FGBitmap* texture = new fgl::FGBitmap("../data/texture.bmp");
	okno->WindowAttachIcon(bmp);
	okno->printf(54, 75, "Hello from FastGL world !\n");
	fgl::FGPushButton* button = okno->AddPushButton(275, 120, 80, 26, "Quit", 'Q', texture, fgl::Control::Quit);

        okno->arc(100,100, 40*(M_PI/180.), 30*(M_PI/180.), 100, CYELLOW);
	MyApp.Run(0);

	delete okno;
	return 0;
}

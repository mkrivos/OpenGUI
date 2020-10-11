/*
 *  This example shows PAGE FLIPPING feature of OpenGUI library
 *
 *	requirement: 1MB videocards (2MB for 16bit colors or 4MB for true colors)
 *	FPS (frames per secs) is based on monitor frequency & CPU speed heavily
*/

#include <fastgl/fastgl.h>


void animate(void)
{
	static int i=0,j=200,l=2000;
	fgl::clear_frame_buffer(fgl::CWHITE);
	fgl::fill_ellipse(fgl::GetXRes()/2, fgl::GetYRes()/2,i,j, fgl::CBLACK, fgl::_GSET);
	fgl::fill_ellipse(fgl::GetXRes()/2, fgl::GetYRes()/2,j,i, fgl::CBLACK, fgl::_GSET);
	i++;
	if (i==200) i=0;
	j--;
	if (j==0) j=200;
	fgl::cApp->Flip();
	if (l-- == 0) fgl::FGApp::AppDone(); // return after 2000 frames
}

int main(int argc, char **argv)
{
	fgl::FGApp app(fgl::G640x480, argc, argv, fgl::CBLACK, fgl::APP_ENABLEALTX);

	// you can use FG_DOUBLEBUFFER or FG_QUADBUFFER
	// NOTE: DOUBLEBUFFER is flicker on some cards & drivers (nVIDIA)
	app.EnableBuffering(fgl::FG_TRIPLEBUFFER);

	// set main loop code
	app.SetDelayProc(animate);

	// go!
	app.Run();

	app.DisableBuffering();
	return 0;
}


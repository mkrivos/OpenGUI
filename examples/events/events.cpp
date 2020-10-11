#include <fastgl/fastgl.h>

fgl::Window *MyWnd;

int AppProc(fgl::FGEvent *e)
{
	fgl::gprintf(fgl::CWHITE, fgl::CDARK,16, 580, "APP_PROC: %s[%d] (%d,%d %d)      ", e->GetName(), e->GetType(),e->GetKey(),e->GetX(),e->GetY());
	return 0;
}

void WindowProc(fgl::FGEvent *e)
{
	e->wnd->printf("WND_PROC: %s[%d] (%d,%d %d)\n", e->GetName(), e->GetType(),e->GetKey(),e->GetX(),e->GetY());
}

int main(int argc, char **argv)
{
	fgl::FGApp MyApp(3, argc, argv, fgl::CDARK, fgl::APP_ENABLEALTX | fgl::APP_MAGNIFIER | fgl::APP_ROOTWINDOW);
	fgl::FGWindow * okno = new fgl::FGWindow(&MyWnd, 400, 100, 400, 500, "Window", WindowProc, fgl::CBLACK, fgl::CGRAY2, fgl::WFRAMED | fgl::WTITLED);
	new fgl::FGWindow(0, 0, 0, 300, 400, "Window 2", WindowProc, 0, fgl::CGRAY2, fgl::WFRAMED | fgl::WTITLED | fgl::WNOPICTO | fgl::WSIZEABLE);

	okno->printf("TEST FOR EVENTS !\n");
	MyApp.Run(AppProc);
	delete MyWnd;
	return 0;
}

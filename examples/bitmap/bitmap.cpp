#include <fastgl/fastgl.h>


fgl::FGBitmap *b;

void exits(fgl::Control *)
{
    fgl::FGApp::AppDone();
}

void proc(fgl::FGEvent *p)
{
	switch(p->GetType())
	{
		case fgl::REPAINTEVENT:
		case fgl::INITEVENT:
			p->wnd->WindowPutBitmap(0, 0, 0, 0, p->wnd->GetWW(), p->wnd->GetHW(), b);
			p->wnd->set_fcolor(fgl::CBLUE);
			p->wnd->set_ppop(fgl::_GCOLORKEY);
#ifdef INDEX_COLORS
			p->wnd->printf(20, 20, " Used colors %d+16=%d \n", 0xf0 - GetFreeColors(), 0xf0 - GetFreeColors() + 16);
#endif
			p->wnd->printf(20, 40, "  You can minimize this window! \n");
			p->wnd->printf(20, 60, "and restore with right button again!\n");
			p->wnd->set_ppop(fgl::_GSET);
			break;
	}
}

int main(int argc, char **argv)
{
	fgl::FGApp MyApp(2, argc, argv, 0, fgl::APP_ENABLEALTX | fgl::APP_ROOTWINDOW);
	fgl::SetColorFuzzy(2);			// how different colors you get
	fgl::FGWindow *root = MyApp.GetRootWindow();

  	b = new fgl::FGBitmap("../data/texture.bmp");	// create from file
  	fgl::FGBitmap c("../data/new.bmp");
  	fgl::FGBitmap icon("../data/icon.bmp");
	const int h = (fgl::GetYRes() / 100) * 40, w = (fgl::GetXRes() / 100) * 40;

	// draw background  (size from header, position 0,0)
	root->WindowPutBitmap(0,0,0,0,fgl::GetXRes(),fgl::GetYRes(), b);				

	root->WindowPutBitmap(200, 150, 0,0,400, 300, &c); // draw bitmap rectangle

	// create a window

	fgl::FGWindow *okno1 = new fgl::Window(0, 20, 20, 360, 180, "The Window", proc, fgl::CBLACK, fgl::CWHITED,fgl::WFRAMED | fgl::WTITLED | fgl::WUSELAST | fgl::WSIZEABLE | fgl::WMINIMIZE);
	okno1->WindowAttachIcon(&icon);

	// add push button for exit

	okno1->AddPushButton(w - 100, h - 60, 80, 24, "The End", 'E', exits);

	// print a message
	// save okno1 to bitmap file !!!
	fgl::FGBitmap d(okno1);
	d.BitmapSave("window.bmp");

	// run while button pushed or ALT+X pressed
	MyApp.Run(0);
	delete b;
	delete okno1;
	return 0;
}

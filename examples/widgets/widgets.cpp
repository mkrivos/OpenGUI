#include <fastgl/fastgl.h>
#include <fastgl/widgets.h>

static fgl::FGProgressBar *pg;
static fgl::FGWindow *w; 

void select_file(char *s, fgl::FGFileDialog *)
{
	w->SetName(s);
}

void timer(int sec)
{
	pg->setProgress(sec);
}

int main(int argc, char **argv)
{
	
	fgl::FGApp app(3,argc, argv, 0, fgl::APP_ENABLEALTX);

	new fgl::FGWindow(&w,140,340,400,130,"The Window",0,0, fgl::CWHITED, fgl::WNOPICTO | fgl::WFRAMED | fgl::WTITLED);
	pg = new fgl::FGProgressBar(w,20,20,352,30, 60);

	fgl::FGFileDialog *a = new fgl::FGFileDialog(select_file, ".", ".cc .o .gpr","Files in this dir", fgl::FDIALOG_SAVE);
	
	app.SetTimerProc( timer );
	app.Run();

	delete w;
	return 0;
}

#include <fastgl/fastgl.h>
#include <fastgl/listbox.h>

fgl::FGListBox *a;

void proc(fgl::FGEvent *p)
{
	int rc;
	switch(p->GetType())
	{
		case fgl::KEYEVENT:
			// next line is equivalent of rest of this case
//			a->DoListBox(p->GetKey());

			switch(p->GetKey())
			{
			case KUP:
				a->Up();
				break;
			case KDOWN:
				a->Down();
				break;
			case HOME:
				a->Begin();
				break;
			case END:
				a->End();
				break;
			}
			break;
		case fgl::CLICKLEFTEVENT:
			rc = a->Test(p->GetX(), p->GetY());
			if (rc>=0) a->SetIndex(rc);
			p->wnd->SetColors(); // set apropriate colors
			p->wnd->printf(8,8,"You clicked to item at index %d", rc);
			break;
		case fgl::TERMINATEEVENT:
			// quit app
			fgl::FGApp::AppDone();
			break;
	}
}

static void reload(fgl::FGListBox * a)
{
	// at the start is FGListBox empty, but else
	// you must first of all reset object to a zero state
	a->clear();
	// insert a ten string items
	// for a delete an items from the list see to 'listbox.h' file
	a->insert("item 1");
	a->insert("item 2");
	a->insert("item 3");
	a->insert("item 4");
	a->insert("item 5");
	a->insert("item 6");
	a->insert("item 7");
	a->insert("item 8");
	a->insert("item 9");
	a->insert("item 10");
	// enable update & redraw object
	a->Update();
}

int main(int argc, char **argv )
{
	fgl::FGApp MyApp(3,argc,argv,fgl::CGRAY1,fgl::APP_ENABLEALTX);
	// create a Window object
	fgl::FGWindow *w = new fgl::FGWindow(&w,100,1000,300,250, "Okno",proc,0,fgl::CWHITE,fgl::WCLICKABLE|fgl::WTITLED|fgl::WFRAMED|fgl::WCENTRED);

	// create an emty FGListBox object
	a=new fgl::FGListBox(40,40,200,18,5,w);

	// load object wiht some items
	reload(a);

	// run program
	MyApp.Run(0);
	return 0;
}

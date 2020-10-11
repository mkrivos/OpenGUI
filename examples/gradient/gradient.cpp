#include <fastgl/fastgl.h>
#include <fastgl/widgets.h>
#include <fastgl/fggradient.h>

static fgl::FGWindow *Example_for_FGUpDown_classPtr;
static fgl::FGPoint start(0,0);
static fgl::FGPoint end(400,300);
static fgl::FGRect rect(0,0,800,600);
static fgl::FGGradient* gradient;
static fgl::FGColor color0("black");
static fgl::FGColor color1(fgl::FGColor::blue);
static fgl::FGColor color2(fgl::FGColor::white);
static bool tristate = true;
int function = '1';

static void Function(int f)
{
	switch(f)
	{
				case '1':
					gradient->SetFunction(FGGradient::Linear);
					break;
				case '2':
					gradient->SetFunction(FGGradient::LinearXY);
					break;
				case '3':
					gradient->SetFunction(FGGradient::Radial);
					break;
				case '4':
					gradient->SetFunction(FGGradient::Sqrt);
					break;
				case '5':
					gradient->SetFunction(FGGradient::Conic);
					break;
				case '6':
					gradient->SetFunction(FGGradient::Diamond);
					break;
	}
}

static void Wrapping(int f)
{
	switch(f)
	{
				case F01:
					gradient->SetMode(FGGradient::Wrap);
					break;
				case F02:
					gradient->SetMode(FGGradient::Repeat);
					break;
				case F03:
					gradient->SetMode(FGGradient::Reflect);
					break;
	}
}

static void Example_for_FGUpDown_classProc(fgl::FGEvent *p)
{
	fgl::FGColorPicker* cp;

	switch(p->GetType())
	{
		case fgl::INITEVENT:
		case fgl::REPAINTEVENT:
			gradient->Draw(*p->wnd, start, end, &rect);
			p->wnd->fcircle(start.x, start.y, 8, CYELLOW);
			p->wnd->circle(start.x, start.y, 8, CBLUE);
			p->wnd->fcircle(end.x, end.y, 8, CYELLOW);
			p->wnd->circle(end.x, end.y, 8, CBLUE);
			p->wnd->printf(600,10," 1 - Linear gradient   ");
			p->wnd->printf(600,30," 2 - LinearXY gradient ");
			p->wnd->printf(600,50," 3 - Radial gradient   ");
			p->wnd->printf(600,70," 4 - Sqrt gradient     ");
			p->wnd->printf(600,90," 5 - Conic gradient    ");
			p->wnd->printf(600,110," 6 - Diamond gradient  ");
			p->wnd->printf(600,130," 0 - switch 2/3 colors ");
			p->wnd->printf(600,150," A - change color 0    ");
			p->wnd->printf(600,170," B - change color 1    ");
			p->wnd->printf(600,190," C - change color 2    ");
			p->wnd->printf(600,210," F1 - WRAP             ");
			p->wnd->printf(600,230," F2 - REPEAT           ");
			p->wnd->printf(600,250," F3 - REFLECT          ");

			p->wnd->printf(600,270," LEFT button start pt. ");
			p->wnd->printf(600,290," RIGHT button end pt.  ");
			p->wnd->WindowRepaint(0,0,800,600);
			break;
		case fgl::TERMINATEEVENT:
			fgl::FGApp::AppDone();
			break;
		case fgl::CLICKLEFTEVENT:
			start = fgl::FGPoint(p->GetX(), p->GetY());
			p->wnd->SendEvent(fgl::REPAINTEVENT);
			break;
		case fgl::CLICKRIGHTEVENT:
			end = fgl::FGPoint(p->GetX(), p->GetY());
			p->wnd->SendEvent(fgl::REPAINTEVENT);
			break;
		case fgl::KEYEVENT:
			switch(p->GetKey())
			{
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
					function = p->GetKey();
					Function(function);
					break;

				case F01:
				case F02:
				case F03:
					function = p->GetKey();
					Wrapping(function);
					break;

				case '0':
					tristate = !tristate;
newgr:
					delete gradient;
					if (tristate)
						gradient = new fgl::FGGradient(color0,color1,color2);
					else
						gradient = new fgl::FGGradient(color0, color2);
					Function(function);
					break;
				case 'a':
				case 'A':
					cp = new fgl::FGColorPicker(&color0);
					delete cp;
					goto newgr;
				case 'b':
				case 'B':
					cp = new fgl::FGColorPicker(&color1);
					delete cp;
					goto newgr;
				case 'c':
				case 'C':
					cp = new fgl::FGColorPicker(&color2);
					delete cp;
					goto newgr;
			}
			p->wnd->SendEvent(fgl::REPAINTEVENT);
			break;
	}
}

int main(int argc, char **argv)
{
	fgl::FGApp MyApp(fgl::G800x600, argc, argv, fgl::CBLACK, fgl::APP_ENABLEALTX + fgl::APP_CFG);

	gradient = new fgl::FGGradient(color0,color1,color2);
	Function(function);
	gradient->SetMode(FGGradient::Repeat);

	Example_for_FGUpDown_classPtr = new fgl::FGWindow(&Example_for_FGUpDown_classPtr, 0, 0, 800, 600, "Example of color gradient", Example_for_FGUpDown_classProc,
		 fgl::CWHITED, fgl::CBLACK, fgl::WCENTRED);

	MyApp.Run();
	if (Example_for_FGUpDown_classPtr) delete Example_for_FGUpDown_classPtr;
	return 0;
}



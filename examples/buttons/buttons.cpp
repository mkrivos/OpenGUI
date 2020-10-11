#include <fastgl/fastgl.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

fgl::FGWindow *w5,*w4;
int number=0, sl=95;

fgl::FGButtonGroup *grp;

int AppProc(fgl::FGEvent *)  // if return 0 = exit
{
	return 1;
}

void myexit(fgl::FGControl *)
{
	fgl::FGApp::AppDone();
}

void number_handler(fgl::FGControl *)
{
	w5->printf(20, 160,"Value of number is %d\n", number);
}

void SliderFnc(fgl::FGControl *)
{
	w5->printf(20, 175,"Value of slider number is %4d\n", sl);
}

extern char mysto1[];  // bitmap in exe

int main(int argc, char **argv )
{
	fgl::FGApp MyApp(3,argc,argv,fgl::CDARK,fgl::APP_ENABLEALTX);
	int i,j=7,h=(fgl::GetYRes()/100)*40,w=(fgl::GetXRes()/100)*40;
	double jjj=.88;
	char *jj=strdup("majo      ");
	fgl::FGControl *eb;

	w4 = new fgl::FGWindow(&w4, 1000,1000,w,h,"Wnd 1", 0, fgl::CYELLOW, fgl::CBLUE, fgl::WFRAMED|fgl::WSIZEABLE|fgl::WNOPICTO);
	w5 = new fgl::FGWindow(&w5,   150,100,w,h+10,"Wnd 2", 0,fgl::CYELLOW,fgl::CGRAY2,fgl::WSTANDARD|fgl::WSIZEABLE|fgl::WUSELAST|fgl::WMENU);

	eb=w4->AddEditBox(20, 40, 70, 60, "Input :", 'I', &j,0,5,10);
	((fgl::FGEditBox *)eb)->ChangeItem(&i);

	w4->AddEditBox(20, 70, 70, 60, "Input :", 'N', jj,0);
	w4->AddEditBox(20,100, 70, 60, "Input :", 'P', &jjj,0, .5, 1.5);

	grp = new fgl::FGButtonGroup;
	int nn=0;
	w4->WindowText(100,132,"Button Group");
	for(i=1;i<=3;i++) for(j=1;j<=3;j++)
	{
		char ss[2]=" ";
		ss[0] = nn+'0';
		grp->AddToGroup(w4->AddPointButton(110+i*40, 124+j*24, ss),nn==0);
		nn++;
	}

	w5->WindowText(20,113,"Horizontal Slider",0);
	w5->AddSlideBarH(20,130, -100,80,1,&sl, SliderFnc);

	eb = w5->AddPushButton(20,10,190,24,"Start Test for Base",'B',0);
	eb->Disable();

	w5->AddPushButton(20,40,190,24,"Start Test for Window",'W',0);

	eb = w5->AddCheckButton(20,90,"Check Button",'C');

	w5->AddPointButton(170,90,"Point Button",'P', &number, number_handler);

	eb=w5->AddPushButton(w-70,10,60,24,"Exit",'E',myexit);

	eb->SetName("Quit");
	eb->SetKey('Q');

	w5->AddBaseMenu("File ",'F', 0);
	w5->AddBaseMenu("Edit ",'E', 0);
	eb=w5->AddBaseMenu("Help ",'H', 0);

	fgl::FGMenuWindow *m=new fgl::MenuWindow(10,10,100,100);
	m->AddMenu("first");
	m->AddMenu("second");
	m->AddMenu("third");

	MyApp.Run(AppProc);

	if (w4) delete w4;
	if (w5) delete w5;

	return 0;
}

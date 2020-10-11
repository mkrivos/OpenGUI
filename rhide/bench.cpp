/*
	OpenGUI - Drawing & Windowing library

	Copyright (C) 1996,2004  Marian Krivos

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Library General Public
	License as published by the Free Software Foundation; either
	version 2 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Library General Public License for more details.

	You should have received a copy of the GNU Library General Public
	License along with this library; if not, write to the Free
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

	nezmar@atlas.sk
*/

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <stdarg.h>

#include "fastgl.h"
#include "widgets.h"
#include "_fastgl.h"
#include "drivers.h"
#include "fggradient.h"

#include "fglistbox.h"

//#ifdef __REGRESS__

#ifdef FG_NAMESPACE
using namespace fgl;
#endif

void cb(CallBack)
{
}

int proc(FGEvent* e)
{
	switch(e->GetType())
	{
		case KEYEVENT:
			break;
		case CLICKLEFTEVENT:
			printf("Click left\n");
			break;
		case DBLCLICKLEFTEVENT:
			printf("Dbl Click left\n");
			break;
	}
	return 0;
}

void a(FGEvent* e)
{
	if (e->GetType() == CLICKLEFTEVENT)
	{
		printf("Click W left\n");
		Puk();
	}
}

int main(int argc, char **argv)
{
	FGApp MyApp(4,argc,argv,0,APP_ENABLEALTX | APP_CFG);

	FGFontProperty *arial23 = new FGFontProperty("meteo.ttf",20);
	arial23->Shaded();
	FGBitmap bmp;
	bmp.LoadImage("test.png");

	FGWindow* w0 = new FGWindow(&w0,100,100,600,400,"Back", 0, 0, CDARK, WESCAPE|WBACK|WTITLED|WFRAMED);
	FGWindow* w1 = new FGWindow(&w1,1000,1000,400,300,"Jano", a, 0, CWHITE, WTITLED|WNOTEBOOK|WUSESELECTEDCONTROL|WMENU|WESCAPE);
	FGWindow* w2 = new FGWindow(&w2,100,100,400,300,"Fero", 0, CWHITE, 0, WSTANDARD|WESCAPE);
	FGWindow* w3 = new FGWindow(&w3,0,0,600,100,"Top", 0, 0, CRED, WESCAPE|WTOP|WTITLED);

	w1->AddBaseMenu("File ",'F', 0);
	w1->AddBaseMenu("Edit ",'E', 0);
	w1->AddBaseMenu("Help ",'H', 0);

	w1->AddTabPage(" 1 ");
	FGControl *c = w1->AddPushButton(100,40,80,23,"button", 'b', cb);
	w1->AddPanel(0,0,1000,1000,"dd");
	w1->AddTabPage(" 2 ");
	c = w1->AddPushButton(200,40,80,23,"button2", 'b', cb);
	w1->AddTabPage(" 3 ");
	c = w1->AddPushButton(300,40,80,23,"button3", 'b', cb);
//	w->SetDefaultControl(c);
	int param=12345678;
	w1->AddEditBox(100, 80, 80,80,"Editbox", 0, &param);
	w1->SetTabPage(" 1 ");

	FGMenuWindow *m = new MenuWindow(100,100,100,100);
	(m->AddMenu("first y", 'y'))->Disable();
	m->AddMenu("second gj", 'g');
	m->AddMenu("third");

	FGPoint points[4];
	points[0] = FGPoint(0,0);
	points[1] = FGPoint(50,100);
	points[2] = FGPoint(100,50);
	points[3] = FGPoint(200,100);

	w1->WindowSpline(points, CRED);

	FGPoint aa(200,150);
	FGPoint bb(240,190);

	FGColor sc(FGColor::blue);
	FGColor sm(FGColor::yellow);
	FGColor se(FGColor::white);
	FGGradient grad(sc, se);
//	FGRect rr(0,0,400,300);
	grad.SetFunction(FGGradient::Diamond);
	grad.Draw(*w1/*, aa, bb*/);
	w1->WindowRepaint(0,0,400,300);

	FGListBoxEx *lb = new FGListBoxEx(0, 0, 200, 20, 6, w1, 0, (void *)0, (XUIEnterHandlerInteger)0 );
	lb->insert("a 1");
	lb->insert("a 3");
	lb->insert("a 4");
	lb->Update();
w0->bitblit(0,0,0,0,250,250, &bmp);

for(int i=0;i<16;i++)
{
	char str[32];
	for(int j=0;j<16;j++)
	{
		str[j] = j+i*16+255;
	}
	str[16]=0;
	strcpy(str, "012345ABCDEFa");
	w0->WindowText(0,63*i,str, arial23, CWHITE, 0);
}
	MyApp.Run(proc);


	delete w1;
	return 0;
}

//#endif




/*
    OpenGUI - Drawing & Windowing library

    Copyright (C) 1996,2000  Marian Krivos

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

    nezmar@internet.alcatel.sk
*/

#include <fastgl/fastgl.h>


class TForm : public fgl::FGApp
{
		void OnKeyPress(int key)
		{
			fgl::gprintf(0,fgl::CWHITE,100,100,"KEYEVENT %4d            ",key);
		}
		void OnMouseMove(int x, int y)
		{
			fgl::gprintf(0,fgl::CWHITE,100,100,"MOVEEVENT %4d %4d",x,y);
		}
		void OnClick(int x, int y)
		{
			fgl::gprintf(0,fgl::CWHITE,100,100,"CLICKLEFTEVENT %4d %4d",x,y);
		}
		void OnContextPopup(int x, int y)
		{
			fgl::gprintf(0,fgl::CWHITE,100,100,"CLICKRIGHTEVENT %4d %4d",x,y);
		}
		void OnClose(void)
		{
			fgl::gprintf(0,fgl::CWHITE,100,100,"QUITEVENT");
		}
		void OnPaint(void)
		{
			fgl::gprintf(0,fgl::CWHITE,100,100,"PAINTEVENT");
		}
		void OnInit(void)
		{
			fgl::gprintf(0,fgl::CWHITE,100,100,"INITEVENT");
		}
		void OnCursorOut(int smer)
		{
			fgl::gprintf(0,fgl::CWHITE,100,100,"CURSOROUTEVENT %d", smer);
		}
		void OnStartDrag(int smer, int x, int y)
		{
			fgl::gprintf(0,fgl::CWHITE,100,100,"DRAGEVENT %d - [%04d:%04d]", smer,x,y);
		}
		void OnEndDrag(int smer, int x, int y, int w, int h)
		{
			fgl::gprintf(0,fgl::CWHITE,100,100,"ENDDRAGEVENT %d - [%04d:%04d-%04d:%04d]", smer,x,y,w,h);
		}
	public:
		TForm(int argc, char **argv)
			: FGApp(3,argc,argv,0,fgl::APP_ALL)
		{
		}
};

class TWindow : public fgl::FGWindow
{
	public:
		TWindow(TWindow **self) : FGWindow((FGWindow **)self,200,300,200,120,"Okno")
		{
		}
		virtual void OnMouseMove(int,int)
		{
			fgl::Puk();
		}
		virtual void OnClose(void)
		{
			fgl::FGApp::AppDone();
		}
};

int main(int argc, char **argv)
{
	TForm MyApp(argc, argv);
	TWindow *MyWnd = new TWindow(&MyWnd);
	MyApp.Run();

	if (MyWnd) 
	    delete MyWnd;
	return 0;
}


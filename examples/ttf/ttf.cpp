/*
	OpenGUI - Drawing & Windowing library

	Copyright (C) 1996,2003  Marian Krivos

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

#include <fastgl/fastgl.h>
#include <fastgl/widgets.h>

#pragma hdrstop

//---------------------------------------------------------------------------

#pragma argsused
int main(int argc, char* argv[])
{
	fgl::FGApp *MyApp = new fgl::FGApp(3,argc,argv,fgl::CWHITED,fgl::APP_ALL);
	fgl::FGWindow *wnd = new fgl::FGWindow(&wnd, fgl::GetXRes()-440, fgl::GetYRes()-340, 400, 220, "Main Window", 0, fgl::CBLUELIGHT, fgl::CWHITE,
		fgl::WSIZEABLE | fgl::WTITLED | fgl::WMINIMIZE|fgl::WFRAMED);

	unsigned short uu[16] = {'h', 'e', 'l', 'l', 'o', ' ', 'w','o','r','l','d','!', 0};

	fgl::FGFontProperty *arial23 = new fgl::FGFontProperty("../data/arir____.ttf", 23);
	fgl::FGFontProperty *helvetica23 = new fgl::FGFontProperty("../data/helr____.ttf", 23);

	wnd->WindowTextUnicode(30,30,uu, arial23, 0, fgl::CWHITE);
	wnd->WindowTextUnicode(30,60,uu, helvetica23, 0, fgl::CWHITE);

	arial23->Shaded();
	wnd->WindowTextUnicode(30,120,uu, arial23, fgl::CBLUELIGHT, fgl::CWHITE);
	helvetica23->Shaded();
	wnd->WindowTextUnicode(30,150,uu, helvetica23, fgl::CBLUELIGHT, fgl::CWHITE);

	fgl::FGFontProperty *arial12 = new fgl::FGFontProperty("../data/arir____.ttf", 12);
	fgl::FGFontProperty *helvetica12 = new fgl::FGFontProperty("../data/helr____.ttf", 24);

	arial12->Solid();
	wnd->WindowTextUnicode(230,90,uu, arial12, fgl::CRED, fgl::CWHITE);
	wnd->WindowTextUnicode(230,30,uu, helvetica12, fgl::CRED, fgl::CWHITE);

	arial12->Shaded();
	wnd->WindowTextUnicode(230,120,uu, arial12, fgl::CBLUELIGHT, fgl::CWHITE);
	helvetica12->Shaded();
	wnd->WindowTextUnicode(230,150,uu, helvetica12, fgl::CBLUELIGHT, fgl::CWHITE);

	MyApp->Run();

	delete wnd;
	delete arial23;
	delete helvetica23;
	delete arial12;
	delete helvetica12;
	delete MyApp;
	
	return 0;
}

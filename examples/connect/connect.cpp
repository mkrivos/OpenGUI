/*
    OpenGUI - Drawing & Windowing library

    Copyright (C) 1996,2002  Marian Krivos

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

/*

*/
class foo : public fgl::FGConnector
{
		void* a;
	public:
		foo() { a = 0; }

		// this is method that is called on the 'Control' activate
		virtual void OnSignal(FGConnector *sender, void* val)
		{
			val = a;
			new fgl::FGWindow(0,rand()%700, rand()%500,90,60,"Event!");
			fgl::Snd(700, 200);
			// this send 'SIGNAL' back to the button -> no action
			RunSignal();
		}
};

class A : public fgl::FGConnector
{
	public:

		virtual void OnSignal(FGConnector *sender, void* val)
		{
			fgl::gprintf(0, fgl::CWHITE,10,40,"Get a signal '%s' from %08x", (char *)val, sender);
		}
};

class B : public fgl::FGConnector
{
	public:

		virtual void OnSignal(FGConnector *sender, void* val)
		{
			fgl::gprintf(0, fgl::CWHITE,10,10,"Get a signal '%s' from %08x", (char *)val, sender);
		}
};

int main(int argc, char **argv)
{
	fgl::FGApp MyApp(3,argc,argv,fgl::CWHITE,fgl::APP_ALL);

	fgl::FGWindow *MyWnd = new fgl::FGWindow(0,300,300,200,120,"Click to button");

	foo *x = new foo;

	// all the Controls contains 'FGConnector' class, i.e. this class
	// is it's parents.

	fgl::FGControl *eb = MyWnd->AddPushButton(68, 40, 90, 24, "New window");
	// connect PushButton with instance (*x) of the 'foo' class and set optional
	// parameter to the '1'.
	FGCONNECT(eb, x, (void *)1);
	// this is also possible ...
	FGCONNECT(x, eb, (void *)2);

	A a;
	B b;

	FGCONNECT(&a, &b, "message A");
	a.RunSignal();

	// back direct
	FGCONNECT(&b, &a, "message B");
	b.RunSignal();

	MyApp.Run();

	return 0;
}


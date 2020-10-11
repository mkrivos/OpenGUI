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
#include <fastgl/widgets.h>

#include "12x24.cpp"

unsigned char font08x16[4096];

static void routine(int fnt)
{
}


int main(int argc, char **argv)
{
	fgl::FGApp myapp(3,argc,argv,0,fgl::APP_ALL);
	fgl::FGWindow *root = fgl::FGApp::GetRootWindow();

	// font from file
	FILE *fp = fopen("../data/8x16.bin", "rb");
	if (fp)
	{
		fread(font08x16, 4096, 1, fp);
		fclose(fp);
	}

	// in-image font
	int fnt1 = fgl::FGFontManager::register_font_fix(
	   (unsigned char*)_12x24,
	   12, 		// width
	   24,		// height
	   256, 	// number of chars per font
	   0,		// characters start at positions
	   "fixed-12x24");

	int fnt2 = fgl::FGFontManager::register_font_fix(
	   font08x16,
	   8,
	   16,
	   256,
	   0,
	   "fix-08x16");

	int fnt4 = fgl::FGFontManager::register_font_ttf("../data/helr____.ttf", 23);
	
	root->set_font(fnt1);
	root->printf(80, 50, "This is a new static linked font!");

	root->set_font(fnt2);
	root->printf(80, 80, "This is a new dynamic loaded font!");

	root->SetInk(fgl::CYELLOW);
	root->set_font(fgl::FONT0406);
	root->printf(80, 130, "Font 0 - 01234 !@#$%qwertQWERTY }{");

	root->set_font(fgl::FONT0808);
	root->printf(80, 150, "Font 1 - 01234 !@#$%qwertQWERTY }{");

	root->set_font(fgl::FONT0816);
	root->printf(80, 180, "Font 2 - 01234 !@#$%qwertQWERTY }{");

	root->set_font(3);
	root->printf(80, 210, "Font 3 - 01234 !@#$%qwertQWERTY }{");

	root->set_font(4);
	root->printf(80, 240, "Font 4 - 01234 !@#$%qwertQWERTY }{");

	root->set_font(5);
	root->printf(80, 270, "Font 5 - 01234 !@#$%qwertQWERTY }{");

	root->set_font(fgl::FONTSYS);
	root->printf(80, 310, "Font 6 - 01234 !@#$%qwertQWERTY }{");

	root->set_font(fgl::FONTSYSLIGHT);
	root->printf(80, 330, "Font 7 - 01234 !@#$%qwertQWERTY }{");

	root->set_font(fnt4);
	root->printf(80, 360, "Font 8 - 01234 !@#$%qwertQWERTY }{");

	root->set_font(fgl::FONT0808);
	root->printf(280, 560, "press <ALT+X> to return");

	// show fonts
	fgl::FGFontDialog * cd = new fgl::FGFontDialog(routine);
	myapp.Run();
	return 0;
}


/*
	Shows using of FGColor class
*/

#include <fastgl/fastgl.h>
#include <fastgl/widgets.h>

void DrawGradient(fgl::FGWindow& dc, int x, int y,
				 fgl::FGColor c1, fgl::FGColor c2, int width, int height)
{
  assert(width > 0);

  float dh = (c2.GetHue() - c1.GetHue()) / width;
  float dl = (c2.GetLuminance() - c1.GetLuminance()) / width;
  float ds = (c2.GetSaturation() - c1.GetSaturation()) / width;

  for (int i = 0; i < width; ++i)
  {
	dc.WindowLine(x + i, y,x + i, y + height, c1);
	c1.SetHue(c1.GetHue() + dh);
	c1.SetLuminance(c1.GetLuminance() + dl);
	c1.SetSaturation(c1.GetSaturation() + ds);
  }
}

void DrawGradients(void)
{
	fgl::FGColor Black;
	fgl::FGColor Gray25 = fgl::FGColor(64,64,64);
	fgl::FGColor Gray50 = fgl::FGColor(128,128,128);
	fgl::FGColor Gray75 = fgl::FGColor(192,192,192);
	fgl::FGColor White = fgl::FGColor::white;

	fill_box( 50,  50, 100,100, Black, fgl::_GSET);
	fill_box( 150, 50, 100,100, Gray25, fgl::_GSET);
	fill_box( 250, 50, 100,100, Gray50, fgl::_GSET);
	fill_box( 350, 50, 100,100, Gray75, fgl::_GSET);
	fill_box( 450, 50, 100,100, White, fgl::_GSET);

	fgl::FGColor Coral(FGColor::blue);

	for(int i=0; i<16; i++)
	{
		fill_box( 50+i*32,  150, 100,80, Black, fgl::_GSET);
		fill_box( 50+i*32,  250, 100,80, Coral, fgl::_GSET);
		Black.Lighter(1.3);
		Coral.Darker(1.1);
	}

	DrawGradient( *fgl::FGApp::GetRootWindow(), 0,350, fgl::FGColor(fgl::FGColor::black), fgl::FGColor(fgl::FGColor::white), 1024, 10);
	DrawGradient( *fgl::FGApp::GetRootWindow(), 0,360, fgl::FGColor(fgl::FGColor::black), fgl::FGColor(fgl::FGColor::red), 1024, 10);
	DrawGradient( *fgl::FGApp::GetRootWindow(), 0,370, fgl::FGColor(fgl::FGColor::black), fgl::FGColor(fgl::FGColor::green), 1024, 10);
	DrawGradient( *fgl::FGApp::GetRootWindow(), 0,380, fgl::FGColor(fgl::FGColor::black), fgl::FGColor(fgl::FGColor::blue), 1024, 10);
	DrawGradient( *fgl::FGApp::GetRootWindow(), 0,390, fgl::FGColor(fgl::FGColor::black), fgl::FGColor(fgl::FGColor::yellow), 1024, 10);

	for(int i=0; i<360; i++)
	{
		fgl::FGColor c((unsigned)0);
		c.SetHLS(i, 0.5, 1.);
		fill_box( 0,  i+400, 1024, 1, c, _GSET);
	}

}

void DrawImages(void)
{
	int i;
	fgl::FGBitmap image("../data/mountains.jpg");
	fgl::FGBitmap image2("../data/mountains.jpg");
	fgl::FGBitmap image3("../data/mountains.jpg");
	fgl::FGBitmap image4("../data/mountains.jpg");

	fgl::FGPixel* bmp = image2.GetArray();

	for(i=400*300; i; i--)
	{
		fgl::FGColor cc(*bmp);
		cc.Desaturate();
		*bmp = cc;
		bmp++;
	}
	bmp = image3.GetArray();

	for(i=400*300; i; i--)
	{
		fgl::FGColor cc(*bmp);
		cc.Darker(1.5);
		*bmp = cc;
		bmp++;
	}
	bmp = image4.GetArray();

	for(i=400*300; i; i--)
	{
		fgl::FGColor cc(*bmp);
		cc.Lighter(1.5);
		*bmp = cc;
		bmp++;
	}
	fgl::FGApp::GetRootWindow()->WindowPutBitmap(50,30,0,0,400,300,&image);
	fgl::FGApp::GetRootWindow()->WindowPutBitmap(500,30,0,0,400,300,&image2);
	fgl::FGApp::GetRootWindow()->WindowPutBitmap(50,430,0,0,400,300,&image3);
	fgl::FGApp::GetRootWindow()->WindowPutBitmap(500,430,0,0,400,300,&image4);
}

void DrawColors(void)
{
	const int w = fgl::FGApp::GetRootWindow()->GetW() / 14;
	const int h = fgl::FGApp::GetRootWindow()->GetH() / 10;
	fgl::FGApp::GetRootWindow()->set_font(0);

	for (int i = 0; i<14; i++)
	{
	  for (int j = 0;j<10; j++)
	  {
		fgl::FGColor color(fgl::FGColor::GetColorFromIndex(j + i * 10));
		int x = i * w;
		int y = j * h;
		if (fgl::FGColor::GetNameFromIndex(j + i * 10))
		{
			fgl::FGApp::GetRootWindow()->WindowBox(x, y, w, h, color);
	    	fgl::FGApp::GetRootWindow()->WindowText(x+8, y+8, FGColor::GetNameFromIndex(j + i * 10));
		}
	  }
	}
}

int main(int argc, char **argv)
{
	fgl::FGApp app(fgl::G1024x768, argc, argv, fgl::CDARK, fgl::APP_ENABLEALTX | fgl::APP_ROOTWINDOW);

	DrawGradients();

	fgl::FGDialog message("Click OK to continue ...");
	message.ShowOk();

	DrawImages();
	message.ShowOk();

	DrawColors();
	message.ShowOk();

	fgl::FGColor test_color(fgl::FGColor::blue);
	fgl::FGColorPicker picker(&test_color);
	message.ShowOk();

	return 0;
}


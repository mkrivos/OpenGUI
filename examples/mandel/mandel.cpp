/*Made by Frantisek Jahoda*/
#include <fastgl/fastgl.h>

fgl::FGDrawBuffer *screen;
double zoom;
double pozice[200];

//a tady vykreslujeme nasi Mandelbrotovu mnozinu
void DrawSet(double pos_x,double pos_y,double zoom, fgl::FGWindow *wnd)
{
	int x,y,c;
	double cr,ci,zr,zi,zr1,zi1,screenw,screenh;
	fgl::FGPixel *misto;

	misto = screen->GetArray();
	screenw=screen->GetW()/2;
	screenh=screen->GetH()/2;
	for (y=0;y<screen->GetH();y++)
	for (x=0;x<screen->GetW();x++)
	{
	 cr=(double(x)-screenw)*zoom+pos_x*screenw;
	 ci=(double(y)-screenh)*zoom+pos_y*screenh;
	 zr1=0;zi1=0;
	 c=0;
	 do{
	  zi=2*zi1*zr1+ci;
	  zr=(zr1*zr1)-(zi1*zi1)+cr;
	  c++;
	  zr1=zr;
	  zi1=zi;
	 }while (((zi>0?zi:-zi)*zoom<screenh) && ((zr>0?zr:-zr)*zoom<screenw) && c<128);
	 //pokud pocet kroku presahne 128 lezi bod uvnitr mnoziny a cyklus konci,
	 //snizenim max. poctu kroku se program zrychli,ale pri vetsim zoomu zaniknou detaily
	   *misto=(256-(c*2))<<8;
	   misto++;
	   //misto 128-c lze doplnit jakykoliv dalsi predpis, pro barvu mnoziny
	}
	wnd->WindowPutBitmap(0,0,0,0,1000,1000,screen);
}

void wndproc(fgl::FGEvent *e)
{
	static int p_pozice=0,mousex,mousey;
	mousex = fgl::FGApp::GetMouseX();
	mousey = fgl::FGApp::GetMouseY();
	
	switch(e->GetType())
	{
		case fgl::INITEVENT:
			zoom=0.02;
			pozice[0]=0;
			pozice[1]=0;
			screen = new fgl::FGDrawBuffer(e->wnd->GetWW(), e->wnd->GetHW());
			DrawSet(pozice[p_pozice*2],pozice[p_pozice*2+1],zoom,e->wnd);
			break;
		case fgl::CLICKLEFTEVENT:
			//na leve tlacitko mysi se priblizuje
			if (p_pozice<99)
			{
				p_pozice++;
				zoom*=0.5;
				pozice[p_pozice*2]  =(double(mousex)/e->wnd->GetW()*2-1)*zoom+pozice[p_pozice*2-2];
				pozice[p_pozice*2+1]=(double(mousey)/e->wnd->GetH()*2-1)*zoom+pozice[p_pozice*2-1];
				DrawSet(pozice[p_pozice*2],pozice[p_pozice*2+1],zoom,e->wnd);
			}
			break;
		case fgl::CLICKRIGHTEVENT:
			//na prave oddaluje
			if (p_pozice>0)
			{
				p_pozice--;
				zoom*=2;
				DrawSet(pozice[p_pozice*2],pozice[p_pozice*2+1],zoom,e->wnd);
			}
			break;
	}
}

int main(int argc, char *argv[])
{
	fgl::FGApp MyApp(fgl::G640x480, argc, argv, fgl::CBLACK, fgl::APP_ENABLEALTX);
	fgl::FGWindow *wnd = new fgl::FGWindow(0, 0,0,640,480,"mandel", wndproc);
	MyApp.Run();
	return 0;
}


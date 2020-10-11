#include <fastgl/fastgl.h>
#include <fastgl/widgets.h>
#include <fastgl/listbox.h>
#include <vector>

#ifdef BPP8
#warning "compile with -DBPP16 for jpg & png support"
#endif


static int zoom = 1;
static int automat = 0;
static int cas = 5;
static int randm = 0;
static fgl::FGWindow *NastaveniaPtr;

class TWindow;
static TWindow *MyWnd;
static void switch_res(fgl::CallBack);

class TWindow : public fgl::FGWindow
{
		std::vector<char *> images;
		std::vector<char *>::iterator cimage;
		fgl::FGBitmap *bmp;
		int x,y,count;
		
		int reload()
		{
			char buf[256];
			images.clear();
			FILE *fp=fopen("file.lst","r");
			if (fp==0)
			{
				system("find -name '*.pcx' >  file.lst");
				system("find -name '*.jpg' >> file.lst");
				system("find -name '*.JPG' >> file.lst");
				system("find -name '*.gif' >> file.lst");
				system("find -name '*.pcx' >> file.lst");
				system("find -name '*.bmp' >> file.lst");
				system("find -name '*.png' >> file.lst");
			}
			if (fp==0) fp=fopen("file.lst","r");
			if (fp)
			{
				while(!feof(fp))
				{
					fgets(buf,255,fp);
					if (buf[strlen(buf)-1] == '\n')
						buf[strlen(buf)-1]=0;
					images.push_back(strdup(buf));
				}
				fclose(fp);
			}
			cimage = images.begin();
			return images.size();
		}
		void render()
		{
//			if (*image)
			{
				if (bmp) delete bmp;
				bmp = new fgl::Bitmap(*cimage);
				if (zoom && (w<bmp->GetW() || h<bmp->GetH()))
				{
					float rx = (float)w/bmp->GetW();
					float ry = (float)h/bmp->GetH();
					if (rx<ry) bmp->stretch(int(bmp->GetW()*rx), int(bmp->GetH()*rx));
					else bmp->stretch(int(bmp->GetW()*ry), int(bmp->GetH()*ry));
				}
			}
		}
		void show()
		{
			if (bmp)
			{
				x = (w-bmp->GetW())/2;
				y = (h-bmp->GetH())/2;
				_draw();
			}
		}
		void _draw()
		{
			set_fcolor(0);
			box(0,0,w,h);
			WindowLock();
			WindowPutBitmap(x,y,0,0,bmp->GetW(),bmp->GetH(),bmp);
			WindowText(4,GetHW()-16,*cimage,fgl::CYELLOW);
			WindowUnLock();
		}
		static void Automat(int t)
		{
			if ((t%cas)==0 && automat)
			MyWnd->OnKeyPress(' ');
		}

		static void casovac(fgl::CallBack)
		{
			fgl::cApp->SetTimerProc(Automat);
		}
		static void Options(fgl::CallBack)
		{
			if (NastaveniaPtr)
				delete NastaveniaPtr;
			NastaveniaPtr = new fgl::FGWindow(&NastaveniaPtr, 2000, 2000, 276, 260, "Options", 0, fgl::CYELLOW, fgl::CGRAYED, fgl::WFRAMED|fgl::WTITLED|fgl::WCLICKABLE);
			NastaveniaPtr->WindowBox(0,164,280,80, fgl::CGRAY2);
			NastaveniaPtr->set_font(fgl::FONTSYSMEDIUM);
			NastaveniaPtr->set_bcolor(fgl::CGRAY2);
			NastaveniaPtr->printf(26,194,"Press the RIGHT BUTTON or SPACE");
			NastaveniaPtr->printf(90,210,"to the next image");
			NastaveniaPtr->AddCheckButton(16, 16, "stretch images if needed", 'z', &zoom);
			NastaveniaPtr->AddCheckButton(16, 48, "automat", 'a', &automat);
			NastaveniaPtr->AddCheckButton(16, 80, "random", 'r', &randm);
			NastaveniaPtr->AddEditBox(112, 72, 72, 40, "time [s]", 't', &cas, casovac, 1, 999);
			NastaveniaPtr->AddPushButton(38, 170, 60, 21, "Close", 'c', fgl::Control::Close);
			NastaveniaPtr->AddPushButton(174, 170, 60, 21, "Quit", 'q', fgl::Control::Quit);
			NastaveniaPtr->AddPushButton(16, 112, 96, 21, "640x480", 0,  switch_res);
			NastaveniaPtr->AddPushButton(16, 138, 96, 21,  "800x600", 0,  switch_res);
			NastaveniaPtr->AddPushButton(152, 112, 96, 21,  "1024x768",  0, switch_res);
			NastaveniaPtr->AddPushButton(152, 138, 96, 21,  "1280x1024", 0, switch_res);
		}
		
	public:
		// overload standard fnc
		virtual void OnEndDrag(int ,int , int , int , int)
		{
			int a,b,c,d;
			fgl::FGApp::GetDragVector(a,b,c,d);
			x += c;
			y += d;
			_draw();
		}
		void OnPaint(void)
		{
			show();
		}
		void OnKeyPress(int key)
		{
			int i;
			switch(key)
			{
			case ' ':
			case PGDOWN:
#ifdef INDEX_COLORS
				render();
#endif
				show();
				for(i=500;i;i--)
				{
					if (randm==0)
					{
						++cimage;
						++cimage;
						if (images.end() == cimage)
							cimage = images.begin();
						--cimage;
					}
					else
					{
#if defined(_WIN32) && !defined(__CYGWIN__)
						cimage = images.begin() + int(random(count));
#else
						cimage = images.begin() + int(random()%count);
#endif
					}
					FILE *f = fopen(*cimage, "r");
					if (f==0) continue;
					fclose(f);
					break;
				}
				if (i==0) fgl::IError("No images available!",1);
#ifndef INDEX_COLORS
				else
					render();
#endif
				break;
			case CR:
				Options(0);
				break;
			}
		}
		TWindow(TWindow **self) : fgl::FGWindow((FGWindow **)self,0,0,fgl::GetXRes(),fgl::GetYRes(),"Okno",0, fgl::CWHITE, fgl::CBLACK, fgl::WUNMOVED|fgl::WNOPICTO|fgl::WCLICKABLE)
		{
			fgl::cCfg->ReadInt("zoom",zoom);
			fgl::cCfg->ReadInt("automat",automat);
			fgl::cCfg->ReadInt("cas",cas);
			fgl::cCfg->ReadInt("random",randm);
			bmp = 0;
			fgl::cApp->SetTimerProc(Automat);
			count = reload();
			if (count)
			{
				++cimage;
				render();
				show();
				++cimage;
				Options(0);
#ifndef INDEX_COLORS
				render();
#endif
			}
			else fgl::IError("No image files!", 1);
		}
		void OnContextPopup(int x, int y)
		{
			OnKeyPress(PGDOWN);
		}
		void OnClick(int x, int y)
		{
			Options(0);
		}
		~TWindow()
		{
			fgl::cCfg->WriteInt("zoom",zoom);
			fgl::cCfg->WriteInt("automat",automat);
			fgl::cCfg->WriteInt("cas",cas);
			fgl::cCfg->WriteInt("random",randm);
			if (bmp) delete bmp;
			fgl::FGApp::AppDone();
		}
};

static void switch_res(fgl::CallBack cb)
{
	switch(cb->GetLocalId()-5)
	{
		case 1:
			fgl::graph_change_mode(640, 480);
			break;
		case 2:
			fgl::graph_change_mode(800, 600);
			break;
		case 3:
			fgl::graph_change_mode(1024, 768);
			break;
		case 4:
			fgl::graph_change_mode(1280, 1024);
			break;
	}
	if (MyWnd)
	{
		MyWnd->WindowShape(0,0,fgl::GetXRes(),fgl::GetXRes());
		fgl::Window::ShowAll();
	}
}

class TForm : public fgl::FGApp
{
	public:
		TForm(int argc, char **argv)
			: fgl::FGApp(fgl::G640x480,argc,argv,0,fgl::APP_CFG | fgl::APP_ENABLEALTX)
		{
			fgl::SetColorFuzzy(3);
			MyWnd = new TWindow(&MyWnd);
		}
		~TForm()
		{
			delete MyWnd;
		}
};

int main(int argc, char **argv)
{
	TForm MyApp(argc, argv);
	MyApp.Run();

	return 0;
}


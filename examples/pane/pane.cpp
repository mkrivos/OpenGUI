#include <fastgl/fastgl.h>
#include <fastgl/widgets.h>
#include <fastgl/fgpane.h>

class MyBlock : public fgl::FGBlock
{
	public:
		MyBlock()
		 : FGBlock(200, 100, "My Block")
		{
		}
		virtual void build(fgl::FGWindow *parent, int x, int y)
		{
			Add(parent->AddPushButton(x+24,y+24,80,21,"Button", 'b'));
			parent->AddPushButton(x+24,y+24+24,80,21,"Button", 'b');
		}
};

class MyBlock2 : public fgl::FGBlock
{
	public:
		MyBlock2()
		 : FGBlock(200, 100, "My Block2")
		{
		}
		virtual void build(fgl::FGWindow *parent, int x, int y)
		{
			parent->AddCheckButton(x+24,y+24,"CheckButton", 'b');
			parent->AddPointButton(x+24,y+24+24,"CheckButton", 'b');
		}
};

class MyBlock3 : public fgl::FGBlock
{
		static void func(fgl::CallBack)
		{
		}
		int variable;
		virtual void build(fgl::FGWindow *parent, int x, int y)
		{
			parent->AddCheckButton(x+24,y+24,"CheckButton", 'b');
			parent->AddEditBox(x+24,y+24+24,80,80,"Edit", 'e', &variable, func);
		}
	public:
		MyBlock3()
		 : FGBlock(200, 100, "My Block3")
		{
			variable = 0;
		}
};

int main(int argc, char* argv[])
{
	fgl::FGApp app(3,argc,argv,0,fgl::APP_ALL);

	fgl::FGWindow *w  = new fgl::FGWindow(0,10,10,440,240,"Window", 0, fgl::CWHITE, fgl::CGRAYED);
	fgl::FGWindow *w2 = new fgl::FGWindow(0,2000,2000,440,200,"Window2 - ShowVertical() when no more space", 0, fgl::CWHITE, fgl::CGRAYED);
	fgl::FGWindow *w3 = new fgl::FGWindow(0,0,2000,220,240,"Window3 - ShowHorizontal() when no more space", 0, fgl::CWHITE, fgl::CGRAYED);

	MyBlock *blok = new MyBlock;
	MyBlock2 *blok2 = new MyBlock2;
	MyBlock3 *blok3 = new MyBlock3;

	fgl::FGPane pane(w);
	pane.Add(blok);
	pane.Add(blok2);
	pane.ShowHorizontal();
	pane.ShowHorizontal(0,110);

	fgl::FGPane pane2(w2);
	pane2.Add(blok);
	pane2.Add(blok2);
	pane2.ShowVertical(0,50);

	fgl::FGPane pane3(w3);
	pane3.AddFrom("curves");
	pane3.ShowVertical(0,0);

	blok->Disable();
//	fgl::FGPaneProvider::Self()->ShowDialog("curves");

	app.Run();
//	delete fgl::FGPaneProvider::Self();
	delete w;
	delete w2;
	delete w3;
	return 0;
}

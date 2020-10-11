/*
*/
#include "fastgl.h"
#include "widgets.h"
#include "listbox.h"
#include "fgpane.h"

#ifdef FG_NAMESPACE
namespace fgl {
#endif

//---------------------------------------------------------------------------

typedef std::vector<FGBlock *> FGBlockContainer;
typedef FGBlockContainer::iterator FGBlockIterator;
typedef std::list<FGControl *> FGCtrlContainer;
typedef FGCtrlContainer::iterator CPTR;

//---------------------------------------------------------------------------

FGPaneProvider *FGPaneProvider::self=0;
FGListBox *FGPaneProvider::listboxPtr0;
FGListBox *FGPaneProvider::listboxPtr1;

/**
    Creates an empty block of controls with its size, name and style.
    @param w width of the block.
    @param h height of the block.
    @param n name of the block.
    @param s the style (facade).
*/
FGBlock::FGBlock(int w, int h, const char *n, FGBlockStyle& s)
	: width(w), height(h), style(s), fixed(false)
{
	strncpy(name, n, sizeof(name)-1);
//	FGPaneProvider::Register(this);
	container = (void*) new FGCtrlContainer;
}

/**
* Destructs block - don't call this destructor directly from your code!
* It will be called automagically from ~FGPaneProvider() at exit.
*/
FGBlock::~FGBlock()
{
	if (container) delete (FGCtrlContainer*)container;
	container = 0;
}

/**
* Adds an FGControl object to the list of FGControl of block.
* @note this list is usefull if you want to do something with all the
* controls into the block, by example: Disable() all at once.
* If you don't want this functionality, simply avoid this call.
*/
void FGAPI FGBlock::Add(FGControl *ctrl)
{
	((FGCtrlContainer* )container)->push_back(ctrl);
}

/**
* Disable all 'Added' object in block at once.
*/
void FGAPI FGBlock::Disable(void)
{
	FGCtrlContainer *p = (FGCtrlContainer *)container;
	for(CPTR pp = p->begin(); pp != p->end(); ++pp)
		(*pp)->Disable();
}

/**
* Enable all 'Added' object in block at once.
*/
void FGAPI FGBlock::Enable(void)
{
	FGCtrlContainer *p = (FGCtrlContainer *)container;
	for(CPTR pp = p->begin(); pp != p->end(); ++pp)
		(*pp)->Enable();
}

//---------------------------------------------------------------------------

FGPaneContainer::FGPaneContainer()
 : destroy(true)
{
	container = (void*)new FGBlockContainer;
}

FGPaneContainer::~FGPaneContainer()
{
	if (destroy)
	{
		for(FGBlockIterator i = ((FGBlockContainer*)container)->begin(); i != ((FGBlockContainer*)container)->end(); i++)
		{
			delete (FGBlock *)(*i);
		}
	}
	if (container) delete (FGBlockContainer*)container;
	container = 0;
}

//---------------------------------------------------------------------------

/**
* An workspace for your blocks. If your FGWindow uses FGTabPage feature (aka tabbed window)
* you can create object FGPane per each TabPage.
* @param w parent FGWindow
*/
FGPane::FGPane(FGWindow *w)
 : parent(w)
{
}

FGPane::~FGPane()
{}

/**
* Adds already created FGBlock object to FGPane.
* @param val a pointer to the object that will be added.
* @param fixed this is intended for FGPaneProvider to known if the added
* object is fixed or ar it is able to manually remove from FGPaneProvider menu.
* @note object itself doesn't check FGControl position to any boundary.
*/
void FGAPI FGPane::Add(FGBlock* val, bool fixed)
{
	val->SetFixed(fixed);
	((FGBlockContainer* )container)->push_back(val);
}

/**
* Adds FGBlock(s) described by plain string from app own pane config file.
* @param name a key to 'xxx.pane' file.
* @see FGPaneProvider
*/
void FGAPI FGPane::AddFrom(char * name)
{
/*
	StoreVectorStr* list = 0;
	FGBlock *b;
	// stringy
	list = FGPaneProvider::Self()->GetStore(name);
	if (list)
	{
		for (StoreVectorStr::TVectorIterator it = list->GetVectorBegin(); it != list->GetVectorEnd(); it++)
		{
			b = FGPaneProvider::Self()->FindBlockByName((*it).c_str());
			if (b) Add(b);
		}
	}
*/
}

/**
* Show all known FGBlocks for this FGPane from this position.
* This call 'realy' constructs & shows GUI Controls.
* Blocks will apears from the left to the rigth.
* @param xx the start x coord where block will be relocated
* @param yy the start y coord where block will be relocated
*/
void FGAPI FGPane::ShowHorizontal(int xx, int yy)
{
	int x=xx, y=yy, oldx;

	for(FGBlockIterator i = ((FGBlockContainer*)container)->begin(); i != ((FGBlockContainer*)container)->end(); i++)
	{
		oldx = x;
		x += (*i)->GetWidth();
		if (x >= parent->GetWW())
		{
			y += (*i)->GetHeight();
			oldx = x = xx;
		}
		(*i)->show(parent,oldx,y);
	}
}

/**
* Show all known FGBlocks for this FGPane from this position.
* This call 'realy' constructs & shows GUI Controls.
* Blocks will apears from the up to the down.
* @param xx the start x coord where block will be relocated
* @param yy the start y coord where block will be relocated
*/
void FGAPI FGPane::ShowVertical(int xx, int yy)
{
	int x=xx, y=yy, oldy;

	for(FGBlockIterator i = ((FGBlockContainer*)container)->begin(); i != ((FGBlockContainer*)container)->end(); i++)
	{
		oldy = y;
		y += (*i)->GetHeight();
		if (y >= parent->GetHW())
		{
			x += (*i)->GetWidth();
			oldy = y = yy;
		}
		(*i)->show(parent,x,oldy);
	}
}

//---------------------------------------------------------------------------

#if 1
/**
* Constructs object for managing FGPane with FGBlock over the all application.
* This object is implemented as Singleton - i.e. one and only one instance at
* the time is allowed.
* Object loads contents of the file named 'your_appname.pane' into memory
* as the database of sets <Pane, Blocks>. These sets are constructed by user
* from FGPaneProvider::ShowDialog().
* Once again Pane contains blocks ones 'hard added' by FGPane::Add() and ones
* 'dynamically binded' bu user (these are stored in *.pane file).
*/
FGPaneProvider::FGPaneProvider()
{
	char cfgname[64];

	destroy = true;
	FGBlock_Provider_DialogPtr = 0;

	sprintf(cfgname, ".%s.pane", cApp->name);
//	cfg = new CfgStore(cfgname);
}

/**
* Destruct object and *ALL* the registered FGBlock into the Application.
*/
FGPaneProvider::~FGPaneProvider()
{
	if (FGBlock_Provider_DialogPtr) delete FGBlock_Provider_DialogPtr;
	FGBlock_Provider_DialogPtr = 0;
//	delete cfg;
}

/**
* Register an instance of FGBlock.
* @note don't call directly!
*/
void FGAPI FGPaneProvider::Register(FGBlock* block)
{
	((FGBlockContainer* )Self()->container)->push_back(block);
}

/*
StoreVectorStr* FGAPI FGPaneProvider::GetStore(char *name)
{
	StoreVectorStr* list = (StoreVectorStr*) (cfg->GetStore(name));
	if (list == 0)
		list = (StoreVectorStr*)cfg->CreateStore(STORE_VECTOR_STR, name);
	return list;
}
*/
FGBlock* FGAPI FGPaneProvider::FindBlockByName(const char s[])
{
	FGBlock *rc=0;
	for(FGBlockIterator i = ((FGBlockContainer*)container)->begin(); i != ((FGBlockContainer*)container)->end(); i++)
	{
		if (strcmp((*i)->GetName(), s) == 0)
		{
			rc = *i;
			break;
		}
	}
	return rc;
}

void FGPaneProvider::FGBlock_Provider_DialogProc(FGEvent *p)
{
	int	xWnd = p->GetX();
	int	yWnd = p->GetY();
	int result;

	switch(p->GetType()) {
		case INITEVENT:
			listboxPtr0 = p->wnd->AddListBox(16, 16, 256, 18, 20);
			listboxPtr1 = p->wnd->AddListBox(344, 16, 256, 18, 20);
			listboxPtr1->SetPaper(CWHITE);
			listboxPtr1->SetColors(CWHITE, CBLACK, CBLUE, CYELLOW);
			p->wnd->AddPushButton(296, 24, 40, 32, ">>>", 0, AddToCfg);
			p->wnd->AddPushButton(296, 64, 40, 32, "DEL", 0, RemoveFromCfg);
//			p->wnd->AddPushButton(296, 104, 40, 32, "UP", 0, Up);
//			p->wnd->AddPushButton(296, 144, 40, 32, "DOWN", 0, Down);
			p->wnd->AddPushButton(296, 304, 40, 32, "UNDO", 0, Cancel);
			p->wnd->AddPushButton(296, 344, 40, 32, "OK", 0, Ok);
			break;
		case KEYEVENT:
			listboxPtr0->DoListBox(p->GetKey());
			break;
		case REPAINTEVENT:
			break;
		case CLICKLEFTEVENT:
			result = listboxPtr0->Test(xWnd,yWnd);
			if (result >= 0) listboxPtr0->SetIndex(result);
			result = listboxPtr1->Test(xWnd,yWnd);
			if (result >= 0) listboxPtr1->SetIndex(result);
			break;
		case ACCELEVENT:
			break;
		case TERMINATEEVENT:
			listboxPtr0 = 0;
			listboxPtr1 = 0;
			break;
	}
}

void FGPaneProvider::AddToCfg(CallBack cb)
{
	FGBlockContainer* cont = (FGBlockContainer*) (Self()->container);
	if (cont->size()<=0) return;
//	StoreVectorStr* list = (StoreVectorStr*) cb->GetOwner()->GetParameter();
	int index = Self()->listboxPtr0->GetIndex();
	const char *blok = (*cont)[index]->GetName();

	Self()->listboxPtr1->insert(blok);
//	list->push_back(blok);
	Self()->listboxPtr1->Draw(1);
}

void FGPaneProvider::RemoveFromCfg(CallBack cb)
{
//	StoreVectorStr* list = (StoreVectorStr*) cb->GetOwner()->GetParameter();
//	if (list->size()<=0) return;
	if (Self()->listboxPtr1->GetSize() <= 0) return;
	int index = Self()->listboxPtr1->GetIndex();

	Self()->listboxPtr1->erase(index);
//	list->erase(index);
	Self()->listboxPtr1->Update();
}

void FGPaneProvider::Up(CallBack)
{
}

void FGPaneProvider::Down(CallBack cb)
{
}

void FGPaneProvider::Ok(CallBack cb)
{
//	Self()->cfg->Save();
	FGControl::Close(cb);
}

void FGPaneProvider::Cancel(CallBack cb)
{
	FGControl::Close(cb);
}

/**
* The methods intended for interactive building Panes with Blocks by user.
*/
void FGAPI FGPaneProvider::ShowDialog(char *store_name)
{
	char gps[128] = "FGBlock Provider Dialog - ";

	if (FGBlock_Provider_DialogPtr)
	{
		FGBlock_Provider_DialogPtr->WindowFocus();
		return;
	}

	strcat(gps, store_name);
	FGBlock_Provider_DialogPtr = new FGWindow(&FGBlock_Provider_DialogPtr, 32, 320, 640, 419, gps, FGBlock_Provider_DialogProc,
		 CBLACK, CWHITED, WFRAMED|WTITLED|WNOPICTO|WCLICKABLE|WUSELAST|WESCAPE);
	for(FGBlockIterator i = ((FGBlockContainer*)container)->begin(); i != ((FGBlockContainer*)container)->end(); i++)
	{
		listboxPtr0->insert( (*i)->GetName() );
	}
	listboxPtr0->Draw(1);
/*
	StoreVectorStr* list = FGPaneProvider::Self()->GetStore(store_name);

	if (list) for (StoreVectorStr::TVectorIterator it = list->GetVectorBegin(); it != list->GetVectorEnd(); it++)
	{
		listboxPtr1->insert( (*it).c_str() );
	}
	listboxPtr1->Draw(1);

	FGBlock_Provider_DialogPtr->SetParameter(list);
*/
}
#endif // if 0

#ifdef FG_NAMESPACE
}
#endif

//---------------------------------------------------------------------------

#ifdef _REGRESS_
class MyBlock : public FGBlock
{
	public:
		MyBlock()
		 : FGBlock(200, 100, "My Block")
		{
		}
		virtual void build(FGWindow *parent, int x, int y)
		{
			Add(parent->AddPushButton(x+24,y+24,80,21,"Button", 'b'));
			parent->AddPushButton(x+24,y+24+24,80,21,"Button", 'b');
		}
};

class MyBlock2 : public FGBlock
{
	public:
		MyBlock2()
		 : FGBlock(200, 100, "My Block2")
		{
		}
		virtual void build(FGWindow *parent, int x, int y)
		{
			parent->AddCheckButton(x+24,y+24,"CheckButton", 'b');
			parent->AddPointButton(x+24,y+24+24,"CheckButton", 'b');
		}
};

class MyBlock3 : public FGBlock
{
		static void func(CallBack)
		{
		}
		int variable;
		virtual void build(FGWindow *parent, int x, int y)
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
	App app(3,argc,argv,0,APP_ALL);

	FGWindow *w = new FGWindow(0,10,10,440,240,"FGWindow", 0, CWHITE, CGRAYED);
	FGWindow *w2 = new FGWindow(0,2000,2000,440,200,"Window2 - ShowVertical() when no more space", 0, CWHITE, CGRAYED);
	FGWindow *w3 = new FGWindow(0,0,2000,220,240,"Window3 - ShowHorizontal() when no more space", 0, CWHITE, CGRAYED);

	MyBlock *blok = new MyBlock;
	MyBlock2 *blok2 = new MyBlock2;
	MyBlock3 *blok3 = new MyBlock3;

	FGPane pane(w);
	pane.Add(blok);
	pane.Add(blok2);
	pane.ShowHorizontal();
	pane.ShowHorizontal(0,110);

	FGPane pane2(w2);
	pane2.Add(blok);
	pane2.Add(blok2);
	pane2.ShowVertical(0,50);

	FGPane pane3(w3);
//	pane3.Add(blok);
//	pane3.Add(blok3);
	pane3.AddFrom("curves");
	pane3.ShowVertical(0,0);

	blok->Disable();
	FGPaneProvider::Self()->ShowDialog("curves");

	app.Run();
	delete FGPaneProvider::Self();
	delete w;
	delete w2;
	delete w3;
	return 0;
}
#endif

//---------------------------------------------------------------------------



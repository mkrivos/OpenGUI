//
//	25.01.1999 Marian Krivos (nezmar@atlas.sk)
//
//	RAD main file

#include <fastgl/fastgl.h>
#include <fastgl/widgets.h>

#include "rad_def.h"
#include "rad_type.h"
#include "rad_sym.h"

#ifdef FG_NAMESPACE
using namespace fgl;
#endif

FGWindow *mWnd;
static FGWindow *OKPtr;
Projekt *prj;
static FGWindow *Bitmap_InventaryPtr;
static FGListBox *widgetPtrx;
static FGTextEditor *editor;
Wind windx={ 0, 256, 424,304, 182, WSTANDARD,IM,PM,-1, "form1" };
Flags flg={1,1,0,0,0,0,0,1,0,0,0,0};
FGEditBox *eb0,*eb1,*eb2,*eb3,*eb4;
static FGPointButton	*pb0, *pb1,	*pb2, *pb3,	*pb4, *pb5,
	*pb6, *pb7,	*pb8, *pb9, *pb10, *pb11, *pb12, *pb13, *pb14, *pb15, *pb16;
FGListBox *lBox, *lBox2;
char gps[256];
char srcname[64];
int isCode, test=0;
FGWindow *Set_GranularityPtr;
int granularity_x=8;
int granularity_y=8;
char * current_bmp;
FGBitmap *nullBmp;
FGColorScheme *old_scheme,
rad_sc;
int rad_ink=CBLUE, rad_paper=CGRAY1;

FGEditBox *eb_x, *eb_y, *eb_w, *eb_h, *eb_max, *eb_min, *eb_name,
		*eb_fnc, *eb_size, *eb_rows, *eb_lins, *eb_var;
static FGEditBox *ebdef;
FGPushButton *pb_paper, *pb_ink, *pb_key, *pb_val;
FGCheckButton *pt_trans, *pt_rc, *pt_check, *pt_grp;
char *sel_bmp;
int cbmp=0;

static Accel cAcc;
static FGWindow *ColorsManagementPtr;
static int rrr;
static int ggg;
static int bbb, ccc=16;
static int nxcolors;
extern int verb;
FGWindow *Default_valuesPtr;
static FGListBox *listboxPtr0;
static int current_value;

#include "rgb.cc"

static char *rad_str[]=
{
"UNDEFINE",
"STRING",
"BOX",
"RECT",
"LINE",
"CIRCLE",
"PUSHBUT",
"PUSHICON",
"CHECKBUT",
"POINTBUT",
"EDITBOX1",
"EDITBOX2",
"EDITBOX3",
"SLIDEBARH",
"SLIDEBARV",
"LISTBOX",
"MENU",
"FCIRCLE",
"BITMAP",
"PROGRESSBAR",
"LABEL",
"ELLIPSE",
"FELLIPSE",
"ARC",
"PANEL",
"TABPAGE",
};

static int *color_settings;
static Accel *current_accel;
static FGWindow * SetColorsPtr;

static void ColorsManagement(FGControl *);
void Schemes(CallBack);
void SetValue(CallBack);

// ------------------------------------------------------------------------

static void Redraw(FGControl *)
{
	prj->Redraw(&prj->Okno[prj->curr]);
	lBox->RedrawItem();
	mWnd->WindowFocus();
}

static void AddToRc(FGControl *)
{
	current_accel->flags ^= ACCF_ADDTORC;
}

static void Transp()
{
	current_accel->flags ^= ACCF_TRANSP;
}

static void RangeCheck(FGControl *)
{
	current_accel->flags ^= ACCF_CHECK;
	if (current_accel->flags & ACCF_CHECK)
	{
		eb_max->Enable();
		eb_min->Enable();
	}
	else
	{
		eb_max->Disable();
		eb_min->Disable();
	}
}

static void RedrawTransp(FGControl *)
{
	Transp();
	Redraw(0);
}

static FGWindow *form1Ptr;

static void form1Proc(FGEvent *p)
{
	switch(p->GetType()) {
		case INITEVENT:
			p->wnd->set_font(5);
			p->wnd->WindowText(16, 32, "OpenGUI Sourcer",CWHITE);
			p->wnd->set_font(1);
			p->wnd->WindowText(16, 116, "This is an utility to generate source");
			p->wnd->WindowText(56, 132, "skeleton for OpenGUI library");
			p->wnd->WindowText(120, 160, "written by");
			p->wnd->WindowText(72, 178, "(c) 1999,2004 MARIAN KRIVOS");
			break;
		case KEYEVENT:
		case CLICKLEFTEVENT:
		case CLICKRIGHTEVENT:
			delete p->wnd;
			break;
		case TERMINATEEVENT:
			break;
	}
}

static void About(FGControl *)
{
	form1Ptr = new FGWindow(&form1Ptr, 272, 334, 340, 208, "", form1Proc, CWHITED, 1, 0x285|WCENTRED);
}

static void SetColorsProc(GuiEvent *p)
{
	int	xWnd = p->GetX()-8;
	int	yWnd = p->GetY()-8;
	int i,j;

	switch(p->GetType())
	{
		case KEYEVENT:
			delete p->wnd;
			break;
		case INITEVENT:
			for(i=0; i<16; i++) for(j=0; j<16; j++)
			{
				if (prj->paleta[(j+i*16)].alfa || !i)
				{
					p->wnd->WindowBox(8+j*20,  8+i*20, 16, 16, j+i*16);
					p->wnd->WindowRect(8+j*20, 8+i*20, 16, 16, 0);
					if (*color_settings == (j+i*16)) p->wnd->WindowRect(8+j*20-1, 8+i*20-1, 18, 18, CYELLOW);
				}
			}
			break;
		case CLICKLEFTEVENT:
			if (yWnd<0 || xWnd<0) return;
			xWnd = xWnd/20;
			yWnd = yWnd/20;
			if (yWnd>15 || xWnd>15) return;
			if (!prj->paleta[(xWnd+yWnd*16)].alfa && !(xWnd<16 && !yWnd)) return;
			if (color_settings) *color_settings = xWnd + yWnd*16;
			color_settings = 0;
			delete p->wnd;
			prj->Redraw(&prj->Okno[prj->curr]);
			break;
	}
}

static void SetColors(char *name)
{
	if (SetColorsPtr) return;
	SetColorsPtr = new FGWindow(&SetColorsPtr, 240, 80, 340, 420-56, name, SetColorsProc, 0, rad_paper, 0x203|WUSELAST);
}

static void SetFontProc(FGEvent *p)
{
	if (p->GetType() == ACCELEVENT)
	{
		prj->font = p->accel->GetLocalId()+1;
		prj->DrawAll(1);
	}
}

static void SetFont(FGControl *)
{
	FGMenuWindow *m1Wnd = new FGMenuWindow(90, 119+12,SetFontProc);
	m1Wnd->AddMenu("Font 04 x 06");
	m1Wnd->AddMenu("Font 08 x 08");
	m1Wnd->AddMenu("Font 08 x 16");
	m1Wnd->AddMenu("Font 12 x 20");
	m1Wnd->AddMenu("Font 16 x 25");
	m1Wnd->AddMenu("Font 20 x 34");
}

FGWindow *Set_KeyPtr;

AccKey acckey[] =
{
{F01, "F1 key", "F01"},
{F02, "F2 key", "F02"},
{F03, "F3 key", "F03"},
{F04, "F4 key", "F04"},
{F05, "F5 key", "F05"},
{F06, "F6 key", "F06"},
{F07, "F7 key", "F07"},
{F08, "F8 key", "F08"},
{F09, "F9 key", "F09"},
{F10, "F10 key", "F10"},
{F11, "F11 key", "F11"},
{F12, "F12 key", "F12"},
{HOME,"HOME key", "HOME"},
{END,"END key", "END"},
{CR,"CR key", "CR"},
{ESC,"ESC key", "ESC"},
{BACKSP,"BCKSPC key", "BACKSP"},
{KLEFT,"LEFT ARROW", "KLEFT"},
{KUP,"UP ARROW", "KUP"},
{KDOWN,"DOWN ARROW", "KDOWN"},
{KRIGHT,"RIGHT ARROW", "KRIGHT"},
{DEL,"DELETE key", "DEL"},
{INSERT,"INSERT key", "INSERT"},
{PGUP,"PAGE UP", "PGUP"},
{PGDOWN,"PAGE DOWN", "PGDOWN"},
{TAB,"TAB key", "TAB"},
{LF,"LF key", "LF"},
{ALT_X,"ALT+X key", "ALT_X"},
{CTRL_TAB,"CTRL+TAB", "CTRL_TAB"},
{ALT_F04,"ALT+F4", "ALT_F04"},
{ALT_UP,"ALT+UP key", "ALT_UP"},
{ALT_DOWN,"ALT+DOWN key", "ALT_DOWN"},
{ALT_RIGHT,"ALT+RIGHT key", "ALT_RIGHT"},
{ALT_LEFT,"ALT+LEFT key", "ALT_LEFT"}
};

static void Set_KeyProc(FGEvent *p);

static void SetKeyProc2(FGEvent *p)
{
	FGEvent e(NOEVENT);
	if (p->GetType() == ACCELEVENT)
	{
		current_accel->flags |= ACCF_PREDEF;
		current_accel->key = -p->accel->GetLocalId();
		Set_KeyPtr->SendToWindow(&e);
	}
}

static void Set_KeyProc(FGEvent *p)
{
	static PushButton *idb1,*idb0,*idb2;
	static EditBox *eb;
	static int old = current_accel->key;

	switch(p->GetType()) {
		case INITEVENT:
			eb = p->wnd->AddEditBox(11, 136, 80, 56, "Key Code", 0, &current_accel->key);
			p->wnd->WindowText(27, 26, "Input a key code or press");
			p->wnd->WindowText(44, 52, "a key on the keyboard only");
			p->wnd->WindowText(68, 80, "NOTE: the ENTER key you must", 11, 5);
			p->wnd->WindowText(100, 105, "assign in the input box!", 11, 5);
			idb2 = p->wnd->AddPushButton(12, 163, 134, 21, "Predefined keys", 0, 0);
			p->wnd->WindowRect(8, 8, 314, 120, 12);
			idb0 = p->wnd->AddPushButton(169, 151, 64, 21, "Ok");
			idb1 = p->wnd->AddPushButton(255, 151, 64, 21, "Cancel");
			break;
		case KEYEVENT:
			if (p->GetKey() == CR)
			{
				delete p->wnd;
				return;
			}
			if (p->GetKey() >=0 && p->GetKey() < 256 && p->GetKey() != CR)
			{
				current_accel->key = p->GetKey();
				eb->ChangeItem(&current_accel->key);
				current_accel->flags &= ~ACCF_PREDEF;
			}
			break;
		case ACCELEVENT:
			if (p->GetKey() == idb2->GetId())
			{
				MenuWindow *m1Wnd = new MenuWindow(98, (sizeof(acckey)/sizeof(AccKey))*20+8, SetKeyProc2);
				for(unsigned i=0; i<sizeof(acckey)/sizeof(AccKey); i++)
				{
					m1Wnd->AddMenu(acckey[i].str);
				}
			}
			else if (p->GetKey() == idb0->GetId())
			{
				delete p->wnd;
			}
			else if (p->GetKey() == idb1->GetId())
			{
				current_accel->key = old;
				delete p->wnd;
			}
		case TERMINATEEVENT:
			return;
	}
	if (current_accel->flags&ACCF_PREDEF)
	{
		sprintf(gps," name %s    ", acckey[-current_accel->key].str);
		eb->Disable();
	}
	else
	{
		sprintf(gps," char '%c', hex %04x ", current_accel->key, current_accel->key);
		eb->Enable();
	}
	p->wnd->WindowText(24,4,gps);
}

static void SetKey(FGControl *)
{
	Set_KeyPtr = new Window(&Set_KeyPtr, 212, 309, 338, 219, "Set Key", Set_KeyProc, 12, 5, 0x283);
}

static void SetInk(FGControl *)
{
	assert(current_accel);
	color_settings = &current_accel->ink;
	SetColors("Set Ink");
}

static void SetPaper(FGControl *)
{
	assert(current_accel);
	color_settings = &current_accel->paper;
	SetColors("Set Paper");
}

static void SetInkW(FGControl *)
{
	color_settings = &prj->Okno[prj->curr].ink;
	SetColors("Set Window Ink");
}

static void SetPaperW(FGControl *)
{
	color_settings = &prj->Okno[prj->curr].paper;
	SetColors("Set Window Paper");
}

static void SetBoxSize(FGControl *)
{
	current_accel->flags |= ACCF_VARSIZE;
}

static void SetBoxSize2(FGControl *)
{
	current_accel->flags &= ~ACCF_VARSIZE;
	current_accel->ww = (current_accel->h-16)/8; // width in chars
	eb_size->ChangeItem(&current_accel->ww);
	Redraw(0);
}

static void SetVideoProc(FGEvent *p)
{
	if (p->GetType() == ACCELEVENT)
	{
		prj->video_mode = p->accel->GetLocalId();
		prj->DrawAll(1);
	}
}

static void Video_mode(FGControl *)
{
	MenuWindow *m1Wnd = new MenuWindow(90, 122+12, SetVideoProc);
	m1Wnd->AddMenu("320 x 200");
	m1Wnd->AddMenu("640 x 480");
	m1Wnd->AddMenu("800 x 600");
	m1Wnd->AddMenu("1024 x 768");
	m1Wnd->AddMenu("1280 x 1024");
	m1Wnd->AddMenu("1600 x 1200");
}

static void Back_color(FGControl *)
{
	color_settings = &prj->back_color;
	SetColors("Set Background Color");
}

static void SetPtrs(Accel *ptr)
{
	eb_x->ChangeItem(&ptr->x);
	eb_y->ChangeItem(&ptr->y);
	eb_w->ChangeItem(&ptr->w);
	eb_h->ChangeItem(&ptr->h);
	eb_name->ChangeItem(ptr->name);
	eb_var->ChangeItem(ptr->variable);
	eb_fnc->ChangeItem(ptr->fnc);
	eb_rows->ChangeItem(&ptr->ww);
	eb_size->ChangeItem(&ptr->ww);
	eb_lins->ChangeItem(&ptr->hh);
	pt_trans->SetTrigger(ptr->flags&ACCF_TRANSP);
	pt_check->SetTrigger(ptr->flags&ACCF_CHECK);
	pt_rc->SetTrigger(ptr->flags&ACCF_ADDTORC);
	pt_grp->SetTrigger(ptr->flags&ACCF_BGROUP);
}

void ShowDialog(RadType type, Accel *ptr, int change_only)
{
	current_accel = ptr;

	if (prj->Okno[prj->curr].items>=MAX_ACCEL && !change_only) return; // no more space

	if (!change_only) // pridanie novej polozky
	{
		memset(ptr,0,sizeof(Accel)); // fill blank item
		ptr->x    = 8;
		ptr->y    = 8;
		ptr->w    = 64;
		ptr->h    = 21;
		ptr->ink  = prj->Okno[prj->curr].ink;
		ptr->paper= prj->Okno[prj->curr].paper;
		ptr->type =	type;
		ptr->flags = 0;
		ptr->min = 0;	// bgrp = 0
		ptr->max = 0;	// bmp  = 0
		ptr->bgrp = 0;	// bgrp = 0
		if (ptr->type==RAD_PROGRESSBAR)
		{
			ptr->w  =116;
			ptr->h  =24;
			ptr->ww =100;
		}
		else if (ptr->type==RAD_LISTBOX)
		{
			ptr->ww   = 1;
			ptr->hh   = 4;
			ptr->h    = 18;
		}
		else if (ptr->type==RAD_PUSHBUTTON1)
		{
			ptr->w  =64;
			ptr->h  =21;
		}
		else if (ptr->type>=RAD_EDITBOX1 && ptr->type<=RAD_EDITBOX3)
		{
			ptr->w  = 64;
			ptr->h  = 40;
			ptr->ww = (ptr->h-16)/8; // width in chars
			if (ptr->type==RAD_EDITBOX1)  // integer
			{
				ptr->max  = 0x7FFFFFFF;
				ptr->min  = 0x80000000;
			}
			else if (ptr->type == RAD_EDITBOX2)
			{
				ptr->mind = -9999999999.;
				ptr->maxd = 9999999999.;
			}
		}
		else if (ptr->type==RAD_SLIDEBARH || ptr->type==RAD_SLIDEBARV)
		{
			ptr->w  = 0;
			ptr->h  = 100;
			ptr->ww = 1;
		}
		else if (ptr->type==RAD_ARC)
		{
			ptr->maxd  = 1.;
			ptr->mind  = 0.;
		}
	}

	if (ptr->type==RAD_PROGRESSBAR || ptr->type==RAD_SLIDEBARH || ptr->type==RAD_SLIDEBARV)  // integer
	{
		eb_rows->SetName("STEPS");
		eb_h->SetHandler(0);
	}
	else
	{
		eb_rows->SetName("ROWS ");
		eb_h->SetHandler(ControlCall(SetBoxSize2));
	}

	eb_size->Disable();
	eb_lins->Disable();
	eb_rows->Disable();
	eb_name->Disable();
	eb_var->Disable();
	eb_fnc->Disable();
	pb_key->Disable();
	pb_ink->Disable();
	pb_paper->Disable();
	pb_val->Disable();

	pt_grp->Disable();
	pt_trans->Disable();
	pt_check->Disable();
	pt_rc->Disable();

	eb_x->Enable();
	eb_y->Enable();
	eb_h->Enable();
	eb_w->Enable();

	pt_grp->SetTrigger(ptr->flags&ACCF_BGROUP);
	pt_trans->SetTrigger(ptr->flags&ACCF_TRANSP);
	pt_rc->SetTrigger(ptr->flags&ACCF_ADDTORC);
	pt_check->SetTrigger(ptr->flags&ACCF_CHECK);

	switch(ptr->type)
	{
		case RAD_TABPAGE:
			eb_x->Disable();
			eb_y->Disable();
		case RAD_CHECKBUTTON:
		case RAD_POINTBUTTON:
		case RAD_MENU:
		case RAD_STRING:
		case RAD_LABEL:
			eb_w->Disable();
			eb_h->Disable();
			break;
	}
	switch(ptr->type)
	{
		case RAD_EDITBOX1:
		case RAD_EDITBOX2:
			pt_check->Enable();
		case RAD_EDITBOX3:
			eb_size->Enable();
		case RAD_CHECKBUTTON:
		case RAD_POINTBUTTON:
			pt_rc->Enable();
			eb_var->Enable();
			pb_val->Enable();
		case RAD_MENU:
		case RAD_LABEL:
		case RAD_PUSHBUTTON1:
			eb_name->Enable();
			eb_fnc->Enable();
			if (ptr->type>=RAD_PUSHBUTTON1 && ptr->type<=RAD_POINTBUTTON)
				pt_grp->Enable();
			pb_key->Enable();
			break;
		case RAD_LISTBOX:
			eb_lins->Enable();
		case RAD_SLIDEBARH:
		case RAD_SLIDEBARV:
			eb_rows->Enable();
			pb_val->Enable();
			eb_var->Enable();
			eb_fnc->Enable();
			break;
		case RAD_PROGRESSBAR:
			eb_rows->Enable();
			break;
		case RAD_STRING:
			pb_paper->Enable();
		case RAD_PANEL:
			eb_name->Enable();
		case RAD_BOX:
		case RAD_RECT:
		case RAD_LINE:
		case RAD_CIRCLE:
		case RAD_ARC:
		case RAD_FILLCIRCLE:
		case RAD_ELLIPSE:
		case RAD_FILLELLIPSE:
			pt_trans->Enable();
			pb_ink->Enable();
			break;
	}

	switch(ptr->type)
	{
		case RAD_CIRCLE:
		case RAD_ARC:
			eb_h->Disable();
			break;
	}
	mWnd->RemoveControl(eb_min);
	mWnd->RemoveControl(eb_max);

	if (ptr->type!=RAD_EDITBOX2 && ptr->type!=RAD_ARC)  // int
	{
		eb_max = mWnd->AddEditBox(635, 83, 48, 140, "MAX", 'a', &ptr->max);
		eb_min = mWnd->AddEditBox(635, 57, 48, 140, "MIN", 'i', &ptr->min);
		if (ptr->flags&ACCF_BGROUP)
		{
			if (ptr->type>=RAD_PUSHBUTTON1 && ptr->type<=RAD_POINTBUTTON)
			{
				eb_min->SetName("GRP");
				eb_min->SetKey('G');
				eb_max->Disable();
				eb_min->ChangeItem(&ptr->bgrp);
			}
		}
		else if (ptr->type!=RAD_EDITBOX1 || !(ptr->flags&ACCF_CHECK))
		{
			eb_max->Disable();
			eb_min->Disable();
		}
	}
	else
	{
		eb_max = mWnd->AddEditBox(635, 83, 48, 140, "MAX", 'a', &ptr->maxd, Redraw);
		eb_min = mWnd->AddEditBox(635, 57, 48, 140, "MIN", 'i', &ptr->mind, Redraw);
		if (!(ptr->flags&ACCF_CHECK))
		{
			eb_max->Disable();
			eb_min->Disable();
		}

		if(ptr->type==RAD_ARC)
		{
			eb_max->Enable();
			eb_min->Enable();
		}
	}
	if (ptr->type==RAD_TABPAGE)
	{
		eb_name->Enable();
	}
	SetPtrs(ptr);
	if (!change_only)
	{
		prj->PridajWidget(change_only);
		mWnd->WindowFocus();
		eb_name->ClickUp(TRUE);
	}
}

static void Dialog(RadType r);

static void AddString(FGControl *)
{
	Dialog(RAD_STRING);
}

static void AddBox(FGControl *)
{
	Dialog(RAD_BOX);
}

static void AddPanel(FGControl *)
{
	Dialog(RAD_PANEL);
}

static void AddTabPage(FGControl *)
{
	Dialog(RAD_TABPAGE);
}

static void AddRect(FGControl *)
{
	Dialog(RAD_RECT);
}

static void AddLine(FGControl *)
{
	Dialog(RAD_LINE);
}

static void AddEllipse(FGControl *)
{
	Dialog(RAD_ELLIPSE);
}

static void AddFillEllipse(FGControl *)
{
	Dialog(RAD_FILLELLIPSE);
}

static void AddArc(FGControl *)
{
	Dialog(RAD_ARC);
}

static void AddCircle(FGControl *)
{
	Dialog(RAD_CIRCLE);
}

static void AddFillCircle(FGControl *)
{
	Dialog(RAD_FILLCIRCLE);
}

static void AddPush1(FGControl *)
{
	Dialog(RAD_PUSHBUTTON1);
}

static void AddCheck(FGControl *)
{
	Dialog(RAD_CHECKBUTTON);
}

static void AddPoint(FGControl *)
{
	Dialog(RAD_POINTBUTTON);
}

static void AddEditBox1(FGControl *)
{
	Dialog(RAD_EDITBOX1);
}

static void AddEditBox2(FGControl *)
{
	Dialog(RAD_EDITBOX2);
}

static void AddEditBox3(FGControl *)
{
	Dialog(RAD_EDITBOX3);
}

static void AddSlideH(FGControl *)
{
	Dialog(RAD_SLIDEBARH);
}

static void AddSlideV(FGControl *)
{
	Dialog(RAD_SLIDEBARV);
}

static void AddProgressBar(FGControl *)
{
	Dialog(RAD_PROGRESSBAR);
}

static void AddListBox(FGControl *)
{
	Dialog(RAD_LISTBOX);
}

static void AddLabel(FGControl *)
{
	Dialog(RAD_LABEL);
}

static void AddMenu(FGControl *)
{
	prj->Okno[prj->curr].flags |= WMENU;
	flg.menu = 1;
	pb5->ChangeItem(&flg.menu);
	Dialog(RAD_MENU);
}

static void BGroups(FGControl *)
{
	current_accel->flags^=ACCF_BGROUP;
	if (current_accel->flags&ACCF_BGROUP)
	{
		eb_min->SetName("GRP");
		eb_min->Enable();
	}
	else
	{
		eb_min->SetName("MIN");
		eb_min->Disable();
	}
}

static void Dialog(RadType r)
{
	Wind *p = &prj->Okno[prj->curr];
	ShowDialog(r, &p->table[p->items], 0);
}

static void FGControlItem(FGControl *)
{
	if (!prj) return;
	MenuWindow *m1Wnd = new MenuWindow(110, 294-22+13*2);
	m1Wnd->AddMenu("Push Button",'P',AddPush1);
	m1Wnd->AddMenu("Check Button",'C',AddCheck);
	m1Wnd->AddMenu("Point Button",'O',AddPoint);
	m1Wnd->AddMenu("Label",'a',AddLabel);
	m1Wnd->Separator();
	m1Wnd->AddMenu("Edit Box [int]",'E',AddEditBox1);
	m1Wnd->AddMenu("Edit Box [float]",'D',AddEditBox2);
	m1Wnd->AddMenu("Edit Box [string]",'B',AddEditBox3);
	m1Wnd->Separator();
	m1Wnd->AddMenu("SlideBarH",'S',AddSlideH);
	m1Wnd->AddMenu("SlideBarV",'V',AddSlideV);
	m1Wnd->Separator();
	m1Wnd->AddMenu("Menu",'M',AddMenu);
	m1Wnd->Separator();
	m1Wnd->AddMenu("ListBox",'L',AddListBox);
	m1Wnd->AddMenu("ProgressBar", 'g', AddProgressBar);
	m1Wnd->AddMenu("TabPage",'d',AddTabPage);
}

static void AddWindow(FGControl *)
{
	prj->AddWindow(0);
}

static void RemoveWindow(FGControl *)
{
	prj->DeleteWindow();
}

static void DuplicateWindow(FGControl *)
{
	prj->AddWindow(1);
}

static void WindowItem(FGControl *)
{
	MenuWindow *m1Wnd = new MenuWindow(120,68);
	m1Wnd->AddMenu("Add Window",'A',AddWindow);
	m1Wnd->AddMenu("Remove Window",'R',RemoveWindow);
	m1Wnd->AddMenu("Duplicate Window  ^D",'D',DuplicateWindow);
}

static void ShapeItem(FGControl *)
{
	MenuWindow *m1Wnd = new MenuWindow(90,206);
	m1Wnd->AddMenu("Text string",'T',AddString);
	m1Wnd->AddMenu("Box",'B',AddBox);
	m1Wnd->AddMenu("Rect",'R',AddRect);
	m1Wnd->AddMenu("Line",'L',AddLine);
	m1Wnd->AddMenu("Circle",'C',AddCircle);
	m1Wnd->AddMenu("Fill Circle",'i',AddFillCircle);
	m1Wnd->AddMenu("Ellipse",'l',AddEllipse);
	m1Wnd->AddMenu("Fill Ellipse",'p',AddFillEllipse);
	m1Wnd->AddMenu("Arc",'a',AddArc);
	m1Wnd->AddMenu("Panel",'d',AddPanel);
}

static void OKProc(FGEvent *p)
{
	switch(p->GetType()) {
		case INITEVENT:
			p->wnd->WindowText(24, 24, "Code has been generated succesfully!", CBLUE);
#ifdef __linux__
			p->wnd->WindowText(24, 44, "USE: gcc -fPIC file.cpp -o file -lfgl", CBLUELIGHT);
#endif
#ifdef __MSDOS__
#ifdef __WATCOMC__
			p->wnd->WindowText(24, 44, "USE: wcc386 file.cc & wlink (as DOS4G) ", CBLUELIGHT);
#else
			p->wnd->WindowText(24, 44, "USE: gcc file.cpp -o file -lfgl ", CBLUELIGHT);
#endif
#endif
#ifdef __QNX__
			p->wnd->WindowText(24, 44, "USE: wcc386 file.cpp & wlink (with libfgl) ", CBLUELIGHT);
#endif
			p->wnd->WindowText(24, 64, "        Press any key ...", CBLUE);
			break;
		case KEYEVENT:
		case CLICKLEFTEVENT:
		case CLICKRIGHTEVENT:
			delete p->wnd;
			break;
	}
}

static void GenerateCode(FGControl *)
{
	if (Compile(prj))
	{
		remove(srcname);
		isCode = 0;
	}
	else
	{
		OKPtr = new Window(&OKPtr, 200, 363, 450, 132, "OK", OKProc, 0, CWHITE, 0x223|WMODAL);
	}
}

static void ViewCode(FGControl *)
{
	if (isCode)
	{
		if (editor) editor->OpenBuffer(srcname);
		else editor = new TextEditor(&editor, srcname, FONT0816, CYELLOW,CBLUE);
	}
}

void SetGran(FGControl *)
{
	Set_GranularityPtr = new Window(&Set_GranularityPtr, 288, 678, 188, 108, "Set Granularity", 0, CBLACK, rad_paper, WFRAMED|WTITLED|WESCAPE);
	Set_GranularityPtr->AddEditBox(8, 8, 120, 40, "Grid size X:", 'x', &granularity_x, 0, 1, 512);
	Set_GranularityPtr->AddEditBox(8, 40, 120, 40, "Grid size Y:", 'y', &granularity_y, 0, 1, 512);
}

static void Testing(CallBack)
{
	if (test) CScheme = &prj->cscheme;
	else CScheme = old_scheme;
	Redraw(0);
}

static void OptionsItem(FGControl *)
{
	MenuWindow *m1Wnd = new MenuWindow(148,228+22);
	m1Wnd->AddPointButton("Enable ALT+X",'X', &prj->app_altx);
	m1Wnd->AddPointButton("Enable cfg",'C', &prj->app_cfg);
	m1Wnd->AddPointButton("Enable magnify",'M', &prj->app_magnify);
	m1Wnd->AddPointButton("Enable wnd saving",'W', &prj->app_wnd);
	m1Wnd->AddPointButton("Enable rootwindow",'R', &prj->app_root);
	m1Wnd->AddPointButton("Add comments",'A', &verb);
	m1Wnd->Separator();
	m1Wnd->AddMenu("Video mode ...",'V', Video_mode);
	m1Wnd->AddMenu("Background ...",'B', Back_color);
	m1Wnd->AddMenu("Font ...",'F', SetFont);
	m1Wnd->Separator();
	m1Wnd->AddMenu("Granularity ...",'G', SetGran);
	m1Wnd->Separator();
	m1Wnd->AddPointButton("Test mode", 'T', &test, Testing);
}

static void NewPrj(FGControl *)
{
	delete prj;
	prj = new Projekt();
	prj->Update();
	disable_all();
	prj->Redraw(prj->Okno);
}

static void OpenPrj2(char *s, FileDialog *fd)
{
	delete prj;
	prj = new Projekt(s, fd->GetName());
	prj->Update();
}

static void OpenPrj(FGControl *)
{
	(void)new FileDialog(OpenPrj2, 0, ".wnd", "Open Projekt", FDIALOG_MODAL | FDIALOG_SAVEDIR, rad_ink, rad_paper);
}

static void SavePrj2(char *s, FileDialog *fd)
{
	char *ss = fd->GetName();
	if (strstr(ss, ".wnd")) strcpy(prj->prjname, ss);
	else
	{
		sprintf(prj->prjname, "%s.wnd", ss);
	}
	prj->Save();
	char *n=strdup(prj->prjname);
	delete prj;
	prj = new Projekt(n, n);
	prj->Update();
	free(n);
}

static void SavePrj(FGControl *)
{
	prj->Save();
}

static void SaveAsPrj(FGControl *)
{
	(void)new FileDialog(SavePrj2, 0, ".wnd", "Save Projekt as ..", FDIALOG_MODAL | FDIALOG_SAVEDIR | FDIALOG_SAVE, rad_ink, rad_paper);
}

static void Forward2(FGControl *)
{
	prj->Forward();
}

static void Backward2(FGControl *)
{
	prj->Backward();
}

static void Delete2(FGControl *)
{
	prj->DeleteItem();
}

static void Clone2(FGControl *)
{
	prj->Clone();
}

static void EditMenu(FGControl *)
{
	MenuWindow *m1Wnd = new MenuWindow(90, 90);
	m1Wnd->AddMenu("Forward   ->", 'F', Forward2);
	m1Wnd->AddMenu("Backward  <-",'B', Backward2);
	m1Wnd->AddMenu("Clone     ^C",'C', Clone2);
	m1Wnd->AddMenu("Delete    DEL",'D', Delete2);
}

static void FileMenu(FGControl *)
{
	MenuWindow *m1Wnd = new MenuWindow(90, 136);
	m1Wnd->AddMenu("New           F4",'N',NewPrj);
	m1Wnd->AddMenu("Open         F3",'O',OpenPrj);
	m1Wnd->AddMenu("Save         F2",'S',SavePrj);
	m1Wnd->AddMenu("Save As    F5",'A',SaveAsPrj);
	m1Wnd->Separator();
	m1Wnd->AddMenu("About",'B',About);
	m1Wnd->AddMenu("Exit",'x',ControlCall(App::AppDone));
}

//void flg::CreatePaletteEntry(int rc, int gc, int bc, int idx);

static void ChangeColor(FGControl *)
{
#ifdef INDEX_COLORS
//	set_colors(0,3);
	CreatePaletteEntry(rrr,ggg,bbb,ccc);
#endif
}

static FGListBox *widgetPtr0;
static SlideBarH *sl1, *sl2, *sl3;

static void xcolProc(FGEvent *p)
{
	int c;
	switch(p->GetType())
	{
		case CLICKLEFTEVENT:
			c = widgetPtr0->Test(p->GetX(), p->GetY());
			if (c>0) widgetPtr0->SetToItem(c);
			prj->paleta[ccc] = xcolors[widgetPtr0->GetCurrent()]->p;
			rrr = prj->paleta[ccc].r;
			ggg = prj->paleta[ccc].g;
			bbb = prj->paleta[ccc].b;
			sl1->draw();
			sl2->draw();
			sl3->draw();
			delete p->wnd;
			break;
		case KEYEVENT:
			switch(p->GetKey())
			{
				case KUP:
					widgetPtr0->Up();
					break;
				case KDOWN:
					widgetPtr0->Down();
					break;
				case CR:
					prj->paleta[ccc] = xcolors[widgetPtr0->GetCurrent()]->p;
					rrr = prj->paleta[ccc].r;
					ggg = prj->paleta[ccc].g;
					bbb = prj->paleta[ccc].b;
					sl1->draw();
					sl2->draw();
					sl3->draw();
					delete p->wnd;
					break;
			}
	}
}

static void ShowPalEntry(int flag, Window *w, int x, int y, void *data, int index)
{
	if (flag)
	{
#ifdef INDEX_COLORS
		vector_palette(ccc, *(unsigned *)&xcolors[index]->p);
#endif
		w->WindowText(x,y,xcolors[index]->s, CRED);
	}
	else w->WindowText(x,y,xcolors[index]->s,0);
}

static void Predefined(FGControl *)
{
	nxcolors=sizeof(xcolors)/sizeof(XColors);
	MenuWindow *w = new MenuWindow(172+16, 300+32,xcolProc);
	widgetPtr0 = w->AddListBox(2, 2, 160, 18, 16, ShowPalEntry);
	widgetPtr0->SetSize(nxcolors);
}

static void ColorsManagementProc(FGEvent *p)
{
	static PushButton *pb1, *pb2;
	int	xWnd = p->GetX()-8;
	int	yWnd = p->GetY()-80;
	int i,j;
	switch(p->GetType())
	{
		case TERMINATEEVENT:
			if (color_settings) *color_settings = ccc;
			color_settings = 0;
			break;
		case INITEVENT:
			p->wnd->AddBaseMenu("Predefined Colors",'p',Predefined);
			rrr = prj->paleta[ccc].r;
			ggg = prj->paleta[ccc].g;
			bbb = prj->paleta[ccc].b;
			sl1 = p->wnd->AddSlideBarH(8, 4,  0, 255, 4, &rrr, ChangeColor);
			sl2 = p->wnd->AddSlideBarH(8, 20, 0, 255, 4, &ggg, ChangeColor);
			sl3 = p->wnd->AddSlideBarH(8, 36, 0, 255, 4, &bbb, ChangeColor);
			p->wnd->WindowBox(140, 4, 48, 48, ccc);
			p->wnd->set_colors(15, rad_paper);
			p->wnd->printf(200, 24, "%3d\n", ccc);
			pb1 = p->wnd->AddPushButton(236, 4,  88, 21, "Add Color", 'A');
			pb2 = p->wnd->AddPushButton(236, 32, 88, 21, "Del Color", 'D');
			for(i=0; i<16; i++) for(j=0; j<16; j++)
			{
				if ((j+i*16) == ccc) p->wnd->WindowRect(8+j*20-2, 64+i*20-2, 20, 20, 0);
				if (prj->paleta[(j+i*16)].alfa || !i) p->wnd->WindowRect(8+j*20-1, 64+i*20-1, 18, 18, CYELLOW);
				p->wnd->WindowBox(8+j*20,  64+i*20, 16, 16, j+i*16);
				p->wnd->WindowRect(8+j*20, 64+i*20, 16, 16, 0);
			}
			break;
		case CLICKLEFTEVENT:
			if (yWnd<0 || xWnd<0) return;
			xWnd = xWnd/20;
			yWnd = yWnd/20;
			if (yWnd>14 || xWnd>15) return;
			p->wnd->WindowRect(8+(ccc%16)*20-2, 64+(ccc/16)*20-2, 20, 20, 3);
			ccc = xWnd + yWnd*16 + 16;
			p->wnd->set_colors(15,rad_paper);
			p->wnd->printf(200, 24, "%3d\n", ccc);
			p->wnd->WindowBox(140, 4, 48, 48, ccc);
			p->wnd->WindowRect(8+(ccc%16)*20-2, 64+(ccc/16)*20-2, 20, 20, 0);
			rrr = prj->paleta[ccc].r;
			ggg = prj->paleta[ccc].g;
			bbb = prj->paleta[ccc].b;
			sl1->draw();
			sl2->draw();
			sl3->draw();
			break;
		case ACCELEVENT:
			if (p->GetKey() == pb1->GetId())
			{
				prj->paleta[ccc].alfa = 1;
				prj->paleta[ccc].r = rrr;
				prj->paleta[ccc].g = ggg;
				prj->paleta[ccc].b = bbb;
				p->wnd->WindowRect(8+(ccc%16)*20-1, 64+(ccc/16)*20-1, 18, 18, CYELLOW);
			}
			else if (p->GetKey() == pb2->GetId())
			{
				prj->paleta[ccc].alfa = 0;
				p->wnd->WindowRect(8+(ccc%16)*20-1, 64+(ccc/16)*20-1, 18, 18, 3);
			}
			break;
	}
}

static void ColorsManagement(FGControl *)
{
	if (ColorsManagementPtr) return;
	ColorsManagementPtr = new Window(&ColorsManagementPtr, 240, 80, 340, 440, "Color Managements", ColorsManagementProc, 0, rad_paper, 0x203|WUSELAST|WMENU);
}

static void RedrawOkno(FGControl *)
{
	unsigned long f=0;
	if (flg.title)	f|=WTITLED;
	if (flg.frame)	f|=WFRAMED;
	if (flg.modal)	f|=WMODAL;
	if (flg.solid)	f|=WNOTIFY;
	if (flg.nomove)	f|=WUNMOVED;
	if (flg.menu)	f|=WMENU;
	if (flg.nopicto)	f|=WNOPICTO;
	if (flg.clickable)	f|=WCLICKABLE;
	if (flg.focus)	f|=WLASTFOCUS;
	if (flg.sizeable)	f|=WSIZEABLE;
	if (flg.uselast)	f|=WUSELAST;
	if (flg.statusbar)	f|=WSTATUSBAR;
	if (flg.center)		f|=WCENTRED;
	if (flg.escape)		f|=WESCAPE;
	if (flg.notify)		f|=WNOTIFY;
	if (flg.fastmove)		f|=WFASTMOVE;
	if (flg.minimize)		f|=WMINIMIZE;
	if (flg.notebook)		f|=WNOTEBOOK;
	prj->Okno[prj->curr].flags = f;
	if (prj) prj->Redraw(&prj->Okno[prj->curr]);
}

static void RedrawOkno2(FGControl *)
{
	RedrawOkno(0);
	lBox2->RedrawItem();
}

void DrawLBox(int f, Window *,  int x,int y, void *, int idx)
{
	int i,p;
	if (!f)
	{
		p = PM;
		i = IM;
	}
	else
	{
		i = PM;
		p = IM;
	}
	mWnd->WindowBox(x,y,160,16,p);
	sprintf(gps,"%s:%s", rad_str[prj->Okno[prj->curr].table[idx].type], prj->Okno[prj->curr].table[idx].name);
	gps[20]=0; //be care
	gps[21]=0; //be care
	mWnd->WindowText(x,y,gps,i,p);
	if (f) ShowDialog((RadType)0, &prj->Okno[prj->curr].table[idx]);
}

void DrawLBox2(int f, Window *,  int x,int y, void *, int idx)
{
	int i,p;
	if (!f)
	{
		p = PM;
		i = IM;
	}
	else
	{
		i = PM;
		p = IM;
	}
	mWnd->WindowBox(x,y,96,16,p);
	sprintf(gps,"%s", prj->Okno[idx].name);
	gps[12]=0; 	//be care
	gps[13]=0; 		//be care
	mWnd->WindowText(x,y,gps,i,p);
}

PushButton *pb_sel, *pb_del;

void disable_all(void)
{
	eb_x->Disable();
	eb_y->Disable();
	eb_w->Disable();
	eb_h->Disable();
	eb_size->Disable();
	eb_max->Disable();
	eb_min->Disable();
	eb_lins->Disable();
	eb_lins->Disable();
	eb_rows->Disable();
	eb_name->Disable();
	eb_var->Disable();
	eb_fnc->Disable();
	pb_key->Disable();
	pb_ink->Disable();
	pb_paper->Disable();
	pb_val->Disable();
	pt_grp->Disable();
	pt_trans->Disable();
	pt_check->Disable();
	pt_rc->Disable();
}

void mainWnd(FGEvent *p)
{
	int k,x,y;
	Wind *w;

	x = p->GetX();
	y = p->GetY();
	k = p->GetKey();

	switch(p->GetType())
	{
		case MOVEEVENT:
			mWnd->set_colors(IM,rad_paper);
			mWnd->printf(928, 138, "%4d:%4d\n", p->GetX(), p->GetY());
			break;
		case CLICKLEFTEVENT:
			k = lBox->Test(x,y);
			if (k != -1)
			{
				lBox->SetToItem(k);
				ShowDialog((RadType)0, &prj->Okno[prj->curr].table[lBox->GetCurrent()]);
				p->wnd->WindowFocus();
				break;
			}
			k = lBox2->Test(x,y);
			if (k != -1)
			{
				lBox2->SetToItem(k);
				prj->curr = k;
				prj->Update();
				prj->Redraw(prj->wind());
			}
			p->wnd->WindowFocus();
			break;
		case KEYEVENT:
			w = prj->Okno+prj->curr;
			if (k==KUP)
			{
				if (w->current != -1)
				{
					SelectCurrent(w);
				}
				lBox->Up();
				w->current = lBox->GetCurrent();
				SelectCurrent(w);
			}
			else if (k==KDOWN)
			{
				if (w->current != -1)
				{
					SelectCurrent(w);
				}
				lBox->Down();
				w->current = lBox->GetCurrent();
				SelectCurrent(w);
			}
			else if (k==INSERT) AddWindow(0);
			else if (k==PGUP) prj->Up();
			else if (k==PGDOWN)	prj->Down();
			else if (k==KRIGHT)   Forward2(0);
			else if (k==KLEFT) Backward2(0);
			else if (k==F02)  SavePrj(0);
			else if (k==F03)  OpenPrj(0);
			else if (k==F03)  NewPrj(0);
			else if (k==F05)  SaveAsPrj(0);
			else if (k==F08)  ViewCode(0);
			else if (k==F09)  GenerateCode(0);
			else if (k==DEL)  Delete2(0);
			else if (k==CR && lBox->GetSize())
			{
				ShowDialog((RadType)0, &prj->Okno[prj->curr].table[lBox->GetCurrent()]);
			}
			else if (k==ALT_D) DuplicateWindow(0);
			else if (k==ALT_C) Clone2(0);
			break;
		case TERMINATEEVENT:
			break;
		case INITEVENT:
			mWnd->set_font(FONT0816);

			mWnd->AddBaseMenu("File ", ALT_F, FileMenu);
			mWnd->AddBaseMenu("Editor ", ALT_E, ViewCode);
			mWnd->AddBaseMenu("Compile ", ALT_M, GenerateCode);
			mWnd->AddBaseMenu("Arrange ", ALT_A, EditMenu);
			mWnd->AddBaseMenu("Options ", ALT_O, OptionsItem);
			mWnd->AddBaseMenu("Windows ", ALT_W, WindowItem);
			mWnd->AddBaseMenu("FGControls ", ALT_N, FGControlItem);
			mWnd->AddBaseMenu("Shapes ", ALT_S, ShapeItem);
			mWnd->AddBaseMenu("Colors ", ALT_L, ColorsManagement);
			mWnd->AddBaseMenu("ColorSchemes ", ALT_H, Schemes);
			mWnd->AddBaseMenu("Defaults ", ALT_U, SetValue);

			eb0 = mWnd->AddEditBox(0,  8,72,256,"Name:" ,0,prj->Okno[prj->curr].name,RedrawOkno2);
			eb1 = mWnd->AddEditBox(0, 32,72,48,"X-coor:",0,&prj->Okno[prj->curr].x,RedrawOkno,0,2000);
			eb2 = mWnd->AddEditBox(0, 56,72,48,"Y-coor:",0,&prj->Okno[prj->curr].y,RedrawOkno,0,2000);
			eb3 = mWnd->AddEditBox(120,32,72,48,"Width:",0,&prj->Okno[prj->curr].w,RedrawOkno,0,1600);
			eb4 = mWnd->AddEditBox(120,56,72,48,"Height:",0,&prj->Okno[prj->curr].h,RedrawOkno,0,1200);
			mWnd->AddPushButton(256,32,72,21,"Ink",0,SetInkW);
			mWnd->AddPushButton(256,56,72,21,"Paper",0, SetPaperW);
			eb0->SetSize(MAXNAME);

			pb0 = mWnd->AddPointButton(8,83,"Title",0,&flg.title,RedrawOkno);
			pb1 = mWnd->AddPointButton(8,83+21*1,"Frame",0,&flg.frame,RedrawOkno);
			pb2 = mWnd->AddPointButton(8,83+21*2,"Modal",0,&flg.modal,RedrawOkno);
			pb3 = mWnd->AddPointButton(8,83+21*3,"FastMove",0,&flg.fastmove,RedrawOkno);
			pb4 = mWnd->AddPointButton(8,83+21*4,"No-move",0,&flg.nomove,RedrawOkno);
			pb10= mWnd->AddPointButton(8,83+21*5,"Save XY",0,&flg.uselast,RedrawOkno);

			pb5 = mWnd->AddPointButton(100,83+21*0,"Menu",0,&flg.menu,RedrawOkno);
			pb6 = mWnd->AddPointButton(100,83+21*1,"No-picto",0,&flg.nopicto,RedrawOkno);
			pb7 = mWnd->AddPointButton(100,83+21*2,"Clickable",0,&flg.clickable, RedrawOkno);
			pb8 = mWnd->AddPointButton(100,83+21*3,"Last focus",0,&flg.focus,RedrawOkno);
			pb9 = mWnd->AddPointButton(100,83+21*4,"Sizeable",0,&flg.sizeable,RedrawOkno);
			pb15= mWnd->AddPointButton(100,83+21*5,"Minimize",0,&flg.minimize,RedrawOkno);

			pb11= mWnd->AddPointButton(210,83+21*0,"Statusbar",0,&flg.statusbar,RedrawOkno);
			pb12= mWnd->AddPointButton(210,83+21*1,"Centred",0,&flg.center, RedrawOkno);
			pb13= mWnd->AddPointButton(210,83+21*2,"Escape",0,&flg.escape, RedrawOkno);
			pb14= mWnd->AddPointButton(210,83+21*3,"Notify",0,&flg.notify, RedrawOkno);
			pb16= mWnd->AddPointButton(210,83+21*4,"Notebook",0,&flg.notebook, RedrawOkno);

// dlg options
			eb_x = p->wnd->AddEditBox(635, 6, 48, 48, "X:", 'X', &cAcc.x, Redraw, 0, 1600);
			eb_y = p->wnd->AddEditBox(635, 30, 48, 48, "Y:", 'Y', &cAcc.y, Redraw, 0, 1600);
			eb_w = p->wnd->AddEditBox(732, 6, 40, 48, "W:", 'W', &cAcc.w, Redraw, 0, 1600);
			eb_h = p->wnd->AddEditBox(732, 30, 40, 48, "H:", 'H', &cAcc.h, SetBoxSize2, 0, 1600);
			eb_name = p->wnd->AddEditBox(635, 180, 48, 320, "NAME:", 'N', cAcc.name, Redraw);
			pb_paper = p->wnd->AddPushButton(936, 30, 72, 21, "PAPER:", 'P', SetPaper);
			pb_ink = p->wnd->AddPushButton(936, 6, 72, 21, "INK:", 'K',  SetInk);
			eb_min = p->wnd->AddEditBox(635, 57, 48, 140, "MIN:", 'I', &cAcc.min);
			eb_max = p->wnd->AddEditBox(635, 83, 48, 140, "MAX:", 'A', &cAcc.max);
			eb_var = p->wnd->AddEditBox(635, 133, 48, 140, "VAR:", 'V', cAcc.variable);
			eb_name->SetSize(MAXNAME);
			eb_var->SetSize(MAXVAR);
			pb_key = p->wnd->AddPushButton(936, 54, 72, 21, "KEY", 'K', SetKey);
			pb_val = p->wnd->AddPushButton(936, 102, 72, 21, "VALUE", 'U', SetValue);
			pt_grp = p->wnd->AddCheckButton(833, 80, "GROUP:",'G', (int *)0, BGroups);
			pt_trans = p->wnd->AddCheckButton(833, 100, "TRANSP:", 'T', (int *)0, RedrawTransp);
			pt_check = p->wnd->AddCheckButton(833, 120, "CHECK:", 'C', (int *)0, RangeCheck);
			pt_rc = p->wnd->AddCheckButton(833, 140, "CONFIG:", 'O', (int *)0, AddToRc);
			eb_fnc = p->wnd->AddEditBox(635, 108, 48, 138, "FNC:", 'F', cAcc.fnc);
			eb_size = p->wnd->AddEditBox(832, 54, 48, 48, "SIZE", 'Z', &cAcc.ww, SetBoxSize, 1, 127);
			eb_rows = p->wnd->AddEditBox(832, 6, 48, 48, "ROWS:", 'R', &cAcc.ww, Redraw, 1, 1000000);
			eb_lins = p->wnd->AddEditBox(832, 30, 48, 48, "LINES:", 'L', &cAcc.hh, Redraw, 1, 1600);
			eb_fnc->SetSize(MAXVAR);

			disable_all();

			lBox = mWnd->AddListBox(456, 6,160,16,12,DrawLBox);
			lBox->SetSize(prj->Okno[prj->curr].items);
			lBox2= mWnd->AddListBox(336, 6,96,16,12,DrawLBox2, 0);
			lBox2->SetSize(prj->nwin);
			lBox2->SetToItem(prj->curr);
			break;
	}
}

void Projekt::Update(void)
{
	flg.title =	prj->Okno[prj->curr].flags&WTITLED?1:0;
	flg.frame =	prj->Okno[prj->curr].flags&WFRAMED?1:0;
	flg.modal =	prj->Okno[prj->curr].flags&WMODAL?1:0;
	flg.solid =	prj->Okno[prj->curr].flags&WNOTIFY?1:0;
	flg.nomove=	prj->Okno[prj->curr].flags&WUNMOVED?1:0;
	flg.menu  =	prj->Okno[prj->curr].flags&WMENU?1:0;
	flg.nopicto=prj->Okno[prj->curr].flags&WNOPICTO?1:0;
	flg.clickable=prj->Okno[prj->curr].flags&WCLICKABLE?1:0;
	flg.focus =	prj->Okno[prj->curr].flags&WLASTFOCUS?1:0;
	flg.sizeable=prj->Okno[prj->curr].flags&WSIZEABLE?1:0;
	flg.uselast=prj->Okno[prj->curr].flags&WUSELAST?1:0;
	flg.statusbar=prj->Okno[prj->curr].flags&WSTATUSBAR?1:0;
	flg.center=prj->Okno[prj->curr].flags&WCENTRED?1:0;
	flg.escape=prj->Okno[prj->curr].flags&WESCAPE?1:0;
	flg.fastmove=prj->Okno[prj->curr].flags&WFASTMOVE?1:0;
	flg.notify=prj->Okno[prj->curr].flags&WNOTIFY?1:0;
	flg.minimize=prj->Okno[prj->curr].flags&WMINIMIZE?1:0;
	flg.notebook=prj->Okno[prj->curr].flags&WNOTEBOOK?1:0;

	if (mWnd==0)
	{
		sprintf(gps, "OpenGUI Sourcer %d.%d - %s      ", VERSION/100, (VERSION%100), prjname);
		mWnd = new Window(&mWnd, 0,0, 1024, DBFHEIGHT, gps, mainWnd,rad_ink,rad_paper,WFRAMED|WMENU|WTITLED|WCLICKABLE|WNOPICTO);
	}
	else // update only
	{
		sprintf(gps, "OpenGUI Sourcer %d.%d - %s      ", VERSION/100, (VERSION%100), prjname);
		mWnd->SetName(gps);
		eb0->ChangeItem(prj->Okno[prj->curr].name);

		eb1->ChangeItem(&prj->Okno[prj->curr].x);
		eb2->ChangeItem(&prj->Okno[prj->curr].y);
		eb3->ChangeItem(&prj->Okno[prj->curr].w);
		eb4->ChangeItem(&prj->Okno[prj->curr].h);

		pb0->ChangeItem(&flg.title);
		pb1->ChangeItem(&flg.frame);
		pb2->ChangeItem(&flg.modal);
		pb3->ChangeItem(&flg.fastmove);
		pb4->ChangeItem(&flg.nomove);
		pb5->ChangeItem(&flg.menu);
		pb6->ChangeItem(&flg.nopicto);
		pb7->ChangeItem(&flg.clickable);
		pb8->ChangeItem(&flg.focus);
		pb9->ChangeItem(&flg.sizeable);
		pb10->ChangeItem(&flg.uselast);
		pb11->ChangeItem(&flg.statusbar);
		pb12->ChangeItem(&flg.center);
		pb13->ChangeItem(&flg.escape);
		pb14->ChangeItem(&flg.notify);
		pb15->ChangeItem(&flg.minimize);
		pb16->ChangeItem(&flg.notebook);
		lBox->SetSize(prj->Okno[prj->curr].items);
		lBox2->SetSize(prj->nwin);
	}
	if (lBox->GetSize()) lBox->SetToItem(0);
	lBox2->SetToItem(prj->curr);
}

int AppProc(FGEvent *p)
{
	if (p->GetType() == MOVEEVENT)
	{
		if (mWnd) mWnd->SendToWindow(p);
	}
	return 0;
}

static void OnSecond(int)
{
	char *buf;
	time_t t = time(0);
	buf = ctime(&t);
	buf[24]=32;
	buf[25]=0;
	App::GetRootWindow()->set_font(2);
	App::GetRootWindow()->WindowText(GetXRes()-27*8,GetYRes()-32,buf,CGREEN, CBLACK);
}

Window *Color_SchemePtr;

static void OldScheme(CallBack)
{
	memcpy(&prj->cscheme, old_scheme, sizeof(FGColorScheme));
	Redraw(0);
}

static void SetCScheme(CallBack t)
{
	color_settings = (int *)(&prj->cscheme)+t->GetLocalId();
	SetColors("Color Dialog");
}

static void Color_SchemeProc(FGEvent *p)
{
	switch(p->GetType()) {
		case INITEVENT:
			p->wnd->AddPushButton(8, 8, 128, 21, "Window Back", 'b', SetCScheme);
			p->wnd->AddPushButton(8, 40, 128, 21, "Window Fore", 'f', SetCScheme);
			p->wnd->AddPushButton(8, 72, 128, 21, "Active Title", 'a', SetCScheme);
			p->wnd->AddPushButton(8, 104, 128, 21, "Inactive Title", 'i', SetCScheme);

			p->wnd->AddPushButton(8, 136, 128, 21, "Wnd Border 1", '1', SetCScheme);
			p->wnd->AddPushButton(8, 168, 128, 21, "Wnd Border 2", '2', SetCScheme);
			p->wnd->AddPushButton(8, 200, 128, 21, "Wnd Border 3", '3', SetCScheme);
			p->wnd->AddPushButton(8, 232, 128, 21, "Status Bar", 'b', SetCScheme);

			p->wnd->AddPushButton(152, 8, 128, 21, "Menu Back", 'b', SetCScheme);
			p->wnd->AddPushButton(152, 40, 128, 21, "Menu Fore", 'b', SetCScheme);
			p->wnd->AddPushButton(152, 72, 128, 21, "Menu Back Active", 'b', SetCScheme);
			p->wnd->AddPushButton(152, 104, 128, 21, "Menu Fore Active", 'b', SetCScheme);

			p->wnd->AddPushButton(296, 40, 128, 21, "Button Fore", 'b', SetCScheme);
			p->wnd->AddPushButton(296, 8, 128, 21, "Button Back", 'b', SetCScheme);
			p->wnd->AddPushButton(440, 104, 200, 21, "Button Fore Active", 'b', SetCScheme);
			p->wnd->AddPushButton(440, 136, 200, 21, "Button Back Active", 'b', SetCScheme);

			p->wnd->AddPushButton(296, 72, 128, 21, "Button Border1", '1', SetCScheme);
			p->wnd->AddPushButton(296, 104, 128, 21, "Button Border2", '1', SetCScheme);
			p->wnd->AddPushButton(296, 136, 128, 21, "Button Border3", '1', SetCScheme);

			p->wnd->AddPushButton(152, 136, 128, 21, "EditBox Back", '1', SetCScheme);
			p->wnd->AddPushButton(152, 168, 128, 21, "EditBox Fore", '1', SetCScheme);
			p->wnd->AddPushButton(152, 200, 128, 21, "EditBox Bord1", '1', SetCScheme);
			p->wnd->AddPushButton(152, 232, 128, 21, "EditBox Bord2", '1', SetCScheme);

			p->wnd->AddPushButton(296, 232, 128, 21, "SLider", '1', SetCScheme);
			p->wnd->AddPushButton(296, 168, 128, 21, "MenuWnd Back", 'b', SetCScheme);
			p->wnd->AddPushButton(296, 200, 128, 21, "MenuWnd Fore", 'b', SetCScheme);

			p->wnd->AddPushButton(440, 8, 200, 21, "MenuWnd Back Active", 'b', SetCScheme);
			p->wnd->AddPushButton(440, 40, 200, 21, "MenuWnd Fore Active", 'b', SetCScheme);
			p->wnd->AddPushButton(440, 72, 200, 21, "MenuWnd Fore Disabled", 'b', SetCScheme);
			p->wnd->AddPushButton(440, 168, 200, 21, "EditBox Disabled", '1', SetCScheme);
			p->wnd->AddPushButton(440, 232, 200, 21, "Set defaults ...", '1', OldScheme);
			break;
	}
}

void Schemes(CallBack)
{
	if (Color_SchemePtr) Color_SchemePtr->WindowFocus();
	else Color_SchemePtr = new Window(&Color_SchemePtr, 16, 272, 656, 290, "Color Scheme", Color_SchemeProc, rad_ink, rad_paper, WFRAMED|WTITLED|WCLICKABLE|WESCAPE);
}

static void show_value(int flag, Window *ww, int x, int y, void *, int index)
{
	char *typ[3]={"int    ", "double ",	"char * " };
	ww->WindowBox(x,y,120+215+150,16,Default_valuesPtr->GetPaper());
	sprintf(gps, "%19s  %s \"%s\"", prj->values[index].name,
		typ[prj->values[index].type-1], prj->values[index].value);
	ww->WindowText(x,y,gps,flag?rad_ink:CBLACK);
	if (flag)
	{
		ebdef->ChangeItem(prj->values[index].value);
		ebdef->SetParameter((void *)index);
	}
}

void DrawVal(CallBack cb)
{
	int typ = prj->values[(int)cb->GetParameter()].type, len;
	char *s;
	switch(typ)
	{
		case V_INTEGER:
			sprintf(prj->values[(int)cb->GetParameter()].value, "%d", atoi(prj->values[(int)cb->GetParameter()].value));
			break;
		case V_DOUBLE:
			sprintf(s=prj->values[(int)cb->GetParameter()].value, "%f", atof(prj->values[(int)cb->GetParameter()].value));
			len = strlen(s);
			len--;
			while(len>1)
			{
				if (s[len]=='0') s[len]=0;
				else break;
				len--;
			}
			break;
	}
	listboxPtr0->RedrawItem();
}

static void Default_valuesProc(FGEvent *p)
{
	int	x = p->GetX();
	int	y = p->GetY(), val;

	switch(p->GetType()) {
		case INITEVENT:
			ebdef = p->wnd->AddEditBox(16, 304, 16, 460, "", 0, prj->values[0].value, DrawVal);
			ebdef->SetSize(63);

			listboxPtr0 = p->wnd->AddListBox(14, 30, 120+215+150, 16, 16, show_value);
			listboxPtr0->SetSize(prj->val_num);
			listboxPtr0->SetIndex(current_value);

			p->wnd->AddPushButton(12, 8, 166, 21, "name");
			p->wnd->AddPushButton(178, 8, 66, 21, "type");
			p->wnd->AddPushButton(244, 8, 271, 21, "value");
			break;
		case KEYEVENT: switch(p->GetKey())
			{
				case CR:
					val = listboxPtr0->GetCurrent();
					ebdef->ClickUp(TRUE);
					break;
				default:
					listboxPtr0->DoListBox(p->GetKey());
					break;
			}
			break;
		case CLICKLEFTEVENT:
			val = listboxPtr0->Test(x,y);
			if (val<0) break;
			listboxPtr0->SetToItem(val);
			ebdef->ClickUp(TRUE);
			break;
		case TERMINATEEVENT:
			ebdef = 0;
			break;
	}
}

void BuildVariables(void)
{
	GenTable(prj);
	CleanupTable(prj, prj->val_num);
	ImportTable(prj, prj->val_num);
}

void SetValue(FGControl *)
{
	BuildVariables();
	if (prj->val_num==0) return;
	if (Default_valuesPtr) Default_valuesPtr->WindowFocus();
	else Default_valuesPtr = new FGWindow(&Default_valuesPtr, 152, 320, 536, 366, "Default values", Default_valuesProc, rad_ink, rad_paper, WFRAMED|WTITLED|WCLICKABLE|WUSELAST|WESCAPE);
}

int main(int argc, char **argv)
{
	int i;
	App MyApp(4,argc,argv,CBLACK,APP_ENABLEALTX|APP_WINDOWDATABASE|APP_MAGNIFIER|APP_CFG|APP_ROOTWINDOW);
	Window * root =MyApp.GetRootWindow();
 	old_scheme = CScheme;
    memcpy(&rad_sc, CScheme, sizeof(rad_sc));
	CScheme = &rad_sc;
	root->set_fcolor(CGRAY3);
	for(int j=0; j<GetYRes(); j+=8) for(i=0; i<GetXRes(); i+=8)
	{
		root->WindowPixel(i,j);
	}
	nullBmp = new Bitmap(48,48,CREDLIGHT);
	cCfg->ReadInt("granularity_x",granularity_x);
	cCfg->ReadInt("granularity_y",granularity_y);
	cCfg->ReadInt("verbose",verb);

	if (argc==2) prj = new Projekt(argv[1], argv[1]);
	else prj = new Projekt("form1.wnd", "form1.wnd");

	prj->Update();
 	for(i=0;i<754;i++) xcolors[i] = new XColors(FGPalette(xxcolors[i].u), xxcolors[i].s);
	MyApp.SetTimerProc(OnSecond);
#ifndef TRUE_COLORS
	CreateColor(255,255,0,CYELLOW);
#endif
	MyApp.Run(AppProc);

 	for(i=0;i<754;i++) delete xcolors[i];
	if (prj) delete prj;
	if (editor) delete editor;
	color_settings = 0;
	cCfg->WriteInt("granularity_x",granularity_x);
	cCfg->WriteInt("granularity_y",granularity_y);
	cCfg->WriteInt("verbose",verb);
	if (Bitmap_InventaryPtr) delete Bitmap_InventaryPtr;
	if (Set_GranularityPtr) delete Set_GranularityPtr;
	if (ColorsManagementPtr) delete ColorsManagementPtr;
	if (OKPtr) delete OKPtr;
	if (mWnd) delete mWnd;
	if (Set_KeyPtr) delete Set_KeyPtr;
	if (Color_SchemePtr) delete Color_SchemePtr;
	delete nullBmp;
	if (form1Ptr) delete form1Ptr;
	return 0;
}



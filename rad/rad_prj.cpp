//
// project
//

#include <fastgl/fastgl.h>
#include <fastgl/widgets.h>

#include "rad_def.h"
#include "rad_type.h"
#include "rad_sym.h"

#ifdef FG_NAMESPACE
using namespace fgl;
#endif


static FGPixel *tmpBmp=0;

// increment number of items in wform and update display & data
void Projekt::PridajWidget(int change_only)
{
	if (!change_only)
	{
		lBox->SetSize(Items()+1);
		lBox->SetToItem(Okno[curr].items++);
	} else lBox->RedrawItem();
	Redraw(&prj->Okno[prj->curr]);
}

static void dummy(int f, Window *p, int x, int y, void *data, int ind)
{
	p->set_colors(IM,PM);
	p->printf(x,y,"item %d\n", ind);
}

void Projekt::DrawWidget(Wind *w, int i)
{
	static int ii=1;
	static double dd=1;
	static char ss[256];
	int oldf=-1;
	Accel *p = w->table+i;
	Bitmap *b;

	if (prj->font) w->form->set_font(prj->font-1);

	switch(p->type)
	{
		case RAD_TABPAGE:
			w->form->AddTabPage(p->name);
			break;
		case RAD_PANEL:
			w->form->AddPanel(p->x, p->y, p->w, p->h, p->name);
			break;
		case RAD_PUSHBUTTON1:
			w->form->AddPushButton(p->x, p->y, p->w, p->h, p->name, p->key);
			break;
		case RAD_CHECKBUTTON:
			w->form->AddCheckButton(p->x, p->y, p->name, p->key);
			break;
		case RAD_LABEL:
			w->form->AddLabel(p->x, p->y, p->name, p->key);
			break;
		case RAD_POINTBUTTON:
			w->form->AddPointButton(p->x, p->y, p->name, p->key);
			break;
		case RAD_EDITBOX1:
			if (p->flags&ACCF_TRANSP) w->form->AddEditBox(p->x, p->y, p->w, p->h, p->name, p->key, &ii, 0, p->min, p->max);
			else w->form->AddEditBox(p->x, p->y, p->w, p->h, p->name, p->key, &ii);
			break;
		case RAD_EDITBOX2:
			if (p->flags&ACCF_TRANSP) w->form->AddEditBox(p->x, p->y, p->w, p->h, p->name, p->key, &dd, 0, p->mind, p->maxd);
			else w->form->AddEditBox(p->x, p->y, p->w, p->h, p->name, p->key, &dd);
			break;
		case RAD_EDITBOX3:
			w->form->AddEditBox(p->x, p->y, p->w, p->h, p->name, p->key, ss);
			break;
		case RAD_SLIDEBARH:
			w->form->AddSlideBarH(p->x, p->y, p->w, p->h, p->ww, &ii);
			break;
		case RAD_SLIDEBARV:
			w->form->AddSlideBarV(p->x, p->y, p->w, p->h, p->ww, &ii);
			break;
		case RAD_MENU:
			w->form->AddBaseMenu(p->name, p->key);
			break;

		case RAD_LISTBOX:
			w->form->AddListBox(p->x, p->y, p->w, p->h,  p->hh, dummy);
			break;
		case RAD_PROGRESSBAR:
			w->form->AddProgressBar(p->x, p->y, p->w, p->h, p->ww);
			break;

		case RAD_STRING:
			if (p->flags&ACCF_TRANSP) w->form->WindowText(p->x, p->y, p->name);
			else w->form->WindowText(p->x, p->y, p->name, p->ink, p->paper);
			break;
		case RAD_BOX:
			if (p->flags&ACCF_TRANSP) w->form->WindowBox(p->x, p->y, p->w, p->h);
			else w->form->WindowBox(p->x, p->y, p->w, p->h, p->ink);
			break;
		case RAD_RECT:
			if (p->flags&ACCF_TRANSP) w->form->WindowRect(p->x, p->y, p->w, p->h);
			else w->form->WindowRect(p->x, p->y, p->w, p->h, p->ink);
			break;
		case RAD_LINE:
			if (p->flags&ACCF_TRANSP) w->form->WindowLine(p->x, p->y, p->w, p->h);
			else w->form->WindowLine(p->x, p->y, p->w, p->h, p->ink);
			break;
		case RAD_CIRCLE:
			if (p->flags&ACCF_TRANSP) w->form->WindowDrawCircle(p->x, p->y, p->w);
			else w->form->WindowDrawCircle(p->x, p->y, p->w, p->ink);
			break;
		case RAD_FILLCIRCLE:
			if (p->flags&ACCF_TRANSP) w->form->WindowFillCircle(p->x, p->y, p->w);
			else w->form->WindowFillCircle(p->x, p->y, p->w, p->ink);
			break;
		case RAD_ELLIPSE:
			if (p->flags&ACCF_TRANSP) w->form->WindowDrawEllipse(p->x, p->y, p->w, p->h);
			else w->form->WindowDrawEllipse(p->x, p->y, p->w, p->h, p->ink);
			break;
		case RAD_FILLELLIPSE:
			if (p->flags&ACCF_TRANSP) w->form->WindowFillEllipse(p->x, p->y, p->w, p->h);
			else w->form->WindowFillEllipse(p->x, p->y, p->w, p->h, p->ink);
			break;
		case RAD_ARC:
			if (p->flags&ACCF_TRANSP) w->form->WindowDrawArc(p->x, p->y, p->mind, p->maxd, p->w);
			else w->form->WindowDrawArc(p->x, p->y, p->mind, p->maxd, p->w, p->ink);
			break;
	}
}

static void Select(Window *wn, int x, int y, int w, int h)
{
	wn->set_ppop(_GNOT);
	wn->WindowRect(x,y,w,h);
	wn->WindowRect(x+1,y+1,w-2,h-2);
	wn->set_ppop(_GSET);
}

void AssignSize(Accel &table, int &X, int &Y, int &W, int &H)
{
	int var = 0, a = -1;

	if (table.type == RAD_LINE)	//exchange param if it is needed
	{
		if (table.x>table.w)
		{
			table.x ^= table.w;
			table.w ^= table.x;
			table.x ^= table.w;
		}
		if (table.y>table.h)
		{
			table.y ^= table.h;
			table.h ^= table.y;
			table.y ^= table.h;
		}
	}
	switch(table.type)
	{
		default:
		case RAD_PUSHBUTTON1:
		case RAD_LABEL:
		case RAD_CHECKBUTTON:
		case RAD_POINTBUTTON:
		case RAD_MENU:
		case RAD_STRING:
		case RAD_LINE:
		case RAD_BOX:
		case RAD_RECT:
		case RAD_EDITBOX1:
		case RAD_EDITBOX2:
		case RAD_EDITBOX3:
		case RAD_SLIDEBARH:
		case RAD_SLIDEBARV:
		case RAD_LISTBOX:
		case RAD_PANEL:
		case RAD_TABPAGE:
			X =	table.x-2;
			Y =	table.y-2;
			break;
		case RAD_CIRCLE:
		case RAD_ARC:
		case RAD_FILLCIRCLE:
			X =	table.x-table.w-2;
			Y =	table.y-table.w-2;
			break;
		case RAD_ELLIPSE:
		case RAD_FILLELLIPSE:
			X =	table.x-table.w-2;
			Y =	table.y-table.h-2;
			break;
		case RAD_PROGRESSBAR:
			X =	table.x-3;
			Y =	table.y-3;
			break;
	}
	switch(table.type)
	{
		default:
		case RAD_PUSHBUTTON1:
		case RAD_BOX:
		case RAD_RECT:
		case RAD_PROGRESSBAR:
		case RAD_PANEL:
			W =	table.w+4;
			H =	table.h+4;
			break;
		case RAD_EDITBOX1:
		case RAD_EDITBOX2:
		case RAD_EDITBOX3:
			W =	table.w + table.h +4;
			H =	21+4;
			break;
		case RAD_CHECKBUTTON:
		case RAD_POINTBUTTON:
			var = 24;
		case RAD_STRING:
		case RAD_LABEL:
//			if (prj->font)	set_font(prj->font-1);
			W = var + strlen(table.name)*8 +4;
			H = 16+4;
//			set_font(a);
			break;
		case RAD_LINE:
			W =	table.w+4-table.x;
			H =	table.h+4-table.y;
			break;
		case RAD_LISTBOX:
			H = table.h*table.hh+2;
			W = table.w*table.ww+16;
			break;
		case RAD_SLIDEBARH:
			H = 16+4;
			W =	(table.h-table.w)/table.ww+60+4;
			break;
		case RAD_SLIDEBARV:
			H =	(table.h-table.w)/table.ww+60+4;
			W = 16+4;
			break;
		case RAD_MENU:
			W = strlen(table.name)*8 + 8 + 4;
			H = -22;
			break;
		case RAD_ARC:
		case RAD_CIRCLE:
		case RAD_FILLCIRCLE:
			W =	table.w*2+1+4;
			H =	table.w*2+1+4;
			break;
		case RAD_ELLIPSE:
		case RAD_FILLELLIPSE:
			W =	table.w*2+1+4;
			H =	table.h*2+1+4;
			break;
	}
}

void SelectCurrent(Wind *w)
{
	int X2,Y2,W2,H2;
	AssignSize(w->table[w->current], X2,Y2,W2,H2);
	Select(w->form, X2,Y2,W2,H2); // clear old
}

//
// Test for clicking to item
//
int Projekt::ClickTest(int x, int y, Wind *w)
{
	int i, X,Y,W,H;
	if (w->items==0) return 0;
	for (i=w->items-1; i>=0; i--)
	{
		if (w->table[i].type == RAD_MENU) continue;
		AssignSize(w->table[i], X, Y, W, H);
		if (x>=X && y>=Y)
			if (x<X+W && y<Y+H)
				break;
	}
	if (i == -1) return 0;	// not found
	if (w->current == i) return 2; // the same
	if (w->current != -1)
	{
		SelectCurrent(w);
	}
	Select(w->form, X,Y,W,H);
	w->current = i;
	if (lBox->GetSize()) lBox->SetToItem(i);
	return 1;
}

static Accel *movedAccel;

static void	DrawShape(int, int, int w, int h)
{
	Window *wn = FGApp::GetCurrentWindow();
	int	a =	wn->set_ppop(_GNOT),X,Y,W,H;

	AssignSize(*movedAccel, X, Y, W, H);
	wn->WindowBox(X+w, Y+h, W-1, H-1);
	wn->set_ppop(a);
}

static void rad_hook(int a, int b, int& dx, int& dy, int fnc)
{
	switch(fnc)
	{
		case 1: // move
			dx = dx - ((dx+a) - ((dx+a)&0xfffffff8));
			dy = dy - ((dy+b) - ((dy+b)&0xfffffff8));
			break;
	}
}
void Projekt::proc(GuiEvent *p)
{
	int	x, y, x1, y1, i, j;
	static Accel *cAccel;
	Wind *w;
	Accel *tab;

	for(i=0; i<prj->nwin; i++)	// find current window
	{
		if (p->wnd==prj->Okno[i].form)
		{
			break;
		}
	}
	if (i==prj->nwin) assert(0); // not found ?
	w = prj->Okno + i;
	tab = w->table;

	if (!test) switch(p->GetType())
	{
		case INITEVENT:
			if (granularity_x>=4 || granularity_x>=4) if(!test)
			{
				int xx=p->wnd->GetWW();
				int yy=p->wnd->GetHW();
				p->wnd->WindowLock();
				p->wnd->set_ppop(_GNOT);
				for (i=granularity_x;i<xx;i+=granularity_x)
					for (j=granularity_y;j<yy;j+=granularity_y)
					{
						p->wnd->WindowPixel(i,j);
					}
				p->wnd->set_ppop(_GSET);
				p->wnd->WindowUnLock();
			}
			p->wnd->InstallWindowHook(rad_hook);
			break;
		case STARTDRAGLEFTEVENT:	// MOVE CONTROLS AT THE NEW POSITION
			if (!test) if (prj->ClickTest(p->GetX(), p->GetY(), w))	// DRAG CONTROL
			{
				App::SetDragShape(DrawShape);
				movedAccel = &tab[w->current];
			}
			break;
		case DRAGLEFTEVENT:	// MOVE CONTROLS AT THE NEW POSITION
			App::SetDragShape(0);
			App::GetDragVector(x, y, x1, y1);
			if (prj->ClickTest(x, y, w))	// DRAG CONTROL
			{
				int a =	tab[w->current].x;
				int b =	tab[w->current].y;

				if (a+x1<0 || b+y1<0 ||	a+x1>=w->form->GetWW() || b+y1>=w->form->GetHW()) break;
				tab[w->current].x += x1;
				tab[w->current].y += y1;
				tab[w->current].x -= (tab[w->current].x % granularity_x);
				tab[w->current].y -= (tab[w->current].y % granularity_y);
				if (tab[w->current].type == RAD_LINE)
				{
					tab[w->current].w += x1;
					tab[w->current].h += y1;
					tab[w->current].w -= (tab[w->current].w % granularity_x);
					tab[w->current].h -= (tab[w->current].h % granularity_y);
				}
				eb_x->ChangeItem(&tab[w->current].x);
				eb_y->ChangeItem(&tab[w->current].y);
				if (tab[w->current].type == RAD_LINE)
				{
					eb_w->ChangeItem(&tab[w->current].w);
					eb_h->ChangeItem(&tab[w->current].h);
				}
				int tmp = w->current;
				prj->Redraw(w);
				w->current = tmp;
				SelectCurrent(w);
			}
			break;
		case LOSTFOCUSEVENT:
			if (w->current != -1)
			{
				SelectCurrent(w);
				w->current = -1;
			}
			break;
		case CLICKLEFTEVENT:
			if (test) break;
			if (prj->curr!=i)
			{
				lBox2->SetToItem(i);
				prj->curr = i;
				prj->Update();
			}
			if (prj->ClickTest(p->GetX(), p->GetY(), w) == 2
				&& (cAccel != &tab[lBox->GetCurrent()])) // the same
			{
				int tmp = w->current;
				ShowDialog((RadType)0, cAccel = &tab[lBox->GetCurrent()]);
				w->current = tmp;
//				SelectCurrent(w);
			}
			else Puk();
			break;
		case WINDOWRESIZEEVENT:
			w->w = p->wnd->GetW();
			w->h = p->wnd->GetH();
			eb3->ChangeItem(&w->w);
			eb4->ChangeItem(&w->h);
			prj->Redraw(w);
			break;
		case WINDOWMOVEEVENT:
			w->x = p->wnd->GetX();
			w->y = p->wnd->GetY();
			eb1->ChangeItem(&w->x);
			eb2->ChangeItem(&w->y);
			put_block(p->wnd->GetX(), p->wnd->GetY(), p->wnd->GetW(), p->wnd->GetH(), tmpBmp, _GSET);
			break;
		case GETFOCUSEVENT:
			if (!test)
			{
				if (tmpBmp) delete [] tmpBmp;
				tmpBmp = new FGPixel[p->wnd->GetW() * p->wnd->GetH()];
				memcpy(tmpBmp, p->wnd->GetArray(), sizeof(FGPixel) * p->wnd->GetW() * p->wnd->GetH());
				p->wnd->RemoveControls();
				put_block(p->wnd->GetX(), p->wnd->GetY(), p->wnd->GetW(), p->wnd->GetH(), tmpBmp, _GSET);
				memcpy(p->wnd->GetArray(), tmpBmp, sizeof(FGPixel) * p->wnd->GetW() * p->wnd->GetH());
			}
			break;
		case KEYEVENT:
		case MOVEEVENT:
			mWnd->SendToWindow(p);
			break;
		case TERMINATEEVENT:
			break;
	}
}

//
// redraw 'i' window
//
void Projekt::Redraw(Wind *w)
{
	int i;
	FGColorScheme *cs = CScheme;
	CScheme = &cscheme;
	if (w->form) delete w->form;
	w->current = -1;
	w->form = new Window(&w->form, w->x,w->y,w->w,w->h,w->name,proc,w->ink,w->paper,(w->flags|WSIZEABLE|WFASTMOVE)& ~(WMODAL|WCENTRED|WESCAPE|WNOTIFY|WUNMOVED));
	for (i=0; i<w->items; i++)
		DrawWidget(w, i);
#ifdef INDEX_COLORS
	for(i=16; i<256; i++) // if color is used
	{
		vector_palette(i, *(unsigned *)&paleta[i]);
	}
#endif
			if (!test)
			{
				if (tmpBmp) delete [] tmpBmp;
				tmpBmp = new FGPixel[w->form->GetW() * w->form->GetH()];
				memcpy(tmpBmp, w->form->GetArray(), sizeof(FGPixel) * w->form->GetW() * w->form->GetH());
				w->form->RemoveControls();
				put_block(w->form->GetX(), w->form->GetY(), w->form->GetW(), w->form->GetH(), tmpBmp, _GSET);
				memcpy(w->form->GetArray(), tmpBmp, sizeof(FGPixel) * w->form->GetW() * w->form->GetH());
			}
	CScheme = cs;
}

Projekt::~Projekt()
{
	Window *w;
	int i;

	for (i=0; i<nwin; i++)
	{
		w =	Okno[i].form;
		if (w) delete w;
	}
	Save();
}

void Projekt::DrawAll(int delFlag)
{
	for (int i=0; i<nwin; i++)
	{
		if (delFlag && Okno[i].form) delete	Okno[i].form;
		Okno[i].form = 0;
		Redraw(Okno+i);
	}
}

void Projekt::Save(void)
{
	int crc;
	FILE *fp=fopen(prjname,"wb");
	if (fp)
	{
		fwrite(this, PRJHDR_SIZE, 1, fp);
		crc = CalculateCRC(0, this, PRJHDR_SIZE);
		fwrite(Okno, sizeof(Wind), nwin, fp);
		crc = CalculateCRC(crc, Okno, sizeof(Wind)*nwin);
		if (bmp_num)
		{
			fwrite(bmp_names, SYMSIZE+1, bmp_num, fp);
			crc = CalculateCRC(crc, bmp_names, (SYMSIZE+1)*bmp_num);
		}
		if (val_num)
		{
			fwrite(&values, sizeof(Values), val_num, fp);
			crc = CalculateCRC(crc, &values, sizeof(Values)*val_num);
		}
		fwrite(&crc, 4, 1, fp);
		fclose(fp);
	}
}

void Projekt::Init(void)
{
	prj = this;
	memset(this, 0, sizeof(Projekt));
	Okno[0]=windx;
	nwin = 1;
	version = VERSION;
	magic = 0x73190821;
	video_mode = 2;
	app_altx = 1;
	app_cfg = 1;
	sprintf(prjname, "%s.wnd", wind()->name);
#ifdef INDEX_COLORS
	for(int i=0; i<256; i++)
	{
   		paleta[i]=get_palette(i);
	}
#endif
	memcpy(&cscheme, old_scheme/*CScheme*/, sizeof(FGColorScheme));
    color_depth = get_colordepth();
}

Projekt::Projekt(char *name, char *name2)
{
	int crc, crc_new, i,j;
	Init();
	FILE *fp=fopen(name,"rb");
	if (fp)
	{
		fread(this, PRJHDR_SIZE, 1, fp);
		crc = CalculateCRC(0, this, PRJHDR_SIZE);
		fread(Okno, sizeof(Wind), nwin, fp);
		crc = CalculateCRC(crc, Okno, sizeof(Wind)*nwin);
		if (bmp_num)
		{
			fread(bmp_names, (SYMSIZE+1), bmp_num, fp);
			crc = CalculateCRC(crc, bmp_names, (SYMSIZE+1)*bmp_num);
		}
		if (val_num)
		{
			fread(&values, sizeof(Values), val_num, fp);
			crc = CalculateCRC(crc, &values, sizeof(Values)*val_num);
			BuildVariables();
		}
		fread(&crc_new, 4, 1, fp);
		prj = this;
		if (magic != 0x73190821 || crc != crc_new || version<LAST_COMPAT_VERSION)
		{
			IError("Bad file format, file rejected!",0);
			Init();
		}
		fclose(fp);
	}
	else Init();
	strcpy(prjname, name2);
	DrawAll();
}

Projekt::Projekt()
{
	Init();
}

void Projekt::DeleteItem(void)
{
	int item = lBox->GetCurrent(), items = Items();
	if (items==0) return;
	if (item < items && items>1)
		memmove(table()+item, table()+item+1, sizeof(Accel)*(items-item-1));
	items = --Okno[curr].items;
	lBox->Resize(-1);
	Redraw(&prj->Okno[prj->curr]);
}

// prehodi aktualny prvok o jeden dalej
void Projekt::Forward(void)
{
	Accel a;
	int poloha = lBox->GetCurrent();
	if (poloha == lBox->GetSize()-1) return;
	memcpy(&a, table()+poloha, sizeof(a));
	memcpy(table()+poloha, table()+poloha+1, sizeof(a));
	memcpy(table()+poloha+1, &a, sizeof(a));
	lBox->SetToItemRel(1);
	lBox->Draw();
	Redraw(&prj->Okno[prj->curr]);
}

// prehodi aktualny prvok o jeden naspat
void Projekt::Backward(void)
{
	Accel a;
	int poloha = lBox->GetCurrent();
	if (poloha == 0) return;
	memcpy(&a, table()+poloha-1, sizeof(a));
	memcpy(table()+poloha-1, table()+poloha, sizeof(a));
	memcpy(table()+poloha, &a, sizeof(a));
	lBox->SetToItemRel(-1);
	lBox->Draw();
	Redraw(&prj->Okno[prj->curr]);
}

// zdvoji aktualny prvok
void Projekt::Clone(void)
{
	int poloha = lBox->GetCurrent(), items = Items();
	if (items==0 || items>=MAX_ACCEL) return;
	memmove(table()+poloha+1, table()+poloha, sizeof(Accel)*(items-poloha));
	Okno[curr].items++;
	lBox->Resize(1);
	lBox->Draw();
	Redraw(&prj->Okno[prj->curr]);
}

void Projekt::AddWindow(int dup)
{
	static int duplicate=1;
	Wind *w;

	if (nwin<MAX_WND)
	{
		if (dup)
		{
			Okno[nwin] = Okno[curr];
		}
		else
		{
			Okno[nwin] = windx;
		}
		curr = nwin++;
		w = Okno + curr;
		w->form = 0;
		w->x += 16;
		w->y += 16;
		sprintf(w->name, "%s %d", w->name, duplicate++);
		Update();
		Redraw(w);
	}
}

void Projekt::DeleteWindow(void)
{
	if (nwin>1)
	{
		delete prj->form();
		if (nwin-1 > curr) memmove(Okno+curr, Okno+curr+1, sizeof(Wind));
		else curr--;
		nwin--;
		memset(Okno+nwin, 0, sizeof(Wind));
		Update();
		Redraw(Okno+curr);
	}
}

void Projekt::Up(void)
{
	if (curr)
	{
		lBox2->Up();
		curr--;
		Update();
		Redraw(Okno+curr);
	}
}

void Projekt::Down(void)
{
	if (curr<nwin-1)
	{
		lBox2->Down();
		curr++;
		Update();
		Redraw(Okno+curr);
	}
}



/*
	OpenGUI - Drawing & Windowing library

	Copyright (C) 1996,2005  Marian Krivos

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

#include <stdarg.h>
#include <stdlib.h>
#include <errno.h>

#include "fastgl.h"
#include "_fastgl.h"


#ifdef FG_NAMESPACE
namespace fgl {
#endif

char FGEditBox::clip[bufsize];
int  FGEditBox::is_clip;

static int alty[26] =
{
ALT_A,ALT_B,ALT_C,ALT_D,ALT_E,ALT_F,ALT_G,ALT_H,
ALT_I,ALT_J,ALT_K,ALT_L,ALT_M,ALT_N,ALT_O,ALT_P,
ALT_Q,ALT_R,ALT_S,ALT_T,ALT_U,ALT_V,ALT_W,ALT_X,
ALT_Y,ALT_Z
};

// -----------------------------------------------------------------------------

FGControlContainer::FGControlContainer(const char *s, int y) : idstring(s?strdup(s):0), ypos(y)
{
	width = FGFontManager::textwidth(FONTSYSLIGHT,(char *)s) + NOTEGAP*2;
	flag = 0;
	default_control = 0;
}

FGControlContainer::FGControlContainer(const FGControlContainer& old)
{
	default_control = old.default_control;
	idstring = strdup(old.idstring);
	width = old.width;
	ypos = old.ypos;
	flag = old.flag;
}

FGControlContainer::~FGControlContainer()
{
	if (idstring)
		free((void *)idstring);
	idstring=0;
}

void FGControlContainer::SetDefaultControl(FGControl* ctrl)
{
	FGControl* tmp = default_control;

	default_control = ctrl;

	if (tmp != 0)
	{
		tmp->OnLostFocus();
		tmp->draw(); 				// draw old as unselected
	}

	if ( ctrl != 0 && (ctrl->owner->GetStatus() & WUSESELECTEDCONTROL) )
	{
		default_control->OnFocus();
		default_control->draw();    // draw a new as selected
	}
}

FGControl*	FGControlContainer::GetDefaultControl(void)
{
	return default_control;
}

void FGControlContainer::Rename(const char *s)
{
	if (idstring)
		free((void *)idstring);
	idstring = strdup(s);
	width = FGFontManager::textwidth(FONTSYSLIGHT,(char *)s) + NOTEGAP*2;
}

void FGControlContainer::SetPreviousControl(void)
{
	int sz = size(), index = 0;

	if (sz < 2) return;

	FGControlIterator iter = begin();
	FGControlIterator ende = end();
	FGControl** ptrs = new FGControl*[sz];

	while(iter != ende)
	{
		ptrs[index] = *iter;
		++iter;
		++index;
	}

	if ( default_control == ptrs[0] )
	{
		SetDefaultControl( ptrs[sz-1] );	// the end
	}
	else for(int i=0; i<sz; i++)
	{
		if ( ptrs[i] == default_control )
		{
			SetDefaultControl( ptrs[i-1] ); // previous
			break;
		}
	}
	delete [] ptrs;
}

void FGControlContainer::SetNextControl(void)
{
	if (size() < 2) return;

	FGControlIterator iter = begin();
	FGControlIterator ende = end();

	while(iter != ende)
	{
		if ( (*iter) == default_control )
		{
			++iter;
			if (iter != ende)
			{
				SetDefaultControl( *iter );
			}
			else
			{
				SetDefaultControl( *begin() );
			}
			return;
		}
		++iter;
	}
}

// -----------------------------------------------------------------------------

FGBaseGui::FGBaseGui(int xs, int ys, int ws, int hs, const char *nm, ObjectType typ, FGPixel i, FGPixel p, long flag)
	: FGDrawBuffer(ws,hs,nm,typ,i,p)
{
	ink = i;
	paper = p;
	status = flag;
	private_data = user_data = (void *)-1;
	temporary_hidden = false;

	if (typ==WINDOW	&& flag&WUSELAST && entryPoint) entryPoint->Add(xs, ys, ws, hs, name,flag);
	// position check  & correction
	if (typ	== WINDOW || typ == MENUWINDOW)
	{
		if (ws > X_width) ws = X_width;
		if (hs > Y_width) hs = Y_width;
		if (ws < 8)	ws = 8;
		if (hs < 8)	hs = 8;
		if (status&WALIGNED)
		{
			xs &= ~3;
			ys &= ~3;
			ws &= ~3;
			hs &= ~3;
		}
		if ((xs + ws) >= X_width) xs = X_width - ws;
		if (xs < 0)	xs = 0;
		if ((ys + hs - 1) >= Y_width) ys = Y_width - hs;
		if (ys < 0)	ys = 0;
	}
	if (typ==WINDOW	&& flag&WCENTRED)
	{
		xs = (X_width-ws)/2;
		ys = (Y_width-hs)/2;
	}
	x =	xs;
	y =	ys;
	Resize(ws-w, hs-h);
	if (status & WMODAL) status &= ~WMINIMIZE;
}

FGBaseGui::~FGBaseGui()
{
}

void FGAPI FGBaseGui::Resize(int dx, int dy)
{
	FGDrawBuffer::Resize(dx, dy);
	if (type == WINDOW && entryPoint)
		entryPoint->DatabaseResize(w,h,name);
}

//! Draws the frame of FGControl.
void FGBaseGui::frame(void)
{
	vline(0, 0, h, CScheme->button_bord1);
	hline(0, 0, w, CScheme->button_bord1);
	vline(0 + w - 2, 0 + 1, h - 2, CScheme->button_bord3);
	hline(0 + 1, 0 + h - 2, w - 2, CScheme->button_bord3);
	vline(0 + w - 1, 0, h - 0, CScheme->button_bord2);
	hline(0, 0 + h - 1, w - 1, CScheme->button_bord2);
}


// -----------------------------------------------------------------------------

FGControl::FGControl(int xs, int ys, int ws, int hs, const char *nm, ObjectType typ, int keycode, FGWindow * own, ControlCall f, FGPixel p, FGPixel i, void* user_data):
  FGBaseGui(xs += own->xoff, ys += own->yoff, ws, hs, nm, typ, i, p, BNORMAL)
{
	owner = own;
	SetUserData(user_data);
	grp=0;
	closure_name = 0;

	FGApp::ResetCurrentControl(this);
	
	if (keycode >= 'a' && keycode <= 'z')
		keycode = toupper(keycode);
	key = keycode;
	fnc = f;
	local_id = owner->ctrl_counter++;

	own->Buttony->push_back(this);
	tab_page_controls = (void *)&*owner->Buttony;

	if (own->Buttony->size() == 1 && ( own->GetStatus() & WUSESELECTEDCONTROL) )
		own->SetDefaultControl(this);
}

FGControl::~FGControl()
{
	if (this==owner->call_handler)
		owner->ResetActive();
	if (owner->GetDefaultControl() == this)
		owner->SetDefaultControl(0);
	// erase from visible !
	owner->WindowBox(x-owner->xoff, y-owner->yoff, w, h, owner->GetPaper());
	owner = 0;
	if (closure_name)
		free(closure_name);
	closure_name = 0;
}

void FGControl::RefreshGroup(void)
{
	if (grp)
		grp->RefreshGroup(this);
}

void FGControl::DrawAsSelected(void)
{
	if ( IsSelected() )
		rect(0,0,w,h, GetColorOfFrame() );
}

void FGControl::AttachSignalName(const char* string)
{
	if (closure_name)
		free(closure_name);
	closure_name = 0;
	if (string)
		closure_name = strdup(string);
}

void FGControl::Enable(void)
{
	SetStatus((GetStatus() & (~7)) | BNORMAL);
	draw();
}

void FGControl::Disable(void)
{
	SetStatus((GetStatus() & (~7)) | BDISABLED);
	// make as unselected
	if (owner->GetDefaultControl() == this)
		owner->SetDefaultControl(0);
	draw();
}

/**
	Redraws a whole FGControl in a parents FGWindow.
*/
void FGControl::update_owner(void)
{
	// draw  only if this is your own TabPage
	if (IsYourTabPage())
	{
		SAVE_CLIPPING(owner)
		owner->bitblit(x,y,0,0,w,h,this);
		RESTORE_CLIPPING(owner)
	}
}

bool FGControl::IsYourTabPage(void)
{
	return (tab_page_controls == (void *)&*owner->Buttony);
}

bool FGControl::IsSelected(void)
{
	return ( this == owner->GetDefaultControl() );
}

int	FGControl::GetXr(void)
{
	return owner->GetX() + x;
}

int	FGControl::GetYr(void)
{
	return owner->GetY() + y;
}

void FGControl::ClickUp(int a)	// if true, so call handler
{
	if ((status & BMASK) == BDISABLED) return;
	status ^= WTRIGGER;
	draw();
	if (a)
	{
		owner->SetActive(this);
	}
}

FGControl *FGControl::ButtonFind(FGEvent * e, int n)
{
	FGWindow *curr = FGApp::GetCurrentWindow();

	if (curr == 0) return 0;

	FGControlContainer* cont = &*curr->GetCurrentControls();
	int c;

	// test for notebooks and its bookmarks
	if ( e->GetButtons()&FG_BUTTON_LEFT )
		if (curr->ClickTabPage(e->GetX(), e->GetY()))
			return 0;

	if (!cont || curr->IsIconized())
		return 0;				// niesu buttony alebo je to ikona

	if (e->GetKey() >= 'a' && e->GetKey() <= 'z')
		c = toupper(e->GetKey());
	else
		c = e->GetKey();

	for(FGControlIterator i = cont->begin(); i != cont->end(); i++)
	{
		FGControl* p = *i;

		if (e->GetType() == KEYEVENT)
		{
			if ((p->GetKey() == c) && (p->GetStatus() & n & 7) == n)
				return p;
		}
		else
		{
			if ((p->owner->GetX() + p->owner->GetW() >= e->GetX()) && ((p->owner->GetY() + p->owner->GetH()) >= e->GetY()))
			{
				if ((p->GetXr() <= e->GetX()) && ((p->GetXr() + p->GetW()) >= e->GetX()))
				{
					if ((p->GetYr() <= e->GetY()) && ((p->GetYr() + p->GetH()) >= e->GetY()))
					{
						if ((p->GetStatus() & n & 7) == n)
						{
							return p;
						}
						else if (p->GetStatus() == BDISABLED)
						{
							return 0;
						}
					}
				}
			}
		}
	}
	return 0;
}

static int is_alted(int c)
{
	for(int i=0;i<26;i++)
		if (alty[i]==c) return i;
	return -1;
}

void FGControl::Underscore(int xo, int yo, int c)
{
	char *s;
	int pos;
	int size=0;
	if (key==0) return;

	if ((pos=is_alted(key)) >= 0)
	{
		if ((s = strchr(name, pos+'a')) != 0)
		{
			size = (long)s - (long) name;
			box(textwidth(name, size) +xo-1, 13+yo, charwidth(pos+'a')+1,1,CREDLIGHT);
		}
		else if ((s = strchr(name, pos+'A')) != 0)
		{
			size = (long)s - (long) name;
			box(textwidth(name, size) +xo-1, 13+yo, charwidth(pos+'A')+1,1,CREDLIGHT);
		}
	}
	else if ((s = strchr(name, key)) != 0)
	{
		size = (long)s - (long) name;
		box(textwidth(name, size) +xo-1, 13+yo, charwidth(key)+1,1,c);
	}
	else if ((s = strchr(name, key ^ 32)) != 0)
	{
		size = (long)s - (long) name;
		box(textwidth(name, size) +xo-1, 13+yo, charwidth(key)+1,1,c);
	}
}

void FGControl::Close(CallBack cb)
{
	delete cb->owner;	// QWERT
}

void FGControl::Quit(CallBack)
{
	FGApp::AppDone();
}

// -----------------------------------------------------------------------------

//! draw only input text with/without cursor
void FGEditBox::CapsMode(int m)
{
	caps = m;
	char *s=buf;
	if (caps==EBOX_LOW)
	{
		while(*s)
		{
			if(isupper(*s)) *s+=32;
			s++;
		}
	}
	else if (caps==EBOX_UPR)
	{
		while(*s)
		{
			if(islower(*s)) *s-=32;
			s++;
		}
	}
	draw();
}

//! redraw input string
void FGEditBox::draw(bool cursor, bool first_time)
{
	char str[130];
	set_font(FONT0816);

	box(2 + w1, 2, w2 - 4, GetH() - 4, CScheme->edit_background_active);
	strcpy(str, buf);
	if (passwd)
	{
		char *x=str;
		while (*x)
		{
			if (*x != ' ') *x = '*';
			x++;
		}
	}
	str[maxpos+offset] = 0;
	if (cursor)
	{
		text(w1 + pos * 8 + 2 + 8, 1 + (GetH() / 2 - GetFontH() / 2), str + offset + pos, CScheme->edit_foreground, CScheme->edit_background_active);
		box(w1 + pos * 8 + 8, 3, 2, GetH() - 6, CScheme->edit_foreground);
		str[pos+offset] = 0;
		text(w1 + 8, 1 + (GetH() / 2 - GetFontH() / 2), str+offset, CScheme->edit_foreground, first_time?CScheme->edit_background : CScheme->edit_background_active);
	}
	else
	{
		text(w1 + 8, 1 + (GetH() / 2 - GetFontH() / 2), str+offset, CScheme->edit_foreground, CScheme->edit_background_active);
	}
	DrawAsSelected();
	update_owner();
	owner->WindowRepaint(x, y, w, h);
}

//! draw all
void FGEditBox::draw(void)
{
	char str[132]="  ", *spt=0;
	set_font(FGFONT_BUTTON);
	int cp;
	unsigned ndl=w1-textwidth(name)-8;

	offset = pos = 0;
	TestRange();

	cp = ((GetStatus() == BDISABLED) ? CScheme->edit_fore_disable : (FGPixel)GetInk());
	box(0, 0, w1, h, paper);
	text(ndl, 1 + (h / 2 - GetFontH() / 2), name, cp, paper);
	Underscore(ndl, 2 + h / 2 - GetFontH() / 2, paper);

	switch (status & 7)
	{
		case BNORMAL:
			Underscore(ndl, 2 + h / 2 - GetFontH() / 2, ink);
		case BDISABLED:
			set_font(FONT0816);
			box(w1, 0, w2, h, CScheme->edit_foreground);
			vline(w1 + w2 - 2, 0 + 1, h - 2, CScheme->edit_background_active);
			hline(1 + w1, 0 - 2 + h, w2 - 2, CScheme->edit_background_active);
			box(2 + w1, 0 + 2, w2 - 4, h - 4, CScheme->edit_background);
			rect(w1, 0, w2, h,CScheme->edit_border);

			switch (data_type)
			{
				case EDIT_INT:
					if (hex)
						sprintf(str, "0x%x", *(int *) ptr);
					else if (octal)
						sprintf(str, "0%o", *(int *) ptr);
					else
						sprintf(str, "%d", *(int *) ptr);
					spt = str;
					break;
				case EDIT_STRING:
					 strcpy(str,(char *)ptr);
					spt = str;
					break;
				case EDIT_DOUBLE:
					sprintf(str, "%f", *(double *) ptr);
					if (*str=='0') strcpy(str, str+1);
					spt = str;
					break;
			}
			spt[maxpos] = 0;	// print visible part only
			if (passwd)
			{
				char *x=spt;
				while (*x)
				{
					if (*x != ' ') *x = '*';
					x++;
				}
			}
			text(w1 + 8, 1 + (h / 2 - GetFontH() / 2), spt, (status == BDISABLED) ? cp : CScheme->edit_foreground, CScheme->edit_background);
			break;
	}
	DrawAsSelected();
	update_owner();
	owner->WindowRepaint(x,y,w,h);
	set_font(FONT0816);
}

//! Returns 1 at the emd
int FGEditBox::inputproc(unsigned c)
{
	char *s, *d;
	int i;

	// input filter
	if (c == KUP || c == KDOWN) return 0;
//::printf("c = %x\n", c);	
	if (c>= ' ' && c <=127)
		switch(int(data_type))
	{
		case EDIT_INT:
			if (c == '-')
				break;
			if (hex && !isdigit(c))
			{
				if (c<'A' || c>'f') return 0;
				else if (c>'F' && c<'a') return 0;
			}
			else if (!isdigit(c))
			{
				return 0;
			}
			if(octal)
			{
				if (c<'0' || c>'7')
					return 0;
			}
			break;
		case EDIT_DOUBLE:
			if (!isdigit(c&255) && c != '-')
				if (c!= '.' && c!='e' && c!='E') return 0;
			break;
	}

	if (caps==EBOX_UPR && islower(c&255)) 
		c-=32;
	else if (caps==EBOX_LOW && isupper(c&255)) 
		c+=32;

	draw(0); // cursor off
	if (first)
	{
		first = 0;
		if ((isprint(c&255) || c == DEL || c == BACKSP) && c < 256)
		{
			for (i = 0; i < size; buf[i++] = ' ');
			offset = pos = 0;
		}
	}

	switch (c)
	{
		case CTRL_INSERT:
			memcpy(clip,buf,sizeof(clip));
			is_clip = 1;
			break;
		case INSERT:
			memcpy(buf,clip,sizeof(clip));
			break;
		case ESC:
			isinput = 0;
			owner->SetInInput(0);
			SetStatus(BNORMAL);
			draw(); // draw final
			return -1;
		case CR:
			isinput = 0;
			owner->SetInInput(0);
			switch (data_type)
			{
				case EDIT_INT:
					int n;
					if (hex)
						n = strtol(buf, 0, 16);
					else if (octal)
						n = strtol(buf, 0, 8);
					else n = atoi(buf);
					{
						*(int *) ptr = n;
						TestRange();
					}
					break;
				case EDIT_STRING:
					pos = size;
					while (pos)
					{
						if (buf[pos - 1] != ' ')
							break;
						pos--;
					}
					if (buf[pos-1]=='\"' && pos>0) pos--;
					buf[pos]=0;
					strcpy((char *) ptr, buf);
					break;
				case EDIT_DOUBLE:
					double d;
					d = atof(buf);
					{
						*(double *) ptr = d;
						TestRange();
					}
					break;
			}
			SetStatus(BNORMAL);
			draw(); // draw final
			owner->SetActive(this);
			return 1;
		case HOME:
			offset = pos = 0;
			break;
		case END:
			{
				pos = size;
				while (pos)
				{
					if (buf[pos - 1] != ' ')
						break;
					pos--;
				}
				offset = pos>maxpos?pos-maxpos:0;
				pos = pos-offset;
			}
			break;
		case BACKSP:
			if (pos+offset)
			{
				s = &buf[offset+pos--];
				d = s - 1;
				while (*s)
					*d++ = *s++;
				*d = ' ';
				if (offset) { offset--; pos++; }
			}
			break;
		case KLEFT:
			if (pos+offset)
			{
				if (pos) pos--;
				else if (offset) offset--;
			}
			break;
		case KRIGHT:
			if (pos+offset < size)
			{
				if (pos<maxpos) pos++;
				else { offset++; }
			}
			break;
		case DEL:
			if (pos+offset < size)
			{
				d = &buf[pos+offset];
				s = d + 1;
				while (*s)
					*d++ = *s++;
				buf[size - 1] = ' ';
			}
			break;

		default:
			if (c<' ' || c>127)
				return 0;
				
			if (pos+offset < size)
			{
				if ((c > 31) || (c < 128))
				{
					d = buf + sizeof(buf) -1;
					while(d>(buf+pos+offset))
					{
						*d = d[-1];
						d--;
					}
					*d = (char)c;
				}
				if (pos+offset < size)
				{
					if (pos<maxpos) pos++;
					else { offset++; }
				}
			}
			break;
	}
	buf[size] = 0;
	draw(1); // cursor on
	return 0;
}

//! Starts the input line
void FGEditBox::input(void)
{
	char str[130];
	set_font(FONT0816);

	isinput = 1;
	owner->SetInInput(this);

	iline = GetY() + 1 + (GetH() / 2 - GetFontH() / 2);
	icol = GetX() + w1 + 8;
	switch (data_type)
	{
		case EDIT_INT:
			if (hex)
				sprintf(str, "%x", *(int *) ptr);
			else if(octal)
				sprintf(str, "%o", *(int *) ptr);
			else
				sprintf(str, "%d", *(int *) ptr);
			break;
		case EDIT_STRING:
			strcpy(str, (const char *) ptr);
			break;
		case EDIT_DOUBLE:
			sprintf(str, "%f", *(double *) ptr);
			if (*str=='0') strcpy(str, str+1);
			break;
	}
	// copy to buffer (and fill it) and find the end of line
	for (int j = 0, i = 0; i < (int)sizeof(buf);)
		if (str[j] == 0)
			buf[i++] = ' ';
		else
			buf[i++] = str[j++];
	buf[size] = 0;
	pos = size-1;
	while (pos >= 0)
	{
		if (buf[pos] > ' ')
			break;
		pos--;
	}
	pos++;
	offset = pos>maxpos?pos-maxpos:0;
	pos -= offset;
	first = 1;
}

FGEditBox::FGEditBox(int xs,	int ys,	int ws1, int ws2, const char *nm, int	key, FGWindow *w,	int *pt, int ink, int paper, ControlCall f, int mn, int mx, int check, void* user_data)
	: FGControl(xs, ys, ws1+ws2, 21, nm, EDITBOX,	key, w, f,paper,ink,user_data)
{
	init();
	w1 = ws1;
	w2 = ws2;
	ptr= pt;
	check_range = check;
	min= mn;
	max=mx;
	data_type=EDIT_INT;
	size = maxpos=(ws2-16)/GetFontW();
	draw();
}

FGEditBox::FGEditBox(int sz, int xs, int ys, int ws1, int ws2, const char *nm, int key, FGWindow *w, char *pt, int ink, int paper, ControlCall f, void* user_data)
: FGControl(xs, ys, ws1+ws2, 21, nm, EDITBOX,	key, w, f,paper,ink, user_data)
{
	init();
	w1 = ws1;
	w2 = ws2;
	ptr= pt;
	data_type=EDIT_STRING;
	maxpos=(ws2-16)/GetFontW();
	size = sz<=0 ? maxpos : ((sz>127) ? 127:sz);
	draw();
}

FGEditBox::FGEditBox(int xs,	int ys,	int ws1, int ws2, const char *nm, int	key, FGWindow *w,	double *pt, int	ink, int paper,	ControlCall f, double mn, double mx, int check, void* user_data)
: FGControl(xs, ys, ws1+ws2, 21, nm, EDITBOX,	key, w, f,paper,ink, user_data)
{
	init();
	w1 = ws1;
	w2 = ws2;
	ptr= pt;
	check_range = check;
	mind= mn;
	maxd=mx;
	data_type=EDIT_DOUBLE;
	size = maxpos=(ws2-16)/GetFontW();
	draw();
}

void FGEditBox::init(void)
{
	check_range = 0;
	offset = 0;
	octal = 0;
	hex = 0;
	passwd = 0;
	caps = 0;
	isinput = 0;
}

FGEditBox::~FGEditBox()
{
	if (isinput)
		owner->SetInInput(0);
}

void FGEditBox::ClickUp(int a)	// if true, so call handler
{
	if ((status & BMASK) == BDISABLED) return;
	if (!a)	return;
	owner->WindowFlushInput();
	SetStatus(BPUSHED);
	input();
	draw(true, true);
}

void FGEditBox::Disable(void)
{
	FlushInput();
	SetStatus((GetStatus() & (~7)) | BDISABLED);
	draw();
}

// -----------------------------------------------------------------------------
//! Draws a FGPushButton
void FGPushButton::draw(void)
{
	int dl= textwidth(name);	// maximalny zobrazitelny pocet znakov

	int xx = w / 2 - dl / 2,
		yy = h / 2 - GetFontH() / 2;

	frame();

	switch (status & 7)
	{
		case BNORMAL:
			if (type == PUSHBUTTON_IMAGE)
				bitblit(2, 2, 0, 0, icon->GetW(), icon->GetH(), icon);
			else
				box(1, 1, w - 3, h - 3, paper);

			if (name[0])
			{
				unsigned oldp = set_ppop(_GCOLORKEY);
				text(xx, yy, name, ink, paper);
				set_ppop(oldp);
				Underscore(xx , yy, CDARK);
			}
			frame();
			DrawAsSelected();
			break;

		case BPUSHED:
			box(0, 0, w, h,CScheme->button_bord2);
			if (type == PUSHBUTTON_IMAGE)
			{
				bitblit(0 + 2, 0 + 2, 0, 0, icon->GetW(), icon->GetH(), pushed ? pushed : icon);
			}
			else
			{
				box(2, 2, w - 4, h - 4, back_pushed);
			}
			vline(0 + w - 2, 0 + 1, h - 2, CScheme->button_bord1);
			hline(0 + 1, 0 + h - 2, w - 2, CScheme->button_bord1);

			if (name[0])
			{
				unsigned oldp = set_ppop(_GCOLORKEY);
				text(xx, yy, name,fore_pushed, back_pushed);
				Underscore(xx , yy, CDARK);
				set_ppop(oldp);
			}
			break;

		case BDISABLED:
			box(0,0,w,h,CScheme->button_bord1);

			if (type == PUSHBUTTON_IMAGE)
			{
				bitblit(0 + 2, 0 + 2, 0, 0, icon->GetW(), icon->GetH(), disabled ? disabled : icon);
			}
			else
				box(2, 2, w - 4, h - 4, paper);

			vline(w - 2, 1, h - 2, fore_pushed);
			hline(1, h - 2, w - 2, fore_pushed);
			rect(0, 0, w, h);

			if (name[0])
			{
				unsigned oldp = set_ppop(_GCOLORKEY);
				text(xx, yy, name,CScheme->button_bord3);
				set_ppop(oldp);
			}
			break;
	}
	update_owner();
	owner->WindowRepaint(x, y, w, h);
}

void FGPushButton::Push(void)
{
	SetStatus((GetStatus() & (~7)) | BPUSHED | WTRIGGER);
	draw();
}

void FGPushButton::ClickDown(int x, int y)
{
	Push();
}

void FGPushButton::Release(void)
{
	SetStatus((GetStatus() & (~(7|WTRIGGER))) | BNORMAL);
	draw();
}

void FGPushButton::ClickUp(int a)	// if true, so call handler
{
	if ((status & BMASK) == BDISABLED) return;
	Release();
	if (a)
	{
		// if error, set cControl to 0
		owner->SetActive(this);
	}
}

// -----------------------------------------------------------------------------
//! Draws a FGCheckBox
void FGCheckBox::draw(void)
{
//	set_font(FGFONT_BUTTON);
	if (strlen(name))
	{
		int ww = textwidth(name)+28;
		int hh = 18;//GetFontH();
		FGDrawBuffer::Resize(ww-w,hh-h);
	}
	clear(paper);
	box(1,1,14,14,CWHITE);
	rect(1,1,14,14,CDARK);
//	rect(0,0,15,15,CDARK);
	switch (status & 7)
	{
		case BNORMAL:
			text(20, 2, name,ink,paper);
			Underscore(20, 2, ink);
			DrawAsSelected();
			break;
		case BDISABLED:
			text(20, 2, name,CScheme->edit_fore_disable,paper);
			Underscore(20, 2, CScheme->edit_fore_disable);
			break;
	}

	if (status & WTRIGGER)
	{
		if (variable)
			*variable |= mask;
		box(4,4,7,7,CBLACK);
	}
	else
	{
		if (variable) *variable &= ~mask;
	}
	update_owner();
	owner->WindowRepaint(x, y, w, h);
}

// -----------------------------------------------------------------------------
//! Draws a FGRadioButton
void FGRadioButton::draw(void)
{
	int  col;
//	set_font(FGFONT_BUTTON);

	if (strlen(name))
	{
		int ww = textwidth(name)+28;
		int hh = 18;//GetFontH();
		FGDrawBuffer::Resize(ww-w,hh-h);
	}

	clear(paper);

	if ((status & BMASK)==BDISABLED)
		col = CScheme->edit_fore_disable;
	else
		col = GetInk();

	text(20, 2, name, col, paper);
	Underscore(20, 2, col);
	int xx=7, yy=4;

	fcircle(xx,yy+4,6,CWHITE);
	circle(xx,yy+4,6,CDARK);

	if (status & WTRIGGER)
	{
		if (variable) *variable |= mask;
		fcircle(xx,yy+4,3,CBLACK);
	}
	else
	{
		if (variable) *variable &= ~mask;
		fcircle(xx,yy+4,3,CWHITE);
	}
	DrawAsSelected();
	update_owner();
	owner->WindowRepaint(x,y,w,h);
}

// -----------------------------------------------------------------------------
/* --------------------------- WINDOW MENU ----------------------------------------- */

FGBaseMenu::FGBaseMenu(const char *nm, int key, FGWindow *w, ControlCall f, int font, void* user_data)
	: FGControl(w->GetXM(FGFontManager::textwidth(font,nm)+8),
		w->GetYM()-2+2,
		FGFontManager::textwidth(font,nm)+8,
		FGFontManager::GetH(font)+4,
		nm,
		BASEMENU,
		key,
		w,
		f,
		CScheme->menu_back,
		CScheme->menu_fore,
		user_data)
{
	set_font(font);
	draw();
}

void FGBaseMenu::ClickUp(int a)	// if true, so call handler
{
	if ((status & BMASK) == BDISABLED) return;
	draw();
	if (a)
	{
		fgstate.__hint_x_menu = x + owner->x;
		fgstate.__hint_y_menu = y + owner->y+20;
		owner->SetActive(this);
	}
}

void FGBaseMenu::DrawAsSelected(void)
{
	if (IsSelected())
	{
		box(0, 0, w, MENUH-2, CScheme->menu_back_active);
		text(4, 2, name,CScheme->menu_fore_active,CScheme->menu_back_active);
		Underscore(4,2,CWHITE);
		FGControl::DrawAsSelected();
	}
}

/*
void FGBaseMenu::draw(void)
{
	switch (status & 7)
	{
		case BNORMAL:
			box(0, -1, GetW(), MENUH-2, paper);
			text(4, 2, GetName(), ink, paper);
			Underscore(4,2, ink);
			DrawAsSelected();
			update_owner();
			owner->WindowRepaint(GetX()-4, GetY()-2, GetW()+8, MENUH);
			break;
		case BDISABLED:
			text(4, 2, GetName(), CScheme->pdmenu_gray, paper);
			Underscore(4,2, paper);
			Underscore(4,3, paper);
			update_owner();
			owner->WindowRepaint(GetX()-4, GetY(), GetW()+8, GetH());
			break;
	}
}
*/

void FGBaseMenu::draw(void)
{
	switch (status & 7)
	{
		case BNORMAL:
			box(0, -1, GetW(), MENUH-2, CScheme->menu_back);
			text(4, 2, GetName(), CScheme->menu_fore, CScheme->menu_back);
			Underscore(4,2, CScheme->menu_fore);
			DrawAsSelected();
			update_owner();
			owner->WindowRepaint(GetX()-4, GetY()-2, GetW()+8, MENUH);
			break;
		case BDISABLED:
			text(4, 2, GetName(), CScheme->pdmenu_gray, CScheme->menu_back);
			Underscore(4,2, CScheme->menu_back);
			Underscore(4,3, CScheme->menu_back);
			update_owner();
			owner->WindowRepaint(GetX()-4, GetY(), GetW()+8, GetH());
			break;
	}
}

// -----------------------------------------------------------------------------
FGBaseMenuItem::FGBaseMenuItem(const char *nm, int key, FGMenuWindow *w, ControlCall f, int font, void* user_data)
	: FGControl(w->GetXM()-4,
		w->GetYM(FGFontManager::GetH(font)+8),
		w->GetWW(),
		FGFontManager::GetH(font)+8,
		nm,
		BASEMENUITEM,
		key,
		w,
		f,
		w->GetPaper(),
		w->GetInk(),
		user_data)
{
	set_font(font);
	draw();
}

void FGBaseMenuItem::ClickUp(int a)	// if true, so call handler
{
	if ((status & BMASK) == BDISABLED) return;
	draw();
	if (a)
	{
		owner->SetActive(this);
	}
}

void FGBaseMenuItem::DrawAsSelected(void)
{
	if (IsSelected())
	{
		box(0, 0, w, h, CScheme->pdmenu_back_active);
		text(4, 3, name, CScheme->pdmenu_fore_active, CScheme->pdmenu_back_active);
		Underscore(4,4,CScheme->pdmenu_fore_active);
		FGControl::DrawAsSelected();
	}
}

void FGBaseMenuItem::draw(void)
{
	switch (status & 7)
	{
		case BNORMAL:
			box(0, 0, w, h, paper);
			text(4, 3, GetName(),GetInk(),GetPaper());
			Underscore(4,4,GetInk());
			DrawAsSelected();
			update_owner();
			owner->WindowRepaint(x, y, w, h);
			break;
		case BDISABLED:
			box(0, 0, w, h, paper);
			text(4, 3, GetName(), CScheme->pdmenu_gray, GetPaper());
			Underscore(4,4,GetPaper());
			DrawAsSelected();
			update_owner();
			owner->WindowRepaint(GetX()-4, GetY(), GetW()+8, GetH());
			break;
	}
}

// -----------------------------------------------------------------------------
void FGAPI FGButtonGroup::DisableGroup(void)
{
	for(int i=0; i<count; i++)
	{
		array[i]->Disable();
	}
}

void FGAPI FGButtonGroup::EnableGroup(void)
{
	for(int i=0; i<count; i++)
	{
		array[i]->Enable();
	}
}

void FGAPI FGButtonGroup::RefreshGroup(FGControl *c)
{
	FGPushButton *pb1=(FGPushButton *)c;
	switch(type)
	{
		case 1:
			if (curr) curr->SetTrigger(0);
			if (curr == c) // to same
			{
				curr->SetTrigger(1);
			}
			break;
		case 2:
			if (curr) ((FGPushButton *)curr)->Release();
			pb1->Push();
			break;
	}
	curr = c;
}

void FGAPI FGButtonGroup::AddToGroup(FGControl *p, int activ)
{
	int i;
	if (count>=size)
	{
		IError("FGButtonGroup - out of range!", 0);
		return; // maximum numbers reached
	}
	for(i=0;i<count;i++)
	{
		assert(p != array[i]);
	}
	i = p->GetType();

	if (i!=FGBaseGui::POINTBUTTON && i!=FGBaseGui::CHECKBUTTON && i!=FGBaseGui::PUSHBUTTON && i!=FGBaseGui::PUSHBUTTON_IMAGE)
		return; // bad type

	if (type==0)
	{
		if (i==FGBaseGui::CHECKBUTTON || i==FGBaseGui::POINTBUTTON)
			type=1;
		else
			type=2;
	}
	else if (type==1 && (i==FGBaseGui::PUSHBUTTON || i==FGBaseGui::PUSHBUTTON_IMAGE) )
		IError("FGButtonGroup mismatch",0);
	else if (type==2 && (i==FGBaseGui::POINTBUTTON || i==FGBaseGui::CHECKBUTTON))
		IError("FGButtonGroup mismatch",0);

	array[count++] = p;

	p->RegisterToGroup(this); // say to button for group members

	if (activ && !curr)
	{
		curr = p;
		if (type==1)
			p->SetTrigger(1);
		else
			((FGPushButton *)p)->Push();
	}
}

// -----------------------------------------------------------------------------
void FGSlider::frame(int x, int y, int w, int h, int f)
{
	FGPixel col = (f?CWHITE:CBLACK);
	vline(x, y, h, col);
	hline(x, y, w, col);
	vline(x + w - 2, y + 1, h - 2, CGRAYED);
	hline(x + 1, y + h - 2, w - 2, CGRAYED);
	col = (f?CBLACK:CWHITE);
	vline(x + w - 1, y + 0, h - 0, col);
	hline(x + 0, y + h - 1, w - 1, col);
}

void FGSlider::ClickDown(int x, int y)
{
	if (smer==0)
		modify(x - GetX()-10);
	else
		modify(y - GetY()-10);
}

void FGSlider::modify(int value)
{
	if (value>= 0)
	{
		*val = maxv;
	}
	else if (value < (maxv-minv)/steps+8)
	{
		*val = minv;
	}
	if (value>= 0 && value < (maxv-minv)/steps)
	{
		*val = minv+value*steps;
	}
	draw();
}

void FGSlideBarV::draw(void)
{
	// WWW
	char str[40];
	unsigned dl=0;
	set_font(FONT0808);

	switch (status & 7)
	{
		case BNORMAL:
			box(1, 1, w - 3, h - 3,CScheme->slider);
			frame(0, 0, w, dl=(maxv-minv)/steps+16+6);
			frame(0,dl, w, h-dl);
			frame(2,3+((*val-minv)/steps),w-4,w,1);
			str[0] = char((*val%1000)/100+'0');
			str[1] = 0;
			str[2] = char((*val%100)/10+'0');
			str[3] = 0;
			str[4] = char(*val%10+'0');
			str[5] = 0;
			str[6] = char((*val%10000)/1000+'0');
			str[7] = 0;
			text(4,dl+3,str+6, CBLACK, CScheme->slider);
			text(4,dl+11,str, CBLACK, CScheme->slider);
			text(4,dl+19,str+2, CBLACK, CScheme->slider);
			text(4,dl+27,str+4, CBLACK, CScheme->slider);
			DrawAsSelected();
			break;

		case BDISABLED:
			box(1,   1, w - 3, h - 3, CGRAY2);
			frame(0, 0, w, dl=(maxv-minv)/steps+16+6);
			frame(0, dl, w, h-dl);
			frame(2, 3+((*val-minv)/steps),w-4,w,1);
			str[0] = char((*val%1000)/100+'0');
			str[1] = 0;
			str[2] = char((*val%100)/10+'0');
			str[3] = 0;
			str[4] = char(*val%10+'0');
			str[5] = 0;
			str[6] = char((*val%10000)/1000+'0');
			str[7] = 0;
			text(4,dl+3,str+6, CScheme->slider_disable, CScheme->slider);
			text(4,dl+11,str, CScheme->slider_disable, CScheme->slider);
			text(4,dl+19,str+2, CScheme->slider_disable, CScheme->slider);
			text(4,dl+27,str+4, CScheme->slider_disable, CScheme->slider);
			break;
	}
	update_owner();
	owner->WindowRepaint(x,y,w,h);
	if (fnc) fnc(this);
}

void FGSlideBarH::draw(void)
{
	char str[40];
	unsigned dl=0;
	set_font(FONT0808);

	switch (status & 7)
	{
		case BNORMAL:
			box(1, 1, w - 3, h - 3,CScheme->slider);
			frame(0,0,dl=(maxv-minv)/steps+16+6,GetH());
			frame(dl,0,w-dl,h);
			frame(3+((*val-minv)/steps),2,h,h-4,1);
			sprintf(str, "%3d", *val);
			text(dl+2, 4,str,CBLACK, CScheme->slider);
			DrawAsSelected();
			break;

		case BDISABLED:
			box(1, 1, w - 3, h - 3,CGRAY2);
			frame(0,0,dl=(maxv-minv)/steps+16+6,GetH());
			frame(dl,0,w-dl,h);
			frame(3+((*val-minv)/steps),2,16,12,1);
			sprintf(str, "%3d", *val);
			text(dl+2, 4,str,CScheme->slider_disable, CGRAY2);
			break;

	}
	update_owner();
	owner->WindowRepaint(x,y,w,h);
	if (fnc) fnc(this);
}

// -----------------------------------------------------------------------------
FGText::FGText(int xs, int ys, const char *nm, FGWindow *wnd, int i, int p)
	: FGControl(xs, ys, 8, 8, nm, WINDOWTEXT, 0, wnd,	0, p, i, UNDEFINED_USER_DATA)
{
	if (image) free(image);
	image = 0;
	set_font(wnd->get_font());
	draw();
}

void FGText::draw(void)
{
	if (IsYourTabPage() && name)
	{
		int ff = owner->get_font();
		owner->set_font(state._font);

		switch (status & 7)
		{
			case BNORMAL:
				owner->WindowText(x-owner->GetXW(), y-owner->GetYW(), name,ink,paper);
				break;
}
		owner->set_font(ff);
	}
}

// -----------------------------------------------------------------------------
FGImage::FGImage(int xs, int ys, FGDrawBuffer* img, FGWindow *wnd)
	: FGControl(xs, ys, img->GetW(), img->GetH(), 0, WINDOWIMAGE, 0, wnd, 0, 0, 0, UNDEFINED_USER_DATA)
{
	bitmap = img;
}

void FGImage::draw(void)
{
	if (IsYourTabPage())
	{
		switch (status & BMASK)
		{
			case BNORMAL:
				DrawAsSelected();
				owner->WindowPutBitmap(x-owner->GetXW(), y-owner->GetYW(), 0, 0, w, h, bitmap);
				break;
		}
	}
}

// -----------------------------------------------------------------------------
FGPanel::FGPanel(int xs, int ys, int ws, int hs, const char *nm, FGWindow *wnd, int i1, int i2, int p)
	: FGControl(xs, ys, ws, hs, nm, WINDOWPANEL, 0, wnd, 0, p, i1, UNDEFINED_USER_DATA), ink1(i1), ink2(i2)
{
	status = 0;
	if (image) free(image);
	image = 0;
	owner->clip(x,y,w,h);
	draw();
}

void FGPanel::draw(void)
{
	if (IsYourTabPage())
	{
		owner->StateLock();
		SAVE_CLIPPING(owner)
		owner->hline(x, y, w, ink1);
		owner->vline(x, y, h, ink1);
		owner->hline(x, y + h - 1, w, ink2);
		owner->vline(x	+ w - 1, y, h, ink2);
		if (name && *name) owner->printf(x+8, y-8, " %s ", name);
		RESTORE_CLIPPING(owner)
		owner->WindowRepaint(x,y,w,h);
		owner->StateUnlock();
	}
}

// -----------------------------------------------------------------------------
void FGProgressBar::draw(void)
{
	int tmp;
	char s[16];
	set_font(2);

	tmp = (int)((w/(float)steps)*value);
	if (tmp!=sirka) // changed
	{
		rect(2,-2,w+4,h+4, CWHITE);
		rect(0,0,w+1,h+1, CWHITE);
		rect(-2,-2,w+3,h+3, CBLACK);
		rect(-1,-1,w+1,h+1, CBLACK);
		box(0,0,tmp,h,paper);
		box(tmp,0,w-tmp,h,ink);

		sprintf(s,"%d%%", (int)((100./steps)*value));   // changed 3.11.2004 because of 101% case
		if (tmp>=textwidth(s))
		{
			text(tmp/2-textwidth(s)/2,h/2-8,s,ink,paper);
		}
		update_owner();
		owner->WindowRepaint(x-2,y-2,w+4,h+4);
	}
	sirka = tmp;
}

FGProgressBar::FGProgressBar(FGWindow *win, int xx, int yy, int ww, int hh, int s)
	: FGControl(xx, yy, ww, hh, 0, PROGRESSBAR, 0, win, 0, CWHITE, CBLACK, UNDEFINED_USER_DATA)
{
	if (s<=0)
	{
		 s=100;
		 IError("FGProgressBar: (step <= 0)", 0);
	}
	steps = s;
	value = 0;
	sirka = -1;
	SetInk(CWHITE);
	SetPaper(CBLUELIGHT);
	draw();
}

// -----------------------------------------------------------------------------
FGLabel::FGLabel(int xs, int ys, const char	*nm, int key, FGWindow *w, ControlCall f, unsigned i, unsigned p, void* user_data)
: FGControl(
	xs,
	ys,
	16,
	16,
	nm,
	LABEL,
	key,
	w,
	f,
	p,
	i,
	user_data)
{
	set_font(w->get_font());
	is_transparent = 0;
	draw();
}

void FGLabel::ClickUp(int a)	// if true, so call handler
{
	if ((status & BMASK) == BDISABLED)
		return;

	draw();

	if (a)
	{
		// if error, set cControl to 0
		owner->SetActive(this);
	}
}

void FGLabel::draw(void)
{
	if (strlen(name))
	{
		int ww = textwidth(name)+28;
		int hh = 17;//GetFontH();
		FGDrawBuffer::Resize(ww-w,hh-h);
	}

	if (is_transparent)
	{
		set_ppop(_GCOLORKEY);
		state.colorkey = paper;
	}
	else
		set_ppop(_GSET);
	clear(paper);
	text(0,0,name, !IsSelected() ? ink:paper, IsSelected() ? ink:paper);
	DrawAsSelected();
	update_owner();
	owner->WindowRepaint(x,y,w,h);
}

// -----------------------------------------------------------------------------

void FGTwoStateButton::draw(void)
{
	int dl= textwidth(name);	// maximalny zobrazitelny pocet znakov
	int xx = w / 2 - dl / 2, yy = h / 2 - GetFontH() / 2;

	frame();
	if (status & WTRIGGER)
	{
		if (variable) *variable |= mask;
			box(0,0,w,h,CScheme->button_bord1);
			vline(w - 2, 1, h - 2,CScheme->button_fore_pushed);
			hline(1, h - 2, w - 2,CScheme->button_fore_pushed);
			rect(0, 0, w, h,CScheme->button_fore_pushed);
			box(1, 1, w - 3, h - 3,CGRAY3);
			text(xx, yy, name, CScheme->button_fore, CGRAY3);
			Underscore(xx , yy, CDARK);
	}
	else
	{
		if (variable) *variable &= ~mask;
				box(1, 1, w - 3, h - 3, CScheme->button_back);
				text(xx, yy, name,CScheme->button_fore,CScheme->button_back);
				Underscore(xx , yy, CDARK);
	}
	DrawAsSelected();
	update_owner();
	owner->WindowRepaint(x, y, w, h);
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------



#ifdef FG_NAMESPACE
}
#endif



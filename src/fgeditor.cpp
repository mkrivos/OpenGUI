/*
    OpenGUI - Drawing & Windowing library

    Copyright (C) 1996,2004  Marian Krivos

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

#include "fastgl.h"
#include "widgets.h"

#define FGCX			(__fg_font_tab[_font][0])
#define FGCY			(__fg_font_tab[_font][1])
#define FONT			FONT0816

#define	OFFSET_X		4
#define	OFFSET_Y		6


#ifdef FG_NAMESPACE
namespace fgl {
#endif

static int __fg_font_tab[5][2]=
{
{4,8},
{8,12},
{8,18},
{12,25},
{16,34},
};

//  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/**
	Close the editor.
*/
void FGAPI FGTextEditor::Close(void)
{
	if (Text_EditorPtr->ShowNotify()==0)
		delete Text_EditorPtr;
}

void FGTextEditor::file_proc(CallBack cb)
{
	FGTextEditor *tt = (FGTextEditor *) cb->GetOwner()->GetParameter();

	switch(cb->GetLocalId())
	{
		case 0:
			tt->NewBuffer();
			break;
		case 1:
			tt->OpenBuffer2();
			break;
		case 2:
			tt->ReopenBuffer();
			break;
		case 3:
			tt->SaveBuffer();
			break;
		case 4:
			tt->SaveAsBuffer2();
			break;
		case 5:
			tt->Close();
			break;
	}
}

void FGAPI FGTextEditor::file(void)
{
	FGMenuWindow *m1Wnd = new FGMenuWindow(100, 132+12);
	FGControl * cc;

	m1Wnd->SetParameter(this);
	m1Wnd->AddMenu("New", 'N', file_proc);
	cc = m1Wnd->AddMenu("Open", 'O', file_proc);
	m1Wnd->AddMenu("Reopen", 'R', file_proc);
	m1Wnd->Separator();
	m1Wnd->AddMenu("Save", 'S', file_proc);
	m1Wnd->AddMenu("Save As", 'A', file_proc);
	m1Wnd->Separator();
	m1Wnd->AddMenu("Close", 'C', file_proc);
	if (ronly)
		cc->Disable();
}

int FGAPI FGTextEditor::_memcmp(unsigned char *from, unsigned char *co, unsigned kolko, unsigned l)
{
	unsigned char *p;
	unsigned c1, c2;
	unsigned i;

	while (kolko--)
	{
		if (toupper(*from) == toupper(*co))
		{
			p = from;
			for (i = 0; i < l; i++)
			{
				c1 = toupper(p[i]);
				c2 = toupper(co[i]);
				if (c1 != c2)
					break;
			}
			if (i == l)
				return 0;
		}
		from++;
	}
	return 1;
}

//
// return line number or -1
//
int FGAPI FGTextEditor::Find(int from, int count, int size)
{
	int i;
	for(i=0;i<count;i++)
	{
		if (!_memcmp(text[from+i].text,(unsigned char *)srch_str, text[from+i].size, size))
		{
			return from + i;
		}
	}
	return -1;
}

void FGAPI FGTextEditor::Clear(void)
{
	Text_EditorPtr->WindowBox(OFFSET_X, OFFSET_Y, WX*FGCX, WY*FGCY, Text_EditorPtr->GetPaper());
}

void FGAPI FGTextEditor::_goto(void)
{
	if (last_found<buf.lines && last_found != buf.line)
	{
		buf.posx = 0;
		buf.posy = 0;
		buf.line = buf.top = last_found;
		Clear();
		ShowBuffer();
	}
}

/**
	Goto to the line 'l'.
*/
void FGAPI FGTextEditor::Goto(int l)
{
	if (l>=buf.lines-WY) l = buf.lines-WY;
	if (l<0 || l>=buf.lines) return;
	last_found = l;
	_goto();
}

void FGTextEditor::SearchProc(FGEvent *p)
{
	if (p->GetType() == ACCELEVENT)
	{
		FGTextEditor *tt = (FGTextEditor *)p->wnd->GetParameter();
		int size = strlen(tt->srch_str);
		
		switch(p->accel->GetLocalId())
		{
			case 0:
				tt->last_found = 0;
				if (size && tt->buf.lines>1)
				{
					tt->last_found = tt->Find(tt->last_found, tt->buf.lines-tt->last_found, size);
					if (tt->last_found<0) tt->last_found=0;
					else tt->_goto();
				}
				break;
			case 1:
				if (size && tt->buf.lines>1)
				{
					tt->last_found++;
					tt->last_found = tt->Find(tt->last_found, tt->buf.lines-tt->last_found,size);
					if (tt->last_found<0) tt->last_found=0;
					else tt->_goto();
				}
				break;

		}
	}
}

void FGAPI FGTextEditor::search(void)
{
	if (String_SearchPtr) String_SearchPtr->WindowFocus();
	else
	{
		create_line(terminate((char *)buf.buffer), buf.line);
		String_SearchPtr = new FGWindow(&String_SearchPtr, Text_EditorPtr->GetX()+160, Text_EditorPtr->GetY()+294, 284, 140, "String Search", SearchProc, CGRAY3, CDARK, 0x203|WESCAPE);
		String_SearchPtr->SetParameter(this);
		FGEditBox *eb = String_SearchPtr->AddEditBox(8, 24, 72, 160, "String", 's', srch_str);
		String_SearchPtr->AddPushButton(80, 64, 160, 21, "Search again", CR);
		eb->SetSize(32);
		eb->ClickUp(1);
	}
}

void FGTextEditor::SetFont(CallBack t)
{
	FGTextEditor *tt = (FGTextEditor *) t->GetParameter();
	tt->Clear();
	tt->_font = t->GetLocalId()-2;
	tt->Text_EditorPtr->set_font(tt->_font);
	tt->create_line(tt->terminate((char *)tt->buf.buffer), tt->buf.line);
	tt->ShowBuffer();
}

void FGAPI FGTextEditor::options(void)
{
	FGControl *pb;
	FGButtonGroup *bg;

	OptionsPtr = new FGWindow(&OptionsPtr, Text_EditorPtr->GetX()+100, Text_EditorPtr->GetY()+56, 300, 156, "Options", 0, CGRAY3, CDARK, 0x287&~WNOPICTO|WESCAPE);
	OptionsPtr->AddCheckBox(8, 16, "LINEFEED ONLY", 'l', &lfonly);
	OptionsPtr->AddEditBox(176, 12, 64, 40, "TAB SIZE", 't', &TABSIZE, 0, 1, 32);
	bg = new FGButtonGroup();
	pb = OptionsPtr->AddRadioButton(8, 80, "1", '1', 0, SetFont);
	pb->SetParameter(this);
	bg->AddToGroup(pb, _font==0);
	pb = OptionsPtr->AddRadioButton(8, 100, "2", '2', 0, SetFont);
	pb->SetParameter(this);
	bg->AddToGroup(pb, _font==1);
	pb = OptionsPtr->AddRadioButton(64, 80, "3", '3', 0, SetFont);
	pb->SetParameter(this);
	bg->AddToGroup(pb, _font==2);
	pb = OptionsPtr->AddRadioButton(64, 100, "4", '4', 0, SetFont);
	pb->SetParameter(this);
	bg->AddToGroup(pb, _font==3);
	pb = OptionsPtr->AddRadioButton(110, 80, "5", '5', 0, SetFont);
	pb->SetParameter(this);
	bg->AddToGroup(pb, _font==4);
	OptionsPtr->WindowText(8, 56, "Font size");
}

char * FGAPI FGTextEditor::terminate(char *s)
{
	int pom = FGMAX_LINESIZE-1;
	char *ss = s;
	while (pom && ss[pom]<=' ') pom--;
	if (pom<FGMAX_LINESIZE-1) pom++;
	ss[pom] = 0;
	return s;
}

//
// vytvori riadok s obsahom 's', na pozicii 'at'
// a inicializuje buffer s jeho obsahom
//
FGTextEditor::FGLINE* FGAPI FGTextEditor::create_line(char *s, int at)
{
	int size = s==0?0:text[at].size = strlen(s);
	char tmp[FGMAX_LINESIZE+4]="";
	if (size>FGMAX_LINESIZE) size = FGMAX_LINESIZE;
	if (size) memmove(tmp,s,size);
	s = tmp;
	char *t = (char *)text[at].text;
	if (t) free((void *)t);
	if (size != 0)
	{
		assert((t = (char *)malloc(size+1))!=0);
		text[at].text = (unsigned char *) t;
		memmove(t, s, size);
		t[size] = 0;
		memset(buf.buffer, ' ', FGMAX_LINESIZE);
		memmove(buf.buffer, t, size);
	}
	else
	{
		memset(buf.buffer, ' ', FGMAX_LINESIZE);
		text[at].text = 0;
	}
	return text + at;
}

void FGAPI FGTextEditor::Init(void)
{
	buf.lines = 1;
	buf.top = buf.line = 0;
	buf.posx = buf.posy = 0;
	memset(text, 0, sizeof(text));
	memset(buf.buffer, ' ', FGMAX_LINESIZE);
	create_line(buf.buffer, 0);
	Text_EditorPtr->ResetChange();
}

void FGAPI FGTextEditor::Open(char *s)
{
	int c,l=0,i=0;
	char b[FGMAX_LINESIZE+1];
	Init();
	FILE *f = fopen(s,"r");
	strcpy(buf.name, s);
	if (f==0) return;
	lfonly = 1;
	for(;;)
	{
		c = fgetc(f);
		if (c==EOF)
		{
			break;
		}
		else if (c>=' ')
		{
			b[i] = (char)c;
			if (i<FGMAX_LINESIZE-1) i++;
		}
		else if (c==TAB)
		{
			do
			{
				b[i] =  ' ';
				if (i<FGMAX_LINESIZE) i++;
			}
			while(i%TABSIZE);
		}
		else if (c == LF)
		{
			b[i]=0;
			create_line(b,l);
			i = 0;
			if (++l == FGMAX_LINE) break;
		}
		else if (c==CR) lfonly = 0;
	}
	if (l) buf.lines = l;
	Text_EditorPtr->ResetChange();
	fclose(f);
	create_line((char *)text[buf.line].text, buf.line);
}

//
// prida riadok nako niec suboru (bez viszualizacie)
// vracia 1 ak OK, inak 0
//
int FGAPI FGTextEditor::AppendLine(char *str)
{
	if (buf.lines >= FGMAX_LINE) return 0;
	create_line(str, buf.line);
	buf.lines++;
	buf.line++;
	create_line("", buf.line);
	Text_EditorPtr->SetChange();
	return 1;
}

/**
	Save editor's text to its file.
*/
void FGAPI FGTextEditor::SaveBuffer(void)
{
	int c,i=0;
	unsigned j;

	if (ronly) return;
	create_line(terminate((char *)buf.buffer), buf.line);
	FILE *f = fopen(buf.name,"wb");
	if (f==0) return;
	for(i=0;i<buf.lines;i++)
	{
		for(j=0;j<text[i].size;j++)
		{
			c = text[i].text[j];
			fputc(c,f);
		}
		if (lfonly==0) fputc(CR,f);
		fputc(LF,f);
	}
	Text_EditorPtr->ResetChange();
	fclose(f);
}

/**
	Save editor's text to this filename.
	@param s a new filename
*/
void FGAPI FGTextEditor::SaveAsBuffer(char *s)
{
	if (ronly) return;
	strcpy(buf.name, s);
	Text_EditorPtr->SetName(buf.name);
	SaveBuffer();
}

void FGTextEditor::SaveAsBuffer1(char *s, FGFileDialog *fd)
{
	FGTextEditor *tt = (FGTextEditor *) fd->GetParameter();
	tt->SaveAsBuffer(s);
}

void FGAPI FGTextEditor::SaveAsBuffer2(void)
{
	FGFileDialog *fd = new FGFileDialog(SaveAsBuffer1, 0, 0, "Save File", FDIALOG_SAVE | FDIALOG_MODAL | FDIALOG_SAVEDIR);
	fd->SetParameter(this);
}

/**
	Reinitialize editor. Clear the window & text buffer. All changes will be lost.
*/
void FGAPI FGTextEditor::NewBuffer(void)
{
	strcpy(buf.name, "Untitled");
	Text_EditorPtr->SetName(buf.name);
	for(int i=0; i<FGMAX_LINE; i++)
	{
		if (text[i].text) free(text[i].text);
		text[i].text = 0;
	}
	Clear();
	Init();
	ShowBuffer();
}

/**
	Reopen the buffer from the file. All changes will be lost.
*/
void FGAPI FGTextEditor::ReopenBuffer(void)
{
	OpenBuffer(buf.name);
}

/**
	Open new text file with the name 's' in the editor immediatelly.
*/
void FGAPI FGTextEditor::OpenBuffer(char *s)
{
	strcpy(buf.name, s);
	Text_EditorPtr->SetName(buf.name);
	Clear();
	Open(s);
	ShowBuffer();
}

void FGTextEditor::OpenBuffer1(char *s, FGFileDialog *fd)
{
	FGTextEditor *tt = (FGTextEditor *) fd->GetParameter();
	tt->OpenBuffer(s);
}

void FGAPI FGTextEditor::OpenBuffer2(void)
{
	FGFileDialog *fd = new FGFileDialog(OpenBuffer1, 0, 0, "Open File", FDIALOG_MODAL | FDIALOG_SAVEDIR);
	fd->SetParameter(this);
}

void FGAPI FGTextEditor::ShowCursor(int trigger)
{
	int i,p,c=0;

	c = buf.buffer[buf.posx];
	if (!trigger)
	{
		p = Text_EditorPtr->GetPaper();
		i = Text_EditorPtr->GetInk();
	}
	else
	{
		i = Text_EditorPtr->GetPaper();
		p = Text_EditorPtr->GetInk();
	}
	Text_EditorPtr->WindowText(buf.posx*FGCX+OFFSET_X, buf.posy*FGCY+OFFSET_Y, (char *)&c, i, p);
	sprintf(s,"%5d:%d [%d] INS: %s       ", buf.line, buf.posx, buf.lines, buf.ovr?"OFF":"ON ");

	Text_EditorPtr->set_font(2);
	Text_EditorPtr->WindowStatusBar(4,s,CWHITED);
	Text_EditorPtr->set_font(_font);
//	trigger ^= 1;
}

void FGAPI FGTextEditor::ShowLine(int scr, char *s)
{
	char ss[1028];
	if (s)
	{
		strncpy(ss,s,WX);
		ss[WX]=0;
	}
	Text_EditorPtr->WindowBox(OFFSET_X, scr*FGCY+OFFSET_Y, Text_EditorPtr->GetWW(), FGCY, Text_EditorPtr->GetPaper());
	if (s) Text_EditorPtr->WindowText(OFFSET_X, OFFSET_Y+scr*FGCY, ss);
}

/**
Refresh visuals.
@param top goto to the start if true.
*/
void FGAPI FGTextEditor::ShowBuffer(int top)
{
   if (top)
   {
	  buf.top = 0;
	  buf.posx=0;
	  buf.posy=0;
   }
   // comments because PGUP & comp. dont work
//	create_line(terminate((char *)buf.buffer), buf.line);
	WX = (Text_EditorPtr->GetWW()-OFFSET_X)/FGCX;
	WY = (Text_EditorPtr->GetHW()-OFFSET_Y)/FGCY;
	for(int i=0;i<WY && buf.top+i<buf.lines;i++)
	{
		ShowLine(i, (char *)text[buf.top+i].text);
	}
   if (top)
   {
	  buf.line = 0;
   }
	create_line((char *)text[buf.line].text, buf.line);
	ShowCursor(1);
}

int FGAPI FGTextEditor::isempty(char *s)
{
	for(int i=0;i<FGMAX_LINESIZE;i++) if (s[i] != ' ') return 0;
	return 1;
}

void FGAPI FGTextEditor::CKey(int k)
{
	int pom;
	switch(k)
	{
		case ALT_F:
			file();
			break;
		case ALT_S:
			search();
			break;
		case ALT_O:
			options();
			break;
		case CTRL_F09:
			if (inblock ^= 1)
			{
				buf.sel_line1 = buf.line;
				buf.sel_pos1  = buf.posx;
			}
			else
			{
				buf.sel_line2 = buf.line;
				buf.sel_pos2  = buf.posx;
			}
			break;
		case BACKSP:
			if (ronly) break;
			if (isempty(buf.buffer)) CKey(CTRL_Y);
			else if (buf.posx)
			{
				memmove(buf.buffer+buf.posx-1, buf.buffer+buf.posx, FGMAX_LINESIZE-buf.posx);
				buf.buffer[FGMAX_LINESIZE-1] = ' ';
				buf.posx--;
			}
			Text_EditorPtr->SetChange();
			break;
		case DEL:
			if (ronly) break;
			if (isempty(buf.buffer)) CKey(CTRL_Y);
			else if (buf.posx<FGMAX_LINESIZE-1)
			{
				memmove(buf.buffer+buf.posx, buf.buffer+buf.posx+1, FGMAX_LINESIZE-buf.posx);
				buf.buffer[FGMAX_LINESIZE-1] = ' ';
			}
			Text_EditorPtr->SetChange();
			break;
		case INSERT:
			buf.ovr = !buf.ovr;
			break;
		case HOME:
			buf.posx = 0;
			break;
		case END:
			buf.posx = FGMAX_LINESIZE-1;
			while (buf.posx && buf.buffer[buf.posx]<=' ') buf.posx--;
			if (buf.posx<FGMAX_LINESIZE-2) buf.posx++;
			break;
		case KUP:
			if (buf.line)
			{
				create_line(terminate(buf.buffer), buf.line);
				buf.line--;
				create_line((char *)text[buf.line].text, buf.line);
				if (buf.posy) buf.posy--;
				if (buf.top>buf.line)
				{
					buf.top = buf.line;
					Text_EditorPtr->WindowScrollDown(OFFSET_X, OFFSET_Y,WX*FGCX,(WY-1)*FGCY,FGCY);
				}
			}
			break;
		case KDOWN:
			if (buf.lines-1>buf.line)
			{
				create_line(terminate(buf.buffer), buf.line);
				buf.line++;
				create_line((char *)text[buf.line].text, buf.line);
				if (buf.posy<WY-1) buf.posy++;
			    if (buf.top+WY<=buf.line)
				{
					buf.top++;
					Text_EditorPtr->WindowScrollUp(OFFSET_X, OFFSET_Y+FGCY,WX*FGCX,(WY-1)*FGCY,FGCY);
				}
			}
			break;
		case CTRL_PGUP:
			create_line(terminate(buf.buffer), buf.line);
			buf.line = buf.top = buf.posx = buf.posy = 0;
			ShowBuffer();
			break;
		case CTRL_PGDOWN:
			create_line(terminate(buf.buffer), buf.line);
			if (buf.lines >= WY)
			{
				buf.line = buf.top = buf.lines-WY;
				buf.posx = buf.posy = 0;
				ShowBuffer();
			}
			break;
		case PGUP:
			create_line(terminate(buf.buffer), buf.line);
			if (buf.top-WY>=0)
			{
				buf.line -= WY;
				buf.top  -= WY;
				ShowBuffer();
			}
			else CKey(CTRL_PGUP);
			break;
		case PGDOWN:
			create_line(terminate(buf.buffer), buf.line);
			if (buf.line+WY<buf.lines)
			{
				buf.line += WY;
				buf.top  += WY;
				Clear();
				ShowBuffer();
			}
			else if (buf.line+WY>buf.lines) CKey(CTRL_PGDOWN);
			break;
		case KLEFT:
			if (buf.posx) buf.posx--;
			nodraw = 1;
			break;
		case KRIGHT:
			if (buf.posx<FGMAX_LINESIZE-1) buf.posx++;
			nodraw = 1;
			break;
		case CTRL_Y:
			if (ronly) break;
			if (buf.line+1<buf.lines)
			{
				memmove(&text[buf.line], &text[buf.line+1], sizeof(FGLINE)*(FGMAX_LINE-buf.line));
				text[FGMAX_LINE-1].size = 0;
				text[FGMAX_LINE-1].text = 0;
				buf.lines--;
				create_line((char *)text[buf.line].text, buf.line);
				if (buf.posy<WY-1)
				{
					int sz = WY-buf.posy;
					Text_EditorPtr->WindowScrollUp(OFFSET_X, (buf.posy+1)*FGCY+OFFSET_Y,WX*FGCX,(sz-1)*FGCY,FGCY);
					if (buf.line+sz<buf.lines) ShowLine(WY-1, (char *)text[buf.line+sz-1].text);
				}
			}
			else create_line("", buf.line); // at the end of document
			Text_EditorPtr->SetChange();
			break;
		case CR:
			if (ronly) break;
			pom = FGMAX_LINESIZE-1;
			while (pom && buf.buffer[pom]<=' ') pom--;
			if (pom<FGMAX_LINESIZE-1) pom++;
			if (pom>buf.posx) pom -= buf.posx; // set size of part 2
			else pom = 0; // no part 2
			strncpy(s, buf.buffer+buf.posx, pom);
			s[pom]=0;	// terminate line
			buf.buffer[buf.posx] = 0;
			create_line(buf.buffer, buf.line);
			ShowLine(buf.posy, buf.buffer);
			if (buf.lines<FGMAX_LINE)
			{
				buf.lines++;
				buf.line++;
				memmove(&text[buf.line], &text[buf.line-1], sizeof(FGLINE)*(FGMAX_LINE-buf.lines));
				text[buf.line].size = strlen(s)+1;
				text[buf.line].text = (unsigned char *)strdup(s);
				create_line(s, buf.line);
				buf.posx = 0;
				if (buf.posy<WY-1)
				{
					buf.posy++;
					Text_EditorPtr->WindowScrollDown(OFFSET_X, buf.posy*FGCY+OFFSET_Y, WX*FGCX,(WY-buf.posy-1)*FGCY,FGCY);
				}
			    else
				{
					buf.top++;
					ShowBuffer();
				}
			}
			Text_EditorPtr->SetChange();
			break;
	}
}

void FGAPI FGTextEditor::Key(int k)
{
	ShowCursor(0);
	if (k<' ' || k>0x7e) CKey(k);
	else if (!ronly)
	{
		if (buf.ovr==0) memmove(buf.buffer+buf.posx+1, buf.buffer+buf.posx, FGMAX_LINESIZE-buf.posx-1);
		buf.buffer[buf.posx] = (char)k;
		if (buf.posx<FGMAX_LINESIZE-1) buf.posx++;
		Text_EditorPtr->SetChange();
	}
	if (!nodraw) ShowLine(buf.posy, buf.buffer);
	nodraw = 0;
	ShowCursor(1);
}

void FGAPI FGTextEditor::SaveChanges(void)
{
	SaveBuffer();
}

void FGTextEditor::Text_EditorProc(FGEvent *p)
{
	if (p->GetType() == INITEVENT)
		return;

	FGTextEditor *tt = (FGTextEditor *) p->wnd->GetParameter();
	int	xWnd = p->GetX()-OFFSET_X;
	int	yWnd = p->GetY()-OFFSET_Y;

	switch(p->GetType()) {
		case ACCELEVENT:
			switch(p->accel->GetLocalId())
			{
				case 0:
					tt->file();
					break;
				case 1:
					tt->search();
					break;
				case 2:
					tt->options();
					break;
			}
			break;
		case KEYEVENT:
			tt->Key(p->GetKey());
			break;
		case MOUSEWHEELEVENT:
			if (p->GetButtons()<0)
				tt->Key(KUP);
			else if (p->GetButtons()>0)
				tt->Key(KDOWN);
			break;
		case CLICKLEFTEVENT:
			xWnd = xWnd/__fg_font_tab[tt->_font][0];
			yWnd = yWnd/__fg_font_tab[tt->_font][1];
			if (xWnd<0 || yWnd<0 || xWnd>=tt->WX || yWnd>=tt->WY) return;
			tt->ShowCursor(0);
			tt->create_line((char *)tt->buf.buffer, tt->buf.line);
			tt->buf.posx = xWnd;
			if (tt->buf.top+yWnd < tt->buf.lines)
				tt->buf.posy = yWnd;
			else
				tt->buf.posy = tt->buf.lines-tt->buf.top;
			tt->buf.line = tt->buf.top + tt->buf.posy;
			tt->create_line((char *)tt->text[tt->buf.line].text, tt->buf.line);
			tt->ShowLine(tt->buf.posy, tt->buf.buffer);
			tt->ShowCursor(1);
			break;
		case CLICKRIGHTEVENT:
			break;
		case WINDOWRESIZEEVENT:
			tt->create_line((char *)tt->buf.buffer, tt->buf.line);
			tt->ShowBuffer();
			tt->W = p->wnd->GetW();
			tt->H = p->wnd->GetH();
		case MOVEEVENT:
			tt->X = p->wnd->GetX();
			tt->Y = p->wnd->GetY();
			break;
		case NOTIFYEVENT:
        	if (p->GetKey()) tt->SaveChanges();
			delete tt;
			break;
		case TERMINATEEVENT:
			break;
	}
}

void FGAPI FGTextEditor::Destruct(void)
{
	for(int i=0; i<FGMAX_LINE; i++)
	{
		if (text[i].text) free(text[i].text);
		text[i].text = 0;
	}
}

FGTextEditor::~FGTextEditor()
{
	cCfg->WriteInt("_editor_lfonly",lfonly);
	cCfg->WriteInt("_editor_TABSIZE",TABSIZE);
	cCfg->WriteInt("_editor_font",_font);
	cCfg->WriteInt("_editor_X", X);
	cCfg->WriteInt("_editor_Y", Y);
	cCfg->WriteInt("_editor_W", W);
	cCfg->WriteInt("_editor_H", H);
	cCfg->Sync();
	if (String_SearchPtr) delete String_SearchPtr;
	if (OptionsPtr) delete OptionsPtr;
	Destruct();
	delete Text_EditorPtr;
	Text_EditorPtr = 0;
	if (self) *self = 0;
}

/**
	Open the text editor with contents of file 'arg'. The colors and font
	is optionally.
*/
FGTextEditor::FGTextEditor(FGTextEditor **me, char *arg, int font, int ink, int paper, int flags)
{
	OptionsPtr = String_SearchPtr = 0;
	lfonly = font_save = 0;
	srch_str[0] = 0;
	last_found = nodraw = inblock = ronly = 0;
	X = Y = 0;
	W = 400;
	H = 300;
	self = me;
	TABSIZE=4;
	cCfg->ReadInt("_editor_lfonly",lfonly);
	cCfg->ReadInt("_editor_TABSIZE",TABSIZE);
	cCfg->ReadInt("_editor_font",_font);
	cCfg->ReadInt("_editor_X", X);
	cCfg->ReadInt("_editor_Y", Y);
	cCfg->ReadInt("_editor_W", W);
	cCfg->ReadInt("_editor_H", H);

	Text_EditorPtr = new FGWindow(0, X, Y, W, H, buf.name, Text_EditorProc, ink, paper, flags);
	_font = FONT0816;
	if (font == -1) font = _font;  // set default
	Text_EditorPtr->set_font(_font = font);

	Open(arg);

	Text_EditorPtr->SetName(buf.name);
	Text_EditorPtr->SetParameter(this);
	(Text_EditorPtr->AddBaseMenu("File", ALT_F))->SetParameter(this);
	(Text_EditorPtr->AddBaseMenu("Search", ALT_S))->SetParameter(this);
	(Text_EditorPtr->AddBaseMenu("Options", ALT_O))->SetParameter(this);
	ShowBuffer();

	if (self==0)
		printf("You may pass the first argument of FGTextEditor() as non NULL!");
	else
		*self = this;
}

#ifdef FG_NAMESPACE
}
#endif


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
    Some widgets

*/

#ifndef _WIN32
#include <sys/time.h>
#endif

#include "fastgl.h"
#include "listbox.h"
#include "widgets.h"
#include "_fastgl.h"

#include <vector>

#ifdef FG_NAMESPACE
namespace fgl {
#endif

typedef std::vector<FGControl *> FGCtrlContainer;
typedef FGCtrlContainer::iterator CPTR;

char FGFileDialog::delimiter[2]=
#if defined (__MSDOS__) || defined (_WIN32)
"\\";
#else
"/";
#endif

//---------------------------------------------------------------------------

unsigned int FGColorDialog::custom_colors[256] = {
0x000000, 0x0000a8, 0x00a800, 0x00a8a8, 0xa80000, 0xa800a8, 0xa85400,
0xa8a8a8, 0x545454, 0x5454fc, 0x54fc54, 0, 0xfc5454, 0xfc54fc,
0xfcfc54, 0xfcfcfc, 0x000000, 0x141414, 0x202020, 0x2c2c2c, 0x383838,
0x444444, 0x505050, 0x606060, 0x707070, 0x808080, 0x909090, 0xa0a0a0,
0xb4b4b4, 0xc8c8c8, 0xe0e0e0, 0xfcfcfc, 0x0000fc, 0x4000fc, 0x7c00fc,
0xbc00fc, 0xfc00fc, 0xfc00bc, 0xfc007c, 0xfc0040, 0xfc0000, 0xfc4000,
0xfc7c00, 0xfcbc00, 0xfcfc00, 0xbcfc00, 0x7cfc00, 0x40fc00, 0x00fc00,
0x00fc40, 0x00fc7c, 0x00fcbc, 0x00fcfc, 0x00bcfc, 0x007cfc, 0x0040fc,
0x7c7cfc, 0x9c7cfc, 0xbc7cfc, 0xdc7cfc, 0xfc7cfc, 0xfc7cdc, 0xfc7cbc,
0xfc7c9c, 0xfc7c7c, 0xfc9c7c, 0xfcbc7c, 0xfcdc7c, 0xfcfc7c, 0xdcfc7c,
0xbcfc7c, 0x9cfc7c, 0x7cfc7c, 0x7cfc9c, 0x7cfcbc, 0x7cfcdc, 0x7cfcfc,
0x7cdcfc, 0x7cbcfc, 0x7c9cfc, 0xb4b4fc, 0xc4b4fc, 0xd8b4fc, 0xe8b4fc,
0xfcb4fc, 0xfcb4e8, 0xfcb4d8, 0xfcb4c4, 0xfcb4b4, 0xfcc4b4, 0xfcd8b4,
0xfce8b4, 0xfcfcb4, 0xe8fcb4, 0xd8fcb4, 0xc4fcb4, 0xb4fcb4, 0xb4fcc4,
0xb4fcd8, 0xb4fce8, 0xb4fcfc, 0xb4e8fc, 0xb4d8fc, 0xb4c4fc, 0x000070,
0x1c0070, 0x380070, 0x540070, 0x700070, 0x700054, 0x700038, 0x70001c,
0x700000, 0x701c00, 0x703800, 0x705400, 0x707400, 0x547000, 0x387000,
0x1c7000, 0x007000, 0x00701c, 0x007038, 0x007054, 0x007070, 0x005470,
0x003870, 0x001c70, 0x383870, 0x443870, 0x543870, 0x603870, 0x703870,
0x703860, 0x703854, 0x703844, 0x703838, 0x704438, 0x705438, 0x706038,
0x707038, 0x607038, 0x547038, 0x447038, 0x387038, 0x387044, 0x387054,
0x387060, 0x387070, 0x386070, 0x385470, 0x384470, 0x505070, 0x585070,
0x605070, 0x685070, 0x705070, 0x705068, 0x705060, 0x705058, 0x705050,
0x705850, 0x706050, 0x706850, 0x707050, 0x687050, 0x607050, 0x587050,
0x507050, 0x507058, 0x507060, 0x507068, 0x507070, 0x506870, 0x506070,
0x505870, 0x000040, 0x100040, 0x200040, 0x300040, 0x400040, 0x400030,
0x400020, 0x400010, 0x400000, 0x401000, 0x402000, 0x403000, 0x404000,
0x304000, 0x204000, 0x104000, 0x004000, 0x004010, 0x004020, 0x004030,
0x004040, 0x003040, 0x002040, 0x001040, 0x202040, 0x282040, 0x302040,
0x382040, 0x402040, 0x402038, 0x402030, 0x402028, 0x402020, 0x402820,
0x403020, 0x403820, 0x404020, 0x384020, 0x304020, 0x284020, 0x204020,
0x204028, 0x204030, 0x204038, 0x204040, 0x203840, 0x203040, 0x202840,
0x2c2c40, 0x302c40, 0x342c40, 0x3c2c40, 0x402c40, 0x402c3c, 0x402c34,
0x402c30, 0x402c2c, 0x40302c, 0x40342c, 0x403c2c, 0x40402c, 0x3c402c,
0x34402c, 0x30402c, 0x2c402c, 0x2c4030, 0x2c4034, 0x2c403c, 0x2c4040,
0x2c3c40, 0x2c3440, 0x2c3040, 0x000000, 0x000000, 0x000000, 0x000000,
0x000000, 0x000000, 0x000000, 0x000000 };

char FileDlgWrapper::tmpfilename[FGFileDialog::max_pathlen];

void  FGAPI FGFileDialog::BuildFullPathname(char *tmp)
{
	*tmp=0;
	strcpy(tmp,path);
#if defined (__MSDOS__) || defined (_WIN32)
	if (*tmp && tmp[1]==':' && tmp[3]) strcat(tmp, delimiter);
#else
	if (*tmp) strcat(tmp, delimiter);
#endif
	if (filename[0]=='.' && filename[1]=='.' && filename[2]!=0)
		strcpy(filename, filename+2);
	strcat(tmp, filename);
}

void  FGAPI FGFileDialog::BuildRelativePathname(char *tmp)
{
	getcwd(tmppath, sizeof(tmppath)-1);
	BuildFullPathname(tmp);
	char* ptr = tmppath, *ptr2 = tmp, relpath[max_pathlen];
	while(*ptr)
	{
		if (*ptr != *ptr2) break;
		ptr++;
		ptr2++;
	}
	if (*ptr == 0)
	{
		while (*ptr2 == *delimiter) ptr2++;
	}
	else
	{
		relpath[0] = 0;
		do {
			strcat(relpath, "..");
	    	strcat(relpath, delimiter);
			if (*ptr == *delimiter) ptr++;
		} while ((ptr = strchr(ptr, delimiter[0])) != 0);
		strcat(relpath, filename);
		ptr2 = relpath;
	}
	strcpy(tmp, ptr2);
}

/**
* This functions 'realy' call user callback with selected filename
* i.e. -> when you click 'OK' in dialog box
*/
bool FGFileDialog::SelectFile(void)
{
	char tmp[max_pathlen*2];

	if (fileselect2)
	{
		BuildFullPathname(tmp);
		FILE *fp=fopen(tmp, "r");  // if exists ..
		if (fp) fclose(fp);
		if (mode&FDIALOG_OPEN)
		{
			if (fp)
			{
				fileselect2(tmp, this);
				return true;
			}
		}
		else
		{
			fileselect2(tmp, this);
			return true;
		}
	}
	return false;
}

int FGFileDialog::Selected(void)
{
	char tmp[max_pathlen*2];
	char tmp2[max_pathlen];
	int retval=0;

	BuildFullPathname(tmp);

	// save original path
	getcwd(tmp2, sizeof(tmp2)-1);
	// change to the new path
	retval = chdir(tmp);

	if (retval!=0 && (filename[0]==*delimiter || filename[1]==':'))
	{
		retval=chdir(filename); // try absolute path
		if (retval==0)
			strcpy(tmp,filename);
	}
	// restore original path
	chdir(tmp2);
	// it was DIRectory, simply reload & return
	if (retval==0)
	{
		Refresh(tmp);
		return false;
	}

//	if (strncmp(tmp, tmp2, strlen(tmp2))==0)
//		strcpy(tmp,tmp+strlen(tmp2)+1);

	return false;
}

void FGFileDialog::Refresh(const char *newpath)
{
	int i;
	char name[max_pathlen];

	if (path != newpath)
		strcpy(path, newpath);

	reload();
	list->clear();
	list2->clear();

	filename[0] = 0;
	nameEBox->ChangeItem(filename);
	wnd->WindowBox(8,224,280,20, wnd->GetPaper());

	for(i=0;i<=dirs;i++)
	{
		sprintf(name, "%s",  filebuffer[i].item.d_name);
		list->insert(name);
	}
	for(;i<files;i++)
	{
		sprintf(name, "%s",  filebuffer[i].item.d_name);
		list2->insert(name);
	}
	list->Update();
	list2->Update();
	list->Down();

	int len = strlen(path);
	wnd->WindowBox(4,8,300,17,wnd->GetPaper());
	if (len<=48)
	    wnd->printf(4,8,"%.48s", path);
	else
		wnd->printf(4,8,"... %.48s", path+(len-48));
	if (fls)
	{
		strcpy(filename, filebuffer[dirs+1].item.d_name);
		filebuffer[dirs+1].selected = true;
		nameEBox->ChangeItem(filename);
	}
}

void FGFileDialog::InputName(CallBack cb)
{
	char s[max_pathlen];
	FGFileDialog *instance = (FGFileDialog *)(cb->GetPrivateData());

	if (instance->mdir)
	{				// no, make directory and stay
	    int err;

		instance->BuildFullPathname(s);
#if (defined(_WIN32) && !defined(__CYGWIN__))
		err = ::mkdir(s);
#else
		err = ::mkdir(s, 0777);
#endif
		if (err==-1) perror(s);
		strcpy(instance->filename, s);
		instance->Selected();
		instance->Refresh(s);
		instance->mdir = 0;
		return;
	}
	else
	{
		if (!instance->Selected()) return;
	    if (instance->wnd) delete instance->wnd;
	}
}

void FGFileDialog::Mkdir(CallBack cb)
{
	FGFileDialog *instance = (FGFileDialog *)(cb->GetPrivateData());

	instance->mdir = 1;
	instance->filename[0] = 0;
	instance->nameEBox->ChangeItem(instance->filename);
	instance->nameEBox->ClickUp(1);
}

void FGFileDialog::Ok(CallBack cb)
{
	FGFileDialog *instance = (FGFileDialog *)(cb->GetPrivateData());

	if (instance->wnd->WindowFlushInput())
		return;
	else
		instance->SelectFile();

	FGControl::Close(cb);
}

void FGFileDialog::Updir(CallBack cb)
{
	FGFileDialog *instance = (FGFileDialog *)(cb->GetPrivateData());
	strcpy(instance->filename, "..");
	instance->Selected();
}

void FGFileDialog::Home(CallBack cb)
{
	FGFileDialog *instance = (FGFileDialog *)(cb->GetPrivateData());
	instance->Refresh(FGApp::homedir);
}

void FGFileDialog::myproc(FGEvent *p)
{
	FGFileDialog *instance = (FGFileDialog *)(p->wnd->GetPrivateData());
	switch(p->GetType())
	{
		case KEYEVENT: if (instance->list->DoListBox(p->GetKey())==0)
		    switch(p->GetKey())
		{
			case ' ':
				instance->nameEBox->ClickUp(1);
				break;
			case CTRL_PGUP:
			case BACKSP:
				instance->list->Begin();
				strcpy(instance->filename, "..");
			case CR:
				instance->Selected();
				break;
			case KLEFT:
				instance->list2->Up();
				break;
			case KRIGHT:
				instance->list2->Down();
				break;
		}
		break;

		case DBLCLICKLEFTEVENT:
		    Ok(instance->ok);
		    break;
		case CLICKLEFTEVENT:
		{
			int rc;
			rc = instance->list->Test(p->GetX(), p->GetY());
			if (rc>=0)
			{
				instance->list->SetIndex(rc);
				instance->Selected();
			}
			else
			{
				rc = instance->list2->Test(p->GetX(), p->GetY());
	    		if (rc>=0)
		    	{
			    	instance->list2->SetIndex(rc);
//				    instance->Selected();
				}
			}
		}
		break;

		case TERMINATEEVENT:
// fix: deleted automatically
/*
			delete instance->list;
			instance->list = 0;
			delete instance->list2;
			instance->list2 = 0;
*/
			delete[] instance->filebuffer;
			instance->filebuffer = 0;
			break;
	}
}

void FGFileDialog::drawone(int flg, FGWindow *wnd,int x, int y, void *ptr, int c)
{
	FGFileDialog *p = (FGFileDialog *) wnd->GetPrivateData();

	if (!flg)
	{
		wnd->WindowText(x+4,y,(char *)ptr, CBLUELIGHT, wnd->GetPaper());
	}
	else
	{
		if ((*(char *)ptr)=='.' && (*(char *)ptr+1)=='.')
		{
			((char *)ptr)[2]=0;
			p->nameEBox->ChangeItem((char *)ptr+2);
		}
		wnd->WindowText(x+4,y,(char *)ptr, wnd->GetPaper(), wnd->GetInk());
		strcpy(p->filename, (char *)ptr);
	}
}

void FGFileDialog::drawone2(int flg, FGWindow *wnd,int x, int y, void *ptr, int c)
{
	FGFileDialog *p = (FGFileDialog *) wnd->GetPrivateData();
	struct stat s;
	getcwd(p->tmppath, sizeof(p->tmppath)-1);
	chdir(p->path);
	stat(p->list2->item(c).str, &s);
	chdir(p->tmppath);

	if (!flg)
	{
		wnd->WindowText(x+4,y,(char *)ptr, s.st_mode&0100 ? CGREEN : wnd->GetInk(), wnd->GetPaper());
	}
	else
	{
		wnd->WindowText(x+4,y,(char *)ptr, wnd->GetPaper(), wnd->GetInk());
		strcpy(p->filename, (char *)ptr);
		p->nameEBox->ChangeItem(p->filename);
		wnd->WindowBox(8,224,280,20, wnd->GetPaper());
		char *ss = ctime(&s.st_atime)+4;
		ss[strlen(ss)-1] = 0;
		wnd->SetColors();
		wnd->printf(8,228,"%s, uid(%d):%04o, %d bytes", ss, s.st_uid, s.st_mode&0xfff, s.st_size);
	}
}

static int sort_name(void const *a, void const *b)
{
	return stricmp(((mydirent *)a)->item.d_name, ((mydirent *)b)->item.d_name);
}

static int sort_type(void const *a, void const *b)
{
	return ((mydirent *)a)->type - ((mydirent *)b)->type;
}

void FGFileDialog::reload(void)
{
	DIR *dirp;
	struct dirent *direntp;
	struct stat st;
	int i = 0, j=0, ne=0, q=0, isdir;
	char extensions[64][16];
	const char* p=filter;

	dirs = fls = files = 0;
	getcwd(tmppath, sizeof(tmppath)-1);
	chdir(path);
	getcwd(path, sizeof(path)-1);  // reload path string in case "."
	dirp = opendir(path);

	if (dirp == NULL) goto ende;
	else
	{	// expand filters
		if (p)
		{
			while(*p)
			{
				extensions[ne][j] = *p;
				if (*p==' ' || j==15)
				{
					extensions[ne++][j]=0;
					j=0;
					if (ne==64) break;
				}
				else j++;
				p++;
			}
			extensions[ne++][j]=0;
		}

		for (;;)
		{
			direntp = readdir(dirp);
			if (direntp == NULL) // finale
				break;
			if (strcmp(direntp->d_name, "."))
			{
				stat(direntp->d_name, &st);
				isdir = S_ISDIR(st.st_mode);
				if (isdir) dirs++;
				if (ne && !isdir) for(q=0;q<ne;q++)
				{
					if (strstr(direntp->d_name, extensions[q])) break;
				}
				if (ne && q==ne && strcmp(direntp->d_name, "..") && !isdir)
				{
					continue; // no valid extension
				}
				memcpy(&filebuffer[i].item, direntp, sizeof(struct dirent));
				filebuffer[i].type = !isdir;
				filebuffer[i].selected = false;
				if (!isdir) fls++;
				if (++i == maxfiles)
				{
					maxfiles *= 2;
					struct mydirent *xmemory = new mydirent[maxfiles];
					memcpy(xmemory, filebuffer, maxfiles/2*sizeof(mydirent));
					delete [] filebuffer;
					filebuffer = xmemory;
				}
			}
		}
		closedir(dirp);
	}
	files = i;
	qsort(filebuffer, files, sizeof(mydirent), sort_type);
	qsort(filebuffer, dirs, sizeof(mydirent), sort_name);
	qsort(filebuffer+dirs, fls, sizeof(mydirent), sort_name);
	dirs--;
ende:
	chdir(tmppath);
	return;
}

bool FGFileDialog::TestForOverwrite(void)
{
	char here[sizeof(path)-1];

	BuildRelativePathname(here);

	FILE* fp = fopen(here, "r");

	if (fp == 0)
	{
		return true;
	}
	else
		fclose(fp);

	FGWindow* wnd = new FGWindow(&wnd, 0,0,240,120,"Replace the file?", 0, IM, PM, WSTANDARD|WCENTRED);
	FGControl* ctrl;

	wnd->WindowText(8,16,here);

	ctrl = wnd->AddPushButton(32,40,64,23,"Yes",'y');
	ctrl->SetParameter((void *) (mrYes));
	ctrl = wnd->AddPushButton(140,40,64,23,"No",'n');
	ctrl->SetParameter((void *) (mrNo));

	int rc = wnd->ShowModal();

	delete wnd;

	switch( rc )
	{
		case mrYes:
		    return true;
	}
	return false;
}

void FGFileDialog::_init(const char *dir, const char *flt, const char *namewnd, int m, int ink, int paper)
{
	mdir = 0;
	maxfiles = 256;
	filebuffer = new mydirent[maxfiles];
	if (!(m&FDIALOG_SAVEDIR && path[0]))
	{
		if (dir)
		    strcpy(path, dir);
		else
		    getcwd(path, sizeof(path)-1);
	}
	filter = flt;
	mode = m;
	wnd = new FGWindow(&wnd, 100,100,464,280, namewnd, myproc, ink, paper, WUSELAST|WSTANDARD|WESCAPE+WMODAL*(m&FDIALOG_MODAL?1:0));
	wnd->SetPrivateData((void *)(this));
	wnd->set_font(FONTSYSLIGHT);

	strcpy(filename, "");
	nameEBox = wnd->AddEditBox(8,196,72,368,"File name:",'N',filename,InputName);
	nameEBox->SetPrivateData((void *) this);

	ok = wnd->AddPushButton(316,224,64,21,"Ok", 'o', Ok);
	ok->SetPrivateData((void *) this);
	cancel = wnd->AddPushButton(384,224,64,21,"Cancel", 'c', FGControl::Close);
	cancel->SetPrivateData((void *) this);
	updir = wnd->AddPushButton(306,3,32,21,"UP",'u', Updir);
	updir->SetPrivateData((void *) this);
	mkdir = wnd->AddPushButton(340,3,52,21,"MKDIR",'m', Mkdir);
	mkdir->SetPrivateData((void *) this);
	home = wnd->AddPushButton(394,3,52,21,"HOME",'h', Home);
	home->SetPrivateData((void *) this);

	list = new FGListBox(8,28,name_width,16, 10, wnd, drawone);
	list2 = new FGListBox(228,28,name_width,16, 10, wnd, drawone2);
	Refresh(path);
}

FGFileDialog::FGFileDialog(FileDialogCallBack filesel, const char *dir, const char *flt, const char *namewnd, int m, int ink, int paper)
{
	fileselect2 = filesel;
	_init(dir, flt, namewnd, m, ink, paper);
}

// ----------------------------------------------------------------------------------------

void FGColorDialog::SetColorsProc(FGEvent *p)
{
	int	xWnd = p->GetX()-8;
	int	yWnd = p->GetY()-8;
	int item;

	FGColorDialog *ptr = (FGColorDialog *)p->wnd->GetParameter();

	switch(p->GetType())
	{
		case ACCELEVENT:
			if (p->accel->GetLocalId()==0)
			{
#ifdef INDEX_COLORS
					CreateColor(ptr->cd_r>>2, ptr->cd_g>>2, ptr->cd_b>>2, ptr->color);
#endif
				delete p->wnd;
				if (ptr->fnc)
					ptr->fnc(ptr->color);
				ptr->fnc = 0;
				break;
			}
			else if (p->accel->GetLocalId()==1)
			{
				delete ptr->wnd;
				break;
			}
			break;
		case TERMINATEEVENT:
#ifdef INDEX_COLORS
//			_set_fgl_palette();
#endif
			break;
		case CLICKLEFTEVENT:
			if (yWnd<0 || xWnd<0) return;
			xWnd = xWnd/20;
			yWnd = yWnd/20;
			if (yWnd>15 || xWnd>15) return;
			item = xWnd + yWnd*16;
			ptr->FocusTo(item);
			break;
	}
}

void FGColorDialog::CSliderFnc(CallBack cb)
{
	FGColorDialog *ptr = (FGColorDialog *)cb->GetParameter();
	if (ptr != (FGColorDialog *)-1)
		ptr->UpdatePalette();
}

FGColorDialog::FGColorDialog(char *capture, ColorDialogCallBack fnc, FGPixel bgc)
{
	int i,j;
	FGColorDialog::fnc = fnc;
	curr = 0;
	cd_r = cd_g = cd_b = 0;
	wnd = new FGWindow(&wnd, 240, 80, 520, 364, capture, SetColorsProc, 0, bgc, 0x203|WUSELAST|WESCAPE);
	UpdatePalette();
	wnd->SetParameter(this);
	(wnd->AddPushButton(428, 32, 72, 21, "Ok", CR))->SetFont(FONT0816);
	(wnd->AddPushButton(428, 64, 72, 21, "Cancel"))->SetFont(FONT0816);
	sl1 = wnd->AddSlideBarV(340, 5, 0, 255, 1, &cd_r, CSliderFnc);
	sl1->SetParameter((void *) (this));
	sl2 = wnd->AddSlideBarV(370, 5, 0, 255, 1, &cd_g, CSliderFnc);
	sl2->SetParameter((void *) (this));
	sl3 = wnd->AddSlideBarV(400, 5, 0, 255, 1, &cd_b, CSliderFnc);
	sl3->SetParameter((void *) (this));

	for(i=0; i<16; i++)
		for(j=0; j<16; j++)
	{
#ifdef INDEX_COLORS
		wnd->WindowBox(8+j*20,  8+i*20, 16, 16, j+i*16);
		if (ColorsArray[(j+i*16)].alfa)
			wnd->WindowRect(8+j*20-1, 8+i*20-1, 18, 18, CYELLOW);
#else
		wnd->WindowBox(8+j*20,  8+i*20, 16, 16, FGDirectColor(custom_colors[j+i*16]));
#endif
	}
	FocusTo(0);
}

void FGAPI FGColorDialog::FocusTo(int a)
{
	int cc;
	if (ColorsArray[curr].alfa) cc = CYELLOW;
	else cc = wnd->GetPaper();
	wnd->WindowRect(6+(curr%16)*20, 6+(curr/16)*20, 20, 20, wnd->GetPaper());
	wnd->WindowRect(7+(curr%16)*20, 7+(curr/16)*20, 18, 18, cc);
	curr=a;
#ifdef INDEX_COLORS
	color = a;
#else
	color = FGDirectColor(custom_colors[curr]);
#endif
	wnd->WindowBox(428,128,72,72,color);
	wnd->WindowRect(6+(curr%16)*20, 6+(curr/16)*20, 20, 20, CWHITE);
	wnd->WindowRect(7+(curr%16)*20, 7+(curr/16)*20, 18, 18, CWHITE);
	wnd->SetColors();
	wnd->printf(450,8,"%03d\n", curr);
#ifdef INDEX_COLORS
	unsigned pal = get_palette(curr);
	cd_r = pal>>16;
	cd_g = (pal>>8)&255;
	cd_b = (pal)&255;
#else
	cd_r = custom_colors[curr]>>16;
	cd_g = (custom_colors[curr]>>8)&255;
	cd_b = custom_colors[curr]&255;
#endif
	sl1->redraw();
	sl2->redraw();
	sl3->redraw();
	UpdatePalette();
}

void FGColorDialog::UpdatePalette(void)
{
#ifdef INDEX_COLORS
	unsigned pal = ((cd_r<<16) + (cd_g<<8) + (cd_b));
	vector_palette(curr, pal);
#else
	custom_colors[curr] = (cd_r<<16) + (cd_g<<8) + cd_b;
	color = FGDirectColor(custom_colors[curr]);
	wnd->WindowBox(428,128,72,72,color);
	wnd->WindowBox(8+(curr%16)*20, 8+(curr/16)*20, 16, 16, color);
#endif
	wnd->printf(438,240,"R: %03d\n", cd_r);
	wnd->printf(438,260,"G: %03d\n", cd_g);
	wnd->printf(438,280,"B: %03d\n", cd_b);
}

int FGAPI FGColorDialog::Import(char *name)
{
	FILE *f = fopen(name, "rb");
	if (f)
	{
#ifdef INDEX_COLORS
		memcpy(custom_colors, _fgl_palette, sizeof(_fgl_palette));
#endif
		int rc = fread(custom_colors, sizeof(custom_colors), 1, f);
		fclose(f);
#ifdef INDEX_COLORS
		memcpy(_fgl_palette, custom_colors, sizeof(_fgl_palette));
#endif
		return rc;
	}
	return 0;
}

int FGAPI FGColorDialog::Export(char *name)
{
	FILE *f = fopen(name, "wb");
	if (f)
	{
		int rc = fwrite(custom_colors, sizeof(custom_colors), 1, f);
		fclose(f);
		return rc;
	}
	return 0;
}

void FGUpDown::handler1(CallBack cb)
{
	FGUpDown * p = (FGUpDown *)cb->GetParameter();
	p->eb->UpDown(p->step);
	if (p->eb->GetStatus() == BPUSHED)
		p->eb->FlushInput(true);
	p->eb->draw();
	if (p->eb->fnc)
		p->eb->fnc(p->eb);
}

void FGUpDown::handler2(CallBack cb)
{
	FGUpDown * p = (FGUpDown *)cb->GetParameter();
	p->eb->UpDown(-p->step);
	if (p->eb->GetStatus() == BPUSHED)
		p->eb->FlushInput(true);
	p->eb->draw();
	if (p->eb->fnc)
		p->eb->fnc(p->eb);
}

FGUpDown::FGUpDown(FGEditBox *e, float _step)
{
	eb = e;
	step = _step;
	FGWindow *w = eb->GetOwner();
	int x = eb->GetXr() - w->GetX() + eb->GetW() - w->GetXW();
	int y = eb->GetYr() - w->GetY() - w->GetYW();

	up   = w->AddPushButton(x+1, y-2,   13,13,"+",0,handler1);
	up->SetParameter((void *) (this));
	down = w->AddPushButton(x+1, y+11,  13,13,"--",0,handler2);
	down->SetParameter((void *) (this));
}

FGFontDialog::FGFontDialog(FontDialogCallBack f, int which, char *capture, FGPixel bgc)
{
	userfnc = f;
	curr = 0;
	wnd = new FGWindow(&wnd, 240, 80, 300, 240, capture, SetFontProc, 0, bgc, 0x203|WUSELAST|WESCAPE);
	wnd->SetParameter( this );
	wnd->set_font(FONT1222);
	wnd->set_font(FONT1625);
	wnd->set_font(FONT2034);
	wnd->set_font(FONT0816);
	(wnd->AddPushButton(208, 16, 72, 23, "Ok", CR))->SetFont(FONT0816);
	(wnd->AddPushButton(208, 46, 72, 23, "Cancel"))->SetFont(FONT0816);
	lb = wnd->AddListBox(10,16,168,16,8,0);
	
	for(int i=0; i<FGFontManager::GetCounter(); i++)
	{
		if (which&FONT_DIALOG_FIXED && FGFontManager::IsVariableFont(i) == 0
		 || which&FONT_DIALOG_VARIABLE && FGFontManager::IsVariableFont(i))
			lb->insert(id_string(FGFontManager::GetFontName(i)));
	}
	lb->Update();
	ShowFont();
}

void FGFontDialog::ShowFont(void)
{
	int old = wnd->get_font();
	wnd->WindowBox(8,156,276,48, CWHITE);
	wnd->WindowRect(8,156,276,48, CDARK);
	wnd->set_font(curr);
	wnd->WindowText(20,160,"AaBbXxZz",CBLACK, CWHITE);
	wnd->set_font(old);
}

void FGFontDialog::SetFontProc(FGEvent *p)
{
	FGFontDialog *ptr = (FGFontDialog *)p->wnd->GetParameter();

	switch(p->GetType())
	{
		case KEYEVENT:
			if (ptr->lb->DoListBox(p->GetKey())==0) switch(p->GetKey())
			{
				case CR:
					if (ptr->userfnc)
						ptr->userfnc(ptr->curr);
					ptr->userfnc = 0;
				case ESC:
					if (ptr->wnd)
						delete ptr->wnd;
					break;
			}
			else
			{
				ptr->curr = ptr->lb->GetIndex();
				ptr->ShowFont();
			}
			break;
		case ACCELEVENT:
			if (p->accel->GetLocalId()==0)
			{
				delete p->wnd;
				if (ptr->userfnc)
					ptr->userfnc(ptr->curr);
				ptr->userfnc = 0;
				break;
			}
			else if (p->accel->GetLocalId()==1)
			{
				delete ptr->wnd;
				break;
			}
			break;
		case TERMINATEEVENT:
#ifdef INDEX_COLORS
//			_set_fgl_palette();
#endif
			break;
		case CLICKLEFTEVENT:
		{
			int rc = ptr->lb->Test(p->GetX(), p->GetY());
			if (rc>=0)
			{
				ptr->lb->SetIndex(rc);
				ptr->curr = rc;
				ptr->ShowFont();
			}
		}
		break;
	}
}

FGWidgetContainer::FGWidgetContainer()
{
	ptr = (void *)new FGCtrlContainer;
}

FGWidgetContainer::~FGWidgetContainer()
{
	delete (FGCtrlContainer *)ptr;
}

void FGWidgetContainer::DisableContainer(void)
{
	FGCtrlContainer *p = (FGCtrlContainer *)ptr;
	for(CPTR pp = p->begin(); pp != p->end(); ++pp)
		(*pp)->Disable();
}

void FGWidgetContainer::EnableContainer(void)
{
	FGCtrlContainer *p = (FGCtrlContainer *)ptr;
	for(CPTR pp = p->begin(); pp != p->end(); ++pp)
		(*pp)->Enable();
}

void FGWidgetContainer::AddToContainer(FGControl *w)
{
	FGCtrlContainer *p = (FGCtrlContainer *)ptr;
	p->push_back(w);
}

void FGWidgetContainer::SetUserData(void* val)
{
	FGCtrlContainer *p = (FGCtrlContainer *)ptr;
	for(CPTR pp = p->begin(); pp != p->end(); ++pp)
		(*pp)->SetUserData(val);
}

FGRadioGroup::FGRadioGroup(int x, int y, char *names[], int* init_value, void (*cb)(FGControl* ,int), FGWindow* w, int step)
{
	char** n = names;

	count=0;
	userfnc = cb;
	current = -1;
	value = init_value;;
	if (value)
		current = *value;

	while(*n) { n++; count++; }
	grp = new FGButtonGroup(count);
}

void FGRadioGroup::ChangeItem(int* new_value)
{
	FGCtrlContainer *p = (FGCtrlContainer *)ptr;
	// switch-off old button
	if (current>=0)
	{
		FGRadioButton* pointbutton  = (FGRadioButton*) (*p)[current];
		pointbutton->SetTrigger(false);
	}
	if (new_value)
		if (*new_value < count)
	{
		value = new_value;
		FGRadioButton* pointbutton  = (FGRadioButton*) (*p)[*value];
		pointbutton->SetTrigger(true);
        grp->RefreshGroup(pointbutton);
		current = *new_value;
	}
}

FGColorPicker::FGColorPicker(FGColor* _base_color)
{
	base_color = _base_color;

	wnd = new FGWindow(&wnd, 0,0, 640, 480, "Color Picker", PickerProc, CWHITE, CDARK, WSTANDARD|WMODAL|WCENTRED|WUSELAST);
	wnd->SetUserData(this);

	SetState(*base_color);

	DrawColorCircle();

	wnd->WindowText(8, 310, "RED  :");
	ctrl_r = wnd->AddSlideBarH(64, 310, 0, 255, 1, &m_r, DrawColorRGB);
	wnd->WindowText(8, 330, "GREEN:");
	ctrl_g = wnd->AddSlideBarH(64, 330, 0, 255, 1, &m_g, DrawColorRGB);
	wnd->WindowText(8, 350, "BLUE :");
	ctrl_b = wnd->AddSlideBarH(64, 350, 0, 255, 1, &m_b, DrawColorRGB);

	wnd->WindowText(8, 380, "SAT  :");
	ctrl_sat = wnd->AddSlideBarH(64, 380, 0, 100, 1, &m_saturation, DrawColorHSV);
	wnd->WindowText(8, 400, "HUE  :");
	ctrl_hue = wnd->AddSlideBarH(64, 400, 0, 360, 1, &m_hue, DrawColorHSV);
	wnd->WindowText(8, 420, "LUM  :");
	ctrl_lum = wnd->AddSlideBarH(64, 420, 0, 100, 1, &m_luminance, DrawColorHSV);

	FGControl*c = wnd->AddPushButton(530,370,80, 23, "Ok", 'o', Ok);
	c->SetParameter((void *)mrOk);
	c = wnd->AddPushButton(530,410,80, 23, "Cancel", 'c', FGControl::Close);
	c->SetParameter((void *)mrCancel);
	wnd->ShowModal();
}

void FGColorPicker::PickerProc(FGEvent* e)
{
	FGColorPicker *_THIS = (FGColorPicker *)e->wnd->GetUserData();
	int x = e->GetX();
	int y = e->GetY();
	unsigned raw_color = 0;
	FGPixel orig;

	switch(e->GetType())
	{
		case CLICKLEFTEVENT:
			orig = e->wnd->getpixel(x,y);
#ifndef INDEX_COLORS
			raw_color = ExpandColor(orig);
#endif
			FGColor color( raw_color );
			_THIS->SetState(color);
			_THIS->ctrl_r->draw();
            _THIS->ctrl_g->draw();
            _THIS->ctrl_b->draw();
            _THIS->ctrl_hue->draw();
            _THIS->ctrl_lum->draw();
            _THIS->ctrl_sat->draw();
            break;
    }
}

void FGColorPicker::Box(int x, int y, FGColor paper)
{
    char s[64];
    FGColor ink = paper;
    ink.Opposite();

    wnd->WindowBox(x, y, 80, 48, paper);
    sprintf(s,"#%02X%02X%02X", paper.GetRed(), paper.GetGreen(), paper.GetBlue() );
    wnd->WindowText(x+8, y+16, s, ink, paper);
}

void FGColorPicker::DrawColor(void)
{
    FG4Colors c4(the_color);
    Box(300, 32, c4.GetColor0());
    Box(380, 32, c4.GetColor1());
	Box(460, 32, c4.GetColor2());
    Box(540, 32, c4.GetColor3());

    FG3Colors c3(the_color);
    Box(340, 120, c3.GetColor0() );
    Box(420, 120, c3.GetColor1() );
    Box(500, 120, c3.GetColor2() );

    FGRelatedColors c5(the_color);
    Box(300, 220, c5.GetColor0());
    Box(380, 220, c5.GetColor1());
    Box(460, 220, c5.GetColor2());
    Box(540, 220, c5.GetColor3());
}

void FGColorPicker::DrawColorCircle(void)
{
  static const double pi = M_PI;
  FGRect m_circle(0,0,300,300);

  int    border  = m_circle.w;
  double border2 = border / 2;
  int    cx = border / 2 + 1; // Mittelpunkt
  int    cy = cx;

  FGColor c;

  for (double i = 0; i<360.; i+=.15)
  {
    for (double j = 0; j<border2; j++)
    {
        c.SetHLS(i, 0.5, j/border2);

        wnd->putpixel(
            int(cx - cos(pi * i / 180.0)*j),
            int(cy - sin(pi * i / 180.0)*j),
             c);
    }
  }
  wnd->WindowRepaintUser(0,0,300,300);
}

#ifdef FG_NAMESPACE
}
#endif


//
//
//

#ifndef _RAD_TYPE_H_
#define _RAD_TYPE_H_

#ifdef FG_NAMESPACE
using namespace fgl;
#endif


#pragma pack(1)
enum
{
	RAD_UNDEFINE,
	RAD_STRING,
	RAD_BOX,
	RAD_RECT,
	RAD_LINE,
	RAD_CIRCLE,
	RAD_PUSHBUTTON1,
	RAD_PUSHBUTTON2x,
	RAD_CHECKBUTTON,
	RAD_POINTBUTTON,
	RAD_EDITBOX1,
	RAD_EDITBOX2,
	RAD_EDITBOX3,
	RAD_SLIDEBARH,
	RAD_SLIDEBARV,
	RAD_LISTBOX,
	RAD_MENU,
	RAD_FILLCIRCLE,
	RAD_BITMAPx,
	RAD_PROGRESSBAR,
	RAD_LABEL,
	RAD_ELLIPSE,
	RAD_FILLELLIPSE,
	RAD_ARC,
	RAD_PANEL,
	RAD_TABPAGE,
// must be last
	RAD_LAST
};

typedef char RadType;

typedef struct
{
	int		key;
	char	*str;
	char	*name;
} AccKey;

typedef struct
{
	char	*name;
	int		value;
} WFLAG;

typedef struct
{
	char	name[MAXVAR+1];
	char	value[64];
	int		type;
} Values;

struct Accel
{
	RadType type;
	char flags;				// bit 0 is set for variable size of editbox
							// bit 1 is set for predefined keys
							// bit 2 is set for transparent
							// bit 3 is set for add to .RC
							// bit 4 is set for	range checking
							// bit 5 is set when has bitmap
							// bit 6 is set when is member of button group
	int x,y,w,h,ww,hh,
	key,
	ink,paper;
	union {
		int		min;
		double  mind;
	};
	union {
		int		max;
		double  maxd;
	};
	int		bgrp;
	int		stuff[2];
	union {
		char	bmp[SYMSIZE+1];
		char 	name[MAXNAME+1];
	};
	char 	fnc[MAXVAR+1], variable[MAXVAR+1];
};

struct Flags
{
	int title,frame,modal,solid,nomove,menu,
	nopicto,clickable,focus,sizeable,uselast,
	statusbar,center,escape, fastmove, notify,
	minimize, notebook;
};

struct Wind
{
	int 	typ,x,y,w,h,flags,ink,paper;
	int		current;				// -1 nor none
	char 	name[MAXNAME+1];
	Accel	table[MAX_ACCEL];
	int		items;					// num of complete items in window
	int		stuff;
	Window	*form;					// ptr to each window
	int		isSelected;
};

class Projekt
{
	public:
		int		magic;
		char	prjname[64];
		int		version;
		int		curr;				// index to current window
		int		nwin;		// num of windows
		int		font;					// 0 default, else 1 .. 5
		int		video_mode, back_color;
		int		app_altx, app_cfg, app_magnify, app_wnd, app_root;
		int		bmp_num;
		int		val_num;
        int		color_depth;
		int		stuff[9];
		FGPalette	paleta[256]; // vzdy 0 bajt je pouzita farba, a 1, 2 & 3 su RGB
		FGColorScheme cscheme;
	// ---------------------------------
		Wind	Okno[MAX_WND];		// windows
		char	bmp_names[MAX_BMP][SYMSIZE+1];
		Values	values[MAX_VAL];
#define PRJHDR_SIZE ((unsigned)(this->Okno) - (unsigned)this)
	// ---------------------------------
		int		Items(void) { return Okno[curr].items; }
		Wind	*wind(void) { return &Okno[curr]; }
		Accel	*accel(void) { return &Okno[curr].table[Okno[curr].items]; }
		Accel	*table(void) { return Okno[curr].table; }
		Window	*form(void) { return Okno[curr].form; }
		static  void proc(GuiEvent *);
		void	Redraw(Wind *);
		void	DrawAll(int f=0);
		Projekt();
		Projekt(char *, char *);
		void Save(void);
		~Projekt();
		void Init(void);
		void DrawWidget(Wind *, int);
		void PridajWidget(int change_only);
		void DeleteItem(void);
		void Update(void);  // update controls
		void Forward(void);
		void Backward(void);
		void Clone(void);
		void AddWindow(int);
		void DeleteWindow(void);
		void Up(void);
		void Down(void);
		int  ClickTest(int x, int y, Wind *w);
};

class XColors {
	public:
		FGPalette p;
		char   *s;
		XColors(FGPalette pp, char *ss)
		{
			p = pp;
			s = ss;
		}
};


#pragma pack()
#endif

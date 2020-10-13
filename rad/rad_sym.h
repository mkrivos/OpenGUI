//
//
//

#include "rad_def.h"
#include "rad_type.h"
#include <fastgl/listbox.h>

#ifdef FG_NAMESPACE
using namespace fgl;
#endif


extern Window *mWnd, *ew;
extern Projekt *prj;
extern Wind	Okno;
extern Flags flg;
extern EditBox *eb1, *eb2, *eb3, *eb4, *eb5;
extern EditBox *eb_x, *eb_y, *eb_w, *eb_h, *eb_max, *eb_min, *eb_name,
		*eb_fnc, *eb_size, *eb_rows, *eb_lins, *eb_var;
extern FGListBox *lBox, *lBox2;
extern Wind windx;
extern int change_only;
extern int build_mode;
extern char gps[256], srcname[64];
extern int isCode, test;
extern Window *dlgWnd;
extern AccKey acckey[];
extern int granularity_x;
extern int granularity_y;
extern char symtab[MAX_SYM][SYMSIZE+1], gps2[256], symtype[MAX_SYM];
extern int symindex;
extern FGColorScheme *old_scheme, rad_sc;
extern int rad_ink, rad_paper;

int Compile(Projekt *p);
int GenTable(Projekt *p);
void CleanupTable(Projekt *p, int& sym);
void ImportTable(Projekt *p, int& sym);
void BuildVariables(void);
//void OknoEditor(char *nazov, Widget OnCloseAction);
void ShowDialog(RadType type, Accel *ptr, int a=1); // default is change only
void SelectCurrent(Wind *w);
void disable_all(void);



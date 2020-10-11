#include "fgbase.h"
#include "fgscheme.h"

#ifdef FG_NAMESPACE
namespace fgl {
#endif

FGColorScheme::FGColorScheme()
{
	window_back     = CWHITED;
	window_fore     = CBLACK;		// window colors
	active_title    = CBLUELIGHT;   // active title
	inactive_title  = CBROWN;		// inactive title;
	wnd_bord1       = CWHITE;		// wnd border 1
	wnd_bord2       = CGRAYED;	    // wnd border 2
	wnd_bord3       = CGRAY2;		// wnd border 3
	statusbar       = CGRAY2;		// statusbar
	menu_back       = CGRAY2;		// menu bg
	menu_fore       = CBLUE;		// menu fore
	menu_back_active = CGRAY2;		// menu bg active
	menu_fore_active = CWHITE;		// menu fore active

	button_fore     = CBLACK;  	    // pbutton fore
	button_back     = CGRAY1;		// pbutton back
	button_fore_pushed = CDARK;		// pbutton fore - pushed
	button_back_pushed = CGRAY3;	// pbutton back - pushed
	button_bord1    = CWHITE;		// control::frame1
	button_bord2    = CBLACK;		// control::frame2
	button_bord3    = CGRAYED;	    // control::frame3

	edit_background_active = CWHITE;// edit back
	edit_foreground = CBLACK;		// edit fore
	edit_border     = CDARK; 		// edit bord1
	edit_background = CGRAY3;		// edit bord2

	slider          = CGRAY1;		// slider

	menuwindow_back = CGRAY3;		// menuwindow back
	menuwindow_fore = CBLACK;		// menuwindow fore
	menuwindow_frame= CBLACK;		/** a default pulldown menu frame color */

	pdmenu_back_active = CGRAYED;   // pdmenu back
	pdmenu_fore_active = CWHITE;	// pdmenu fore
	pdmenu_gray     = CGRAY1;		// pdmenu gray
	pdmenu_color_of_frame = CYELLOW;// pdmenu selected frame
	edit_fore_disable = CGRAY1;		// editbox disable
	notebook_active = CBLUELIGHT;   // notebook active
	slider_disable  = CGRAY3;		// slider disable
};

static FGColorScheme _FGCS;
FGColorScheme * CScheme = &_FGCS;

#ifdef FG_NAMESPACE
}
#endif

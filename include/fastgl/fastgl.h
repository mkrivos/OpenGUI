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

	fastgl.h   - declarations of many objects for FastGL

  $Id: fastgl.h 8523 2007-08-22 09:22:19Z majo $
*/

#ifndef	__WINDOW_H
#define	__WINDOW_H

#include "fgbase.h"
#include <xmlconfig.h>

#include <list>		// for window and control lists.

#ifdef FG_NAMESPACE
namespace fgl {
#endif

/* HJH modification: protect compiler options for structure alignment and enum
 * size if the compiler is Borland C++ */
#ifdef __BORLANDC__
#pragma option push -b
#pragma option push -a4
#endif

enum
{
	BNORMAL	= 7,
	BMASK = BNORMAL,
	BPUSHED	= 3,
	BDISABLED =	5
};

#define	FGFONT_TITLE	FONTSYS
#define	FGFONT_MENU		FONTSYSLIGHT
#define	FGFONT_BUTTON	FONTSYSLIGHT

/**
	@addtogroup Defines
	@{
*/
//! predefined FGWindow foreground color
#define	IM		 	CScheme->window_fore // CBLACK standard menu fore & background colors
//! predefined FGWindow background color
#define	PM			CScheme->window_back

//! Enable to remember FGWindow's positions and sizes.
const unsigned 	APP_WINDOWDATABASE =		1;
//! Enable Configuration file in form EXEFILENAME.rc
const unsigned 	APP_CFG	=				2;
//! Enable The ALT+X hotkey to terminates your application.
const unsigned 	APP_ENABLEALTX =			4;
//! Enable the rectangle selections by mouse.
const unsigned 	APP_MAGNIFIER =			8;
//! Enable and initialize the root FGWindow, i.e. background.
const unsigned 	APP_ROOTWINDOW =			16;
//! Enable visual efects.
const unsigned 	APP_ENABLEEFECTS =		0x20;
//! Enable all the above.
const unsigned APP_ALL =					0x1F;
//! Use local application directory for Config files instead of HOMEDIR.
const unsigned 	APP_LOCALDIR =			0x80000000;

/**	@} */

/**
	The current Operating System code.
	@ingroup Enums
*/
enum ENUM_OS { OSTYPE_LINUX=1, OSTYPE_MSDOS, OSTYPE_WIN32, OSTYPE_QNX };

/**
	Return codes for FGApp::ShowModal()
	@ingroup Enums
*/
enum MODAL_RETURN
{	mrNone   = 	-1,
	mrOk	 = 	-2,
	mrCancel =	-3,
	mrYes	 =	-2,
	mrNo	 =	-3,
	mrAll	 =	-4,
	mrIgnore =	-5,
	mrClose	 =	-6,
	mrQuit	 =	-7,
	mrRetry  =	-8,
	mrUnknown=	-9
};

class FGBaseGui;
class FGFileDialog;
class FGColorDialog;
class FGControl;
class FGWindow;
class FGMenuWindow;
class FGBitmap;
class FGEditBox;
class FGButtonGroup;
class FGListBox;
class FGPushButton  ;
class FGRadioButton;
class FGCheckBox;
class FGBaseMenuItem;
class FGPushButton;
class FGSlider;

#define FGCheckButton FGCheckBox
#define FGPointButton FGRadioButton
#define AddPointButton AddRadioButton
#define AddCheckButton AddCheckBox

#ifndef NO_ABSOLETE
#define Panel       FGPanel
#define Label       FGLabel
#define App         FGApp
#define Window      FGWindow
#define Control     FGControl
#define FontProperty FGFontProperty
#define DrawBuffer  FGDrawBuffer
#define MenuWindow  FGMenuWindow
#define TextEditor  FGTextEditor
#define PushButton  FGPushButton
#define CheckButton FGCheckButton
#define PointButton FGPointButton
#define BaseGui     FGBaseGui
#define GuiEvent    FGEvent
#define FontDialog  FGFontDialog
#define FileDialog  FGFileDialog
#define ColorDialog FGColorDialog
#define Bitmap      FGBitmap
#define EditBox     FGEditBox
#define ButtonGroup FGButtonGroup
#define ProgressBar FGProgressBar
#define Slider      FGSlider
#define SlideBarV   FGSlideBarV
#define SlideBarH   FGSlideBarH
#define RadioGroupV FGRadioGroupVertical
#define RadioGroupH FGRadioGroupHorizontal
#define TwoStateButton FGTwoStateButton
#define ColorScheme FGColorScheme
#endif

#ifdef FG_NAMESPACE
}
#endif

#include "fgscheme.h"
#include "fgtimer.h"
#include "fgevent.h"
#include "fgbitmap.h"
#include "fgcontrols.h"
#include "fgwindow.h"
#include "fgapp.h"

#ifdef FG_NAMESPACE
namespace fgl {
#endif

/**
	@addtogroup Types
	@{
*/
/**
The prototype for a FGFileDialog container callback procedure.
*/
typedef void (*FileDialogCallBack)(char * filename, FGFileDialog *);
/**
The prototype for a ColorDialog container callback procedure.
*/
typedef void (*ColorDialogCallBack)(FGPixel color);
/**
The prototype for a ColorDialog container callback procedure.
*/
typedef void (*FontDialogCallBack)(int f);

/** @} Types */

/**
* Object intended for easy error dialogs.
*/
class FGDialog
{
		FGWindow* wnd;
		FGControl* ctrl;
		unsigned width;

	public:
		FGDialog(char* format, ...);
		FGDialog(unsigned ink, unsigned paper, const char* title, char* format, ...);
		~FGDialog()
		{
			delete wnd;
		}
		FGWindow* GetWindow(void) { return wnd; }
		MODAL_RETURN ShowOk(void);
		MODAL_RETURN ShowYesNo(const char* str1="Yes", int key1='y', const char* str2="No", int key2='n' );
		MODAL_RETURN ShowRetryIgnoreCancel(void);
};

//---------------------------------------------------------------------------

unsigned int CalculateCRC(unsigned StartCRC, void *Addr, unsigned Size);

/**
	The pointer to the applicatons Config object
	@ingroup Globals
*/
extern ConfigInterface *cCfg;

//---------------------------------------------------------------------------

/**
	@addtogroup fgx
	@{
*/
typedef void ( * XUIEventHandler )(CallBack cb, void* user_data);
typedef void ( * XUIEnterHandlerText )(CallBack cb, const char* text, void* user_data);
typedef void ( * XUIEnterHandlerInteger )(CallBack cb, const int integer, void* user_data);
typedef void ( * XUIEnterHandlerDouble )(CallBack cb, const double integer, void* user_data);
typedef void ( * XUIWindowHandler )(FGEvent* event, void* user_data);
/**
	@}
*/

/**
	@ingroup fgx
*/
struct FGClosure
{
	static const int name_size = 48;
	enum { ON_CLICK, ON_INTEGER_CHANGE, ON_DOUBLE_CHANGE, ON_TEXT_CHANGE, HANDLER };
	char event_name[name_size];
	union {
		XUIEventHandler function;
		XUIWindowHandler handler;
		XUIEnterHandlerText text_handler;
		XUIEnterHandlerInteger int_handler;
		XUIEnterHandlerDouble dbl_handler;
		};
	void* user_data;
	int type;

	FGClosure(const char* n, XUIEventHandler func = 0, void* user_data = 0);
	FGClosure(const char* n, XUIEnterHandlerText func = 0, void* user_data = 0);
	FGClosure(const char* n, XUIEnterHandlerInteger func = 0, void* user_data = 0);
	FGClosure(const char* n, XUIEnterHandlerDouble func = 0, void* user_data = 0);
	FGClosure(const char* n, XUIWindowHandler hwnd = 0, void* user_data = 0);
};

typedef std::list<FGClosure> ClosureList;
typedef ClosureList::iterator ClosureIterator;

void RegisterOnClickSignal(const char* signal_name, XUIEventHandler fnc, void* user_data = 0);
void RegisterOnEnterSignal(const char* signal_name, XUIEnterHandlerText fnc, void* user_data = 0);
void RegisterOnEnterSignal(const char* signal_name, XUIEnterHandlerInteger fnc, void* user_data = 0);
void RegisterOnEnterSignal(const char* signal_name, XUIEnterHandlerDouble fnc, void* user_data = 0);
void RegisterControl(FGControl* ctrl, XUIEventHandler fnc, void* user_data);

bool ChangeSignalData(const char* signal_name, void* func, void* user_data);
bool DeregisterSignal(const char* signal_name, void* func);
bool DeregisterSignal(const char* signal_name, void* func, void* data);

void RegisterWindowHandler(const char* handler_name, XUIWindowHandler fnc, void* user_data = 0);

void CallClosure(const char* signal_name, CallBack cb);
void CallClosure(const char* signal_name, CallBack cb, int value);
void CallClosure(const char* signal_name, FGEvent* e);
void CallClosureDebug();

void GetTempName(char* buffer, int size);

//---------------------------------------------------------------------------

#ifdef FG_NAMESPACE
}
#endif

#ifdef __BORLANDC__
#pragma option pop /* pop -a switch */
#pragma option pop /* pop -b */
#endif

#endif

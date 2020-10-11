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
    declarations for widgets

  $Id: widgets.h 2086 2005-05-12 10:52:34Z majo $

  $Log$
  Revision 1.1  2005/05/12 10:52:38  majo
  Initial revision

  Revision 1.18  2005/01/31 08:44:59  majo
  doxy, aliases

  Revision 1.17  2005/01/31 07:46:30  majo
  fgx + pointbutton&checkbutton rename

  Revision 1.15  2005/01/03 07:47:19  majo
  xmas, gradient, lrmi

  Revision 1.14  2004/11/05 07:59:25  majo
  update FGEditBox

  Revision 1.13  2004/09/29 10:32:05  majo
  stabilized TabPages

  Revision 1.12.2.1  2004/09/07 11:41:09  majo
  verzia 5.0.0 alpha1
  new containers for Windows & Controls

  Revision 1.12  2004/05/30 11:52:55  majo
  dox

  Revision 1.11  2004/04/16 08:58:19  majo
  added FGRadioGroup::SetUserData(void *)

  Revision 1.10  2004/03/24 20:06:54  majo

  WIN32 compiling

  Revision 1.9  2004/03/24 18:44:43  majo
  update FGColorPicker dox

  Revision 1.8  2004/03/24 17:13:58  majo
  added FGXColor
  added FGColorPicker

  Revision 1.7  2004/03/17 21:19:12  majo
  added FGColor, FGImage

  Revision 1.6  2004/02/23 20:08:01  majo
  all classes are with prefix FG* on now
  polygon functions uses FGPointArray from now
  class GuiEvent is renamed to FGEvent
  some by parameters overloaded methods was removed (class FGWindow)
  many other small changes

  Revision 1.5  2004/02/12 17:52:11  majo
  *

  Revision 1.4  2004/02/10 16:18:59  majo
  fixed compile warnings

  Revision 1.3  2004/02/10 16:16:53  majo
  added method: bool FGFileDialog::TestForOvervrite(void)

  Revision 1.2  2004/01/28 18:08:58  majo
  header file cleanups - widgets.h no more include fastgl.h

  Revision 1.1.1.1  2004/01/22 16:25:08  majo
  initial release

*/

//
//	WIDGETS.CC		20.11.1998
//

#ifndef _WIDGETS_H_
#define _WIDGETS_H_

#include <sys/stat.h>
#include <string.h>

#ifdef __WATCOMC__
#ifdef __DOS__
#include <direct.h>
#else
#include <dirent.h>
#endif // dos
#elif !defined(_MSC_VER)
#include <dirent.h>
#endif // watcom

#ifdef __BORLANDC__
#include <dir.h>
#endif

#ifdef _MSC_VER
#include <direct.h>
#include "mydirent.h"
#endif

#ifdef __BORLANDC__
#pragma option push -b
#pragma option push -a4
#endif

#ifdef FG_NAMESPACE
namespace fgl {
#endif

/**
Use FGUpDown to add an up-down control to a FGWindow. Up-down controls consist
of a pair of arrow buttons, such as the arrows that appear in a spin box.
Up-down controls allow users to change the size of a numerical value by clicking
on arrow buttons.
*/
class FGUpDown
{
		FGPushButton *up, *down;
		FGEditBox *eb;
		float step;
		static void handler1(CallBack);
		static void handler2(CallBack);

	public:
		FGUpDown(FGEditBox *eb, float _step);
		//! Disable object
		void Disable(void)
		{
			up->Disable();
			down->Disable();
			eb->Disable();
		}
		//! Enable object
		void Enable(void)
		{
			up->Enable();
			down->Enable();
			eb->Enable();
		}
};

/**
	Provides a container for multiple Enable/Disable of the Control
	in a FGWindow.
	Use this if you want work with some Controls as with a group.
*/
class FGWidgetContainer
{
	protected:
		void *ptr;

	public:
		FGWidgetContainer();
		~FGWidgetContainer();
		//! Disables all FGControls
		void DisableContainer(void);
		//! Enables all FGControls
		void EnableContainer(void);
		//! Adds new FGControl
		void AddToContainer(FGControl *);
		//! Set user data pointer
		void SetUserData(void* val);
};

/**
    Base class for RadioGroup classes
*/
class FGRadioGroup : public FGWidgetContainer
{
	protected:
		FGButtonGroup*      grp;
		unsigned 		    id;
		int*			    value;
		int					current;
		int                 count;
		void 			    (*userfnc)(FGControl* ,int);

		static void callback(CallBack cb)
		{
			FGRadioGroup* _THIS = (FGRadioGroup *)cb->GetPrivateData();
			_THIS->current = cb->GetId() - _THIS->id;
			if (_THIS->value) *_THIS->value = _THIS->current;
			if (_THIS->userfnc)
				_THIS->userfnc(cb, _THIS->current);
		}
	public:
		FGRadioGroup(int x, int y, char *names[], int *init_value, void (*cb)(FGControl* ,int), FGWindow* w, int step=20);
		/**
		* Call destructor on TERMINATEEVENT explicitly.
		*/
		virtual ~FGRadioGroup()
		{
			if (grp) delete grp;
			grp = 0;
		}
		void ChangeItem(int* new_value);
};

/**
* Widget to easier using of vertical arrays of RadioButtons.
* @image html radio.png
*/
class FGRadioGroupVertical : public FGRadioGroup
{
		FGControl* DrawOne(int x, int y, int offset, char* name, int key, FGWindow* w, ControlCall cb)
		{
			return w->AddRadioButton(x, y+offset, name, key, 0, cb);
		}
	public:
		/**
		* Creates the group of RadioButtons at exact coords into the FGWindow. The number of created
		* objects is equivalent of the number of passed strings.
		* @param x start x coord
		* @param y start y coord
		* @param names an array of ASCIIZ strings terminated by 0 pointer, by example: char *names[]={"one","two","three",0};
		* @param keys an arrray (int []) of keys
		* @param init_value an initial index of active object on create time, by example: 1 for string "two"
		* @param cb standard OpenGUI CallBack routine
		* @param w the parent FGWindow object
		* @param step offset of y coordinates for each next line (default=20)
		*/
		FGRadioGroupVertical(int x, int y, char *names[], int keys[], int *init_value, void (*cb)(FGControl* ,int), FGWindow* w, int step=20)
		 : FGRadioGroup(x,y,names,init_value,cb,w,step)
		 {
			 FGControl *c;
			 for(int i=0; i<count; i++)
			 {
				 c = DrawOne(x, y, i*step, names[i], keys[i], w, callback);
				 if (i==0) id = c->GetId();
				 c->SetPrivateData((void *)this);
				 grp->AddToGroup(c, init_value ? i == *init_value : 0);
				 AddToContainer(c);
			 }
		 }
};

/**
* Widget to easier using of horizontal arrays of RadioButtons.
* @image html radio.png
*/
class FGRadioGroupHorizontal : public FGRadioGroup
{
		FGControl* DrawOne(int x, int y, int offset, char* name, int key, FGWindow* w, ControlCall cb)
		{
			return w->AddRadioButton(x+offset, y, name, key, 0, cb);
		}
	public:
		/**
		* Creates the group of RadioButtons at exact coords into the FGWindow. The number of created
		* objects is equivalent of the number of passed strings.
		* @param x start x coord
		* @param y start y coord
		* @param names an array of ASCIIZ strings terminated by 0 pointer, by example: char *names[]={"one","two","three",0};
		* @param keys an arrray (int []) of keys
		* @param init_value an initial index of active object on create time, by example: 1 for string "two"
		* @param cb standard OpenGUI CallBack routine
		* @param w the parent FGWindow object
		* @param step offset of x coordinates for each next button
		*/
		FGRadioGroupHorizontal(int x, int y, char *names[], int keys[], int *init_value, void (*cb)(FGControl* ,int), FGWindow* w, int step)
		: FGRadioGroup(x,y,names,init_value,cb,w,step)
		{
			FGControl *c;
			for(int i=0; i<count; i++)
			{
				c = DrawOne(x, y, i*step, names[i], keys[i], w, callback);
				if (i==0) id = c->GetId();
				c->SetPrivateData((void *)this);
				grp->AddToGroup(c, init_value ? i == *init_value : 0);
				AddToContainer(c);
			}
		}
};

#include "fgfontdlg.h"
#include "fgcolordlg.h"
#include "fgfiledlg.h"
#include "fgeditor.h"

#ifdef __BORLANDC__
#pragma option pop /* pop -a switch */
#pragma option pop /* pop -b */
#endif

#ifdef FG_NAMESPACE
}
#endif

#endif

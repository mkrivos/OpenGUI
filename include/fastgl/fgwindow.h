#ifdef __BORLANDC__
#pragma option push -b
#pragma option push -a4
#endif

#ifdef FG_NAMESPACE
namespace fgl {
#endif

/**
The prototype for a LISTBOX container callback procedure.
*/
typedef void (*FGListBoxCallBack)(int flag, FGWindow *wnd, int x, int y, void *data, int index);

/**
	It is a rectangle space at the screen that is intended to the communication
	with the user. Its look & feel is derived from the flags assigned
	on the create time. The maximal size of the window is limited
	by current video mode resolutions only. It's have own fore
	and back color, title (or no). You can use many windows
	at once (it is limited with free memory only), but only and just
	only one may be in the active state. You can draw your windows everywhere,
	but if you go out of bounds of the screen, then graphics system
	adjust the size and position at the right values. You can set
	that windows to remember its position and size. The each window is active
	immediate after it is created. To the active window goes user's input,
	key pressing, mouse events and other system events. You can serve
	these events by defining your own FGWindow Handler. It is a single
	procedure with type GuiHwnd and it is described below. Because using
	FGWindow methods to a not existing object, library has implemented
	safe mechanism (it is not mandatory but it is a good practise).
	@callgraph
	@image html wnd.png
*/
class FGWindow : public FGBaseGui
{
		friend	class FGControl;
		friend	class FGPanel;
		friend	class FGEditBox;
		friend  class FGBaseMenu;
		friend  class FGApp;

		FGControlBox ctrl_list; 	  		// list of FGControlContainer(s)
		FGControlBoxIterator Buttony;		// for current buttons, labels ... etc.
		const char* current_tab_page;
		int			c_menupos;

		int 		ctrl_counter;           // to generate proper local_id for controls
		unsigned 	first_draw:1,
					changed:1; 		  		//

		int			xpos;					// lineto x
		int			ypos;                   // lineto y

		void 		(*handler)(FGEvent	*);
		FGControl 	*call_handler; // set if need call handler (in translateevent)
		void		(*CallWindowHook)(int x, int y, int& nx, int& ny, int);

		// for locking dynamic (WindowMove(), WindowLine() ... )
		FGMutex 	StateMutex;
		int			state_locked;
		static FGMutex WindowListMutex;

		FGPixel		statuscolor;
		FGEditBox 	*ininput;
		int			icon_x, icon_y;
		FGWindow	**itself;
		FGWindow	*iconized;

		FGWindow(FGWindow **, FGPixel);		// root window
		virtual int iconize(int xs, int ys);
		int		FGAPI Rozporcuj(int X1, int Y1, int X2, int Y2, int *in, int *out);
		void	FGAPI Relocate(int xx, int yy);
		static	void FindNextFindow(void);
		FGEditBox* FGAPI FindNextControl(void);
		int	 	FGAPI TitleFind(FGEvent	*e);
		void 	FGAPI WindowUpdate(int dx, int dy, int all=0);
		void	FGAPI ResetActive(void)
		{
			call_handler = 0;
		}
		FGControl * FGAPI IsActive(void)
		{
			return call_handler;
		}
		int		FGAPI IsInInput(void) const { return !!ininput; }
		void 	FGAPI SetInInput(FGEditBox *e)
		{
			ininput = e;
		}
		void	FGAPI WindowInputChar(unsigned c)
		{
			ininput->inputproc(c);
		}
		void	FGAPI WindowUpdateBlock(int x, int y, int w, int h);

		void	FGAPI SetNextTabPage(void);
		void	FGAPI DrawTabPages(void);
		bool	FGAPI ClickTabPage(int, int);
		void	FGAPI RelocateTabPage(int xx, int yy);

		void 	FGAPI puts(const char *s) { while(*s) wputc(*s++);	}
		void	FGAPI wputc(int);
		void	FGAPI move(int x=0,	int	y=0) { wputc(-1); xpos=x; ypos=y; }
		/** internal */
		int		FGAPI GetXM(int);
		/** internal */
		int		FGAPI GetYM(void);
		//! Returns the pointer to the first FGControl in the FGWindow.
		FGControlContainer* GetBaseControls(void) { return & (*ctrl_list.begin()); }
	protected:
		FGDrawBuffer 	*icon;

		virtual void	FGAPI RepaintBlock(int x,	int	y, int w, int h);
		void	FGAPI init(void);
		void	FGAPI default_clipping(void);
		long	FGAPI WindowStatus(long status);
		virtual void WindowPiktograms(int x, int y);
		virtual void WindowDrawTitle(int color); // old virtual
	public:
		// redraws object completely
		virtual	void draw(void);
		void	FGAPI SetDefaultControl(FGControl* ctrl)
		{
			Buttony->SetDefaultControl(ctrl);
		}
		void	SetNextControl(void)
		{
			Buttony->SetNextControl();
		}
		void	SetPreviousControl(void)
		{
			Buttony->SetPreviousControl();
		}
		FGControl* GetDefaultControl(void)
		{
			return Buttony->GetDefaultControl();
		}
		/** internal */
		void	InstallWindowHook(void(*hook)(int, int, int&, int&, int))
		{
			CallWindowHook = hook;
		}
		/** internal */
		void	SetActive(FGControl *c);
		/** internal */
		void	RunHandler(void);
		//! @deprecated
		static	void ShowAll(void);
		static 	void FGAPI WindowVirtualRoot(int, int, int, int);

		//! Lock the FGWindow to access from other threads.
		void	StateLock(void)
		{
			StateMutex.Lock();
			state_locked++;
		}
		//! Unlock the FGWindow to access from other threads.
		void	StateUnlock(void)
		{
			StateMutex.Unlock();
			state_locked--;
		}
		// the window notify interface
		//! Marks the FGWindow state as 'CHANGED'. Works together with notifycation on window close.
		void	FGAPI SetChange(void) { changed = 1; }
		//! Marks the FGWindow state as 'NOT CHANGED'. Works together with notifycation on window close.
		void	FGAPI ResetChange(void) { changed = 0; }
		bool	FGAPI ShowNotify(void);
		void	AbsToWindowPosition(int& _x, int& _y) { _x -= x; _y -= y; }
		//! Sets the icon position.
		void	FGAPI SetIconPosition(int x, int y) { icon_x = x; icon_y = y; }
		//! Returns true/false if the FGWindow is iconized.
		bool	FGAPI IsIconized(void) const { return !!iconized; }
		//! Iconize the FGWindow at its icon position or if not available, at the mouse position.
		int 	FGAPI WindowIconize(void);
		//! Iconize the FGWindow at the exact position.
		int 	FGAPI WindowIconize(int, int);
		//! Attaches the image as icon to the FGWindow.
		void 	FGAPI WindowAttachIcon(FGDrawBuffer *ico) { icon = ico; }

		//! Returns the pointer to the first FGControl in the FGWindow.
		FGControlBoxIterator GetCurrentControls(void) { return Buttony; }

		void	FGAPI WindowRepaint(int	xr,	int	yr,	int	w, int h);
		void	FGAPI WindowRepaintUser(int	xr,	int	yr,	int	w, int h);

		FGWindow(FGWindow **, int xs, int ys, int ws, int hs, const char *nm, GuiHwnd=0, int i=IM, int p=PM, int flag=WTITLED|WFRAMED, void* user_data = UNDEFINED_USER_DATA);
		virtual ~FGWindow();
		bool	WindowClose(void);
		/**
			Flushes the input line if one is currently edited.
			@return true if any input flushed or false if there is nothing to do.
		*/
		bool	FGAPI WindowFlushInput(void)
		{
			if (ininput)
			{
				WindowInputChar(CR);
				ininput = 0;
				return true;
			}
			return false;
		}

		FGPushButton*       AddPushButton(int xs, int ys, int ws, int hs, const char *nm, int key=0, ControlCall f=0, void* user_data = UNDEFINED_USER_DATA);
		FGPushButton*       AddPushButton(int xs, int ys, int ws, int hs, const char *nm, int key, FGDrawBuffer *bm, ControlCall f, void* user_data = UNDEFINED_USER_DATA);
		FGPushButton*       AddPushButton(int xs, int ys, int key, FGDrawBuffer *bm, ControlCall f, void* user_data = UNDEFINED_USER_DATA);
		FGSlideBarH*        AddSlideBarH(int xs, int ys, int min, int max, int step, int *val, ControlCall f=0, void* user_data = UNDEFINED_USER_DATA);
		FGSlideBarV*        AddSlideBarV(int xs, int ys, int min, int max, int step, int *val, ControlCall f=0, void* user_data = UNDEFINED_USER_DATA);
		FGCheckBox*      AddCheckBox(int	xs,	int	ys,	const char *nm, int key=0, int * variable=0, ControlCall=0, void* user_data = UNDEFINED_USER_DATA);
		FGCheckBox*      AddCheckBox(int	xs,	int	ys,	const char *nm, int key, bool * variable, ControlCall=0, void* user_data = UNDEFINED_USER_DATA);
		FGRadioButton*      AddRadioButton(int	xs,	int	ys,	const char *nm, int key=0, int * variable=0, ControlCall=0, void* user_data = UNDEFINED_USER_DATA);
		FGCheckBox*      AddCheckBoxMask(int mask, int xs, int ys,	const char *nm, int key=0, int * variable=0, ControlCall=0, void* user_data = UNDEFINED_USER_DATA);
		FGRadioButton*      AddRadioButtonMask(int mask, int xs, int ys,	const char *nm, int key=0, int * variable=0, ControlCall=0, void* user_data = UNDEFINED_USER_DATA);
		FGTwoStateButton*   AddTwoStateButton(int	xs,	int	ys,	int w, int h, const char *nm, int key=0, int * variable=0, ControlCall=0, void* user_data = UNDEFINED_USER_DATA);
		FGEditBox*          AddEditBox(int	xs,	int	ys,	int	ws1, int ws2, const char *nm,	int	key, int *p, ControlCall f, int min, int max, void* user_data = UNDEFINED_USER_DATA);
		FGEditBox*          AddEditBox(int	xs,	int	ys,	int	ws1, int ws2, const char *nm,	int	key, int *p, ControlCall f=0, void* user_data = UNDEFINED_USER_DATA);
		FGEditBox*          AddEditBox(int	xs,	int	ys,	int	ws1, int ws2, const char *nm,	int	key, char *p, ControlCall f=0, void* user_data = UNDEFINED_USER_DATA);
		FGEditBox*          AddEditBox(int sz, int	xs,	int	ys,	int	ws1, int ws2, const char *nm,	int	key, char *p, ControlCall f=0, void* user_data = UNDEFINED_USER_DATA);
		FGEditBox*          AddEditBox(int	xs,	int	ys,	int	ws1, int ws2, const char *nm,	int	key, double *p, ControlCall f, double min, double max, void* user_data = UNDEFINED_USER_DATA);
		FGEditBox*          AddEditBox(int	xs,	int	ys,	int	ws1, int ws2, const char *nm,	int	key, double *p, ControlCall f=0, void* user_data = UNDEFINED_USER_DATA);
		FGBaseMenu*         AddBaseMenu(const char *nm,	int	key=0, ControlCall f=0, void* user_data = UNDEFINED_USER_DATA);
		FGProgressBar*      AddProgressBar(int xs, int ys, int ws, int hs, int size);
		FGListBox* 	        AddListBox(int xs, int ys, int w, int h, int dropdown, FGListBoxCallBack drawone=0, void* userdata = 0);
		FGLabel*	        AddLabel(int x, int y, const char *s, int key=0, ControlCall f=0, unsigned  i = UNDEFINED_COLOR, unsigned p = UNDEFINED_COLOR, void* user_data = UNDEFINED_USER_DATA);
		FGPanel*	        AddPanel(int, int, int, int, const char *s=0, unsigned paper = UNDEFINED_COLOR, unsigned ink=CWHITED, unsigned ink2=CDARK);
		FGText*		        AddText(int x, int y, const char *s, unsigned ink = UNDEFINED_COLOR, unsigned paper = UNDEFINED_COLOR);
		FGImage*            AddImage(int x, int y, FGDrawBuffer* img);

		void 		FGAPI WindowText(int x, int y, const char *s, unsigned Ink = UNDEFINED_COLOR, unsigned Paper = UNDEFINED_COLOR);

		void 		FGAPI WindowText(int x, int y, const char *s, FGFontProperty * f, unsigned color, unsigned bk);
		void 		FGAPI WindowTextUTF8(int x, int y, const char *s, FGFontProperty * f, unsigned color, unsigned bk);
		void 		FGAPI WindowTextUnicode(int x, int y, const unsigned short *s, FGFontProperty * f, unsigned color, unsigned bk);

		void 		FGAPI WindowPanel(int, int, int, int, const char *s=0, int unsigned = CWHITED, unsigned ink2=CDARK);

		void 		FGAPI WindowBox(int x, int y, int a, int b, unsigned color = UNDEFINED_COLOR);

		void 		FGAPI WindowPixel(int x,	int	y, unsigned color = UNDEFINED_COLOR);

		void 		FGAPI WindowRect(int	x, int y, int a, int b, unsigned color = UNDEFINED_COLOR);

		void 		FGAPI WindowPatternRect(int	x, int y, int a, int b, FGPattern *);

		void 		FGAPI WindowLine(int	x, int y, int a, int b,	unsigned color = UNDEFINED_COLOR);
		void 		FGAPI WindowPatternLine(int	x, int y, int a, int b, FGPattern *p=&PatternDot);

		void 		FGAPI WindowDrawCircle(int x, int y, int	r, unsigned color = UNDEFINED_COLOR);
		void 		FGAPI WindowFillCircle(int x, int y, int	r, unsigned color = UNDEFINED_COLOR);

		void 		FGAPI WindowDrawEllipse(int x, int y, int	rx, int ry, unsigned color = UNDEFINED_COLOR);
		void 		FGAPI WindowFillEllipse(int x, int y, int	rx, int ry, unsigned color = UNDEFINED_COLOR);

		void 		FGAPI WindowDrawArc(int x, int y, double ang1, double ang2, int r, unsigned color = UNDEFINED_COLOR);

		FGRect 		FGAPI WindowFillPolygon(const FGPointArray& , unsigned col = UNDEFINED_COLOR);
		void 		FGAPI WindowDrawPolygon(const FGPointArray&, unsigned col = UNDEFINED_COLOR);

		FGRect 		FGAPI WindowFillTriangle(int x1, int y1, int x2, int y2, int x3, int y3, unsigned col = UNDEFINED_COLOR);

		void 		FGAPI WindowSpline(FGPoint points[4], unsigned color = UNDEFINED_COLOR);

		void 		FGAPI WindowScrollDown(int xx, int yy, int ww, int hh ,int about);
		void 		FGAPI WindowScrollUp(int xx, int yy, int ww, int hh ,int about);
		void		FGAPI WindowScrollLeft(int xx, int yy, int ww, int hh ,int about);
		void		FGAPI WindowScrollRight(int xx, int yy, int ww, int hh ,int about);

		void 		FGAPI WindowStatusBar(int, const char *s, int c=CBLACK);

		int	 		printf(const char * , ... );
		int	 		printf(int,int,const char *, ... );

		//! The helper for event sending to the FGWindow.
		void 		FGAPI SendEvent(int event, int key=0, int x=0, int y=0)
		{
			FGEvent e(event,key,x,y);
			SendToWindow(&e);
		}
		//! The helper for event sending to the FGWindow.
		void 		FGAPI SendEvent(int event, int x, int y, int ww, int hh)
		{
			FGEvent e(event,0,x,y,ww,hh);
			SendToWindow(&e);
		}
		//! Send the event to the FGWindow handler.
		void 		FGAPI SendToWindow(FGEvent *p);

		/**
			Lock the FGWindow for the immediate update to the screen. This lock disable
			the FGWindow redrawing on the screen until WindowUnlock() is called.
			Useful when you draw many small objects at once and you want optimize for speed.
			@note this is not related to threading
		*/
		void 		FGAPI WindowLock(void) {	status|=WLOCKED; }
		/**
			Unlock (and update on the screen if needed) the Locked ( WindowLock() ) FGWindow.
			@note this is not related to threading
		*/
		void 		FGAPI WindowUnLock(void);
		void 		FGAPI WindowResize(int, int);
		void 		FGAPI WindowMove(int,int);
		void 		FGAPI WindowFocus(void);
		void 		FGAPI WindowShape(int, int, int, int);
		void 		FGAPI WindowPutBitmap(int x,	int	y, int xs, int ys,	int	w, int h, FGDrawBuffer *p);
		#define		WindowCopyFrom WindowPutBitmap
		void 		FGAPI WindowGetBitmap(int x,	int	y,	int	xs,	int	ys,	int	w, int h, FGDrawBuffer *p);
		MODAL_RETURN FGAPI ShowModal(void);
		bool 		FGAPI WindowSetWorkRect(int,int,int,int);

		//! Redraw controls.
		void 		FGAPI RedrawControls(void);
		//! Removes all the controls from window (buttons, listoboxes, editboxes etc.).
		void 		FGAPI RemoveControls(void);
		//! Removes all the controls from window (buttons, listoboxes, editboxes etc.).
		void 		FGAPI RemoveControls(FGControlBoxIterator iter);
		//! Removes the control from window (button, etc.).
		bool 		FGAPI RemoveControl(FGControlBoxIterator iter, FGControl* ctrl);
		//! Removes the control from window (button, etc.).
		void 		FGAPI RemoveControl(FGControl* ctrl);
		//! Disables all the controls in the FGWindow at once.
		void		FGAPI DisableControls(void);
		//! Disables all the controls in the FGWindow at once.
		void		FGAPI DisableControls(FGControlBoxIterator iter);
		//! Enables all the controls in the FGWindow at once.
		void		FGAPI EnableControls(void);
		//! Enables all the controls in the FGWindow at once.
		void		FGAPI EnableControls(FGControlBoxIterator iter);
		//! Sets the FGWindow foreground color for layout and default foreground color for drawing.
		void		FGAPI SetInk(FGPixel i)
		{
			state._ink = ink = i;
		}
		//! Sets the FGWindow background color for layout and default background color for drawing.
		void		FGAPI SetPaper(FGPixel p)
		{
			state._paper = paper = p;
		}
		//! Sets the new caption for the FGWindow.
		void 		FGAPI SetName(const char *s);
		/**
		Adds a TabPage to the FGWindow. This Tab will contens all controls added to the FGWindow before this.
		From now, you can add controls to the next Tab.
		@param name caption / id string
		@return returns false if the name is already used (and all current controls are dropped!)
		@note that the call to this function creates a tab with controls that
		are ALREADY in the window but not with controls that you will add after this!
		@see SetTabPage() GetCurrentPage()
		*/
		bool		FGAPI AddTabPage(const char *name);
		/**
		Returns current tab page name or null.
		@see SetTabPage()
		*/
		const char* FGAPI GetCurrentTabPage(void)
		{
			return current_tab_page;
		}
		/**
		Sets the new current tab page.
		@param name caption / id string
		@see GetCurrentPage()
		*/
		void FGAPI SetTabPage(const char *name);
		//! Removes all tab pages from the window.
		void FGAPI DeleteTabPages(void);
		//! Removes tab page from the window.
		void FGAPI DeleteTabPage(const char *name);
		//! Returns the number of attached TabPages.
		int	 GetNumberOfTabPages(void);
		// the virtual callback interface

		//! override this member when you want your own processing of KEYPRESS
		virtual void OnKeyPress(int akey) { }
		//! override this member when you want your own processing of MOUSE MOVING
		virtual void OnMouseMove(int x, int y) { }
		//! override this member when you want your own processing of MOUSE LEFT CLICK
		virtual void OnClick(int x, int y) { }
		//! override this member when you want your own processing of DOUBLE MOUSE LEFT CLICK
		virtual void OnDoubleClick(int x, int y) { }
		//! overload this member when you want catch the click on MIDDLE mouse's button event.
		virtual void OnMiddleButton(int, int) { }
		//! override this member when you want your own processing of MOUSE RIGHT CLICK
		virtual void OnContextPopup(int x, int y) { }
		//! overload this member when you want catch the mouse's wheel spin.
		virtual void OnWheel(int x, int y, int delta) { }
		//! override this member when you want your own processing of REPAINT event.
		virtual void OnPaint(void) { }
		//! override this member when you want your own processing of DRAG&DROP
		virtual void OnStartDrag(int , int, int) {}
		//! override this member when you want your own processing of DRAG&DROP
		virtual void OnEndDrag(int ,int, int, int, int) {}
		//! override this member when you want your own processing of WINDOW MOVE
		virtual void OnMove(int dx, int dy) { }
		//! override this member when you want your own processing of WINDOW RESIZE
		virtual void OnResize(int dx, int dy) { }
		//! override this member when you want your own processing of WINDOW ICONIZE
		virtual void OnIconize(void) { }
		//! override this member when you want your own processing of ACCELERATORS
		virtual void OnAccelerator(FGControl *which) { }
		//! override this member when you want to handle the TabPage switch
        virtual void OnTabSwitch(const char *) { }
};

typedef std::list<FGWindow *> FGWindowContainer;
typedef FGWindowContainer::iterator FGWindowIterator;
typedef FGWindowContainer::reverse_iterator FGWindowRIterator;

/**
	Used for Pop-up menus.
	@see class FGWindow
*/
class FGMenuWindow : public FGWindow
{
		friend class FGWindow;
		friend class FGBaseMenuItem;

		int		offset;

		static GuiHwnd Proc;
		static FGMenuWindow *	CurrentMenu;
		static void	MenuWindowHandler(FGEvent *p);

		void _initmw(GuiHwnd proc)
		{
			offset = 0;
			Proc = proc;
			CurrentMenu	= this;
			set_font(FGFONT_MENU);
		}
		int	GetXM(void)	const
		{
			return 4;
		}
		int	GetYM(int a)
		{
			return (offset+=a,offset-a);
		}
	public:
		/**
		Creates pulldown menu.
		@param x x coordinate
		@param y y coordinate
		@param w a width of menu
		@param h a height of menu
		@param proc an optional window callback procedure
		@param user_data an optional user data pointer (for better binding between GUI and user's objects).
		*/
		FGMenuWindow(int x, int y, int w,	int	h, GuiHwnd proc=0, void* user_data = UNDEFINED_USER_DATA ):
			FGWindow(0,x,y,w,h,"", MenuWindowHandler, CScheme->menuwindow_fore, CScheme->menuwindow_back,WFRAMED|WNOPICTO|WMENUWINDOWTYPE|WNODRAWFROMCONSTRUCTOR|WUSESELECTEDCONTROL, user_data)
		{
			draw();
			_initmw(proc);
		}
		/**
		Creates pulldown menu at current mouse position.
		@param w a width of menu
		@param h a height of menu
		@param proc an optional window callback procedure
		@param user_data an optional user data pointer (for better binding between GUI and user's objects).
		*/
		FGMenuWindow(int w, int h, GuiHwnd proc=0, void* user_data = UNDEFINED_USER_DATA );

		virtual ~FGMenuWindow()
		{
			CurrentMenu	= 0;
		}

		virtual void draw(void);

		FGBaseMenuItem* AddMenu(const char *nm,	int	key=0, ControlCall f=0, void* user_data = UNDEFINED_USER_DATA );
		FGLabel*        AddLabel(char *nm, int key=0, ControlCall f=0, unsigned i = UNDEFINED_COLOR, unsigned p = UNDEFINED_COLOR, void* user_data = UNDEFINED_USER_DATA );
		FGText*		    AddText(const char *s, unsigned ink = UNDEFINED_COLOR, unsigned paper = UNDEFINED_COLOR, void* user_data = UNDEFINED_USER_DATA );
		FGCheckBox*  	AddCheckBox(char *nm, int key=0, int * variable=0, ControlCall f=0, void* user_data = UNDEFINED_USER_DATA );
		FGCheckBox*  	AddCheckBox(char *nm, int key=0, bool * variable=0, ControlCall f=0, void* user_data = UNDEFINED_USER_DATA );
		FGRadioButton*  AddRadioButton(char *nm, int key=0, int * variable=0, ControlCall f=0, void* user_data = UNDEFINED_USER_DATA );
		FGRadioButton*  AddRadioButton(char *nm, int key=0, bool * variable=0, ControlCall f=0, void* user_data = UNDEFINED_USER_DATA );
		FGCheckBox*  	AddCheckBoxMask(int, char *nm, int key=0, int * variable=0, ControlCall f=0, void* user_data = UNDEFINED_USER_DATA );
		FGRadioButton*  AddRadioButtonMask(int, char *nm, int key=0, int * variable=0, ControlCall f=0, void* user_data = UNDEFINED_USER_DATA );
		FGEditBox*      AddEditBox(int	ws1, int ws2, char *nm,	int	key, int *p,ControlCall f, int min, int max, void* user_data = UNDEFINED_USER_DATA );
		FGEditBox*      AddEditBox(int	ws1, int ws2, char *nm,	int	key, int *p, ControlCall f=0, void* user_data = UNDEFINED_USER_DATA );
		FGEditBox*      AddEditBox(int	ws1, int ws2, char *nm,	int	key, char *p, ControlCall f=0, void* user_data = UNDEFINED_USER_DATA );
		FGEditBox*      AddEditBox(int sz, int	ws1, int ws2, char *nm,	int	key, char *p, ControlCall f=0, void* user_data = UNDEFINED_USER_DATA );
		FGEditBox*      AddEditBox(int	ws1, int ws2, char *nm,	int	key, double	*p,	ControlCall f, double min, double max, void* user_data = UNDEFINED_USER_DATA );
		FGEditBox*      AddEditBox(int	ws1, int ws2, char *nm,	int	key, double	*p,	ControlCall f=0, void* user_data = UNDEFINED_USER_DATA );
		void            Separator(void);
};

#ifdef FG_NAMESPACE
}
#endif

#ifdef __BORLANDC__
#pragma option pop /* pop -a switch */
#pragma option pop /* pop -b */
#endif


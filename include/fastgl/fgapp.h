#ifdef __BORLANDC__
#pragma option push -b
#pragma option push -a4
#endif

#ifdef FG_NAMESPACE
namespace fgl {
#endif

/**
The type of an Application handler. This procedure is called when system EVENT (of type FGEvent) is occured.
*/
typedef	int	(*MainHwnd)(FGEvent *);

/**
The type of an FGApp::SetTimerProc procedure callback.
*/
typedef	void (*TimerProc)(int);

/**
	The instance of this class encapsulates the application.
	You have to declare this construct at the start of your
	program. See the "Hello World" example.
*/
class FGApp : public FGConnector
{
		static const int pathsize = 256;
		friend class FGWindow;
		int mouseflag;
		static  int 	fulldrag;
		//! Container that contains all windows.
		static FGWindowContainer Windows;
		static FGWindow* Current;
		int XX, YY;
		bool locked;
		static FGInputDevice *user_input_drv;
		static int  __fg_fps, __fg_fps_total;
		int         mousefirst;
		static int  current_mouse_x;
		static int  current_mouse_y;
		int         current_mouse_buttons;
		int	        old_mouse_x;
		int         old_mouse_y;
		int         old_mouse_buttons;
		int         window_drag_in_action;
		int         button_reached;
		int	        window_resize_in_action;
		int			window_resize_width;
		int         window_resize_height;
		int         tmp_event;
		int         drag_width, drag_height;
		int			clickx,	clicky,	clickw,	clickh;
		int         selection_steps;

		static FGControl *idcb;
		static FGControl *idc2;

		FGWindow *drag_window;
		static int repeat_status, repeat_delay1, repeat_delay2;
		FGEvent *hold_event;
		static bool is_ctrl, is_alt, is_shift;

		enum { MAX_EVENT_QUEQUE=32	}; // numbers of events to app per one user event

		void blue_rect(int x, int y, int w, int h);
		void blue_rect2(int x, int y, int w, int h);
		void AutoRepeatStart(FGControl *);
		void AutoRepeatDo(void);
		void AutoRepeatEnd(void);
		void DoubleClick(int& event);

		static	FGEvent queque[MAX_EVENT_QUEQUE];
		static	void (*DelayProc)(void);
		static	void (*OnEverySecond)(int);
		static	int	quequeIndex;
		static  FGWindow *Root;
		static	MainHwnd appHandler;
		static  unsigned long ticks;
		static  int ctrl_break;
		static  int startXdrag;
		static  int startYdrag;
		static  int endXdrag;
		static  int endYdrag;
		static void (*DragShape)(int , int ,int	, int );
		static FGPixel __p4[8];
		static FGPattern __patt;
		void CallDelayProc(void)
		{
			if (DelayProc)
			{
				DelayProc();
				return;
			}
			else if (OnIdle() == true)
			{
				return ;
			}
			// wait 10 msec if nothing to do
			delay(10);
		}
		void GetUserEvent(FGEvent& e, int);
		void TranslateUserEvent(FGEvent& e);
		void ButtonClick(FGEvent& e, FGControl* ctrl);
		void Timer(void);
		int isgmkey(int	k);

	protected:
		static void SetCurrentWindow(FGWindow* novy) { Current = novy; }
		static void RemoveIterator(FGWindow* wnd);
		static FGWindowIterator GetIterator(FGWindow* Colise);
		static FGWindowRIterator GetRIterator(FGWindow* Colise);
		static 	FGWindow* WindowFind(FGEvent *e);
		static FGWindow* GetLastWindow(void)
		{
			return *(Windows.rbegin());
		}
		//! Finds a FGWindow by ID.
		static FGWindow* WindowFind(int idw);
		static void	FGAPI intersect(FGWindow* This, int, int, int, int);
		static FGAPI int _over(int, int, int, int, int xx, int yy, int ww, int hh);
		static FGWindow* OverSprite(FGWindowRIterator This, FGWindowRIterator _od, int	x, int y, int w, int h);
		static FGWindow* FGAPI OdkryteOkno(FGWindowRIterator This, FGWindowRIterator pokial, int x, int y, int w, int h);
		static void FGAPI DestroyWindow(void);
		static int NumberOfWindow(void)
		{
			return Windows.size();
		}
		static void AddWindowToList(FGWindow* wnd, int where);

	public:
		static void ResetCurrentControl(FGControl* ctrl )
		{
			if (ctrl == idcb)
			{
				idcb = idc2 = 0;
			}
		}
		//! Returns current focussed FGWindow if any.
		static FGWindow* GetCurrentWindow(void);
		int  RemoveMousePointer(void);
		void ShowMousePointer(void);
		int get_key(void);
		static  const FGMouseCursor *__fg_cursor;
		//! Internal flags.
		static	int		flags;
		//! Videomode number that is used for application.
		static	int		video;
		//! The color of application background.
		static	int		background;
		//! Standard C 'argc' on startup.
		static	int		Argc;
		//! Standard C 'argv' on startup.
		static	char	**Argv;
		//! Name of the application exefile.
		static	char	* name;
		//! Home directory (e.g. '/home/user' or '/root' ).
		static	char	*homedir;
		//! The application's starts directory.
		static  char	currdir[pathsize];
		//! true/false if TrueType fonts are supported.
		static	int 	ttf_support;
		/**
		Set the procedure that will be call many times per second when application will be idle.
		When the system is waiting to user input, it waits into inner loop.
		Because this time, I add to the library little hack. You can define
        some very little bit of code, procedure that will be called
		many times per second. Call with parameter 0 to switch this feature off.
		@param fnc your callback procedure of type void (*fnc)(void).
		*/
		void SetDelayProc(void (*fnc)(void))
		{
			DelayProc =	fnc;
		}
		void UpdateMousePointer(void);
		static void SaveScreen(void);
		/**
		By default, after <CTRL+C> hitting application normally shutdowns.
		You can switch this behaviour on/off by single calling this method.
		*/
		static void DisableCtrlBreak(void)
		{
			ctrl_break ^= 1;
		}
        /** To change window drawing when WindowMove event is performed.
			@param a true for opaque, false for window frame drawing only (fast).
		*/
		static void SetWindowMoveStyle(int a)
		{
			fulldrag = a;
		}
		FGApp(int m, int& argc, char **& argv, int bck, int appFlags=APP_ENABLEALTX);
		virtual ~FGApp();
		/**
		When you define the application procedure also,
		you can send to this procedure any event by this method.
		*/
		static	void FGAPI SendToApp(FGEvent	*x);
		void	FGAPI Run(MainHwnd hwnd=0);
		void	FGAPI FGYield(void);
		/**
		Call this methods, for example, from menu item "EXIT".
		When it is once called, it will causes jump out of the main
		application loop - return from Run().
		*/
		static	void AppDone(void)
		{
			FGEvent e(QUITEVENT);
			SendToApp(&e);
		}
		/**
		    Close the inner most FGApp loop only.
		*/
		static	void AppClose(void)
		{
			FGEvent e(CLOSEEVENT);
			SendToApp(&e);
		}
		/**
		This is another call-back procedure that will be call per each 't' msec.
		Your user-defined procedure expects one int parameter - number
		of second from the start of your program.
		Call with parameter 0 switch this feature off.
		NOTE! resolution is cca. 20ms
		@param p the callback routine
		@param t number of milisec. between two calls the callback (default is 1 sec.)
		*/
		void SetTimerProc(TimerProc p, int t=1000)
		{
			OnEverySecond =	p;
			ticks = t;
		}
		/**
		   Returns pointer to the ROOT window or 0 if one is not initialized.
		   Be sure before using this function, that you add switch APP_ROOTWINDOW
		   to the FGApp constructor else program will be aborted.
		   You can draw using this pointer with standard window stuff (WindowLine (),  WindowText () etc.)
		   NOTE: You can't use any Controls into this window, it is only for drawing!
		   You can initialize one in FGApp constructor time by passing APP_ROOTWINDOW as flags.
		   @return pointer to whole screen window ROOT.
		*/
		static FGWindow * GetRootWindow(void)
		{
			return Root;
		}
		MODAL_RETURN FGAPI RunModal(FGWindow *which);
		//! Returns current mouse coordinate in X axis.
		static int	GetMouseX(void)
		{
			return current_mouse_x;
		}
		//! Returns current mouse coordinate in Y axis.
		static int	GetMouseY(void)
		{
			return current_mouse_y;
		}
		/** @return true if the key is pressed at the time. */
		static bool IsShift(void);
		/** @return true if the key is pressed at the time. */
		static bool IsCtrl(void);
        /** @return true if the key is pressed at the time. */
		static bool IsAlt(void);

		static void set_mmx(void);
		static void reset_mmx(void);
		static int test_mmx(void);

		static void SetCaption(char *new_name);

		const FGMouseCursor * CursorLoad(const FGMouseCursor *cur);

		int EnableBuffering(int mode);
		int Flip(void);
		void DisableBuffering(void);

		ENUM_OS GetOS(void);

		/**
        Set variables with actual data. It is dependent on the app flag APP_MAGNIFIFIER.
		@param x position of top left corner
        @param y position of top left corner
		@param w the width of selected rectangle that is showed when you drag mouse with button hold-down.
		@param h the height of selected rectangle that is showed when you drag mouse with button hold-down.
        */
		static void	GetDragVector(int &x, int &y, int &w, int &h);
		/**
		Set procedure that will be called when you drag&drop to draw out dragging object.
        This procedures has four argument x,y for position in the current window
		and offset_y, offset_x are current offset from [x,y]. To restore default,
        use with argument 0. For more see the RAD project - source rad_prj.cpp.
		*/
		static void	SetDragShape(void(*a)(int,int,int,int)=0);
		static void SetRepeatDelay(int	c1,	int	c2);

		static void SetInputDevice(FGInputDevice *idev) { user_input_drv = idev; }
		void ResetInputDevice(void) { user_input_drv = 0; }

		void LockInput(void) { locked = true; }
		void UnlockInput(void) { locked = false; }

		// Object-oriented API via overloaded methods
		//! overload this member when you want catch any keypress event.
		virtual void OnKeyPress(int) { }
		//! overload this member when you want catch any mousemove event.
		virtual void OnMouseMove(int , int) { }
		//! overload this member when you want catch the click on left mouse's button event.
		virtual void OnClick(int, int) { }
		//! overload this member when you want catch the double click on left mouse's button event.
		virtual void OnDoubleClick(int, int) { }
		//! overload this member when you want catch the click on middle mouse's button event.
		virtual void OnMiddleButton(int, int) { }
		//! overload this member when you want catch the click on right mouse's button event.
		virtual void OnContextPopup(int, int) { }
		//! overload this member when you want catch the mouses wheel spin.
		virtual void OnWheel(int x, int y, int delta) { }
		//! overload this member when you want catch the start of mouse drag.
		virtual void OnStartDrag(int , int, int) {}
		//! overload this member when you want catch the end of mouse drag.
		virtual void OnEndDrag(int ,int, int, int, int) {}
		//! overload this member when you want catch the cursor arrives screen boundary.
		virtual void OnCursorOut(int)
		{
		}
		/**
		The wrapper for an application idle proc.
		@return If you override this on in your class and you call
		some code from it you must return true from this callback to know that the system
		will not wait. Elsewhere your system may go slowdown.
		@note if call SetDelayProc() with non-null argument then this callback will not be called!
		@see SetDelayProc()
		*/
		virtual bool OnIdle(void)
		{
			return false;	// returns true if do some work
		}
		/**
		The wrapper for application timer proc.
		@note if call SetTimerProc() with non-null first argument then this callback will not be called!
		The default time slice is 1000 msec. You can change this by call SetTimerProc() fith first argument
		null and second with number of msecs for a new timeslice.
		@param secs in secs from the start of app.
		@see SetTimerProc()
		*/
		virtual void OnTimer(int secs)
		{
		}
		//! override this member when you want to handle the TabPage switch
		virtual void OnTabSwitch(const char *) { }
};

/**
	The pointer to the current FGApp object
	@ingroup Globals
*/
extern FGApp *cApp;

#ifdef FG_NAMESPACE
}
#endif

#ifdef __BORLANDC__
#pragma option pop /* pop -a switch */
#pragma option pop /* pop -b */
#endif


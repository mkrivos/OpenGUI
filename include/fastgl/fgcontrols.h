#ifdef __BORLANDC__
#pragma option push -b
#pragma option push -a4
#endif

#ifdef FG_NAMESPACE
namespace fgl {
#endif

/**
 @internal.
*/
typedef	void (*ControlCall)(FGControl *);

/**
The pointer to a FGControl type.
*/
typedef	FGControl * CallBack;

//!	base objects for all graphics objects
/**
	This class is base parent of all graphics objects into OpenGUI library
	and encapsulates all basic methods. These methods are used in FGWindow,
	FGEditBox, FGRadioButton and other GUI classes.
	It implements a linked list of objects also. This is used to release memory
	when FGWindow is destroyed.
*/
class FGBaseGui : public FGDrawBuffer
{
	protected:
		enum bs	{ EBOX_LOW=1, EBOX_UPR, BDOWN, BUP, WHIDE, WVISIBLE, WDEACTIVE, WACTIVE };

		int		 	x;
		int		 	y;
		void* 		tab_page_controls;	// to test who is who
		void*		private_data;
		void*		user_data;
		unsigned    ink;
		unsigned	paper;
		bool        temporary_hidden;

		void		FGAPI Resize(int dx, int dy);
		void 		frame(void);
		virtual		void DrawAsSelected(void) {}

		virtual		~FGBaseGui();

	public:
		FGBaseGui(int	xs,	int	ys,	int	ws,	int	hs,	const char *nm, ObjectType typ, FGPixel i, FGPixel p, long flag);

		//! Returns a foreground color of the object.
		unsigned  	FGAPI GetInk(void)  const { return ink; }
		//! Retutns a background color of the object.
		unsigned   	FGAPI GetPaper(void)  const { return paper; }

		/**
		Returns vivibility of the object.
		*/
		bool	  	IsVisible(void)  const	{ return ! ( (status&WHIDEN) || temporary_hidden );	}
		/**
		Set the object's color as default for drawing.
		*/
		void        SetColors(void)
		{
			set_colors(ink, paper);
		}
		/**
		Returns the 'x' coordinates of the object.
		*/
		int		  	GetX(void)  const { return	x; }
		/**
		Returns the 'y' coordinates of the object.
		*/
		int		  	GetY(void)  const { return	y; }
		/**
		Callback on focus get.
		*/
		virtual void OnFocus(void) { }
		/**
		Callback on focus lost.
		*/
		virtual void OnLostFocus(void) { }
		/** @internal */
		void*		GetPrivateData(void) const { return private_data; }
		/** @internal */
		void		SetPrivateData(void* val) { private_data = val; }
		//! Get user data pointer
		void*		GetUserData(void) const { return user_data; }
		//! Set user data pointer
		void		SetUserData(void* val) { user_data = val; }
};

/**
When you write the code to manage some menus, sometimes you need program
the group of buttons, that would worked as welded. When you choose one,
previous goes to the inactive state - this is called a FGButtonGroup -
and make possible to you easy aggregate items like CheckBoxs,
RadioButtons, or FGPushButton (the word or is at a right place,
because you can't mix these types together!).
The default number of items is 32.
*/
class FGButtonGroup
{
		friend class FGControl;
		FGControl* 	curr;
		FGControl**	array;
		int			count;
		int			size;
		int			type; // 1-check,point; 2-push; 0 undefined

	public:
		/**
		Creates an empty group of the FGControl. Default number of items is 32. You can pass your own value.
		*/
		FGButtonGroup(int maximum=32)
		{
			size = maximum;
			count = type = 0;
			curr = 0;
			array = new FGControl *[size];
		}
		FGButtonGroup(const FGButtonGroup& old)
		{
			size = old.size;
			count = old.count;
			type = old.type;
			curr = old.curr;
			array = new FGControl *[size];
			for(int i=0; i<count; i++)
			{
				array[i] = old.array[i];
			}
		}
		~FGButtonGroup() { delete [] array; }
		/**
		Add FGControl object to the FGButtonGroup. The argument active is
		not mandatory (default = false) and if you set this to true,
		then object will have the active state at the start.
		If you mix no acceptable items together, or if you add other
		than allowed types, an error dialog will be displayed.
		*/
		void FGAPI AddToGroup(FGControl *p, int activ = 0);
		/**
			Reset the group. You can add controls now again.
		*/
		void FGAPI Clear(void) { type = count = 0; curr = 0; }
		//! Redraw all the group completely.
		void FGAPI RefreshGroup(FGControl *c);
		//! Disable all the group completely.
		void FGAPI DisableGroup(void);
		//! Enable all the group completely.
		void FGAPI EnableGroup(void);
};

//! The predecessor for the all widgets.
/**
This family of graphics controls contains the class FGPushButton, FGCheckBox,
FGEditBox and Base menu. It is child objects of class Controls, which is
a child object of base class FGBaseGui. There is a more information.
NOTE: All of the FGControl class objects and its parents are on the create
time automatically attached with its FGWindow. When a FGWindow is destroyed,
Controls are destroyed either automatically. Don't delete it explicitly!
@code
void call_back_procedure(CallBack cb)
{
	... // your code
}
@endcode
*/
class FGControl :	public FGBaseGui
{
		friend class FGWindow;
		friend class FGMenuWindow;
		friend class FGButtonGroup;
		friend class FGApp;
		friend class FGControlContainer;

		ControlCall	GetHandler(void)  const { return fnc; }

		void		RegisterToGroup(FGButtonGroup *g) { grp = g;}
		void		RefreshGroup(void);
		void		RemoveFromGroup(void) { grp = 0; }
		static 		FGControl * ButtonFind(FGEvent *e,int	n);

	protected:
		int			local_id;
		int			key;		   	// hotkey
		ControlCall	fnc;			// handler
		FGWindow*   owner;			// owner of this object
		FGButtonGroup *grp;			// if is group members
		char*		closure_name;

		FGControl(int	xs,	int	ys,	int	ws,	int	hs,	const char *nm, ObjectType typ, int key, FGWindow *, ControlCall f, FGPixel p, FGPixel i, void* user_data);
		virtual		~FGControl();

		int	   		GetXr(void);
		int	   		GetYr(void);
		void		Underscore(int xo, int yo, int c);
		void 		update_owner(void);
		virtual		void ClickDown(int x, int y) {}
		virtual		void WheelSpin(int) {}	// -1 up, 1 down
		virtual     void GetHints(int& xx, int& yy, int& ww, int& hh) { xx = x; yy = y; ww = w; hh = h; }
		virtual		void DrawAsSelected(void);
		bool        IsYourTabPage(void);
		bool		IsSelected(void);
		virtual 	FGPixel GetColorOfFrame(void) const { return CYELLOW; }
	public:
		/**
		Redraws whole the object.
		*/
		virtual		void draw(void) {}
		/**
		Change the name of the object and redraws one.
		*/
		virtual		void SetName(const char *s) { FGDrawBuffer::SetName(s); draw(); }
		/**
		Callback on the FGControl activation by user by click or hotkey pressing.
		You can override this in child object.
		*/
		virtual		void OnActivate(void) { RunSignal(); }
		/**
		Change the 'hotkey' of the object.
		*/
		void		SetKey(int k) {	if (k>='a' && k<='z') k	= toupper(k); key=k; draw(); }
		/**
			Returns current 'hotkey'.
		*/
		int			GetKey(void) const { return key; }
		/**
		Changes the state of the object (draws it as normal and allows activate one).
		*/
		void		Enable(void);
		/**
		Changes the state of the object (draws it as grayed and don't allows activate one).
		*/
		void		Disable(void);
		/**
		Change the state of the object to state "active" (as you as click
		with mouse at one). It is a good choice if you want input
		to the FGEditBox immediately without clicking at one.
		*/
		virtual		void ClickUp(int a);  // if TRUE, so call handler
		/**
		With this procedure you can change the state of the two-state (ON/OFF) switches.
		*/
		void		SetTrigger(int a) {	if (a) status |= WTRIGGER;	else	status &= (~WTRIGGER);	draw();}
		/**
		Returns the state of the FGControl (TRUE or FALSE).
		*/
		int	   		GetTrigger(void) const { return status&WTRIGGER?1:0;	}
		/**
		Returns the parent Widget, i.e. window that contains this FGControl.
		*/
		FGWindow		*GetOwner(void) const { return owner;	}
		/**
		Each FGControl in the FGWindow has local ID. You can use this ID with handling ACCELEVENT by example.
		The all controls have local id. This value goes from 0 to .., for each
		window and its control items. For example: when window contains 5 buttons,
		then these buttons have local id values from 0 to 4,according to the order
		of its creating. This is useful for fast and easy testing which button
		(when you use one call-back procedure for more buttons i.e.).
		*/
		int			GetLocalId() const { return local_id; }
		/**
		Set new foreground color and redraws object.
		*/
		void		SetInk(FGPixel i) { state._ink = ink = i; draw(); }
		/**
		Set new background color and redraws object.
		*/
		void		SetPaper(FGPixel p) { state._paper = paper = p; draw(); }
		/**
		Set new background color and redraws object.
		*/
		void		SetFont(unsigned f) { set_font(f); draw(); }
		/**
		Set a new handler for object.
		*/
		void		SetHandler(ControlCall w) {	fnc=w; }
		//! Set event name.
		void		AttachSignalName(const char* val);
		//! Get event name.
		const char* GetSignalName() { return closure_name; }
		/**
		You can use this static method as predefined callback when you can
		close the parent FGWindow.
		@code
			my_window->AddBaseMenu("Close the window", 'C', FGControl::Close);
		@endcode
		*/
		static		void Close(CallBack cb);
		/**
		You can use this static method as predefined callback when you can
		close the parent FGWindow.
		@code
			my_window->AddBaseMenu("Quit the Application", 'C', Quit::Close);
		@endcode
		*/
		static		void Quit(CallBack cb);

		//! Dynamic cast
		virtual const double ToDouble() const { assert(!"Not implemented!!!"); return 0; }
		//! Dynamic cast
		virtual const int ToInteger() const { assert(!"Not implemented!!!"); return 0; }
		//! Dynamic cast
		virtual const char* ToString() const { assert(!"Not implemented!!!"); return 0; }
};

/**
	Use Label to add text that user can't edit to a FGWindow. This text can be
	used to label another FGControl object. To add an object to a FGWindow that displays text that
	a user can edit use FGEditBox.

	This button can be grouped into the set of mutually exclussive options
	to the user - that is, only one radio button in a set can be selected
	at a time. When the user selects a button, the previously selected
	button becomes unselected. Buttons are frequently grouped in
	a button group box (FGButtonGroup). Add the group box to the FGWindow first,
	then put the buttons into the group box.
	For example, two buttons on a FGWindow can be checked at the same time only if they are
	contained in separate containers, such as two different group boxes.

	Use FGWindow::AddLabel() instead.
*/
class FGLabel : public FGControl
{
	protected:
		void ClickUp(int a);
		unsigned int is_transparent:1;
		virtual ~FGLabel() { }
	public:
		// redraws object completely
		virtual	void draw(void);
		FGLabel(int xs, int ys, const char *nm, int key, FGWindow *w, ControlCall f, unsigned i, unsigned p, void* user_data);
		void SetTransparent(void) { is_transparent = 1; draw(); }
};

/**
* A 'decorate' class. This is not a real GUI FGControl item. It is
* intended for visual purposes only. There was an old method WindowPanel()
* that not success because of it's static character - when FGWindow was overdrawed
* the all Controls redraws correctly but static text and - WindowPanel was washed out.
* This new class is redrawed automagically (as child of FGControl class).
*/
class FGPanel : public FGControl
{
	protected:
		FGPixel	ink1, ink2;
		virtual ~FGPanel() { }
	public:
		// redraws object completely
		virtual	void draw(void);
		FGPanel(int xs, int ys, int ws, int hs, const char *nm, FGWindow *w, int i1, int i2, int p);
};

/**
* A 'decorate' class. This is not a real GUI FGControl item. It is
* intended for visual purposes only - i.e. use with TabPages.
* This new class is redrawed automagically (as child of FGControl class).
* @see Window::AddImage()
*/
class FGImage : public FGControl
{
	protected:
		FGDrawBuffer*   bitmap;
		virtual ~FGImage() { }
	public:
		//! redraws object completely
		virtual	void draw(void);
		//! create FGImage object and attach FGDrawImage object to the one.
		FGImage(int xs, int ys, FGDrawBuffer* img, FGWindow *w);
};

/**
* A 'decorate' class. This is not a real GUI FGControl item. It is
* intended for visual purposes only. When FGWindow was overdrawed
* the all Controls redraws correctly but static text and - WindowPanel was washed out.
* This new class is redrawed automagically (as child of FGControl class).
*/
class FGText : public FGControl
{
	protected:
		virtual ~FGText() { }
	public:
		FGText(int xs, int ys, const char *nm, FGWindow *w, int i, int p);
		// redraws object completely
		virtual	void draw(void);
};

/**
	Use FGPushButton to put a standard push button on a FGWindow. FGPushButton
	introduces several properties to control its behaviour in a dialog box settings.
	Users choose button controls to initiate actions.

	@image html pushbutt.png

	This button can be grouped into the set of mutually exclussive options
	to the user - that is, only one radio button in a set can be selected
	at a time. When the user selects a button, the previously selected
	button becomes unselected. Buttons are frequently grouped in
	a button group box (FGButtonGroup). Add the group box to the FGWindow first,
	then put the buttons into the group box.
	For example, two buttons on a FGWindow can be checked at the same time only if they are
	contained in separate containers, such as two different group boxes.

	To add this FGControl use FGWindow::AddPushButton()
*/
class FGPushButton : public FGControl {
		FGDrawBuffer*   icon;
		FGDrawBuffer*   pushed;
		FGDrawBuffer*   disabled;
		FGPixel			back_pushed;
		FGPixel			fore_pushed;
	protected:
		virtual ~FGPushButton() { }
	public:
		FGPushButton(int xs, int ys, int ws, int hs, const char	*nm, int key, FGWindow *w, ControlCall f, void* user_data)
			: FGControl(xs, ys, ws, hs, nm, PUSHBUTTON, key, w, f, CWHITE, CBLACK, user_data)
			{
				set_font(FGFONT_BUTTON);
				icon = 0;
				pushed = disabled = 0;
				ink = CScheme->button_fore;
				paper = CScheme->button_back;
				back_pushed = CScheme->button_back_pushed;
				fore_pushed = CScheme->button_fore_pushed;
				draw();
			}
		FGPushButton(int xs, int ys, int key, FGWindow *w, FGDrawBuffer	*bm, ControlCall f, void* user_data)
			: FGControl(xs, ys, bm->GetW() + 4, bm->GetH() + 4, "", PUSHBUTTON_IMAGE, key, w, f, CWHITE, CBLACK, user_data)
			{
				set_font(FONTSYS);
				icon = bm;
				pushed = disabled = 0;
				draw();
			}
		FGPushButton(int xs, int ys, int ws, int hs, const char* nm, int key, FGWindow *w, FGDrawBuffer	*bm, ControlCall f, void* user_data)
			: FGControl(xs, ys, ws, hs, nm, PUSHBUTTON_IMAGE, key, w, f, CWHITE, CBLACK, user_data)
			{
				set_font(FONTSYS);
				icon = bm;
				pushed = disabled = 0;
				draw();
			}
		void ClickDown(int x, int y);
		void ClickUp(int);
		void Push(void);
		void Release(void);
		// redraws object completely
		virtual	void draw(void);
		void AddBitmaps(FGDrawBuffer* push=0, FGDrawBuffer* disable=0)
		{
			pushed = push;
			disabled = disable;
			draw();
		}
};

/**
	FGCheckBox represents a check box that can be on (checked)
	or off (unchecked). It differents from FGRadioButton only visually.

	@image html check.png

	This button can be grouped into the set of mutually exclussive options
	to the user - that is, only one radio button in a set can be selected
	at a time. When the user selects a button, the previously selected
	button becomes unselected. Buttons are frequently grouped in
	a button group box (FGButtonGroup). Add the group box to the FGWindow first,
	then put the buttons into the group box.
	For example, two buttons on a FGWindow can be checked at the same time only if they are
	contained in separate containers, such as two different group boxes.

	To add this FGControl use FGWindow::AddCheckBox() .
*/
class FGCheckBox :	public FGControl {
	protected:
		int		mask;
		int		* variable;
		virtual ~FGCheckBox() { }
	public:
		FGCheckBox(int m, int	xs,	int	ys,	const char *nm, int key, FGWindow	*w,	int	fg,	int	bg,	ControlCall f, int *var, void* user_data)
			: FGControl(xs, ys, strlen(nm)?strlen(nm)*8+20:16, 16, nm, CHECKBUTTON,	key, w, f, bg, fg, user_data)
			{
				set_font(FGFONT_BUTTON);
				variable = var;
				mask = m;
				if (variable) SetTrigger(*variable&mask); else draw();
			}
		FGCheckBox(int	xs,	int	ys,	const char *nm, int key, FGWindow	*w,	int	fg,	int	bg,	ControlCall f, bool *var, void* user_data)
			: FGControl(xs, ys, strlen(nm)?strlen(nm)*8+20:16, 16, nm, CHECKBUTTON,	key, w, f,bg,fg, user_data)
			{
				set_font(FGFONT_BUTTON);
				variable = (int *)var;
				mask = 1;
				if (variable) SetTrigger(*variable&mask); else draw();
			}
		void ChangeItem(int *v) { variable = v; SetTrigger(*variable&mask); draw(); }
		void ChangeItem(bool *v) { variable = (int *)v; SetTrigger(*variable&mask); draw(); }
		// redraws object completely
		virtual	void draw(void);
		virtual const int ToInteger() const { return *variable; }
};

/**
	FGRadioButton represents a radio button that can be on (checked)
	or off (unchecked). It differents from FGCheckBox only visually.

	@image html point.png

	This button can be grouped into the set of mutually exclussive options
	to the user - that is, only one radio button in a set can be selected
	at a time. When the user selects a button, the previously selected
	button becomes unselected. Buttons are frequently grouped in
	a button group box (FGButtonGroup). Add the group box to the FGWindow first,
	then put the buttons into the group box.
	For example, two buttons on a FGWindow can be checked at the same time only if they are
	contained in separate containers, such as two different group boxes.

	To add this FGControl use FGWindow::AddCheckBox() .
*/
class FGRadioButton :	public FGControl {
	protected:
		int		mask;
		int		* variable;
		virtual ~FGRadioButton() { }
	public:
		FGRadioButton(int m, int xs, int ys, const char *nm, int key, FGWindow	*w,	int	fg,	int	bg,	ControlCall f, int *var, void* user_data)
			: FGControl(xs, ys, strlen(nm)?strlen(nm)*8+20:16, 16, nm, POINTBUTTON,	key, w, f,bg,fg,user_data)
			{
				set_font(FGFONT_BUTTON);
				variable = var;
				mask = m;
				if (variable) SetTrigger(*variable&mask); else draw();
			}
		FGRadioButton(int m, int xs, int ys, const char *nm, int key, FGWindow	*w,	int	fg,	int	bg,	ControlCall f, bool *var, void* user_data)
			: FGControl(xs, ys, strlen(nm)?strlen(nm)*8+20:16, 16, nm, POINTBUTTON,	key, w, f,bg,fg,user_data)
			{
				set_font(FGFONT_BUTTON);
				variable = (int *)var;
				mask = m;
				if (variable) SetTrigger(*variable&mask); else draw();
			}
		void ChangeItem(int *v) { variable = v; SetTrigger(*v&mask); draw(); }
		void ChangeItem(bool *v) { variable = (int *)v; SetTrigger(*variable&mask); draw(); }
		// redraws object completely
		virtual	void draw(void);
		virtual const int ToInteger() const { return *variable; }
};

/**
	The two state (like FGCheckBox) FGPushButton.
*/
class FGTwoStateButton : public FGRadioButton
{
	protected:
		virtual ~FGTwoStateButton() { }
	public:
		FGTwoStateButton(int xs, int ys, int ws, int hs, const char *nm, int key, FGWindow *wnd, int *var, ControlCall f, void* user_data) :
			FGRadioButton(1, xs,ys,"", key, wnd, 0, 15, f, var, user_data)
		{
			Resize(ws-w, hs-h);
			SetName(nm); // ma v sebe draw
		}
		// redraws object completely
		virtual	void draw(void);
};

/**
You can use this object to get any text or number from user.
When you activate such object, you can use keyboard to edit
the text (HOME, DEL, BACKSPACE, ARROWS and ESC work properly).
The input is passed to your program when the user hits
the 'ENTER' key. The ESC key pressing terminates the input without change.
You can type input line up to 127 characters long. The special shortcuts
CTRL+INS and SHIFT+INS are used to standard COPY&PASTE mechanism.

@image html editbox.png

*/
class FGEditBox :	public FGControl
{
	public:
		//! max. size of the edited text string
		static const int bufsize = 252;
	private:
		friend class FGUpDown;
		friend class FGWindow;

		enum dtype { EDIT_INT, EDIT_STRING, EDIT_DOUBLE};

		void init(void);
		void UpDown(double step)
		{
			if (data_type==EDIT_INT)
			{
				 *(int	*)ptr += int(step);
			}
			else if	(data_type==EDIT_DOUBLE)
			{
				 *(double *)ptr += step;
			}
			TestRange();
		}
		void TestRange(void)
		{
			if (check_range==0) return;
			if (data_type==EDIT_INT)
			{
				int	dato = *(int *)ptr;
				if (dato<min) *(int	*)ptr =	min;
				else if	(dato>max) *(int *)ptr = max;
			}
			else if	(data_type==EDIT_DOUBLE)
			{
				double dato	= *(double *)ptr;
				if (dato<mind) *(double	*)ptr =	mind;
				else if	(dato>maxd)	*(double *)ptr = maxd;
			}
		}
		int	inputproc(unsigned);
		void input(void);
		void FlushInput(bool reset=false)
		{
			if (isinput)				   // if input, force ESC
			{
				inputproc(reset ? ESC : CR);
			}
		}
		virtual	void WheelSpin(int direction)
		{
			if (direction<0) UpDown(-1);
			else UpDown(1);
			draw();
			if (fnc)
				fnc(this);
		}

	protected:
		int		w1,w2,min,max;
		dtype   data_type;
		bool first,passwd,check_range,hex,isinput, octal;
		int  caps;
		double	mind,maxd;
		void  *	ptr;
		char	buf[bufsize];
		static	char clip[bufsize];
		static	int is_clip;
		int		pos, maxpos, iline, icol, offset, size;

		//! draw in input modet
		virtual	void draw(bool cursor, bool first_time=false);
		virtual ~FGEditBox();

	public:
		// redraws object completely
		virtual	void draw(void);
		//! Dynamic cast
		const double ToDouble() const { if (data_type!=EDIT_DOUBLE) assert(!"Not a double type!!!"); return *(double*)ptr; }
		//! Dynamic cast
		const int ToInteger() const { if (data_type!=EDIT_INT) assert(!"Not an integer type!!!"); return *(int*)ptr; }
		//! Dynamic cast
		const char* ToString() const { if (data_type!=EDIT_STRING) assert(!"Not a string type!!!"); return (const char*)ptr; }
		/**
		Switch the inputbox into input mode (shows cursor and redirect all FGWindow's keypress into one).
		*/
		void ClickUp(int state=1);
		/**
		Sets password mode. All typed characters will be shown as '*'.
		*/
		void PasswdMode(int m) { passwd = m; draw(); }
		/**
		The input will parsed as a hexadecimal number (without '0x' prefix).
		*/
		void HexMode(int m) { hex = m; draw(); }
		/**
		The input will parsed as an octal number (without '0' prefix).
		*/
		void OctalMode(int m) { octal = m; draw(); }
		/**
		All letters will be converted to the capitals.
		*/
		void CapsMode(int m);
		/**
		Change the data that are edited by editbox.
		You must call this methods to apropriate FGEditBox type - this is for an integer type of the object.
		*/
		void ChangeItem(int	*p)
		{
			assert(data_type==EDIT_INT);
			ptr	= p;
			draw();
		}
		/**
		Change the data that are edited by editbox.
		You must call this methods to apropriate FGEditBox type - this is for an string type of the object.
		*/
		void ChangeItem(char *p)
		{
			assert(data_type==EDIT_STRING);
			ptr= p;
			draw();
		}
		/**
		Change the data that are edited by editbox.
		You must call this methods to apropriate FGEditBox type - this is for an double type of the object.
		*/
		void ChangeItem(double *p)
		{
			assert(data_type==EDIT_DOUBLE);
			ptr	= p;
			draw();
		}
		/**
		Change the range for edited values.
		You must call this methods to apropriate FGEditBox type - this one is for a double type of the object.
		*/
		void SetRange(double mi, double ma)
		{
			assert(data_type==EDIT_DOUBLE);
			mind = mi;
            maxd = ma;
			TestRange();
			draw();
		}
		/**
		Change the range for edited values.
		You must call this methods to apropriate FGEditBox type - this one is for a int type of the object.
		*/
		void SetRange(int mi, int ma)
		{
			assert(data_type==EDIT_INT);
			min = mi;
            max = ma;
            TestRange();
			draw();
		}
		/**
		Sets a new size of the input string. Default is calculated from the objects width. You can set values from 1 to 127.
		*/
		void SetSize(int ns)
		{
			pos = offset = 0;
			if(ns>0 && ns<bufsize)
			{
				size = ns;
				draw();
			}
		}
		/**
		Override the disable methods.
		*/
		void Disable(void);
		/**
		Create an editbox for an integer number input. Don't call this directly. Rather use FGWindow::AddEditBox() methods instead.
		*/
		FGEditBox(int xs,	int ys,	int ws1, int ws2, const char *nm, int	key, FGWindow *w,	int *pt, int ink, int paper, ControlCall f, int mn, int mx, int check, void* user_data);
		/**
		Create an editbox for an text input. Don't call this directly. Rather use FGWindow::AddEditBox() methods instead.
		*/
		FGEditBox(int sz, int	xs,	int	ys,	int	ws1, int ws2, const char *nm,	int	key, FGWindow	*w,	char *pt, int ink, int paper, ControlCall f, void* user_data);
		/**
		Create an editbox for an double precission number input. Don't call this directly. Rather use FGWindow::AddEditBox() methods instead.
		*/
		FGEditBox(int xs,	int ys,	int ws1, int ws2, const char *nm, int	key, FGWindow *w,	double *pt, int	ink, int paper,	ControlCall f, double mn, double mx, int check, void* user_data);
};

/**
When your application performs a time-consuming operation, you can use a progress bar
to show how much of the task is completed. A progress bar displays a dotted line
that grows from left to right.

@image html progress.png

*/
class FGProgressBar : public FGControl
{
	protected:
		int		steps, value;
		int		sirka;
		virtual ~FGProgressBar() { }
	public:
		void	draw(void);
		/**
			Set new value of object in range <0,steps>
		*/
		void	FGAPI setProgress(int a) { if (a>=0 && a<=steps) value = a; draw(); }
		/**
			Returns current value of object.
		*/
		int	FGAPI progress(void) const { return value; }
		/**
			Creates a progress bar.
			@param w parent FGWindow
			@param xx x coord
			@param yy y coord
			@param ww width in pixels
			@param hh height in pixels
			@param s step
		*/
		FGProgressBar(FGWindow *w, int xx, int yy, int ww, int hh, int s);
};

/**
	Use FGBaseMenu to specify the apperance and behaviour of an item
	in a menu. Each FGWindow can contain multiple menu items.
	@see FGWindow::AddMenu().
*/
class FGBaseMenu : public	FGControl
{
	protected:
		virtual ~FGBaseMenu() { }
	public:
		// redraws object completely
		virtual	void draw(void);
		void DrawAsSelected(void);
		virtual	void ClickUp(int a);  // if TRUE, so call handler
		FGBaseMenu(const char *nm, int key, FGWindow *w, ControlCall f, int font, void* user_data);
		virtual 	FGPixel GetColorOfFrame(void) const { return CYELLOW; }
};

/**
	Use FGBaseMenuItem to specify the apperance and behaviour of an item
	in a menu. Each FGMenuWindow can contain multiple menu items.
*/
class FGBaseMenuItem : public	FGControl
{
	protected:
		virtual ~FGBaseMenuItem() { }
	public:
		// redraws object completely
		virtual	void draw(void);
		void DrawAsSelected(void);
		virtual	void ClickUp(int a);  // if TRUE, so call handler
		FGBaseMenuItem(const char *nm, int key, FGMenuWindow *w, ControlCall f, int font, void* user_data);
		virtual 	FGPixel GetColorOfFrame(void) const { return CScheme->pdmenu_color_of_frame; }
};

/**
	The parent for all Sliders.
*/
class FGSlider  : public FGControl
{
	friend class listboxEx;
	protected:
		virtual void draw(void) {}
		int		minv,maxv,*val,steps,smer; // 0 je vodorovny, 1 zvysly
		void 	frame(int,int,int,int,int f=0);
		virtual void ClickDown(int x, int y);
		virtual	void WheelSpin(int direction)
		{
			if (direction<0) StepDown();
			else if (direction>0) StepUp();
		}
		void StepDown(void)
		{
			modify(*val-minv-steps);
		}
		void StepUp(void)
		{
			modify(*val-minv+steps);
		}
		virtual ~FGSlider() { }
	public:
		void	modify(int);
		void    redraw(void)
		{
			ControlCall tmp = fnc;
			fnc = 0;
			draw();
			fnc = tmp;
		}
		FGSlider(FGWindow *win, int xs, int ys, int ws, int hs, int minimal, int maximal, int s, int *v, ControlCall f, void* user_data) :
			FGControl(xs, ys, ws, hs, 0, SLIDEBAR, 0, win, f, CWHITE, CBLACK, user_data), minv(0), maxv(0)
		{
			if (minimal>maximal)
			{
				minv = maximal;
				maxv = minimal;
			}
			else
			{
				minv = minimal;
				maxv = maximal;
			}
			if (*v<minv)
				*v=minv;
			if (*v>maxv)
				*v=maxv;
			val  = v;
			steps= s;
			smer = h>w;
		}
		virtual const int ToInteger() const { return *val; }
};

/**
	A horizontal FGSlider.
	@image html sliderh.png
*/
class FGSlideBarH : public FGSlider
{
	protected:
		virtual ~FGSlideBarH() { }

	public:
		FGSlideBarH(int x, int y, int min, int max, int step, int *val, FGWindow *win, ControlCall f, void* user_data):
			FGSlider(win,x,y,(max-min)/step+16+12+38,16,min,max,step,val,0,user_data)
		{
			draw();
			fnc = f;
		}
		// redraws object completely
		virtual	void draw(void);
};

/**
	A vertical FGSlider.
	@image html sliderv.png
*/
class FGSlideBarV : public FGSlider
{
	protected:
		virtual ~FGSlideBarV() { }
	public:
		FGSlideBarV(int x, int y, int min, int max, int step, int *val, FGWindow *win, ControlCall f, void* user_data):
			FGSlider(win,x,y,16,(max-min)/step+16+12+38,min,max,step,val,0,user_data)
		{
			draw();
			fnc = f;
		}
		virtual void draw(void);
};

/**
	Internal implementation of tab pages
	@internal
*/
class FGControlContainer : public std::list<FGControl *>
{
		FGControl* default_control;
		const char* idstring;
		int	width, ypos, flag;
	public:
		FGControlContainer(const FGControlContainer& old);
		FGControlContainer(const char *s, int y);
		~FGControlContainer();

		const char	*GetString(void) { return idstring; }
		int			GetWidth(void) { return width; }
		int			GetY(void) { return ypos; }
		int			GetFlag(void) { return flag; }
		void		SetFlag(int f) { flag = f; }
		void		SetDefaultControl(FGControl* ctrl);
		FGControl*	GetDefaultControl(void);
		void		SetNextControl(void);
		void		SetPreviousControl(void);
		void		Rename(const char *);
};

typedef FGControlContainer::iterator FGControlIterator;

typedef std::list<FGControlContainer> FGControlBox;
typedef FGControlBox::iterator FGControlBoxIterator;

#ifdef FG_NAMESPACE
}
#endif

#ifdef __BORLANDC__
#pragma option pop /* pop -a switch */
#pragma option pop /* pop -b */
#endif


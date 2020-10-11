#ifdef __BORLANDC__
#pragma option push -b
#pragma option push -a4
#endif

#ifdef FG_NAMESPACE
namespace fgl {
#endif

//! The FGWindow has frame.
const unsigned  WFRAMED =		0x0001;
//! The FGWindow has caption.
const unsigned 	WTITLED	=	0x0002;
//! The FGWindow will be shown as modal.
const unsigned 	WMODAL =		0x0004;
//! The FGWindow sends EVENTNOTIFY to its handler when it is destroyed.
const unsigned 	WNOTIFY	=	0x0008;
//! internal
const unsigned 	WHIDEN =		0x0010;
//! The FGWindow is unable to move/change its position.
const unsigned 	WUNMOVED =	0x0020;
//! The FGWindow has a line menu.
const unsigned 	WMENU =		0x0040;
//! The FGWindow is without pictograms (to minimize and to hide it).
const unsigned 	WNOPICTO =	0x0080;
//! internal
const unsigned 	WBITMAP	=	0x0100;
//! @deprecated
const unsigned 	WCLICKABLE =	0x0200;
//! internal
const unsigned 	WDIRTY =		0x0400;
//! Set the first control as SELECTED when is added.
const unsigned 	WUSESELECTEDCONTROL =	0x0800;
//! internal
const unsigned 	WLOCKED	=	0x1000;
//! internal
const unsigned 	WTRIGGER =	0x2000;
//! The FGWindow don't get focus on create.
const unsigned 	WLASTFOCUS =	0x4000;
//! The FGWindow is able to change its size.
const unsigned 	WSIZEABLE =	0x8000;
//! The FGWindow sets its size and position from WindowDatabase if any on create time.
const unsigned 	WUSELAST =	0x00010000;
//! Show the minimize pictogram in the FGWindow title.
const unsigned 	WMINIMIZE =	0x00020000;
//! The FGWindow has a statusbar in the bottom.
const unsigned 	WSTATUSBAR =	0x00040000;
//! internal
//const unsigned 	WEXIST =		0x00080000;
//! internal
const unsigned 	WGLCONTEXT =	0x00100000;
/**
The FGWindow uses notebooks. Use this flag to create multiple page dialog
or tabbed notebook. It displays multiple overlapping pages that are TabPage
objects. The user selects selects a page by clicking the page's tab that appears
at the top of control.
@see FGWindow::AddTabPage()
*/
const unsigned 	WNOTEBOOK =	0x00200000;
//! most bottom
const unsigned 	WBACK =	0x00400000;
//! Show the FGWindow in the center of screen.
const unsigned 	WCENTRED =	0x00800000;
//! Close the FGWindow when ESCAPE key is pressed.
const unsigned 	WESCAPE	 =	0x01000000;
//! Alligne the FGWindow to 32 bit sizes
const unsigned 	WALIGNED =	0x02000000;
//! Enable fast move of the FGWindow, i.e. don't redraw opaque.
const unsigned 	WFASTMOVE =	0x04000000;
//! Top most window.
const unsigned 	WTOP =	0x08000000;
//! internal
const unsigned 	WNOICON	=	0x10000000;
//! internal
const unsigned 	WNODRAWFROMCONSTRUCTOR = 0x20000000;
//! internal
const unsigned 	WICONWINDOWTYPE	=   0x40000000;
//! internal
const unsigned 	WMENUWINDOWTYPE	=   0x80000000;
//! standard predefined values.
const unsigned 	WSTANDARD =	(WTITLED|WFRAMED);

/**
	A base drawing object.
	This object is used as a parent for the all graphics stuff.
	You can use this one for in-memory drawing and after drawing
	complete update the screen with contents of this object.
*/
class FGDrawBuffer : public FGConnector
{
		struct internal_state
		{
			unsigned       _ink;
			unsigned	   _paper;
			int		 	   _font;	// font code
			unsigned       _ppop;
			unsigned	   colorkey;
			unsigned 	   alpha;
			
			internal_state()
			{
			    _ink = CWHITE;
			    _paper = CBLACK;
			    _font = 1;
			    _ppop = _GSET;
			    colorkey = DEFAULT_COLOR_KEY;
			    alpha = DEFAULT_ALPHA_VALUE;
			}
		};

	public:
		enum ObjectType {	WINDOW=0x4000, CHECKBUTTON, PUSHBUTTON, POINTBUTTON, EDITBOX, PROGRESSBAR,
			BASEMENU, BASEMENUITEM, MENUWINDOW, SLIDEBAR, MULTILINEEDIT, LISTBOX, LABEL, WINDOWPANEL,
			BMP_FILE, BMP_IMAGE, BMP_MEM, ROOTWINDOW, FGFRAMEBUFFER, COMBOBOX, WINDOWTEXT, WINDOWIMAGE,
			PUSHBUTTON_IMAGE,
			BMP_NONE=0 };
		friend class GLSurface;
		friend class listbox;
		friend class listboxEx;
	private:
		static	int ID_counter;
		int		id;
		FGMutex	atomic;

		inline void	lock(void)
		{
			atomic.Lock();
		}
		inline void	unlock(void)
		{
			atomic.Unlock();
		}
		void	FGAPI _symetry(int xs, int ys, int x,	int	y, FGPixel color);
		void	FGAPI _symetry2(int xs, int ys, int x, int y, FGPixel color);
		void	FGAPI _init(int ww, int hh, ObjectType t, FGPixel ii=CBLACK, FGPixel pp=CWHITE, const char *nm=0);
		void	FGAPI turn_bitmap(int cnt, FGPixel *from, FGPixel *to);
	protected:
		int		w;
		int		h;
		int		xoff,yoff;	// coor
		int		wwrk,hwrk;	// sizes!!!
		unsigned long	status;
		FGMutex	clip_lock;
		FGPixel	*image;
		char 	*name;
		union 	{ ObjectType	type; int _asd; };

		internal_state state;
		internal_state state_backup;
		bool        state_saved;

		static	const char *empty_string;
		static	int oldx, oldy;

		FGPixel *	FGAPI SetArray(FGPixel *array)
		{
			if (image) free(image);
			return image = array;
		}

		FGDrawBuffer(int ww, int hh, const char * nm, ObjectType t, FGPixel i, FGPixel p);

		void 	FGAPI SetStatus(long s) {	status = s;	}
		static int FGAPI GetCounter(void) { return ID_counter; }
		//! Clip coordinates and size to the client space.
		inline int FGAPI clip(int& xr, int& yr, int& wr, int& hr)
		{
			if (xr<xoff)
			{
				wr -= (xoff-xr);
				xr = xoff;
			}
			if (yr<yoff)
			{
				hr -= (yoff-yr);
				yr = yoff;
			}
			if (xr+wr > xoff+wwrk)
			{
// FIXME
				wr = wwrk-(xr-xoff);
//				wr = wwrk;
			}
			int yoff2 = h - (hwrk+yoff);
			if (yr+hr > h-yoff2)
			{
				hr = h-yr-yoff2;
			}
			return (wr<1 || hr<1) ? 0 : 1; // return if valid
		}
		inline int FGAPI clip_shape(int& xr, int& yr, int& wr, int& hr)
		{
			if (xr<0)
			{
				wr += xr;
				xr = 0;
			}
			if (yr<0)
			{
				hr += yr;
				yr = 0;
			}
			if (xr+wr>w)
			{
				wr = w-xr;
			}
			if (yr+hr>h)
			{
				hr = h-yr;
			}
			return (wr<1 || hr<1) ? 0 : 1; // return if valid
		}
		void 	FGAPI modify_point(FGPixel *ptr, FGPixel ink);
		void 	__line(int,int,int,int, unsigned color);
	public:
        //! Save the internal state of object described by struct internal_state (ink, paper, font, alpha, colorkey anf ppop).
        //! @note the state is not stacked, i.e. you can call this methotd only once before restore_state().
	    void save_state(void)
		{
			lock();
			if (state_saved)
			{
			    ::printf("FGDrawBuffer::save_state() multiple call!\n");
			}
			else
			{
				state_backup = state;
			}
			state_saved = true;
			unlock();
		}
        //! Restore the state saved by save_state().
		void restore_state(void)
		{
			lock();
			if (state_saved == false)
			{
				::printf("FGDrawBuffer::restore_state() called without save_state()!\n");
			}
			else
			{
				state = state_backup;
			}
			state_saved = false;
			unlock();
		}
		/**
		Calculate the absolute address of [x:y] pixel. There are no relative offsets used.
		*/
		inline FGPixel* FGAPI CalcAddr(int x, int y)
		{
			return image + (w * y + x);
		}
		/**
		Resizes the buffer by relative sizes.
		*/
		void	FGAPI Resize(int dx, int dy);
		/**
		Returns the status of object.
		*/
		long	FGAPI GetStatus(void)  const	{ return status; }
		//! Try get the font image and set requested font ID
		//! if success for the object as current.
		inline 	void FGAPI set_font(int font_id)
		{
			if (FGFontManager::get_font_ptr(font_id))
				state._font = font_id;
		}
		//! Returns a current object's font ID.
		inline 	int FGAPI get_font(void)
		{
			return state._font;
		}
		//! Returns a width of the 'text' in pixels for current font.
		inline int	FGAPI textwidth(const char *text)
		{
			return FGFontManager::textwidth(state._font, text);
		}
        //! Returns a width for the first 'cnt' characters
        //! of the 'text' in pixels for current font.
		inline int	FGAPI textwidth(char *txt, int cnt)
		{
			return FGFontManager::textwidth(state._font, txt, cnt);
		}
        //! Returns a width of the character 'c' in pixels for current font.
		inline int	FGAPI charwidth(int c)
		{
			return FGFontManager::charwidth(state._font, c);
		}
		//! Returns a width of one character for current font if one is fixed-size.
		inline int	FGAPI GetFontW(void)
		{
			return FGFontManager::GetW(state._font);
		}
        //! Returns a height of one character for current font if one is fixed-size.
		inline int	FGAPI GetFontH(void)
		{
			return FGFontManager::GetH(state._font);
		}
		//! Set the ROP for all drawing operations used with the object.
		//! See enum ENUM_PPOP for more details.
		inline unsigned FGAPI set_ppop(unsigned op)
		{
			unsigned oldp = state._ppop;
			state._ppop = op;
			return oldp;
		}
        //! Set the foreground color for drawing operations.
		//! This color will be used when you choose function
		//! that don't uses color as parameter immediatelly.
		inline 	void FGAPI set_fcolor(FGPixel val) { state._ink = val; }
		//! Set the background color for text drawing operations.
		//! This color will be used when you choose function
		//! that don't uses color as parameter immediatelly.
		inline 	void FGAPI set_bcolor(FGPixel val) { state._paper = val; }
		//! Set the foreground (ink) and the background (paper)
		//! color for drawing operations.
		inline 	void FGAPI set_colors(FGPixel i, FGPixel p) { state._ink = i; state._paper = p; }
		//! Returns the ID of the object.
        /**
        The all controls have an exclusive ID. This value goes from 0 to ..,
		for each window and its control items. For example: when window
		contains 5 buttons, then these buttons have local id values
        from 0 to 4,according to the order of its creating. This is
		useful for fast and easy testing which button (when you use
        one call-back procedure for more buttons i.e.) especially
        with using GetLastActive();
		*/
		int		FGAPI GetId(void)  const	{ return id; }
		//! Set position and size of the client space for the object (ww & hh are size).
		inline bool FGAPI set_clip_rect(int xx, int yy, int ww, int hh)
		{
			if (hh<1 || ww<1 || xx<0 || yy<0) return false;
			xoff = xx;
			yoff = yy;
			if ((xx + ww)	> w) ww -= ((xx + ww) - w);
			if ((yy + hh)	> h) hh -= ((yy + hh) - h);
			wwrk = ww;
			hwrk = hh;
			return true;
		}
        //! Returns current values of the client space of the object.
		inline void	FGAPI get_clip_rect(int& x,int& y,int& w,int& h)
		{
			x = xoff;
			y = yoff;
			w = wwrk;
			h = hwrk;
		}
		FGDrawBuffer(int ww=0, int hh=0, ObjectType=BMP_MEM, int color=CBLACK, FGPixel *buf=0);
		FGDrawBuffer(int ww, int hh, FGPixel *buf);
        //! Makes the deep copy of object.
		FGDrawBuffer(FGDrawBuffer &);

		virtual ~FGDrawBuffer();

		// copy block from one buffer to other (with ppop or alpha)
		// this is software blitter only.
		int			FGAPI bitblit(int xdst, int ydst, int xsrc, int ysrc, int ww, int hh, class FGDrawBuffer *src, int opcia=BITBLIT_COPY);
		int			FGAPI hline(int x, int y, int size, FGPixel color);
		int			FGAPI vline(int x, int y, int size, FGPixel color);

		//! Sets the start position for first time used lineto().
		void    	FGAPI moveto(int x, int y) { oldx = x; oldy = y; }
		int			FGAPI line(int x, int y, int x2, int y2, unsigned color = UNDEFINED_COLOR);
		int			FGAPI lineto(int x, int y, unsigned color = UNDEFINED_COLOR);
		int			FGAPI pline(int x1, int y1, int x2, int y2, FGPattern *p);

		void		FGAPI spline(FGPoint points[4], unsigned color = UNDEFINED_COLOR);

		int			FGAPI rect(int xdst, int ydst, int ww, int hh, unsigned color = UNDEFINED_COLOR);
		int			FGAPI box(int xdst, int ydst, int ww, int hh, unsigned color = UNDEFINED_COLOR);

		int 		FGAPI text(int x, int y, const char *txt, unsigned i = UNDEFINED_COLOR, unsigned p = UNDEFINED_COLOR);
#undef printf
		int			printf(int , int , const char * , ... );
		int			FGAPI putpixel(int x, int y, unsigned color = UNDEFINED_COLOR);
		/**
		 Returns the value of color at the position [x:y]
		 of the object or (-1) if coordinates are out of range.
		*/
		int			FGAPI getpixel(int x, int y)
		{
			if (x<0	|| y<0 || x>=wwrk || y>=hwrk)
				return 0xFFFFFFFF;
			return *(image+(x+xoff)+(y+yoff)*w);
		}
		void		FGAPI stretch(int _w, int _h);
		void		FGAPI stretch(double ratiox, double ratioy);

		int			FGAPI arc(int	x,	int	y,	double ang1, double ang2, int r, unsigned color = UNDEFINED_COLOR);
		int			FGAPI arc2(int	x,	int	y,	int ang1, int ang2, int r, unsigned color = UNDEFINED_COLOR);
		int			FGAPI ellipse(int x, int y, int rx, int ry, unsigned color = UNDEFINED_COLOR);
		int			FGAPI fellipse(int x, int y, int rx, int ry, unsigned color = UNDEFINED_COLOR);
		int			FGAPI circle(int	xs,	int	ys,	int	r, unsigned color = UNDEFINED_COLOR);
		int			FGAPI fcircle(int	xs,	int	ys,	int	r, unsigned color = UNDEFINED_COLOR);

		void 		FGAPI polygon(const FGPointArray& vertices, unsigned color = UNDEFINED_COLOR);
		FGRect		FGAPI fpolygon(const FGPointArray& vertices, unsigned color = UNDEFINED_COLOR);

		FGRect		FGAPI ftriangle(int x1, int y1, int x2, int y2, int x3, int y3, unsigned color = UNDEFINED_COLOR);

		//! Returns the address of pixel data storage. There are pixels, from right to left, line by line.
		FGPixel*    FGAPI GetArray(void) const { return image; }
		//! Returns an ALPHA (transparency) value of the object.
		unsigned	FGAPI GetAlpha(void) const { return state.alpha; }
		//! Returns a foreground color of the object.
		FGPixel	  	FGAPI GetInk(void)  const { return (FGPixel)state._ink; }
		//! Returns a background color of the object.
		FGPixel	  	FGAPI GetPaper(void)  const { return	(FGPixel)state._paper; }
		//! Returns a width of the object.
		int		  	FGAPI GetW(void)  const { return	w; }
		//! Returns a height of the object.
		int		  	FGAPI GetH(void)  const { return	h; }
		//! Returns the X coordinate of the client space of the object.
		int	   		FGAPI GetXW(void) const { return xoff; }
		//! Returns the Y coordinate of the client space of the object.
		int	   		FGAPI GetYW(void) const { return yoff; }
		//! Returns a client space width of the object.
		int			FGAPI GetWW(void) const {	return wwrk; }
		//! Returns a client space height of the object.
		int			FGAPI GetHW(void) const {	return hwrk; }
		//! Returns the type of the object (WINDOW, PUSHBUTTON ..)
		enum ObjectType 	FGAPI GetType(void)  const {	return type; }
		//! Returns the COLOR_KEY of the object (i.e. color that is used as transparent).
		unsigned	FGAPI GetColorKey(void) const { return state.colorkey; }
		//! Returns the name of the object.
		char*       FGAPI GetName(void)  const {	return name; }
		//! Set the COLOR_KEY of the object (i.e. color that will be transparent when bitblit
		//! operation will be performed with this object).
		void		FGAPI SetColorKey(unsigned ck)
		{
			state.colorkey = ck;
		}
		void	  	FGAPI SetName(const char *s);
		//! Set the level of transparency for the object when bitblit
		//! operation will be performed with this object. The level
		//! is number from 0 to 255, where 0 is completely transparent
		//! and 255 completely opaque.
		void		FGAPI SetAlpha(unsigned a) { state.alpha = a; }
		//! Clear the entire object image with this color.
		void		FGAPI clear(int color);
		/**
			Rotates image to apropriate direction.
		*/
		void		FGAPI rotate(ENUM_ROTATE_DIRECTION direction);
};

#define	SAVE_CLIPPING(THIS) int _a,_b,_c,_d; clip_lock.Lock(); _a = (THIS)->xoff; _b = (THIS)->yoff; _c = (THIS)->wwrk; _d = (THIS)->hwrk; (THIS)->xoff = 0; (THIS)->yoff = 0; (THIS)->wwrk = (THIS)->w; (THIS)->hwrk = (THIS)->h;
#define	SAVE_CLIPPING2(THIS) clip_lock.Lock(); _a = (THIS)->xoff; _b = (THIS)->yoff; _c = (THIS)->wwrk; _d = (THIS)->hwrk; (THIS)->xoff = 0; (THIS)->yoff = 0; (THIS)->wwrk = (THIS)->w; (THIS)->hwrk = (THIS)->h;
#define	RESTORE_CLIPPING(THIS)	(THIS)->xoff = _a; (THIS)->yoff = _b; (THIS)->wwrk = _c; (THIS)->hwrk = _d; clip_lock.Unlock();

#ifdef FG_NAMESPACE
}
#endif

#ifdef __BORLANDC__
#pragma option pop /* pop -a switch */
#pragma option pop /* pop -b */
#endif


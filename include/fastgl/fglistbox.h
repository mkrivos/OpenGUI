/*
	An virtual parent for FGListBox that implement all stuff
*/

#ifndef __LISTBOX__
#define __LISTBOX__

#include <vector>

#ifdef __BORLANDC__
#pragma option push -b
#pragma option push -a4
#endif

#ifdef FG_NAMESPACE
namespace fgl {
#endif

/**
	Abstract parent for all visual containers - i.e. LISTBOX-es.
	Don't use this class directly!
*/
class listboxEx : public FGControl
{
		static const int SLIDER_WIDTH = 18;

		void	corecture(void);
	protected:
		int			slider;			 	// for position of slider
		int			drop_down;			// size in visible lines
		unsigned	color_bg0, color_bg1, color_fg0, color_fg1; // colors
		int 		xx, yy,				// top left of widgets
					curr,				// current item or -1
					count,				// 0 ...
					fpol,				// first visible
					line,				// Y-position of line
					onew, oneh; 		// width x height of one item
		char tempname[8];
		XUIEnterHandlerInteger func;
		
		//! Try move cursor at new index and adjust all variables to new values.
		int		AdjustCursor(int item);
		//! Returns false if there aren't data int the object.
		int		Enabled(void) { if (curr<0 || count<=0) return 0; return 1;}

		listboxEx(int xs, int ys, int w, int h, int DropDown, FGWindow *wind, void* user_data, XUIEnterHandlerInteger func);
		~listboxEx();

		virtual void DrawItem(int , int , int , int ) {}
		virtual	void WheelSpin(int direction)
		{
			if (direction<0) Up();
			else if (direction>0) Down();
		}
		virtual	void ClickDown(int x, int y);
		virtual void OnFocus(void) { }
		virtual void OnLostFocus(void) { }

	public:
		virtual void draw(void);
		/**
		Sets the listbox's colors.
		@param a paper for normal item
		@param b ink for normal item
		@param c paper for selected item
		@param d ink for selected item
		*/
		void	SetColors(int a, int b, int c, int d)
		{
			color_bg0 = a;
			color_fg0 = b;
			color_bg1 = c;
			color_fg1 = d;
		}
		//! Redraw the whole object on the screen, the parameter 'i'
		//! is flag for cursor drawing (yes or no).
		//! @deprecated
//		void 	Draw(int i=1);	// for listbox, default with cursor
		//! Draw cursor at its position.
//		void 	ShowCursor(void);
		//! Hide cursor at its position.
//		void 	HideCursor(void);
		//! Move one line UP in the listbox.
		void 	Up(void);
		//! Move one line DOWN in the listbox.
		void 	Down(void);
		//! Move at the start of the listbox.
		void 	Begin(void);
		//! Move at the end (last line) of the listbox.
		void 	End(void);
		//! Set the new size of list box (i.e. number of items in it).
		void 	SetSize(int);
		//! Returns current size (number of lines).
		int 	GetSize(void) const { return count; };
		//! Set the current item in the listbox.
		void 	SetIndex(int);
		//! Returns the current item in the listbox.
		int 	GetIndex(void) const { return curr; };
		//! Set the current item relative to the current item in the listbox.
		void 	SetIndexRel(int);
		//! Set the new size of list box.
		void 	Resize(int dx, int dy);
		//! Redraw the current item/line in the listbox.
		void	RedrawItem(void)
		{
			draw();
		}
		int  	Test(int, int);
		int		DoListBox(int key);
		virtual const int ToInteger() const { return GetIndex(); }
};

/**
	Common abstract template for all LISTBOX based objects.
	For using you must instantiate this one with particular
	object type - it generates object of the container type (std::vector).
*/
template <typename T> class FGListBoxTemplateEx : public listboxEx
{
	protected:

		typedef typename std::vector<T>::iterator Iter;
		std::vector<T>		items;
		int					update;
		FGListBoxCallBack 	show;

		virtual void DrawItem(int x, int y, int index, int flag) { }
	public:
		/**
			Create the visual container of the type T.
		*/
		FGListBoxTemplateEx(
		//! x coordinate in thw window
		int xs,
		//! y coordinate in thw window
		int ys,
		//! visual width
		int w,
		//! visual height
		int h,
		//! number of lines
		int DropDown,
		//! parent window
		FGWindow *wind,
		//! callback procedure of type FGListBoxCallBack
		FGListBoxCallBack show_cb,
		//! your data ptr
		void* user_ptr,
		XUIEnterHandlerInteger func)
		: listboxEx(xs,ys,w,h,DropDown,wind,user_ptr, func)
		{
			update = 0;
			show = show_cb;
		}

		//! Disable visual update for of objects.
		void DisableUpdate(void) { update = 0; }
		//! Update uncoditionally visual of objects. This method also enable visual update.
		void Update(void)
		{
			update = 1;
			draw();
		}
		//! Erase the item at index 'i' from the container.
		void erase(int i)
		{
			items.erase(items.begin()+i);
			count--;
			if (update) draw();
		}
		//! Set null state.
		void clear(void)
		{
			count = fpol = line = curr = 0;
			draw();
			update = 0;
			items.clear();
		}
		//! Insert the item 's' to the container.
		void insert(T s)
		{
			items.push_back(s);
			count++;
			if (update) draw();
		}
		//! Insert the item 's' at the index 'i' to the container.
		void insert(int i, T s)
		{
			items.insert(items.begin()+i,s);
			count++;
			if (update) draw();
		}
		//! Replace the item 's' at the index 'i' in the container.
		void replace(int i, T s)
		{
			Iter ti = items.begin();
			ti[i] = T(s);
		}
		const T& item(int i) const { return items[i]; }
};

/**
	Structure for string encapsulation for FGListBox container
	@internal
*/
struct id_string
{
	char str[64];
	id_string()
	{
		*str=0;
	}
	id_string(const char *s)
	{
		strncpy(str,s,63);
		str[63]=0;
	}
};

/**
	A LISTBOX (visual container) of the ordinary "C" strings (up to 63 chars length).
	It is used for a FileDialog object to show files and directories.
*/
class FGListBoxEx : public FGListBoxTemplateEx<struct id_string>
{
		typedef std::vector<id_string>::iterator Iter;
		friend class FGWindow;
		
		/**
			Draws one item (at index) at position [x,y]
			if user_data is defined, only this pointer is passed without any indexing
		*/
		virtual void DrawItem(int x, int y, int index, int flag)
		{
			if (index>=count) return; // out of range
			Iter i=items.begin();

			if (user_data)
			{
				if (show)
					show(flag, owner, x, y, user_data, index);
			}
			else
			{
				if (show==0)
					owner->WindowText(x+4, y, i[index].str, flag?color_fg1:color_fg0,flag?color_bg1:color_bg0);
				else
					show(flag, owner, x, y, i[index].str, index);
			}
		}
		~FGListBoxEx() {}
	public:
		/**
		Creates and adds a LISTBOX container of type <char str[64]> with specified size to the FGWindow.
		@param xs x coordinate of object in the FGWindow
		@param ys y coordinate of object in the FGWindow
		@param w  a width of the one line (item)
		@param h  a height of the one line (item)
		@param DropDown a number of lines of the object
		@param wind a pointer to the parent FGWindow
		@param show_cb an optional callback to show an item (default is simply print string).
		@param user_data pointer to the user defined data
		*/
		FGListBoxEx(int xs, int ys, int w, int h, int DropDown, FGWindow *wind, FGListBoxCallBack show_cb, void* user_data, XUIEnterHandlerInteger func)
		 : FGListBoxTemplateEx<id_string>(xs, ys, w, h, DropDown, wind, show_cb, user_data, func)
		{
		}
		const char *GetString(int i) const { return item(i).str; }
		virtual const char* ToString() const { return GetString(GetIndex()); }
};

#ifdef __BORLANDC__
#pragma option pop /* pop -a switch */
#pragma option pop /* pop -b */
#endif

#ifdef FG_NAMESPACE
}
#endif

#endif


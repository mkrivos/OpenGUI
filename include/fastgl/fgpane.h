/**
  $Id: fgpane.h 2369 2005-06-17 09:36:09Z majo $

  $Log$
  Revision 1.2  2005/06/17 09:36:09  majo
  reenabled FGPaneProvider:wq

  Revision 1.1.1.1  2005/05/12 10:52:38  majo
  i

  Revision 1.4  2004/11/04 06:18:55  majo
  added 'bool' version of FGPointButton
  added FGRect(FGDrawConstructor*)
  removed CfgStore (instead of XML)

  Revision 1.3  2004/05/24 17:41:44  majo
  documentation

  Revision 1.2  2004/02/23 20:08:01  majo
  all classes are with prefix FG* on now
  polygon functions uses FGPointArray from now
  class GuiEvent is renamed to FGEvent
  some by parameters overloaded methods was removed (class FGWindow)
  many other small changes

  Revision 1.1.1.1  2004/01/22 16:25:08  majo
  initial release

*/

#ifndef _PANE_H_
#define _PANE_H_

#ifdef __BORLANDC__
#pragma option push -b
#pragma option push -a4
#endif

#ifdef FG_NAMESPACE
namespace fgl {
#endif

class CfgStore;
class StoreVectorStr;

/**
* The 'visualizer' class
* you can override frame() method
* @see FGBlock
*/
class FGBlockStyle
{
		friend class FGBlock;
		/**
		* Default 'facade' of FGBlock
		*/
		virtual void frame(FGWindow *wnd, int x, int y, int w, int h, char* name)
		{
			wnd->WindowBox(x+4,y+4,w-8,h-8,CGRAYED);
			wnd->AddPanel(x+8,y+8,w-16,h-16,name,CWHITE,CWHITED,CDARK);
		}
	public:
		FGBlockStyle() { }
		virtual ~FGBlockStyle() { }
};

static FGBlockStyle default_style;

//---------------------------------------------------------------------------

/**
* Top level class for GUI controls compounds
* You must inherit one in your own class and override method 'build'
*/
class FGBlock
{
	protected:
		int     width;
		int     height;
		char 	name[64];
		FGBlockStyle& style;
		void*   container;
		bool	fixed;

		void FGAPI Add(FGControl *);
	public:
		/**
            Creates new logical block (for FGControls).
		*/
		FGBlock(int w, int h, const char *n="", FGBlockStyle& s=default_style);
		virtual ~FGBlock();
		/**
		* Override this methods to define Controls contained into the block.
		*/
		virtual void build(FGWindow *parent, int x, int y) = 0;
		/**
		* Build block - don't call directly!
		*/
		void FGAPI show(FGWindow *parent, int x, int y)
		{
			style.frame(parent,x,y,width,height,name);
			build(parent,x,y);
		}

		void FGAPI Disable(void);
		void FGAPI Enable(void);

		/**
		* Returns height of block.
		*/
		int FGAPI GetHeight(void) { return height; }
		/**
		* Returns width of block.
		*/
		int FGAPI GetWidth(void) { return width; }
		/**
		* Returns symbolic name of block.
		*/
		char* FGAPI GetName(void) { return name; }
		/**
		* Sets as 'fixed'
		*/
		void FGAPI SetFixed(bool value) { fixed = value; }
};

//---------------------------------------------------------------------------

/**
* Container for blocks - used in other class - not for direct use!
*/
class FGPaneContainer
{
	protected:
		void *container;
		bool  destroy;
	public:
		FGPaneContainer();
		virtual ~FGPaneContainer();
};

//---------------------------------------------------------------------------

/**
* The Workspace where blocks of controls are shown
*/
class FGPane : public FGPaneContainer
{
	protected:
		FGWindow *parent;
	public:
		FGPane(FGWindow *w);
		virtual ~FGPane();

		void FGAPI Add(FGBlock* val, bool fixed=false);
		void FGAPI AddFrom(char * name);
		void FGAPI ShowHorizontal(int xx=0, int yy=0);
		void FGAPI ShowVertical(int xx=0, int yy=0);
};

//---------------------------------------------------------------------------

#if 1

/**
* The palette of all blocks created by program.
* This container for choosing blocks from and (explicit) common deallocator for
* the blocks on exit.
*/
class FGPaneProvider : public FGPaneContainer
{
		friend class FGBlock;
		friend class FGPane;

		static FGPaneProvider *self;
		static void FGBlock_Provider_DialogProc(FGEvent *p);
		static FGListBox *listboxPtr0;
		static FGListBox *listboxPtr1;
		FGWindow *FGBlock_Provider_DialogPtr;

//		CfgStore* cfg;

		static void AddToCfg(CallBack);
		static void RemoveFromCfg(CallBack);
		static void Up(CallBack);
		static void Down(CallBack cb);
		static void Ok(CallBack cb);
		static void Cancel(CallBack cb);
		static void FGAPI Register(FGBlock* block);

//		StoreVectorStr* FGAPI GetStore(char *);
		FGBlock* FGAPI FindBlockByName(const char s[]);

	protected:
		FGPaneProvider();
	public:
		~FGPaneProvider();
		/**
		* Returns FGPaneProvider object (implemented as 'Singleton')
		*/
		inline static FGPaneProvider* Self(void)
		{
			if (self == 0)
			{
				self = new FGPaneProvider;
			}
			return self;
		}
		void FGAPI ShowDialog(char *store_name);
};
#endif

#ifdef __BORLANDC__
#pragma option pop /* pop -a switch */
#pragma option pop /* pop -b */
#endif

#ifdef FG_NAMESPACE
}
#endif

#endif

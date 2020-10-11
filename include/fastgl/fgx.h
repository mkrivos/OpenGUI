#ifndef fgxH
#define fgxH

// -----------------------------------------------------------------------------

#include <list>

/**
	@defgroup fgx FGX dynamic GUI builder from XML file
	Instead of generating code from the XML interface description, FGX loads
	and parses the description at runtime. It also provides functions that can be
	used to connect signal handlers to parts of the interface.
	In this way, it allows you to separate your program code from the interface code.
	Of course, you would also add your own signal handlers to the code. Note that
	the signals are connected the same way as if you had hand coded the interface.
	There is no extra overhead to interfaces generated by FGX (after the initial
	generating of course, and this is not much of an overhead) when compared to
	a hand crafted interface.

	The file format that describes GUI is XML flavour and is called XUI. It looks like
	that:

	@code
	<?xml version="1.0" standalone="yes" ?>
	<xui>
		<widget id="id0" label="This is my Window!" w="400" h="600" foreground="000000" background="FFFFFF" handler="MyHandler" frame="0">
			<pushbutton id="id3" label="New Window" x="32" y="30" w="128" h="25" onclick="CreateWindow" hotkey="78" />
			<pushbutton id="id4" label="Close Window" x="32" y="70" w="128" h="25" onclick="CloseWindow" hotkey="67" />
			<pushbutton id="id5" label="Close Application" x="32" y="120" w="128" h="25" onclick="__CloseApplication" hotkey="65" selected="1" />
			<editbox id="id7" label="Text Entry :" x="120" y="220" w="128" h="25" onclick="MyTextEntry" hotkey="84" scrambled="1" />
			<editbox id="id8" label="Integer Entry :" x="120" y="250" w="128" h="25" onclick="MyIntegerEntry" hotkey="84" hexadecimal="1" />
			<editbox id="id9" label="Double Entry :" x="120" y="280" w="128" h="25" onclick="MyDoubleEntry" hotkey="84" />
			<checkbox id="id10" label="CheckBox" x="32" y="320" onclick="MyIntegerEntry" hotkey="67" />
			<slidebar id="id15" x="32" y="350" onclick="MyIntegerEntry" horizontal="1" step="10" minimum="-100" maximum="100" />
			<listbox id="id16" x="32" y="380" w="100" h="20" onclick="MyIntegerEntry" dropdown="5" />
			<radiogroup id="id12" x="200" y="20" onclick="MyIntegerEntry" span="40">
				<radiobutton label="prvy" x="200" y="20" hotkey="112" />
				<radiobutton label="druhy" x="200" y="60" hotkey="100" />
				<radiobutton label="treti" x="200" y="100" />
				<radiobutton label="stvrty" x="200" y="140" />
				<radiobutton label="piaty" x="200" y="180" />
			</radiogroup>
			<menubar>
				<menu id="id13" label="File" hotkey="70" popup="id13_1">
					<menupopup id="id13_1" w="200" h="74">
						<menuitem id="id13_1_1" label="Item 1" hotkey="1" />
						<menuitem id="id13_1_2" label="Item 2" hotkey="2" />
						<menuitem id="id13_1_3" label="Quit" onclick="__CloseApplication" hotkey="113" selected="1" />
					</menupopup>
				</menu>
				<menu id="id14" label="Edit" onclick="__CloseApplication" hotkey="69">
					<menupopup />
				</menu>
			</menubar>
		</widget>
		<widget id="id1" label="Yet another window" x="400" w="400" h="600" foreground="000000" background="FFFFFF" persistent="1" resize="1">
			<text label="A text label" x="32" y="32" foreground="000000" background="FFFFFF" />
		</widget>
	</xui>
	@endcode

	There are three ways to create XUI file:

	- by hand in your text editor
	- by hand with C++ code
	- visually with RAD tool (future)

	<h3>Syntax of XUI file</h3>

	Todo.

	<h3>Generating XUI file pragramatically</h3>

	Todo.

	<h3>Visual creating of XUI file</h3>

	Todo.
*/

class TiXmlDocument;
class TiXmlElement;

#ifdef __BORLANDC__
#pragma option push -b
#pragma option push -a4
#endif

#ifdef FG_NAMESPACE
namespace fgl {
#endif

class FGWindow;
class XUIComponent;

typedef std::list<XUIComponent *> ComponentContainer;
typedef ComponentContainer::iterator ComponentIterator;
typedef ComponentContainer::const_iterator CComponentIterator;

/**
	@ingroup fgx
*/
class XUIComponent
{
		friend class XUIBuilder;
		void init();

	public:
		static const int DEFAULT_VALUE = 0;

	protected:

		const char* id;
		const char* label;
		char		onclick[FGClosure::name_size];

	public:
		FGRect		shape;

		XUIComponent();
		XUIComponent(const char* _id, const char* _label, int x, int y, int w, int h);
		XUIComponent(const XUIComponent& old);
		virtual ~XUIComponent();

		virtual void AddComponent(XUIComponent *) { }
		virtual void RemoveComponent(void) { }
		virtual ComponentContainer* GetComponents() { return 0; }
		virtual int GetComponentCount() { return 0; }

		void SetId(const char* name);
		void SetLabel(const char* name);
		void SetSignalName(const char* name);

		const char* GetId() const { return id; }
		const char* GetLabel() const { return label; }
		const char* GetSignalName() const { return onclick; }

		virtual void Save(TiXmlElement* doc);
		virtual void Load(TiXmlElement* doc);
		bool LoadColor(TiXmlElement* doc, const char* name, FGColor& color);
		void SaveColor(TiXmlElement* doc, const char* name, FGColor& color);

		virtual XUIComponent* Clone() = 0;
		virtual void Show(XUIComponent* parent = 0) = 0;
		virtual bool SetData(const char data[], int size) { return false; }
		virtual bool SetData(const int data) { return false; }
		virtual bool SetData(const double data) { return false; }
		virtual bool Selected() { return false; }
		virtual FGWindow* GetParent() { return 0; }
};

/**
	@ingroup fgx
*/
class XUIComposite : public XUIComponent
{
		friend class XUIBuilder;
	protected:
		ComponentContainer 	childs;
	public:
		XUIComposite();
		XUIComposite(const char* _id, const char* _label, int x, int y, int w, int h);
		XUIComposite(const XUIComposite& old);
		virtual ~XUIComposite();

		virtual void Save(TiXmlElement* doc);
		virtual void Load(TiXmlElement* doc);

		virtual XUIComponent* Clone();
		virtual void Show(XUIComponent* parent = 0);
		virtual void AddComponent(XUIComponent *);
		virtual void RemoveComponent(void);
		virtual ComponentContainer* GetComponents() { return &childs; }
		virtual int GetComponentCount();

		XUIComponent* FindObject(const char* name);
};

/**
	Controls helper;
	@ingroup fgx
*/
class XUIControl : public XUIComponent
{
		void init();
	protected:
		FGControl*	control;
	public:
		int			selected;
		int			hotkey;
		int			disable;

		XUIControl();
		XUIControl(const char* _id, const char* _label, int key, int x, int y, int w, int h);
		XUIControl(const XUIControl& );
		virtual ~XUIControl() {}

		virtual void Save(TiXmlElement* doc);
		virtual void Load(TiXmlElement* doc);
		void FixControl(FGControl* ctrl);
		virtual bool Selected() { return selected; }
		FGControl* GetControl() { return control; }

};

// -----------------------------------------------------------------------------

/**
	@ingroup fgx
*/
class XUIText : public XUIComponent
{
		friend class XUIBuilder;
	protected:
		FGColor ink, paper;
	public:
		XUIText();
		XUIText(const char* _label, int x, int y, FGColor& ink, FGColor& paper);
		virtual ~XUIText();

		virtual void Save(TiXmlElement* doc);
		virtual void Load(TiXmlElement* doc);

		virtual XUIComponent* Clone();
		virtual void Show(XUIComponent* parent);
};

// -----------------------------------------------------------------------------

/**
	@ingroup fgx
*/
class XUIPushButton : public XUIControl
{
		friend class XUIBuilder;

	protected:
	public:
		XUIPushButton();
		XUIPushButton(const char* _id, const char* _label, int key=0,
			int x = 0,
			int y = 0,
			int w = 80,
			int h = 23);

		virtual void Save(TiXmlElement* doc);
		virtual void Load(TiXmlElement* doc);

		virtual XUIComponent* Clone();
		virtual void Show(XUIComponent* parent);
};

/**
	@ingroup fgx
*/
class XUIEditBox : public XUIControl
{
		friend class XUIBuilder;
		int inputtype;
	protected:
		void init();

		char 		text[FGEditBox::bufsize];
		double		high_precision;
		int			integer;
	public:
		int			scrambled;
		int			hexadecimal;

		enum { STRING = 1, DOUBLE, INT };

		XUIEditBox();
		XUIEditBox(const char* _id, const char* _label, int key = 0,
			int x = 8,
			int y = 8,
			int w = 64,
			int h = 0);

		virtual void Save(TiXmlElement* doc);
		virtual void Load(TiXmlElement* doc);

		virtual XUIComponent* Clone();
		virtual void Show(XUIComponent* parent);
		virtual bool SetData(const char data[], int size);
		virtual bool SetData(const int data);
		virtual bool SetData(const double data);
};

/**
	@ingroup fgx
*/
class XUICheckBox : public XUIControl
{
		friend class XUIBuilder;
		int			integer;

	public:

		XUICheckBox();
		XUICheckBox(const char* _id, const char* _label, int key, int x, int y);

		virtual void Save(TiXmlElement* doc);
		virtual void Load(TiXmlElement* doc);

		virtual XUIComponent* Clone();
		virtual void Show(XUIComponent* parent);
		virtual bool SetData(const int data);
};

/**
	@ingroup fgx
*/
class XUISlideBar : public XUIControl
{
		friend class XUIBuilder;
		int			minimum;
		int			maximum;
		int			step;
		int			value;
		int			horizontal;
	public:

		XUISlideBar();
		XUISlideBar(const char* _id, int x, int y, int min, int max, int step = 1);

		virtual void Save(TiXmlElement* doc);
		virtual void Load(TiXmlElement* doc);

		virtual XUIComponent* Clone();
		virtual void Show(XUIComponent* parent);
		virtual bool SetData(const int data);
};

/**
	@ingroup fgx
*/
class XUIRadioButton : public XUIControl
{
		friend class XUIBuilder;
		int			integer;
	protected:

	public:

		XUIRadioButton();
		virtual ~XUIRadioButton();
		XUIRadioButton(const char* _id, const char* _label, int key, int x, int y);

		virtual void Save(TiXmlElement* doc);
		virtual void Load(TiXmlElement* doc);

		virtual XUIComponent* Clone();
		virtual void Show(XUIComponent* parent);
		virtual bool SetData(const int data);
};

/**
	@ingroup fgx
*/
class XUIListBox : public XUIControl
{
		friend class XUIBuilder;
		int			dropdown;
	protected:
	public:
		XUIListBox();
		XUIListBox(const XUIListBox& old);
		virtual ~XUIListBox();
		XUIListBox(const char* _id, int x, int y, int h, int w, int drop);

		virtual void Save(TiXmlElement* doc);
		virtual void Load(TiXmlElement* doc);

		virtual XUIComponent* Clone();
		virtual void Show(XUIComponent* parent);
		virtual bool SetData(const char data[], int size);
		virtual bool SetData(const int data);
};

// -----------------------------------------------------------------------------

/**
	@ingroup fgx
*/
class XUIRadioGroup : public XUIComposite
{
		friend class XUIBuilder;

		char 			cbname[16];
		FGButtonGroup	group;
		long			value;
		int 			span;
		int				horizontal;

		static void callback(CallBack cb, void* data);

	public:
		XUIRadioGroup();
		XUIRadioGroup(const XUIRadioGroup& old);
		XUIRadioGroup(const char* _id, const char* _label[], int key[], int x, int y, int span = 20, bool horizont=false);
		virtual ~XUIRadioGroup();

		virtual void Save(TiXmlElement* doc);
		virtual void Load(TiXmlElement* doc);

		virtual XUIComponent* Clone();
		virtual void Show(XUIComponent* parent);
		virtual bool SetData(const int data);
};

// -----------------------------------------------------------------------------

/**
	@ingroup fgx
*/
class XUIMenuItem : public XUIControl
{
		friend class XUIBuilder;
	public:
		XUIMenuItem();
		XUIMenuItem(const XUIMenuItem&);
		XUIMenuItem(const char* _id, const char* _label, int key);

		virtual void Save(TiXmlElement* doc);
		virtual void Load(TiXmlElement* doc);

		virtual XUIComponent* Clone();
		virtual void Show(XUIComponent* parent);
};


/**
	PULLDOWN menu object with menu items.
	It is implemented as separate FGMenuWindow instance contains MenuItems.
	@ingroup fgx
*/
class XUIPopupMenu : public XUIComposite
{
		friend class XUIBuilder;
		FGMenuWindow*			parent;
	public:

		XUIPopupMenu();
		XUIPopupMenu(const XUIPopupMenu& old);
		XUIPopupMenu(const char* _id, int x=0, int y=0);
		virtual ~XUIPopupMenu();

		virtual void Save(TiXmlElement* doc);
		virtual void Load(TiXmlElement* doc);

		virtual XUIComponent* Clone();
		virtual void Show(XUIComponent* parent);
		FGWindow* GetParent() { return parent; }
};

/**
	@ingroup fgx
*/
class XUIMenu : public XUIControl
{
		friend class XUIBuilder;
		XUIComponent*	popup;				// if this memeber is valid, then it is concrete submenu
	public:
		XUIMenu();
		XUIMenu(const XUIMenu&);
		XUIMenu(const char* _id, const char* _label, int key);
		virtual ~XUIMenu();

		virtual void Save(TiXmlElement* doc);
		virtual void Load(TiXmlElement* doc);

		virtual void AddComponent(XUIComponent *);
		virtual XUIComponent* Clone();
		virtual void Show(XUIComponent* parent);

		static void PullDownMenuActivator(CallBack cb, void *);
};

/**
	@ingroup fgx
*/
class XUIMenuBar : public XUIComposite
{
		friend class XUIBuilder;

		static void callback(CallBack cb, void* data);

	public:
		XUIMenuBar();
		XUIMenuBar(const char* _id);
		virtual ~XUIMenuBar();

		virtual void Save(TiXmlElement* doc);
		virtual void Load(TiXmlElement* doc);

		virtual XUIComponent* Clone();
		virtual void Show(XUIComponent* parent);
};

/**
	@ingroup fgx
*/
class XUIWindow : public XUIComposite
{
		friend class XUIBuilder;

		FGWindow*			parent;
		char				handler[FGClosure::name_size];

		void init();
		void LoadChilds(TiXmlElement* doc);
		static void GlobalHandler(FGEvent *);

	public:

		FGColor		 	foreground;
		FGColor		 	background;
		int			 	title;
		int				frame;
		int				modal;
		int				close_on_escape;
		int				center;
		int				persistent;
		int				resize;
		int				statusbar;
		int				glcontext;
		int				sticky;
		int				focus;
		int				pictograms;
		int				selectmode;
		int				withmenu;

		XUIWindow(const char* _id, const char* _label, int x, int y, int w, int h);
		XUIWindow();
		XUIWindow(const XUIWindow& old);
		virtual ~XUIWindow();

		virtual void Save(TiXmlElement* doc);
		virtual void Load(TiXmlElement* doc);

		virtual XUIComponent* Clone() { return 0; };
		virtual void Show(XUIComponent* parent);

		void SetHandlerName(const char* name);
		FGWindow* GetParent() { return parent; }
};

// -----------------------------------------------------------------------------

/**
	This class is intended for creating, loading, saving and building of XUI files.
	It is container of top level widgets (FGWindow).
	All objects, widgets (aka Window) and its childrens (aka buttons) are identified
	by ID (const char*) only.
	@note Each object in the system *MUST* have unique ID. 

	Your basic FGX program will look something like this:
	@code
	The file: 'some_widgets.xui'

	<?xml version="1.0" standalone="yes" ?>
	<xui>
		<widget id="id1" label="Yet another window" x="400" w="400" h="600" persistent="1" resize="1">
			<text label="A text label" x="32" y="32" foreground="000000" background="FFFFFF" />
		</widget>
	</xui>

	int main(int argc, char** argv)
	{
		FGApp app(2, argc, argv, APP_ALL);
		XUIBuilder ui;

		if (ui.LoadGUI("some_widgets.xui"))
		{
			ui.ConnectSignal("CloseApp", __CloseApplication);
			ui.Show("MyWindow");
			app.Run();
		}
		return 0;
	}
	@endcode

	@ingroup fgx
*/
class XUIBuilder
{
	typedef std::list<XUIWindow> WidgetList;
	typedef WidgetList::iterator WidgetIterator;

	protected:
		WidgetList	widgets;

		static void CloseApplication(CallBack cb, void*)
		{
			FGControl::Quit(cb);
		}
		static void CloseWindow(CallBack cb, void*)
		{
			FGControl::Close(cb);
		}
	public:
		XUIBuilder();

		bool LoadGUI(const char* fname, const char* widget = 0);
		bool SaveGUI(const char* fname);
		void Show(const char* widget = 0);
		FGWindow* GetWidget(const char* id);
		XUIComponent* FindObject(const char* widget_id, const char* component_id);

		void AddWidget(const XUIWindow& widget);
		bool DeleteWidget(const char* id);

		bool SetData(const char* widget_id, const char* component_id, const char data[], int size);
		bool SetData(const char* widget_id, const char* component_id, const int);
		bool SetData(const char* widget_id, const char* component_id, const double);
};

// $Id: fgx.h 2086 2005-05-12 10:52:34Z majo $
//---------------------------------------------------------------------------

#ifdef FG_NAMESPACE
}
#endif

#ifdef __BORLANDC__
#pragma option pop /* pop -a switch */
#pragma option pop /* pop -b */
#endif

#endif
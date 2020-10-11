#ifdef __BORLANDC__
#pragma option push -b
#pragma option push -a4
#endif

#ifdef FG_NAMESPACE
namespace fgl {
#endif

/**
Provides simply and easy theme like object.
The library creates & initializes an instance of this structure on the startup
and uses it for a default settings for Controls and Windows that are created then.
*/
struct FGColorScheme
{
	/** a default FGWindow background color */
	int		window_back;
	/** a default FGWindow foreground color */
	int		window_fore;
	/** an active FGWindow title color */
	int		active_title;
	/** an inactive FGWindow title color */
	int		inactive_title;
	/** a default FGWindow shaded frame color 1 */
	int		wnd_bord1;
	/** a default FGWindow shaded frame color 2 */
	int		wnd_bord2;
	/** a default FGWindow shaded frame color 3 */
	int		wnd_bord3;
	/** a default FGWindow statusbar color */
	int		statusbar;
	/** a default base menu background color */
	int		menu_back;
	/** a default base menu foreground color */
	int		menu_fore;
	/** a default base menu background color when active */
	int		menu_back_active;
	/** a default base menu foreground color when active */
	int		menu_fore_active;
	/** a default push button foreground color */
	int		button_fore;
	/** a default push button background color */
	int		button_back;
	/** a default push button foreground color when pushed */
	int		button_fore_pushed;
	/** a default push button background color when pushed */
	int		button_back_pushed;
	/** a default push button border 1 */
	int		button_bord1;
	/** a default push button border 2 */
	int		button_bord2;
	/** a default push button border 3 */
	int		button_bord3;
	/** a default edit box background color if not activated */
	int		edit_background_active;
	/** a default edit box foreground color */
	int		edit_foreground;
	/** a default edit box border 1 */
	int		edit_border;
	/** a default edit box border 2 */
	int		edit_background;
	/** a default slider color */
	int		slider;
	/** a default pulldown menu background color */
	int		menuwindow_back;
	/** a default pulldown menu foreground color */
	int		menuwindow_fore;
	/** a default pulldown menu frame color */
	int		menuwindow_frame;
	/** a default pulldown menu item background color */
	int		pdmenu_back_active;
	/** a default pulldown menu item foreground color */
	int		pdmenu_fore_active;
	/** a default pulldown menu item color when disabled */
	int		pdmenu_gray;
	/** a default pulldown menu frame color */
	int		pdmenu_color_of_frame;
	/** a default edit box color when disabled */
	int		edit_fore_disable;
	/** a default notebook color (current item)*/
	int		notebook_active;
	/** a default slider color when disabled */
	int		slider_disable;
  public:
	FGColorScheme();
};

// subbitmap

extern FGColorScheme * CScheme;

#ifdef FG_NAMESPACE
}
#endif

#ifdef __BORLANDC__
#pragma option pop /* pop -a switch */
#pragma option pop /* pop -b */
#endif


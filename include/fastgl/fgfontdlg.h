#ifdef __BORLANDC__
#pragma option push -b
#pragma option push -a4
#endif

const unsigned FONT_DIALOG_FIXED = 1;
const unsigned FONT_DIALOG_VARIABLE = 2;
const unsigned FONT_DIALOG_ALL = (FONT_DIALOG_FIXED | FONT_DIALOG_VARIABLE);

/**
 A nice widget intended for the font selection by user for various things.

 @image html fntdlg.png

*/
class FGFontDialog : public FGConnector
{
		FGWindow 	*wnd;
		FontDialogCallBack userfnc;
		int		curr;
		FGListBox *lb;
		static void SetFontProc(FGEvent *p);
		void ShowFont(void);
	public:
		/**
		Creates a widget for easy and visual color selecting.
		@param f the callback function, it is called with selected color
		@param which you can show to select only fixed, or only proportional or both fonts.
		Predefined consts are: FONT_DIALOG_FIXED, FONT_DIALOG_VARIABLE and FONT_DIALOG_ALL
		@param capture a FGWindow caption
		@param bgc the background color of widget
		*/
		FGFontDialog(FontDialogCallBack f, int which=FONT_DIALOG_ALL, char *capture="Font selection dialog", FGPixel bgc=PM);
		virtual ~FGFontDialog()
		{
		}
};

#ifdef __BORLANDC__
#pragma option pop /* pop -a switch */
#pragma option pop /* pop -b */
#endif


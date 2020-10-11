#ifdef __BORLANDC__
#pragma option push -b
#pragma option push -a4
#endif

/**
An andvanced widget to edit the plain text files or create the new one.
It has a traditional Open, Save & Close menu and simply search engine.

@image html tedit.png

*/
class FGTextEditor : public FGConnector
{
	public:
		enum 	{ FGMAX_LINE=32768, FGMAX_LINESIZE=512 };
	private:
		class FGLINE
		{
			public:
				unsigned size;         		/* amount of memory allocated */
				unsigned char *text;         /* the characters */
		};

		class FGBUFFER
		{
			public:
			char name[256];              /* name of the file */
			char buffer[FGMAX_LINESIZE];   /* name of the file */
			int  posx, posy;
			int  line;      		        /* cursor position within the line */
			int  lines;                  // all lines
			int  flags;                  /* flags about the buffer state */
			int  top;                    /* top line on the screen */
			int  ovr;
			int  hscroll;                /* offset when screen is scrolled sideways */
			int  sel_line1;               /* line that the selection began at */
			int  sel_pos1;                /* selection offset within the line */
			int  sel_line2;               /* offset in lines from sel_line to cursor */
			int  sel_pos2;                /* selection offset within the line */
			FGBUFFER()
			{
				memset(this,0,sizeof(*this));
			}
		};

	protected:
		FGWindow *Text_EditorPtr;
		FGWindow *OptionsPtr;
		FGWindow *String_SearchPtr;
		int lfonly, font_save;
		FGBUFFER buf;
		FGLINE text[FGMAX_LINE];
		char s[1028];
		int WX, WY;
		char srch_str[33];
		int last_found;
		int _font;
		int TABSIZE;
		int nodraw,inblock,ronly;
		int X, Y, W, H;
		FGTextEditor **self;
//
		void FGAPI file(void);
		int  FGAPI _memcmp(unsigned char *from, unsigned char *co, unsigned kolko, unsigned l);
		int  FGAPI Find(int from, int count, int size);
		void FGAPI Clear(void);
		void FGAPI _goto(void);
		static void SearchProc(FGEvent *p);
		void FGAPI search(void);
		static void SetFont(CallBack);
		static void OptionsProc(FGEvent *p);
		static void file_proc(CallBack);
		void FGAPI options(void);
		static char* FGAPI terminate(char *s);
		FGLINE* FGAPI create_line(char *s, int at);
		void FGAPI Init(void);
		void FGAPI SaveAsBuffer2(void);
		void FGAPI OpenBuffer2(void);
		static void OpenBuffer1(char *s, FGFileDialog *);
		static void SaveAsBuffer1(char *s, FGFileDialog *);
		void FGAPI ShowCursor(int trigger);
		void FGAPI ShowLine(int scr, char *s);
		int  FGAPI isempty(char *s);
		void FGAPI CKey(int k);
		void FGAPI Key(int k);
		void FGAPI SaveChanges(void);
		static void Text_EditorProc(FGEvent *p);
		void FGAPI Open(char *s);
		void FGAPI Destruct(void);
	public:
		FGTextEditor(FGTextEditor **e=0, char *arg="Untitled", int font=FONT0816, int ink=CGRAY3, int paper=CBLACK, int flags = WSTANDARD|WMENU|WSTATUSBAR|WSIZEABLE|WNOTIFY|WMINIMIZE|WUSELAST|WNOPICTO);
		virtual ~FGTextEditor();
		void FGAPI ShowBuffer(int top=0);
		void FGAPI NewBuffer(void);
		void FGAPI OpenBuffer(char *s);
		void FGAPI ReopenBuffer(void);
		void FGAPI SaveBuffer(void);
		void FGAPI SaveAsBuffer(char *s);
		void FGAPI Close(void);
		void FGAPI Goto(int);
		/**
		Don't allow any changes on the file. View mode only.
		*/
		void FGAPI ReadOnlyMode(void) { ronly = 1; Text_EditorPtr->ResetChange(); };
		/**
		Move the editor at the top of all opened windows.
		*/
		void FGAPI SetFocus(void) { Text_EditorPtr->WindowFocus(); }
		/**
		Append this line at the end of text buffer.
		*/
		int  FGAPI AppendLine(char *str);
};

#ifdef __BORLANDC__
#pragma option pop /* pop -a switch */
#pragma option pop /* pop -b */
#endif


#ifdef __BORLANDC__
#pragma option push -b
#pragma option push -a4
#endif

const unsigned FDIALOG_OPEN	= 1;
const unsigned FDIALOG_SAVE	= 2;
const unsigned FDIALOG_MODAL = 4;
const unsigned FDIALOG_SAVEDIR = 8;
const unsigned FDIALOG_MULTISELECT = 16;

/**
	Internal helper structure for FGFileDialog
	@internal
*/
struct mydirent
{
	struct dirent item;
	bool		type;	// 0 for dir, 1 for file
	bool        selected;
	mydirent() { memset(this, 0, sizeof(*this)); }
};

/**
 A nice widget intended for the file selection by user for various things.
 filename length max. 511 chars
 max. visible is ~30 chars

 @image html fdialog.png

*/
class FGFileDialog : public FGConnector
{
	public:
		enum    { name_width=200, max_pathlen=512 };
	private:
		char	path[max_pathlen];
		char	tmppath[max_pathlen];
		char 	filename[max_pathlen];
		const char	*filter;
		int 	mode;
		int		files,fls,dirs;
		int		maxfiles;
		FileDialogCallBack fileselect2;
		void	reload(void);
		FGWindow	*wnd;
		FGEditBox *nameEBox;
		struct 	mydirent *filebuffer, *unused;
		int		param;
		FGListBox *list;
		FGListBox *list2;
		FGPushButton *ok, *cancel, *mkdir, *home, *updir;
		int		mdir;
//		int     num_of_selected;

		int		Selected();
		void	Refresh(const char *);
		bool    SelectFile(void);
		void 	_init(const char *dir, const char *flt, const char *namewnd, int m, int ink, int paper);

		static 	void InputName(CallBack);
		static 	void Mkdir(CallBack);
		static 	void Ok(CallBack);
		static 	void Updir(CallBack);
		static 	void Home(CallBack);
		static  void myproc(FGEvent *p);
		static  void drawone(int flag, FGWindow *wnd,int x,int y,void *str,int index);
		static  void drawone2(int flag, FGWindow *wnd,int x,int y,void *str,int index);
		static  char delimiter[2];
	protected:
	public:
		/**
		Creates visual object for the file selection by user.
		@param filesel your callback, it is called when you enter or click a filename
		@param dir the start directory, default is current app's directory or last used (with flag FDIALOG_SAVEDIR)
		@param flt text string that contains file extension patterns separated by space, ".exe .com" by example
		@param namewnd the caption
		@param m mode of operation (SAVE or OPEN). See to SetMode()
		@param ink the FGWindow foreground color
		@param paper the FGWindow background color
		*/
		FGFileDialog(FileDialogCallBack filesel, const char *dir=0, const char *flt=0, const char *namewnd="File Dialog", int m=FDIALOG_OPEN, int ink=0, int paper=PM);
		virtual ~FGFileDialog()
		{
			delete [] filebuffer;
			filebuffer = 0;
			if (wnd) /* if (wnd->GetStatus() & WEXIST) */ delete wnd;
		}
		/**
		Returns the current browsed directory.
		*/
		char*   FGAPI GetDir(void) { return path; }
		//! this filename is passed to the callback directly.
		char*   FGAPI GetName(void) { return filename; }
		/**
		* Makes full path && name string into user defined buffer
		*/
		void FGAPI BuildFullPathname(char *buffer);
		/**
		* Makes relative path && name string into user defined buffer
		*/
		void FGAPI BuildRelativePathname(char *buffer);
		/**
		Change the current directory and reload it.
		*/
		void	FGAPI SetDir(char *d) { strcpy(path, d); reload(); }
		/**
		Change the current input filter mask (".cpp .h" by example) and reload it.
		*/
		void	FGAPI SetFilter(char *f) { filter=f; reload(); }
		/**
		Sets object's mode:
		-	FDIALOG_OPEN
		-	FDIALOG_SAVE
		-	FDIALOG_MODAL
		-	FDIALOG_SAVEDIR
		*/
		void	FGAPI SetMode(int m) { mode=m; }
		/**
		* Shows dialog to ask to overwrite the selected file.
		* Is it possible to call this function from user callback (on file select)
		* @return true if file can be replaced.
		*/
		bool TestForOverwrite(void);
		FGWindow* GetWindow() const { return wnd; }
};

/**
	Simplified wrapper to FGFileDialog widget (without callback).
	It runs dialog in synchronous way.
	@see FGFileDialog
*/
class FileDlgWrapper
{
		static char tmpfilename[FGFileDialog::max_pathlen];
		static void filesel(char* file, FGFileDialog* fd)
		{
			strcpy(tmpfilename, file);
		}
	public:
		/**
			@return true if file was choosen
		*/
		bool ChooseFileName(char *dir=0, char *flt=0, char *namewnd="File Dialog", int m=FDIALOG_OPEN, int ink=0, int paper=PM)
		{
			FGFileDialog* fd = new FGFileDialog(filesel, dir, flt, namewnd, m, ink, paper);
			tmpfilename[0] = 0;
			fd->GetWindow()->ShowModal();
			if (tmpfilename[0])
			{
				return true;
			}
			return false;
		}
		/**
			Returns file if choosen, else ptr to '\0'
		*/
		const char* GetFileName() const { return tmpfilename; }
};


#ifdef __BORLANDC__
#pragma option pop /* pop -a switch */
#pragma option pop /* pop -b */
#endif


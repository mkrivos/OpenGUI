/*
*/

#define MODELIST_SIZE		256
#define FG_LINEAR			1
#define CAPABLE_LINEAR		16

#ifdef FG_NAMESPACE
namespace fgl {
#endif

/**
	The class describes physical screen layout.
*/
class FGScreen : public FGDrawBuffer
{
	public:
		FGScreen() : FGDrawBuffer(0, 0, FGFRAMEBUFFER)
		{
			assert(image==0);
		}
		void Resize(int xx, int yy)
		{
			wwrk = w = xx;
			hwrk = h = yy;
			X_width = w;
			Y_width = h;
			fgl::set_clip_rect(w, h, 0, 0);
			set_font(FONT0816);
			set_colors(0, CWHITE);
			set_ppop(_GSET);
			image = videobase;
		}
};

union gii_event;
/**
	Base interface between the graphics layer and hardware.
*/
class FGDriver
{
		friend class FGDGADriver;
		friend void _giiEvQueueAdd(gii_event *ev);
		friend class FGApp;
		friend int graph_set_mode(int mode);

		static int sort(const void *a, const void *b)
		{
			int val1 = ((vmode *)a)->w;
			int val2 = ((vmode *)b)->w;
			int rc   = val2 - val1;
			if (rc==0)
				rc = ((vmode *)b)->h - ((vmode *)a)->h;
			if (rc==0)
				rc = ((vmode *)b)->refresh - ((vmode *)a)->refresh;
			return rc;
		}
		FGDrawBuffer *back_store;
	protected:
		/**
			Describes a video mode;
		*/
		struct vmode
		{
			short w,h,mode;
			char flag, refresh;
		};

		char	*name, *subname;
		int		accel_hint;		// suggested accel type
		int		total_modes;
		bool	ismouse, iskeyboard, isgraph,
				islinear, mapped, verbose, ps2_mouse;
		int		w,h,				// logical size of screen
				virt_w, virt_h,     // real size of screen
				x_offset, y_offset; // offset of top-left corner
		int		req_w, req_h,       // requested size
				pitch,				// in bytes !!!
				mode_num,			// index to table mode !!!
				bpp;				// bytes per pixel
		unsigned long synch;   		// monitor vertical freq.
		int		current_buffer,		// current buffer index
				bufnum;				// num. of buffers, 0 if disabled
		void 	*linear_base,		// aperture addr.
				*mmio_base;
		unsigned long linear_size,	// aperture size
				mmio_size;
		unsigned char *mmio;		// virtualna adresa pre MMIO
		int		driver;
		FGPixel	*videobase;	  		// virtualna adresa pre VRAM
		FGPixel *buffer[4];			// max quad-buffers
		vmode	modelist[MODELIST_SIZE];
		char	*accel_name;
		unsigned	ops;	// 0 fillrect
		int		root;
		int 	msefd;
		int		oldx, oldy;
		int		mousex, mousey, but;
		char	*mouse_string;
		static bool is_ctrl, is_alt, is_shift;

		void vram_line(int x, int y, int w, int h, FGPixel ink, unsigned ppop);
		virtual int	find(int& ww, int& hh);
		int	postinit();
		int detect(void);
		int preinit(void);
		virtual int text_mode(void)
		{
			isgraph = 0;
			return true;
		}
		virtual int set_mouse(void)
		{
			if (verbose) printf("DRIVER %s: set_mouse() called\n", name);
			ismouse = 1;
			return true;
		}
		virtual void reset_mouse(void)
		{
			if (verbose) printf("DRIVER %s: reset_mouse() called\n", name);
			ismouse = 0;
		}
		virtual int set_keyboard(void)
		{
			if (verbose) printf("DRIVER %s: set_keyboard() called\n", name);
			iskeyboard = 1;
			return true;
		}
		virtual void reset_keyboard(void)
		{
			if (verbose) printf("DRIVER %s: reset_keyboard() called\n", name);
			iskeyboard = 0;
		}
		virtual void get_all_modes(vmode *) // default
		{
		}
		int	build_modelist(void)
		{
			total_modes = 0;
			get_all_modes(modelist);
			if ( total_modes == 0 )
			{
				printf("No video modes available for %d BPP\n", FASTGL_BPP);
				return 0;
			}
			// sort modes via size from bigger to smaller
			if (total_modes>1) qsort(modelist, total_modes, sizeof(vmode), sort);
			if (verbose)
            {
            	printf("\nVideo modes:\n");
            	for(int i=0; i<total_modes; i++)
                {
					if (modelist[i].w) printf("\tmode \"%dx%d-%d@%d\" available as %s\n", modelist[i].w, modelist[i].h, bpp*8, modelist[i].refresh ,modelist[i].flag&FG_LINEAR?"linear":"banked");
				}
			}
			return total_modes;
		}
		virtual int	available(void)
		{
			if (verbose) printf("DRIVER %s: available() called\n", name);
			return false;
		}
		virtual int	link(void)
		{
			if (verbose) printf("DRIVER %s: link() called\n", name);
			return false;
		}
	public:
		int		offset;
		FGDriver(char *nm);
		virtual ~FGDriver()
		{
			if (verbose) printf("DRIVER %s: unload() called\n", name);
		}
		int launch(int ww, int hh, int syn, int verb);
		void message(void);
		virtual int get_event(int& , long& , int& , int& , int& )
		{
			return false;
		}
		virtual int set_mode(int w, int h)
		{
			if (isgraph)
			{
				back_store = new FGDrawBuffer(w, h);
				get_block(0,0, w, h, back_store->GetArray());
			}
			return true;
		}
		virtual void change_mouse(void)
		{
			if (verbose) printf("DRIVER %s: change_mouse() called\n", name);
		}
		virtual void shutdown(void)
		{
			if (ismouse) reset_mouse();
			if (iskeyboard) reset_keyboard();
			text_mode();
		}
		void	SetVerbose(int v) { verbose = !!v; }
		void	SetFrequency(int v) { synch = v; }

		virtual void SetCaption(char *)
		{
		}

		virtual int EnableBuffering(int mode);
		virtual void DisableBuffering(void)
		{
			set_page(current_buffer=0);
			fgl::videobase = videobase = buffer[0];
			offset = 0;
		}

		virtual int set_page(int)
		{
			return 0;
		}
		virtual int Flip(void);
		virtual int set_virtual_size(int , int )
		{
			return 0;			  // default is disabled
		}
		virtual int set_virtual_pos(int , int )
		{
			return 0;			  // default is disabled
		}
		int GetBuffersNum(void)
		{
			return bufnum;
		}
#ifdef INDEX_COLORS
		void ReservedColors(void);
#endif
		int GetW(void) const { return w; }
		int GetH(void) const { return h; }
		virtual void UpdateRect(int x, int y, int xm, int ym, int w, int h) { }
		int GetPriv(void) const { return root; }

		static void SetShiftState(bool pressed) { is_shift = pressed; }
		static void SetControlState(bool pressed) { is_ctrl = pressed; }
		static void SetAltState(bool pressed) { is_alt = pressed; }
};

#ifdef FG_NAMESPACE
}
#endif


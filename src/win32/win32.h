// ---------------------------------------------------------------------------
//
//		Windows - DirectDraw frontend driver
//

// ---------------------------------------------------------------------------

#ifdef _WIN32
#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <ddraw.h>
#include "dll.h"

#define WM_MBUTTONDBLCLK                0x0209
#define WM_MOUSEWHEEL                   0x020A
#define WM_XBUTTONDOWN                  0x020B
#define WM_XBUTTONUP                    0x020C
#define WM_XBUTTONDBLCLK                0x020D

/*============================================================================
 *
 * DirectDraw Structures
 *
 * Various structures used to invoke DirectDraw.
 *
 *==========================================================================*/

#ifdef FG_NAMESPACE
namespace fgl {
#endif


class FGWin32DXDriver : public FGDriver
{
#ifndef _MSC_VER
  typedef HRESULT WINAPI (*tDrawCreate)(GUID FAR *lpGUID, LPDIRECTDRAW FAR *lplpDD, IUnknown FAR *pUnkOuter );
#else
  typedef HRESULT (WINAPI * tDrawCreate)(GUID FAR *lpGUID, LPDIRECTDRAW FAR *lplpDD, IUnknown FAR *pUnkOuter );
#endif
		DDSURFACEDESC       desc;
#ifndef _MSC_VER
		_DDCAPS_DX5
#else
		DDCAPS
#endif
			ddscaps;
		tDrawCreate			dDrawCreate;
		Dll					*DDrawDLL;
		LPDIRECTDRAW2		ddraw2;
		LPDIRECTDRAWCLIPPER g_lpClipper;  // the windowed mode clipper
		bool 				g_bExclusive; // (0) current mode -- windowed or fullscreen

		int					DX5_Load(void);
		void				DX5_Unload(void);
		int					DX5_UpdateVideoInfo(void);

		bool CreateDirectDraw(void);
		void DestroyDirectDraw(void);
		void CreateSurfaces(bool bExclusive, int nWidth, int nHeight, int nBPP);
		void DestroySurfaces(void);
		void CreateMainWindow(void);
		void DestroyMainWindow(void);
		RECT CalculateWindowRect(HWND hWindow, SIZE szDesiredClient);
		void AdjustMainWindow(bool bExclusive, int nScreenWidth, int nScreenHeight);
		void SwitchMode(bool bExclusive, int nWidth, int nHeight, int nBPP);
		void ToggleScreenMode(void);
		inline BOOL IsKeyDown      (UINT vk) { return (GetAsyncKeyState(vk) & 0x8000); }
		
	protected:
		static HWND         hwnd;
		static HINSTANCE	hInstance;			// Instance handle
		char *classname;

		static int win_cursor(int,int,int a,int&)
		{
			return !a;
		}
		static HRESULT WINAPI EnumModes2(DDSURFACEDESC *desc, VOID *udata);
		static LPDIRECTDRAWPALETTE palette;
		virtual void get_all_modes(vmode *) // default
		{
			int result;
			CreateDirectDraw();
			if(FAILED(result=ddraw2->EnumDisplayModes(0, NULL, this,EnumModes2)))
				SetDDerror("EnumDisplayModes", result);
			DestroyDirectDraw();
		}
		static void fillbox(int x, int y, int w, int h, FGPixel ink, unsigned ppop)
		{
			LPDIRECTDRAWSURFACE3 dst_surface;
			RECT area;
			DDBLTFX bltfx;
			HRESULT result;
			if (ppop) // fallback
			{
				__fill_box(x,y,w,h,ink,ppop);
				return;
			}
#ifdef DDRAW_DEBUG
			fprintf(stderr, "HW accelerated fill at (%d,%d)\n", dstrect->x, dstrect->y);
#endif
			area.top = y;
			area.bottom = y+h;
			area.left = x;
			area.right = x+w;
			bltfx.dwSize = sizeof(bltfx);
			bltfx.dwFillColor = ink;
			result = IDirectDrawSurface3_Blt(FGWin32DXDriver::current,
				&area, NULL, NULL, DDBLT_COLORFILL|DDBLT_WAIT, &bltfx);
			if ( result == DDERR_SURFACELOST )
			{
				IDirectDrawSurface3_Restore(FGWin32DXDriver::primary);
				result = IDirectDrawSurface3_Blt(FGWin32DXDriver::current,
					&area, NULL, NULL, DDBLT_COLORFILL|DDBLT_WAIT, &bltfx);
			}
			if ( result != DD_OK )
			{
				FGWin32DXDriver::SetDDerror("IDirectDrawSurface3::Blt", result);
			}
		}
	public:
		static LPDIRECTDRAWSURFACE primary;
		static LPDIRECTDRAWSURFACE secondary;
		static LPDIRECTDRAWSURFACE current;

		static 				void SetDDerror(const char *function, int code);
		static 				void SetDIerror(const char *function, int code);
		static long CALLBACK HandleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
		FGWin32DXDriver(char *name, bool mode);
		~FGWin32DXDriver();
		virtual int get_event(int& type, int& key, int& x, int& y, int& buttons);
		virtual int available(void)
		{
			return 1;
		}
		virtual int set_mouse(void)
		{
			mouse_string = "WINDOWS_MOUSE";
			return TRUE;
		}
		virtual void change_mouse(void)
		{
		}
		virtual void reset_mouse(void)
		{
			ismouse = 0;
		}
		virtual int set_keyboard(void)
		{
			return TRUE;
		}
		virtual void reset_keyboard(void)
		{
			iskeyboard = 0;
		}
		virtual int text_mode(void)
		{
			return TRUE;
		}
		virtual int	link(void);
		virtual int set_mode(int ww, int hh);
		virtual int EnableBuffering(int);
		virtual int set_page(int);
#ifdef INDEX_COLORS
		static void dx_palette(unsigned col, unsigned rgb)
		{
			unsigned rgb2 = ((rgb>>16)&0xFF) + (rgb&0xFF00) + ((rgb<<16)&0xFF0000);
			palette->SetEntries(0, col, 1,(PALETTEENTRY *)&rgb2);
		}
#endif
		virtual void DisableBuffering(void)
		{
			current_buffer=0;
			bufnum = 0;
			fgl::videobase = videobase = buffer[3];
			current = primary;
			offset = 0;
		}
		virtual void UpdateRect(int x, int y, int xm, int ym, int w, int h)
		{
			if (g_bExclusive == true) return;

			// copy the rect's data into two points
			POINT p1;
			POINT p2;
			// blit the back buffer to our window's position
			RECT rect;
			RECT rect2;
//			ZeroMemory(&rect, sizeof( rect ));
			// get the client area
			GetClientRect(hwnd, &rect);
/*
			p1.x = rect.left;
			p1.y = rect.top;
			p2.x = rect.right;
			p2.y = rect.bottom;
			// convert it to screen coordinates (like DirectDraw uses)
			ClientToScreen(hwnd, &p1);
			ClientToScreen(hwnd, &p2);
			// copy the two points' data back into the rect
			rect.left   = p1.x;
			rect.top    = p1.y;
			rect.right  = p2.x;
			rect.bottom = p2.y;
*/
			p1.x = rect.left+x;
			p1.y = rect.top+y;
			p2.x = p1.x + w;
			p2.y = p1.y + h;

			rect2.left = p1.x;
			rect2.top  = p1.y;
			rect2.right= p2.x;
			rect2.bottom=p2.y;

			ClientToScreen(hwnd, &p1);
			ClientToScreen(hwnd, &p2);

			// copy the two points' data back into the rect
			rect.left   = p1.x;
			rect.top    = p1.y;
			rect.right  = p2.x;
			rect.bottom = p2.y;

			primary->Blt(&rect, secondary, &rect2, DDBLT_WAIT, NULL);
		}
};

#endif // win32

#ifdef FG_NAMESPACE
}
#endif


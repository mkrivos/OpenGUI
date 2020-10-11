/*
    OpenGUI - Drawing & Windowing library

    Copyright (C) 1996,2004  Marian Krivos

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    nezmar@atlas.sk

    base.cc - base graphics support routines
*/

// driver objects

#include <stdlib.h>

#include "fgbase.h"
#include "_fastgl.h"
#include "drivers.h"

#ifdef __linux__
#include "linkeyb.h"
#include "linux.h"
#endif

#if defined (__sun__) || defined (__sun)
#include "solaris.h"
#endif

#ifdef _WIN32
#include "win32.h"
#include <io.h>
#endif

#ifdef FG_NAMESPACE
namespace fgl {
#endif

FGDriver		*__fg_driver = 0;

static int firsttime = 1;

FGScreen FGFrameBuffer;
bool FGDriver::is_ctrl=0, FGDriver::is_alt=0, FGDriver::is_shift=0;

void FGDriver::vram_line(int x, int y, int w, int h, FGPixel ink, unsigned ppop)
{
	FGFrameBuffer.set_ppop(ppop);
	FGFrameBuffer.line(x,y,w,h,ink);
}

#ifdef INDEX_COLORS
//
// Warning: do not reorder colors !!! (due to base.h colors definition)
//
void FGDriver::ReservedColors(void)
{
	CreateColor(0, 0, 0, 0);	// cblack
	CreateColor(68, 68, 68, 1);	// CDARK);
	CreateColor(88, 88, 88, 2);	// CGRAYED);
	CreateColor(124, 128, 116, 3);	// CGRAY1);
	CreateColor(104, 120, 132, 4);	// CGRAY2);
	CreateColor(168, 184, 172, 5);	// CGRAY3);
	CreateColor(0, 0, 128, 6);	// CBLUE);
	CreateColor(0, 0, 248, 7);	// CBLUELIGHT);
	CreateColor(0, 120, 0, 8);	// CGREEN);
	CreateColor(0, 248, 0, 9);	// CGREENLIGHT);
	CreateColor(212, 68, 72, 10);	// CRED);
	CreateColor(248, 0, 0, 11);	// CREDLIGHT);
	CreateColor(104, 64, 40, 12);	// CBROWN);
	CreateColor(255, 224, 0, 13);	// CYELLOW);
	CreateColor(216, 208, 184, 14);	// CWHITED);
	CreateColor(255, 255, 255, 15);	// CWHITE);
	CreatePaletteEntry(255, 255, 255, 255);	// for CWHITE - mouse rect when dragging ...
}
#endif

int FGDriver::preinit(void)
{
       	// not available
		if (available() == 0) return 0;
        // DLL not present
		if (link() == 0) return 0;
		// no mode for current hardware configuration
		if (build_modelist() == 0) return 0;
		return true;
}

int FGDriver::Flip(void)
{
	if (!bufnum)
	{
		static bool already = false;
		if (already == false) printf("PageFliping is disabled or unavailable!\n");
		already = true;
		return 0;
	}
	if (!set_page(current_buffer)) // set current as visible
		return 0;
	if (bufnum == ++current_buffer)
		current_buffer = 0;
	// and adjust ptr to VRAM to the next hidden page
	fgl::videobase = videobase = buffer[current_buffer];
	offset = current_buffer*h*pitch*bpp;
	return 1;
}

int FGDriver::launch(int ww, int hh, int syn, int verb)
{
	SetVerbose(verb);
	SetFrequency(syn);
	if (preinit())
	{
		if (set_mode(ww,hh))
	    {
			if (verbose) printf("\n");
			set_mouse();
			set_keyboard();
			message();
#ifdef INDEX_COLORS
			// force 8bit DAC
			if (fgstate.force_dac8) fgstate.palette_8=8;
#endif
			if (verbose)
			{
				printf("	driver name   : %s\n", name);
				if (subname) printf("	description   : %s\n", subname);
#ifndef FG_THREADED
				printf("	OpenGUI ver.  : %s\n", FG_VERSION);
#else
				printf("	OpenGUI ver.  : %s (THREAD_SAFE)\n", FG_VERSION);
#endif
				printf("	screen width  : %d, virtual %d, pitch %d\n", w, virt_w, pitch);
				printf("	screen height : %d, virtual %d\n", h, virt_h);
#ifdef INDEX_COLORS
				printf("	DAC width     : %d\n", fgstate.palette_8?8:6);
#endif
				printf("	bitsPerPixel  : %d\n", FASTGL_BPP);
				printf("	page-flipping : POSSIBLE\n");
				printf("	videomemory   : 0x%lx, 0x%lx => 0x%lx\n", (long)linear_base, linear_size, (long)videobase);
				if (mmio_base) printf("	memory io     : 0x%lx, 0x%lx => 0x%lx\n", (long)mmio_base, mmio_size, (long)mmio);
				printf("	addressing    : linear\n");
				printf(synch?"	monitor freq. : %d HZ\n":"	monitor freq. : default\n", (int)synch);
				printf("	accelerator   : %s\n", accel_hint?"hardware":"fg_soft_accel");
				printf("	mouse         : %s\n", mouse_string);
			}
			return 1;
	    }
	}
	return 0;
}

//
// set CLUT palette register - default hardware function!
// if you can predefine this, do it in the driver
//
static void __palette(unsigned col, unsigned rgb)
{
#if defined( __linux__) || defined(__sun__)
	if (GetPriv() == 0)	// no privilegged user
		return;
#endif
	outpb((unsigned short) 0x3c8, (unsigned char) col);
	if (fgstate.palette_8)
	{
		outpb(0x3c9, rgb >> 16);
		outpb(0x3c9, rgb >> 8);
		outpb(0x3c9, rgb);
	}
	else
	{
		rgb &= 0xfcfcfc;
		outpb(0x3c9, rgb >> 18);
		outpb(0x3c9, rgb >> 10);
		outpb(0x3c9, rgb >> 2);
	}
}

FGDriver::FGDriver(char *nm)
{
	assert(fgl::bpp == (FASTGL_BPP+1)/8);  // bad colors config
	if (strcmp(GetVer(), FG_VERSION))
	{
		printf("WARNING! Version mismatch: %s != %s\n", GetVer(), FG_VERSION);
	}
	memset(this, 0, sizeof(*this)-4);
	name = nm;
	subname = "";
	accel_name = "";
	vector_draw_line = fgl::__draw_line;
	vector_fill_box  = fgl::__fill_box;
	vector_draw_point= fgl::__draw_point;
	vector_palette   = fgl::__palette;
	vector_blit_copy = L1RamToVideo8;
	vector_blit_op   = L1RamToVideo2;
	vector_blit_a    = L1RamToVideoA;
	bpp = (FASTGL_BPP+1)/8;
	back_store=0;
	accel_hint = 0;
	w = h = virt_w = virt_h = 0;
	offset = 0;
	root = 0;
	msefd = -1;
	mouse_string = "default";
	is_ctrl = is_alt = is_shift=0;
}

void FGDriver::message(void)
{
	if (verbose && accel_name && accel_name!="")
	{
		printf("\t%s hardware found\n", accel_name);
		if (ops&1) printf("\tFILL_RECTANGLE: replaced with HW\n");
		if (ops&2) printf("\tDRAW_LINE     : replaced with HW\n");
		if (ops&4) printf("\tBLITTER       : replaced with HW\n");
		if (ops&8) printf("\tMOUSE CURSOR  : replaced with HW\n");
		printf("\n");
	}
}

/**
	sanity - for backing to previous console mode only.
	@ingroup Misc
*/
void cleanup(void)
{
	FGDriver *drv = __fg_driver;
	__fg_driver = 0;
	if (drv)
		drv->shutdown();
	delete drv;
}

// internal for mode switching
static void GetModeSize(int mode)
{
	switch (mode)
	{
		case G320x200:
			X_width = 320;
			Y_width = 200;
			break;
		case G640x480:
			X_width = 640;
			Y_width = 480;
			break;
		case G800x600:
			X_width = 800;
			Y_width = 600;
			break;
		case G1024x768:
			X_width = 1024;
			Y_width = 768;
			break;
		case G1280x1024:
			X_width = 1280;
			Y_width = 1024;
			break;
		case G1600x1200:
			X_width = 1600;
			Y_width = 1200;
			break;
		default:
			if (fgstate.__force_x && fgstate.__force_y)
			{
				X_width = fgstate.__force_x;
				Y_width = fgstate.__force_y;
			}
			else
			{
				X_width = Y_width = 1;
			}
			break;
	}
}

//!  first of all, try X11 if not disabled one
//!  then try DGA if not disabled one
//! then try FRAMEBUFFER if not disabled
//! then try SVGA/VESA as last chance
FGDriver * GetDriver(int priority)
{
	// test for X11 enviroment
#ifndef _WIN32
	char * x11_present = getenv("DISPLAY");
#endif
	switch(priority)
	{
#ifdef VESA_DRIVER
// debug only
		case 6:
			return new FGVesaDriver;
			break;
#endif

		case 5:
#if defined( __linux__) && defined(X11_DRIVER)
		if (fgstate.nox11 == 0 && x11_present)
			return new FGX11Driver;		// try standard X11 driver if available
		else
			return 0;
#endif
#if defined (__sun__) || defined (__sun)
			return new FGSolarisDriver;		// try standard X11 driver if available
#endif
			break;

		case 4:
#ifdef __linux__
#ifdef DGA_DRIVER
		if (fgstate.nodga == 0 && x11_present)
			return new FGDGADriver;	// try DGA2 as second driver if available
		else
			return 0;
#endif
#endif
			break;

		case 3:
#ifdef __linux__
			if (x11_present)
			{
				printf("Compile library with -DX11_DRIVER, relink app and try again!\n");
				exit(-1);
			}
#endif
			break;

		case 2:
#ifdef __linux__
			if (fgstate.nofb == 0)
				return new FGFBDriver;
			else
				return 0;
#endif
			break;

		case 1:
#ifdef __linux__
			return new FGSVGADriver;
#endif
			break;

		case 0:
#ifdef VESA_DRIVER
			return new FGVesaDriver;
#endif
#ifdef _WIN32
			return new FGWin32DXDriver("myExe", !fgstate.windowed_mode);
#endif
			break;

		default:
			return 0;
	}
	return 0;
}

/**
	Set the graphics mode with its resolution.
	@ingroup Misc
	@param mode the videomode (the screen resolution)
	@return TRUE if all is OK.
*/
int graph_set_mode(int mode)
{
	// call first time for calibration
	FGClock();
	GetModeSize(mode);

	if (__fg_driver==0)
	{
		for(int i=6; i>=0; i--)
		{
			__fg_driver = GetDriver(i);
			if (__fg_driver==0)
				continue;

			if (__fg_driver->launch(X_width, Y_width, fgstate.synch, fgstate.verbose) == 0)
			{
				cleanup();
			}
			else break;
		}

		// don't work any driver
		if (__fg_driver==0)
			return 0;

		if (firsttime==1)
			atexit(cleanup);
		firsttime = 0;
		memmove(_fgl_palette, _default_palette, sizeof(_fgl_palette));
#ifdef INDEX_COLORS
		_set_default_palette();
		__fg_driver->ReservedColors();
#endif
	}
	else if (mode == GTEXT && __fg_driver)			// text mode?
	{
		cleanup();
	}
	else if (mode != GTEXT && __fg_driver)			// resize mode?
	{
		graph_change_mode(X_width, Y_width);
	}
	return 1;
}

extern void RootResize(int,int);

int graph_change_mode(int ww, int hh)
{
	if (__fg_driver->GetW()==ww && __fg_driver->GetH()==hh) return 1;
	if (__fg_driver->set_mode(ww,hh))
	{
#ifdef INDEX_COLORS
		_set_fgl_palette();
#endif
		__fg_driver->change_mouse();
		RootResize(ww,hh);
		// restart buffering
		if (__fg_driver->GetBuffersNum())
			__fg_driver->EnableBuffering(__fg_driver->GetBuffersNum());
		return 1;
	}
	if (ww>=1024)
		idc_normal = &IDC_NORMAL_LARGE;
	else
		idc_normal = &IDC_NORMAL_SMALL;
	return 0;
}

int FGDriver::detect(void)
{
#ifndef _WIN32
	if (root==0)	// X11 drv
		return driver=0;

	if (driver < 0)
		driver = 0;			// no detect, force VESA
	else if	(driver	== 0)
		driver = detect_video(fgstate.verbose);	// detect card

	mmio_base = CardInfo.MMIO_ADDR_DETECTED;
	linear_base = CardInfo.LFB_ADDR_DETECTED;
#endif
	return driver;
}

int FGDriver::postinit()
{
	isgraph=1;
	// WORKAROUND: if the base address doesn't exist, there are banked mode only!
#if !(defined(DGA_DRIVER) || defined(_WIN32))
	if (linear_base==0) islinear = 0;
#endif
	if (islinear==0 && bpp>1)
	{
		text_mode();
		printf("This color depth requires linear addressing mode!\n");
		return 0;
	}
	w = req_w;
	h = req_h;
	x_offset = y_offset = 0;
	current_buffer = 0;
	bufnum = 0;
	if (virt_w) pitch = virt_w*bpp;
	else        pitch = w*bpp;
	X_virtual = virt_w;
	Y_virtual = virt_h;
	buffer[0] = buffer[1] = buffer[2] = videobase; // ak aindex
	// force linear if videomode 0x13
	if (w==320 && h==200) islinear=1;
	fgl::videobase = videobase;
	FGFrameBuffer.Resize(w, h);
	if (back_store)
	{
		put_block(0,0, back_store->GetW(), back_store->GetH(), back_store->GetArray(), _GSET);
		delete back_store;
		back_store = 0;
	}
	return 1;
}

void UpdateRect(int x, int y, int xm, int ym, int w, int h)
{
	if (__fg_driver)
		__fg_driver->UpdateRect(x,y,xm,ym,w,h);
}

int GetPriv(void)
{
	if (__fg_driver)
		return __fg_driver->GetPriv();
	return 0;
}

int FGDriver::EnableBuffering(int mode)
{
	bufnum = 0;
	current_buffer = 0;
	buffer[0] = videobase;

	if (!set_page(1))
			return  0;	// not supported
	if (verbose)
	{
		printf("	page flipping : 0\n");
		printf("			%x\n", h*pitch);
	}
	buffer[1] = (FGPixel *)((char *)videobase+(h*pitch));
	bufnum = 2;
	if (mode==FG_DOUBLEBUFFER)
		return bufnum;
	if (set_page(2))
	{
		bufnum = 3; // triple-buffering
		buffer[2] = (FGPixel *)((char *)videobase+(h*pitch*2));
		if (verbose) printf("			%x\n", h*pitch*2);
	}
	set_page(0);
	if (mode==FG_TRIPLEBUFFER)
		return bufnum;
	if (set_page(3))
	{
		bufnum = 4; // quad-buffering
		buffer[3] = (FGPixel *)((char *)videobase+(h*pitch*3));
		if (verbose) printf("			%x\n", h*pitch*3);
	}
	set_page(0);
	return bufnum;
}

int	FGDriver::find(int& ww, int& hh)
{
	int last=-1, i;
	for(i=0; i<total_modes; i++)
	{
		if (modelist[i].w == ww && modelist[i].h == hh)
		{
			return i;
		}
		else if (modelist[i].w <= ww)
		{
			last = i;
		}
	}
	if (last != -1)
	{
			ww = modelist[last].w;
			hh = modelist[last].h;
	}
	else if (total_modes>0) return total_modes-1; // try lowest resolution if there any
	return last;
}

//////////////////////////////////////////////////////////////////////////////

#if !defined(  __linux__) && !defined(  __rtems__) && !defined(__sun__) && !defined(__sun) && !defined(_WIN32)
//! generate a HZ tone for b msec
void FGAPI Snd(int a, int b)
{
	if (sound_enabled) sound(a);
	delay(b);
	nosound();
}
#endif

//! sound efect
void FGAPI Puk(void)
{
	if (sound_enabled) Snd(100, 10);
}


#if __linux__
int atexit(void (*cc)(void))
{
	return on_exit((void (*)(int, void *))cc, 0);
}
#endif

#ifdef FG_NAMESPACE
}
#endif


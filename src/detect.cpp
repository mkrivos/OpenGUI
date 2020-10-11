/*
    OpenGUI - Drawing & Windowing library

    Copyright (C) 1996,2003  Marian Krivos

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

    detect.c - detect some graphics chipset

*/


#include <stdio.h>

#include "fgbase.h"
#include "_fastgl.h"

#ifdef __cplusplus
extern "C" {
#endif
int ioperm (unsigned long int __from, unsigned long int __num,
			int __turn_on);

/* Set the I/O privilege level to LEVEL.  If LEVEL>3, permission to
   access any I/O port is granted.  This call requires root
   privileges. */
int iopl (int __level);
#ifdef __cplusplus
}
#endif

#ifdef FG_NAMESPACE
namespace fgl {
#endif

static const int SEQ = 0x3C4;
static int old, old2, SubVers;
static unsigned long buf[16], pci_found;
struct PCICardInfo CardInfo;
enum { LEAVE, ENTER };
static unsigned vgaIOBase=0, CHIPSchipset, ctisHiQV;

#define CT_520	 0
#define CT_525	 1
#define CT_530	 2
#define CT_535	 3
#define CT_540	 4
#define CT_545	 5
#define CT_546	 6
#define CT_548	 7
#define CT_550	 8
#define CT_554	 9
#define CT_555	10
#define CT_8554	11
#define CT_9000	12
#define CT_4300	13

static int rdinx(short pt, short inx)	//       {read register PT index INX}
 {
	short x;

	if (pt == 0x3C0)
	{
		x = inpb(fgstate.CRTC + 6);
	}							//    {If Attribute Register then reset Flip-Flop}

	outpb(pt, inx);
	return inpb(pt + 1);
}

static void wrinx(short pt, short inx, short val)	//    {write VAL to register PT index INX}
 {
	short x;

	if (pt == 0x3C0)
	{
		x = inpb(fgstate.CRTC + 6);
		outpb(pt, inx);
		outpb(pt, val);
	}
	else
	{
		outpb(pt, inx);
		outpb(pt + 1, val);
	}
}

//  Returns true if the bits in MSK
//  of register PT index RG are
//  read/writable

static int testinx2(short pt, short rg, short msk)
{
	short old, nw1, nw2;

	old = rdinx(pt, rg);
	wrinx(pt, rg, old & (~msk));
	nw1 = rdinx(pt, rg) & msk;
	wrinx(pt, rg, old | msk);
	nw2 = rdinx(pt, rg) & msk;
	wrinx(pt, rg, old);
	return ((nw1 == 0) && (nw2 == msk));
}

// Returns true if the bits in MSK
// of register PT are read/writable}

static int tstrg(short pt, short msk)
{
	short old, nw1, nw2;

	old = inpb(pt);
	outpb(pt, old & ~msk);
	nw1 = inpb(pt) & msk;
	outpb(pt, old | msk);
	nw2 = inpb(pt) & msk;
	outpb(pt, old);
	return ((nw1 == 0) && (nw2 == msk));
}

// Returns true if all bits of
// register PT index RG are
// read/writable.}
static int testinx(short pt, short rg)
{
	return testinx2(pt, rg, 0xff);
}

static void modinx(unsigned pt, unsigned inx, unsigned mask, unsigned nwv)
{
	unsigned temp;

	temp = ((rdinx(pt, inx) & (~mask)) + (nwv & mask));
	wrinx(pt, inx, temp);
}

static void setinx(unsigned pt, unsigned inx, unsigned val)
{
	unsigned x;

	x = rdinx(pt, inx);
	wrinx(pt, inx, x | val);
}

#define PCI_CONF_ADDR  0xcf8
#define PCI_CONF_DATA  0xcfc


static int pci_read_header (unsigned char bus, unsigned char device,
		unsigned char fn, unsigned long *buf)
{
  int i;
  unsigned long bx = ((fn&7)<<8) | ((device&31)<<11) | (bus<<16) |
						0x80000000;

  for (i=0; i<16; i++) {
		outpl (PCI_CONF_ADDR, bx|(i<<2));
		buf[i] = inpl (PCI_CONF_DATA);
  }
  return 0;
}


/* find a vga device of the specified vendor, and return
   its configuration (16 dwords) in conf
   return zero if device found.
   */
int pci_find_vendor_vga(unsigned long *conf)
{ unsigned long buf[16];
  int bus,device,cont;

  cont=1;
  pci_found = 0;
#ifdef __linux__
  if (getenv("IOPERM") == 0) {
        if (iopl(3) < 0) {
	    printf("svgalib: vgapci: cannot get I/O permissions\n");
	    exit(1);
	}
  }
#endif
  for(bus=0;(bus<16)&&cont;bus++)
    for(device=0;(device<32)&&cont;device++)
	{
      pci_read_header(bus,device,0,buf);
      if ((((buf[2]>>16)&0xffff)==0x0300)) cont=0;
    }
	CardInfo.vendor = 0;
  if (!cont)
  {
  	memcpy(conf,buf,sizeof(buf));
	CardInfo.vendor = (short)buf[0];
	CardInfo.device = (short)(buf[0]>>16);
	CardInfo.MMIO_ADDR_DETECTED = (void *)buf[4];
	CardInfo.LFB_ADDR_DETECTED = (void *)buf[5];
	CardInfo.LFB_SIZE = CardInfo.MMIO_SIZE = 0;
	pci_found = buf[0];
  }
#ifdef __linux__
  if (getenv("IOPERM") == 0)
  	iopl(0);
#endif
  return cont;
}

//  (* First test for Cirrus 54xx *)
static int detect_cirrus(void)
{
	if ((pci_found&0xFFFF) != 0x1013) // no pci detected
	{
	old = rdinx(0x3C4, 6);
	wrinx(0x3C4, 6, 0);
	if (rdinx(0x3C4, 6) == 15)
	{
		wrinx(0x3c4, 6, 0x12);
		if ((rdinx(0x3C4, 6) == 0x12) && testinx2(0x3C4, 0x1E, 0x3F))
		{
			SubVers = rdinx(0x3d4, 0x27);
			if (testinx(0x3CE, 9))
				return 1;		// cirrus CL-54xx
			else if (testinx(0x3C4, 0x19))
				return 2;		// CL-GD62xx
			else
				return 3;		//name:='Cirrus AVGA2 (5402)';
		}
	}
	else
		wrinx(0x3C4, 6, old);

//  (* Now test for 64xx *)

	old = rdinx(0x3CE, 0xA);
	wrinx(0x3CE, 0xA, 0xCE);
	if (rdinx(0x3CE, 0xA) == 0)
	{
		wrinx(0x3CE, 0xA, 0xEC);
		if (rdinx(0x3CE, 0xA) == 1)
		{
			SubVers = rdinx(0x3CE, 0xAA);
			return 4;			// 'Cirrus CL-GD64xx';

		}
	}
	wrinx(0x3CE, 0xA, old);
	return 0;
	}
	return (buf[0]>>16) & 0xffffU;
}

static int detect_trident(void)
{
	short old, val;

	if ((pci_found&0xFFFF) != 0x1023) // no pci detected
	{
		wrinx(SEQ, 0xB, 0);
		SubVers = inpb(SEQ + 1);
		old = rdinx(SEQ, 0xE);
		outpb(SEQ + 1, 0);
		val = inpb(SEQ + 1);
		outpb(SEQ + 1, old);
		if ((val & 15) == 2)
		{
			outpb(0x3c5, old ^ 2);	// (* Trident should restore bit 1 reversed *)
			return 1;
		}
		return 0;
	}
	return (buf[0]>>16) & 0xffffU;
}

static int detect_tseng(void)
{
	short old2;

	old = rdinx(0x3BF, 3);
	old2 = rdinx(0x3D8, 0xA0);
	outpb(0x3BF, 3);
	outpb(0x3D8, 0xA0);			//  {Enable ET4000 extensions}

	if (tstrg(0x3CD, 0x3F))
	{
		if (testinx2(fgstate.CRTC, 0x33, 0xF))
			return 4000;
		else
			return 3000;
	}
	wrinx(0x3d8, 0xA0, old2);
	wrinx(0x3bf, 3, old);
	return 0;
}

static int detect_s3(void)
{
	if ((pci_found&0xFFFF) != 0x5333) // no pci detected
	{
		old = rdinx(fgstate.CRTC, 0x38);
		wrinx(fgstate.CRTC, 0x38, 0);		//   {disable extensions}

		if (!testinx2(fgstate.CRTC, 0x35, 0xF))
		{
			wrinx(fgstate.CRTC, 0x38, 0x48);
			if (testinx2(fgstate.CRTC, 0x35, 0xF))
			{
				wrinx(fgstate.CRTC, 0x38, old);
				return rdinx(fgstate.CRTC, 0x30);
			}
		}
		wrinx(fgstate.CRTC, 0x38, old);
		return 0;
	}
	return (buf[0]>>16) & 0xffffU;
}

/*----------------------------------------------------------------------*/
/* CHIPS_EnterLeave --							*/
/*									*/
/* This function is called when the virtual terminal on which the	*/
/* server is running is entered or left, as well as when the server	*/
/* starts up and is shut down. Its function is to obtain and		*/
/* relinquish I/O  permissions for the SVGA device. This includes	*/
/* unlocking access to any registers that may be protected on the	*/
/* chipset, and locking those registers again on exit.			*/
/*									*/
/*----------------------------------------------------------------------*/
static void CHIPS_EnterLeave(int enter)
{
	unsigned char temp;

#ifdef DEBUG
    printf("CHIPS: CHIPS_EnterLeave(%d)\n", enter);
#endif

	/* (taken from XFree86) */

	if (enter)
	{
		/*
		 * This is a global. The CRTC base address depends on
		 * whether the VGA is functioning in color or mono mode.
		 * This is just a convenient place to initialize this
		 * variable.
		 */
		vgaIOBase = (inpb(0x3CC) & 0x01) ? 0x3D0 : 0x3B0;

		/*
		 * Here we deal with register-level access locks. This
		 * is a generic VGA protection; most SVGA chipsets have
		 * similar register locks for their extended registers
		 * as well.
		 */
		/* Unprotect CRTC[0-7] */
		outpb(vgaIOBase + 4, 0x11);
		temp = inpb(vgaIOBase + 5);
		outpb(vgaIOBase + 5, temp & 0x7F);

		/* Enters Setup Mode */
/*		outb(0x46E8, inb(0x46E8) | 16); */

		/* Extension registers access enable */
/*		outb(0x103, inb(0x103) | 0x80); */
	}
	else
	{
		/*
		 * Here undo what was done above.
		 */
		/* Exits Setup Mode */
/*		outb(0x46E8, inb(0x46E8) & 0xEF); */

		/* Extension registers access disable */
/*		outb(0x103, inb(0x103) & 0x7F); */

		/* Protect CRTC[0-7] */
		outpb(vgaIOBase + 4, 0x11);
		temp = inpb(vgaIOBase + 5);
		outpb(vgaIOBase + 5, (temp & 0x7F) | 0x80);
	}
}

/*----------------------------------------------------------------------*/
/* Indentify chipset; return non-zero if detected			*/
/*----------------------------------------------------------------------*/
static int CHIPS_test(void)
{
	unsigned char temp;

#ifdef DEBUG
    printf("CHIPS: CHIPS_test\n");
#endif

	/*
	 * OK. We have to actually test the hardware. The
	 * EnterLeave() function (described below) unlocks access
	 * to registers that may be locked, and for OSs that require
	 * it, enables I/O access. So we do this before we probe,
	 * even though we don't know for sure that this chipset
	 * is present.
	 */
	CHIPS_EnterLeave(ENTER);

	/*
	 * Here is where all of the probing code should be placed.
	 * The best advice is to look at what the other drivers are
	 * doing. If you are lucky, the chipset reference will tell
	 * how to do this. Other resources include SuperProbe/vgadoc2,
	 * and the Ferraro book.
	 */
	outpb(0x3D6, 0x00);
	temp = inpb(0x3D6+1);

/*
 *	Reading 0x103 causes segmentation violation, like 46E8 ???
 *	So for now just force what I want!
 *
 *	Need to look at ioctl(console_fd, PCCONIOCMAPPORT, &ior)
 *	for bsdi!
 */
	CHIPSchipset = 99;

	if (temp != 0xA5)
	{
		if ((temp & 0xF0) == 0x70)
		{	CHIPSchipset = CT_520;
		}
		else
		if ((temp & 0xF0) == 0x80)	/* Could also be a 65525 */
		{	CHIPSchipset = CT_530;
		}
		else
		if ((temp & 0xF8) == 0xC0)
		{	CHIPSchipset = CT_535;
		}
		else
		if ((temp & 0xF8) == 0xD0)
		{	CHIPSchipset = CT_540;
		}
		else
		if ((temp & 0xF8) == 0xD8)
		{
		    switch (temp&0x7) {
		      case 3:
			CHIPSchipset = CT_546;
			break;
		      case 4:
			CHIPSchipset = CT_548;
			break;
		      default:
			CHIPSchipset = CT_545;
			}
		}
	}

	/* At this point the chip could still be a ct65550, so check */
	if ((temp != 0) && (CHIPSchipset == 99)) {
		outpb(0x3D6, 0x02);
		temp = inpb(0x03D7);
		if (temp == 0xE0) {
		CHIPSchipset = CT_550;
		ctisHiQV = true;
		}
		if (temp == 0xE4) {
		CHIPSchipset = CT_554;
		ctisHiQV = true;
		}
		if (temp == 0xE5) {
		CHIPSchipset = CT_555;
		ctisHiQV = true;
		}
		if (temp == 0xF4) {
		CHIPSchipset = CT_8554;
		ctisHiQV = true;
		}
		if ((temp == 0xC0) || (temp == 0x30)) { /* 0x30 is for 69030 */
		CHIPSchipset = CT_9000;
		ctisHiQV = true;
		}
	}

	if (CHIPSchipset == 99)
	{	/* failure, if no good, then leave */

		/*
		 * Turn things back off if the probe is going to fail.
		 * Returning FALSE implies failure, and the server
		 * will go on to the next driver.
		 */
		CHIPS_EnterLeave(LEAVE);

		return(false);
	}


//	CHIPS_init(0, 0, 0);

	return true;
}

/*----------------------------------------------------------------------*/
/* Bank switching function - set 64K bank number			*/
/*----------------------------------------------------------------------*/
void CHIPS_setpage(int page)
{
#ifdef DEBUG
	printf("CHIPS: CHIPS_setpage(%d)\n",page);
#endif

	if (ctisHiQV) {
		outpw(0x3D6, ((page&0x7F) << 8) | 0x0E);
	} else {
#if !defined(seperated_read_write_bank)
		outpw(0x3D6, (page << 12) | 0x10);	/* bank 0  ( 64k window at 0xA0000 )*/
		if (CHIPSchipset == CT_4300) {
		  unsigned char tmp;
		  outpb(0x3D6, 0x0C);
		  tmp = inpb(0x3D7) & 0xEF;
		  outpw(0x3D6, ((((page & 0x10) | tmp) << 8) | 0x0C));
		}
#else
		int	temp;
		temp = (page << 12);
		outpw(0x3D6, temp | 0x10);		/* bank 0 ( 32k window at 0xA0000 ) */
		outpw(0x3D6, temp + ((1 << 11) | 0x11));	/* bank 1 ( 32k window at 0xA8000 ) */
		if (CHIPSchipset == CT_4300) {
		  unsigned char tmp;
		  outpb(0x3D6, 0x0C);
		  tmp = inpb(0x3D7) & 0xAF;
		  outpw(0x3D6, ((((page & 0x10) | (( ((page << 2) + 2) & 0x40) |
						  tmp) << 8) | 0x0C));
	    }
#endif
	}

}

/*----------------------------------------------------------------------*/
/* W A R N I N G  :							*/
/* 	when using seperate banks, each bank can only access a maximum	*/
/*	of 32k. bank0 is accessed at 0xA0000 .. 0xA7FFF and bank1	*/
/*	is accessed at 0xA8000 .. 0xAFFFF				*/
/*	The GL library shipped with SVGALIB expects to be able to	*/
/*	access a 64k contiguoius window at 0xA0000 .. 0xAFFFF		*/
/*----------------------------------------------------------------------*/


/*----------------------------------------------------------------------*/
/* Bank switching function - set 32K bank number			*/
/* WARNING: this function uses a granularity or 32k not 64k		*/
/*----------------------------------------------------------------------*/
void CHIPS_setreadpage(int page)
{
#ifdef DEBUG
    printf("CHIPS: CHIPS_setreadpage(%d)\n",page);
#endif
    if (CHIPSchipset == CT_4300) {
      unsigned char tmp;
	  outpw(0x3D6, (page << 11) | 0x10);	/* bank 0 */
	  outpb(0x3D6, 0x0C);
	  tmp = inpb(0x3D7) & 0xEF;
	  outpw(0x3D6, (((((page >> 1) & 0x10) | tmp) << 8) | 0x0C));
	} else {
	  outpw(0x3D6, (page << 11) | 0x10);	/* bank 0 */
    }
}

/*----------------------------------------------------------------------*/
/* Bank switching function - set 32K bank number			*/
/* WARNING: this function uses a granularity or 32k not 64k		*/
/*----------------------------------------------------------------------*/
void CHIPS_setwritepage(int page)
{
#ifdef DEBUG
    printf("CHIPS: CHIPS_setwritepage(%d)\n",page);
#endif
    if (CHIPSchipset == CT_4300) {
      unsigned char tmp;
	  outpw(0x3D6, (page << 11) | 0x11);	/* bank 1 */
	  outpb(0x3D6, 0x0C);
	  tmp = inpb(0x3D7) & 0xBF;
	  outpw(0x3D6, (((((page << 1) & 0x40) | tmp) << 8) | 0x0C));
	} else {
      outpw(0x3D6, (page << 11) | 0x11);	/* bank 1 */
    }
}


static int detect_chips(void)
{
	return CHIPS_test() ? CHIPSchipset : 0;
}

static int detect_WesternDigital(void)
{
	SubVers = 0;
	old = rdinx(0x3CE, 15);
	setinx(0x3CE, 15, 0x17);	//  {Lock registers}

	if (!testinx2(0x3CE, 9, 0x7F))
	{
		modinx(0x3CE, 0xF, 0x17, 5);	//   {Unlock again}

		if (testinx2(0x3CE, 9, 0x7F))
		{
			old2 = rdinx(fgstate.CRTC, 0x29);
			modinx(fgstate.CRTC, 0x29, 0x8F, 0x85);		// {Unlock WD90Cxx registers}

			if (testinx(fgstate.CRTC, 0x2B))
				SubVers = 1;	// 'Paradise PVGA1A'

			else
			{
				wrinx(0x3C4, 6, 0x48);
				if (!testinx2(0x3C4, 7, 0xF0))
					SubVers = 2;	// 'Western Digital WD90C00'

				else if (!testinx(0x3C4, 0x10))
				{
					if (testinx2(fgstate.CRTC, 0x31, 0x68))
						SubVers = 3;	// 'Western Digital WD90C22'

					else if (testinx2(fgstate.CRTC, 0x31, 0x90))
						SubVers = 4;	// 'Western Digital WD90C20A'

					else
						SubVers = 5;	// 'Western Digital WD90C20';

					wrinx(0x3d4, 0x34, 0xA6);
					if ((rdinx(0x3d4, 0x32) & 0x20) != 0)
						wrinx(0x3d4, 0x34, 0);
				}
				else if (testinx2(0x3C4, 0x14, 0xF))
				{
					SubVers = (rdinx(fgstate.CRTC, 0x36) << 8) + rdinx(fgstate.CRTC, 0x37);
					switch (SubVers)
					{
						case 0x3234:	//'Western Digital WD90C24'

						case 0x3236:	//'Western Digital WD90C26'

						case 0x3330:	//'Western Digital WD90C30'

						case 0x3331:	//'Western Digital WD90C31'

						case 0x3333:	//'Western Digital WD90C33'

							break;
						default:
							SubVers = -1;
							break;
					}
				}
				else if (!testinx2(0x3C4, 0x10, 4))
					SubVers = 5;	// 'Western Digital WD90C10'

				else
					SubVers = 6;	//'Western Digital WD90C11';

			}
			wrinx(0x3d4, 0x29, old2);
		}
	}
	wrinx(0x3CE, 0xF, old);
	return SubVers;
}

static int detect_nvidia(void)
{
	if ((pci_found&0xFFFF) != 0x12d2 && (pci_found&0xFFFF) != 0x10de) return 0;
	if (((buf[0]>>16)&0xffff)<0x18) return 0;
	return buf[0]>>16;
}

static int detect_matrox(void)
{
	if ((pci_found&0xFFFF) != 0x102b) return 0;
	return buf[0]>>16;
}

static int detect_banshee(void)
{
	if ((pci_found&0xFFFF) != 0x121A) return 0;
	if (((buf[0]>>16)&0xffff)<3) return 0; // banshee and better
	return buf[0]>>16;
}

static int detect_rendition(void)
{
	if ((pci_found&0xFFFF) != 0x1163) return 0;
	if (((buf[0]>>16)&0xffff) != 0x2000) return 0;
	return buf[0]>>16;
}

static int detect_permedia(void)
{
	if ((pci_found&0xFFFF) != 0x3d3d) return 0;
	return buf[0]>>16;
}

static int detect_sis(void)
{
	if ((pci_found&0xFFFF) != 0x1039) return 0;
	return buf[0]>>16;
}

static int detect_intel(void)
{
	if ((pci_found&0xFFFF) != 0x8086) return 0;
	if (((buf[0]>>16)&0xffff)==0x7800) return 0x7800;
	if (((buf[0]>>16)&0xffff)==0x7121) return 0x7121;
	if (((buf[0]>>16)&0xffff)==0x7123) return 0x7125;
	if (((buf[0]>>16)&0xffff)==0x7125) return 0x7125;
	return 0;
}

int detect_video(int v)
{
	int val, rc = 0;
#ifdef __linux__
	ioperm(0x3b4, 0x3df - 0x3b4 + 1, 1);
#endif
	if (inpb(0x3CC) & 1)
		fgstate.CRTC = 0x3D4;
	else
		fgstate.CRTC = 0x3B4;

	memset(&CardInfo, 0, sizeof(CardInfo));
	pci_find_vendor_vga(buf);

	val = detect_nvidia();
	if (val)
	{
		rc = FG_NVIDIA;
#ifndef __linux__
		wrinx(fgstate.CRTC, 0x6, 0x57); // unlock card
		if (val>0x18)
		{
			modinx(fgstate.CRTC, 0x11, 0x80, 0); // TNT and above
			wrinx(fgstate.CRTC, 0x1f, 0x57); // unlock card
		}
#endif
	}
	if (v)
		printf("Testing for nVidia       : %x\n", val);
	if (rc) return rc;

	val = detect_intel();
	if (val==0x7800)
		rc = FG_INTEL740;
	else if (val==0x7121)
		rc = FG_INTEL810;
	if (v)
		printf("Testing for INTEL        : %x\n", val);
	if (rc) return rc;

	val = detect_s3();
	if (val)
	{
		if (val >= 225)
			rc = FG_S3V2;
		else
			rc = FG_S3;
	}
	if (v)
		printf("Testing for S3           : %x\n", val);
	if (rc) return rc;

	val = detect_matrox();
	if (val)
		rc = FG_MATROX;
	if (v)
		printf("Testing for MATROX       : %x\n", val);
	if (rc) return rc;

	val = detect_permedia();
	if (val)
		rc = FG_PERMEDIA;
	if (v)
		printf("Testing for PERMEDIA     : %x\n", val);
	if (rc) return rc;

	val = detect_banshee();
	if (val)
		rc = FG_BANSHEE;
	if (v)
		printf("Testing for 3DFX VOODOO  : %x\n", val);
	if (rc) return rc;

	val = detect_rendition();
	if (val)
		rc = FG_RENDITION;
	if (v)
		printf("Testing for RENDITION    : %x\n", val);
	if (rc) return rc;

	val = detect_cirrus();
	if (val)
		rc = FG_CIRRUS;
	if (v)
		printf("Testing for CIRRUS LOGIC : %x\n", val);
	if (rc) return rc;

	val = detect_sis();
	if (val)
		rc = FG_SIS;
	if (v)
		printf("Testing for SIS          : %x\n", val);
	if (rc) return rc;

	val = detect_trident();
	if (val)
		rc = FG_TRIDENT;
	if (v)
		printf("Testing for TRIDENT      : %d\n", val);
	if (rc) return rc;

	val = detect_tseng();
	if (val)
	{
		if ((val = 3000) != 0)
			rc = FG_TSENG3;
		else
			rc = FG_TSENG4;
	}
	if (v)
		printf("Testing for TSENG        : %d\n", val);
	if (rc) return rc;

	val = detect_chips();
	if (val)
		rc = FG_CHIPS;
	if (v)
		printf("Testing for CHIPS&TECH   : %d\n", val);
	if (rc) return rc;

	val = detect_WesternDigital();
	if (val)
		rc = FG_WDIGITAL;
	if (v)
		printf("Testing for WESTERN DIGIT: %d\n", val);
	if (rc) return rc;

	return rc=FG_VESA;
}

#ifdef FG_NAMESPACE
}
#endif


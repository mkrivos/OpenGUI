/*
   Copyright (C) 1998-1999  Andrew Apted   [andrew@ggi-project.org]

   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
   THE AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
   IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


  $Id: linkeyb.cpp 2524 2005-07-04 06:40:22Z majo $

  $Log$
  Revision 1.2  2005/07/04 06:40:22  majo
  QtCore

  Revision 1.1.1.1  2005/05/12 10:52:37  majo
  i

  Revision 1.4  2004/12/16 10:44:51  majo
  valgrindize

  Revision 1.3  2004/04/27 17:37:47  majo
  fixed key buffering on linux console

  Revision 1.2  2004/02/23 20:08:01  majo
  all classes are with prefix FG* on now
  polygon functions uses FGPointArray from now
  class GuiEvent is renamed to FGEvent
  some by parameters overloaded methods was removed (class FGWindow)
  many other small changes

  Revision 1.1.1.1  2004/01/22 16:25:08  majo
  initial release

*/

#include <ctype.h>
#include <assert.h>

#include "fgbase.h"
#include "_fastgl.h"
#include "drivers.h"

#include "linkeyb.h"

#ifdef FG_NAMESPACE
namespace fgl {
#endif

static int got_stopped;
static gii_input inp;
static keyboard_hook kb_hook;

static void sighandler(int unused)
{
	got_stopped = 1;
}

static uint32 basic_trans(int sym, int islabel, uint32 *modifiers,
			  uint32 label, int keycode)
{
	int typ = KTYP(sym);
	int val = KVAL(sym);

	/* Handle some special cases */
	switch (sym) {
	case 0x1c:
		return GIIK_PrintScreen;
	}

	/* Handle the rest */
	switch (typ) {
	case KT_LATIN:
	case KT_LETTER:
		if (islabel && (val == 0x7f || val == 0x08)) {
			/* Backspace and delete mapping is a mess under Linux.
			   We simply use the keycode to make it right.
			*/
#ifdef __i386__ /* Keycodes for other platforms are welcome. */
			if (keycode == 0x0e) return GIIUC_BackSpace;
			if (keycode == 0x6f) return GIIUC_Delete;
#endif
			return val;
		}
		if (islabel || (*modifiers & GII_MOD_CAPS)) {
			if ((val >= 'a' && val <= 'z') ||
			    (val >= GIIUC_agrave &&
				 val <= GIIUC_ydiaeresis &&
			     val !=  GIIUC_Division)) {
				return (val - 0x20);
			}
		}
		return val;

	case KT_FN:
		if (val < 20) {
			return GII_KEY(GII_KT_FN, val+1);
		} else if (val >= 30) {
#ifdef K_UNDO
			if (val == K_UNDO) return GIIK_Undo;
#endif
			return GII_KEY(GII_KT_FN, val-9);
		}
		break;

	case KT_META:
		*modifiers = *modifiers | GII_MOD_META;
		return val;
	case KT_PAD:
		if (val <= 9) {
			if (islabel) {
				return GII_KEY(GII_KT_PAD, '0' + val);
			} else if ((*modifiers & GII_MOD_NUM)) {
				return '0' + val;
			}
			switch (val) {
			case 0:
				return GIIK_Insert;
			case 1:
				return GIIK_End;
			case 2:
				return GIIK_Down;
			case 3:
				return GIIK_PageDown;
			case 4:
				return GIIK_Left;
			case 5:
				/* X does this, so why not...*/
				return GIIK_Begin;
			case 6:
				return GIIK_Right;
			case 7:
				return GIIK_Home;
			case 8:
				return GIIK_Up;
			case 9:
				return GIIK_PageUp;
			}
		}
		switch (sym) {
		case K_PPLUS:
			if (islabel)	return GIIK_PPlus;
			else 		return GIIUC_Plus;
		case K_PMINUS:
			if (islabel)	return GIIK_PMinus;
			else 		return GIIUC_Minus;
		case K_PSTAR:
			if (islabel)	return GIIK_PAsterisk;
			else 		return GIIUC_Asterisk;
		case K_PSLASH:
			if (islabel)	return GIIK_PSlash;
			else 		return GIIUC_Slash;
		case K_PENTER:
			if (islabel)	return GIIK_PEnter;
			else 		return GIIUC_Return;
			/* Label won't work on keyboards which has both, but
			   it's the best we can do. */
		case K_PCOMMA:
			if (islabel)	return GIIK_PDecimal;
			else if ((*modifiers & GII_MOD_NUM)) {
				return GIIUC_Comma;
			} else {
				return GIIUC_Delete;
			}
		case K_PDOT:
			if (islabel)	return GIIK_PDecimal;
			else if ((*modifiers & GII_MOD_NUM)) {
				return GIIUC_Period;
			} else {
				return GIIUC_Delete;
			}
		case K_PPLUSMINUS:
			if (islabel)	return GIIK_PPlusMinus;
			else 		return GIIUC_PlusMinus;
#ifdef K_PPARENL
		case K_PPARENL:
			if (islabel)	return GIIK_PParenLeft;
			else 		return GIIUC_ParenLeft;
		case K_PPARENR:
			if (islabel)	return GIIK_PParenRight;
			else 		return GIIUC_ParenRight;
#endif
		default:
				/* Unhandled PAD key */
			return GIIK_VOID;
		}
		break;

	case KT_CONS:
		return GIIK_VOID;

	case KT_DEAD:
		switch (sym) {
		case K_DGRAVE:
			return GIIK_DeadGrave;
		case K_DACUTE:
			return GIIK_DeadAcute;
		case K_DCIRCM:
			return GIIK_DeadCircumflex;
		case K_DTILDE:
			return GIIK_DeadTilde;
		case K_DDIERE:
			return GIIK_DeadDiaeresis;
		}
		return GIIK_VOID;

	case KT_SPEC:
	case KT_CUR:
		break;

	default:  /* The rest not handled yet */
		return GIIK_VOID;
	}

	switch (sym) {
	case K_HOLE:		return GIIK_VOID;

	case K_FIND:		return GIIK_Home;
	case K_SELECT:		return GIIK_End;
	case K_INSERT:		return GIIK_Insert;
	case K_REMOVE:		return GIIK_Delete;
	case K_PGUP:		return GIIK_PageUp;
	case K_PGDN:		return GIIK_PageDown;
	case K_MACRO:		return GIIK_Macro;
	case K_HELP:		return GIIK_Help;
	case K_DO:		return GIIK_Do;
	case K_PAUSE:		return GIIK_Pause;
	case K_ENTER:		return GIIK_Enter;
	case K_BREAK:		return GIIK_Break;
	case K_CAPS:		return GIIK_CapsLock;
	case K_NUM:		return GIIK_NumLock;
	case K_HOLD:		return GIIK_ScrollLock;
	case K_BOOT:		return GIIK_Boot;
	case K_CAPSON:		return GIIK_CapsLock;
	case K_COMPOSE:		return GIIK_Compose;
	case K_SAK:		return GIIK_SAK;
#ifdef K_CAPSSHIFT
	case K_CAPSSHIFT: 	return GIIK_CapsLock;
#endif

	case K_SCROLLFORW:
		if (!islabel && label == GIIK_PageDown &&
		    (*modifiers & GII_MOD_SHIFT)) {
			return GIIK_PageDown;
		}
		return GIIK_ScrollForw;
	case K_SCROLLBACK:
		if (!islabel && label == GIIK_PageUp &&
		    (*modifiers & GII_MOD_SHIFT)) {
			return GIIK_PageUp;
		}
		return GIIK_ScrollBack;
	case K_BARENUMLOCK: 	return GIIK_NumLock;

	case K_DOWN:		return GIIK_Down;
	case K_LEFT:		return GIIK_Left;
	case K_RIGHT:		return GIIK_Right;
	case K_UP:		return GIIK_Up;
	}

	/* Undo some Linux braindamage */

	if (sym >= 0x2000) {
		return sym ^ 0xf000;
	}

	return GIIK_VOID;
}


int _gii_linkey_trans(int keycode, int labelval, int symval, gii_key_event *ev)
{
	/* Set label field */
	switch (labelval) {
	case K_ALT:		ev->label = GIIK_AltL;		break;
	case K_ALTGR:		ev->label = GIIK_AltR;		break;
	case K_SHIFT:
#ifdef __i386__
		/* If you map shift keys to K_SHIFTL and K_SHIFTR they won't
		   work as shift keys, aieee what braindamage!
		   We try using the keycode to detect whether the user
		   actually pressed right shift.
		   (Non-ix86 platforms are very probably just as broken, but
		   they	are likely to have other keycodes) */
		if (keycode == 0x36) {
				ev->label = GIIK_ShiftR;	break;
		}
#endif
	case K_SHIFTL:		ev->label = GIIK_ShiftL;	break;
	case K_SHIFTR:		ev->label = GIIK_ShiftR;	break;
	case K_CTRL:
#ifdef __i386__
		/* Same thing with control keys... */
		if (keycode == 0x61) {
				ev->label = GIIK_CtrlR;		break;
		}
#endif
	case K_CTRLL:		ev->label = GIIK_CtrlL;		break;
	case K_CTRLR:		ev->label = GIIK_CtrlR;		break;

#ifdef K_SHIFTLOCK
		/* What are these things??? */
	case K_ALTLOCK:		ev->label = GIIK_AltL;		break;
	case K_ALTGRLOCK:	ev->label = GIIK_AltR;		break;
	case K_SHIFTLOCK:
	case K_SHIFTLLOCK:	ev->label = GIIK_ShiftL;	break;
	case K_SHIFTRLOCK:	ev->label = GIIK_ShiftR;	break;
	case K_CTRLLOCK:
	case K_CTRLLLOCK:	ev->label = GIIK_CtrlL;		break;
	case K_CTRLRLOCK:	ev->label = GIIK_CtrlR;		break;
#endif

#ifdef K_SHIFT_SLOCK
		/* What are these things??? */
	case K_ALT_SLOCK:	ev->label = GIIK_AltL;		break;
	case K_ALTGR_SLOCK:	ev->label = GIIK_AltR;		break;
	case K_SHIFT_SLOCK:
	case K_SHIFTL_SLOCK:	ev->label = GIIK_ShiftL;	break;
	case K_SHIFTR_SLOCK:	ev->label = GIIK_ShiftR;	break;
	case K_CTRL_SLOCK:
	case K_CTRLL_SLOCK:	ev->label = GIIK_CtrlL;		break;
	case K_CTRLR_SLOCK:	ev->label = GIIK_CtrlR;		break;
#endif
	default:
		ev->label = basic_trans(labelval, 1, &ev->modifiers, 0,
					keycode);
		break;
	}

	/* Set sym field */
	switch (symval) {
	case K_ALT:		ev->sym = GIIK_Alt;	break;
	case K_ALTGR:		ev->sym = GIIK_AltGr;	break;
	case K_SHIFT:
	case K_SHIFTL:
	case K_SHIFTR:		ev->sym = GIIK_Shift;	break;
	case K_CTRL:
	case K_CTRLL:
	case K_CTRLR:		ev->sym = GIIK_Ctrl;	break;

#ifdef K_SHIFTLOCK
	case K_ALTLOCK:		ev->sym = GIIK_Alt;	break;
	case K_ALTGRLOCK:	ev->sym = GIIK_AltGr;	break;
	case K_SHIFTLOCK:
	case K_SHIFTLLOCK:
	case K_SHIFTRLOCK:	ev->sym = GIIK_Shift;	break;
	case K_CTRLLOCK:
	case K_CTRLLLOCK:
	case K_CTRLRLOCK:	ev->sym = GIIK_Ctrl;	break;
#endif

#ifdef K_SHIFT_SLOCK
	case K_ALT_SLOCK:	ev->sym = GIIK_Alt;	break;
	case K_ALTGR_SLOCK:	ev->sym = GIIK_AltGr;	break;
	case K_SHIFT_SLOCK:
	case K_SHIFTL_SLOCK:
	case K_SHIFTR_SLOCK:	ev->sym = GIIK_Shift;	break;
	case K_CTRL_SLOCK:
	case K_CTRLL_SLOCK:
	case K_CTRLR_SLOCK:	ev->sym = GIIK_Ctrl;	break;
#endif
	default:
		ev->sym = basic_trans(symval, 0, &ev->modifiers, ev->label,
				      keycode);
		break;
	}

	/* Handle keys not recognized by Linux keymaps */
	if (ev->label == GIIK_VOID && ev->sym == GIIK_VOID) {
		switch (keycode) {
		case 0x7d:
			/* Left Windows key */
			ev->label = GIIK_MetaL;
			ev->sym = GIIK_Meta;
			break;
		case 0x7e:
			/* Right Windows key */
			ev->label = GIIK_MetaR;
			ev->sym = GIIK_Meta;
			break;
		case 0x7f:
			/* Menu key */
			ev->label = GIIK_Menu;
			ev->sym = GIIK_Meta;
			break;
		}
	}

	return 0;
}

//
// return kbd fd or negative
//
int GII_keyboard_init(int fd)
{
	struct termios new1;
	void (*oldinhandler)(int);
	void (*oldouthandler)(int);

	inp.priv = &kb_hook;

	/* put tty into "straight through" mode.
	 */
	if (tcgetattr(fd, &inp.priv->old_termios) < 0) {
		perror("Linux-kbd: tcgetattr failed");
	}

	new1 = inp.priv->old_termios;

	new1.c_lflag &= ~(ICANON | ECHO  | ISIG);
	new1.c_iflag &= ~(ISTRIP | IGNCR | ICRNL | INLCR | IXOFF | IXON);
	new1.c_iflag |= IGNBRK;
#if __rtems__
	new1.c_cc[VTIME] = 10;
#else
	new1.c_cc[VTIME] = 0;
#endif
	got_stopped = 0;
#ifndef __rtems__
	oldinhandler  = signal(SIGTTIN, sighandler);
	oldouthandler = signal(SIGTTOU, sighandler);
#endif
	if (tcsetattr(fd, TCSANOW, &new1) < 0) {
		perror("linkbd.c: tcsetattr failed");
	}
#ifndef __rtems__
	signal(SIGTTIN, oldinhandler);
	signal(SIGTTOU, oldouthandler);
#endif

	if (got_stopped) {
		/* We're a background process on this tty... */
		fprintf(stderr,
			"Linux-kbd: can't be run in the background!\n");
		close(fd);
		return GGI_EUNKNOWN;
	}

	/* Put the keyboard into MEDIUMRAW mode.  Despite the name, this
	 * is really "mostly raw", with the kernel just folding long
	 * scancode sequences (e.g. E0 XX) onto single keycodes.
	 */
	if (ioctl(fd, KDGKBMODE, &inp.priv->old_mode) < 0) {
		perror("Linux-kbd: couldn't get mode");
		inp.priv->old_mode = K_XLATE;
		return -1;
	}
	if (ioctl(fd, KDSKBMODE, K_MEDIUMRAW) < 0) {
		perror("Linux-kbd: couldn't set raw mode");
		tcsetattr(fd, TCSANOW, &(inp.priv->old_termios));
		close(fd);
		return GGI_ENODEVICE;
	}

	inp.priv->fd = fd;
	inp.priv->eof = 0;
	inp.priv->call_vtswitch = 0;
#ifdef __linux__
	inp.priv->call_vtswitch = 1;
#endif
	memset(inp.priv->keydown_buf, 0, sizeof(inp.priv->keydown_buf));

	/* Make sure LEDs match the flags */
	ioctl(inp.priv->fd, KDSKBLED, 0x22);

   if (ioctl(fd, KDGKBLED, &inp.priv->old_kbled) != 0)
   {
		perror("Linux-kbd: unable to get keylock status");
		inp.priv->old_kbled = 0x7f;
		inp.priv->lockedmod = 0;
	}
   else
   {
		inp.priv->lockedmod = LED2MASK(inp.priv->old_kbled);
	}

	inp.priv->normalmod = 0;
	inp.priv->modifiers = inp.priv->lockedmod | inp.priv->normalmod;
	inp.priv->lockedmod2 = inp.priv->lockedmod;
/*
	if (ioctl(fd, KDGKBDIACR, &inp.priv->accent_table) != 0) {
		inp.priv->accent_table.kb_cnt = 0;
	} else {
		unsigned i;
		for (i = 0; i < inp.priv->accent_table.kb_cnt; i++) {
			switch (inp.priv->accent_table.kbdiacr[i].diacr) {
			case '"':
				inp.priv->accent_table.kbdiacr[i].diacr =
					GIIUC_Diaeresis;
				break;
			case '\'':
				inp.priv->accent_table.kbdiacr[i].diacr =
					GIIUC_Acute;
				break;
			}
		}
	}
*/	inp.priv->accent = 0;
	inp.priv->needctrl2switch = 0;
#ifdef __linux__
	inp.priv->needctrl2switch = 1;
#endif
	inp.priv->ctrlstate = 0;

	inp.priv = inp.priv;
	inp.maxfd = inp.priv->fd + 1;
	FD_SET(inp.priv->fd, &inp.fdset);
	return fd;
}


static gii_event_mask GII_keyboard_flush_keys(void)
{
	gii_event_mask rc = 0;
	gii_event ev;
	int code;

	for (code=0; code < 128; code++) {

		if (! inp.priv->keydown_buf[code]) continue;
		inp.priv->keydown_buf[code] = 0;

		/* Send a key-release event */
		memset(&ev, 0, sizeof(gii_event));

		ev.any.type   = evKeyRelease;
		ev.any.size   = sizeof(gii_key_event);
		ev.key.button = code;
		ev.key.sym    = inp.priv->keydown_sym[code];
		ev.key.label  = inp.priv->keydown_label[code];
		ev.key.modifiers = inp.priv->modifiers;
		_giiEvQueueAdd(&ev);
		rc |= emKeyRelease;
	}

	inp.priv->normalmod = 0;
	inp.priv->modifiers = inp.priv->lockedmod;

	if (inp.priv->needctrl2switch) {
		inp.priv->ctrlstate = 0;
	}

	return rc;
}


static inline void handle_accent(linkbd_priv *priv, int symval, gii_event *ev)
{
	unsigned char diacr = priv->accent;
	unsigned i;

	for (i = 0; i < priv->accent_table.kb_cnt; i++) {
		if (priv->accent_table.kbdiacr[i].diacr == diacr &&
		    priv->accent_table.kbdiacr[i].base
		    == ev->key.sym) {
			ev->key.sym
				= priv->accent_table.kbdiacr[i].result;
				break;
		}
	}
	if (ev->key.sym == GIIUC_Space) ev->key.sym = priv->accent;

	priv->accent = 0;
}


static inline void handle_modifier(linkbd_priv *priv, gii_event *ev)
{
	uint32 mask;

	/* Handle AltGr properly */
	if (ev->key.label == GIIK_AltR) {
		if (ev->key.sym == GIIK_VOID) {
			ev->key.sym = GIIK_AltGr;
		}
		mask = 1 << (ev->key.sym & GII_KM_MASK);
	} else {
		mask = 1 << (ev->key.label & GII_KM_MASK);
	}

	if (GII_KVAL(ev->key.label) & GII_KM_LOCK) {
		if (ev->key.type == evKeyPress) {
			if (!(priv->lockedmod & mask)) {
				priv->lockedmod |= mask;
				ioctl(priv->fd, KDSKBLED,
				      MASK2LED(priv->lockedmod));
			} else {
				ev->key.sym = GIIK_VOID;
			}
		} else {
			if ((priv->lockedmod & mask)) {
				if (!(priv->lockedmod2 & mask)) {
					priv->lockedmod2 |= mask;
					ev->key.sym = GIIK_VOID;
				} else {
					priv->lockedmod2 &= ~mask;
					priv->lockedmod &= ~mask;
					ioctl(priv->fd, KDSKBLED,
					      MASK2LED(priv->lockedmod));
				}
			}
		}
	} else {
		if (ev->key.type == evKeyRelease) {
			priv->normalmod &= ~mask;
		} else {
			priv->normalmod |= mask;
		}
	}
	priv->modifiers = priv->lockedmod | priv->normalmod;
}

static inline gii_event_mask GII_keyboard_handle_data(gii_input *inp, int code)
{
	gii_event ev;
	struct kbentry entry;
	int symval=0, labelval=0;
	linkbd_priv *priv = inp->priv;

	memset(&ev,0,sizeof(gii_event));
	memset(&entry,0,sizeof(entry));

	if (code & 0x80)
	{
		code &= 0x7f;
		ev.key.type = evKeyRelease;
		priv->keydown_buf[code] = 0;
	}
	else if (priv->keydown_buf[code] == 0)
	{
		ev.key.type = evKeyPress;
		priv->keydown_buf[code] = 1;

	}
	else
	{
		ev.key.type = evKeyRepeat;
	}

	ev.key.button = code;
	ev.key.modifiers = priv->modifiers;

	if (ev.key.type == evKeyRelease &&
		GII_KTYP(priv->keydown_sym[code]) != GII_KT_MOD &&
		priv->keydown_sym[code] != GIIK_VOID)
	{
		/* We can use the cached values */
		ev.key.sym   = priv->keydown_sym[code];
		ev.key.label = priv->keydown_label[code];
	}
	else
	{
		/* Look up the keysym without modifiers, which will give us
		 * the key label (more or less).
		 */
		entry.kb_table = 0;
		entry.kb_index = code;

		if (ioctl(priv->fd, KDGKBENT, &entry) < 0)
		{
			return 0;
		}

		labelval = entry.kb_value;

		/* Now look up the full keysym in the kernel's table */

		/* calculate kernel-like shift value */
		entry.kb_table =
		  ((priv->modifiers & GII_MOD_SHIFT) ? (1<<KG_SHIFT)     : 0) |
		  ((priv->modifiers & GII_MOD_CTRL)  ? (1<<KG_CTRL)      : 0) |
		  ((priv->modifiers & GII_MOD_ALT)   ? (1<<KG_ALT)       : 0) |
		  ((priv->modifiers & GII_MOD_ALTGR) ? (1<<KG_ALTGR)     : 0) |
		  ((priv->modifiers & GII_MOD_META)  ? (1<<KG_ALT)       : 0) |
		  ((priv->modifiers & GII_MOD_CAPS)  ? (1<<KG_CAPSSHIFT) : 0);

		entry.kb_index = code;

		if (ioctl(priv->fd, KDGKBENT, &entry) < 0)
		{
			return 0;
		}

		symval = entry.kb_value;

		_gii_linkey_trans(code, labelval, symval, &ev.key);

		if (ev.key.type == evKeyPress) {
			if (priv->accent) {
				if (GII_KTYP(ev.key.sym) != GII_KT_MOD) {
					handle_accent(priv, symval, &ev);
				}
			} else if (KTYP(symval) == KT_DEAD) {
				priv->accent = GII_KVAL(ev.key.sym);
			}
		}
		if (GII_KTYP(ev.key.sym) == GII_KT_DEAD) {
			ev.key.sym = GIIK_VOID;
		}
	}

	/* Keep track of modifier state */
	if (GII_KTYP(ev.key.label) == GII_KT_MOD) {
		/* Modifers don't repeat */
		if (ev.key.type == evKeyRepeat) return 0;

		handle_modifier(priv, &ev);
	}

	if (ev.any.type == evKeyPress) {
		priv->keydown_sym[code]    = ev.key.sym;
		priv->keydown_label[code]  = ev.key.label;
	}
#ifdef DEBUG
printf("KEY-%s [ %c ] button=0x%02x modifiers=0x%02x "
		"sym=0x%04x label=0x%04x\n",
		(ev.key.type == evKeyRelease) ? "UP" :
		((ev.key.type == evKeyPress) ? "DN" : "RP"),
		ev.key.sym,
		ev.key.button, ev.key.modifiers,
		ev.key.sym,  ev.key.label);
#endif
	if (priv->call_vtswitch) {
		if (ev.key.label == GIIK_CtrlL && priv->needctrl2switch) {
			if (ev.key.type == evKeyRelease) {
				priv->ctrlstate = 0;
			} else if (ev.key.type == evKeyPress) {
				priv->ctrlstate = 1;
			}
		}
		/* Check for console switch.  Unfortunately, the kernel doesn't
		 * recognize KT_CONS when the keyboard is in RAW or MEDIUMRAW
		 * mode, so _we_ have to.  Sigh.
		 */
		if ((ev.key.type == evKeyPress) &&
		    (KTYP(entry.kb_value) == KT_CONS) && priv->ctrlstate) {
			int rc;
			int new_cons = 1+KVAL(entry.kb_value);

			/* Clear the keydown buffer, since we'll never know
			   what keys the user pressed (or released) while they
			   were away.
			 */
			rc = GII_keyboard_flush_keys();

#ifdef __linux__

			if (ioctl(priv->fd, VT_ACTIVATE, new_cons) < 0) {
				perror("ioctl(VT_ACTIVATE)");
			}
#endif
			return rc;
		}
	}

	/* finally queue the key event */
	ev.any.size   = sizeof(gii_key_event);
	_giiEvQueueAdd(&ev);
	return (1 << ev.any.type);
}

//
// return true if any event
//
static gii_event_mask GII_keyboard_poll(gii_input *inp, void *arg)
{
	unsigned char buf[256];
	gii_event_mask result = 0;
	int readlen, i;

	if (inp->priv->eof)
	{
		/* End-of-file, don't do any polling */
		return 0;
	}

	if (arg == 0)
	{
		fd_set fds = inp->fdset;
		struct timeval tv = { 0, 0 };
		if (select(inp->maxfd, &fds, NULL, NULL, &tv) <= 0)
		{
			return 0;
		}
	}
	else
	{
		if (! FD_ISSET(inp->priv->fd, ((fd_set*)arg))) {
			/* Nothing to read on our fd */
			return 0;
		}
	}

	/* Read keyboard data */
	while ((readlen = read(inp->priv->fd, buf, 256)) > 0)
	{
		for (i = 0; i < readlen; i++)
		{
			result |= GII_keyboard_handle_data(inp, buf[i]);
		}
		if (readlen != 256)
			break;
		else
		{
			fd_set fds = inp->fdset;
			struct timeval tv = { 0, 0 };
			if (select(inp->maxfd, &fds, NULL, NULL, &tv) <= 0)
			{
				return 0;
			}
		}
	}

	if (readlen == 0) {
		/* End-of-file occured */
		if (errno != EINTR)
		{
			inp->priv->eof = 1;
		}
	}
	else if (readlen < 0)
	{
		/* Error, we try again next time */
		perror("Linux-kbd: Error reading keyboard");
	}
	return result;
}

/* ---------------------------------------------------------------------- */

int GII_close(void)
{
	if (inp.priv == NULL) {
		/* Make sure we're only called once */
		return 0;
	}

	if (inp.priv->old_kbled != 0x7f) {
		ioctl(inp.priv->fd, KDSKBLED, inp.priv->old_kbled);
	}

	if (ioctl(inp.priv->fd, KDSKBMODE, inp.priv->old_mode) < 0) {
		perror("Linux-kbd: couldn't restore mode");
	}

	if (tcsetattr(inp.priv->fd, TCSANOW, &inp.priv->old_termios) < 0) {
		perror("Linux-kbd: tcsetattr failed");
	}
	inp.priv = NULL;
	return 0;
}

static int fronta;

void _giiEvQueueAdd(gii_event *ev)
{
//printf("key = %04x %x %x %x - %d %d %d\n",ev->key.sym,ev->key.type,ev->key.label,ev->key.modifiers, bool(ev->key.modifiers&1), bool(ev->key.modifiers&2), bool(ev->key.modifiers&4));
	if (ev->key.sym == GIIK_Shift)
	{
		FGDriver::is_shift = (ev->key.type == evKeyPress);
	}
	if (ev->key.sym == GIIK_Ctrl)
	{
		FGDriver::is_ctrl = (ev->key.type == evKeyPress);
	}
	if (ev->key.sym == GIIK_Alt || ev->key.sym == GIIK_AltGr)
	{
		FGDriver::is_alt = (ev->key.type == evKeyPress);
	}

	int key = ev->key.sym;
	if (ev->key.type == evKeyRelease || fronta || ev->key.label==0xE387) return;
	if (ev->key.modifiers&2 && ev->key.label==9) key = 0xfff;
	else
	{
		if ((key&0xFF00) == 0xe300) return; // shift, ctrl, alt, meta
		if (key == 0xE000 || !key || key == 0x7F) key = ev->key.label;
		if ((ev->key.sym&0xFE00) == 0xE000) // Fx keys, controls
		{
			if (ev->key.modifiers & 2) key = (key&255)+0x300;
			else key = (key&255)+0x200;
			if ((ev->key.sym&0xFF00) == 0xE100 ) // Fx keys + CTRL
			{
//				Snd(900, 100);
				key += 0x100;
			}
		}
		if (ev->key.modifiers & 4)
		{
			if (isupper(key)) key = tolower(key);
			key += 0x1000;
		}
	}
	fronta = key;
// button=0x3c modifiers=0x04 sym=0xe000 label=0xe102 // ALT +F2
// key = 1202 1 0
// button=0x3c modifiers=0x02 sym=0xe11a label=0xe102 // CTRL+F2
// key = 1302 1 0
}


int GII_get_key(void)
{
	int key = fronta;
	if (key)
	{
		fronta=0;
		return key;
	}
	GII_keyboard_poll(&inp,0);
/*
	if (GII_keyboard_poll(&inp,0))
	{
		return GII_get_key();
	}
*/
	return 0;
}
#ifdef __rtems__
int rtems_get_key( unsigned char scancode )
{
	int key = fronta[out];
	if (key)
	{
		fronta[out]=0;
		out = (out +1 ) & 31;
		return key;
	}
	GII_keyboard_handle_data( &inp, scancode );
	return 0;
}
#endif

#ifdef FG_NAMESPACE
}
#endif



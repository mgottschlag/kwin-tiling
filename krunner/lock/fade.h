/* xscreensaver, Copyright (c) 1992-1997, 2003 Jamie Zawinski <jwz@jwz.org>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation.  No representations are made about the suitability of this
 * software for any purpose.  It is provided "as is" without express or 
 * implied warranty.
 */


/* Shameless stolen from xscreensaver and integrated into kdesktop_lock
   (c) 2004 Chris Howells <howells@kde.org> */ 
 
#ifndef __FADE_H__
#define __FADE_H__

#include <X11/Xlib.h>

extern void fade_screens (Display *dpy,
			  Colormap *cmaps, Window *black_windows, int nwindows,
			  int seconds, int ticks,
			  Bool out_p, Bool clear_windows);

extern Visual *get_visual_resource (Screen *, char *, char *, Bool);

typedef struct saver_info saver_info;
typedef struct saver_screen_info saver_screen_info;

#endif /* __FADE_H__ */

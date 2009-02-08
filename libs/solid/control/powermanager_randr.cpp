/*
 * Copyright © 2007 Keith Packard
 * Copyright (c) 2009 Stephan Kulow
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of the copyright holders not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  The copyright holders make no representations
 * about the suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 */

#include "config-randr.h"
#include <X11/Xatom.h>
#include <X11/Xlib.h>

#if defined( HAS_RANDR_1_2 )
#include <X11/extensions/Xrandr.h>
#endif

float xrandr_brightlight(Display *dpy, long new_value = -1)
{
    if ( !dpy )
        return -1;

    ( void )new_value;

#if defined( HAS_RANDR_1_2 )
    int  major, minor;
    if (!XRRQueryVersion (dpy, &major, &minor))
        return -1;
    if (major < 1 || (major == 1 && minor < 2))
    {
        //fprintf (stderr, "RandR version %d.%d too old\n", major, minor);
        return -1;
    }

    Atom backlight = XInternAtom (dpy, "BACKLIGHT", True);
    if (backlight == None)
    {
        return -1;
    }

    double  cur, min, max;
    cur = min = max = -1;

    for (int screen = 0; screen < ScreenCount (dpy); screen++)
    {
        Window                    root = RootWindow (dpy, screen);
        XRRScreenResources  *resources = XRRGetScreenResources (dpy, root);
        int                    o;

        if (!resources) continue;

        for (o = 0; o < resources->noutput; o++)
        {
            RROutput    output = resources->outputs[o];
            XRRPropertyInfo *info;

            unsigned long   nitems;
            unsigned long   bytes_after;
            unsigned char   *prop;
            Atom            actual_type;
            int             actual_format;

            if (XRRGetOutputProperty (dpy, output, backlight,
                                      0, 4, False, False, None,
                                      &actual_type, &actual_format,
                                      &nitems, &bytes_after, &prop) != Success)
                continue;
            if (actual_type != XA_INTEGER || nitems != 1 || actual_format != 32)
                continue;
            cur = *((long *) prop);
            XFree (prop);
            min = max = -1;
            info = XRRQueryOutputProperty (dpy, output, backlight);
            if (info)
            {
                if (info->range && info->num_values == 2)
                {
                    min = info->values[0];
                    max = info->values[1];

                    if ( new_value >= 0 )
                    {
                        cur = new_value * (max - min) / 100 + min;
                        if (cur > max) cur = max;
                        if (cur < min) cur = min;
                        long value = cur;
                        XRRChangeOutputProperty (dpy, output, backlight, XA_INTEGER, 32,
                                                 PropModeReplace, (unsigned char *) &value, 1);
                    }
                }
            }
            XFree (info);
        }
    }
    if ( max > min )
        return ( (cur - min) * 100 / (max - min) );
#endif
    return -1;
}

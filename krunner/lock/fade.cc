/* xscreensaver, Copyright (c) 1992-2003 Jamie Zawinski <jwz@jwz.org>
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

#include <sys/time.h> /* for gettimeofday() */
#include <unistd.h>
#include <stdlib.h>
#include <kdebug.h>

#include "fade.h"

#define HAVE_XF86VMODE_GAMMA
#define HAVE_XF86VMODE_GAMMA_RAMP

/* XFree86 4.x+ Gamma fading */

#ifdef HAVE_XF86VMODE_GAMMA

#include <X11/extensions/xf86vmode.h>

int xf86_gamma_fade (Display *dpy,
                 Window *black_windows, int nwindows,
                 int seconds, int ticks,
                 Bool out_p, Bool clear_windows)
{
  int steps = seconds * ticks;
  long usecs_per_step = (long)(seconds * 1000000) / (long)steps;
  XEvent dummy_event;
  int nscreens = ScreenCount(dpy);
  struct timeval then, now;
  struct timezone tzp;
  int i, screen;
  int status = -1;
  xf86_gamma_info *info = 0;

  static int ext_ok = -1;

  /* Only probe the extension once: the answer isn't going to change. */
  if (ext_ok == -1)
    ext_ok = xf86_check_gamma_extension (dpy);

  /* If this server doesn't have the gamma extension, bug out. */
  if (ext_ok == 0)
    goto FAIL;

# ifndef HAVE_XF86VMODE_GAMMA_RAMP
  if (ext_ok == 2) ext_ok = 1;  /* server is newer than client! */
# endif

  info = (xf86_gamma_info *) calloc(nscreens, sizeof(*info));

  /* Get the current gamma maps for all screens.
     Bug out and return -1 if we can't get them for some screen.
   */
  for (screen = 0; screen < nscreens; screen++)
    {
      if (ext_ok == 1)  /* only have gamma parameter, not ramps. */
        {
          if (!XF86VidModeGetGamma(dpy, screen, &info[screen].vmg))
            goto FAIL;
        }
# ifdef HAVE_XF86VMODE_GAMMA_RAMP
      else if (ext_ok == 2)  /* have ramps */
        {
          if (!XF86VidModeGetGammaRampSize(dpy, screen, &info[screen].size))
            goto FAIL;
          if (info[screen].size <= 0)
            goto FAIL;

          info[screen].r = (unsigned short *)
            calloc(info[screen].size, sizeof(unsigned short));
          info[screen].g = (unsigned short *)
            calloc(info[screen].size, sizeof(unsigned short));
          info[screen].b = (unsigned short *)
            calloc(info[screen].size, sizeof(unsigned short));

          if (!(info[screen].r && info[screen].g && info[screen].b))
            goto FAIL;

          if (!XF86VidModeGetGammaRamp(dpy, screen, info[screen].size,
                                       info[screen].r,
                                       info[screen].g,
                                       info[screen].b))
            goto FAIL;
        }
# endif /* HAVE_XF86VMODE_GAMMA_RAMP */
    }

  gettimeofday(&then, &tzp);

  /* Iterate by steps of the animation... */
  for (i = (out_p ? steps : 0);
       (out_p ? i > 0 : i < steps);
       (out_p ? i-- : i++))
    {
      for (screen = 0; screen < nscreens; screen++)
	{
	  xf86_whack_gamma(dpy, screen, &info[screen],
                           (((float)i) / ((float)steps)));

	  /* If there is user activity, bug out.  (Bug out on keypresses or
	     mouse presses, but not motion, and not release events.  Bugging
	     out on motion made the unfade hack be totally useless, I think.)

	     We put the event back so that the calling code can notice it too.
	     It would be better to not remove it at all, but that's harder
	     because Xlib has such a non-design for this kind of crap, and
	     in this application it doesn't matter if the events end up out
	     of order, so in the grand unix tradition we say "fuck it" and
	     do something that mostly works for the time being.
	   */
	  if (XCheckMaskEvent (dpy, (KeyPressMask|ButtonPressMask),
			       &dummy_event))
	    {
	      XPutBackEvent (dpy, &dummy_event);
	      goto DONE;
	    }

	  gettimeofday(&now, &tzp);

	  /* If we haven't already used up our alotted time, sleep to avoid
	     changing the colormap too fast. */
	  {
	    long diff = (((now.tv_sec - then.tv_sec) * 1000000) +
			 now.tv_usec - then.tv_usec);
	    then.tv_sec = now.tv_sec;
	    then.tv_usec = now.tv_usec;
	    if (usecs_per_step > diff)
	      usleep (usecs_per_step - diff);
	  }
	}
    }
  

 DONE:

  if (out_p && black_windows)
    {
      kdDebug() << "in out" << endl;
      for (screen = 0; screen < nwindows; screen++)
	{
	  if (clear_windows)
	    XClearWindow (dpy, black_windows[screen]);
	  XMapRaised (dpy, black_windows[screen]);
	}
      XSync(dpy, False);
    }
    else
    {
     kdDebug() << "not executing code loop" << endl;
    }

  /* I can't explain this; without this delay, we get a flicker.
     I suppose there's some lossage with stale bits being in the
     hardware frame buffer or something, and this delay gives it
     time to flush out.  This sucks! */
  //usleep(100000);  /* 1/10th second */
    for (screen = 0; screen < nscreens; screen++)
      xf86_whack_gamma(dpy, screen, &info[screen], 1.0);
  XSync(dpy, False);

  status = 0;

 FAIL:
  if (info)
    {
      for (screen = 0; screen < nscreens; screen++)
        {
          if (info[screen].r) free(info[screen].r);
          if (info[screen].g) free(info[screen].g);
          if (info[screen].b) free(info[screen].b);
        }
      free(info);
    }

  return status;
}


/* XFree doesn't let you set gamma to a value smaller than this.
   Apparently they didn't anticipate the trick I'm doing here...
 */
#define XF86_MIN_GAMMA  0.1


Bool
xf86_whack_gamma(Display *dpy, int screen, xf86_gamma_info *info,
                 float ratio)
{
  Bool status;

  if (ratio < 0) ratio = 0;
  if (ratio > 1) ratio = 1;

  if (info->size == 0)    /* we only have a gamma number, not a ramp. */
    {
      XF86VidModeGamma g2;

      g2.red   = info->vmg.red   * ratio;
      g2.green = info->vmg.green * ratio;
      g2.blue  = info->vmg.blue  * ratio;

# ifdef XF86_MIN_GAMMA
      if (g2.red   < XF86_MIN_GAMMA) g2.red   = XF86_MIN_GAMMA;
      if (g2.green < XF86_MIN_GAMMA) g2.green = XF86_MIN_GAMMA;
      if (g2.blue  < XF86_MIN_GAMMA) g2.blue  = XF86_MIN_GAMMA;
# endif

      status = XF86VidModeSetGamma (dpy, screen, &g2);
    }
  else
    {
# ifdef HAVE_XF86VMODE_GAMMA_RAMP

      unsigned short *r, *g, *b;
      int i;
      r = (unsigned short *) malloc(info->size * sizeof(unsigned short));
      g = (unsigned short *) malloc(info->size * sizeof(unsigned short));
      b = (unsigned short *) malloc(info->size * sizeof(unsigned short));

      for (i = 0; i < info->size; i++)
        {
          r[i] = info->r[i] * ratio;
          g[i] = info->g[i] * ratio;
          b[i] = info->b[i] * ratio;
        }

      status = XF86VidModeSetGammaRamp(dpy, screen, info->size, r, g, b);

      free (r);
      free (g);
      free (b);

# endif /* !HAVE_XF86VMODE_GAMMA_RAMP */
    }

  XSync(dpy, False);
  return status;
}


/* This bullshit is needed because the VidMode extension doesn't work
   on remote displays -- but if the remote display has the extension
   at all, XF86VidModeQueryExtension returns true, and then
   XF86VidModeQueryVersion dies with an X error.  Thank you XFree,
   may I have another.
 */

static Bool error_handler_hit_p = False;

static int
ignore_all_errors_ehandler (Display *dpy, XErrorEvent *error)
{
  error_handler_hit_p = True;
  return 0;
}

static Bool
safe_XF86VidModeQueryVersion (Display *dpy, int *majP, int *minP)
{
  Bool result;
  XErrorHandler old_handler;
  XSync (dpy, False);
  error_handler_hit_p = False;
  old_handler = XSetErrorHandler (ignore_all_errors_ehandler);

  result = XF86VidModeQueryVersion (dpy, majP, minP);

  XSync (dpy, False);
  XSetErrorHandler (old_handler);
  XSync (dpy, False);

  return (error_handler_hit_p
          ? False
          : result);
}



/* VidModeExtension version 2.0 or better is needed to do gamma.
   2.0 added gamma values; 2.1 added gamma ramps.
 */
# define XF86_VIDMODE_GAMMA_MIN_MAJOR 2
# define XF86_VIDMODE_GAMMA_MIN_MINOR 0
# define XF86_VIDMODE_GAMMA_RAMP_MIN_MAJOR 2
# define XF86_VIDMODE_GAMMA_RAMP_MIN_MINOR 1


/* Returns 0 if gamma fading not available; 1 if only gamma value setting
   is available; 2 if gamma ramps are available.
 */
int
xf86_check_gamma_extension (Display *dpy)
{
  int event, error, major, minor;

  if (!XF86VidModeQueryExtension (dpy, &event, &error))
    return 0;  /* display doesn't have the extension. */

  if (!safe_XF86VidModeQueryVersion (dpy, &major, &minor))
    return 0;  /* unable to get version number? */

  if (major < XF86_VIDMODE_GAMMA_MIN_MAJOR || 
      (major == XF86_VIDMODE_GAMMA_MIN_MAJOR &&
       minor < XF86_VIDMODE_GAMMA_MIN_MINOR))
    return 0;  /* extension is too old for gamma. */

  if (major < XF86_VIDMODE_GAMMA_RAMP_MIN_MAJOR || 
      (major == XF86_VIDMODE_GAMMA_RAMP_MIN_MAJOR &&
       minor < XF86_VIDMODE_GAMMA_RAMP_MIN_MINOR))
    return 1;  /* extension is too old for gamma ramps. */

  /* Copacetic */
  return 2;
}


#endif /* HAVE_XF86VMODE_GAMMA */

#ifndef __RANDR_H__
#define __RANDR_H__

extern "C"
{
#include <X11/Xlib.h>
#define INT8 _X11INT8
#define INT32 _X11INT32
#include <X11/Xproto.h>
#undef INT8
#undef INT32
#include <X11/extensions/Xrandr.h>

#if RANDR_MAJOR > 1 || (RANDR_MAJOR == 1 && RANDR_MINOR >= 2)
#define HAS_RANDR_1_2 1
#endif
}

namespace RandR
{
	extern bool has_1_2;
}

#endif

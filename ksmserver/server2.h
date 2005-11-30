// This is in a separate file only because dcopidl doesn't handle
// the extern "C" { ... } construct.

#define INT32 QINT32
#include <X11/Xlib.h>
#include <X11/Xmd.h>
#include <X11/ICE/ICElib.h>
extern "C" {
#include <X11/ICE/ICEutil.h>
#include <X11/ICE/ICEmsg.h>
#include <X11/ICE/ICEproto.h>
#include <X11/SM/SM.h>
#include <X11/SM/SMlib.h>
}

#include <fixx11h.h>

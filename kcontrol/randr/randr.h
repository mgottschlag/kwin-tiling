#ifndef __RANDR_H__
#define __RANDR_H__

#include <QString>
#include <QPixmap>

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

class RandR
{
public:
	static bool has_1_2;

	enum Orientations {
		Rotate0			= 0x1,
		Rotate90		= 0x2,
		Rotate180		= 0x4,
		Rotate270		= 0x8,
		RotateMask		= 15,
		RotationCount	= 4,
		ReflectX		= 0x10,
		ReflectY		= 0x20,
		ReflectMask		= 48,
		OrientationMask	= 63,
		OrientationCount = 6
	};

	static QString	rotationName(int rotation, bool pastTense = false, bool capitalised = true);
	
	static QPixmap	rotationIcon(int rotation, int currentRotation);

};

#endif

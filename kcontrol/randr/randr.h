/*
 * Copyright (c) 2007      Gustavo Pichorim Boiko <gustavo.boiko@kdemail.net>
 * Copyright (c) 2002,2003 Hamish Rodda <rodda@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef __RANDR_H__
#define __RANDR_H__

#include <KDebug>
#include <KLocale>
#include <QString>
#include <QPixmap>
#include <config-randr.h>
#include "ktimerdialog.h"

extern "C"
{
#include <X11/Xlib.h>
#define INT8 _X11INT8
#define INT32 _X11INT32
#include <X11/Xproto.h>
#undef INT8
#undef INT32
#include <X11/extensions/Xrandr.h>
}

#ifdef HAS_RANDR_1_2
class RandRScreen;
class RandRCrtc;
class RandROutput;
class RandRMode;

// maps
typedef QMap<RRCrtc,RandRCrtc*> CrtcMap;
typedef QMap<RROutput,RandROutput*> OutputMap;
typedef QMap<RRMode,RandRMode> ModeMap;

//lists
typedef QList<RandRScreen*> ScreenList;
typedef QList<RROutput> OutputList;
typedef QList<RRCrtc> CrtcList;
typedef QList<RRMode> ModeList;
#endif

typedef QList<float> RateList;
typedef QList<QSize> SizeList;

class LegacyRandRScreen;
typedef QList<LegacyRandRScreen*> LegacyScreenList;

class RandR
{
public:
	static bool has_1_2;
	static Time timestamp;

	enum Orientations 
	{
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

	enum Changes 
	{
		ChangeCrtc       = 0x01,
		ChangeOutputs    = 0x02,
		ChangeMode       = 0x04,
		ChangeRotation   = 0x08,
		ChangeConnection = 0x10,
		ChangeRect       = 0x20,
		ChangeRate       = 0x40
	};

	static QString rotationName(int rotation, bool pastTense = false, bool capitalised = true);
	static QPixmap rotationIcon(int rotation, int currentRotation);

	static bool confirm(const QRect &rect = QRect());

	static SizeList sortSizes(const SizeList &sizes);
};

#endif

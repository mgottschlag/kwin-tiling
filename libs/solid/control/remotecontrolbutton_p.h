/*
    Copyright (C) <2009>  Michael Zanetti <michael_zanetti@gmx.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*/

#ifndef SOLID_REMOTECONTROLBUTTON_P_H
#define SOLID_REMOTECONTROLBUTTON_P_H

#include "remotecontrolbutton.h"

#include <QSharedData>
#include <QString>

namespace Solid
{
namespace Control
{
	class RemoteControlButtonPrivate: public QSharedData
	{
	public:
		RemoteControlButtonPrivate() {
			id = RemoteControlButton::Unknown;
			remoteName.clear();
			name.clear();
			repeatCounter = -1;
		};
		
		RemoteControlButtonPrivate(const RemoteControlButtonPrivate &other) : QSharedData(other)
					, remoteName(other.remoteName), id(other.id), name(other.name), repeatCounter(other.repeatCounter) {};
		
		QString remoteName;
		Solid::Control::RemoteControlButton::ButtonId id;
		QString name;
		int repeatCounter;
	};
}
}

#endif // SOLID_REMOTECONTROLBUTTON_P_H
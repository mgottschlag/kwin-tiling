/*
 *  Copyright (C) 2007 Andriy Rysin (rysin@kde.org)
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

#ifndef XKLAVIER_ADAPTOR_H
#define XKLAVIER_ADAPTOR_H

#include <X11/Xlib.h>

#include <QHash>
#include <QList>


class LayoutUnit;
class XKlavierAdaptorPriv;

class XKlavierAdaptor {

public:
	~XKlavierAdaptor();	
		
	void loadXkbConfig(bool layoutsOnly);

	QHash<QString, QString> getModels();
	QHash<QString, QString> getLayouts();
	QHash<QString, XkbOption> getOptions();
	QHash<QString, XkbOptionGroup> getOptionGroups();
	QHash<QString, QStringList*> getVariants();

	QList<LayoutUnit> getGroupNames();
	int filterEvents(XEvent* ev);
	int startListening();
	
	static XKlavierAdaptor* getInstance(Display* dpy);
		
private:
	XKlavierAdaptor(Display* dpy);

	XKlavierAdaptorPriv* priv;
};

#endif

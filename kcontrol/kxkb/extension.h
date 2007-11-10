/*
 *  Copyright (C) 2003-2006 Andriy Rysin (rysin@kde.org)
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

#ifndef __EXTENSION_H__
#define __EXTENSION_H__

#include <X11/Xlib.h>

class QString;

class XKBExtension
{
public:
    XKBExtension(Display *display=NULL);
    ~XKBExtension();
    bool init();


    bool setGroup(unsigned int group);
    unsigned int getGroup() const;

    static bool setXkbOptions(const QStringList& options, bool resetOldOptions);
    static bool setLayoutGroups(const QString& model, const QStringList& layouts, const QStringList& variants,
            const QStringList& options=QStringList(), bool resetOldOptions=true);

    static QString getXkbOptionsCommand(const QStringList& options, bool resetOldOptions);
    static QString getLayoutGroupsCommand(const QString& model, const QStringList& layouts, const QStringList& variants);

    static bool isGroupSwitchEvent(XEvent* event);
    static bool isLayoutSwitchEvent(XEvent* event);	

    bool isXkbEvent(XEvent* event) { return event->type == xkb_opcode; }

private:
    int xkb_opcode;

    Display *m_dpy;

    static QString m_setxkbmap_exe;
    static QString getSetxkbmapExe();
};

#endif

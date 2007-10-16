/*
 *  Copyright (C) 2006 Andriy Rysin (rysin@kde.org)
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

#ifndef X11HELPER_H_
#define X11HELPER_H_

#include <X11/Xlib.h>
#include <QHash>

struct XkbOption;

struct XkbOptionGroup {
    QString name;
    QString description;
    bool exclusive;
    QList<XkbOption> options;
};

struct XkbOption {
    QString name;
    QString description;
    XkbOptionGroup* group;
};

struct RulesInfo {
    QHash<QString, QString> models;
    QHash<QString, QString> layouts;
    QHash<QString, XkbOption> options;
    QHash<QString, XkbOptionGroup> optionGroups;
};


class X11Helper
{
public:
	static const Window UNKNOWN_WINDOW_ID = (Window) 0;
	static const QString X11_WIN_CLASS_ROOT;
	static const QString X11_WIN_CLASS_UNKNOWN;

	static QString getWindowClass(Window winId, Display* dpy);

#ifndef HAVE_XKLAVIER
	/**
	 * Tries to find X11 xkb config dir
	 */
	static const QString findX11Dir();
	static const QString findXkbRulesFile(const QString &x11Dir, Display* dpy);
	static QStringList* getVariants(const QString& layout, const QString& x11Dir);
	static RulesInfo* loadRules(const QString& rulesFile, bool layoutsOnly=false);
private:

	static XkbOptionGroup createMissingGroup(const QString& groupName);
	static bool isGroupExclusive(const QString& groupName);
#endif

};

#endif /*X11HELPER_H_*/

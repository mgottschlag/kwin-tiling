/*
 *  Copyright (C) 2010 Andriy Rysin (rysin@kde.org)
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


#ifndef KEYBOARD_DAEMON_H_
#define KEYBOARD_DAEMON_H_

#include <kdedmodule.h>
#include <QtCore/QStringList>

#include "layout_memory.h"
#include "keyboard_dbus.h"


class KActionCollection;
class XInputEventNotifier;
class LayoutTrayIcon;
class KeyboardConfig;


class KDE_EXPORT KeyboardDaemon : public KDEDModule
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.KeyboardLayouts")

	KeyboardConfig* keyboardConfig;
    KActionCollection *actionCollection;
    XInputEventNotifier* xEventNotifier;
    LayoutTrayIcon* layoutTrayIcon;
    LayoutMemory layoutMemory;
    LayoutUnit currentLayout;
    QObject* oldDbusApiObject;

    void registerListeners();
    void registerShortcut();
    void unregisterListeners();
    void unregisterShortcut();
    void setupTrayIcon();

private Q_SLOTS:
	void switchToNextLayout();
	void globalSettingsChanged(int category);
    void configureKeyboard();
    void configureMouse();
    void layoutChanged();

public Q_SLOTS:
	Q_SCRIPTABLE bool setLayout(const QString& layout);
	Q_SCRIPTABLE QString getCurrentLayout();
	Q_SCRIPTABLE QStringList getLayoutsList();

Q_SIGNALS:
	Q_SCRIPTABLE void currentLayoutChanged(QString layout);

public:
    KeyboardDaemon(QObject *parent, const QList<QVariant>&);
    virtual ~KeyboardDaemon();
};

// to support old org.kde.kxkb dbus API (will be removed in 4.7)
class QDBusConnection;
class KDE_EXPORT OldDbusKeyboardDaemon : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.KXKB")
    KeyboardDaemon& actor;

    void warn();
public:
    OldDbusKeyboardDaemon(KeyboardDaemon& daemon):
    	actor(daemon) {}
    void registerApi(QDBusConnection& dbus);
    void unregisterApi(QDBusConnection& dbus);

public Q_SLOTS:
	Q_SCRIPTABLE bool setLayout(const QString& layout) { warn(); return actor.setLayout(layout); }
	Q_SCRIPTABLE QString getCurrentLayout() { warn(); return actor.getCurrentLayout(); }
	Q_SCRIPTABLE QStringList getLayoutsList() { warn(); return actor.getLayoutsList(); }
};

#endif /* KEYBOARD_DAEMON_H_ */

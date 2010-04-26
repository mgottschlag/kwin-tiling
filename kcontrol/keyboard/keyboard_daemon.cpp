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

#include "keyboard_daemon.h"

#include <QtGui/QX11Info>
#include <QtDBus/QtDBus>

#include <kdebug.h>
#include <kpluginfactory.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kglobalsettings.h>
#include <klocale.h>

#include "x11_helper.h"
#include "xkb_helper.h"
#include "keyboard_dbus.h"

// for sys tray icon
#include "layout_widget.h"


K_PLUGIN_FACTORY(KeyboardFactory, registerPlugin<KeyboardDaemon>();)
K_EXPORT_PLUGIN(KeyboardFactory("keyboard"))

KeyboardDaemon::KeyboardDaemon(QObject *parent, const QList<QVariant>&)
	: KDEDModule(parent),
	  actionCollection(NULL),
	  xEventNotifier(NULL)
{
	if( ! X11Helper::xkbSupported(NULL) )
		return;		//TODO: shut down the daemon?

    QDBusConnection dbus = QDBusConnection::sessionBus();
	dbus.registerService(KEYBOARD_DBUS_SERVICE_NAME);
	dbus.registerObject(KEYBOARD_DBUS_OBJECT_PATH, this, QDBusConnection::ExportScriptableSlots);
    dbus.connect(QString(), KEYBOARD_DBUS_OBJECT_PATH, KEYBOARD_DBUS_SERVICE_NAME, KEYBOARD_DBUS_CONFIG_RELOAD_MESSAGE, this, SLOT( configureKeyboard() ));

	configureKeyboard();
	registerListeners();
	registerShortcut();

	layoutTrayIcon = new LayoutTrayIcon();
}

KeyboardDaemon::~KeyboardDaemon()
{
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.disconnect(QString(), KEYBOARD_DBUS_OBJECT_PATH, KEYBOARD_DBUS_SERVICE_NAME, KEYBOARD_DBUS_CONFIG_RELOAD_MESSAGE, this, SLOT( configureKeyboard() ));
	dbus.unregisterObject(KEYBOARD_DBUS_OBJECT_PATH);
	dbus.unregisterService(KEYBOARD_DBUS_SERVICE_NAME);

	unregisterListeners();
	unregisterShortcut();

	delete xEventNotifier;

	if( layoutTrayIcon ) {
		delete layoutTrayIcon;
	}
}

void KeyboardDaemon::configureKeyboard()
{
	KeyboardConfig config;
	config.load();
	XkbHelper::initializeKeyboardLayouts(config);
	layoutMemory.setSwitchingPolicy(config.switchingPolicy);
}

void KeyboardDaemon::registerShortcut()
{
	if( actionCollection == NULL ) {
         actionCollection = new KActionCollection( this );
         KAction* a = NULL;
#include "bindings.cpp"
         connect(a, SIGNAL(triggered()), this, SLOT(switchToNextLayout()));
         connect(KGlobalSettings::self(), SIGNAL(settingsChanged(int)), this, SLOT(globalSettingsChanged(int)));
         kDebug() << "keyboard switching KDE shortcut" << a->globalShortcut().toString();
     }
}

void KeyboardDaemon::unregisterShortcut()
{
	// register KDE keyboard shortcut for switching layouts
    if( actionCollection != NULL ) {
        KAction* kAction = static_cast<KAction*>(actionCollection->action(0));
        disconnect(kAction, SIGNAL(triggered()), this, SLOT(switchToNextLayout()));
        disconnect(KGlobalSettings::self(), SIGNAL(settingsChanged(int)), this, SLOT(globalSettingsChanged(int)));
        actionCollection->clear();
        delete actionCollection;
        actionCollection = NULL;
    }
}

void KeyboardDaemon::registerListeners()
{
	//TODO: list for config changes
	// connect(SIGNAL(configChanges), SLOT(configureKeyboard));

	//TODO: use solid ???
	if( xEventNotifier == NULL ) {
		xEventNotifier = new XEventNotifier(XEventNotifier::ALL);
	}
	connect(xEventNotifier, SIGNAL(newDevice()), this, SLOT(configureKeyboard()));
	connect(xEventNotifier, SIGNAL(layoutChanged()), &layoutMemory, SLOT(layoutChanged()));
	connect(xEventNotifier, SIGNAL(layoutMapChanged()), &layoutMemory, SLOT(clear()));
	xEventNotifier->start();
}

void KeyboardDaemon::unregisterListeners()
{
	//TODO: unlist for config changes
	// disconnect(SIGNAL(configChanges), SLOT(configureKeyboard));

	xEventNotifier->stop();
	disconnect(xEventNotifier, SIGNAL(newDevice()), this, SLOT(configureKeyboard()));
	disconnect(xEventNotifier, SIGNAL(layoutChanged()), &layoutMemory, SLOT(layoutChanged()));
	disconnect(xEventNotifier, SIGNAL(layoutMapChanged()), &layoutMemory, SLOT(clear()));
}

void KeyboardDaemon::globalSettingsChanged(int category)
{
	if ( category == KGlobalSettings::SETTINGS_SHORTCUTS) {
		// TODO: can we do it more efficient or recreating action collection is the only way?
		unregisterShortcut();
		registerShortcut();
	}
}

void KeyboardDaemon::switchToNextLayout()
{
	X11Helper::switchToNextLayout();
}

bool KeyboardDaemon::setLayout(const QString& layout)
{
	return X11Helper::setLayout(layout);
}

QString KeyboardDaemon::getCurrentLayout()
{
	return X11Helper::getCurrentLayout();
}

QStringList KeyboardDaemon::getLayoutsList()
{
	return X11Helper::getLayoutsList();
}

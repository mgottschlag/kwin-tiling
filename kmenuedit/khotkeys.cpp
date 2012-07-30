/*
 *   Copyright (C) 2000 Matthias Elter <elter@kde.org>
 *                      Lubos Lunak    <l.lunak@email.cz>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "khotkeys.h"
#include "khotkeys_interface.h"

#include <KDebug>
#include <KLocale>
#include <KMessageBox>

static bool khotkeys_present = false;
static bool khotkeys_inited = false;
static OrgKdeKhotkeysInterface *khotkeysInterface = NULL;


bool KHotKeys::init()
{
    khotkeys_inited = true;

    // Check if khotkeys is running
    QDBusConnection bus = QDBusConnection::sessionBus();
    khotkeysInterface = new OrgKdeKhotkeysInterface(
        "org.kde.kded",
        "/modules/khotkeys",
        bus,
        NULL);

    if(!khotkeysInterface->isValid()) {
        QDBusError err = khotkeysInterface->lastError();
        if (err.isValid()) {
            kError() << err.name() << ":" << err.message();
        }
        KMessageBox::error(
            NULL,
            "<qt>" + i18n("Unable to contact khotkeys. Your changes are saved, but they could not be activated.") + "</qt>" );
    }

    khotkeys_present = khotkeysInterface->isValid();
    return true;
}

void KHotKeys::cleanup()
{
    if( khotkeys_inited && khotkeys_present ) {
        // CleanUp ???
    }

    khotkeys_inited = false;
}

bool KHotKeys::present()
{
    if( !khotkeys_inited )
        init();

    return khotkeys_present;
}

QString KHotKeys::getMenuEntryShortcut( const QString& entry_P )
{
    if( !khotkeys_inited )
        init();

    if( !khotkeys_present || !khotkeysInterface->isValid())
        return "";

    QDBusReply<QString> reply = khotkeysInterface->get_menuentry_shortcut(entry_P);
    if (!reply.isValid()) {
        kError() << reply.error();
        return "";

    } else {
        return reply;
    }
}

QString KHotKeys::changeMenuEntryShortcut(
        const QString& entry_P,
        const QString shortcut_P )
{
    if( !khotkeys_inited )
        init();

    if( !khotkeys_present || !khotkeysInterface->isValid())
        return "";

    QDBusReply<QString> reply = khotkeysInterface->register_menuentry_shortcut(
            entry_P,
            shortcut_P);

    if (!reply.isValid()) {
        kError() << reply.error();
        return "";
    } else {
        return reply;
    }
}


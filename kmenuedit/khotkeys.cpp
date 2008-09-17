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


#include "kdebug.h"
#include "kmessagebox.h"

static bool khotkeys_present = false;
static bool khotkeys_inited = false;

bool KHotKeys::init()
{
    khotkeys_inited = true;

    // Check if khotkeys is running
    QDBusConnection bus = QDBusConnection::sessionBus();
    OrgKdeKhotkeysInterface khotkeysInterface(
        "org.kde.kded",
        "/modules/khotkeys",
        bus,
        NULL);

    QDBusError err;
    if(!khotkeysInterface.isValid())
        {
        err = khotkeysInterface.lastError();
        if (err.isValid())
            {
            kError() << err.name() << ":" << err.message();
            }
        KMessageBox::error(
            NULL,
            "<qt>" + i18n("Unable to contact khotkeys. Your changes are saved but i failed to activate them") + "</qt>" );
        }

    khotkeys_present = khotkeysInterface.isValid();
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
    kDebug() << khotkeys_present;

    if( !khotkeys_inited )
        init();
    return khotkeys_present;
}

QString KHotKeys::getMenuEntryShortcut( const QString& entry_P )
{
    if( !khotkeys_inited )
        init();
    if( !khotkeys_present )
        return "";
    // TODO
    return "";
}

QString KHotKeys::changeMenuEntryShortcut(
        const QString& entry_P,
        const QString shortcut_P )
    {
    if( !khotkeys_inited )
        init();
    if( !khotkeys_present )
        return "";

    kDebug() << entry_P << "," << shortcut_P;
    return "";
    }

bool KHotKeys::menuEntryMoved( const QString& new_P, const QString& old_P )
{
    if( !khotkeys_inited )
        init();
    if( !khotkeys_present )
        return false;

    // For now so i don't forget to check this
    Q_ASSERT(false);
    return false;
}

void KHotKeys::menuEntryDeleted( const QString& entry_P )
{
    if( !khotkeys_inited )
        init();
    if( !khotkeys_present )
        return;
    // TODO
    Q_ASSERT(false);
}

QStringList KHotKeys::allShortCuts( )
{
    if( !khotkeys_inited )
        init();
    // TODO
    Q_ASSERT(false);
    return QStringList();
}

KService::Ptr KHotKeys::findMenuEntry( const QString &shortcut_P )
{
    if( !khotkeys_inited )
        init();
    // TODO
    Q_ASSERT(false);
    return KService::Ptr();
}

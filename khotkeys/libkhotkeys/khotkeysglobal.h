/****************************************************************************

 KHotKeys

 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.

****************************************************************************/

#ifndef _KHOTKEYSGLOBAL_H_
#define _KHOTKEYSGLOBAL_H_

#define KHOTKEYS_VERSION "2.1"
#define KHOTKEYS_CONFIG_FILE "khotkeysrc"

//#ifndef NDEBUG
//#define KHOTKEYS_DEBUG
//#endif



#include <klocale.h>

#include <QtCore/QPointer>

class QObject;

namespace KHotKeys
{

class Windows;
class ShortcutsHandler;

extern QPointer<ShortcutsHandler> keyboard_handler;
extern QPointer<Windows> windows_handler;

// CHECKME hmms :(
KDE_EXPORT bool khotkeys_active();
KDE_EXPORT void khotkeys_set_active( bool active_P );

QString get_menu_entry_from_path( const QString& path_P );

KDE_EXPORT void init_global_data( bool active_P, QObject* owner_P );

const char* const MENU_EDITOR_ENTRIES_GROUP_NAME = I18N_NOOP( "Menu Editor entries" );

KDE_EXPORT bool haveArts();
KDE_EXPORT void disableArts();

//***************************************************************************
// Inline
//***************************************************************************

} // namespace KHotKeys

#endif

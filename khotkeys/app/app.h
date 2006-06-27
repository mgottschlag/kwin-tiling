/****************************************************************************

 KHotKeys
 
 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.
 
****************************************************************************/

#ifndef _KHOTKEYS_APP_H_
#define _KHOTKEYS_APP_H_

#include <kuniqueapplication.h>
#include <QtCore/QObject>
#include <dbus/qdbus.h>

namespace KHotKeys
{

class Action_data_group;

class KHotKeysApp
    : public KUniqueApplication
    {
    Q_OBJECT
    public Q_SLOTS:
        Q_ASYNC void reread_configuration();
        Q_ASYNC void quit(); 
    public:
        KHotKeysApp();
        virtual ~KHotKeysApp();
    private:
        Action_data_group* actions_root;
        QObject* delete_helper;
    };

//***************************************************************************
// Inline
//***************************************************************************

} // namespace KHotKeys

#endif

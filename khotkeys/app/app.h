/****************************************************************************

 KHotKeys
 
 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.
 
****************************************************************************/

#ifndef _KHOTKEYS_APP_H_
#define _KHOTKEYS_APP_H_

#include <kuniqueapplication.h>
#include <QtCore/QObject>
#include <QtDBus/QtDBus>

namespace KHotKeys
{

class Action_data_group;

class KHotKeysApp
    : public KUniqueApplication
    {
    Q_OBJECT
    public Q_SLOTS:
        Q_NOREPLY void reread_configuration();
        Q_NOREPLY void quit(); 
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

/****************************************************************************

 KHotKeys
 
 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.
 
****************************************************************************/

#ifndef _KHOTKEYS_KDED_H_
#define _KHOTKEYS_KDED_H_

#include <kdedmodule.h>
#include <QtCore/QObject>
#include <dbus/qdbus.h>


namespace KHotKeys
{

class Action_data_group;

class KHotKeysModule
    : public KDEDModule
    {
    Q_OBJECT
    public Q_SLOTS:
        Q_ASYNC void reread_configuration();
        Q_ASYNC void quit(); 
    public:
        KHotKeysModule( );
        virtual ~KHotKeysModule();
    private:
        Action_data_group* actions_root;
    };

//***************************************************************************
// Inline
//***************************************************************************

} // namespace KHotKeys

#endif

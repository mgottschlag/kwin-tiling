/****************************************************************************

 KHotKeys
 
 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.
 
****************************************************************************/

#ifndef _KHOTKEYS_KDED_H_
#define _KHOTKEYS_KDED_H_

#include <kdedmodule.h>
#include <QtCore/QObject>
#include <QtDBus/QtDBus>


namespace KHotKeys
{

class Action_data_group;

class KHotKeysModule
    : public KDEDModule
    {
    Q_OBJECT
    public Q_SLOTS:
        Q_NOREPLY void reread_configuration();
        Q_NOREPLY void quit(); 
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

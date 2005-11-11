/****************************************************************************

 KHotKeys
 
 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.
 
****************************************************************************/

#ifndef _KHOTKEYS_KDED_H_
#define _KHOTKEYS_KDED_H_

#include <kdedmodule.h>
#include <dcopclient.h>

namespace KHotKeys
{

class Action_data_group;

class KHotKeysModule
    : public KDEDModule
    {
    Q_OBJECT
    K_DCOP
    k_dcop:
        ASYNC reread_configuration();
        ASYNC quit(); 
    public:
        KHotKeysModule( const DCOPCString& obj );
        virtual ~KHotKeysModule();
    private:
        Action_data_group* actions_root;
        DCOPClient client;
    };

//***************************************************************************
// Inline
//***************************************************************************

} // namespace KHotKeys

#endif

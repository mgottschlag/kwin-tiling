/****************************************************************************

 KHotKeys
 
 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.
 
****************************************************************************/

#ifndef _KHOTKEYS_H_
#define _KHOTKEYS_H_

#include <kuniqueapplication.h>

#include <actions.h>

namespace KHotKeys
{

class KHotKeysApp
    : public KUniqueApplication
    {
    Q_OBJECT
    K_DCOP
    k_dcop:
        virtual ASYNC reread_configuration();
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

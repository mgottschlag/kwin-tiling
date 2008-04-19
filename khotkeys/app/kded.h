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

class KhotkeysAdaptor;

namespace KHotKeys
{
class Action_data_group;
}

class KHotKeysModule
    : public KDEDModule
    {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.khotkeys")
    public Q_SLOTS:
        Q_NOREPLY void reread_configuration();
        Q_NOREPLY void quit();
    public:
        KHotKeysModule(QObject* parent, const QList<QVariant>&);
        virtual ~KHotKeysModule();
    private:
        KHotKeys::Action_data_group* actions_root;
        KhotkeysAdaptor *dbus_adaptor;
    };

//***************************************************************************
// Inline
//***************************************************************************


#endif

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
#include <QtGui/QKeySequence>

#include <KService>

namespace KHotKeys
{
class ActionDataGroup;
}

class KHotKeysModule
    : public KDEDModule
    {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.khotkeys")
    public Q_SLOTS:
        Q_NOREPLY void reread_configuration();
        Q_NOREPLY void quit();

        /**
         * Register an shortcut for service @serviceStorageId with the key
         * sequence @seq.
         *
         * @param serviceStorageId the KService::storageId of the service
         * @param sequence         the key sequence to use
         *
         * @returns @c true if the key sequence was successfully set, @c if
         * the sequence is not available.
         */
        QString register_menuentry_shortcut(const QString &storageId, const QString &sequence);

        /**
         * Get the currently active shortcut for service @p serviceStorageId.
         *
         * @param serviceStorageId the KService::storageId of the service
         *
         * @returns the active global shortcuts for that service
         */
        QString get_menuentry_shortcut(const QString &storageId) const;

    public:
        KHotKeysModule(QObject* parent, const QList<QVariant>&);
        virtual ~KHotKeysModule();
    private:
        KHotKeys::ActionDataGroup* actions_root;
    };

//***************************************************************************
// Inline
//***************************************************************************


#endif

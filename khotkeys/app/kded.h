/****************************************************************************

 KHotKeys

 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.

****************************************************************************/

#ifndef _KHOTKEYS_KDED_H_
#define _KHOTKEYS_KDED_H_

#include <kdedmodule.h>
#include "settings.h"

#include <QtCore/QObject>
#include <QtDBus/QtDBus>
#include <QtGui/QKeySequence>

#include <KService>

namespace KHotKeys
    {
    class ActionDataGroup;
    class SimpleActionData;
    }

class KHotKeysModule
    : public KDEDModule
    {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.khotkeys")

    public Q_SLOTS:

        Q_SCRIPTABLE Q_NOREPLY void reread_configuration();

        Q_SCRIPTABLE Q_NOREPLY void quit();

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
        Q_SCRIPTABLE QString register_menuentry_shortcut(const QString &storageId, const QString &sequence);

        /**
         * Get the currently active shortcut for service @p serviceStorageId.
         *
         * @param serviceStorageId the KService::storageId of the service
         *
         * @returns the active global shortcuts for that service
         */
        Q_SCRIPTABLE QString get_menuentry_shortcut(const QString &storageId);

    private Q_SLOTS:

        //! Save
        void save();

        //! Initialize the module. Delayed initialization.
        void initialize();

    public:

        KHotKeysModule(QObject* parent, const QList<QVariant>&);
        virtual ~KHotKeysModule();

    private:

        //! The action list from _settings for convenience
        KHotKeys::ActionDataGroup* actions_root;

        //! The current settings
        KHotKeys::Settings _settings;

        //! Is the module initialized
        bool _initialized;

        /** 
         * @name Some method in need for a better home 
         */
        //@{
            //! Get the group for the menuentries. Will create it if needed
            KHotKeys::ActionDataGroup *menuentries_group();

            //! Find a menuentry_action for the service with @storageId in group @group
            KHotKeys::SimpleActionData *menuentry_action(const QString &storageId);
        //@}
    };

//***************************************************************************
// Inline
//***************************************************************************


#endif

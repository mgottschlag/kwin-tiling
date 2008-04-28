/*
   Copyright (C) 2008 Michael Jansen <kde@michael-jansen.biz>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kcm_hotkeys.h"


// ACTION_DATAS
#include "action_data_group.h"
// OUR ACTION WIDGETS
#include "global_settings_widget.h"
#include "action_group_widget.h"
#include "simple_action_data_widget.h"
// REST
#include "daemon/daemon.h"
#include "hotkeys_model.h"
#include "hotkeys_proxy_model.h"
#include "hotkeys_tree_view.h"
#include "khotkeysglobal.h"

#include <QtGui/QHBoxLayout>
#include <QtGui/QSplitter>
#include <QtGui/QStackedWidget>
#include <QtGui/QWidget>

#include <QtDBus/QtDBus>

#include <KDE/KAboutData>
#include <KDE/KDebug>
#include <KDE/KGlobalAccel>
#include <KDE/KLocale>
#include <KDE/KMessageBox>
#include <KDE/KPluginFactory>
#include <KDE/KPluginLoader>
#include <KDE/KToolInvocation>

K_PLUGIN_FACTORY(
    KCMHotkeysFactory,
    registerPlugin<KCMHotkeys>();
    )
K_EXPORT_PLUGIN(KCMHotkeysFactory("kcm_khotkeys"));


extern "C"
{
    KDE_EXPORT void kcminit_khotkeys()
        {
        kDebug() << "Starting khotkeys daemon";

        // If the daemon is not enabled there is nothing to do
        if (!KHotKeys::Daemon::isEnabled())
            {
            kDebug() << "KHotKeys daemon is disabled.";
            return;
            }

#if 0
        // BUG: 2008-04-13 mjansen
        // KGlobalAccel is currently unable to handle components correctly.
        //
        // 1. A action in a kded module allways belongs to the component kded. The
        // ActionCollections component is ignored. Our actions belong to khotkeys
        // and are known to khotkeys under that component.
        // 2. KGlobalAccel::overrideMainComponentData disables all actions
        // So we disable the kded module for now. _Whenever_ that is fixed we
        // could go back.
#endif
        KToolInvocation::kdeinitExec( "khotkeys" );

        } // kcminit_khotkeys()
}


class KCMHotkeysPrivate
    {
    public:

        KCMHotkeysPrivate( KCMHotkeys *host );

        // Treeview displaying the shortcuts
        QTreeView *treeView;

        /** The model holding the shortcut settings. Beware! There a proxy
         * between us and that model */
        KHotkeysModel *model;

        //! Our host
        KCMHotkeys *q;

        //! Container for all editing widgets
        QStackedWidget *stack;

        //! Widget to edit an action group
        ActionGroupWidget *action_group;

        //! The currently shown dialog
        HotkeysWidgetBase *current;

        //! The global settings dialog
        GlobalSettingsWidget *global_settings;

        SimpleActionDataWidget *simple_action;

        /**
         * Show the widget. If the current widget has changes allow
         * cancelation ! of this action
         */
        bool maybeShowWidget();

        /**
         * Save the currentely shown item
         */
        void saveCurrentItem();

        void load();
        void save();
    };


KCMHotkeys::KCMHotkeys( QWidget *parent, const QVariantList & /* args */ )
    : KCModule( KCMHotkeysFactory::componentData(), parent )
     ,d( new KCMHotkeysPrivate(this) )
    {
    // Inform KCModule of the buttons we support
    KCModule::setButtons(KCModule::Buttons(KCModule::Default | KCModule::Apply));

    // Add the about data
    KAboutData *about = new KAboutData(
        "khotkeys",
        0,
        ki18n("KDE Hotkeys Configuration Module"),
        KDE_VERSION_STRING,
        KLocalizedString(),
        KAboutData::License_GPL,
        ki18n("Copyright 2008 (c) Michael Jansen")
        );
    about->addAuthor(
        ki18n("Michael Jansen"),
        ki18n("Maintainer"),
        "kde@michael-jansen.biz" );
    setAboutData(about);

    // Tell KCModule we were changed.
    connect(
        d->action_group, SIGNAL(changed(bool)),
        this, SIGNAL(changed(bool)) );
    connect(
        d->simple_action, SIGNAL(changed(bool)),
        this, SIGNAL(changed(bool)) );

    // Inform KGlobalAccel we only want to configure shortcuts
    KGlobalAccel::self()->overrideMainComponentData( KComponentData("khotkeys") );

    // Load the settings
    load();
    }


void KCMHotkeys::currentChanged( const QModelIndex &pCurrent, const QModelIndex &pPrevious )
    {
    // We're not interested in changes of columns. Just compare the rows
    QModelIndex current =
        pCurrent.isValid()
        ? pCurrent.sibling( pCurrent.row(), 0 )
        : QModelIndex();
    QModelIndex previous =
        pPrevious.isValid()
        ? pPrevious.sibling( pPrevious.row(), 0 )
        : QModelIndex();

    // Now it's possible for previous and current to be the same
    if (current==previous)
        {
        return;
        }

    // Current and previous differ. Ask user if there are unsaved changes
    if ( !d->maybeShowWidget() )
        {
        return;
        }

    // If there is no current item, cleanup the dialog.
    if (!current.isValid())
        {
        d->current = 0;
        d->stack->setCurrentWidget(d->global_settings);
        return;
        }

    // Now go on and activate the new item;
    KHotKeys::ActionDataBase *item = d->model->indexToActionDataBase( current );
    QModelIndex typeOfIndex = d->model->index( current.row(), KHotkeysModel::TypeColumn, current.parent() );

    switch (d->model->data( typeOfIndex ).toInt())
        {

        case KHotkeysModel::SimpleActionData:
            {
            KHotKeys::SimpleActionData *data = dynamic_cast<KHotKeys::SimpleActionData*>(item);
            if (data)
                {
                d->simple_action->setActionData( data );
                d->current = d->simple_action;
                }
            }
            break;

        case KHotkeysModel::ActionDataGroup:
            {
            KHotKeys::ActionDataGroup *group = dynamic_cast<KHotKeys::ActionDataGroup*>(item);
            if (group)
                {
                d->action_group->setActionData( group );
                d->current = d->action_group;
                }
            }
            break;

        default:
            {
            const std::type_info &ti = typeid(*item);
            kDebug() << "##### Unknown ActionDataType " << ti.name();
            }

        } // switch

    d->stack->setCurrentWidget( d->current );
    }


KCMHotkeys::~KCMHotkeys()
    {
    delete d; d=0;
    }


void KCMHotkeys::defaults()
    {
    kWarning() << "not yet implemented!";
    }


void KCMHotkeys::load()
    {
    d->load();
    }


void KCMHotkeys::slotChanged()
    {
    emit changed(true);
    }


void KCMHotkeys::save()
    {
    d->save();
    }


// ==========================================================================
// KCMHotkeysPrivate


KCMHotkeysPrivate::KCMHotkeysPrivate( KCMHotkeys *host )
    : treeView( new HotkeysTreeView )
     ,model(new KHotkeysModel)
     ,q(host)
     ,stack(0)
     ,action_group(0)
     ,current(0)
     ,global_settings(0)
     ,simple_action(0)
    {
    action_group = new ActionGroupWidget(q);
    global_settings = new GlobalSettingsWidget(q);
    simple_action = new SimpleActionDataWidget(q);

    // Setup the stack
    stack = new QStackedWidget;
    stack->addWidget( global_settings );
    stack->addWidget( action_group );
    stack->addWidget( simple_action );

    // A splitter for the treeview and the stack
    QSplitter *splitter = new QSplitter;
    splitter->addWidget( treeView );
    splitter->addWidget( stack );

    // The global layout
    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget( splitter );
    q->setLayout( layout );

    // Initialize the global part of the khotkeys lib ( handler ... )
    KHotKeys::init_global_data(false, q);

    treeView->setModel(model);
    }


void KCMHotkeysPrivate::load()
    {
    // disconnect the signals
    if (treeView->selectionModel())
        {
        QObject::disconnect(
            treeView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            q, SLOT(currentChanged(QModelIndex,QModelIndex)) );
        }

    model->load();

    QObject::connect(
        model, SIGNAL( rowsRemoved( QModelIndex, int, int )),
        q,  SLOT( slotChanged() ));
    QObject::connect(
        model, SIGNAL( rowsInserted( QModelIndex, int, int )),
        q,  SLOT( slotChanged() ));
    QObject::connect(
        model, SIGNAL( dataChanged( QModelIndex, QModelIndex )),
        q,  SLOT( slotChanged() ));

    // reconnect the signals
    QObject::connect(
        treeView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
        q, SLOT(currentChanged(QModelIndex,QModelIndex)) );
    }


bool KCMHotkeysPrivate::maybeShowWidget()
    {
    // If the current widget is changed, ask user if switch is ok
    if (current && current->isChanged())
        {
        int choice = KMessageBox::warningContinueCancel(
             q,
             i18n("The current action has unsaved changes. If you continue those changes will be lost!"),
             i18n("Save changes") );
        if (choice != KMessageBox::Continue)
            {
            return false;
            }
        // Save the current Item
        saveCurrentItem();
        }
    return true;
    }


void KCMHotkeysPrivate::save()
    {
    if ( current && current->isChanged() )
        {
        saveCurrentItem();
        }

    // Write the settings
    model->save();

    // Inform kdedkhotkeys demon to reload settings
    QDBusConnection bus = QDBusConnection::sessionBus();
    QPointer<QDBusInterface> iface = new QDBusInterface("org.kde.kded", "/KHotKeys",
                                                        "org.kde.khotkeys", bus, q);
    if(!iface->isValid())
        {
        QDBusError err = iface->lastError();
        if (err.isValid())
            {
            kError() << err.name() << ":" << err.message();
            }
        QDBusInterface kdedInterface( "org.kde.kded", "/kded","org.kde.kded" );
        QDBusReply<bool> reply = kdedInterface.call( "loadModule", "khotkeys"  );
        err = iface->lastError();
        if (err.isValid())
            {
            kError() << err.name() << ":" << err.message();
            }

        if ( reply.isValid() )
            {
            if ( reply.value() )
                KMessageBox::error(q, "<qt>" + i18n("Started server <em>org.kde.khotkeys</em>.") + "</qt>");
            else
                KMessageBox::error(q, "<qt>" + i18n("Unable to start server <em>org.kde.khotkeys</em>.") + "</qt>");
            }
        else
            {
            KMessageBox::error(
                q,
                "<qt>" + i18n("Unable to start service <em>org.kde.khotkeys</em>.<br /><br /><i>Error: %1</i>",
                              reply.error().message()) + "</qt>" );
            }

        // kDebug() << "Starting khotkeys demon";
        // KToolInvocation::kdeinitExec( "khotkeys" );
        }
    else
        {
        kDebug() << "Pinging khotkeys demon";
        QDBusMessage reply = iface->call("reread_configuration");
        QDBusError err = iface->lastError();
        if (err.isValid())
            {
            kError() << err.name() << ":" << err.message();
            }
        }
    }



void KCMHotkeysPrivate::saveCurrentItem()
    {
    Q_ASSERT( current );
    // Only save when really changed
    if (current->isChanged())
        {
        current->copyToObject();
        model->emitChanged(current->data());
        save();
        }
    }


#include "moc_kcm_hotkeys.cpp"

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
#include "hotkeys_context_menu.h"
#include "ui_kcm_hotkeys.h"
#include "kcm_module_factory.h"

#include <typeinfo>


// ACTION_DATAS
#include "action_data/action_data_group.h"
// OUR ACTION WIDGETS
#include "action_group_widget.h"
#include "simple_action_data_widget.h"
#include "global_settings_widget.h"
// REST
#include "daemon/daemon.h"
#include "khotkeys_interface.h"
#include "hotkeys_model.h"
#include "hotkeys_proxy_model.h"
#include "hotkeys_tree_view.h"
#include "khotkeysglobal.h"

#include <QtDBus/QtDBus>

#include <KDE/KAboutData>
#include <KDE/KDebug>
#include <KDE/KLocale>
#include <KDE/KMessageBox>
#include <KDE/KPluginLoader>



class KCMHotkeysPrivate : public Ui::KCMHotkeysWidget
    {
    public:

        KCMHotkeysPrivate( KCMHotkeys *host );

        /** The model holding the shortcut settings. Beware! There is a proxy
         * model between us and that model */
        KHotkeysModel *model;

        //! Our host
        KCMHotkeys *q;

        //! The currently shown dialog
        HotkeysWidgetIFace *current;

        //! The currently shown item
        QModelIndex currentIndex;

        /**
         * Show the widget. If the current widget has changes allow
         * cancelation ! of this action
         */
        bool maybeShowWidget(const QModelIndex &next);

        /**
         * Applies the changes from the current item
         */
        void applyCurrentItem();

        /**
         * A Hotkey was changed. Update the treeview.
         */
        void _k_hotkeyChanged(KHotKeys::ActionDataBase*);

        /**
         * Activate the current item. The method is needed because of qt bug
         */
        void _k_activateCurrentItem();

        void load();
        void save();
    };


KCMHotkeys::KCMHotkeys( QWidget *parent, const QVariantList & /* args */ )
    : KCModule( KCMModuleFactory::componentData(), parent )
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
    connect(
        d->global_settings, SIGNAL(changed(bool)),
        this, SIGNAL(changed(bool)) );
    // Update TreeView if hotkeys was changed
    connect(
        d->simple_action, SIGNAL(changed(KHotKeys::ActionDataBase*)),
        this, SLOT(_k_hotkeyChanged(KHotKeys::ActionDataBase*)));
    connect(
        d->action_group, SIGNAL(changed(KHotKeys::ActionDataBase*)),
        this, SLOT(_k_hotkeyChanged(KHotKeys::ActionDataBase*)));

    // Show the context menu
    d->menu_button->setMenu(new HotkeysTreeViewContextMenu(d->tree_view));

    // Switch to the global settings dialog
    connect(d->settings_button, SIGNAL(clicked(bool)),
            SLOT(showGlobalSettings()));
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
    if (current==previous || current==d->currentIndex)
        {
        return;
        }

    // Current and previous differ. Ask user if there are unsaved changes
    if ( !d->maybeShowWidget(current) )
        {
        // Bring focus back to the current item
        d->tree_view->selectionModel()->setCurrentIndex(
                d->currentIndex,
                QItemSelectionModel::SelectCurrent);

        // See http://www.qtsoftware.com/developer/task-tracker/index_html?method=entry&id=132516
        QTimer::singleShot(0, this, SLOT(_k_activateCurrentItem()));
        return;
        }

    if (!current.isValid())
        {
        return showGlobalSettings();
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

    d->currentIndex = current;
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
    showGlobalSettings();
    d->load();
    }


void KCMHotkeys::showGlobalSettings()
    {
    d->current = d->global_settings;
    d->currentIndex = QModelIndex();

    d->tree_view->setCurrentIndex(d->currentIndex);
    d->global_settings->copyFromObject();
    d->stack->setCurrentWidget( d->global_settings );
    }


void KCMHotkeys::slotChanged()
    {
    emit changed(true);
    }


void KCMHotkeys::slotReset()
    {
    showGlobalSettings();
    }


void KCMHotkeys::save()
    {
    d->save();
    emit changed(false);
    }


// ==========================================================================
// KCMHotkeysPrivate


KCMHotkeysPrivate::KCMHotkeysPrivate( KCMHotkeys *host )
    : Ui::KCMHotkeysWidget()
     ,model(NULL)
     ,q(host)
     ,current(NULL)
    {
    setupUi(q);

    // Initialize the global part of the khotkeys lib ( handler ... )
    KHotKeys::init_global_data(false, q);
    }


void KCMHotkeysPrivate::load()
    {
    // Start khotkeys
    KHotKeys::Daemon::start();

    // disconnect the signals
    if (tree_view->selectionModel())
        {
        QObject::disconnect(
            tree_view->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            q, SLOT(currentChanged(QModelIndex,QModelIndex)) );
        }

    // Create a new model;
    tree_view->setModel(new KHotkeysModel);
    // Delete the old
    delete model;
    // Now use the old
    model = tree_view->model();

    model->load();
    global_settings->setModel(model);

    QObject::connect(
        model, SIGNAL(rowsRemoved(QModelIndex,int,int)),
        q,  SLOT(slotChanged()));
    QObject::connect(
        model, SIGNAL(rowsInserted(QModelIndex,int,int)),
        q,  SLOT(slotChanged()));
    QObject::connect(
        model, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
        q,  SLOT(slotChanged()));

    QObject::connect(
            model, SIGNAL(modelAboutToBeReset()),
            q,  SLOT(slotReset()));

    // reconnect the signals
    QObject::connect(
        tree_view->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
        q, SLOT(currentChanged(QModelIndex,QModelIndex)) );
    }


bool KCMHotkeysPrivate::maybeShowWidget(const QModelIndex &nextIndex)
    {
    kDebug();

    // If the current widget is changed, ask user if switch is ok
    if (current && (currentIndex != nextIndex) && current->isChanged())
        {
        int choice = KMessageBox::warningContinueCancel(
             q,
             i18n("The current action has unsaved changes. If you continue these changes will be lost."),
             i18n("Save changes") );
        if (choice != KMessageBox::Continue)
            {
            return false;
            }
        // Apply the changes from the current item
        //applyCurrentItem();
        //save();
        }
    return true;
    }


void KCMHotkeysPrivate::save()
    {
    if (current)
        applyCurrentItem();

    // Write the settings
    model->save();

    if (!KHotKeys::Daemon::isRunning())
        {
        if (!KHotKeys::Daemon::start())
            {
            // On startup the demon does the updating stuff, therefore reload
            // the actions.
            model->load();
            }
        else
            {
            KMessageBox::error(
                q,
                "<qt>" + i18n("Unable to contact khotkeys. Your changes are saved, but they could not be activated.") + "</qt>" );
            }

        return;
        }

    // Inform kdedkhotkeys demon to reload settings
    QDBusConnection bus = QDBusConnection::sessionBus();
    QPointer<OrgKdeKhotkeysInterface> iface = new OrgKdeKhotkeysInterface(
        "org.kde.kded",
        "/modules/khotkeys",
        bus,
        q);

    QDBusError err;
    if(!iface->isValid())
        {
        err = iface->lastError();
        if (err.isValid())
            {
            kError() << err.name() << ":" << err.message();
            }
        KMessageBox::error(
            q,
            "<qt>" + i18n("Unable to contact khotkeys. Your changes are saved, but they could not be activated.") + "</qt>" );
        return;
        }

    // Reread the configuration. We have no possibility to check if it worked.
    iface->reread_configuration();
    }


void KCMHotkeysPrivate::applyCurrentItem()
    {
    Q_ASSERT( current );
    // Only save when really changed
    if (current->isChanged())
        {
        current->apply();
        }
    }


void KCMHotkeysPrivate::_k_hotkeyChanged(KHotKeys::ActionDataBase* hotkey)
    {
    model->emitChanged(hotkey);
    }


void KCMHotkeysPrivate::_k_activateCurrentItem()
    {
    tree_view->selectionModel()->setCurrentIndex(
            tree_view->selectionModel()->currentIndex(),
            QItemSelectionModel::SelectCurrent);
    }

#include "moc_kcm_hotkeys.cpp"

/*
    Copyright 2007 Robert Knight <robertknight@gmail.com>
    Copyright 2007 Kevin Ottens <ervin@kde.org>

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

// Own
#include "ui/contextmenufactory.h"

// Qt
#include <QAbstractItemView>
#include <QDebug>
#include <QMap>
#include <QDBusMessage>
#include <QDBusConnection>

// KDE
#include <KIcon>
#include <KMenu>
#include <KActionCollection>
#include <KFileItem>
#include <KParts/BrowserExtension>
#include <KBookmarkManager>
#include <Solid/Device>
#include <Solid/StorageAccess>
#include <Solid/OpticalDrive>
#include <Solid/OpticalDisc>
#include <KUrl>
#include <KStandardDirs>
#include <KWindowSystem>

// Plasma
#include <Plasma/Containment>
#include <Plasma/Corona>

// Local
#include "core/favoritesmodel.h"
#include "core/models.h"

Q_DECLARE_METATYPE(QPersistentModelIndex)

using namespace Kickoff;

class ContextMenuFactory::Private
{
public:
    Private()
            : applet(0), packagekitAvailable(false) {
    }

    QAction *advancedActionsMenu(const QString& url) const {
        KUrl kUrl(url);
        KActionCollection actionCollection((QObject*)0);
        KFileItemList items;
        const QString mimeType = KMimeType::findByUrl(kUrl, 0, false, true)->name();
        items << KFileItem(url, mimeType, KFileItem::Unknown);
        KParts::BrowserExtension::PopupFlags browserFlags = KParts::BrowserExtension::DefaultPopupItems;
        if (items.first().isLocalFile()) {
            browserFlags |= KParts::BrowserExtension::ShowProperties;
        }
        KParts::BrowserExtension::ActionGroupMap actionGroupMap;
        return 0;
        // ### TODO: remove kdebase-apps dependency
#if 0
        KonqPopupMenu *menu = new KonqPopupMenu(items, kUrl, actionCollection,
                                                0, 0, browserFlags,
                                                0, KBookmarkManager::userBookmarksManager(), actionGroupMap);

        if (!menu->isEmpty()) {
            QAction *action = menu->menuAction();
            action->setText(i18n("Advanced"));
            action->setIcon(KIcon("list-add"));
            return action;
        } else {
            delete menu;
            return 0;
        }
#endif
    }

    QMap<QAbstractItemView*, QList<QAction*> > viewActions;
    Plasma::Applet *applet;
    bool packagekitAvailable;
};

ContextMenuFactory::ContextMenuFactory(QObject *parent)
        : QObject(parent)
        , d(new Private)
{
    // QDBusServiceWatcher is not suitable for this code because
    // the org.freedesktop.PackageKit interface might not be available
    // due to it being DBus activated.
    QDBusMessage message;
    message = QDBusMessage::createMethodCall("org.freedesktop.DBus",
                                             "/org/freedesktop/DBus",
                                             "org.freedesktop.DBus",
                                             "ListActivatableNames");

    QDBusMessage reply = QDBusConnection::sessionBus().call(message);
    if (reply.type() == QDBusMessage::ReplyMessage
     && reply.arguments().size() == 1) {
        QStringList list = reply.arguments().first().toStringList();
        if (list.contains("org.freedesktop.PackageKit")) {
            d->packagekitAvailable = true;
        }
    }
}

ContextMenuFactory::~ContextMenuFactory()
{
    delete d;
}

void ContextMenuFactory::showContextMenu(QAbstractItemView *view,
                                         const QPersistentModelIndex& index, const QPoint& pos)
{
    Q_UNUSED(pos);

    if (!index.isValid()) {
        return;
    }

    QString url = index.data(UrlRole).value<QString>();
    qDebug() << "ContextMenuFactory::showContextMenu: " << url;

    if (url.isEmpty()) {
        return;
    }

    // ivan: The url handling is dirty - instead of handling it in
    // the source data models (that define them), we are handling
    // them here. So, we need to make urls from KRunner model
    // to behave properly
    if (url.startsWith(QLatin1String("krunner://"))) {
        url = url.remove("krunner://");
        qDebug() << "ContextMenuFactory::showContextMenu: 1 " << url;
        if (url.startsWith(QLatin1String("services/services_"))) {
            url = url.remove("services/services_");
        } else {
            return;
        }

        KService::Ptr service = KService::serviceByStorageId(url);
        if(!service) {
            return;
        }

        url = service->entryPath();

        qDebug() << "ContextMenuFactory::showContextMenu: " << "KRunner service runner: " << url;
    }


    const bool isFavorite = FavoritesModel::isFavorite(url);

    QList<QAction*> actions;

    QAction *favoriteAction = 0;

    if (url.endsWith(QLatin1String(".desktop"))) {
        // add to / remove from favorites
        favoriteAction = new QAction(this);
        if (isFavorite) {
            favoriteAction->setText(i18n("Remove From Favorites"));
            favoriteAction->setIcon(KIcon("list-remove"));
            actions << favoriteAction;
            //exclude stuff in the leave tab
        } else if (KUrl(url).protocol() != "leave") {
            favoriteAction->setText(i18n("Add to Favorites"));
            favoriteAction->setIcon(KIcon("bookmark-new"));
            actions << favoriteAction;
        }
    }

    // add to desktop
    QAction *addToDesktopAction = new QAction(this);

    // add to main panel
    QAction *addToPanelAction = new QAction(this);

    //### FIXME :   icons in leave-view are not properly based on a .desktop file
    //so you cant put them in desktop or panel
    //### TODO : do not forget to remove (kurl.scheme() != "leave") and kurl declaration
    //when proper action for such case will be provided
    KUrl kurl(url);
    if ((d->applet) && (kurl.scheme() != "leave")) {
        Plasma::Containment *containment = d->applet->containment();

        // There might be relative paths for .desktop installed in
        // /usr/shar/applnk, we need to locate them
        bool urlFound = true;
        if (kurl.isRelative() && kurl.url().endsWith(QLatin1String(".desktop"))) {
            kurl = KStandardDirs::locate("apps", url);
            urlFound = !kurl.isEmpty();
        }

        if (urlFound && containment && containment->corona()) {
            Plasma::Containment *desktop = containment->corona()->containmentForScreen(containment->screen());

            if (desktop && desktop->immutability() == Plasma::Mutable) {
                addToDesktopAction->setText(i18n("Add to Desktop"));
                actions << addToDesktopAction;
            }
        }

        if (urlFound && containment && containment->immutability() == Plasma::Mutable &&
            (containment->containmentType() == Plasma::Containment::PanelContainment ||
             containment->containmentType() == Plasma::Containment::CustomPanelContainment)) {
            addToPanelAction->setText(i18n("Add to Panel"));
            actions << addToPanelAction;
        }
    }

    QAction *pkUninstall = 0;
    // If we have PackageKit session interface we might be able to remove applications
    if (d->packagekitAvailable) {
        KService::Ptr service = KService::serviceByStorageId(url);
        if(service && service->isApplication()) {
            pkUninstall = new QAction(this);

            // PackageKit uninstall action
            pkUninstall->setText(i18n("Uninstall"));
            actions << pkUninstall;
        }
    }

    QAction *advancedSeparator = new QAction(this);
    if (actions.count() > 0) {
        // advanced item actions
        advancedSeparator->setSeparator(true);
        actions << advancedSeparator;
    }

    QAction *advanced = d->advancedActionsMenu(url);
    if (advanced) {
        actions << advanced;
    }

    // device actions
    const QString udi = index.data(DeviceUdiRole).toString();
    Solid::Device device(udi);
    Solid::StorageAccess *access = device.as<Solid::StorageAccess>();
    QAction *ejectAction = 0;
    if (device.isValid() && access) {
        ejectAction = new QAction(this);
        if (device.is<Solid::OpticalDisc>()) {
            ejectAction->setText(i18n("Eject"));
        } else {
            ejectAction->setText(i18n("Safely Remove"));
        }
        actions << ejectAction;
    }

    // add view specific actions
    QAction *viewSeparator = new QAction(this);
    if (view) {
        if (actions.count() > 0) {
            viewSeparator->setSeparator(true);
            actions << viewSeparator;
        }
        actions << viewActions(view);
    }

    //return if we added just a separator so far
    if (actions.count() < 2) {
        return;
    }


    // display menu
    KMenu menu;
    menu.addTitle(index.data(Qt::DisplayRole).value<QString>());
    foreach(QAction* action, actions) {
        menu.addAction(action);
    }

    QAction *result = menu.exec(QCursor::pos());

    if (favoriteAction && result == favoriteAction) {
        if (isFavorite) {
            FavoritesModel::remove(url);
        } else {
            FavoritesModel::add(url);
        }
    } else if (ejectAction && result == ejectAction) {
        if (device.is<Solid::OpticalDisc>()) {
            Solid::OpticalDrive *drive = device.parent().as<Solid::OpticalDrive>();
            drive->eject();
        } else {
            access->teardown();
        }
    } else if (addToDesktopAction && result == addToDesktopAction) {
        if (d->applet) {
            Plasma::Containment *containment = d->applet->containment();
            if (containment) {
                Plasma::Corona *corona = containment->corona();
                if (corona) {
                    int vdesk = KWindowSystem::currentDesktop() - 1;
                    Plasma::Containment *desktop = corona->containmentForScreen(containment->screen(), vdesk);
                    //PVDA disabled?
                    if (!desktop) {
                        desktop = corona->containmentForScreen(containment->screen(), -1);
                    }
                    if (desktop) {
                        QVariantList args;
                        args << kurl.url() << index.data(Kickoff::IconNameRole);
                        if (kurl.scheme() == "applications") { // it's a service group
                            desktop->addApplet("simplelauncher", args);
                        } else if (desktop->metaObject()->indexOfSlot("addUrls(KUrl::List)") != -1) {
                            QMetaObject::invokeMethod(desktop, "addUrls",
                            Qt::DirectConnection, Q_ARG(KUrl::List, KUrl::List(kurl)));
                        } else {
                            desktop->addApplet("icon", args);
                        }
                    }
                }
            }
        }
    } else if (pkUninstall && result == pkUninstall) {
        QStringList files;
        files << url;
        QDBusMessage message;
        message = QDBusMessage::createMethodCall("org.freedesktop.PackageKit",
                                                 "/org/freedesktop/PackageKit",
                                                 "org.freedesktop.PackageKit.Modify",
                                                 "RemovePackageByFiles");
        message << (uint) 0;
        message << files;
        message << QString();

        QDBusConnection::sessionBus().call(message, QDBus::NoBlock);
    }else if (addToPanelAction && result == addToPanelAction) {
        if (d->applet) {
            // we assume that the panel is the same containment where the kickoff is located
            Plasma::Containment *panel = d->applet->containment();
            if (panel) {
                QVariantList args;
                args << kurl.url() << index.data(Kickoff::IconNameRole);

                // move it to the middle of the panel
                QRectF rect(panel->geometry().width() / 2, 0, 150, panel->boundingRect().height());
                if (kurl.scheme() == "applications") { // it's a service group
                    panel->addApplet("simplelauncher", args);
                } else {
                    panel->addApplet("icon", args, rect);
                }
            }
        }
    }

    delete favoriteAction;
    delete addToDesktopAction;
    delete addToPanelAction;
    delete pkUninstall;
    delete advancedSeparator;
    delete viewSeparator;
    delete ejectAction;
}

void ContextMenuFactory::setViewActions(QAbstractItemView *view, const QList<QAction*>& actions)
{
    if (actions.isEmpty()) {
        d->viewActions.remove(view);
    } else {
        d->viewActions.insert(view, actions);
    }
}
QList<QAction*> ContextMenuFactory::viewActions(QAbstractItemView *view) const
{
    return d->viewActions[view];
}

void ContextMenuFactory::setApplet(Plasma::Applet *applet)
{
    d->applet = applet;
}

#include "contextmenufactory.moc"

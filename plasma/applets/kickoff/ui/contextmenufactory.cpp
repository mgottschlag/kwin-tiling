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
#include <QtDebug>
#include <QMap>

// KDE
#include <KIcon>
#include <KLocalizedString>
#include <KMenu>
#include <KActionCollection>
#include <KFileItem>
#include <KParts/BrowserExtension>
#include <KBookmarkManager>
#include <Solid/Device>
#include <Solid/StorageAccess>

// Local
#include "core/favoritesmodel.h"
#include "core/models.h"

using namespace Kickoff;

class ContextMenuFactory::Private
{
public:
    QAction *advancedActionsMenu(const QString& url) const
    {
       KUrl kUrl(url);
       KActionCollection actionCollection((QObject*)0);
       KFileItemList items;
       QString mimeType = KMimeType::findByUrl(kUrl,0,false,true)->name();
       items << KFileItem(url,mimeType,KFileItem::Unknown);
       KParts::BrowserExtension::PopupFlags browserFlags = KParts::BrowserExtension::DefaultPopupItems;
       if (items.first().isLocalFile()) {
           browserFlags |= KParts::BrowserExtension::ShowProperties;
       }
       KParts::BrowserExtension::ActionGroupMap actionGroupMap;
       return 0;
       // ### TODO: remove kdebase-apps dependency
#if 0
       KonqPopupMenu *menu = new KonqPopupMenu(items, kUrl,actionCollection,
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

    QMap<QAbstractItemView*,QList<QAction*> > viewActions;
};

ContextMenuFactory::ContextMenuFactory(QObject *parent)
 : QObject(parent)
 , d(new Private)
{
}

ContextMenuFactory::~ContextMenuFactory()
{
  delete d;
}

void ContextMenuFactory::showContextMenu(QAbstractItemView *view,const QPoint& pos)
{
    Q_ASSERT(view);

    const QModelIndex index = view->indexAt(pos);
    const QString url = index.data(UrlRole).value<QString>();

    if (url.isEmpty()) {
        return;
    }

    bool isFavorite = FavoritesModel::isFavorite(url);

    QList<QAction*> actions;

    // add to / remove from favorites
    QAction *favoriteAction = new QAction(this);
    if (isFavorite) {
        favoriteAction->setText(i18n("Remove from Favorites"));
        favoriteAction->setIcon(KIcon("list-remove"));
    } else {
        favoriteAction->setText(i18n("Add to Favorites"));
        favoriteAction->setIcon(KIcon("bookmark-new"));
    }

    actions << favoriteAction;

    // add to desktop
    QAction *addToDesktopAction = new QAction(this);
    addToDesktopAction->setText(i18n("Add to Desktop"));
    addToDesktopAction->setEnabled(false);
    actions << addToDesktopAction;

    // add to main panel
    QAction *addToPanelAction = new QAction(this);
    addToPanelAction->setText(i18n("Add to Panel"));
    addToPanelAction->setEnabled(false);
    actions << addToPanelAction;

    // advanced item actions
    QAction *advancedSeparator = new QAction(this);
    advancedSeparator->setSeparator(true);
    actions << advancedSeparator;

    QAction *advanced = d->advancedActionsMenu(url);
    if (advanced) {
        actions << advanced;
    }

    // device actions
    QString udi = index.data(DeviceUdiRole).toString();
    Solid::Device device(udi);
    Solid::StorageAccess *access = device.as<Solid::StorageAccess>();
    QAction *ejectAction = 0;
    if (device.isValid() && access) {
        ejectAction = new QAction(this);
        ejectAction->setText("Eject");
        actions << ejectAction;
    }

    // add view specific actions
    QAction *viewSeparator = new QAction(this);
    viewSeparator->setSeparator(true);
    actions << viewSeparator;
    actions << viewActions(view);

    // display menu
    KMenu menu;
    menu.addTitle(index.data(Qt::DisplayRole).value<QString>());
    foreach (QAction* action, actions) {
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
        access->teardown();
    }

    delete favoriteAction;
    delete addToDesktopAction;
    delete addToPanelAction;
    delete viewSeparator;
    delete ejectAction;
}
void ContextMenuFactory::setViewActions(QAbstractItemView *view,const QList<QAction*>& actions)
{
    if (actions.isEmpty()) {
        d->viewActions.remove(view);
    } else {
        d->viewActions.insert(view,actions);
    }
}
QList<QAction*> ContextMenuFactory::viewActions(QAbstractItemView *view) const
{
    return d->viewActions[view];
}

#include "contextmenufactory.moc"

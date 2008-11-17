/*
    Copyright 2007 Robert Knight <robertknight@gmail.com>

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

//Own
#include "core/favoritesmodel.h"

// Qt
#include <QHash>
#include <QList>
#include <QMimeData>
#include <QFileInfo>

// KDE
#include <KConfigGroup>
#include <KService>
#include <kdebug.h>

// Local
#include "core/models.h"

using namespace Kickoff;

class FavoritesModel::Private
{
public:
    Private(FavoritesModel *parent)
            : q(parent) {
        headerItem = new QStandardItem(i18n("Favorites"));
        q->appendRow(headerItem);
    }

    void addFavoriteItem(const QString& url) {
        QStandardItem *item = StandardItemFactory::createItemForUrl(url);
        headerItem->appendRow(item);
    }
    void moveFavoriteItem(int startRow, int destRow) {
        if (destRow == startRow)
            return;

        QStandardItem *item = headerItem->takeChild(startRow);

        headerItem->removeRow(startRow);
        headerItem->insertRow(destRow, item);
    }
    void removeFavoriteItem(const QString& url) {
        QModelIndexList matches = q->match(q->index(0, 0), UrlRole,
                                           url, -1,
                                           Qt::MatchFlags(Qt::MatchStartsWith | Qt::MatchWrap | Qt::MatchRecursive));

        kDebug() << "Removing item matches" << matches;

        foreach(const QModelIndex& index, matches) {
            QStandardItem *item = q->itemFromIndex(index);
            if (item->parent()) {
                item->parent()->removeRow(item->row());
            } else {
                qDeleteAll(q->takeRow(item->row()));
            }
        }
    }

    FavoritesModel * const q;
    QStandardItem *headerItem;

    static void loadFavorites() {
        KConfigGroup favoritesGroup = componentData().config()->group("Favorites");
        QList<QString> favoriteList = favoritesGroup.readEntry("FavoriteURLs", QList<QString>());
        if (favoriteList.isEmpty()) {
            favoriteList = defaultFavorites();
        }

        foreach(const QString &favorite, favoriteList) {
            FavoritesModel::add(favorite);
        }
    }
    static QList<QString> defaultFavorites() {
        QList<QString> applications;
        applications << "konqbrowser" << "kmail" << "systemsettings" << "dolphin";

        QList<QString> desktopFiles;

        foreach(const QString& application, applications) {
            KService::Ptr service = KService::serviceByStorageId("kde4-" + application + ".desktop");
            if (service) {
                desktopFiles << service->entryPath();
            }
        }

        return desktopFiles;
    }
    static void saveFavorites() {
        KConfigGroup favoritesGroup = componentData().config()->group("Favorites");
        favoritesGroup.writeEntry("FavoriteURLs", globalFavoriteList);
        favoritesGroup.config()->sync();
    }
    static QList<QString> globalFavoriteList;
    static QSet<QString> globalFavoriteSet;
    static QSet<FavoritesModel*> models;
};

QList<QString> FavoritesModel::Private::globalFavoriteList;
QSet<QString> FavoritesModel::Private::globalFavoriteSet;
QSet<FavoritesModel*> FavoritesModel::Private::models;

FavoritesModel::FavoritesModel(QObject *parent)
        : KickoffModel(parent)
        , d(new Private(this))
{
    Private::models << this;
    if (Private::models.count() == 1 && Private::globalFavoriteList.isEmpty()) {
        Private::loadFavorites();
    } else {
        foreach(const QString &url, Private::globalFavoriteList) {
            d->addFavoriteItem(url);
        }
    }

}
FavoritesModel::~FavoritesModel()
{
    Private::models.remove(this);

    if (Private::models.isEmpty()) {
        Private::saveFavorites();
    }

    delete d;
}
void FavoritesModel::add(const QString& url)
{
    Private::globalFavoriteList << url;
    Private::globalFavoriteSet << url;

    foreach(FavoritesModel* model, Private::models) {
        model->d->addFavoriteItem(url);
    }

    // save after each add in case we crash
    Private::saveFavorites();
}

void FavoritesModel::move(int startRow, int destRow)
{
    // just move the item
    Private::globalFavoriteList.move(startRow, destRow);

    foreach(FavoritesModel* model, Private::models) {
        model->d->moveFavoriteItem(startRow, destRow);
    }

    // save after each add in case we crash
    Private::saveFavorites();
}

void FavoritesModel::remove(const QString& url)
{
    Private::globalFavoriteList.removeAll(url);
    Private::globalFavoriteSet.remove(url);

    foreach(FavoritesModel* model, Private::models) {
        model->d->removeFavoriteItem(url);
    }

    // save after each remove in case of crash or other mishaps
    Private::saveFavorites();
}

bool FavoritesModel::isFavorite(const QString& url)
{
    return Private::globalFavoriteSet.contains(url);
}

int FavoritesModel::numberOfFavorites()
{
    foreach(FavoritesModel* model, Private::models) {
        return model->d->headerItem->rowCount() - 1;
    }

    return 0;
}

void FavoritesModel::sortFavorites(Qt::SortOrder order)
{
    foreach(FavoritesModel *model, Private::models) {
        model->d->headerItem->sortChildren(0, order);
    }
}

bool FavoritesModel::dropMimeData(const QMimeData *data, Qt::DropAction action,
                                  int row, int column, const QModelIndex & parent)
{
    Q_UNUSED(parent);

    if (action == Qt::IgnoreAction) {
        return true;
    }

    if (column > 0) {
        return false;
    }

    if (action == Qt::MoveAction) {
        QModelIndex modelIndex;
        QStandardItem *startItem;
        int startRow = 0, destRow;

        destRow = row;

        // look for the favorite that was dragged
        for (int i = 0; i < d->headerItem->rowCount(); i++) {
            startItem = d->headerItem->child(i, 0);
            if (QFileInfo(startItem->data(Kickoff::UrlRole).toString()).completeBaseName()
                    == QFileInfo(data->text()).completeBaseName()) {
                startRow = i;
                break;
            }
        }

        if (destRow < 0)
            return false;

        // now move the item to it's new location
        FavoritesModel::move(startRow, destRow);

        return true;
    }

    return true;
}
#include "favoritesmodel.moc"

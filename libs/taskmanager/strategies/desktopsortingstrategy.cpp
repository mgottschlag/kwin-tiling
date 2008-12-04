/*****************************************************************

Copyright 2008 Christian Mollekopf <chrigi_1@hotmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#include "desktopsortingstrategy.h"

#include <QMap>
#include <QString>
#include <QtAlgorithms>
#include <QList>

#include <KDebug>

#include "abstractgroupableitem.h"


namespace TaskManager
{

DesktopSortingStrategy::DesktopSortingStrategy(QObject *parent)
:AbstractSortingStrategy(parent)
{
    setType(GroupManager::DesktopSorting);
}

void DesktopSortingStrategy::sortItems(ItemList &items)
{
    kDebug();
    QMap<QString, AbstractGroupableItem*> map;
    foreach (AbstractGroupableItem *item, items) {
        if (!item) {
            kDebug() << "Null Pointer";
            continue;
        }
        kDebug() << item->name() << item->desktop();
        map.insertMulti(QString::number(item->desktop())+item->name(), item);
    }

    items.clear();
    items = map.values();
    kDebug();
    foreach (AbstractGroupableItem *item, items) {
        kDebug() << item->name() << item->desktop();
    }
}

void DesktopSortingStrategy::handleItem(AbstractItemPtr item)
{
    kDebug();
    disconnect(item, 0, this, 0); //To avoid duplicate connections
    connect(item, SIGNAL(changed(::TaskManager::TaskChanges)), this, SLOT(check()));
    AbstractSortingStrategy::handleItem(item);
}

} //namespace

#include "desktopsortingstrategy.moc"


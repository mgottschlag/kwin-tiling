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
    qStableSort(items.begin(), items.end(), DesktopSortingStrategy::lessThan);
}

bool DesktopSortingStrategy::lessThan(const AbstractItemPtr &left, const AbstractItemPtr &right)
{
    if (left->desktop() == right->desktop()) {
        return left->name().toLower() < right->name().toLower();
    }

    return left->desktop() < right->desktop();
}

void DesktopSortingStrategy::handleItem(AbstractItemPtr item)
{
    disconnect(item, 0, this, 0); //To avoid duplicate connections
    connect(item, SIGNAL(changed(::TaskManager::TaskChanges)), this, SLOT(check()));
    AbstractSortingStrategy::handleItem(item);
}

} //namespace

#include "desktopsortingstrategy.moc"


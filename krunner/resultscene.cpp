/***************************************************************************
 *   Copyright 2007 by Enrico Ros <enrico.ros@gmail.com>                   *
 *   Copyright 2007 by Riccardo Iaconelli <ruphy@kde.org>                  *
 *   Copyright 2008 by Aaron Seigo <aseigo@kde.org>                        *
 *   Copyright 2008 by Davide Bettio <davide.bettio@kdemail.net>           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/


#include "resultscene.h"
#include <QtCore/QDebug>
#include <QtGui/QKeyEvent>
#include <QtGui/QPainter>
#include <QtCore/QTimeLine>
#include <QtGui/QGraphicsGridLayout>
#include <QtGui/QGraphicsWidget>
#include <QtGui/QGraphicsProxyWidget>

#include <KDE/KDebug>
#include <KDE/KLineEdit>

#include <plasma/runnermanager.h>

#include "resultitem.h"

ResultScene::ResultScene(QObject * parent)
    : QGraphicsScene(parent),
      m_itemCount(0)
{
    setItemIndexMethod(NoIndex);

    m_mainWidget = new QGraphicsWidget(0);

    QGraphicsGridLayout *layout = new QGraphicsGridLayout(m_mainWidget);
    layout->setContentsMargins(0, 0, 0, 0);

    m_iconArea = new QGraphicsWidget(m_mainWidget);
    layout->addItem(m_iconArea, 1, 0);

    m_mainWidget->resize(sceneRect().size());
    addItem(m_mainWidget);

    m_runnerManager = new Plasma::RunnerManager(this);
    connect(m_runnerManager, SIGNAL(matchesChanged(const QList<Plasma::QueryMatch>&)),
            this, SLOT(setQueryMatches(const QList<Plasma::QueryMatch>&)));

    m_resizeTimer.setSingleShot(true);
    connect(&m_resizeTimer, SIGNAL(timeout()), this, SLOT(layoutIcons()));

    m_clearTimer.setSingleShot(true);
    connect(&m_clearTimer, SIGNAL(timeout()), this, SLOT(clearMatches()));

    //QColor bg(255, 255, 255, 126);
    //setBackgroundBrush(bg);
}

ResultScene::~ResultScene()
{
    delete m_mainWidget;
}

void ResultScene::resize(int width, int height)
{
    // optimize
    if (m_size.width() == width && m_size.height() == height) {
        return;
    }

    m_size = QSize(width, height);
    setSceneRect(0.0, 0.0, (qreal)width, (qreal)height);
    m_mainWidget->resize(m_size);
    m_resizeTimer.start(150);
}

void ResultScene::layoutIcons()
{
    // resize
    int rowStride = sceneRect().width() / (ResultItem::BOUNDING_SIZE);

    QListIterator<ResultItem *> it(m_items);

    while (it.hasNext()) {
        ResultItem *item = it.next();
        item->setRowStride(rowStride);
    }
}

void ResultScene::addQueryMatch(const Plasma::QueryMatch &match)
{
    // TODO: check for duplicates
    //kDebug() << "adding" << match.id() << m_itemsById.count();
    /*QMapIterator<QString, ResultItem*> dbgIt(m_itemsById);
    while (dbgIt.hasNext()) {
        dbgIt.next();
        kDebug() << dbgIt.key() << dbgIt.value()->id();
    }*/

    QMap<QString, ResultItem*>::iterator it = m_itemsById.find(match.id());
    ResultItem *item = 0;

    if (it == m_itemsById.end()) {
        //kDebug() << "did not find";
        item = new ResultItem(match, m_iconArea);
        item->hide();
        m_itemsById.insert(match.id(), item);
        m_items.append(item);
        int rowStride = sceneRect().width() / (ResultItem::BOUNDING_SIZE);
        item->setRowStride(rowStride);
        item->setIndex(m_itemCount++);
        connect(item, SIGNAL(activated(ResultItem*)), this, SIGNAL(itemActivated(ResultItem*)));
        connect(item, SIGNAL(hoverEnter(ResultItem*)), this, SIGNAL(itemHoverEnter(ResultItem*)));
        connect(item, SIGNAL(hoverLeave(ResultItem*)), this, SIGNAL(itemHoverLeave(ResultItem*)));
    } else {
        item = it.value();
        item->setMatch(match);
    }

    item->setUpdateId(m_updateId);
    //connect(item, SIGNAL(indexReleased(int)), this, SLOT(indexReleased(int)));
}

void ResultScene::removeQueryMatch(const Plasma::QueryMatch &match)
{
    QMap<QString, ResultItem*>::iterator it = m_itemsById.find(match.id());
    if (it != m_itemsById.end()) {
        m_itemsById.erase(it);
        removeMatch(it.value());
    }
}

void ResultScene::clearMatches()
{
    QMutableMapIterator<QString, ResultItem*> it(m_itemsById);
    while (it.hasNext()) {
        it.next();
        ResultItem *item = it.value();
        it.remove();
        m_items.removeOne(item);
        indexReleased(item->index());
        item->remove();
    }
}

void ResultScene::setQueryMatches(const QList<Plasma::QueryMatch> &m)
{
    //kDebug() << "matches retreived: " << matches.count();
    if (m.count() == 0) {
        //kDebug() << "clearing";
        m_clearTimer.start(200);
        return;
    }

    m_clearTimer.stop();

    QList<Plasma::QueryMatch> matches = m;
    qStableSort(matches);

    ++m_updateId;
    // be sure all the new elements are in
    QList<Plasma::QueryMatch>::const_iterator newMatchIt = matches.constEnd();
    while (newMatchIt != matches.constBegin()) {
        --newMatchIt;
        addQueryMatch(*newMatchIt);
    }

    // now delete the stragglers
    QList<ResultItem*> remove;
    QMutableListIterator<ResultItem *> it(m_items);
    while (it.hasNext()) {
        it.next();
        if (it.value()->updateId() != m_updateId) {
            //kDebug() << it.value()->id() << "was not updated (" << it.value()->updateId() << " vs " << m_updateId << ")";
            m_itemsById.remove(it.value()->id());
            remove << it.value();
            //removeMatch(it.value());
        }
    }

    removeMatches(remove);
}

void ResultScene::removeMatches(const QList<ResultItem*> &items)
{
    foreach (ResultItem *item, items) {
        m_items.removeOne(item);
        indexReleased(item->index());
        item->remove();
    }

    if (!m_items.isEmpty()) {
        m_items.at(0)->setIsDefault(true);
    }
}

void ResultScene::removeMatch(ResultItem* item)
{
    qWarning("It's a song to say goodbye ...");
    m_items.removeOne(item);
    indexReleased(item->index());
    item->remove();

    if (!m_items.isEmpty()) {
        m_items.at(0)->setIsDefault(true);
    }
}

void ResultScene::keyPressEvent(QKeyEvent * keyEvent)
{
    switch (keyEvent->key()) {
        case Qt::Key_Up:
            qWarning("ResultScene: key up");
            break;

        case Qt::Key_Down:
            qWarning("ResultScene: key down");
            break;

        case Qt::Key_Left:
            qWarning("ResultScene: key left");
            break;

        case Qt::Key_Right:
            qWarning("ResultScene: key right");
            break;

        case Qt::Key_Return:
        case Qt::Key_Space:
        default:
            // pass the event to the item
            QGraphicsScene::keyPressEvent(keyEvent);
            break;
    }
}

/// SLOTS <- ResultItems
void ResultScene::slotArrowResultItemPressed()
{

}

void ResultScene::slotArrowResultItemReleased()
{

}

void ResultScene::indexReleased(int index)
{
    --m_itemCount;
    QMutableListIterator<ResultItem *> it(m_items);
    while (it.hasNext()) {
        ResultItem *item = it.next();
        if (item->index() == index) {
            //qDebug() << "found our boy to remove" << item->name();
            it.remove();
        } else if (item->index() > index) {
            //qDebug() << "decrementing" << item->name() << "from" << item->index();
            item->setIndex(item->index() - 1);
            //qDebug() << "now is" << item->index();
        }
    }
}

void ResultScene::launchQuery(const QString &term)
{
    m_runnerManager->launchQuery(term);
}

void ResultScene::clearQuery()
{
    m_runnerManager->reset();
}

ResultItem* ResultScene::defaultResultItem() const
{
    if (m_items.isEmpty()) {
        kDebug() << "empty";
        return 0;
    }

    kDebug() << (QObject*) m_items[0] << m_items.count();
    //fixme: this should really be a sorted list!
    return m_items[0];
}

#include "resultscene.moc"


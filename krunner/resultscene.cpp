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
      m_itemCount(0),
      m_cIndex(0)
{
    setItemIndexMethod(NoIndex);

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
}

QSize ResultScene::minimumSizeHint() const
{
    return QSize(ResultItem::BOUNDING_SIZE * 4 + 6, ResultItem::BOUNDING_SIZE * 2 + 6);
}

void ResultScene::resize(int width, int height)
{
    // optimize
    if (m_size.width() == width && m_size.height() == height) {
        return;
    }

    m_size = QSize(width, height);
    setSceneRect(0.0, 0.0, (qreal)width, (qreal)height);
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
    QMap<QString, ResultItem*>::iterator it = m_itemsById.find(match.id());
    ResultItem *item = 0;

    if (it == m_itemsById.end()) {
        //kDebug() << "did not find";
        item = new ResultItem(match, 0);
        addItem(item);
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
}

void ResultScene::clearMatches()
{
    foreach (ResultItem *item, m_items) {
        item->remove();
    }

    m_itemsById.clear();
    m_items.clear();
    m_itemCount = 0;
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
    QMutableListIterator<ResultItem *> it(m_items);
    while (it.hasNext()) {
        ResultItem *item = it.next();
        if (item->updateId() != m_updateId) {
            kDebug() << item->id() << "was not updated (" << item->updateId() << " vs " << m_updateId << ")";
            m_itemsById.remove(item->id());
            indexReleased(item->index());
            item->remove(); 
            it.remove();
        }
    }

    if (!m_items.isEmpty()) {
        emit itemHoverEnter(m_items.at(0));
        setFocusItem(m_items.at(0));
    }
}

void ResultScene::focusOutEvent(QFocusEvent *focusEvent)
{
    QGraphicsScene::focusOutEvent(focusEvent);
    if (!m_items.isEmpty()) {
        emit itemHoverEnter(m_items.at(0));
    }
}

void ResultScene::keyPressEvent(QKeyEvent * keyEvent)
{
    //kDebug() << "m_items (size): " << m_items.size() << "\n";
    ResultItem *currentFocus = dynamic_cast<ResultItem*>(focusItem());
    int m_cIndex = currentFocus ? currentFocus->index() : 0;
    switch (keyEvent->key()) {
        case Qt::Key_Up:{
            int rowStride = sceneRect().width() / (ResultItem::BOUNDING_SIZE);
            if (m_cIndex < rowStride) {
                if (m_items.size() < rowStride) {
                    // we have less than one row of items, so lets just move to the next item
                    m_cIndex = (m_cIndex + 1) % m_items.size();
                } else {
                    m_cIndex = m_items.size() - (m_items.size() % rowStride) - 1 + (m_cIndex % m_items.size());
                    if (m_cIndex >= m_items.size()) {
                        // we should be on the bottom row, but there is nothing there; move up one row
                        m_cIndex -= rowStride % m_items.size();
                    }
                }
            } else {
                m_cIndex = m_cIndex - rowStride;
            }
            break;
        }

        case Qt::Key_Down:{
            int rowStride = sceneRect().width() / (ResultItem::BOUNDING_SIZE);
            if (m_cIndex + rowStride >= m_items.size()) {
                // warp to the top
                m_cIndex = (m_cIndex + 1) % rowStride % m_items.size();
            } else {
                // next row!
                m_cIndex += rowStride;
            }

            break;
        }

        case Qt::Key_Left:
            m_cIndex = (m_cIndex == 0) ? m_items.size() - 1 : m_cIndex - 1;
        break;

        case Qt::Key_Right:
            m_cIndex = (m_cIndex + 1) % m_items.size();
        break;

        case Qt::Key_Return:
            //TODO: run the item
        case Qt::Key_Space:
        default:
            // pass the event to the item
            QGraphicsScene::keyPressEvent(keyEvent);
            return;
        break;
    }

    // If we arrive here, it was due to an arrow button.
    Q_ASSERT(m_cIndex  >= 0);
    Q_ASSERT(m_cIndex < m_items.count());
    //kDebug() << "m_cIndex: " << m_cIndex << "\n";
    setFocusItem(m_items.at(m_cIndex));
}

void ResultScene::slotArrowResultItemPressed()
{

}

void ResultScene::slotArrowResultItemReleased()
{

}

void ResultScene::indexReleased(int index)
{
    --m_itemCount;
    QListIterator<ResultItem *> it(m_items);
    while (it.hasNext()) {
        ResultItem *item = it.next();
        if (item->index() > index) {
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

void ResultScene::launchQuery(const QString &term, const QString &runner)
{
    m_runnerManager->launchQuery(term, runner);
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

void ResultScene::run(ResultItem *item) const
{
    if (!item) {
        return;
    }

    item->run(m_runnerManager);
}

Plasma::RunnerManager* ResultScene::manager() const
{
    return m_runnerManager;
}

#include "resultscene.moc"


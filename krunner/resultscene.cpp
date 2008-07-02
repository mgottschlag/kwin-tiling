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

ResultScene::ResultScene(QObject *parent)
    : QGraphicsScene(parent),
      m_itemCount(0),
      m_cIndex(0),
      m_rowStride(0),
      m_pageStride(0),
      m_pageCount(0),
      m_currentPage(0)
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
    QFontMetrics fm(font());
    return QSize(ResultItem::BOUNDING_WIDTH * 4 + 6, (ResultItem::BOUNDING_HEIGHT + fm.height()) * 2 + 6);
}

void ResultScene::resize(int width, int height)
{
    // optimize
    if (m_size.width() == width && m_size.height() == height) {
        return;
    }

    m_size = QSize(width, height);
    m_rowStride = width / (ResultItem::BOUNDING_WIDTH);
    m_pageStride = height / (ResultItem::BOUNDING_WIDTH) * m_rowStride;
    setSceneRect(0.0, 0.0, (qreal)width, (qreal)height);
    m_resizeTimer.start(150);
}

void ResultScene::layoutIcons()
{
    QListIterator<ResultItem *> it(m_items);

    while (it.hasNext()) {
        ResultItem *item = it.next();
        item->setRowStride(m_rowStride);
    }
}

void ResultScene::clearMatches()
{
    foreach (ResultItem *item, m_items) {
        item->remove();
    }

    m_itemsById.clear();
    m_items.clear();
    m_itemCount = 0;
    m_pageCount = 0;
    setPage(0);
    emit matchCountChanged(0);
}

void ResultScene::setQueryMatches(const QList<Plasma::QueryMatch> &m)
{
    //kDebug() << "============================" << endl << "matches retreived: " << m.count();
    if (m.count() == 0) {
        //kDebug() << "clearing";
        emit itemHoverEnter(0);
        m_clearTimer.start(200);
        return;
    }

    m_clearTimer.stop();
    m_items.clear();

    QList<Plasma::QueryMatch> matches = m;
    QMutableListIterator<Plasma::QueryMatch> newMatchIt(matches);

    // first pass: we try and match up items with existing ids (match persisitence)
    while (!m_itemsById.isEmpty() && newMatchIt.hasNext()) {
        ResultItem *item = addQueryMatch(newMatchIt.next(), false);

        if (item) {
            m_items.append(item);
            newMatchIt.remove();
        }
    }

    // second pass: now we just use any item that exists (item re-use)
    newMatchIt.toFront();
    while (newMatchIt.hasNext()) {
        m_items.append(addQueryMatch(newMatchIt.next(), true));
    }

    // delete the stragglers
    QMapIterator<QString, ResultItem *> it(m_itemsById);
    while (it.hasNext()) {
        it.next().value()->remove();
    }

    // organize the remainders
    int i = 0;
    m_itemsById.clear();

    // this will leave them in *reverse* order
    qSort(m_items.begin(), m_items.end(), ResultItem::compare);

    m_pageCount = m.count();
    m_pageCount = m_pageCount / m_pageStride + (m_pageCount % m_pageStride != 0 ? 1 : 0);
    setPage(0);
    //kDebug() << "gots us" << m_pageCount << "m_pageCount of items";

    emit matchCountChanged(m.count());

    QListIterator<ResultItem*> matchIt(m_items);
    QGraphicsWidget *tab = 0;
    while (matchIt.hasNext()) {
        ResultItem *item = matchIt.next();
        //kDebug()  << item->name() << item->id() << item->priority() << i;
        QGraphicsWidget::setTabOrder(tab, item);
        m_itemsById.insert(item->id(), item);
        item->setIndex(i);

        // it is vital that focus is set *after* the index
        if (i == 0) {
            setFocusItem(item);
        }

        ++i;
        tab = item;
    }

    emit itemHoverEnter(m_items.at(0));
}

ResultItem* ResultScene::addQueryMatch(const Plasma::QueryMatch &match, bool useAnyId)
{
    QMap<QString, ResultItem*>::iterator it = useAnyId ? m_itemsById.begin() : m_itemsById.find(match.id());
    ResultItem *item = 0;
    //kDebug() << "attempting" << match.id();

    if (it == m_itemsById.end()) {
        //kDebug() << "did not find for" << match.id();
        if (useAnyId) {
            //kDebug() << "creating for" << match.id();
            item = new ResultItem(match, 0);
            addItem(item);
            item->hide();
            int rowStride = sceneRect().width() / (ResultItem::BOUNDING_WIDTH);
            item->setRowStride(rowStride);
            connect(item, SIGNAL(activated(ResultItem*)), this, SIGNAL(itemActivated(ResultItem*)));
            connect(item, SIGNAL(hoverEnter(ResultItem*)), this, SIGNAL(itemHoverEnter(ResultItem*)));
            connect(item, SIGNAL(hoverLeave(ResultItem*)), this, SIGNAL(itemHoverLeave(ResultItem*)));
        } else {
            //kDebug() << "returning failure for" << match.id();
            return 0;
        }
    } else {
        item = it.value();
        //kDebug() << "reusing" << item->name() << "for" << match.id();
        item->setMatch(match);
        m_itemsById.erase(it);
    }

    return item;
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
            if (m_cIndex < m_rowStride) {
                if (m_items.size() < m_rowStride) {
                    // we have less than one row of items, so lets just move to the next item
                    m_cIndex = (m_cIndex + 1) % m_items.size();
                } else {
                    m_cIndex = m_items.size() - (m_items.size() % m_rowStride) - 1 + (m_cIndex % m_items.size());
                    if (m_cIndex >= m_items.size()) {
                        // we should be on the bottom row, but there is nothing there; move up one row
                        m_cIndex -= m_rowStride % m_items.size();
                    }
                }
            } else {
                m_cIndex = m_cIndex - m_rowStride;
            }
            break;
        }

        case Qt::Key_Down:{
            if (m_cIndex + m_rowStride >= m_items.size()) {
                // warp to the top
                m_cIndex = (m_cIndex + 1) % m_rowStride % m_items.size();
            } else {
                // next row!
                m_cIndex += m_rowStride;
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

uint ResultScene::pageCount() const
{
    return m_pageCount;
}

void ResultScene::nextPage()
{
    setPage(m_currentPage + 1);
}

void ResultScene::previousPage()
{
    setPage(m_currentPage - 1);
}

void ResultScene::setPage(uint index)
{
    if (index > m_pageCount || index == m_currentPage) {
        return;
    }

    m_currentPage = index;
    setSceneRect(0.0, ((m_currentPage * (m_pageStride / m_rowStride))) * ResultItem::BOUNDING_HEIGHT,
                 width(), height());
}
#include "resultscene.moc"


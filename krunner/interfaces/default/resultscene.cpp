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
#include <QtCore/QMutexLocker>
#include <QtGui/QPainter>
#include <QtCore/QTimeLine>
#include <QtGui/QGraphicsSceneWheelEvent>
#include <QtGui/QGraphicsGridLayout>
#include <QtGui/QGraphicsWidget>
#include <QtGui/QGraphicsProxyWidget>

#include <KDE/KDebug>
#include <KDE/KLineEdit>

#include <Plasma/AbstractRunner>
#include <Plasma/RunnerManager>

#include "resultitem.h"
#include "selectionbar.h"

ResultScene::ResultScene(Plasma::RunnerManager *manager, QObject *parent)
    : QGraphicsScene(parent),
      m_runnerManager(manager),
      m_currentIndex(0)
{
    setItemIndexMethod(NoIndex);

    connect(m_runnerManager, SIGNAL(matchesChanged(const QList<Plasma::QueryMatch>&)),
            this, SLOT(setQueryMatches(const QList<Plasma::QueryMatch>&)));

    m_clearTimer.setSingleShot(true);
    connect(&m_clearTimer, SIGNAL(timeout()), this, SLOT(clearMatches()));

    m_selectionBar = new SelectionBar(0);
    addItem(m_selectionBar);
    m_selectionBar->hide();
    updateItemMargins();

    connect(m_selectionBar, SIGNAL(graphicsChanged()), this, SLOT(updateItemMargins()));
    //QColor bg(255, 255, 255, 126);
    //setBackgroundBrush(bg);
}

ResultScene::~ResultScene()
{
    clearMatches();
    delete m_selectionBar;
}

QSize ResultScene::minimumSizeHint() const
{
    QFontMetrics fm(font());
    return QSize(ResultItem::DEFAULT_ICON_SIZE * 4, (fm.height() * 5) * 3);
}

void ResultScene::resize(int width, int height)
{
    bool resizeItems = width != sceneRect().width();
    setSceneRect(0.0, 0.0, (qreal)width, (qreal)height);

    if (resizeItems) {
        foreach (ResultItem *item, m_items) {
            item->calculateSize();
        }
    }
}

void ResultScene::clearMatches()
{
    foreach (ResultItem *item, m_items) {
        item->remove();
    }

    m_itemsById.clear();
    m_items.clear();
    emit matchCountChanged(0);
}

void ResultScene::setQueryMatches(const QList<Plasma::QueryMatch> &m)
{
    //kDebug() << "============================" << endl << "matches retrieved: " << m.count();
    /*
    foreach (const Plasma::QueryMatch &match, m) {
        kDebug() << "    " << match.id() << match.text();
    }
    */

    if (m.isEmpty()) {
        kDebug() << "clearing";
        emit itemHoverEnter(0);
        m_clearTimer.start(200);
        return;
    }

    //resize(width(), m.count() * ResultItem::BOUNDING_HEIGHT);
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

    emit matchCountChanged(m.count());

    QListIterator<ResultItem*> matchIt(m_items);
    QGraphicsWidget *tab = 0;
    int y = 0;
    while (matchIt.hasNext()) {
        ResultItem *item = matchIt.next();
        //kDebug()  << item->name() << item->id() << item->priority() << i;
        QGraphicsWidget::setTabOrder(tab, item);
        m_itemsById.insert(item->id(), item);
        item->setIndex(i);
        item->setPos(0, y);
        item->show();
        //kDebug() << item->pos();
        y += item->geometry().height();

        // it is vital that focus is set *after* the index
        if (i == 0) {
            setFocusItem(item);
            item->setSelected(true);
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
            item->setContentsMargins(m_itemMarginLeft, m_itemMarginTop,
                                     m_itemMarginRight, m_itemMarginBottom);
            item->hide();
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
    switch (keyEvent->key()) {
        case Qt::Key_Up:
        case Qt::Key_Left:
            selectNextItem();
            break;

        case Qt::Key_Down:
        case Qt::Key_Right:
            selectPreviousItem();
        break;

        default:
            // pass the event to the item
            QGraphicsScene::keyPressEvent(keyEvent);
            return;
        break;
    }
}

void ResultScene::selectNextItem()
{
    ResultItem *currentFocus = dynamic_cast<ResultItem*>(focusItem());
    int m_currentIndex = currentFocus ? currentFocus->index() : 0;

    if (m_currentIndex > 0) {
        --m_currentIndex;
    } else {
        m_currentIndex = m_items.size() - 1;
    }

    setFocusItem(m_items.at(m_currentIndex));
    clearSelection();
    m_items.at(m_currentIndex)->setSelected(true);
}

void ResultScene::selectPreviousItem()
{
    ResultItem *currentFocus = dynamic_cast<ResultItem*>(focusItem());
    int m_currentIndex = currentFocus ? currentFocus->index() : 0;

    ++m_currentIndex;

    if (m_currentIndex >= m_items.size()) {
        m_currentIndex = 0;
    }

    setFocusItem(m_items.at(m_currentIndex));
    clearSelection();
    m_items.at(m_currentIndex)->setSelected(true);
}

void ResultScene::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    if (event->delta() > 0) {
        selectNextItem();
    } else {
        selectPreviousItem();
    }
}

bool ResultScene::launchQuery(const QString &term)
{
    bool temp = !(term.isEmpty() || m_runnerManager->query() == term);
    m_runnerManager->launchQuery(term);
    return temp;
}

bool ResultScene::launchQuery(const QString &term, const QString &runner)
{
    bool temp = !(term.isEmpty() || m_runnerManager->query() == term);
    m_runnerManager->launchQuery(term, runner);
    return temp;
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

/*
Plasma::RunnerManager* ResultScene::manager() const
{
    return m_runnerManager;
}
*/
void ResultScene::updateItemMargins()
{
    m_selectionBar->getMargins(m_itemMarginLeft, m_itemMarginTop,
                               m_itemMarginRight, m_itemMarginBottom);

    foreach (ResultItem *item, m_items) {
        item->setContentsMargins(m_itemMarginLeft, m_itemMarginTop,
                                m_itemMarginRight, m_itemMarginBottom);
    }
}

#include "resultscene.moc"


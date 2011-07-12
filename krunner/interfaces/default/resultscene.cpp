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
#include <QtGui/QGraphicsGridLayout>
#include <QtGui/QGraphicsWidget>
#include <QtGui/QGraphicsProxyWidget>

#include <KDE/KDebug>
#include <KDE/KIconLoader>
#include <KDE/KLineEdit>

#include <Plasma/AbstractRunner>
#include <Plasma/RunnerManager>

#include "selectionbar.h"

ResultScene::ResultScene(SharedResultData *resultData, Plasma::RunnerManager *manager, QWidget *focusBase, QObject *parent)
    : QGraphicsScene(parent),
      m_runnerManager(manager),
      m_viewableHeight(0),
      m_currentIndex(0),
      m_focusBase(focusBase),
      m_resultData(resultData)
{
    setItemIndexMethod(NoIndex);

    connect(m_runnerManager, SIGNAL(matchesChanged(const QList<Plasma::QueryMatch>&)),
            this, SLOT(setQueryMatches(const QList<Plasma::QueryMatch>&)));

    m_clearTimer.setSingleShot(true);
    m_clearTimer.setInterval(200);
    connect(&m_clearTimer, SIGNAL(timeout()), this, SLOT(clearMatches()));

    m_arrangeTimer.setSingleShot(true);
    m_arrangeTimer.setInterval(50);
    connect(&m_arrangeTimer, SIGNAL(timeout()), this, SLOT(arrangeItems()));

    m_selectionBar = new SelectionBar(0);
    connect(m_selectionBar, SIGNAL(appearanceChanged()), this, SLOT(updateItemMargins()));
    connect(m_selectionBar, SIGNAL(targetItemReached(QGraphicsItem*)), this, SLOT(highlightItem(QGraphicsItem*)));
    m_selectionBar->hide();
    updateItemMargins();

    addItem(m_selectionBar);
}

ResultScene::~ResultScene()
{
    clearMatches();
    delete m_selectionBar;
}

QSize ResultScene::minimumSizeHint() const
{
    QFontMetrics fm(font());
    return QSize(KIconLoader::SizeMedium * 4, (fm.height() * 5) * 3);
}

void ResultScene::setWidth(int width)
{
    const bool resizeItems = width != sceneRect().width();

    if (resizeItems) {
        foreach (ResultItem *item, m_items) {
            item->calculateSize(width);
        }

        setSceneRect(itemsBoundingRect());
    }
}

void ResultScene::clearMatches()
{
    clearSelection();
    Plasma::QueryMatch dummy(0);
    foreach (ResultItem *item, m_items) {
        item->hide();
        item->setMatch(dummy);
    }

    m_viewableHeight = 0;
    emit matchCountChanged(0);
}

bool ResultScene::canMoveItemFocus() const
{
    // We prevent a late query result from stealing the item focus from the user
    // The item focus can be moved only if
    // 1) there is no item currently focused
    // 2) the currently focused item is not visible anymore
    // 3) the focusBase widget (the khistorycombobox) has focus (i.e. the user is still typing or waiting) AND the currently focused item has not been hovered

    ResultItem * focusedItem = currentlyFocusedItem();

    return !(focusedItem) ||
            (!m_items.contains(focusedItem)) ||
            (m_focusBase->hasFocus() && !focusedItem->mouseHovered()) ;
}

int ResultScene::viewableHeight() const
{
    return m_viewableHeight;
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
        //kDebug() << "clearing";
        //resize(width(), 0);
        m_clearTimer.start();
        return;
    }

    m_clearTimer.stop();
    const int maxItemsAllowed = 50;

    if (m_items.isEmpty()) {
        QTime t;
        t.start();
        Plasma::QueryMatch dummy(0);
        for (int i = 0; i < maxItemsAllowed; ++i) {
            ResultItem *item = new ResultItem(m_resultData, dummy, m_runnerManager, 0);
            item->setContentsMargins(m_itemMarginLeft, m_itemMarginTop,
                                     m_itemMarginRight, m_itemMarginBottom);
            item->hide();
            item->setIndex(i);
            connect(item, SIGNAL(ensureVisibility(QGraphicsItem*)), this, SIGNAL(ensureVisibility(QGraphicsItem*)));
            connect(item, SIGNAL(activated(ResultItem*)), this, SIGNAL(itemActivated(ResultItem*)));
            connect(item, SIGNAL(sizeChanged(ResultItem*)), this, SLOT(scheduleArrangeItems()));

            m_items << item;
            addItem(item);
        }

        arrangeItems();
        kDebug() << "creating all items took" << t.elapsed();
    }

    // we keep track of what was previously focused so if the user changes focus
    // and more items come in ... we don't reset that on them unecessarily
    // see the keepFocus bool down below
    ResultItem *currentFocus = currentlyFocusedItem();
    QString lastFocusId;
    if (currentFocus && currentFocus->isValid()) {
        lastFocusId = currentFocus->id();
    }

    QList<Plasma::QueryMatch> matches = m;
    qSort(matches.begin(), matches.end());
    QListIterator<Plasma::QueryMatch> mit(matches);
    mit.toBack();
    QListIterator<ResultItem *> rit(m_items);
    QGraphicsWidget *prevTabItem = 0;

    const int viewableHeight = m_viewableHeight;
    while (mit.hasPrevious() && rit.hasNext()) {
        ResultItem * item = rit.next();
        item->setMatch(mit.previous());
        prevTabItem = item->arrangeTabOrder(prevTabItem);
        item->show();
        m_viewableHeight = item->sceneBoundingRect().bottom();
    }

    Plasma::QueryMatch dummy(0);
    while (rit.hasNext()) {
        ResultItem *item = rit.next();
        item->hide();
        if (item->isValid()) {
            item->setMatch(dummy);
        }
    }

    const bool keepFocus = currentFocus && currentFocus->isValid() && currentFocus->id() == lastFocusId;
    if (!keepFocus) {
        clearSelection();
        ResultItem *first = m_items.at(0);
        setFocusItem(first);
        first->setSelected(true);
        first->highlight(true);
        emit ensureVisibility(first);
    }

    emit matchCountChanged(qMin(m.count(), maxItemsAllowed));
    if (viewableHeight != m_viewableHeight) {
        emit viewableHeightChanged();
    }
}

void ResultScene::scheduleArrangeItems()
{
    if (!m_arrangeTimer.isActive()) {
        m_arrangeTimer.start();
    }
}

void ResultScene::arrangeItems()
{
    int y = 0;
    QListIterator<ResultItem*> matchIt(m_items);
    const int viewableHeight = m_viewableHeight;
    while (matchIt.hasNext()) {
        ResultItem *item = matchIt.next();
        //kDebug()  << item->name() << item->id() << item->priority() << i;

        item->setPos(0, y);
        //kDebug() << item->pos();
        y += item->geometry().height();
        if (item->isVisible()) {
            m_viewableHeight = item->sceneBoundingRect().bottom();
        }
    }

    //kDebug() << "setting scene rect to" << itemsBoundingRect();
    const QRectF newRect = itemsBoundingRect();
    setSceneRect(newRect);
    if (viewableHeight != m_viewableHeight) {
        emit viewableHeightChanged();
    }
}

void ResultScene::highlightItem(QGraphicsItem *item)
{
    ResultItem *rItem = dynamic_cast<ResultItem *>(item);
    if (rItem) {
        rItem->highlight(true);
    }
}

void ResultScene::focusInEvent(QFocusEvent *focusEvent)
{

    // The default implementation of focusInEvent assumes that if the scene has no focus
    // then it has no focused item; thus, when a scene gains focus, focusInEvent gives
    // focus to the last focused item.
    // In our case this assumption is not true, as an item can be focused before the scene,
    // therefore we revert the behaviour by re-selecting the previously selected item

    ResultItem *currentFocus = currentlyFocusedItem();

    QGraphicsScene::focusInEvent(focusEvent);

    switch (focusEvent->reason()) {
    case Qt::TabFocusReason:
    case Qt::BacktabFocusReason:
        // on tab focus in, we want to actually select the second item
        // since the first item is always "passively" selected by default
        if (!currentFocus || currentFocus == m_items.first()) {
            ResultItem *newFocus = m_items[1];
            if (newFocus->isVisible()) {
                setFocusItem(newFocus);
                emit ensureVisibility(newFocus);
            }
        }
        break;
    default:
        if (currentFocus) {
            setFocusItem(currentFocus);
        }
        break;
    }
}

void ResultScene::keyPressEvent(QKeyEvent * keyEvent)
{
    //kDebug() << "m_items (size): " << m_items.size() << "\n";
    switch (keyEvent->key()) {
        case Qt::Key_Up:
        case Qt::Key_Left:
            selectPreviousItem();
            break;

        case Qt::Key_Down:
        case Qt::Key_Right:
            selectNextItem();
        break;

        default:
            // pass the event to the item
            QGraphicsScene::keyPressEvent(keyEvent);
            return;
        break;
    }
}

ResultItem* ResultScene::currentlyFocusedItem() const
{
    QGraphicsWidget* widget = static_cast<QGraphicsWidget*>(focusItem());
    if (!widget) {
        return 0;
    }

    ResultItem *currentFocus = qobject_cast<ResultItem*>(widget);
    if (!currentFocus) {
        //If we focused an action button, find the resultItem
        //FIXME: the config button
        currentFocus = qobject_cast<ResultItem*>(widget->parentWidget()->parentWidget());
    }

    return currentFocus;
}

void ResultScene::selectPreviousItem()
{
    ResultItem *currentFocus = currentlyFocusedItem();
    int currentIndex = currentFocus ? currentFocus->index() : 0;

    if (currentIndex == 0) {
        m_focusBase->setFocus();
        return;
    }

    currentFocus = m_items.at(currentIndex - 1);

    if (currentFocus->isVisible()) {
        setFocusItem(currentFocus);
        emit ensureVisibility(currentFocus);
    }
}

void ResultScene::selectNextItem()
{
    ResultItem *currentFocus = currentlyFocusedItem();
    int currentIndex = currentFocus ? currentFocus->index() : 0;

    do {
        ++currentIndex;
        if (currentIndex >= m_items.size()) {
            return;
        }
        currentFocus = m_items.at(currentIndex);
    } while (!currentFocus->isVisible() && currentIndex < m_items.size());

    if (currentFocus->isVisible()) {
        setFocusItem(currentFocus);
        emit ensureVisibility(currentFocus);
    }
}

bool ResultScene::launchQuery(const QString &term)
{
    bool temp = !(term.trimmed().isEmpty() || m_runnerManager->query() == term.trimmed());
    m_runnerManager->launchQuery(term);
    return temp;
}

bool ResultScene::launchQuery(const QString &term, const QString &runner)
{
    bool temp = !(term.trimmed().isEmpty() || m_runnerManager->query() == term.trimmed() ) || (!runner.isEmpty());
    m_runnerManager->launchQuery(term, runner);
    return temp;
}

void ResultScene::clearQuery()
{
    //m_selectionBar->setTargetItem(0);
    setFocusItem(0);
    clearSelection();
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

void ResultScene::updateItemMargins()
{
    m_selectionBar->getContentsMargins(&m_itemMarginLeft, &m_itemMarginTop,
                                       &m_itemMarginRight, &m_itemMarginBottom);

    foreach (ResultItem *item, m_items) {
        item->setContentsMargins(m_itemMarginLeft, m_itemMarginTop,
                                m_itemMarginRight, m_itemMarginBottom);
    }
}

#include "resultscene.moc"


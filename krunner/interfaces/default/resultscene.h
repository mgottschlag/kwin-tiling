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

#ifndef __ResultScene_h__
#define __ResultScene_h__

#include <QtCore/QList>
#include <QtCore/QMultiMap>
#include <QtCore/QTimer>
#include <QtGui/QGraphicsScene>

#include <Plasma/QueryMatch>

namespace Plasma
{
    class RunnerManager;
}

class ResultItem;
class SelectionBar;

class ResultScene : public QGraphicsScene
{
    Q_OBJECT

    public:
        explicit ResultScene(Plasma::RunnerManager *runnerManager, QWidget *focusBase, QObject *parent = 0);
        ~ResultScene();

        void resize(int width, int height);
        ResultItem* defaultResultItem() const;
        void run(ResultItem* item) const;
        QSize minimumSizeHint() const;
        void setItemsAcceptHoverEvents(bool enable);
        bool itemsAcceptHoverEvents();
//        Plasma::RunnerManager* manager() const;


    public slots:
        void setQueryMatches(const QList<Plasma::QueryMatch> &matches);
        bool launchQuery(const QString &term);
        bool launchQuery(const QString &term, const QString &runner);
        void clearQuery();

    signals:
        void itemActivated(ResultItem *item);
        void itemHoverEnter(ResultItem *item);
        void itemHoverLeave(ResultItem *item);
        void matchCountChanged(int count);
        void ensureVisibility(QGraphicsItem *item);
        void actionTriggered();

    protected:
        void keyPressEvent(QKeyEvent * keyEvent);
        void focusInEvent(QFocusEvent *focusEvent);
        void focusOutEvent(QFocusEvent *focusEvent);

    private:
        void selectPreviousItem();
        void selectNextItem();

        ResultItem* addQueryMatch(const Plasma::QueryMatch &match, bool useAnyId);

        bool canMoveItemFocus() const;

    private slots:
        void clearMatches();
        void updateItemMargins();
        void arrangeItems(ResultItem *);
        void initItemsHoverEvents();

    private:
        Plasma::RunnerManager *m_runnerManager;

        QSize       m_size;
        QTimer      m_clearTimer;
        QTimer      m_hoverTimer;

        QList<ResultItem *>  m_items;
        QMultiMap<QString, ResultItem *>  m_itemsById;
        SelectionBar *m_selectionBar;

        int m_currentIndex;
        qreal m_itemMarginLeft;
        qreal m_itemMarginTop;
        qreal m_itemMarginRight;
        qreal m_itemMarginBottom;

        QWidget *m_focusBase;
        bool m_itemsAcceptHoverEvents;
};

#endif

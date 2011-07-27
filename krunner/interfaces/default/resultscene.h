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

#include "resultitem.h"

namespace Plasma
{
    class RunnerManager;
}

class SelectionBar;

class ResultScene : public QGraphicsScene
{
    Q_OBJECT

    public:
        explicit ResultScene(SharedResultData *resultData, Plasma::RunnerManager *runnerManager, QWidget *focusBase, QObject *parent = 0);
        ~ResultScene();

        void setWidth(int width);
        ResultItem* defaultResultItem() const;
        void run(ResultItem* item) const;
        QSize minimumSizeHint() const;
        int viewableHeight() const;

    public slots:
        void setQueryMatches(const QList<Plasma::QueryMatch> &matches);
        void queryCleared();

    signals:
        void itemActivated(ResultItem *item);
        void matchCountChanged(int count);
        void viewableHeightChanged();
        void ensureVisibility(QGraphicsItem *item);

    protected:
        void keyPressEvent(QKeyEvent * keyEvent);
        void focusInEvent(QFocusEvent *focusEvent);

    private:
        void selectPreviousItem();
        void selectNextItem();

        ResultItem* currentlyFocusedItem() const;

        bool canMoveItemFocus() const;
        void arrangeItems(bool setFocusAndTabbing);

    private slots:
        void clearMatches();
        void updateItemMargins();
        void scheduleArrangeItems();
        void arrangeItems();
        void highlightItem(QGraphicsItem *item);

    private:
        Plasma::RunnerManager *m_runnerManager;

        QTimer      m_clearTimer;
        QTimer      m_arrangeTimer;

        QList<ResultItem *> m_items;
        SelectionBar *m_selectionBar;

        int m_viewableHeight;
        int m_currentIndex;
        qreal m_itemMarginLeft;
        qreal m_itemMarginTop;
        qreal m_itemMarginRight;
        qreal m_itemMarginBottom;

        QWidget *m_focusBase;

        SharedResultData *m_resultData;
};

#endif

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
#include <QtGui/QIcon>
#include <QtGui/QPixmap>
#include <plasma/querymatch.h>

namespace Plasma
{
    class RunnerManager;
}

class ResultItem;

class ResultScene : public QGraphicsScene
{
    Q_OBJECT

    public:
        explicit ResultScene(QObject * parent = 0);
        ~ResultScene();

        void resize(int width, int height);

        void removeMatches(const QList<ResultItem *> &items);

        ResultItem* defaultResultItem() const;

        Plasma::RunnerManager *m_runnerManager;

    public slots:
        void setQueryMatches(const QList<Plasma::QueryMatch> &matches);
        void launchQuery(const QString &term);
        void clearQuery();

        void itemHoverEnterSlot(ResultItem *item);
        void itemHoverLeaveSlot(ResultItem *item);

    signals:
        void itemActivated(ResultItem *item);
        void itemHoverEnter(ResultItem *item);
        void itemHoverLeave(ResultItem *item);

    protected:
        void keyPressEvent( QKeyEvent * keyEvent );

    private:
        void addQueryMatch(const Plasma::QueryMatch &match);
        void removeQueryMatch(const Plasma::QueryMatch &match);
        void removeMatch(ResultItem *item);
        void performResize(int width, int height);

        QSize       m_size;
        QPixmap     m_backPixmap;
        QPixmap     m_forePixmap1;
        QPixmap     m_forePixmap2;

        //for resize optimisation
        QTimer      m_resizeTimer;
        QTimer      m_clearTimer;
        bool        m_successfullyResized;
        int         m_resizeW;
        int         m_resizeH;

        int m_itemCount;
        int m_updateId;
        QList<ResultItem *>  m_items;
        QMultiMap<QString, ResultItem *>  m_itemsById;

        // the UI items
        QGraphicsWidget *m_mainWidget;
        QGraphicsWidget *m_iconArea;

        int m_cIndex;

    private slots:
        void layoutIcons();
        void slotArrowResultItemPressed();
        void slotArrowResultItemReleased();
        void indexReleased(int index);
        void clearMatches();
};

#endif

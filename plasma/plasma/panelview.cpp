/*
*   Copyright 2007 by Matt Broadstone <mbroadst@kde.org>
*   Copyright 2007 by Robert Knight <robertknight@gmail.com>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU Library General Public License version 2,
*   or (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details
*
*   You should have received a copy of the GNU Library General Public
*   License along with this program; if not, write to the
*   Free Software Foundation, Inc.,
*   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "panelview.h"
#include "panelview.moc"

#include <QApplication>
#include <QDesktopWidget>
#include <QTimeLine>

#include <KWindowSystem>
#include <KDebug>

#include <plasma/containment.h>
#include <plasma/corona.h>
#include <plasma/plasma.h>
#include <plasma/svg.h>

PanelView::PanelView(Plasma::Containment *panel, QWidget *parent)
    : QGraphicsView(parent),
      m_containment(panel)
{
    Q_ASSERT(qobject_cast<Plasma::Corona*>(m_containment->scene()));
    setScene(m_containment->scene());
    updatePanelGeometry();
    kDebug() << "Panel geometry is" << m_containment->geometry();

    // Graphics view setup
    setFrameStyle(QFrame::NoFrame);
    setAutoFillBackground(true);
    setCacheMode(QGraphicsView::CacheBackground);
   // setDragMode(QGraphicsView::RubberBandDrag);
    setCacheMode(QGraphicsView::CacheBackground);
    setInteractive(true);
    setAcceptDrops(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // KWin setup
    KWindowSystem::setType(winId(), NET::Dock);
    KWindowSystem::setState(winId(), NET::Sticky | NET::KeepAbove);
    KWindowSystem::setOnAllDesktops(winId(), true);

    updateStruts();
}


void PanelView::setLocation(Plasma::Location loc)
{
    m_containment->setLocation(loc);
    updatePanelGeometry();
}

Plasma::Location PanelView::location() const
{
    return m_containment->location();	
}

Plasma::Containment *PanelView::containment() const
{
    return m_containment;
}

Plasma::Corona *PanelView::corona() const
{
    return qobject_cast<Plasma::Corona*>(scene());
}

void PanelView::updatePanelGeometry()
{
    kDebug() << "New panel geometry is" << m_containment->geometry();
    QPoint pos = m_containment->pos().toPoint();
    QSize size = m_containment->size().toSize();
    QRect geom(pos, size);
    setGeometry(geom);
    pos = m_containment->scenePos().toPoint();
    geom.moveTopLeft(pos);
    kDebug() << "I think the panel is at " << geom;
    setSceneRect(geom);
}

void PanelView::updateStruts()
{
    NETExtendedStrut strut;

    //QRect geom = geometry();
    //QRect virtRect(QApplication::desktop()->geometry());

    switch (location())
    {
        case Plasma::TopEdge:
            strut.top_width = height();
            //strut.top_width = geom.y() + h;
            //strut.top_start = x();
            //strut.top_end = x() + width() - 1;
            break;

        case Plasma::BottomEdge:
            // also claim the non-visible part at the bottom
            strut.bottom_width = height();
            //strut.bottom_width = (virtRect.bottom() - geom.bottom()) + h;
            //strut.bottom_start = x();
            //strut.bottom_end = x() + width() - 1;
            break;

        case Plasma::RightEdge:
            strut.right_width = width();
            //strut.right_width = (virtRect.right() - geom.right()) + w;
            //strut.right_start = y();
            //strut.right_end = y() + height() - 1;
            break;

        case Plasma::LeftEdge:
            strut.left_width = width();
            //strut.left_width = geom.x() + w;
            //strut.left_start = y();
            //strut.left_end = y() + height() - 1;
            break;

        default:
            break;
    }
    KWindowSystem::setStrut(winId(), strut.left_width,
                                     strut.right_width,
                                     strut.top_width,
                                     strut.bottom_width);
}

void PanelView::moveEvent(QMoveEvent *event)
{
    QWidget::moveEvent(event);
    updateStruts();
}

void PanelView::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateStruts();
}


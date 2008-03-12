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

#include <QApplication>
#include <QDesktopWidget>
#include <QTimeLine>

#include <KWindowSystem>
#include <KDebug>

#include <plasma/containment.h>
#include <plasma/corona.h>
#include <plasma/plasma.h>
#include <plasma/svg.h>

#include "plasmaapp.h"

PanelView::PanelView(Plasma::Containment *panel, QWidget *parent)
    : Plasma::View(panel, parent)
{
    Q_ASSERT(qobject_cast<Plasma::Corona*>(panel->scene()));
    updatePanelGeometry();

    if (panel) {
        connect(panel, SIGNAL(showAddWidgets()), this, SLOT(showAppletBrowser()));
        connect(panel, SIGNAL(destroyed(QObject*)), this, SLOT(deleteLater()));
        connect(this, SIGNAL(sceneRectAboutToChange()), this, SLOT(updatePanelGeometry()));
    }

    kDebug() << "Panel geometry is" << panel->geometry();

    // Graphics view setup
    setFrameStyle(QFrame::NoFrame);
    //setAutoFillBackground(true);
    //setDragMode(QGraphicsView::RubberBandDrag);
    //setCacheMode(QGraphicsView::CacheBackground);
    setInteractive(true);
    setAcceptDrops(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // KWin setup
    KWindowSystem::setType(winId(), NET::Dock);
    KWindowSystem::setState(winId(), NET::Sticky);
    KWindowSystem::setOnAllDesktops(winId(), true);

    updateStruts();
}


void PanelView::setLocation(Plasma::Location loc)
{
    containment()->setLocation(loc);
}

Plasma::Location PanelView::location() const
{
    return containment()->location();
}

Plasma::Corona *PanelView::corona() const
{
    return qobject_cast<Plasma::Corona*>(scene());
}

void PanelView::updatePanelGeometry()
{
    kDebug() << "New panel geometry is" << containment()->geometry();
    QSize size = containment()->size().toSize();
    QRect geom(QPoint(0,0), size);
    int screen = containment()->screen();

    if (screen < 0) {
        //TODO: is there a valid use for -1 with a panel? floating maybe?
        screen = 0;
    }

    QRect screenGeom = QApplication::desktop()->screenGeometry(screen);

    //FIXME: we need to support center, left, right, etc.. perhaps
    //       pixel precision placed containments as well?
    switch (location()) {
        case Plasma::TopEdge:
            geom.moveTopLeft(screenGeom.topLeft());
            break;
        case Plasma::LeftEdge:
            geom.moveTopLeft(screenGeom.topLeft());
            break;
        case Plasma::RightEdge:
            geom.moveTopLeft(QPoint(screenGeom.right() - size.width() + 1, screenGeom.top()));
            break;
        case Plasma::BottomEdge:
        default:
            geom.moveTopLeft(QPoint(screenGeom.left(), screenGeom.bottom() - size.height() + 1));
            break;
    }

    kDebug() << (QObject*)this << "thinks its panel is at " << geom;
    setGeometry(geom);
}

void PanelView::showAppletBrowser()
{
    PlasmaApp::self()->showAppletBrowser(containment());
}

void PanelView::updateStruts()
{
    NETExtendedStrut strut;

    QRect thisScreen = QApplication::desktop()->screenGeometry(containment()->screen());
    QRect wholeScreen = QApplication::desktop()->screenGeometry();

    // extended struts are to the combined screen geoms, not the single screen
    int leftOffset = wholeScreen.x() - thisScreen.x();
    int rightOffset = wholeScreen.right() - thisScreen.right();
    int bottomOffset = wholeScreen.bottom() - thisScreen.bottom();
    int topOffset = wholeScreen.top() - thisScreen.top();
    kDebug() << "screen l/r/b/t offsets are:" << leftOffset << rightOffset << bottomOffset << topOffset;

    switch (location())
    {
        case Plasma::TopEdge:
            strut.top_width = height() + topOffset;
            strut.top_start = x();
            strut.top_end = x() + width() - 1;
            break;

        case Plasma::BottomEdge:
            strut.bottom_width = height() + bottomOffset;
            strut.bottom_start = x();
            strut.bottom_end = x() + width() - 1;
            break;

        case Plasma::RightEdge:
            strut.right_width = width() + rightOffset;
            strut.right_start = y();
            strut.right_end = y() + height() - 1;
            break;

        case Plasma::LeftEdge:
            strut.left_width = width() + leftOffset;
            strut.left_start = y();
            strut.left_end = y() + height() - 1;
            break;

        default:
            break;
    }

    KWindowSystem::setExtendedStrut(winId(), strut.left_width,
                                             strut.left_start,
                                             strut.left_end,
                                             strut.right_width,
                                             strut.right_start,
                                             strut.right_end,
                                             strut.top_width,
                                             strut.top_start,
                                             strut.top_end,
                                             strut.bottom_width,
                                             strut.bottom_start,
                                             strut.bottom_end);
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

#include "panelview.moc"


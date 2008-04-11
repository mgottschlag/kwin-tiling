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


PanelView::PanelView(Plasma::Containment *panel, int id, QWidget *parent)
    : Plasma::View(panel, id, parent)
{
    Q_ASSERT(qobject_cast<Plasma::Corona*>(panel->scene()));
    
    m_viewConfig =  config();

    m_offset = m_viewConfig.readEntry("Offset", 0);
    m_alignment = alignmentFilter((Qt::Alignment)m_viewConfig.readEntry("Alignment", (int)Qt::AlignLeft));

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

    // Don't change the geometry again if a same one was asked
    if (geom == screenGeom) {
        return;
    }

    if (m_alignment != Qt::AlignCenter) {
        m_offset = qMax(m_offset, 0);
    }


    //Sanity controls
    switch (location()) {
    case Plasma::TopEdge:
    case Plasma::BottomEdge:
        //resize the panel if is too large
        if (geom.width() > screenGeom.width()) {
            geom.setWidth(screenGeom.width());
        }

        //move the panel left/right if there is not enough room
        if (m_alignment == Qt::AlignLeft) {
             if (m_offset + screenGeom.left() + geom.width() > screenGeom.right() + 1) {
                 m_offset = screenGeom.right() - geom.width();
             }
        } else if (m_alignment == Qt::AlignRight) {
             if (screenGeom.right() - m_offset - geom.width() < 0 ) {
                 m_offset = screenGeom.right() - geom.width();
             }
        } else if (m_alignment == Qt::AlignCenter) {
             if (screenGeom.center().x() + m_offset + geom.width()/2 > screenGeom.right() + 1) {
                 m_offset = screenGeom.right() - geom.width()/2 - screenGeom.center().x();
             } else if (screenGeom.center().x() + m_offset - geom.width()/2 < 0) {
                 m_offset = screenGeom.center().x() - geom.width()/2;
             }
        }
        break;

    case Plasma::LeftEdge:
    case Plasma::RightEdge:
        //resize the panel if is too tall
        if (geom.height() > screenGeom.height()) {
            geom.setHeight(screenGeom.height());
        }

        //move the panel bottom if there is not enough room
        //FIXME: still using alignleft/alignright is simpler and less error prone, but aligntop/alignbottom is more correct?
        if (m_alignment == Qt::AlignLeft) {
            if (m_offset + screenGeom.top() + geom.height() > screenGeom.bottom() + 1) {
                m_offset = screenGeom.height() - geom.height();
            }
        } else if (m_alignment == Qt::AlignRight) {
            if (screenGeom.bottom() - m_offset - geom.height() < 0) {
                m_offset = screenGeom.bottom() - geom.height();
            }
        } else if (m_alignment == Qt::AlignCenter) {
            if (screenGeom.center().y() + m_offset + geom.height()/2 > screenGeom.bottom() + 1) {
                m_offset = screenGeom.bottom() - geom.height()/2 - screenGeom.center().y();
             } else if (screenGeom.center().y() + m_offset - geom.width()/2 < 0) {
                m_offset = screenGeom.center().y() - geom.width()/2;
             }
        }
        break;

    //TODO: floating panels (probably they will save their own geometry)
    default:
        break;
    }

    //Actual movement
    switch (location()) {
    case Plasma::TopEdge:
        if (m_alignment == Qt::AlignLeft) {
            geom.moveTopLeft(QPoint(m_offset, screenGeom.top()));
        } else if (m_alignment == Qt::AlignRight) {
            geom.moveTopRight(QPoint(screenGeom.right() - m_offset, screenGeom.top()));
        } else if (m_alignment == Qt::AlignCenter) {
            geom.moveCenter(QPoint(screenGeom.center().x() + m_offset, screenGeom.top() + geom.height()/2  - 1));
        }

        //enable borders if needed
        //containment()->setGeometry(QRect(geom.left(), containment()->geometry().top(), geom.width(), geom.height()));
        break;

    case Plasma::LeftEdge:
        if (m_alignment == Qt::AlignLeft) {
            geom.moveTopLeft(QPoint(screenGeom.left(), m_offset));
        } else if (m_alignment == Qt::AlignRight) {
            geom.moveBottomLeft(QPoint(screenGeom.left(), screenGeom.bottom() - m_offset));
        } else if (m_alignment == Qt::AlignCenter) {
            geom.moveCenter(QPoint(screenGeom.left()+geom.height()/2, screenGeom.center().y() + m_offset));
        }

        //enable borders if needed
        //containment()->setGeometry(QRect(containment()->geometry().left(), geom.top(), geom.width(), geom.height()));
        break;

    case Plasma::RightEdge:
        if (m_alignment == Qt::AlignLeft) {
            geom.moveTopLeft(QPoint(screenGeom.right() - size.width() + 1, m_offset));
        } else if (m_alignment == Qt::AlignRight) {
            geom.moveBottomLeft(QPoint(screenGeom.right() - size.width() + 1, screenGeom.bottom() - m_offset));
        } else if (m_alignment == Qt::AlignCenter) {
            geom.moveCenter(QPoint(screenGeom.right() - size.width()/2 + 1, screenGeom.center().y() + m_offset));
        }

        //enable borders if needed
        //containment()->setGeometry(QRect(containment()->geometry().left(), geom.top(), geom.width(), geom.height()));
        break;

    case Plasma::BottomEdge:
    default:
        if (m_alignment == Qt::AlignLeft) {
            geom.moveTopLeft(QPoint(m_offset, screenGeom.bottom() - size.height() + 1));
        } else if (m_alignment == Qt::AlignRight) {
            geom.moveTopRight(QPoint(screenGeom.right() - m_offset, screenGeom.bottom() - size.height() + 1));
        } else if (m_alignment == Qt::AlignCenter) {
            geom.moveCenter(QPoint(screenGeom.center().x() + m_offset, screenGeom.bottom() - size.height()/2 + 1));
        }

        //enable borders if needed
        //containment()->setGeometry(QRect(geom.left(), containment()->geometry().top(), geom.width(), geom.height()));
        break;
    }

    kDebug() << (QObject*)this << "thinks its panel is at " << geom;

    setGeometry(geom);
    
}

void PanelView::setOffset(int newOffset)
{
    m_offset = newOffset;
}

int PanelView::offset() const
{
    return m_offset;
}

void PanelView::setAlignment(Qt::Alignment align)
{
    m_alignment = alignmentFilter(align);
}

Qt::Alignment PanelView::alignment() const
{
    return m_alignment;
}

void PanelView::showAppletBrowser()
{
    PlasmaApp::self()->showAppletBrowser(containment());
}

void PanelView::saveConfig()
{
    m_viewConfig.writeEntry("Offset", m_offset);
    m_viewConfig.writeEntry("Alignment", (int)m_alignment);
}

Qt::Alignment PanelView::alignmentFilter(Qt::Alignment align) const
{
    //If it's not a supported alignment default to Qt::AlignLeft
    if (align == Qt::AlignLeft || align == Qt::AlignRight || align == Qt::AlignCenter) {
        return align;
    } else {
        return Qt::AlignLeft;
    }
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


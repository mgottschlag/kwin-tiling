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
#include <QGraphicsLinearLayout>
#include <QTimeLine>
#include <QTimer>
#include <QX11Info>

#include <KWindowSystem>
#include <KDebug>

#include <plasma/containment.h>
#include <plasma/corona.h>
#include <plasma/plasma.h>
#include <plasma/svg.h>
#include <plasma/theme.h>

#include "panelappletoverlay.h"
#include "panelcontroller.h"
#include "plasmaapp.h"

PanelView::PanelView(Plasma::Containment *panel, int id, QWidget *parent)
    : Plasma::View(panel, id, parent),
      m_panelController(0),
      m_timeLine(0),
#ifdef Q_WS_X11
      m_unhideTrigger(None),
#endif
      m_lastHorizontal(true),
      m_editting(false),
      m_autohide(false),
      m_windowsCover(false),
      m_firstPaint(true)
{
    Q_ASSERT(qobject_cast<Plasma::Corona*>(panel->scene()));
    KConfigGroup viewConfig = config();

    m_offset = viewConfig.readEntry("Offset", 0);
    m_alignment = alignmentFilter((Qt::Alignment)viewConfig.readEntry("Alignment", (int)Qt::AlignLeft));
    m_autohide = viewConfig.readEntry("autohide", m_autohide);
    m_windowsCover = !viewConfig.readEntry("letWindowsCover", m_windowsCover);

    // pinchContainment calls updatePanelGeometry for us

    QRect screenRect = QApplication::desktop()->screenGeometry(containment()->screen());
    m_lastHorizontal = isHorizontal();
    KConfigGroup sizes = KConfigGroup(&viewConfig, "Sizes");
    m_lastSeenSize = sizes.readEntry("lastsize", m_lastHorizontal ? screenRect.width() : screenRect.height());
    pinchContainment(screenRect);
    m_lastMin = containment()->minimumSize();
    m_lastMax = containment()->maximumSize();


    if (panel) {
        connect(panel, SIGNAL(showAddWidgetsInterface(QPointF)), this, SLOT(showAppletBrowser()));
        connect(panel, SIGNAL(destroyed(QObject*)), this, SLOT(deleteLater()));
        connect(panel, SIGNAL(toolBoxToggled()), this, SLOT(togglePanelController()));
    }

    connect(this, SIGNAL(sceneRectAboutToChange()), this, SLOT(updatePanelGeometry()));

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
    KWindowSystem::setOnAllDesktops(winId(), true);
    unsigned long state = NET::Sticky;
    if (!m_windowsCover) {
        state |= NET::StaysOnTop;
    }
    KWindowSystem::setState(winId(), state);

#ifdef Q_WS_WIN
    registerAccessBar(winId(), true);
#endif

    updateStruts();
}

PanelView::~PanelView()
{
#ifdef Q_WS_WIN
    registerAccessBar(winId(), false);
#endif
}

void PanelView::setLocation(Plasma::Location location)
{
    Plasma::Containment *c = containment();
    QSizeF s = c->size();
    QSizeF min = c->minimumSize();
    QSizeF max = c->maximumSize();
    qreal panelWidth = s.width();
    qreal panelHeight = s.height();

    Plasma::FormFactor formFactor = c->formFactor();
    bool wasHorizontal = formFactor == Plasma::Horizontal;
    bool wasFullSize = m_lastSeenSize == (wasHorizontal ? s.width() : s.height());

    if (location == Plasma::TopEdge || location == Plasma::BottomEdge) {
        if (!wasHorizontal) {
            // we're switching! swap the sizes about
            panelHeight = s.width();
            if (wasFullSize) {
                QRect screenGeom = QApplication::desktop()->screenGeometry(c->screen());
                panelWidth = screenGeom.width();
            } else {
                panelWidth = s.height();
            }
            min = QSizeF(panelWidth, min.width());
            max = QSizeF(panelWidth, max.width());
        }

        formFactor = Plasma::Horizontal;
    } else {
        if (wasHorizontal) {
            // we're switching! swap the sizes about

            if (wasFullSize) {
                QRect screenGeom = QApplication::desktop()->screenGeometry(c->screen());
                panelHeight = screenGeom.height();
            } else {
                panelHeight = s.width();
            }

            panelWidth = s.height();
            min = QSizeF(min.height(), panelHeight);
            max = QSizeF(max.height(), panelHeight);
        }

        formFactor = Plasma::Vertical;
    }

    //kDebug() << "!!!!!!!!!!!!!!!!!! about to set to" << location << panelHeight << formFactor;
    disconnect(this, SIGNAL(sceneRectAboutToChange()), this, SLOT(updatePanelGeometry()));
    c->setFormFactor(formFactor);
    c->setLocation(location);

    c->setMinimumSize(0, 0);
    c->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    c->resize(panelWidth, panelHeight);
    c->setMinimumSize(min);
    c->setMaximumSize(max);

    QRect screenRect = QApplication::desktop()->screenGeometry(c->screen());
    pinchContainment(screenRect);
    //updatePanelGeometry();
    connect(this, SIGNAL(sceneRectAboutToChange()), this, SLOT(updatePanelGeometry()));
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
    Plasma::Containment *c = containment();
    //kDebug() << "New panel geometry is" << c->geometry();

    QSize size = c->size().toSize();
    QRect geom(QPoint(0,0), size);
    int screen = c->screen();

    if (screen < 0) {
        //TODO: is there a valid use for -1 with a panel? floating maybe?
        screen = 0;
    }

    QRect screenGeom = QApplication::desktop()->screenGeometry(screen);

    if (m_alignment != Qt::AlignCenter) {
        m_offset = qMax(m_offset, 0);
    }

    //Sanity controls
    switch (location()) {
    case Plasma::TopEdge:
    case Plasma::BottomEdge:
        if (m_alignment != Qt::AlignCenter) {
            m_offset = qMax(m_offset, screenGeom.left());
        }
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
             if (screenGeom.right() - m_offset - geom.width() < -1 ) {
                 m_offset = screenGeom.right() - geom.width();
             }
        } else if (m_alignment == Qt::AlignCenter) {
             if (screenGeom.center().x() + m_offset + geom.width()/2 > screenGeom.right() + 1) {
                 m_offset = screenGeom.right() - geom.width()/2 - screenGeom.center().x();
             } else if (screenGeom.center().x() + m_offset - geom.width()/2 < -1) {
                 m_offset = screenGeom.center().x() - geom.width()/2;
             }
        }
        break;

    case Plasma::LeftEdge:
    case Plasma::RightEdge:
        if (m_alignment != Qt::AlignCenter) {
            m_offset = qMax(m_offset, screenGeom.top());
        }
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
            if (screenGeom.bottom() - m_offset - geom.height() < -1) {
                m_offset = screenGeom.bottom() - geom.height();
            }
        } else if (m_alignment == Qt::AlignCenter) {
            if (screenGeom.center().y() + m_offset + geom.height()/2 > screenGeom.bottom() + 1) {
                m_offset = screenGeom.bottom() - geom.height()/2 - screenGeom.center().y();
             } else if (screenGeom.center().y() + m_offset - geom.width()/2 < -1) {
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
        //c->setGeometry(QRect(geom.left(), c->geometry().top(), geom.width(), geom.height()));
        break;

    case Plasma::LeftEdge:
        if (m_alignment == Qt::AlignLeft) {
            geom.moveTopLeft(QPoint(screenGeom.left(), m_offset));
        } else if (m_alignment == Qt::AlignRight) {
            geom.moveBottomLeft(QPoint(screenGeom.left(), screenGeom.bottom() - m_offset));
        } else if (m_alignment == Qt::AlignCenter) {
            geom.moveCenter(QPoint(screenGeom.left()+size.width()/2 - 1, screenGeom.center().y() + m_offset -1));
        }

        //enable borders if needed
        //c->setGeometry(QRect(c->geometry().left(), geom.top(), geom.width(), geom.height()));
        break;

    case Plasma::RightEdge:
        if (m_alignment == Qt::AlignLeft) {
            geom.moveTopLeft(QPoint(screenGeom.right() - size.width() + 1, m_offset));
        } else if (m_alignment == Qt::AlignRight) {
            geom.moveBottomLeft(QPoint(screenGeom.right() - size.width() + 1, screenGeom.bottom() - m_offset));
        } else if (m_alignment == Qt::AlignCenter) {
            geom.moveCenter(QPoint(screenGeom.right() - size.width()/2, screenGeom.center().y() + m_offset));
        }

        //enable borders if needed
        //c->setGeometry(QRect(c->geometry().left(), geom.top(), geom.width(), geom.height()));
        break;

    case Plasma::BottomEdge:
    default:
        if (m_alignment == Qt::AlignLeft) {
            geom.moveTopLeft(QPoint(m_offset, screenGeom.bottom() - size.height() + 1));
        } else if (m_alignment == Qt::AlignRight) {
            geom.moveTopRight(QPoint(screenGeom.right() - m_offset, screenGeom.bottom() - size.height() + 1));
        } else if (m_alignment == Qt::AlignCenter) {
            geom.moveCenter(QPoint(screenGeom.center().x() + m_offset, screenGeom.bottom() - size.height()/2));
        }

        //enable borders if needed
        //c->setGeometry(QRect(geom.left(), c->geometry().top(), geom.width(), geom.height()));
        break;
    }

    kDebug() << (QObject*)this << "thinks its panel is at " << geom;
    if (geom == geometry()) {
        // our geometry is the same, but the panel moved around
        // so make sure our struts are still valid
        updateStruts();
    } else {
        setGeometry(geom);
    }

    m_lastMin = c->minimumSize();
    m_lastMax = c->maximumSize();

    //update the panel controller location position and size
    if (m_panelController) {
        m_panelController->setLocation(c->location());

        if (m_panelController->isVisible()) {
            m_panelController->resize(m_panelController->sizeHint());
            m_panelController->move(m_panelController->positionForPanelGeometry(geometry()));
        }

        foreach (PanelAppletOverlay *o, m_moveOverlays) {
            o->syncOrientation();
        }
    }
}

bool PanelView::isHorizontal() const
{
    return location() == Plasma::BottomEdge ||
           location() == Plasma::TopEdge;
}

void PanelView::pinchContainment(const QRect &screenGeom)
{
    //kDebug() << "**************************** pinching" << screenGeom << m_lastSeenSize;
    bool horizontal = isHorizontal();

    int sw = screenGeom.width();
    int sh = screenGeom.height();

    Plasma::Containment *c = containment();
    QSizeF min = c->minimumSize();
    QSizeF max = c->maximumSize();

    KConfigGroup sizes = config();
    sizes = KConfigGroup(&sizes, "Sizes");

    if (m_lastHorizontal != horizontal ||
        m_lastSeenSize != (horizontal ? sw : sh)) {
        // we're adjusting size. store the current size now
        KConfigGroup lastSize(&sizes, (m_lastHorizontal ? "Horizontal" : "Vertical") +
                                      QString::number(m_lastSeenSize));
        lastSize.writeEntry("size", size());
        lastSize.writeEntry("offset", m_offset);
        lastSize.writeEntry("min", m_lastMin);
        lastSize.writeEntry("max", m_lastMax);

        QString last = (horizontal ? "Horizontal" : "Vertical") +
                       QString::number(horizontal ? sw : sh);
        if (sizes.hasGroup(last)) {
            KConfigGroup thisSize(&sizes, last);

            /*
            kDebug() << "has saved properties..." << last
                     << thisSize.readEntry("min", min)
                     << thisSize.readEntry("max", max)
                     << thisSize.readEntry("size", c->geometry().size())
                     << thisSize.readEntry("offset", 0);
            */
            c->setMinimumSize(0, 0);
            c->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
            c->resize(thisSize.readEntry("size", c->geometry().size()));
            c->setMinimumSize(thisSize.readEntry("min", min));
            c->setMaximumSize(thisSize.readEntry("max", max));
            m_offset = thisSize.readEntry("offset", 0);
        } else if (m_lastSeenSize < (horizontal ? sw : sh) &&
                   c->geometry().width() == m_lastSeenSize) {
            // we are moving from a smaller space where we are 100% to a larger one
            if (horizontal) {
                c->setMaximumSize(sw, max.height());
                c->resize(sw, c->geometry().height());
            } else {
                c->setMaximumSize(max.width(), sh);
                c->resize(c->geometry().width(), sh);
            }
        }
    }

    // Pinching strategy:
    // if our containment is too big for the size of the screen we are now on,
    // then we first try and limit the offset and then if that still doesn't
    // give us enough room, we limit the size of the panel itself by setting
    // the minimum and maximum sizes.

    //kDebug() << "checking panel" << c->geometry() << "against" << screenGeom;
    if (horizontal) {
        //kDebug() << "becoming horizontal with" << m_offset << min.width() << max.width() << sw;
        if (m_offset + min.width() > sw) {
            //kDebug() << "min size is too wide!";
            if (min.width() > sw) {
                c->setMinimumSize(sw, min.height());
            } else {
                m_offset = sw - min.width();
            }
        }

        if (m_offset + max.width() > sw) {
            //kDebug() << "max size is too wide!";
            if (max.width() > sw) {
                c->setMaximumSize(sw, max.height());
            } else {
                m_offset = sw - max.width();
            }
        }
    } else {
        if (m_offset + min.height() > sh) {
            //kDebug() << "min size is too tall!";
            if (min.height() > sh) {
                c->setMinimumSize(min.width(), sh);
            } else {
                m_offset = sh - min.height();
            }
        }

        if (m_offset + max.height() > sh) {
            //kDebug() << "max size is too tall!";
            if (max.height() > sh) {
                c->setMaximumSize(max.width(), sh);
            } else {
                m_offset = sh - max.height();
            }
        }
    }

    if (m_lastHorizontal != horizontal ||
        m_lastSeenSize != (horizontal ? sw : sh)) {
        m_lastHorizontal = horizontal;
        m_lastSeenSize = (horizontal ? sw : sh);
        sizes.writeEntry("lastsize", m_lastSeenSize);
    }

    updatePanelGeometry();

    if (m_panelController) {
        m_panelController->setContainment(c);

        m_panelController->setOffset(m_offset);
    }
}

void PanelView::setOffset(int newOffset)
{
    m_offset = newOffset;
    KConfigGroup viewConfig = config();
    viewConfig.writeEntry("Offset", m_offset);

    containment()->update();

    //TODO: do we ever need to worry about pinching here, or
    //      do we just assume that the offset is always < screenSize - containmentSize?
    updatePanelGeometry();
}

int PanelView::offset() const
{
    return m_offset;
}

void PanelView::setAlignment(Qt::Alignment align)
{
    m_alignment = alignmentFilter(align);
    KConfigGroup viewConfig = config();
    viewConfig.writeEntry("Alignment", (int)m_alignment);
}

Qt::Alignment PanelView::alignment() const
{
    return m_alignment;
}

void PanelView::showAppletBrowser()
{
    PlasmaApp::self()->showAppletBrowser(containment());
}

void PanelView::togglePanelController()
{
    //kDebug();
    m_editting = false;
    if (containment()->immutability() != Plasma::Mutable) {
        delete m_panelController;
        m_panelController = 0;
        return;
    }

    if (!m_panelController) {
        m_panelController = new PanelController(this);
        m_panelController->setContainment(containment());
        m_panelController->setLocation(containment()->location());
        m_panelController->setAlignment(m_alignment);
        m_panelController->setOffset(m_offset);

        connect(m_panelController, SIGNAL(destroyed(QObject*)), this, SLOT(edittingComplete()));
        connect(m_panelController, SIGNAL(offsetChanged(int)), this, SLOT(setOffset(int)));
        connect(m_panelController, SIGNAL(alignmentChanged(Qt::Alignment)), this, SLOT(setAlignment(Qt::Alignment)));
        connect(m_panelController, SIGNAL(locationChanged(Plasma::Location)), this, SLOT(setLocation(Plasma::Location)));

        if (dynamic_cast<QGraphicsLinearLayout*>(containment()->layout())) {
            // we only support mouse over drags for panels with linear layouts
            QColor overlayColor(Plasma::Theme::defaultTheme()->color(Plasma::Theme::BackgroundColor));
            QBrush overlayBrush(overlayColor);
            QPalette p(palette());
            p.setBrush(QPalette::Window, overlayBrush);
            foreach (Plasma::Applet *applet, containment()->applets()) {
                PanelAppletOverlay *moveOverlay = new PanelAppletOverlay(applet, this);
                moveOverlay->setPalette(p);
                moveOverlay->show();
                moveOverlay->raise();
                m_moveOverlays << moveOverlay;
                //kDebug() << moveOverlay << moveOverlay->geometry();
            }

            setTabOrder(0, m_panelController);
            QWidget *prior = m_panelController;
            foreach (PanelAppletOverlay *w, m_moveOverlays) {
                setTabOrder(prior, w);
                prior = w;
            }
        }
    }

    if (!m_panelController->isVisible()) {
        m_editting = true;
        m_panelController->resize(m_panelController->sizeHint());
        m_panelController->move(m_panelController->positionForPanelGeometry(geometry()));
        m_panelController->show();
    } else {
        m_panelController->close();
    }
}

void PanelView::edittingComplete()
{
    //kDebug();
    m_panelController = 0;
    m_editting = false;
    qDeleteAll(m_moveOverlays);
    m_moveOverlays.clear();
    containment()->closeToolBox();
    m_firstPaint = true; // triggers autohide
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

    if (!m_windowsCover && !m_autohide) {
        QRect thisScreen = QApplication::desktop()->screenGeometry(containment()->screen());
        QRect wholeScreen = QApplication::desktop()->geometry();

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
                //kDebug() << "setting bottom edge to" << strut.bottom_width
                //         << strut.bottom_start << strut.bottom_end;
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
                //kDebug() << "where are we?";
            break;
        }
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
    kDebug();
    QWidget::moveEvent(event);
    updateStruts();
}

void PanelView::resizeEvent(QResizeEvent *event)
{
    kDebug();
    QWidget::resizeEvent(event);
    updateStruts();
}

QTimeLine *PanelView::timeLine()
{
    if (!m_timeLine) {
        m_timeLine = new QTimeLine(200, this);
        m_timeLine->setCurveShape(QTimeLine::EaseOutCurve);
        m_timeLine->setUpdateInterval(10);
        connect(m_timeLine, SIGNAL(valueChanged(qreal)), this, SLOT(animateHide(qreal)));
    }

    return m_timeLine;
}

void PanelView::unhide()
{
#ifdef Q_WS_X11
    if (m_unhideTrigger != None) {
        XDestroyWindow(QX11Info::display(), m_unhideTrigger);
        m_unhideTrigger = None;
#else
    {
#endif
        PlasmaApp::self()->panelHidden(false);

        QTimeLine * tl = timeLine();
        tl->setDirection(QTimeLine::Backward);
        // with composite, we can quite do some nice animations with transparent
        // backgrounds; without it we can't so we just show/hide
        if (PlasmaApp::hasComposite()) {
            if (tl->state() == QTimeLine::NotRunning) {
                tl->start();
            }
        }

        show();
        KWindowSystem::setOnAllDesktops(winId(), true);
        unsigned long state = NET::Sticky;
        if (!m_windowsCover) {
            state |= NET::StaysOnTop;
        }
        KWindowSystem::setState(winId(), state);
    }
}

void PanelView::leaveEvent(QEvent *event)
{
    if (m_autohide && !m_editting) {
        QTimeLine * tl = timeLine();
        tl->setDirection(QTimeLine::Forward);

        // with composite, we can quite do some nice animations with transparent
        // backgrounds; without it we can't so we just show/hide
        if (PlasmaApp::hasComposite()) {
            if (tl->state() == QTimeLine::NotRunning) {
                tl->start();
            }
        } else {
            animateHide(1.0);
        }
    }

    Plasma::View::leaveEvent(event);
}

void PanelView::drawBackground(QPainter *painter, const QRectF &rect)
{
    if (PlasmaApp::hasComposite()) {
        painter->setCompositionMode(QPainter::CompositionMode_Source);
        painter->fillRect(rect, Qt::transparent);
    } else {
        Plasma::View::drawBackground(painter, rect);
    }
}

void PanelView::paintEvent(QPaintEvent *event)
{
    Plasma::View::paintEvent(event);
    if (m_firstPaint) {
        if (m_autohide) {
            QTimeLine * tl = timeLine();
            tl->setDirection(QTimeLine::Forward);
            tl->start();
        }

        m_firstPaint = false;
    }
}

void PanelView::animateHide(qreal progress)
{
    int margin = 0;
    Plasma::Location loc = location();

    if (loc == Plasma::TopEdge || loc == Plasma::BottomEdge) {
        margin = progress * height();
    } else {
        margin = progress * width();
    }

    int xtrans = 0;
    int ytrans = 0;
    int triggerWidth = 1;
    int triggerHeight = 1;
    QPoint triggerPoint = pos();

    switch (loc) {
        case Plasma::TopEdge:
            ytrans = -margin;
            triggerWidth = width();
            break;
        case Plasma::BottomEdge:
            ytrans = margin;
            triggerWidth = width();
            triggerPoint = geometry().bottomLeft();
            break;
        case Plasma::RightEdge:
            xtrans = margin;
            triggerHeight = height();
            triggerPoint = geometry().topRight();
            break;
        case Plasma::LeftEdge:
            xtrans = -margin;
            triggerHeight = height();
            break;
        default:
            // no hiding unless we're on an edge.
            return;
            break;
    }

    //kDebug() << progress << xtrans << ytrans;
    if (PlasmaApp::hasComposite()) {
        viewport()->move(xtrans, ytrans);
    }

    QTimeLine *tl = timeLine();
    if (qFuzzyCompare(qreal(1.0), progress) && tl->direction() == QTimeLine::Forward) {
        //kDebug() << "**************** hide complete" << triggerPoint << triggerWidth << triggerHeight;

#ifdef Q_WS_X11
        if (m_unhideTrigger != None) {
            XDestroyWindow(QX11Info::display(), m_unhideTrigger);
        } else {
            PlasmaApp::self()->panelHidden(true);
        }

        XSetWindowAttributes attributes;
        attributes.override_redirect = True;
        attributes.event_mask = EnterWindowMask;
        unsigned long valuemask = CWOverrideRedirect | CWEventMask;
        m_unhideTrigger = XCreateWindow(QX11Info::display(), QX11Info::appRootWindow(),
                                        triggerPoint.x(), triggerPoint.y(), triggerWidth, triggerHeight,
                                        0, CopyFromParent, InputOnly, CopyFromParent,
                                        valuemask, &attributes);
        XMapWindow(QX11Info::display(), m_unhideTrigger);
#else
        PlasmaApp::self()->panelHidden(true);
#endif

        hide();
    } else if (qFuzzyCompare(qreal(0.0), progress) && tl->direction() == QTimeLine::Backward) {
        kDebug() << "show complete";
    }
}

#include "panelview.moc"


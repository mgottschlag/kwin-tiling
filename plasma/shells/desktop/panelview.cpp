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
#ifdef Q_WS_X11
#include <QX11Info>
#endif

#include <KWindowSystem>
#include <KDebug>

#include <Plasma/Containment>
#include <Plasma/Corona>
#include <Plasma/Plasma>
#include <Plasma/Svg>
#include <Plasma/Theme>

#include "panelappletoverlay.h"
#include "panelcontroller.h"
#include "plasmaapp.h"

#include <kephal/screens.h>

class GlowBar : public QWidget
{
public:
    GlowBar(Plasma::Direction direction, const QRect &triggerZone)
        : QWidget(0),
          m_strength(0.3),
          m_svg(new Plasma::Svg(this)),
          m_direction(direction)
    {
        KWindowSystem::setOnAllDesktops(winId(), true);
        unsigned long state = NET::Sticky | NET::StaysOnTop | NET::KeepAbove;
        KWindowSystem::setState(winId(), state);
        KWindowSystem::setType(winId(), NET::Dock);
        m_svg->setImagePath("widgets/glowbar");

        QRect glowGeom = triggerZone;
        QSize s = sizeHint();
        switch (m_direction) {
            case Plasma::Up:
                glowGeom.setY(glowGeom.y() - s.height() + 1);
                // fallthrough
            case Plasma::Down:
                glowGeom.setHeight(s.height());
                break;
            case Plasma::Left:
                glowGeom.setX(glowGeom.x() - s.width() + 1);
                // fallthrough
            case Plasma::Right:
                glowGeom.setWidth(s.width());
                break;
        }

        //kDebug() << "glow geom is" << glowGeom << "from" << triggerZone;
        setGeometry(glowGeom);
        m_buffer = QPixmap(size());
    }

    void paintEvent(QPaintEvent* e)
    {
        Q_UNUSED(e)
        QPixmap l, r, c;
        const QSize glowRadius = m_svg->elementSize("hint-glow-radius");
        QPoint pixmapPosition(0, 0);

        m_buffer.fill(QColor(0, 0, 0, 255*m_strength));
        QPainter p(&m_buffer);
        p.setCompositionMode(QPainter::CompositionMode_SourceIn);

        switch (m_direction) {
            case Plasma::Down:
                l = m_svg->pixmap("bottomleft");
                r = m_svg->pixmap("bottomright");
                c = m_svg->pixmap("bottom");
                pixmapPosition = QPoint(0, -glowRadius.height());
                break;
            case Plasma::Up:
                l = m_svg->pixmap("topleft");
                r = m_svg->pixmap("topright");
                c = m_svg->pixmap("top");
                break;
            case Plasma::Right:
                l = m_svg->pixmap("topright");
                r = m_svg->pixmap("bottomright");
                c = m_svg->pixmap("right");
                pixmapPosition = QPoint(-glowRadius.width(), 0);
                break;
            case Plasma::Left:
                l = m_svg->pixmap("topleft");
                r = m_svg->pixmap("bottomleft");
                c = m_svg->pixmap("left");
                break;
        }

        if (m_direction == Plasma::Left || m_direction == Plasma::Right) {
            p.drawPixmap(QPoint(0, 0), l);
            p.drawTiledPixmap(QRect(0, l.height(), c.width(), height() - l.height() - r.height()), c);
            p.drawPixmap(QPoint(0, height() - r.height()), r);
        } else {
            p.drawPixmap(pixmapPosition, l);
            p.drawTiledPixmap(QRect(l.width(), pixmapPosition.y(), width() - l.width() - r.width(), c.height()), c);
            p.drawPixmap(QPoint(width() - r.width(), pixmapPosition.y()), r);
        }

        p.end();
        p.begin(this);
        p.drawPixmap(QPoint(0, 0), m_buffer);
    }

    QSize sizeHint() const
    {
        return m_svg->elementSize("bottomright") - m_svg->elementSize("hint-glow-radius");
    }

    bool event(QEvent *event)
    {
        if (event->type() == QEvent::Paint) {
            QPainter p(this);
            p.setCompositionMode(QPainter::CompositionMode_Source);
            p.fillRect(rect(), Qt::transparent);
        }
        return QWidget::event(event);
    }

    void updateStrength(QPoint point)
    {
        QPoint localPoint = mapFromGlobal(point);

        qreal newStrength;
        switch (m_direction) {
        case Plasma::Up: // when the panel is at the bottom.
            newStrength = 1 - qreal(-localPoint.y())/m_triggerDistance;
            break;
        case Plasma::Right:
            newStrength = 1 - qreal(localPoint.x())/m_triggerDistance;
            break;
        case Plasma::Left: // when the panel is right-aligned
            newStrength = 1 - qreal(-localPoint.x())/m_triggerDistance;
            break;
        case Plasma::Down:
        default:
            newStrength = 1- qreal(localPoint.y())/m_triggerDistance;
            break;
        }
        if (qAbs(newStrength - m_strength) > 0.01 && newStrength >= 0 && newStrength <= 1) {
            m_strength = newStrength;
            update();
        }
    }


private:
    static const int m_triggerDistance = 30;
    qreal m_strength;
    Plasma::Svg *m_svg;
    Plasma::Direction m_direction;
    QPixmap m_buffer;
};

PanelView::PanelView(Plasma::Containment *panel, int id, QWidget *parent)
    : Plasma::View(panel, id, parent),
      m_panelController(0),
      m_glowBar(0),
      m_mousePollTimer(0),
      m_timeLine(0),
      m_spacer(0),
      m_spacerIndex(-1),
#ifdef Q_WS_X11
      m_unhideTrigger(None),
#endif
      m_panelMode(NormalPanel),
      m_lastHorizontal(true),
      m_editting(false),
      m_firstPaint(true),
      m_triggerEntered(false)
{
    Q_ASSERT(qobject_cast<Plasma::Corona*>(panel->scene()));
    KConfigGroup viewConfig = config();

    m_offset = viewConfig.readEntry("Offset", 0);
    m_alignment = alignmentFilter((Qt::Alignment)viewConfig.readEntry("Alignment", (int)Qt::AlignLeft));
    setPanelMode((PanelMode)viewConfig.readEntry("panelMode", (int)m_panelMode));

    // pinchContainment calls updatePanelGeometry for us
    QRect screenRect = Kephal::ScreenUtils::screenGeometry(containment()->screen());
    m_lastHorizontal = isHorizontal();
    KConfigGroup sizes = KConfigGroup(&viewConfig, "Sizes");
    m_lastSeenSize = sizes.readEntry("lastsize", m_lastHorizontal ? screenRect.width() : screenRect.height());
    pinchContainment(screenRect);
    m_lastMin = containment()->minimumSize();
    m_lastMax = containment()->maximumSize();


    if (panel) {
        connect(panel, SIGNAL(destroyed(QObject*)), this, SLOT(panelDeleted()));
        connect(panel, SIGNAL(toolBoxToggled()), this, SLOT(togglePanelController()));
        kDebug() << "Panel geometry is" << panel->geometry();
    }

    connect(this, SIGNAL(sceneRectAboutToChange()), this, SLOT(updatePanelGeometry()));


    // Graphics view setup
    setFrameStyle(QFrame::NoFrame);
    //setAutoFillBackground(true);
    //setDragMode(QGraphicsView::RubberBandDrag);
    //setCacheMode(QGraphicsView::CacheBackground);
    setInteractive(true);
    setAcceptDrops(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QPalette pal = palette();
    pal.setBrush(backgroundRole(), Qt::transparent);
    setPalette(pal);

    // KWin setup
    KWindowSystem::setOnAllDesktops(winId(), true);

#ifdef Q_WS_WIN
    registerAccessBar(true);
#endif

    updateStruts();

    Kephal::Screens *screens = Kephal::Screens::self();
    connect(screens, SIGNAL(screenResized(Kephal::Screen *, QSize, QSize)),
            this, SLOT(pinchContainmentToCurrentScreen()));
    connect(screens, SIGNAL(screenMoved(Kephal::Screen *, QPoint, QPoint)),
            this, SLOT(updatePanelGeometry()));
}

PanelView::~PanelView()
{
    delete m_glowBar;
    destroyUnhideTrigger();
#ifdef Q_WS_WIN
    registerAccessBar(false);
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
                QRect screenGeom = Kephal::ScreenUtils::screenGeometry(c->screen());
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
                QRect screenGeom = Kephal::ScreenUtils::screenGeometry(c->screen());
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
#ifdef Q_WS_WIN
    appBarPosChanged();
#endif
    QRect screenRect = Kephal::ScreenUtils::screenGeometry(c->screen());
    pinchContainment(screenRect);
    KWindowSystem::setOnAllDesktops(winId(), true);
    //updatePanelGeometry();
    connect(this, SIGNAL(sceneRectAboutToChange()), this, SLOT(updatePanelGeometry()));
}

Plasma::Location PanelView::location() const
{
    return containment()->location();
}

void PanelView::setPanelMode(PanelView::PanelMode mode)
{
    unsigned long state = NET::Sticky;

    KWindowSystem::setType(winId(), NET::Dock);
    if (mode == LetWindowsCover) {
        createUnhideTrigger();
        KWindowSystem::clearState(winId(), NET::StaysOnTop | NET::KeepAbove);
        state |= NET::KeepBelow;
    } else {
        //kDebug() << "panel shouldn't let windows cover it!";
        state |= NET::StaysOnTop;
    }

    if (mode == NormalPanel) {
        // we need to kill the input window if it exists!
        destroyUnhideTrigger();
    }

    //kDebug() << "panel state set to" << state << NET::Sticky;
    KWindowSystem::setState(winId(), state);
    KWindowSystem::setOnAllDesktops(winId(), true);

    m_panelMode = mode;
    config().writeEntry("panelMode", (int)mode);
}

PanelView::PanelMode PanelView::panelMode() const
{
    return m_panelMode;
}

Plasma::Corona *PanelView::corona() const
{
    return qobject_cast<Plasma::Corona*>(scene());
}

void PanelView::updatePanelGeometry()
{
    Plasma::Containment *c = containment();
    kDebug() << "New panel geometry is" << c->geometry();

    QSize size = c->size().toSize();
    QRect geom(QPoint(0,0), size);
    int screen = c->screen();

    if (screen < 0) {
        //TODO: is there a valid use for -1 with a panel? floating maybe?
        screen = 0;
    }

    QRect screenGeom = Kephal::ScreenUtils::screenGeometry(screen);

    if (m_alignment != Qt::AlignCenter) {
        m_offset = qMax(m_offset, 0);
    }

    //Sanity controls
    switch (location()) {
    case Plasma::TopEdge:
    case Plasma::BottomEdge:
        if (m_alignment != Qt::AlignCenter) {
            m_offset = qMax(m_offset, 0);
        }
        //resize the panel if is too large
        if (geom.width() > screenGeom.width()) {
            geom.setWidth(screenGeom.width());
        }

        //move the panel left/right if there is not enough room
        if (m_alignment == Qt::AlignLeft) {
             if (m_offset + geom.width() > screenGeom.width() + 1) {
                 m_offset = screenGeom.width() - geom.width();
             }
        } else if (m_alignment == Qt::AlignRight) {
             if (screenGeom.width() - m_offset - geom.width() < -1 ) {
                 m_offset = screenGeom.width() - geom.width();
             }
        } else if (m_alignment == Qt::AlignCenter) {
             if (screenGeom.center().x() - screenGeom.x() + m_offset + geom.width()/2 > screenGeom.width() + 1) {
                 m_offset = screenGeom.width() - geom.width()/2 - (screenGeom.center().x() - screenGeom.x());
             } else if (screenGeom.center().x() - screenGeom.x() + m_offset - geom.width()/2 < -1) {
                 m_offset = (screenGeom.center().x() - screenGeom.x()) - geom.width()/2;
             }
        }
        break;

    case Plasma::LeftEdge:
    case Plasma::RightEdge:
        if (m_alignment != Qt::AlignCenter) {
            m_offset = qMax(m_offset, 0);
        }
        //resize the panel if is too tall
        if (geom.height() > screenGeom.height()) {
            geom.setHeight(screenGeom.height());
        }

        //move the panel bottom if there is not enough room
        //FIXME: still using alignleft/alignright is simpler and less error prone, but aligntop/alignbottom is more correct?
        if (m_alignment == Qt::AlignLeft) {
            if (m_offset + geom.height() > screenGeom.height() + 1) {
                m_offset = screenGeom.height() - geom.height();
            }
        } else if (m_alignment == Qt::AlignRight) {
            if (screenGeom.height() - m_offset - geom.height() < -1) {
                m_offset = screenGeom.height() - geom.height();
            }
        } else if (m_alignment == Qt::AlignCenter) {
            if (screenGeom.center().y() - screenGeom.top() + m_offset + geom.height()/2 > screenGeom.height() + 1) {
                m_offset = screenGeom.height() - geom.height()/2 - (screenGeom.center().y() - screenGeom.top());
             } else if (screenGeom.center().y() - screenGeom.top() + m_offset - geom.height()/2 < -1) {
                m_offset = (screenGeom.center().y() - screenGeom.top()) - geom.width()/2;
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
            geom.moveTopLeft(QPoint(m_offset + screenGeom.left(), screenGeom.top()));
        } else if (m_alignment == Qt::AlignRight) {
            geom.moveTopRight(QPoint(screenGeom.right() - m_offset, screenGeom.top()));
        } else if (m_alignment == Qt::AlignCenter) {
            geom.moveTopLeft(QPoint(screenGeom.center().x() - geom.width()/2 + 1 - geom.width()%2 + m_offset, screenGeom.top()));
        }

        //enable borders if needed
        //c->setGeometry(QRect(geom.left(), c->geometry().top(), geom.width(), geom.height()));
        break;

    case Plasma::LeftEdge:
        if (m_alignment == Qt::AlignLeft) {
            geom.moveTopLeft(QPoint(screenGeom.left(), m_offset + screenGeom.top()));
        } else if (m_alignment == Qt::AlignRight) {
            geom.moveBottomLeft(QPoint(screenGeom.left(), screenGeom.bottom() - m_offset));
        } else if (m_alignment == Qt::AlignCenter) {
            geom.moveTopLeft(QPoint(screenGeom.left(), screenGeom.center().y() - geom.height()/2 + 1 - geom.height()%2 + m_offset));
        }

        //enable borders if needed
        //c->setGeometry(QRect(c->geometry().left(), geom.top(), geom.width(), geom.height()));
        break;

    case Plasma::RightEdge:
        if (m_alignment == Qt::AlignLeft) {
            geom.moveTopRight(QPoint(screenGeom.right(), m_offset + screenGeom.top()));
        } else if (m_alignment == Qt::AlignRight) {
            geom.moveBottomRight(QPoint(screenGeom.right(), screenGeom.bottom() - m_offset));
        } else if (m_alignment == Qt::AlignCenter) {
            geom.moveTopRight(QPoint(screenGeom.right(), screenGeom.center().y() - geom.height()/2 + 1 - geom.height()%2 + m_offset));
        }

        //enable borders if needed
        //c->setGeometry(QRect(c->geometry().left(), geom.top(), geom.width(), geom.height()));
        break;

    case Plasma::BottomEdge:
    default:
        if (m_alignment == Qt::AlignLeft) {
            geom.moveBottomLeft(QPoint(m_offset + screenGeom.left(), screenGeom.bottom()));
        } else if (m_alignment == Qt::AlignRight) {
            geom.moveBottomRight(QPoint(screenGeom.right() - m_offset, screenGeom.bottom()));
        } else if (m_alignment == Qt::AlignCenter) {
            geom.moveBottomLeft(QPoint(screenGeom.center().x() - geom.width()/2 + 1 - geom.width()%2 + m_offset, screenGeom.bottom()));
        }

        //enable borders if needed
        //c->setGeometry(QRect(geom.left(), c->geometry().top(), geom.width(), geom.height()));
        break;
    }

    kDebug() << (QObject*)this << "thinks its panel is at " << geom << "was" << geometry();
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

void PanelView::pinchContainmentToCurrentScreen()
{
    QRect screenRect = Kephal::ScreenUtils::screenGeometry(containment()->screen());
    pinchContainment(screenRect);
}

void PanelView::pinchContainment(const QRect &screenGeom)
{
    kDebug() << "**************************** pinching" << screenGeom << m_lastSeenSize;
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

    kDebug() << "Done pinching, containement's geom" << c->geometry() << "own geom" << geometry();
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
        m_panelController->setPanelMode(m_panelMode);

        connect(m_panelController, SIGNAL(destroyed(QObject*)), this, SLOT(edittingComplete()));
        connect(m_panelController, SIGNAL(offsetChanged(int)), this, SLOT(setOffset(int)));
        connect(m_panelController, SIGNAL(alignmentChanged(Qt::Alignment)), this, SLOT(setAlignment(Qt::Alignment)));
        connect(m_panelController, SIGNAL(locationChanged(Plasma::Location)), this, SLOT(setLocation(Plasma::Location)));
        connect(m_panelController, SIGNAL(panelModeChanged(PanelView::PanelMode)), this, SLOT(setPanelMode(PanelView::PanelMode)));

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
        updateStruts();
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
    updateStruts();
    m_firstPaint = true; // triggers autohide

    // not overly efficient since we may not have changed any settings,
    // but ensures that if we have, a config sync will occur
    PlasmaApp::self()->corona()->requestConfigSync();
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

    if (m_panelMode == NormalPanel) {
        QRect thisScreen = Kephal::ScreenUtils::screenGeometry(containment()->screen());
        QRect wholeScreen = Kephal::ScreenUtils::desktopGeometry();

        // extended struts are to the combined screen geoms, not the single screen
        int leftOffset = wholeScreen.x() - thisScreen.x();
        int rightOffset = wholeScreen.right() - thisScreen.right();
        int bottomOffset = wholeScreen.bottom() - thisScreen.bottom();
        int topOffset = wholeScreen.top() - thisScreen.top();
        kDebug() << "screen l/r/b/t offsets are:" << leftOffset << rightOffset << bottomOffset << topOffset << location();

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
    //kDebug();
    QWidget::moveEvent(event);
    updateStruts();
}

void PanelView::resizeEvent(QResizeEvent *event)
{
    //kDebug() << event->oldSize() << event->size();
    QWidget::resizeEvent(event);
    updateStruts();
#ifdef Q_WS_WIN
    appBarPosChanged();
#endif
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

void PanelView::unhideHintMousePoll()
{
#ifdef Q_WS_X11
    QPoint mousePos = QCursor::pos();
    m_glowBar->updateStrength(mousePos);

    if (!m_unhideTriggerGeom.contains(mousePos)) {
        unhintHide();
        XMoveResizeWindow(QX11Info::display(), m_unhideTrigger, m_unhideTriggerGeom.x(), m_unhideTriggerGeom.y(), m_unhideTriggerGeom.width(), m_unhideTriggerGeom.height());
    }
#endif
}

QRect PanelView::unhideHintGeometry() const
{
#ifdef Q_WS_X11
    return m_unhideTriggerGeom;
#else
    return QRect();
#endif
}

void PanelView::hintOrUnhide(const QPoint &point)
{
#ifdef Q_WS_X11
    if (!shouldHintHide()) {
        unhide();
        return;
    }

    //kDebug() << point << m_triggerZone;
    if (point == QPoint()) {
        //kDebug() << "enter, we should start glowing!";
        if (!m_glowBar) {
            Plasma::Direction direction = Plasma::locationToDirection(location());
            m_glowBar = new GlowBar(direction, m_triggerZone);
            m_glowBar->show();
            XMoveResizeWindow(QX11Info::display(), m_unhideTrigger, m_triggerZone.x(), m_triggerZone.y(), m_triggerZone.width(), m_triggerZone.height());
            //FIXME: This is ugly as hell but well, yeah
            m_mousePollTimer = new QTimer(this);
            connect(m_mousePollTimer, SIGNAL(timeout()), this, SLOT(unhideHintMousePoll()));
            m_mousePollTimer->start(200);
        }
    } else if (m_triggerZone.contains(point)) {
        //kDebug() << "unhide!" << point;
        unhide();
    } else {
        //this if we could avoid the polling
        //m_glowBar->updateStrength(point);
        //kDebug() << "keep glowing";
    }
#endif
}

void PanelView::unhintHide()
{
    //kDebug() << "hide the glow";
    if (m_mousePollTimer) {
        m_mousePollTimer->stop();
        m_mousePollTimer->deleteLater();
        m_mousePollTimer = 0;
    }
    delete m_glowBar;
    m_glowBar = 0;
}

void PanelView::unhide()
{
    //kDebug();
    unhintHide();
    destroyUnhideTrigger();

    // with composite, we can quite do some nice animations with transparent
    // backgrounds; without it we can't so we just show/hide
    QTimeLine * tl = timeLine();
    tl->setDirection(QTimeLine::Backward);

    if (m_panelMode == AutoHide) {
        // LetWindowsCover panels are alwys shown, so don't bother and prevent
        // some unsightly flickers
        show();
    }

    KWindowSystem::setOnAllDesktops(winId(), true);
    unsigned long state = NET::Sticky;
    KWindowSystem::setState(winId(), state);

    if (m_panelMode == LetWindowsCover) {
        m_triggerEntered = true;
        KWindowSystem::raiseWindow(winId());
    } else if (shouldHintHide()) {
        if (tl->state() == QTimeLine::NotRunning) {
            tl->start();
        }
    } else {
        //if the hide before  compositing was active now the view is wrong
        viewport()->move(0,0);
    }
}

void PanelView::leaveEvent(QEvent *event)
{
    if (m_panelMode == AutoHide && !m_editting) {
        // try not to hide if we have an associated popup or window about
        bool havePopup = QApplication::activePopupWidget() != 0;

        if (!havePopup) {
            QWidget *popup = QApplication::activeWindow();

            if (popup &&
                (popup->window()->parentWidget() == this ||
                 popup->parentWidget() == this ||
                 (popup->parentWidget() && popup->parentWidget()->window() == this))) {
                    kDebug() << "got a popup!" << popup
                             << popup->window() << popup->window()->parentWidget() << popup->parentWidget() << this;

                    havePopup = true;
            } /* else {
                kDebug() << "no popup?!";
            } */
        } else {
            kDebug() << "gota a popup widget";
        }

        if (!havePopup) {
            // with composite, we can quite do some nice animations with transparent
            // backgrounds; without it we can't so we just show/hide
            QTimeLine * tl = timeLine();
            tl->setDirection(QTimeLine::Forward);

            if (shouldHintHide()) {
                if (tl->state() == QTimeLine::NotRunning) {
                    tl->start();
                }
            } else {
                animateHide(1.0);
            }
        }
    }

    Plasma::View::leaveEvent(event);
}

void PanelView::drawBackground(QPainter *painter, const QRectF &rect)
{
    if (PlasmaApp::hasComposite()) {
        painter->setCompositionMode(QPainter::CompositionMode_Source);
        painter->fillRect(rect.toAlignedRect(), Qt::transparent);
    } else {
        Plasma::View::drawBackground(painter, rect);
    }
}

void PanelView::paintEvent(QPaintEvent *event)
{
    Plasma::View::paintEvent(event);
    if (m_firstPaint) {
        // set up our auothide system after we paint it visibly to the user
        if (m_panelMode == AutoHide) {
            QTimeLine * tl = timeLine();
            tl->setDirection(QTimeLine::Forward);
            if (tl->state() == QTimeLine::NotRunning) {
                tl->start();
            }
        }

        m_firstPaint = false;
    }
}

bool PanelView::event(QEvent *event)
{
    if (event->type() == QEvent::Paint) {
        QPainter p(this);
        p.setCompositionMode(QPainter::CompositionMode_Source);
        p.fillRect(rect(), Qt::transparent);
    } else if (m_panelMode == LetWindowsCover && event->type() == QEvent::Leave) {
        if (m_triggerEntered) {
            m_triggerEntered = false;
        } else {
            createUnhideTrigger();
        }
    }

    return Plasma::View::event(event);
}

void PanelView::animateHide(qreal progress)
{
    if (m_panelMode == AutoHide && shouldHintHide()) {
        int margin = 0;
        Plasma::Location loc = location();

        if (loc == Plasma::TopEdge || loc == Plasma::BottomEdge) {
            margin = progress * height();
        } else {
            margin = progress * width();
        }

        int xtrans = 0;
        int ytrans = 0;

        switch (loc) {
            case Plasma::TopEdge:
                ytrans = -margin;
                break;
            case Plasma::BottomEdge:
                ytrans = margin;
                break;
            case Plasma::RightEdge:
                xtrans = margin;
                break;
            case Plasma::LeftEdge:
                xtrans = -margin;
                break;
            default:
                // no hiding unless we're on an edge.
                return;
                break;
        }

    //kDebug() << progress << xtrans << ytransLetWindowsCover;
        viewport()->move(xtrans, ytrans);
    }

    QTimeLine *tl = timeLine();
    if (qFuzzyCompare(qreal(1.0), progress) && tl->direction() == QTimeLine::Forward) {
        //kDebug() << "**************** hide complete" << triggerPoint << triggerWidth << triggerHeight;
        createUnhideTrigger();
        hide();
    } else if (qFuzzyCompare(qreal(1.0), progress + 1.0) && tl->direction() == QTimeLine::Backward) {
        //if the show before accel was off now viewport position is wrong, so ensure it's visible
        //kDebug() << "show complete";
        viewport()->move(0,0);
    }
}

bool PanelView::shouldHintHide() const
{
    return m_panelMode == AutoHide && PlasmaApp::hasComposite();
}

void PanelView::recreateUnhideTrigger()
{
#ifdef Q_WS_X11
    if (m_unhideTrigger == None) {
        return;
    }

    XDestroyWindow(QX11Info::display(), m_unhideTrigger);
    m_unhideTrigger = None;
    createUnhideTrigger();
#endif
}

void PanelView::createUnhideTrigger()
{
#ifdef Q_WS_X11
    if (m_unhideTrigger != None) {
        return;
    }

    bool fancy = shouldHintHide();
    int actualWidth = 1;
    int actualHeight = 1;
    int triggerWidth = fancy ? 30 : 1;
    int triggerHeight = fancy ? 30 : 1;

    QPoint actualTriggerPoint = pos();
    QPoint triggerPoint = pos();

    switch (location()) {
        case Plasma::TopEdge:
            actualWidth = triggerWidth = width();

            if (fancy) {
                triggerWidth += 30;
                triggerPoint.setX(qMax(0, triggerPoint.x() - 15));
            }
            break;
        case Plasma::BottomEdge:
            actualWidth = triggerWidth = width();
            actualTriggerPoint = triggerPoint = geometry().bottomLeft();

            if (fancy) {
                triggerWidth += 30;
                triggerPoint.setX(qMax(0, triggerPoint.x() - 15));
                triggerPoint.setY(qMax(0, triggerPoint.y() - 29));
            }
            break;
        case Plasma::RightEdge:
            actualHeight = triggerHeight = height();
            actualTriggerPoint = triggerPoint = geometry().topRight();

            if (fancy) {
                triggerHeight += 30;
                triggerPoint.setY(qMax(0, triggerPoint.y() - 15));
                triggerPoint.setX(qMax(0, triggerPoint.x() - 29));
            }
            break;
        case Plasma::LeftEdge:
            actualHeight = triggerHeight = height();

            if (fancy) {
                triggerHeight += 30;
                triggerPoint.setY(qMax(0, triggerPoint.y() - 15));
            }
            break;
        default:
            // no hiding unless we're on an edge.
            return;
            break;
    }


    XSetWindowAttributes attributes;
    attributes.override_redirect = True;
    attributes.event_mask = EnterWindowMask;


    attributes.event_mask = EnterWindowMask | LeaveWindowMask | PointerMotionMask |
                            KeyPressMask | KeyPressMask | ButtonPressMask |
                            ButtonReleaseMask | ButtonMotionMask |
                            KeymapStateMask | VisibilityChangeMask |
                            StructureNotifyMask | ResizeRedirectMask |
                            SubstructureNotifyMask |
                            SubstructureRedirectMask | FocusChangeMask |
                            PropertyChangeMask | ColormapChangeMask | OwnerGrabButtonMask;

    unsigned long valuemask = CWOverrideRedirect | CWEventMask;
    m_unhideTrigger = XCreateWindow(QX11Info::display(), QX11Info::appRootWindow(),
                                    triggerPoint.x(), triggerPoint.y(), triggerWidth, triggerHeight,
                                    0, CopyFromParent, InputOnly, CopyFromParent,
                                    valuemask, &attributes);
    XMapWindow(QX11Info::display(), m_unhideTrigger);
    m_unhideTriggerGeom = QRect(triggerPoint, QSize(triggerWidth, triggerHeight));
    m_triggerZone = QRect(actualTriggerPoint, QSize(actualWidth, actualHeight));
//    KWindowSystem::setState(m_unhideTrigger, NET::StaysOnTop);

#endif
    //kDebug() << m_unhideTrigger;
    PlasmaApp::self()->panelHidden(true);
}

void PanelView::destroyUnhideTrigger()
{
#ifdef Q_WS_X11
    if (m_unhideTrigger == None) {
        return;
    }

    XDestroyWindow(QX11Info::display(), m_unhideTrigger);
    m_unhideTrigger = None;
    m_triggerZone = m_unhideTriggerGeom = QRect();
#endif

    //kDebug();
    PlasmaApp::self()->panelHidden(false);
}

void PanelView::panelDeleted()
{
    if (!QApplication::closingDown()) {
        // the panel was removed at runtime; clean up our configuration object as well
        KConfigGroup c = config();
        c.deleteGroup();
    }

    deleteLater();
}

void PanelView::positionSpacer(const QPoint pos)
{
    if (!containment()) {
        return;
    }

    QGraphicsLinearLayout *lay = dynamic_cast<QGraphicsLinearLayout*>(containment()->layout());

    if (!lay) {
        return;
    }

    Plasma::FormFactor f = containment()->formFactor();
    int insertIndex = -1;

    //FIXME: needed in two places, make it a function?
    for (int i = 0; i < lay->count(); ++i) {
        QRectF siblingGeometry = lay->itemAt(i)->geometry();

        if (f == Plasma::Horizontal) {
            qreal middle = (siblingGeometry.left() + siblingGeometry.right()) / 2.0;
            if (pos.x() < middle) {
                insertIndex = i;
                break;
            } else if (pos.x() <= siblingGeometry.right()) {
                insertIndex = i + 1;
                break;
            }
        } else { // Plasma::Vertical
            qreal middle = (siblingGeometry.top() + siblingGeometry.bottom()) / 2.0;
            if (pos.y() < middle) {
                insertIndex = i;
                break;
            } else if (pos.y() <= siblingGeometry.bottom()) {
                insertIndex = i + 1;
                break;
            }
        }
    }

    m_spacerIndex = insertIndex;
    if (insertIndex != -1) {
        if (!m_spacer) {
            m_spacer = new QGraphicsWidget(containment());
            //m_spacer->panel = this;
        }
        lay->removeItem(m_spacer);
        m_spacer->show();
        lay->insertItem(insertIndex, m_spacer);
    }
}

void PanelView::dragEnterEvent(QDragEnterEvent *event)
{
    Plasma::Containment *c = containment();
    if (c && c->immutability() == Plasma::Mutable &&
        (event->mimeData()->hasFormat(static_cast<Plasma::Corona*>(scene())->appletMimeType()) ||
         KUrl::List::canDecode(event->mimeData()))) {
        containment()->showDropZone(event->pos());
    }

    //the containment will do the last decision whether accept it or not
    Plasma::View::dragEnterEvent(event);
}

void PanelView::dragMoveEvent(QDragMoveEvent *event)
{
    Plasma::Containment *c = containment();
    if (c && c->immutability() == Plasma::Mutable &&
        (event->mimeData()->hasFormat(static_cast<Plasma::Corona*>(scene())->appletMimeType()) ||
         KUrl::List::canDecode(event->mimeData()))) {
        containment()->showDropZone(event->pos());
    }

    Plasma::View::dragMoveEvent(event);
}

void PanelView::dragLeaveEvent(QDragLeaveEvent *event)
{
    if (containment()) {
        containment()->showDropZone(QPoint());
    }

    Plasma::View::dragLeaveEvent(event);
}

void PanelView::dropEvent(QDropEvent *event)
{
    Plasma::View::dropEvent(event);

    if (containment()) {
        containment()->showDropZone(QPoint());
    }
}


#include "panelview.moc"


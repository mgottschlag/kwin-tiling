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
#include <QGraphicsLinearLayout>
#include <QPropertyAnimation>
#include <QTimer>
#ifdef Q_WS_X11
#include <X11/Xatom.h>
#include <QX11Info>
#include <X11/extensions/shape.h>
#endif

#include <KDebug>
#include <KIdleTime>
#include <KWindowSystem>

#include <Plasma/Containment>
#include <Plasma/Corona>
#include <Plasma/Plasma>
#include <Plasma/PopupApplet>
#include <Plasma/Svg>
#include <Plasma/Theme>
#include <Plasma/WindowEffects>

#include <kephal/screens.h>

#include "desktopcorona.h"
#include "panelappletoverlay.h"
#include "panelcontroller.h"
#include "panelshadows.h"
#include "plasmaapp.h"

class GlowBar : public QWidget
{
public:
    GlowBar(Plasma::Direction direction, const QRect &triggerZone)
        : QWidget(0),
          m_strength(0.3),
          m_svg(new Plasma::Svg(this)),
          m_direction(direction)
    {
        setAttribute(Qt::WA_TranslucentBackground);
        KWindowSystem::setOnAllDesktops(winId(), true);
        KWindowSystem::setType(winId(), NET::Dock);
        m_svg->setImagePath("widgets/glowbar");

        QPalette pal = palette();
        pal.setColor(backgroundRole(), Qt::transparent);
        setPalette(pal);

#ifdef Q_WS_X11
        QRegion region(QRect(0,0,1,1));
        XShapeCombineRegion(QX11Info::display(), winId(), ShapeInput, 0, 0,
                            region.handle(), ShapeSet);
#endif

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

        m_buffer.fill(QColor(0, 0, 0, int(qreal(255)*m_strength)));
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
            p.drawPixmap(pixmapPosition, l);
            p.drawTiledPixmap(QRect(pixmapPosition.x(), l.height(), c.width(), height() - l.height() - r.height()), c);
            p.drawPixmap(QPoint(pixmapPosition.x(), height() - r.height()), r);
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
      m_strutsTimer(new QTimer(this)),
      m_rehideAfterAutounhideTimer(new QTimer(this)),
      m_spacer(0),
      m_spacerIndex(-1),
#ifdef Q_WS_X11
      m_unhideTrigger(None),
#endif
      m_visibilityMode(NormalPanel),
      m_lastHorizontal(true),
      m_editing(false),
      m_triggerEntered(false),
      m_respectStatus(true)
{
    setAttribute(Qt::WA_TranslucentBackground);
    PlasmaApp::self()->panelShadows()->addWindow(this);

    // KWin setup
    KWindowSystem::setOnAllDesktops(winId(), true);
    KWindowSystem::setType(winId(), NET::Dock);
    setWindowRole(QString("panel_%1").arg(id));

    m_strutsTimer->setSingleShot(true);
    connect(m_strutsTimer, SIGNAL(timeout()), this, SLOT(updateStruts()));

    // this timer controls checks to re-hide a panel after it's been unhidden
    // for the user because, e.g., something is demanding attention
    m_rehideAfterAutounhideTimer->setSingleShot(true);
    connect(m_rehideAfterAutounhideTimer, SIGNAL(timeout()), this, SLOT(checkAutounhide()));

    // Graphics view setup
    setFrameStyle(QFrame::NoFrame);
    setInteractive(true);
    setAcceptDrops(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QPalette pal = palette();
    pal.setBrush(backgroundRole(), Qt::transparent);
    setPalette(pal);

#ifdef Q_WS_WIN
    registerAccessBar(true);
#endif

    KConfigGroup viewConfig = config();
    KConfigGroup sizes = KConfigGroup(&viewConfig, "Sizes");
    m_lastHorizontal = isHorizontal();

    const bool onScreen = panel->screen() < PlasmaApp::self()->corona()->numScreens();
    const QRect screenRect = onScreen ?  PlasmaApp::self()->corona()->screenGeometry(panel->screen()) : QRect();
    const int sw = screenRect.width();
    const int sh = screenRect.height();
    m_lastSeenSize = sizes.readEntry("lastsize", m_lastHorizontal ? sw : sh);

    if (onScreen) {
        const QString last = m_lastHorizontal ? QString::fromLatin1("Horizontal%1").arg(QString::number(sw)) :
                                                QString::fromLatin1("Vertical%1").arg(QString::number(sh));
        if (sizes.hasGroup(last)) {
            KConfigGroup thisSize(&sizes, last);
            resize(thisSize.readEntry("size", m_lastHorizontal ? QSize(sw, 27) : QSize(27, sh)));
        }
    } else {
        resize(panel->size().toSize());
    }

    m_alignment = alignmentFilter((Qt::Alignment)viewConfig.readEntry("Alignment", (int)Qt::AlignLeft));
    KConfigGroup sizeConfig(&viewConfig, (m_lastHorizontal ? "Horizontal" : "Vertical") +
                            QString::number(m_lastSeenSize));
    m_offset = sizeConfig.readEntry("offset", 0);
    setVisibilityMode((VisibilityMode)viewConfig.readEntry("panelVisibility", (int)m_visibilityMode));

    connect(this, SIGNAL(sceneRectAboutToChange()), this, SLOT(pinchContainmentToCurrentScreen()));

    Kephal::Screens *screens = Kephal::Screens::self();
    connect(screens, SIGNAL(screenResized(Kephal::Screen*,QSize,QSize)),
            this, SLOT(pinchContainmentToCurrentScreen()));
    connect(screens, SIGNAL(screenMoved(Kephal::Screen*,QPoint,QPoint)),
            this, SLOT(updatePanelGeometry()));
    connect(screens, SIGNAL(screenAdded(Kephal::Screen*)),
            this, SLOT(updateStruts()));
    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), this, SLOT(themeChanged()), Qt::QueuedConnection);
}

PanelView::~PanelView()
{
    if (m_panelController) {
        disconnect(m_panelController, 0, this, 0);
        delete m_panelController;
    }

    delete m_glowBar;
    destroyUnhideTrigger();
#ifdef Q_WS_WIN
    registerAccessBar(false);
#endif
}

void PanelView::setContainment(Plasma::Containment *containment)
{
    kDebug() << "Panel geometry is" << containment->geometry();

    Plasma::Containment *oldContainment = this->containment();
    if (oldContainment) {
        disconnect(oldContainment);
    }

    PlasmaApp::self()->prepareContainment(containment);

    connect(containment, SIGNAL(newStatus(Plasma::ItemStatus)), this, SLOT(statusUpdated(Plasma::ItemStatus)));
    connect(containment, SIGNAL(destroyed(QObject*)), this, SLOT(panelDeleted()));
    connect(containment, SIGNAL(toolBoxToggled()), this, SLOT(togglePanelController()));
    connect(containment, SIGNAL(appletAdded(Plasma::Applet*,QPointF)), this, SLOT(appletAdded(Plasma::Applet*)));
    connect(containment, SIGNAL(showAddWidgetsInterface(QPointF)), this, SLOT(showWidgetExplorer()));
    connect(containment, SIGNAL(screenChanged(int,int,Plasma::Containment*)), this, SLOT(pinchContainmentToCurrentScreen()));
    connect(containment, SIGNAL(immutabilityChanged(Plasma::ImmutabilityType)), this, SLOT(immutabilityChanged(Plasma::ImmutabilityType)));

    KConfigGroup viewIds(KGlobal::config(), "ViewIds");

    if (oldContainment) {
        viewIds.deleteEntry(QString::number(oldContainment->id()));
    }

    if (containment) {
        viewIds.writeEntry(QString::number(containment->id()), id());
        if (containment->corona()) {
            containment->corona()->requestConfigSync();
        }
    }

    // ensure we aren't overlapping other panels
    const QRect screenRect = PlasmaApp::self()->corona()->screenGeometry(containment->screen());

    View::setContainment(containment);

    // pinchContainment calls updatePanelGeometry for us
    pinchContainment(screenRect);
    m_lastMin = containment->minimumSize();
    m_lastMax = containment->maximumSize();

    kDebug() << "about to set the containment" << (QObject*)containment;

    updateStruts();

    // if we are an autohiding panel, then see if the status mandates we do something about it
    if (m_visibilityMode != NormalPanel && m_visibilityMode != WindowsGoBelow) {
        checkUnhide(containment->status());
    }
}

void PanelView::themeChanged()
{
    recreateUnhideTrigger();
}

void PanelView::setPanelDragPosition(const QPoint &point)
{
    QRect screenGeom = PlasmaApp::self()->corona()->screenGeometry(containment()->screen());
    QRect geom = geometry();
    geom.translate(-point);
    if (screenGeom.contains(geom)) {
        move(pos() - point);
        if (m_panelController) {
            m_panelController->move(m_panelController->pos() - point);
        }
    }
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
                QRect screenGeom = PlasmaApp::self()->corona()->screenGeometry(c->screen());
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
                QRect screenGeom = PlasmaApp::self()->corona()->screenGeometry(c->screen());
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
    disconnect(this, SIGNAL(sceneRectAboutToChange()), this, SLOT(pinchContainmentToCurrentScreen()));
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
    const QRect screenRect = PlasmaApp::self()->corona()->screenGeometry(c->screen());
    pinchContainment(screenRect);
    KWindowSystem::setOnAllDesktops(winId(), true);
    connect(this, SIGNAL(sceneRectAboutToChange()), this, SLOT(pinchContainmentToCurrentScreen()));
}

Plasma::Location PanelView::location() const
{
    if (containment()) {
        return containment()->location();
    } else {
        return Plasma::BottomEdge;
    }
}

void PanelView::setVisibilityMode(PanelView::VisibilityMode mode)
{
    m_visibilityMode = mode;

    if (mode == LetWindowsCover) {
        KWindowSystem::setState(winId(), NET::KeepBelow);
    } else {
        KWindowSystem::clearState(winId(), NET::KeepBelow);
    }
    //life is vastly simpler if we ensure we're visible now
    unhide();

    disconnect(containment(), SIGNAL(activate()), this, SLOT(unhide()));
    disconnect(containment(), SIGNAL(newStatus(Plasma::ItemStatus)), this, SLOT(checkUnhide(Plasma::ItemStatus)));
    if (mode == NormalPanel || mode == WindowsGoBelow) {
        //remove the last remnants of hide/unhide
        delete m_mousePollTimer;
        m_mousePollTimer = 0;
    } else {
        connect(containment(), SIGNAL(activate()), this, SLOT(unhide()));
        connect(containment(), SIGNAL(newStatus(Plasma::ItemStatus)), this, SLOT(checkUnhide(Plasma::ItemStatus)));
    }

    config().writeEntry("panelVisibility", (int)mode);

    //if the user didn't cause this, hide again in a bit
    if ((mode == AutoHide || mode == LetWindowsCover) && !m_editing) {
        if (m_mousePollTimer) {
            m_mousePollTimer->stop();
        }

        QTimer::singleShot(2000, this, SLOT(startAutoHide()));
    }

    KWindowSystem::setOnAllDesktops(winId(), true);
}

PanelView::VisibilityMode PanelView::visibilityMode() const
{
    return m_visibilityMode;
}

void PanelView::updatePanelGeometry()
{
    Plasma::Containment *c = containment();

    if (!c) {
        return;
    }

    kDebug() << "New panel geometry is" << c->geometry();

    QSize size = c->size().expandedTo(c->minimumSize()).toSize();
    QRect geom(QPoint(0,0), size);
    int screen = c->screen();

    if (screen < 0) {
        //TODO: is there a valid use for -1 with a panel? floating maybe?
        screen = 0;
    }

    QRect screenGeom = PlasmaApp::self()->corona()->screenGeometry(screen);

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

        break;

    case Plasma::LeftEdge:
        if (m_alignment == Qt::AlignLeft) {
            geom.moveTopLeft(QPoint(screenGeom.left(), m_offset + screenGeom.top()));
        } else if (m_alignment == Qt::AlignRight) {
            geom.moveBottomLeft(QPoint(screenGeom.left(), screenGeom.bottom() - m_offset));
        } else if (m_alignment == Qt::AlignCenter) {
            geom.moveTopLeft(QPoint(screenGeom.left(), screenGeom.center().y() - geom.height()/2 + 1 - geom.height()%2 + m_offset));
        }

        break;

    case Plasma::RightEdge:
        if (m_alignment == Qt::AlignLeft) {
            geom.moveTopRight(QPoint(screenGeom.right(), m_offset + screenGeom.top()));
        } else if (m_alignment == Qt::AlignRight) {
            geom.moveBottomRight(QPoint(screenGeom.right(), screenGeom.bottom() - m_offset));
        } else if (m_alignment == Qt::AlignCenter) {
            geom.moveTopRight(QPoint(screenGeom.right(), screenGeom.center().y() - geom.height()/2 + 1 - geom.height()%2 + m_offset));
        }

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

        break;
    }

    kDebug() << (QObject*)this << "thinks its panel is at " << geom << "was" << geometry();
    if (geom == geometry()) {
        // our geometry is the same, but the panel moved around
        // so make sure our struts are still valid
        m_strutsTimer->stop();
        m_strutsTimer->start(STRUTSTIMERDELAY);
    } else {
        if (m_panelController && QPoint(pos() - geom.topLeft()).manhattanLength() > 100) {
            resize(geom.size());
            QPropertyAnimation *panelAnimation = new QPropertyAnimation(this, "pos", this);
            panelAnimation->setEasingCurve(QEasingCurve::InOutQuad);
            panelAnimation->setDuration(300);
            panelAnimation->setStartValue(pos());
            panelAnimation->setEndValue(geom.topLeft());
            panelAnimation->start(QAbstractAnimation::DeleteWhenStopped);

            QPropertyAnimation *controllerAnimation = new QPropertyAnimation(m_panelController, "pos", m_panelController);
            controllerAnimation->setEasingCurve(QEasingCurve::InOutQuad);
            controllerAnimation->setDuration(300);
            controllerAnimation->setStartValue(m_panelController->pos());
            controllerAnimation->setEndValue(m_panelController->positionForPanelGeometry(geom));
            controllerAnimation->start(QAbstractAnimation::DeleteWhenStopped);
        } else {
            setGeometry(geom);
        }
    }

    m_lastMin = c->minimumSize();
    m_lastMax = c->maximumSize();

    //update the panel controller location position and size
    if (m_panelController) {
        m_panelController->setLocation(c->location());

        foreach (PanelAppletOverlay *o, m_appletOverlays) {
            o->syncOrientation();
        }
    }

    recreateUnhideTrigger();
}

bool PanelView::isHorizontal() const
{
    return location() == Plasma::BottomEdge ||
           location() == Plasma::TopEdge;
}

void PanelView::pinchContainmentToCurrentScreen()
{
    kDebug() << "pinching to current screen";
    const QRect screenRect = PlasmaApp::self()->corona()->screenGeometry(containment()->screen());
    pinchContainment(screenRect);
}

void PanelView::pinchContainment(const QRect &screenGeom)
{
    kDebug() << "**************************** pinching" << screenGeom << m_lastSeenSize;
    disconnect(this, SIGNAL(sceneRectAboutToChange()), this, SLOT(pinchContainmentToCurrentScreen()));
    bool horizontal = isHorizontal();

    const int oldOffset = m_offset;
    const int sw = screenGeom.width();
    const int sh = screenGeom.height();

    Plasma::Containment *c = containment();
    QSizeF min = c->minimumSize();
    QSizeF max = c->maximumSize();

    KConfigGroup sizes = config();
    sizes = KConfigGroup(&sizes, "Sizes");

    if (m_lastHorizontal != horizontal || m_lastSeenSize != (horizontal ? sw : sh)) {
        // we're adjusting size. store the current size now
        KConfigGroup lastSize(&sizes, (m_lastHorizontal ? "Horizontal" : "Vertical") +
                                      QString::number(m_lastSeenSize));
        lastSize.writeEntry("size", size());
        lastSize.writeEntry("offset", m_offset);
        lastSize.writeEntry("min", m_lastMin);
        lastSize.writeEntry("max", m_lastMax);
        configNeedsSaving();

        const QString last = horizontal ? QString::fromLatin1("Horizontal%1").arg(QString::number(sw)) :
                                          QString::fromLatin1("Vertical%1").arg(QString::number(sh));
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
        }
        if (m_lastSeenSize < (horizontal ? sw : sh) &&
                   (horizontal ? c->geometry().width() :
                                 c->geometry().height()) >= m_lastSeenSize) {
            // we are moving from a smaller space where we are 100% to a larger one
            kDebug() << "we are moving from a smaller space where we are 100% to a larger one";
            c->setMinimumSize(0, 0);
            c->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);

            if (horizontal) {
                c->setMaximumSize(sw, max.height());
                c->resize(sw, c->geometry().height());
                if (min.width() == max.width()) {
                    c->setMinimumSize(sw, min.height());
                }
            } else {
                c->setMaximumSize(max.width(), sh);
                c->resize(c->geometry().width(), sh);
                if (min.height() == max.height()) {
                    c->setMinimumSize(min.width(), sh);
                }
            }
        } else if (m_lastSeenSize > (horizontal ? sw : sh) &&
                    (m_offset + (horizontal ? c->geometry().width() :
                                 c->geometry().height())) > (horizontal ? sw : sh)) {
            kDebug() << "we are moving from a bigger space to a smaller one where the panel won't fit!!";
            if ((horizontal ? c->geometry().width() :
                                 c->geometry().height()) > (horizontal ? sw : sh)) {
                kDebug() << "panel is larger than screen, adjusting panel size";
                setOffset(0);
                c->setMinimumSize(0, 0);
                c->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);

                if (horizontal) {
                    c->setMaximumSize(sw, max.height());
                    c->resize(sw, c->geometry().height());
                    if (min.width() == max.width()) {
                        c->setMinimumSize(sw, min.height());
                    }
                } else {
                    c->setMaximumSize(max.width(), sh);
                    c->resize(c->geometry().width(), sh);
                    if (min.height() == max.height()) {
                        c->setMinimumSize(min.width(), sh);
                    }
                }
            } else {
                kDebug() << "reducing offset so the panel fits in screen";
                setOffset((horizontal ? sw : sh) -
                          (horizontal ? c->geometry().width() : c->geometry().height()));
            }
        }
    }

    // Pinching strategy:
    // if our containment is too big for the size of the screen we are now on,
    // then we first try and limit the offset and then if that still doesn't
    // give us enough room, we limit the size of the panel itself by setting
    // the minimum and maximum sizes.

    //kDebug() << "checking panel" << c->geometry() << "against" << screenGeom;

    // resize to max if for some reason the size is empty
    // otherwise its not possible to interact with the panel at all
    if (c->size().isEmpty()) {
        c->resize(max);
    }

    // write to the config file if the size has changed, or if we haven't recorded the lastsize
    // previously which ensures we'll always have a value even after first run
    const bool writeConfig = m_lastSeenSize != (horizontal ? sw : sh) || !sizes.hasKey("lastsize");
    m_lastHorizontal = horizontal;
    m_lastSeenSize = (horizontal ? sw : sh);
    if (writeConfig) {
        sizes.writeEntry("lastsize", m_lastSeenSize);
        configNeedsSaving();
    }

    updatePanelGeometry();

    if (m_offset != oldOffset) {
        sizes.writeEntry("offset", m_lastSeenSize);
        configNeedsSaving();
    }

    if (m_panelController) {
        m_panelController->setOffset(m_offset);
    }

    connect(this, SIGNAL(sceneRectAboutToChange()), this, SLOT(pinchContainmentToCurrentScreen()));
    recreateUnhideTrigger();
    kDebug() << "Done pinching, containment's geom" << c->geometry() << "own geom" << geometry();
}

void PanelView::setOffset(int newOffset)
{
    m_offset = newOffset;

    //TODO: do we ever need to worry about pinching here, or
    //      do we just assume that the offset is always < screenSize - containmentSize?
    updatePanelGeometry();

    KConfigGroup viewConfig = config();
    viewConfig = KConfigGroup(&viewConfig, (m_lastHorizontal ? "Horizontal" : "Vertical") +
                              QString::number(m_lastSeenSize));
    viewConfig.writeEntry("offset", m_offset);
    configNeedsSaving();
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
    configNeedsSaving();
}

Qt::Alignment PanelView::alignment() const
{
    return m_alignment;
}

void PanelView::immutabilityChanged(Plasma::ImmutabilityType immutability)
{
    if (immutability != Plasma::Mutable) {
        delete m_panelController;
        m_panelController = 0;
    }
}

void PanelView::togglePanelController()
{
    //kDebug();
    m_editing = false;
    if (containment()->immutability() != Plasma::Mutable) {
        delete m_panelController;
        m_panelController = 0;
        return;
    }

    if (!m_panelController) {
        m_panelController = new PanelController(0);
        m_panelController->setContainment(containment());
        m_panelController->setLocation(containment()->location());
        m_panelController->setAlignment(m_alignment);
        m_panelController->setOffset(m_offset);
        m_panelController->setVisibilityMode(m_visibilityMode);

        connect(m_panelController, SIGNAL(destroyed(QObject*)), this, SLOT(editingComplete()));
        connect(m_panelController, SIGNAL(offsetChanged(int)), this, SLOT(setOffset(int)));
        connect(m_panelController, SIGNAL(partialMove(QPoint)), this, SLOT(setPanelDragPosition(QPoint)));
        connect(m_panelController, SIGNAL(alignmentChanged(Qt::Alignment)), this, SLOT(setAlignment(Qt::Alignment)));
        connect(m_panelController, SIGNAL(locationChanged(Plasma::Location)), this, SLOT(setLocation(Plasma::Location)));
        connect(m_panelController, SIGNAL(panelVisibilityModeChanged(PanelView::VisibilityMode)), this, SLOT(setVisibilityMode(PanelView::VisibilityMode)));

        if (containment()->containmentType() == Plasma::Containment::PanelContainment && 
	    dynamic_cast<QGraphicsLinearLayout*>(containment()->layout())) {
            setTabOrder(0, m_panelController);
            QWidget *prior = m_panelController;

            // we only support mouse over drags for panels with linear layouts
            QColor overlayColor(Plasma::Theme::defaultTheme()->color(Plasma::Theme::BackgroundColor));
            QBrush overlayBrush(overlayColor);
            QPalette p(palette());
            p.setBrush(QPalette::Window, overlayBrush);
            foreach (Plasma::Applet *applet, containment()->applets()) {
                PanelAppletOverlay *moveOverlay = new PanelAppletOverlay(applet, this);
                connect(moveOverlay, SIGNAL(removedWithApplet(PanelAppletOverlay*)),
                        this, SLOT(overlayDestroyed(PanelAppletOverlay*)));
                connect(moveOverlay, SIGNAL(moved(PanelAppletOverlay*)),
                        this, SLOT(overlayMoved(PanelAppletOverlay*)));
                moveOverlay->setPalette(p);
                moveOverlay->show();
                moveOverlay->raise();
                m_appletOverlays << moveOverlay;
                //kDebug() << moveOverlay << moveOverlay->geometry();
                setTabOrder(prior, moveOverlay);
                prior = moveOverlay;
            }
        }
    }

    if (m_panelController->isVisible()) {
        if (m_panelController->showingWidgetExplorer() ||
            m_panelController->showingActivityManager()) {
            m_panelController->switchToController();
            m_panelController->move(m_panelController->positionForPanelGeometry(geometry()));
        } else {
            Plasma::WindowEffects::slideWindow(m_panelController, location());
            m_panelController->close();
            updateStruts();
        }
    } else {
        m_editing = true;
        m_panelController->resize(m_panelController->sizeHint());
        m_panelController->move(m_panelController->positionForPanelGeometry(geometry()));
        Plasma::WindowEffects::slideWindow(m_panelController, location());
        kDebug() << "showing panel controller!" << m_panelController->geometry();
        m_panelController->show();
    }
}

void PanelView::editingComplete()
{
    //kDebug();
    m_panelController = 0;
    m_editing = false;
    qDeleteAll(m_appletOverlays);
    m_appletOverlays.clear();

    if (!containment()) {
        return;
    }

    containment()->closeToolBox();
    updateStruts();

    if (m_visibilityMode == LetWindowsCover || m_visibilityMode == AutoHide) {
        startAutoHide();
    }
}

void PanelView::overlayDestroyed(PanelAppletOverlay *overlay)
{
    m_appletOverlays.remove(overlay);
}

void PanelView::overlayMoved(PanelAppletOverlay *overlay)
{
    Q_UNUSED(overlay)
    foreach (PanelAppletOverlay *o, m_appletOverlays) {
        o->syncIndex();
    }
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
    if (!containment()) {
        return;
    }

    NETExtendedStrut strut;

    if (m_visibilityMode == NormalPanel) {
        const QRect thisScreen = PlasmaApp::self()->corona()->screenGeometry(containment()->screen());
        const QRect wholeScreen = Kephal::ScreenUtils::desktopGeometry();

        //Extended struts against a screen edge near to another screen are really harmful, so windows maximized under the panel is a lesser pain
        //TODO: force "windows can cover" in those cases?
        const int numScreens = PlasmaApp::self()->corona()->numScreens();
        for (int i = 0; i < numScreens; ++i) {
            if (i == containment()->screen()) {
                continue;
            }

            const QRect otherScreen = PlasmaApp::self()->corona()->screenGeometry(i);

            switch (location())
            {
            case Plasma::TopEdge:
                if (otherScreen.bottom() <= thisScreen.top()) {
                    KWindowSystem::setExtendedStrut(winId(), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
                    return;
                }
                break;
            case Plasma::BottomEdge:
                if (otherScreen.top() >= thisScreen.bottom()) {
                    KWindowSystem::setExtendedStrut(winId(), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
                    return;
                }
                break;
            case Plasma::RightEdge:
                if (otherScreen.left() >= thisScreen.right()) {
                    KWindowSystem::setExtendedStrut(winId(), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
                    return;
                }
                break;
            case Plasma::LeftEdge:
                if (otherScreen.right() <= thisScreen.left()) {
                    KWindowSystem::setExtendedStrut(winId(), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
                    return;
                }
                break;
            default:
                return;
            }
        }
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

    if (m_panelController) {
        m_panelController->setLocation(containment()->location());

        if (m_panelController->isVisible()) {
            m_panelController->resize(m_panelController->sizeHint());
            m_panelController->move(m_panelController->positionForPanelGeometry(geometry()));
            Plasma::WindowEffects::slideWindow(m_panelController, location());
        }

        foreach (PanelAppletOverlay *o, m_appletOverlays) {
            o->syncOrientation();
        }
    }

    recreateUnhideTrigger();
}

bool PanelView:: migratedFrom(int screenId) const
{
    KConfigGroup cg = config();
    const QList<int> migrations = cg.readEntry("Migrations", QList<int>());
    return migrations.contains(screenId);
}

void PanelView::migrateTo(int screenId)
{
    KConfigGroup cg = config();
    QList<int> migrations = cg.readEntry("Migrations", QList<int>());

    const int index = migrations.indexOf(screenId);
    if (index == -1) {
        migrations.append(screenId);
    } else {
        migrations = migrations.mid(0, migrations.length() - index - 1);
    }

    cg.writeEntry("Migrations", migrations);
    setScreen(screenId);
}

void PanelView::showWidgetExplorer()
{
    if (!containment()) {
        return;
    }

    if (!m_panelController) {
        m_editing = true;
        ControllerWindow *controller = PlasmaApp::self()->showWidgetExplorer(screen(), containment());
        connect(controller, SIGNAL(destroyed(QObject*)), this, SLOT(editingComplete()), Qt::UniqueConnection);
    } else {
        m_panelController->showWidgetExplorer();
    }
}

void PanelView::moveEvent(QMoveEvent *event)
{
    Plasma::View::moveEvent(event);
    m_strutsTimer->stop();
    m_strutsTimer->start(STRUTSTIMERDELAY);
    recreateUnhideTrigger();

    if (containment()) {
        foreach (Plasma::Applet *applet, containment()->applets()) {
            applet->updateConstraints(Plasma::PopupConstraint);
        }
    }
}

void PanelView::resizeEvent(QResizeEvent *event)
{
    //kDebug() << event->oldSize() << event->size();
    Plasma::View::resizeEvent(event);
    recreateUnhideTrigger();
    m_strutsTimer->stop();
    m_strutsTimer->start(STRUTSTIMERDELAY);
#ifdef Q_WS_WIN
    appBarPosChanged();
#endif

    if (containment()) {
        foreach (Plasma::Applet *applet, containment()->applets()) {
            applet->updateConstraints(Plasma::PopupConstraint);
        }
    }
}

void PanelView::updateHinter()
{
#ifdef Q_WS_X11
    QPoint mousePos = QCursor::pos();
    m_glowBar->updateStrength(mousePos);

    if (!m_unhideTriggerGeom.contains(mousePos)) {
        hideHinter();
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

bool PanelView::hintOrUnhide(const QPoint &point, bool dueToDnd)
{
#ifdef Q_WS_X11
    if (m_visibilityMode != LetWindowsCover && isVisible()) {
        return false;
    }

    KWindowInfo activeWindow = KWindowSystem::windowInfo(KWindowSystem::activeWindow(), NET::WMState);
    if (activeWindow.state() & NET::FullScreen) {
        return false;
    }

    if (!shouldHintHide()) {
        //kDebug() << "should not hint hide";
        unhide(!dueToDnd);
        return true;
    }

    //kDebug() << point << m_triggerZone;
    if (m_triggerZone.contains(point)) {
        //kDebug() << "unhide!" << point;
        unhide(!dueToDnd);
        return true;
    } else if (!m_glowBar) {
        Plasma::Direction direction = Plasma::locationToDirection(location());
        m_glowBar = new GlowBar(direction, m_triggerZone);
        m_glowBar->show();
        XMoveResizeWindow(QX11Info::display(), m_unhideTrigger, m_triggerZone.x(), m_triggerZone.y(), m_triggerZone.width(), m_triggerZone.height());

        //FIXME: This is ugly as hell but well, yeah
        if (!m_mousePollTimer) {
            m_mousePollTimer = new QTimer(this);
        }

        connect(m_mousePollTimer, SIGNAL(timeout()), this, SLOT(updateHinter()), Qt::UniqueConnection);
        m_mousePollTimer->start(200);
    }

    return false;
#else
    return false;
#endif
}

void PanelView::hideHinter()
{
    //kDebug() << "hide the glow";
    if (m_mousePollTimer) {
        m_mousePollTimer->stop();
        disconnect(m_mousePollTimer, SIGNAL(timeout()), this, SLOT(updateHinter()));
    }

    delete m_glowBar;
    m_glowBar = 0;
}

bool PanelView::hasPopup()
{
    if (QApplication::activePopupWidget() || m_panelController) {
        return true;
    }

    if (containment()) {
        foreach (Plasma::Applet *applet, containment()->applets()) {
            if (applet->isPopupShowing()) {
                return true;
            }
        }
    }

    return false;
}

void PanelView::unhide(bool destroyTrigger)
{
    //kill the unhide stuff
    hideHinter();
    if (destroyTrigger) {
        destroyUnhideTrigger();
    }

    //ensure it's visible
    if (!isVisible()) {
        Plasma::WindowEffects::slideWindow(this, location());
        show();
        KWindowSystem::raiseWindow(winId());
    }

    KWindowSystem::setOnAllDesktops(winId(), true);

    //non-hiding panels stop here
    if (m_visibilityMode == NormalPanel || m_visibilityMode == WindowsGoBelow) {
        return;
    }

    //set up the re-hiding stuff
    if (!m_mousePollTimer) {
        m_mousePollTimer = new QTimer(this);
    }

    connect(m_mousePollTimer, SIGNAL(timeout()), this, SLOT(startAutoHide()), Qt::UniqueConnection);
    m_mousePollTimer->start(500);

    //avoid hide-show loops
    if (m_visibilityMode == LetWindowsCover) {
        m_triggerEntered = true;
        KWindowSystem::clearState(winId(), NET::KeepBelow);
        KWindowSystem::raiseWindow(winId());
        QTimer::singleShot(0, this, SLOT(resetTriggerEnteredSuppression()));
    }
}

void PanelView::statusUpdated(Plasma::ItemStatus newStatus)
{
    if (newStatus == Plasma::AcceptingInputStatus) {
        KWindowSystem::forceActiveWindow(winId());
    }
}

void PanelView::checkUnhide(Plasma::ItemStatus newStatus)
{
    //kDebug() << "================= got a status: " << newStatus << Plasma::ActiveStatus;
    m_respectStatus = true;

    if (newStatus > Plasma::ActiveStatus) {
        unhide();
        if (newStatus == Plasma::NeedsAttentionStatus) {
            //kDebug() << "starting the timer!";
            // start our rehide timer, so that the panel doesn't stay up and stuck forever and a day
            m_rehideAfterAutounhideTimer->start(AUTOUNHIDE_CHECK_DELAY);
        }
    } else {
        //kDebug() << "new status, just autohiding";
        startAutoHide();
    }
}

void PanelView::checkAutounhide()
{
    //kDebug() << "***************************" << KIdleTime::instance()->idleTime();
    if (KIdleTime::instance()->idleTime() >= AUTOUNHIDE_CHECK_DELAY) {
        // the user is idle .. let's not hige the panel on them quite yet, but rather given them a
        // chance to see this thing!
        connect(KIdleTime::instance(), SIGNAL(resumingFromIdle()), this, SLOT(checkAutounhide()),
                Qt::UniqueConnection);
        KIdleTime::instance()->catchNextResumeEvent();
        //kDebug() << "exit 1 ***************************";
        return;
    }

    m_respectStatus = false;
    //kDebug() << "in to check ... who's resonsible?" << sender() << KIdleTime::instance();
    if (sender() == KIdleTime::instance()) {
        //kDebug() << "doing a 2s wait";
        QTimer::singleShot(2000, this, SLOT(startAutoHide()));
    } else {
        //kDebug() << "just starting autohide!";
        startAutoHide();
    }

    // this line must come after the check on sender() as it *clears* that value!
    disconnect(KIdleTime::instance(), SIGNAL(resumingFromIdle()), this, SLOT(checkAutounhide()));
    //kDebug() << "exit 0 ***************************";
}

void PanelView::unhide()
{
    unhide(true);
}

void PanelView::resetTriggerEnteredSuppression()
{
    m_triggerEntered = false;
}

void PanelView::startAutoHide()
{
    /*
    kDebug() << m_editing << (containment() ? containment()->status() : 0) << Plasma::ActiveStatus
             << geometry().adjusted(-10, -10, 10, 10).contains(QCursor::pos()) << hasPopup();
    if (containment() && containment()->status() > Plasma::ActiveStatus) {
        foreach (Plasma::Applet *applet, containment()->applets()) {
            kDebug() << "     " << applet->name() << applet->status();
        }
    }
    */


    if (m_editing || (m_respectStatus && (containment() && containment()->status() > Plasma::ActiveStatus))) {
        if (m_mousePollTimer) {
            m_mousePollTimer->stop();
            disconnect(m_mousePollTimer, SIGNAL(timeout()), this, SLOT(startAutoHide()));
        }

        return;
    }

    // since we've gotten this far, we don't need to worry about rehiding-after-auto-unhide, so just
    // stop the timer
    m_rehideAfterAutounhideTimer->stop();

    if (geometry().adjusted(-10, -10, 10, 10).contains(QCursor::pos()) || hasPopup()) {
        if (!m_mousePollTimer) {
            leaveEvent(0);
        }
        return;
    }

    if (m_mousePollTimer) {
        m_mousePollTimer->stop();
        disconnect(m_mousePollTimer, SIGNAL(timeout()), this, SLOT(startAutoHide()));
    }

    if (m_visibilityMode == LetWindowsCover) {
        KWindowSystem::setState(winId(), NET::KeepBelow);
        KWindowSystem::lowerWindow(winId());
        createUnhideTrigger();
    } else {
        Plasma::WindowEffects::slideWindow(this, location());
        createUnhideTrigger();
        hide();
    }
}

void PanelView::leaveEvent(QEvent *event)
{
    if (m_visibilityMode == LetWindowsCover && m_triggerEntered) {
        //this prevents crazy hide-unhide loops that can happen at times
        m_triggerEntered = false;
    } else if (containment() &&
               (m_visibilityMode == AutoHide || m_visibilityMode == LetWindowsCover) && !m_editing) {
        // even if we dont have a popup, we'll start a timer, so
        // that the panel stays if the mouse only leaves for a
        // few ms
        if (!m_mousePollTimer) {
            m_mousePollTimer = new QTimer(this);
        }

        connect(m_mousePollTimer, SIGNAL(timeout()), this, SLOT(startAutoHide()), Qt::UniqueConnection);
        m_mousePollTimer->start(500);
    }

    if (event) {
        // startAutoHide calls this with a null event pointer, so we have to check it
        Plasma::View::leaveEvent(event);
    }
}

void PanelView::appletAdded(Plasma::Applet *applet)
{
    if (m_panelController && containment()->containmentType() == Plasma::Containment::PanelContainment) {
        QColor overlayColor(Plasma::Theme::defaultTheme()->color(Plasma::Theme::BackgroundColor));
        QBrush overlayBrush(overlayColor);
        QPalette p(palette());
        p.setBrush(QPalette::Window, overlayBrush);

        PanelAppletOverlay *moveOverlay = new PanelAppletOverlay(applet, this);
        connect(moveOverlay, SIGNAL(removedWithApplet(PanelAppletOverlay*)),
                this, SLOT(overlayDestroyed(PanelAppletOverlay*)));
        moveOverlay->setPalette(p);
        moveOverlay->show();
        moveOverlay->raise();
        m_appletOverlays << moveOverlay;

        QWidget *prior = m_panelController;
        Plasma::Applet *priorApplet = 0;
        foreach (Plasma::Applet *otherApplet, containment()->applets()) {
            if (applet == otherApplet) {
                break;
            }

            priorApplet = otherApplet;
        }

        if (priorApplet) {
            foreach (PanelAppletOverlay *overlay, m_appletOverlays) {
                if (overlay->applet() == priorApplet) {
                    prior = overlay;
                    break;
                }
            }
        }

        setTabOrder(prior, moveOverlay);
    }
}

bool PanelView::shouldHintHide() const
{
    return m_visibilityMode == AutoHide && PlasmaApp::hasComposite();
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
    //kDebug() << m_unhideTrigger << None;
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
                            KeyPressMask | ButtonPressMask |
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

    XChangeProperty(QX11Info::display(), m_unhideTrigger, PlasmaApp::self()->m_XdndAwareAtom,
                    XA_WINDOW, 32, PropModeReplace, (unsigned char *)&PlasmaApp::self()->m_XdndVersionAtom, 1);
    XMapWindow(QX11Info::display(), m_unhideTrigger);
    m_unhideTriggerGeom = QRect(triggerPoint, QSize(triggerWidth, triggerHeight));
    m_triggerZone = QRect(actualTriggerPoint, QSize(actualWidth, actualHeight));
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

    //kDebug();
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
        configNeedsSaving();
    }

    delete m_mousePollTimer;
    m_mousePollTimer = 0;
    m_strutsTimer->stop();

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


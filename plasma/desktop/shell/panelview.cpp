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
#include <QTimer>
#ifdef Q_WS_X11
#include <X11/Xatom.h>
#include <QX11Info>
#include <X11/extensions/shape.h>
#endif

#include <KWindowSystem>
#include <KDebug>

#include <Plasma/Containment>
#include <Plasma/Corona>
#include <Plasma/Plasma>
#include <Plasma/PopupApplet>
#include <Plasma/Svg>
#include <Plasma/Theme>
#include <Plasma/WindowEffects>

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


class ShadowWindow : public QWidget
{
public:
    ShadowWindow(PanelView *panel)
       : QWidget(0),
         m_panel(panel),
         m_valid(false)
    {
        setAttribute(Qt::WA_TranslucentBackground);
        setAttribute(Qt::WA_NoSystemBackground, false);
        setAutoFillBackground(false);
#ifdef Q_WS_X11
        QRegion region(QRect(0,0,1,1));
        XShapeCombineRegion(QX11Info::display(), winId(), ShapeInput, 0, 0,
                            region.handle(), ShapeSet);
#endif

        connect(m_shadow, SIGNAL(repaintNeeded()), m_panel, SLOT(checkShadow()));
        m_shadow = new Plasma::FrameSvg(this);
    }

    void setSvg(const QString &path)
    {
        m_shadow->setImagePath(path);

        if (!m_shadow->hasElementPrefix("shadow")) {
            hide();
            m_valid = false;
        } else {
            m_valid = true;
        }

        m_shadow->setElementPrefix("shadow");

        adjustMargins(geometry());
    }

    bool isValid() const
    {
        return m_valid;
    }

    void adjustMargins(const QRect &geo)
    {
        QRect screenRect = Kephal::ScreenUtils::screenGeometry(m_panel->screen());

        Plasma::FrameSvg::EnabledBorders enabledBorders = Plasma::FrameSvg::AllBorders;

        if (geo.left() <= screenRect.left()) {
            enabledBorders ^= Plasma::FrameSvg::LeftBorder;
        }
        if (geo.top() <= screenRect.top()) {
            enabledBorders ^= Plasma::FrameSvg::TopBorder;
        }
        if (geo.bottom() >= screenRect.bottom()) {
            enabledBorders ^= Plasma::FrameSvg::BottomBorder;
        }
        if (geo.right() >= screenRect.right()) {
            enabledBorders ^= Plasma::FrameSvg::RightBorder;
        }

        m_shadow->setEnabledBorders(enabledBorders);

        qreal left, top, right, bottom;

        m_shadow->getMargins(left, top, right, bottom);
        setContentsMargins(left, top, right, bottom);
    }

protected:
    bool event(QEvent *event)
    {
        if (event->type() == QEvent::Paint) {
            QPainter p(this);
            p.setCompositionMode(QPainter::CompositionMode_Source);
            p.fillRect(rect(), Qt::transparent);
        }
        return QWidget::event(event);
    }

    void resizeEvent(QResizeEvent *event)
    {
        m_shadow->resizeFrame(event->size());
        adjustMargins(geometry());
    }

    void paintEvent(QPaintEvent *e)
    {
        QPainter p(this);
        //p.setCompositionMode(QPainter::CompositionMode_Source);
        m_shadow->paintFrame(&p, e->rect(), e->rect());
    }


private:
    Plasma::FrameSvg *m_shadow;
    PanelView *m_panel;
    bool m_valid;
};

PanelView::PanelView(Plasma::Containment *panel, int id, QWidget *parent)
    : Plasma::View(panel, id, parent),
      m_panelController(0),
      m_glowBar(0),
      m_mousePollTimer(0),
      m_strutsTimer(new QTimer(this)),
      m_delayedUnhideTimer(new QTimer(this)),
      m_spacer(0),
      m_spacerIndex(-1),
      m_shadowWindow(0),
#ifdef Q_WS_X11
      m_unhideTrigger(None),
#endif
      m_visibilityMode(NormalPanel),
      m_lastHorizontal(true),
      m_editing(false),
      m_triggerEntered(false)
{
    // KWin setup
    KWindowSystem::setOnAllDesktops(winId(), true);
    KWindowSystem::setType(winId(), NET::Dock);
    setWindowRole(QString("panel_%1").arg(id));

    m_delayedUnhideTimer->setSingleShot(true);
    m_delayedUnhideTimer->setInterval(200);
    connect(m_delayedUnhideTimer, SIGNAL(timeout()), this, SLOT(delayedUnhide()));

    m_strutsTimer->setSingleShot(true);
    connect(m_strutsTimer, SIGNAL(timeout()), this, SLOT(updateStruts()));

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

#ifdef Q_WS_WIN
    registerAccessBar(true);
#endif

    KConfigGroup viewConfig = config();
    KConfigGroup sizes = KConfigGroup(&viewConfig, "Sizes");
    m_lastHorizontal = isHorizontal();

    const bool onScreen = panel->screen() < Kephal::ScreenUtils::numScreens();
    const QRect screenRect = onScreen ?  Kephal::ScreenUtils::screenGeometry(panel->screen()) : QRect();
    if (!onScreen) {
        resize(panel->size().toSize());
    }
    m_lastSeenSize = sizes.readEntry("lastsize", m_lastHorizontal ? screenRect.width() : screenRect.height());

    m_alignment = alignmentFilter((Qt::Alignment)viewConfig.readEntry("Alignment", (int)Qt::AlignLeft));
    m_offset = viewConfig.readEntry("Offset", 0);
    setVisibilityMode((VisibilityMode)viewConfig.readEntry("panelVisibility", (int)m_visibilityMode));

    connect(this, SIGNAL(sceneRectAboutToChange()), this, SLOT(pinchContainmentToCurrentScreen()));

    Kephal::Screens *screens = Kephal::Screens::self();
    connect(screens, SIGNAL(screenResized(Kephal::Screen *, QSize, QSize)),
            this, SLOT(pinchContainmentToCurrentScreen()));
    connect(screens, SIGNAL(screenMoved(Kephal::Screen *, QPoint, QPoint)),
            this, SLOT(updatePanelGeometry()));
    connect(screens, SIGNAL(screenAdded(Kephal::Screen *)),
            this, SLOT(updateStruts()));
}

PanelView::~PanelView()
{
    if (m_panelController) {
        disconnect(m_panelController, 0, this, 0);
        delete m_panelController;
    }

    delete m_glowBar;
    delete m_shadowWindow;
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
        disconnect(oldContainment, 0, this, 0);
    }

    connect(containment, SIGNAL(newStatus(Plasma::ItemStatus)), this, SLOT(setStatus(Plasma::ItemStatus)));
    connect(containment, SIGNAL(destroyed(QObject*)), this, SLOT(panelDeleted()));
    connect(containment, SIGNAL(toolBoxToggled()), this, SLOT(togglePanelController()));
    connect(containment, SIGNAL(appletAdded(Plasma::Applet *, const QPointF &)), this, SLOT(appletAdded(Plasma::Applet *)));
    connect(containment, SIGNAL(showAddWidgetsInterface(QPointF)), this, SLOT(showWidgetExplorer()));
    connect(containment, SIGNAL(screenChanged(int,int,Plasma::Containment*)), this, SLOT(pinchContainmentToCurrentScreen()));

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
    const QRect screenRect = Kephal::ScreenUtils::screenGeometry(containment->screen());
    const QRegion availGeom = PlasmaApp::self()->corona()->availableScreenRegion(containment->screen());
    const int w = containment->size().width();
    const int h = containment->size().height();
    const int length = containment->formFactor() == Plasma::Horizontal ? w : h;

    View::setContainment(containment);

    switch (location()) {
        case Plasma::LeftEdge: {
            QRect r = availGeom.intersected(QRect(0, m_offset, w, length)).boundingRect();
            if (m_offset != r.top()) {
                setOffset(r.top());
            }
        }
        break;

        case Plasma::RightEdge: {
            QRect r = availGeom.intersected(QRect(screenRect.right() - w, m_offset, w, length)).boundingRect();
            setOffset(r.top());
        }
        break;

        case Plasma::TopEdge: {
            QRect r = availGeom.intersected(QRect(m_offset, 0, length, h)).boundingRect();
            setOffset(r.left());
        }
        break;

        case Plasma::BottomEdge:
        default: {
            QRect r = availGeom.intersected(QRect(m_offset, screenRect.bottom() - h, length, h)).boundingRect();
            setOffset(r.left());
        }
        break;
    }

    // pinchContainment calls updatePanelGeometry for us
    pinchContainment(screenRect);
    m_lastMin = containment->minimumSize();
    m_lastMax = containment->maximumSize();

    kDebug() << "about to set the containment" << (QObject*)containment;

    updateStruts();

    checkShadow();
}

void PanelView::checkShadow()
{
    if (KWindowSystem::compositingActive() && containment()->property("shadowPath").isValid()) {
        if (!m_shadowWindow) {
            m_shadowWindow = new ShadowWindow(this);
            KWindowSystem::setOnAllDesktops(winId(), true);
        }
        KWindowSystem::setType(m_shadowWindow->winId(), NET::Dock);
        KWindowSystem::setState(m_shadowWindow->winId(), NET::KeepBelow);
        KWindowSystem::setOnAllDesktops(m_shadowWindow->winId(), true);
        m_shadowWindow->setSvg(containment()->property("shadowPath").toString());
        int left, right, top, bottom;
        m_shadowWindow->adjustMargins(geometry());
        m_shadowWindow->getContentsMargins(&left, &top, &right, &bottom);
        m_shadowWindow->setGeometry(geometry().adjusted(-left, -top, right, bottom));
        if (m_shadowWindow->isValid() && isVisible()) {
            m_shadowWindow->show();
        }
    } else {
        m_shadowWindow->deleteLater();
        m_shadowWindow = 0;
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
    const QRect screenRect = Kephal::ScreenUtils::screenGeometry(c->screen());
    pinchContainment(screenRect);
    KWindowSystem::setOnAllDesktops(winId(), true);
    //updatePanelGeometry();
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
        m_strutsTimer->stop();
        m_strutsTimer->start(STRUTSTIMERDELAY);
    } else {
        setGeometry(geom);
        if (m_shadowWindow) {
            int left, right, top, bottom;
            m_shadowWindow->adjustMargins(geometry());
            m_shadowWindow->getContentsMargins(&left, &top, &right, &bottom);
            m_shadowWindow->setGeometry(geometry().adjusted(-left, -top, right, bottom));
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
    const QRect screenRect = Kephal::ScreenUtils::screenGeometry(containment()->screen());
    pinchContainment(screenRect);
}

void PanelView::pinchContainment(const QRect &screenGeom)
{
    kDebug() << "**************************** pinching" << screenGeom << m_lastSeenSize;
    disconnect(this, SIGNAL(sceneRectAboutToChange()), this, SLOT(pinchContainmentToCurrentScreen()));
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
        configNeedsSaving();

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
                   (horizontal ? c->geometry().width() :
                                 c->geometry().height()) >= m_lastSeenSize) {
            // we are moving from a smaller space where we are 100% to a larger one
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
                m_offset = sw - int(min.width());
            }
        }

        // if the maximum-size is 0, set it to 100%
        if (max.width() <= 0) {
            c->setMaximumSize(sw, max.height());
            max = c->maximumSize();
        }

        if (m_offset + max.width() > sw) {
            //kDebug() << "max size is too wide!";
            if (max.width() > sw) {
                c->setMaximumSize(sw, max.height());
            } else {
                m_offset = sw - int(max.width());
            }
        }
    } else {
        if (m_offset + min.height() > sh) {
            //kDebug() << "min size is too tall!";
            if (min.height() > sh) {
                c->setMinimumSize(min.width(), sh);
            } else {
                m_offset = sh - int(min.height());
            }
        }

        // if the maximum-size is 0, set it to 100%
        if (max.height() <= 0) {
            c->setMaximumSize(max.width(), sh);
            max = c->maximumSize();
        }

        if (m_offset + max.height() > sh) {
            //kDebug() << "max size is too tall!";
            if (max.height() > sh) {
                c->setMaximumSize(max.width(), sh);
            } else {
                m_offset = sh - int(max.height());
            }
        }
    }

    // resize to max if for some reason the size is empty
    // otherwise its not possible to interact with the panel at all
    if (c->size().isEmpty()) {
        c->resize(max);
    }

    if (m_lastHorizontal != horizontal ||
        m_lastSeenSize != (horizontal ? sw : sh)) {
        m_lastHorizontal = horizontal;
        m_lastSeenSize = (horizontal ? sw : sh);
        sizes.writeEntry("lastsize", m_lastSeenSize);
        configNeedsSaving();
    }

    updatePanelGeometry();

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
    KConfigGroup viewConfig = config();
    viewConfig.writeEntry("Offset", m_offset);

    containment()->update();

    //TODO: do we ever need to worry about pinching here, or
    //      do we just assume that the offset is always < screenSize - containmentSize?
    updatePanelGeometry();
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
        connect(m_panelController, SIGNAL(alignmentChanged(Qt::Alignment)), this, SLOT(setAlignment(Qt::Alignment)));
        connect(m_panelController, SIGNAL(locationChanged(Plasma::Location)), this, SLOT(setLocation(Plasma::Location)));
        connect(m_panelController, SIGNAL(panelVisibilityModeChanged(PanelView::VisibilityMode)), this, SLOT(setVisibilityMode(PanelView::VisibilityMode)));

        if (dynamic_cast<QGraphicsLinearLayout*>(containment()->layout())) {
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

    if (!m_panelController->isVisible()) {
        m_editing = true;
        m_panelController->resize(m_panelController->sizeHint());
        m_panelController->move(m_panelController->positionForPanelGeometry(geometry()));
        Plasma::WindowEffects::slideWindow(m_panelController, location());
        kDebug() << "showing panel controller!" << m_panelController->geometry();
        m_panelController->show();
    } else {
        Plasma::WindowEffects::slideWindow(m_panelController, location());
        m_panelController->close();
        updateStruts();
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
        hideIfNotInUse();
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
        QRect thisScreen = Kephal::ScreenUtils::screenGeometry(containment()->screen());
        QRect wholeScreen = Kephal::ScreenUtils::desktopGeometry();

        //Extended struts against a screen edge near to another screen are really harmful, so windows maximized under the panel is a lesser pain
        //TODO: force "windows can cover" in those cases?
        foreach (Kephal::Screen *screen, Kephal::Screens::self()->screens()) {
            QRect otherScreen = screen->geom();
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

void PanelView::enterEvent(QEvent *event)
{
    // allow unhiding to happen again even if we were delay-unhidden
    m_delayedUnhideTs = QTime();
/*
// handy for debugging :)
    if (containment()) {
        kDebug() << sceneRect() << containment()->geometry();
    }
*/
    Plasma::View::enterEvent(event);
}

void PanelView::showWidgetExplorer()
{
    if (!containment()) {
        return;
    }

    if (!m_panelController) {
        PlasmaApp::self()->showWidgetExplorer(screen(), containment());
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

void PanelView::hideIfNotInUse()
{
    //kDebug() << m_delayedUnhideTs.elapsed() << geometry().contains(QCursor::pos()) << hasPopup();
    //TODO: is 5s too long? not long enough?
    if ((m_delayedUnhideTs.isNull() || m_delayedUnhideTs.elapsed() > 5000) &&
        !geometry().adjusted(-10, -10, 10, 10).contains(QCursor::pos()) && !hasPopup()) {
        startAutoHide();
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
    m_delayedUnhideTimer->stop();
    hideHinter();
    if (destroyTrigger) {
        destroyUnhideTrigger();
    }

    //ensure it's visible
    if (!isVisible()) {
        Plasma::WindowEffects::slideWindow(this, location());
        show();
        KWindowSystem::setOnAllDesktops(winId(), true);

        if (m_shadowWindow && m_shadowWindow->isValid()) {
            Plasma::WindowEffects::slideWindow(m_shadowWindow, location());
            m_shadowWindow->show();
            KWindowSystem::setState(m_shadowWindow->winId(), NET::KeepBelow);
            KWindowSystem::setOnAllDesktops(m_shadowWindow->winId(), true);
        }
    }

    //non-hiding panels stop here
    if (m_visibilityMode == NormalPanel || m_visibilityMode == WindowsGoBelow) {
        return;
    }

    //set up the re-hiding stuff
    if (!m_mousePollTimer) {
        m_mousePollTimer = new QTimer(this);
    }

    connect(m_mousePollTimer, SIGNAL(timeout()), this, SLOT(hideIfNotInUse()), Qt::UniqueConnection);
    m_mousePollTimer->start(200);

    //avoid hide-show loops
    if (m_visibilityMode == LetWindowsCover) {
        m_triggerEntered = true;
        KWindowSystem::raiseWindow(winId());
        QTimer::singleShot(0, this, SLOT(resetTriggerEnteredSuppression()));
    }
}

void PanelView::setStatus(Plasma::ItemStatus newStatus)
{
    if (newStatus == Plasma::AcceptingInputStatus) {
        KWindowSystem::forceActiveWindow(winId());
    }
}

void PanelView::checkUnhide(Plasma::ItemStatus newStatus)
{
    //kDebug() << "================= got a new status: " << newStatus << Plasma::ActiveStatus;

    if (newStatus > Plasma::ActiveStatus) {
        if (!m_delayedUnhideTimer->isActive()) {
            m_delayedUnhideTimer->start();
        }

    } else {
        m_delayedUnhideTimer->stop();
    }
}

void PanelView::delayedUnhide()
{
    m_delayedUnhideTs.start();
    //kDebug() << "starting at:" << m_delayedUnhideTs << m_delayedUnhideTs.elapsed();
    unhide(true);
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
    if (m_mousePollTimer) {
        m_mousePollTimer->stop();
        disconnect(m_mousePollTimer, SIGNAL(timeout()), this, SLOT(hideIfNotInUse()));
    }

    if (m_visibilityMode == LetWindowsCover) {
        KWindowSystem::lowerWindow(winId());
        createUnhideTrigger();
    } else {
        Plasma::WindowEffects::slideWindow(this, location());
        createUnhideTrigger();
        if (m_shadowWindow) {
            Plasma::WindowEffects::slideWindow(m_shadowWindow, location());
            m_shadowWindow->hide();
        }
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

        connect(m_mousePollTimer, SIGNAL(timeout()), this, SLOT(hideIfNotInUse()), Qt::UniqueConnection);
        m_mousePollTimer->start(200);
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
}

bool PanelView::event(QEvent *event)
{
    if (event->type() == QEvent::Paint) {
        QPainter p(this);
        p.setCompositionMode(QPainter::CompositionMode_Source);
        p.fillRect(rect(), Qt::transparent);
    }

    return Plasma::View::event(event);
}

void PanelView::appletAdded(Plasma::Applet *applet)
{
    if (m_panelController) {
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


/*
 *   Copyright (C) 2006 Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
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

#include "krunnerdialog.h"

#include <QPainter>
#include <QSvgRenderer>
#include <QResizeEvent>
#include <QMouseEvent>
#ifdef Q_WS_X11
#include <QX11Info>
#endif
#include <QBitmap>
#include <QTimer>

#include <KDebug>
#include <KWindowSystem>
#include <KPluginInfo>
#ifdef Q_WS_X11
#include <NETRootInfo>
#endif

#include <QtCore/QStringBuilder> // % operator for QString

#include "kworkspace/kdisplaymanager.h"

#include <Plasma/AbstractRunner>
#include <Plasma/FrameSvg>
#include <Plasma/RunnerManager>
#include <Plasma/Theme>
#include <Plasma/WindowEffects>

#include "configdialog.h"
#include "krunnerapp.h"
#include "krunnersettings.h"
#include "panelshadows.h"

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#endif

KRunnerDialog::KRunnerDialog(Plasma::RunnerManager *runnerManager, QWidget *parent, Qt::WindowFlags f)
    : QWidget(parent, f),
      m_runnerManager(runnerManager),
      m_configWidget(0),
      m_shadows(new PanelShadows(this)),
      m_background(new Plasma::FrameSvg(this)),
      m_shownOnScreen(-1),
      m_offset(.5),
      m_floating(!KRunnerSettings::freeFloating()),
      m_resizing(false),
      m_rightResize(false),
      m_vertResize(false),
      m_runningTimer(false)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setMouseTracking(true);
    //setButtons(0);
    setWindowTitle(i18n("Run Command"));
    setWindowIcon(KIcon(QLatin1String("system-run")));

    QPalette pal = palette();
    pal.setColor(backgroundRole(), Qt::transparent);
    setPalette(pal);

    m_iconSvg = new Plasma::Svg(this);
    m_iconSvg->setImagePath(QLatin1String("widgets/configuration-icons"));

    connect(m_background, SIGNAL(repaintNeeded()), this, SLOT(themeUpdated()));

    connect(Kephal::Screens::self(), SIGNAL(screenRemoved(int)),
            this, SLOT(screenRemoved(int)));
    connect(Kephal::Screens::self(), SIGNAL(screenResized(Kephal::Screen*,QSize,QSize)),
            this, SLOT(screenGeometryChanged(Kephal::Screen*)));
    connect(Kephal::Screens::self(), SIGNAL(screenMoved(Kephal::Screen*,QPoint,QPoint)),
            this, SLOT(screenGeometryChanged(Kephal::Screen*)));
    connect(KWindowSystem::self(), SIGNAL(workAreaChanged()), this, SLOT(resetScreenPos()));
    connect(KWindowSystem::self(), SIGNAL(compositingChanged(bool)), this, SLOT(compositingChanged(bool)));

    setFreeFloating(KRunnerSettings::freeFloating());
}

KRunnerDialog::~KRunnerDialog()
{
    //kDebug( )<< "!!!!!!!!!! deleting" << m_floating << m_screenPos.count();
    if (!m_floating) {
        KConfigGroup cg(KGlobal::config(), "EdgePositions");
        cg.writeEntry(QLatin1String("Offset"), m_offset);
    }
}

void KRunnerDialog::screenRemoved(int screen)
{
    if (isVisible() && m_shownOnScreen == screen) {
        positionOnScreen();
    }
}

void KRunnerDialog::screenGeometryChanged(Kephal::Screen* screen)
{
    if (isVisible() && screen->id() == m_shownOnScreen) {
        positionOnScreen();
    }
}

void KRunnerDialog::resetScreenPos()
{
    if (isVisible() && !m_floating) {
        positionOnScreen();
    }
}

void KRunnerDialog::positionOnScreen()
{
    if (Kephal::ScreenUtils::numScreens() < 2) {
        m_shownOnScreen = Kephal::ScreenUtils::primaryScreenId();
    } else if (isVisible()) {
        m_shownOnScreen = Kephal::ScreenUtils::screenId(geometry().center());
    } else {
        m_shownOnScreen = Kephal::ScreenUtils::screenId(QCursor::pos());
    }

    QRect r = Kephal::ScreenUtils::screenGeometry(m_shownOnScreen);

    if (m_floating && !m_customPos.isNull()) {
        int x = qBound(r.left(), m_customPos.x(), r.right() - width());
        int y = qBound(r.top(), m_customPos.y(), r.bottom() - height());
        move(x, y);
        show();
        return;
    }

    const int w = width();
    int x = r.left() + (r.width() * m_offset) - (w / 2);

    int y = r.top();
    if (m_floating) {
        y += r.height() / 3;
    }

    x = qBound(r.left(), x, r.right() - width());
    y = qBound(r.top(), y, r.bottom() - height());

    move(x, y);

    if (!m_floating) {
        checkBorders(r);
    }

    show();

    if (m_floating) {
        KWindowSystem::setOnDesktop(winId(), KWindowSystem::currentDesktop());
        //Turn the sliding effect off
        Plasma::WindowEffects::slideWindow(this, Plasma::Floating);
    } else {
        KWindowSystem::setOnAllDesktops(winId(), true);
        Plasma::WindowEffects::slideWindow(this, Plasma::TopEdge);
    }

    KWindowSystem::forceActiveWindow(winId());
    //kDebug() << "moving to" << m_screenPos[screen];
}

void KRunnerDialog::moveEvent(QMoveEvent *)
{
    if (m_floating) {
        m_customPos = pos();
    } else {
        const int screenWidth = Kephal::ScreenUtils::screenGeometry(m_shownOnScreen).width();
        m_offset = qRound(geometry().center().x() / qreal(screenWidth) * 100) / 100.0;
    }
}

void KRunnerDialog::setFreeFloating(bool floating)
{
    if (m_floating == floating) {
        return;
    }

    m_floating = floating;
    m_shownOnScreen = -1;
    unsetCursor();
    updatePresentation();
}

void KRunnerDialog::updatePresentation()
{
    if (m_floating) {
        KWindowSystem::setType(winId(), NET::Normal);

        //m_shadows->setImagePath(QLatin1String("dialogs/krunner"));
        m_background->setImagePath(QLatin1String("dialogs/krunner"));
        m_background->setElementPrefix(QString());

        themeUpdated();
    } else {
        //m_shadows->setImagePath(QLatin1String("widgets/panel-background"));
        m_background->setImagePath(QLatin1String("widgets/panel-background"));
        m_background->resizeFrame(size());
        m_background->setElementPrefix("north-mini");
        // load the positions for each screen from our config
        KConfigGroup cg(KGlobal::config(), "EdgePositions");
        m_offset = cg.readEntry(QLatin1String("Offset"), m_offset);
        QRect r = Kephal::ScreenUtils::screenGeometry(m_shownOnScreen);
        checkBorders(r);
        KWindowSystem::setType(winId(), NET::Dock);
    }

    if (isVisible()) {
        positionOnScreen();
    }
}

void KRunnerDialog::compositingChanged(bool)
{
    updatePresentation();
    updateMask();
    adjustSize();
}

bool KRunnerDialog::freeFloating() const
{
    return m_floating;
}

KRunnerDialog::ResizeMode KRunnerDialog::manualResizing() const
{
    if (!m_resizing) {
        return NotResizing;
    } else if (m_vertResize) {
        return VerticalResizing;
    } else {
        return HorizontalResizing;
    }
}

void KRunnerDialog::setStaticQueryMode(bool staticQuery)
{
    Q_UNUSED(staticQuery)
}

void KRunnerDialog::toggleConfigDialog()
{
    if (m_configWidget) {
        delete m_configWidget;
        m_configWidget = 0;

        if (!m_floating) {
            KWindowSystem::setType(winId(), NET::Dock);
        }
    } else {
        m_configWidget = new KRunnerConfigWidget(m_runnerManager, this);
        connect(m_configWidget, SIGNAL(finished()), this, SLOT(configCompleted()));
        setConfigWidget(m_configWidget);
        KWindowSystem::setType(winId(), NET::Normal);
    }
}

void KRunnerDialog::configCompleted()
{
    if (m_configWidget) {
        m_configWidget->deleteLater();
        m_configWidget = 0;
    }

    if (!m_floating) {
        KWindowSystem::setType(winId(), NET::Dock);
    }
}

void KRunnerDialog::themeUpdated()
{
    m_shadows->addWindow(this);

    bool useShadowsForMargins = false;
    if (m_floating) {
        // recalc the contents margins
        m_background->blockSignals(true);
        if (KWindowSystem::compositingActive()) {
            m_background->setEnabledBorders(Plasma::FrameSvg::NoBorder);
            useShadowsForMargins = true;
        } else {
            m_background->setEnabledBorders(Plasma::FrameSvg::AllBorders);
        }
        m_background->blockSignals(false);
    }

    if (useShadowsForMargins) {
        m_shadows->getMargins(m_topBorderHeight, m_rightBorderWidth, m_bottomBorderHeight, m_leftBorderWidth);
    } else {
        m_leftBorderWidth = qMax(0, int(m_background->marginSize(Plasma::LeftMargin)));
        m_rightBorderWidth = qMax(0, int(m_background->marginSize(Plasma::RightMargin)));
        m_bottomBorderHeight = qMax(0, int(m_background->marginSize(Plasma::BottomMargin)));
        // the -2 in the non-floating case is not optimal, but it gives it a bit of a "more snug to the
        // top" feel; best would be if we could tell exactly where the edge/shadow of the frame svg was
        // but this works nicely
        m_topBorderHeight = m_floating ? qMax(0, int(m_background->marginSize(Plasma::TopMargin)))
                                       : Plasma::Theme::defaultTheme()->windowTranslucencyEnabled()
                                            ? qMax(1, m_bottomBorderHeight / 2)
                                            : qMax(1, m_bottomBorderHeight - 2);
    }

    kDebug() << m_leftBorderWidth << m_topBorderHeight << m_rightBorderWidth << m_bottomBorderHeight;
    // the +1 gives us the extra mouseMoveEvent needed to always reset the resize cursor
    setContentsMargins(m_leftBorderWidth + 1, m_topBorderHeight, m_rightBorderWidth + 1, m_bottomBorderHeight + 1);

    update();
}

void KRunnerDialog::paintEvent(QPaintEvent *e)
{
    QPainter p(this);
    p.setCompositionMode(QPainter::CompositionMode_Source);
    p.setClipRect(e->rect());
    //kDebug() << "clip rect set to: " << e->rect();

    m_background->paintFrame(&p);
}

void KRunnerDialog::showEvent(QShowEvent *)
{
    unsigned long state = NET::SkipTaskbar | NET::KeepAbove | NET::StaysOnTop;
    if (m_floating) {
        KWindowSystem::clearState(winId(), state);
    } else {
        KWindowSystem::setState(winId(), state);
    }
    m_runnerManager->setupMatchSession();
}

void KRunnerDialog::hideEvent(QHideEvent *)
{
    // We delay the call to matchSessionComplete until next event cycle
    // This is necessary since we might hide the dialog right before running
    // a match, and the runner might still need to be prepped to
    // succesfully run a match
    QTimer::singleShot(0, m_runnerManager, SLOT(matchSessionComplete()));
    delete m_configWidget;
    m_configWidget = 0;
}

void KRunnerDialog::updateMask()
{
    // Enable the mask only when compositing is disabled;
    // As this operation is quite slow, it would be nice to find some
    // way to workaround it for no-compositing users.

    if (KWindowSystem::compositingActive()) {
        clearMask();
        const QRegion mask = m_background->mask();
        Plasma::WindowEffects::enableBlurBehind(winId(), true, mask);
        Plasma::WindowEffects::overrideShadow(winId(), true);
    } else {
        setMask(m_background->mask());
    }
}

void KRunnerDialog::resizeEvent(QResizeEvent *e)
{
    m_background->resizeFrame(e->size());

    bool maskDirty = true;
    if (m_resizing && !m_vertResize) {
        QRect r = Kephal::ScreenUtils::screenGeometry(m_shownOnScreen);
        //kDebug() << "if" << x() << ">" << r.left() << "&&" << r.right() << ">" << (x() + width());
        const Plasma::FrameSvg::EnabledBorders borders = m_background->enabledBorders();
        if (borders & Plasma::FrameSvg::LeftBorder) {
            const int dx = x() + (e->oldSize().width() - width()) / 2 ;
            const int dy = (m_floating ? pos().y() : r.top());
            move(qBound(r.left(), dx, r.right() - width() + 1), dy);
            maskDirty = m_floating || !checkBorders(r);
        }
    }

    if (maskDirty) {
        updateMask();
    }
}

void KRunnerDialog::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        m_lastPressPos = e->globalPos();

        const bool leftResize = e->x() < qMax(5, m_leftBorderWidth);
        m_rightResize = e->x() > width() - qMax(5, m_rightBorderWidth);
        m_vertResize = e->y() > height() - qMax(5, m_bottomBorderHeight);
        kWarning() << "right:" << m_rightResize << "left:" << leftResize << "vert:" << m_vertResize;
        if (m_rightResize || m_vertResize || leftResize) {
            // let's do a resize! :)
            grabMouse();
            m_resizing = true;
        } else if (m_floating) {
#ifdef Q_WS_X11
            m_lastPressPos = QPoint();
            // We have to release the mouse grab before initiating the move operation.
            // Ideally we would call releaseMouse() to do this, but when we only have an
            // implicit passive grab, Qt is unaware of it, and will refuse to release it.
            XUngrabPointer(x11Info().display(), CurrentTime);

            // Ask the window manager to start an interactive move operation.
            NETRootInfo rootInfo(x11Info().display(), NET::WMMoveResize);
            rootInfo.moveResizeRequest(winId(), e->globalX(), e->globalY(), NET::Move);
#endif
        }
        e->accept();
    }
}

void KRunnerDialog::mouseReleaseEvent(QMouseEvent *)
{
    if (!m_lastPressPos.isNull()) {
        releaseMouse();
        unsetCursor();
        m_lastPressPos = QPoint();
        m_resizing = false;
    }
}

bool KRunnerDialog::checkBorders(const QRect &screenGeom)
{
    Q_ASSERT(!m_floating);
    Plasma::FrameSvg::EnabledBorders borders = Plasma::FrameSvg::BottomBorder;

    if (x() > screenGeom.left()) {
        borders |= Plasma::FrameSvg::LeftBorder;
    }

    if (x() + width() < screenGeom.right()) {
        borders |= Plasma::FrameSvg::RightBorder;
    }

    if (borders != m_background->enabledBorders()) {
        m_background->setEnabledBorders(borders);
        themeUpdated();
        updateMask();
        update();
        return true;
    }

    return false;
}

void KRunnerDialog::leaveEvent(QEvent *)
{
    unsetCursor();
}

void KRunnerDialog::mouseMoveEvent(QMouseEvent *e)
{
    //kDebug() << e->x() << m_leftBorderWidth << width() << m_rightBorderWidth;
    if (m_lastPressPos.isNull()) {
        // not press positiong, so we aren't going to resize or move.
        checkCursor(e->pos());
    } else if (m_resizing) {
        // resizing
        if (m_vertResize) {
            const int deltaY = e->globalY() - m_lastPressPos.y();
            resize(width(), qMax(80, height() + deltaY));
            m_lastPressPos = e->globalPos();
        } else {
            QRect r = Kephal::ScreenUtils::screenGeometry(m_shownOnScreen);
            const int deltaX = (m_rightResize ? -1 : 1) * (m_lastPressPos.x() - e->globalX());
            int newWidth = width() + deltaX;

            // don't let it grow beyond the opposite screen edge
            if (m_rightResize) {
                if (m_leftBorderWidth > 0) {
                    newWidth += qMin(deltaX, x() - r.left());
                }
            } else if (m_rightBorderWidth > 0) {
                newWidth += qMin(deltaX, r.right() - (x() + width() - 1));
            } else if (newWidth > minimumWidth() && newWidth < width()) {
                move(r.right() - newWidth + 1, y());
            }

            if (newWidth > minimumWidth()) {
                resize(newWidth, height());
                m_lastPressPos = e->globalPos();
            }
        }
    } else {
        // moving
        QRect r = Kephal::ScreenUtils::screenGeometry(m_shownOnScreen);
        int newX = qBound(r.left(), x() - (m_lastPressPos.x() - e->globalX()), r.right() - width() + 1);
        if (abs(r.center().x() - (newX + (width() / 2))) < 20) {
            newX = r.center().x() - (width() / 2);
        } else {
            m_lastPressPos = e->globalPos();
        }

        move(newX, y());
        checkBorders(r);
    }
}

void KRunnerDialog::timerEvent(QTimerEvent *event)
{
    killTimer(event->timerId());
    if (checkCursor(mapFromGlobal(QCursor::pos()))) {
        m_runningTimer = true;
        startTimer(100);
    } else {
        m_runningTimer = false;
    }
}

bool KRunnerDialog::checkCursor(const QPoint &pos)
{
    //Plasma::FrameSvg borders = m_background->enabledBoders();
    if ((m_leftBorderWidth > 0 && pos.x() < qMax(5, m_leftBorderWidth)) ||
        (m_rightBorderWidth > 0 && pos.x() > width() - qMax(5, m_rightBorderWidth))) {
        if (cursor().shape() != Qt::SizeHorCursor) {
            setCursor(Qt::SizeHorCursor);
            if (!m_runningTimer) {
                m_runningTimer = true;
                startTimer(100);
            }
            return false;
        }

        return true;
    } else if ((pos.y() > height() - qMax(5, m_bottomBorderHeight)) && (pos.y() < height())) {
        if (cursor().shape() != Qt::SizeVerCursor) {
            setCursor(Qt::SizeVerCursor);
            if (!m_runningTimer) {
                m_runningTimer = true;
                startTimer(100);
            }
            return false;
        }

        return true;
    }

    unsetCursor();
    return false;
}

#include "krunnerdialog.moc"

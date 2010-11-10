/*
 *   Copyright 2007-2008 Aaron Seigo <aseigo@kde.org>
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

#include "netview.h"
#include "netcorona.h"
#include "netpanelcontroller.h"

#include <QAction>
#include <QPropertyAnimation>

#ifndef QT_NO_OPENGL
#include <QtOpenGL/QtOpenGL>
#endif

#include <KDebug>
#include <KWindowSystem>

#include <Plasma/Applet>
#include <Plasma/PopupApplet>
#include <Plasma/Corona>
#include <Plasma/Containment>
#include <Plasma/WindowEffects>

NetView::NetView(Plasma::Containment *containment, int uid, QWidget *parent)
    : Plasma::View(containment, uid, parent),
      m_panelController(0),
      m_configurationMode(false),
      m_useGL(false)
{
    setFocusPolicy(Qt::NoFocus);
    connectContainment(containment);

    connect(this, SIGNAL(lostContainment()), SLOT(grabContainment()));
    //setOptimizationFlags(QGraphicsView::DontSavePainterState);

    setAttribute(Qt::WA_TranslucentBackground, uid == controlBarId());

    m_containmentSwitchAnimation = new QPropertyAnimation(this, "sceneRect", this);
}

NetView::~NetView()
{
}

void NetView::setUseGL(const bool on)
{
#ifndef QT_NO_OPENGL
    if (on) {
      QGLWidget *glWidget = new QGLWidget;
      glWidget->setAutoFillBackground(false);
      setViewport(glWidget);
    } else {
      QWidget *widget = new QWidget;
      widget->setAutoFillBackground(false);
      setViewport(widget);
    }
#endif
    m_useGL = on;
}

bool NetView::useGL() const
{
    return m_useGL;
}

void NetView::connectContainment(Plasma::Containment *containment)
{
    if (!containment) {
        return;
    }

    connect(containment, SIGNAL(activate()), this, SIGNAL(containmentActivated()));
    connect(this, SIGNAL(sceneRectAboutToChange()), this, SLOT(updateGeometry()));
    connect(containment, SIGNAL(toolBoxVisibilityChanged(bool)), this, SLOT(updateConfigurationMode(bool)));
    connect(containment, SIGNAL(immutabilityChanged(ImmutabilityType)), this, SLOT(immutabilityChanged(ImmutabilityType)));

    QAction *a = containment->action("next containment");
    if (a) {
        connect(a, SIGNAL(triggered()), this, SLOT(nextContainment()));
    }
    a = containment->action("previous containment");
    if (a) {
        connect(a, SIGNAL(triggered()), this, SLOT(previousContainment()));
    }
}

void NetView::setContainment(Plasma::Containment *c)
{
    if (containment()) {
        disconnect(containment(), 0, this, 0);

        QAction *a = containment()->action("next containment");
        if (a) {
            disconnect(a, SIGNAL(triggered()), this, SLOT(nextContainment()));
        }
        a = containment()->action("previous containment");
        if (a) {
            disconnect(a, SIGNAL(triggered()), this, SLOT(previousContainment()));
        }
    }

    if (containment() && id() == mainViewId()) {
        setTrackContainmentChanges(false);
        Plasma::WindowEffects::enableBlurBehind(effectiveWinId(), false);
    } else if (containment() && id() == controlBarId()) {
        Plasma::WindowEffects::enableBlurBehind(effectiveWinId(), true);
    }

    Plasma::View::setContainment(c);
    connectContainment(c);
    updateGeometry();

    if (containment() && id() == mainViewId()) {
        if (c) {
            m_containmentSwitchAnimation->setDuration(250);
            m_containmentSwitchAnimation->setStartValue(sceneRect());
            m_containmentSwitchAnimation->setEndValue(c->geometry());
            m_containmentSwitchAnimation->start();
        }
        setTrackContainmentChanges(true);
    }
}

bool NetView::autoHide() const
{
    return config().readEntry("panelAutoHide", true);
}

void NetView::setAutoHide(bool hide)
{
    if (hide != autoHide()) {
        emit autoHideChanged(hide);
    }
    config().writeEntry("panelAutoHide", hide);
}

void NetView::immutabilityChanged(Plasma::ImmutabilityType immutability)
{
    if (m_configurationMode && immutability == Plasma::Mutable) {
        updateConfigurationMode(true);
    } else if (m_configurationMode) {
        updateConfigurationMode(false);
    }
}

// This function is reimplemented from QGraphicsView to work around the problem
// that QPainter::fillRect(QRectF/QRect, QBrush), which QGraphicsView uses, is
// potentially slow when the anti-aliasing hint is set and as implemented won't
// hit accelerated code at all when it isn't set.  This implementation avoids
// the problem by using integer coordinates and by using drawTiledPixmap() in
// the case of a texture brush, and fillRect(QRect, QColor) in the case of a
// solid pattern.  As an additional optimization it draws the background with
// CompositionMode_Source.
void NetView::drawBackground(QPainter *painter, const QRectF &rect)
{
    if (!testAttribute(Qt::WA_TranslucentBackground)) {
        painter->fillRect(rect.toAlignedRect(), Qt::black);
    } else {
        if (KWindowSystem::compositingActive()) {
            painter->setCompositionMode(QPainter::CompositionMode_Source);
            painter->fillRect(rect.toAlignedRect(), Qt::transparent);
        } else {
            Plasma::View::drawBackground(painter, rect);
        }
    }
}


void NetView::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event)
    updateGeometry();
    emit geometryChanged();
}

bool NetView::event(QEvent *e)
{
    if (e->type() == QEvent::Close) {
        //prevent ALT+F4 from killing the shell
        e->ignore();
        return true;
    } else {
        return Plasma::View::event(e);
    }
}

void NetView::screenOwnerChanged(int wasScreen, int isScreen, Plasma::Containment* containment)
{
    kDebug() << "was, is, containment:" << wasScreen << isScreen << (QObject*)containment;
    if (containment->containmentType() == Plasma::Containment::PanelContainment) {
        // we don't care about panel containments changing screens on us
        return;
    }

    if (wasScreen == screen() && this->containment() == containment) {
        setContainment(0);
    }

    if ((isScreen == screen() || screen() == -1) && this->containment( )!= containment) {
        setContainment(containment);
    }
}

Plasma::Location NetView::location() const
{
    return containment()->location();
}

Plasma::FormFactor NetView::formFactor() const
{
    return containment()->formFactor();
}

void NetView::updateGeometry()
{
    Plasma::Containment *c = containment();
    if (!c) {
        return;
    }

    kDebug() << "New containment geometry is" << c->geometry();

    switch (c->location()) {
    case Plasma::TopEdge:
    case Plasma::BottomEdge:
        setMinimumWidth(0);
        setMaximumWidth(QWIDGETSIZE_MAX);
        setFixedHeight(c->size().height());
        emit locationChanged(this);
        break;
    case Plasma::LeftEdge:
    case Plasma::RightEdge:
        setMinimumHeight(0);
        setMaximumHeight(QWIDGETSIZE_MAX);
        setFixedWidth(c->size().width());
        emit locationChanged(this);
        break;
    //ignore changes in the main view
    default:
        break;
    }

    if (c->size().toSize() != size()) {
        c->setMaximumSize(size());
        c->setMinimumSize(size());
        c->resize(size());
    }
}

void NetView::grabContainment()
{

    NetCorona *corona = qobject_cast<NetCorona*>(scene());
    if (!corona) {
        kDebug() << "no corona :(";
        return;
    }

    Plasma::Containment *cont = corona->findFreeContainment();
    if (cont) {
        cont->setScreen(screen(), desktop());
    }
}

void NetView::updateConfigurationMode(bool config)
{
    m_configurationMode = config;

    Plasma::Containment *cont = containment();
    if (config && cont && cont->immutability() == Plasma::Mutable &&
        (cont->location() != Plasma::Desktop && cont->location() != Plasma::Floating)) {
        m_panelController = new NetPanelController(0, this, cont);
    } else {
        delete m_panelController;
        m_panelController = 0;
    }
}

void NetView::nextContainment()
{
    QList<Plasma::Containment*> containments = containment()->corona()->containments();
    int start = containments.indexOf(containment());
    int i = (start + containments.size() - 1) % containments.size();
    Plasma::Containment *cont = containments.at(i);

    //FIXME this is a *horrible* way of choosing a "previous" containment.
    while (i != start) {
        if ((cont->location() == Plasma::Desktop || cont->location() == Plasma::Floating) &&
            cont->screen() == -1) {
            break;
        }

        if (--i < 0) {
            i += containments.size();
        }
        cont = containments.at(i);
    }

    cont->setScreen(screen(), desktop());
}

void NetView::previousContainment()
{
    QList<Plasma::Containment*> containments = containment()->corona()->containments();
    int start = containments.indexOf(containment());
    int i = (start + 1) % containments.size();
    Plasma::Containment *cont = containments.at(i);
    //FIXME this is a *horrible* way of choosing a "next" containment.
    while (i != start) {
        if ((cont->location() == Plasma::Desktop || cont->location() == Plasma::Floating) &&
            cont->screen() == -1) {
            break;
        }

        i = (i + 1) % containments.size();
        cont = containments.at(i);
    }

    cont->setScreen(screen(), desktop());
}

#include "netview.moc"


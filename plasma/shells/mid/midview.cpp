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

#include "midview.h"

#include <QAction>
#include <QDesktopWidget>
#include <QFile>
#include <QWheelEvent>
#include <QCoreApplication>

#include <KAuthorized>
#include <KMenu>
#include <KRun>
#include <KToggleAction>
#include <KWindowSystem>

#include "plasma/applet.h"
#include "plasma/corona.h"
#include "plasma/containment.h"

#include "plasmaapp.h"

static const int MIDVIEWID = 1;

MidView::MidView(Plasma::Containment *containment, QWidget *parent)
    : Plasma::View(containment, MIDVIEWID, parent)
{
    setFocusPolicy(Qt::NoFocus);

    if (containment) {
        connectContainment(containment);
        containment->enableAction("add sibling containment", false);
    }

    //FIXME should we have next/prev or up/down/left/right or what?
    QAction *action = new QAction(i18n("Next Activity"), this);
    action->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    action->setShortcut(QKeySequence("ctrl+shift+n"));
    connect(action, SIGNAL(triggered()), this, SLOT(nextContainment()));
    addAction(action);
    action = new QAction(i18n("Previous Activity"), this);
    action->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    action->setShortcut(QKeySequence("ctrl+shift+p"));
    connect(action, SIGNAL(triggered()), this, SLOT(previousContainment()));
    addAction(action);

    const int w = 25;
    QPixmap tile(w * 2, w * 2);
    tile.fill(palette().base().color());
    QPainter pt(&tile);
    QColor color = palette().mid().color();
    color.setAlphaF(.6);
    pt.fillRect(0, 0, w, w, color);
    pt.fillRect(w, w, w, w, color);
    pt.end();
    QBrush b(tile);
    setBackgroundBrush(tile);
}

MidView::~MidView()
{
}

void MidView::connectContainment(Plasma::Containment *containment)
{
    if (containment) {
        connect(containment, SIGNAL(showAddWidgetsInterface(QPointF)), this, SLOT(showAppletBrowser()));
        connect(containment, SIGNAL(focusRequested(Plasma::Containment *)), this, SLOT(setContainment(Plasma::Containment *)));
        connect(containment, SIGNAL(configureRequested()), this, SLOT(configureContainment()));
    }
}

void MidView::adjustSize()
{
    // adapt to screen resolution changes
    QDesktopWidget *desktop = QApplication::desktop();
    QRect geom = desktop->screenGeometry(screen());
    setGeometry(geom);
    containment()->resize(geom.size());
}

void MidView::setIsDesktop(bool isDesktop)
{
    if (isDesktop) {
#ifdef Q_WS_WIN
        setWindowFlags(Qt::FramelessWindowHint);
        SetWindowPos(winId(), HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        HWND hwndDesktop = ::FindWindowW(L"Progman", NULL);
        SetParent(winId(),hwndDesktop);
#else
        setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
#endif

        KWindowSystem::setOnAllDesktops(winId(), true);
        KWindowSystem::setType(winId(), NET::Desktop);
        lower();

        adjustSize();
    } else {
        setWindowFlags(windowFlags() & ~Qt::FramelessWindowHint);

        KWindowSystem::setOnAllDesktops(winId(), false);
        KWindowSystem::setType(winId(), NET::Normal);
    }
}

bool MidView::isDesktop() const
{
#ifndef Q_WS_WIN
    return KWindowInfo(winId(), NET::WMWindowType).windowType(NET::Desktop);
#else
    return true;
#endif
}

void MidView::configureContainment()
{
    /* TODO: implement; suggestion: as an overlay that takes the whole screen
    m_configDialog->show();
    KWindowSystem::setOnDesktop(m_configDialog->winId(), KWindowSystem::currentDesktop());
    KWindowSystem::activateWindow(m_configDialog->winId());
    */
}

void MidView::showAppletBrowser()
{
    //TODO: implement
}

// This function is reimplemented from QGraphicsView to work around the problem
// that QPainter::fillRect(QRectF/QRect, QBrush), which QGraphicsView uses, is
// potentially slow when the anti-aliasing hint is set and as implemented won't
// hit accelerated code at all when it isn't set.  This implementation avoids
// the problem by using integer coordinates and by using drawTiledPixmap() in
// the case of a texture brush, and fillRect(QRect, QColor) in the case of a
// solid pattern.  As an additional optimization it draws the background with
// CompositionMode_Source.
void MidView::drawBackground(QPainter *painter, const QRectF &rect)
{
    const QPainter::CompositionMode savedMode = painter->compositionMode();
    const QBrush brush = backgroundBrush();

    switch (brush.style())
    {
    case Qt::TexturePattern:
    {
        // Note: this assumes that the brush origin is (0, 0), and that
        //       the brush has an identity transformation matrix.
        const QPixmap texture = brush.texture();
        QRect r = rect.toAlignedRect();
        r.setLeft(r.left() - (r.left() % texture.width()));
        r.setTop(r.top() - (r.top() % texture.height()));
        painter->setCompositionMode(QPainter::CompositionMode_Source);
        painter->drawTiledPixmap(r, texture);
        painter->setCompositionMode(savedMode);
        return;
    }

    case Qt::SolidPattern:
        painter->setCompositionMode(QPainter::CompositionMode_Source);
        painter->fillRect(rect.toAlignedRect(), brush.color());
        painter->setCompositionMode(savedMode);
        return;

    default:
        QGraphicsView::drawBackground(painter, rect);
    }
}

void MidView::screenOwnerChanged(int wasScreen, int isScreen, Plasma::Containment* containment)
{
    kDebug() << "was, is, containment:" << wasScreen << isScreen << (QObject*)containment;
    if (containment->containmentType() == Plasma::Containment::PanelContainment) {
        // we don't care about panel containments changing screens on us
        return;
    }

    if (wasScreen == screen()) {
        if (this->containment() == containment) {
            setContainment(0);
        }
    }

    if (isScreen == screen()) {
        setContainment(containment);
    }
}

void MidView::nextContainment()
{
    QList<Plasma::Containment*> containments = containment()->corona()->containments();
    int start = containments.indexOf(containment());
    int i = (start + 1) % containments.size();
    //FIXME this is a *horrible* way of choosing a "next" containment.
    while (i != start) {
        if (containments.at(i)->containmentType() != Plasma::Containment::PanelContainment &&
            containments.at(i)->screen() == -1) {
            break;
        }
        i = (i + 1) % containments.size();
    }

    Plasma::Containment *c = containments.at(i);
    setContainment(c);
}

void MidView::previousContainment()
{
    QList<Plasma::Containment*> containments = containment()->corona()->containments();
    int start = containments.indexOf(containment());
    //fun fact: in c++, (-1 % foo) == -1
    int i = start - 1;
    if (i < 0) {
        i += containments.size();
    }
    //FIXME this is a *horrible* way of choosing a "previous" containment.
    while (i != start) {
        if (containments.at(i)->containmentType() != Plasma::Containment::PanelContainment &&
            containments.at(i)->screen() == -1) {
            break;
        }
        if (--i < 0) {
            i += containments.size();
        }
    }

    Plasma::Containment *c = containments.at(i);
    setContainment(c);
}

#include "midview.moc"


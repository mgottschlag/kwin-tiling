/*
*   Copyright 2011 by Aaron Seigo <aseigo@kde.org>
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

#include "panelshadows.h"

#ifdef Q_WS_X11
#include <QX11Info>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <fixx11h.h>
#endif

PanelShadows::PanelShadows(QObject *parent)
    : Plasma::Svg(parent)
{
    setImagePath("widgets/panel-background");
    connect(this, SIGNAL(repaintNeeded()), this, SLOT(updateShadows()));
}

void PanelShadows::addWinId(WId id)
{
    m_wids << id;
    updateShadows(id);
}

void PanelShadows::removeWinId(WId id)
{
    m_wids.remove(id);
}

void PanelShadows::updateShadows()
{
    setupPixmaps();
    foreach (const WId &wid, m_wids) {
        updateShadows(wid);
    }
}

void PanelShadows::setupPixmaps()
{
    m_shadowPixmaps.clear();
    m_shadowPixmaps << pixmap("shadow-top");
    m_shadowPixmaps << pixmap("shadow-topright");
    m_shadowPixmaps << pixmap("shadow-right");
    m_shadowPixmaps << pixmap("shadow-bottomright");
    m_shadowPixmaps << pixmap("shadow-bottom");
    m_shadowPixmaps << pixmap("shadow-bottomleft");
    m_shadowPixmaps << pixmap("shadow-left");
    m_shadowPixmaps << pixmap("shadow-topleft");
}

void PanelShadows::updateShadows(WId wid)
{
#ifdef Q_WS_X11
    if (m_shadowPixmaps.isEmpty()) {
        setupPixmaps();
    }

    QVector<unsigned long> data;
    foreach (const QPixmap &pixmap, m_shadowPixmaps) {
        data.push_back(pixmap.handle());
    }

    data << m_shadowPixmaps[0].height(); // top
    data << m_shadowPixmaps[2].width(); // right
    data << m_shadowPixmaps[4].height(); // bottom
    data << m_shadowPixmaps[6].width(); // left

    Display *dpy = QX11Info::display();
    Atom atom = XInternAtom(dpy, "_KDE_NET_WM_SHADOW", False);

    //kDebug() << "going to set the shadow of" << winId() << "to" << data;
        XChangeProperty(dpy, wid, atom, XA_CARDINAL, 32, PropModeReplace,
                        reinterpret_cast<const unsigned char *>(data.constData()), data.size());
#endif
}

#include "panelshadows.moc"


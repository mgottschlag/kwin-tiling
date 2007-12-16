/*  
    Copyright 2007 Robert Knight <robertknight@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

// Own
#include "applet/applet.h"

// Qt
#include <QApplication>
#include <QDesktopWidget>
#include <QGraphicsView>
#include <QtDebug>

// KDE
#include <KIcon>

// Plasma
#include <plasma/layouts/boxlayout.h>
#include <plasma/widgets/icon.h>
#include <plasma/containment.h>

// Local
#include "ui/launcher.h"

LauncherApplet::LauncherApplet(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent,args),
      m_launcher(0)
{
//    setDrawStandardBackground(true);
    Plasma::HBoxLayout *layout = new Plasma::HBoxLayout(this);
    layout->setMargin(0);
    m_icon = new Plasma::Icon(KIcon("start-here"), QString(), this);
    m_icon->setFlag(ItemIsMovable, false);
    connect(m_icon, SIGNAL(pressed(bool)), this, SLOT(toggleMenu(bool)));
}

LauncherApplet::~LauncherApplet()
{
    delete m_launcher;
}

QSizeF LauncherApplet::contentSizeHint() const
{
    return QSizeF(48,48);
}

Qt::Orientations LauncherApplet::expandingDirections() const
{
    return 0;
}

void LauncherApplet::toggleMenu(bool pressed)
{
    if (!pressed) {
        return;
    }

    //qDebug() << "Launcher button clicked";
    if (!m_launcher) {
        m_launcher = new Kickoff::Launcher(0);
        m_launcher->setWindowFlags(m_launcher->windowFlags()|Qt::WindowStaysOnTopHint|Qt::Popup);
        m_launcher->setAutoHide(true);
        m_launcher->adjustSize();
        connect(m_launcher, SIGNAL(aboutToHide()), m_icon, SLOT(setUnpressed()));
    }

    // try to position the launcher alongside the top or bottom edge of the
    // applet with and aligned to the left or right of the applet
    if (!m_launcher->isVisible()) {
        QGraphicsView *viewWidget = view();
        QDesktopWidget *desktop = QApplication::desktop();
        if (viewWidget) {
            QPoint viewPos = viewWidget->mapFromScene(scenePos());
            QPoint globalPos = viewWidget->mapToGlobal(viewPos);
            QRect desktopRect = desktop->availableGeometry(viewWidget);
            QRect size = mapToView(viewWidget, contentRect());
            // Prefer to open below the icon so as to act like a regular menu
            if (globalPos.y() + size.height() + m_launcher->height()
                < desktopRect.bottom()) {
                globalPos.ry() += size.height();
            } else {
                globalPos.ry() -= m_launcher->height();
            }
            if (globalPos.x() + m_launcher->width() > desktopRect.right()) {
                globalPos.rx() -= m_launcher->width() - size.width();
            }
            m_launcher->move(globalPos);
        }
        if (containment()) {
            containment()->emitLaunchActivated();
        }
    }

    m_launcher->setVisible(!m_launcher->isVisible());
    m_icon->setPressed();
}

#include "applet.moc"

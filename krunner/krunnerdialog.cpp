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
#include <QX11Info>
#include <QBitmap>

#include <KDebug>
#include <NETRootInfo>

#include <Plasma/PanelSvg>
#include <Plasma/Theme>

#include "krunnerapp.h"

#include <X11/Xlib.h>

KRunnerDialog::KRunnerDialog( QWidget * parent, Qt::WindowFlags f )
    : KDialog(parent, f | Qt::WindowStaysOnTopHint)
{
    setButtons(0);
    m_background = new Plasma::PanelSvg(this);
    m_background->setImagePath("dialogs/krunner");
    m_background->setEnabledBorders(Plasma::PanelSvg::AllBorders);

    connect(m_background, SIGNAL(repaintNeeded()), this, SLOT(update()));

    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), this, SLOT(themeUpdated()));
    themeUpdated();
}

KRunnerDialog::~KRunnerDialog()
{
}

void KRunnerDialog::themeUpdated()
{
    const int topHeight = m_background->marginSize(Plasma::TopMargin);
    const int leftWidth = m_background->marginSize(Plasma::LeftMargin);
    const int rightWidth = m_background->marginSize(Plasma::RightMargin);
    const int bottomHeight = m_background->marginSize(Plasma::BottomMargin);
    setContentsMargins(leftWidth, topHeight, rightWidth, bottomHeight);
}

void KRunnerDialog::paintEvent(QPaintEvent *e)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setClipRect(e->rect());
    //kDebug() << "clip rect set to: " << e->rect();

    if (KRunnerApp::self()->hasCompositeManager()) {
        //kDebug() << "gots us a compmgr!";
        p.setCompositionMode(QPainter::CompositionMode_Source );
        p.fillRect(rect(), Qt::transparent);
    }

    m_background->paint(&p, e->rect());
}

void KRunnerDialog::resizeEvent(QResizeEvent *e)
{
    m_background->resize(e->size());
    setMask(m_background->mask());
    KDialog::resizeEvent(e);
}

void KRunnerDialog::mousePressEvent(QMouseEvent *e)
{
    // We have to release the mouse grab before initiating the move operation.
    // Ideally we would call releaseMouse() to do this, but when we only have an
    // implicit passive grab, Qt is unaware of it, and will refuse to release it.
    XUngrabPointer(x11Info().display(), CurrentTime);

    // Ask the window manager to start an interactive move operation.
    NETRootInfo rootInfo(x11Info().display(), NET::WMMoveResize);
    rootInfo.moveResizeRequest(winId(), e->globalX(), e->globalY(), NET::Move);

    e->accept();
}

#include "krunnerdialog.moc"

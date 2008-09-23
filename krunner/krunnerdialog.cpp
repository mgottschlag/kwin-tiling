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

#include <KDebug>
#include <KWindowSystem>
#ifdef Q_WS_X11
#include <NETRootInfo>
#endif

#include "plasma/panelsvg.h"
#include "plasma/runnermanager.h"
#include "plasma/theme.h"

#include "configdialog.h"
#include "krunnerapp.h"

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#endif

KRunnerDialog::KRunnerDialog(Plasma::RunnerManager *runnerManager, QWidget *parent, Qt::WindowFlags f )
    : KDialog(parent, f),
      m_configDialog(0),
      m_runnerManager(runnerManager)
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

void KRunnerDialog::showConfigDialog()
{
    if (!m_configDialog) {
        m_configDialog = new KRunnerConfigDialog(m_runnerManager);
        connect(m_configDialog, SIGNAL(finished()), this, SLOT(configCompleted()));
    }

    KWindowSystem::setOnDesktop(m_configDialog->winId(), KWindowSystem::currentDesktop());
    KWindowSystem::activateWindow(m_configDialog->winId());
    m_configDialog->show();
}

void KRunnerDialog::configCompleted()
{
    m_configDialog->deleteLater();
    m_configDialog = 0;
}

void KRunnerDialog::themeUpdated()
{
    int margin = marginHint();
    const int topHeight = qMax(0, int(m_background->marginSize(Plasma::TopMargin)) - margin);
    const int leftWidth = qMax(0, int(m_background->marginSize(Plasma::LeftMargin)) - margin);
    const int rightWidth = qMax(0, int(m_background->marginSize(Plasma::RightMargin)) - margin);
    const int bottomHeight = qMax(0, int(m_background->marginSize(Plasma::BottomMargin)) - margin);

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

    m_background->paintPanel(&p);
}

void KRunnerDialog::resizeEvent(QResizeEvent *e)
{
    m_background->resizePanel(e->size());
    setMask(m_background->mask());
    KDialog::resizeEvent(e);
}

void KRunnerDialog::mousePressEvent(QMouseEvent *e)
{
#ifdef Q_WS_X11
    // We have to release the mouse grab before initiating the move operation.
    // Ideally we would call releaseMouse() to do this, but when we only have an
    // implicit passive grab, Qt is unaware of it, and will refuse to release it.
    XUngrabPointer(x11Info().display(), CurrentTime);

    // Ask the window manager to start an interactive move operation.
    NETRootInfo rootInfo(x11Info().display(), NET::WMMoveResize);
    rootInfo.moveResizeRequest(winId(), e->globalX(), e->globalY(), NET::Move);

    e->accept();
#endif    
}

#include "krunnerdialog.moc"

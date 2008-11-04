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
#include <KPluginInfo>
#ifdef Q_WS_X11
#include <NETRootInfo>
#endif

#include "kworkspace/kdisplaymanager.h"

#include "plasma/framesvg.h"
#include "plasma/runnermanager.h"
#include "plasma/theme.h"

#include "configdialog.h"
#include "krunnerapp.h"

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#endif

KRunnerDialog::KRunnerDialog(Plasma::RunnerManager *runnerManager, QWidget *parent, Qt::WindowFlags f )
    : KDialog(parent, f),
      m_runnerManager(runnerManager),
      m_configDialog(0)
{
    setButtons(0);
    setWindowTitle( i18n("Run Command") );
    setWindowIcon(KIcon("system-run"));

    m_background = new Plasma::FrameSvg(this);
    m_background->setImagePath("dialogs/krunner");
    m_background->setEnabledBorders(Plasma::FrameSvg::AllBorders);

    m_iconSvg = new Plasma::Svg(this);
    m_iconSvg->setImagePath("widgets/configuration-icons");
    m_iconSvg->setContainsMultipleImages(true);
    m_iconSvg->resize(KIconLoader::SizeSmall, KIconLoader::SizeSmall);

    connect(m_background, SIGNAL(repaintNeeded()), this, SLOT(update()));

    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), this, SLOT(themeUpdated()));
    themeUpdated();
}

KRunnerDialog::~KRunnerDialog()
{
}

void KRunnerDialog::setStaticQueryMode(bool staticQuery)
{
    Q_UNUSED(staticQuery)
}

void KRunnerDialog::switchUser()
{
    KService::Ptr service = KService::serviceByStorageId("plasma-runner-sessions.desktop");
    KPluginInfo info(service);

    if (info.isValid()) {
        SessList sessions;
        KDisplayManager dm;
        dm.localSessions(sessions);

        if (sessions.isEmpty()) {
            // no sessions to switch between, let's just start up another session directly
            Plasma::AbstractRunner *sessionRunner = m_runnerManager->runner(info.pluginName());
            if (sessionRunner) {
                Plasma::QueryMatch switcher(sessionRunner);
                sessionRunner->run(*m_runnerManager->searchContext(), switcher);
            }
        } else {
            display(QString());
            //TODO: create a "single runner" mode
            //m_header->setText(i18n("Switch users"));
            //m_header->setPixmap("system-switch-user");

            //TODO: ugh, magic strings. See sessions/sessionrunner.cpp
            setStaticQueryMode(true);
            m_runnerManager->launchQuery("SESSIONS", info.pluginName());
        }
    }
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

    m_background->paintFrame(&p);
}

bool KRunnerDialog::event(QEvent *event)
{
    if (event->type() == QEvent::Paint) {
        QPainter p(this);
        p.setCompositionMode(QPainter::CompositionMode_Source);
        p.fillRect(rect(), Qt::transparent);
    }

    return KDialog::event(event);
}

void KRunnerDialog::resizeEvent(QResizeEvent *e)
{
    m_background->resizeFrame(e->size());
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

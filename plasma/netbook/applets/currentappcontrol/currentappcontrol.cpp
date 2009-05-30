/***************************************************************************
 *                                                                         *
 *   Copyright (C) 2009 Marco Martin <notmart@gmail.com>                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "currentappcontrol.h"

//Qt
#include <QGraphicsLinearLayout>
#include <QX11Info>

//KDE
#include <kwindowsystem.h>
#include <kiconloader.h>
#include <netwm.h>

//Plasma
#include <Plasma/IconWidget>
#include <Plasma/View>

//X
#ifdef Q_WS_X11
#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>
#include <X11/keysym.h>
#endif

CurrentAppControl::CurrentAppControl(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args)
{
    m_currentTask = new Plasma::IconWidget(this);
    m_currentTask->setOrientation(Qt::Horizontal);
    m_closeTask = new Plasma::IconWidget(this);
    m_closeTask->setSvg("widgets/configuration-icons", "close");
    m_closeTask->setMaximumWidth(KIconLoader::SizeSmallMedium);

    connect(m_closeTask, SIGNAL(clicked()), this, SLOT(closeWindow()));
    connect(m_currentTask, SIGNAL(clicked()), this, SLOT(listWindows()));
}


CurrentAppControl::~CurrentAppControl()
{

}

void CurrentAppControl::init()
{
    connect(KWindowSystem::self(), SIGNAL(activeWindowChanged(WId)),
            this, SLOT(activeWindowChanged(WId)));
    QGraphicsLinearLayout *lay = new QGraphicsLinearLayout(Qt::Horizontal, this);
    lay->addItem(m_currentTask);
    lay->addItem(m_closeTask);
    activeWindowChanged(KWindowSystem::activeWindow());
}

void CurrentAppControl::activeWindowChanged(WId id)
{
    if (id == view()->effectiveWinId()) {
        m_activeWindow = 0;
        m_currentTask->setIcon("preferences-system-windows");
        m_currentTask->setText(i18np("%1 running app", "%1 running apps", KWindowSystem::windows().count()-1));
        m_closeTask->setEnabled("false");
    } else {
        m_activeWindow = id;
        KWindowInfo info = KWindowSystem::windowInfo(m_activeWindow, NET::WMName);
        m_currentTask->setIcon(KWindowSystem::icon(m_activeWindow, KIconLoader::SizeSmallMedium, KIconLoader::SizeSmallMedium));
        m_currentTask->setText(info.name());
    }
}

void CurrentAppControl::closeWindow()
{
    if (m_activeWindow) {
        NETRootInfo ri( QX11Info::display(), NET::CloseWindow );
        ri.closeWindowRequest(m_activeWindow);
    }
}

void CurrentAppControl::listWindows()
{
    //FIXME: there has to be a better way to trigger a kwin effect than faking ctrl+f9 :p

    XTestFakeKeyEvent(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XK_Control_L), 1, 0);
    XTestFakeKeyEvent(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XK_F9), 1, 0);

    XTestFakeKeyEvent(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XK_Control_L), 0, 0);
    XTestFakeKeyEvent(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XK_F9), 0, 0);
}

#include "currentappcontrol.moc"

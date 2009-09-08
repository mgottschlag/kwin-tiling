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
#include <QTimer>
#include <QFontMetrics>
#include <QVarLengthArray>

//KDE
#include <kwindowsystem.h>
#include <kiconloader.h>
#include <netwm.h>

//Plasma
#include <Plasma/IconWidget>
#include <Plasma/View>
#include <Plasma/Theme>

//X
#ifdef Q_WS_X11
#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>
#include <X11/keysym.h>
#endif

CurrentAppControl::CurrentAppControl(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args),
      m_syncDelay(false),
      m_activeWindow(0),
      m_pendingActiveWindow(0)
{
    m_currentTask = new Plasma::IconWidget(this);
    m_currentTask->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_currentTask->setTextBackgroundColor(QColor());
    m_closeTask = new Plasma::IconWidget(this);
    m_closeTask->setSvg("widgets/configuration-icons", "close");
    m_closeTask->setMaximumWidth(KIconLoader::SizeSmallMedium);

    connect(m_closeTask, SIGNAL(clicked()), this, SLOT(closeWindow()));
    connect(m_closeTask, SIGNAL(pressed(bool)), this, SLOT(setSyncDelay(bool)));
    connect(m_currentTask, SIGNAL(clicked()), this, SLOT(listWindows()));
}


CurrentAppControl::~CurrentAppControl()
{
}

void CurrentAppControl::init()
{
    connect(KWindowSystem::self(), SIGNAL(activeWindowChanged(WId)),
            this, SLOT(activeWindowChanged(WId)));
    connect(KWindowSystem::self(), SIGNAL(windowChanged(WId)),
            this, SLOT(windowChanged(WId)));
    QGraphicsLinearLayout *lay = new QGraphicsLinearLayout(Qt::Horizontal, this);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(0);
    lay->addItem(m_currentTask);
    lay->addItem(m_closeTask);
    activeWindowChanged(KWindowSystem::activeWindow());
}

void CurrentAppControl::constraintsEvent(Plasma::Constraints constraints)
{
    if (constraints && Plasma::FormFactorConstraint) {
        QFontMetrics fm(Plasma::Theme::defaultTheme()->font(Plasma::Theme::DefaultFont));
        if (formFactor() == Plasma::Vertical) {
            m_currentTask->setOrientation(Qt::Vertical);
            //FIXME: all this minimum/maximum sizes shouldn't be necessary
            m_currentTask->setMaximumSize(size().width(), QWIDGETSIZE_MAX);
            setMinimumSize(0, KIconLoader::SizeSmallMedium*2 + fm.xHeight()*20);
        } else {
            m_currentTask->setOrientation(Qt::Horizontal);
            m_currentTask->setMaximumSize(QWIDGETSIZE_MAX, size().height());
            setMinimumSize(KIconLoader::SizeSmallMedium*2 + fm.width('M')*20, 0);
        }
    }
}

void CurrentAppControl::windowChanged(WId id)
{
    if (id == m_activeWindow) {
        m_pendingActiveWindow = m_activeWindow;
        syncActiveWindow();
    }
}

void CurrentAppControl::activeWindowChanged(WId id)
{
    m_pendingActiveWindow = id;
    //delay the switch to permit to pass the close action to the proper window if our view accepts focus
    //QTimer::singleShot(100, this, SLOT(syncActiveWindow()));
    if (!m_syncDelay) {
        syncActiveWindow();
    }
}

void CurrentAppControl::syncActiveWindow()
{
    m_syncDelay = false;
    bool applicationActive = false;

    foreach (QWidget *widget, QApplication::topLevelWidgets()) {
         if (widget->winId() == m_pendingActiveWindow) {
             applicationActive = true;
             break;
         }
     }

    if (m_pendingActiveWindow <= 0 || applicationActive) {
        m_activeWindow = 0;
        m_currentTask->setIcon("preferences-system-windows");
        m_currentTask->setText(i18np("%1 running app", "%1 running apps", KWindowSystem::windows().count()-1));
        m_closeTask->hide();
    } else {
        m_activeWindow = m_pendingActiveWindow;
        KWindowInfo info = KWindowSystem::windowInfo(m_activeWindow, NET::WMName);
        m_currentTask->setIcon(KWindowSystem::icon(m_activeWindow, KIconLoader::SizeSmallMedium, KIconLoader::SizeSmallMedium));
        m_currentTask->setText(info.name());
        //FIXME: this is utterly bad: the layout seems to -not- resize it?
        m_currentTask->resize(size().width() - m_closeTask->size().width(), m_currentTask->size().height());
        m_closeTask->show();
    }

    m_pendingActiveWindow = 0;
}

void CurrentAppControl::setSyncDelay(bool delay)
{
    m_syncDelay = delay;
}

void CurrentAppControl::closeWindow()
{
    m_syncDelay = false;

    if (m_activeWindow) {
#ifdef Q_WS_X11
        NETRootInfo ri( QX11Info::display(), NET::CloseWindow );
        ri.closeWindowRequest(m_activeWindow);
#endif
    }

    syncActiveWindow();
}

void CurrentAppControl::listWindows()
{
#ifdef Q_WS_X11
    QVarLengthArray<long, 32> data(1);
    data[0] = KWindowSystem::currentDesktop();
    Display *dpy = QX11Info::display();
    const WId winId = view()->winId();
    Atom atom = XInternAtom(dpy, "_KDE_PRESENT_WINDOWS_DESKTOP", False);
    XChangeProperty(dpy, winId, atom, atom, 32, PropModeReplace,
                     reinterpret_cast<unsigned char *>(data.data()), data.size());
#endif
}

#include "currentappcontrol.moc"

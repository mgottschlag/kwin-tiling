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
#include <Plasma/Dialog>
#include <Plasma/Containment>
#include <Plasma/Corona>
#include <Plasma/WindowEffects>

//X
#ifdef Q_WS_X11
#include <X11/extensions/XTest.h>
#include <X11/keysym.h>
#endif

CurrentAppControl::CurrentAppControl(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args),
      m_syncDelay(false),
      m_activeWindow(0),
      m_pendingActiveWindow(0),
      m_listDialog(0),
      m_listWidget(0)
{
    m_currentTask = new Plasma::IconWidget(this);
    m_currentTask->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_currentTask->setTextBackgroundColor(QColor());
    m_closeTask = new Plasma::IconWidget(this);
    m_closeTask->setSvg("widgets/configuration-icons", "close");
    m_closeTask->setMaximumWidth(KIconLoader::SizeSmallMedium);

    m_maximizeTask = new Plasma::IconWidget(this);
    m_maximizeTask->setSvg("widgets/configuration-icons", "maximize");
    m_maximizeTask->setMaximumWidth(KIconLoader::SizeSmallMedium);
    m_maximizeTask->setZValue(999);

    connect(m_closeTask, SIGNAL(clicked()), this, SLOT(closeWindow()));
    connect(m_closeTask, SIGNAL(pressed(bool)), this, SLOT(setSyncDelay(bool)));
    connect(m_maximizeTask, SIGNAL(clicked()), this, SLOT(toggleMaximizedWindow()));
    connect(m_maximizeTask, SIGNAL(pressed(bool)), this, SLOT(setSyncDelay(bool)));
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
    lay->addItem(m_maximizeTask);
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
    bool applicationActive = false;

    foreach (QWidget *widget, QApplication::topLevelWidgets()) {
         if (widget->winId() == id) {
             applicationActive = true;
             break;
         }
    }
    if (!applicationActive && id == m_activeWindow) {
        m_pendingActiveWindow = m_activeWindow;
        syncActiveWindow();
    }
}

void CurrentAppControl::activeWindowChanged(WId id)
{
    m_pendingActiveWindow = id;
    //delay the switch to permit to pass the close action to the proper window if our view accepts focus
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
        const int activeWindows = qMax(0, KWindowSystem::windows().count()-2);
        if (activeWindows) {
            m_currentTask->setText(i18np("%1 running app", "%1 running apps", activeWindows));
        } else {
            m_currentTask->setText(i18n("No running apps"));
        }
        m_closeTask->hide();
        m_maximizeTask->hide();
    } else {
        m_activeWindow = m_pendingActiveWindow;
        KWindowInfo info = KWindowSystem::windowInfo(m_activeWindow, NET::WMName|NET::WMState);
        m_currentTask->setIcon(KWindowSystem::icon(m_activeWindow, KIconLoader::SizeSmallMedium, KIconLoader::SizeSmallMedium));
        m_currentTask->setText(info.name());
        //FIXME: this is utterly bad: the layout seems to -not- resize it?
        m_currentTask->resize(size().width() - m_closeTask->size().width(), m_currentTask->size().height());
        m_closeTask->show();
        m_maximizeTask->show();

        if (info.state() & (NET::MaxVert|NET::MaxHoriz)) {
            m_maximizeTask->setSvg("widgets/configuration-icons", "unmaximize");
        } else {
            m_maximizeTask->setSvg("widgets/configuration-icons", "maximize");
        }
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

void CurrentAppControl::toggleMaximizedWindow()
{
    //TODO: change the icon
#ifdef Q_WS_X11
    KWindowInfo info = KWindowSystem::windowInfo(m_activeWindow, NET::WMState | NET::XAWMState | NET::WMDesktop);
    bool on_current = info.isOnCurrentDesktop();

    if (!on_current) {
        KWindowSystem::setCurrentDesktop(info.desktop());
    }

    if (info.isMinimized()) {
        KWindowSystem::unminimizeWindow(m_activeWindow);
    }

    NETWinInfo ni(QX11Info::display(), m_activeWindow, QX11Info::appRootWindow(), NET::WMState);

    if (!(ni.state() & NET::Max)) {
        ni.setState(NET::Max, NET::Max);
        m_maximizeTask->setSvg("widgets/configuration-icons", "unmaximize");
    } else {
        ni.setState(0, NET::Max);
        m_maximizeTask->setSvg("widgets/configuration-icons", "maximize");
    }

    if (!on_current) {
        KWindowSystem::forceActiveWindow(m_activeWindow);
    }
#endif
}

void CurrentAppControl::listWindows()
{
    if (Plasma::WindowEffects::isEffectAvailable(Plasma::WindowEffects::PresentWindows)) {
        Plasma::WindowEffects::presentWindows(view()->winId() , KWindowSystem::currentDesktop());
    } else if (!m_listDialog) {
        m_listDialog = new Plasma::Dialog();
        m_listWidget = new QGraphicsWidget(this);
        m_listDialog->setGraphicsWidget(m_listWidget);
        Plasma::Corona *corona = 0;
        if (containment() && containment()->corona()) {
            corona = containment()->corona();
            corona->addOffscreenWidget(m_listWidget);
        }

        m_listDialog->setWindowFlags(Qt::FramelessWindowHint|Qt::Dialog);
        KWindowSystem::setType(m_listDialog->winId(), NET::PopupMenu);
        m_listDialog->setAttribute(Qt::WA_DeleteOnClose);
        m_listDialog->installEventFilter(this);

        connect(m_listDialog, SIGNAL(destroyed()), this, SLOT(closePopup()));

        QGraphicsLinearLayout *lay = new QGraphicsLinearLayout(m_listWidget);
        lay->setOrientation(Qt::Vertical);

        foreach(WId window, KWindowSystem::stackingOrder()) {
            KWindowInfo info = KWindowSystem::windowInfo(window, NET::WMName);
            Plasma::IconWidget *icon = new Plasma::IconWidget(m_listWidget);
            icon->setOrientation(Qt::Horizontal);
            icon->setText(info.name());
            icon->setIcon(KWindowSystem::icon(window, KIconLoader::SizeSmallMedium, KIconLoader::SizeSmallMedium));
            icon->setTextBackgroundColor(QColor());
            icon->setDrawBackground(true);
            icon->setMinimumSize(icon->effectiveSizeHint(Qt::PreferredSize));
            connect(icon, SIGNAL(clicked()), this, SLOT(windowItemClicked()));
            m_windowIcons[icon] = window;
            lay->addItem(icon);
        }
        if (corona) {
            m_listDialog->move(containment()->corona()->popupPosition(this, m_listDialog->size()));
        }
        m_listDialog->show();
    } else {
        closePopup();
    }
}

int CurrentAppControl::windowsCount() const
{
    int count = 0;
    foreach(WId window, KWindowSystem::stackingOrder()) {
        KWindowInfo info = KWindowSystem::windowInfo(window, NET::WMWindowType | NET::WMPid | NET::WMState);
        if (!(info.state() & NET::SkipTaskbar) &&
            info.windowType(NET::NormalMask | NET::DialogMask |
                            NET::OverrideMask | NET::UtilityMask) != NET::Utility) {
            ++count;
        }
    }
    return count;
}

void CurrentAppControl::windowItemClicked()
{
    if (sender() && m_windowIcons.contains(static_cast<Plasma::IconWidget*>(sender()))) {
        KWindowSystem::forceActiveWindow(m_windowIcons.value(static_cast<Plasma::IconWidget*>(sender())));
    }
}

void CurrentAppControl::closePopup()
{
    m_listDialog->deleteLater();
    m_listWidget->deleteLater();
    m_listDialog = 0;
    m_listWidget = 0;
}

bool CurrentAppControl::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_listDialog && event->type() == QEvent::WindowDeactivate) {
        closePopup();
    }
    return false;
}

#include "currentappcontrol.moc"

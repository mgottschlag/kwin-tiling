/***************************************************************************
 *   Copyright (C) 2007 by Alexis Ménard <darktears31@gmail.com>           *
 *   Copyright (C) 2009 by Frederik Gladhorn <gladhorn@kde.org>            *
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

#include "lockout.h"

// Plasma
#include <Plasma/IconWidget>
#include <Plasma/ToolTipManager>

// Qt
#include <QtGui/QWidget> // QWIDGETSIZE_MAX
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>
#include <QtDBus/QDBusConnectionInterface>
#include <QGraphicsLinearLayout>

// KDE
#include <KDebug>
#include <KIcon>
#include <KJob>
#include <KAuthorized>
#ifndef Q_OS_WIN
#include <KConfigDialog>
#include <KSharedConfig>
#include <kworkspace/kworkspace.h>
#include <krunner_interface.h>
#include <screensaver_interface.h>
#endif

// Windows
#ifdef Q_OS_WIN
#include <windows.h>
#endif // Q_OS_WIN

static const int MINBUTTONSIZE = 16;
static const int MARGINSIZE = 1;

LockOut::LockOut(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args), 
      m_iconLock(0), 
      m_iconSwitchUser(0), 
      m_iconLogout(0), 
      m_iconSleep(0), 
      m_iconHibernate(0),
      m_changed(false)
{
#ifndef Q_OS_WIN
    setHasConfigurationInterface(true);
#endif
    setAspectRatioMode(Plasma::IgnoreAspectRatio);
}

void LockOut::init()
{
    m_layout = new QGraphicsLinearLayout(this);
    m_layout->setContentsMargins(0,0,0,0);
    m_layout->setSpacing(0);

    configChanged();
}

void LockOut::configChanged()
{
    #ifndef Q_OS_WIN
        KConfigGroup cg = config();
        m_showLockButton = cg.readEntry("showLockButton", true);
        m_showSwitchUserButton = cg.readEntry("showSwitchUserButton", false);
        m_showLogoutButton = cg.readEntry("showLogoutButton", true);
        m_showSleepButton = cg.readEntry("showSleepButton", false);
        m_showHibernateButton = cg.readEntry("showHibernateButton", false);
    #endif
    countButtons();

    if (m_visibleButtons == 0 ) {
        m_showLockButton = true;
        m_showSwitchUserButton = false;
        m_showLogoutButton = true;
        m_showSleepButton =  false;
        m_showHibernateButton =  false;
        countButtons();
    }
    showButtons();
}
void LockOut::buttonChanged()
{
#ifndef Q_WS_WIN
    if (m_showLockButton != ui.checkBox_lock->isChecked()) {
       m_showLockButton = !m_showLockButton;
       m_changed = true;
    }

    if (m_showSwitchUserButton != ui.checkBox_switchUser->isChecked()) {
        m_showSwitchUserButton = !m_showSwitchUserButton;
        m_changed = true;
    }

    if (m_showLogoutButton != ui.checkBox_logout->isChecked()) {
        m_showLogoutButton = !m_showLogoutButton;
        m_changed = true;
    }

    if (m_showSleepButton != ui.checkBox_sleep->isChecked()) {
        m_showSleepButton = !m_showSleepButton;
        m_changed = true;
    }

    if (m_showHibernateButton != ui.checkBox_hibernate->isChecked()) {
        m_showHibernateButton = !m_showHibernateButton;
        m_changed = true;
    }

    setCheckable();

    emit configUiChanged();
#endif
}
void LockOut::setCheckable()
{
#ifndef Q_WS_WIN
    countButtons();
    if (m_visibleButtons == 1) {
        if (ui.checkBox_lock->isChecked()) {
            ui.checkBox_lock->setEnabled(false);
        }
        if (ui.checkBox_switchUser->isChecked()) {
            ui.checkBox_switchUser->setEnabled(false);
        }
        if (ui.checkBox_logout->isChecked()) {
            ui.checkBox_logout->setEnabled(false);
        }
        if (ui.checkBox_sleep->isChecked()) {
            ui.checkBox_sleep->setEnabled(false);
        }
        if(ui.checkBox_hibernate->isChecked()) {
            ui.checkBox_hibernate->setEnabled(false);
        }
    } else {
        ui.checkBox_lock->setEnabled(true);
        ui.checkBox_switchUser->setEnabled(true);
        ui.checkBox_logout->setEnabled(true);
        ui.checkBox_sleep->setEnabled(true);
        ui.checkBox_hibernate->setEnabled(true);
    }
#endif
}

void LockOut::countButtons()
{
    m_visibleButtons = 0;

    if (m_showLockButton) {
        m_visibleButtons++;
    }

    if (m_showSwitchUserButton) {
        m_visibleButtons++;
    }

    if (m_showLogoutButton) {
        m_visibleButtons++;
    }

    if (m_showSleepButton) {
        m_visibleButtons++;
    }

    if (m_showHibernateButton) {
        m_visibleButtons++;
    }
}

LockOut::~LockOut()
{
}

void LockOut::checkLayout()
{
    qreal left,top, right, bottom;
    getContentsMargins(&left,&top, &right, &bottom);
    int width = geometry().width() - left - right;
    int height = geometry().height() - top - bottom;

    Qt::Orientation direction = Qt::Vertical;
    int minWidth = 0;
    int minHeight = 0;

    switch (formFactor()) {
        case Plasma::Vertical:
            if (width >= (MINBUTTONSIZE + MARGINSIZE) * m_visibleButtons) {
                direction = Qt::Horizontal;
                height = qMax(width / m_visibleButtons, MINBUTTONSIZE);
                minHeight = MINBUTTONSIZE;
            } else {
                minHeight = MINBUTTONSIZE * m_visibleButtons + top + bottom;
            }
            break;

        case Plasma::Horizontal:
            if (height < (MINBUTTONSIZE + MARGINSIZE) * m_visibleButtons) {
                direction = Qt::Horizontal;
                minWidth = MINBUTTONSIZE * m_visibleButtons + left + right;
            } else {
                width = qMax(height / m_visibleButtons, MINBUTTONSIZE);
                minWidth = MINBUTTONSIZE;
            }
            break;

        default:
            if (width > height) {
                direction = Qt::Horizontal;
                minWidth = MINBUTTONSIZE * m_visibleButtons + left + right;
                minHeight = MINBUTTONSIZE + top + bottom;
            } else {
                minWidth = MINBUTTONSIZE + left + right;
                minHeight = MINBUTTONSIZE * m_visibleButtons + top + bottom;
            }
            break;
    }

    m_layout->setOrientation(direction);
    setMinimumSize(minWidth, minHeight);
    if (direction == Qt::Horizontal) {
        setPreferredSize(height * m_visibleButtons + left + right, height + top + bottom);
    } else {
        setPreferredSize(width + left + right, width * m_visibleButtons + top + bottom);
    }
}

void LockOut::constraintsEvent(Plasma::Constraints constraints)
{
    if (constraints & Plasma::FormFactorConstraint ||
        constraints & Plasma::SizeConstraint) {
        checkLayout();
    }
}

void LockOut::clickLock()
{
    kDebug()<<"LockOut:: lock clicked ";

#ifndef Q_OS_WIN
    QString interface("org.freedesktop.ScreenSaver");
    org::freedesktop::ScreenSaver screensaver(interface, "/ScreenSaver",
                                              QDBusConnection::sessionBus());
    if (screensaver.isValid()) {
        screensaver.Lock();
    }
#else
    LockWorkStation();
#endif // !Q_OS_WIN
}

void LockOut::clickLogout()
{
    if (!KAuthorized::authorizeKAction("logout")) {
        return;
    }

    kDebug()<<"LockOut:: logout clicked ";
#ifndef Q_OS_WIN
    KWorkSpace::requestShutDown( KWorkSpace::ShutdownConfirmDefault,
                                 KWorkSpace::ShutdownTypeDefault,
                                 KWorkSpace::ShutdownModeDefault);
#endif
}

void LockOut::clickSwitchUser()
{
#ifndef Q_OS_WIN
    // Taken from kickoff/core/itemhandlers.cpp
    QString interface("org.kde.krunner");
    org::kde::krunner::App krunner(interface, "/App", QDBusConnection::sessionBus());
    krunner.switchUser();
#endif
}

#include <KMessageBox>

void LockOut::clickSleep()
{
    if (KMessageBox::questionYesNo(0,
                                   i18n("Do you want to suspend to RAM (sleep)?"),
                                   i18n("Suspend"))
            != KMessageBox::Yes) {
        return;
    }
    // Check if KDE Power Management System is running, and use its methods to suspend if available
    if (QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.Solid.PowerManagement")) {
        kDebug() << "Using KDE Power Management System to suspend";
        QDBusMessage call = QDBusMessage::createMethodCall("org.kde.Solid.PowerManagement",
                                                           "/org/kde/Solid/PowerManagement",
                                                           "org.kde.Solid.PowerManagement",
                                                           "suspendToRam");
        QDBusConnection::sessionBus().asyncCall(call);
    } else {
        kDebug() << "KDE Power Management System not available, suspend failed";
    }
}

void LockOut::clickHibernate()
{
    if (KMessageBox::questionYesNo(0,
                                   i18n("Do you want to suspend to disk (hibernate)?"),
                                   i18n("Hibernate"))
            != KMessageBox::Yes) {
        return;
    }
    // Check if KDE Power Management System is running, and use its methods to hibernate if available
    if (QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.Solid.PowerManagement")) {
        kDebug() << "Using KDE Power Management System to hibernate";
        QDBusMessage call = QDBusMessage::createMethodCall("org.kde.Solid.PowerManagement",
                                                           "/org/kde/Solid/PowerManagement",
                                                           "org.kde.Solid.PowerManagement",
                                                           "suspendToDisk");
        QDBusConnection::sessionBus().asyncCall(call);
    } else {
        kDebug() << "KDE Power Management System not available, hibernate failed";
    }
}

void LockOut::configAccepted()
{
#ifndef Q_OS_WIN
    KConfigGroup cg = config();
    if (m_changed) {
        int oldButtonCount = m_visibleButtons;
        countButtons();
	if(m_visibleButtons == 0) {
	    configChanged(); // if no button was selected, reject configuration and reload previous
	    return;
	}
	
	cg.writeEntry("showHibernateButton", m_showHibernateButton);
	cg.writeEntry("showSleepButton", m_showSleepButton);
	cg.writeEntry("showLogoutButton", m_showLogoutButton);
	cg.writeEntry("showSwitchUserButton", m_showSwitchUserButton);
	cg.writeEntry("showLockButton", m_showLockButton);
	
        showButtons();
        if (formFactor() != Plasma::Horizontal && formFactor() != Plasma::Vertical) {
            resize(size().width(), (size().height() / oldButtonCount) * m_visibleButtons);
        }
        emit configNeedsSaving();
    }
#endif
}

void LockOut::createConfigurationInterface(KConfigDialog *parent)
{
#ifndef Q_OS_WIN
    QWidget *widget = new QWidget(parent);
    ui.setupUi(widget);
    parent->addPage(widget, i18n("Actions"), Applet::icon());
    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
    
    countButtons();
    if (m_visibleButtons == 1) {
        ui.checkBox_lock->setEnabled(!m_showLockButton);
        ui.checkBox_switchUser->setEnabled(!m_showSwitchUserButton);
        ui.checkBox_logout->setEnabled(!m_showLogoutButton);
        ui.checkBox_sleep->setEnabled(!m_showSleepButton);
        ui.checkBox_hibernate->setEnabled(!m_showHibernateButton);
    } 

    ui.checkBox_lock->setChecked(m_showLockButton);
    connect(ui.checkBox_lock, SIGNAL(toggled(bool)), this, SLOT(buttonChanged()));
    ui.checkBox_switchUser->setChecked(m_showSwitchUserButton);
    connect(ui.checkBox_switchUser, SIGNAL(toggled(bool)), this, SLOT(buttonChanged()));
    ui.checkBox_logout->setChecked(m_showLogoutButton);
    connect(ui.checkBox_logout, SIGNAL(toggled(bool)), this, SLOT(buttonChanged()));
    ui.checkBox_sleep->setChecked(m_showSleepButton);
    connect(ui.checkBox_sleep, SIGNAL(toggled(bool)), this, SLOT(buttonChanged()));
    ui.checkBox_hibernate->setChecked(m_showHibernateButton);
    connect(ui.checkBox_hibernate, SIGNAL(toggled(bool)), this, SLOT(buttonChanged()));

    connect(this, SIGNAL(configUiChanged()), parent, SLOT(settingsModified()));
    connect(ui.checkBox_logout, SIGNAL(toggled(bool)), parent, SLOT(settingsModified()));
    connect(ui.checkBox_lock, SIGNAL(toggled(bool)), parent, SLOT(settingsModified()));
    connect(ui.checkBox_switchUser, SIGNAL(toggled(bool)), parent, SLOT(settingsModified()));
    connect(ui.checkBox_hibernate, SIGNAL(toggled(bool)), parent, SLOT(settingsModified()));
    connect(ui.checkBox_sleep, SIGNAL(toggled(bool)), parent, SLOT(settingsModified()));
#endif
}

void LockOut::showButtons()
{
    while(m_layout->count() > 0) {
	m_layout->removeAt(0);
    }
    
    // can this be done more beautiful?
    delete m_iconLock;
    m_iconLock = 0;
    delete m_iconSwitchUser;
    m_iconSwitchUser = 0;
    delete m_iconLogout;
    m_iconLogout = 0;
    delete m_iconSleep;
    m_iconSleep = 0;
    delete m_iconHibernate;
    m_iconHibernate = 0;
    
#ifdef Q_OS_WIN
    //Tooltip strings maybe should be different (eg. "Leave..."->"Logout")?
    m_iconLock = new Plasma::IconWidget(KIcon("system-lock-screen"), "", this);
    connect(m_iconLock, SIGNAL(clicked()), this, SLOT(clickLock()));
    Plasma::ToolTipContent lockToolTip(i18n("Lock"),i18n("Lock the screen"),m_iconLock->icon());
    Plasma::ToolTipManager::self()->setContent(m_iconLock, lockToolTip);
    m_layout->addItem(m_iconLock);
#else

    if (m_showLockButton) {
	//Tooltip strings maybe should be different (eg. "Leave..."->"Logout")?
	m_iconLock = new Plasma::IconWidget(KIcon("system-lock-screen"), "", this);
	connect(m_iconLock, SIGNAL(clicked()), this, SLOT(clickLock()));
	Plasma::ToolTipContent lockToolTip(i18n("Lock"),i18n("Lock the screen"),m_iconLock->icon());
	Plasma::ToolTipManager::self()->setContent(m_iconLock, lockToolTip);
        m_layout->addItem(m_iconLock);
    }

    if (m_showSwitchUserButton) {
	m_iconSwitchUser = new Plasma::IconWidget(KIcon("system-switch-user"), "", this);
	connect(m_iconSwitchUser, SIGNAL(clicked()), this, SLOT(clickSwitchUser()));
	Plasma::ToolTipContent switchUserToolTip(i18n("Switch user"),i18n("Start a parallel session as a different user"),m_iconSwitchUser->icon());
	Plasma::ToolTipManager::self()->setContent(m_iconSwitchUser, switchUserToolTip);
        m_layout->addItem(m_iconSwitchUser);
    }

    if (m_showLogoutButton) {
	m_iconLogout = new Plasma::IconWidget(KIcon("system-shutdown"), "", this);
	connect(m_iconLogout, SIGNAL(clicked()), this, SLOT(clickLogout()));
	Plasma::ToolTipContent logoutToolTip(i18n("Leave..."),i18n("Logout, turn off or restart the computer"),m_iconLogout->icon());
	Plasma::ToolTipManager::self()->setContent(m_iconLogout, logoutToolTip);
        m_layout->addItem(m_iconLogout);
    }

    if (m_showSleepButton) {
	m_iconSleep = new Plasma::IconWidget(KIcon("system-suspend"), "", this);
	connect(m_iconSleep, SIGNAL(clicked()), this, SLOT(clickSleep()));
	Plasma::ToolTipContent sleepToolTip(i18n("Suspend"),i18n("Sleep (suspend to RAM)"),m_iconSleep->icon());
	Plasma::ToolTipManager::self()->setContent(m_iconSleep, sleepToolTip);	
        m_layout->addItem(m_iconSleep);
    }

    if (m_showHibernateButton) {
	m_iconHibernate = new Plasma::IconWidget(KIcon("system-suspend-hibernate"), "", this);
	connect(m_iconHibernate, SIGNAL(clicked()), this, SLOT(clickHibernate()));
	Plasma::ToolTipContent hibernateToolTip(i18n("Hibernate"),i18n("Hibernate (suspend to disk)"),m_iconHibernate->icon());
	Plasma::ToolTipManager::self()->setContent(m_iconHibernate, hibernateToolTip);
        m_layout->addItem(m_iconHibernate);
    }

    setConfigurationRequired(!m_showLockButton && !m_showSwitchUserButton && !m_showLogoutButton && !m_showSleepButton && !m_showHibernateButton);
    checkLayout();
#endif // !Q_OS_WIN
}

#include "lockout.moc"

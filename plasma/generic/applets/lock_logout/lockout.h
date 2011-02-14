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

#ifndef LOCKOUT_H
#define LOCKOUT_H

#include <Plasma/Applet>

namespace Plasma
{
    class IconWidget;
}

#ifndef Q_OS_WIN
#include "ui_lockoutConfig.h"
#endif

class QGraphicsLinearLayout;

class LockOut : public Plasma::Applet
{
    Q_OBJECT

    public:
        LockOut(QObject *parent, const QVariantList &args);
        ~LockOut();
        void init();
        virtual void constraintsEvent(Plasma::Constraints constraints);

    public slots:
        void configChanged();
        void clickLogout();
        void clickSwitchUser();
        void clickLock();
        void clickSleep();
        void clickHibernate();
        void buttonChanged();

    protected Q_SLOTS:
        void configAccepted();

    protected:
        void createConfigurationInterface(KConfigDialog *parent);
        void countButtons();
        void setCheckable();

    private:
#ifndef Q_OS_WIN
        Ui::lockoutConfig ui;
#endif
        bool m_showLockButton;
        bool m_showSwitchUserButton;
        bool m_showLogoutButton;
        bool m_showSleepButton;
        bool m_showHibernateButton;

        Plasma::IconWidget *m_iconLock;
        Plasma::IconWidget *m_iconSwitchUser;
        Plasma::IconWidget *m_iconLogout;
        Plasma::IconWidget *m_iconSleep;
        Plasma::IconWidget *m_iconHibernate;
        QGraphicsLinearLayout *m_layout;
        void checkLayout();
        void showButtons();

        int m_visibleButtons;
        bool m_changed;

    signals:
        void configUiChanged();
};

K_EXPORT_PLASMA_APPLET(lockout, LockOut)

#endif

/*
 *   Copyright (C) 2010 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <QGraphicsWidget>
#include <QGraphicsLinearLayout>
#include <Plasma/Label>
#include <Plasma/PushButton>

#include "activityicon.h"

class ActivityControls : public QGraphicsWidget {
    Q_OBJECT
public:
    ActivityControls(ActivityIcon * parent);

Q_SIGNALS:
    void closed();

};

class ActivityRemovalConfirmation: public ActivityControls {
    Q_OBJECT
public:
    ActivityRemovalConfirmation(ActivityIcon * parent);

Q_SIGNALS:
    void removalConfirmed();

private:
    QGraphicsLinearLayout * m_layout;

    Plasma::Label         * m_labelRemoveActivity;
    Plasma::PushButton    * m_buttonConfirmRemoval;

    Plasma::PushButton    * m_buttonCancel;
};

class ActivityConfiguration: public ActivityControls {
    Q_OBJECT
public:
    ActivityConfiguration();

};

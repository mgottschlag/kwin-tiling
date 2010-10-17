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

#include "activitycontrols.h"

ActivityControls::ActivityControls(ActivityIcon * parent)
    : QGraphicsWidget(parent)
{
}

// ActivityRemovalConfirmation

ActivityRemovalConfirmation::ActivityRemovalConfirmation(ActivityIcon * parent)
    : ActivityControls(parent)
{

    m_layout = new QGraphicsLinearLayout(this);
    m_layout->setOrientation(Qt::Vertical);
    m_layout->setContentsMargins(0, 0, 0, 0);
    setLayout(m_layout);

    m_labelRemoveActivity = new Plasma::Label(this);
    m_labelRemoveActivity->setText(i18n("Remove activity?"));
    m_labelRemoveActivity->setAlignment(Qt::AlignCenter);
    m_layout->addItem(m_labelRemoveActivity);

    m_buttonConfirmRemoval = new Plasma::PushButton(this);
    m_buttonConfirmRemoval->setText(i18n("Confirm Removal"));
    m_layout->addItem(m_buttonConfirmRemoval);
    connect(m_buttonConfirmRemoval, SIGNAL(clicked()), this, SIGNAL(removalConfirmed()));

    m_buttonCancel = new Plasma::PushButton(this);
    m_buttonCancel->setText(i18n("Cancel Removal"));
    m_layout->addItem(m_buttonCancel);
    connect(m_buttonCancel, SIGNAL(clicked()), this, SIGNAL(closed()));
}


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

// ActivityConfiguration

ActivityConfiguration::ActivityConfiguration(ActivityIcon * parent)
    : ActivityControls(parent)
{
    m_layoutButtons = new QGraphicsLinearLayout(this);
    m_layoutButtons->setOrientation(Qt::Vertical);
    m_layoutButtons->setContentsMargins(0, 0, 0, 0);
    setLayout(m_layoutButtons);

    m_labelConfiguration = new Plasma::Label(this);
    m_labelConfiguration->setText(i18n("Accept changes?"));
    m_labelConfiguration->setAlignment(Qt::AlignCenter);
    m_layoutButtons->addItem(m_labelConfiguration);

    m_buttonConfirmChanges = new Plasma::PushButton(this);
    m_buttonConfirmChanges->setText(i18n("Apply Changes"));
    m_layoutButtons->addItem(m_buttonConfirmChanges);
    connect(m_buttonConfirmChanges, SIGNAL(clicked()), this, SIGNAL(applyChanges()));

    m_buttonCancel = new Plasma::PushButton(this);
    m_buttonCancel->setText(i18n("Cancel Changes"));
    m_layoutButtons->addItem(m_buttonCancel);
    connect(m_buttonCancel, SIGNAL(clicked()), this, SIGNAL(closed()));

    m_layoutMain = new QGraphicsLinearLayout(parent);
    m_layoutMain->setOrientation(Qt::Vertical);
    m_layoutMain->setContentsMargins(0, 0, 0, 0);

    m_activityName = new Plasma::LineEdit(parent);
    m_layoutMain->addItem(m_activityName);

    m_activityIcon = new Plasma::PushButton(parent);
    m_activityIcon->setIcon(KIcon("plasma"));
    m_layoutMain->addItem(m_activityIcon);

    m_activityName->setZValue(2);
    m_activityIcon->setZValue(2);

    m_layoutMain->setGeometry(QRectF(0, 0, this->geometry().left(), parent->geometry().height()));
}

ActivityConfiguration::~ActivityConfiguration()
{
    m_activityName->hide();
    m_activityIcon->hide();

    m_activityName->deleteLater();
    m_activityIcon->deleteLater();
}

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
#include <QGraphicsScene>
#include <KPushButton>
#include <KIconDialog>
#include <KWindowSystem>
#include "kworkspace/kactivityinfo.h"
#include <QApplication>

ActivityControls::ActivityControls(ActivityIcon * parent)
    : QGraphicsWidget(parent)
{
}

bool ActivityControls::hidesContents() const
{
    return false;
}

// ActivityRemovalConfirmation

ActivityRemovalConfirmation::ActivityRemovalConfirmation(ActivityIcon * parent)
    : ActivityControls(parent)
{
    m_layout = new QGraphicsLinearLayout(this);
    m_layout->setOrientation(Qt::Vertical);
    m_layout->setContentsMargins(16, 0, 0, 0);
    setLayout(m_layout);

    m_labelRemoveActivity = new Plasma::Label(this);
    m_labelRemoveActivity->setText(i18n("Remove activity?"));
    m_labelRemoveActivity->setAlignment(Qt::AlignCenter);
    m_labelRemoveActivity->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    m_layout->addItem(m_labelRemoveActivity);

    m_buttonConfirmRemoval = new Plasma::PushButton(this);
    m_buttonConfirmRemoval->setText(i18n("Remove"));
    m_layout->addItem(m_buttonConfirmRemoval);
    connect(m_buttonConfirmRemoval, SIGNAL(clicked()), this, SIGNAL(removalConfirmed()));

    m_buttonCancel = new Plasma::PushButton(this);
    m_buttonCancel->setText(i18n("Cancel"));
    m_layout->addItem(m_buttonCancel);
    connect(m_buttonCancel, SIGNAL(clicked()), this, SIGNAL(closed()));
}

// ActivityConfiguration

ActivityConfiguration::ActivityConfiguration(ActivityIcon * parent, Activity * activity)
    : ActivityControls(parent), m_activity(activity)
{
    m_layoutButtons = new QGraphicsLinearLayout(this);
    m_layoutButtons->setOrientation(Qt::Vertical);
    m_layoutButtons->setContentsMargins(16, 0, 0, 0);
    setLayout(m_layoutButtons);

    m_labelConfiguration = new Plasma::Label(this);
    m_labelConfiguration->setText(i18n("Accept changes?"));
    m_labelConfiguration->setAlignment(Qt::AlignCenter);
    m_labelConfiguration->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    m_layoutButtons->addItem(m_labelConfiguration);

    m_buttonConfirmChanges = new Plasma::PushButton(this);
    m_buttonConfirmChanges->setText(i18n("Apply"));
    m_layoutButtons->addItem(m_buttonConfirmChanges);
    connect(m_buttonConfirmChanges, SIGNAL(clicked()), this, SLOT(applyChanges()));
    connect(m_buttonConfirmChanges, SIGNAL(clicked()), this, SIGNAL(closed()));

    m_buttonCancel = new Plasma::PushButton(this);
    m_buttonCancel->setText(i18n("Cancel"));
    m_layoutButtons->addItem(m_buttonCancel);
    connect(m_buttonCancel, SIGNAL(clicked()), this, SIGNAL(closed()));

    m_main = new QGraphicsWidget(parent);

    m_layoutMain = new QGraphicsLinearLayout(m_main);
    m_layoutMain->setOrientation(Qt::Vertical);
    m_layoutMain->setContentsMargins(0, 0, 0, 0);
    m_layoutMain->setSpacing(0);

    m_activityName = new Plasma::LineEdit(this);
    m_layoutMain->addItem(m_activityName);

    m_activityIcon = new Plasma::PushButton(this);
    m_activityIcon->setIcon(KIcon("plasma"));
    m_layoutMain->addItem(m_activityIcon);
    connect(m_activityIcon, SIGNAL(clicked()), this, SLOT(chooseIcon()));

    m_main->setGeometry(parent->contentsRect());

    m_activityName->setText(m_activity->name());

    m_activityIcon->setIcon(
            QIcon(parent->pixmap(QSize(32, 32))));

    if (m_activity && m_activity->info() && m_activity->info()->availability() == KActivityInfo::Everything) {
        m_activityIcon->setEnabled(true);
    } else {
        m_activityIcon->setEnabled(false);
    }
}

void ActivityConfiguration::hideEvent(QHideEvent * event)
{
    ActivityControls::hideEvent(event);

    m_main->hide();
}

void ActivityConfiguration::showEvent(QShowEvent * event)
{
    ActivityControls::showEvent(event);

    m_main->setZValue(zValue());
    m_main->show();
}

ActivityConfiguration::~ActivityConfiguration()
{
    // delete m_layoutMain;
    m_main->deleteLater();
}

void ActivityConfiguration::applyChanges()
{
    m_activity->setName(m_activityName->text());

    if (!m_iconName.isEmpty()) {
        m_activity->setIcon(m_iconName);
    }
}

void ActivityConfiguration::chooseIcon()
{
    QString iconName = KIconDialog::getIcon();

    if (!iconName.isEmpty()) {
        m_activityIcon->setIcon(KIcon(iconName));
        m_iconName = iconName;
    }

    // somehow, after closing KIconDialog, plasma loses focus
    // and the panel controller is closed, soforcing focus to
    // any of the top level windows that are shown
    foreach (QWidget * widget, QApplication::topLevelWidgets()) {
        if (widget->isVisible()) {
            KWindowSystem::forceActiveWindow(widget->winId(), 0);
            break;
        }
    }
}

bool ActivityConfiguration::hidesContents() const
{
    return true;
}

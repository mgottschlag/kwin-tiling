/***************************************************************************
 *   Copyright (C) 2008 - 2009 by Lukas Appelhans <l.appelhans@gmx.de>     *
 *   Copyright (C) 2010 by Ingomar Wesp <ingomar@wesp.name>                *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/
#include "quicklaunch.h"

// Qt
#include <Qt>
#include <QtGlobal>
#include <QtCore/QDir>
#include <QtCore/QEvent>
#include <QtCore/QPoint>
#include <QtCore/QSize>
#include <QtGui/QGraphicsLinearLayout>
#include <QtGui/QGraphicsSceneContextMenuEvent>
#include <QtGui/QGraphicsWidget>

// KDE
#include <KConfigDialog>
#include <KDesktopFile>
#include <KDialog>
#include <KMenu>
#include <KMessageBox>
#include <KMimeType>
#include <KOpenWithDialog>
#include <KPropertiesDialog>
#include <KWindowSystem>
#include <KUrl>

// Plasma
#include <Plasma/Applet>
#include <Plasma/IconWidget>
#include <Plasma/Svg>

// Own
#include "icongrid.h"
#include "icongridlayout.h"
#include "itemdata.h"
#include "popup.h"

using Plasma::Applet;
using Plasma::Constraints;
using Plasma::FormFactor;
using Plasma::IconWidget;
using Plasma::Location;
using Plasma::Svg;

namespace Quicklaunch {

K_EXPORT_PLASMA_APPLET(quicklaunch, Quicklaunch)

Quicklaunch::Quicklaunch(QObject *parent, const QVariantList &args)
  : Applet(parent, args),
    m_iconGrid(0),
    m_layout(0),
    m_popupTrigger(0),
    m_popup(0),
    m_addIconAction(0),
    m_removeIconAction(0),
    m_currentIconGrid(0),
    m_currentIconIndex(-1)
{
    setHasConfigurationInterface(true);
    setAspectRatioMode(Plasma::IgnoreAspectRatio);
    setBackgroundHints(TranslucentBackground);
}

Quicklaunch::~Quicklaunch()
{
    if (m_popup) {
        deletePopup();
    }
}

void Quicklaunch::init()
{
    // Initialize outer layout
    m_layout = new QGraphicsLinearLayout();
    m_layout->setContentsMargins(2, 2, 2, 2);
    m_layout->setSpacing(4);

    // Initialize icon grid
    m_iconGrid = new IconGrid();
    m_iconGrid->installEventFilter(this);

    m_layout->addItem(m_iconGrid);
    m_layout->setStretchFactor(m_iconGrid, 1);

    // Read config
    readConfig();

    connect(m_iconGrid, SIGNAL(iconsChanged()), SLOT(onIconsChanged()));

    setLayout(m_layout);
}

void Quicklaunch::createConfigurationInterface(KConfigDialog *parent)
{
    QWidget *widget = new QWidget(parent);
    uiConfig.setupUi(widget);
    connect(parent, SIGNAL(accepted()), SLOT(onConfigAccepted()));

    FormFactor appletFormFactor = formFactor();

    if (appletFormFactor == Plasma::Horizontal) {
        uiConfig.forceMaxRowsOrColumnsLabel->setText(i18n("Force row settings:"));
        uiConfig.maxRowsOrColumnsLabel->setText(i18n("Max row count:"));
    } else if (appletFormFactor == Plasma::Planar) {
        // Hide maxRowsOrColumns / maxRowsOrColumnsForced when in planar
        // form factor.
        uiConfig.forceMaxRowsOrColumnsLabel->hide();
        uiConfig.forceMaxRowsOrColumnsCheckBox->hide();
        uiConfig.maxRowsOrColumnsLabel->hide();
        uiConfig.maxRowsOrColumnsSpinBox->hide();
    } else {
        uiConfig.forceMaxRowsOrColumnsLabel->setText(i18n("Force column settings:"));
        uiConfig.maxRowsOrColumnsLabel->setText(i18n("Max column count:"));
    }

    uiConfig.forceMaxRowsOrColumnsCheckBox->setChecked(
        m_iconGrid->layout()->maxRowsOrColumnsForced());

    uiConfig.maxRowsOrColumnsSpinBox->setValue(
        m_iconGrid->layout()->maxRowsOrColumns());

    uiConfig.iconNamesVisibleCheckBox->setChecked(
        m_iconGrid->iconNamesVisible());

    uiConfig.dialogEnabledCheckBox->setChecked(m_popup != 0);

    parent->addPage(widget, i18n("General"), icon());
}

bool Quicklaunch::eventFilter(QObject *watched, QEvent *event)
{
    const QEvent::Type eventType = event->type();

    if (eventType == QEvent::GraphicsSceneContextMenu) {

        IconGrid *source = qobject_cast<IconGrid*>(watched);

        if (source) {
            QGraphicsSceneContextMenuEvent *contextMenuEvent =
                static_cast<QGraphicsSceneContextMenuEvent*>(event);

            int iconIndex =
                source->iconIndexAtPosition(
                    source->mapFromScene(contextMenuEvent->scenePos()));

            showContextMenu(contextMenuEvent->screenPos(), source, iconIndex);
            return true;
        }
    }
    else if ((eventType == QEvent::Hide || eventType == QEvent::Show) &&
             m_popup != 0 &&
             qobject_cast<Popup*>(watched) == m_popup) {

            updatePopupTrigger();
    }
    else if (eventType == QEvent::GraphicsSceneDragEnter &&
             m_popupTrigger != 0 &&
             m_popup->isHidden() &&
             qobject_cast<IconWidget*>(watched) == m_popupTrigger) {

        m_popup->show();
        event->setAccepted(false);
        return true;
    }

    return false;
}

void Quicklaunch::constraintsEvent(Constraints constraints)
{
    if (constraints & Plasma::FormFactorConstraint) {
        FormFactor newFormFactor = formFactor();

        m_iconGrid->layout()->setMode(
            newFormFactor == Plasma::Horizontal
                ? IconGridLayout::PreferRows
                : IconGridLayout::PreferColumns);

        // Ignore maxRowsOrColumns / maxRowsOrColumnsForced when in planar
        // form factor.
        if (newFormFactor == Plasma::Planar) {
            m_iconGrid->layout()->setMaxRowsOrColumns(0);
            m_iconGrid->layout()->setMaxRowsOrColumnsForced(false);
        }

        m_layout->setOrientation(
            newFormFactor == Plasma::Vertical ? Qt::Vertical : Qt::Horizontal);
    }

    if (constraints & Plasma::LocationConstraint) {

        if (m_popupTrigger) {
            updatePopupTrigger();
        }
    }

    if (constraints & Plasma::ImmutableConstraint) {

        if (immutability() == Plasma::Mutable) {
            m_iconGrid->setLocked(false);
            if (m_popup) {
                m_popup->iconGrid()->setLocked(false);
            }

        } else {
            m_iconGrid->setLocked(true);
            if (m_popup) {
                m_popup->iconGrid()->setLocked(true);
            }
        }
    }
}

void Quicklaunch::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    showContextMenu(event->screenPos(), 0, -1);
}

void Quicklaunch::onConfigAccepted()
{
    const bool maxRowsOrColumnsForced = uiConfig.forceMaxRowsOrColumnsCheckBox->isChecked();
    const int maxRowsOrColumns = uiConfig.maxRowsOrColumnsSpinBox->value();
    const bool iconNamesVisible = uiConfig.iconNamesVisibleCheckBox->isChecked();
    const bool dialogEnabled = uiConfig.dialogEnabledCheckBox->isChecked();

    KConfigGroup config = this->config();
    bool changed = false;

    if (maxRowsOrColumnsForced != m_iconGrid->layout()->maxRowsOrColumnsForced()) {
        m_iconGrid->layout()->setMaxRowsOrColumnsForced(maxRowsOrColumnsForced);
        config.writeEntry("maxRowsOrColumnsForced", maxRowsOrColumnsForced);
        changed = true;
    }

    if (maxRowsOrColumns != m_iconGrid->layout()->maxRowsOrColumns()) {
        m_iconGrid->layout()->setMaxRowsOrColumns(maxRowsOrColumns);
        config.writeEntry("maxRowsOrColumns", maxRowsOrColumns);
        changed = true;
    }

    if (iconNamesVisible != m_iconGrid->iconNamesVisible()) {

        m_iconGrid->setIconNamesVisible(iconNamesVisible);

        if (m_popup != 0) {
            m_popup->iconGrid()->setIconNamesVisible(iconNamesVisible);
            // syncDialogSize();
        }

        config.writeEntry("iconNamesVisible", iconNamesVisible);
        changed = true;
    }

    if (dialogEnabled != (m_popup != 0)) {

        if (!m_popup) {
            initPopup();
        } else {
            // Move all icons from the popup into the our own icon grid.
            while(m_popup->iconGrid()->iconCount() > 0) {
                m_iconGrid->insert(-1, m_popup->iconGrid()->iconAt(0));
                m_popup->iconGrid()->removeAt(0);
            }

            deletePopup();
        }

        config.writeEntry("dialogEnabled", dialogEnabled);
        changed = true;
    }

    if (changed) {
        Q_EMIT configNeedsSaving();
    }
}

void Quicklaunch::onIconsChanged()
{
    // Save new icon list
    QStringList icons;
    QStringList dialogIcons;

    for (int i = 0; i < m_iconGrid->iconCount(); i++) {
        icons.append(m_iconGrid->iconAt(i).url().prettyUrl());
    }

    if (m_popup) {
        for (int i = 0; i < m_popup->iconGrid()->iconCount(); i++) {
            // XXX: Is prettyUrl() really needed?
            dialogIcons.append(m_popup->iconGrid()->iconAt(i).url().prettyUrl());
        }
    }

    KConfigGroup config = this->config();

    config.writeEntry("icons", icons);
    config.writeEntry("dialogIcons", dialogIcons);
    Q_EMIT configNeedsSaving();
}

void Quicklaunch::onPopupTriggerClicked()
{
    Q_ASSERT(m_popup);

    if (m_popup->isVisible()) {
        m_popup->hide();
    } else {
        m_popup->show();
    }
}

void Quicklaunch::onRemoveIconAction()
{
    Q_ASSERT(m_currentIconGrid);
    Q_ASSERT(m_currentIconIndex != -1);

    m_currentIconGrid->removeAt(m_currentIconIndex);
}

void Quicklaunch::onAddIconAction()
{
    KOpenWithDialog appChooseDialog(0);
    appChooseDialog.hideRunInTerminal();
    appChooseDialog.setSaveNewApplications(true);

    if (appChooseDialog.exec() == QDialog::Accepted) {
        QString programPath = appChooseDialog.service()->entryPath();

        if (appChooseDialog.service()->icon() == 0) {
            // If the program chosen doesn't have an icon, then we give
            // it a default icon and open up its properties in a dialog
            // so the user can change it's icon and name etc
            KConfig kc(programPath, KConfig::SimpleConfig);
            KConfigGroup kcg = kc.group("Desktop Entry");
            kcg.writeEntry("Icon","system-run");
            kc.sync();

            KPropertiesDialog propertiesDialog(KUrl(programPath), NULL);
            if (propertiesDialog.exec() != QDialog::Accepted) {
                return;
            }

            // In case the name changed
            programPath = propertiesDialog.kurl().path();
        }

        if (m_currentIconGrid) {
            m_currentIconGrid->insert(
                m_currentIconIndex, KUrl::fromPath(programPath));
        }
        else {
            m_iconGrid->insert(
                -1, KUrl::fromPath(programPath));
        }
    }
}

void Quicklaunch::showContextMenu(
    const QPoint& screenPos,
    IconGrid *component,
    int iconIndex)
{
    if (m_addIconAction == 0) {
        initActions();
    }

    m_currentIconGrid = component;
    m_currentIconIndex = iconIndex;

    KMenu m;
    m.addAction(action("configure"));
    m.addSeparator();
    m.addAction(m_addIconAction);
    if (component != 0 && iconIndex != -1) {
        m.addAction(m_removeIconAction);
    }
    m.addSeparator();
    m.addAction(action("remove"));
    m.exec(screenPos);

    m_currentIconGrid = 0;
    m_currentIconIndex = -1;
}

void Quicklaunch::initActions()
{
    Q_ASSERT(!m_addIconAction);

    m_addIconAction = new QAction(KIcon("list-add"), i18n("Add Launcher..."), this);
    connect(m_addIconAction, SIGNAL(triggered(bool)), SLOT(onAddIconAction()));

    m_removeIconAction = new QAction(KIcon("list-remove"), i18n("Remove Launcher"), this);
    connect(m_removeIconAction, SIGNAL(triggered(bool)), SLOT(onRemoveIconAction()));
}

void Quicklaunch::initPopup()
{
    Q_ASSERT(!m_popupTrigger);
    Q_ASSERT(!m_popup);

    m_popup = new Popup(this);

    m_popup->installEventFilter(this);
    m_popup->iconGrid()->installEventFilter(this);
    connect(m_popup->iconGrid(), SIGNAL(iconsChanged()), SLOT(onIconsChanged()));

    // Initialize popup trigger
    m_popupTrigger = new IconWidget(this);
    m_popupTrigger->setContentsMargins(0, 0, 0, 0);
    m_popupTrigger->setPreferredWidth(KIconLoader::SizeSmall);
    m_popupTrigger->setPreferredHeight(KIconLoader::SizeSmall);
    m_popupTrigger->setAcceptDrops(true);
    m_popupTrigger->installEventFilter(this);
    updatePopupTrigger();

    m_layout->addItem(m_popupTrigger);
    m_layout->setStretchFactor(m_popupTrigger, 0);
    m_popupTrigger->show();

    connect(m_popupTrigger, SIGNAL(clicked()), SLOT(onPopupTriggerClicked()));
}

void Quicklaunch::updatePopupTrigger()
{
    Q_ASSERT(m_popupTrigger);
    Q_ASSERT(m_popup);

    switch (location()) {
        case Plasma::TopEdge:
            if (m_popup->isHidden()) {
                m_popupTrigger->setSvg("widgets/arrows", "down-arrow");
            } else {
                m_popupTrigger->setSvg("widgets/arrows", "up-arrow");
            }
            break;
        case Plasma::LeftEdge:
            if (m_popup->isHidden()) {
                m_popupTrigger->setSvg("widgets/arrows", "right-arrow");
            } else {
                m_popupTrigger->setSvg("widgets/arrows", "left-arrow");
            }
            break;
        case Plasma::RightEdge:
            if (m_popup->isHidden()) {
                m_popupTrigger->setSvg("widgets/arrows", "left-arrow");
            } else {
                m_popupTrigger->setSvg("widgets/arrows", "right-arrow");
            }
            break;
        default:
            if (m_popup->isHidden()) {
                m_popupTrigger->setSvg("widgets/arrows", "up-arrow");
            } else {
                m_popupTrigger->setSvg("widgets/arrows", "down-arrow");
            }
    }
}

void Quicklaunch::deletePopup()
{
    Q_ASSERT(m_popupTrigger);
    Q_ASSERT(m_popup);

    delete m_popup;
    delete m_popupTrigger;

    m_popup = 0;
    m_popupTrigger = 0;
}

void Quicklaunch::readConfig()
{
    KConfigGroup config = this->config();

    migrateConfig(config);

    const bool maxRowsOrColumnsForced = config.readEntry("maxRowsOrColumnsForced", false);
    const int maxRowsOrColumns = config.readEntry("maxRowsOrColumns", 0);
    const bool iconNamesVisible = config.readEntry("iconNamesVisible", false);
    const bool dialogEnabled = config.readEntry("dialogEnabled", false);

    QList<ItemData> primaryItems;
    QList<ItemData> dialogItems;

    { // Read item lists
        QStringList icons = config.readEntry("icons", QStringList());
        QStringList dialogIcons = config.readEntry("dialogIcons", QStringList());

        if (icons.isEmpty() && dialogIcons.isEmpty()) {
            QStringList defaultApps;
            defaultApps << "konqbrowser" << "dolphin" << "kopete";

            Q_FOREACH (const QString &defaultApp, defaultApps) {
                KService::Ptr service = KService::serviceByStorageId(defaultApp);
                if (service && service->isValid()) {
                    QString path = service->entryPath();

                    if (!path.isEmpty() && QDir::isAbsolutePath(path)) {
                        icons.append(path);
                    }
                }
            }
        }

        Q_FOREACH(QString icon, icons) {
            primaryItems.append(ItemData(icon));
        }


        Q_FOREACH(QString icon, dialogIcons) {
            dialogItems.append(ItemData(icon));
        }
    }

    m_iconGrid->layout()->setMaxRowsOrColumnsForced(maxRowsOrColumnsForced);
    m_iconGrid->layout()->setMaxRowsOrColumns(maxRowsOrColumns);
    m_iconGrid->setIconNamesVisible(iconNamesVisible);

    m_iconGrid->insert(-1, primaryItems);

    if (!dialogItems.isEmpty() || dialogEnabled) {
        initPopup();
        m_popup->iconGrid()->insert(-1, dialogItems);
    }
}

void Quicklaunch::migrateConfig(KConfigGroup &config)
{
    if (config.hasKey("dialogIconSize") ||
        config.hasKey("iconSize") ||
        config.hasKey("iconUrls") ||
        config.hasKey("showIconNames") ||
        config.hasKey("visibleIcons")) {

        // Migrate from Quicklaunch 0.1 config format to 0.2 config format
        QStringList iconUrls = config.readEntry("iconUrls", QStringList());

        int visibleIcons =
            qBound(-1, config.readEntry("visibleIcons", -1), iconUrls.size());

        bool showIconNames = config.readEntry("showIconNames", false);

        config.deleteEntry("dialogIconSize");
        config.deleteEntry("iconSize");
        config.deleteEntry("iconUrls");
        config.deleteEntry("showIconNames");
        config.deleteEntry("visibleIcons");

        QStringList icons;
        QStringList dialogIcons;

        for (int i = 0; i < iconUrls.size(); i++) {

            if (visibleIcons == -1 || i < visibleIcons) {
                icons.append(iconUrls.at(i));
            } else {
                dialogIcons.append(iconUrls.at(i));
            }
        }

        config.writeEntry("icons", icons);
        config.writeEntry("dialogIcons", dialogIcons);
        config.writeEntry("iconNamesVisible", showIconNames);
    }
}
}

#include "quicklaunch.moc"

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
#include <QtCore/QDir>
#include <QtCore/QEvent>
#include <QtCore/QMargins>
#include <QtCore/QSize>
#include <QtGui/QGraphicsLinearLayout>
#include <QtGui/QGraphicsSceneContextMenuEvent>
#include <QtGui/QGraphicsWidget>

// KDE
#include <KConfigDialog>
#include <KDesktopFile>
#include <KDialog>
#include <KIconLoader>
#include <KMenu>
#include <KMessageBox>
#include <KMimeType>
#include <KOpenWithDialog>
#include <KPropertiesDialog>
#include <KWindowSystem>
#include <KUrl>

// Plasma
#include <Plasma/Applet>
#include <Plasma/Containment>
#include <Plasma/Corona>
#include <Plasma/Dialog>
#include <Plasma/IconWidget>

// Own
#include "icongrid.h"
#include "itemdata.h"

using Plasma::Corona;

namespace Quicklaunch {

K_EXPORT_PLASMA_APPLET(quicklaunch, Quicklaunch)

Quicklaunch::Quicklaunch(QObject *parent, const QVariantList &args)
  : Plasma::Applet(parent, args),
    m_primaryIconGrid(0),
    m_layout(0),
    m_dialogArrow(0),
    m_dialog(0),
    m_dialogIconGrid(0),
    m_addIconAction(0),
    m_removeIconAction(0),
    m_currentIconGrid(0),
    m_currentIconIndex(-1)
{
    setHasConfigurationInterface(true);
    setAspectRatioMode(Plasma::IgnoreAspectRatio);
}

Quicklaunch::~Quicklaunch()
{
    if (m_dialog) {
        deleteDialog();
    }
}

void Quicklaunch::init()
{
    // Initialize outer layout
    m_layout = new QGraphicsLinearLayout();
    m_layout->setContentsMargins(2, 2, 2, 2);
    m_layout->setSpacing(0);

    // Initialize icon grid
    m_primaryIconGrid = new IconGrid(formFactor());
    m_primaryIconGrid->installEventFilter(this);

    m_layout->addItem(m_primaryIconGrid);
    m_layout->setStretchFactor(m_primaryIconGrid, 1);

    // Read config
    readConfig();

    connect(m_primaryIconGrid, SIGNAL(iconsChanged()), SLOT(onIconsChanged()));
    connect(m_primaryIconGrid, SIGNAL(displayedItemCountChanged()), SLOT(onDisplayedItemsChanged()));

    setLayout(m_layout);
}

void Quicklaunch::createConfigurationInterface(KConfigDialog *parent)
{
    QWidget *widget = new QWidget(parent);
    uiConfig.setupUi(widget);
    connect(parent, SIGNAL(accepted()), SLOT(onConfigAccepted()));

    if (formFactor() == Plasma::Horizontal) {
        uiConfig.forceMaxRowsOrColumnsLabel->setText(i18n("Force row settings:"));
        uiConfig.maxRowsOrColumnsLabel->setText(i18n("Max row count:"));
    } else if (formFactor() == Plasma::Planar) {
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
        m_primaryIconGrid->maxRowsOrColumnsForced());

    uiConfig.maxRowsOrColumnsSpinBox->setValue(
        m_primaryIconGrid->maxRowsOrColumns());

    uiConfig.iconNamesVisibleCheckBox->setChecked(
        m_primaryIconGrid->iconNamesVisible());

    uiConfig.dialogEnabledCheckBox->setChecked(m_dialog != 0);

    parent->addPage(widget, i18n("General"), icon());
}

bool Quicklaunch::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::GraphicsSceneContextMenu) {

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
    return false;
}

void Quicklaunch::constraintsEvent(Plasma::Constraints constraints)
{
    if (constraints & Plasma::FormFactorConstraint) {
        m_primaryIconGrid->setFormFactor(formFactor());

        m_layout->setOrientation(
            formFactor() == Plasma::Vertical
                ? Qt::Vertical
                : Qt::Horizontal);
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

    if (maxRowsOrColumnsForced != m_primaryIconGrid->maxRowsOrColumnsForced()) {
        m_primaryIconGrid->setMaxRowsOrColumnsForced(maxRowsOrColumnsForced);
        config.writeEntry("maxRowsOrColumnsForced", maxRowsOrColumnsForced);
        changed = true;
    }

    if (maxRowsOrColumns != m_primaryIconGrid->maxRowsOrColumns()) {
        m_primaryIconGrid->setMaxRowsOrColumns(maxRowsOrColumns);
        config.writeEntry("maxRowsOrColumns", maxRowsOrColumns);
        changed = true;
    }

    if (iconNamesVisible != m_primaryIconGrid->iconNamesVisible()) {

        m_primaryIconGrid->setIconNamesVisible(iconNamesVisible);

        if (m_dialog != 0) {
            m_dialogIconGrid->setIconNamesVisible(iconNamesVisible);
            syncDialogSize();
        }

        config.writeEntry("iconNamesVisible", iconNamesVisible);
        changed = true;
    }

    if (dialogEnabled != (m_dialog != 0)) {

        if (!m_dialog) {
            initDialog();
        } else {
            // Move all icons from the dialog into the primary icon grid.
            while(m_dialogIconGrid->iconCount() > 0) {
                m_primaryIconGrid->insert(-1, m_dialogIconGrid->iconAt(0));
                m_dialogIconGrid->removeAt(0);
            }

            deleteDialog();
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

    for (int i = 0; i < m_primaryIconGrid->iconCount(); i++) {
        icons.append(m_primaryIconGrid->iconAt(i).url().prettyUrl());
    }

    if (m_dialog) {
        for (int i = 0; i < m_dialogIconGrid->iconCount(); i++) {
            dialogIcons.append(m_dialogIconGrid->iconAt(i).url().prettyUrl());
        }

        if (m_dialog->isVisible()) {
            syncDialogSize();
        }
    }

    KConfigGroup config = this->config();

    config.writeEntry("icons", icons);
    config.writeEntry("dialogIcons", dialogIcons);
    Q_EMIT configNeedsSaving();
}

void Quicklaunch::onDisplayedItemsChanged() {

    if (m_dialog && m_dialog->isVisible()) {
        syncDialogSize();
    }
}

void Quicklaunch::onDialogArrowClicked()
{
    Q_ASSERT(m_dialog);

    if (m_dialog->isVisible()) {
        m_dialog->hide();
    } else {
        KWindowSystem::setState(m_dialog->winId(), NET::SkipTaskbar);
        m_dialog->show();
        syncDialogSize();
    }
}

void Quicklaunch::onDialogIconClicked()
{
    Q_ASSERT(m_dialog);

    if (m_dialog->isVisible()) {
        m_dialog->hide();
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
            m_primaryIconGrid->insert(
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

void Quicklaunch::syncDialogSize()
{
    Q_ASSERT(m_dialog);

    int dialogItemCount = m_dialogIconGrid->displayedItemCount();

    const int dialogIconGridWidth =
        dialogItemCount *
            (KIconLoader::SizeMedium + m_dialogIconGrid->cellSpacing()) -
        m_dialogIconGrid->cellSpacing();

    const int dialogIconGridHeight = KIconLoader::SizeMedium;

    QMargins dialogMargins = m_dialog->contentsMargins();

    QSize newDialogSize(
        dialogIconGridWidth + dialogMargins.left() + dialogMargins.right(),
        dialogIconGridHeight + dialogMargins.top() + dialogMargins.bottom());

    m_dialog->resize(newDialogSize);

    if (containment() && containment()->corona()) {
        m_dialog->move(
            containment()->corona()->popupPosition(
                m_dialogArrow, m_dialog->size()));
    }
}

void Quicklaunch::initActions()
{
    Q_ASSERT(!m_addIconAction);

    m_addIconAction = new QAction(KIcon("list-add"), i18n("Add Launcher..."), this);
    connect(m_addIconAction, SIGNAL(triggered(bool)), SLOT(onAddIconAction()));

    m_removeIconAction = new QAction(KIcon("list-remove"), i18n("Remove Launcher"), this);
    connect(m_removeIconAction, SIGNAL(triggered(bool)), SLOT(onRemoveIconAction()));
}

void Quicklaunch::initDialog()
{
    Q_ASSERT(!m_dialogArrow);
    Q_ASSERT(!m_dialog);

    m_dialog = new Plasma::Dialog(0, Qt::X11BypassWindowManagerHint);

    m_dialogIconGrid = new IconGrid(Plasma::Horizontal, this);

    m_dialogIconGrid->installEventFilter(this);
    connect(m_dialogIconGrid, SIGNAL(iconsChanged()), SLOT(onIconsChanged()));
    connect(m_dialogIconGrid, SIGNAL(displayedItemCountChanged()), SLOT(onDisplayedItemsChanged()));
    connect(m_dialogIconGrid, SIGNAL(iconClicked()), SLOT(onDialogIconClicked()));

    m_dialog->setGraphicsWidget(m_dialogIconGrid);

    Corona *corona = qobject_cast<Corona*>(m_dialogIconGrid->scene());
    corona->addOffscreenWidget(m_dialogIconGrid);

    // Initialize "more icons" arrow
    m_dialogArrow = new Plasma::IconWidget(this);
    m_dialogArrow->setContentsMargins(0, 0, 0, 0);
    m_dialogArrow->setIcon(KIcon("arrow-right"));

    m_layout->addItem(m_dialogArrow);
    m_layout->setStretchFactor(m_dialogArrow, 0);
    m_dialogArrow->show();

    connect(m_dialogArrow, SIGNAL(clicked()), SLOT(onDialogArrowClicked()));
}

void Quicklaunch::deleteDialog()
{
    Q_ASSERT(m_dialogArrow);
    Q_ASSERT(m_dialog);

    m_dialog->close();

    delete m_dialogIconGrid;
    delete m_dialog;
    delete m_dialogArrow;

    m_dialogIconGrid = 0;
    m_dialog = 0;
    m_dialogArrow = 0;
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

    m_primaryIconGrid->setMaxRowsOrColumnsForced(maxRowsOrColumnsForced);
    m_primaryIconGrid->setMaxRowsOrColumns(maxRowsOrColumns);
    m_primaryIconGrid->setIconNamesVisible(iconNamesVisible);

    m_primaryIconGrid->insert(-1, primaryItems);

    if (!dialogItems.isEmpty() || dialogEnabled) {
        initDialog();
        m_dialogIconGrid->insert(-1, dialogItems);
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

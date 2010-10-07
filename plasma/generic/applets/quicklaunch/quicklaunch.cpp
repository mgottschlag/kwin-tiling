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
#include <Plasma/ToolTipContent>
#include <Plasma/ToolTipManager>

// Own
#include "icongridlayout.h"
#include "launcherdata.h"
#include "launcherlist.h"
#include "popup.h"

using Plasma::Applet;
using Plasma::Constraints;
using Plasma::FormFactor;
using Plasma::IconWidget;
using Plasma::Location;
using Plasma::Svg;
using Plasma::ToolTipContent;
using Plasma::ToolTipManager;

namespace Quicklaunch {

K_EXPORT_PLASMA_APPLET(quicklaunch, Quicklaunch)

Quicklaunch::Quicklaunch(QObject *parent, const QVariantList &args)
  : Applet(parent, args),
    m_launcherList(0),
    m_layout(0),
    m_popupTrigger(0),
    m_popup(0),
    m_addIconAction(0),
    m_removeIconAction(0),
    m_currentLauncherList(0),
    m_currentLauncherIndex(-1)
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

    // Initialize icon area
    m_launcherList = new LauncherList(LauncherList::IconGrid);
    m_launcherList->installEventFilter(this);

    m_layout->addItem(m_launcherList);
    m_layout->setStretchFactor(m_launcherList, 1);

    configChanged();

    connect(m_launcherList, SIGNAL(launchersChanged()), SLOT(onLaunchersChanged()));
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
        m_launcherList->gridLayout()->maxRowsOrColumnsForced());

    uiConfig.maxRowsOrColumnsSpinBox->setValue(
        m_launcherList->gridLayout()->maxRowsOrColumns());

    uiConfig.launcherNamesVisibleCheckBox->setChecked(
        m_launcherList->launcherNamesVisible());

    uiConfig.popupEnabledCheckBox->setChecked(m_popup != 0);

    parent->addPage(widget, i18n("General"), icon());
}

bool Quicklaunch::eventFilter(QObject *watched, QEvent *event)
{
    const QEvent::Type eventType = event->type();

    if (eventType == QEvent::GraphicsSceneContextMenu) {

        LauncherList *source = qobject_cast<LauncherList*>(watched);

        if (source) {
            QGraphicsSceneContextMenuEvent *contextMenuEvent =
                static_cast<QGraphicsSceneContextMenuEvent*>(event);

            int launcherIndex =
                source->launcherIndexAtPosition(
                    source->mapFromScene(contextMenuEvent->scenePos()));

            showContextMenu(contextMenuEvent->screenPos(), source, launcherIndex);
            return true;
        }
    }
    else if ((eventType == QEvent::Hide || eventType == QEvent::Show) &&
             m_popup != 0 &&
             watched == m_popup) {

            updatePopupTrigger();
    }
    else if (eventType == QEvent::GraphicsSceneDragEnter &&
             m_popupTrigger != 0 &&
             m_popup->isHidden() &&
             watched == m_popupTrigger) {

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

        m_launcherList->gridLayout()->setMode(
            newFormFactor == Plasma::Horizontal
                ? IconGridLayout::PreferRows
                : IconGridLayout::PreferColumns);

        // Ignore maxRowsOrColumns / maxRowsOrColumnsForced when in planar
        // form factor.
        if (newFormFactor == Plasma::Planar) {
            m_launcherList->gridLayout()->setMaxRowsOrColumns(0);
            m_launcherList->gridLayout()->setMaxRowsOrColumnsForced(false);
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

        bool lock = immutability() != Plasma::Mutable;

        m_launcherList->setLocked(lock);
        if (m_popup) {
            m_popup->launcherList()->setLocked(lock);
        }
    }
}

void Quicklaunch::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    showContextMenu(event->screenPos(), 0, -1);
}

void Quicklaunch::configChanged()
{
    KConfigGroup config = this->config();

    // Migrate old configuration keys
    if (config.hasKey("dialogIconSize") ||
        config.hasKey("iconSize") ||
        config.hasKey("iconUrls") ||
        config.hasKey("showIconNames") ||
        config.hasKey("visibleIcons")) {

        // Migrate from Quicklaunch 0.1 config format
        QStringList iconUrls = config.readEntry("iconUrls", QStringList());

        int visibleIcons =
            qBound(-1, config.readEntry("visibleIcons", -1), iconUrls.size());

        bool showIconNames = config.readEntry("showIconNames", false);

        config.deleteEntry("dialogIconSize");
        config.deleteEntry("iconSize");
        config.deleteEntry("iconUrls");
        config.deleteEntry("showIconNames");
        config.deleteEntry("visibleIcons");

        QStringList launchers;
        QStringList launchersOnPopup;

        for (int i = 0; i < iconUrls.size(); i++) {

            if (visibleIcons == -1 || i < visibleIcons) {
                launchers.append(iconUrls.at(i));
            } else {
                launchersOnPopup.append(iconUrls.at(i));
            }
        }

        config.writeEntry("launchers", launchers);
        config.writeEntry("launchersOnPopup", launchersOnPopup);
        config.writeEntry("launcherNamesVisible", showIconNames);
    }

    if (config.hasKey("icons") ||
        config.hasKey("dialogIcons") ||
        config.hasKey("dialogEnabled") ||
        config.hasKey("iconNamesVisible")) {

        // Migrate from quicklaunch 0.2 config format
        if (config.hasKey("icons")) {
            if (!config.hasKey("launchers")) {
                config.writeEntry(
                    "launchers",
                    config.readEntry("icons", QStringList()));
            }
            config.deleteEntry("icons");
        }

        if (config.hasKey("dialogIcons")) {
            if (!config.hasKey("launchersOnPopup")) {
                config.writeEntry(
                    "launchersOnPopup",
                    config.readEntry("dialogIcons", QStringList()));
            }
            config.deleteEntry("dialogIcons");
        }

        if (config.hasKey("dialogEnabled")) {
            if (!config.hasKey("popupEnabled")) {
                config.writeEntry(
                    "popupEnabled",
                    config.readEntry("dialogEnabled", false));
            }
            config.deleteEntry("dialogEnabled");
        }

        if (config.hasKey("iconNamesVisible")) {
            if (!config.hasKey("launcherNamesVisible")) {
                config.writeEntry(
                    "launcherNamesVisible",
                    config.readEntry("iconNamesVisible", false));
            }
            config.deleteEntry("iconNamesVisible");
        }
    }

    // Read new configuration
    const bool maxRowsOrColumnsForced = config.readEntry("maxRowsOrColumnsForced", false);
    const int maxRowsOrColumns = config.readEntry("maxRowsOrColumns", 0);
    const bool launcherNamesVisible = config.readEntry("launcherNamesVisible", false);
    const bool popupEnabled = config.readEntry("popupEnabled", false);

    QList<LauncherData> newLaunchers;
    QList<LauncherData> newLaunchersOnPopup;

    { // Read item lists
        QStringList newLauncherUrls =
            config.readEntry("launchers", QStringList());
        QStringList newLaunchersOnPopupUrls =
            config.readEntry("launchersOnPopup", QStringList());

        if (newLauncherUrls.isEmpty() && newLaunchersOnPopupUrls.isEmpty()) {
            QStringList defaultApps;
            defaultApps << "konqbrowser" << "dolphin" << "kopete";

            Q_FOREACH (const QString &defaultApp, defaultApps) {
                KService::Ptr service = KService::serviceByStorageId(defaultApp);
                if (service && service->isValid()) {
                    QString path = service->entryPath();

                    if (!path.isEmpty() && QDir::isAbsolutePath(path)) {
                        newLauncherUrls.append(path);
                    }
                }
            }
        }

        Q_FOREACH(QString launcherUrl, newLauncherUrls) {
            newLaunchers.append(LauncherData(launcherUrl));
        }

        Q_FOREACH(QString launcherUrl, newLaunchersOnPopupUrls) {
            newLaunchersOnPopup.append(LauncherData(launcherUrl));
        }
    }

    // Apply new configuration
    m_launcherList->gridLayout()->setMaxRowsOrColumnsForced(maxRowsOrColumnsForced);
    m_launcherList->gridLayout()->setMaxRowsOrColumns(maxRowsOrColumns);
    m_launcherList->setLauncherNamesVisible(launcherNamesVisible);

    // Make sure the popup is in a proper state for the new configuration
    if (m_popup == 0 && (popupEnabled || !newLaunchersOnPopup.empty())) {
        initPopup();
    }
    else if (m_popup != 0 && (!popupEnabled && newLaunchersOnPopup.empty())) {
        deletePopup();
    }

    { // Check if any of the launchers in the main area have changed
        bool launchersChanged = false;

        if (newLaunchers.length() != m_launcherList->launcherCount()) {
            launchersChanged = true;
        } else {
            for (int i = 0; i < newLaunchers.length(); i++) {
                if (newLaunchers.at(i) != m_launcherList->launcherAt(i)) {
                    launchersChanged = true;
                }
            }
        }

        if (launchersChanged) {
            // Re-populate primary launchers
            m_launcherList->clear();
            m_launcherList->insert(-1, newLaunchers);
        }
    }

    { // Check if any of the launchers in the popup have changed
        bool popupLaunchersChanged = false;

        int currentPopupLauncherCount =
            m_popup == 0 ? 0 : m_popup->launcherList()->launcherCount();

        if (newLaunchersOnPopup.length() != currentPopupLauncherCount) {
            popupLaunchersChanged = true;
        }
        else if (m_popup != 0) {
            for (int i = 0; i < newLaunchersOnPopup.length(); i++) {
                if (newLaunchersOnPopup.at(i) != m_popup->launcherList()->launcherAt(i)) {
                    popupLaunchersChanged = true;
                }
            }
        }

        if (popupLaunchersChanged && !newLaunchersOnPopup.empty()) {
            // Re-populate popup launchers
            m_popup->launcherList()->clear();
            m_popup->launcherList()->insert(-1, newLaunchersOnPopup);
        }
    }
}

void Quicklaunch::onConfigAccepted()
{
    const bool maxRowsOrColumnsForced = uiConfig.forceMaxRowsOrColumnsCheckBox->isChecked();
    const int maxRowsOrColumns = uiConfig.maxRowsOrColumnsSpinBox->value();
    const bool launcherNamesVisible = uiConfig.launcherNamesVisibleCheckBox->isChecked();
    const bool popupEnabled = uiConfig.popupEnabledCheckBox->isChecked();

    KConfigGroup config = this->config();
    bool changed = false;

    if (maxRowsOrColumnsForced !=
          m_launcherList->gridLayout()->maxRowsOrColumnsForced()) {

        m_launcherList->gridLayout()->setMaxRowsOrColumnsForced(maxRowsOrColumnsForced);
        config.writeEntry("maxRowsOrColumnsForced", maxRowsOrColumnsForced);
        changed = true;
    }

    if (maxRowsOrColumns != m_launcherList->gridLayout()->maxRowsOrColumns()) {
        m_launcherList->gridLayout()->setMaxRowsOrColumns(maxRowsOrColumns);
        config.writeEntry("maxRowsOrColumns", maxRowsOrColumns);
        changed = true;
    }

    if (launcherNamesVisible != m_launcherList->launcherNamesVisible()) {
        m_launcherList->setLauncherNamesVisible(launcherNamesVisible);
        config.writeEntry("launcherNamesVisible", launcherNamesVisible);
        changed = true;
    }

    if (popupEnabled != (m_popup != 0)) {

        if (m_popup == 0) {
            initPopup();
        } else {
            while (m_popup->launcherList()->launcherCount() > 0) {
                m_launcherList->insert(-1, m_popup->launcherList()->launcherAt(0));
            }
        }

        config.writeEntry("popupEnabled", popupEnabled);
        changed = true;
    }

    if (changed) {
        Q_EMIT configNeedsSaving();
    }
}

void Quicklaunch::onLaunchersChanged()
{
    // Save new launcher lists
    QStringList launchers;
    QStringList launchersOnPopup;

    for (int i = 0; i < m_launcherList->launcherCount(); i++) {
        launchers.append(m_launcherList->launcherAt(i).url().prettyUrl());
    }

    if (m_popup) {
        for (int i = 0; i < m_popup->launcherList()->launcherCount(); i++) {
            // XXX: Is prettyUrl() really needed?
            launchersOnPopup.append(m_popup->launcherList()->launcherAt(i).url().prettyUrl());
        }
    }

    KConfigGroup config = this->config();

    config.writeEntry("launchers", launchers);
    config.writeEntry("launchersOnPopup", launchersOnPopup);
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
    Q_ASSERT(m_currentLauncherList);
    Q_ASSERT(m_currentLauncherIndex != -1);

    m_currentLauncherList->removeAt(m_currentLauncherIndex);
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

        if (m_currentLauncherList) {
            m_currentLauncherList->insert(
                m_currentLauncherIndex, KUrl::fromPath(programPath));
        }
        else {
            m_launcherList->insert(
                -1, KUrl::fromPath(programPath));
        }
    }
}

void Quicklaunch::showContextMenu(
    const QPoint& screenPos,
    LauncherList *component,
    int iconIndex)
{
    if (m_addIconAction == 0) {
        initActions();
    }

    m_currentLauncherList = component;
    m_currentLauncherIndex = iconIndex;

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

    m_currentLauncherList = 0;
    m_currentLauncherIndex = -1;
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
    m_popup->launcherList()->installEventFilter(this);
    connect(m_popup->launcherList(), SIGNAL(launchersChanged()), SLOT(onLaunchersChanged()));

    // Initialize popup trigger
    m_popupTrigger = new IconWidget(this);
    m_popupTrigger->setContentsMargins(0, 0, 0, 0);
    m_popupTrigger->setPreferredWidth(KIconLoader::SizeSmall);
    m_popupTrigger->setPreferredHeight(KIconLoader::SizeSmall);
    m_popupTrigger->setAcceptDrops(true);
    m_popupTrigger->installEventFilter(this);
    ToolTipManager::self()->registerWidget(m_popupTrigger);
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

    bool popupHidden = m_popup->isHidden();

    // Set icon
    switch (location()) {
        case Plasma::TopEdge:
            m_popupTrigger->setSvg(
                "widgets/arrows", popupHidden ? "down-arrow" : "up-arrow");
            break;
        case Plasma::LeftEdge:
            m_popupTrigger->setSvg(
                "widgets/arrows", popupHidden ? "right-arrow" : "left-arrow");
            break;
        case Plasma::RightEdge:
            m_popupTrigger->setSvg(
                "widgets/arrows", popupHidden ? "left-arrow" : "right-arrow");
            break;
        default:
            m_popupTrigger->setSvg(
                "widgets/arrows", popupHidden ? "up-arrow" : "down-arrow");
    }

    // Set tooltip
    ToolTipContent toolTipContent;
    toolTipContent.setSubText(
        popupHidden ? i18n("Show hidden icons") : i18n("Hide icons"));
    ToolTipManager::self()->setContent(m_popupTrigger, toolTipContent);
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
}

#include "quicklaunch.moc"

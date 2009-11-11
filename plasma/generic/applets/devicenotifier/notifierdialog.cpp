/*
    Copyright 2008 by Alexis Ménard <darktears31@gmail.com>
    Copyright 2009 by Giulio Camuffo <giuliocamuffo@gmail.com>
    Copyright 2009 by Jacopo De Simoi <wilderkde@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "notifierdialog.h"

//Qt
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>
#include <QMetaEnum>
#include <QGraphicsLinearLayout>
#include <QGraphicsSceneContextMenuEvent>

//KDE
#include <KDebug>
#include <KDiskFreeSpaceInfo>
#include <KGlobalSettings>

//plasma
#include <Plasma/Theme>
#include <Plasma/ItemBackground>
#include <Plasma/ScrollWidget>
#include <Plasma/Separator>

//solid
#include <solid/device.h>
#include <solid/opticaldisc.h>
#include <solid/storageaccess.h>
#include <solid/opticaldrive.h>
#include <solid/deviceinterface.h>

//own
#include "devicenotifier.h"
#include "deviceitem.h"

using namespace Notifier;

NotifierDialog::NotifierDialog(DeviceNotifier * notifier, QObject *parent)
    : QObject(parent),
      m_widget(0),
      m_notifier(notifier)
{
    buildDialog();

    m_hideItem = new QAction(this);
    m_hideItem->setCheckable(true);
    m_showAll = new QAction(i18n("Show hidden devices"), this);
    m_showAll->setCheckable(true);
    m_separator = new QAction(this);
    m_separator->setSeparator(true);
    connect(m_hideItem, SIGNAL(triggered()), this, SLOT(setItemVisibility()));
    connect(m_showAll, SIGNAL(toggled(bool)), this, SIGNAL(globalVisibilityChanged(bool)));

    m_clearItemBackgroundTargetTimer.setSingleShot(true);
    m_clearItemBackgroundTargetTimer.setInterval(100);
    connect(&m_clearItemBackgroundTargetTimer, SIGNAL(timeout()), this, SLOT(clearItemBackgroundTarget()));
    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), this, SLOT(updateColorsLater()));
}

NotifierDialog::~NotifierDialog()
{

}

QGraphicsWidget *NotifierDialog::dialog()
{
    return m_widget;
}

int NotifierDialog::searchOrCreateDeviceCategory(const QString &categoryName)
{
    for (int i = 0; i < m_deviceLayout->count(); ++i) {
        Plasma::Label *category = dynamic_cast<Plasma::Label *>(m_deviceLayout->itemAt(i));
        if (category && category->text() == categoryName) {
            if (i != 1) {
                int j = i - 1;
                Plasma::Separator *nextSeparator = 0;
                while (!nextSeparator && j < m_deviceLayout->count()) {
                    QGraphicsLayoutItem *item = m_deviceLayout->itemAt(j);
                    m_deviceLayout->removeAt(j);
                    m_deviceLayout->insertItem(j - (i - 1), item);

                    ++j;
                    nextSeparator = dynamic_cast<Plasma::Separator *>(m_deviceLayout->itemAt(j));
                }
            }
            return 1;
        }
    }

    Plasma::Separator *separator = new Plasma::Separator();
    separator->setOrientation(Qt::Horizontal);
    separator->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    m_deviceLayout->insertItem(0, separator);

    Plasma::Label *category = new Plasma::Label();
    category->setText(categoryName);
    category->setAlignment(Qt::AlignRight);
    category->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    updateCategoryColors(category);
    m_deviceLayout->insertItem(1, category);

    return 1;
}

void NotifierDialog::insertDevice(const QString &udi)
{
    if (udi.isNull()) {
        return;
    }

    DeviceItem *devItem = new DeviceItem(udi);
    connect(devItem, SIGNAL(leftActionActivated(DeviceItem *)), this, SLOT(leftActionActivated(DeviceItem *)));
    connect(devItem, SIGNAL(actionActivated(DeviceItem *, const QString &, const QString &)),
            this, SLOT(actionActivated(DeviceItem *, const QString &, const QString &)));
    connect(devItem, SIGNAL(activated(DeviceItem *)), this, SLOT(deviceActivated(DeviceItem *)));
    connect(devItem, SIGNAL(highlightActionItem(QGraphicsItem *)), this, SLOT(highlightDeviceAction(QGraphicsItem*)));
    devItem->installEventFilter(this);

    devItem->setData(SolidUdiRole, udi);
    devItem->setData(VisibilityRole, true);

    //search or create the category for inserted device
    Solid::Device device(udi);
    QString categoryOfInsertedDevice = getCategoryNameOfDevice(device);
    int index = searchOrCreateDeviceCategory(categoryOfInsertedDevice);

    m_deviceLayout->insertItem(index + 1, devItem);

    collapseDevices();
    resetSelection();
    updateMainLabelText();
}

void NotifierDialog::insertAction(const QString& udi, const QString& action)
{
    DeviceItem *devItem = itemForUdi(udi);
    if (devItem) {
        devItem->addAction(action);
    }
}

void NotifierDialog::clearItemBackgroundTarget()
{
    m_itemBackground->setTargetItem(0);
}

void NotifierDialog::itemHoverEnter(DeviceItem * item)
{
    // make sure the popup is not only shown, but doesn't automatically retract on us when we're
    // mousing around in it
    m_notifier->showPopup(0);

    item->setHovered(true);
    if (item->isCollapsed()) {
        m_clearItemBackgroundTargetTimer.stop();
        m_itemBackground->setTargetItem(item);
    } else {
        m_clearItemBackgroundTargetTimer.start();
    }

    updateFreeSpace(item);
}

void NotifierDialog::itemHoverLeave(DeviceItem * item)
{
    if (item->isCollapsed()) {
        item->setHovered(false);
        m_clearItemBackgroundTargetTimer.start();
        if (m_selectedItemBackground->targetItem() == item) {
            m_selectedItemBackground->setTargetItem(0);
        }
    }
}

bool NotifierDialog::eventFilter(QObject* obj, QEvent *event)
{
    if (m_notifier->isPopupShowing() && event->type() == QEvent::GraphicsSceneContextMenu) {
        QGraphicsSceneContextMenuEvent *contextEvent = static_cast<QGraphicsSceneContextMenuEvent *>(event);
        setMenuActionsAt(contextEvent->scenePos());
    }

    DeviceItem *item = qobject_cast<DeviceItem*>(obj);
    if (item) {
        switch (event->type()) {
            case QEvent::GraphicsSceneHoverLeave:
                itemHoverLeave(item);
                break;
            case QEvent::GraphicsSceneHoverEnter:
                itemHoverEnter(item);
                break;
            default:
                break;
        }
    }

    QGraphicsWidget *widget = qobject_cast<QGraphicsWidget*>(obj);
    if (widget == m_widget && event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        switch (keyEvent->key()) {
            case Qt::Key_Down:
            case Qt::Key_Right:
                selectNextItem();
                break;
            case Qt::Key_Up:
            case Qt::Key_Left:
                selectPreviousItem();
                break;
            case Qt::Key_Enter:
            case Qt::Key_Return:
            case Qt::Key_Space:
                if (hoveredAction()) {
                    hoveredItem()->actionClicked(hoveredAction());
                } else if (hoveredItem()) {
                    hoveredItem()->clicked();
                }
                break;
            default:
                break;
        }
    }

    return false;
}

void NotifierDialog::selectNextItem()
{
    DeviceItem *item = hoveredItem();
    Plasma::IconWidget *action = hoveredAction();

    if (action || (item &&  !item->isCollapsed())) {
        // We are hovering an action or an expanded device with no action selected;
        // we need to let the item select the next action
        // if no next action is available, we will continue selecting the next item.
        if (item->selectNextAction(action)) {
            return;
        }
    }

    DeviceItem *firstDevice = 0;
    DeviceItem *nextDevice = 0;
    bool grabNext = false;

    for (int i = 0; i < m_deviceLayout->count(); ++i) {
        DeviceItem *currentItem = dynamic_cast<DeviceItem *>(m_deviceLayout->itemAt(i));
        if (currentItem && !firstDevice) {
            firstDevice = currentItem;
        }
        if (currentItem && grabNext) {
            nextDevice = currentItem;
            grabNext = false;
        }
        if (currentItem && currentItem == item) {
            grabNext = true;
        }

    }

    if (!nextDevice) {
        nextDevice = firstDevice;
    }

    if (item) {
        itemHoverLeave(item);
    }
    if (nextDevice) {
        itemHoverEnter(nextDevice);
        m_devicesScrollWidget->ensureItemVisible(nextDevice);

    }

}

void NotifierDialog::selectPreviousItem()
{
    DeviceItem *item = hoveredItem();
    Plasma::IconWidget *action = hoveredAction();

    if (action || (item &&  !item->isCollapsed())) {
        // We are hovering an action or an expanded device with no action selected;
        // we need to let the item select the previous action
        // if no previous action is available, we will continue selecting the previous item.
        if (item->selectPreviousAction(action)) {
            return;
        }
    }

    DeviceItem *previousDevice = 0;
    DeviceItem *deviceAbove = 0;

    for (int i = 0; i < m_deviceLayout->count(); ++i) {
        DeviceItem *currentItem = dynamic_cast<DeviceItem *>(m_deviceLayout->itemAt(i));

        if (currentItem && currentItem == item) {
            previousDevice = deviceAbove;
        }

        if (currentItem) {
            deviceAbove = currentItem;
        }

    }

    if (!previousDevice) {
        previousDevice = deviceAbove;
    }

    if (item) {
        itemHoverLeave(item);
    }
    if (previousDevice) {
        itemHoverEnter(previousDevice);
        m_devicesScrollWidget->ensureItemVisible(previousDevice);
        if (!previousDevice->isCollapsed()) {
            previousDevice->selectPreviousAction(0, true);
        }
    }

}


Plasma::IconWidget* NotifierDialog::hoveredAction()
{
    if (m_itemBackground) {
        return dynamic_cast<Plasma::IconWidget*>(m_itemBackground->targetItem());
    }

    return 0;
}

DeviceItem* NotifierDialog::hoveredItem()
{
    if (m_itemBackground) {
        DeviceItem* devItem = dynamic_cast<DeviceItem*>(m_itemBackground->targetItem());
        if (devItem) {
            return devItem;
        }
    }

    if (m_selectedItemBackground) {
        return dynamic_cast<DeviceItem*>(m_selectedItemBackground->targetItem());
    }

    return 0;
}


void NotifierDialog::setMounted(bool mounted, const QString &udi)
{
    DeviceItem *item = itemForUdi(udi);
    item->setMounted(mounted);
    updateFreeSpace(item);
}

void NotifierDialog::setDeviceLeftAction(const QString &udi, DeviceItem::LeftActions action)
{
    DeviceItem *item = itemForUdi(udi);
    item->setLeftAction(action);
}

void NotifierDialog::setDeviceData(const QString &udi, QVariant data, int role)
{
    DeviceItem *item = itemForUdi(udi);
    if (item) {
        item->setData(role, data);
        item->update();
    }
}

QVariant NotifierDialog::getDeviceData(const QString &udi, int role)
{
    DeviceItem *item = itemForUdi(udi);
    if (!item) {
        return QVariant();
    }

    return item->data(role);
}

void NotifierDialog::removeDevice(const QString &udi)
{
    DeviceItem *item = itemForUdi(udi);

    if (!item) {
        return;
    }

    disconnect(item, 0, this, 0);
    item->removeEventFilter(this);

    resetSelection();
    m_deviceLayout->removeItem(item);
    item->deleteLater();

    for (int i = 0; i < m_deviceLayout->count(); ++i) {
        Plasma::Separator *separator = dynamic_cast<Plasma::Separator *>(m_deviceLayout->itemAt(i));
        if (separator) {
            DeviceItem *nextItem = dynamic_cast<DeviceItem *>(m_deviceLayout->itemAt(i + 2));
            if (!nextItem) {
                m_deviceLayout->removeAt(i);
                QGraphicsLayoutItem *category = m_deviceLayout->itemAt(i);
                m_deviceLayout->removeAt(i);
                delete separator;
                delete category;
                --i;
            }
        }
    }

    Solid::Device solidDevice(udi);
    if (solidDevice.is<Solid::StorageVolume>()) {
        Solid::StorageAccess *access = solidDevice.as<Solid::StorageAccess>();
        if (access) {
            disconnect(access, 0, this, 0);
        }
    }

    updateMainLabelText();

    m_devicesScrollWidget->widget()->adjustSize();
}

void NotifierDialog::buildDialog()
{
    m_widget = new QGraphicsWidget(m_notifier);
    m_widget->installEventFilter(this);
    m_widget->setFocusPolicy(Qt::ClickFocus);

    QGraphicsLinearLayout *l_layout = new QGraphicsLinearLayout(Qt::Vertical, m_widget);
    l_layout->setSpacing(0);

    Plasma::IconWidget *icon = new Plasma::IconWidget(m_widget);
    icon->setIcon(KIcon("emblem-mounted"));
    icon->setMaximumHeight(KIconLoader::SizeMedium);
    icon->setMinimumHeight(KIconLoader::SizeMedium);
    icon->setAcceptHoverEvents(false);
    m_mainLabel = new Plasma::Label(m_widget);
    m_mainLabel->setMaximumHeight(KIconLoader::SizeMedium);
    m_mainLabel->nativeWidget()->setWordWrap(false);
    m_mainLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    QGraphicsLinearLayout *l_layout2 = new QGraphicsLinearLayout;
    l_layout2->setSpacing(0);
    l_layout2->setOrientation(Qt::Horizontal);

    l_layout2->addStretch();
    l_layout2->addItem(icon);
    l_layout2->addItem(m_mainLabel);
    l_layout2->addStretch();

    QGraphicsWidget *titleWidget = new QGraphicsWidget();
    titleWidget->setLayout(l_layout2);

    l_layout->addItem(titleWidget);

    m_devicesScrollWidget = new Plasma::ScrollWidget(m_widget);
    QGraphicsWidget *devicesWidget = new QGraphicsWidget(m_devicesScrollWidget);
    m_devicesScrollWidget->setWidget(devicesWidget);
    m_devicesScrollWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_deviceLayout = new QGraphicsLinearLayout(Qt::Vertical, devicesWidget);
    m_deviceLayout->setContentsMargins(0, 0, 0, 8);
    devicesWidget->setLayout(m_deviceLayout);

    l_layout->addItem(m_devicesScrollWidget);

    m_itemBackground = new Plasma::ItemBackground(devicesWidget);
    m_itemBackground->hide();
    connect(m_itemBackground, SIGNAL(animationStep(qreal)), this, SLOT(itemBackgroundMoving(qreal)));

    m_selectedItemBackground = new Plasma::ItemBackground(devicesWidget);
    connect(m_selectedItemBackground, SIGNAL(targetItemReached(QGraphicsItem*)),
            this, SLOT(selectedItemAnimationComplete(QGraphicsItem*)));
    m_selectedItemBackground->hide();

    devicesWidget->adjustSize();
    updateMainLabelText();

    m_widget->setLayout(l_layout);
    m_widget->setMinimumSize(250, 300);
}

void NotifierDialog::storageTeardownDone(Solid::ErrorType error, QVariant errorData, const QString & udi)
{
    if (error && errorData.isValid()) {
        m_notifier->showErrorMessage(i18n("Could not unmount the device.\nOne or more files on this device are open within an application."));
    } else {
        m_notifier->changeNotifierIcon("dialog-ok");
        m_notifier->update();
        QTimer::singleShot(5000, this, SLOT(resetNotifierIcon()));
    }

    //show the message only one time
    disconnect(sender(), SIGNAL(teardownDone(Solid::ErrorType, QVariant, const QString &)),
               this, SLOT(storageTeardownDone(Solid::ErrorType, QVariant, const QString &)));
    itemForUdi(udi)->setReady();

}

void NotifierDialog::storageEjectDone(Solid::ErrorType error, QVariant errorData, const QString &udi)
{
    Q_UNUSED(udi);

    if (error && errorData.isValid()) {
        m_notifier->showErrorMessage(i18n("Cannot eject the disc.\nOne or more files on this disc are open within an application."));
    } else {
        m_notifier->changeNotifierIcon("dialog-ok");
        m_notifier->update();
        QTimer::singleShot(2000, this, SLOT(resetNotifierIcon()));
    }

    //show the message only one time
    disconnect(sender(), SIGNAL(ejectDone(Solid::ErrorType, QVariant, const QString &)),
               this, SLOT(storageEjectDone(Solid::ErrorType, QVariant, const QString &)));
}

void NotifierDialog::storageSetupDone(Solid::ErrorType error, QVariant errorData, const QString &udi)
{
     if (error && errorData.isValid()) {
        m_notifier->showErrorMessage(i18n("Cannot mount the disc."));
    } else {
        m_notifier->changeNotifierIcon("dialog-ok");
        m_notifier->update();
        QTimer::singleShot(2000, this, SLOT(resetNotifierIcon()));
    }

    //show the message only one time
    disconnect(sender(), SIGNAL(setupDone(Solid::ErrorType, QVariant, const QString &)),
               this, SLOT(storageSetupDone(Solid::ErrorType, QVariant, const QString &)));
    itemForUdi(udi)->setReady();
}

DeviceItem *NotifierDialog::itemForUdi(const QString &udi) const
{
    for (int i=0; i<m_deviceLayout->count(); i++) {
        DeviceItem* item = dynamic_cast<DeviceItem *>(m_deviceLayout->itemAt(i));
        if (item && item->udi() == udi) {
            return item;
        }
    }

    kDebug() << "We should not be here!";
    return 0;
}

void NotifierDialog::collapseDevices()
{
    for (int i = 0; i < m_deviceLayout->count(); ++i) {
        DeviceItem *devItem = dynamic_cast<DeviceItem *>(m_deviceLayout->itemAt(i));
        if (devItem) {
            devItem->collapse();
        }
    }
}

void NotifierDialog::leftActionActivated(DeviceItem *item)
{
    Solid::Device device(item->udi());

    if (item->leftAction() == DeviceItem::Umount) {
        if (device.is<Solid::OpticalDisc>()) {
            Solid::OpticalDrive *drive = device.parent().as<Solid::OpticalDrive>();
            if (drive) {
                item->setBusy();
                connect(drive, SIGNAL(ejectDone(Solid::ErrorType, QVariant, const QString &)),
                        this, SLOT(storageEjectDone(Solid::ErrorType, QVariant, const QString &)));
                drive->eject();
            }
        } else if (device.is<Solid::StorageVolume>()) {
            Solid::StorageAccess *access = device.as<Solid::StorageAccess>();
            if (access && access->isAccessible()) {
                item->setBusy();
                connect(access, SIGNAL(teardownDone(Solid::ErrorType, QVariant, const QString &)),
                        this, SLOT(storageTeardownDone(Solid::ErrorType, QVariant, const QString &)));
                access->teardown();
            }
        }
    } else if (item->leftAction() == DeviceItem::Mount && device.is<Solid::StorageAccess>()) {
        Solid::StorageAccess *access = device.as<Solid::StorageAccess>();

        // only unmounted devices
        if (access && !access->isAccessible()) {
            item->setBusy();
            connect(access, SIGNAL(setupDone(Solid::ErrorType, QVariant, const QString &)),
                    this, SLOT(storageSetupDone(Solid::ErrorType, QVariant , const QString &)));
            access->setup();
        }
    }
}

void NotifierDialog::deviceActivated(DeviceItem *item)
{
    m_devicesScrollWidget->ensureItemVisible(item);

    for (int i = 0; i < m_deviceLayout->count(); ++i) {
        DeviceItem *devItem = dynamic_cast<DeviceItem *>(m_deviceLayout->itemAt(i));
        if (devItem && devItem != item) {
            devItem->collapse();
            devItem->setHovered(false);
        }
    }

    m_selectedItemBackground->setTarget(m_itemBackground->geometry());
    m_selectedItemBackground->show();
    m_itemBackground->setTargetItem(0);
    m_selectedItemBackground->setTargetItem(item);

    emit deviceSelected();
}

void NotifierDialog::highlightDeviceAction(QGraphicsItem* item)
{
    m_clearItemBackgroundTargetTimer.stop();
    m_itemBackground->setTargetItem(item);
}

void NotifierDialog::actionActivated(DeviceItem *item, const QString &udi, const QString &action)
{
    item->collapse();

    QStringList desktopFiles;
    desktopFiles.append(action);
    kDebug() << "DeviceNotifier:: call Solid Ui Server with params :" << udi << "," << desktopFiles;
    QDBusInterface soliduiserver("org.kde.kded", "/modules/soliduiserver", "org.kde.SolidUiServer");
    QDBusReply<void> reply = soliduiserver.call("showActionsDialog", udi, desktopFiles);

    emit actionSelected();
}

void NotifierDialog::selectedItemAnimationComplete(QGraphicsItem *item)
{
    DeviceItem *devItem = dynamic_cast<DeviceItem *>(item);
    if (devItem && devItem->isCollapsed() && !devItem->hovered()) {
        m_selectedItemBackground->setTargetItem(0);
    }
}

void NotifierDialog::itemBackgroundMoving(qreal step)
{
    for (int i = 0; i < m_deviceLayout->count(); ++i) {
        DeviceItem *item = dynamic_cast<DeviceItem *>(m_deviceLayout->itemAt(i));
        if (item && item->hovered() && item->isCollapsed()) {
            qreal normalizedDistance = qAbs(m_itemBackground->pos().ry()-item->pos().ry()) / qreal (item->rect().height());
            qreal saturatedDistance = 1.-qMin(normalizedDistance*1.5, 1.);
            item->setHoverDisplayOpacity(saturatedDistance*step);
            return;
        }
    }
}

QString NotifierDialog::getCategoryNameOfDevice(const Solid::Device &device)
{
    int index = Solid::DeviceInterface::staticMetaObject.indexOfEnumerator("Type");
    QMetaEnum typeEnum = Solid::DeviceInterface::staticMetaObject.enumerator(index);
    for (int i = typeEnum.keyCount() - 1 ; i > 0; i--) {
        Solid::DeviceInterface::Type type = (Solid::DeviceInterface::Type)typeEnum.value(i);
        const Solid::DeviceInterface *interface = device.asDeviceInterface(type);
        if (interface) {
            return Solid::DeviceInterface::typeDescription(type);
        }
    }

    return 0;
}

void NotifierDialog::resetNotifierIcon()
{
    m_notifier->changeNotifierIcon();
    m_notifier->update();
}

void NotifierDialog::resetSelection()
{
    m_itemBackground->hide();
    m_itemBackground->setTargetItem(0);

    for (int i = 0; i < m_deviceLayout->count(); ++i) {
        DeviceItem *item = dynamic_cast<DeviceItem *>(m_deviceLayout->itemAt(i));
        if (item && item->hovered()) {
            item->setHovered(false);
        }
    }
}

void NotifierDialog::updateFreeSpace(DeviceItem *item)
{
    if (item->isMounted()) {
        Solid::Device device(item->udi());
        Solid::StorageAccess *access = device.as<Solid::StorageAccess>();
        if (access) {
            KDiskFreeSpaceInfo info = KDiskFreeSpaceInfo::freeSpaceInfo(access->filePath());
            if (info.size()) {
                item->setFreeSpace(info.available(), info.size());
            }
        }
    }
}

void NotifierDialog::setMenuActionsAt(QPointF scenePos)
{
    m_showAll->setVisible(m_notifier->areThereHiddenDevices());

    bool hideItemVisible = false;
    if (m_devicesScrollWidget->geometry().contains(m_widget->mapFromScene(scenePos))) {
        QGraphicsWidget *w = m_devicesScrollWidget->widget();
        for (int i = 0; i <  m_deviceLayout->count(); ++i) {
            DeviceItem *devItem = dynamic_cast<DeviceItem *>(m_deviceLayout->itemAt(i));
            if (devItem && devItem->geometry().contains(w->mapFromScene(scenePos))) {
                hideItemVisible = true;
                QString name = devItem->data(Qt::DisplayRole).toString();
                QString udi = devItem->data(SolidUdiRole).toString();
                m_hideItem->setChecked(!devItem->data(VisibilityRole).toBool());
                m_hideItem->setText(i18nc("Hide a device", "Hide %1", name));
                m_hideItem->setData(udi);
                break;
            }
        }
    }
    m_hideItem->setVisible(hideItemVisible);
}

QList<QAction *> NotifierDialog::contextualActions()
{
    //TODO: this works around the fact that we get hover events while the menu is up
    //      which we should really be ignoring; libplasma doesn't let us know when
    //      our context menu appears or disappears, however. interesting problem,
    //      but non-trivial: worth fixing?
    resetSelection();

    QList<QAction *> list;
    list << m_hideItem << m_showAll << m_separator;
    return list;
}

void NotifierDialog::setItemVisibility()
{
    QString udi = m_hideItem->data().toString();
    bool value = m_hideItem->isChecked();

    m_notifier->setDeviceVisibility(udi, !value);
}

void NotifierDialog::updateColorsLater()
{
    //This dance is needed to update our colors later than Plasma::Label
    QTimer::singleShot(0, this, SLOT(updateColors()));
}

void NotifierDialog::updateColors()
{
    for (int i = 0; i < m_deviceLayout->count(); ++i) {
        Plasma::Label *category = dynamic_cast<Plasma::Label *>(m_deviceLayout->itemAt(i));
        if (category) {
            updateCategoryColors(category);
        }
        DeviceItem *item = dynamic_cast<DeviceItem *>(m_deviceLayout->itemAt(i));
        if (item) {
            item->updateColors();
        }
    }
}

void NotifierDialog::updateCategoryColors(Plasma::Label * category)
{
    QPalette p = category->nativeWidget()->palette();
    QColor color = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
    color.setAlphaF(0.6);
    p.setColor(QPalette::Normal, QPalette::WindowText, color);
    p.setColor(QPalette::Inactive, QPalette::WindowText, color);
    category->nativeWidget()->setPalette(p);
    category->update();
}

void NotifierDialog::updateMainLabelText()
{
    if (m_deviceLayout->count() == 0) {
        m_mainLabel->setText(i18n("No devices plugged in."));
    } else {
        m_mainLabel->setText(i18n("Devices recently plugged in:"));
    }
}

#include "notifierdialog.moc"

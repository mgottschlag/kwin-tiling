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
#include <QLabel>
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
#include <Plasma/TextBrowser>

//solid
#include <solid/device.h>
#include <solid/opticaldisc.h>
#include <solid/storageaccess.h>
#include <solid/opticaldrive.h>
#include <solid/deviceinterface.h>

//own
#include "devicenotifier.h"
#include "deviceitem.h"

namespace Notifier
{

NotifierDialog::NotifierDialog(DeviceNotifier * notifier, QObject *parent)
    : QObject(parent),
      m_widget(0),
      m_notifier(notifier),
      m_deviceCount(0),
      m_collapsing(false)
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

    setMenuActionsAt(QPointF(0, 0));
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

    //Poor man's category separator
    //TODO: make own widget?

    Plasma::Separator *separator = new Plasma::Separator();
    separator->setOrientation(Qt::Horizontal);
    separator->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    m_deviceLayout->insertItem(0, separator);

    Plasma::Label *category = new Plasma::Label();
    category->setText(categoryName);
    category->setAlignment(Qt::AlignLeft);
    // The margins here are needed in order to have the category label
    // vertically aligned with device icons, which just looks better
    category->nativeWidget()->setContentsMargins(4+4, 0, 0, 0);
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

    ++m_deviceCount;
    Solid::Device device(udi);
    Solid::Device parentDevice(device.parentUdi());
    bool unpluggable = true;
    if (parentDevice.is<Solid::StorageDrive>()) {
        Solid::StorageDrive *drive = parentDevice.as<Solid::StorageDrive>();
        unpluggable = (drive->isHotpluggable() || drive->isRemovable());
    }
    DeviceItem *devItem = new DeviceItem(udi, unpluggable);
    connect(devItem, SIGNAL(leftActionActivated(DeviceItem*)), this, SLOT(leftActionActivated(DeviceItem*)));
    connect(devItem, SIGNAL(actionActivated(DeviceItem*,QString,QString)),
            this, SLOT(actionActivated(DeviceItem*,QString,QString)));
    connect(devItem, SIGNAL(activated(DeviceItem*)), this, SLOT(deviceActivated(DeviceItem*)));
    connect(devItem, SIGNAL(collapsed(DeviceItem*)), this, SLOT(deviceCollapsed(DeviceItem*)));
    connect(devItem, SIGNAL(highlightActionItem(QGraphicsItem*)), this, SLOT(highlightDeviceAction(QGraphicsItem*)));
    devItem->installEventFilter(this);

    devItem->setData(SolidUdiRole, udi);
    devItem->setData(VisibilityRole, true);

    //search or create the category for inserted device
    QString categoryOfInsertedDevice = getCategoryNameOfDevice(device);
    int index = searchOrCreateDeviceCategory(categoryOfInsertedDevice);

    m_deviceLayout->insertItem(index + 1, devItem);

    if (device.is<Solid::OpticalDisc>()) {
        Solid::OpticalDrive *drive = device.parent().as<Solid::OpticalDrive>();
        if (drive) {
            connect(drive, SIGNAL(ejectRequested(QString)),
                    this, SLOT(ejectRequested(QString)));
            connect(drive, SIGNAL(ejectDone(Solid::ErrorType,QVariant,QString)),
                    this, SLOT(storageEjectDone(Solid::ErrorType,QVariant,QString)));
        }
    } else if (device.is<Solid::StorageVolume>()) {
        Solid::StorageAccess *access = device.as<Solid::StorageAccess>();
        if (access ) {
            connect(access, SIGNAL(teardownRequested(QString)),
                    this, SLOT(teardownRequested(QString)));
            connect(access, SIGNAL(teardownDone(Solid::ErrorType,QVariant,QString)),
                    this, SLOT(storageTeardownDone(Solid::ErrorType,QVariant,QString)));
            connect(access, SIGNAL(setupRequested(QString)),
                    this, SLOT(setupRequested(QString)));
            connect(access, SIGNAL(setupDone(Solid::ErrorType,QVariant,QString)),
                    this, SLOT(storageSetupDone(Solid::ErrorType,QVariant,QString)));
        }
    }

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

QStringList NotifierDialog::deviceActions(const QString &udi) const
{
    return itemForUdi(udi)->actions();
}

void NotifierDialog::clearItemBackgroundTarget()
{
    m_itemBackground->setTargetItem(0);
}

void NotifierDialog::itemHoverEnter(DeviceItem *item)
{
    item->setHovered(true);
    if (item->isCollapsed()) {
        m_clearItemBackgroundTargetTimer.stop();
        m_collapsing = false;
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
            case QEvent::GraphicsSceneHoverMove:
                if (m_notifier->poppedUpInternally()) {
                    m_notifier->showPopup(DeviceNotifier::LONG_NOTIFICATION_TIMEOUT);
                }
                break;
            case QEvent::GraphicsSceneMousePress:
                m_notifier->keepPopupOpen();
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

    if (mounted) {
        QString parentUdi = Solid::Device(udi).parent().udi();
        foreach (DeviceItem* sibling, itemsForParentUdi(parentUdi)) {
            sibling->setSafelyRemovable(false);
        }
    } else {
        bool safelyRemovable = true;
        Solid::Device parent = Solid::Device(udi).parent();
        if (parent.is<Solid::StorageDrive>()) {
                safelyRemovable = !parent.as<Solid::StorageDrive>()->isInUse();
            }

        foreach (DeviceItem* sibling, itemsForParentUdi(parent.udi())) {
                sibling->setSafelyRemovable(safelyRemovable);
        }
    }

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

void NotifierDialog::expandDevice(const QString &udi)
{
    DeviceItem *item = itemForUdi(udi);
    if (item) {
        itemHoverEnter(item);
        item->expand();
        deviceActivated(item);
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

    expireStatusBar(udi);

    disconnect(item, 0, this, 0);
    item->removeEventFilter(this);

    resetSelection();
    m_deviceLayout->removeItem(item);
    item->deleteLater();
    --m_deviceCount;

    for (int i = 0; i < m_deviceLayout->count(); ++i) {
        Plasma::Separator *separator = dynamic_cast<Plasma::Separator *>(m_deviceLayout->itemAt(i));
        if (separator) {
            DeviceItem *nextItem = dynamic_cast<DeviceItem *>(m_deviceLayout->itemAt(i + 2));
            if (!nextItem) {
                m_deviceLayout->removeAt(i);
                QGraphicsLayoutItem *category = m_deviceLayout->itemAt(i);
                if (category) {
                    m_deviceLayout->removeAt(i);
                    delete category;
                }
                delete separator;
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
}

void NotifierDialog::removeAction(const QString &udi, const QString &action)
{
    itemForUdi(udi)->removeAction(action);
}

void NotifierDialog::buildDialog()
{
    m_widget = new QGraphicsWidget(m_notifier);
    m_widget->installEventFilter(this);
    m_widget->setFocusPolicy(Qt::ClickFocus);

    m_mainLayout = new QGraphicsLinearLayout(Qt::Vertical);
    m_mainLayout->setSpacing(0);

    m_mainLabel = new Plasma::Label();
    m_mainLabel->nativeWidget()->setWordWrap(false);
    m_mainLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_mainLabel->nativeWidget()->setContentsMargins(0, 0, 0, 4);

    m_mainLayout->addItem(m_mainLabel);
    m_mainLayout->setAlignment(m_mainLabel, Qt::AlignCenter);

    m_devicesScrollWidget = new Plasma::ScrollWidget();
    QGraphicsWidget *devicesWidget = new QGraphicsWidget();
    m_devicesScrollWidget->setWidget(devicesWidget);
    m_devicesScrollWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    //These hardcoded sizes should die one day
    m_devicesScrollWidget->setMinimumSize(240, 250);
    m_deviceLayout = new QGraphicsLinearLayout(Qt::Vertical);
    m_deviceLayout->setContentsMargins(0, 0, 0, 8);
    devicesWidget->setLayout(m_deviceLayout);

    m_mainLayout->addItem(m_devicesScrollWidget);
    m_mainLayout->setStretchFactor(m_devicesScrollWidget, 100);

    m_itemBackground = new Plasma::ItemBackground(devicesWidget);
    m_selectedItemBackground = new Plasma::ItemBackground(devicesWidget);
    m_itemBackground->hide();
    m_selectedItemBackground->hide();

    connect(m_itemBackground, SIGNAL(animationStep(qreal)), this, SLOT(itemBackgroundMoving(qreal)));
    connect(m_selectedItemBackground, SIGNAL(animationStep(qreal)), this, SLOT(itemBackgroundMoving(qreal)));

    m_statusWidget = new QGraphicsWidget();
    QGraphicsLinearLayout *statusLayout = new QGraphicsLinearLayout(Qt::Vertical);
    m_statusWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

    m_statusWidget->setLayout(statusLayout);
    Plasma::Separator *statusSeparator = new Plasma::Separator();
    statusSeparator->setOrientation(Qt::Horizontal);
    statusSeparator->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    statusLayout->addItem(statusSeparator);

    m_statusText = new Plasma::Label();
    m_statusText->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

    Plasma::IconWidget *closeButton = new Plasma::IconWidget();
    closeButton->setSvg("widgets/configuration-icons", "close");
    closeButton->setMaximumSize(closeButton->sizeFromIconSize(KIconLoader::SizeSmall));
    closeButton->setMinimumSize(closeButton->maximumSize());

    m_statusExpandButton = new Plasma::IconWidget();
    m_statusExpandButton->setMaximumSize(closeButton->sizeFromIconSize(KIconLoader::SizeSmall));
    m_statusExpandButton->setMinimumSize(closeButton->maximumSize());

    connect(closeButton, SIGNAL(clicked()), this, SLOT(dismissStatusBar()));
    connect(m_statusExpandButton, SIGNAL(clicked()), this, SLOT(triggerExpandStatusBar()));

    //WORKAROUND: We need to wrap our label layout in a QGW before adding it to our
    //            main layout to avoid issues with Qt not properly dealing with
    //            wordwrap enabled labels

    QGraphicsWidget *labelWidget = new QGraphicsWidget();
    labelWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    labelWidget->setContentsMargins(0, 0, 0, 0);

    QGraphicsLinearLayout *labelLayout = new QGraphicsLinearLayout(Qt::Horizontal);

    labelLayout->addItem(m_statusText);
    labelLayout->setAlignment(m_statusText, Qt::AlignTop);
    labelLayout->addItem(m_statusExpandButton);
    labelLayout->setAlignment(m_statusExpandButton, Qt::AlignTop);
    labelLayout->addItem(closeButton);
    labelLayout->setAlignment(closeButton, Qt::AlignTop);
    labelWidget->setLayout(labelLayout);

    m_statusDetailsText = new Plasma::TextBrowser();
    m_statusDetailsText->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    m_statusDetailsText->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_statusDetailsText->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    QFont font = m_statusDetailsText->font();
    font.setPointSize(KGlobalSettings::smallestReadableFont().pointSize());
    m_statusDetailsText->setFont(font);
    m_statusDetailsText->hide();

    statusLayout->addItem(labelWidget);

    devicesWidget->adjustSize();
    updateMainLabelText();

    m_widget->setLayout(m_mainLayout);
}

void NotifierDialog::dismissStatusBar()
{
    m_statusWidget->hide();
    m_mainLayout->removeItem(m_statusWidget);
    m_errorUdi = QString();
}

void NotifierDialog::expireStatusBar(const QString& udi)
{
    if (udi == m_errorUdi) {
        dismissStatusBar();
    }
}

void NotifierDialog::showStatusBarMessage(const QString & message, const QString &details, const QString &udi)
{
    m_statusText->setText(message);
    m_mainLayout->addItem(m_statusWidget);
    m_statusWidget->show();

    m_statusDetailsText->setText(details);
    if (details.isEmpty()) {
        m_statusExpandButton->hide();
    } else {
        m_statusExpandButton->show();
    }

    showStatusBarDetails(false);
    m_errorUdi = udi;
}

void NotifierDialog::triggerExpandStatusBar()
{
    showStatusBarDetails(!m_statusDetailsText->isVisible());
}

void NotifierDialog::showStatusBarDetails(bool show)
{
    Plasma::Svg* svg = new Plasma::Svg();
    svg->setImagePath("widgets/configuration-icons");
    svg->resize();

    if (show) {
        m_statusDetailsText->show();
        static_cast<QGraphicsLinearLayout*>(m_statusWidget->layout())->addItem(m_statusDetailsText);
        m_statusExpandButton->setIcon(QIcon(svg->pixmap("collapse")));
    } else {
        m_statusDetailsText->hide();
        static_cast<QGraphicsLinearLayout*>(m_statusWidget->layout())->removeItem(m_statusDetailsText);
        m_statusExpandButton->setIcon(QIcon(svg->pixmap("restore")));
    }
    delete svg;
}

void NotifierDialog::storageTeardownDone(Solid::ErrorType error, QVariant errorData, const QString & udi)
{
    DeviceItem* devItem = itemForUdi(udi);
    if (!devItem) {
        return;
    }

    if (!error || !errorData.isValid()) {
        m_notifier->changeNotifierIcon("dialog-ok", DeviceNotifier::LONG_NOTIFICATION_TIMEOUT);
        expireStatusBar(udi);
    } else {
        m_notifier->changeNotifierIcon("dialog-error", DeviceNotifier::LONG_NOTIFICATION_TIMEOUT);
    }

    m_notifier->update();
    devItem->setState(DeviceItem::Idle);
}

void NotifierDialog::storageEjectDone(Solid::ErrorType error, QVariant errorData, const QString &udi)
{
    Q_UNUSED(udi);

    if (!error || !errorData.isValid()) {
        m_notifier->changeNotifierIcon("dialog-ok", DeviceNotifier::SHORT_NOTIFICATION_TIMEOUT);
        expireStatusBar(udi);
    } else {
        m_notifier->changeNotifierIcon("dialog-error", DeviceNotifier::LONG_NOTIFICATION_TIMEOUT);
    }

    m_notifier->update();

    QList<DeviceItem*> deviceList = itemsForParentUdi(udi);
    if (deviceList.isEmpty()) {
        kDebug() << "This should just not happen";
        return;
    }

    foreach (DeviceItem* item, deviceList) {
        item->setState(DeviceItem::Idle);
    }
}

void NotifierDialog::storageSetupDone(Solid::ErrorType error, QVariant errorData, const QString &udi)
{
    DeviceItem *devItem = itemForUdi(udi);
    if (!devItem) {
        return;
    }

    if (!error || !errorData.isValid()) {
        m_notifier->changeNotifierIcon("dialog-ok", DeviceNotifier::SHORT_NOTIFICATION_TIMEOUT);
        expireStatusBar(udi);
    } else {
        m_notifier->changeNotifierIcon("dialog-error", DeviceNotifier::LONG_NOTIFICATION_TIMEOUT);
    }

    m_notifier->update();

    devItem->setState(DeviceItem::Idle);
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

QList<DeviceItem*> NotifierDialog::itemsForParentUdi(const QString &udi) const
{
    QList<DeviceItem*> deviceList;
    for (int i=0; i<m_deviceLayout->count(); i++) {
        DeviceItem* item = dynamic_cast<DeviceItem *>(m_deviceLayout->itemAt(i));
        if (item && Solid::Device(item->udi()).parent().udi() == udi) {
            deviceList << item;
        }
    }

    return deviceList;
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

void NotifierDialog::setupRequested(const QString& udi)
{
    DeviceItem *item = itemForUdi(udi);
    if (!item) {
        kDebug() << "udi not found -- This should just not happen";
        return;
    }

    item->setState(DeviceItem::Mounting);
}

void NotifierDialog::teardownRequested(const QString& udi)
{
    DeviceItem *item = itemForUdi(udi);
    if (!item) {
        kDebug() << "udi not found -- This should just not happen";
        return;
    }

    item->setState(DeviceItem::Umounting);
}

void NotifierDialog::ejectRequested(const QString& udi)
{
    QList<DeviceItem*> deviceList = itemsForParentUdi(udi);
    if (deviceList.isEmpty()) {
        kDebug() << "This should just not happen";
        return;
   }

    foreach (DeviceItem* item, deviceList) {
        item->setState(DeviceItem::Umounting);
    }
}


void NotifierDialog::leftActionActivated(DeviceItem *item)
{
    Solid::Device device(item->udi());

    if ((item->leftAction() == DeviceItem::Umount) || (item->leftAction() == DeviceItem::Lock)) {
        if (device.is<Solid::OpticalDisc>()) {
            Solid::OpticalDrive *drive = device.parent().as<Solid::OpticalDrive>();
            if (drive) {
                drive->eject();
            }
        } else if (device.is<Solid::StorageAccess>()) {
            Solid::StorageAccess *access = device.as<Solid::StorageAccess>();
            if (access && access->isAccessible()) {
                access->teardown();
            }
        }
    } else if (((item->leftAction() == DeviceItem::Mount) || (item->leftAction() == DeviceItem::Unlock)) && device.is<Solid::StorageAccess>()) {
        Solid::StorageAccess *access = device.as<Solid::StorageAccess>();

        // only unmounted devices
        if (access && !access->isAccessible()) {
            access->setup();
        }
    }
}

void NotifierDialog::deviceActivated(DeviceItem *item)
{
    m_devicesScrollWidget->ensureItemVisible(item);

    m_selectedItemBackground->setTargetItem(0);

    for (int i = 0; i < m_deviceLayout->count(); ++i) {
        DeviceItem *devItem = dynamic_cast<DeviceItem *>(m_deviceLayout->itemAt(i));
        if (devItem && devItem != item) {
            devItem->collapse();
            devItem->setHovered(false);
        }
    }

    item->setHoverDisplayOpacity(1);

    Plasma::ItemBackground *tmp = m_itemBackground;
    m_itemBackground = m_selectedItemBackground;
    m_selectedItemBackground = tmp;

    m_itemBackground->setTargetItem(0);

    emit activated();
}

void NotifierDialog::deviceCollapsed(DeviceItem *item)
{
    Q_UNUSED(item);
    Plasma::ItemBackground *tmp = m_itemBackground;
    m_itemBackground = m_selectedItemBackground;
    m_selectedItemBackground = tmp;

    m_collapsing = true;

    m_selectedItemBackground->setTargetItem(0);
}

void NotifierDialog::highlightDeviceAction(QGraphicsItem* item)
{
    m_clearItemBackgroundTargetTimer.stop();
    m_collapsing = false;
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

void NotifierDialog::itemBackgroundMoving(qreal step)
{
    Plasma::ItemBackground *itemBackground = qobject_cast<Plasma::ItemBackground*>(sender());

    for (int i = 0; i < m_deviceLayout->count(); ++i) {
        DeviceItem *item = dynamic_cast<DeviceItem *>(m_deviceLayout->itemAt(i));
        if (!m_collapsing && item && item->hovered() && item->isCollapsed()) {
            qreal normalizedDistance = qAbs(itemBackground->pos().ry()-item->pos().ry()) / qreal (item->rect().height());
            qreal saturatedDistance = 1.-qMin(normalizedDistance*1.5, 1.);
            item->setHoverDisplayOpacity(saturatedDistance*step);
            return;
        }
    }

    if (qFuzzyCompare(step, (qreal)1.0)) {
        m_collapsing = false;
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

void NotifierDialog::resetSelection()
{
    m_itemBackground->setTargetItem(0);
    m_selectedItemBackground->setTargetItem(0);

    for (int i = 0; i < m_deviceLayout->count(); ++i) {
        DeviceItem *item = dynamic_cast<DeviceItem *>(m_deviceLayout->itemAt(i));
        if (item && item->hovered()) {
            item->setHovered(false);
        }
    }
}

void NotifierDialog::updateFreeSpace(DeviceItem *item)
{
    if (item->isMounted() && (item->state() != DeviceItem::Umounting)) {
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

void NotifierDialog::setMenuActionsAt(const QPointF& scenePos)
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
    if (m_deviceCount > 0) {
        m_mainLabel->setText(i18n("Available Devices"));
    } else {
        m_mainLabel->setText(i18n("No Devices Available"));
    }
}

} // namespace Notifier

#include "notifierdialog.moc"

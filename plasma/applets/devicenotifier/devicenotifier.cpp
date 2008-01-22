/***************************************************************************
 *   Copyright (C) 2007 by Alexis Ménard <darktears31@gmail.com>           *
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

#include "devicenotifier.h"
#include "itemdelegate.h"
#include "notifierview.h"

#include <QPainter>
#include <QColor>
#include <QTreeView>
#include <QApplication>
#include <QStandardItemModel>
#include <QGraphicsView>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>

#include <KDialog>
#include <KRun>
#include <KStandardDirs>
#include <KDesktopFile>
#include <kdesktopfileactions.h>
#include <KGlobalSettings>

#include <plasma/svg.h>
#include <plasma/widgets/widget.h>
#include <plasma/containment.h>
#include <plasma/dialog.h>
#include <plasma/phase.h>

//use for desktop view
#include <plasma/layouts/vboxlayout.h>
#include <plasma/widgets/icon.h>

#include <solid/device.h>

using namespace Plasma;
using namespace Notifier;

DeviceNotifier::DeviceNotifier(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args),
      m_icon(""),
      m_solidEngine(0),
      m_hotplugModel(0),
      m_widget(0),
      m_dialog(0),
      m_displayTime(0),
      m_numberItems(0),
      m_itemsValidity(0),
      m_timer(0)
{
    setHasConfigurationInterface(true);
}

void DeviceNotifier::init()
{
    KConfigGroup cg = config();
    m_timer = new QTimer();
    m_displayTime = cg.readEntry("TimeDisplayed", 8);
    m_numberItems = cg.readEntry("NumberItems", 4);
    m_itemsValidity = cg.readEntry("ItemsValidity", 5);

    m_solidEngine = dataEngine("hotplug");

    //connect to engine when a device is plug
    connect(m_solidEngine, SIGNAL(newSource(const QString&)),
            this, SLOT(onSourceAdded(const QString&)));
    connect(m_solidEngine, SIGNAL(sourceRemoved(const QString&)),
            this, SLOT(onSourceRemoved(const QString&)));

    kDebug() << "I'm in containment : " << containment()->containmentType();
    if (containment()->containmentType() == Containment::DesktopContainment) {
        initDesktop();
        isOnDesktop = true;
    } else {
        setContentSize(128, 128);
        initSysTray();
        isOnDesktop = false;
    }
}

void DeviceNotifier::initDesktop()
{
    Plasma::VBoxLayout *layout = new Plasma::VBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);
    m_layout_list = new Plasma::VBoxLayout(layout);
    m_layout_list->setMargin(0);
    m_layout_list->setSpacing(0);
    Plasma::Icon *icon_top = new Plasma::Icon(KIcon("emblem-mounted"), QString(i18n("Recently plugged devices :")), this);
    icon_top->setMaximumSize(QSize(275,48));
    icon_top->setAcceptsHoverEvents(false);
    icon_top->setAcceptedMouseButtons(0);
    icon_top->setOrientation(Qt::Horizontal);
    layout->addItem(icon_top);
    layout->addItem(m_layout_list);

    setContentSize(275, 275);
}

void DeviceNotifier::initSysTray()
{
    //we display the icon corresponding to the computer
    QList<Solid::Device> list = Solid::Device::allDevices();
    if (list.size() > 0) {
        Solid::Device device=list[0];

        while (device.parent().isValid()) {
            device = device.parent();
        }
        m_icon = KIcon(device.icon());
    } else {
        //default icon if problem
        m_icon = KIcon("computer");
    }
    m_widget = new Dialog();

    QVBoxLayout *m_layout = new QVBoxLayout();
    m_layout->setSpacing(0);
    m_layout->setMargin(0);

    m_hotplugModel = new QStandardItemModel(this);

    QLabel *label = new QLabel(i18n("<font color=\"white\">Recently plugged devices:</font>"));
    QLabel *icon = new QLabel();
    icon->setPixmap(KIcon("emblem-mounted").pixmap(ItemDelegate::ICON_SIZE, ItemDelegate::ICON_SIZE));
    icon->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    QHBoxLayout *m_layout2 = new QHBoxLayout();
    m_layout2->setSpacing(0);
    m_layout2->setMargin(0);

    m_layout2->addWidget(icon);
    m_layout2->addWidget(label);

    Notifier::NotifierView *m_notifierView= new NotifierView(m_widget);
    m_notifierView->setModel(m_hotplugModel);
    ItemDelegate *delegate = new ItemDelegate;
    m_notifierView->setItemDelegate(delegate);
    m_widget->setFocusPolicy(Qt::NoFocus);

    m_layout->addLayout(m_layout2);
    m_layout->addWidget(m_notifierView);
    m_widget->setLayout(m_layout);

    m_widget->setWindowFlags(m_notifierView->windowFlags() | Qt::WindowStaysOnTopHint |
                             Qt::FramelessWindowHint | Qt::X11BypassWindowManagerHint);
    m_widget->adjustSize();

    //FIXME : For KDE4.1 need to use to KStyle to use correct click behaviour
    if (KGlobalSettings::singleClick()) {
        connect(m_notifierView, SIGNAL(clicked (const QModelIndex &)), this, SLOT(slotOnItemClicked(const QModelIndex &)));
    } else {
        connect(m_notifierView, SIGNAL(doubleClicked (const QModelIndex &)), this, SLOT(slotOnItemClicked(const QModelIndex &)));
    }
    connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimerExpired()));

    updateGeometry();
    update();
}

DeviceNotifier::~DeviceNotifier()
{
    delete m_widget;
    delete m_dialog;
    delete m_hotplugModel;
    delete m_timer;
}

Qt::Orientations DeviceNotifier::expandingDirections() const
{
    return Qt::Vertical;
}

QSizeF DeviceNotifier::contentSizeHint() const
{
    QSizeF sizeHint = contentSize();
    switch (formFactor()) {
    case Plasma::Vertical:
        sizeHint.setHeight(sizeHint.width());
        break;
    case Plasma::Horizontal:
        sizeHint.setWidth(sizeHint.height());
        break;
    default:
        break;
    }

    return sizeHint;
}

void DeviceNotifier::paintInterface(QPainter *p, const QStyleOptionGraphicsItem *option, const QRect &rect)
{
    Q_UNUSED(option)
    Q_UNUSED(rect)
    if(!isOnDesktop) {
        p->setRenderHint(QPainter::SmoothPixmapTransform);
        p->drawPixmap(rect, m_icon.pixmap(rect.size()));
    }
}

void DeviceNotifier::dataUpdated(const QString &source, Plasma::DataEngine::Data data)
{
    if (data.size() > 0) {
        int nb_actions = 0;
        QString last_action_label;
        foreach (QString desktop, data["predicateFiles"].toStringList()) {
            QString filePath = KStandardDirs::locate("data", "solid/actions/" + desktop);
            QList<KServiceAction> services = KDesktopFileActions::userDefinedServices(filePath, true);
            nb_actions += services.size();
            if (services.size() > 0) {
                last_action_label = QString(services[0].text());
            }
        }
        if (!isOnDesktop) {
            performSourceUpdatedInSystray(source, data, nb_actions, last_action_label);
        } else {
            performSourceUpdatedInDesktop(source, data, nb_actions, last_action_label);
        }
    }
}

void DeviceNotifier::performSourceUpdatedInDesktop(const QString &source, Plasma::DataEngine::Data data, int nb_actions, const QString &last_action_label)
{
    QString s;
    if (nb_actions > 1) {
        s = i18np("1 action for this device", "%1 actions for this device", nb_actions);
    } else {
        s = last_action_label;
    }
    QString display=data["text"].toString();
    display = display + " - " + s;
    Plasma::Icon *icon_item = new Plasma::Icon(KIcon(data["icon"].toString()), display, this);
    icon_item->setMaximumSize(QSize(275,48));
    m_layout_list->addItem(icon_item);
    icon_item->setOrientation(Qt::Horizontal);
    ItemType type;
    type.icon = icon_item;
    type.predicateFiles = data["predicateFiles"].toStringList();
    type.text = data["text"].toString();
    type.udi = source;
    connect(icon_item, SIGNAL(pressed(bool)), this, SLOT(slotOnItemDesktopClicked(bool)));
    m_map_item.insert(source, type);
}

void DeviceNotifier::performSourceUpdatedInSystray(const QString &source, Plasma::DataEngine::Data data,int nb_actions, const QString &last_action_label)
{
    QModelIndex index = indexForUdi(source);
    Q_ASSERT(index.isValid());

    m_hotplugModel->setData(index, data["predicateFiles"], PredicateFilesRole);
    m_hotplugModel->setData(index, data["text"], Qt::DisplayRole);
    m_hotplugModel->setData(index, KIcon(data["icon"].toString()), Qt::DecorationRole);

    if (nb_actions > 1) {
        QString s = i18np("1 action for this device",
                          "%1 actions for this device",
                          nb_actions);
        m_hotplugModel->setData(index, s, ActionRole);
    } else {
        m_hotplugModel->setData(index,last_action_label, ActionRole);
    }
    m_widget->move(popupPosition(m_widget->sizeHint()));
    m_widget->show();
    m_timer->start(m_displayTime*1000);
}

void DeviceNotifier::onSourceAdded(const QString &name)
{
    kDebug() << "DeviceNotifier:: source added" << name;
    if(!isOnDesktop) {
        performSourceAddedInSystray(name);
    } else {
        performSourceAddedInDesktop(name);
    }
    m_solidEngine->connectSource(name, this);
}

void DeviceNotifier::performSourceAddedInSystray(const QString &name)
{
    if (m_hotplugModel->rowCount() == m_numberItems && m_numberItems != 0) {
        m_hotplugModel->removeRow(m_hotplugModel->rowCount() - 1);
    }
    QStandardItem *item = new QStandardItem();
    item->setData(name, SolidUdiRole);
    m_hotplugModel->insertRow(0, item);
}

void DeviceNotifier::performSourceAddedInDesktop(const QString &name)
{
    Q_UNUSED(name);
}

void DeviceNotifier::onSourceRemoved(const QString &name)
{
    if(!isOnDesktop) {
        performSourceRemovedInSystray(name);
    } else {
        performSourceRemovedInDesktop(name);
    }

}
void DeviceNotifier::performSourceRemovedInSystray(const QString &name)
{
    QModelIndex index = indexForUdi(name);
    Q_ASSERT(index.isValid());
    m_hotplugModel->removeRow(index.row());
    if (m_hotplugModel->rowCount() == 0) {
        m_widget->hide();
    }
}

void DeviceNotifier::performSourceRemovedInDesktop(const QString &name)
{
    if (m_map_item.contains(name)) {
        Plasma::Icon *icon = m_map_item.take(name).icon;
        m_layout_list->removeItem(icon);
        delete icon;
    }
}

QModelIndex DeviceNotifier::indexForUdi(const QString &udi) const
{
    int rowCount = m_hotplugModel->rowCount();
    for (int i=0; i < rowCount; ++i) {
        QModelIndex index = m_hotplugModel->index(i, 0);
        QString itemUdi = m_hotplugModel->data(index, SolidUdiRole).toString();
        if (itemUdi == udi) {
            return index;
        }
    }
    //Is it possible to go here?no...
    kDebug() << "We should not be here!";
    return QModelIndex();
}

void DeviceNotifier::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event)
    if(!isOnDesktop) {
        if (event->buttons () == Qt::LeftButton &&
            contentRect().contains(event->pos())) {
            m_widget->isVisible() ? m_widget->hide() : m_widget->show();
            event->accept();
            return;
        }
        Applet::mousePressEvent(event);
    }
}

void DeviceNotifier::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    if(!isOnDesktop) {
        m_widget->move(popupPosition(m_widget->sizeHint()));
    }
    Applet::hoverEnterEvent(event);
}

void DeviceNotifier::showConfigurationInterface()
{
    if (m_dialog == 0) {
        kDebug() << "DeviceNotifier:: Enter in configuration interface";
        m_dialog = new KDialog;
        m_dialog->setCaption(i18n("Configure New Device Notifier"));

        QWidget *widget = new QWidget;
        ui.setupUi(widget);
        m_dialog->setMainWidget(widget);
        m_dialog->setButtons(KDialog::Ok | KDialog::Cancel | KDialog::Apply);
        connect(m_dialog, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
        connect(m_dialog, SIGNAL(okClicked()), this, SLOT(configAccepted()));
        ui.spinTime->setValue(m_displayTime);
        ui.spinItems->setValue(m_numberItems);
        ui.spinTimeItems->setValue(m_itemsValidity);
    }

    m_dialog->show();
}

void DeviceNotifier::configAccepted()
{
    kDebug() << "DeviceNotifier:: Config Accepted with params" << ui.spinTime->value() \
             << "," << ui.spinItems->value() \
             << "," << ui.spinTimeItems->value();
    m_displayTime = ui.spinTime->value();
    m_numberItems = ui.spinItems->value();
    m_itemsValidity = ui.spinTimeItems->value();
    KConfigGroup cg = config();
    cg.writeEntry("TimeDisplayed", m_displayTime);
    cg.writeEntry("NumberItems", m_numberItems);
    cg.writeEntry("ItemsValidity", m_itemsValidity);
    emit configNeedsSaving();
}

void DeviceNotifier::slotOnItemClicked(const QModelIndex &index)
{
    m_widget->hide();
    m_timer->stop();
    QString udi = QString(m_hotplugModel->data(index, SolidUdiRole).toString());
    QStringList desktop_files = m_hotplugModel->data(index, PredicateFilesRole).toStringList();
    kDebug() << "DeviceNotifier:: call Solid Ui Server with params :" << udi \
             << "," << desktop_files;
    QDBusInterface soliduiserver("org.kde.kded", "/modules/soliduiserver", "org.kde.SolidUiServer");
    QDBusReply<void> reply = soliduiserver.call("showActionsDialog", udi, desktop_files);
}

void DeviceNotifier::slotOnItemDesktopClicked(bool pressed)
{
    if (!pressed) {
        return;
    }
    QObject *obj = sender();
    if (obj) {
        Plasma::Icon *icon = dynamic_cast<Plasma::Icon *>(obj);
        if (icon) {
            bool find = false;
            ItemType object;
            foreach (ItemType item, m_map_item) {
                if (item.icon == icon) {
                    find = true;
                    object = item;
                    break;
                }
            }
            if (find) {
                QString udi = object.udi;
                QStringList desktop_files = object.predicateFiles;
                kDebug() << "DeviceNotifier:: call Solid Ui Server with params :" << udi \
                         << "," << desktop_files;
                QDBusInterface soliduiserver("org.kde.kded", "/modules/soliduiserver", "org.kde.SolidUiServer");
                QDBusReply<void> reply = soliduiserver.call("showActionsDialog", udi, desktop_files);
            }
        }
    }
}

void DeviceNotifier::onTimerExpired()
{
    m_timer->stop();
    m_widget->hide();
}

#include "devicenotifier.moc"

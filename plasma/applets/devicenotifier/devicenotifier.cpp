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

#include <QPainter>
#include <QColor>
#include <QApplication>
#include <QStandardItemModel>
#include <QGraphicsView>
#include <QGraphicsSceneMouseEvent>


#include <plasma/layouts/hboxlayout.h>
#include <plasma/widgets/label.h>
#include <plasma/widgets/widget.h>
#include <plasma/widgets/icon.h>
#include <plasma/containment.h>

#include <KDialog>
#include <kdesktopfileactions.h>
#include <kstandarddirs.h>

#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>

#include <solid/device.h>

#include "itemdelegate.h"
#include "listview.h"

using namespace Plasma;
using namespace Notifier;

DeviceNotifier::DeviceNotifier(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args),
      m_icon(""),
      m_hotplugModel(new QStandardItemModel(this)),
      m_dialog(0)    
{
    setHasConfigurationInterface(true);
}

void DeviceNotifier::init()
{
    KConfigGroup cg = config();
    m_timer=new QTimer();
    m_displayTime = cg.readEntry("TimeDisplayed", 8);
    m_numberItems = cg.readEntry("NumberItems", 4);
    m_itemsValidity = cg.readEntry("ItemsValidity", 5);


    //we display the icon corresponding to the computer
    Solid::Device device=Solid::Device::allDevices()[0];
   
    while (device.parent().isValid())
    {
	device=device.parent();
    }
    m_icon=KIcon(device.icon());

    m_widget= new QWidget(0,Qt::Window);
    m_widget->setStyleSheet("{ border : 0px }");
    m_listView= new ListView(m_widget);
    QVBoxLayout *m_layout = new QVBoxLayout();
    m_layout->setSpacing(0);
    m_layout->setMargin(0);
    
    QLabel *Label = new QLabel(i18n("Recently plugged devices:"));
    QLabel *Icon = new QLabel();
    Icon->setPixmap(KIcon("emblem-mounted").pixmap(ItemDelegate::ICON_SIZE, ItemDelegate::ICON_SIZE));
    Icon->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
    
    QHBoxLayout *m_layout2 = new QHBoxLayout();
    m_layout2->setSpacing(0);
    m_layout2->setMargin(0);

    m_layout2->addWidget(Icon);
    m_layout2->addWidget(Label);

    m_listView->setModel(m_hotplugModel);
    ItemDelegate *delegate = new ItemDelegate;
    m_listView->setItemDelegate(delegate);
    m_widget->setFocusPolicy(Qt::NoFocus);

    m_layout->addLayout(m_layout2);
    m_layout->addWidget(m_listView);
    m_widget->setLayout(m_layout);
    
    m_widget->setWindowFlags(m_listView->windowFlags()|Qt::WindowStaysOnTopHint|Qt::Popup);
    m_widget->adjustSize();

    /*QPointF scenePos = mapToScene(boundingRect().topLeft());
    QGraphicsScene * scene=containment()->scene();
    if (scene)
    {
	QList<QGraphicsView *> list=scene->views();
	kDebug()<<"size"<<list.size();
	QGraphicsView *view=list[0];
	if (view) {
	    QPoint viewPos = view->mapFromScene(scenePos);
	    QPoint globalPos = view->mapToGlobal(viewPos);
	    globalPos.ry() -= m_widget->height(); 
	    m_widget->move(globalPos);
	}
    }*/

    setSize(128,128);

    m_solidEngine = dataEngine("hotplug");

    //connect to engine when a device is plug
    connect(m_solidEngine, SIGNAL(newSource(const QString&)),
            this, SLOT(onSourceAdded(const QString&)));
    connect(m_solidEngine, SIGNAL(sourceRemoved(const QString&)),
            this, SLOT(onSourceRemoved(const QString&)));
    
    connect(m_listView,SIGNAL(doubleClicked ( const QModelIndex & )),this,SLOT(slotOnItemDoubleclicked( const QModelIndex & )));

    connect(m_timer,SIGNAL(timeout()),this,SLOT(onTimerExpired()));
    updateGeometry();
    update();
}

DeviceNotifier::~DeviceNotifier()
{
}

void DeviceNotifier::paintInterface(QPainter *p, const QStyleOptionGraphicsItem *option, const QRect &rect)
{
    Q_UNUSED(option)
    Q_UNUSED(rect)
    p->setRenderHint(QPainter::SmoothPixmapTransform);
    p->drawPixmap(rect,m_icon.pixmap(rect.size()));
}

void DeviceNotifier::dataUpdated(const QString &source, Plasma::DataEngine::Data data)
{
    if (data.size()>0) {
        QModelIndex index = indexForUdi(source);
        Q_ASSERT(index.isValid());

        m_hotplugModel->setData(index, data["predicateFiles"], PredicateFilesRole);
        m_hotplugModel->setData(index, data["text"], Qt::DisplayRole);
        m_hotplugModel->setData(index, KIcon(data["icon"].toString()), Qt::DecorationRole);
	
	int nb_actions = 0;
	QString last_action;
	foreach (QString desktop, data["predicateFiles"].toStringList()) {
	    QString filePath = KStandardDirs::locate("data", "solid/actions/"+desktop);
	    QList<KServiceAction> services = KDesktopFileActions::userDefinedServices(filePath, true);
	    nb_actions+=services.size();
	    if (services.size()>0)
	    {
		last_action=QString("Open with "+services[0].text());
	    }
	}
	if (nb_actions!=1)
	{
	    QString s = i18np("1 action available", "%1 actions available", nb_actions);
	    m_hotplugModel->setData(index,s, ActionRole);
	    kDebug()<<"DeviceNotifier:: Nb Actions"<<nb_actions;
	}
	else
	{	 
	    kDebug()<<"DeviceNotifier:: Actions"<<last_action;
	    m_hotplugModel->setData(index,last_action, ActionRole);	    
	}

    }
    m_widget->show();
    m_timer->start(m_displayTime*1000);

}

void DeviceNotifier::onSourceAdded(const QString& name)
{
    kDebug()<<"DeviceNotifier:: source added"<<name;
    QStandardItem *item = new QStandardItem();
    item->setData(name, SolidUdiRole);
    m_hotplugModel->insertRow(0, item);

    // TODO: Update model
    m_solidEngine->connectSource(name, this);


}

void DeviceNotifier::onSourceRemoved(const QString &name)
{
    QModelIndex index = indexForUdi(name);
    Q_ASSERT(index.isValid());
    m_hotplugModel->removeRow(index.row());
    if (m_hotplugModel->rowCount()==0) {
	m_widget->hide();
    }
}

QModelIndex DeviceNotifier::indexForUdi(const QString &udi) const
{
    int rowCount = m_hotplugModel->rowCount();
    for (int i=0; i<rowCount; ++i) {
        QModelIndex index = m_hotplugModel->index(i, 0);
        QString itemUdi = m_hotplugModel->data(index, SolidUdiRole).toString();
        if (itemUdi==udi) {
            return index;
        }
    }
    //Is it possible to go here?no...
    return QModelIndex();
}

void DeviceNotifier::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event)
    m_widget->show();
}

void DeviceNotifier::hoverEnterEvent ( QGraphicsSceneHoverEvent  * event )
{
    QWidget *viewWidget = event->widget() ? event->widget()->parentWidget() : 0;
    QPointF scenePos = mapToScene(boundingRect().topLeft());
    QGraphicsView *view = qobject_cast<QGraphicsView*>(viewWidget);
    if (view) {
	QPoint viewPos = view->mapFromScene(scenePos);
	QPoint globalPos = view->mapToGlobal(viewPos);
	globalPos.ry() -= m_widget->height(); 
	m_widget->move(globalPos);
    }
}

void DeviceNotifier::showConfigurationInterface()
{
     if (m_dialog == 0) {
	kDebug()<<"DeviceNotifier:: Enter in configuration interface";
     	m_dialog = new KDialog;
        m_dialog->setCaption( i18n("Configure New Device Notifier") );

        ui.setupUi(m_dialog->mainWidget());
        m_dialog->setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Apply );
        connect( m_dialog, SIGNAL(applyClicked()), this, SLOT(configAccepted()) );
        connect( m_dialog, SIGNAL(okClicked()), this, SLOT(configAccepted()) );
	ui.spinTime->setValue(m_displayTime);
	ui.spinItems->setValue(m_numberItems);
	ui.spinTimeItems->setValue(m_itemsValidity);	
      }
      m_dialog->show();
}

void DeviceNotifier::configAccepted()
{
    kDebug()<<"DeviceNotifier:: Config Accepted with params"<<ui.spinTime->value()<<","<<ui.spinItems->value()<<","<<ui.spinTimeItems->value();
    m_displayTime=ui.spinTime->value();
    m_numberItems=ui.spinItems->value();
    m_itemsValidity=ui.spinTimeItems->value();
    KConfigGroup cg = config();
    cg.writeEntry("TimeDisplayed", m_displayTime);
    cg.writeEntry("NumberItems", m_numberItems);
    cg.writeEntry("ItemsValidity", m_itemsValidity);
}

QSizeF DeviceNotifier::contentSizeHint() const
{
    QSizeF sizeHint = contentSize();
    int max = qMax(sizeHint.width(), sizeHint.height());
    sizeHint = QSizeF(max, max);
    return sizeHint;
}

void DeviceNotifier::slotOnItemDoubleclicked(const QModelIndex & index)
{
    m_widget->hide();
    m_timer->stop();
    QString udi=QString(m_hotplugModel->data(index, SolidUdiRole).toString());
    QStringList desktop_files=m_hotplugModel->data(index, PredicateFilesRole).toStringList();
    kDebug()<<"DeviceNotifier:: call Solid Ui Server with params :"<<udi<<","<<desktop_files;
    QDBusInterface soliduiserver("org.kde.kded", "/modules/soliduiserver", "org.kde.SolidUiServer");
    QDBusReply<void> reply = soliduiserver.call("showActionsDialog", udi,desktop_files);
}

void DeviceNotifier::onTimerExpired()
{
    m_timer->stop();
    m_widget->hide();
}

#include "devicenotifier.moc"

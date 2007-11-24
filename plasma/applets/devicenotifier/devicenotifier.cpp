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

#include <KDialog>

#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>

#include <solid/device.h>

using namespace Plasma;


DeviceNotifier::DeviceNotifier(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args),
      m_icon(""),
      m_hotplugModel(new QStandardItemModel(this)),
      m_dialog(0)    
{
    setHasConfigurationInterface(true);
    
    //we display the icon corresponding to the computer
    Solid::Device device=Solid::Device::allDevices()[0];
   
    while (device.parent().isValid())
    {
	device=device.parent();
    }
    m_icon=KIcon(device.icon());

    m_widget= new QWidget(0,Qt::Window);
    m_listView= new QListView(m_widget);
    QVBoxLayout *m_layout = new QVBoxLayout();
    m_layout->setSpacing(0);
    m_layout->setMargin(0);
    
    m_listView->setModel(m_hotplugModel);
    m_widget->setFocusPolicy(Qt::NoFocus);
    
    m_layout->addWidget(m_listView);
    m_widget->setLayout(m_layout);
    
    m_widget->setWindowFlags(m_listView->windowFlags()|Qt::WindowStaysOnTopHint|Qt::Popup);
    m_widget->adjustSize();


    setSize(128,128);

    m_solidEngine = dataEngine("hotplug");

    //connect to engine when a device is plug
    connect(m_solidEngine, SIGNAL(newSource(const QString&)),
            this, SLOT(onSourceAdded(const QString&)));
    connect(m_solidEngine, SIGNAL(sourceRemoved(const QString&)),
            this, SLOT(onSourceRemoved(const QString&)));
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
        // TODO: Update model
	//kDebug()<<"DeviceNotifier:: "<<data[source].toString();
	//desktop_files=data["predicateFiles"].toStringList();
	//kDebug()<<data["icon"].toString();
	//QString icon_temp = data["icon"].toString();
	//m_icon = KIcon(icon_temp);
    }
}

void DeviceNotifier::onSourceAdded(const QString& source)
{
    kDebug()<<"DeviceNotifier:: source added"<<source;
    // TODO: Update model
    m_solidEngine->connectSource(source, this);


}

void DeviceNotifier::onSourceRemoved(const QString &name)
{
    // TODO: Update model
}

void DeviceNotifier::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QPointF scenePos = mapToScene(boundingRect().topLeft());
    QWidget *viewWidget = event->widget() ? event->widget()->parentWidget() : 0;
    QGraphicsView *view = qobject_cast<QGraphicsView*>(viewWidget);
    if (view) {
	QPoint viewPos = view->mapFromScene(scenePos);
	QPoint globalPos = view->mapToGlobal(viewPos);
	globalPos.ry() -= m_widget->height(); 
	m_widget->move(globalPos);
    }
    m_widget->show();
}

void DeviceNotifier::showConfigurationInterface()
{
#if 0
     if (m_dialog == 0) {
	kDebug()<<"DeviceNotifier:: Enter in configuration interface";
     	m_dialog = new KDialog;
        m_dialog->setCaption( i18n("Configure New Device Notifier") );

        ui.setupUi(m_dialog->mainWidget());
        m_dialog->setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Apply );
        connect( m_dialog, SIGNAL(applyClicked()), this, SLOT(configAccepted()) );
        connect( m_dialog, SIGNAL(okClicked()), this, SLOT(configAccepted()) );
	ui.spinTime->setValue(m_time);
      }
      ui.spinHeight->setValue(m_height);
      m_dialog->show();
#endif
}

void DeviceNotifier::configAccepted()
{
#if 0
    kDebug()<<"DeviceNotifier:: Config Accepted with params"<<ui.spinTime->value()<<","<<ui.spinHeight->value();
    m_time=ui.spinTime->value();
    m_height=ui.spinHeight->value();
#endif
}

QSizeF DeviceNotifier::contentSizeHint() const
{
    kDebug() << "content size hint is being asked for and we return" << size();

    QSizeF sizeHint = contentSize();
    int max = qMax(sizeHint.width(), sizeHint.height());
    sizeHint = QSizeF(max, max);
    return sizeHint;
}

#include "devicenotifier.moc"

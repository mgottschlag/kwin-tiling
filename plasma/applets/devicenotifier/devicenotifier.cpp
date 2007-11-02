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

#include <plasma/widgets/hboxlayout.h>
#include <plasma/widgets/label.h>
#include <plasma/widgets/widget.h>
#include <plasma/widgets/icon.h>

#include <KDialog>

#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>

using namespace Plasma;


DeviceNotifier::DeviceNotifier(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args),m_dialog(0)
{
    setDrawStandardBackground(true);
    setHasConfigurationInterface(true);
    m_layout = new Plasma::HBoxLayout(this);
    m_layout->setMargin(0);
    m_layout->setSpacing(0);
    m_label=new Plasma::Label(this);
    m_label->setText(i18n("Welcome to Device Notifier"));
    m_label->setPen(QPen(Qt::white));    

    m_time=5;
    m_height=150;

    SolidEngine = dataEngine("hotplug");
    m_udi="";
    icon = false;
    first=true;
    
    m_font = QApplication::font();
    m_font.setWeight(QFont::Bold);


    //connect the timer to MoveDown Animation
    t=new QTimer(this);
    connect(t,SIGNAL(timeout()),this,SLOT(moveDown()));

    //connect to engine when a device is plug
    connect(SolidEngine, SIGNAL(newSource(const QString&)),this, SLOT(SourceAdded(const QString&)));
}

DeviceNotifier::~DeviceNotifier()
{
}

void DeviceNotifier::paintInterface(QPainter *p, const QStyleOptionGraphicsItem *option, const QRect &contentsRect)
{
    Q_UNUSED(option);
    Q_UNUSED(p);
    Q_UNUSED(contentsRect);
    if(icon)
    {
	  m_layout->addItem(m_icon);
    }
    m_layout->addItem(m_label);
    kDebug()<<"DeviceNotifier:: geometry "<<geometry().width();
    //hide();
}

void DeviceNotifier::hideNotifier(QGraphicsItem * item)
{
    icon=false; 
    item->hide();
}

void DeviceNotifier::moveUp()
{
    t->start(m_time*1000);
    disconnect(Phase::self(),SIGNAL(movementComplete(QGraphicsItem *)),this,SLOT(hideNotifier(QGraphicsItem *)));
    show();
    Phase::self()->moveItem(this, Phase::SlideIn,QPoint(geometry().x(),geometry().y()-m_height));
}


void DeviceNotifier::moveDown()
{
    t->stop();
    Phase::self()->moveItem(this, Phase::SlideOut,QPoint(geometry().x(),geometry().y()+m_height));
    connect(Phase::self(),SIGNAL(movementComplete(QGraphicsItem *)),this,SLOT(hideNotifier(QGraphicsItem *)));
}


void DeviceNotifier::updated(const QString &source, Plasma::DataEngine::Data data)
{
    if(data.size()>0)
    {
		kDebug()<<"DeviceNotifier:: "<<data[source].toString();
		desktop_files=data["predicateFiles"].toStringList();
		kDebug()<<data["icon"].toString();
		QString icon_temp = data["icon"].toString();

		if(first)
		{
		  origin_size=geometry();
		  first=false;
 		  m_icon=new Plasma::Icon(KIcon(icon_temp),"",this);
		}

		m_icon->setIcon(KIcon(icon_temp));

		icon = true;

		device_name=i18n("A new device has been detected: \n");
		device_name+=data["text"].toString();
		m_label->setPen(QPen(Qt::white));
		m_label->setText(device_name);
		float size_h=0.0;
		float size_w=0.0;
		size_h+=m_icon->iconSize().height();
		size_h+=m_label->geometry().height();
		size_w+=m_label->geometry().width();
		size_w+=m_icon->iconSize().width();
		QRectF temp(origin_size.x(),origin_size.y(),size_w,size_h);
		setGeometry(temp);
		updateGeometry();	
		update();
		moveUp();
     }
}

void DeviceNotifier::SourceAdded(const QString& source)
{
    kDebug()<<"DeviceNotifier:: source added"<<source;
    m_udi = source;
    SolidEngine->connectSource(source, this);
}

void DeviceNotifier::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event);
    if(icon)
    {
	kDebug()<<"DeviceNotifier:: call Solid Ui Server with params :"<<m_udi<<","<<desktop_files;
	QDBusInterface soliduiserver("org.kde.kded", "/modules/soliduiserver", "org.kde.kded.SolidUiServer");
	QDBusReply<void> reply = soliduiserver.call("showActionsDialog", m_udi,desktop_files);
    }
}

void DeviceNotifier::showConfigurationInterface()
{
     if (m_dialog == 0) 
     {
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
}

void DeviceNotifier::configAccepted()
{
    kDebug()<<"DeviceNotifier:: Config Accepted with params"<<ui.spinTime->value()<<","<<ui.spinHeight->value();
	m_time=ui.spinTime->value();
    m_height=ui.spinHeight->value();
}

#include "devicenotifier.moc"

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

#include <plasma/layouts/hboxlayout.h>
#include <plasma/widgets/label.h>
#include <plasma/widgets/widget.h>
#include <plasma/widgets/icon.h>

#include <KDialog>

#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>

using namespace Plasma;


DeviceNotifier::DeviceNotifier(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args),m_icon("multimedia-player"),m_dialog(0)
{
    setHasConfigurationInterface(true);
    
    setSize(128,128);
    SolidEngine = dataEngine("hotplug");
    
    //connect to engine when a device is plug
    connect(SolidEngine, SIGNAL(newSource(const QString&)),this, SLOT(SourceAdded(const QString&)));
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

	kDebug()<<"DeviceNotifier:: "<<data[source].toString();
	desktop_files=data["predicateFiles"].toStringList();
	kDebug()<<data["icon"].toString();
	QString icon_temp = data["icon"].toString();
	m_icon=KIcon(icon_temp);
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
    /*if (icon) {
	kDebug()<<"DeviceNotifier:: call Solid Ui Server with params :"<<m_udi<<","<<desktop_files;
	QDBusInterface soliduiserver("org.kde.kded", "/modules/soliduiserver", "org.kde.SolidUiServer");
	QDBusReply<void> reply = soliduiserver.call("showActionsDialog", m_udi,desktop_files);
    }*/
    m_icon=KIcon("multimedia-player");
    
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

QSizeF DeviceNotifier::contentSizeHint() const
{
    kDebug() << "content size hint is being asked for and we return" << size();

    QSizeF sizeHint = contentSize();
    int max = qMax(sizeHint.width(), sizeHint.height());
    sizeHint = QSizeF(max, max);
    return sizeHint;
}


#include "devicenotifier.moc"

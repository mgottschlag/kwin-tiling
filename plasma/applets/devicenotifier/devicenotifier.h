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

#ifndef DEVICENOTIFIER_H
#define DEVICENOTIFIER_H

//Solid
#include <solid/solidnamespace.h>

//Plasma
#include <plasma/popupapplet.h>
#include <plasma/dataengine.h>
#include <plasma/tooltipmanager.h>

class QStandardItemModel;
class QGraphicsLinearLayout;
class QGraphicsProxyWidget;
class QTimer;

class KIcon;

namespace Notifier
{
    class NotifierDialog;
}

//desktop view
namespace Plasma
{
    class Icon;
}

/**
* @short Applet used to display hot plug devices
*
*/
class DeviceNotifier : public Plasma::PopupApplet
{
    Q_OBJECT

    public:
        /**
        * Constructor of the applet
        * @param parent the parent of this object
        **/
        DeviceNotifier(QObject *parent, const QVariantList &args);
        
        /**
        * Default destructor
        **/
        ~DeviceNotifier();
        
        /**
        * initialize the applet (called by plasma automatically)
        **/
        void init();

        /**
        *  allow to change the icon of the notifier if this applet is in icon mode
        **/
        void changeNotifierIcon(const QString& name = QString());

        /**
         * The widget that displays the list of devices.
         */
        QWidget *widget();

    public slots:
        /**
         * @internal Sets the tooltip content properly before showing.
         */
         void toolTipAboutToShow();

        /**
         * @internal Clears memory when needed.
         */
         void toolTipHidden();
    
    protected slots:
        /**
        * slot called when a source/device is added in the hotplug engine
        * @param name the name of the new source
        **/
        void onSourceAdded(const QString &name);

        /**
        * @internal slot called when a source/device is removed in the hotplug engine
        * @param name the name of the removed source
        **/
        void onSourceRemoved(const QString &name);

        /**
        * slot called when a source of the hotplug engine is updated
        * @param source the name of the source
        * @param data the data of the source
        **/
        void dataUpdated(const QString &source, Plasma::DataEngine::Data data);

    protected:
        void constraintsEvent(Plasma::Constraints constraints);


    private:
        /**
        * @internal Used to fill the notifier from previous plugged devices
        **/
        void fillPreviousDevices();

        /**
         * @internal Used to popup the device view.
         */
        void notifyDevice(const QString &name);

        /**
         * @internal Used to remove the last device notification.
         */
        void removeLastDeviceNotification(const QString &name);

        ///the engine used to get hot plug devices
        Plasma::DataEngine *m_solidEngine;

        ///The engine used to manage devices in the applet (unmount,...)
        Plasma::DataEngine *m_solidDeviceEngine;
  
        ///the icon used when the applet is in the taskbar
        Plasma::Icon *m_icon;
    
        ///default icon of the notifier
        QString m_iconName;

        ///The dialog where devices are displayed
        Notifier::NotifierDialog * m_dialog;
        
        ///the time durin when the dialog will be show
        int m_displayTime;
      
        ///the number of items displayed in the dialog
        int m_numberItems;
      
        ///the time during when the item will be displayed
        int m_itemsValidity;

        ///the timer for different use cases
        QTimer *m_timer;
       
        ///bool to know if notifications are enabled
        bool isNotificationEnabled;

        ///last plugged udi
        QList<QString> m_lastPlugged;

        ///true if fillPreviousDevices is running
        bool m_fillingPreviousDevices;
};

#endif

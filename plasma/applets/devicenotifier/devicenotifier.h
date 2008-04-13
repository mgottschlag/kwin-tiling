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

#include <solid/solidnamespace.h>
#include <plasma/applet.h>
#include <plasma/dataengine.h>
#include <plasma/phase.h>
#include <plasma/dialog.h>
#include <KIcon>
#include <QModelIndex>
#include <QGraphicsProxyWidget>
#include <QGraphicsGridLayout>
#include <QTimer>
#include <QListView>

#include "ui_deviceNotifierConfig.h"

class QStandardItemModel;
class QGraphicsLinearLayout;
class KDialog;

namespace Notifier
{
    class NotifierView;
}

//desktop view
namespace Plasma
{
    class Icon;
}

class DeviceNotifier : public Plasma::Applet
{
    Q_OBJECT

    public:
        struct ItemType
        {
            Plasma::Icon *icon;
            QStringList predicateFiles;
            QString text;
            QString udi;
        };

        enum SpecificRoles {
            SolidUdiRole = Qt::UserRole + 1,
            PredicateFilesRole = Qt::UserRole + 2,
            ActionRole = Qt::UserRole + 3,
            IconNameRole = Qt::UserRole + 4,
            ScopeRole = Qt::UserRole + 5,
            SubTitleMandatoryRole = Qt::UserRole + 6
        };

        DeviceNotifier(QObject *parent, const QVariantList &args);
        ~DeviceNotifier();

        void init();
        Qt::Orientations expandingDirections() const;
        //QSizeF contentSizeHint() const;
        void paintInterface(QPainter *p, const QStyleOptionGraphicsItem *option, const QRect &rect);
        void constraintsUpdated(Plasma::Constraints constraints);
    public slots:
        void onSourceAdded(const QString &name);
        void onSourceRemoved(const QString &name);
        void dataUpdated(const QString &source, Plasma::DataEngine::Data data);
        void showConfigurationInterface();
        void configAccepted();
        void slotOnItemClicked(const QModelIndex &);
        void onTimerExpired();
	void onClickNotifier();
    private slots:
        void storageEjectDone(Solid::ErrorType error, QVariant errorData);
        void storageTeardownDone(Solid::ErrorType error, QVariant errorData);

    private:
        QModelIndex indexForUdi(const QString &udi) const;
        void initSysTray();

        Plasma::DataEngine *m_solidEngine;
        Plasma::DataEngine *m_solidDeviceEngine;
        QStandardItemModel *m_hotplugModel;

        Plasma::Dialog *m_widget;
	Plasma::Icon *m_icon;
	//QGraphicsGridLayout *m_layout;
	QGraphicsLinearLayout *m_layout;
	QGraphicsProxyWidget * m_proxy;
        KDialog *m_dialog;
        int m_displayTime;
        int m_numberItems;
        int m_itemsValidity;
        Notifier::NotifierView *m_notifierView;
        QTimer *m_timer;
        bool isNotificationEnabled;

	/// Designer Config file
        Ui::solidNotifierConfig ui;

};

K_EXPORT_PLASMA_APPLET(devicenotifier, DeviceNotifier)

#endif

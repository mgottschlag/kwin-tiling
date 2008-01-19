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

#include <plasma/applet.h>
#include <plasma/dataengine.h>
#include <plasma/phase.h>
#include <plasma/dialog.h>
#include <KIcon>
#include <QModelIndex>
#include <QTimer>
#include <QListView>

#include "ui_deviceNotifierConfig.h"

class QStandardItemModel;
class KDialog;

//desktop view
namespace Plasma
{
    class VBoxLayout;
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

        DeviceNotifier(QObject *parent, const QVariantList &args);
        ~DeviceNotifier();

        void init();
        Qt::Orientations expandingDirections() const;
        QSizeF contentSizeHint() const;
        void mousePressEvent(QGraphicsSceneMouseEvent *event);
        void hoverEnterEvent (QGraphicsSceneHoverEvent *event);
        void paintInterface(QPainter *p, const QStyleOptionGraphicsItem *option, const QRect &rect);
    public slots:
        void onSourceAdded(const QString &name);
        void onSourceRemoved(const QString &name);
        void dataUpdated(const QString &source, Plasma::DataEngine::Data data);
        void showConfigurationInterface();
        void configAccepted();
        void slotOnItemClicked(const QModelIndex &);
        void slotOnItemDesktopClicked(bool);
        void onTimerExpired();

    private:
        QModelIndex indexForUdi(const QString &udi) const;
        void initDesktop();
        void initSysTray();
        void performSourceAddedInSystray(const QString &name);
        void performSourceAddedInDesktop(const QString &name);
        void performSourceRemovedInSystray(const QString &name);
        void performSourceRemovedInDesktop(const QString &name);
        void performSourceUpdatedInSystray(const QString &source, Plasma::DataEngine::Data data, int nb_actions, const QString &last_action_label);
        void performSourceUpdatedInDesktop(const QString &source, Plasma::DataEngine::Data data, int nb_actions, const QString &last_action_label);

        KIcon m_icon;
        Plasma::DataEngine *m_solidEngine;
        QStandardItemModel *m_hotplugModel;

        Plasma::Dialog *m_widget;
        KDialog *m_dialog;
        int m_displayTime;
        int m_numberItems;
        int m_itemsValidity;
        bool isOnDesktop;
        QTimer *m_timer;

        //desktop view
        Plasma::VBoxLayout *m_layout_list;
        QMap<QString,ItemType> m_map_item;
        /// Designer Config file
        Ui::solidNotifierConfig ui;

};

K_EXPORT_PLASMA_APPLET(devicenotifier, DeviceNotifier)

#endif

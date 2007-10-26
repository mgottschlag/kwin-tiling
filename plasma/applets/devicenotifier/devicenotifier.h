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
#include <KIcon>
#include <QTimer>

#include "ui_deviceNotifierConfig.h"

namespace Plasma
{
    class Svg;
    class Widget;
    class Label;
    class Icon;
    class HBoxLayout;
    class ProgressBar;
} // namespace Plasma

class KDialog;

class DeviceNotifier : public Plasma::Applet
{
    Q_OBJECT


    public:
        DeviceNotifier(QObject *parent, const QVariantList &args);
        ~DeviceNotifier();

        void paintInterface(QPainter *painter, const QStyleOptionGraphicsItem *option, const QRect &contentsRect);
        void mousePressEvent(QGraphicsSceneMouseEvent *event);
		void moveUp();

    public slots:
        void SourceAdded(const QString &name);
        void updated(const QString &source, Plasma::DataEngine::Data data);
        void moveDown();
        void hideNotifier(QGraphicsItem * item);
		void showConfigurationInterface();
		void configAccepted();

    private:
        Plasma::Label * m_label;
		Plasma::Icon * m_icon;
        QFont m_font;
		QRectF origin_size;
		
        Plasma::HBoxLayout *m_layout;
        bool icon;
		bool first;

        QTimer *t;
        Plasma::DataEngine* SolidEngine;
        QStringList desktop_files;
        QString m_udi;
        QString device_name;
		int m_pixelSize;
		int m_height;
		int m_time;
		KDialog *m_dialog;
        /// Designer Config file
        Ui::solidNotifierConfig ui;

};

K_EXPORT_PLASMA_APPLET(devicenotifier, DeviceNotifier)

#endif

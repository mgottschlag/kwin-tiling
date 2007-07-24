/*
 * Copyright (C) 2007 Menard Alexis <darktears31@gmail.com>
 *
 * This program is free software you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef SOLIDNOTIFIER_H
#define SOLIDNOTIFIER_H


#include <QTimer>
#include <QPainter>
#include <KIcon>
#include <plasma/applet.h>
#include <plasma/dataengine.h>
#include <plasma/phase.h>

class QTimer;

class KDialog;

namespace Plasma
{
    class Svg;
}

class SolidNotifier : public Plasma::Applet
{
    Q_OBJECT
    public:
        SolidNotifier(QObject *parent, const QStringList &args);
        ~SolidNotifier();

        void paintInterface(QPainter *painter, const QStyleOptionGraphicsItem *option, const QRect &contentsRect);
        QRectF boundingRect() const;
        void constraintsUpdated();
        void moveUp();

    public slots:
        void SourceAdded(const QString &name);
        void updated(const QString &source, Plasma::DataEngine::Data data);
        void moveDown();
        void hideNotifier(QGraphicsItem * item);

    private :

        QRectF m_bounds;
        int m_pixelSize;
        QString m_timezone;
        Plasma::Svg* m_theme;
        QString m_udi;
        QString device_name;
        KIcon * k_icon;
        int x;
        int y;
        bool icon;
        QTimer *t;
        Plasma::DataEngine* SolidEngine;
        QStringList desktop_files;
};

K_EXPORT_PLASMA_APPLET(solidnotifier, SolidNotifier)

#endif

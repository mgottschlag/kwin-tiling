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

#include <plasma/applet.h>
#include <plasma/dataengine.h>

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

        void paintInterface(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget =0);
        QRectF boundingRect() const;
        void constraintsUpdated();

    public slots:
        void updated(const QString &name, const Plasma::DataEngine::Data &data);

    protected slots:
        void moveMyself();

    private :

        QRectF m_bounds;
        int m_pixelSize;
        QString m_timezone;
        Plasma::Svg* m_theme;
        bool up_down;
        int x;
        int y;
        QTimer *t;
};

K_EXPORT_PLASMA_APPLET(solidnotifier, SolidNotifier)

#endif

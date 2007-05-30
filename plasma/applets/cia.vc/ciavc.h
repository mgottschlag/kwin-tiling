/***************************************************************************
 *   Copyright (C) 2005,2006,2007 by Siraj Razick                          *
 *   siraj@kdemail.net                                                     *
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

#ifndef CLOCK_H
#define CLOCK_H

#include <QImage>
#include <QPaintDevice>
#include <QLabel>
#include <QPixmap>
#include <QTimer>
#include <QPaintEvent>
#include <QPainter>
#include <QTime>
#include <QX11Info>
#include <QWidget>
#include <QGraphicsItem>
#include <QColor>

#include <applet.h>
#include <dataengine.h>

class QTimer;
class KDialog;
class QCheckBox;

namespace Plasma
{
    class VBoxLayout;
    class DataEngine;
}

class CiaVc : public Plasma::Applet
{
        Q_OBJECT
    public:
        CiaVc(QObject *parent, const QStringList &args);
        ~CiaVc();

        QRectF boundingRect() const;
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget =0);

    public slots:
        void configureDialog();

    private slots:
        void sourceAdded(const QString& source);
        void sourceRemoved(const QString& source);
        void acceptedConfigDialog();

    private:
        Plasma::VBoxLayout* m_layout;
        Plasma::DataEngine* m_engine;
};

K_EXPORT_PLASMA_APPLET(ciavc, CiaVc)

#endif

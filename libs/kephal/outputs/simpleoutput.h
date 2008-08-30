/*
 *   Copyright 2008 Aike J Sommer <dev@aikesommer.name>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2,
 *   or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */


#ifndef KEPHAL_SIMPLEOUTPUT_H
#define KEPHAL_SIMPLEOUTPUT_H

#include "kephal/outputs.h"

#include <QSize>
#include <QPoint>
#include <QObject>


namespace kephal {

    class SimpleOutput : public Output {
        Q_OBJECT
        public:
            SimpleOutput(QObject * parent, QString id, QSize resolution, QPoint position, bool connected, bool activated);
            SimpleOutput(QObject * parent);
            SimpleOutput(QObject * parent, Output * output);
            
            QString id();

            QSize size();
            QSize preferredSize();
            QList<QSize> availableSizes();
            QPoint position();
            QString vendor();
            int productId();
            unsigned int serialNumber();
            
            bool isConnected();
            bool isActivated();
            
            Rotation rotation();
            bool reflectX();
            bool reflectY();
            float rate();
            QList<float> availableRates();

            void _setId(const QString & id);
            void _setSize(const QSize & size);
            void _setPreferredSize(const QSize & size);
            void _setAvailableSizes(const QList<QSize> & sizes);
            void _setPosition(const QPoint & position);
            void _setActivated(bool activated);
            void _setConnected(bool connected);
            void _setVendor(const QString &  vendor);
            void _setProductId(int productId);
            void _setSerialNumber(unsigned int serialNumber);
            void _setRotation(Rotation rotation);
            void _setReflectX(bool reflect);
            void _setReflectY(bool reflect);
            void _setRate(float rate);
            void _setAvailableRates(const QList<float> & rates);
            
        private:
            QString m_id;
            QSize m_size;
            QSize m_preferredSize;
            QList<QSize> m_availableSizes;
            QPoint m_position;
            bool m_activated;
            bool m_connected;
            QString m_vendor;
            int m_productId;
            unsigned int m_serialNumber;
            Rotation m_rotation;
            bool m_reflectX;
            bool m_reflectY;
            float m_rate;
            QList<float> m_rates;
    };
    
}


#endif // KEPHAL_SIMPLESCREEN_H


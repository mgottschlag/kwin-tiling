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


#ifndef KEPHAL_CONFIGURATIONS_H
#define KEPHAL_CONFIGURATIONS_H

#include <QObject>
#include <QMap>
#include <QPoint>
#include <QRect>


namespace kephal {

    class Output;

    class Configuration : public QObject {
        Q_OBJECT
        public:
            Configuration(QObject * parent);

            virtual QString name() = 0;
            virtual bool modifiable() = 0;
            virtual bool isActivated() = 0;
            virtual QMap<int, QPoint> layout() = 0;
            
            QMap<int, QRect> realLayout(QMap<int, QPoint> simpleLayout, const QMap<Output *, int> & outputScreens);
            QMap<int, QRect> realLayout(const QMap<Output *, int> & outputScreens);
            QMap<int, QRect> realLayout();
            
        public Q_SLOTS:
            virtual void activate() = 0;
            
        private:
            void simpleToReal(QMap<int, QPoint> & simpleLayout, const QMap<int, QSize> & screenSizes, const int & index, QMap<int, QRect> & screens);
    };
    

    class Configurations : public QObject {
        Q_OBJECT
        public:
            static Configurations * instance();
            
            Configurations(QObject * parent);
            virtual QMap<QString, Configuration *> configurations() = 0;
            virtual Configuration * findConfiguration() = 0;
            virtual Configuration * activeConfiguration() = 0;
            virtual QList<Configuration *> alternateConfigurations() = 0;
            virtual QList<QPoint> possiblePositions(Output *) = 0;
            
            static void translateOrigin(QMap<int, QPoint> & layout);
            static void translateOrigin(QMap<int, QPoint> & layout, QPoint origin);
            static void translateOrigin(QMap<int, QRect> & layout);
            
        protected:
            static Configurations * m_instance;
    };
    
}


#endif // KEPHAL_CONFIGURATIONS_H


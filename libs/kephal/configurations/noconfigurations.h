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


#ifndef KEPHAL_NOCONFIGURATIONS_H
#define KEPHAL_NOCONFIGURATIONS_H

#include "kephal/configurations.h"


namespace Kephal {

    class NoConfigurations;

    class SimpleConfiguration : public Configuration {
        Q_OBJECT
        public:
            SimpleConfiguration(NoConfigurations * parent);
            
            QString name();
            bool isModifiable();
            bool isActivated();
            QMap<int, QPoint> layout();
            int primaryScreen();
            
        public Q_SLOTS:
            void activate();
    };

    class NoConfigurations : public Configurations {
        Q_OBJECT
        public:
            NoConfigurations(QObject * parent);
            
            QMap<QString, Configuration *> configurations();
            Configuration * findConfiguration();
            Configuration * activeConfiguration();
            QList<Configuration *> alternateConfigurations();
            QList<QPoint> possiblePositions(Output * output);
            bool move(Output * output, const QPoint & position);
            bool resize(Output * output, const QSize & size);
            bool rotate(Output * output, Rotation rotation);
            bool changeRate(Output * output, float rate);
            bool reflectX(Output * output, bool reflect);
            bool reflectY(Output * output, bool reflect);
            int screen(Output * output);
            void applyOutputSettings();
            void setPolling(bool polling);
            bool polling();
            void confirm();
            void revert();
            
        private:
            SimpleConfiguration * m_config;
    };
    
}

#endif // KEPHAL_NOCONFIGURATIONS_H


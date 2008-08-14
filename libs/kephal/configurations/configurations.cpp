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


#include <QDebug>

#include "configurations.h"

#ifdef CONFIGURATIONS_FACTORY
void CONFIGURATIONS_FACTORY();
#endif

#include "outputs/outputs.h"
#include "screens/screens.h"


namespace kephal {

    Configurations * Configurations::instance() {
#ifdef CONFIGURATIONS_FACTORY
        if (Configurations::m_instance == 0) {
            CONFIGURATIONS_FACTORY();
        }
#endif
        return Configurations::m_instance;
    }
    
    Configurations::Configurations(QObject * parent)
            : QObject(parent)
    {
        Configurations::m_instance = this;
    }
    
    Configurations * Configurations::m_instance = 0;
    
    void Configurations::translateOrigin(QMap<int, QPoint> & layout) {
        QPoint origin;
        bool first = true;
        for (QMap<int, QPoint>::const_iterator i = layout.constBegin(); i != layout.constEnd(); ++i) {
            if (first || (i.value().x() < origin.x())) {
                origin.setX(i.value().x());
            }
            if (first || (i.value().y() < origin.y())) {
                origin.setY(i.value().y());
            }
            first = false;
        }
        translateOrigin(layout, origin);
    }
    
    void Configurations::translateOrigin(QMap<int, QPoint> & layout, QPoint origin) {
        for (QMap<int, QPoint>::iterator i = layout.begin(); i != layout.end(); ++i) {
            i.value() -= origin;
        }
    }
    
    void Configurations::translateOrigin(QMap<int, QRect> & layout) {
        QPoint origin;
        bool first = true;
        for (QMap<int, QRect>::const_iterator i = layout.constBegin(); i != layout.constEnd(); ++i) {
            if (first || (i.value().x() < origin.x())) {
                origin.setX(i.value().x());
            }
            if (first || (i.value().y() < origin.y())) {
                origin.setY(i.value().y());
            }
            first = false;
        }
        QPoint offset(0, 0);
        offset -= origin;
        for (QMap<int, QRect>::iterator i = layout.begin(); i != layout.end(); ++i) {
            i.value().translate(offset);
        }
    }
    
    
    
    Configuration::Configuration(QObject * parent)
            : QObject(parent)
    {
    }
    
    QMap<int, QRect> Configuration::realLayout() {
        QMap<Output *, int> outputScreens;
        foreach (Output * output, Outputs::instance()->outputs()) {
            Screen * screen = output->screen();
            outputScreens.insert(output, screen ? screen->id() : -1);
        }
        return realLayout(outputScreens);
    }

    QMap<int, QRect> Configuration::realLayout(const QMap<Output *, int> & outputScreens) {
        QMap<int, QPoint> simpleLayout = layout();
        return realLayout(simpleLayout, outputScreens);
    }
    
    QMap<int, QRect> Configuration::realLayout(QMap<int, QPoint> simpleLayout, const QMap<Output *, int> & outputScreens) {
        QMap<int, QRect> screens;
        
        QMap<int, QSize> screenSizes;
        foreach (int screen, simpleLayout.keys()) {
            screenSizes.insert(screen, QSize());
        }
        
        foreach (Output * output, outputScreens.keys()) {
            if (outputScreens[output] < 0) {
                continue;
            }
            
            if (! screenSizes.contains(outputScreens[output])) {
                qDebug() << "outputs and configuration dont match";
                return screens;
            }
            screenSizes[outputScreens[output]] = screenSizes[outputScreens[output]].expandedTo(output->isActivated() ? output->size() : output->preferredSize());
        }
        
        int begin = simpleLayout.begin().key();
        screens.insert(begin, QRect(QPoint(0, 0), screenSizes[begin]));
        simpleToReal(simpleLayout, screenSizes, begin, screens);
        Configurations::translateOrigin(screens);
        
        for (QMap<int, QRect>::const_iterator i = screens.constBegin(); i != screens.constEnd(); ++i) {
            for (QMap<int, QRect>::const_iterator j = (i + 1); j != screens.constEnd(); ++j) {
                if (i.value().intersects(j.value())) {
                    qDebug() << "invalid configuration (overlapping):" << name() << screens;
                    screens.clear();
                    return screens;
                }
            }
        }
        
        return screens;
    }
    
    void Configuration::simpleToReal(QMap<int, QPoint> & simpleLayout, const QMap<int, QSize> & screenSizes, const int & index, QMap<int, QRect> & screens) {
        QPoint pos = simpleLayout.take(index);
        
        // to the right
        QPoint nextPos(pos.x() + 1, pos.y());
        int nextIndex = simpleLayout.key(nextPos, -1);
        if (nextIndex >= 0) {
            screens.insert(nextIndex, QRect(screens[index].topRight() + QPoint(1, 0), screenSizes[nextIndex]));
            simpleToReal(simpleLayout, screenSizes, nextIndex, screens);
        }
        
        // to the left
        nextPos = QPoint(pos.x() - 1, pos.y());
        nextIndex = simpleLayout.key(nextPos, -1);
        if (nextIndex >= 0) {
            QSize screenSize = screenSizes[nextIndex];
            screens.insert(nextIndex, QRect(screens[index].topLeft() - QPoint(screenSize.width(), 0), screenSize));
            simpleToReal(simpleLayout, screenSizes, nextIndex, screens);
        }
        
        // to the bottom
        nextPos = QPoint(pos.x(), pos.y() + 1);
        nextIndex = simpleLayout.key(nextPos, -1);
        if (nextIndex >= 0) {
            screens.insert(nextIndex, QRect(screens[index].bottomLeft() + QPoint(0, 1), screenSizes[nextIndex]));
            simpleToReal(simpleLayout, screenSizes, nextIndex, screens);
        }

        // to the top
        nextPos = QPoint(pos.x(), pos.y() - 1);
        nextIndex = simpleLayout.key(nextPos, -1);
        if (nextIndex >= 0) {
            QSize screenSize = screenSizes[nextIndex];
            screens.insert(nextIndex, QRect(screens[index].topLeft() - QPoint(0, screenSize.height()), screenSize));
            simpleToReal(simpleLayout, screenSizes, nextIndex, screens);
        }
    }
    
}


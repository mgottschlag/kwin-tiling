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


#include "desktopwidgetscreens.h"
#include "../simplescreen.h"

#include <QDesktopWidget>
#include <QApplication>
#include <QDebug>


namespace kephal {

    DesktopWidgetScreens::DesktopWidgetScreens(QObject * parent)
            : Screens(parent)
    {
        QDesktopWidget * desktop = QApplication::desktop();
        for (int i = 0; i < desktop->numScreens(); i++) {
            QRect geom = desktop->screenGeometry(i);
            qDebug() << "adding a screen" << i << "with geom: " << geom;
            
            SimpleScreen * screen = new SimpleScreen(i,
                    geom.size(),
                    geom.topLeft(),
                    false,
                    i == 0);
            connect(screen, SIGNAL(selectedAsPrimary(SimpleScreen *)),
                    this, SLOT(selectedAsPrimary(SimpleScreen *)));
            _screens.append(screen);
        }
        _primaryScreen = _screens.at(0);
        
        connect(desktop, SIGNAL(resized(int)), this, SLOT(screenChanged(int)));
    }
    
    DesktopWidgetScreens::~DesktopWidgetScreens() {
        foreach(Screen * screen, _screens) {
            delete screen;
        }
        _screens.clear();
    }

    QList<Screen *> DesktopWidgetScreens::getScreens()
    {
        QList<Screen *> result;
        foreach(SimpleScreen * screen, _screens) {
            result.append(screen);
        }
        return result;
    }
    
    void DesktopWidgetScreens::screenChanged(int screen)
    {
        QDesktopWidget * desktop = QApplication::desktop();
        for(int i = _screens.size() - 1; i >= desktop->numScreens(); i--) {
            qDebug() << "removing screen" << i;
            SimpleScreen * screen = _screens.takeLast();
            emit screenRemoved(screen);
        }
        
        for(int i = 0; i < _screens.size(); i++) {
            SimpleScreen * screen = _screens.at(i);
            QRect geom = desktop->screenGeometry(i);
            if (screen->getPosition() != geom.topLeft()) {
                QPoint oldPos = screen->getPosition();
                QPoint newPos = geom.topLeft();
                qDebug() << "screen" << i << "moved" << oldPos << "->" << newPos;
                
                screen->_setPosition(newPos);
                emit screenMoved(screen, oldPos, newPos);
            }
            if (screen->getResolution() != geom.size()) {
                QSize oldSize = screen->getResolution();
                QSize newSize = geom.size();
                qDebug() << "screen" << i << "resized" << oldSize << "->" << newSize;
                
                screen->_setResolution(newSize);
                emit screenResized(screen, oldSize, newSize);
            }
        }
        
        for(int i = _screens.size(); i < desktop->numScreens(); i++) {
            QRect geom = desktop->screenGeometry(i);
            qDebug() << "adding a screen" << i << "with geom: " << geom;
            
            SimpleScreen * screen = new SimpleScreen(i,
                    geom.size(),
                    geom.topLeft(),
                    false,
                    false);
            connect(screen, SIGNAL(selectedAsPrimary(SimpleScreen *)),
                    this, SLOT(selectedAsPrimary(SimpleScreen *)));
            _screens.append(screen);
        }
    }
    
    void DesktopWidgetScreens::selectedAsPrimary(SimpleScreen * screen) {
        if (screen == _primaryScreen) {
            return;
        }
        _primaryScreen->_setPrimary(false);
        _primaryScreen = screen;
        screen->_setPrimary(true);
    }

}


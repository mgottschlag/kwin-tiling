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
            
            SimpleScreen * screen = new SimpleScreen(this,
                    i,
                    geom.size(),
                    geom.topLeft(),
                    false,
                    i == 0);
            connect(screen, SIGNAL(selectedAsPrimary(SimpleScreen *)),
                    this, SLOT(selectedAsPrimary(SimpleScreen *)));
            m_screens.append(screen);
        }
        m_primaryScreen = m_screens.at(0);
        
        connect(desktop, SIGNAL(resized(int)), this, SLOT(screenChanged(int)));
    }
    
    DesktopWidgetScreens::~DesktopWidgetScreens() {
        foreach(Screen * screen, m_screens) {
            delete screen;
        }
        m_screens.clear();
    }

    QList<Screen *> DesktopWidgetScreens::screens()
    {
        QList<Screen *> result;
        foreach(SimpleScreen * screen, m_screens) {
            result.append(screen);
        }
        return result;
    }
    
    void DesktopWidgetScreens::screenChanged(int screen)
    {
        QDesktopWidget * desktop = QApplication::desktop();
        for(int i = m_screens.size() - 1; i >= desktop->numScreens(); i--) {
            qDebug() << "removing screen" << i;
            SimpleScreen * screen = m_screens.takeLast();
            emit screenRemoved(screen);
            delete screen;
        }
        
        for(int i = 0; i < m_screens.size(); i++) {
            SimpleScreen * screen = m_screens.at(i);
            QRect geom = desktop->screenGeometry(i);
            if (screen->position() != geom.topLeft()) {
                QPoint oldPos = screen->position();
                QPoint newPos = geom.topLeft();
                qDebug() << "screen" << i << "moved" << oldPos << "->" << newPos;
                
                screen->_setPosition(newPos);
                emit screenMoved(screen, oldPos, newPos);
            }
            if (screen->size() != geom.size()) {
                QSize oldSize = screen->size();
                QSize newSize = geom.size();
                qDebug() << "screen" << i << "resized" << oldSize << "->" << newSize;
                
                screen->_setSize(newSize);
                emit screenResized(screen, oldSize, newSize);
            }
        }
        
        for(int i = m_screens.size(); i < desktop->numScreens(); i++) {
            QRect geom = desktop->screenGeometry(i);
            qDebug() << "adding a screen" << i << "with geom: " << geom;
            
            SimpleScreen * screen = new SimpleScreen(this,
                    i,
                    geom.size(),
                    geom.topLeft(),
                    false,
                    false);
            connect(screen, SIGNAL(selectedAsPrimary(SimpleScreen *)),
                    this, SLOT(selectedAsPrimary(SimpleScreen *)));
            m_screens.append(screen);
        }
    }
    
    void DesktopWidgetScreens::selectedAsPrimary(SimpleScreen * screen) {
        if (screen == m_primaryScreen) {
            return;
        }
        m_primaryScreen->_setPrimary(false);
        m_primaryScreen = screen;
        screen->_setPrimary(true);
    }

}


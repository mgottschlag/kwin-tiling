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


#include "kephal/screens.h"
#include "kephal/configurations.h"

#include <QApplication>
#include <QDesktopWidget>

#ifdef SCREENS_FACTORY
void SCREENS_FACTORY();
#endif


namespace Kephal {

    Screens * Screens::self() {
#ifdef SCREENS_FACTORY
        if (Screens::m_instance == 0) {
            SCREENS_FACTORY();
        }
#endif
        return Screens::m_instance;
    }
    
    Screens::Screens(QObject * parent)
            : QObject(parent)
    {
        Screens::m_instance = this;
    }
    
    Screens::~Screens()
    {
        Screens::m_instance = 0;
    }
    
    Screen * Screens::screen(int id) {
        foreach (Screen * screen, screens()) {
            if (screen->id() == id) {
                return screen;
            }
        }
        return 0;
    }
    
    Screen * Screens::primaryScreen() {
        Configuration * config = Configurations::self()->activeConfiguration();
        if (! config) {
            return 0;
        }
        int id = config->primaryScreen();
        
        return screen(id);
    }
    
    Screens * Screens::m_instance = 0;
    
    
    
    Screen::Screen(QObject * parent)
        : QObject(parent)
    {
    }
    
    QRect Screen::geom() {
        return QRect(position(), size());
    }
    
    bool Screen::isPrimary() {
        return Screens::self()->primaryScreen() == this;
    }
    
    int ScreenUtils::numScreens() {
        return Screens::self()->screens().size();
    }
    
    QRect ScreenUtils::screenGeometry(int id) {
        if (id >= numScreens()) 
            return QRect();
        
        if (id == -1) 
            return QApplication::desktop()->screenGeometry();
        else
            return Screens::self()->screen(id)->geom();
    }
    
    QSize ScreenUtils::screenSize(int id) {
        if (id >= numScreens()) 
            return QSize();
        
        if (id == -1) 
            return QApplication::desktop()->screenGeometry().size();
        else
            return Screens::self()->screen(id)->size();
    }            

    QRect ScreenUtils::desktopGeometry() {
        return QApplication::desktop()->geometry();
    }
    
    int ScreenUtils::distance(const QRect & r, const QPoint & p) {
        if (! r.isValid()) {
            return p.manhattanLength();
        } else if (r.contains(p)) {
            return 0;
        } else if (p.x() >= r.left() && p.x() <= r.right()) {
            return p.y() < r.top() ? (r.top() - p.y()) : (p.y() - r.bottom());
        } else if (p.y() >= r.top() && p.y() <= r.bottom()) {
            return p.x() < r.left() ? (r.left() - p.x()) : (p.x() - r.right());
        } else if (p.x() < r.left()) {
            return ((p.y() < r.top() ? r.topLeft() : r.bottomLeft()) - p).manhattanLength();
        } else {
            return ((p.y() < r.top() ? r.topRight() : r.bottomRight()) - p).manhattanLength();
        }
    }
    
    int ScreenUtils::screenId(QPoint p) {
        if (numScreens() == 0) {
            return 0;
        }
        
        int minDist = distance(screenGeometry(0), p);
        int minScreen = 0;
        
        for(int i = 1; i < numScreens() && minDist > 0; i++) {
            int dist = distance(screenGeometry(i), p);
            if (dist < minDist) {
                minDist = dist;
                minScreen = i;
            }
        }
        
        return minScreen;
    }
    
    int ScreenUtils::primaryScreenId() {
        return Screens::self()->primaryScreen()->id();
    }

}


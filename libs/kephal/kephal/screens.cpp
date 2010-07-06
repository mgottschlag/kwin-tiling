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


#include "screens.h"
#include "configurations.h"

#include <QApplication>
#include <QDesktopWidget>

#ifdef SCREENS_FACTORY
void SCREENS_FACTORY();
#endif

#include <QDebug>

namespace Kephal {

    Screens * Screens::self() {
#ifdef SCREENS_FACTORY
        if (Screens::s_instance == 0) {
            SCREENS_FACTORY();
        }
#endif
        return Screens::s_instance;
    }

    Screens::Screens(QObject * parent)
            : QObject(parent)
    {
        Screens::s_instance = this;
    }

    Screens::~Screens()
    {
        Screens::s_instance = 0;
    }

    Screen * Screens::screen(int id) {
        foreach (Screen * screen, screens()) {
            if (screen->id() == id) {
                return screen;
            }
        }
        return 0;
    }

    Screen * Screens::primaryScreen()
    {
#if 1
        return screen(QApplication::desktop()->primaryScreen());
#else
        Configuration * config = Configurations::self()->activeConfiguration();
        if (! config) {
            return 0;
        }
        int id = config->primaryScreen();

        return screen(id);
#endif
    }

    Screens * Screens::s_instance = 0;



    Screen::Screen(QObject * parent)
        : QObject(parent)
    {
    }

    QRect Screen::geom() {
        return QRect(position(), size());
    }

    bool Screen::isPrimary() const {
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
        //QApplication::desktop()->geometry() was used before, 
        //but returns the wrong size just after a screen has been added
        QRect desktopRect;
        for(int i = 0; i < numScreens(); i++){
            desktopRect |= screenGeometry(i);
        }

        return desktopRect;
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
        if (!Screens::self()->primaryScreen()) {
            return 0;
        }

        return Screens::self()->primaryScreen()->id();
    }

}


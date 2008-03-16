/*
 *   Copyright 2006, 2007 Aaron Seigo <aseigo@kde.org>
 *   Copyright 2007 Matt Broadstone <mbroadst@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
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

#ifndef ROOTWIDGET_H
#define ROOTWIDGET_H

#include <QList>
#include <QWidget>

namespace Plasma
{
    class Containment;
}

class DesktopView;

/**
 * @short The base widget that contains the desktop
 */
class RootWidget : public QWidget
{
    Q_OBJECT

    public:
        RootWidget();
        ~RootWidget();

        /**
         * Sets this RootWidget as a desktop window if @p asDesktop is
         * true or an ordinary window otherwise.
         *
         * Desktop windows are displayed beneath all other windows, have
         * no window decoration and occupy the full size of the desktop.
         *
         * RootWidget instances are automatically set as desktop windows
         * when they are created.  
         */
        void setAsDesktop(bool asDesktop);

        /** 
         * Returns true if this widget is currently a desktop window.
         * See setAsDesktop()
         */
        bool isDesktop() const;

        /**
         * Creates a view for the given containment
         */
        void createDesktopView(Plasma::Containment *containment);

        /**
         * Returns the view, if any, for the given screen
         */
        DesktopView* viewForScreen(int screen) const;

    public slots:
        void toggleDashboard();

    protected slots:
        void adjustSize(int screen);
        void screenOwnerChanged(int wasScreen, int isScreen, Plasma::Containment* containment);

    private:
        QList<DesktopView*> m_desktops;
};

#endif

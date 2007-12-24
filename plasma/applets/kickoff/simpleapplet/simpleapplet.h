/*  
    Copyright 2007 Robert Knight <robertknight@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef SIMPLEAPPLET_H
#define SIMPLEAPPLET_H

// Plasma
#include <plasma/applet.h>

class QAction;

class MenuLauncherApplet : public Plasma::Applet
{
Q_OBJECT

public:
        MenuLauncherApplet(QObject *parent, const QVariantList &args);
        virtual ~MenuLauncherApplet();

        void init();

        virtual QSizeF sizeHint() const;
        void constraintsUpdated(Plasma::Constraints constraints);
        Qt::Orientations expandingDirections() const;

public slots:
        void showConfigurationInterface();

protected slots:
        void configAccepted();
        void toggleMenu(bool pressed);
        void actionTriggered(QAction *action);

private:
        class Private;
        Private * const d;
};

K_EXPORT_PLASMA_APPLET(menulauncher, MenuLauncherApplet)

#endif

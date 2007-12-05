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

// KDE
#include <KIcon>

// Plasma
#include <plasma/applet.h>

class QGraphicsSceneMouseEvent;

namespace Kickoff
{
    class MenuView;
}
namespace Plasma
{
    class Icon;
    class PushButton;
}

class MenuLauncherApplet : public Plasma::Applet
{
Q_OBJECT

public:
        MenuLauncherApplet(QObject *parent, const QVariantList &args);
        virtual ~MenuLauncherApplet();

        QSizeF contentSizeHint() const;
        Qt::Orientations expandingDirections() const;

protected slots:
        void toggleMenu(bool pressed, QGraphicsSceneMouseEvent *event);

private:
        Kickoff::MenuView *m_menuview;
        Plasma::Icon *m_icon;
};

K_EXPORT_PLASMA_APPLET(menulauncher, MenuLauncherApplet)

#endif

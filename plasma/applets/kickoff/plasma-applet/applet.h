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

#ifndef APPLET_H
#define APPLET_H

// KDE
#include <KIcon>

// Plasma
#include <plasma/applet.h>

class QGraphicsSceneMouseEvent;

namespace Kickoff
{
    class Launcher;
}
namespace Plasma
{
    class Icon;
    class PushButton;
}

class LauncherApplet : public Plasma::Applet
{
Q_OBJECT

public:
        LauncherApplet(QObject *parent, const QVariantList &args);
        virtual ~LauncherApplet();

        virtual void paintInterface(QPainter *painter, 
                                    const QStyleOptionGraphicsItem *option, 
                                    const QRect &contentsRect);
        QSizeF contentSizeHint() const;
        Qt::Orientations expandingDirections() const;
        virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

private Q_SLOTS:
        void launcherButtonClicked();

private:
        Kickoff::Launcher *m_launcher;
        Plasma::PushButton *m_button;
        QIcon m_icon;
};

K_EXPORT_PLASMA_APPLET(launcher, LauncherApplet)

#endif

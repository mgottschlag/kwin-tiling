/*
*   Copyright 2007 by Aaron Seigo <aseigo@kde.org>
*   Copyright 2008 by Alexis Ménard <darktears31@gmail.com>
*
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU Library General Public License version 2,
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

#ifndef PLASMA_DESKTOP_H
#define PLASMA_DESKTOP_H

#include <QGraphicsItem>
#include <QObject>
#include <QStyleOptionGraphicsItem>


#include <Plasma/Containment>
#include <Plasma/Animator>

#include "desktoplayout.h"

class QTimer;

namespace Plasma
{
}

/*class Tool : public QObject, public QGraphicsItem
{
    Q_OBJECT

public:
    explicit Tool(QGraphicsItem *parent = 0);
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

};*/

class DefaultDesktop : public Plasma::Containment
{
    Q_OBJECT

public:
    DefaultDesktop(QObject *parent, const QVariantList &args);
    ~DefaultDesktop();
    void constraintsEvent(Plasma::Constraints constraints);

protected:
    void dropEvent(QGraphicsSceneDragDropEvent *event);
    void keyPressEvent(QKeyEvent *event);

protected Q_SLOTS:
    void onAppletAdded(Plasma::Applet *, const QPointF &);
    void onAppletRemoved(Plasma::Applet *);
    void onAppletTransformedByUser();
    void onAppletTransformedItself();
    void refreshWorkingArea();

private:
    DesktopLayout *m_layout;
    QTimer *m_delayedRefreshTimer;
    int m_refreshFails;
    bool dropping;
    bool m_startupCompleted;

    static const int DELAYED_REFRESH_WAIT = 100;
    static const int MAX_REFRESH_FAILS = 20;
};

#endif // PLASMA_PANEL_H

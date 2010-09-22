/*
 * Copyright 20010 by Marco Martin <notmart@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */

#ifndef PANELAPPLETHANDLE_H
#define PANELAPPLETHANDLE_H

#include <Plasma/Dialog>
#include <Plasma/Svg>

class QBoxLayout;
class QPropertyAnimation;
class QLabel;
class QTimer;

namespace Plasma
{
    class Applet;
    class Svg;
}

class ToolButton;

class PanelAppletHandle : public Plasma::Dialog
{
    Q_OBJECT
public:
    PanelAppletHandle(QWidget * parent = 0, Qt::WindowFlags f =  Qt::Window);
    ~PanelAppletHandle();

    void setApplet(Plasma::Applet *applet);
    void startHideTimeout();

protected:
    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

protected Q_SLOTS:
    void configureApplet();
    void closeApplet();
    void appletDestroyed();
    void updatePalette();

Q_SIGNALS:
    void mousePressed(Plasma::Applet *applet, QMouseEvent *event);
    void mouseMoved(Plasma::Applet *applet, QMouseEvent *event);
    void mouseReleased(Plasma::Applet *applet, QMouseEvent *event);

private:
    void moveToApplet();

    QBoxLayout *m_layout;
    ToolButton *m_configureButton;
    ToolButton *m_closeButton;
    Plasma::Svg *m_icons;
    QLabel *m_title;
    QWeakPointer<Plasma::Applet> m_applet;
    QTimer *m_hideTimer;
    QPropertyAnimation *m_moveAnimation;
};

#endif

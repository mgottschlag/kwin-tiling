/*
 *   Copyright 2009 Marco Martin <notmart@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
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

#ifndef APPLETTITLEBAR_H
#define APPLETTITLEBAR_H

#include <QGraphicsWidget>
#include <QParallelAnimationGroup>
#include <QWeakPointer>

#include <plasma/animations/animation.h>

namespace Plasma
{
    class Applet;
    class FrameSvg;
    class Svg;
    class AbstractAnimation;
}

class AppletTitleBar : public QGraphicsWidget
{
    Q_OBJECT

public:
    explicit AppletTitleBar(Plasma::Applet *parent = 0);
    ~AppletTitleBar();

    void setButtonsVisible(bool force);
    bool buttonsVisible() const;

    void setForcedButtonsVisible(bool force);
    bool forcedButtonsVisible() const;

protected:
    void syncSize();
    void syncIconRects();

    //reimplementations
    bool eventFilter(QObject *watched, QEvent *event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void resizeEvent(QGraphicsSceneResizeEvent *event);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    void initAnimations();

protected Q_SLOTS:
    void syncMargins();
    void appletRemoved(Plasma::Applet *applet);
    void themeChanged();

private:
    enum PressedButton{
        NoButton,
        MaximizeButton,
        ConfigureButton,
        CloseButton
    };

    Plasma::Applet *m_applet;

    PressedButton m_pressedButton;
    QRectF m_maximizeButtonRect;
    QRectF m_configureButtonRect;
    QRectF m_closeButtonRect;

    QWeakPointer<QParallelAnimationGroup> m_animations;
    Plasma::Svg *m_icons;
    Plasma::Svg *m_separator;
    Plasma::FrameSvg *m_background;

    Plasma::Animation *m_pulse;

    qreal m_savedAppletTopMargin;
    bool m_underMouse;
    bool m_buttonsVisible;
    bool m_appletHasBackground;
    bool m_forcedButtonsVisible;
};

#endif

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

#ifndef TABBAR_H
#define TABBAR_H

#include <QTabBar>
#include <QTimer>

namespace Kickoff
{

class TabBar : public QTabBar
{
    Q_OBJECT

public:
    TabBar(QWidget *parent);

    /** Like the setCurrentIndex() method but switches the tab without using any
    animation. This is used e.g. within Launcher::reset() to switch back to the
    favorite tab before Kickoff got shown. */
    void setCurrentIndexWithoutAnimation(int index);

    /** Specifies whether hovering switches between tabs or if a click is required to switch the tabs. */
    void setSwitchTabsOnHover(bool switchOnHover);
    bool switchTabsOnHover() const;
    void setAnimateSwitch(bool animateSwitch);
    bool animateSwitch();
    void setShape( Shape shape );

protected:
    int lastIndex() const;

    // reimplemented from QTabBar
    virtual QSize tabSizeHint(int index) const;
    virtual void paintEvent(QPaintEvent *event);
    virtual void leaveEvent(QEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void resizeEvent(QResizeEvent* event);

    bool isHorizontal() const;
    bool isVertical() const;

protected slots:
    void switchToHoveredTab();
    void animationFinished();
    void startAnimation();
    void onValueChanged(qreal val);

private:
    QPainterPath tabPath(const QRect &r);

    static const int TAB_CONTENTS_MARGIN = 6;
    int m_hoveredTabIndex;
    QTimer m_tabSwitchTimer;
    bool m_switchOnHover;
    bool m_animateSwitch;
    QRect m_currentAnimRect;
    int m_lastIndex[2];
    qreal m_animProgress;

    QSize tabSize(int index) const;
    void storeLastIndex();
};

}

#endif // TABBAR_H

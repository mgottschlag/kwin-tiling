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
#include <QTimeLine>
#include <QVector>

namespace Kickoff
{

class TabBar : public QTabBar
{
Q_OBJECT

public:
    TabBar(QWidget *parent);

    /** Specifies whether hovering switches between tabs or if a click is required to switch the tabs. */
    void setSwitchTabsOnHover(bool switchOnHover);
    bool switchTabsOnHover() const;

protected:
    // reimplemented from QTabBar
    virtual QSize tabSizeHint(int index) const;
    virtual void paintEvent(QPaintEvent *event);
    virtual void leaveEvent(QEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);

protected slots:
    void switchToHoveredTab();
    void animationFinished();
    void startAnimation();

private:
    static const int TAB_CONTENTS_MARGIN = 5;
    int m_hoveredTabIndex;
    QTimer m_tabSwitchTimer;
    QTimeLine m_animator;
    QVector<int> m_animStates;
    bool m_switchOnHover;
};

}

#endif // TABBAR_H


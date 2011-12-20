/*
 *   Copyright (C) 2009 by Ana Cecília Martins <anaceciliamb@gmail.com>
 *   Copyright 2010 Chani Armitage <chani@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library/Lesser General Public License
 *   version 2, or (at your option) any later version, as published by the
 *   Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library/Lesser General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef FILTERBAR_H
#define FILTERBAR_H

#include <QGraphicsWidget>

#include <Plasma/Plasma>

class QGraphicsLinearLayout;
class KMenu;
namespace Plasma {
    class LineEdit;
    class PushButton;
    class TabBar;
}

class FilterBar : public QGraphicsWidget
{
    Q_OBJECT

public:
    explicit FilterBar(Qt::Orientation orientation = Qt::Horizontal, QGraphicsItem *parent = 0);
    virtual ~FilterBar();
    Plasma::LineEdit* textSearch();
    void setOrientation(Qt::Orientation orientation);

public Q_SLOTS:
    void immutabilityChanged(Plasma::ImmutabilityType immutability);
    void setFocus();

Q_SIGNALS:
    void searchTermChanged(const QString &text);
    void addWidgetsRequested();

protected Q_SLOTS:
    void setMenuPos();
    void populateActivityMenu();
    void createActivity(QAction *action);

protected:
    void resizeEvent(QGraphicsSceneResizeEvent *event);

private:

    QGraphicsLinearLayout *m_linearLayout;
    //Plasma::TabBar *m_categoriesTabs;
    Plasma::LineEdit *m_textSearch;
    Qt::Orientation m_orientation;
    Plasma::PushButton *m_addWidgetsButton;
    Plasma::PushButton *m_newActivityButton;
    Plasma::PushButton *m_unlockButton;
    KMenu *m_newActivityMenu;
};

#endif // APPLETSFILTERING_H

/*
 *   Copyright 2009 by Artur Duque de Souza <morpheuz@gmail.com>
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

#ifndef STRIPWIDGET_H
#define STRIPWIDGET_H

#include <QGraphicsWidget>
#include <QGraphicsLinearLayout>
#include <QGraphicsSceneResizeEvent>


namespace Plasma
{
    class IconWidget;
    class Frame;
    class PushButton;
    class QueryMatch;
    class RunnerManager;
}


class StripWidget : public QGraphicsWidget
{
    Q_OBJECT;

public:
    StripWidget(Plasma::RunnerManager *rm, QGraphicsItem *parent = 0);
    ~StripWidget();

    void add(Plasma::QueryMatch match);
    void remove(Plasma::IconWidget *favourite);

protected:
    void createIcon(Plasma::QueryMatch *match, int idx);

private slots:
    void removeFavourite();
    void launchFavourite();
    void goLeft();
    void goRight();

private:
    Plasma::PushButton *leftArrow;
    Plasma::PushButton *rightArrow;
    Plasma::Frame *background;
    QGraphicsLinearLayout *stripLayout;

    Plasma::RunnerManager *runnermg;
    QList<Plasma::QueryMatch*> m_favouritesMatches;
    QMap<Plasma::IconWidget*,Plasma::QueryMatch*> m_favouriteMap;
};

#endif

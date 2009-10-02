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


#include <KConfigGroup>

#include <QPair>
#include <QGraphicsLinearLayout>
#include <QGraphicsSceneResizeEvent>


#include <Plasma/Frame>
#include <Plasma/RunnerContext>

namespace Plasma
{
    class IconWidget;
    class ToolButton;
    class QueryMatch;
    class RunnerManager;
    class ScrollWidget;
}

class QGraphicsGridLayout;
class ItemContainer;

class StripWidget : public Plasma::Frame
{
    Q_OBJECT

public:
    StripWidget(Plasma::RunnerManager *rm, QGraphicsWidget *parent = 0);
    ~StripWidget();

    void add(Plasma::QueryMatch match, const QString &query);
    void remove(Plasma::IconWidget *favourite);

    void save(KConfigGroup &cg);
    void restore(KConfigGroup &cg);

protected:
    void createIcon(Plasma::QueryMatch *match, int idx);
    bool eventFilter(QObject *watched, QEvent *event);
    void resizeEvent(QGraphicsSceneResizeEvent *event);
    void focusInEvent(QFocusEvent *event);

private slots:
    void removeFavourite();
    void launchFavourite();
    void launchFavourite(Plasma::IconWidget *icon);
    void selectFavourite(Plasma::IconWidget *icon);
    void goLeft();
    void goRight();

private:
    Plasma::ToolButton *m_leftArrow;
    Plasma::ToolButton *m_rightArrow;
    QGraphicsLinearLayout *m_arrowsLayout;
    ItemContainer *m_iconsBackground;

    Plasma::RunnerManager *m_runnermg;
    QList<Plasma::QueryMatch*> m_favouritesMatches;
    QHash<Plasma::QueryMatch*, QString> m_favouritesQueries;
    QHash<Plasma::IconWidget*, Plasma::QueryMatch*> m_favouritesIcons;
    Plasma::ScrollWidget *m_scrollWidget;
    Plasma::RunnerContext *m_context;
    int m_shownIcons;
    int m_offset;
    Plasma::IconWidget *m_currentIcon;
    int m_currentIconIndex;
};

#endif

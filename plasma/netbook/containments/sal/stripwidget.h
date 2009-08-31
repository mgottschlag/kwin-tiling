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
#include <QGraphicsWidget>
#include <QGraphicsLinearLayout>
#include <QGraphicsSceneResizeEvent>


#include <Plasma/RunnerContext>

namespace Plasma
{
    class IconWidget;
    class Frame;
    class PushButton;
    class QueryMatch;
    class RunnerManager;
    class ItemBackground;
}


class StripWidget : public QGraphicsWidget
{
    Q_OBJECT

public:
    StripWidget(Plasma::RunnerManager *rm, QGraphicsItem *parent = 0);
    ~StripWidget();

    void add(Plasma::QueryMatch match, const QString &query);
    void remove(Plasma::IconWidget *favourite);

    void save(KConfigGroup &cg);
    void restore(KConfigGroup &cg);

protected:
    void createIcon(Plasma::QueryMatch *match, int idx);
    bool eventFilter(QObject *watched, QEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    void resizeEvent(QGraphicsSceneResizeEvent *event);
    void focusInEvent(QFocusEvent *event);
    void focusOutEvent(QFocusEvent *event);
    void keyPressEvent(QKeyEvent *event);

private slots:
    void removeFavourite();
    void launchFavourite();
    void goLeft();
    void goRight();

private:
    Plasma::PushButton *m_leftArrow;
    Plasma::PushButton *m_rightArrow;
    Plasma::Frame *m_background;
    QGraphicsLinearLayout *m_stripLayout;
    QGraphicsLinearLayout *m_arrowsLayout;

    Plasma::RunnerManager *m_runnermg;
    QList<Plasma::QueryMatch*> m_favouritesMatches;
    QHash<Plasma::QueryMatch*, QString> m_favouritesQueries;
    QHash<Plasma::IconWidget*, Plasma::QueryMatch*> m_favouritesIcons;
    Plasma::RunnerContext *m_context;
    Plasma::ItemBackground *m_hoverIndicator;
    int m_shownIcons;
    int m_offset;
    int m_currentIconIndex;
};

#endif

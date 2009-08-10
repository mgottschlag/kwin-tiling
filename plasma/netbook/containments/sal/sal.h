/*
 *   Copyright 2007 by Alex Merry <alex.merry@kdemail.net>
 *   Copyright 2008 by Alexis Ménard <darktears31@gmail.com>
 *   Copyright 2009 by Marco Martin <notmart@gmail.com>
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

#ifndef SEARCHLAUNCH_CONTAINMENT_H
#define SEARCHLAUNCH_CONTAINMENT_H

#include <Plasma/Containment>
#include <Plasma/DataEngine>

#include <QGraphicsLinearLayout>
#include <QGraphicsGridLayout>

class QAction;
class QTimer;
class StripWidget;

class ItemBackground;

namespace Plasma
{
    class IconWidget;
    class LineEdit;
    class RunnerManager;
    class QueryMatch;
    class ScrollWidget;
    class Frame;
}

class SearchLaunch : public Plasma::Containment
{
    Q_OBJECT
public:
    SearchLaunch(QObject *parent, const QVariantList &args);
    ~SearchLaunch();
    void init();

    QList<QAction*> contextualActions();
    void constraintsEvent(Plasma::Constraints constraints);
    void paintInterface(QPainter *painter,
                        const QStyleOptionGraphicsItem *option,
                        const QRect &contentsRect);
protected:
    bool eventFilter(QObject *watched, QEvent *event);

public slots:
    void dataUpdated(const QString &sourceName, const Plasma::DataEngine::Data &data);

private slots:
    void themeUpdated();
    void updateSize();
    void layoutApplet(Plasma::Applet* applet, const QPointF &pos);
    void appletRemoved(Plasma::Applet* applet);

    void doSearch(const QString query);
    void setQueryMatches(const QList<Plasma::QueryMatch> &m);
    void launch();
    void addFavourite();
    void reset();
    void relayout();

private:
    /**
     * update the formfactor based on the location
     */
    void setFormFactorFromLocation(Plasma::Location loc);

    Plasma::FrameSvg *m_background;
    Plasma::RunnerManager *m_runnermg;
    Plasma::IconWidget *m_homeButton;

    int m_queryCounter;
    int m_maxColumnWidth;
    QTimer *m_relayoutTimer;
    QMultiMap<qreal, Plasma::IconWidget*> m_items;
    QHash<Plasma::IconWidget*, Plasma::QueryMatch> m_matches;

    QGraphicsWidget *m_viewMainWidget;
    Plasma::ScrollWidget *m_gridScroll;
    StripWidget *m_stripWidget;
    ItemBackground *m_hoverIndicator;

    QGraphicsLinearLayout *m_mainLayout;
    QGraphicsGridLayout *m_launchGrid;
};


#endif // PLASMA_SEARCHLAUNCH_H

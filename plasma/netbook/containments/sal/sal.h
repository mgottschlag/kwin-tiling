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
#include <Plasma/IconWidget>

#include <QGraphicsLinearLayout>
#include <QGraphicsGridLayout>

#include <QSignalMapper>

class QAction;
class KDialog;
class KIntNumInput;

namespace Plasma
{
    class IconWidget;
    class LineEdit;
    class RunnerManager;
    class QueryMatch;
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

private slots:
    void layoutApplet(Plasma::Applet* applet, const QPointF &pos);
    void appletRemoved(Plasma::Applet* applet);
    void updateSize();
    void doSearch();
    void setQueryMatches(const QList<Plasma::QueryMatch> &m);
    void launch();
    void launchFavourite();
    void addFavourite();
    void removeFavourite();

private:
    /**
     * update the formfactor based on the location
     */
    void setFormFactorFromLocation(Plasma::Location loc);

    Plasma::LineEdit *tedit;
    Plasma::RunnerManager *runnermg;

    int queryCounter;
    QList<Plasma::IconWidget*> m_items;
    QList<Plasma::IconWidget*> m_favourites;

    QList<Plasma::QueryMatch> m_matches;
    QList<Plasma::QueryMatch> m_favouritesMatches;

    QGraphicsLinearLayout *favourites;
    QGraphicsGridLayout *launchGrid;
};


#endif // PLASMA_SEARCHLAUNCH_H

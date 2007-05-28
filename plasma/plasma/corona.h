/*
 *   Copyright (C) 2007 Matt Broadstone <mbroadst@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
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

#ifndef CORONA_H
#define CORONA_H

#include <QGraphicsView>

#include "plasma.h"
#include "applet.h"

class QGraphicsScene;
namespace Plasma
{
    class Layout;
    class Svg;
}

/**
 * @short The view that displays the all the desktop
 */
class Corona : public QGraphicsView
{
Q_OBJECT

//typedef QHash<QString, QList<Plasma::Applet*> > layouts;

public:
    Corona(QWidget *parent = 0);
    ~Corona();

    /**
     * The location of the Corona. @see Plasma::Location
     */
    Plasma::Location location() const;

    /**
     * Informs the Corona as to what position it is in. This is informational
     * only, as the Corona doesn't change it's actual location. This is,
     * however, passed on to Applets that may be managed by this Corona.
     *
     * @param location the new location of this Corona
     */
    void setLocation(Plasma::Location location);

    /**
     * The current form factor for this Corona. @see Plasma::FormFactor
     **/
    Plasma::FormFactor formFactor() const;

    /**
     * Sets the form factor for this Corona. This may cause changes in both
     * the arrangement of Applets as well as the display choices of individual
     * Applets.
     */
    void setFormFactor(Plasma::FormFactor formFactor);

public Q_SLOTS:
    void addPlasmoid(const QString& name);

protected:
    void resizeEvent(QResizeEvent* event);
    void dragEnterEvent(QDragEnterEvent* event);
    void dragLeaveEvent(QDragLeaveEvent* event);
    void dragMoveEvent(QDragMoveEvent* event);
    void dropEvent(QDropEvent* event);
    void drawBackground(QPainter * painter, const QRectF & rect);

protected Q_SLOTS:
    void displayContextMenu(const QPoint& point);
    void launchExplorer(bool /*param*/);

private:
    QGraphicsScene *m_graphicsScene;
    QAction *m_engineExplorerAction;
    Plasma::Applet::List m_applets;
    Plasma::FormFactor m_formFactor;
    Plasma::Location m_location;
    Plasma::Layout* m_layout;

    //TODO: replace m_background with something actually useful.
    Plasma::Svg* m_background;
};

#endif



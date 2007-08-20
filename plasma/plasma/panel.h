/*
*   Copyright 2007 by Matt Broadstone <mbroadst@kde.org>
*   Copyright 2007 by Robert Knight <robertknight@gmail.com>
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

#ifndef PLASMA_PANEL_H
#define PLASMA_PANEL_H

#include <QGraphicsView>

#include <plasma/plasma.h>

namespace Plasma
{

class Panel : public QGraphicsView
{
    Q_OBJECT
public:

   /**
    * Constructs a new panel.
    * @arg parent the QWidget this panel is parented to
    */
    Panel(QWidget *parent = 0);
    ~Panel();

    /**
     * Sets the location (screen edge) where this panel is positioned.
     * @param location the location to place the panel at
     */
    void setLocation(Plasma::Location location);

    /**
     * @return the location (screen edge) where this panel is positioned.
     */
    Plasma::Location location() const;

    /**
     * Sets the Corona (scene) to be used for this panels items.
     * @param corona the corona to use for this panel.
     */
    void setCorona(Plasma::Corona* corona);

    /**
     * @return the Corona (scene) associated with this panel.
     */
    Plasma::Corona *corona() const;

    /**
     * @return this panel's layout.
     */
    Plasma::Layout *layout() const;

private:
    class Private;
    Private *const d;

};

} // Namespace
#endif


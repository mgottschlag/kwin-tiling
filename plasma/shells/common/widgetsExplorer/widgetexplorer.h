/*
 *   Copyright (C) 2007 by Ivan Cukic <ivan.cukic+kde@gmail.com>
 *   Copyright (C) 2009 by Ana Cecília Martins <anaceciliamb@gmail.com>
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


#ifndef WIDGETEXPLORER_H
#define WIDGETEXPLORER_H

#include <QtGui>

#include <KDE/KDialog>

#include <plasma/framesvg.h>

#include "plasmaappletitemmodel_p.h"

namespace Plasma
{

class Corona;
class Containment;
class Applet;
class WidgetExplorerPrivate;
class WidgetExplorerPrivate;

class WidgetExplorer : public QGraphicsWidget
{

    Q_OBJECT

public:
    WidgetExplorer(QGraphicsItem *parent = 0);
    ~WidgetExplorer();

    QString application();
    void setApplication(const QString &application = QString());
    /**
     * Changes the current default containment to add applets to
     *
     * @arg containment the new default
     */
    void setContainment(Plasma::Containment *containment);

    /**
     * @return the current default containment to add applets to
     */
    Containment *containment() const;
    /**
     * @return the current corona this widget is added to
     */
    Plasma::Corona *corona() const;

    /**
     * Set the orientation of the widgets explorer
     *
     * @arg the new orientation
     */
    void setOrientation(Qt::Orientation orientation);

    Qt::Orientation orientation();

protected:

    void resizeEvent(QGraphicsSceneResizeEvent *event);

Q_SIGNALS:

    void orientationChanged(Qt::Orientation orientation);

public Q_SLOTS:
    /**
     * Adds currently selected applets
     */
    void addApplet();

    /**
     * Adds applet
     */
    void addApplet(PlasmaAppletItem *appletItem);

    /**
     * Destroy all applets with this name
     */
    void destroyApplets(const QString &name);

    /**
     * Launches a download dialog to retrieve new applets from the Internet
     *
     * @arg type the type of widget to download; an empty string means the default
     *           Plasma widgets will be accessed, any other value should map to a
     *           PackageStructure PluginInfo-Name entry that provides a widget browser.
     */
    void downloadWidgets(const QString &type = QString());

    /**
     * Opens a file dialog to open a widget from a local file
     */
    void openWidgetFile();

    void populateWidgetsMenu();

private:
    Q_PRIVATE_SLOT(d, void appletAdded(Plasma::Applet*))
    Q_PRIVATE_SLOT(d, void appletRemoved(Plasma::Applet*))
    Q_PRIVATE_SLOT(d, void containmentDestroyed())

    WidgetExplorerPrivate * const d;

};

} // namespace Plasma

#endif // WIDGETEXPLORER_H

/*
*   Copyright 2007 by Alex Merry <huntedhacker@tiscali.co.uk>
*   Copyright 2008 by Alexis Ménard <darktears31@gmail.com>
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

#ifndef PLASMA_PANEL_H
#define PLASMA_PANEL_H

#include <plasma/containment.h>

class QComboBox;
class QAction;
class KDialog;
class KIntNumInput;

namespace Plasma
{
    class PanelSvg;
}

class Panel : public Plasma::Containment
{
    Q_OBJECT
public:
    Panel(QObject *parent, const QVariantList &args);
    ~Panel();
    void init();
    QList<QAction*> contextualActions();

    void constraintsEvent(Plasma::Constraints constraints);

    void paintInterface(QPainter *painter,
                        const QStyleOptionGraphicsItem *option,
                        const QRect &contentsRect);
    void paintBackground(QPainter *painter, const QRect &contentsRect);

protected:
    void saveState(KConfigGroup* config) const;

private slots:
    void remove();
    void themeUpdated();
    void backgroundChanged();
    void layoutApplet(Plasma::Applet* applet, const QPointF &pos);
    void appletRemoved(Plasma::Applet* applet);

private:
    /**
     * update the formfactor based on the location
     */
    void setFormFactorFromLocation(Plasma::Location loc);

    /**
     * recalculate which borders to show
     */
    void updateBorders(const QRect &geom);

    Plasma::PanelSvg *m_background;
    QAction* m_appletBrowserAction;
    QAction* m_configureAction;
    QAction* m_removeAction;

    //cached values
    QSize m_currentSize;
    QRect m_lastViewGeom;
};


#endif // PLASMA_PANEL_H

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
    QList<QAction*> contextActions();

    void constraintsUpdated(Plasma::Constraints constraints);

    void paintInterface(QPainter *painter,
                        const QStyleOptionGraphicsItem *option,
                        const QRect &contentsRect);
    void paintBackground(QPainter *painter, const QRect &contentsRect);
    /**
     * resize the appropriate dimension to newSize
     * if the panel is horizontal this changes the height
     * if it's vertical this changes the width
     * the other dimension will be resized to the screen width/height.
     */
    void updateSize(const QSize &newSize);

private slots:
    void configure();
    void remove();
    void applyConfig();
    void themeUpdated();
    void sizeComboChanged();
    void backgroundChanged();
    void layoutApplet(Plasma::Applet* applet, const QPointF &pos);

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
    KDialog* m_dialog;
    QComboBox* m_sizeCombo;
    KIntNumInput* m_sizeEdit;
    KIntNumInput* m_lengthEdit;
    QComboBox* m_locationCombo;
    QAction* m_appletBrowserAction;
    QAction* m_configureAction;
    QAction* m_removeAction;

    //cached values
    QSize m_currentSize;
    QRect m_lastViewGeom;
};


#endif // PLASMA_PANEL_H

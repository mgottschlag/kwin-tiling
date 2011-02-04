/*
*   Copyright 2007 by Alex Merry <alex.merry@kdemail.net>
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

#include <Plasma/Containment>

class QAction;
class QTimer;
class QGraphicsLinearLayout;
class Spacer;

namespace Plasma
{
    class FrameSvg;
}

class Panel : public Plasma::Containment
{
    Q_OBJECT
    Q_PROPERTY(QString shadowPath READ shadowPath)

public:
    Panel(QObject *parent, const QVariantList &args);
    ~Panel();
    void init();
    QList<QAction*> contextualActions();

    QString shadowPath() {return "widgets/panel-background";}

    void constraintsEvent(Plasma::Constraints constraints);

    void paintInterface(QPainter *painter,
                        const QStyleOptionGraphicsItem *option,
                        const QRect &contentsRect);
    void paintBackground(QPainter *painter, const QRect &contentsRect);
    void showDropZone(const QPoint pos);

protected:
    void saveState(KConfigGroup &config) const;

    void saveContents(KConfigGroup &group) const;
    void restore(KConfigGroup &group);

private slots:
    void backgroundChanged();
    void layoutApplet(Plasma::Applet* applet, const QPointF &pos);
    void appletWasRemoved(Plasma::Applet* applet);
    void delayedUpdateSize();
    void updateSize();
    void adjustLastSpace();
    void enableUpdateSize();

private:
    /**
     * update the formfactor based on the location
     */
    void setFormFactorFromLocation(Plasma::Location loc);

    /**
     * recalculate which borders to show
     */
    void updateBorders(const QRect &geom, bool inPaintEvent = false);

    Plasma::FrameSvg *m_background;
    QAction* m_configureAction;

    //cached values
    QSize m_currentSize;
    QRect m_lastViewGeom;
    bool m_maskDirty;
    bool m_canResize;
    int m_spacerIndex;
    Spacer *m_spacer;
    Spacer *m_lastSpace;
    QTimer *m_lastSpaceTimer;
    QTimer *m_enableUpdateResizeTimer;
    QTimer *m_updateSizeTimer;
    QGraphicsLinearLayout *m_layout;
    int m_resizedApplets;

    friend class Spacer;
};


#endif // PLASMA_PANEL_H

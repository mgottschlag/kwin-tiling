/*
*   Copyright 2007 by Alex Merry <alex.merry@kdemail.net>
*   Copyright 2008 by Alexis Ménard <darktears31@gmail.com>
*   Copyright 2008 by Aaron Seigo <aseigo@kde.org>
*   Copyright 2009 by Marco Martin <notmart@gmail.com>
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

#ifndef PLASMA_NEWSPAPER_H
#define PLASMA_NEWSPAPER_H

#include <Plasma/Containment>

class AppletOverlay;
class QGraphicsLinearLayout;
class AppletsContainer;
class AppletsView;

namespace Plasma
{
    class FrameSvg;
}

class Newspaper : public Plasma::Containment
{
    Q_OBJECT
    friend class AppletOverlay;

public:
    Newspaper(QObject *parent, const QVariantList &args);
    ~Newspaper();
    void init();

    void constraintsEvent(Plasma::Constraints constraints);

public Q_SLOTS:
    Plasma::Applet *addApplet(const QString &appletName, const int row = -1, const int column = -1);
    Plasma::Applet *addApplet(Plasma::Applet *applet, const int row, const int column);

protected:
    void changeEvent(QEvent *event);

    void saveContents(KConfigGroup &group) const;
    void restore(KConfigGroup &group);

    void setOrientation(Qt::Orientation orientation);
    Qt::Orientation orientation() const;

    void dragEnterEvent(QGraphicsSceneDragDropEvent *event);
    void dragLeaveEvent(QGraphicsSceneDragDropEvent *event);
    void dragMoveEvent(QGraphicsSceneDragDropEvent *event);
    void dropEvent(QGraphicsSceneDragDropEvent *event);

private Q_SLOTS:
    void toggleImmutability();
    void toggleExpandAllApplets();
    void updateSize();
    void appletSizeHintChanged();
    void updateConfigurationMode(bool config);
    void refreshLayout();

    void updateRemoveActionVisibility();
    void containmentAdded(Plasma::Containment *containment);
    void containmentRemoved(QObject *containment);
    void availableScreenRegionChanged();

private:
    AppletsView *m_scrollWidget;
    QGraphicsLinearLayout *m_externalLayout;
    Qt::Orientation m_orientation;
    bool m_expandAll;
    Plasma::FrameSvg *m_background;
    AppletOverlay *m_appletOverlay;
    bool m_dragging;
    QTimer *m_updateSizeTimer;
    QTimer *m_relayoutTimer;
    AppletsContainer *m_container;
};


#endif // PLASMA_PANEL_H

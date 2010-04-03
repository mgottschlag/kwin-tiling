/*
 *   Copyright (C) 2009 by Ana Cecília Martins <anaceciliamb@gmail.com>
 *   Copyright (C) 2009 by Ivan Cukic <ivan.cukic+kde@gmail.com>
 *   Copyright (C) 2010 by Chani Armitage <chani@kde.org>
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

#ifndef ABSTRACTICONLIST_H
#define ABSTRACTICONLIST_H

#include <QTimer>
#include <QGraphicsWidget>
#include <QGraphicsLinearLayout>

#include <Plasma/Svg>
#include <plasma/widgets/toolbutton.h>

#include "abstracticon.h"

namespace Plasma
{
    class Animation;
    class ItemBackground;
    class ToolButton;
} // namespace Plasma

class AbstractIconList : public QGraphicsWidget
{

    Q_OBJECT

public:
    AbstractIconList(Qt::Orientation orientation = Qt::Horizontal, QGraphicsItem *parent = 0);
    ~AbstractIconList();

    void setOrientation(Qt::Orientation orientation);
    Qt::Orientation orientation();

    // default size of the icons
    void setIconSize(int size);
    int iconSize() const;

protected:
    /**
     * add a new icon (or rather, do a bunchofstuff that needs doing when it's added)
     * this could do with some cleanup still.
     */
    void addIcon(AbstractIcon *applet);

    /**
     * Adds the icon to the list layout
     */
    void insertAppletIcon(AbstractIcon *appletIconWidget);

    /**
     * subclasses must implement this:
     * it should insert all items that should be visible
     * FIXME the wording is kinda bad. but first I need to choose whether this stuff is
     * adding/removing from hte layout or simply changing hte visibility of items.
     */
    virtual void populateList() = 0;

    /**
     * subclasses must implement this:
     * respond to changes in the search string (eg. tell your model)
     */
    virtual void setSearch(const QString &searchString) = 0;

    /**
     * scroll to a specific item
     */
    virtual void scrollTo(int index);

private:
    void init();

    //see how many icons is visible at once, approximately
    int maximumAproxVisibleIconsOnList();

    //removes all the icons from the widget
    void eraseList();

    //returns the what's the visible rect of the list widget
    QRectF visibleListRect();
    //returns window's start position
    qreal visibleStartPosition();
    //returns window's end position
    qreal visibleEndPosition();
    //returns list size
    qreal listSize();
    //returns windows size relative to list
    qreal windowSize();
    //returns item position
    qreal itemPosition(int i);

    void wheelEvent(QGraphicsSceneWheelEvent *event);

public slots:
    void searchTermChanged(const QString &text);
    void updateList();

private slots:
    void scrollDownRight();
    void scrollUpLeft();
    void scrollStepFinished();

    //checks if arrows should be enabled or not
    void manageArrows();

    //moves list to position 0,0
    void resetScroll();

    void itemSelected(AbstractIcon *icon);
    void iconHoverEnter(AbstractIcon *icon);

    void setSearch();

protected:
    bool eventFilter(QObject *obj, QEvent *event);

    //Hash containing all widgets that represents the applets
    //FIXME the subclasses use this more than me so maybe they should store it?
    QHash<QString, AbstractIcon *> m_allAppletsHash;
private:
    //list containing the applet icons of the filter proxy model
    QList<AbstractIcon *> m_currentAppearingAppletsOnList;

    QGraphicsLinearLayout *m_appletListLinearLayout;
    QGraphicsWidget *m_appletsListWidget;
    QGraphicsWidget *m_appletsListWindowWidget;
    QGraphicsLinearLayout *m_arrowsLayout;

    Plasma::ToolButton *m_downRightArrow;
    Plasma::ToolButton *m_upLeftArrow;
    Plasma::Svg *m_arrowsSvg;

    Qt::Orientation m_orientation;

    Plasma::ItemBackground *m_hoverIndicator;

    //index of current first item
    //nothing to do with mvc indices.
    int m_firstItemIndex;
    
    AbstractIcon *m_selectedItem;

    QTimer *m_searchDelayTimer;
    QString m_searchString;

    int m_scrollStep;
    int m_iconSize;

    Plasma::Animation *m_slide;
};

#endif //ICONLIST_H

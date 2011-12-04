/*
 *   Copyright (C) 2009 by Ana Cecília Martins <anaceciliamb@gmail.com>
 *   Copyright (C) 2009 by Ivan Cukic <ivan.cukic+kde@gmail.com>
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

#ifndef APPLETSLIST_H
#define APPLETSLIST_H

#include <QBasicTimer>
#include <QTimeLine>

#include "kcategorizeditemsviewmodels_p.h"
#include "plasmaappletitemmodel_p.h"
#include "appleticon.h"
#include "applettooltip.h"
#include "abstracticonlist.h"


class AppletsListWidget : public Plasma::AbstractIconList
{

    Q_OBJECT

public:
    AppletsListWidget(Plasma::Location location = Plasma::BottomEdge, QGraphicsItem *parent = 0);
    ~AppletsListWidget();

    void setItemModel(PlasmaAppletItemModel *model);
    void setFilterModel(QStandardItemModel *model);

private:
    KCategorizedItemsViewModels::AbstractItem *getItemByProxyIndex(const QModelIndex &index) const;
    //Creates a new applet icon and puts it into the hash
    AppletIconWidget *createAppletIcon(PlasmaAppletItem *appletItem);

    void setToolTipPosition();

private slots:
    void populateAllAppletsHash();
    void filterChanged(int index);

    void appletIconDoubleClicked(Plasma::AbstractIcon *icon);
    void appletIconHoverLeave(Plasma::AbstractIcon *icon);
    void appletIconHoverEnter(Plasma::AbstractIcon *icon);
    void appletIconDragging(Plasma::AbstractIcon *icon);
    void onToolTipEnter();
    void onToolTipLeave();

    void animateToolTipMove();
    void toolTipMoveTimeLineFrameChanged(int frame);
    void rowsAboutToBeRemoved(const QModelIndex& parent, int row, int column);

protected: //FIXME wuh?
    void timerEvent(QTimerEvent *event);
    QVariant itemChange(GraphicsItemChange change, const QVariant & value);

    //virtuals from AbstractIconList
    void updateVisibleIcons();
    void setSearch(const QString &searchString);

Q_SIGNALS:
    void appletDoubleClicked(PlasmaAppletItem *appletItem);

private:

    //One single tootip to show applets info
    AppletToolTipWidget *m_toolTip;

    QStandardItemModel *m_modelItems;

    //categories models
    QStandardItemModel *m_modelFilters;

    //model that filters the item models
    KCategorizedItemsViewModels::DefaultItemFilterProxyModel *m_modelFilterItems;

    QVariant m_dataFilterAboutToApply;
    QBasicTimer m_filterApplianceTimer;
    QBasicTimer m_toolTipAppearTimer;
    QBasicTimer m_toolTipDisappearTimer;
    QBasicTimer m_toolTipAppearWhenAlreadyVisibleTimer;

    QTimeLine toolTipMoveTimeLine;
    QPoint toolTipMoveFrom;
    QPoint toolTipMoveTo;
};

#endif //APPLETSLIST_H

/*
 *   Copyright © 2008 Fredrik Höglund <fredrik@kde.org>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Library General Public
 *   License as published by the Free Software Foundation; either
 *   version 2 of the License, or (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Library General Public License for more details.
 *
 *   You should have received a copy of the GNU Library General Public License
 *   along with this library; see the file COPYING.LIB.  If not, write to
 *   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *   Boston, MA 02110-1301, USA.
 */

#ifndef FOLDERVIEW_H
#define FOLDERVIEW_H

#include <QPersistentModelIndex>
#include <QStyleOption>

#include <plasma/applet.h>
#include "ui_folderviewConfig.h"

class KDirModel;
class KFileItemDelegate;
class QItemSelectionModel;
class ProxyModel;

struct ViewItem
{
    QRect rect;
};

class FolderView : public Plasma::Applet
{
    Q_OBJECT

public:
    FolderView(QObject *parent, const QVariantList &args);
    ~FolderView();

    void init();
    void paintInterface(QPainter *painter, const QStyleOptionGraphicsItem *option, const QRect &contentsRect);
    void setPath(const QString&);

protected:
    void createConfigurationInterface(KConfigDialog *parent);

private slots:
    void rowsInserted(const QModelIndex &parent, int first, int last);
    void rowsRemoved(const QModelIndex &parent, int first, int last);
    void modelReset();
    void layoutChanged();
    void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void configAccepted();
    void customFolderToggled(bool checked);

private:
    int columnsForWidth(qreal width) const;
    void layoutItems() const;
    QModelIndex indexAt(const QPointF &point) const;
    QRectF visualRect(const QModelIndex &index) const;
    QSize iconSize() const;
    QSize gridSize() const;
    QStyleOptionViewItemV4 viewOptions() const;
    void startDrag(const QPointF &pos, QWidget *widget);
    void constraintsEvent(Plasma::Constraints constraints);
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void dragEnterEvent(QGraphicsSceneDragDropEvent *event);
    void dragMoveEvent(QGraphicsSceneDragDropEvent *event);
    void dropEvent(QGraphicsSceneDragDropEvent *event);

private:
    KFileItemDelegate *m_delegate;
    KDirModel *m_dirModel;
    ProxyModel *m_model;
    QItemSelectionModel *m_selectionModel;
    KUrl m_url;
    QString m_filterFiles;
    mutable QVector<ViewItem> m_items;
    mutable int m_columns;
    mutable bool m_layoutValid;
    QPersistentModelIndex m_hoveredIndex;
    QPersistentModelIndex m_pressedIndex;
    QRect m_rubberBand;
    QPointF m_buttonDownPos;
    QTime m_pressTime;
    Ui::folderviewConfig ui;
    bool m_updatesDisabled;
    bool m_doubleClick;
    bool m_dragInProgress;
    bool m_noBg;
};

K_EXPORT_PLASMA_APPLET(folderview, FolderView)

#endif

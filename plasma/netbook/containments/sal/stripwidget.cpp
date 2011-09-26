/*
 *   Copyright 2009 by Artur Duque de Souza <asouza@kde.org>
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

#include "stripwidget.h"
#include "models/favouritesmodel.h"
#include "models/krunnermodel.h"
#include "models/kservicemodel.h"
#include "models/commonmodel.h"

#include <QGraphicsGridLayout>
#include <QGraphicsScene>
#include <QTimer>

#include <KIcon>
#include <KIconLoader>
#include <KRun>

#include <Plasma/Animation>
#include <Plasma/Animator>
#include <Plasma/Frame>
#include <Plasma/ToolButton>
#include <Plasma/IconWidget>
#include <Plasma/AbstractRunner>
#include <Plasma/RunnerManager>
#include <Plasma/ScrollWidget>


#include "itemview.h"
#include "iconactioncollection.h"

StripWidget::StripWidget(QGraphicsWidget *parent)
    : QGraphicsWidget(parent),
      m_itemView(0),
      m_deleteTarget(0),
      m_iconActionCollection(0),
      m_offset(0),
      m_startupCompleted(false)
{
    m_favouritesModel = new FavouritesModel(this);

    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    setAcceptDrops(true);

    Plasma::Applet *applet = qobject_cast<Plasma::Applet *>(parent);
    if (applet) {
        m_iconActionCollection = new IconActionCollection(applet, this);
    }

    m_arrowsLayout = new QGraphicsLinearLayout(this);
    m_arrowsLayout->setContentsMargins(0, 0, 0, 0);
    setFocusPolicy(Qt::StrongFocus);

    m_leftArrow = new Plasma::ToolButton(this);
    m_leftArrow->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    m_leftArrow->setPreferredWidth(KIconLoader::SizeMedium);
    m_leftArrow->setImage("widgets/arrows", "left-arrow");
    connect(m_leftArrow, SIGNAL(clicked()), this, SLOT(goLeft()));
    connect(m_leftArrow, SIGNAL(pressed()), this, SLOT(scrollTimeout()));

    m_rightArrow = new Plasma::ToolButton(this);
    m_rightArrow->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    m_rightArrow->setPreferredWidth(KIconLoader::SizeMedium);
    m_rightArrow->setImage("widgets/arrows", "right-arrow");
    connect(m_rightArrow, SIGNAL(clicked()), this, SLOT(goRight()));
    connect(m_rightArrow, SIGNAL(pressed()), this, SLOT(scrollTimeout()));

    m_leftArrow->setEnabled(false);
    m_rightArrow->setEnabled(false);
    m_leftArrow->hide();
    m_rightArrow->hide();

    m_itemView = new ItemView(this);
    m_itemView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_itemView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_itemView->installEventFilter(this);
    m_itemView->setOrientation(Qt::Horizontal);
    m_itemView->setIconSize(KIconLoader::SizeLarge);
    m_itemView->setDragAndDropMode(ItemContainer::MoveDragAndDrop);
    m_itemView->setModel(m_favouritesModel);

    connect(m_itemView, SIGNAL(itemActivated(QModelIndex)), this, SLOT(launchFavourite(QModelIndex)));
    connect(m_itemView, SIGNAL(scrollBarsNeededChanged(ItemView::ScrollBarFlags)), this, SLOT(arrowsNeededChanged(ItemView::ScrollBarFlags)));
    connect(m_itemView, SIGNAL(itemAskedReorder(QModelIndex,QPointF)), this, SLOT(reorderItem(QModelIndex,QPointF)));
    connect(m_itemView, SIGNAL(dragStartRequested(QModelIndex)), this, SLOT(showDeleteTarget()));

    m_arrowsLayout->addItem(m_leftArrow);
    m_arrowsLayout->addItem(m_itemView);
    m_arrowsLayout->addItem(m_rightArrow);

    m_scrollTimer = new QTimer(this);
    m_scrollTimer->setSingleShot(false);
    connect(m_scrollTimer, SIGNAL(timeout()), this, SLOT(scrollTimeout()));
}

StripWidget::~StripWidget()
{
}

void StripWidget::showDeleteTarget()
{
    if (!m_deleteTarget) {
        m_deleteTarget = new Plasma::IconWidget();
        scene()->addItem(m_deleteTarget);
        m_deleteTarget->setIcon("user-trash");
        m_deleteTarget->resize(KIconLoader::SizeHuge, KIconLoader::SizeHuge);
        m_deleteTarget->setZValue(99);
    }
    m_deleteTarget->setPos(mapToScene(boundingRect().bottomLeft()));
    m_deleteTarget->show();
    Plasma::Animation *zoomAnim = Plasma::Animator::create(Plasma::Animator::ZoomAnimation);
    zoomAnim->setTargetWidget(m_deleteTarget);
    zoomAnim->setProperty("zoom", 1.0);
    zoomAnim->start(QAbstractAnimation::DeleteWhenStopped);
}

void StripWidget::setImmutability(Plasma::ImmutabilityType immutability)
{
    if (immutability == Plasma::Mutable) {
        m_itemView->setDragAndDropMode(ItemContainer::MoveDragAndDrop);
    } else {
        m_itemView->setDragAndDropMode(ItemContainer::NoDragAndDrop);
    }
}


void StripWidget::reorderItem(const QModelIndex &index, const QPointF &pos)
{
    if (m_deleteTarget && m_deleteTarget->geometry().contains(m_itemView->widget()->mapToItem(this, pos))) {
        m_favouritesModel->removeRow(index.row());
    } else {
        QList<QStandardItem *>items = m_favouritesModel->takeRow(index.row());
        int row = m_itemView->rowForPosition(pos);

        m_favouritesModel->insertRow(row, items);
    }

    Plasma::Animation *zoomAnim = Plasma::Animator::create(Plasma::Animator::ZoomAnimation);
    zoomAnim->setTargetWidget(m_deleteTarget);
    zoomAnim->start(QAbstractAnimation::DeleteWhenStopped);
}

void StripWidget::save(KConfigGroup &cg)
{
    m_favouritesModel->save(cg);
}

void StripWidget::restore(KConfigGroup &cg)
{
    m_favouritesModel->restore(cg);
}

void StripWidget::launchFavourite(const QModelIndex &index)
{
    KUrl url(index.data(CommonModel::Url).value<QString>());
    if (!KServiceItemHandler::openUrl(url)) {
        KRunnerItemHandler::openUrl(url);
    }
}

void StripWidget::add(const QUrl &url)
{
    m_favouritesModel->add(url);
    emit saveNeeded();
}

void StripWidget::goRight()
{
    QRectF rect(m_itemView->boundingRect());
    rect.moveLeft(rect.right() - m_itemView->widget()->pos().x());
    rect.setWidth(rect.width()/4);

    m_itemView->ensureRectVisible(rect);
}

void StripWidget::goLeft()
{
    QRectF rect(m_itemView->boundingRect());
    rect.setWidth(rect.width()/4);
    rect.moveRight(- m_itemView->widget()->pos().x());

    m_itemView->ensureRectVisible(rect);
}

void StripWidget::scrollTimeout()
{
    if (!m_scrollTimer->isActive()) {
        m_scrollTimer->start(250);
    } else if (m_leftArrow->isDown()) {
        goLeft();
    } else if (m_rightArrow->isDown()) {
        goRight();
    } else {
        m_scrollTimer->stop();
    }
}

void StripWidget::setIconSize(int iconSize)
{
    m_itemView->setIconSize(iconSize);
}

int StripWidget::iconSize() const
{
    return m_itemView->iconSize();
}

void StripWidget::arrowsNeededChanged(ItemView::ScrollBarFlags flags)
{
    bool leftNeeded = false;
    bool rightNeeded = false;

    if (flags & ItemView::HorizontalScrollBar) {
        leftNeeded = m_itemView->scrollPosition().x() > 0;
        rightNeeded = m_itemView->contentsSize().width() - m_itemView->scrollPosition().x() > m_itemView->size().width();
    }

    m_leftArrow->setEnabled(leftNeeded);
    m_rightArrow->setEnabled(rightNeeded);
    m_leftArrow->setVisible(leftNeeded|rightNeeded);
    m_rightArrow->setVisible(leftNeeded|rightNeeded);
    m_arrowsLayout->invalidate();
}



void StripWidget::focusInEvent(QFocusEvent *event)
{
    Q_UNUSED(event)

    m_itemView->setFocus();
}

void StripWidget::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
     event->setAccepted(event->mimeData()->hasFormat("application/x-plasma-salquerymatch") || event->mimeData()->hasFormat("text/uri-list"));
}

void StripWidget::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
    m_itemView->setScrollPositionFromDragPosition(event->pos());
    m_itemView->showSpacer(m_itemView->widget()->mapFromScene(event->scenePos()));
}

void StripWidget::dragLeaveEvent(QGraphicsSceneDragDropEvent *event)
{
    Q_UNUSED(event)

    m_itemView->showSpacer(QPointF());
}

void StripWidget::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    m_itemView->showSpacer(QPointF());
    if (event->mimeData()->hasFormat("application/x-plasma-salquerymatch")) {
         QByteArray itemData = event->mimeData()->data("application/x-plasma-salquerymatch");
         QDataStream dataStream(&itemData, QIODevice::ReadOnly);

         QUrl url;

         dataStream >>url;

         int row = m_itemView->rowForPosition(m_itemView->widget()->mapFromScene(event->scenePos()));
         QModelIndex index = m_favouritesModel->index(row, 0, QModelIndex());
         //TODO: proper index
         m_favouritesModel->add(url.toString(), index);
         emit saveNeeded();

     } else if (event->mimeData()->urls().size() > 0) {
         int row = m_itemView->rowForPosition(m_itemView->widget()->mapFromScene(event->scenePos()));
         QModelIndex index = m_favouritesModel->index(row, 0, QModelIndex());
         m_favouritesModel->add(event->mimeData()->urls().first().path(), index);
         emit saveNeeded();
     } else {
         event->ignore();
     }
}

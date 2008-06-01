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

#include "folderview.h"
#include "folderview.moc"

#include <QApplication>
#include <QDebug>
#include <QDrag>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneMouseEvent>
#include <QItemSelectionModel>
#include <QPainter>
#include <QStyleOptionGraphicsItem>

#include <KConfigDialog>
#include <KDirLister>
#include <KDirModel>
#include <KFileItemDelegate>
#include <KGlobalSettings>

#include "proxymodel.h"
#include "plasma/theme.h"


FolderView::FolderView(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args)
{
    setAspectRatioMode(Plasma::IgnoreAspectRatio);
    setHasConfigurationInterface(true);
    setAcceptHoverEvents(true);
    setAcceptDrops(true);
    resize(600, 400);

    m_dirModel = new KDirModel(this);

    m_model = new ProxyModel(this);
    m_model->setSourceModel(m_dirModel);
    m_model->setSortLocaleAware(true);
    m_model->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_model->sort(0, Qt::AscendingOrder);

    connect(m_model, SIGNAL(rowsInserted(QModelIndex,int,int)), SLOT(rowsInserted(QModelIndex,int,int)));
    connect(m_model, SIGNAL(rowsRemoved(QModelIndex,int,int)), SLOT(rowsRemoved(QModelIndex,int,int)));
    connect(m_model, SIGNAL(modelReset()), SLOT(modelReset()));
    connect(m_model, SIGNAL(layoutChanged()), SLOT(layoutChanged()));
    connect(m_model, SIGNAL(dataChanged(QModelIndex,QModelIndex)), SLOT(layoutChanged(QModelIndex,QModelIndex)));

    m_delegate = new KFileItemDelegate(this);
    m_selectionModel = new QItemSelectionModel(m_model, this);

    m_layoutValid = false;
    m_doubleClick = false;
    m_dragInProgress = false;
}

void FolderView::init()
{
    KConfigGroup cg = config();
    m_url = cg.readEntry("url", KUrl(QDir::homePath()));
    m_filterFiles = cg.readEntry("filterFiles", "*");

    KDirLister *lister = new KDirLister(this);
    lister->openUrl(m_url);

    m_model->setFilterFixedString(m_filterFiles);

    m_dirModel->setDirLister(lister);
}

FolderView::~FolderView()
{
}

void FolderView::createConfigurationInterface(KConfigDialog *parent)
{
    QWidget *widget = new QWidget;
    ui.setupUi(widget);
    if (m_url == KUrl("desktop:/")) {
        ui.showDesktopFolder->setChecked(true);
        ui.selectLabel->setEnabled(false);
        ui.lineEdit->setEnabled(false);
    } else {
        ui.showCustomFolder->setChecked(true);
        ui.lineEdit->setUrl(m_url);
    }
    
    ui.lineEdit->setMode(KFile::Directory); 
    ui.filterFiles->setText(m_filterFiles);

    parent->addPage(widget, parent->windowTitle(), icon());
    parent->setButtons(KDialog::Ok | KDialog::Cancel | KDialog::Apply);
    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
    connect(ui.showCustomFolder, SIGNAL(toggled(bool)), this, SLOT(customFolderToggled(bool)));
}

void FolderView::configAccepted()
{
    KUrl url;

    if (ui.showDesktopFolder->isChecked())
        url = KUrl("desktop:/");
    else
        url = ui.lineEdit->url();

    if (m_url != url || m_filterFiles != ui.filterFiles->text()) {
        m_dirModel->dirLister()->openUrl(url);
        m_model->setFilterFixedString(ui.filterFiles->text());
        m_url = url;
        m_filterFiles = ui.filterFiles->text();
 
        KConfigGroup cg = config();
        cg.writeEntry("url", m_url);
        cg.writeEntry("filterFiles", m_filterFiles);

        emit configNeedsSaving();
    }
}

void FolderView::customFolderToggled(bool checked)
{
    ui.selectLabel->setEnabled(checked);
    ui.lineEdit->setEnabled(checked);
}

void FolderView::rowsInserted(const QModelIndex &parent, int first, int last)
{
    Q_UNUSED(parent)
    Q_UNUSED(first)
    Q_UNUSED(last)

    m_layoutValid = false;
    update();
}

void FolderView::rowsRemoved(const QModelIndex &parent, int first, int last)
{
    Q_UNUSED(parent)
    Q_UNUSED(first)
    Q_UNUSED(last)

    m_layoutValid = false;
    update();
}

void FolderView::modelReset()
{
    m_layoutValid = false;
    update();
}

void FolderView::layoutChanged()
{
    m_layoutValid = false;
    update();
}

void FolderView::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    Q_UNUSED(topLeft)
    Q_UNUSED(bottomRight)

    m_layoutValid = false;
    update();
}

int FolderView::columnsForWidth(qreal width) const
{
    int spacing = 10;
    int margin = 10;

    qreal available = width - 2 * margin + spacing;
    return qFloor(available / (gridSize().width() + spacing));
}

void FolderView::layoutItems() const
{
    QStyleOptionViewItemV4 option = viewOptions();
    m_items.resize(m_model->rowCount());

    const QRectF rect = contentsRect();
    int spacing = 10;
    int margin = 10;
    int x = rect.x() + margin; 
    int y = rect.y() + margin;

    QSize grid = gridSize();
    int rowHeight = 0;
    int maxColumns = columnsForWidth(rect.width());
    int column = 0;

    m_delegate->setMaximumSize(grid);

    for (int i = 0; i < m_items.size(); i++) {
        const QModelIndex index = m_model->index(i, 0);
        QSize size = m_delegate->sizeHint(option, index).boundedTo(grid);

        QPoint pos(x + (grid.width() - size.width()) / 2, y);
        m_items[i].rect = QRect(pos, size);

        rowHeight = qMax(rowHeight, size.height());
        x += grid.width() + spacing;

        if (++column >= maxColumns) {
            y += rowHeight + spacing;
            rowHeight = 0;
            column = 0;
            x = rect.x() + margin;
        }
    }

    m_columns = maxColumns;
    m_layoutValid = true;
}

void FolderView::paintInterface(QPainter *painter, const QStyleOptionGraphicsItem *option, const QRect &contentRect)
{
    QStyleOptionViewItemV4 opt = viewOptions();
    opt.palette.setColor(QPalette::All, QPalette::Text, Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor));
    m_delegate->setShadowColor(Plasma::Theme::defaultTheme()->color(Plasma::Theme::BackgroundColor));

    if (!m_layoutValid)
        layoutItems();

    const QRect clipRect = contentRect & option->exposedRect.toAlignedRect();
    if (clipRect.isEmpty())
        return;

    painter->save();
    painter->setClipRect(clipRect, Qt::IntersectClip);

    for (int i = 0; i < m_items.size(); i++) {
        opt.rect = m_items[i].rect;

        if (!opt.rect.intersects(clipRect))
            continue;

        const QModelIndex index = m_model->index(i, 0);
        opt.state &= ~(QStyle::State_HasFocus | QStyle::State_MouseOver | QStyle::State_Selected);

        if (index == m_hoveredIndex)
            opt.state |= QStyle::State_MouseOver;

        if (m_selectionModel->isSelected(index))
            opt.state |= QStyle::State_Selected;

        if (hasFocus() && index == m_selectionModel->currentIndex())
            opt.state |= QStyle::State_HasFocus;

        m_delegate->paint(painter, opt, index);
    }

    if (m_rubberBand.isValid()) {
        QStyleOptionRubberBand opt;
        initStyleOption(&opt);
        opt.rect   = m_rubberBand;
        opt.shape  = QRubberBand::Rectangle;
        opt.opaque = false;

        style()->drawControl(QStyle::CE_RubberBand, &opt, painter);
    }

    painter->restore();
}

QModelIndex FolderView::indexAt(const QPointF &point) const
{
    if (!m_layoutValid)
        layoutItems();

    if (!contentsRect().contains(point))
        return QModelIndex();

    for (int i = 0; i < m_items.size(); i++) {
        if (m_items[i].rect.contains(point.toPoint()))
            return m_model->index(i, 0);
    }

    return QModelIndex();
}

QRectF FolderView::visualRect(const QModelIndex &index) const
{
    if (!m_layoutValid)
        layoutItems();

    if (!index.isValid() || index.row() < 0 || index.row() > m_items.size())
        return QRectF();

    return m_items[index.row()].rect;
}

void FolderView::constraintsEvent(Plasma::Constraints constraints)
{
    // We should probably only do this when acting as the desktop containment
    //if (constraints & Plasma::FormFactorConstraint)
    //   setBackgroundHints(Applet::NoBackground);

    setBackgroundHints(Applet::TranslucentBackground);

    if ((constraints & Plasma::SizeConstraint) &&
        columnsForWidth(contentsRect().width()) != m_columns)
    {
        m_layoutValid = false;
    }
}

void FolderView::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    const QModelIndex index = indexAt(event->pos());
    if (index.isValid()) {
        m_hoveredIndex = index;
        update(visualRect(index));
    }
}

void FolderView::hoverLeaveEvent(QGraphicsSceneHoverEvent *)
{
    if (m_hoveredIndex.isValid()) {
        update(visualRect(m_hoveredIndex));
        m_hoveredIndex = QModelIndex();
    }
}

void FolderView::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    const QModelIndex index = indexAt(event->pos());
    if (index != m_hoveredIndex) {
        update(visualRect(index));
        update(visualRect(m_hoveredIndex));
        m_hoveredIndex = index;
    }
}

void FolderView::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        const QModelIndex index = indexAt(event->pos());

        // If an icon was pressed
        if (index.isValid())
        {
            if (event->modifiers() & Qt::ControlModifier) {
                m_selectionModel->select(index, QItemSelectionModel::Toggle);
                update(visualRect(index));
            } else if (!m_selectionModel->isSelected(index)) {
                m_selectionModel->select(index, QItemSelectionModel::ClearAndSelect);
                m_selectionModel->setCurrentIndex(index, QItemSelectionModel::NoUpdate);
                update();
            }
            m_pressedIndex = index;
            m_buttonDownPos = event->pos();
            event->accept();
            return;
        }

        // If empty space was pressed
        if (contentsRect().contains(event->pos()))
        {
            m_pressedIndex = QModelIndex();
            if (m_selectionModel->hasSelection()) {
                m_selectionModel->clearSelection();
                update();
            }
            event->accept();
            return;
        }
    }

    Plasma::Applet::mousePressEvent(event);
}

void FolderView::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        if (m_rubberBand.isValid()) {
            update(m_rubberBand);
            m_rubberBand = QRect();
            return;
        }

        const QModelIndex index = indexAt(event->pos());
        if (index.isValid() && index == m_pressedIndex) {
            if (!m_doubleClick && KGlobalSettings::singleClick()) {
                const KFileItem item = m_model->itemForIndex(index);
                item.run();
            }
            // We don't clear and update the selection and current index in
            // mousePressEvent() if the item is already selected when it's pressed,
            // so we need to do that here.
            if (m_selectionModel->currentIndex() != index ||
                m_selectionModel->selectedIndexes().count() > 1)
            {
                m_selectionModel->select(index, QItemSelectionModel::ClearAndSelect);
                m_selectionModel->setCurrentIndex(index, QItemSelectionModel::NoUpdate);
                update();
            }
            event->accept();
            m_doubleClick = false;
            return;
        }
    }

    m_doubleClick = false;
    m_pressedIndex = QModelIndex();
    Plasma::Applet::mouseReleaseEvent(event);
}

void FolderView::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() != Qt::LeftButton)
    {
        Plasma::Applet::mouseDoubleClickEvent(event);
        return;
    }

    // So we don't activate the item again on the release event
    m_doubleClick = true;

    // We don't want to invoke default implementation in this case, since it
    // calls mousePressEvent().
    if (KGlobalSettings::singleClick())
        return;

    const QModelIndex index = indexAt(event->pos());
    if (!index.isValid())
        return;

    // Activate the item
    const KFileItem item = m_model->itemForIndex(index);
    item.run();
}

void FolderView::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    // If an item is pressed
    if (m_pressedIndex.isValid())
    {
        const QPointF point = event->pos() - event->buttonDownPos(Qt::LeftButton);

        if (point.toPoint().manhattanLength() >= QApplication::startDragDistance())
        {
            startDrag(event->buttonDownPos(Qt::LeftButton), event->widget());
        }
        event->accept();
        return;
    }

    const QRectF rubberBand = QRectF(event->buttonDownPos(Qt::LeftButton), event->pos()).normalized();
    const QRect r = QRectF(rubberBand & contentsRect()).toAlignedRect();

    if (r != m_rubberBand)
    {
        QRectF dirtyRect = m_rubberBand | r;
        m_rubberBand = r;

        dirtyRect |= visualRect(m_hoveredIndex);
        m_hoveredIndex = QModelIndex();

        foreach (const QModelIndex &index, m_selectionModel->selectedIndexes())
            dirtyRect |= visualRect(index);

        // Select the indexes inside the rubber band
        QItemSelection selection;
        for (int i = 0; i < m_items.size(); i++)
        {
            if (!m_items[i].rect.intersects(m_rubberBand))
                continue;

            int start = i;
            int end = i;

            while (i < m_items.size() && m_items[i].rect.intersects(m_rubberBand)) {
                dirtyRect |= m_items[i].rect;
                if (m_items[i].rect.contains(event->pos().toPoint()))
                    m_hoveredIndex = m_model->index(i, 0);
                end = i++;
            }

            selection.select(m_model->index(start, 0), m_model->index(end, 0));
        }
        m_selectionModel->select(selection, QItemSelectionModel::ClearAndSelect);
        update(dirtyRect);
    }

    event->accept();
}

void FolderView::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
    event->setAccepted(event->mimeData()->hasUrls());
}

void FolderView::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
    event->accept();
}

void FolderView::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    // Check if the drop event originated from this applet.
    // Normally we'd do this by checking if the source widget matches the target widget
    // in the drag and drop operation, but since two QGraphicsItems can be part of the
    // same widget, we can't use that method here.
    if (!m_dragInProgress) {
        m_model->dropMimeData(event->mimeData(), event->dropAction(), -1, -1, QModelIndex());
        return;
    }

    // ### We should check if the items were dropped on a child item that
    //     accepts drops.

    // If we get to this point, the drag was started from within the applet,
    // so instead of moving/copying/linking the dropped URL's to the folder,
    // we'll move the items in the view.
    const QPoint delta = (event->pos() - m_buttonDownPos).toPoint();
    foreach (const QUrl &url, event->mimeData()->urls()) {
        const QModelIndex index = m_model->indexForUrl(url);
        if (index.isValid()) {
            m_items[index.row()].rect.translate(delta);
        }
    }

    update();
}

// pos is the position where the mouse was clicked in the applet.
// widget is the widget that sent the mouse event that triggered the drag.
void FolderView::startDrag(const QPointF &pos, QWidget *widget)
{
    QModelIndexList indexes = m_selectionModel->selectedIndexes();
    QRectF boundingRect;
    foreach (const QModelIndex &index, indexes) {
        boundingRect |= visualRect(index);
    }

    QPixmap pixmap(boundingRect.toAlignedRect().size());
    pixmap.fill(Qt::transparent);

    QStyleOptionViewItemV4 option = viewOptions(); 
    option.state |= QStyle::State_Selected;

    QPainter p(&pixmap);
    foreach (const QModelIndex &index, indexes)
    {
        option.rect = visualRect(index).translated(-boundingRect.topLeft()).toAlignedRect();
        if (index == m_hoveredIndex)
            option.state |= QStyle::State_MouseOver;
        else
            option.state &= ~QStyle::State_MouseOver;
        m_delegate->paint(&p, option, index);
    }
    p.end();

    m_dragInProgress = true;

    QDrag *drag = new QDrag(widget);
    drag->setMimeData(m_model->mimeData(indexes));
    drag->setPixmap(pixmap);
    drag->setHotSpot((pos - boundingRect.topLeft()).toPoint());
    drag->exec(m_model->supportedDragActions());

    m_dragInProgress = false;
}

QSize FolderView::iconSize() const
{
    KIconTheme *theme = KIconLoader::global()->theme();
    int size = theme ? theme->defaultSize(KIconLoader::Desktop) : 48;
    return QSize(size, size);
}

QSize FolderView::gridSize() const
{
    QSize size = iconSize();
    size.rwidth()  *= 2;
    size.rheight() *= 2;
    return size;
}

QStyleOptionViewItemV4 FolderView::viewOptions() const
{
    QStyleOptionViewItemV4 option;
    initStyleOption(&option);

    option.font                = font();
    option.decorationAlignment = Qt::AlignTop | Qt::AlignHCenter;
    option.decorationPosition  = QStyleOptionViewItem::Top;
    option.decorationSize      = iconSize();
    option.displayAlignment    = Qt::AlignHCenter;
    option.textElideMode       = Qt::ElideRight;
    option.features            = QStyleOptionViewItemV2::WrapText;
    option.locale              = QLocale::system();
    option.widget              = 0;
    option.viewItemPosition    = QStyleOptionViewItemV4::OnlyOne;

    return option;
}


/*
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

#include "appletsfiltering.h"

#include <kglobalsettings.h>
#include <klineedit.h>

#include <plasma/theme.h>

//FilteringTreeView

FilteringTreeView::FilteringTreeView(QGraphicsItem * parent, Qt::WindowFlags wFlags)
    : QGraphicsWidget(parent, wFlags)
{
    init();
    connect(m_treeView->nativeWidget(), SIGNAL(clicked(const QModelIndex &)), this, SLOT(filterChanged(const QModelIndex &)));
}

FilteringTreeView::~FilteringTreeView()
{
}

void FilteringTreeView::init()
{
    QFont listFont;
    QPalette plasmaPalette;
    QColor textColor;
    QColor color;
    QGraphicsLinearLayout *linearLayout;

    //init treeview
    m_treeView = new Plasma::TreeView();
    m_treeView->nativeWidget()->setAttribute(Qt::WA_NoSystemBackground);
    m_treeView->nativeWidget()->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_treeView->nativeWidget()->setRootIsDecorated(false);
    m_treeView->nativeWidget()->setAttribute(Qt::WA_TranslucentBackground);

    //set font and palette
    listFont = m_treeView->nativeWidget()->font();
    listFont.setPointSize(KGlobalSettings::smallestReadableFont().pointSize());
    m_treeView->nativeWidget()->setFont(listFont);
    plasmaPalette = QPalette();
    textColor = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
    color = Plasma::Theme::defaultTheme()->color(Plasma::Theme::BackgroundColor);
    //transparent base color
    plasmaPalette.setColor(QPalette::Base,
                           QColor(color.red(), color.green(), color.blue(), 0));
    plasmaPalette.setColor(QPalette::Text, textColor);
    m_treeView->setPalette(plasmaPalette);
    m_treeView->nativeWidget()->setAutoFillBackground(true);

    //layout
    linearLayout = new QGraphicsLinearLayout();
    linearLayout->addItem(m_treeView);
    setLayout(linearLayout);
    m_treeView->nativeWidget()->header()->setVisible(false);
}

void FilteringTreeView::setModel(QStandardItemModel *model)
{
    m_model = model;
    m_treeView->setModel(m_model);
}


void FilteringTreeView::filterChanged(const QModelIndex & index)
{
    emit(filterChanged(index.row()));
}

//FilteringTabs

FilteringTabs::FilteringTabs(QGraphicsWidget *parent)
    : Plasma::TabBar(parent),
      m_model(0)
{
    setAttribute(Qt::WA_NoSystemBackground);
    nativeWidget()->setUsesScrollButtons(true);
    connect(this, SIGNAL(currentChanged(int)), this, SIGNAL(filterChanged(int)));
}

FilteringTabs::~FilteringTabs()
{

}

void FilteringTabs::populateList()
{
    while (count() > 0) {
        removeTab(0);
    }

    if (!m_model) {
        return;
    }

    for (int i = 0; i < m_model->rowCount(); i++){
        QStandardItem *item = getItemByProxyIndex(m_model->index(i, 0));
        if (item) {
            addTab(item->icon(), item->text());
            nativeWidget()->setTabEnabled(i, item->isEnabled());
        }
    }
}

QStandardItem *FilteringTabs::getItemByProxyIndex(const QModelIndex &index) const
{
    return m_model->itemFromIndex(index);
}

void FilteringTabs::setModel(QStandardItemModel *model)
{
    m_model = model;
    populateList();
}

//FilteringWidget

FilteringWidget::FilteringWidget(QGraphicsItem * parent, Qt::WindowFlags wFlags)
    : QGraphicsWidget(parent, wFlags),
      m_model(0),
      m_categoriesTreeView(0),
      m_categoriesTabs(0)
{
    m_orientation = Qt::Horizontal;
    init();
}

FilteringWidget::FilteringWidget(Qt::Orientation orientation, QGraphicsItem * parent,
                                 Qt::WindowFlags wFlags)
    : QGraphicsWidget(parent, wFlags),
      m_model(0),
      m_categoriesTreeView(0),
      m_categoriesTabs(0)
{
    m_orientation = orientation;
    init();
}

FilteringWidget::~FilteringWidget()
{
    delete m_categoriesTabs;
    delete m_categoriesTreeView;
}

void FilteringWidget::init()
{
    //init text search
    m_textSearch = new Plasma::LineEdit();
    m_textSearch->nativeWidget()->setClickMessage(i18n("Enter Search Term"));
    m_textSearch->setFocus();
    m_textSearch->setAttribute(Qt::WA_NoSystemBackground);
    m_textSearch->setClearButtonShown(true);

    //layout
    m_linearLayout = new QGraphicsLinearLayout();
    m_linearLayout->addItem(m_textSearch);
    setLayout(m_linearLayout);
}

void FilteringWidget::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    Q_UNUSED(event)

    QSizeF contentsSize = m_linearLayout->contentsRect().size();
//    m_linearLayout->invalidate();
//    m_linearLayout->activate();

    if(m_orientation == Qt::Horizontal) {
        //don't let it occupy the whole layout width
        m_textSearch->setMaximumWidth(contentsSize.width()/6);
        m_textSearch->setMinimumWidth(contentsSize.width()/6);
    } else {
        //let it occupy the whole width
        m_textSearch->setMaximumWidth(-1);
        m_textSearch->setMinimumWidth(-1);
    }
}

Plasma::LineEdit *FilteringWidget::textSearch()
{
    return m_textSearch;
}

void FilteringWidget::setModel(QStandardItemModel *model)
{
    m_model = model;
    if (m_categoriesTreeView) {
        m_categoriesTreeView->setModel(model);
    }

    if (m_categoriesTabs) {
        m_categoriesTabs->setModel(model);
    }
}

void FilteringWidget::setListOrientation(Qt::Orientation orientation)
{
    if (m_orientation == orientation && (m_categoriesTreeView || m_categoriesTabs)) {
        return;
    }

    m_orientation = orientation;
    m_linearLayout->setOrientation(orientation);

    if (orientation == Qt::Horizontal) {
        delete m_categoriesTreeView;
        m_categoriesTreeView = 0;

        if (!m_categoriesTabs) {
            m_categoriesTabs = new FilteringTabs();
            connect(m_categoriesTabs, SIGNAL(filterChanged(int)), this, SIGNAL(filterChanged(int)));
            m_categoriesTabs->setModel(m_model);
        }

        m_textSearch->setPreferredWidth(200);
        m_textSearch->setPreferredHeight(-1);
        m_linearLayout->addItem(m_categoriesTabs);
        m_categoriesTabs->setVisible(true);
    } else {
        delete m_categoriesTabs;
        m_categoriesTabs = 0;

        if (!m_categoriesTreeView) {
            m_categoriesTreeView = new FilteringTreeView();
            connect(m_categoriesTreeView, SIGNAL(filterChanged(int)), this, SIGNAL(filterChanged(int)));
            m_categoriesTreeView->setModel(m_model);
        }

        m_textSearch->setPreferredHeight(30);
        m_textSearch->setPreferredWidth(-1);
        m_linearLayout->addItem(m_categoriesTreeView);
        m_categoriesTreeView->setVisible(true);
    }

    m_linearLayout->invalidate();
}


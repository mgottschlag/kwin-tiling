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
#include <kmenu.h>
#include <kservicetypetrader.h>
#include <kpushbutton.h>

#include <plasma/theme.h>
#include <plasma/corona.h>
#include <plasma/packagestructure.h>
#include <plasma/framesvg.h>
#include <plasma/widgets/label.h>
#include <plasma/widgets/lineedit.h>
#include <plasma/widgets/treeview.h>
#include <plasma/widgets/pushbutton.h>

#include "widgetexplorer.h"
#include "openwidgetassistant_p.h"
//FilteringTreeView

FilteringTreeView::FilteringTreeView(QGraphicsWidget *parent, Qt::WindowFlags wFlags)
    : Plasma::TreeView(parent)
{
    setWindowFlags(wFlags);
    init();
    connect(nativeWidget(), SIGNAL(clicked(const QModelIndex &)), this, SLOT(filterChanged(const QModelIndex &)));
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

    //init treeview
    nativeWidget()->setAttribute(Qt::WA_NoSystemBackground);
    nativeWidget()->setEditTriggers(QAbstractItemView::NoEditTriggers);
    nativeWidget()->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    nativeWidget()->setRootIsDecorated(false);
    nativeWidget()->setAttribute(Qt::WA_TranslucentBackground);

    //set font and palette
    listFont = nativeWidget()->font();
    listFont.setPointSize(KGlobalSettings::smallestReadableFont().pointSize());
    nativeWidget()->setFont(listFont);
    plasmaPalette = QPalette();
    textColor = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
    color = Plasma::Theme::defaultTheme()->color(Plasma::Theme::BackgroundColor);
    //transparent base color
    plasmaPalette.setColor(QPalette::Base,
                           QColor(color.red(), color.green(), color.blue(), 0));
    plasmaPalette.setColor(QPalette::Text, textColor);
    setPalette(plasmaPalette);
    nativeWidget()->setAutoFillBackground(true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    nativeWidget()->header()->setVisible(false);
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
      m_categoriesTabs(0),
      m_widgetExplorer(0)
{
    init();
    setListOrientation(Qt::Horizontal);
}

FilteringWidget::FilteringWidget(Qt::Orientation orientation,
                                 Plasma::WidgetExplorer *widgetExplorer,
                                 QGraphicsItem * parent,
                                 Qt::WindowFlags wFlags)
    : QGraphicsWidget(parent, wFlags),
      m_model(0),
      m_categoriesTreeView(0),
      m_categoriesTabs(0),
      m_widgetExplorer(widgetExplorer)
{
    init();
    setListOrientation(orientation);
}

FilteringWidget::~FilteringWidget()
{
    delete m_newWidgetsMenu;
}

void FilteringWidget::init()
{
    setFocusPolicy(Qt::StrongFocus);

    //init text search
    m_textSearch = new Plasma::LineEdit();

    m_textSearch->nativeWidget()->setClickMessage(i18n("Enter Search Term"));
    m_textSearch->setAttribute(Qt::WA_NoSystemBackground);
    m_textSearch->setClearButtonShown(true);

    m_newWidgetsButton = new Plasma::PushButton(this);
    m_newWidgetsButton->setText(i18n("Get New Widgets"));
    m_newWidgetsMenu = new KMenu(i18n("Get New Widgets"));
    connect(m_newWidgetsMenu, SIGNAL(aboutToShow()), this, SLOT(populateWidgetsMenu()));
    m_newWidgetsButton->nativeWidget()->setMenu(m_newWidgetsMenu);
    m_newWidgetsButton->setMinimumWidth(m_newWidgetsButton->effectiveSizeHint(Qt::PreferredSize).width());

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
        int rowHeight = m_categoriesTreeView->nativeWidget()->sizeHintForRow(0);
        if (rowHeight > 0) {
            m_categoriesTreeView->setMaximumHeight(rowHeight * 6);
        }
    }

    if (m_categoriesTabs) {
        m_categoriesTabs->setModel(model);
    }
}

void FilteringWidget::setListOrientation(Qt::Orientation orientation)
{
    if (m_orientation == orientation) {
        if ((orientation == Qt::Vertical && m_categoriesTreeView) || 
            (orientation == Qt::Horizontal && m_categoriesTabs)) {
            return;
        }
    }

    m_orientation = orientation;
    m_linearLayout->setOrientation(orientation);

    if (orientation == Qt::Horizontal) {
        if (m_categoriesTreeView) {
            m_categoriesTreeView->deleteLater();
            m_categoriesTreeView = 0;
        }

        if (!m_categoriesTabs) {
            m_categoriesTabs = new FilteringTabs(this);
            connect(m_categoriesTabs, SIGNAL(filterChanged(int)), this, SIGNAL(filterChanged(int)));
            m_categoriesTabs->setModel(m_model);
        }

        m_textSearch->setPreferredWidth(200);
        m_textSearch->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        m_textSearch->setPreferredHeight(-1);
        m_linearLayout->addItem(m_newWidgetsButton);
        m_linearLayout->addItem(m_categoriesTabs);
        m_categoriesTabs->setVisible(true);
    } else {
        if (m_categoriesTabs) {
            m_categoriesTabs->deleteLater();
            m_categoriesTabs = 0;
        }

        if (!m_categoriesTreeView) {
            m_categoriesTreeView = new FilteringTreeView(this);
            connect(m_categoriesTreeView, SIGNAL(filterChanged(int)), this, SIGNAL(filterChanged(int)));
            m_categoriesTreeView->setModel(m_model);
            int rowHeight = m_categoriesTreeView->nativeWidget()->sizeHintForRow(0);
            if (rowHeight > 0) {
                m_categoriesTreeView->setMaximumHeight(rowHeight * 6);
            }
        }

        m_textSearch->setPreferredHeight(30);
        m_textSearch->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        m_textSearch->setPreferredWidth(-1);
        m_linearLayout->addItem(m_newWidgetsButton);
        m_categoriesTreeView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
        m_linearLayout->addItem(m_categoriesTreeView);
        m_categoriesTreeView->setVisible(true);
    }

    m_linearLayout->invalidate();
}

void FilteringWidget::setMenuPos()
{
    QPoint position(0, 0);
    if (m_widgetExplorer) {
        Plasma::Corona *corona = m_widgetExplorer->corona();

        if (corona) {
            position = corona->popupPosition(m_newWidgetsButton,
                                             m_newWidgetsMenu->geometry().size());
        }
    }
    m_newWidgetsMenu->move(position);
}

void FilteringWidget::setFocus()
{
    QGraphicsWidget::setFocus();
    m_textSearch->setFocus();
    m_textSearch->nativeWidget()->setFocus();
 }

void FilteringWidget::populateWidgetsMenu()
{
    QTimer::singleShot(0, this, SLOT(setMenuPos()));
    if (!m_newWidgetsMenu->actions().isEmpty()) {
        // already populated.
        return;
    }
    QSignalMapper *mapper = new QSignalMapper(this);
    QObject::connect(mapper, SIGNAL(mapped(QString)), this, SLOT(downloadWidgets(QString)));

    QAction *action = new QAction(KIcon("applications-internet"),
                                  i18n("Download New Plasma Widgets"), this);
    QObject::connect(action, SIGNAL(triggered(bool)), mapper, SLOT(map()));
    mapper->setMapping(action, QString());
    m_newWidgetsMenu->addAction(action);

    KService::List offers = KServiceTypeTrader::self()->query("Plasma/PackageStructure");
    foreach (const KService::Ptr &service, offers) {
        //kDebug() << service->property("X-Plasma-ProvidesWidgetBrowser");
        if (service->property("X-Plasma-ProvidesWidgetBrowser").toBool()) {
            QAction *action = new QAction(KIcon("applications-internet"),
                                          i18nc("%1 is a type of widgets, as defined by "
                                                "e.g. some plasma-packagestructure-*.desktop files",
                                                "Download New %1", service->name()), this);
            QObject::connect(action, SIGNAL(triggered(bool)), mapper, SLOT(map()));
            mapper->setMapping(action, service->property("X-KDE-PluginInfo-Name").toString());
            m_newWidgetsMenu->addAction(action);
        }
    }

    m_newWidgetsMenu->addSeparator();

    action = new QAction(KIcon("package-x-generic"),
                         i18n("Install Widget From Local File..."), this);
    QObject::connect(action, SIGNAL(triggered(bool)), this, SLOT(openWidgetFile()));
    m_newWidgetsMenu->addAction(action);
}

void FilteringWidget::downloadWidgets(const QString &type)
{
    Plasma::PackageStructure *installer = 0;

    if (!type.isEmpty()) {
        QString constraint = QString("'%1' == [X-KDE-PluginInfo-Name]").arg(type);
        KService::List offers = KServiceTypeTrader::self()->query("Plasma/PackageStructure",
                                                                  constraint);
        if (offers.isEmpty()) {
            kDebug() << "could not find requested PackageStructure plugin" << type;
        } else {
            KService::Ptr service = offers.first();
            QString error;
            installer = service->createInstance<Plasma::PackageStructure>(topLevelWidget(),
                                                                          QVariantList(), &error);
            if (installer) {
                connect(installer, SIGNAL(newWidgetBrowserFinished()),
                        installer, SLOT(deleteLater()));
            } else {
                kDebug() << "found, but could not load requested PackageStructure plugin" << type
                         << "; reported error was" << error;
            }
        }
    }

    if (installer) {
        installer->createNewWidgetBrowser();
    } else {
        // we don't need to delete the default Applet::packageStructure as that
        // belongs to the applet
       Plasma::Applet::packageStructure()->createNewWidgetBrowser();
    }
}

void FilteringWidget::openWidgetFile()
{
    // TODO: if we already have one of these showing and the user clicks to
    // add it again, show the same window?
    Plasma::OpenWidgetAssistant *assistant = new Plasma::OpenWidgetAssistant(0);
    assistant->setAttribute(Qt::WA_DeleteOnClose, true);
    assistant->show();
}


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

#include <QMenu>

#include <KGlobalSettings>
#include <KLineEdit>
#include <KMenu>
#include <KPushButton>
#include <KServiceTypeTrader>
#include <KWindowSystem>

#include <Plasma/Theme>
#include <Plasma/Corona>
#include <Plasma/PackageStructure>
#include <Plasma/FrameSvg>
#include <Plasma/Label>
#include <Plasma/LineEdit>
#include <Plasma/PushButton>
#include <Plasma/ToolButton>

#include <kephal/kephal/screens.h>

#include "widgetexplorer.h"
#include "openwidgetassistant_p.h"

//CategoriesWidget

CategoriesWidget::CategoriesWidget(QGraphicsWidget *parent)
    : Plasma::PushButton(parent),
      m_model(0)
{
//    setAttribute(Qt::WA_NoSystemBackground);
//    nativeWidget()->setUsesScrollButtons(true);
//    connect(this, SIGNAL(currentChanged(int)), this, SIGNAL(filterChanged(int)));
    m_menu = new QMenu();
    m_menu->setTitle(i18n("Categories"));
    connect(m_menu, SIGNAL(triggered(QAction*)), this, SLOT(menuItemTriggered(QAction*)));
    setAction(m_menu->menuAction());
}

CategoriesWidget::~CategoriesWidget()
{
    m_menu->deleteLater();
}

void CategoriesWidget::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    Plasma::PushButton::mousePressEvent(event);
    QGraphicsView *view = Plasma::viewFor(this);
    if (view) {
        m_menu->adjustSize();
        QPoint pos = view->mapToGlobal(view->mapFromScene(scenePos()));
        const QSize msize = m_menu->size();
        const QRect screen = Kephal::ScreenUtils::screenGeometry(Kephal::ScreenUtils::screenId(view->rect().center()));
        if (pos.y() - msize.height() > screen.y()) {
            pos.setY(pos.y() - msize.height());
        } else {
            pos = view->mapToGlobal(view->mapFromScene(sceneBoundingRect().bottomLeft()));
        }
        m_menu->popup(pos);
    } else {
        m_menu->popup(event->screenPos());
    }
    QTimer::singleShot(0, this, SLOT(unpressButton()));
}

void CategoriesWidget::unpressButton()
{
    nativeWidget()->setDown(false);
}

void CategoriesWidget::menuItemTriggered(QAction *action)
{
    setText(action->text());
    setIcon(action->icon());
    emit filterChanged(action->data().toInt());
}

void CategoriesWidget::populateList()
{
    m_menu->clear();
    if (!m_model) {
        return;
    }

    for (int i = 0; i < m_model->rowCount(); i++){
        QStandardItem *item = getItemByProxyIndex(m_model->index(i, 0));
        if (item) {
            QAction *action = m_menu->addAction(item->icon(), item->text());
            action->setData(i);
            action->setEnabled(item->isEnabled());
        }
    }
}

QStandardItem *CategoriesWidget::getItemByProxyIndex(const QModelIndex &index) const
{
    return m_model->itemFromIndex(index);
}

void CategoriesWidget::setModel(QStandardItemModel *model)
{
    m_model = model;
    populateList();
}

//FilteringWidget

FilteringWidget::FilteringWidget(QGraphicsItem * parent, Qt::WindowFlags wFlags)
    : QGraphicsWidget(parent, wFlags),
      m_model(0),
      m_categories(0),
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
      m_categories(0),
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
    //init text search
    m_textSearch = new Plasma::LineEdit();

    m_textSearch->setClickMessage(i18n("Enter Search Term"));
    m_textSearch->setAttribute(Qt::WA_NoSystemBackground);
    m_textSearch->setClearButtonShown(true);

    m_newWidgetsButton = new Plasma::PushButton(this);
    m_newWidgetsButton->setText(i18n("Get New Widgets..."));
    m_newWidgetsButton->setIcon(KIcon("get-hot-new-stuff"));
    m_newWidgetsMenu = new KMenu(i18n("Get New Widgets..."));
    connect(m_newWidgetsMenu, SIGNAL(aboutToShow()), this, SLOT(populateWidgetsMenu()));
    m_newWidgetsButton->nativeWidget()->setMenu(m_newWidgetsMenu);
    m_newWidgetsButton->setMinimumWidth(m_newWidgetsButton->effectiveSizeHint(Qt::PreferredSize).width());

    //close button
    m_closeButton = new Plasma::ToolButton(this);
    m_closeButton->setIcon(KIcon("dialog-close"));
    // Make sure the height of the button is the same as the height of the filtering widget when orientation is Horizontal
    m_closeButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
    connect(m_closeButton, SIGNAL(clicked()), SIGNAL(closeClicked()));

    //layout
    setListOrientation(m_orientation);
}

void FilteringWidget::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    Q_UNUSED(event)

    QSizeF contentsSize = layout()->contentsRect().size();
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

void FilteringWidget::updateActions(const QList<QAction *> actions)
{
    foreach (QWeakPointer<Plasma::PushButton> button, m_actionButtons) {
        delete button.data();
    }
    m_actionButtons.clear();

    foreach (QAction *action, actions) {
        Plasma::PushButton *button = new Plasma::PushButton(this);
        button->setAction(action);
        int index = qMax(0, layout()->count());
        if (m_orientation == Qt::Horizontal) {
            QGraphicsLinearLayout *l = dynamic_cast<QGraphicsLinearLayout*>(layout());
            l->insertItem(index, button);
        } else {
            QGraphicsGridLayout *l = dynamic_cast<QGraphicsGridLayout*>(layout());
            l->addItem(button, l->rowCount(), 0, 1, 2);
        }
        m_actionButtons.append(button);
    }
}

void FilteringWidget::setModel(QStandardItemModel *model)
{
    m_model = model;

    if (m_categories) {
        m_categories->setModel(model);
    }
}

void FilteringWidget::setListOrientation(Qt::Orientation orientation)
{
    if (m_orientation == orientation && m_categories)
        return;

    m_orientation = orientation;

    if (!m_categories) {
        m_categories = new CategoriesWidget(this);
        connect(m_categories, SIGNAL(filterChanged(int)), this, SIGNAL(filterChanged(int)));
        m_categories->setModel(m_model);
    }


    if (orientation == Qt::Horizontal) {
        m_textSearch->setPreferredWidth(200);
        m_textSearch->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        m_textSearch->setPreferredHeight(-1);

        QGraphicsLinearLayout *newLayout = new QGraphicsLinearLayout(this);
        newLayout->addItem(m_textSearch);
        newLayout->addItem(m_categories);
        newLayout->addStretch();
        newLayout->addItem(m_newWidgetsButton);
        newLayout->addItem(m_closeButton);
        setLayout(newLayout);
        newLayout->setContentsMargins(4, 0, 0, 4);
    } else {
        m_textSearch->setPreferredHeight(30);
        m_textSearch->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        m_textSearch->setPreferredWidth(-1);

        QGraphicsGridLayout *newLayout = new QGraphicsGridLayout(this);
        newLayout->addItem(m_textSearch, 0, 0);
        newLayout->addItem(m_closeButton, 0, 1);
        newLayout->addItem(m_categories, 1, 0, 1, 2);
        newLayout->addItem(m_newWidgetsButton, 2, 0, 1, 2);
        setLayout(newLayout);
        newLayout->setContentsMargins(0, 4, 4, 0);
    }

    m_categories->setVisible(true);
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

    emit closeClicked();
    if (installer) {
        installer->createNewWidgetBrowser();
    } else {
        // we don't need to delete the default Applet::packageStructure as that
        // belongs to the applet
       Plasma::Applet::packageStructure()->createNewWidgetBrowser();
        /**
          for reference in a libplasma2 world, the above line equates to this:

          KNS3::DownloadDialog *knsDialog = m_knsDialog.data();
          if (!knsDialog) {
          m_knsDialog = knsDialog = new KNS3::DownloadDialog("plasmoids.knsrc", parent);
          connect(knsDialog, SIGNAL(accepted()), this, SIGNAL(newWidgetBrowserFinished()));
          }

          knsDialog->show();
          knsDialog->raise();
         */
    }
}

void FilteringWidget::openWidgetFile()
{
    emit closeClicked();

    Plasma::OpenWidgetAssistant *assistant = m_openAssistant.data();
    if (!assistant) {
        assistant = new Plasma::OpenWidgetAssistant(0);
        m_openAssistant = assistant;
    }

    KWindowSystem::setOnDesktop(assistant->winId(), KWindowSystem::currentDesktop());
    assistant->setAttribute(Qt::WA_DeleteOnClose, true);
    assistant->show();
    assistant->raise();
    assistant->setFocus();
}


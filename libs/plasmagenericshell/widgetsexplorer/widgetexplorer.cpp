/*
 *   Copyright (C) 2007 by Ivan Cukic <ivan.cukic+kde@gmail.com>
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

#include "widgetexplorer.h"

#include <QDeclarativeContext>
#include <QDeclarativeEngine>
#include <QDeclarativeComponent>

#include <kaction.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kmenu.h>
#include <kpushbutton.h>
#include <kservicetypetrader.h>
#include <kstandardaction.h>
#include <kaboutdata.h>
#include <kaboutapplicationdialog.h>
#include <kcomponentdata.h>
#include <kpluginloader.h>
#include <klineedit.h>
#include <KStandardDirs>
#include <KWindowSystem>

#include <plasma/applet.h>
#include <plasma/corona.h>
#include <plasma/containment.h>
#include <plasma/widgets/toolbutton.h>
#include <plasma/widgets/lineedit.h>
#include <Plasma/DeclarativeWidget>

#include "kcategorizeditemsviewmodels_p.h"
#include "plasmaappletitemmodel_p.h"
#include "openwidgetassistant_p.h"

//getting the user local
//KGlobal::dirs()->localkdedir();
//Compare it to the entryPath of the KPluginInfo
//and see if it can be uninstalled

using namespace KCategorizedItemsViewModels;

namespace Plasma
{

class WidgetExplorerPrivate
{

public:
    WidgetExplorerPrivate(WidgetExplorer *w)
        : q(w),
          containment(0),
          itemModel(w),
          filterModel(w),
          iconSize(16)
    {
    }

    void initFilters();
    void init(Plasma::Location loc);
    void initRunningApplets();
    void containmentDestroyed();
    void setLocation(Plasma::Location loc);
    void finished();

    /**
     * Tracks a new running applet
     */
    void appletAdded(Plasma::Applet *applet);

    /**
     * A running applet is no more
     */
    void appletRemoved(Plasma::Applet *applet);

    //this orientation is just for convenience, is the location that is important
    Qt::Orientation orientation;
    Plasma::Location location;
    WidgetExplorer *q;
    QString application;
    Plasma::Containment *containment;

    QHash<QString, int> runningApplets; // applet name => count
    //extra hash so we can look up the names of deleted applets
    QHash<Plasma::Applet *,QString> appletNames;
    QWeakPointer<Plasma::OpenWidgetAssistant> openAssistant;

    PlasmaAppletItemModel itemModel;
    KCategorizedItemsViewModels::DefaultFilterModel filterModel;
    DefaultItemFilterProxyModel filterItemModel;

    Plasma::DeclarativeWidget *declarativeWidget;

    QGraphicsLinearLayout *mainLayout;
    int iconSize;
};

void WidgetExplorerPrivate::initFilters()
{
    filterModel.addFilter(i18n("All Widgets"),
                          KCategorizedItemsViewModels::Filter(), KIcon("plasma"));

    // Filters: Special
    filterModel.addFilter(i18n("Running"),
                          KCategorizedItemsViewModels::Filter("running", true),
                          KIcon("dialog-ok"));

    filterModel.addSeparator(i18n("Categories:"));

    typedef QPair<QString, QString> catPair;
    QMap<QString, catPair > categories;
    QSet<QString> existingCategories = itemModel.categories();
    foreach (const QString &category, Plasma::Applet::listCategories(application)) {
        const QString lowerCaseCat = category.toLower();
        if (existingCategories.contains(lowerCaseCat)) {
            const QString trans = i18n(category.toLocal8Bit());
            categories.insert(trans.toLower(), qMakePair(trans, lowerCaseCat));
        }
    }

    foreach (const catPair &category, categories) {
        filterModel.addFilter(category.first,
                              KCategorizedItemsViewModels::Filter("category", category.second));
    }

}

void WidgetExplorerPrivate::init(Plasma::Location loc)
{
    q->setFocusPolicy(Qt::StrongFocus);

    //init widgets
    location = loc;
    orientation = ((location == Plasma::LeftEdge || location == Plasma::RightEdge)?Qt::Vertical:Qt::Horizontal);
    mainLayout = new QGraphicsLinearLayout(Qt::Vertical);
    mainLayout->setContentsMargins(0, 4, 0, 0);
    mainLayout->setSpacing(0);

    //connect
 //QObject::connect(filteringWidget, SIGNAL(closeClicked()), q, SIGNAL(closeClicked()));

    initRunningApplets();

    declarativeWidget = new Plasma::DeclarativeWidget(q);
    declarativeWidget->setInitializationDelayed(true);
    declarativeWidget->setQmlPath(KStandardDirs::locate("data", "plasma/widgetsexplorer/widgetsexplorer.qml"));
    mainLayout->addItem(declarativeWidget);

    if (declarativeWidget->engine()) {
        QDeclarativeContext *ctxt = declarativeWidget->engine()->rootContext();
        if (ctxt) {
            filterItemModel.setSortCaseSensitivity(Qt::CaseInsensitive);
            filterItemModel.setDynamicSortFilter(true);
            filterItemModel.setSourceModel(&itemModel);
            filterItemModel.sort(0);
            ctxt->setContextProperty("appletsModel", &filterItemModel);
            ctxt->setContextProperty("filterModel", &filterModel);
            QObject::connect(declarativeWidget, SIGNAL(finished()), q, SLOT(finished()));
        }
    }

    q->setLayout(mainLayout);
}

void WidgetExplorerPrivate::finished()
{
    if (declarativeWidget->mainComponent()->isError()) {
        return;
    }

    QObject::connect(declarativeWidget->rootObject(), SIGNAL(addAppletRequested(const QString &)),
                     q, SLOT(addApplet(const QString &)));
    QObject::connect(declarativeWidget->rootObject(), SIGNAL(closeRequested()),
                     q, SIGNAL(closeClicked()));

    QObject::connect(declarativeWidget->rootObject(), SIGNAL(openWidgetFileRequested()),
                     q, SLOT(openWidgetFile()));
    QObject::connect(declarativeWidget->rootObject(), SIGNAL(downloadWidgetsRequested(QString)),
                     q, SLOT(downloadWidgets(QString)));

    QList<QObject *> actionList;
    foreach (QAction *action, q->actions()) {
        actionList << action;
    }
    declarativeWidget->rootObject()->setProperty("extraActions", QVariant::fromValue(actionList));
}

void WidgetExplorerPrivate::setLocation(const Plasma::Location loc)
{
    if (location == loc) {
        return;
    }

    location = loc;
    orientation = ((location == Plasma::LeftEdge || location == Plasma::RightEdge)?Qt::Vertical:Qt::Horizontal);

}

void WidgetExplorerPrivate::initRunningApplets()
{
    //get applets from corona, count them, send results to model
    if (!containment) {
        return;
    }

    Plasma::Corona *c = containment->corona();

    //we've tried our best to get a corona
    //we don't want just one containment, we want them all
    if (!c) {
        kDebug() << "can't happen";
        return;
    }

    appletNames.clear();
    runningApplets.clear();
    QList<Containment*> containments = c->containments();
    foreach (Containment *containment, containments) {
        QObject::connect(containment, SIGNAL(appletAdded(Plasma::Applet*,QPointF)), q, SLOT(appletAdded(Plasma::Applet*)));
        QObject::connect(containment, SIGNAL(appletRemoved(Plasma::Applet*)), q, SLOT(appletRemoved(Plasma::Applet*)));

        foreach (Applet *applet, containment->applets()) {
            runningApplets[applet->pluginName()]++;
        }
    }

    //kDebug() << runningApplets;
    itemModel.setRunningApplets(runningApplets);
}

void WidgetExplorerPrivate::containmentDestroyed()
{
    containment = 0;
}

void WidgetExplorerPrivate::appletAdded(Plasma::Applet *applet)
{
    QString name = applet->pluginName();

    runningApplets[name]++;
    appletNames.insert(applet, name);
    itemModel.setRunningApplets(name, runningApplets[name]);
}

void WidgetExplorerPrivate::appletRemoved(Plasma::Applet *applet)
{
    Plasma::Applet *a = (Plasma::Applet *)applet; //don't care if it's valid, just need the address

    QString name = appletNames.take(a);

    int count = 0;
    if (runningApplets.contains(name)) {
        count = runningApplets[name] - 1;

        if (count < 1) {
            runningApplets.remove(name);
        } else {
            runningApplets[name] = count;
        }
    }

    itemModel.setRunningApplets(name, count);
}

//WidgetExplorer

WidgetExplorer::WidgetExplorer(Plasma::Location loc, QGraphicsItem *parent)
        :QGraphicsWidget(parent),
        d(new WidgetExplorerPrivate(this))
{
    d->init(loc);
}

WidgetExplorer::WidgetExplorer(QGraphicsItem *parent)
        :QGraphicsWidget(parent),
        d(new WidgetExplorerPrivate(this))
{
    d->init(Plasma::BottomEdge);
}

WidgetExplorer::~WidgetExplorer()
{
     delete d;
}

void WidgetExplorer::setLocation(Plasma::Location loc)
{
    d->setLocation(loc);
    emit(locationChanged(loc));
}

Plasma::Location WidgetExplorer::location() const
{
    return d->location;
}

void WidgetExplorer::setIconSize(int size)
{
    //TODO
    adjustSize();
}

int WidgetExplorer::iconSize() const
{
    //TODO
    return 32;
}

void WidgetExplorer::populateWidgetList(const QString &app)
{
    d->application = app;
    d->itemModel.setApplication(app);
    d->initFilters();

    d->itemModel.setRunningApplets(d->runningApplets);

}

QString WidgetExplorer::application()
{
    return d->application;
}

void WidgetExplorer::setContainment(Plasma::Containment *containment)
{
    if (d->containment != containment) {
        if (d->containment) {
            d->containment->disconnect(this);
        }

        d->containment = containment;

        if (d->containment) {
            connect(d->containment, SIGNAL(destroyed(QObject*)), this, SLOT(containmentDestroyed()));
            connect(d->containment, SIGNAL(immutabilityChanged(Plasma::ImmutabilityType)), this, SLOT(immutabilityChanged(Plasma::ImmutabilityType)));

            setLocation(containment->location());
        }

        d->initRunningApplets();
    }
}

Containment *WidgetExplorer::containment() const
{
    return d->containment;
}

Plasma::Corona *WidgetExplorer::corona() const
{
    if (d->containment) {
        return d->containment->corona();
    }

    return 0;
}

void WidgetExplorer::addApplet(const QString &pluginName)
{
    d->containment->addApplet(pluginName);
}

void WidgetExplorer::immutabilityChanged(Plasma::ImmutabilityType type)
{
    if (type != Plasma::Mutable) {
        emit closeClicked();
    }
}

void WidgetExplorer::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        // have to treat escape specially, as it makes text() return " "
        QGraphicsWidget::keyPressEvent(event);
        return;
    }

}

bool WidgetExplorer::event(QEvent *event)
{
    switch (event->type()) {
        case QEvent::ActionAdded:
        case QEvent::ActionChanged:
        case QEvent::ActionRemoved:
            if (d->declarativeWidget->rootObject()) {
                QList<QObject *> actionList;
                foreach (QAction *action, actions()) {
                    actionList << action;
                }
                d->declarativeWidget->rootObject()->setProperty("extraActions", QVariant::fromValue(actionList));
            }
            break;
        default:
            break;
    }

    return QGraphicsWidget::event(event);
}

void WidgetExplorer::focusInEvent(QFocusEvent* event)
{
    Q_UNUSED(event);
}

void WidgetExplorer::downloadWidgets(const QString &type)
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

void WidgetExplorer::openWidgetFile()
{
    emit closeClicked();

    Plasma::OpenWidgetAssistant *assistant = d->openAssistant.data();
    if (!assistant) {
        assistant = new Plasma::OpenWidgetAssistant(0);
        d->openAssistant = assistant;
    }

    KWindowSystem::setOnDesktop(assistant->winId(), KWindowSystem::currentDesktop());
    assistant->setAttribute(Qt::WA_DeleteOnClose, true);
    assistant->show();
    assistant->raise();
    assistant->setFocus();
}

} // namespace Plasma

#include "widgetexplorer.moc"

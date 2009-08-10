#include "widgetexplorer.h"

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

#include <plasma/applet.h>
#include <plasma/corona.h>
#include <plasma/containment.h>

#include "kcategorizeditemsviewmodels_p.h"
#include "plasmaappletitemmodel_p.h"
#include "openwidgetassistant_p.h"
#include "appletslist.h"
#include "managewidgets.h"
#include "appletsfiltering.h"

using namespace KCategorizedItemsViewModels;

namespace Plasma
{

class WidgetExplorerPrivate
{

public:
    WidgetExplorerPrivate(WidgetExplorer *w)
        : q(w),
          containment(0),
          config("plasmarc"),
          configGroup(&config, "Applet Browser"),
          itemModel(configGroup, w),
          filterModel(w)
    {
    }

    void initFilters();
    void init(Qt::Orientation orientation);
    void initPushButtonWidgetMenu();
    void initRunningApplets();
    void containmentDestroyed();
    void setOrientation(Qt::Orientation orientation);
    void adjustContentsSize();
    void setMainSize();

    /**
     * Tracks a new running applet
     */
    void appletAdded(Plasma::Applet *applet);

    /**
     * A running applet is no more
     */
    void appletRemoved(Plasma::Applet *applet);

    Qt::Orientation orientation;
    WidgetExplorer *q;
    QString application;
    Plasma::Containment *containment;
    Plasma::Corona *corona;
    
    QHash<QString, int> runningApplets; // applet name => count
    //extra hash so we can look up the names of deleted applets
    QHash<Plasma::Applet *,QString> appletNames;

    KConfig config;
    KConfigGroup configGroup;

    PlasmaAppletItemModel itemModel;
    KCategorizedItemsViewModels::DefaultFilterModel filterModel;

    /**
     * Button to install new widgets and its menu
     */
    ManageWidgetsPushButton *pushButtonWidget;
    KMenu *pushButtonWidgetMenu;
    /**
     * Widget that lists the applets
     */
    AppletsList *appletsListWidget;

    /**
     * Widget that contains the search and categories filters
     */
    FilteringWidget *filteringWidget;
    QGraphicsLinearLayout *mainLayout;

};

void WidgetExplorerPrivate::initFilters()
{
    filterModel.clear();

    filterModel.addFilter(i18n("All Widgets"),
                          KCategorizedItemsViewModels::Filter(), KIcon("plasma"));

    // Filters: Special
    filterModel.addFilter(i18n("Used Before"),
                          KCategorizedItemsViewModels::Filter("used", true),
                          KIcon("view-history"));
    filterModel.addFilter(i18n("Running"),
                          KCategorizedItemsViewModels::Filter("running", true),
                          KIcon("view-history"));

    filterModel.addSeparator(i18n("Categories:"));

    foreach (const QString &category, Plasma::Applet::listCategories(application)) {
        filterModel.addFilter(i18n(category.toLocal8Bit()),
                              KCategorizedItemsViewModels::Filter("category", category.toLower()));
    }
}

void WidgetExplorerPrivate::init(Qt::Orientation orient)
{
    //init widgets
    mainLayout = new QGraphicsLinearLayout(Qt::Vertical);
    orientation = orient;
    filteringWidget = new FilteringWidget(orientation);
    appletsListWidget = new AppletsList(orientation);

    setMainSize();

    //connect
    //QObject::connect(appletsListWidget, SIGNAL(doubleClicked(const QModelIndex &)), q, SLOT(addApplet()));
    QObject::connect(appletsListWidget, SIGNAL(appletDoubleClicked(PlasmaAppletItem*)), q, SLOT(addApplet(PlasmaAppletItem*)));
    QObject::connect(appletsListWidget, SIGNAL(infoButtonClicked(QString)), q, SLOT(infoAboutApplet(QString)));
    QObject::connect(filteringWidget->textSearch()->nativeWidget(), SIGNAL(textChanged(QString)), appletsListWidget, SLOT(searchTermChanged(QString)));
    QObject::connect(filteringWidget, SIGNAL(filterChanged(int)), appletsListWidget, SLOT(filterChanged(int)));

    //adding to layout
    mainLayout->addItem(filteringWidget);
    mainLayout->addItem(appletsListWidget);
    mainLayout->setAlignment(appletsListWidget, Qt::AlignHCenter);
    mainLayout->setAlignment(appletsListWidget, Qt::AlignVCenter);

    //filters & models
    initFilters();
    filteringWidget->setModel(&filterModel);
    appletsListWidget->setFilterModel(&filterModel);
    appletsListWidget->setItemModel(&itemModel);
    initRunningApplets();

    q->setContentsMargins(15,15,15,15);
    //set the layout
    q->setLayout(mainLayout);

}

void WidgetExplorerPrivate::setMainSize()
{
    // **** find a fancier way to use screen ****
    QDesktopWidget *screen = new QDesktopWidget();
    QSize screenSize = screen->screenGeometry(-1).size();
    qDebug() << "screen size " << screen->screenGeometry(-1).size();

    if(orientation == Qt::Horizontal) {
       q->setMinimumWidth(screenSize.width());
       q->setMaximumWidth(screenSize.width());
       q->setMinimumHeight(screenSize.height()/4);
       q->setMaximumHeight(screenSize.height()/4);
    } else {
       q->setMinimumHeight(screenSize.height());
       q->setMaximumHeight(screenSize.height());
       q->setMinimumWidth(screenSize.width()/5);
       q->setMaximumWidth(screenSize.width()/5);
    }
}

void WidgetExplorerPrivate::adjustContentsSize()
{
    mainLayout->invalidate();

    QSizeF contentsSize = q->contentsRect().size();
    qDebug() << "contents size: " << contentsSize;

    if(orientation == Qt::Horizontal) {
        if(filteringWidget != 0) {
            filteringWidget->setPreferredSize(-1, -1);
        }

        if(appletsListWidget != 0) {
            appletsListWidget->setPreferredSize(-1,-1);
//            appletsListWidget->setMaximumHeight(contentsSize.height()
//                                                - filteringWidget->size().height());
        }

    } else {
        if(filteringWidget != 0) {
            filteringWidget->setMinimumHeight(contentsSize.height()/4);
            filteringWidget->setMaximumHeight(contentsSize.height()/4);
            filteringWidget->setMinimumWidth(contentsSize.width());
            filteringWidget->setMaximumWidth(contentsSize.width());
        }

        if(appletsListWidget != 0) {
            appletsListWidget->setMinimumWidth(contentsSize.width());
            appletsListWidget->setMaximumWidth(contentsSize.width());
        }
    }
    mainLayout->activate();
}

void WidgetExplorerPrivate::setOrientation(Qt::Orientation orient)
{
    orientation = orient;
    filteringWidget->setListOrientation(orientation);
    appletsListWidget->setOrientation(orientation);
    setMainSize();
}

void WidgetExplorerPrivate::initPushButtonWidgetMenu()
{
    pushButtonWidgetMenu = new KMenu(i18n("Get New Widgets"));
    QObject::connect(pushButtonWidgetMenu, SIGNAL(aboutToShow()), q, SLOT(populateWidgetsMenu()));
    pushButtonWidget->button()->setMenu(pushButtonWidgetMenu);

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
    //kDebug() << name;

    runningApplets[name]++;
    appletNames.insert(applet, name);
    itemModel.setRunningApplets(name, runningApplets[name]);
}

void WidgetExplorerPrivate::appletRemoved(Plasma::Applet *applet)
{
    //kDebug() << (QObject*)applet;
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

/* This is just a wrapper around KAboutApplicationDialog that deletes
the KAboutData object that it is associated with, when it is deleted.
This is required to free memory correctly when KAboutApplicationDialog
is called with a temporary KAboutData that is allocated on the heap.
(see the code below, in WidgetExplorer::infoAboutApplet())
*/
class KAboutApplicationDialog2 : public KAboutApplicationDialog
{
public:
    KAboutApplicationDialog2(KAboutData *ab, QWidget *parent = 0)
    : KAboutApplicationDialog(ab, parent), m_ab(ab) {}

    ~KAboutApplicationDialog2()
    {
        delete m_ab;
    }

private:
    KAboutData *m_ab;
};


//WidgetExplorer

WidgetExplorer::WidgetExplorer(QGraphicsItem *parent)
        :QGraphicsWidget(parent),
        d(new WidgetExplorerPrivate(this))
{
    d->init(Qt::Horizontal);
    m_backgroundSvg = new Plasma::FrameSvg(this);
    m_backgroundSvg->setImagePath("widgets/translucentbackground");
}

WidgetExplorer::~WidgetExplorer()
{
     delete d;
}

void WidgetExplorer::resizeEvent(QGraphicsSceneResizeEvent *event) {
    Q_UNUSED(event);
    d->adjustContentsSize();
}

void WidgetExplorer::setOrientation(Qt::Orientation orientation)
{
    d->setOrientation(orientation);
    emit(orientationChanged(orientation));
}

void WidgetExplorer::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
 {
     QGraphicsWidget::paint(painter, option, widget);
     m_backgroundSvg->resizeFrame(size());
     m_backgroundSvg->paintFrame(painter, pos());
     //again
     m_backgroundSvg->paintFrame(painter, pos());
 }

void WidgetExplorer::setApplication(const QString &app)
{
    d->application = app;
    d->initFilters();
    d->itemModel.setApplication(app);

    //FIXME: AFAIK this shouldn't be necessary ... but here it is. need to find out what in that
    //       maze of models and views is screwing up
    d->appletsListWidget->setItemModel(&d->itemModel);

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
        }

        d->initRunningApplets();
    }
}

void WidgetExplorer::setCorona(Plasma::Corona *corona)
{
    if (d->corona != corona) {
        d->corona = corona;
    }
}

Containment *WidgetExplorer::containment() const
{
    return d->containment;
}

Plasma::Corona *WidgetExplorer::corona() const
{
    return d->corona;
}

void WidgetExplorer::addApplet()
{
    if (!d->containment) {
        return;
    }

    foreach (AbstractItem *item, d->appletsListWidget->selectedItems()) {
        PlasmaAppletItem *selectedItem = (PlasmaAppletItem *) item;
        kDebug() << "Adding applet " << selectedItem->name() << "to containment";
        d->containment->addApplet(selectedItem->pluginName(), selectedItem->arguments());
    }
}

void WidgetExplorer::addApplet(PlasmaAppletItem *appletItem)
{
    if (!d->containment) {
        return;
    }

    kDebug() << appletItem->pluginName() << appletItem->arguments();

    d->containment->addApplet(appletItem->pluginName(), appletItem->arguments());
}

void WidgetExplorer::destroyApplets(const QString &name)
{
    if (!d->containment) {
        return;
    }

    Plasma::Corona *c = d->containment->corona();

    //we've tried our best to get a corona
    //we don't want just one containment, we want them all
    if (!c) {
        kDebug() << "can't happen";
        return;
    }

    foreach (Containment *containment, c->containments()) {
        QList<Applet*> applets = containment->applets();
        foreach (Applet *applet, applets) {
            if (applet->pluginName() == name) {
                d->appletNames.remove(applet);
                applet->disconnect(this);
                applet->destroy();
            }
        }
    }

    d->runningApplets.remove(name);
    d->itemModel.setRunningApplets(name, 0);
}

void WidgetExplorer::infoAboutApplet(const QString &name)
{

    qDebug() << name;

    if (!d->containment) {
        return;
    }

    KPluginInfo::List applets = Plasma::Applet::listAppletInfo();
    foreach (const KPluginInfo &info, applets) {
        if (info.pluginName() == name) {
            KAboutData *aboutData = new KAboutData(info.name().toUtf8(),
                                              info.name().toUtf8(),
                                              ki18n(info.name().toUtf8()),
                                              info.version().toUtf8(), ki18n(info.comment().toUtf8()),
                                              info.fullLicense().key(), ki18n(QByteArray()), ki18n(QByteArray()), info.website().toLatin1(),
                                              info.email().toLatin1());

            aboutData->setProgramIconName(info.icon());

            aboutData->addAuthor(ki18n(info.author().toUtf8()), ki18n(QByteArray()), info.email().toLatin1());

            //TODO should recycle this dialog if it is called up twice
            //KAboutApplication nao recebe QGraphicsWidget como parent, entao coloquei 0
            //KAboutApplicationDialog *aboutDialog = new KAboutApplicationDialog2(aboutData, this);
            KAboutApplicationDialog *aboutDialog = new KAboutApplicationDialog2(aboutData, 0);
            aboutDialog->show();
            break;
        }
    }
}

void WidgetExplorer::downloadWidgets(const QString &type)
{
    PackageStructure *installer = 0;

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
       Applet::packageStructure()->createNewWidgetBrowser();
    }
}

void WidgetExplorer::openWidgetFile()
{
    // TODO: if we already have one of these showing and the user clicks to
    // add it again, show the same window?
    OpenWidgetAssistant *assistant = new OpenWidgetAssistant(0);
    assistant->setAttribute(Qt::WA_DeleteOnClose, true);
    assistant->show();
}

void WidgetExplorer::populateWidgetsMenu()
{

    if (!d->pushButtonWidgetMenu->actions().isEmpty()) {
        // already populated.
        return;
    }

    QSignalMapper *mapper = new QSignalMapper(this);
    QObject::connect(mapper, SIGNAL(mapped(QString)), this, SLOT(downloadWidgets(QString)));

    QAction *action = new QAction(KIcon("applications-internet"),
                                  i18n("Download New Plasma Widgets"), this);
    QObject::connect(action, SIGNAL(triggered(bool)), mapper, SLOT(map()));
    mapper->setMapping(action, QString());
    d->pushButtonWidgetMenu->addAction(action);

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
            d->pushButtonWidgetMenu->addAction(action);
        }
    }

    d->pushButtonWidgetMenu->addSeparator();

    action = new QAction(KIcon("package-x-generic"),
                         i18n("Install Widget From Local File..."), this);
    QObject::connect(action, SIGNAL(triggered(bool)), this, SLOT(openWidgetFile()));
    d->pushButtonWidgetMenu->addAction(action);
}

} // namespace Plasma

#include "widgetexplorer.moc"

#include "desktop.h"

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QVBoxLayout>
#include <QDesktopWidget>
#include <QApplication>
#include <QDebug>
#include <QGLWidget>
#include <QProcess>
#include <KWindowSystem>
#include <KMenu>
#include <KLocale>

#include <KLibrary>
#include <KLibLoader>
#include <QFile>

#include "applet.h"
#include "dataengine.h"
#include "svg.h"
#include "widgets/lineedit.h"

//#include <clock.h>
#include "plasmaapp.h"

using namespace Plasma;
extern "C" {
    typedef QGraphicsItem* (*loadKaramba)(const KUrl &theme, QGraphicsView *view);
}

Desktop::Desktop(QWidget *parent)
    : QGraphicsView(parent)
{
    setFrameShape(QFrame::NoFrame);
/*    setBackgroundMode(Qt::NoBackground);*/
    setAutoFillBackground(true);

/*    QPalette pal = palette();
    pal.setBrush(QPalette::Base, Qt::transparent);
    setPalette(pal);*/
    //setViewport(new QGLWidget  ( QGLFormat(QGL::StencilBuffer | QGL::AlphaChannel)   ));

    m_graphicsScene = new QGraphicsScene(rect());
    m_graphicsScene->setItemIndexMethod(QGraphicsScene::NoIndex);
    setScene(m_graphicsScene);
    setDragMode(QGraphicsView::RubberBandDrag);
    setCacheMode(QGraphicsView::CacheBackground);
    setInteractive(true);

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    KPluginInfo::List applets = Plasma::Applet::knownApplets();
    kDebug() << "======= Applets =========" << endl;
    foreach (KPluginInfo* info, applets) {
        kDebug() << info->pluginName() << ": " << info->name() << endl;
    }
    kDebug() << "=========================" << endl;

//    Clock *clock = new Clock(0, 1);
    Plasma::Applet* clock = Plasma::Applet::loadApplet("clock");
    if (clock) {
        m_graphicsScene->addItem(clock);
    } else {
        kDebug() << "what, no pretty clocks? *sob*" << endl;
    }

//    Plasma::DataEngine* time = PlasmaApp::self()->loadDataEngine("time");
//     Plasma::LineEdit* l = new Plasma::LineEdit;
//     m_graphicsScene->addItem(l);
//     l->moveBy(400, 400);
//     time->connectSource("time", l);

    // Loading SuperKaramba themes example
    /*
    QString karambaLib = QFile::encodeName(KLibLoader::self()->findLibrary(QLatin1String("libsuperkaramba")));
    KLibLoader* loader = KLibLoader::self();
    KLibrary* lib = loader->library(karambaLib);

    if (lib) {
        loadKaramba startKaramba = 0;
        startKaramba = (loadKaramba)lib->resolveFunction("startKaramba");
        if (startKaramba) {
            startKaramba(KUrl("~/themes/aero_aio.skz"), this);
        }

        lib->unload();
    }
    */
    m_background = new Plasma::Svg("background/dialog", this);

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(displayContextMenu(QPoint)));
    engineExplorer = new QAction(i18n("Engine Explorer"), this);
    connect(engineExplorer, SIGNAL(triggered(bool)), this, SLOT(launchExplorer(bool)));
    exitPlasma = new QAction(i18n("Exit Plasma"), this);
    connect(exitPlasma, SIGNAL(triggered(bool)), kapp, SLOT(quit()));

    connect(exitPlasma, SIGNAL(triggered(bool)), kapp, SLOT(quit()));
}

Desktop::~Desktop()
{
    foreach (Plasma::Applet* plasmoid, loadedPlasmoidList) {
        delete plasmoid;
    }
    loadedPlasmoidList.clear();
}

void Desktop::addPlasmoid(const QString& name)
{
    Plasma::Applet* plasmoid = Plasma::Applet::loadApplet(name);
    if (plasmoid) {
        m_graphicsScene->addItem(plasmoid);
        loadedPlasmoidList << plasmoid;
    } else {
        kDebug() << "Plasmoid " << name << " could not be loaded." << endl;
    }
}

void Desktop::drawBackground(QPainter * painter, const QRectF &)
{
    m_background->paint(painter, rect());
}

void Desktop::resizeEvent(QResizeEvent* event)
{
    Q_UNUSED(event);
    m_graphicsScene->setSceneRect(rect());
    m_background->resize(width(), height());
}

void Desktop::displayContextMenu(const QPoint& point)
{
    Plasma::Applet* applet = qgraphicsitem_cast<Plasma::Applet*>( itemAt( point ) );
    KMenu desktopMenu(this);
    if(!applet) {
        desktopMenu.setTitle("Desktop");
        desktopMenu.addAction("The");
        desktopMenu.addAction("desktop");
        desktopMenu.addAction("menu");
        desktopMenu.addAction(engineExplorer);
        desktopMenu.addAction(exitPlasma);
    } else {
        //desktopMenu.setTitle( applet->name() ); //This isn't implemented in Applet yet...
        desktopMenu.addAction("Widget");
        desktopMenu.addAction("settings");
        desktopMenu.addAction("like");
        desktopMenu.addAction("opacity");
        desktopMenu.addSeparator();
        QAction* configureApplet = new QAction(i18n("Configure Applet..."), this);
        connect(configureApplet, SIGNAL(triggered(bool)),
                applet, SLOT(configureDialog())); //This isn't implemented in Applet yet...
        desktopMenu.addAction(configureApplet);
    }
    desktopMenu.exec(point);
}

void Desktop::launchExplorer(bool /*param*/)
{
    QProcess::execute("plasmaengineexplorer");
}

#include "desktop.moc"


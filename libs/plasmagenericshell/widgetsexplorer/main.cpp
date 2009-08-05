#include <QtGui>
// #include "customwidgets.h"
#include <plasma/applet.h>
#include "widgetexplorer.h"
#include <plasma/containment.h>
#include <QDesktopWidget>
#include <plasma/corona.h>
#include <plasma/view.h>
#include <plasma/containment.h>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    Plasma::Containment *containment = new Plasma::Containment();

    KPluginInfo::List list = Plasma::Containment::listContainments();

    foreach(KPluginInfo info, list) {
        kDebug() << info.pluginName();
    }

    Plasma::WidgetExplorer *appletBrowser;

    appletBrowser = new Plasma::WidgetExplorer();
    appletBrowser->setContainment(containment);
    appletBrowser->setApplication();

    Plasma::Corona *scene = new Plasma::Corona();
    appletBrowser->setCorona(scene);

    scene->addItem(appletBrowser);
    scene->setSceneRect(QRectF(0, 0, appletBrowser->minimumWidth(), appletBrowser->minimumHeight()));

    Plasma::View *view = new Plasma::View(appletBrowser->containment(), 0);
    view->setScene(scene);
    view->setWindowFlags(Qt::FramelessWindowHint);
    view->setAttribute(Qt::WA_TranslucentBackground, true);
    view->setStyleSheet("background: transparent");

    QDesktopWidget *screen = new QDesktopWidget();

    view->setMinimumWidth(appletBrowser->minimumWidth());
    view->setMaximumWidth(appletBrowser->minimumWidth());
    view->setMinimumHeight(appletBrowser->minimumHeight() + 30);
    view->setMaximumHeight(appletBrowser->minimumHeight() + 30);
    view->move(0, screen->height() - view->maximumHeight());
    view->show();

    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    return app.exec();
}

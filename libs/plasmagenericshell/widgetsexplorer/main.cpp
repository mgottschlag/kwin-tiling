#include <QtGui>
#include "customwidgets.h"
#include <plasma/applet.h>
#include "widgetexplorer.h"
#include <plasma/containment.h>
#include <QDesktopWidget>
#include <plasma/corona.h>
#include <plasma/view.h>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    Plasma::Containment *containment = new Plasma::Containment();
    Plasma::WidgetExplorerMainWidget *appletBrowser;

    appletBrowser = new Plasma::WidgetExplorerMainWidget();
    appletBrowser->setContainment(containment);
    appletBrowser->setApplication();

    Plasma::Corona *scene = new Plasma::Corona();

    scene->addItem(appletBrowser);
    scene->setSceneRect(QRectF(0, 0, appletBrowser->minimumWidth(), appletBrowser->minimumHeight()));

    Plasma::View *view = new Plasma::View(appletBrowser->containment(), 0);
    view->setScene(scene);
    view->setWindowFlags(Qt::FramelessWindowHint);
    view->setAttribute(Qt::WA_TranslucentBackground, true);
    view->setStyleSheet("background: transparent");

    view->setMinimumWidth(appletBrowser->minimumWidth());
    view->setMaximumWidth(appletBrowser->minimumWidth());
    view->setMinimumHeight(appletBrowser->minimumHeight() + 30);
    view->setMaximumHeight(appletBrowser->minimumHeight() + 30);
    view->show();

    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    return app.exec();
}

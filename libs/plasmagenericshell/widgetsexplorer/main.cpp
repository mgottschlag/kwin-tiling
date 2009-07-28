#include <QtGui>
#include "customwidgets.h"
#include <plasma/applet.h>
#include "widgetexplorer.h"
#include <plasma/containment.h>
#include <QDesktopWidget>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    Plasma::Containment *containment = new Plasma::Containment();
    Plasma::WidgetExplorerMainWidget *appletBrowser;

    appletBrowser = new Plasma::WidgetExplorerMainWidget();
    appletBrowser->setContainment(containment);
    appletBrowser->setApplication();

    QGraphicsScene *scene = new QGraphicsScene();
    scene->addItem(appletBrowser);
    scene->setSceneRect(QRectF(0, 0, appletBrowser->minimumWidth(), appletBrowser->minimumHeight()));

    QGraphicsView *view = new QGraphicsView(scene);
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

#include <QtGui>
#include "customwidgets.h"
#include "standardcustomwidget.h"
#include <plasma/applet.h>
#include "widgetexplorer.h"
#include <plasma/containment.h>
#include <KWindowSystem>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    Plasma::Containment *containment = new Plasma::Containment();
    Plasma::WidgetExplorer *appletBrowser;

    appletBrowser = new Plasma::WidgetExplorer();
    appletBrowser->setContainment(containment);
    appletBrowser->setApplication();
    //appletBrowser->setAttribute(Qt::WA_DeleteOnClose);

    KWindowSystem::setOnDesktop(appletBrowser->winId(), KWindowSystem::currentDesktop());
    appletBrowser->show();
    KWindowSystem::activateWindow(appletBrowser->winId());

    return app.exec();
}

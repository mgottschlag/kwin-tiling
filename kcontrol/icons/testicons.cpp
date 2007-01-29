/* Test program for icons setup module. */

#include <qapplication.h>
#include <kcomponentdata.h>
#include "icons.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv, "testicons");
    KComponentData componentData("testicons");
    KIconConfig *w = new KIconConfig(componentData, 0L);
    app.setMainWidget(w);
    w->show();
    return app.exec();
}

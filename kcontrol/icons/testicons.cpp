/* Test program for icons setup module. */

#include <qapplication.h>
#include <kinstance.h>
#include "icons.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv, "testicons");
    KInstance instance("testicons");
    KIconConfig *w = new KIconConfig(&instance, 0L);
    app.setMainWidget(w);
    w->show();
    return app.exec();
}

/* Test program for icons setup module. */

#include <kapplication.h>
#include "icons.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv, "testicons");
    KIconConfig *w = new KIconConfig(KGlobal::instance(), 0L);
    app.setMainWidget(w);
    w->show();
    return app.exec();
}

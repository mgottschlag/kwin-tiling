#include <QApplication>
#include <QWidget>

#include <kcmodule.h>
#include <kinstance.h>

#include "kcmlayout.h"


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QWidget window;

	KInstance* inst = new KInstance("kcmlayout");
	new LayoutConfig(inst, &window);

    window.show();
    return app.exec();
}

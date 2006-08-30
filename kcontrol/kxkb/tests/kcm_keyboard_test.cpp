#include <QApplication>
#include <QWidget>

#include <kcmodule.h>
#include <kinstance.h>
#include <kgenericfactory.h>

#include "kcmlayout.h"


typedef KGenericFactory<LayoutConfig> LayoutConfigFactory;
K_EXPORT_COMPONENT_FACTORY(keyboard_layout, LayoutConfigFactory("kcmlayout"))

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QWidget window;

//  KCMShellMultiDialog* dlg = new KCMShellMultiDialog(KPageDialog::Plain);
//  dlg->addModule(KService::Ptr ...);
//	KInstance* inst = new KInstance("kcmlayout");
//	new LayoutConfig(inst, &window);
	init_keyboard_layout();

	QStringList list;
	new LayoutConfig(&window, list);

    window.show();
    return app.exec();
}

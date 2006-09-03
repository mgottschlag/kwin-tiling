#include <QApplication>
#include <QWidget>

#include <kcmodule.h>
#include <kinstance.h>
#include <kgenericfactory.h>

#include <ksystemtrayicon.h>
#include <kwin.h>
#include <kdebug.h>

#include "kcmlayout.h"


typedef KGenericFactory<LayoutConfig> LayoutConfigFactory;
K_EXPORT_COMPONENT_FACTORY(keyboard_layout, LayoutConfigFactory("kcmlayout"))

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
	app.setApplicationName("kxkb.test");

    QWidget window;

//  KCMShellMultiDialog* dlg = new KCMShellMultiDialog(KPageDialog::Plain);
//  dlg->addModule(KService::Ptr ...);
//	KInstance* inst = new KInstance("kcmlayout");
//	new LayoutConfig(inst, &window);
	init_keyboard_layout();

	QStringList list;
	new LayoutConfig(&window, list);

//	KSystemTrayIcon icon(&window);
//	icon.setIcon(KSystemTrayIcon::loadIcon("kxkb"));
//	icon.show();

//KWin::setSystemTrayWindowFor( window.winId(), 0 );

    window.show();
    return app.exec();
}

#include <kglobal.h>
#include <klocale.h>
#include "widgetsettings.h"

extern "C" {
    KCModule *create_general(QWidget *parent, const char *name) {
      KGlobal::locale()->insertCatalogue(QString::fromLatin1("kcmwidgetsettings"));
      return new KWidgetSettingsModule(parent, name);
    }
}

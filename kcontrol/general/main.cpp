#include <kglobal.h>
#include <klocale.h>
#include <kgenericfactory.h>

#include "widgetsettings.h"

extern "C" {
    KCModule *create_widgetsettings(QWidget *parent, const char *name) {
      KGlobal::locale()->insertCatalogue(QString::fromLatin1("kcmwidgetsettings"));
      return new KWidgetSettingsModule(parent, name);
    }
}

/*
typedef KGenericFactory<KWidgetSettingsModule, QWidget> GeneralFactory;
K_EXPORT_COMPONENT_FACTORY( libkcm_widgetsettings, GeneralFactory );
*/
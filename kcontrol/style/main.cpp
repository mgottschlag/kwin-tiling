#include <kglobal.h>
#include <klocale.h>
#include <kgenericfactory.h>

#include "kcmstyle.h"

extern "C" {
    KCModule *create_style(QWidget *parent, const char *name) {
      KGlobal::locale()->insertCatalogue(QString::fromLatin1("kcmstyle"));
      return new KCMStyle(parent, name);
    }
}

/*
typedef KGenericFactory<KWidgetSettingsModule, QWidget> GeneralFactory;
K_EXPORT_COMPONENT_FACTORY( libkcm_kcmstyle, GeneralFactory );
*/

#include <kglobal.h>
#include <klocale.h>
#include <kgenericfactory.h>

#include "kcmstyle.h"

extern "C" {
    KCModule *create_style(QWidget *parent, const char *) {
      return new KCMStyle(parent, "kcmstyle");
    }
}

/*
typedef KGenericFactory<KWidgetSettingsModule, QWidget> GeneralFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_style, GeneralFactory )
*/

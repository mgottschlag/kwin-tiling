#include <kglobal.h>
#include <klocale.h>
#include <kgenericfactory.h>

#include "widgetsettings.h"

extern "C" {
    KCModule *create_widgetsettings(QWidget *parent, const char *name) {
	return new KWidgetSettingsModule(parent, "kcmwidgetsettings");
    }
}

/*
typedef KGenericFactory<KWidgetSettingsModule, QWidget> GeneralFactory;
K_EXPORT_COMPONENT_FACTORY( libkcm_widgetsettings, GeneralFactory );
*/

#include <kglobal.h>
#include <klocale.h>
#include "general.h"

extern "C" {
    KCModule *create_general(QWidget *parent, const char *name) {
      KGlobal::locale()->insertCatalogue(QString::fromLatin1("kcmgeneral"));
      return new KGeneralModule(parent, name);
    }
}

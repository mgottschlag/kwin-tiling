#ifndef classSystemDSN_included
#define classSystemDSN_included

#include <qwidget.h>
#include <qpushbt.h>

#include "classDSNList.h"

class classSystemDSN : public QWidget
{
    Q_OBJECT

public:

    classSystemDSN( QWidget* parent = NULL, const char* name = NULL );
    ~classSystemDSN();

protected:
    QPushButton* pbAdd;
    QPushButton* pbRemove;
    QPushButton* pbConfigure;
    classDSNList* pDSNList;

};
#endif

#ifndef classUserDSN_included
#define classUserDSN_included

#include <qwidget.h>
#include <qpushbt.h>

#include "classDSNList.h"

class classUserDSN : public QWidget
{
    Q_OBJECT

public:

    classUserDSN( QWidget* parent = NULL, const char* name = NULL );
    ~classUserDSN();

protected:
    QPushButton* pbAdd;
    QPushButton* pbRemove;
    QPushButton* pbConfigure;
    classDSNList* pDSNList;

};
#endif

#ifndef classAbout_included
#define classAbout_included

#include <qwidget.h>
#include <qmessagebox.h>

class classAbout : public QWidget
{
    Q_OBJECT

public:

    classAbout( QWidget* parent = NULL, const char* name = NULL );
    virtual ~classAbout();

protected slots:
    void pbODBCConfig_Clicked();
    void pbODBC_Clicked();
    void pbDatabase_Clicked();
    void pbDriverManager_Clicked();
    void pbDriver_Clicked();
    void pbODBCDrivers_Clicked();
    void pbCredits_Clicked();
    void pbApplication_Clicked();
};
#endif

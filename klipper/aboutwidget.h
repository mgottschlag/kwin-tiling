#ifndef ABOUTWIDGET_H
#define ABOUTWIDGET_H

class QPushButton;

#include <qvbox.h>

class AboutWidget : public QVBox
{
    Q_OBJECT
public:
    AboutWidget( QWidget *parent, const char *name );
    ~AboutWidget();
        
private slots:
    void slotBugreport();
 
private:
    QPushButton *m_bugReport;
};


#endif // ABOUTWIDGET_H

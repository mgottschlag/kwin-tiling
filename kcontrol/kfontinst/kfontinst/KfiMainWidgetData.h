/****************************************************************************
** Form interface generated from reading ui file 'KfiMainWidget.ui'
**
** Created: Sun Jul 1 16:18:01 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef CKFIMAINWIDGETDATA_H
#define CKFIMAINWIDGETDATA_H

#include <qvariant.h>
#include <qwidget.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class CAboutWidget;
class CFontsWidget;
class CSettingsWidget;
class CXftConfigSettingsWidget;
class QTabWidget;

class CKfiMainWidgetData : public QWidget
{ 
    Q_OBJECT

public:
    CKfiMainWidgetData( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~CKfiMainWidgetData();

    QTabWidget* itsTab;
    QWidget* itsFontsTab;
    CFontsWidget* itsFonts;
    QWidget* itsAATab;
    CXftConfigSettingsWidget* itsAA;
    QWidget* itsSettingsTab;
    CSettingsWidget* itsSettings;
    QWidget* itsAboutTab;
    CAboutWidget* CAboutWidget1;

public slots:
    virtual void tabChanged(QWidget *);

protected:
    QGridLayout* CKfiMainWidgetDataLayout;
    QGridLayout* itsFontsTabLayout;
    QGridLayout* itsAATabLayout;
    QGridLayout* itsSettingsTabLayout;
    QGridLayout* itsAboutTabLayout;
};

#endif // CKFIMAINWIDGETDATA_H

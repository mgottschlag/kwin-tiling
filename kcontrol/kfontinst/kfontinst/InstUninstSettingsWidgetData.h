/****************************************************************************
** Form interface generated from reading ui file 'InstUninstSettingsWidget.ui'
**
** Created: Wed Oct 24 21:21:38 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef CINSTUNINSTSETTINGSWIDGETDATA_H
#define CINSTUNINSTSETTINGSWIDGETDATA_H

#include <qvariant.h>
#include <qwidget.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QButtonGroup;
class QCheckBox;
class QGroupBox;
class QLabel;
class QPushButton;

class CInstUninstSettingsWidgetData : public QWidget
{ 
    Q_OBJECT

public:
    CInstUninstSettingsWidgetData( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~CInstUninstSettingsWidgetData();

    QGroupBox* GroupBox5;
    QCheckBox* itsFixTtfPsNamesUponInstall;
    QButtonGroup* ButtonGroup1;
    QPushButton* itsUninstallDirButton;
    QLabel* itsUninstallDirText;


public slots:
    virtual void fixTtfNamesSelected(bool);
    virtual void moveToSelected(bool);
    virtual void processAfmsSelected(bool);
    virtual void uninstallDirButtonPressed();

protected:
    QGridLayout* CInstUninstSettingsWidgetDataLayout;
    QGridLayout* GroupBox5Layout;
    QGridLayout* ButtonGroup1Layout;
};

#endif // CINSTUNINSTSETTINGSWIDGETDATA_H

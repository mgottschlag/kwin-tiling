/****************************************************************************
** Form interface generated from reading ui file 'DisplaySettingsWidget.ui'
**
** Created: Wed Nov 21 00:35:18 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef CDISPLAYSETTINGSWIDGETDATA_H
#define CDISPLAYSETTINGSWIDGETDATA_H

#include <qvariant.h>
#include <qwidget.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QButtonGroup;
class QCheckBox;
class QLineEdit;
class QRadioButton;

class CDisplaySettingsWidgetData : public QWidget
{ 
    Q_OBJECT

public:
    CDisplaySettingsWidgetData( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~CDisplaySettingsWidgetData();

    QButtonGroup* ButtonGroup5_3;
    QRadioButton* itsAdvanced;
    QRadioButton* itsBasic;
    QCheckBox* itsCustomCheck;
    QLineEdit* itsCustomText;
    QButtonGroup* ButtonGroup2;
    QRadioButton* itsLeftAndRight;
    QRadioButton* itsTopAndBottom;


public slots:
    virtual void advancedSelected(bool);
    virtual void customStrChecked(bool);
    virtual void textChanged(const QString &);
    virtual void topAndBottomSelected(bool);

protected:
    QGridLayout* CDisplaySettingsWidgetDataLayout;
    QGridLayout* ButtonGroup5_3Layout;
    QGridLayout* ButtonGroup2Layout;
};

#endif // CDISPLAYSETTINGSWIDGETDATA_H

/****************************************************************************
** Form interface generated from reading ui file 'XftConfigSettingsWidget.ui'
**
** Created: Wed Oct 24 21:21:42 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef CXFTCONFIGSETTINGSWIDGETDATA_H
#define CXFTCONFIGSETTINGSWIDGETDATA_H

#include <qvariant.h>
#include <qwidget.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QCheckBox;
class QLabel;
class QLineEdit;
class QPushButton;

class CXftConfigSettingsWidgetData : public QWidget
{ 
    Q_OBJECT

public:
    CXftConfigSettingsWidgetData( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~CXftConfigSettingsWidgetData();

    QCheckBox* itsUseSubPixelHintingCheck;
    QLabel* TextLabel2_2_2;
    QLineEdit* itsFromText;
    QLabel* TextLabel2;
    QLineEdit* itsToText;
    QLabel* TextLabel3;
    QLabel* itsConfigFileText;
    QPushButton* itsConfigFileButton;
    QPushButton* itsAdvancedButton;
    QCheckBox* itsExcludeRangeCheck;
    QPushButton* itsSaveButton;


public slots:
    virtual void advancedButtonPressed();
    virtual void excludeRangeChecked(bool);
    virtual void fileButtonPressed();
    virtual void fromChanged(const QString &);
    virtual void saveButtonPressed();
    virtual void toChanged(const QString &);
    virtual void useSubPixelChecked(bool);

protected:
    QGridLayout* CXftConfigSettingsWidgetDataLayout;
};

#endif // CXFTCONFIGSETTINGSWIDGETDATA_H

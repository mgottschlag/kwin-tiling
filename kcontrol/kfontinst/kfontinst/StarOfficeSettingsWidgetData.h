/****************************************************************************
** Form interface generated from reading ui file 'StarOfficeSettingsWidget.ui'
**
** Created: Fri Sep 7 00:50:45 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef CSTAROFFICESETTINGSWIDGETDATA_H
#define CSTAROFFICESETTINGSWIDGETDATA_H

#include <qvariant.h>
#include <qwidget.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QCheckBox;
class QComboBox;
class QLabel;
class QPushButton;

class CStarOfficeSettingsWidgetData : public QWidget
{ 
    Q_OBJECT

public:
    CStarOfficeSettingsWidgetData( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~CStarOfficeSettingsWidgetData();

    QCheckBox* itsCheck;
    QLabel* itsDirText;
    QLabel* TextLabel2_2_2;
    QLabel* TextLabel1_3_3;
    QComboBox* itsPpdCombo;
    QPushButton* itsDirButton;
    QLabel* itsNote;

public slots:
    virtual void configureSelected(bool);
    virtual void dirButtonPressed();
    virtual void ppdSelected(const QString &);

protected:
    QGridLayout* CStarOfficeSettingsWidgetDataLayout;
};

#endif // CSTAROFFICESETTINGSWIDGETDATA_H

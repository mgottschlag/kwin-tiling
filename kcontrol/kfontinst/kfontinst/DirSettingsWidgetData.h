/****************************************************************************
** Form interface generated from reading ui file 'DirSettingsWidget.ui'
**
** Created: Wed Nov 21 00:35:17 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef CDIRSETTINGSWIDGETDATA_H
#define CDIRSETTINGSWIDGETDATA_H

#include <qvariant.h>
#include <qwidget.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class QPushButton;

class CDirSettingsWidgetData : public QWidget
{ 
    Q_OBJECT

public:
    CDirSettingsWidgetData( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~CDirSettingsWidgetData();

    QGroupBox* GroupBox4_3;
    QGroupBox* GroupBox3;
    QComboBox* itsTTCombo;
    QComboBox* itsT1Combo;
    QLabel* TextLabel1_2_2_2;
    QLabel* TextLabel1_2_3;
    QLabel* itsFontsDirText;
    QPushButton* itsFontsDirButton;
    QLabel* TextLabel1_3;
    QPushButton* itsEncodingsDirButton;
    QLabel* itsEncodingsDirText;
    QLabel* TextLabel1_3_2;
    QLabel* itsXConfigFileText;
    QLabel* TextLabel2_2_2;
    QPushButton* itsXConfigFileButton;
    QCheckBox* itsGhostscriptCheck;
    QLabel* itsGhostscriptFileText;
    QPushButton* itsGhostscriptFileButton;
    QLabel* itsCupsDirText;
    QCheckBox* itsCupsCheck;
    QPushButton* itsCupsDirButton;


public slots:
    virtual void cupsButtonPressed();
    virtual void encodingsDirButtonPressed();
    virtual void ghostscriptChecked(bool);
    virtual void gsFontmapButtonPressed();
    virtual void cupsChecked(bool);
    virtual void t1SubDir(const QString &);
    virtual void ttSubDir(const QString &);
    virtual void xConfigButtonPressed();
    virtual void xDirButtonPressed();

protected:
    QGridLayout* CDirSettingsWidgetDataLayout;
    QGridLayout* GroupBox4_3Layout;
    QGridLayout* GroupBox3Layout;
};

#endif // CDIRSETTINGSWIDGETDATA_H

/****************************************************************************
** Form interface generated from reading ui file 'SettingsWizard.ui'
**
** Created: Sun Sep 30 12:59:39 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef CSETTINGSWIZARDDATA_H
#define CSETTINGSWIZARDDATA_H

#include <qvariant.h>
#include <kwizard.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class CDirSettingsWidget;
class CStarOfficeSettingsWidget;
class CXftConfigSettingsWidget;
class QLabel;
class QWidget;

class CSettingsWizardData : public KWizard
{ 
    Q_OBJECT

public:
    CSettingsWizardData( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~CSettingsWizardData();

    QWidget* itsIntroPage;
    QLabel* itsMainText;
    QLabel* TextLabel1;
    QLabel* itsNonRootText;
    QWidget* itsDirsAndFilesPage;
    CDirSettingsWidget* itsDirsAndFilesWidget;
    QLabel* TextLabel2;
    QWidget* itsAAPage;
    QLabel* TextLabel1_2;
    CXftConfigSettingsWidget* XftWizard;
    QWidget* itsStarOfficePage;
    CStarOfficeSettingsWidget* itsSOWidget;
    QLabel* TextLabel1_2_2;
    QWidget* itsCompletePage;
    QLabel* TextLabel1_3;


protected:
    QGridLayout* itsIntroPageLayout;
    QGridLayout* itsDirsAndFilesPageLayout;
    QGridLayout* itsAAPageLayout;
    QGridLayout* itsStarOfficePageLayout;
    QGridLayout* itsCompletePageLayout;
};

#endif // CSETTINGSWIZARDDATA_H

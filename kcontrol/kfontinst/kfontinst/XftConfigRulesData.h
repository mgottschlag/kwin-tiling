/****************************************************************************
** Form interface generated from reading ui file 'XftConfigRules.ui'
**
** Created: Wed Jul 4 21:45:25 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef FORM1_H
#define FORM1_H

#include <qvariant.h>
#include <kdialog.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class CXftConfigIncludesWidget;
class QGroupBox;
class QListView;
class QListViewItem;
class QPushButton;
class QTabWidget;
class QWidget;

class CXftConfigRulesData : public KDialog
{ 
    Q_OBJECT

public:
    CXftConfigRulesData( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~CXftConfigRulesData();

    QPushButton* itsCancelButton;
    QPushButton* itsOkButton;
    QTabWidget* TabWidget2;
    QWidget* itsRulesTab;
    QGroupBox* GroupBox1;
    QListView* itsList;
    QPushButton* itsRemoveButton;
    QPushButton* itsAddButton;
    QPushButton* itsEditButton;
    QWidget* itsIncludesTab;
    CXftConfigIncludesWidget* itsIncludes;
    CXftConfigIncludesWidget* itsIncludeIfs;

public slots:
    virtual void addButtonPressed();
    virtual void itemSelected(QListViewItem *);
    virtual void editButtonPressed();
    virtual void removeButtonPressed();

protected:
    QGridLayout* Form1Layout;
    QGridLayout* itsRulesTabLayout;
    QGridLayout* GroupBox1Layout;
    QGridLayout* itsIncludesTabLayout;
};

#endif // FORM1_H

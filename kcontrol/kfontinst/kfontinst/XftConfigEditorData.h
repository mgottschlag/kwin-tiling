/****************************************************************************
** Form interface generated from reading ui file 'XftConfigEditor.ui'
**
** Created: Mon Sep 17 00:15:51 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef CXFTCONFIGEDITORDATA_H
#define CXFTCONFIGEDITORDATA_H

#include <qvariant.h>
#include <kdialog.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QComboBox;
class QFrame;
class QGroupBox;
class QLineEdit;
class QListBox;
class QListBoxItem;
class QPushButton;

class CXftConfigEditorData : public KDialog
{ 
    Q_OBJECT

public:
    CXftConfigEditorData( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~CXftConfigEditorData();

    QGroupBox* GroupBox10;
    QFrame* Frame10;
    QPushButton* itsMatchAddButton;
    QPushButton* itsMatchRemoveButton;
    QListBox* itsMatchList;
    QFrame* Frame5;
    QComboBox* itsMatchQualCombo;
    QComboBox* itsMatchFieldNameCombo;
    QComboBox* itsMatchCompareCombo;
    QFrame* Frame7;
    QLineEdit* itsMatchString;
    QComboBox* itsMatchCombo;
    QLineEdit* itsMatchOther;
    QPushButton* PushButton15;
    QPushButton* itsOkButton;
    QGroupBox* GroupBox10_2;
    QFrame* Frame5_2;
    QComboBox* itsEditFieldNameCombo;
    QComboBox* itsEditAssignCombo;
    QFrame* Frame7_2;
    QLineEdit* itsEditString;
    QComboBox* itsEditCombo;
    QLineEdit* itsEditOther;

public slots:
    virtual void addMatch();
    virtual void editCombo(const QString &);
    virtual void editFieldSelected(const QString &);
    virtual void matchCombo(const QString &);
    virtual void matchFieldSelected(const QString &);
    virtual void matchSelected(QListBoxItem *);
    virtual void removeMatch();

protected:
    QGridLayout* CXftConfigEditorDataLayout;
    QGridLayout* GroupBox10Layout;
    QGridLayout* Frame10Layout;
    QGridLayout* Frame5Layout;
    QGridLayout* Frame7Layout;
    QGridLayout* GroupBox10_2Layout;
    QGridLayout* Frame5_2Layout;
    QGridLayout* Frame7_2Layout;
};

#endif // CXFTCONFIGEDITORDATA_H

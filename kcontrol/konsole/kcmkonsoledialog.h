/****************************************************************************
** Form interface generated from reading ui file './kcmkonsoledialog.ui'
**
** Created: Fri Apr 20 16:07:58 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef KCMKONSOLEDIALOG_H
#define KCMKONSOLEDIALOG_H

#include <qvariant.h>
#include <qwidget.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QButtonGroup;
class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QTabWidget;
class SchemaEditor;

class KCMKonsoleDialog : public QWidget
{ 
    Q_OBJECT

public:
    KCMKonsoleDialog( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~KCMKonsoleDialog();

    QTabWidget* TabWidget2;
    QWidget* tab;
    QButtonGroup* ButtonGroup1;
    QCheckBox* showMenuBarCB;
    QCheckBox* showFrameCB;
    QLabel* TextLabel1;
    QComboBox* scrollBarCO;
    QCheckBox* showToolBarCB;
    QGroupBox* GroupBox1;
    QLabel* TextLabel5;
    QComboBox* fontCO;
    QPushButton* fontPB;
    QCheckBox* fullScreenCB;
    QGroupBox* GroupBox2;
    QCheckBox* historyCB;
    QCheckBox* warnCB;
    QLabel* TextLabel8;
    QComboBox* codecCO;
    QLineEdit* terminalLE;
    QLabel* TextLabel1_2;
    QCheckBox* terminalCB;
    QWidget* tab_2;
    SchemaEditor* SchemaEditor1;
    QWidget* tab_3;
    QLabel* TextLabel9;
    QWidget* tab_4;
    QLabel* TextLabel10;

protected:
    QGridLayout* KCMKonsoleDialogLayout;
    QGridLayout* tabLayout;
    QGridLayout* ButtonGroup1Layout;
    QGridLayout* GroupBox1Layout;
    QGridLayout* GroupBox2Layout;
    QGridLayout* tabLayout_2;
    QGridLayout* tabLayout_3;
    QGridLayout* tabLayout_4;
};

#endif // KCMKONSOLEDIALOG_H

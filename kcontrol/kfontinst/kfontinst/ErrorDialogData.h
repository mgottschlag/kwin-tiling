/****************************************************************************
** Form interface generated from reading ui file 'ErrorDialog.ui'
**
** Created: Wed Sep 26 18:43:55 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef ERRORDIALOG_H
#define ERRORDIALOG_H

#include <qvariant.h>
#include <kdialog.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QGroupBox;
class QListView;
class QListViewItem;
class QPushButton;

class CErrorDialogData : public KDialog
{ 
    Q_OBJECT

public:
    CErrorDialogData( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~CErrorDialogData();

    QPushButton* buttonOk;
    QGroupBox* itsGroupBox;
    QListView* itsListView;


protected:
    QGridLayout* ErrorDialogLayout;
    QHBoxLayout* Layout1;
    QGridLayout* itsGroupBoxLayout;
};

#endif // ERRORDIALOG_H

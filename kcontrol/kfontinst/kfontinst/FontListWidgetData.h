/****************************************************************************
** Form interface generated from reading ui file 'FontListWidget.ui'
**
** Created: Mon Sep 10 00:09:00 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef CFONTLISTWIDGETDATA_H
#define CFONTLISTWIDGETDATA_H

#include <qvariant.h>
#include <qwidget.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QGroupBox;
class QListView;
class QListViewItem;
class QPushButton;

class CFontListWidgetData : public QWidget
{ 
    Q_OBJECT

public:
    CFontListWidgetData( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~CFontListWidgetData();

    QGroupBox* itsBox;
    QListView* itsList;
    QPushButton* itsButton2;
    QPushButton* itsButton1;

public slots:
    virtual void selectionChanged();

protected:
    QGridLayout* CFontListWidgetDataLayout;
    QGridLayout* itsBoxLayout;
};

#endif // CFONTLISTWIDGETDATA_H

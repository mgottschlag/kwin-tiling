/****************************************************************************
** Form interface generated from reading ui file 'XftConfigIncludesWidget.ui'
**
** Created: Wed Oct 24 21:21:41 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef CXFTCONFIGINCLUDESWIDGETDATA_H
#define CXFTCONFIGINCLUDESWIDGETDATA_H

#include <qvariant.h>
#include <qwidget.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QGroupBox;
class QListBox;
class QListBoxItem;
class QPushButton;

class CXftConfigIncludesWidgetData : public QWidget
{ 
    Q_OBJECT

public:
    CXftConfigIncludesWidgetData( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~CXftConfigIncludesWidgetData();

    QGroupBox* itsGroupBox;
    QListBox* itsList;
    QPushButton* itsRemoveButton;
    QPushButton* itsEditButton;
    QPushButton* itsAddButton;


public slots:
    virtual void addPressed();
    virtual void itemSelected(QListBoxItem *);
    virtual void editPressed();
    virtual void removePressed();

protected:
    QGridLayout* CXftConfigIncludesWidgetDataLayout;
    QGridLayout* itsGroupBoxLayout;
};

#endif // CXFTCONFIGINCLUDESWIDGETDATA_H

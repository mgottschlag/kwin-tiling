/****************************************************************************
** Form interface generated from reading ui file 'FontsWidget.ui'
**
** Created: Mon Nov 5 00:26:22 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef CFONTSWIDGETDATA_H
#define CFONTSWIDGETDATA_H

#include <qvariant.h>
#include <qwidget.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class CFontPreview;
class QFrame;
class QGroupBox;
class KProgress;
class QSplitter;

class CFontsWidgetData : public QWidget
{ 
    Q_OBJECT

public:
    CFontsWidgetData( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~CFontsWidgetData();

    QGroupBox* itsBox;
    QFrame* itsBackground;
    KProgress* itsProgress;
    CFontPreview* itsLabel;
    QSplitter* itsSplitter;


public slots:
    virtual void preview(const QString &);

protected:
    QGridLayout* CFontsWidgetDataLayout;
    QGridLayout* itsBoxLayout;
    QGridLayout* itsBackgroundLayout;
};

#endif // CFONTSWIDGETDATA_H

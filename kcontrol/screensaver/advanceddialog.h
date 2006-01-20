#ifndef ADVANCEDDIALOG_H
#define ADVANCEDDIALOG_H

#include <kdialogbase.h>
#include <qwidget.h>
#include <kconfig.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qwhatsthis.h>
#include <qgroupbox.h>
#include <qobject.h>
#include <qcheckbox.h>
#include <qslider.h>

#include "advanceddialogimpl.h"

class AdvancedDialog : public AdvancedDialogImpl
{
public:
	AdvancedDialog(QWidget *parent = 0, const char *name = 0);
	~AdvancedDialog();
	void setMode(QComboBox *box, int i);
	int mode(QComboBox *box);
};

/* =================================================================================================== */

class KScreenSaverAdvancedDialog : public KDialogBase
{
    Q_OBJECT
public:
    KScreenSaverAdvancedDialog(QWidget *parent, const char* name = 0);
      
public Q_SLOTS:
    void slotOk();
         
protected Q_SLOTS:
    void slotPriorityChanged(int val);
    void slotChangeBottomRightCorner(int);
    void slotChangeBottomLeftCorner(int);
    void slotChangeTopRightCorner(int);
    void slotChangeTopLeftCorner(int);
                        
private:
    void readSettings();
                     
    QCheckBox *m_topLeftCorner;
    QCheckBox *m_bottomLeftCorner;
    QCheckBox *m_topRightCorner;
    QCheckBox *m_bottomRightCorner;
    QSlider   *mPrioritySlider;
                                          
    bool mChanged;
    int  mPriority;
    AdvancedDialog *dialog;

};


#endif


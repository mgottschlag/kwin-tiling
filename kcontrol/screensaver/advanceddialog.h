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

class KScreenSaverAdvancedDialog : public KDialogBase
{
    Q_OBJECT
public:
    KScreenSaverAdvancedDialog(QWidget *parent, const char* name = 0);
      
public slots:
    void slotOk();
         
protected slots:
    void slotPriorityChanged( int val );
    void slotChangeBottomRightCorner( bool );
    void slotChangeBottomLeftCorner( bool );
    void slotChangeTopRightCorner( bool );
    void slotChangeTopLeftCorner( bool );
                        
private:
    void readSettings();
                     
    QCheckBox *m_topLeftCorner;
    QCheckBox *m_bottomLeftCorner;
    QCheckBox *m_topRightCorner;
    QCheckBox *m_bottomRightCorner;
    QSlider   *mPrioritySlider;
                                          
    bool mTopLeftCorner;
    bool mTopRightCorner;
    bool mBottomLeftCorner;
    bool mBottomRightCorner;
    bool mChanged;
    int  mPriority;
};

#endif

#ifndef BACKGND_H
#define BACKGND_H

#include <qobject.h>
#include <qstring.h>
#include <qcolor.h>
#include <qmap.h>
#include <qevent.h>
#include <qwidget.h>
#include <qptrvector.h>

#include "backgndbase.h"

class KBackgroundRenderer;
class KConfig;
class KStandardDirs;


class KBGMonitor : public QWidget
{
    Q_OBJECT
public:

    KBGMonitor(QWidget *parent, const char *name=0L);

signals:
    void imageDropped(QString);

protected:
    virtual void dropEvent(QDropEvent *);
    virtual void dragEnterEvent(QDragEnterEvent *);
};


class Backgnd : public BackgndBase
{ 
    Q_OBJECT

public:
    Backgnd( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~Backgnd();

    void setWidgets();
    void load();
    void save();
    void defaults();

protected slots:
    void slotColorBlendMode(int);
    void slotBGMode(int);

    void slotColor1(const QColor &);
    void slotColor2(const QColor &);
    void slotPreviewDone(int);
    void slotWPBlendBalance(int value);
    void slotWPBlendMode(int mode);
    void slotWPMode(int);
    void slotWPChangeInterval(int);
    void slotWPAdd();
    void slotImageDropped(QString);
    void slotWPDelete();
    void slotWPSet(int);
    void slotImageOrder(int);
    void slotProgramSetup();
    void slotPatternEdit();
    void slotDesktop(int);
    void slotAdvanced();

signals:
    void changed(bool);
    
private:
    void init();
    void adjustMultiWP();
    
    int m_Desk, m_Max;
    int m_oldMode;

    KGlobalBackgroundSettings *m_pGlobals;
    
    QPtrVector<KBackgroundRenderer> m_Renderer;
    KBGMonitor *m_pMonitor;
    
    KConfig *m_pConfig;
    KStandardDirs *m_pDirs;



};

#endif // BACKGND_H

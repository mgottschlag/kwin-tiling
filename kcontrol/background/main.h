/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kcmdisplay.
 * Copyright (C) 1999 Geert Jansen <g.t.jansen@stud.tue.nl>
 * 
 * You can Freely distribute this program under the GNU General Public
 * License. See the file "COPYING" for the exact licensing terms.
 */

#ifndef __Bgnd_h_Included__
#define __Bgnd_h_Included__

#include <qobject.h>
#include <qstring.h>
#include <qcolor.h>
#include <qmap.h>
#include <qevent.h>
#include <qwidget.h>
#include <qptrvector.h>

#include <kcmodule.h>
#include <bgdefaults.h>
#include <backgnd.h>

class QCheckBox;
class QListBox;
class QComboBox;
class QStringList;
class QHButtonGroup;
class QPalette;
class QLabel;
class QSlider;
class QTabWidget;
class QSpinBox;

class KColorButton;
class KBackgroundRenderer;
class KGlobalBackgroundSettings;
class KConfig;
class KStandardDirs;
class KAboutData;


/**
 * This class handles drops on the preview monitor.
 */
/*  Moved to backgnd.h
class KBGMonitor : public QWidget
{
    Q_OBJECT
public:

    KBGMonitor(QWidget *parent, const char *name=0L);

signals:
    void imageDropped(QString);

protected:backgndbase.h -o backgndbase.moc
    virtual void dropEvent(QDropEvent *);
    virtual void dragEnterEvent(QDragEnterEvent *);
};
*/

/**
 * The Desktop/Background tab in kcontrol.
 */
class KBackground: public KCModule
{
    Q_OBJECT

public:
    KBackground(QWidget *parent, const char *name, const QStringList &);
    ~KBackground();
    
    virtual void load();
    virtual void save();
    virtual void defaults();

    QString quickHelp() const;
    const KAboutData* aboutData() const;

protected slots:
    void slotChildChanged(bool);
    
signals:
    void changed(bool);

private:
//    void apply();
    
//    KConfig *m_pConfig;
//    KStandardDirs *m_pDirs;
    Backgnd       *m_base;
};


#endif // __Bgnd_h_Included__

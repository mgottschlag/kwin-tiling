/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kdesktop.
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

#include <kcontrol.h>

#include <bgdefaults.h>
#include "display.h"

class QCheckBox;
class QListBox;
class QComboBox;
class QStringList;
class QButtonGroup;
class QPalette;
class QLabel;

class KColorButton;
class KBackgroundRenderer;
class KGlobalBackgroundSettings;
class KConfig;
class KStandardDirs;


/**
 * This class handles drops on the preview monitor.
 */
class KBGMonitor : public QWidget
{
    Q_OBJECT
public:

    KBGMonitor(QWidget *parent) : QWidget(parent) {
	setAcceptDrops( true);
    };
	     
    // we don't want no steenking palette change
    virtual void setPalette(const QPalette &) {};

signals:
    void imageDropped(QString);

protected:
    virtual void dropEvent(QDropEvent *);
    virtual void dragEnterEvent(QDragEnterEvent *);
};



/**
 * A class to read/modify the global desktop background settings.
 */
class KGlobalBackgroundSettings
{
public:
    KGlobalBackgroundSettings();

    QString deskName(int desk);
    void setDeskName(int desk, QString name);

    int cacheSize() { return m_CacheSize; }
    void setCacheSize(int size);

    bool limitCache() { return m_bLimitCache; }
    void setLimitCache(bool limit);

    bool commonBackground() { return m_bCommon; }
    void setCommonBackground(bool common);

    bool dockPanel() { return m_bDock; }
    void setDockPanel(bool dock);

    bool exportBackground() {return m_bExport; }
    void setExportBackground(bool _export);

    void readSettings();
    void writeSettings();

private:
    bool dirty, m_bKWM;
    bool m_bCommon, m_bDock;
    bool m_bLimitCache, m_bExport;
    int m_CacheSize;
    QStringList m_Names;

    KConfig *m_pConfig;
};


/**
 * The "background" tab in kcmdisplay.
 */
class KBackground: public KDisplayModule
{
    Q_OBJECT

public:
    KBackground(QWidget *parent, Mode mode);

    virtual void loadSettings();
    virtual void applySettings();
    virtual void defaultSettings();

private slots:
    void slotSelectDesk(int desk);
    void slotCommonDesk(bool common);
    void slotBGMode(int mode);
    void slotBGSetup();
    void slotColor1(const QColor &);
    void slotColor2(const QColor &);
    void slotImageDropped(QString);
    void slotWPMode(int);
    void slotWallpaper(const QString &);
    void slotBrowseWallpaper();
    void slotSetupMulti();
    void slotPreviewDone(int);
    void slotMultiMode(bool);

private:
    void init();
    void apply();

    int m_Desk, m_Max;

    QListBox *m_pDeskList;
    QCheckBox *m_pCBCommon, *m_pCBMulti;
    QComboBox *m_pBackgroundBox, *m_pWallpaperBox;
    QComboBox *m_pArrangementBox;
    QPushButton *m_pBGSetupBut, *m_pMSetupBut;
    QPushButton *m_pBrowseBut;
    QMap<QString,int> m_Wallpaper;

    KBackgroundRenderer *m_Renderer[_maxDesktops];
    KGlobalBackgroundSettings *m_pGlobals;
    KColorButton *m_pColor1But, *m_pColor2But;
    KBGMonitor *m_pMonitor;

    KConfig *m_pConfig;
    KStandardDirs *m_pDirs;
};


#endif // __Bgnd_h_Included__

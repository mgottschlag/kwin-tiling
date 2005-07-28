/* vi: ts=8 sts=4 sw=4
 *
 * This file is part of the KDE project, module kdesktop.
 * Copyright (C) 1999 Geert Jansen <g.t.jansen@stud.tue.nl>
 *
 * You can Freely distribute this program under the GNU Library General
 * Public License. See the file "COPYING.LIB" for the exact licensing terms.
 */

#ifndef BGRender_h_Included
#define BGRender_h_Included

#include <qobject.h>
//Added by qt3to4:
#include <QPixmap>

#include "bgsettings.h"

class QSize;
class QRect;
class QString;
class QImage;
class QPixmap;
class QTimer;

class KConfig;
class KProcess;
class KTempFile;
class KShellProcess;
class KStandardDirs;

/**
 * This class renders a desktop background to a QImage. The operation is
 * asynchronous: connect to the signal imageDone() to find out when the
 * rendering is finished. It also has support for preview images, like
 * the monitor in kcmdisplay.
 */
class KBackgroundRenderer:
	public QObject,
	public KBackgroundSettings
{
    Q_OBJECT

public:
    KBackgroundRenderer(int desk, KConfig *config=0);
    ~KBackgroundRenderer();
    
    void load(int desk, bool reparseConfig=true);

    void setPreview(const QSize &size);
    
    QPixmap *pixmap();
    QImage *image();
    bool isActive() { return m_State & Rendering; }
    void cleanup();

public slots:
    void start(bool enableBusyCursor = false);
    void stop();
    void desktopResized();

signals:
    void imageDone(int desk);
    void programFailure(int desk, int exitstatus); //Guaranteed either programFailure or 
    void programSuccess(int desk);                //programSuccess is emitted after imageDone

private slots:
    void slotBackgroundDone(KProcess *);
    void render();
    void done();

private:
    enum { Error, Wait, WaitUpdate, Done };
    enum { Rendering = 1, BackgroundStarted = 2,
	BackgroundDone = 4, WallpaperStarted = 8,
	WallpaperDone = 0x10, AllDone = 0x20 };

    QString buildCommand();
    void createTempFile();
    void tile(QImage *dst, QRect rect, QImage *src);
    void blend(QImage *dst, QRect dr, QImage *src, QPoint soffs = QPoint(0, 0), int blendFactor=100);

    void wallpaperBlend( const QRect& d, QImage& wp, int ww, int wh );
    void fastWallpaperBlend( const QRect& d, QImage& wp, int ww, int wh );
    void fullWallpaperBlend( const QRect& d, QImage& wp, int ww, int wh );

    int doBackground(bool quit=false);
    int doWallpaper(bool quit=false);
    void setBusyCursor(bool isBusy);
    
    bool m_isBusyCursor;
    bool m_enableBusyCursor;
    bool m_bPreview;
    int m_State;

    KTempFile* m_Tempfile;
    QSize m_Size, m_rSize;
    QImage *m_pImage, *m_pBackground;
    QPixmap *m_pPixmap;
    QTimer *m_pTimer;

    KStandardDirs *m_pDirs;
    KShellProcess *m_pProc;
    
};

#endif // BGRender_h_Included


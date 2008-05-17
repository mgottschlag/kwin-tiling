/* vi: ts=8 sts=4 sw=4
 * kate: space-indent on; tab-width 8; indent-width 4; indent-mode cstyle;
 *
 * This file is part of the KDE project, module kdesktop.
 * Copyright (C) 1999 Geert Jansen <g.t.jansen@stud.tue.nl>
 *
 * You can Freely distribute this program under the GNU Library General
 * Public License. See the file "COPYING.LIB" for the exact licensing terms.
 */

#ifndef BGRender_h_Included
#define BGRender_h_Included

#include <QObject>
#include <QPixmap>
#include <QImage>
#include <KProcess>
#include <ksharedconfig.h>

#include "bgsettings.h"

class QSize;
class QRect;
class QString;
class QTimer;

class KTemporaryFile;
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
    KBackgroundRenderer(int screen, bool drawBackgroundPerScreen, const KSharedConfigPtr &config);
    ~KBackgroundRenderer();

    void load(int screen, bool drawBackgroundPerScreen, bool reparseConfig=true);

    void setPreview(const QSize &size);
    void setSize(const QSize &size);

    QPixmap pixmap();
    QImage image();
    bool isActive() { return m_State & Rendering; }
    void cleanup();
    void saveCacheFile();
    void enableTiling( bool enable ) { m_TilingEnabled = enable; }

public Q_SLOTS:
    void start(bool enableBusyCursor = false);
    void stop();
    void desktopResized();

Q_SIGNALS:
    void imageDone(int screen);
    void programFailure(int exitstatus); //Guaranteed either programFailure or
    void programSuccess();                //programSuccess is emitted after imageDone

private Q_SLOTS:
    void slotBackgroundDone(int exitCode, QProcess::ExitStatus exitStatus);
    void render();
    void done();

private:
    enum { Error, Wait, WaitUpdate, Done };
    enum { Rendering = 1, InitCheck = 2,
           BackgroundStarted = 4, BackgroundDone = 8,
           WallpaperStarted = 0x10, WallpaperDone = 0x20,
           AllDone = 0x40 };

    QString buildCommand();
    void createTempFile();
    void tile(QImage& dst, const QRect &rect, const QImage& src);
    void blend(QImage& dst, const QRect &dr, const QImage& src, const QPoint &soffs = QPoint(0, 0), int blendFactor=100);

    void wallpaperBlend();
    void fastWallpaperBlend();
    void fullWallpaperBlend();

    int doBackground(bool quit=false);
    int doWallpaper(bool quit=false);
    void setBusyCursor(bool isBusy);
    QString cacheFileName();
    bool useCacheFile() const;
    bool canTile() const;

    bool m_isBusyCursor;
    bool m_enableBusyCursor;
    bool m_bPreview;
    int m_State;
    bool m_Cached;
    bool m_TilingEnabled;

    KTemporaryFile* m_Tempfile;
    QSize m_Size, m_rSize;
    QRect m_WallpaperRect;
    QImage m_Image, m_Background, m_Wallpaper;
    QPixmap m_Pixmap;
    QTimer *m_pTimer;

    KStandardDirs *m_pDirs;
    KProcess *m_pProc;
};

/**
 * In xinerama mode, each screen is rendered separately by KBackgroundRenderer.
 * This class controls a set of renderers for a desktop, and coallates the
 * images. Usage is similar to KBackgroundRenderer: connect to the imageDone
 * signal.
 */
class KVirtualBGRenderer : public QObject
{
    Q_OBJECT
public:
    explicit KVirtualBGRenderer(const KSharedConfigPtr &config);
    ~KVirtualBGRenderer();

    KBackgroundRenderer * renderer(unsigned screen);
    unsigned numRenderers() const { return m_numRenderers; }

    QPixmap pixmap();

    void setPreview(const QSize & size);

    bool needProgramUpdate();
    void programUpdate();

    bool needWallpaperChange();
    void changeWallpaper();

    int hash();
    bool isActive();
    void setEnabled( bool enable );
    void desktopResized();

    void load(bool reparseConfig=true);
    void start();
    void stop();
    void cleanup();
    void saveCacheFile();
    void enableTiling( bool enable );

signals:
    void imageDone();

private slots:
    void screenDone(int screen);

private:
    QSize renderSize(int screen); // the size the renderer should be
    void initRenderers();

    KSharedConfigPtr m_pConfig;
    float m_scaleX;
    float m_scaleY;
    int m_numRenderers;
    bool m_bDrawBackgroundPerScreen;
    bool m_bCommonScreen;
    QSize m_size;

    QVector<bool> m_bFinished;
    QVector<KBackgroundRenderer *> m_renderer;
    QPixmap *m_pPixmap;
};


#endif // BGRender_h_Included


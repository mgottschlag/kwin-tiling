/* vi: ts=8 sts=4 sw=4
 *
 * This file is part of the KDE project, module kdesktop.
 * Copyright (C) 1999,2000 Geert Jansen <jansen@kde.org>
 *
 * You can Freely distribute this program under the GNU General Public
 * License. See the file "COPYING" for the exact licensing terms.
 */

#ifndef __BGManager_h_Included__
#define __BGManager_h_Included__

#include <kconfig.h>
#include <QString>
#include <QVector>

class KConfig;
class QTimer;
class QPixmap;
class KMenu;
class KWinModule;
class QPixmap;
class KVirtualBGRenderer;
class KPixmapServer;

/**
 * Internal struct for KBackgroundManager.
 */
struct KBackgroundCacheEntry
{
    int hash;
    int atime;
    int exp_from;
    QPixmap *pixmap;
};


/**
 * Background manager for KDE. This class is to be used in kdesktop. Usage is
 * very simple: instantiate this class once and the desktop background will
 * be painted automatically from now on.
 *
 * The background manager also has a DBus interface to remotely control its
 * operation. See kbackgroundadaptor.h.
 */

class KBackgroundManager : public QObject
{
    Q_OBJECT

public:
    KBackgroundManager(QWidget *desktop, KWinModule* kwinModule);
    ~KBackgroundManager();

    // Most of this is DBus-exported
    void configure();
    void setCommon(int);
    bool isCommon() { return m_bCommon; }
    void setExport(int);
    bool isExport() { return m_bExport; }
    void setCache(int, int);
    void setWallpaper(int desk, QString wallpaper, int mode);
    void setWallpaper(QString wallpaper, int mode);
    void setWallpaper(QString wallpaper);
    void changeWallpaper();
    QString currentWallpaper(int desk);
    void setColor(const QColor & c, bool isColorA = true);
    void setBackgroundEnabled(const bool enable);
    QStringList wallpaperList(int desk);
    QStringList wallpaperFiles(int desk);

    static const char* backgroundDBusObjectPath;

Q_SIGNALS:
    void initDone();
    // DBUS signal
    void backgroundChanged(int desk);

private Q_SLOTS:
    void slotTimeout();
    void slotImageDone(int desk);
    void slotChangeDesktop(int);
    void slotChangeNumberOfDesktops(int);
    void repaintBackground();
    void desktopResized();
    void clearRoot();
    void saveImages();

private:
    void applyCommon(bool common);
    void applyExport(bool _export);
    void applyCache(bool limit, int size);

    int realDesktop();
    int effectiveDesktop();
    int validateDesk(int desk);

    void renderBackground(int desk);
    void exportBackground(int pixmap, int desk);
    int pixmapSize(QPixmap *pm);
    int cacheSize();
    void removeCache(int desk);
    bool freeCache(int size);
    void addCache(QPixmap *pm, int hash, int desk);
    void setPixmap(QPixmap *pm, int hash, int desk);

    bool m_bExport, m_bCommon;
    bool m_bLimitCache, m_bInit;
    bool m_bBgInitDone;
    bool m_bEnabled;

    int m_CacheLimit;
    int m_Serial, m_Hash, m_Current;

    KSharedConfigPtr m_pConfig;
    QWidget *m_pDesktop;
    QTimer *m_pTimer;

    QVector<KVirtualBGRenderer*> m_Renderer;
    QVector<KBackgroundCacheEntry*> m_Cache;

    KWinModule *m_pKwinmodule;
    KPixmapServer *m_pPixmapServer;

    unsigned long m_xrootpmap;
};

#endif // __BGManager_h_Included__

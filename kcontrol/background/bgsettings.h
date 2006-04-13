/* vi: ts=8 sts=4 sw=4
 * kate: space-indent on; indent-width 4; indent-mode cstyle;
 *
 * This file is part of the KDE project, module kdesktop.
 * Copyright (C) 1999 Geert Jansen <g.t.jansen@stud.tue.nl>
 *
 * You can Freely distribute this program under the GNU Library General
 * Public License. See the file "COPYING.LIB" for the exact licensing terms.
 */

#ifndef __BGSettings_h_Included__
#define __BGSettings_h_Included__


#include <qstringlist.h>
#include <qcolor.h>
#include <QMap>
#include <QVector>

template <class QString, class T> class QMap;
class KStandardDirs;
class KSimpleConfig;
class KConfig;
class QString;
class QImage;

/**
 * A class to manipulate/read/write/list KDE desktop patterns.
 *
 * A pattern is a raster image. An entry for earch pattern is
 * stored as a .desktop file in $(datadir)/kdesktop/patterns.
 */
class KBackgroundPattern
{
public:
    KBackgroundPattern(QString name=QString());
    ~KBackgroundPattern();

    void copyConfig(const KBackgroundPattern*);

    QString name() const { return m_Name; }
    void load(QString name);

    void setComment(const QString &comment);
    QString comment() const {return m_Comment; }

    void setPattern(QString file);
    QString pattern() const { return m_Pattern; }

    void readSettings();
    void writeSettings();

    bool isAvailable();
    bool isGlobal()const { return m_bReadOnly; }
    bool remove();

    int hash();

    static QStringList list();

private:
    void init(bool force_rw=false);
    QString fingerprint();

    bool dirty, hashdirty;
    bool m_bReadOnly;
    int m_Hash;
    QString m_Name, m_Comment;
    QString m_Pattern, m_File;
    KStandardDirs *m_pDirs;
    KSimpleConfig *m_pConfig;
};


/**
 * A class to manipulate/read/write/list KDE desktop programs (a la xearth).
 *
 * A program is described by a string like:
 *
 *   a_program -xres %x -yres %y -outfile %f
 *
 * Possible escape sequences:
 *
 *   %x    Horizontal resolution in pixels.
 *   %y    Vertical resolution in pixels.
 *   %f    Filename to dump to.
 *
 * An entry for each program is stored as a .desktop file in
 * $(datadir)/kdesktop/programs.
 */
class KBackgroundProgram
{
public:
    KBackgroundProgram(QString name=QString());
    ~KBackgroundProgram();

    void copyConfig(const KBackgroundProgram*);

    QString name()const { return m_Name; }
    void load(const QString & name);

    void setComment(const QString &comment);
    QString comment()const { return m_Comment; }

    void setCommand(const QString &command);
    QString command()const { return m_Command; }

    void setPreviewCommand(const QString &command);
    QString previewCommand()const { return m_PreviewCommand; }

    void setRefresh(int refresh);
    int refresh()const { return m_Refresh; }

    void setExecutable(const QString &executable);
    QString executable()const { return m_Executable; }

    void readSettings();
    void writeSettings();

    void update();
    bool needUpdate();

    int hash();

    bool isAvailable();
    bool isGlobal()const { return m_bReadOnly; }
    bool remove();

    static QStringList list();

private:
    void init(bool force_rw=false);
    QString fingerprint();

    bool dirty, hashdirty;
    bool m_bReadOnly;
    int m_Refresh, m_Hash, m_LastChange;
    QString m_Name, m_Command;
    QString m_PreviewCommand, m_Comment;
    QString m_Executable, m_File;
    KStandardDirs *m_pDirs;
    KSimpleConfig *m_pConfig;
};


/**
 * KBackgroundSettings: A class to read/write/manipulate
 * KDE desktop settings.
 */
class KBackgroundSettings
    : public KBackgroundPattern,
      public KBackgroundProgram
{
public:
    /**
     * @param drawBackgroundPerScreen if false, then all screens (in xinerama
     * mode) will be treated as one big display, and the "screen" paramater
     * will be ignored.
     */
    KBackgroundSettings(int desk, int screen, bool drawBackgroundPerScreen, KConfig *config);
    ~KBackgroundSettings();

    void copyConfig(const KBackgroundSettings*);

    bool drawBackgroundPerScreen() const { return m_bDrawBackgroundPerScreen; }
    void setDrawBackgroundPerScreen(bool draw);

    int desk() const { return m_Desk; }
    int screen() const { return m_Screen; }
//     void load(int desk, int screen, bool drawBackgroundPerScreen, bool reparseConfig=true);
    void load(int desk, int screen, bool drawBackgroundPerScreen, bool reparseConfig);

    void setColorA(const QColor &color);
    QColor colorA() const { return m_ColorA; }
    void setColorB(const QColor &color);
    QColor colorB() const { return m_ColorB; }

    void setProgram(QString program);
    void setPatternName(QString pattern);

    enum BackgroundMode {
	Flat, Pattern, Program,
	HorizontalGradient, VerticalGradient, PyramidGradient,
	PipeCrossGradient, EllipticGradient, lastBackgroundMode
    };
    void setBackgroundMode(int mode);
    int backgroundMode() const { return m_BackgroundMode; }

    enum BlendMode {
        NoBlending, FlatBlending,
	HorizontalBlending, VerticalBlending, PyramidBlending,
	PipeCrossBlending, EllipticBlending,
	IntensityBlending, SaturateBlending, ContrastBlending,
	HueShiftBlending, lastBlendMode
    };
    void setBlendMode(int mode);
    int blendMode() const { return m_BlendMode; }

    void setReverseBlending(bool value);
    bool reverseBlending() const { return m_ReverseBlending; }

    void setBlendBalance(int value);
    int blendBalance() const { return m_BlendBalance; }

    void setWallpaper(QString name);
    QString wallpaper() const { return m_Wallpaper; }

    enum WallpaperMode {
	NoWallpaper, Centred, Tiled, CenterTiled, CentredMaxpect, TiledMaxpect,
	Scaled, CentredAutoFit, ScaleAndCrop, lastWallpaperMode
    };
    void setWallpaperMode(int mode);
    int wallpaperMode() const { return m_WallpaperMode; }

    void setWallpaperList(QStringList);
    QStringList wallpaperList() const;
    QStringList wallpaperFiles() const;

    void setWallpaperChangeInterval(int);
    int wallpaperChangeInterval() const { return m_Interval; }

    enum MultiMode {
	NoMulti, InOrder, Random, NoMultiRandom
    };
    void setMultiWallpaperMode(int mode);
    int multiWallpaperMode() const { return m_MultiMode; }

    enum MinOptDepth {
        AlwaysOpt, Opt16bpp, Opt15bpp, NeverOpt };

    void setMinOptimizationDepth( int mode );
    int minOptimizationDepth() const { return m_MinOptimizationDepth; }
    bool optimize() const;

    void setUseShm( bool use );
    bool useShm() const { return m_bShm; }

    void changeWallpaper(bool init=false);
    void updateWallpaperFiles();
    void randomizeWallpaperFiles();

    QString currentWallpaper();
    /**
     * @return true if the currentWallpaper has changed
     */
    bool discardCurrentWallpaper();
    int lastWallpaperChange() const { return m_LastChange; }
    bool needWallpaperChange();

    void readSettings(bool reparse=false);
    void writeSettings();
    QString configGroupName() const;

    int hash();
    QString fingerprint();

    void setEnabled( const bool enable );
    bool enabled() const { return m_bEnabled; }

private:
    void updateHash();

    bool dirty;
    bool hashdirty;
    int m_Screen, m_Desk, m_Hash;

    QColor m_ColorA, defColorA;
    QColor m_ColorB, defColorB;
    QString m_Wallpaper;
    QStringList m_WallpaperList, m_WallpaperFiles;

    int m_BackgroundMode, defBackgroundMode;
    int m_WallpaperMode, defWallpaperMode;
    int m_BlendMode, defBlendMode;
    int m_BlendBalance, defBlendBalance;
    bool m_ReverseBlending, defReverseBlending;
    int m_MinOptimizationDepth;
    bool m_bShm;
    bool m_bDrawBackgroundPerScreen;

    int m_MultiMode, defMultiMode;
    int m_Interval, m_LastChange;
    int m_CurrentWallpaper;

    KConfig *m_pConfig;
    KStandardDirs *m_pDirs;
    bool m_bDeleteConfig;
    bool m_bEnabled;

public:
    QMap<QString,int> m_BMMap;
    QMap<QString,int> m_WMMap;
    QMap<QString,int> m_MMMap;
    QMap<QString,int> m_BlMMap;
    char *m_BMRevMap[16];
    char *m_WMRevMap[16];
    char *m_MMRevMap[16];
    char *m_BlMRevMap[16];
};


/**
 * A class to read/modify the global desktop background settings.
 */
class KGlobalBackgroundSettings
{
public:
    KGlobalBackgroundSettings(KConfig *config);

    QString deskName(int desk);
    //void setDeskName(int desk, QString name);

    int cacheSize() { return m_CacheSize; }
    void setCacheSize(int size);

    bool drawBackgroundPerScreen(int desk) const;
    void setDrawBackgroundPerScreen(int desk, bool perScreen);

    bool limitCache() { return m_bLimitCache; }
    void setLimitCache(bool limit);

    bool commonScreenBackground() { return m_bCommonScreen; }
    void setCommonScreenBackground(bool common);

    bool commonDeskBackground() { return m_bCommonDesk; }
    void setCommonDeskBackground(bool common);

    bool dockPanel() { return m_bDock; }
    void setDockPanel(bool dock);

    bool exportBackground() {return m_bExport; }
    void setExportBackground(bool _export);

    void setTextColor(QColor _color);
    QColor textColor() const { return m_TextColor; }

    void setTextBackgroundColor(QColor _color);
    QColor textBackgroundColor() const { return m_TextBackgroundColor; }

    void setShadowEnabled(bool enabled);
    bool shadowEnabled() const { return m_shadowEnabled; }

    void setTextLines(int lines);
    int textLines() const { return m_textLines; }
    void setTextWidth(int width);
    int textWidth() const { return m_textWidth; }

    void readSettings();
    void writeSettings();

private:
    bool dirty;
    bool m_bCommonDesk;
    bool m_bCommonScreen;
    bool m_bDock;
    bool m_bLimitCache, m_bExport;
    int m_CacheSize;
    QStringList m_Names;

    QColor m_TextColor;
    QColor m_TextBackgroundColor;
    bool m_shadowEnabled;
    int m_textLines;
    int m_textWidth;
    KConfig *m_pConfig;
    bool m_bDeleteConfig;
    QVector<bool> m_bDrawBackgroundPerScreen; // m_bDrawBackgroundPerScreen[desk]
};


#endif // __BGSettings_h_Included__

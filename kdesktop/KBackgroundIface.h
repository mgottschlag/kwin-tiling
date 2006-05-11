/**
 * DCOP interface for the background manager.
 */

#ifndef __KBackgroundIface_h__
#define __KBackgroundIface_h__

#include <dcopobject.h>
#include <QColor>

class KBackgroundIface : virtual public DCOPObject
{
    K_DCOP
public:

k_dcop:
    /** Reread the configuration */
    virtual void configure() = 0;

    /** Enable/disable export of the background pixmap. */
    virtual void setExport(int xport) = 0;

    /** Returns the export desktop pixmap state. */
    virtual bool isExport() = 0;

    /** Enable/disable common desktop background. */
    virtual void setCommon(int common) = 0;

    /** Returns the common desktop background state. */
    virtual bool isCommon() = 0;

    /** Change caching behaviour.
     * @param bLimit Limit cache if not equal to zero.
     * @param size Cache size in kilobytes. */
    virtual void setCache(int bLimit, int size) = 0;

    /** Change the wallpaper.
     * @param desk desktop number, or 0 for the current visible desktop.
     * @param wallpaper The (local) path to the wallpaper.
     * @param mode The tiling mode. */
    virtual void setWallpaper(int desk, QString wallpaper, int mode) = 0;

    /** Change the wallpaper.
     * @param wallpaper The (local) path to the wallpaper.
     * @param mode The tiling mode. */
    virtual void setWallpaper(QString wallpaper, int mode) = 0;

    /** Set color.
     * @param c The color.
     * @param isColorA true for foreground and false for background color. */
    virtual void setColor(const QColor &c, bool isColorA) = 0;

    /** Change the wallpaper in "multi mode". */
    virtual void changeWallpaper() = 0;

    /** Enable/disable desktop background. */
    virtual void setBackgroundEnabled(const bool enable) = 0;

    /** Return the current wallpaper for specified desk.
     * @param desk desktop number, or 0 for the current visible desktop.
     */
    virtual QString currentWallpaper( int desk ) = 0;

    /** Return the wallpaper list for specified desk.
     * @param desk desktop number, or 0 for the current visible desktop.
     */
    virtual QStringList wallpaperList(int desk) = 0;

    /** Return the wallpaper files for specified desk.
     * @param desk desktop number, or 0 for the current visible desktop.
     */
    virtual QStringList wallpaperFiles(int desk) = 0;
};

#endif // __KBackgroundIface_h__
